/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015 UniZG, Zagreb. All rights reserved.
 * (c) 2015 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingPixelSelector.cpp
  \brief  Pixel selection.

  This file contains functions for selection of valid pixels.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGPIXELSELECTOR_CPP
#define __BATCHACQUISITIONPROCESSINGPIXELSELECTOR_CPP


#include "BatchAcquisitionProcessingPixelSelector.h"



//! Get absolute threshold.
/*!
  Computes absolute threshold from relative one.
  Conversion depends on the input image datatype; the function
  simply mutliplies relative threshold with the maximum intensity value of
  a particular pixel type may hold.

  \param AllImages      Pointer to all recorded images.
  \param rel_thr        Relative threshold. Must be in the [0,1] interval.
  \return Function returns absolute threshold or NaN if it fails.
*/
double
GetAbsoluteThreshold(
                     ImageSet * const AllImages,
                     double const rel_thr
                     )
{
  double abs_thr = BATCHACQUISITION_qNaN_dv;

  assert(NULL != AllImages);
  if (NULL == AllImages) return abs_thr;

  assert( (0.0 <= rel_thr) && (rel_thr <= 1.0) );
  if ( (rel_thr < 0.0) || (1.0 < rel_thr) ) return abs_thr;

  uint const nbits = MSBPositionInOpenCVFromImageDataType_inline(AllImages->PixelFormat);
  if (0 < nbits)
    {
      double const maximum = pow( 2.0, double(nbits + 1) ) - 1.0;
      abs_thr = maximum * rel_thr;
    }
  /* if */

  return abs_thr;
}
/* GetAbsoluteThreshold */



//! Creates matrices holding pixel coordinates.
/*!
  Function creates matrices holding pixel coordinates.
  Pixel coordinates follow C/C++ convention so first pixel has both coordinates 0.

  \param dynamic_range  Pointer to dynamic range image.
  \param threshold      Threshold for the dynamic range image.
  \param crd_x_out      Address where a pointer to a matrix holding x (column) image coordinates will be stored.
  \param crd_y_out      Address where a pointer to a matrix holding y (row) image coordinates will be stored.
  \param range_out      Address where a pointer to a matrix holding dynamic range values will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
GetValidPixelCoordinates(
                         cv::Mat * const dynamic_range,
                         float const threshold,
                         cv::Mat * * const crd_x_out,
                         cv::Mat * * const crd_y_out,
                         cv::Mat * * const range_out
                         )
{
  assert(NULL != dynamic_range);
  if (NULL == dynamic_range) return false;

  assert(NULL != dynamic_range->data);
  if (NULL == dynamic_range->data) return false;

  int const cols = dynamic_range->cols;
  int const rows = dynamic_range->rows;

  int const N = cols * rows;
  assert( 0 < N );
  if ( 0 >= N ) return false;

  assert( (CV_MAT_DEPTH(dynamic_range->type()) == CV_32F) && (CV_MAT_CN(dynamic_range->type()) == 1) );
  if ( (CV_MAT_DEPTH(dynamic_range->type()) != CV_32F) || (CV_MAT_CN(dynamic_range->type()) != 1) ) return false;

  bool result = true; // Assume processing succeeded.

  cv::Mat * crd_x = new cv::Mat(1, N, CV_32S, cv::Scalar(-1));
  assert(NULL != crd_x);

  cv::Mat * crd_y = new cv::Mat(1, N, CV_32S, cv::Scalar(-1));
  assert(NULL != crd_y);

  cv::Mat * range = new cv::Mat(1, N, CV_32F, cv::Scalar(-1.0));
  assert(NULL != range);

  if ( (NULL == crd_x) || (NULL == crd_y) || (NULL == range) )
    {
      result = false;
      goto GetValidPixelCoordinates_EXIT;
    }
  /* if */

  // Get row pointers.
  int * const ptr_crd_x = (int *)( (BYTE *)(crd_x->data) + crd_x->step[0] * 0 );
  int * const ptr_crd_y = (int *)( (BYTE *)(crd_y->data) + crd_y->step[0] * 0 );
  float * const ptr_range = (float *)( (BYTE *)(range->data) + range->step[0] * 0 );
  int k = 0;

  // Get coordinates of valid pixels.
  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      float const * const row_dynamic_range = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );

      // Assign coordinates sequentially..
      for (int x = 0; x < cols; ++x)
        {
          if (threshold < row_dynamic_range[x])
            {
              ptr_crd_x[k] = x;
              ptr_crd_y[k] = y;
              ptr_range[k] = row_dynamic_range[x];
              ++k;
            }
          /* if */
        }
      /* for */
    }
  /* for */

  // Create new cv::Mat headers for output buffers. Note that we do not reallocate
  // allocated storage; instead only header data of cv::Mat is changed to reflect
  // the correct number of valid points.
  cv::Range r[2] = {cv::Range::all(), cv::Range(0, k)};

  if (0 < k)
    {
      cv::Mat * crd_x_tmp = new cv::Mat(*crd_x, r);
      assert(NULL != crd_x_tmp);
      if (NULL != crd_x_tmp)
        {
          SAFE_DELETE(crd_x);
          crd_x = crd_x_tmp;
          crd_x_tmp = NULL;
        }
      else
        {
          result = false;
        }
      /* if */

      cv::Mat * crd_y_tmp = new cv::Mat(*crd_y, r);
      assert(NULL != crd_y_tmp);
      if (NULL != crd_y_tmp)
        {
          SAFE_DELETE(crd_y);
          crd_y = crd_y_tmp;
          crd_y_tmp = NULL;
        }
      else
        {
          result = false;
        }
      /* if */

      cv::Mat * range_tmp = new cv::Mat(*range, r);
      assert(NULL != range_tmp);
      if (NULL != range_tmp)
        {
          SAFE_DELETE(range);
          range = range_tmp;
          range_tmp = NULL;
        }
      else
        {
          result = false;
        }
      /* if */
    }
  else
    {
      crd_x->create(1, 0, CV_32S);
      crd_y->create(1, 0, CV_32S);
      range->create(1, 0, CV_32F);
      assert(true == result);
    }
  /* if */

  
  SAFE_ASSIGN_PTR( crd_x, crd_x_out );
  SAFE_ASSIGN_PTR( crd_y, crd_y_out );
  SAFE_ASSIGN_PTR( range, range_out );

 GetValidPixelCoordinates_EXIT:

  SAFE_DELETE( crd_x );
  SAFE_DELETE( crd_y );
  SAFE_DELETE( range );

  return result;
}
/* GetValidPixelCoordinates */



