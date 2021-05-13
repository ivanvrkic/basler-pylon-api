/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2015-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingTriangulation.cpp
  \brief  Point triangulation.

  This file contains functions for triangulation between camera and projector.
  Auxiliary functions for loading camera and projector geometric calibration
  are also defined here.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2017-06-16
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGTRIANGULATION_CPP
#define __BATCHACQUISITIONPROCESSINGTRIANGULATION_CPP


#include "BatchAcquisitionProcessingTriangulation.h"
#include "BatchAcquisitionProcessingPixelSelector.h"

#pragma intrinsic(sqrt)


/****** INLINE HELPER FUNCTIONS ******/

//! Validates row array.
/*!
  Input coordinates must be stored in a row array.

  \param depth  Matrix datatype.
  \param x      Row array.
  \return Function returns true if input is a row array of specified datatype.
*/
inline
bool
CheckRowArray_inline(
                     int const depth,
                     cv::Mat * const x
                     )
{
  //assert( (NULL != x) && ( (NULL != x->data) != ((0 == x->cols) || (0 == x->rows)) ) );
  if ( (NULL == x) || ( (NULL == x->data) == ((0 < x->cols) && (0 < x->rows)) ) ) return false;

  assert( (1 == x->rows) && (0 <= x->cols) );
  if ( (1 != x->rows) || (0 > x->cols) ) return false;

  assert( depth == CV_MAT_DEPTH(x->type()) );
  if ( depth != CV_MAT_DEPTH(x->type()) ) return false;

  return true;
}
/* CheckRowArray_inline */



//! Validates coefficient arrays.
/*!
  Validates up to four array of coefficients.
  All arrays must have same the type and number of elements as the first array
  for the validation to pass.

  \param depth   Array datatype.
  \param a1      First array.
  \param a2      Second array; may be NULL.
  \param a3      Third array; may be NULL.
  \param a4      Fourth array; may be NULL.
  \return Function returns the number of valid arrays or 0 if the first array is NULL or -1 if any of arrays is invalid.
*/
inline
int
CheckRowArrays_inline(
                      int const depth,
                      cv::Mat * const a1,
                      cv::Mat * const a2 = NULL,
                      cv::Mat * const a3 = NULL,
                      cv::Mat * const a4 = NULL
                      )
{
  int num_valid = 0;

  // Check first array.
  bool const valid1 = CheckRowArray_inline(depth, a1);
  if (true != valid1) return num_valid;
  ++num_valid;

  // Then match remaining arrays to first.
  if (NULL != a2)
    {
      bool const valid2 = CheckCoordinateArrays_inline(a1, a2, depth);
      if (false == valid2) return -1;
      ++num_valid;
    }
  /* if */

  if (NULL != a3)
    {
      bool const valid3 = CheckCoordinateArrays_inline(a1, a3, depth);
      if (false == valid3) return -1;
      ++num_valid;
    }
  /* if */

  if (NULL != a4)
    {
      bool const valid4 = CheckCoordinateArrays_inline(a1, a4, depth);
      if (false == valid4) return -1;
      ++num_valid;
    }
  /* if */

  return num_valid;
}
/* CheckRowArrays_inline */




/****** LINE-POINT INTERSECTIONS ******/

//! Find the closest point on a line (inline variant).
/*!
  Finds the closest point to a specified test point pt_in on
  a line defined by two points x and y.
  The point coordinates are stored in pt_out.

  This function assumes all point coordinates are normalized.

  \param x First line point.
  \param y Second line point.
  \param pt_in Test point.
  \param pt_out The closest point on a line to a test point t.
  \param t_out If set function will return the value of parameter t.
  \return Function returns true if successfull.
*/
inline
bool
ClosestPointOnLineFromPoint_inline(
                                   double_a_V3 const x,
                                   double_a_V3 const y,
                                   double_a_V3 const pt_in,
                                   double_a_V3 pt_out,
                                   double * const t_out
                                   )
{
  bool result = true;

  /* First compute the plane normal (or line direction). */
  double const a = y[0] - x[0];
  double const b = y[1] - x[1];
  double const c = y[2] - x[2];

  double const d = a*a + b*b + c*c;

  /* Then compute the parameter t. The point is determined by the value of the
     parameter t. If d is zero solution cannot be obtained.
  */
  assert( FLT_EPSILON < d );
  if ( FLT_EPSILON < d )
    {
      double const t =
        a * ( pt_in[0] - x[0] ) +
        b * ( pt_in[1] - x[1] ) +
        c * ( pt_in[2] - x[2] );
      double const m = t / d;

      pt_out[0] = a * m + x[0];
      pt_out[1] = b * m + x[1];
      pt_out[2] = c * m + x[2];

      if (NULL != t_out) *t_out = m;
    }
  else
    {
      pt_out[0] = pt_in[0];
      pt_out[1] = pt_in[1];
      pt_out[2] = pt_in[2];
      result = false;
    }
  /* if */

  return result;
}
/* ClosestPointOnLineFromPoint_inline */




//! Find the closest point on a line.
/*!
  Finds the closest point to a specified test point pt_in on
  a line defined by two points x and y.
  The point coordinates are stored in pt_out.

  \see ClosestPointOnLineFromPoint_inline

  \param x First line point.
  \param y Second line point.
  \param pt_in Test point.
  \param pt_out The closest point on a line to a test point t.
  \return Function returns true if successfull.
*/
bool
ClosestPointOnLineFromPoint(
                            double_a_V3 const x,
                            double_a_V3 const y,
                            double_a_V3 const pt_in,
                            double_a_V3 pt_out
                            )
{
  bool const result = ClosestPointOnLineFromPoint_inline(x, y, pt_in, pt_out, NULL);
  assert(true == result);
  return result;
}
/* ClosestPointOnLineFromPoint */



