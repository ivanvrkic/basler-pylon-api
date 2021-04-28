/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2016 UniZG, Zagreb. All rights reserved.
 * (c) 2016 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingPointCloud.cpp
  \brief  Point cloud processing.

  Functions for point cloud processing.

  \author Tomislav Petkovic
  \date   2019-07-05
*/

#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGPOINTCLOUD_CPP
#define __BATCHACQUISITIONPROCESSINGPOINTCLOUD_CPP

#include "BatchAcquisitionProcessingPointCloud.h"

#pragma intrinsic(sqrt)


//! Finds center of mass of a point cloud.
/*!
  Function computes center of mass of a point cloud. Note that
  center of mass may be badly affected by outliers in the data.

  \param points Pointer to array holding point coordinates.
  \param cm_out Pointer where found center of mass will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
PointCloudCenterOfMass(
                       cv::Mat * const points,
                       cv::Mat * const cm_out
                       )
{
  assert( (NULL != points) && (NULL != points->data) );
  if ( (NULL == points) || (NULL == points->data) ) return false;

  assert(NULL != cm_out);
  if (NULL == cm_out) return false;

  int const points_type = points->type();
  int const points_depth = CV_MAT_DEPTH(points_type);
  assert( 1 == CV_MAT_CN(points_type) );
  if (1 != CV_MAT_CN(points_type) ) return false;

  bool result = true;

  int const N = points->rows;
  int const D = points->cols;

  // Re-allocate output.
  cm_out->create(1, D, CV_64F);
  double * const dst = (double *)( (BYTE *)cm_out->data + 0 * cm_out->step[0] );
  for (int i = 0; i < D; ++i) dst[i] = 0;

  // Compute center of mass.
  switch (points_depth)
    {
    case CV_32F:
      {
        for (int i = 0; i < N; ++i)
          {
            float const * const src = (float *)( (BYTE *)points->data + i * points->step[0] );
            for (int j = 0; j < D; ++j)
              {
                dst[j] += (double)( src[j] );
              }
            /* for */
          }
        /* for */
        double const w = 1.0 / (double)( N );
        for (int i = 0; i < D; ++i) dst[i] *= w;
      }
      break;

    case CV_64F:
      {
        for (int i = 0; i < N; ++i)
          {
            double const * const src = (double *)( (BYTE *)points->data + i * points->step[0] );
            for (int j = 0; j < D; ++j)
              {
                dst[j] += src[j];
              }
            /* for */
          }
        /* for */
        double const w = 1.0 / (double)( N );
        for (int i = 0; i < D; ++i) dst[i] *= w;
      }
      break;

    default:
      result = false;
    }
  /* switch */

  return result;
}
/* PointCloudCenterOfMass */



