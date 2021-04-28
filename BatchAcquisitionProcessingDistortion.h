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
  \file   BatchAcquisitionProcessingDistortion.h
  \brief  Image distortion.

  Functions for removing distortion.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#ifndef __BATCHACQUISITIONPROCESSINGDISTORTION_H
#define __BATCHACQUISITIONPROCESSINGDISTORTION_H


#include "BatchAcquisitionProcessing.h"


//! Undistort image coordinates.
bool UndistortImageCoordinatesForRadialDistorsion(
                                                  cv::Mat * const,
                                                  cv::Mat * const,
                                                  int const,
                                                  int const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  cv::Mat * * const,
                                                  cv::Mat * * const
                                                  );

//! Undistort image coordinates.
bool UndistortImageCoordinatesForRadialDistorsion(
                                                  cv::Mat * const,
                                                  cv::Mat * const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  double const,
                                                  cv::Mat * * const,
                                                  cv::Mat * * const
                                                  );


#endif /* !__BATCHACQUISITIONPROCESSINGDISTORTION_H */