//! Return distance along line from point.
/*!
  Finds the distance between point x on a line and the point closest
  to the point pt_in on the same line, i.e. it computes the distance
  between line origin x and orthogonal projection of pt_in on the line.

  \param x      First line point.
  \param y      Second line point.
  \param pt_in  Test point.
  \param dst    Distance in v = y - x units.
  If the distance between x and y is normalized to one then dst is true distance.
  \return Returns true if successfull.
*/
bool
DistanceAlongLineFromPoint(
                           double_a_V3 const x,
                           double_a_V3 const y,
                           double_a_V3 const pt_in,
                           double * const dst
                           )
{
  double_a_V3 pt_out; // Unused.
  bool const result = ClosestPointOnLineFromPoint_inline(x, y, pt_in, pt_out, dst);
  assert(true == result);
  return result;
}
/* DistanceAlongLineFromPoint */




/****** PLANE-RAY INTERSECTIONS ******/

//! Plane-ray intersections.
/*!
  Computes plane-ray intersections for given plane equations are ray directions.
  All rays are assumed to pass through common point pt.
  If intersection is ill-defined or does not exist then the corresponding
  output coordinates are set to NaNs.

  \param A      First plane equation coefficient (multiplies x).
  \param B      Second plane equation coefficient (multiplies y).
  \param C      Third plane equation coefficient (multiplies z).
  \param D      Fourth plane equation coefficient (free).
  \param vx     Ray direction vector components along x coordinate.
  \param vy     Ray direction vector components along y coordinate.
  \param vz     Ray direction vector components along z coordinate.
  \param pt     Common point for all rays (camera center).
  \param x_out  Address where x coordinates of intersection points will be stored.
  \param y_out  Address where y coordinates of intersection points will be stored.
  \param z_out  Address where z coordinates of intersection points will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
PlaneRayIntersection(
                     cv::Mat * const A,
                     cv::Mat * const B,
                     cv::Mat * const C,
                     cv::Mat * const D,
                     cv::Mat * const vx,
                     cv::Mat * const vy,
                     cv::Mat * const vz,
                     cv::Mat * const pt,
                     cv::Mat * * const x_out,
                     cv::Mat * * const y_out,
                     cv::Mat * * const z_out
                     )
{
  int const valid_plane = CheckRowArrays_inline(CV_64F, A, B, C, D);
  assert(4 == valid_plane);
  if (4 != valid_plane) return false;

  int const valid_ray = CheckRowArrays_inline(CV_64F, A, vx, vy, vz);
  assert(4 == valid_ray);
  if (4 != valid_ray) return false;

  bool const valid_pt = CheckRowArray_inline(CV_64F, pt);
  assert(true == valid_pt);
  if (true != valid_pt) return false;

  int const N = A->cols;

  bool result = true; // Assume success.

  // Allocate outputs.
  cv::Mat * x = new cv::Mat(1, N, CV_64F);
  assert(NULL != x);

  cv::Mat * y = new cv::Mat(1, N, CV_64F);
  assert(NULL != y);

  cv::Mat * z = new cv::Mat(1, N, CV_64F);
  assert(NULL != z);

  if ( (NULL == x) || (NULL == y) || (NULL == z) )
    {
      result = false;
      goto PlaneRayIntersection_EXIT;
    }
  /* if */

  // Get row pointers.
  double const * const row_A = (double *)( (BYTE *)(A->data) + A->step[0] * 0 );
  double const * const row_B = (double *)( (BYTE *)(B->data) + B->step[0] * 0 );
  double const * const row_C = (double *)( (BYTE *)(C->data) + C->step[0] * 0 );
  double const * const row_D = (double *)( (BYTE *)(D->data) + D->step[0] * 0 );
  double const * const row_vx = (double *)( (BYTE *)(vx->data) + vx->step[0] * 0 );
  double const * const row_vy = (double *)( (BYTE *)(vy->data) + vy->step[0] * 0 );
  double const * const row_vz = (double *)( (BYTE *)(vz->data) + vz->step[0] * 0 );
  double * const row_x = (double *)( (BYTE *)(x->data) + x->step[0] * 0 );
  double * const row_y = (double *)( (BYTE *)(y->data) + y->step[0] * 0 );
  double * const row_z = (double *)( (BYTE *)(z->data) + z->step[0] * 0 );

  // Get camera center.
  double const cx = pt->at<double>(0);
  double const cy = pt->at<double>(1);
  double const cz = pt->at<double>(2);

  // Compute intersections.
  for (int i = 0; i < N; ++i)
    {
      double const det = row_A[i] * row_vx[i] + row_B[i] * row_vy[i] + row_C[i] * row_vz[i];
      if ( (FLT_EPSILON < det) || (det < -FLT_EPSILON) )
        {
          double const t = -(row_A[i] * cx + row_B[i] * cy + row_C[i] * cz + row_D[i]) / det;

          row_x[i] = cx + t * row_vx[i];
          row_y[i] = cy + t * row_vy[i];
          row_z[i] = cz + t * row_vz[i];
        }
      else
        {
          row_x[i] = BATCHACQUISITION_qNaN_dv;
          row_y[i] = BATCHACQUISITION_qNaN_dv;
          row_z[i] = BATCHACQUISITION_qNaN_dv;
        }
      /* if */
    }
  /* for */


  SAFE_ASSIGN_PTR( x, x_out );
  SAFE_ASSIGN_PTR( y, y_out );
  SAFE_ASSIGN_PTR( z, z_out );

 PlaneRayIntersection_EXIT:

  SAFE_DELETE( x );
  SAFE_DELETE( y );
  SAFE_DELETE( z );

  return result;
}
/* PlaneRayIntersection */



