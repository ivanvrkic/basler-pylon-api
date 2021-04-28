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
  \file   BatchAcquisitionProcessingDistortion.cpp
  \brief  Image distortion.

  This file contains functions for removing geometric distortions from acquired data.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGDISTORTION_CPP
#define __BATCHACQUISITIONPROCESSINGDISTORTION_CPP


#include "BatchAcquisitionProcessingDistortion.h"
#include "BatchAcquisitionProcessingPixelSelector.h"




//! Undistort image coordinates.
/*!
  Function undistorts image coordinates for radial distorsion.
  Inputs are internal parameters of a pinhole camera and image row and column indices.
  Note that optional shfit may be applied to the indices, i.e. to change C/C++ coordinates to Matlab.

  \param x_dis  Image column index. Must be CV_32S type.
  \param y_dis  Image row index. Must be CV_32S type.
  \param shift_x        Optional column integer shift in pixels. Set to 1 if internal camera parameters were computed for Matlab indices, and to 0 otherwise.
  \param shift_y        Optional row integer shift in pixels.
  \param fx     Focal distance.
  \param fy     Focal distance.
  \param cx     Optical axis center.
  \param cy     Optical axis center.
  \param kappa2  Tylor expansion coefficient of radial distortion offset for squared radius.
  \param kappa4  Tylor expansion coefficient of radial distortion offset for double squared radius.
  \param x_un_out       Address where pointer to undistorted x coordinates will be stored.
  \param y_un_out       Address where pointer to undistorted y coordinates will be stored.
  \return Function returns true if successfull.
*/
bool
UndistortImageCoordinatesForRadialDistorsion(
                                             cv::Mat * const x_dis,
                                             cv::Mat * const y_dis,
                                             int const shift_x,
                                             int const shift_y,
                                             double const fx,
                                             double const fy,
                                             double const cx,
                                             double const cy,
                                             double const kappa2,
                                             double const kappa4,
                                             cv::Mat * * const x_un_out,
                                             cv::Mat * * const y_un_out
                                             )
{
  bool const valid = CheckCoordinateArrays_inline(x_dis, y_dis, CV_32S);
  if (true != valid) return valid;

  bool result = true; // Assume processing succeeded.

  int const N = x_dis->cols;
  assert(1 == x_dis->rows);
  assert(N == y_dis->cols);
  assert(1 == y_dis->rows);

  cv::Mat * x_un = new cv::Mat(1, N, CV_64F, 0.0);
  assert(NULL != x_un);

  cv::Mat * y_un = new cv::Mat(1, N, CV_64F, 0.0);
  assert(NULL != y_un);

  if ( (NULL == x_un) || (NULL == y_un) )
    {
      result = false;
      goto UndistortImageCoordinates_EXIT;
    }
  /* if */

  // Invert focal distances so only operations are multiplications.
  double const fx_inv = 1.0 / fx;
  double const fy_inv = 1.0 / fy;

  // Get row pointers.
  int const * const ptr_x_dis = (int *)( (BYTE *)(x_dis->data) + x_dis->step[0] * 0 );
  int const * const ptr_y_dis = (int *)( (BYTE *)(y_dis->data) + y_dis->step[0] * 0 );
  double * const ptr_x_un = (double *)( (BYTE *)(x_un->data) + x_un->step[0] * 0 );
  double * const ptr_y_un = (double *)( (BYTE *)(y_un->data) + y_un->step[0] * 0 );

  // Undistort coordinates.
  for (int i = 0; i < N; ++i)
    {
      double const x = ( (double)( ptr_x_dis[i] + shift_x ) - cx ) * fx_inv;
      double const y = ( (double)( ptr_y_dis[i] + shift_y ) - cy ) * fy_inv;

      double const r2 = x*x + y*y;
      double const L = 1.0 + (kappa2 + kappa4 * r2 ) * r2;
      double const L_inv = 1.0 / L;

      ptr_x_un[i] = cx + fx * x * L_inv;
      ptr_y_un[i] = cy + fy * y * L_inv;
    }
  /* for */


  SAFE_ASSIGN_PTR( x_un, x_un_out );
  SAFE_ASSIGN_PTR( y_un, y_un_out );

 UndistortImageCoordinates_EXIT:

  SAFE_DELETE( x_un );
  SAFE_DELETE( y_un );

  return result;
}
/* UndistortImageCoordinatesForRadialDistorsion */



