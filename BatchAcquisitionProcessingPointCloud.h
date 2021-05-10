/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2016-2021 UniZG, Zagreb. All rights reserved.
 * (c) 2016-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingPointCloud.h
  \brief  Point cloud processing.

  Functions for point cloud processing.

  \author Tomislav Petkovic
  \date   2021-04-20
*/


#ifndef __BATCHACQUISITIONPROCESSINGPOINTCLOUD_H
#define __BATCHACQUISITIONPROCESSINGPOINTCLOUD_H


#include "BatchAcquisition.h"


//! Finds center of mass of a point cloud.
bool PointCloudCenterOfMass(cv::Mat * const, cv::Mat * const);

//! Finds median of a point cloud.
bool PointCloudWeiszfeld(cv::Mat * const, cv::Mat * const, cv::Mat * const, int, int);

//! Counts points inside a sphere.
int PointCloundInsideASphere(cv::Mat * const, double const, double const, double const, double const, cv::Mat * const);

//! Counts points in front of a plane.
int PointCloudInFrontOfAPlane(cv::Mat * const, double const, double const, double const, double const, cv::Mat * const);

//! Save point cloud to PLY.
bool PointCloudSaveToPLY(
                         wchar_t const * const,
                         std::vector<cv::Mat *> &,
                         std::vector<cv::Mat *> &,
                         std::vector<cv::Mat *> &
                         );


#endif /* !__BATCHACQUISITIONPROCESSINGPOINTCLOUD_H */