//! Ray-ray intersection.
/*!
  Function computes shortest segments between two rays and returns the midpoint
  of the segment and its squared length.

  \param vx1     First ray direction vector components along x coordinate.
  \param vy1     First ray direction vector components along y coordinate.
  \param vz1     First ray direction vector components along z coordinate.
  \param pt1     Common point for all rays (first camera center).
  \param vx2     Second ray direction vector components along x coordinate.
  \param vy2     Second ray direction vector components along y coordinate.
  \param vz2     Second ray direction vector components along z coordinate.
  \param pt2     Common point for all rays (second camera center).
  \param x_out  Address where x coordinates of intersection points will be stored.
  \param y_out  Address where y coordinates of intersection points will be stored.
  \param z_out  Address where z coordinates of intersection points will be stored.
  \param dst2_out Squared length of the shortest line segment that connects two rays.
  \return Function returns true if successfull, false otherwise.
*/
bool
RayRayIntersection(
                   cv::Mat * const vx1,
                   cv::Mat * const vy1,
                   cv::Mat * const vz1,
                   cv::Mat * const pt1,
                   cv::Mat * const vx2,
                   cv::Mat * const vy2,
                   cv::Mat * const vz2,
                   cv::Mat * const pt2,
                   cv::Mat * * const x_out,
                   cv::Mat * * const y_out,
                   cv::Mat * * const z_out,
                   cv::Mat * * const dst2_out
                   )
{
  int const valid_ray_1 = CheckRowArrays_inline(CV_64F, vx1, vy1, vz1);
  assert(3 == valid_ray_1);
  if (3 != valid_ray_1) return false;

  bool const valid_pt_1 = CheckRowArray_inline(CV_64F, pt1);
  assert(true == valid_pt_1);
  if (true != valid_pt_1) return false;

  int const valid_ray_2 = CheckRowArrays_inline(CV_64F, vx1, vx2, vy2, vz2);
  assert(4 == valid_ray_2);
  if (4 != valid_ray_2) return false;

  bool const valid_pt_2 = CheckRowArray_inline(CV_64F, pt2);
  assert(true == valid_pt_2);
  if (true != valid_pt_2) return false;

  int const N = vx1->cols;

  bool result = true; // Assume success.

  // Allocate outputs.
  cv::Mat * x = new cv::Mat(1, N, CV_64F);
  assert(NULL != x);

  cv::Mat * y = new cv::Mat(1, N, CV_64F);
  assert(NULL != y);

  cv::Mat * z = new cv::Mat(1, N, CV_64F);
  assert(NULL != z);

  cv::Mat * dst2 = new cv::Mat(1, N, CV_64F);
  assert(NULL != dst2);

  if ( (NULL == x) || (NULL == y) || (NULL == z) || (NULL == dst2) )
    {
      result = false;
      goto RayRayIntersection_EXIT;
    }
  /* if */

  // Get row pointers.
  double const * const row_vx1 = (double *)( (BYTE *)(vx1->data) + vx1->step[0] * 0 );
  double const * const row_vy1 = (double *)( (BYTE *)(vy1->data) + vy1->step[0] * 0 );
  double const * const row_vz1 = (double *)( (BYTE *)(vz1->data) + vz1->step[0] * 0 );

  double const * const row_vx2 = (double *)( (BYTE *)(vx2->data) + vx2->step[0] * 0 );
  double const * const row_vy2 = (double *)( (BYTE *)(vy2->data) + vy2->step[0] * 0 );
  double const * const row_vz2 = (double *)( (BYTE *)(vz2->data) + vz2->step[0] * 0 );

  double * const row_x = (double *)( (BYTE *)(x->data) + x->step[0] * 0 );
  double * const row_y = (double *)( (BYTE *)(y->data) + y->step[0] * 0 );
  double * const row_z = (double *)( (BYTE *)(z->data) + z->step[0] * 0 );
  double * const row_dst2 = (double *)( (BYTE *)(dst2->data) + dst2->step[0] * 0 );

  // Get camera centers.
  double const cx1 = pt1->at<double>(0);
  double const cy1 = pt1->at<double>(1);
  double const cz1 = pt1->at<double>(2);

  double const cx2 = pt2->at<double>(0);
  double const cy2 = pt2->at<double>(1);
  double const cz2 = pt2->at<double>(2);

  double const dx = cx1 - cx2;
  double const dy = cy1 - cy2;
  double const dz = cz1 - cz2;

  double const F = dx * dx + dy * dy + dz * dz;

  // Compute intersections.
  for (int i = 0; i < N; ++i)
    {
      double const A =         row_vx1[i] * row_vx1[i]  +  row_vy1[i] * row_vy1[i]  +  row_vz1[i] * row_vz1[i];
      double const C = 2.0 * ( row_vx1[i] * row_vx2[i]  +  row_vy1[i] * row_vy2[i]  +  row_vz1[i] * row_vz2[i] );
      double const E =         row_vx2[i] * row_vx2[i]  +  row_vy2[i] * row_vy2[i]  +  row_vz2[i] * row_vz2[i];

      double const det = C * C - 4.0 * A * E;

      if ( (FLT_EPSILON < det) || (det < -FLT_EPSILON) )
        {
          double const B =  2.0 * ( dx * row_vx1[i]  +  dy * row_vy1[i]  +  dz * row_vz1[i] );
          double const D = -2.0 * ( dx * row_vx2[i]  +  dy * row_vy2[i]  +  dz * row_vz2[i] );

          double const det_inv = 1.0 / det;

          double const t1 = ( 2.0 * B * E  +  C * D ) * det_inv;
          double const t2 = ( 2.0 * A * D  +  B * C ) * det_inv;

          row_dst2[i] = A * t1 * t1  +  B * t1  -  C * t1 * t2  +  D * t2  +  E * t2 * t2  +  F;

          double const x1 = cx1 + row_vx1[i] * t1;
          double const y1 = cy1 + row_vy1[i] * t1;
          double const z1 = cz1 + row_vz1[i] * t1;

          double const x2 = cx2 + row_vx2[i] * t2;
          double const y2 = cy2 + row_vy2[i] * t2;
          double const z2 = cz2 + row_vz2[i] * t2;

          row_x[i] = 0.5 * (x1 + x2);
          row_y[i] = 0.5 * (y1 + y2);
          row_z[i] = 0.5 * (z1 + z2);
        }
      else
        {
          row_x[i] = BATCHACQUISITION_qNaN_dv;
          row_y[i] = BATCHACQUISITION_qNaN_dv;
          row_z[i] = BATCHACQUISITION_qNaN_dv;
          row_dst2[i] = BATCHACQUISITION_qNaN_dv;
        }
      /* if */
    }
  /* for */


  SAFE_ASSIGN_PTR( x, x_out );
  SAFE_ASSIGN_PTR( y, y_out );
  SAFE_ASSIGN_PTR( z, z_out );
  SAFE_ASSIGN_PTR( dst2, dst2_out );

 RayRayIntersection_EXIT:

  SAFE_DELETE( x );
  SAFE_DELETE( y );
  SAFE_DELETE( z );
  SAFE_DELETE( dst2 );

  return result;
}
/* RayRayIntersection */