//! Finds median of a point cloud.
/*!
  Function computes median of a point cloud using Weiszfeld's algorithm.
  Note that median is not affected of the number of outliers in the data is limited.

  \param points Pointer to array holding point coordinates.
  \param md_out Pointer where found median will be stored.
  \param cm_out Pointer where found center of mass will be stored.
  \param niter_stop Maximal number of iterations.
  \param dst_stop Minimal distance between two successive median estimates.
  \return Function returns true if successfull, false otherwise.
*/
bool
PointCloudWeiszfeld(
                    cv::Mat * const points,
                    cv::Mat * const md_out,
                    cv::Mat * const cm_out,
                    int niter_stop,
                    int dst_stop
                    )
{
  assert( (NULL != points) && (NULL != points->data) );
  if ( (NULL == points) || (NULL == points->data) ) return false;

  assert(NULL != md_out);
  if (NULL == md_out) return false;

  int const points_type = points->type();
  int const points_depth = CV_MAT_DEPTH(points_type);

  assert( 1 == CV_MAT_CN(points_type) );
  if (1 != CV_MAT_CN(points_type) ) return false;

  assert( CV_64F == points_depth );
  if ( CV_64F != points_depth ) return false;

  bool result = true;

  int const N = points->rows;
  int const D = points->cols;

  // Get center of mass.
  cv::Mat cm;
  bool const get_cm = PointCloudCenterOfMass(points, &cm);

  // Declare matrices we need.
  cv::Mat tmp1, tmp2;
  cv::Mat distances, distances_inv;
  cv::Mat mask, num, den;
  cv::Mat md, md_prev;
  cv::Mat dst;

  // Starting median is the center of mass.
  cm.copyTo(md);

  // Check stopping conditions.
  if (0 >= niter_stop) niter_stop = 25;
  if (0 >= dst_stop) dst_stop = 1;

  int niter = 0;
  bool converged = false;

  while (false == converged)
    {
      // Use more efficient code for the case of three dimensions.
      if (3 == D)
        {
          BYTE * const ptr_points = (BYTE *)( points->data );
          int const step_points = (int)( points->step[0] );

          double acc = 0.0;

          double acc_x = 0.0;
          double acc_y = 0.0;
          double acc_z = 0.0;

          double const mx = md.at<double>(0, 0);
          double const my = md.at<double>(0, 1);
          double const mz = md.at<double>(0, 2);

          for (int j = 0; j < N; ++j)
            {
              double const * const ptr_row_points = (double *)( ptr_points + j * step_points );

              double const x = ptr_row_points[0];
              double const y = ptr_row_points[1];
              double const z = ptr_row_points[2];

              double const dx = x - mx;
              double const dy = y - my;
              double const dz = z - mz;

              double const sum2 = dx * dx + dy * dy + dz * dz;
              double const d = sqrt(sum2);
              double const d_inv = 1.0 / d;

              if (FLT_EPSILON < d)
                {
                  acc += d_inv;

                  acc_x += x * d_inv;
                  acc_y += y * d_inv;
                  acc_z += z * d_inv;
                }
              /* if */
            }
          /* for */

          den.create(1, 1, CV_64FC1);
          den.at<double>(0, 0) = acc;

          num.create(1, 3, CV_64FC1);
          num.at<double>(0, 0) = acc_x;
          num.at<double>(0, 1) = acc_y;
          num.at<double>(0, 2) = acc_z;
        }
      else
        {
          // Compute Euclidean distances.
          points->copyTo(tmp1);
          for (int i = 0; i < D; ++i) tmp1.col(i) = tmp1.col(i) - md.at<double>(0, i);
          pow(tmp1, 2, tmp2);
          reduce(tmp2, tmp1, 1, CV_REDUCE_SUM);
          cv::sqrt(tmp1, distances);

          // Invert Euclidean distances.
          distances_inv = 1.0 / distances;

          // Compute mask.
          mask = abs(distances) > FLT_EPSILON;

          // Compute denominator.
          tmp1 = cv::Mat::zeros(1, 1, CV_64F);
          distances_inv.copyTo(tmp1, mask);
          reduce(tmp1, den, 0, CV_REDUCE_SUM);

          // Compute numerator.
          points->copyTo(tmp2);
          for (int i = 0; i < D; ++i) tmp2.col(i) = tmp2.col(i).mul( tmp1.col(0) );
          reduce(tmp2, num, 0, CV_REDUCE_SUM);
        }
      /* if */

      // Apply iterative map.
      niter = niter + 1;
      md.copyTo(md_prev);
      md = num / den;

      // Test for convergence.
      absdiff(md, md_prev, dst);
      double maxVal = 0;
      minMaxLoc(dst, NULL, &maxVal, NULL, NULL);
      if (maxVal < FLT_EPSILON)
        {
          converged = true;
        }
      else
        {
          pow(dst, 2, tmp1);
          reduce(tmp1, tmp2, 1, CV_REDUCE_SUM);
          cv::sqrt(tmp2, tmp1);
          converged = (niter > niter_stop) || (tmp1.at<double>(0,0) < dst_stop);
        }
      /* if */
    }
  /* while */

  // Assign outputs.
  if (NULL != md_out) md.copyTo(*md_out);
  if (NULL != cm_out) cm.copyTo(*cm_out);

  return result;
}
/* PointCloudWeiszfeld */