//! Get projector row or column.
/*!
  Function returns projector row or column coordinate from the unwrapped phase image.

  \param x_img  Column indices of selected pixels.
  \param y_img  Row indices of selected pixels.
  \param abs_phase      Pointer to normalized unwrapped phase map. Must be CV_64F type.
  \param scale   Scaling factor; either projector width or height depending on the input unwrapped phase map.
  \param crd_pr_out Address where projector coordinate will be stored.
  \return Function returns true if successfull.
*/
bool
GetProjectorCoordinate(
                       cv::Mat * const x_img,
                       cv::Mat * const y_img,
                       cv::Mat * const abs_phase,
                       double const scale,
                       cv::Mat * * const crd_pr_out
                       )
{
  bool const valid = CheckCoordinateArrays_inline(x_img, y_img, CV_32S);
  if (true != valid) return valid;

  assert( (NULL != abs_phase) && (NULL != abs_phase->data) );
  if ( (NULL == abs_phase) || (NULL == abs_phase->data) ) return false;

  int const nrows = abs_phase->rows;
  int const ncols = abs_phase->cols;

  assert( (CV_MAT_DEPTH(abs_phase->type()) == CV_64F) && (CV_MAT_CN(abs_phase->type()) == 1) );
  if ( (CV_MAT_DEPTH(abs_phase->type()) != CV_64F) || (CV_MAT_CN(abs_phase->type()) != 1) ) return false;

  bool result = true; // Assume processing succeeded.

  int const N = x_img->cols;
  assert(1 == x_img->rows);
  assert(N == y_img->cols);
  assert(1 == y_img->rows);

  cv::Mat * crd_pr = new cv::Mat(1, N, CV_64F, 0.0);
  assert(NULL != crd_pr);

  if (NULL == crd_pr)
    {
      result = false;
      goto GetProjectorCoordinate_EXIT;
    }
  /* if */

  // Get row pointers.
  int const * const ptr_x_img = (int *)( (BYTE *)(x_img->data) + x_img->step[0] * 0 );
  int const * const ptr_y_img = (int *)( (BYTE *)(y_img->data) + y_img->step[0] * 0 );
  double * const ptr_crd_pr = (double *)( (BYTE *)(crd_pr->data) + crd_pr->step[0] * 0 );

  // Coordinates are listed row-wise. Therefore we recompute row pointer only when necessary.
  int y_prev = -1;
  double const * row_abs_phase = NULL;

  for (int i = 0; i < N; ++i)
    {
      // Get point coordinates.
      int const y = ptr_y_img[i];
      int const x = ptr_x_img[i];

      assert( (0 <= y) && (y < nrows) );
      assert( (0 <= x) && (x < ncols) );

      // Get row addresses.
      if (y != y_prev) row_abs_phase = (double *)( (BYTE *)(abs_phase->data) + abs_phase->step[0] * y );

      // Compute projector coordinate.
      ptr_crd_pr[i] = row_abs_phase[x] * scale;
    }
  /* for */


 GetProjectorCoordinate_EXIT:

  SAFE_ASSIGN_PTR( crd_pr, crd_pr_out );

  SAFE_DELETE( crd_pr );

  return result;

}
/* GetProjectorCoordinate */



#endif /* !__BATCHACQUISITIONPROCESSINGPIXELSELECTOR_CPP */