/****** RAY GENERATORS ******/

//! Get camera planes.
/*!
  Computes coefficients of all camera planes for input coordinates.
  Note that exactly one of x and y input coordinates must be defined and
  that other coordinate must be NULL.

  \param x      X coordinate; if non NULL then column planes are computed. Must be CV_64F.
  \param y      Y coordinate; if x is NULL and y is given then row planes are computed. Must be CV_64F.
  \param P      Pinhole camera geometry.
  \param A_out  Address where array of plane coefficients will be stored.
  \param B_out  Address where array of plane coefficients will be stored.
  \param C_out  Address where array of plane coefficients will be stored.
  \param D_out  Address where array of plane coefficients will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
GetCameraPlanes(
                cv::Mat * const x,
                cv::Mat * const y,
                ProjectiveGeometry * const P,
                cv::Mat * * const A_out,
                cv::Mat * * const B_out,
                cv::Mat * * const C_out,
                cv::Mat * * const D_out
                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const x_valid = CheckRowArray_inline(CV_64F, x);
  bool const y_valid = CheckRowArray_inline(CV_64F, y);
  assert( x_valid != y_valid );
  if (x_valid == y_valid) return false;

  bool result = true; // Assume success.

  int const N = (true == x_valid)? x->cols : y->cols;

  cv::Mat * A = new cv::Mat(1, N, CV_64F);
  assert(NULL != A);

  cv::Mat * B = new cv::Mat(1, N, CV_64F);
  assert(NULL != B);

  cv::Mat * C = new cv::Mat(1, N, CV_64F);
  assert(NULL != C);

  cv::Mat * D = new cv::Mat(1, N, CV_64F);
  assert(NULL != D);

  if ( (NULL == A) || (NULL == B) || (NULL == C) || (NULL == D) )
    {
      result = false;
      goto GetCameraPlanes_EXIT;
    }
  /* if */

  // Get row pointers.
  double * const row_A = (double *)( (BYTE *)(A->data) + A->step[0] * 0 );
  double * const row_B = (double *)( (BYTE *)(B->data) + B->step[0] * 0 );
  double * const row_C = (double *)( (BYTE *)(C->data) + C->step[0] * 0 );
  double * const row_D = (double *)( (BYTE *)(D->data) + D->step[0] * 0 );

  // Get projection matrix coefficients.
  double const pxA = P->projection[0][0];
  double const pxB = P->projection[0][1];
  double const pxC = P->projection[0][2];
  double const pxD = P->projection[0][3];

  double const pyA = P->projection[1][0];
  double const pyB = P->projection[1][1];
  double const pyC = P->projection[1][2];
  double const pyD = P->projection[1][3];

  double const phA = P->projection[2][0];
  double const phB = P->projection[2][1];
  double const phC = P->projection[2][2];
  double const phD = P->projection[2][3];

  // Compute plane coefficients.
  if (true == x_valid)
    {
      double const * const row_x = (double *)( (BYTE *)(x->data) + x->step[0] * 0 );
      for (int i = 0; i < N; ++i)
        {
          double const val_A = phA * row_x[i] - pxA;
          double const val_B = phB * row_x[i] - pxB;
          double const val_C = phC * row_x[i] - pxC;
          double const val_D = phD * row_x[i] - pxD;

          double const v2 = val_A * val_A + val_B * val_B + val_C * val_C;
          double const v = sqrt(v2);
          double const k = 1.0 / v;

          row_A[i] = k * val_A;
          row_B[i] = k * val_B;
          row_C[i] = k * val_C;
          row_D[i] = k * val_D;
        }
      /* for */
    }
  else
    {
      double const * const row_y = (double *)( (BYTE *)(y->data) + y->step[0] * 0 );
      for (int i = 0; i < N; ++i)
        {
          double const val_A = phA * row_y[i] - pyA;
          double const val_B = phB * row_y[i] - pyB;
          double const val_C = phC * row_y[i] - pyC;
          double const val_D = phD * row_y[i] - pyD;

          double const v2 = val_A * val_A + val_B * val_B + val_C * val_C;
          double const v = sqrt(v2);
          double const k = 1.0 / v;

          row_A[i] = k * val_A;
          row_B[i] = k * val_B;
          row_C[i] = k * val_C;
          row_D[i] = k * val_D;
        }
      /* for */
    }
  /* if */


  SAFE_ASSIGN_PTR( A, A_out );
  SAFE_ASSIGN_PTR( B, B_out );
  SAFE_ASSIGN_PTR( C, C_out );
  SAFE_ASSIGN_PTR( D, D_out );

 GetCameraPlanes_EXIT:

  SAFE_DELETE( A );
  SAFE_DELETE( B );
  SAFE_DELETE( C );
  SAFE_DELETE( D );

  return result;
}
/* GetCameraPlanes */