//! Counts points inside a sphere.
/*!
  Counts points inside a sphere.
  Sphere equation is (x - cx) ^ 2 + (y - cy) ^ 2 + (z - cz) ^ 2 = r ^ 2.

  \param points Pointer to cv::Mat which holds point coordinates in three columns.
  \param cx     X coordinate of sphere center.
  \param cy     Y coordinate of sphere center.
  \param cz     Z coordinate of sphere center.
  \param r      Sphere radius.
  \param inside_out     Address where coordinates of points inside a sphere will be copied.
  May be NULL.
  \return Returns number of points inside a sphere.
*/
int
PointCloundInsideASphere(
                         cv::Mat * const points,
                         double const cx,
                         double const cy,
                         double const cz,
                         double const r,
                         cv::Mat * const inside_out
                         )
{
  assert(NULL != points);
  if (NULL == points) return 0;

  int const points_type = points->type();
  int const points_depth = CV_MAT_DEPTH(points_type);

  assert( 1 == CV_MAT_CN(points_type) );
  if (1 != CV_MAT_CN(points_type) ) return false;

  assert( CV_64F == points_depth );
  if ( CV_64F != points_depth ) return false;

  assert(3 == points->cols);
  if (3 != points->cols) return 0;

  int const N = points->rows;

  int count = 0;
  double const r2 = r * r;

  if (NULL == inside_out)
    {
      for (int i = 0; i < N; ++i)
        {
          double const * const pt = (double *)( (BYTE *)points->data + i * points->step[0] );
          double const dx = pt[0] - cx;
          double const dy = pt[1] - cy;
          double const dz = pt[2] - cz;
          double const d2 = dx * dx + dy * dy + dz * dz;
          if (d2 < r2) ++count;
        }
      /* for */
    }
  else
    {
      cv::Mat inside;
      inside.create(N, 3, points_depth);

      for (int i = 0; i < N; ++i)
        {
          double const * const pt = (double *)( (BYTE *)points->data + i * points->step[0] );
          double const dx = pt[0] - cx;
          double const dy = pt[1] - cy;
          double const dz = pt[2] - cz;
          double const d2 = dx * dx + dy * dy + dz * dz;
          if (d2 < r2)
            {
              double * const dst = (double *)( (BYTE *)inside.data + count * inside.step[0] );
              dst[0] = pt[0];
              dst[1] = pt[1];
              dst[2] = pt[2];
              ++count;
            }
          /* if */
        }
      /* for */

      cv::Range r[2] = {cv::Range(0, count), cv::Range::all()};
      *inside_out = cv::Mat(inside, r);
    }
  /* if */

  return count;
}
/* PointCloundInsideASphere */



