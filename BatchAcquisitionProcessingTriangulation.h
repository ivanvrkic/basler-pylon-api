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
  \file   BatchAcquisitionProcessingTriangulation.h
  \brief  Point triangulation.

  Functions for point triangulation.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#ifndef __BATCHACQUISITIONPROCESSINGTRIANGULATION_H
#define __BATCHACQUISITIONPROCESSINGTRIANGULATION_H


#include "BatchAcquisitionProcessing.h"



/****** LINE-POINT INTERSECTIONS ******/

//! Find the closest point on a line.
bool ClosestPointOnLineFromPoint(double_a_V3 const, double_a_V3 const, double_a_V3 const, double_a_V3);

//! Return distance along line from point.
bool DistanceAlongLineFromPoint(double_a_V3 const, double_a_V3 const, double_a_V3 const, double * const);


/****** PLANE-RAY INTERSECTIONS ******/

//! Plane-ray intersections.
bool
PlaneRayIntersection(
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * const,
                     cv::Mat * * const,
                     cv::Mat * * const,
                     cv::Mat * * const
                     );

//! Ray-ray intersection.
bool
RayRayIntersection(
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * const,
                   cv::Mat * * const,
                   cv::Mat * * const,
                   cv::Mat * * const,
                   cv::Mat * * const
                   );


/****** RAY GENERATORS ******/

//! Get camera planes.
bool GetCameraPlanes(cv::Mat * const, cv::Mat * const, ProjectiveGeometry * const, cv::Mat * * const, cv::Mat * * const, cv::Mat * * const, cv::Mat * * const);

//! Get camera rays.
bool GetCameraRays(cv::Mat * const, cv::Mat * const, ProjectiveGeometry * const, cv::Mat * * const, cv::Mat * * const, cv::Mat * * const);


/****** TRIANGULATION ******/

//! Triangulates two views.
bool
TriangulateTwoViews(
                    ProjectiveGeometry * const,
                    cv::Mat * const,
                    cv::Mat * const,
                    ProjectiveGeometry * const,
                    cv::Mat * const,
                    cv::Mat * const,
                    cv::Mat * * const,
                    cv::Mat * * const,
                    cv::Mat * * const,
                    cv::Mat * * const
                    );


/****** PROJECTION ******/

//! Projects points.
bool ProjectPoints(ProjectiveGeometry * const, cv::Mat * const, cv::Mat * const, cv::Mat * const, cv::Mat * * const, cv::Mat * * const);


/****** AUXILIARY FUNCTIONS ******/

//! Data assembly for VTK visualization.
bool
SelectValidPointsAndAssembleDataForVTK(
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       double const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       ImageSet * const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       cv::Mat * const,
                                       cv::Mat * * const,
                                       cv::Mat * * const,
                                       cv::Mat * * const
                                       );


#endif /* !__BATCHACQUISITIONPROCESSINGTRIANGULATION_H */