//! Get camera rays.
/*!
  Computes coefficients of all camera rays for input coordinates.
  Only directions of rays are computed. All rays pass through
  camera center.

  \param x      X coordinate in the image plane. Must be CV_64F.
  \param y      Y coordinate in the image plane. Must be CV_64F.
  \param PG      Pinhole camera geometry.
  \param vx_out  Address where x ray direction coefficients will be stored.
  \param vy_out  Address where y ray direction coefficients will be stored.
  \param vz_out  Address where z ray direction coefficients will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
GetCameraRays(
              cv::Mat * const x,
              cv::Mat * const y,
              ProjectiveGeometry * const PG,
              cv::Mat * * const vx_out,
              cv::Mat * * const vy_out,
              cv::Mat * * const vz_out
              )
{
  bool const valid = CheckCoordinateArrays_inline(x, y, CV_64F);
  if (true != valid) return valid;

  assert(NULL != PG);
  if (NULL == PG) return false;

  bool result = true; // Assume success.

  // Compute pseudoinverse of the projection matrix.
  cv::Mat P(3, 4, CV_64F, &(PG->projection[0][0]), 4 * sizeof(double));
  cv::Mat PT = P.t();
  cv::Mat PPT = P * PT;
  cv::Mat Pinv = PT * PPT.inv(cv::DECOMP_SVD);

  // Preallocate outputs.
  int const N = x->cols;
  assert(1 == x->rows);
  assert(N == y->cols);
  assert(1 == y->rows);

  cv::Mat * vx = new cv::Mat(1, N, CV_64F);
  assert(NULL != vx);

  cv::Mat * vy = new cv::Mat(1, N, CV_64F);
  assert(NULL != vy);

  cv::Mat * vz = new cv::Mat(1, N, CV_64F);
  assert(NULL != vz);

  if ( (NULL == vx) || (NULL == vy) || (NULL == vz) )
    {
      result = false;
      goto GetCameraRays_EXIT;
    }
  /* if */

  // Get row pointers.
  double const * const row_x = (double *)( (BYTE *)(x->data) + x->step[0] * 0 );
  double const * const row_y = (double *)( (BYTE *)(y->data) + x->step[0] * 0 );
  double * const row_vx = (double *)( (BYTE *)(vx->data) + vx->step[0] * 0 );
  double * const row_vy = (double *)( (BYTE *)(vy->data) + vy->step[0] * 0 );
  double * const row_vz = (double *)( (BYTE *)(vz->data) + vz->step[0] * 0 );

  // Get values of the pseudoinverse projection matrix.
  double const pxx = Pinv.at<double>(0);  double const pxy = Pinv.at<double>( 1);  double const pxh = Pinv.at<double>( 2);
  double const pyx = Pinv.at<double>(3);  double const pyy = Pinv.at<double>( 4);  double const pyh = Pinv.at<double>( 5);
  double const pzx = Pinv.at<double>(6);  double const pzy = Pinv.at<double>( 7);  double const pzh = Pinv.at<double>( 8);
  double const phx = Pinv.at<double>(9);  double const phy = Pinv.at<double>(10);  double const phh = Pinv.at<double>(11);

  // Get camera center.
  double const cx = PG->center[0];
  double const cy = PG->center[1];
  double const cz = PG->center[2];

  // Compute ray directions.
  for (int i = 0; i < N; ++i)
    {
      double const x3 = pxx * row_x[i] + pxy * row_y[i] + pxh;
      double const y3 = pyx * row_x[i] + pyy * row_y[i] + pyh;
      double const z3 = pzx * row_x[i] + pzy * row_y[i] + pzh;
      double const h3 = phx * row_x[i] + phy * row_y[i] + phh;

      double const h3_inv = 1.0 / h3;

      double const val_vx = x3 * h3_inv - cx;
      double const val_vy = y3 * h3_inv - cy;
      double const val_vz = z3 * h3_inv - cz;

      double const v2 = val_vx * val_vx + val_vy * val_vy + val_vz * val_vz;
      double const v = sqrt(v2);
      double const k = 1.0 / v;

      row_vx[i] = k * val_vx;
      row_vy[i] = k * val_vy;
      row_vz[i] = k * val_vz;
    }
  /* for */


  SAFE_ASSIGN_PTR( vx, vx_out );
  SAFE_ASSIGN_PTR( vy, vy_out );
  SAFE_ASSIGN_PTR( vz, vz_out );

 GetCameraRays_EXIT:

  SAFE_DELETE( vx );
  SAFE_DELETE( vy );
  SAFE_DELETE( vz );

  return result;
}
/* GetCameraRays */




/****** TRIANGULATION ******/