//! Counts points in front of a plane.
/*!
  Counts points in front of a plane.
  The plane equation is A * x + B * y + C * z + D = 0.

  \param points Pointer to cv::Mat which holds point coordinates in three columns.
  \param A      First plane coefficient.
  \param B      Second plane coefficient.
  \param C      Third plane coefficient.
  \param D      Fourth plane coefficient.
  \param in_front_out   Address where coordinates of points inside a sphere will be copied.
  May be NULL.
  \return Returns number of points in front of a plane.
*/
int
PointCloudInFrontOfAPlane(
                          cv::Mat * const points,
                          double const A,
                          double const B,
                          double const C,
                          double const D,
                          cv::Mat * const in_front_out
                          )
{
  assert(NULL != points);
  if (NULL == points) return 0;

  int const points_type = points->type();
  int const points_depth = CV_MAT_DEPTH(points_type);

  assert( 1 == CV_MAT_CN(points_type) );
  if (1 != CV_MAT_CN(points_type) ) return false;

  assert( CV_64F == points_depth );
  if ( CV_64F != points_depth ) return false;

  assert(3 == points->cols);
  if (3 != points->cols) return 0;

  int const N = points->rows;
  int count = 0;

  if (NULL == in_front_out)
    {
      for (int i = 0; i < N; ++i)
        {
          double const * const pt = (double *)( (BYTE *)points->data + i * points->step[0] );
          double const d = A * pt[0] + B * pt[1] + C * pt[2] + D;
          if (d < 0) ++count;
        }
      /* for */
    }
  else
    {
      cv::Mat in_front;
      in_front.create(N, 3, points_depth);

      for (int i = 0; i < N; ++i)
        {
          double const * const pt = (double *)( (BYTE *)points->data + i * points->step[0] );
          double const d = A * pt[0] + B * pt[1] + C * pt[2] + D;
          if (d < 0)
            {
              double * const dst = (double *)( (BYTE *)in_front.data + count * in_front.step[0] );
              dst[0] = pt[0];
              dst[1] = pt[1];
              dst[2] = pt[2];
              ++count;
            }
          /* if */
        }
      /* for */

      cv::Range r[2] = {cv::Range(0, count), cv::Range::all()};
      *in_front_out = cv::Mat(in_front, r);
    }
  /* if */

  return count;
}
/* PointCloudInFrontOfAPlane */