//! Undistort image coordinates.
/*!
  Function undistorts image coordinates for radial distorsion.
  Inputs are internal parameters of a pinhole camera and image row and column indices.
  Note that optional shfit may be applied to the indices, i.e. to change C/C++ coordinates to Matlab.

  \param x_dis  Image column index. Must be CV_64F type.
  \param y_dis  Image row index. Must be CV_64F type.
  \param fx     Focal distance.
  \param fy     Focal distance.
  \param cx     Optical axis center.
  \param cy     Optical axis center.
  \param kappa2  Tylor expansion coefficient of radial distortion offset for squared radius.
  \param kappa4  Tylor expansion coefficient of radial distortion offset for double squared radius.
  \param x_un_out       Address where pointer to undistorted x coordinates will be stored.
  \param y_un_out       Address where pointer to undistorted y coordinates will be stored.
  \return Function returns true if successfull.
*/
bool
UndistortImageCoordinatesForRadialDistorsion(
                                             cv::Mat * const x_dis,
                                             cv::Mat * const y_dis,
                                             double const fx,
                                             double const fy,
                                             double const cx,
                                             double const cy,
                                             double const kappa2,
                                             double const kappa4,
                                             cv::Mat * * const x_un_out,
                                             cv::Mat * * const y_un_out
                                             )
{
  bool const valid = CheckCoordinateArrays_inline(x_dis, y_dis, CV_64F);
  if (true != valid) return valid;

  bool result = true; // Assume processing succeeded.

  int const N = x_dis->cols;
  assert(1 == x_dis->rows);
  assert(N == y_dis->cols);
  assert(1 == y_dis->rows);

  cv::Mat * x_un = new cv::Mat(1, N, CV_64F, 0.0);
  assert(NULL != x_un);

  cv::Mat * y_un = new cv::Mat(1, N, CV_64F, 0.0);
  assert(NULL != y_un);

  if ( (NULL == x_un) || (NULL == y_un) )
    {
      result = false;
      goto UndistortImageCoordinates_EXIT;
    }
  /* if */

  // Invert focal distances so only operations are multiplications.
  double const fx_inv = 1.0 / fx;
  double const fy_inv = 1.0 / fy;

  // Get row pointers.
  double const * const ptr_x_dis = (double *)( (BYTE *)(x_dis->data) + x_dis->step[0] * 0 );
  double const * const ptr_y_dis = (double *)( (BYTE *)(y_dis->data) + y_dis->step[0] * 0 );
  double * const ptr_x_un = (double *)( (BYTE *)(x_un->data) + x_un->step[0] * 0 );
  double * const ptr_y_un = (double *)( (BYTE *)(y_un->data) + y_un->step[0] * 0 );

  // Undistort coordinates.
  for (int i = 0; i < N; ++i)
    {
      double const x = ( ptr_x_dis[i] - cx ) * fx_inv;
      double const y = ( ptr_y_dis[i] - cy ) * fy_inv;

      double const r2 = x*x + y*y;
      double const L = 1.0 + (kappa2 + kappa4 * r2 ) * r2;
      double const L_inv = 1.0 / L;

      ptr_x_un[i] = cx + fx * x * L_inv;
      ptr_y_un[i] = cy + fy * y * L_inv;
    }
  /* for */


  SAFE_ASSIGN_PTR( x_un, x_un_out );
  SAFE_ASSIGN_PTR( y_un, y_un_out );

 UndistortImageCoordinates_EXIT:

  SAFE_DELETE( x_un );
  SAFE_DELETE( y_un );

  return result;
}
/* UndistortImageCoordinatesForRadialDistorsion */



#endif /* !__BATCHACQUISITIONPROCESSINGDISTORTION_CPP */