//! Triangulates two views.
/*!
  Function computes intersections of two views.
  Note that exactly one of x1, y2, x2, and y2 may be NULL; if so
  then dst2 will not be computed.

  \param PG1    Projective geometry for the first view.
  \param x1     Undistorted image x coordinates for the first view.
  \param y1     Undistorted image y coordinates for the first view.
  \param PG2    Projective geometry for the second view.
  \param x2     Undistorted image x coordinates for the second view.
  \param y2     Undistorted image y coordinates for the second view.
  \param x_out  Address where x coordinates of intersection points will be stored.
  \param y_out  Address where y coordinates of intersection points will be stored.
  \param z_out  Address where z coordinates of intersection points will be stored.
  \param dst2_out Squared length of the shortest line segment that connects two rays.
  \return Function returns true if successfull, false otherwise.
*/
bool
TriangulateTwoViews(
                    ProjectiveGeometry * const PG1,
                    cv::Mat * const x1,
                    cv::Mat * const y1,
                    ProjectiveGeometry * const PG2,
                    cv::Mat * const x2,
                    cv::Mat * const y2,
                    cv::Mat * * const x_out,
                    cv::Mat * * const y_out,
                    cv::Mat * * const z_out,
                    cv::Mat * * const dst2_out
                    )
{
  int const E1 = ( (NULL == x1)? 1 : 0 ) + ( (NULL == y1)? 1 : 0 );
  int const E2 = ( (NULL == x2)? 1 : 0 ) + ( (NULL == y2)? 1 : 0 );
  assert( 1 >= E1 + E2 );
  if (1 < E1 + E2) return false;

  int const num_valid = (0 == E1)? CheckRowArrays_inline(CV_64F, x1, y1, x2, y2) : CheckRowArrays_inline(CV_64F, x2, y2, x1, y1);
  assert( 3 <= num_valid );
  if (3 > num_valid) return false;

  assert( (NULL != PG1) && (NULL != PG2) );
  if ( (NULL == PG1) || (NULL == PG2) ) return false;

  bool result = true; // Assume success.

  // Declare required variables.
  cv::Mat * x = NULL;  cv::Mat * y = NULL;  cv::Mat * z = NULL;
  cv::Mat * dst2 = NULL;
  cv::Mat * A1 = NULL;  cv::Mat * B1 = NULL;  cv::Mat * C1 = NULL;  cv::Mat * D1 = NULL;
  cv::Mat * A2 = NULL;  cv::Mat * B2 = NULL;  cv::Mat * C2 = NULL;  cv::Mat * D2 = NULL;
  cv::Mat * vx1 = NULL;  cv::Mat * vy1 = NULL;  cv::Mat * vz1 = NULL;
  cv::Mat * vx2 = NULL;  cv::Mat * vy2 = NULL;  cv::Mat * vz2 = NULL;
  cv::Mat pt1, pt2;

  // Get rays or planes for the first view.
  if (true == result)
    {
      if (0 == E1)
        {
          result = GetCameraRays(x1, y1, PG1, &vx1, &vy1, &vz1);
          assert(true == result);
          pt1 = cv::Mat(1, 3, CV_64F, &(PG1->center[0]), 3 * sizeof(double));
        }
      else
        {
          result = GetCameraPlanes(x1, y1, PG1, &A1, &B1, &C1, &D1);
          assert(true == result);
        }
      /* if */
    }
  /* if */

  // Get rays or planes for the second view.
  if (true == result)
    {
      if (0 == E2)
        {
          result = GetCameraRays(x2, y2, PG2, &vx2, &vy2, &vz2);
          assert(true == result);
          pt2 = cv::Mat(1, 3, CV_64F, &(PG2->center[0]), 3 * sizeof(double));
        }
      else
        {
          result = GetCameraPlanes(x2, y2, PG2, &A2, &B2, &C2, &D2);
          assert(true == result);
        }
      /* if */
    }
  /* if */

  // Triangulate views.
  if (true == result)
    {
      if (NULL == vx1)
        {
          result = PlaneRayIntersection(A1, B1, C1, D1, vx2, vy2, vz2, &pt2, &x, &y, &z);
          assert(true == result);
        }
      else if (NULL == vx2)
        {
          result = PlaneRayIntersection(A2, B2, C2, D2, vx1, vy1, vz1, &pt1, &x, &y, &z);
          assert(true == result);
        }
      else
        {
          result = RayRayIntersection(vx1, vy1, vz1, &pt1, vx2, vy2, vz2, &pt2, &x, &y, &z, &dst2);
          assert(true == result);
        }
      /* if */
    }
  /* if */

  // Assign outputs.
  SAFE_ASSIGN_PTR( x, x_out );
  SAFE_ASSIGN_PTR( y, y_out );
  SAFE_ASSIGN_PTR( z, z_out );
  SAFE_ASSIGN_PTR( dst2, dst2_out );

  // Release memory.
  SAFE_DELETE( x );  SAFE_DELETE( y );  SAFE_DELETE( z );
  SAFE_DELETE( dst2 );
  SAFE_DELETE( A1 );  SAFE_DELETE( B1 );  SAFE_DELETE( C1 );  SAFE_DELETE( D1 );
  SAFE_DELETE( A2 );  SAFE_DELETE( B2 );  SAFE_DELETE( C2 );  SAFE_DELETE( D2 );
  SAFE_DELETE( vx1 );  SAFE_DELETE( vy1 );  SAFE_DELETE( vz1 );
  SAFE_DELETE( vx2 );  SAFE_DELETE( vy2 );  SAFE_DELETE( vz2 );

  return result;
}
/* TriangulateTwoViews */



/****** PROJECTION ******/

//! Projects points.
/*!
  Function projects points in 3D to 2D using pinhole camera model.

  \param PG     Pinhole camera geometry.
  \param x_3D   X coordinates in 3D.
  \param y_3D   Y coordinates in 3D.
  \param z_3D   Z coordinates in 3D.
  \param x_2D_out       Address where x coordinates in 2D will be stored.
  \param y_2D_out       Address where y coordiantes in 2D will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
ProjectPoints(
              ProjectiveGeometry * const PG,
              cv::Mat * const x_3D,
              cv::Mat * const y_3D,
              cv::Mat * const z_3D,
              cv::Mat * * const x_2D_out,
              cv::Mat * * const y_2D_out
              )
{
  assert(NULL != PG);
  if (NULL == PG) return false;

  int const num_3D = CheckRowArrays_inline(CV_64F, x_3D, y_3D, z_3D);
  assert(3 == num_3D);
  if (3 != num_3D) return false;

  bool result = true; // Assume success.

  int const N = x_3D->cols;

  cv::Mat * x_2D = new cv::Mat(1, N, CV_64F);
  assert(NULL != x_2D);

  cv::Mat * y_2D = new cv::Mat(1, N, CV_64F);
  assert(NULL != y_2D);

  if ( (NULL == x_2D) || (NULL == y_2D) )
    {
      result = false;
      goto ProjectPoints_EXIT;
    }
  /* if */

  // Get row pointers.
  double const * const row_x_3D = (double *)( (BYTE *)(x_3D->data) + x_3D->step[0] * 0 );
  double const * const row_y_3D = (double *)( (BYTE *)(y_3D->data) + y_3D->step[0] * 0 );
  double const * const row_z_3D = (double *)( (BYTE *)(z_3D->data) + z_3D->step[0] * 0 );
  double * const row_x_2D = (double *)( (BYTE *)(x_2D->data) + x_2D->step[0] * 0 );
  double * const row_y_2D = (double *)( (BYTE *)(y_2D->data) + y_2D->step[0] * 0 );

  // Get projection matrix coefficients.
  double const pxx = PG->projection[0][0];
  double const pxy = PG->projection[0][1];
  double const pxz = PG->projection[0][2];
  double const pxh = PG->projection[0][3];

  double const pyx = PG->projection[1][0];
  double const pyy = PG->projection[1][1];
  double const pyz = PG->projection[1][2];
  double const pyh = PG->projection[1][3];

  double const phx = PG->projection[2][0];
  double const phy = PG->projection[2][1];
  double const phz = PG->projection[2][2];
  double const phh = PG->projection[2][3];

  // Project points.
  for (int i = 0; i < N; ++i)
    {
      double const x2 = pxx * row_x_3D[i] + pxy * row_y_3D[i] + pxz * row_z_3D[i] + pxh;
      double const y2 = pyx * row_x_3D[i] + pyy * row_y_3D[i] + pyz * row_z_3D[i] + pyh;
      double const h2 = phx * row_x_3D[i] + phy * row_y_3D[i] + phz * row_z_3D[i] + phh;

      double const h2_inv = 1.0 / h2;

      row_x_2D[i] = x2 * h2_inv;
      row_y_2D[i] = y2 * h2_inv;
    }
  /* for */


  SAFE_ASSIGN_PTR( x_2D, x_2D_out );
  SAFE_ASSIGN_PTR( y_2D, y_2D_out );

 ProjectPoints_EXIT:

  SAFE_DELETE( x_2D );
  SAFE_DELETE( y_2D );

  return result;
}
/* ProjectPoints */