//! Save point cloud to PLY.
/*!
  Function saves multiple point clouds to PLY format.

  \param filename     Filename where to store the point cloud.
  \param points       Vector of pointers to point clouds. Elements of this vector cannot be NULL.
  \param colors       Vector of pointers to color data. Elements of this vector may be NULL if color is not available.
  \param normals      Vector of pointers to normal data. Elements of this vector may be NULL if normals are not available.
  \return Function returns true if successfull, false otherwise.
*/
bool
PointCloudSaveToPLY(
                    wchar_t const * const filename,
                    std::vector<cv::Mat * const> & points,
                    std::vector<cv::Mat * const> & colors,
                    std::vector<cv::Mat * const> & normals
                    )
{
  bool saved = false;

  assert(NULL != filename);
  if (NULL == filename) return saved;

  size_t const M = points.size();
  assert(0 < M);
  if (0 >= M) return saved;

  assert( (M == colors.size()) && (M == normals.size()) );
  if ( (M != colors.size()) || (M != normals.size()) ) return saved;

  int N_all = 0; // Total number of points in all clouds.
  bool have_all_colors = true; // Colors may be saved only if all point clouds have color.
  bool have_all_normals = true; // Normals may be saved only if all points have normals.

  for (size_t i = 0; i < M; ++i)
    {
      cv::Mat * const pts = points[i];
      cv::Mat * const clr = colors[i];
      cv::Mat * const nrm = normals[i];

      assert(NULL != pts);
      if (NULL == pts) return saved;

      assert(3 == pts->cols);
      if ( (NULL == pts->data) || (3 != pts->cols) || (0 >= pts->rows) ) return saved;

      int const points_type = pts->type();
      int const points_depth = CV_MAT_DEPTH(points_type);
      assert( (1 == CV_MAT_CN(points_type)) && (CV_32F == points_depth) );
      if ( (1 != CV_MAT_CN(points_type)) || (CV_32F != points_depth) ) return saved;

      int const N = (int)(pts->rows);
      assert(0 < N);

      bool have_colors = false;
      if (NULL != clr)
        {
          assert(NULL != clr->data);
          if (NULL == clr->data) return saved;

          assert(N == (int)(clr->rows));
          if (N != (int)(clr->rows)) return saved;

          assert( (3 == clr->cols) != (4 == clr->cols) );
          if ( (3 != clr->cols) && (4 != clr->cols) ) return saved;

          int const colors_type = clr->type();
          int const colors_depth = CV_MAT_DEPTH(colors_type);

          assert( (1 == CV_MAT_CN(colors_type)) && (CV_8U == colors_depth) );
          if ( (1 != CV_MAT_CN(colors_type)) || (CV_8U != colors_depth) ) return saved;

          have_colors = true;
        }
      /* if */

      bool have_normals = false;
      if (NULL != nrm)
        {
          assert(NULL != nrm->data);
          if (NULL == nrm->data) return saved;

          assert( (N == (int)(nrm->rows)) && (3 == nrm->cols) );
          if ( (N != (int)(nrm->rows)) || (3 != nrm->cols) ) return saved;

          int const normals_type = nrm->type();
          int const normals_depth = CV_MAT_DEPTH(normals_type);

          assert( (1 == CV_MAT_CN(normals_type)) && (CV_32F == normals_depth) );
          if ( (1 != CV_MAT_CN(normals_type)) || (CV_32F != normals_depth) ) return saved;

          have_normals = true;
        }
      /* if */

      N_all = N_all + N;
      have_all_colors = have_all_colors && have_colors;
      have_all_normals = have_all_normals && have_normals;
    }
  /* for */

  FILE * FP = NULL;
  errno_t const open = _wfopen_s(&FP, filename, L"wb");
  assert( (0 == open) && (NULL != FP) );

  if (NULL != FP)
    {
      saved = true;

      fprintf(
              FP,
              "ply\n"
              "format %s 1.0\n"
              "comment BatchAcquisition.exe " __DATE__ " " __TIME__ "\n",
              "binary_little_endian"
              );

      fprintf(FP, "element vertex %d\n", N_all);

      fprintf(
              FP,
              "property float x\n"
              "property float y\n"
              "property float z\n"
              );

      if (true == have_all_normals)
        {
          fprintf(
                  FP,
                  "property float nx\n"
                  "property float ny\n"
                  "property float nz\n"
                  );
        }
      /* if */

      if (true == have_all_colors)
        {
          fprintf(
                  FP,
                  "property uchar red\n"
                  "property uchar green\n"
                  "property uchar blue\n"
                  );
        }
      /* if */

      fprintf(FP, "element face 0\n");
      fprintf(FP, "property list uchar int vertex_indices\n");
      fprintf(FP, "end_header\n");

      BYTE * buffer = NULL;
      size_t row_sz = 0;
      if ( (false == have_all_normals) && (false == have_all_colors) )
        {
          row_sz = 3 * sizeof(float);
          buffer = new BYTE[row_sz * N_all];
          assert(NULL != buffer);
          if (NULL != buffer)
            {
              int idx = 0;
              for (size_t i = 0; i < M; ++i)
                {
                  cv::Mat * const pts = points[i];
                  int const N = (int)(pts->rows);

                  for (int j = 0; j < N; ++j, ++idx)
                    {
                      float const * const src = (float *)( (BYTE *)( pts->data ) + j * pts->step[0] );
                      float * const dst = (float *)( buffer + idx * row_sz );

                      dst[0] = src[0];
                      dst[1] = src[1];
                      dst[2] = src[2];
                    }
                  /* for */
                }
              /* for */
              assert(idx == N_all);
            }
          /* if */
        }
      else if ( (true == have_all_normals) && (false == have_all_colors) )
        {
          row_sz = 3 * sizeof(float) + 3 * sizeof(float);
          buffer = new BYTE[row_sz * N_all];
          assert(NULL != buffer);
          if (NULL != buffer)
            {
              int idx = 0;
              for (size_t i = 0; i < M; ++i)
                {
                  cv::Mat * const pts = points[i];
                  cv::Mat * const nrm = normals[i];
                  int const N = (int)(pts->rows);
                  assert(N == (int)(nrm->rows));

                  for (int j = 0; j < N; ++j, ++idx)
                    {
                      float const * const src_points = (float *)( (BYTE *)( pts->data ) + j * pts->step[0] );
                      float const * const src_normals = (float *)( (BYTE *)( nrm->data ) + j * nrm->step[0] );
                      float * const dst = (float *)( buffer + idx * row_sz );

                      dst[0] = src_points[0];
                      dst[1] = src_points[1];
                      dst[2] = src_points[2];

                      dst[3] = src_normals[0];
                      dst[4] = src_normals[1];
                      dst[5] = src_normals[2];
                    }
                  /* for */
                }
              /* for */
              assert(idx == N_all);
            }
          /* if */
        }
      else if ( (false == have_all_normals) && (true == have_all_colors) )
        {
          row_sz = 3 * sizeof(float) + 3 * sizeof(unsigned char);
          buffer = new BYTE[row_sz * N_all];
          assert(NULL != buffer);
          if (NULL != buffer)
            {
              int idx = 0;
              for (size_t i = 0; i < M; ++i)
                {
                  cv::Mat * const pts = points[i];
                  cv::Mat * const clr = colors[i];
                  int const N = (int)(pts->rows);
                  assert(N == (int)(clr->rows));

                  for (int j = 0; j < N; ++j, ++idx)
                    {
                      float const * const src_points = (float *)( (BYTE *)( pts->data ) + j * pts->step[0] );
                      unsigned char const * const src_colors = (unsigned char *)( (BYTE *)( clr->data ) + j * clr->step[0] );

                      float * const dst_points = (float *)( buffer + idx * row_sz );
                      unsigned char * const dst_colors = (unsigned char *)( buffer + idx * row_sz + 3 * sizeof(float) );

                      dst_points[0] = src_points[0];
                      dst_points[1] = src_points[1];
                      dst_points[2] = src_points[2];

                      dst_colors[0] = src_colors[0];
                      dst_colors[1] = src_colors[1];
                      dst_colors[2] = src_colors[2];
                    }
                  /* for */
                }
              /* for */
              assert(idx == N_all);
            }
          /* if */
        }
      else if ( (true == have_all_normals) && (true == have_all_colors) )
        {
          row_sz = 3 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(unsigned char);
          buffer = new BYTE[row_sz * N_all];
          assert(NULL != buffer);
          if (NULL != buffer)
            {
              int idx = 0;
              for (size_t i = 0; i < M; ++i)
                {
                  cv::Mat * const pts = points[i];
                  cv::Mat * const nrm = normals[i];
                  cv::Mat * const clr = colors[i];
                  int const N = (int)(pts->rows);
                  assert(N == (int)(nrm->rows));
                  assert(N == (int)(clr->rows));

                  for (int j = 0; j < N; ++j, ++idx)
                    {
                      float const * const src_points = (float *)( (BYTE *)( pts->data ) + j * pts->step[0] );
                      float const * const src_normals = (float *)( (BYTE *)( nrm->data ) + j * nrm->step[0] );
                      unsigned char const * const src_colors = (unsigned char *)( (BYTE *)( clr->data ) + j * clr->step[0] );

                      float * const dst = (float *)( buffer + idx * row_sz );
                      unsigned char * const dst_colors = (unsigned char *)( buffer + idx * row_sz + 6 * sizeof(float) );

                      dst[0] = src_points[0];
                      dst[1] = src_points[1];
                      dst[2] = src_points[2];

                      dst[3] = src_normals[0];
                      dst[4] = src_normals[1];
                      dst[5] = src_normals[2];

                      dst_colors[0] = src_colors[0];
                      dst_colors[1] = src_colors[1];
                      dst_colors[2] = src_colors[2];
                    }
                  /* for */
                }
              /* for */
              assert(idx == N_all);
            }
          /* if */
        }
      /* if */

      if (NULL != buffer)
        {
          size_t const cnt = fwrite(buffer, row_sz, N_all, FP);
          saved = saved && (cnt == N_all);
        }
      /* if */

      SAFE_DELETE_ARRAY( buffer );

      fclose(FP);
      FP = NULL;
    }
  /* if */

  return saved;
}
/* PointCloudSaveToPLY */



#endif /* !__BATCHACQUISITIONPROCESSINGPOINTCLOUD_CPP */