/****** AUXILIARY FUNCTIONS ******/

//! Data assembly for VTK visualization.
/*!
  Creates inputs required for VTK visualization.

  \param x_3D   Array of x coordinates.
  \param y_3D   Array of y coordinates.
  \param z_3D   Array of z coordinates.
  \param dst2_3D        Array of squared distances between rays. May be NULL.
  \param dst2_thr       Threshold for the array of squared distances. Only points having distance lower than this threshold will be output.
  \param x_img  Image x coordinate.
  \param y_img  Image y coordinate.
  \param range_img Dynamic range of selected points.
  \param AllImages      Pointer to all captured images. If NULL then points will not have color information.
  \param texture      Pointer to cv::Mat which stores texture image. Texture image must be either grayscale or BGR with 8 bits per pixel.
  \param abs_phase_distance  Image storing distance to constellation of the absolute phase.
  \param abs_phase_deviation  Image storing deviation of the absolute phase.
  \param points_out     Address where selected 3D points will be stored.
  \param colors_out     Address where texture information for 3D points will be stored.
  \param data_out       Address where additional data for 3D points will be stored.
  \return Returns true if successfull.
*/
bool
SelectValidPointsAndAssembleDataForVTK(
                                       cv::Mat * const x_3D,
                                       cv::Mat * const y_3D,
                                       cv::Mat * const z_3D,
                                       cv::Mat * const dst2_3D,
                                       double const dst2_thr,
                                       cv::Mat * const x_img,
                                       cv::Mat * const y_img,
                                       cv::Mat * const range_img,
                                       ImageSet * const AllImages,
                                       cv::Mat * const texture,
                                       cv::Mat * const abs_phase_distance,
                                       cv::Mat * const abs_phase_deviation,
                                       cv::Mat * * const points_out,
                                       cv::Mat * * const colors_out,
                                       cv::Mat * * const data_out
                                       )
{
  int const num_3D = CheckRowArrays_inline(CV_64F, x_3D, y_3D, z_3D, dst2_3D);
  assert( 3 <= num_3D );
  if (3 > num_3D) return false;

  int const N = x_3D->cols;

  cv::Mat * points = NULL;
  cv::Mat * colors = NULL;
  //cv::Mat * img = NULL;
  cv::Mat * data = NULL;

  bool const prune = (NULL != dst2_thr);
  bool const is_grayscale = (NULL != AllImages) && (true == AllImages->IsGrayscale());
  bool const have_texture = (NULL != texture) && (NULL != texture->data);
  bool const have_range = (NULL != range_img) && (NULL != range_img->data);
  bool const have_phase_distance = (NULL != abs_phase_distance) && (NULL != abs_phase_distance->data);
  bool const have_phase_deviation = (NULL != abs_phase_deviation) && (NULL != abs_phase_deviation->data);

  bool const have_data = (true == have_texture) || (true == have_range) || (true == have_phase_distance) || (true == have_phase_deviation);
  if ( true == have_data )
    {
      int const num_2D = CheckRowArrays_inline(CV_32S, x_img, y_img);
      assert(2 == num_2D);
      if (2 != num_2D) return false;

      assert(N == x_img->cols);
      if (N != x_img->cols) return false;

      if (true == have_texture)
        {
          assert( (CV_MAT_DEPTH(texture->type()) == CV_8U) &&
                  ( (true == is_grayscale) ?
                    (CV_MAT_CN(texture->type()) == 1) :
                    (CV_MAT_CN(texture->type()) == 3)
                    )
                  );
          if ( (CV_MAT_DEPTH(texture->type()) != CV_8U) ||
               ( (true == is_grayscale) ?
                 (CV_MAT_CN(texture->type()) != 1) :
                 (CV_MAT_CN(texture->type()) != 3)
                 )
               )
            {
              return false;
            }
          /* if */
        }
      /* if */

      if (true == have_range)
        {
          bool const range_valid = CheckRowArray_inline(CV_32F, range_img);
          assert(true == range_valid);
          if (true != range_valid) return false;

          assert(N == range_img->cols);
          if (N != range_img->cols) return false;
        }
      /* if */
    }
  /* if */

  bool result = true; // Assume success.

  // Allocate storage for point coordinates.
  points = new cv::Mat(N, 3, CV_64F);
  assert(NULL != points);
  if (NULL == points)
    {
      result = false;
      goto SelectValidPointsAndAssembleDataForVTK_EXIT;
    }
  /* if */

  // Create row indices into images.
  int y_prev_tex = -1;
  int y_prev_phase_distance = -1;
  int y_prev_phase_deviation = -1;

  UINT8 const * row_tex = NULL;
  float const * row_phase_distance = NULL;
  float const * row_phase_deviation = NULL;

  // Allocate storage for additional data.
  if ( true == have_data )
    {
      data = new cv::Mat(N, 4, CV_32F, cv::Scalar(0.0f));
      assert( (NULL != data) && (NULL != data->data) );
      if ( (NULL == data) || (NULL == data->data) )
        {
          result = false;
          goto SelectValidPointsAndAssembleDataForVTK_EXIT;
        }
      /* if */

      // Allocate storage for texture.
      if (true == have_texture)
        {
          colors = new cv::Mat(N, is_grayscale ? 1 : 3, CV_8U);
          assert( (NULL != colors) && (NULL != colors->data) );
          if ( (NULL == colors) || (NULL == colors->data) )
            {
              result = false;
              goto SelectValidPointsAndAssembleDataForVTK_EXIT;
            }
          /* if */
        }
      /* if */
    }
  /* if */

  // Get row pointers.
  double const * const row_x_3D = (double *)( (BYTE *)(x_3D->data) + x_3D->step[0] * 0 );
  double const * const row_y_3D = (double *)( (BYTE *)(y_3D->data) + y_3D->step[0] * 0 );
  double const * const row_z_3D = (double *)( (BYTE *)(z_3D->data) + z_3D->step[0] * 0 );
  double const * const row_dst2_3D = (true == prune)? (double *)( (BYTE *)(dst2_3D->data) + dst2_3D->step[0] * 0 ) : NULL;
  int const * const row_x_img = (true == have_texture)? (int *)( (BYTE *)(x_img->data) + x_img->step[0] * 0 ) : NULL;
  int const * const row_y_img = (true == have_texture)? (int *)( (BYTE *)(y_img->data) + y_img->step[0] * 0 ) : NULL;
  float const * const row_range_img = (true == have_range)? (float *)( (BYTE *)(range_img->data) + range_img->step[0] * 0 ) : NULL;

  // Copy point coordinates.
  int k = 0;
  for (int i = 0; i < N; ++i)
    {
      if ( (true == prune) && (dst2_thr < row_dst2_3D[i]) ) continue;

      double const x3 = row_x_3D[i];
      double const y3 = row_y_3D[i];
      double const z3 = row_z_3D[i];

      if ( isnanorinf_inline(x3) || isnanorinf_inline(y3) || isnanorinf_inline(z3) ) continue;

      assert( (0 <= k) && (k < points->rows) );
      double * const row_points = (double *)( (BYTE *)(points->data) + points->step[0] * k );

      row_points[0] = x3;
      row_points[1] = y3;
      row_points[2] = z3;

      if ( true == have_data )
        {
          assert( (NULL != data) && (NULL != data->data) );

          int const x = row_x_img[i];
          int const y = row_y_img[i];
          assert( (0 <= x) && (0 <= y) && (0 <= k) );

          if (true == have_texture)
            {
              assert( k < colors->rows );
              UINT8 * const row_colors = (UINT8 *)( (BYTE *)(colors->data) + colors->step[0] * k );

              assert( (x < texture->cols) && (y < texture->rows) );
              if (y != y_prev_tex)
                {
                  row_tex = (UINT8 const *)( (BYTE *)(texture->data) + texture->step[0] * y );
                  y_prev_tex = y;
                }
              /* if */

              if (false == is_grayscale)
                {
                  row_colors[0] = row_tex[3*x + 2];
                  row_colors[1] = row_tex[3*x + 1];
                  row_colors[2] = row_tex[3*x    ];
                }
              else
                {
                  row_colors[0] = row_tex[x];
                }
              /* if */
            }
          /* if */

          assert( k < data->rows );
          float * const row_data = (float *)( (BYTE *)(data->data) + data->step[0] * k );

          if (true == have_range)
            {
              row_data[0] = row_range_img[i];
              row_data[1] = (float)( sqrt(row_dst2_3D[i]) );
            }
          /* if */

          if (true == have_phase_distance)
            {
              assert( (x < abs_phase_distance->cols) && (y < abs_phase_distance->rows) );
              if (y != y_prev_phase_distance)
                {
                  row_phase_distance = (float const *)( (BYTE *)(abs_phase_distance->data) + abs_phase_distance->step[0] * y );
                  y_prev_phase_distance = y;
                }
              /* if */
              row_data[2] = row_phase_distance[x];
            }
          /* if */

          if (true == have_phase_deviation)
            {
              assert( (x < abs_phase_deviation->cols) && (y < abs_phase_deviation->rows) );
              if (y != y_prev_phase_deviation)
                {
                  row_phase_deviation = (float const *)( (BYTE *)(abs_phase_deviation->data) + abs_phase_deviation->step[0] * y );
                  y_prev_phase_deviation = y;
                }
              /* if */
              row_data[3] = row_phase_deviation[x];
            }
          /* if */
        }
      /* if */

      ++k;
    }
  /* for */

  // Create new cv::Mat headers for output buffers. Note that we do not reallocate
  // allocated storage; instead only header data of cv::Mat is changed to reflect
  // the correct number of valid points.
  cv::Range r[2] = {cv::Range(0, k), cv::Range::all()};

  if (0 < k)
    {
      cv::Mat * points_tmp = new cv::Mat(*points, r);
      assert(NULL != points_tmp);
      if (NULL != points_tmp)
        {
          SAFE_DELETE(points);
          points = points_tmp;
          points_tmp = NULL;
        }
      else
        {
          result = false;
        }
      /* if */

      if (true == have_texture)
        {
          cv::Mat * colors_tmp = new cv::Mat(*colors, r);
          assert(NULL != colors_tmp);
          if (NULL != colors_tmp)
            {
              SAFE_DELETE(colors);
              colors = colors_tmp;
              colors_tmp = NULL;
            }
          else
            {
              result = false;
            }
          /* if */
        }
      /* if */

      if (true == have_data)
        {
          cv::Mat * data_tmp = new cv::Mat(*data, r);
          assert(NULL != data_tmp);
          if (NULL != data_tmp)
            {
              SAFE_DELETE(data);
              data = data_tmp;
              data_tmp = NULL;
            }
          else
            {
              result = false;
            }
          /* if */
        }
      /* if */
    }
  else
    {
      points->create(0, 3, CV_64F);
      if (true == have_texture) colors->create(0, is_grayscale ? 1 : 3, CV_8U);
      if (true == have_range) data->create(0, 2, CV_32F);
      assert(true == result);
    }
  /* if */


  SAFE_ASSIGN_PTR( points, points_out );
  SAFE_ASSIGN_PTR( colors, colors_out );
  SAFE_ASSIGN_PTR( data, data_out );

 SelectValidPointsAndAssembleDataForVTK_EXIT:

  SAFE_DELETE( points );
  SAFE_DELETE( colors );
  SAFE_DELETE( data );

  return result;
}
/* SelectValidPointsAndAssembleDataForVTK */



#endif /* !__BATCHACQUISITIONPROCESSINGTRIANGULATION_CPP */
