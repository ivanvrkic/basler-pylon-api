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
  \file   BatchAcquisitionVTK.cpp
  \brief  VTK viewer window.

  Functions to create empty VTK window that runs in a separate thread.
  Displaying objects is supported through injection of vtkActor classes, i.e.
  vtkActor may be prepared in other processing threads and once it is complete
  it may be pushed into the visualisation thread for display.

  \author Tomislav Petkovic
  \date   2017-05-15
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONVTK_CPP
#define __BATCHACQUISITIONVTK_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionVTK.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionProcessingTriangulation.h"
#include "BatchAcquisitionFileList.h"
#include "BatchAcquisitionProcessingPointCloud.h"
#include "BatchAcquisitionDialogs.h"



/****** INLINE FUNCTIONS ******/

//! Blanks surface structure.
/*!
  Function blanks all parameters in the surface structure to default failsafes.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankSurfaceData_inline(
                           VTKsurfacedata * const P
                           )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->surfaceExtractor = NULL;
  P->surfaceFilter = NULL;
  P->surfaceMapper = NULL;
  P->surfaceActor = NULL;
}
/* VTKBlankSurfaceData_inline */



//! Blanks outline structure.
/*!
  Function blanks all parameters in the outline structure to default failsafes.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankOutlineData_inline(
                           VTKoutlinedata * const P
                           )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->outlineExtractor = NULL;
  P->outlineMapper = NULL;
  P->outlineActor = NULL;
}
/* VTKBlankOutlineData_inline */




//! Blanks point cloud data structure.
/*!
  Function blanks all parameters in the point cloud data structure to default failsafs.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankPointCloudData_inline(
                              VTKpointclouddata * const P
                              )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->CameraID = -1;
  P->ProjectorID = -1;

  P->cmx = 0.0;
  P->cmy = 0.0;
  P->cmz = 0.0;

  P->mdx = 0.0;
  P->mdy = 0.0;
  P->mdz = 0.0;

  P->rangeMin = 0.0f;
  P->rangeThr = 0.0f;
  P->rangeMax = 255.0f;

  P->rayDistanceMin = 0.0f;
  P->rayDistanceThr = 0.0f;
  P->rayDistanceMax = 100.0f;

  P->phaseDistanceMin = 0.0f;
  P->phaseDistanceThr = 0.0f;
  P->phaseDistanceMax = 5.0f;

  P->phaseDeviationMin = 0.0f;
  P->phaseDeviationThr = 0.0f;
  P->phaseDeviationMax = 5.0f;

  P->colorScale = 1.0f;
  P->colorOffset = 0.0f;

  P->acquisition_name = NULL;

  P->pDynamicRange = NULL;
  P->pRayDistance = NULL;
  P->pPhaseDistance = NULL;
  P->pPhaseDeviation = NULL;

  P->pMask = NULL;

  P->thresholdType = VTK_THRESHOLD_UNKNOWN;

  P->Cloud = NULL;

  P->ColorsMapped = NULL;
  P->ColorsOriginal = NULL;

  P->CloudPoints = NULL;
  P->PointsToVertexes = NULL;
  P->CloudVertexes = NULL;

  P->Mapper = NULL;
  P->Actor = NULL;

  P->surface = NULL;
  P->outline = NULL;
}
/* VTKBlankPointCloudData_inline */



//! Blanks VTK slicing plane structure.
/*!
  Function blanks all parameters in the slicing plane structure to default failsafes.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankSlicingPlaneData_inline(
                                VTKslicingplane * const P
                                )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->nx = BATCHACQUISITION_qNaN_dv;
  P->ny = BATCHACQUISITION_qNaN_dv;
  P->nz = BATCHACQUISITION_qNaN_dv;

  P->px = BATCHACQUISITION_qNaN_dv;
  P->py = BATCHACQUISITION_qNaN_dv;
  P->pz = BATCHACQUISITION_qNaN_dv;

  P->bounds[0] = -1.0;  P->bounds[1] = 1.0;
  P->bounds[2] = -1.0;  P->bounds[3] = 1.0;
  P->bounds[4] = -1.0;  P->bounds[5] = 1.0;

  P->Points = NULL;
  P->Polygon = NULL;
  P->Polygons = NULL;
  P->Plane = NULL;
  P->Mapper = NULL;
  P->Actor = NULL;
}
/* VTKBlankSlicingPlaneData_inline */



//! Blanks viewpoint data structure.
/*!
  Function blanks all parameters in the viewpoint data structure to default failsafe values.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankViewPointData_inline(
                             VTKviewpoint * const P
                             )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->camera3D = NULL;
  P->cameraTop = NULL;
  P->cameraFront = NULL;
  P->cameraSide = NULL;

  P->geometry = NULL;
}
/* VTKBlankViewPointData_inline */



//! Blanks VTK display window structure.
/*!
  Function blanks all parameters in the display window structure to default failsafes.

  \param P      Pointer to structure to blank.
*/
inline
void
VTKBlankWindowData_inline(
                          VTKwindowdata * const P
                          )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->ren3D = NULL;
  P->renTop = NULL;
  P->renFront = NULL;
  P->renSide = NULL;

  P->sldThr = NULL;
  P->sldThrRep = NULL;
  P->sldThrCallback = NULL;

  P->planeAxial1 = NULL;
  P->planeAxial2 = NULL;
  P->planeCoronal1 = NULL;
  P->planeCoronal2 = NULL;
  P->planeSagittal1 = NULL;
  P->planeSagittal2 = NULL;

  P->representationAxial1 = NULL;
  P->representationAxial2 = NULL;
  P->representationCoronal1 = NULL;
  P->representationCoronal2 = NULL;
  P->representationSagittal1 = NULL;
  P->representationSagittal2 = NULL;

  P->callbackAxial1 = NULL;
  P->callbackAxial2 = NULL;
  P->callbackCoronal1 = NULL;
  P->callbackCoronal2 = NULL;
  P->callbackSagittal1 = NULL;
  P->callbackSagittal2 = NULL;

  P->callbackViewpointChange = NULL;

  P->planeAxial = NULL;
  P->planeCoronal = NULL;
  P->planeSagittal = NULL;

  P->callbackAxial = NULL;
  P->callbackCoronal = NULL;
  P->callbackSagittal = NULL;

  P->slicingStatistics = NULL;

  P->renWin = NULL;
  P->renWinInt = NULL;
  P->renWinIntStyle = NULL;

  P->pushCallback = NULL;
  P->popCallback = NULL;
  P->keypressCallback = NULL;

  P->slicing_planes_visibility = VTK_ACTOR_VISIBILITY_UNDEFINED;

  P->interactorRunning = false;

  ZeroMemory( &(P->rendererCS), sizeof(P->rendererCS) );
}
/* VTKBlankWindowData_inline */



//! Blanks VTK display thread data storage structure.
/*!
  Function blanks all parameters in the VTK display thread structure to default failsafes.

  \param P      Pointer to a structure to blank.
*/
inline
void
VTKBlankDisplayThreadData_inline(
                                 VTKdisplaythreaddata * const P
                                 )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->thread = (HANDLE)( NULL );
  P->threadID = 0;

  P->window = NULL;

  P->camera = NULL;
  P->cameraNEW = NULL;

  P->point_clouds = NULL;
  P->point_cloudsNEW = NULL;

  P->projector_geometries = NULL;
  P->projector_geometriesNEW = NULL;

  P->camera_geometries = NULL;
  P->camera_geometriesNEW = NULL;

  P->CloudID = -1;
  P->CameraID = -1;
  P->ProjectorID = -1;

  P->camera_pushed = false;
  P->point_cloud_pushed = false;
  P->projector_geometry_pushed = false;
  P->camera_geometry_pushed = false;

  P->clear_all = false;

  P->terminate = false;

  ZeroMemory( &(P->dataCS), sizeof(P->dataCS) );
  ZeroMemory( &(P->pushCS), sizeof(P->pushCS) );

  P->myAddress = P;
}
/* VTKBlankDisplayThreadData_inline */



//! Destroys VTK display thread data.
/*!
  Function destroys the VTK display thread structure.

  \param P      Pointer to a structure to be destroyed.
*/
inline
void
VTKDestroyDisplayThreadData_inline(
                                   VTKdisplaythreaddata * const P
                                   )
{
  assert(NULL != P);
  if (NULL == P) return;

  EnterCriticalSection( &(P->dataCS) );
  {
    if (NULL != P->point_clouds)
      {
        int const n = (int)( P->point_clouds->size() );
        for (int i = 0; i < n; ++i)
          {
            VTKDeletePointCloudData( (*P->point_clouds)[i] );
            (*P->point_clouds)[i] = NULL;
          }
        /* for */
        SAFE_DELETE( P->point_clouds );
      }
    /* if */

    if (NULL != P->projector_geometries)
      {
        int const n = (int)( P->projector_geometries->size() );
        for (int i = 0; i < n; ++i)
          {
            SAFE_DELETE( (*P->projector_geometries)[i] );
          }
        /* for */
        SAFE_DELETE( P->projector_geometries );
      }
    /* if */

    if (NULL != P->camera_geometries)
      {
        int const n = (int)( P->camera_geometries->size() );
        for (int i = 0; i < n; ++i)
          {
            SAFE_DELETE( (*P->camera_geometries)[i] );
          }
        /* for */
        SAFE_DELETE( P->camera_geometries );
      }
    /* if */
  }
  LeaveCriticalSection( &(P->dataCS) );

  EnterCriticalSection( &(P->pushCS) );
  {
    if (NULL != P->point_cloudsNEW)
      {
        int const n = (int)( P->point_cloudsNEW->size() );
        for (int i = 0; i < n; ++i)
          {
            VTKDeletePointCloudData( (*P->point_cloudsNEW)[i] );
            (*P->point_cloudsNEW)[i] = NULL;
          }
        /* for */
        SAFE_DELETE( P->point_cloudsNEW );
      }
    /* if */

    if (NULL != P->projector_geometriesNEW)
      {
        int const n = (int)( P->projector_geometriesNEW->size() );
        for (int i = 0; i < n; ++i)
          {
            SAFE_DELETE( (*P->projector_geometriesNEW)[i] );
          }
        /* for */
        SAFE_DELETE( P->projector_geometriesNEW );
      }
    /* if */

    if (NULL != P->camera_geometriesNEW)
      {
        int const n = (int)( P->camera_geometriesNEW->size() );
        for (int i = 0; i < n; ++i)
          {
            SAFE_DELETE( (*P->camera_geometriesNEW)[i] );
          }
        /* for */
        SAFE_DELETE( P->camera_geometriesNEW );
      }
    /* if */
  }
  LeaveCriticalSection( &(P->pushCS) );

  /* Destroy critical sections. */
  DeleteCriticalSection( &(P->dataCS) );
  DeleteCriticalSection( &(P->pushCS) );

  VTKBlankDisplayThreadData_inline( P );

  free( P );
}
/* VTKDestroyDisplayThreadData_inline */



//! Creates VTK display thread data.
/*!
  Function creates and initializes the VTK display thread structure.

  \param P      Pointer to a structure in which containers should be created.
  \return Function returns true if successfull, false otherwise.
*/
inline
VTKdisplaythreaddata *
VTKCreateDisplayThreadData_inline(
                                  void
                                  )
{
  VTKdisplaythreaddata * const P = (VTKdisplaythreaddata *)malloc( sizeof(VTKdisplaythreaddata) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  VTKBlankDisplayThreadData_inline( P );

  /* Initialize critical sections. */
  InitializeCriticalSection( &(P->dataCS) );
  InitializeCriticalSection( &(P->pushCS) );

  /* Create all data containers. */
  assert(NULL == P->point_clouds);
  P->point_clouds = new std::vector<VTKpointclouddata_ *>();
  assert(NULL != P->point_clouds);

  assert(NULL == P->point_cloudsNEW);
  P->point_cloudsNEW = new std::vector<VTKpointclouddata_ *>();
  assert(NULL != P->point_cloudsNEW);

  assert(NULL == P->projector_geometries);
  P->projector_geometries = new std::vector<ProjectiveGeometry_ *>();
  assert(NULL != P->projector_geometries);

  assert(NULL == P->projector_geometriesNEW);
  P->projector_geometriesNEW = new std::vector<ProjectiveGeometry_ *>();
  assert(NULL != P->projector_geometriesNEW);

  assert(NULL == P->camera_geometries);
  P->camera_geometries = new std::vector<ProjectiveGeometry_ *>();
  assert(NULL != P->camera_geometries);

  assert(NULL == P->camera_geometriesNEW);
  P->camera_geometriesNEW = new std::vector<ProjectiveGeometry_ *>();
  assert(NULL != P->camera_geometriesNEW);

  if ( (NULL == P->point_clouds) ||
       (NULL == P->point_cloudsNEW) ||
       (NULL == P->projector_geometries) ||
       (NULL == P->projector_geometriesNEW) ||
       (NULL == P->camera_geometries) ||
       (NULL == P->camera_geometriesNEW)
       )
    {
      VTKDestroyDisplayThreadData_inline(P);
      return NULL;
    }
  /* if */

  /* Reserve space for some non-negative number of cameras. */
  int const expected_num_cam = 6;

  P->point_clouds->reserve(expected_num_cam);
  P->point_cloudsNEW->reserve(expected_num_cam);

  P->projector_geometries->reserve(expected_num_cam);
  P->projector_geometriesNEW->reserve(expected_num_cam);

  P->camera_geometries->reserve(expected_num_cam);
  P->camera_geometriesNEW->reserve(expected_num_cam);

  /* There must always exist at least one camera and one projector slot which may contain NULL values. */
  P->point_clouds->push_back( NULL );
  P->point_cloudsNEW->push_back( NULL );

  P->projector_geometries->push_back( NULL );
  P->projector_geometriesNEW->push_back( NULL );

  P->camera_geometries->push_back( NULL );
  P->camera_geometriesNEW->push_back( NULL );

  return P;
}
/* VTKCreateDisplayThreadData_inline */



//! Resize VTK display thread data storage for points.
/*!
  Function enlarges display thread data storage for points if necessary.

  \param P      Pointer to display thread data.
  \param N      Number of point clouds to concurrently store.
  \param point_clouds    Flag which indicates resize operation is for point cloud storage.
  \param projector_geometries   Flag which indicates resize operation if for camera geometry storage.
  \param camera_geometries        Flag which indicates resize operation is for projector geometry storage.
*/
inline
void
VTKResizeDisplayThreadData_inline(
                                  VTKdisplaythreaddata * const P,
                                  size_t const N,
                                  bool const point_clouds,
                                  bool const projector_geometries,
                                  bool const camera_geometries
                                  )
{
  assert(NULL != P);
  if (NULL == P) return;

  EnterCriticalSection( &(P->dataCS) );
  {
    if ( (true == point_clouds) && (NULL != P->point_clouds) && (N > P->point_clouds->size()) )
      {
        P->point_clouds->resize(N, NULL);
      }
    /* if */

    if ( (true == projector_geometries) && (NULL != P->projector_geometries) && (N > P->projector_geometries->size()) )
      {
        P->projector_geometries->resize(N, NULL);
      }
    /* if */

    if ( (true == camera_geometries) && (NULL != P->camera_geometries) && (N > P->camera_geometries->size()) )
      {
        P->camera_geometries->resize(N, NULL);
      }
    /* if */
  }
  LeaveCriticalSection( &(P->dataCS) );

  EnterCriticalSection( &(P->pushCS) );
  {
    if ( (true == point_clouds) && (NULL != P->point_cloudsNEW) && (N > P->point_cloudsNEW->size()) )
      {
        P->point_cloudsNEW->resize(N, NULL);
      }
    /* if */

    if ( (true == projector_geometries) && (NULL != P->projector_geometriesNEW) && (N > P->projector_geometriesNEW->size()) )
      {
        P->projector_geometriesNEW->resize(N, NULL);
      }
    /* if */

    if ( (true == camera_geometries) && (NULL != P->camera_geometriesNEW) && (N > P->camera_geometriesNEW->size()) )
      {
        P->camera_geometriesNEW->resize(N, NULL);
      }
    /* if */
  }
  LeaveCriticalSection( &(P->pushCS) );
}
/* VTKResizeDisplayThreadData_inline */



//! Test if VTK thread is running.
/*!
  Function tests if VTK thread is running.

  \param P      Pointer to VTK thread data.
*/
inline
bool
isVTKthreadrunning_inline(
                          VTKdisplaythreaddata * const P
                          )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->window);
  if (NULL == P->window) return false;

  assert( (HANDLE)( NULL ) != P->thread );
  if ( (HANDLE)( NULL ) == P->thread ) return false;

  /* Check if the VTK window is opened. */
  DWORD const VTKwait = WaitForSingleObjectEx(P->thread, 0, TRUE);
  assert(WAIT_OBJECT_0 != VTKwait);
  if (WAIT_OBJECT_0 == VTKwait) return false;

  return true;
}
/* isVTKthreadrunning_inline */



//! Combine bounds.
/*!
  Function combines two bounding boxes. Operation is done in-place.
  Bounding box is described by six numbers which denote minimum and maximum
  coordinate value for x, y, and z coordinates respectively.

  \param bounds_in_out  Pointer to the first bounding box. Resulting bounding box will replace the first bounding box.
  \param bounds_in      Pointer to the second bounding box.
*/
inline
void
VTKCombineBounds_inline(
                        double * const bounds_in_out,
                        double const * const bounds_in
                        )
{
  assert(NULL != bounds_in_out);
  if (NULL == bounds_in_out) return;

  assert(NULL != bounds_in);
  if (NULL == bounds_in) return;

  if (bounds_in[0] < bounds_in_out[0]) bounds_in_out[0] = bounds_in[0];
  if (bounds_in[1] > bounds_in_out[1]) bounds_in_out[1] = bounds_in[1];
  if (bounds_in[2] < bounds_in_out[2]) bounds_in_out[2] = bounds_in[2];
  if (bounds_in[3] > bounds_in_out[3]) bounds_in_out[3] = bounds_in[3];
  if (bounds_in[4] < bounds_in_out[4]) bounds_in_out[4] = bounds_in[4];
  if (bounds_in[5] > bounds_in_out[5]) bounds_in_out[5] = bounds_in[5];
}
/* VTKCombineBounds_inline */



//! Combine centers.
/*!
  Function combines centers of point clouds. Combined center is simply the artihmetic mean
  of added centers.

  \param mean_in_out  Pointer to accumulated center (three values). Must be set to zero for the first call to this function.
  \param n_in_out       Pointer to the number of accumulated centers. Must be set to zero for the first call to this function.
  \param center_in      Pointer to the point cloud center to add to the mix.
*/
inline
void
VTKCombineCenters_inline(
                         double * const mean_in_out,
                         double * const n_in_out,
                         double const * const center_in
                         )
{
  assert(NULL != mean_in_out);
  if (NULL == mean_in_out) return;

  assert(NULL != n_in_out);
  if (NULL == n_in_out) return;

  assert(NULL != center_in);
  if (NULL == center_in) return;

  *n_in_out += 1.0;
  assert(0.0 < *n_in_out);

  double const delta_x = center_in[0] - mean_in_out[0];
  double const delta_y = center_in[1] - mean_in_out[1];
  double const delta_z = center_in[2] - mean_in_out[2];

  double const w = 1.0 / (*n_in_out);
  mean_in_out[0] += w * delta_x;
  mean_in_out[1] += w * delta_y;
  mean_in_out[2] += w * delta_z;
}
/* VTKCombineCenters_inline */



//! Fetch data center and bounds.
/*!
  Function returns data center and bounding box.

  \param D      Pointer to display thread parameters.
  \param center_out     Address where the center will be stored (three double values).
  \param bounds_out     Address where the bounds will be stored (six double values).
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
VTKFetchDataCenterAndBounds_inline(
                                   VTKdisplaythreaddata * const D,
                                   double * const center_out,
                                   double * const bounds_out
                                   )
{
  bool result = false;

  assert(NULL != D);
  if (NULL == D) return result;

  assert(NULL != D->point_clouds);
  if (NULL == D->point_clouds) return result;

  double center_data[3] = {0.0, 0.0, 0.0};
  double center_mean[3] = {0.0, 0.0, 0.0};
  double center_n = 0.0;

  double bounds_data_1[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
  double bounds_data_2[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

  bool have_center = false;
  bool have_bounds = false;

  EnterCriticalSection( &(D->dataCS) );
  {
    if (NULL != D->point_clouds)
      {
        int const n = (int)( D->point_clouds->size() );
        for (int i = 0; i < n; ++i)
          {
            VTKpointclouddata_ * const points = ( *(D->point_clouds) )[i];
            if (NULL != points)
              {
                center_data[0] = points->mdx;
                center_data[1] = points->mdy;
                center_data[2] = points->mdz;
                VTKCombineCenters_inline(center_mean, &center_n, center_data);
                if (false == have_center) have_center = true;

                if (NULL != points->Cloud)
                  {
                    if (true == have_bounds)
                      {
                        points->Cloud->GetBounds(bounds_data_2);
                        VTKCombineBounds_inline(bounds_data_1, bounds_data_2);
                      }
                    else
                      {
                        points->Cloud->GetBounds(bounds_data_1);
                        have_bounds = true;
                      }
                    /* if */
                  }
                /* if */
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  LeaveCriticalSection( &(D->dataCS) );

  if (NULL != center_out)
    {
      if (true == have_center)
        {
          center_out[0] = center_mean[0];
          center_out[1] = center_mean[1];
          center_out[2] = center_mean[2];
        }
      else
        {
          center_out[0] = BATCHACQUISITION_qNaN_dv;
          center_out[1] = BATCHACQUISITION_qNaN_dv;
          center_out[2] = BATCHACQUISITION_qNaN_dv;
        }
      /* if */
    }
  else
    {
      have_center = true;
    }
  /* if */

  if (NULL != bounds_out)
    {
      if (true == have_bounds)
        {
          bounds_out[0] = bounds_data_1[0];  bounds_out[1] = bounds_data_1[1];
          bounds_out[2] = bounds_data_1[2];  bounds_out[3] = bounds_data_1[3];
          bounds_out[4] = bounds_data_1[4];  bounds_out[5] = bounds_data_1[5];
        }
      else
        {
          bounds_out[0] = BATCHACQUISITION_qNaN_dv;  bounds_out[1] = BATCHACQUISITION_qNaN_dv;
          bounds_out[2] = BATCHACQUISITION_qNaN_dv;  bounds_out[3] = BATCHACQUISITION_qNaN_dv;
          bounds_out[4] = BATCHACQUISITION_qNaN_dv;  bounds_out[5] = BATCHACQUISITION_qNaN_dv;
        }
      /* if */
    }
  else
    {
      have_bounds = true;
    }
  /* if */

  result = have_center && have_bounds;

  return result;
}
/* VTKFetchDataCenterAndBounds_inline */



//! Updates VTK viewpoint.
/*!
  Updates all cameras stored in the VTKviewpoint structure.

  \param camera Pointer to VTKviewpoint structure.
  \param geometry       Viewing geometry.
  \param md     Look-at point (median of point cloud). May be NULL.
  \param bounds Data boundaries. May be NULL.
  \param parallel       Flag indicating projection type.
*/
inline
void
VTKUpdateAllCameras_inline(
                           VTKviewpoint * const camera,
                           ProjectiveGeometry_ * const geometry,
                           double * const md,
                           double * const bounds,
                           bool const parallel
                           )
{
  assert(NULL != camera);
  if (NULL == camera) return;

  assert(NULL != geometry);
  if (NULL == geometry) return;

  bool const gu = VTKSetCameraToMatchGeometry(camera->camera3D, geometry, md, bounds, parallel);
  assert(true == gu);

  double const scale = geometry->GetScale();
  double const vec_x[3] = {1.0, 0.0, 0.0};  double const vec_nx[3] = {-1.0,  0.0,  0.0};
  double const vec_y[3] = {0.0, 1.0, 0.0};  double const vec_ny[3] = { 0.0, -1.0,  0.0};
  double const vec_z[3] = {0.0, 0.0, 1.0};  double const vec_nz[3] = { 0.0,  0.0, -1.0};

  bool const top = VTKSetOrthographicProjectionCamera(camera->cameraTop, vec_y, vec_z, md, bounds, scale);
  assert(true == top);

  bool const front = VTKSetOrthographicProjectionCamera(camera->cameraFront, vec_z, vec_ny, md, bounds, scale);
  assert(true == front);

  bool const side = VTKSetOrthographicProjectionCamera(camera->cameraSide, vec_nx, vec_ny, md, bounds, scale);
  assert(true == side);
}
/* VTKUpdateAllCameras_inline */



//! Set camera clipping planes and focal point.
/*!
  Sets current camera clipping planes and focal point to match the point cloud data.

  \param D      Pointer to VTK thread data structure.
  \param C      Pointer to view point which contains the current camera.
  \param md     Pointer to three element vector which stores data center point.
  \param bounds Pointer to six element vector which stores data bounding box.
*/
inline
void
VTKAdjustCamera_inline(
                       VTKdisplaythreaddata * const D,
                       VTKviewpoint * const C,
                       double * const md,
                       double * const bounds
                       )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != C);
  if (NULL == C) return;

  bool const focus = VTKSetCameraFocalPoint(C->camera3D, md);
  assert(true == focus);

  bool const clip = VTKSetCameraClippingPlanes(C->camera3D, bounds);
  assert(true == clip);
}
/* VTKAdjustCamera_inline */



//! Set camera to the desired camera.
/*!
  Sets the current camera to new viewing geometry.

  \param D      Pointer to VTK thread data structure.
  \param geometry      Pointer to projective geometry.
  \param parallel       Flag indicating projection type, false for perspective and true for parallel.
*/
inline
void
VTKChangeCameraGeometry_inline(
                               VTKdisplaythreaddata * const D,
                               ProjectiveGeometry_ * const geometry,
                               bool const parallel
                               )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != geometry);
  if (NULL == geometry) return;

  double md_data[3] = {0.0, 0.0, 0.0};
  double bounds_data[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

  double * md = NULL;
  double * bounds = NULL;

  EnterCriticalSection( &(D->window->rendererCS) );
  {
    /* Fetch clipping ranges from data. */
    bool const fetch = VTKFetchDataCenterAndBounds_inline(D, md_data, bounds_data);
    //assert(true == fetch);
    if (true == fetch)
      {
        md = md_data;
        bounds = bounds_data;
      }
    /* if */

    /* Create new camera data if none exists. */
    if (NULL == D->camera)
      {
        D->camera = VTKCreateViewPointData(geometry);
        assert(NULL != D->camera);

        /* Bind new camera to window. */
        if ( (NULL != D->window) && (NULL != D->camera) )
          {
            if ( (NULL != D->window->ren3D) && (NULL != D->camera->camera3D) ) D->window->ren3D->SetActiveCamera( D->camera->camera3D );
            if ( (NULL != D->window->renTop) && (NULL != D->camera->cameraTop) ) D->window->renTop->SetActiveCamera( D->camera->cameraTop );
            if ( (NULL != D->window->renFront) && (NULL != D->camera->cameraFront) ) D->window->renFront->SetActiveCamera( D->camera->cameraFront );
            if ( (NULL != D->window->renSide) && (NULL != D->camera->cameraSide) ) D->window->renSide->SetActiveCamera( D->camera->cameraSide );
          }
        /* if */
      }
    /* if */

    if ( NULL != D->camera )
      {
        /* Copy geometry information. */
        assert(NULL != D->camera->geometry);
        if ( NULL != D->camera->geometry )
          {
            *(D->camera->geometry) = *geometry;
          }
        /* if */

        /* Update active cameras. */
        VTKUpdateAllCameras_inline(D->camera, D->camera->geometry, md, bounds, parallel);
      }
    /* if */
  }
  LeaveCriticalSection( &(D->window->rendererCS) );

}
/* VTKChangeCameraGeometry_inline */



//! Sets plane widget position.
/*!
  Places slicing plane widget on default position.

  \param L      Pointer to line widget.
  \param D      Pointer to VTK thread data structure.
  \param vec    Line direction vector.
  \param move   Allowed line translation vector.
  \param plane_type Plane type may be axial, coronal, or sagittal.
*/
inline
void
VTKSetPlaneWidget_inline(
                         vtkLineWidget2 * const L,
                         VTKdisplaythreaddata * const D,
                         double vec[3],
                         double move[3],
                         SlicingPlane plane_type
                         )
{
  assert(NULL != D);
  if (NULL == D) return;

  double md_data[3] = {0.0, 0.0, 0.0};
  double bounds_data[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

  double * md = NULL;
  double * bounds = NULL;

  bool const fetch = VTKFetchDataCenterAndBounds_inline(D, md_data, bounds_data);
  //assert(true == fetch);
  if (true == fetch)
    {
      md = md_data;
      bounds = bounds_data;
    }
  /* if */

  AlignedLineRepresentation * const ptr = reinterpret_cast<AlignedLineRepresentation *>( L->GetRepresentation() );
  ptr->SetLineParameters(md, vec, move, bounds, plane_type);
}
/* VTKSetPlaneWidget_inline */



//! Updates all plane widgets.
/*!
  Updates all plane widgets to match the view geometry.

  \param D      Pointer to VTK thread data structure.
*/
inline
void
VTKUpdateAllPlaneWidgets_inline(
                                VTKdisplaythreaddata * const D
                                )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != D->window);
  if (NULL == D->window) return;

  double vec_x[3] = {1.0, 0.0, 0.0};
  double vec_y[3] = {0.0, 1.0, 0.0};
  double vec_z[3] = {0.0, 0.0, 1.0};

  VTKSetPlaneWidget_inline(D->window->planeCoronal1, D, vec_x, vec_z, VTK_PLANE_CORONAL); // renTop
  VTKSetPlaneWidget_inline(D->window->planeSagittal1, D, vec_z, vec_x, VTK_PLANE_SAGITTAL); // renTop

  VTKSetPlaneWidget_inline(D->window->planeAxial1, D, vec_x, vec_y, VTK_PLANE_AXIAL); // renFront
  VTKSetPlaneWidget_inline(D->window->planeSagittal2, D, vec_y, vec_x, VTK_PLANE_SAGITTAL); // renFront

  VTKSetPlaneWidget_inline(D->window->planeAxial2, D, vec_z, vec_y, VTK_PLANE_AXIAL); // renSide
  VTKSetPlaneWidget_inline(D->window->planeCoronal2, D, vec_y, vec_z, VTK_PLANE_CORONAL); // renSide

}
/* VTKUpdateAllPlaneWidgets_inline */



//! Change active camera.
/*!
  Function changes active camera.

  Note: Function does not reserve critical section object to synchronize access to the camera array.

  \param D      Pointer to VTK display thread parameters.
  \param CameraID       Camera ID.
  \param parallel       Flag indicating projection type, false for perspective and true for parallel.
*/
inline
void
VTKSetActiveCamera_inline(
                          VTKdisplaythreaddata * const D,
                          int const CameraID,
                          bool const parallel
                          )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != D->camera_geometries);
  if (NULL == D->camera_geometries) return;

  int const n = (int)( D->camera_geometries->size() );

  assert( (0 <= CameraID) && (CameraID < n) );
  if ( (CameraID < 0) && (n <= CameraID) ) return;

  ProjectiveGeometry_ * const geometry = ( *(D->camera_geometries) )[CameraID];

  assert(NULL != geometry);
  if (NULL == geometry) return;

  VTKChangeCameraGeometry_inline(D, geometry, parallel);

  D->CameraID = CameraID;
  D->ProjectorID = -1;
}
/* VTKSetActiveCamera_inline */



//! Change active projector.
/*!
  Function changes active projector.

  \param D      Pointer to VTK display thread parameters.
  \param ProjectorID    Projector ID.
  \param parallel       Flag indicating projection type, false for perspective and true for parallel.
*/
inline
void
VTKSetActiveProjector_inline(
                             VTKdisplaythreaddata * const D,
                             int const ProjectorID,
                             bool const parallel
                             )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != D->projector_geometries);
  if (NULL == D->projector_geometries) return;

  int const n = (int)( D->projector_geometries->size() );

  assert( (0 <= ProjectorID) && (ProjectorID < n) );
  if ( (ProjectorID < 0) && (n <= ProjectorID) ) return;

  ProjectiveGeometry_ * const geometry = ( *(D->projector_geometries) )[ProjectorID];

  assert(NULL != geometry);
  if (NULL == geometry) return;

  VTKChangeCameraGeometry_inline(D, geometry, parallel);

  D->ProjectorID = ProjectorID;
  D->CameraID = -1;
}
/* VTKSetActiveProjector_inline */



//! Adds actor to VTK renderer.
/*!
  Addss actor to VTK renderer.

  \param R      Pointer to VTK renderer.
  \param A      Pointer to VTK actor to be added.
  \return Returns true if successfull, false otherwise.
*/
inline
bool
VTKAddActorToRenderer_inline(
                             vtkRenderer * const R,
                             vtkProp * const A
                             )
{
  assert( (NULL != R) && (NULL != A) );
  if ( (NULL == R) || (NULL == A) ) return false;

  bool added = true;

  if (0 == R->GetActors()->IsItemPresent(A))
    {
      R->AddActor(A);
    }
  else
    {
      added = false;
    }
  /* if */

  return added;
}
/* VTKAddActorToRenderer_inline */



//! Removes actor from VTK renderer.
/*!
  Removes actor from VTK renderer.

  \param R      Pointer to VTK renderer.
  \param A      Pointer to VTK actor to be removed.
  \return Returns true if successfull, false otherwise.
  Note that function returns true if given actor does not exist.
*/
inline
bool
VTKRemoveActorFromRenderer_inline(
                                  vtkRenderer * const R,
                                  vtkProp * const A
                                  )
{
  assert( (NULL != R) && (NULL != A) );
  if ( (NULL == R) || (NULL == A) ) return false;

  bool removed = true;

  if (0 != R->GetActors()->IsItemPresent(A))
    {
      R->RemoveActor(A);
    }
  else
    {
      /* If actor does not exist in the rendered we return true
         as the actor is effectively removed.
      */
      //removed = false;
    }
  /* if */

  return removed;
}
/* VTKRemoveActorFromRenderer_inline */



//! Get next visibility status.
/*!
  This function may be used to cycle through all available visibility states.

  \param status Current visibility state.
  \return Returns ID of the next visibility state.
*/
inline
VisibilityStatus
VTKNextVisibilityStatus_inline(
                               VisibilityStatus const status
                               )
{
  VisibilityStatus next = VTK_ACTOR_TRANSPARENT;

  switch (status)
    {
    default:
    case VTK_ACTOR_INVISIBLE: next = VTK_ACTOR_TRANSPARENT; break;
    case VTK_ACTOR_TRANSPARENT: next = VTK_ACTOR_OPAQUE; break;
    case VTK_ACTOR_OPAQUE: next = VTK_ACTOR_INVISIBLE; break;
    }
  /* switch */

  return next;
}
/* VTKNextVisibilityStatus_inline */



//! Changes opacity of slicing planes.
/*!
  Function changes opacity of slicing planes.
  1.0 is totally opaque and 0.0 is completely transparent.

  \param P      Pointer to VTK window data.
  \param opacity        New opacity.
*/
inline
void
VTKSetSlicingPlaneOpacity_inline(
                                 VTKwindowdata * const P,
                                 double const opacity
                                 )
{
  assert(NULL != P);
  if (NULL == P) return;

  if (NULL != P->planeAxial) P->planeAxial->Actor->GetProperty()->SetOpacity(opacity);
  if (NULL != P->planeCoronal) P->planeCoronal->Actor->GetProperty()->SetOpacity(opacity);
  if (NULL != P->planeSagittal) P->planeSagittal->Actor->GetProperty()->SetOpacity(opacity);
}
/* VTKSetSlicingPlaneOpacity_inline */



//! Cycle thorugh slicing plane opacities.
/*!
  Function cycles through slicing plane opacities.

  \param P      Pointer to VTK window data.
*/
inline
void
VTKCycleSlicingPlaneOpacities_inline(
                                     VTKwindowdata * const P
                                     )
{
  assert(NULL != P);
  if (NULL == P) return;

  VisibilityStatus const next = VTKNextVisibilityStatus_inline(P->slicing_planes_visibility);
  P->slicing_planes_visibility = next;

  switch (next)
    {
    default:
    case VTK_ACTOR_INVISIBLE:
      {
        VTKSetSlicingPlaneOpacity_inline(P, 0.0);
      }
      break;

    case VTK_ACTOR_TRANSPARENT:
      {
        VTKSetSlicingPlaneOpacity_inline(P, 0.25);
      }
      break;

    case VTK_ACTOR_OPAQUE:
      {
        VTKSetSlicingPlaneOpacity_inline(P, 1.0);
      }
      break;
    }
  /* switch */
}
/* VTKCycleSlicingPlaneOpacities_inline */



//! Toggles VTK actor in renderer.
/*!
  If an actor is part of a VTK renderer then it is removed, otherwise it is added.

  \param R      Pointer to VTK renderer.
  \param A      Pointer to VTK actor to be added or removed.
  \return Returns true if successfull, false otherwise.
*/
inline
bool
VTKToggleActorInRenderer_inline(
                                vtkRenderer * const R,
                                vtkProp * const A
                                )
{
  assert( (NULL != R) && (NULL != A) );
  if ( (NULL == R) || (NULL == A) ) return false;

  if (0 == R->GetActors()->IsItemPresent(A))
    {
      R->AddActor(A);
    }
  else
    {
      R->RemoveActor(A);
    }
  /* if */

  return true;
}
/* VTKToggleActorInRenderer_inline */



//! Updates slicing plane.
/*!
  Function recomputes vertecis of the polygon that represents the slicing plane.

  \param P      Pointer to slicing plane data.
  \param nrm    Pointer to plane normal. May be NULL.
  \param pt     Pointer to plane point. May be NULL.
  \param bds    Pointer to bounding box values. May be NULL.
*/
inline
void
VTKUpdateSlicingPlane_inline(
                             VTKslicingplane * const P,
                             double const * const nrm,
                             double const * const pt,
                             double const * const bds
                             )
{
  assert(NULL != P);
  if (NULL == P) return;

  bool update = false;

  if (NULL != nrm)
    {
      P->nx = nrm[0];
      P->ny = nrm[1];
      P->nz = nrm[2];
      update = true;
    }
  /* if */

  if (NULL != pt)
    {
      P->px = pt[0];
      P->py = pt[1];
      P->pz = pt[2];
      update = true;
    }
  /* if */

  if (NULL != bds)
    {
      P->bounds[0] = bds[0];  P->bounds[1] = bds[1]; // Bounds in x axis.
      P->bounds[2] = bds[2];  P->bounds[3] = bds[3]; // Bounds in y axis.
      P->bounds[4] = bds[4];  P->bounds[5] = bds[5]; // Bounds in z axis.
      update = true;
    }
  /* if */

  // If no data was changed return immediately.
  if (false == update) return;

  assert(NULL != P->Plane);
  if (NULL == P->Plane) return;

  assert(NULL != P->Points);
  if (NULL == P->Points) return;

  assert(NULL != P->Polygon);
  if (NULL == P->Polygon) return;

  assert(NULL != P->Polygons);
  if (NULL == P->Polygons) return;

  // Predefine 12 possible vertices of the intersection.
  double const x_min = P->bounds[0];  double const x_max = P->bounds[1];
  double const y_min = P->bounds[2];  double const y_max = P->bounds[3];
  double const z_min = P->bounds[4];  double const z_max = P->bounds[5];

  assert( x_min <= x_max );
  assert( y_min <= y_max );
  assert( z_min <= z_max );

  double vtx[12][3] = {
    {0, y_min, z_min},
    {0, y_max, z_min},
    {0, y_min, z_max},
    {0, y_max, z_max},
    {x_min, 0, z_min},
    {x_max, 0, z_min},
    {x_min, 0, z_max},
    {x_max, 0, z_max},
    {x_min, y_min, 0},
    {x_max, y_min, 0},
    {x_min, y_max, 0},
    {x_max, y_max, 0}
  };

  bool valid[12] = {false, false, false, false, false, false, false, false, false, false, false, false};

  // Compute plane equation coefficients.
  double const P0 = P->nx;
  double const P1 = P->ny;
  double const P2 = P->nz;
  double const P3 = -(P->nx * P->px + P->ny * P->py + P->nz * P->pz);

  // Compute intersections.
  if ( FLT_EPSILON < fabs(P0) )
    {
      for (int idx = 0; idx < 4; ++idx)
        {
          valid[idx] = true;
          vtx[idx][0] = - (P1 * vtx[idx][1] + P2 * vtx[idx][2] + P3) / P0;
        }
      /* for */
    }
  /* if */

  if ( FLT_EPSILON < fabs(P1) )
    {
      for (int idx = 4; idx < 8; ++idx)
        {
          valid[idx] = true;
          vtx[idx][1] = - (P0 * vtx[idx][0] + P2 * vtx[idx][2] + P3) / P1;
        }
      /* for */
    }
  /* if */

  if ( FLT_EPSILON < fabs(P2) )
    {
      for (int idx = 8; idx < 12; ++idx)
        {
          valid[idx] = true;
          vtx[idx][2] = - (P0 * vtx[idx][0] + P1 * vtx[idx][1] + P3) / P2;
        }
      /* for */
    }
  /* if */

  // Retain only valid intersections.
  int N = 0;
  double cmx = 0;
  double cmy = 0;
  double cmz = 0;
  for (int i = 0; i < 12; ++i)
    {
      valid[i] = valid[i] &&
        (x_min - FLT_EPSILON <= vtx[i][0]) && (vtx[i][0] <= x_max + FLT_EPSILON) &&
        (y_min - FLT_EPSILON <= vtx[i][1]) && (vtx[i][1] <= y_max + FLT_EPSILON) &&
        (z_min - FLT_EPSILON <= vtx[i][2]) && (vtx[i][2] <= z_max + FLT_EPSILON);
      if (true == valid[i])
        {
          cmx += vtx[i][0];
          cmy += vtx[i][1];
          cmz += vtx[i][2];
          assert(N <= i);
          vtx[N][0] = vtx[i][0];
          vtx[N][1] = vtx[i][1];
          vtx[N][2] = vtx[i][2];
          ++N;
        }
      /* if */
    }
  /* for */
  double const invN = 1.0 / N;
  cmx = cmx * invN;
  cmy = cmy * invN;
  cmz = cmz * invN;

  if (0 < N)
    {
      // Now vertices must be sorted in either clock or anti-clock wise order so
      // filled polygon may be correctly drawn. To to this first we use the
      // center of mass to compute all vectors connecting the center of mass and edge
      // vertices. Then dot-products are used to order points.

      std::vector<VTK3tuple> vec(N);
      for (int i = 0; i < N; ++i)
        {
          VTK3tuple T;
          T.x = vtx[i][0] - cmx;
          T.y = vtx[i][1] - cmy;
          T.z = vtx[i][2] - cmz;
          double const len2 = T.x * T.x + T.y * T.y + T.z * T.z;
          double const ilen = 1.0 / sqrt(len2);
          T.x = T.x * ilen;
          T.y = T.y * ilen;
          T.z = T.z * ilen;
          vec[i] = T;
        }
      /* for */

      VTK3tuple vec_1 = vec[1]; // First in-plane vector.

      std::vector<double> dp_1(N);
      int idx_2 = 0;
      double val_2 = BATCHACQUISITION_pINF_dv;
      for (int i = 0; i < N; ++i)
        {
          VTK3tuple const T = vec[i];
          double const dot_prd = vec_1.x * T.x + vec_1.y * T.y + vec_1.z * T.z;
          dp_1[i] = dot_prd;
          if (fabs(dot_prd) < val_2)
            {
              val_2 = fabs(dot_prd);
              idx_2 = i;
            }
          /* if */
        }
      /* for */

      VTK3tuple vec_2 = vec[idx_2]; // Second in-plane vector orthogonal to vec_1.
      vec_2.x -= dp_1[idx_2] * vec_1.x;
      vec_2.y -= dp_1[idx_2] * vec_1.y;
      vec_2.z -= dp_1[idx_2] * vec_1.z;

      std::vector<double> dp_2(N);
      int N_pos = 0;
      for (int i = 0; i < N; ++i)
        {
          VTK3tuple const T = vec[i];
          double const dot_prd = vec_2.x * T.x + vec_2.y * T.y + vec_2.z * T.z;
          dp_2[i] = dot_prd;
          if (0 <= dot_prd) ++N_pos;
        }
      /* for */
      int const N_neg = N - N_pos;

      std::vector<int> idx(N);
      std::iota(idx.begin(), idx.end(), 0);

      std::vector<int> idx_pos(N_pos);
      std::vector<int> idx_neg(N_neg);

      std::vector<double> dp_pos(N_pos);
      std::vector<double> dp_neg(N_neg);

      int n_pos = 0;
      int n_neg = 0;
      for (int i = 0; i < N; ++i)
        {
          if (0 <= dp_2[i])
            {
              assert( (0 <= n_pos) && (n_pos < N_pos) );
              dp_pos[n_pos] = dp_1[i];
              idx_pos[n_pos] = idx[i];
              ++n_pos;
            }
          else
            {
              assert( (0 <= n_neg) && (n_neg < N_neg) );
              dp_neg[n_neg] = dp_1[i];
              idx_neg[n_neg] = idx[i];
              ++n_neg;
            }
          /* if */
        }
      /* for */

      std::vector<int> srt_pos(N_pos);
      std::iota(srt_pos.begin(), srt_pos.end(), 0);
      std::sort(srt_pos.begin(), srt_pos.end(), [&dp_pos](int i1, int i2) {return dp_pos[i1] > dp_pos[i2];} );

      std::vector<int> srt_neg(N_neg);
      std::iota(srt_neg.begin(), srt_neg.end(), 0);
      std::sort(srt_neg.begin(), srt_neg.end(), [&dp_neg](int i1, int i2) {return dp_neg[i1] < dp_neg[i2];} );

      // Generate new points.
      P->Points->SetDataType(VTK_FLOAT);
      P->Points->Allocate(N);

      int ID = 0;
      for (int i = 0; i < N_pos; ++i)
        {
          int const i1 = srt_pos[i];
          assert( (0 <= i1) && (i1 < N_pos) );
          int const i2 = idx_pos[i1];
          assert( (0 <= i2) && (i2 < N) );
          P->Points->InsertPoint(ID, vtx[i2]);
          ++ID;
        }
      /* for */

      for (int i = 0; i < N_neg; ++i)
        {
          int const i1 = srt_neg[i];
          assert( (0 <= i1) && (i1 < N_neg) );
          int const i2 = idx_neg[i1];
          assert( (0 <= i2) && (i2 < N) );
          P->Points->InsertPoint(ID, vtx[i2]);
          ++ID;
        }
      /* for */

      assert(ID == N);

      // Generate polygon.
      P->Polygon->GetPointIds()->SetNumberOfIds(N);
      for (int i = 0; i < N; ++i) P->Polygon->GetPointIds()->SetId(i, i);

      P->Polygons->Initialize();
      P->Polygons->InsertNextCell(P->Polygon);

      P->Points->Modified();
      P->Polygon->Modified();
      P->Polygons->Modified();

      P->Plane->SetPoints(P->Points);
      P->Plane->SetPolys(P->Polygons);
    }
  else
    {
      P->Points->Initialize();
      P->Polygon->Initialize();
      P->Polygons->Initialize();

      P->Points->Modified();
      P->Polygon->Modified();
      P->Polygons->Modified();

      P->Plane->SetPoints(P->Points);
      P->Plane->SetPolys(P->Polygons);
    }
  /* if */
}
/* VTKUpdateSlicingPlane_inline */



//! Sets bounding box for slicing planes.
/*!
  Updates bounding box for slicing planes.

  \param D      Pointer to VTK thread data structure.
*/
inline
void
VTKSetSlicingPlaneBounds_inline(
                                VTKdisplaythreaddata * const D
                                )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != D->window);
  if (NULL == D->window) return;

  double bounds[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

  bool const fetch = VTKFetchDataCenterAndBounds_inline(D, NULL, bounds);
  assert(true == fetch);
  if (true == fetch)
    {
      VTKUpdateSlicingPlane_inline(D->window->planeAxial, NULL, NULL, bounds);
      VTKUpdateSlicingPlane_inline(D->window->planeCoronal, NULL, NULL, bounds);
      VTKUpdateSlicingPlane_inline(D->window->planeSagittal, NULL, NULL, bounds);
    }
  /* if */
}
/* VTKSetSlicingPlaneBounds_inline */



//! Get next threshold control.
/*!
  This function may be used to cycle through all available threshold controls.

  \param type   Current trehshold control.
  \return   Returns ID of the next threshold control.
*/
inline
ThresholdControl
VTKNextThresholdControl_inline(
                               ThresholdControl const type
                               )
{
  ThresholdControl next = VTK_THRESHOLD_RANGE;

  switch (type)
    {
    case VTK_THRESHOLD_RANGE: next = VTK_THRESHOLD_RAY_DISTANCE; break;
    case VTK_THRESHOLD_RAY_DISTANCE: next = VTK_THRESHOLD_PHASE_DISTANCE; break;
    case VTK_THRESHOLD_PHASE_DISTANCE: next = VTK_THRESHOLD_PHASE_DEVIATION; break;
    default:
    case VTK_THRESHOLD_PHASE_DEVIATION: next = VTK_THRESHOLD_RANGE; break;
    }
  /* switch */

  return next;
}
/* VTKNextThresholdControl_inline */



//! Updates threshold widget.
/*!
  Updates the label and values on the threshold widget.

  \param points Pointer to point cloud data.
  \param window Pointer to VTK window data.
*/
inline
void
VTKUpdateThresholdSliderWidget_inline(
                                      VTKpointclouddata * const points,
                                      VTKwindowdata * const window
                                      )
{
  assert( (NULL != window) && (NULL != window->sldThrRep) );
  if ( (NULL == window) || (NULL == window->sldThrRep) ) return;

  ThresholdControl const type = (NULL != points)? points->thresholdType : VTK_THRESHOLD_UNKNOWN;

  int const bufsz = 1024;
  char buffer[bufsz + 1];
  buffer[bufsz] = 0;

  switch (type)
    {
    case VTK_THRESHOLD_RANGE:
      assert(NULL != points);
      if (NULL != points)
        {
          window->sldThrRep->SetMinimumValue(points->rangeMin);
          window->sldThrRep->SetMaximumValue(points->rangeMax);
          window->sldThrRep->SetValue(points->rangeThr);

          int const cnt = sprintf_s(buffer, bufsz, gMsgThresholdDynamicRange, points->CameraID + 1, points->ProjectorID + 1);
          window->sldThrRep->SetTitleText(buffer);

          break;
        }
      /* if */

    case VTK_THRESHOLD_RAY_DISTANCE:
      assert(NULL != points);
      if (NULL != points)
        {
          window->sldThrRep->SetMinimumValue(points->rayDistanceMin);
          window->sldThrRep->SetMaximumValue(points->rayDistanceMax);
          window->sldThrRep->SetValue(points->rayDistanceThr);

          int const cnt = sprintf_s(buffer, bufsz, gMsgThresholdRayDistance, points->CameraID + 1, points->ProjectorID + 1);
          window->sldThrRep->SetTitleText(buffer);
          break;
        }
      /* if */

    case VTK_THRESHOLD_PHASE_DISTANCE:
      assert(NULL != points);
      if (NULL != points)
        {
          window->sldThrRep->SetMinimumValue(points->phaseDistanceMin);
          window->sldThrRep->SetMaximumValue(points->phaseDistanceMax);
          window->sldThrRep->SetValue(points->phaseDistanceThr);

          int const cnt = sprintf_s(buffer, bufsz, gMsgThresholdPhaseDistance, points->CameraID + 1, points->ProjectorID + 1);
          window->sldThrRep->SetTitleText(buffer);
          break;
        }
      /* if */

    case VTK_THRESHOLD_PHASE_DEVIATION:
      assert(NULL != points);
      if (NULL != points)
        {
          window->sldThrRep->SetMinimumValue(points->phaseDeviationMin);
          window->sldThrRep->SetMaximumValue(points->phaseDeviationMax);
          window->sldThrRep->SetValue(points->phaseDeviationThr);

          int const cnt = sprintf_s(buffer, bufsz, gMsgThresholdPhaseDeviation, points->CameraID + 1, points->ProjectorID + 1);
          window->sldThrRep->SetTitleText(buffer);
          break;
        }
      /* if */

    case VTK_THRESHOLD_UNKNOWN:
    default:
      {
        window->sldThrRep->SetMinimumValue(0);
        window->sldThrRep->SetMaximumValue(255);
        window->sldThrRep->SetValue(0);
        window->sldThrRep->SetTitleText(gMsgThresholdNoData);
      }
      break;
    }
  /* switch */
}
/* VTKUpdateThresholdSliderWidget_inline */



//! Updates point mask.
/*!
  Function updates point mask. Each point in the point cloud has its corresponding mask.
  The mask value of 0 means the point is to be rendered. Values other than zero means
  the point must be masked and should not be rendered. The mask values are assigned
  to bits as follows:
  1) the LSB indicates the point is masked due to dynamic range threshold,
  2) next bit indicates the point is masked due to ray-to-ray 3D distance,
  3) next bit indicates the point is masked due to beeing an outlier.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param points Pointer to point cloud data structure.
*/
inline
void
VTKUpdateSelectionMask_inline(
                              VTKpointclouddata * const points
                              )
{
  assert(NULL != points);
  if (NULL == points) return;

  std::vector<unsigned char> * const pMask = points->pMask;
  assert(NULL != pMask);
  if (NULL == pMask) return;

  int const N = (int)(pMask->size());
  unsigned char * const dst = &( pMask->front() );
  assert(NULL != dst);

  switch (points->thresholdType)
    {

    default:
    case VTK_THRESHOLD_RANGE:
      {
        std::vector<float> * const pDynamicRange = points->pDynamicRange;
        assert( (NULL != pDynamicRange) && (N == (int)(pDynamicRange->size())) );
        if ( (NULL == pDynamicRange) || (N != (int)(pDynamicRange->size())) ) break;

        float const * const src = &( pDynamicRange->front() );
        assert(NULL != src);
        if (NULL == src) break;

        float const thr_f = points->rangeThr;
        assert( (points->rangeMin <= thr_f) && (thr_f <= points->rangeMax) );

        int i = 0;
        int const i_max = N - 7;

        unsigned char const voxel_off = 0x01; // Turn off point visibility.
        unsigned char const voxel_on = 0xFE; // Turn on point visibility.
        assert( 0xFF == (voxel_on ^ voxel_off) );

        // Unroll the loop with step eight.
        for (; i < i_max; i += 8)
          {
            if (src[i    ] >= thr_f) dst[i    ] &= voxel_on; else dst[i    ] |= voxel_off;
            if (src[i + 1] >= thr_f) dst[i + 1] &= voxel_on; else dst[i + 1] |= voxel_off;
            if (src[i + 2] >= thr_f) dst[i + 2] &= voxel_on; else dst[i + 2] |= voxel_off;
            if (src[i + 3] >= thr_f) dst[i + 3] &= voxel_on; else dst[i + 3] |= voxel_off;
            if (src[i + 4] >= thr_f) dst[i + 4] &= voxel_on; else dst[i + 4] |= voxel_off;
            if (src[i + 5] >= thr_f) dst[i + 5] &= voxel_on; else dst[i + 5] |= voxel_off;
            if (src[i + 6] >= thr_f) dst[i + 6] &= voxel_on; else dst[i + 6] |= voxel_off;
            if (src[i + 7] >= thr_f) dst[i + 7] &= voxel_on; else dst[i + 7] |= voxel_off;
          }
        /* for */

        // Process remainding items.
        for (; i < N; ++i)
          {
            if (src[i] >= thr_f) dst[i] &= voxel_on; else dst[i] |= voxel_off;
          }
        /* for */
      }
      break;

    case VTK_THRESHOLD_RAY_DISTANCE:
      {
        std::vector<float> * const pRayDistance = points->pRayDistance;
        assert( (NULL != pRayDistance) && (N == (int)(pRayDistance->size())) );
        if ( (NULL == pRayDistance) || (N != (int)(pRayDistance->size())) ) break;

        float const * const src = &( pRayDistance->front() );
        assert(NULL != src);
        if (NULL == src) break;

        float const thr_f = points->rayDistanceThr;
        assert( (points->rayDistanceMin <= thr_f) && (thr_f <= points->rayDistanceMax) );

        int i = 0;
        int const i_max = N - 7;

        unsigned char const voxel_off = 0x02; // Turn off point visibility.
        unsigned char const voxel_on = 0xFD; // Turn on point visibility.
        assert( 0xFF == (voxel_on ^ voxel_off) );

        // Unroll the loop with step eight.
        for (; i < i_max; i += 8)
          {
            if (src[i    ] <= thr_f) dst[i    ] &= voxel_on; else dst[i    ] |= voxel_off;
            if (src[i + 1] <= thr_f) dst[i + 1] &= voxel_on; else dst[i + 1] |= voxel_off;
            if (src[i + 2] <= thr_f) dst[i + 2] &= voxel_on; else dst[i + 2] |= voxel_off;
            if (src[i + 3] <= thr_f) dst[i + 3] &= voxel_on; else dst[i + 3] |= voxel_off;
            if (src[i + 4] <= thr_f) dst[i + 4] &= voxel_on; else dst[i + 4] |= voxel_off;
            if (src[i + 5] <= thr_f) dst[i + 5] &= voxel_on; else dst[i + 5] |= voxel_off;
            if (src[i + 6] <= thr_f) dst[i + 6] &= voxel_on; else dst[i + 6] |= voxel_off;
            if (src[i + 7] <= thr_f) dst[i + 7] &= voxel_on; else dst[i + 7] |= voxel_off;
          }
        /* for */

        // Process remainding items.
        for (; i < N; ++i)
          {
            if (src[i] <= thr_f) dst[i] &= voxel_on; else dst[i] |= voxel_off;
          }
        /* for */
      }
      break;

    case VTK_THRESHOLD_PHASE_DISTANCE:
      {
        std::vector<float> * const pPhaseDistance = points->pPhaseDistance;
        //assert( (NULL != pPhaseDistance) && (N == (int)(pPhaseDistance->size())) );
        if ( (NULL == pPhaseDistance) || (N != (int)(pPhaseDistance->size())) ) break;

        float const * const src = &( pPhaseDistance->front() );
        assert(NULL != src);
        if (NULL == src) break;

        float const thr_f = points->phaseDistanceThr;
        assert( (points->phaseDistanceMin <= thr_f) && (thr_f <= points->phaseDistanceMax) );

        int i = 0;
        int const i_max = N - 7;

        unsigned char const voxel_off = 0x04; // Turn off point visibility.
        unsigned char const voxel_on = 0xFB; // Turn on point visibility.
        assert( 0xFF == (voxel_on ^ voxel_off) );

        // Unroll the loop with step eight.
        for (; i < i_max; i += 8)
          {
            if (src[i    ] <= thr_f) dst[i    ] &= voxel_on; else dst[i    ] |= voxel_off;
            if (src[i + 1] <= thr_f) dst[i + 1] &= voxel_on; else dst[i + 1] |= voxel_off;
            if (src[i + 2] <= thr_f) dst[i + 2] &= voxel_on; else dst[i + 2] |= voxel_off;
            if (src[i + 3] <= thr_f) dst[i + 3] &= voxel_on; else dst[i + 3] |= voxel_off;
            if (src[i + 4] <= thr_f) dst[i + 4] &= voxel_on; else dst[i + 4] |= voxel_off;
            if (src[i + 5] <= thr_f) dst[i + 5] &= voxel_on; else dst[i + 5] |= voxel_off;
            if (src[i + 6] <= thr_f) dst[i + 6] &= voxel_on; else dst[i + 6] |= voxel_off;
            if (src[i + 7] <= thr_f) dst[i + 7] &= voxel_on; else dst[i + 7] |= voxel_off;
          }
        /* for */

        // Process remainding items.
        for (; i < N; ++i)
          {
            if (src[i] <= thr_f) dst[i] &= voxel_on; else dst[i] |= voxel_off;
          }
        /* for */
      }
      break;

    case VTK_THRESHOLD_PHASE_DEVIATION:
      {
        std::vector<float> * const pPhaseDeviation = points->pPhaseDeviation;
        //assert( (NULL != pPhaseDeviation) && (N == (int)(pPhaseDeviation->size())) );
        if ( (NULL == pPhaseDeviation) || (N != (int)(pPhaseDeviation->size())) ) break;

        float const * const src = &( pPhaseDeviation->front() );
        assert(NULL != src);
        if (NULL == src) break;

        float const thr_f = points->phaseDeviationThr;
        assert( (points->phaseDeviationMin <= thr_f) && (thr_f <= points->phaseDeviationMax) );

        int i = 0;
        int const i_max = N - 7;

        unsigned char const voxel_off = 0x08; // Turn off point visibility.
        unsigned char const voxel_on = 0xF7; // Turn on point visibility.
        assert( 0xFF == (voxel_on ^ voxel_off) );

        // Unroll the loop with step eight.
        for (; i < i_max; i += 8)
          {
            if (src[i    ] <= thr_f) dst[i    ] &= voxel_on; else dst[i    ] |= voxel_off;
            if (src[i + 1] <= thr_f) dst[i + 1] &= voxel_on; else dst[i + 1] |= voxel_off;
            if (src[i + 2] <= thr_f) dst[i + 2] &= voxel_on; else dst[i + 2] |= voxel_off;
            if (src[i + 3] <= thr_f) dst[i + 3] &= voxel_on; else dst[i + 3] |= voxel_off;
            if (src[i + 4] <= thr_f) dst[i + 4] &= voxel_on; else dst[i + 4] |= voxel_off;
            if (src[i + 5] <= thr_f) dst[i + 5] &= voxel_on; else dst[i + 5] |= voxel_off;
            if (src[i + 6] <= thr_f) dst[i + 6] &= voxel_on; else dst[i + 6] |= voxel_off;
            if (src[i + 7] <= thr_f) dst[i + 7] &= voxel_on; else dst[i + 7] |= voxel_off;
          }
        /* for */

        // Process remainding items.
        for (; i < N; ++i)
          {
            if (src[i] <= thr_f) dst[i] &= voxel_on; else dst[i] |= voxel_off;
          }
        /* for */
      }
      break;
    }
  /* switch */

}
/* VTKUpdateSelectionMask_inline */



//! Clears selection mask.
/*!
  Resets selection mask for the active threshold.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param points Pointer to point cloud data structure.
*/
inline
void
VTKClearSelectionMask_inline(
                             VTKpointclouddata * const points
                             )
{
  assert(NULL != points);
  if (NULL == points) return;

  std::vector<unsigned char> * const pMask = points->pMask;
  assert(NULL != pMask);
  if (NULL == pMask) return;

  int const N = (int)(pMask->size());
  unsigned char * const msk = &( pMask->front() );
  assert(NULL != msk);

  switch (points->thresholdType)
    {

    default:
    case VTK_THRESHOLD_RANGE:
      {
        unsigned char const voxel_on = 0xFE; // Turn on point visibility.
        for (int i = 0; i < N; ++i) msk[i] &= voxel_on;
      }
      break;

    case VTK_THRESHOLD_RAY_DISTANCE:
      {
        unsigned char const voxel_on = 0xFD; // Turn on point visibility.
        for (int i = 0; i < N; ++i) msk[i] &= voxel_on;
      }
      break;

    case VTK_THRESHOLD_PHASE_DISTANCE:
      {
        unsigned char const voxel_on = 0xFB; // Turn on point visibility.
        for (int i = 0; i < N; ++i) msk[i] &= voxel_on;
      }
      break;

    case VTK_THRESHOLD_PHASE_DEVIATION:
      {
        unsigned char const voxel_on = 0xF7; // Turn on point visibility.
        for (int i = 0; i < N; ++i) msk[i] &= voxel_on;
      }
      break;
    }
  /* switch */

}
/* VTKClearSelectionMask_inline */



//! Resets all selection masks.
/*!
  Resets all selection masks and adjusts threshold so all points are visible.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param points Pointer to point cloud data structure.
*/
inline
void
VTKResetSelectionMask_inline(
                             VTKpointclouddata * const points
                             )
{
  assert(NULL != points);
  if (NULL == points) return;

  std::vector<unsigned char> * const pMask = points->pMask;
  assert(NULL != pMask);
  if (NULL == pMask) return;

  vtkUnsignedCharArray * const ColorsMapped = points->ColorsMapped;
  assert(NULL != ColorsMapped);
  if (NULL == ColorsMapped) return;

  // Get number of points.
  int const N = (int)(pMask->size());

  // Reset mask.
  unsigned char * const msk = &( pMask->front() );
  assert(NULL != msk);
  int i = 0;
  for (; i < N - 3; i += 4) msk[i] = 0, msk[i+1] = 0, msk[i+2] = 0, msk[i+3] = 0;
  for (; i < N; ++i) msk[i] = 0;

  // Remove transparency from all points.
  assert( N == ColorsMapped->GetNumberOfTuples() );
  assert( 4 == ColorsMapped->GetNumberOfComponents() );
  unsigned char * const clr = ColorsMapped->WritePointer(0, 0) + 3;
  assert(NULL != clr);
  i = 0;
  for (; i < N; ++i) clr[4*i] = 255;

  // Adjust thresholds.
  points->rangeThr = points->rangeMin;
  points->rayDistanceThr = points->rayDistanceMax;
  points->phaseDistanceThr = points->phaseDistanceMax;
  points->phaseDeviationThr = points->phaseDeviationMax;

  // Mark color data updated.
  ColorsMapped->DataChanged();
  ColorsMapped->Modified();
}
/* VTKResetSelectionMask_inline */



//! Updates point colors.
/*!
  Updates point colors.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param points Pointer to point cloud data structure.
*/
inline
void
VTKUpdatePointColors_inline(
                            VTKpointclouddata * const points
                            )
{
  assert(NULL != points);
  if (NULL == points) return;

  vtkUnsignedCharArray * const clrM = points->ColorsMapped;
  vtkUnsignedCharArray * const clrO = points->ColorsOriginal;
  assert( (NULL != clrM) && (NULL != clrO) );
  if ( (NULL == clrM) || (NULL == clrO) ) return;

  int const N = clrM->GetNumberOfTuples();
  assert( N == clrO->GetNumberOfTuples() );
  assert( 4 == clrM->GetNumberOfComponents() );
  assert( 4 == clrO->GetNumberOfComponents() );

  unsigned char const * const src = clrO->WritePointer(0,0);
  unsigned char * const dst = clrM->WritePointer(0,0);

  float const scale = points->colorScale;
  float const offset = points->colorOffset;

  for (int i = 0; i < N; ++i)
    {
      int const adr = 4 * i;

      float const r1 = (float)( src[adr    ] );
      float const g1 = (float)( src[adr + 1] );
      float const b1 = (float)( src[adr + 2] );

      float const r2 = scale * r1 + offset;
      float const g2 = scale * g1 + offset;
      float const b2 = scale * b1 + offset;

      dst[adr    ] = (r2 > 255.0)? 255 : ( (r2 < 0.0) ? 0 : (unsigned char)(r2) );
      dst[adr + 1] = (g2 > 255.0)? 255 : ( (g2 < 0.0) ? 0 : (unsigned char)(g2) );
      dst[adr + 2] = (b2 > 255.0)? 255 : ( (b2 < 0.0) ? 0 : (unsigned char)(b2) );
    }
  /* for */

  clrM->DataChanged();
  clrM->Modified();
}
/* VTKUpdatePointColors_inline */



//! Resets point colors.
/*!
  Resets point colors.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param points Pointer to point cloud data structure.
*/
inline
void
VTKResetPointColors_inline(
                           VTKpointclouddata * const points
                           )
{
  assert(NULL != points);
  if (NULL == points) return;

  vtkUnsignedCharArray * const clrM = points->ColorsMapped;
  vtkUnsignedCharArray * const clrO = points->ColorsOriginal;
  assert( (NULL != clrM) && (NULL != clrO) );
  if ( (NULL == clrM) || (NULL == clrO) ) return;

  int const N = clrM->GetNumberOfTuples();
  assert( N == clrO->GetNumberOfTuples() );
  assert( 4 == clrM->GetNumberOfComponents() );
  assert( 4 == clrO->GetNumberOfComponents() );

  unsigned char const * const src = clrO->WritePointer(0,0);
  unsigned char * const dst = clrM->WritePointer(0,0);

  for (int i = 0; i < N; ++i)
    {
      int const adr = 4 * i;
      dst[adr    ] = src[adr    ];
      dst[adr + 1] = src[adr + 1];
      dst[adr + 2] = src[adr + 2];
    }
  /* for */

  clrM->DataChanged();
  clrM->Modified();
}
/* VTKResetPointColors_inline */



//! Counts point in regard to slicing plane.
/*!
  Counts points in the front and in the back of a slicing plane.
  Only opaque points are counted; transparent points are ignored.

  \param plane  Pointer to slicing plane structure.
  \param points Pointer to point cloud.
  \param total_out      Address where the total number of points will be stored.
  \param front_out      Address where the number of points in front will be stored.
  \param back_out       Address where the number of points in the back will be stored.
*/
inline
void
VTKCountPointsInFrontOfSlicingPlanes_inline(
                                            VTKslicingplane * const plane,
                                            VTKpointclouddata * const points,
                                            double * const total_out,
                                            double * const front_out,
                                            double * const back_out
                                            )
{
  assert(NULL != plane);
  if (NULL == plane) return;

  assert(NULL != points);
  if (NULL == points) return;

  assert( (NULL != points->Cloud) && (NULL != points->ColorsMapped) );
  if ( (NULL == points->Cloud) || (NULL == points->ColorsMapped) ) return;

  int const type = points->Cloud->GetDataType();
  assert(VTK_FLOAT == type);
  if (VTK_FLOAT != type) return;

  // Get plane coefficients.
  double const A = plane->nx;
  double const B = plane->ny;
  double const C = plane->nz;
  double const D = -(plane->px * A + plane->py * B + plane->pz * C);

  // Get data pointers.
  int const N = points->Cloud->GetNumberOfPoints();
  float const * const src_pt = (float *)( points->Cloud->GetVoidPointer(0) );

  assert( N == points->ColorsMapped->GetNumberOfTuples() );
  assert( 4 == points->ColorsMapped->GetNumberOfComponents() );
  unsigned char const * const src_rgba = points->ColorsMapped->WritePointer(0, 0);

  int total = 0;
  int front = 0;
  int back = 0;

  for (int i = 0; i < N; ++i)
    {
      int const adr_pt = 3 * i;
      double const x = src_pt[adr_pt    ];
      double const y = src_pt[adr_pt + 1];
      double const z = src_pt[adr_pt + 2];

      int const adr_rgba = 4 * i;
      double const o = src_rgba[adr_rgba + 3];

      if (0 < o)
        {
          double const dst = A * x + B * y + C * z + D;
          if (0 < dst)
            {
              ++front;
            }
          else
            {
              ++back;
            }
          /* if */
          ++total;
        }
      /* if */
    }
  /* for */
  assert(total <= N);
  assert(total == front + back);

  if (NULL != total_out) *total_out = (double)( total );
  if (NULL != front_out) *front_out = (double)( front );
  if (NULL != back_out) *back_out = (double)( back );
}
/* VTKCountPointsInFrontOfSlicingPlanes_inline */



//! Remove point cloud from display window.
/*!
  Function removes all actors associated with a point cloud.

  Note that this function does not reserve critical section object
  dataCS so it should be reserved before calling.

  \param W      Pointer to VTK window structure.
  \param P      Pointer to point cloud structure.
  \return Function returns true if all actors are successfully removed.
*/
inline
bool
VTKRemovePointCloudFromDisplayWindow_inline(
                                            VTKwindowdata_ * const W,
                                            VTKpointclouddata_ * const P
                                            )
{
  assert( (NULL != W) && (NULL != P) );
  if ( (NULL == W) || (NULL == P) ) return false;

  bool removed = true;

  bool const remove_cloud = VTKRemoveActorFromDisplayWindow(W, P->Actor);
  assert(true == remove_cloud);
  removed = removed && remove_cloud;

  if (NULL != P->outline)
    {
      bool const remove_outline = VTKRemoveActorFromDisplayWindow(W, P->outline->outlineActor);
      assert(true == remove_outline);
      removed = removed && remove_outline;
    }
  /* if */

  if (NULL != P->surface)
    {
      bool const remove_surface = VTKRemoveActorFromDisplayWindow(W, P->surface->surfaceActor);
      assert(true == remove_surface);
      removed = removed && remove_surface;
    }
  /* if */

  return removed;
}
/* VTKRemovePointCloudFromDisplayWindow_inline */



//! Change active point cloud.
/*!
  Function changes active point cloud.

  \param D      Pointer to VTK display thread parameters.
  \param CloudID        Point cloud ID.
*/
inline
void
VTKSetActivePointCloud_inline(
                              VTKdisplaythreaddata * const D,
                              int const CloudID
                              )
{
  assert(NULL != D);
  if (NULL == D) return;

  if (CloudID == D->CloudID) return;

  assert(NULL != D->point_clouds);
  if (NULL == D->point_clouds) return;

  EnterCriticalSection( &(D->dataCS) );
  {
    if (NULL != D->point_clouds)
      {
        int const n = (int)( D->point_clouds->size() );
        //assert( (0 <= CloudID) && (CloudID < n) );
        if ( (0 <= CloudID) && (CloudID < n) )
          {
            VTKpointclouddata_ * const points = ( *(D->point_clouds) )[CloudID];
            //assert(NULL != points);

            /* Update window title. */
            {
              int const bufsz = 1024;
              char buffer[bufsz + 1];
              buffer[bufsz] = 0;

              if (NULL != points)
                {
                  int const CameraID = points->CameraID;
                  int const ProjectorID = points->ProjectorID;

                  int const cnt = sprintf_s(buffer, bufsz, gMsgWindowTitleHaveCloud, CloudID + 1, CameraID + 1, ProjectorID + 1);
                  assert(0 < cnt);
                }
              else
                {
                  int const cnt = sprintf_s(buffer, bufsz, gMsgWindowTitleNoCloud, CloudID + 1);
                  assert(0 < cnt);
                }
              /* if */

              if ( (NULL != D->window) && (NULL != D->window->renWin) ) D->window->renWin->SetWindowName(buffer);
            }

            /* Update threshold slider. */
            VTKUpdateThresholdSliderWidget_inline(points, D->window);

            /* Change point cloud ID. */
            D->CloudID = CloudID;
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(D->dataCS) );

  return;
}
/* VTKSetActivePointCloud_inline */



//! Toggle visibility of a point cloud.
/*!
  Function toggles the visibility of a point cloud.

  \param D      Pointer to VTK display thread parameters.
  \param CloudID        Point cloud ID.
*/
inline
void
VTKTogglePointCloudVisibility_inline(
                                     VTKdisplaythreaddata * const D,
                                     int const CloudID
                                     )
{
  assert(NULL != D);
  if (NULL == D) return;

  assert(NULL != D->window);
  if (NULL == D->window) return;

  assert(NULL != D->point_clouds);
  if (NULL == D->point_clouds) return;

  EnterCriticalSection( &(D->dataCS) );
  {
    if (NULL != D->point_clouds)
      {
        int const n = (int)( D->point_clouds->size() );

        assert( (0 <= CloudID) && (CloudID < n) );
        if ( (0 <= CloudID) && (CloudID < n) )
          {
            VTKpointclouddata_ * const points = ( *(D->point_clouds) )[CloudID];

            if (NULL != points)
              {
                int const count = D->window->ren3D->GetActors()->IsItemPresent(points->Actor);
                if (0 == count)
                  {
                    bool const add = VTKAddActorToDisplayWindow(D->window, points->Actor);
                    assert(true == add);
                  }
                else
                  {
                    bool const remove = VTKRemovePointCloudFromDisplayWindow_inline(D->window, points);
                    assert(true == remove);
                  }
                /* if */
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(D->dataCS) );
}
/* VTKTogglePointCloudVisibility_inline */



//! Save point cloud to PLY.
/*!
  Saves point cloud to PLY format.

  \param data   Pointer to point cloud.
*/
inline
void
VTKSavePointCloudToPLY_inline(
                              VTKpointclouddata_ const * const data
                              )
{
  assert(NULL != data);
  if (NULL == data) return;

  assert(NULL != data->Cloud);
  if (NULL == data->Cloud) return;

  cv::Mat * points = NULL;
  cv::Mat * normals = NULL;
  cv::Mat * colors = NULL;

  HRESULT hr = S_OK;

  UINT iFileType = 1;
  int const rgSize = 1; // Must match number of entries in rgSpec.
  COMDLG_FILTERSPEC const rgSpec[rgSize] =
    {
      { gMsgSaveToPLYExtensionDescription, L"*.ply" } // 1
    };
  int const extSize = 1; // Must match number of entries in extNames.
  wchar_t const * const extNames[extSize] =
    {
      L".ply"
    };
  int const type_to_idx[rgSize + 1] = {-1, 0}; // There are rgSize + 1 types, 0 maps to -1.

  std::wstring filename(L"");
  if (NULL != data->acquisition_name)
    {
      filename = *(data->acquisition_name);
      filename += L".ply";
    }
  /* if */

  std::wstring title(L"");
  if ( (0 <= data->CameraID) && (0 <= data->ProjectorID) )
    {
      int const bufsz = 1024;
      wchar_t buffer[bufsz + 1];
      buffer[bufsz] = 0;

      int const cnt = swprintf_s(buffer, bufsz, gMsgSaveToPLYTitleOneCloud, data->CameraID + 1, data->ProjectorID + 1);
      assert(0 < cnt);

      title = buffer;
    }
  else
    {
      title = gMsgSaveToPLYTitle;
    }
  /* if */

  // Open file dialog box and let user pick the filename.
  hr = FileSaveDialog(
                      filename,
                      title.c_str(),
                      rgSize,
                      rgSpec,
                      extSize,
                      extNames,
                      type_to_idx,
                      &iFileType
                      );
  assert( SUCCEEDED(hr) || (0x800704C7 == hr) );
  if ( !SUCCEEDED(hr) ) return;

  // Create temporary header for point data.
  int const N = data->Cloud->GetNumberOfPoints();
  {
    void * const ptr = data->Cloud->GetVoidPointer(0);
    points = new cv::Mat(N, 3, CV_32FC1, ptr, 3 * sizeof(float));
  }

  // Create temporary header for color data.
  if (NULL != data->ColorsOriginal)
    {
      assert(N == data->ColorsOriginal->GetNumberOfTuples());
      assert(4 == data->ColorsOriginal->GetNumberOfComponents());
      void * const ptr = data->ColorsOriginal->WritePointer(0,0);

      colors = new cv::Mat(N, 4, CV_8UC1, ptr, 4 * sizeof(unsigned char));
    }
  /* if */

  std::vector<cv::Mat * const> points_all;
  std::vector<cv::Mat * const> colors_all;
  std::vector<cv::Mat * const> normals_all;

  points_all.push_back(points);
  colors_all.push_back(colors);
  normals_all.push_back(normals);

  bool const saved = PointCloudSaveToPLY(filename.c_str(), points_all, colors_all, normals_all);
  assert(true == saved);

  SAFE_DELETE( points );
  SAFE_DELETE( normals );
  SAFE_DELETE( colors );
}
/* VTKSavePointCloudToPLY_inline */



//! Save point clouds to PLY.
/*!
  Saves point clouds to PLY format.

  \param data   Pointer to point cloud.
*/
inline
void
VTKSavePointCloudsToPLY_inline(
                               std::vector<VTKpointclouddata_ *> * const point_clouds
                               )
{
  assert(NULL != point_clouds);
  if (NULL == point_clouds) return;

  assert(0 < point_clouds->size());
  if (0 == point_clouds->size()) return;

  HRESULT hr = S_OK;

  UINT iFileType = 1;
  int const rgSize = 1; // Must match number of entries in rgSpec.
  COMDLG_FILTERSPEC const rgSpec[rgSize] =
    {
      { gMsgSaveToPLYExtensionDescription, L"*.ply" } // 1
    };
  int const extSize = 1; // Must match number of entries in extNames.
  wchar_t const * const extNames[extSize] =
    {
      L".ply"
    };
  int const type_to_idx[rgSize + 1] = {-1, 0}; // There are rgSize + 1 types, 0 maps to -1.

  std::wstring filename(L"");

  int const n = (int)( point_clouds->size() );
  for (int i = 0; i < n; ++i)
    {
      VTKpointclouddata_ * const data = ( *(point_clouds) )[i];
      if (NULL == data) continue;
      if (NULL != data->acquisition_name)
        {
          filename = *(data->acquisition_name);
          filename += L".ply";
          break;
        }
      /* if */
    }
  /* for */

  // Open file dialog box and let user pick the filename.
  hr = FileSaveDialog(
                      filename,
                      gMsgSaveToPLYTitleAllClouds,
                      rgSize,
                      rgSpec,
                      extSize,
                      extNames,
                      type_to_idx,
                      &iFileType
                      );
  assert( SUCCEEDED(hr) || (0x800704C7 == hr) );
  if ( !SUCCEEDED(hr) ) return;

  std::vector<cv::Mat * const> points_all;
  std::vector<cv::Mat * const> colors_all;
  std::vector<cv::Mat * const> normals_all;   

  points_all.reserve(n);
  colors_all.reserve(n);
  normals_all.reserve(n);
  
  for (int i = 0; i < n; ++i)
    {
      VTKpointclouddata_ * const data = ( *(point_clouds) )[i];

      cv::Mat * points = NULL;
      cv::Mat * normals = NULL;
      cv::Mat * colors = NULL;

      // Create temporary headers for point data.
      int const N = data->Cloud->GetNumberOfPoints();
      {
        void * const ptr = data->Cloud->GetVoidPointer(0);
        points = new cv::Mat(N, 3, CV_32FC1, ptr, 3 * sizeof(float));
      }

      // Create temporary headers for color data.
      if (NULL != data->ColorsOriginal)
        {
          assert(N == data->ColorsOriginal->GetNumberOfTuples());
          assert(4 == data->ColorsOriginal->GetNumberOfComponents());
          void * const ptr = data->ColorsOriginal->WritePointer(0,0);

          colors = new cv::Mat(N, 4, CV_8UC1, ptr, 4 * sizeof(unsigned char));
        }
      /* if */

      points_all.push_back(points);
      colors_all.push_back(colors);
      normals_all.push_back(normals);
    }
  /* for */

  bool const saved = PointCloudSaveToPLY(filename.c_str(), points_all, colors_all, normals_all);
  assert(true == saved);

  for (int i = 0; i < n; ++i)
    {
      SAFE_DELETE( points_all[i] );
      SAFE_DELETE( normals_all[i] );
      SAFE_DELETE( colors_all[i] );
    }
  /* for */
}
/* VTKSavePointCloudsToPLY_inline */



/****** AUXILIARY FUNCTIONS ******/

//! Save VTK scene to file.
/*!
  Save VTK scene to file. File type is determined from its extension.

  \param renderWindow   Pointer to VTK render window.
  \param filename_in       Filename where to store the scene. If NULL file pick dialog will be opened.
  \return Function returns true if successfull.
*/
HRESULT
VTKSaveRenderWindowToFile(
                          vtkRenderWindow * const renderWindow
                          )
{
  assert(NULL != renderWindow);
  if (NULL == renderWindow) return E_POINTER;

  HRESULT hr = S_OK;

  UINT iFileType = 1;
  int const rgSize = 2; // Must match number of entries in rgSpec.
  COMDLG_FILTERSPEC const rgSpec[rgSize] =
    {
      { gMsgSaveToX3DExtensionDescription, L"*.x3d;*.x3dv" }, // 1
      { gMsgSaveToVRMLExtensionDescription, L"*.wrl;*.vrml" } // 2
    };
  int const extSize = 4; // Must match number of entries in extNames.
  wchar_t const * const extNames[extSize] =
    {
      L".x3d", L".x3dv", // 1
      L".wrl", L".vrml" // 2
    };
  int const type_to_idx[rgSize + 1] = {-1, 0, 2}; // There are rgSize + 1 types, 0 maps to -1.

  std::wstring filename(L"");

  // Open file dialog box and let user pick the filename.
  hr = FileSaveDialog(
                      filename,
                      gMsgSaveVTKScene,
                      rgSize,
                      rgSpec,
                      extSize,
                      extNames,
                      type_to_idx,
                      &iFileType
                      );
  assert( SUCCEEDED(hr) || (0x800704C7 == hr) );
  if ( !SUCCEEDED(hr) ) return hr;

  // Try to open file.
  FILE * fid = NULL;
  {
    errno_t const open = _wfopen_s(&fid, filename.c_str(), L"wb");
    assert( (0 == open) && (NULL != fid) );
    if ( (0 != open) || (NULL == fid) )
      {
        hr = E_FAIL;
        goto VTKSaveRenderWindowToFile_Exit;
      }
    /* if */

    int const closed = fclose(fid);
    assert(0 == closed);
    if (0 == closed) fid = NULL;
  }

  // Convert wide string to ANSI string.
  int const buffer_sz = 4 * (int)filename.size();
  char * buffer = new char[buffer_sz + 1];
  assert(NULL != buffer_sz);
  if (NULL == buffer_sz) return E_OUTOFMEMORY;

  int const numch = WideCharToMultiByte(
                                        CP_ACP, // Transcode for ANSI fopen/fclose function.
                                        0,
                                        filename.c_str(), // Input string.
                                        -1,
                                        buffer, // Output string.
                                        buffer_sz,
                                        NULL,
                                        NULL
                                        );
  assert( (0 < numch) && (numch < buffer_sz) );

  if ( (0 >= numch) || (buffer_sz <= numch) )
    {
      DWORD const error_code = GetLastError();
      assert(ERROR_SUCCESS == error_code);
      hr = E_FAIL;
      goto VTKSaveRenderWindowToFile_Exit;
    }
  /* if */

  // Save using VTK exporter class.
  switch ( iFileType )
    {
    case 1: // X3D
      {
        vtkSmartPointer<vtkX3DExporter> exporter = vtkSmartPointer<vtkX3DExporter>::New();
        exporter->SetFileName( buffer );
        exporter->SetRenderWindow( renderWindow );
        exporter->Write();
      }
      break;

    case 2: // VRML
    default:
      {
        vtkSmartPointer<vtkVRMLExporter> exporter = vtkSmartPointer<vtkVRMLExporter>::New();
        exporter->SetFileName( buffer );
        exporter->SetRenderWindow( renderWindow );
        exporter->Write();
      }
      break;
    }
  /* switch */

 VTKSaveRenderWindowToFile_Exit:

  SAFE_DELETE_ARRAY( buffer );

  return hr;
}
/* VTKSaveRenderWindowToFile */



//! Invalidates window region.
/*!
  Function invalidates window region thus forcing redraw.

  \param hwnd   Window handle.
  \param lParam Unused.
  \return Function returns value from InvalidatesRect.
*/
BOOL
CALLBACK
VTKUpdateDisplayHelper(
                       HWND hwnd,
                       LPARAM lParam
                       )
{
  assert(0 == lParam);

  BOOL const inv = InvalidateRect(hwnd, NULL, FALSE);
  assert(0 != inv);

  return( inv );
}
/* VTKUpdateDisplayHelper */



//! Disable window close command.
/*!
  Function disables window close command.

  \param hwnd   Window handle.
  \param lParam Unused.
  \return Function returns result of .
*/
BOOL
CALLBACK
VTKDisableCloseCommandHelper(
                             HWND hwnd,
                             LPARAM lParam
                             )
{
  assert(0 == lParam);

  BOOL result = TRUE;
  ULONG_PTR style = 0;

  if (TRUE == result)
    {
      style = GetClassLongPtr(hwnd, GCL_STYLE);
      assert(0 != style);
      if (0 == style) result = FALSE;
    }
  /* if */

  if (TRUE == result)
    {
      ULONG_PTR old_style = SetClassLongPtr(hwnd, GCL_STYLE, style | CS_NOCLOSE);
      //assert(0 != old_style);
      //if (0 == old_style) result = FALSE;
    }
  /* if */

  return( result );
}
/* VTKDisableCloseCommandHelper */



//! Update camera to new geometry.
/*!
  Function updates the stored camera data to match new viewing geometry.

  \param camera      VTK camera class.
  \param geometry    Pointer to pinhole camera geometry class.
  \param md     Pointer to three element array holding the median of point cloud. If NULL then (0,0,0) is assumed.
  \param bounds     Pointer to six element array holding the data bounds. If NULL then clipping planes are not updated.
  \param parallel       Flag indicating projection type, false for perspective and true for parallel.
  \return Function returns true if successfull.
*/
bool
VTKSetCameraToMatchGeometry(
                            vtkCamera * const camera,
                            ProjectiveGeometry * const geometry,
                            double const * const md,
                            double const * const bounds,
                            bool const parallel
                            )
{
  assert(NULL != camera);
  if (NULL == camera) return false;

  assert(NULL != geometry);
  if (NULL == geometry) return false;

  bool result = true;

  // Camera position.
  double const cx = geometry->center[0];
  double const cy = geometry->center[1];
  double const cz = geometry->center[2];

  // View up vector (note the coordinate system difference).
  double const vx = -geometry->rotation[1][0];
  double const vy = -geometry->rotation[1][1];
  double const vz = -geometry->rotation[1][2];

  // Look-at vector.
  double const lx = geometry->rotation[2][0];
  double const ly = geometry->rotation[2][1];
  double const lz = geometry->rotation[2][2];

  /* Focal point. Focal point will be set as close as possible to the
     origin of the world coordinate system. This is necessary to as used trackball actor
     rotates the view around the camera focus.
  */
  double_a_V3 ln1 = {cx, cy, cz};
  double_a_V3 ln2 = {cx + lx, cy + ly, cz + lz};
  double_a_V3 pt = {0.0, 0.0, 0.0};
  if (NULL != md) pt[0] = md[0], pt[1] = md[1], pt[2] = md[2];
  double_a_V3 focus = {0.0, 0.0, 0.0};
  bool const res = ClosestPointOnLineFromPoint(ln1, ln2, pt, focus);
  double const fx = (true == res)? focus[0] : ln2[0];
  double const fy = (true == res)? focus[1] : ln2[1];
  double const fz = (true == res)? focus[2] : ln2[2];

  // Viewing angle and scale.
  double const pi = 3.141592653589793238462643383279502884197169399375;
  double const ang_rad = geometry->GetViewAngle();
  double const ang_deg = ang_rad * 180.0 / pi;
  double const scale = geometry->GetScale();

  // Update camera data.
  camera->SetPosition(cx, cy, cz);
  camera->SetFocalPoint(fx, fy, fz);
  camera->SetViewUp(vx, vy, vz);
  if (false == isnan_inline(ang_deg)) camera->SetViewAngle(ang_deg);
  if (false == isnan_inline(scale)) camera->SetParallelScale(scale);

  if (NULL != bounds)
    {
      bool const clip = VTKSetCameraClippingPlanes(camera, bounds);
      assert(true == clip);
    }
  /* if */

  if (true == parallel)
    {
      camera->ParallelProjectionOn();
    }
  else
    {
      camera->ParallelProjectionOff();
    }
  /* if */

  return result;
}
/* VTKSetCameraToMatchGeometry */



//! Update camera to new orthographic projection.
/*!
  Function updates the stored camera to match new orthographic projection.

  \param camera  Pointer to VTK camera class.
  \param look_at  Camera look-at vector. If NULL then z-axis is assumed to be look-at vector.
  \param look_up  Camera look-up vector. If NULL then x-axis is assumed to be look-up vector.
  \param md     Pointer to three element array holding the camera look-at point. If NULL then (0,0,0) is assumed.
  \param bounds Pointer to six element array holding the data bounds.
  If NULL then camera may be positioned so some part of data is not visible.
  \param scale Scale of parallel projection.
  \return Function returns true if successfull.
*/
bool
VTKSetOrthographicProjectionCamera(
                                   vtkCamera * const camera,
                                   double const * look_at,
                                   double const * look_up,
                                   double const * md,
                                   double const * bounds,
                                   double const scale
                                   )
{
  assert(NULL != camera);
  if (NULL == camera) return false;

  double const look_at_default[3] = {0.0, 0.0, 1.0};
  double const look_up_default[3] = {0.0, -1.0, 0.0};
  double const md_default[3] = {0.0, 0.0, 0.0};
  double const bounds_default[6] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

  if (NULL == look_at) look_at = look_at_default;
  if (NULL == look_up) look_up = look_up_default;
  if (NULL == md) md = md_default;
  if (NULL == bounds) bounds = bounds_default;

  // Place camera above object center-of-mass.
  double_a_V3 focus = {md[0], md[1], md[2]};
  double_a_V3 position = {md[0] - look_at[0], md[1] - look_at[1], md[2] - look_at[2]};

  // If bounds are given then move camera outside of the bounding box.
  double D = BATCHACQUISITION_pINF_dv;
  if (NULL != bounds)
    {
      // Object bounds.
      double const xmin = bounds[0];
      double const xmax = bounds[1];
      double const ymin = bounds[2];
      double const ymax = bounds[3];
      double const zmin = bounds[4];
      double const zmax = bounds[5];

      // Find critical point which will be closest to the camera.
      double_a_V3 pt = {xmin, ymin, zmin};

      double const D1 = xmin * look_at[0] + ymin * look_at[1] + zmin * look_at[2];
      if (D1 < D) D = D1, pt[0] = xmin, pt[1] = ymin, pt[2] = zmin;

      double const D2 = xmin * look_at[0] + ymin * look_at[1] + zmax * look_at[2];
      if (D2 < D) D = D2, pt[0] = xmin, pt[1] = ymin, pt[2] = zmax;

      double const D3 = xmin * look_at[0] + ymax * look_at[1] + zmin * look_at[2];
      if (D3 < D) D = D3, pt[0] = xmin, pt[1] = ymax, pt[2] = zmin;

      double const D4 = xmin * look_at[0] + ymax * look_at[1] + zmax * look_at[2];
      if (D4 < D) D = D4, pt[0] = xmin, pt[1] = ymax, pt[2] = zmax;

      double const D5 = xmax * look_at[0] + ymin * look_at[1] + zmin * look_at[2];
      if (D5 < D) D = D5, pt[0] = xmax, pt[1] = ymin, pt[2] = zmin;

      double const D6 = xmax * look_at[0] + ymin * look_at[1] + zmax * look_at[2];
      if (D6 < D) D = D6, pt[0] = xmax, pt[1] = ymin, pt[2] = zmax;

      double const D7 = xmax * look_at[0] + ymax * look_at[1] + zmin * look_at[2];
      if (D7 < D) D = D7, pt[0] = xmax, pt[1] = ymax, pt[2] = zmin;

      double const D8 = xmax * look_at[0] + ymax * look_at[1] + zmax * look_at[2];
      if (D8 < D) D = D8, pt[0] = xmax, pt[1] = ymax, pt[2] = zmax;

      // Move critical point outside of the bounding box.
      double const c = (xmax - xmin) + (ymax - ymin) + (zmax - zmin);
      pt[0] += c * look_at[0];
      pt[1] += c * look_at[1];
      pt[2] += c * look_at[2];

      double_a_V3 ln1 = {md[0], md[1], md[2]};
      double_a_V3 ln2 = {md[0] - look_at[0], md[1] - look_at[1], md[2] - look_at[2]};

      bool const res = ClosestPointOnLineFromPoint(ln1, ln2, pt, position);
      assert(true == res);
    }
  /* if */

  // Configure camera.
  camera->ParallelProjectionOn();
  camera->SetPosition(position[0], position[1], position[2]);
  camera->SetFocalPoint(focus[0], focus[1], focus[2]);
  camera->SetViewUp(-look_up[0], -look_up[1], -look_up[2]);
  if (false == isnan_inline(scale)) camera->SetParallelScale(scale);

  // Fix camera clipping planes.
  if (NULL != bounds)
    {
      bool const clip = VTKSetCameraClippingPlanes(camera, bounds);
      assert(true == clip);
    }
  /* if */

  return true;
}
/* VTKSetOrthographicProjectionCamera */



//! Update camera clipping planes.
/*!
  Updates camera clipping planes using the object bounding box.

  \param camera VTK camera class.
  \param bounds Pointer to six element array holding the data bounds.
  \return Returns true if successfull.
*/
bool
VTKSetCameraClippingPlanes(
                           vtkCamera * const camera,
                           double const * const bounds
                           )
{
  assert(NULL != camera);
  if (NULL == camera) return false;

  assert(NULL != bounds);
  if (NULL == bounds) return false;

  bool result = true;

  // Object bounds.
  double const xmin = bounds[0];
  double const xmax = bounds[1];
  double const ymin = bounds[2];
  double const ymax = bounds[3];
  double const zmin = bounds[4];
  double const zmax = bounds[5];

  // VTK camera default clipping range.
  double cmin = 1.0;
  double cmax = 1000.0;

  double_a_V3 ln1;
  camera->GetPosition(ln1);

  double_a_V3 ln2;
  camera->GetFocalPoint(ln2);

  double const dstx = ln2[0] - ln1[0];
  double const dsty = ln2[1] - ln1[1];
  double const dstz = ln2[2] - ln1[2];
  double const dst = sqrt( dstx * dstx + dsty * dsty + dstz * dstz );
  double const scale = 1.0 / dst;

  ln2[0] = ln1[0] + dstx * scale;
  ln2[1] = ln1[1] + dsty * scale;
  ln2[2] = ln1[2] + dstz * scale;

  double_a_V3 pt = {xmin, ymin, zmin};

  double c = 0.0;

  bool const r1 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r1) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[2] = zmax;
  bool const r2 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r2) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[1] = ymax, pt[2] = zmin;
  bool const r3 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r3) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[2] = zmax;
  bool const r4 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r4) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[0] = xmax, pt[1] = ymin, pt[2] = zmin;
  bool const r5 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r5) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[2] = zmax;
  bool const r6 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r6) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[1] = ymax, pt[2] = zmin;
  bool const r7 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r7) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  pt[2] = zmax;
  bool const r8 = DistanceAlongLineFromPoint(ln1, ln2, pt, &c);
  if (true == r8) { if (cmin > c) cmin = c; if (cmax < c) cmax = c; };

  assert(cmin <= cmax);

  if (cmin > 0.1 + FLT_EPSILON) cmin -= 0.1;
  cmax += 10.0;
  if (cmin <= 0) cmin = 1.0;
  if (cmax <= 0) cmax = 1000.0;

  camera->SetClippingRange(cmin, cmax);

  return result;

}
/* VTKSetCameraClippingPlanes */



//! Set camera focal point.
/*!
  Moves camera focal point as close as possible to the center of mass of the point cloud.

  \param camera VTK camera class.
  \param md     Median of point cloud (three elements).
  \return Function returns true if successfull.
*/
bool
VTKSetCameraFocalPoint(
                       vtkCamera * const camera,
                       double const * const md
                       )
{
  assert(NULL != camera);
  if (NULL == camera) return false;

  assert(NULL != md);
  if (NULL == md) return false;

  double_a_V3 ln1;
  camera->GetPosition(ln1);

  double_a_V3 ln2;
  camera->GetFocalPoint(ln2);

  double_a_V3 pt = {md[0], md[1], md[2]};
  double_a_V3 focus = {0.0, 0.0, 0.0};

  bool const result = ClosestPointOnLineFromPoint(ln1, ln2, pt, focus);
  assert(true == result);

  if (true == result) camera->SetFocalPoint(focus[0], focus[1], focus[2]);

  return result;
}
/* VTKSetCameraFocalPoint */



//! Destroy surface data.
/*!
  Function destroys complete VTK pipeline for the surface data. Ensure that
  the actor is removed from renderer prior to destroying the pipeline.

  \param P      Pointer to surface data storage structure.
*/
void
VTKDeleteSurfaceData(
                     VTKsurfacedata * const P
                     )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SAFE_VTK_DELETE( P->surfaceActor );
  SAFE_VTK_DELETE( P->surfaceMapper );
  SAFE_VTK_DELETE( P->surfaceFilter );
  SAFE_VTK_DELETE( P->surfaceExtractor );

  VTKBlankSurfaceData_inline( P );

  free( P );
}
/* VTKDeleteSurfaceData */



//! Create surface model from the point cloud.
/*!
  Uses VTK surface reconstruction filter to obtain a surface from the point cloud.
  The data data supplied as input should not be erased/released/altered before
  the returned actor is destroyed.

  The obtained surface is colored in gray and is semitransparent.

  \param points A class containing points.
  \param CameraID ID of the camera used to acquire the point cloud.
  \param ProjectorID ID of the projector used to acquire the point cloud.
  \return Returns pointer to storage structure or NULL if unsuccessfull.
*/
VTKsurfacedata *
VTKCreateSurfaceData(
                     vtkPolyData * const points,
                     int const CameraID,
                     int const ProjectorID
                     )
{
  assert(NULL != points);
  if (NULL == points) return NULL;

  /* Allocate storage. */
  VTKsurfacedata * const P = (VTKsurfacedata *)malloc( sizeof(VTKsurfacedata) );
  assert(NULL != P);
  if (NULL == P) return NULL ;

  /* Start timer. */
  DEBUG_TIMER * const debug_timer = DebugTimerInit();
  Debugfprintf(stderr, gMsgSurfaceStart, CameraID + 1, ProjectorID + 1);


  VTKBlankSurfaceData_inline( P );

  /* Create all required objects. */
  P->surfaceExtractor = vtkSurfaceReconstructionFilter::New();
  assert(NULL != P->surfaceExtractor);

  P->surfaceFilter = vtkContourFilter::New();
  assert(NULL != P->surfaceFilter);

  P->surfaceMapper = vtkPolyDataMapper::New();
  assert(NULL != P->surfaceMapper);

  P->surfaceActor = vtkActor::New();
  assert(NULL != P->surfaceActor);

  if ( (NULL == P->surfaceExtractor) ||
       (NULL == P->surfaceFilter) ||
       (NULL == P->surfaceMapper) ||
       (NULL == P->surfaceActor)
       )
    {
      VTKDeleteSurfaceData( P );
      DebugTimerDestroy( debug_timer );
      return NULL;
    }
  /* if */

  /* Create surface extraction pipeline. */
  P->surfaceExtractor->SetInputData( points );
  //P->surfaceExtractor->SetSampleSpacing( 0.5 );
  P->surfaceExtractor->SetNeighborhoodSize( 8 );

  P->surfaceFilter->SetInputConnection( P->surfaceExtractor->GetOutputPort() );
  P->surfaceFilter->SetValue(0, 0.0);

  P->surfaceMapper->SetInputConnection( P->surfaceFilter->GetOutputPort() );
  P->surfaceMapper->ScalarVisibilityOff();

  P->surfaceActor->SetMapper( P->surfaceMapper );
  P->surfaceActor->GetProperty()->SetDiffuseColor(0.7f, 0.7f, 0.7f);
  P->surfaceActor->GetProperty()->SetSpecularColor(1.0f, 1.0f, 1.0f);
  P->surfaceActor->GetProperty()->SetSpecular(.4);
  P->surfaceActor->GetProperty()->SetSpecularPower(50);

  /* Execute pipline and extract the surface. */
  P->surfaceFilter->Update();

  /* Stop timer. */
  double const SURFtime = DebugTimerQueryStart( debug_timer );
  Debugfprintf(stderr, gMsgSurfaceComplete, CameraID + 1, ProjectorID + 1, SURFtime);
  DebugTimerDestroy( debug_timer );

  /* Return created object. */
  return( P );
}
/* VTKCreateSurfaceData */



//! Destroys outline data.
/*!
  Function destroys complete VTK pipeline for the outline data. Ensure that
  the actor is removed from renderer prior to destroying the pipeline.

  \param P      Pointer to surface data storage structure.
*/
void
VTKDeleteOutlineData(
                     VTKoutlinedata * const P
                     )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SAFE_VTK_DELETE( P->outlineExtractor );
  SAFE_VTK_DELETE( P->outlineMapper );
  SAFE_VTK_DELETE( P->outlineActor );

  VTKBlankOutlineData_inline( P );

  free( P );
}
/* VTKDeleteOutlineData */



//! Creates outline wireframe for point cloud.
/*!
  Uses VTK outline filter to obtain a bounding box for the point cloud
  (actually we obtain a wireframe model of the data bounding box).

  Obtained wireframe outline is colored in blue.

  \param points A class containing points.
  \param CameraID ID of the camera used to acquire the point cloud.
  \param ProjectorID ID of the projector used to acquire the point cloud.
  \return Returns pointer to storage structure or NULL if unsuccessfull.
*/
VTKoutlinedata *
VTKCreateOutlineData(
                     vtkPolyData * const points,
                     int const CameraID,
                     int const ProjectorID
                     )
{
  assert(NULL != points);
  if (NULL == points) return NULL;

  /* Allocate storage. */
  VTKoutlinedata * const P = (VTKoutlinedata *)malloc( sizeof(VTKoutlinedata) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  /* Start timer. */
  DEBUG_TIMER * const debug_timer = DebugTimerInit();
  Debugfprintf(stderr, gMsgOutlineStart, CameraID + 1, ProjectorID + 1);


  VTKBlankOutlineData_inline( P );

  /* Create all required objects. */
  P->outlineExtractor = vtkOutlineFilter::New();
  assert(NULL != P->outlineExtractor);

  P->outlineMapper = vtkPolyDataMapper::New();
  assert(NULL != P->outlineMapper);

  P->outlineActor = vtkActor::New();
  assert(NULL != P->outlineActor);

  if ( (NULL == P->outlineExtractor) ||
       (NULL == P->outlineMapper) ||
       (NULL == P->outlineActor)
       )
    {
      VTKDeleteOutlineData( P );
      DebugTimerDestroy( debug_timer );
      return NULL;
    }
  /* if */

  /* Create outilne extraction pipeline. */
  P->outlineExtractor->SetInputData( points );

  P->outlineMapper->SetInputConnection( P->outlineExtractor->GetOutputPort() );

  P->outlineActor->SetMapper( P->outlineMapper );
  P->outlineActor->GetProperty()->SetColor(0.1f, 0.1f, 0.9f); // Dark blue!
  P->outlineActor->GetProperty()->SetLineWidth(1.0f);
  P->outlineActor->GetProperty()->SetOpacity(0.5f);

  /* Extract surface. */
  P->outlineExtractor->Update();

  /* Stop timer. */
  double const BBOXtime = DebugTimerQueryStart( debug_timer );
  Debugfprintf(stderr, gMsgOutlineComplete, CameraID + 1, ProjectorID + 1, BBOXtime);
  DebugTimerDestroy( debug_timer );

  /* Return created object. */
  return( P );
}
/* VTKCreateOutlineData */



//! Destroy point cloud.
/*!
  Function destroys complete VTK pipeline for the point cloud data. Ensure that
  the actor is removed from renderer prior to destroying the pipeline.

  \param P      Pointer to point data data storage structure.
*/
void
VTKDeletePointCloudData(
                        VTKpointclouddata * const P
                        )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SAFE_VTK_DELETE( P->Actor );
  SAFE_VTK_DELETE( P->Mapper );

  SAFE_VTK_DELETE( P->CloudVertexes );
  SAFE_VTK_DELETE( P->PointsToVertexes );
  SAFE_VTK_DELETE( P->CloudPoints );

  SAFE_VTK_DELETE( P->Cloud );

  SAFE_VTK_DELETE( P->ColorsMapped );
  SAFE_VTK_DELETE( P->ColorsOriginal );

  SAFE_DELETE( P->acquisition_name );

  SAFE_DELETE( P->pDynamicRange );
  SAFE_DELETE( P->pRayDistance );
  SAFE_DELETE( P->pPhaseDistance );
  SAFE_DELETE( P->pPhaseDeviation );

  SAFE_DELETE( P->pMask );

  VTKDeleteSurfaceData(P->surface);
  VTKDeleteOutlineData(P->outline);

  VTKBlankPointCloudData_inline( P );

  free( P );
}
/* VTKDeletePointCloudData */



//! Create point cloud VTK actor for the point data.
/*!
  Creates VTK point set and associated VTK actor that represents the point cloud.

  Default obtained points are colored in blue.

  \param points Pointer to cv::Mat matrix having 3 columns and at least one row that contains point coordinates.
  Type of matrix must be CV_32F or CV_64F.
  \param colors Pointer to cv::Mat matrix having 1 or 3 columns and the same number of rows as points matrix.
  If matrix has one column then value is interpreted as graylevel intensity, if matrix has three columns
  then columns contain red, green, and blue color components. Type of the matrix must be CV_8U.
  \param data Pointer to cv::Mat matrix having several columns which contain additional data.
  The matrix data must have the same number of rows as points matrix.
  The first column contains dynamic range of the input, the second column contains minimal
  distance between triangulation rays, etc.
  \param CameraID ID of the camera used to acquire the point cloud.
  \param ProjectorID ID of the projector used to acquire the point cloud.
  \param name   Name of the current acquisition. May be NULL.
  \return Returns pointer to storage structure or NULL if unsuccessfull.
*/
VTKpointclouddata *
VTKCreatePointCloudData(
                        cv::Mat * const points,
                        cv::Mat * const colors,
                        cv::Mat * const data,
                        int const CameraID,
                        int const ProjectorID,
                        wchar_t const * const name
                        )
{
  //assert(NULL != points);
  if (NULL == points) return NULL;

  //assert(NULL != points->data);
  assert(3 == points->cols);
  //assert(0 < points->rows);
  if ( (NULL == points->data) || (3 != points->cols) || (0 >= points->rows) ) return NULL;

  /* Allocate storage. */
  VTKpointclouddata * const P = (VTKpointclouddata *)malloc( sizeof(VTKpointclouddata) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  VTKBlankPointCloudData_inline( P );

  /* Create all required objects. */
  P->pDynamicRange = new std::vector<float>;
  assert(NULL != P->pDynamicRange);

  P->pRayDistance = new std::vector<float>;
  assert(NULL != P->pRayDistance);

  P->pPhaseDistance = new std::vector<float>;
  assert(NULL != P->pPhaseDistance);

  P->pPhaseDeviation = new std::vector<float>;
  assert(NULL != P->pPhaseDeviation);

  P->pMask = new std::vector<unsigned char>;
  assert(NULL != P->pMask);

  P->Cloud = vtkPoints::New();
  assert(NULL != P->Cloud);

  P->ColorsMapped = vtkUnsignedCharArray::New();
  assert(NULL != P->ColorsMapped);

  P->ColorsOriginal = vtkUnsignedCharArray::New();
  assert(NULL != P->ColorsOriginal);

  P->CloudPoints = vtkPolyData::New();
  assert(NULL != P->CloudPoints);

  P->PointsToVertexes = vtkVertexGlyphFilter::New();
  assert(NULL != P->PointsToVertexes);

  P->CloudVertexes = vtkPolyData::New();
  assert(NULL != P->CloudVertexes);

  P->Mapper = vtkPolyDataMapper::New();
  assert(NULL != P->Mapper);

  P->Actor = vtkActor::New();
  assert(NULL != P->Actor);

  if ( (NULL == P->pDynamicRange) ||
       (NULL == P->pRayDistance) ||
       (NULL == P->pPhaseDistance) ||
       (NULL == P->pPhaseDeviation) ||
       (NULL == P->pMask) ||
       (NULL == P->Cloud) ||
       (NULL == P->ColorsMapped) ||
       (NULL == P->ColorsOriginal) ||
       (NULL == P->CloudPoints) ||
       (NULL == P->PointsToVertexes) ||
       (NULL == P->CloudVertexes) ||
       (NULL == P->Mapper) ||
       (NULL == P->Actor)
       )
    {
      VTKDeletePointCloudData( P );
      return NULL;
    }
  /* if */

  /* Get data size and type. */
  int const N = (int)(points->rows);
  assert(0 <= N);

  int const points_type = points->type();
  int const points_depth = CV_MAT_DEPTH(points_type);
  assert( 1 == CV_MAT_CN(points_type) );

  /* Copy supplied coordinates. We may set the data using method InsertPoint or SetPoint.
     Note that InsertPoint does boundary checking on each insert while the SetPoint does not.
     Therefore, memory pre-allocation when using SetNumberOfPoints is necessary.
     However, once memory is pre-allocated we may fetch the data pointer directly and avoid
     the overhead of tuple copy loop in SetPoint method.
  */
  P->Cloud->SetNumberOfPoints(N); // Pre-allocate storage.

  int const type = P->Cloud->GetDataType();
  assert(VTK_FLOAT == type);
  if (VTK_FLOAT != type)
    {
      VTKDeletePointCloudData( P );
      return NULL;
    }
  /* if */

  float * const dst_pt = (float *)( P->Cloud->GetVoidPointer(0) ); // Fetch data pointer.

  switch (points_depth)
    {
    case CV_32F:
      {
        for (int i = 0; i < N; ++i)
          {
            float const * const rowptr = (float *)( (BYTE *)( points->data ) + i * points->step[0] );
            int const adr = 3 * i;
            dst_pt[adr    ] = rowptr[0];
            dst_pt[adr + 1] = rowptr[1];
            dst_pt[adr + 2] = rowptr[2];
          }
        /* for */
      }
      break;

    case CV_64F:
      {
        for (int i = 0; i < N; ++i)
          {
            double const * const rowptr = (double *)( (BYTE *)( points->data ) + i * points->step[0] );
            int const adr = 3 * i;
            dst_pt[adr    ] = (float)( rowptr[0] );
            dst_pt[adr + 1] = (float)( rowptr[1] );
            dst_pt[adr + 2] = (float)( rowptr[2] );
          }
        /* for */
      }
      break;

    default:
      {
        // Unsupported input.
        VTKDeletePointCloudData( P );
        return NULL;
      }
    }
  /* switch */

  /* Copy supplied colors. Colors are in RGBA mode where last value is opacity.
     Opacity of 0 means the vertex associated with the point will be completely
     transparent and therefore invisible while opacity of 255 means the vertex
     in solid. Again, we use InsertTupleValue which does no boundary checking
     so memory must be preallocated.
  */
  unsigned char default_color[4] = {240, 240, 240, 255};

  assert( 1.0f == P->colorScale );
  assert( 0.0f == P->colorOffset );

  P->ColorsOriginal->SetNumberOfComponents(4);
  P->ColorsOriginal->SetNumberOfTuples(N);
  P->ColorsMapped->SetNumberOfComponents(4);
  P->ColorsMapped->SetNumberOfTuples(N);
  if ( (NULL != colors) && (NULL != colors->data) )
    {
      int const colors_type = colors->type();
      int const colors_depth = CV_MAT_DEPTH(colors_type);
      assert( 1 == CV_MAT_CN(colors_type) );

      if ( (N == colors->rows) && (3 == colors->cols) && (CV_8U == colors_depth) )
        {
          // Every point has different color.
          unsigned char clr[4] = {0, 0, 0, 255};
          for (int i = 0; i < N; ++i)
            {
              unsigned char const * const rowptr = (unsigned char *)( (BYTE *)( colors->data ) + i * colors->step[0] );
              clr[0] = rowptr[0];
              clr[1] = rowptr[1];
              clr[2] = rowptr[2];
              P->ColorsOriginal->SetTupleValue(i, clr);
              P->ColorsMapped->SetTupleValue(i, clr);
            }
          /* for */
        }
      else if ( (N == colors->rows) && (1 == colors->cols) && (CV_8U == colors_depth) )
        {
          // Every point has different graylevel.
          unsigned char clr[4] = {0, 0, 0, 255};
          for (int i = 0; i < N; ++i)
            {
              unsigned char const * const rowptr = (unsigned char *)( (BYTE *)( colors->data ) + i * colors->step[0] );
              clr[0] = rowptr[0];
              clr[1] = rowptr[0];
              clr[2] = rowptr[0];
              P->ColorsOriginal->SetTupleValue(i, clr);
              P->ColorsMapped->SetTupleValue(i, clr);
            }
          /* for */
        }
      else if ( (1 == colors->rows) && (3 == colors->cols) && (CV_8U == colors_depth) )
        {
          // One color for all points.
          unsigned char clr[4] = {
            ( (BYTE *)colors->data )[0],
            ( (BYTE *)colors->data )[1],
            ( (BYTE *)colors->data )[2],
            255
          };
          for (int i = 0; i < N; ++i)
            {
              P->ColorsOriginal->SetTupleValue(i, clr);
              P->ColorsMapped->SetTupleValue(i, clr);
            }
          /* for */
        }
      else if ( (1 == colors->rows) && (1 == colors->cols) && (CV_8U == colors_depth) )
        {
          // One graylevel for all points.
          unsigned char clr[4] = {
            ( (BYTE *)colors->data )[0],
            ( (BYTE *)colors->data )[0],
            ( (BYTE *)colors->data )[0],
            255
          };
          for (int i = 0; i < N; ++i)
            {
              P->ColorsOriginal->SetTupleValue(i, clr);
              P->ColorsMapped->SetTupleValue(i, clr);
            }
          /* for */
        }
      else
        {
          // Unsupported input.
          for (int i = 0; i < N; ++i)
            {
              P->ColorsOriginal->SetTupleValue(i, default_color);
              P->ColorsMapped->SetTupleValue(i, default_color);
            }
          /* for */
        }
      /* if */
    }
  else
    {
      for (int i = 0; i < N; ++i)
        {
          P->ColorsOriginal->SetTupleValue(i, default_color);
          P->ColorsMapped->SetTupleValue(i, default_color);
        }
      /* for */
    }
  /* if */

  /* Copy supplied additional data. */
  P->pDynamicRange->clear();
  P->pRayDistance->clear();
  P->pPhaseDistance->clear();
  P->pPhaseDeviation->clear();
  if ( (NULL != data) && (NULL != data->data) )
    {
      int const data_type = data->type();
      int const data_depth = CV_MAT_DEPTH(data_type);
      assert( 1 == CV_MAT_CN(data_type) );

      P->rangeMin = BATCHACQUISITION_pINF_fv;
      P->rangeMax = BATCHACQUISITION_nINF_fv;

      if ( (N == data->rows) && (1 <= data->cols) && (CV_32F == data_depth) )
        {
          P->pDynamicRange->resize(N);
          assert( N == P->pDynamicRange->size() );
          for (int i = 0; i < N; ++i)
            {
              float const * const rowptr = (float *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const range = rowptr[0];
              ( *(P->pDynamicRange) ) [i] = range;
              if (range < P->rangeMin) P->rangeMin = range;
              if (range > P->rangeMax) P->rangeMax = range;
            }
          /* for */

        }
      else if ( (N == data->rows) && (1 <= data->cols) && (CV_64F == data_depth) )
        {
          P->pDynamicRange->resize(N);
          assert( N == P->pDynamicRange->size() );
          for (int i = 0; i < N; ++i)
            {
              double const * const rowptr = (double *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const range = (float)( rowptr[0] );
              ( *(P->pDynamicRange) ) [i] = range;
              if (range < P->rangeMin) P->rangeMin = range;
              if (range > P->rangeMax) P->rangeMax = range;
            }
          /* for */
        }
      /* if */

      P->rangeThr = P->rangeMin;

      P->rayDistanceMin = BATCHACQUISITION_pINF_fv;
      P->rayDistanceMax = BATCHACQUISITION_nINF_fv;

      if ( (N == data->rows) && (2 <= data->cols) && (CV_32F == data_depth) )
        {
          P->pRayDistance->resize(N);
          assert( N == P->pRayDistance->size() );
          for (int i = 0; i < N; ++i)
            {
              float const * const rowptr = (float *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const distance = rowptr[1];
              ( *(P->pRayDistance) ) [i] = distance;
              if (distance < P->rayDistanceMin) P->rayDistanceMin = distance;
              if (distance > P->rayDistanceMax) P->rayDistanceMax = distance;
            }
          /* for */

        }
      else if ( (N == data->rows) && (2 <= data->cols) && (CV_64F == data_depth) )
        {
          P->pRayDistance->resize(N);
          assert( N == P->pRayDistance->size() );
          for (int i = 0; i < N; ++i)
            {
              double const * const rowptr = (double *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const distance = (float)( rowptr[2] );
              ( *(P->pRayDistance) ) [i] = distance;
              if (distance < P->rayDistanceMin) P->rayDistanceMin = distance;
              if (distance > P->rayDistanceMax) P->rayDistanceMax = distance;
            }
          /* for */
        }
      /* if */

      P->rayDistanceThr = P->rayDistanceMax;

      P->phaseDistanceMin = BATCHACQUISITION_pINF_fv;
      P->phaseDistanceMax = BATCHACQUISITION_nINF_fv;

      if ( (N == data->rows) && (3 <= data->cols) && (CV_32F == data_depth) )
        {
          P->pPhaseDistance->resize(N);
          assert( N == P->pPhaseDistance->size() );
          for (int i = 0; i < N; ++i)
            {
              float const * const rowptr = (float *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const distance = rowptr[2];
              ( *(P->pPhaseDistance) ) [i] = distance;
              if (distance < P->phaseDistanceMin) P->phaseDistanceMin = distance;
              if (distance > P->phaseDistanceMax) P->phaseDistanceMax = distance;
            }
          /* for */

        }
      else if ( (N == data->rows) && (3 <= data->cols) && (CV_64F == data_depth) )
        {
          P->pPhaseDistance->resize(N);
          assert( N == P->pPhaseDistance->size() );
          for (int i = 0; i < N; ++i)
            {
              double const * const rowptr = (double *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const distance = (float)( rowptr[2] );
              ( *(P->pPhaseDistance) ) [i] = distance;
              if (distance < P->phaseDistanceMin) P->phaseDistanceMin = distance;
              if (distance > P->phaseDistanceMax) P->phaseDistanceMax = distance;
            }
          /* for */
        }
      /* if */

      P->phaseDistanceThr = P->phaseDistanceMax;

      P->phaseDeviationMin = BATCHACQUISITION_pINF_fv;
      P->phaseDeviationMax = BATCHACQUISITION_nINF_fv;

      if ( (N == data->rows) && (4 <= data->cols) && (CV_32F == data_depth) )
        {
          P->pPhaseDeviation->resize(N);
          assert( N == P->pPhaseDeviation->size() );
          for (int i = 0; i < N; ++i)
            {
              float const * const rowptr = (float *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const deviation = rowptr[3];
              ( *(P->pPhaseDeviation) ) [i] = deviation;
              if (deviation < P->phaseDeviationMin) P->phaseDeviationMin = deviation;
              if (deviation > P->phaseDeviationMax) P->phaseDeviationMax = deviation;
            }
          /* for */

        }
      else if ( (N == data->rows) && (4 <= data->cols) && (CV_64F == data_depth) )
        {
          P->pPhaseDeviation->resize(N);
          assert( N == P->pPhaseDeviation->size() );
          for (int i = 0; i < N; ++i)
            {
              double const * const rowptr = (double *)( (BYTE *)( data->data ) + i * data->step[0] );
              float const deviation = (float)( rowptr[3] );
              ( *(P->pPhaseDeviation) ) [i] = deviation;
              if (deviation < P->phaseDeviationMin) P->phaseDeviationMin = deviation;
              if (deviation > P->phaseDeviationMax) P->phaseDeviationMax = deviation;
            }
          /* for */
        }
      /* if */

      if (P->phaseDeviationMin + 0.125f < P->phaseDeviationMax)
        {
          P->phaseDeviationMax = P->phaseDeviationMin + 0.125f;
        }
      /* if */
      P->phaseDeviationThr = P->phaseDeviationMax;
    }
  /* if */
  assert( P->rangeMin <= P->rangeMax );

  /* Create point mask. */
  P->pMask->resize(N, (unsigned char)(0));

  /* Copy IDs. */
  P->ProjectorID = ProjectorID;
  P->CameraID = CameraID;

  /* Copy name. */
  if (NULL != name)
    {
      assert(NULL == P->acquisition_name);
      P->acquisition_name = new std::wstring(name);
      assert(NULL != P->acquisition_name);
    }
  /* if */

  /* Compute the center of mass and the median of the input point cloud. */
  cv::Mat cm, md;
  bool const get_md_cm = PointCloudWeiszfeld(points, &md, &cm, 0, 0);
  assert(true == get_md_cm);
  if (true == get_md_cm)
    {
      P->cmx = cm.at<double>(0,0);
      P->cmy = cm.at<double>(0,1);
      P->cmz = cm.at<double>(0,2);

      P->mdx = md.at<double>(0,0);
      P->mdy = md.at<double>(0,1);
      P->mdz = md.at<double>(0,2);
    }
  /* if */

  /* Get data bounds. */
  P->Cloud->ComputeBounds();
  P->CloudPoints->SetPoints( P->Cloud );

  /* Extract outline. */
  P->outline = VTKCreateOutlineData( P->CloudPoints, ProjectorID, CameraID );
  assert(NULL != P->outline);

  if (NULL == P->outline)
    {
      VTKDeletePointCloudData( P );
      return NULL;
    }
  /* if */

  /* Create VTK visualization pipeline. */
  P->PointsToVertexes->SetInputData(P->CloudPoints);
  P->PointsToVertexes->Update();

  P->CloudVertexes->ShallowCopy(P->PointsToVertexes->GetOutput());
  P->ColorsMapped->SetName("Colors");
  P->CloudVertexes->GetPointData()->SetScalars(P->ColorsMapped);

  P->Mapper->SetInputData(P->CloudVertexes);
  P->Mapper->ScalarVisibilityOn();
  P->Mapper->SetScalarModeToUsePointFieldData();
  P->Mapper->SelectColorArray("Colors");

  P->Actor->SetMapper( P->Mapper );
  P->Actor->GetProperty()->SetPointSize( 3.0f );
  P->Actor->GetProperty()->SetOpacity( 1.0 );

  /* Return created object. */
  return( P );
}
/* VTKCreatePointCloudData */



//! Destroy slicing plane data.
/*!
  Function destroys slicing plane data.

  \param P      Pointer to slicing plane data.
*/
void
VTKDeleteSlicingPlaneData(
                          VTKslicingplane * const P
                          )
{
  assert(NULL != P);
  if (NULL == P) return;

  SAFE_VTK_DELETE( P->Actor );
  SAFE_VTK_DELETE( P->Mapper );
  SAFE_VTK_DELETE( P->Plane );
  SAFE_VTK_DELETE( P->Polygons );
  SAFE_VTK_DELETE( P->Polygon );
  SAFE_VTK_DELETE( P->Points );

  VTKBlankSlicingPlaneData_inline( P );

  free( P );
}
/* VTKDeleteSlicingPlaneData */



//! Creates slicing plane data.
/*!
  Function creates slicign plane data.

  \param nrm Plane normal.
  \param pt Plane point.
  \param bds Plane bounding box. May be NULL.
*/
VTKslicingplane *
VTKCreateSlicingPlaneData(
                          double const * const nrm,
                          double const * const pt,
                          double const * const bds
                          )
{
  assert( (NULL != nrm) && (NULL != pt) );
  if ( (NULL == nrm) || (NULL == pt) ) return NULL;

  /* Allocate memory. */
  VTKslicingplane * const P = (VTKslicingplane *)malloc( sizeof(VTKslicingplane) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  VTKBlankSlicingPlaneData_inline(P);

  /* Create all required elements. */
  P->Points = vtkPoints::New();
  assert(NULL != P->Points);

  P->Polygon = vtkPolygon::New();
  assert(NULL != P->Polygon);

  P->Polygons = vtkCellArray::New();
  assert(NULL != P->Polygons);

  P->Plane = vtkPolyData::New();
  assert(NULL != P->Plane);

  P->Mapper = vtkPolyDataMapper::New();
  assert(NULL != P->Mapper);

  P->Actor = vtkActor::New();
  assert(NULL != P->Actor);

  if ( (NULL == P->Points) ||
       (NULL == P->Polygon) ||
       (NULL == P->Polygons) ||
       (NULL == P->Plane) ||
       (NULL == P->Mapper) ||
       (NULL == P->Actor)
       )
    {
      VTKDeleteSlicingPlaneData(P);
      return NULL;
    }
  /* if */

  /* Update plane data. */
  VTKUpdateSlicingPlane_inline(P, nrm, pt, bds);

  P->Mapper->SetInputData( P->Plane );
  P->Actor->SetMapper(P->Mapper);
  P->Actor->GetProperty()->SetOpacity(0.0f);

  return P;
}
/* VTKCreateSlicingPlaneData */



//! Destroy view point data.
/*!
  Function destroys view point information.

  \param P      Pointer to view point storage structure.
*/
void
VTKDeleteViewPointData(
                       VTKviewpoint * const P
                       )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SAFE_VTK_DELETE( P->camera3D );
  SAFE_VTK_DELETE( P->cameraTop );
  SAFE_VTK_DELETE( P->cameraFront );
  SAFE_VTK_DELETE( P->cameraSide );

  SAFE_DELETE( P->geometry );

  VTKBlankViewPointData_inline( P );

  free( P );
}
/* VTKDeleteViewPointData */



//! Create view point data.
/*!
  Creates structure to hold camera or projector view point.

  \param geometry Pointer to pinhole camera (or projector) geometry.
  \return Function returns pointer to created structure if successfull.
*/
VTKviewpoint *
VTKCreateViewPointData(
                       ProjectiveGeometry * const geometry
                       )
{
  assert(NULL != geometry);
  if (NULL == geometry) return NULL;

  /* Allocate memory. */
  VTKviewpoint * const P = (VTKviewpoint *)malloc( sizeof(VTKviewpoint) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  VTKBlankViewPointData_inline( P );

  /* Create all required elements. */
  P->camera3D = vtkCamera::New();
  assert(NULL != P->camera3D);

  P->cameraTop = vtkCamera::New();
  assert(NULL != P->cameraTop);

  P->cameraFront = vtkCamera::New();
  assert(NULL != P->cameraFront);

  P->cameraSide = vtkCamera::New();
  assert(NULL != P->cameraSide);

  P->geometry = new ProjectiveGeometry();
  assert(NULL != P->geometry);

  if ( (NULL == P->camera3D) ||
       (NULL == P->cameraTop) ||
       (NULL == P->cameraFront) ||
       (NULL == P->cameraSide) ||
       (NULL == P->geometry)
       )
    {
      VTKDeleteViewPointData( P );
      return NULL;
    }
  /* if */

  /* Copy geometry. */
  *(P->geometry) = *geometry;

  /* Update geometry data. */
  VTKUpdateAllCameras_inline(P, P->geometry, NULL, NULL, false);

  return( P );
}
/* VTKCreateViewPointData */



//! Adds actor to VTK renderer.
/*!
  Adds actor to VTK renderer. As only one thread can modify the renderer
  data function call is blocking until resources are free.

  \param W      Pointer to display window structure.
  \param A      Pointer to actor to add.
  \return Function returns true if successfull.
*/
bool
VTKAddActorToDisplayWindow(
                           VTKwindowdata * const W,
                           vtkProp * const A
                           )
{
  bool added = false;

  assert(NULL != W);
  assert(NULL != A);
  if ( (NULL == W) || (NULL == A) ) return added;

  /* Only one instance can change variables. */
  EnterCriticalSection( &(W->rendererCS) );
  {
    bool const res3D = VTKAddActorToRenderer_inline(W->ren3D, A);
    bool const resTop = VTKAddActorToRenderer_inline(W->renTop, A);
    bool const resFront = VTKAddActorToRenderer_inline(W->renFront, A);
    bool const resSide = VTKAddActorToRenderer_inline(W->renSide, A);

    added = res3D && resTop && resFront && resSide;
  }
  LeaveCriticalSection( &(W->rendererCS) );

  return added;
}
/* VTKAddActorToDisplayWindow */



//! Removes actor from VTK renderer.
/*!
  Removes actor from VTK renderer. As only one thread can modify the renderer
  data call is blocking until resources are free.

  \see VTKRemoveActorFromRenderer_inline

  \param W      Pointer to display window structure.
  \param A      Pointer to actor to add.
  \return Function returns true if successfull.
*/
bool
VTKRemoveActorFromDisplayWindow(
                                VTKwindowdata * const W,
                                vtkProp * const A
                                )
{
  bool removed = false;

  assert(NULL != W);
  assert(NULL != A);
  if ( (NULL == W) || (NULL == A) ) return removed;

  /* Only one instance can change variables. */
  EnterCriticalSection( &(W->rendererCS) );
  {
    bool const res3D = VTKRemoveActorFromRenderer_inline(W->ren3D, A);
    bool const resTop = VTKRemoveActorFromRenderer_inline(W->renTop, A);
    bool const resFront = VTKRemoveActorFromRenderer_inline(W->renFront, A);
    bool const resSide = VTKRemoveActorFromRenderer_inline(W->renSide, A);

    removed = res3D && resTop && resFront && resSide;
  }
  LeaveCriticalSection( &(W->rendererCS) );

  return removed;
}
/* VTKRemoveActorFromDisplayWindow */



//! Toggle actor in VTK renderer.
/*!
  If actor is a member of VTK renderer then it is removed, if not then it is added.
  As only one thread can modify the renderer data call is blocking until resources are free.

  \see VTKToggleActorInRenderer_inline

  \param W      Pointer to display window structure.
  \param A      Pointer to actor to add/remove.
  \return Function returns true if successfull.
*/
bool
VTKToggleActorInDisplayWindow(
                              VTKwindowdata * const W,
                              vtkActor * const A
                              )
{
  bool toggled = false;

  assert(NULL != W);
  assert(NULL != A);
  if ( (NULL == W) || (NULL == A) ) return toggled;

  /* Only one instance can change variables. */
  EnterCriticalSection( &(W->rendererCS) );
  {
    bool const res3D = VTKToggleActorInRenderer_inline(W->ren3D, A);
    bool const resTop = VTKToggleActorInRenderer_inline(W->renTop, A);
    bool const resFront = VTKToggleActorInRenderer_inline(W->renFront, A);
    bool const resSide = VTKToggleActorInRenderer_inline(W->renSide, A);

    toggled = res3D && resTop && resFront && resSide;
  }
  LeaveCriticalSection( &(W->rendererCS) );

  return toggled;
}
/* VTKToggleActorInDisplayWindow */



/****** CLASSES ******/

//! Static new method to create class instances.
vtkStandardNewMacro(CustomInteractorStyle);


//! Override keypress handler.
/*!
  We override default keypress handler to gobble up unwanted commands.

  Default keypresses are (taken from the VTK documentation):
  - Keypress j / Keypress t: toggle between joystick (position sensitive)
  and trackball (motion sensitive) styles. In joystick style, motion occurs
  continuously as long as a mouse button is pressed. In trackball style,
  motion occurs when the mouse button is pressed and the mouse pointer moves.
  - Keypress c / Keypress a: toggle between camera and actor modes. In camera
  mode, mouse events affect the camera position and focal point. In actor
  mode, mouse events affect the actor that is under the mouse pointer.
  - Button 1: rotate the camera around its focal point (if camera mode)
  or rotate the actor around its origin (if actor mode). The rotation is
  in the direction defined from the center of the renderer's viewport towards
  the mouse position. In joystick mode, the magnitude of the rotation is
  determined by the distance the mouse is from the center of the render window.
  - Button 2: pan the camera (if camera mode) or translate the actor
  (if actor mode). In joystick mode, the direction of pan or translation is
  from the center of the viewport towards the mouse position. In trackball
  mode, the direction of motion is the direction the mouse moves.
  (Note: with 2-button mice, pan is defined as shift-Button 1.)
  - Button 3: zoom the camera (if camera mode) or scale the actor
  (if actor mode). Zoom in/increase scale if the mouse position is in
  the top half of the viewport; zoom out/decrease scale if the mouse
  position is in the bottom half. In joystick mode, the amount of zoom
  is controlled by the distance of the mouse pointer from the horizontal
  centerline of the window.
  - Keypress 3: toggle the render window into and out of stereo mode.
  By default, red-blue stereo pairs are created. Some systems support
  Crystal Eyes LCD stereo glasses; you have to invoke
  SetStereoTypeToCrystalEyes() on the rendering window.
  - Keypress e: exit the application.
  - Keypress f: fly to the picked point
  - Keypress p: perform a pick operation. The render window interactor
  has an internal instance of vtkCellPicker that it uses to pick.
  - Keypress r: reset the camera view along the current view direction.
  Centers the actors and moves the camera so that all actors are visible.
  - Keypress s: modify the representation of all actors so that they are surfaces.
  - Keypress u: invoke the user-defined function. Typically, this keypress will
  bring up an interactor that you can type commands in.
  - Keypress w: modify the representation of all actors so that they are wireframe.
*/
void
CustomInteractorStyle::OnChar()
{
  // Get the keypress.
  vtkRenderWindowInteractor * const iren = this->Interactor;
  if (NULL == iren) return;
  char key = iren->GetKeyCode();

  // Gobble-up unwanted events.
  if ( (key == 'j') || (key == 'J') ) return;
  if ( (key == 't') || (key == 'T') ) return;
  if ( (key == 'c') || (key == 'C') ) return;
  if ( (key == 'A') || (key == 'A') ) return;
  if (key == '3') return;
  if ( (key == 'e') || (key == 'E') ) return;
  if ( (key == 'f') || (key == 'F') ) return;
  if ( (key == 'p') || (key == 'P') ) return;
  if ( (key == 'r') || (key == 'R') ) return;
  if ( (key == 's') || (key == 'S') ) return;
  if ( (key == 'w') || (key == 'W') ) return;
  if ( (key == 'q') || (key == 'Q') ) return;

  // Forward all other events.
  vtkInteractorStyleTrackballCamera::OnChar();
}
/* CustomInteractorStyle::OnChar() */



//! Override default state handler.
/*!
  As the user interaction requires both 3D and 2D interactor styles here
  we test over which renderer the action was started and set the corresponding
  flag as needed.

  \param newstate State ID.
*/
void
CustomInteractorStyle::StartState(
                                  int newstate
                                  )
{
  if (NULL != this->CurrentRenderer)
    {
      vtkRenderWindowInteractor * const rwi = this->Interactor;
      double const x = rwi->GetEventPosition()[0];
      double const y = rwi->GetEventPosition()[1];
      int * const size = this->GetInteractor()->GetRenderWindow()->GetSize();
      if (NULL != size)
        {
          if (x > this->border_x * size[0])
            {
              this->limit_to_2D = true;
              this->InvokeEvent(vtkCommand::UserEvent);
            }
          else
            {
              this->limit_to_2D = false;
            }
          /* if */
        }
      /* if */
    }
  /* if */

  vtkInteractorStyleTrackballCamera::StartState(newstate);
}
/* CustomInteractorStyle::StartState */



//! Override default rotate handler.
/*!
  Rotation is translated to pan for 2D interaction.
*/
void
CustomInteractorStyle::Rotate()
{
  if (false == limit_to_2D)
    {
      vtkInteractorStyleTrackballCamera::Rotate();
    }
  else
    {
      vtkInteractorStyleTrackballCamera::Pan();
      this->InvokeEvent(vtkCommand::UserEvent);
    }
  /* if */
}
/* CustomInteractorStyle::Rotate */



//! Override deafult spin handler.
/*!
  Spin is not allowed for 2D interaction.
*/
void
CustomInteractorStyle::Spin()
{
  if (false == limit_to_2D)
    {
      vtkInteractorStyleTrackballCamera::Spin();
    }
  else
    {
      // Gobble up event!
    }
  /* if */
}
/* CustomInteractorStyle::Spin */



//! Override default dolly handler.
/*!
  Depending on where the action was started the camera motion changes.
*/
void
CustomInteractorStyle::Dolly(
                             double factor
                             )
{
  if (false == limit_to_2D)
    {
      vtkInteractorStyleTrackballCamera::Dolly(factor);
    }
  else
    {
      vtkInteractorStyleTrackballCamera::Dolly(factor);
      this->InvokeEvent(vtkCommand::UserEvent);
    }
  /* if */
}
/* CustomInteractorStyle::Dolly */



//! Callback to update the visible points in the point cloud.
/*!
  Function updates visible points in the point cloud. Visibility
  is impletented via alpha channel by setting the opacity to
  either fully transparent or fully opaque. Any other approach
  must invalidate the visualization pipeline and therefore
  would be computationally more demanding.

  \param caller Pointer to caller class.
  \param eventId Event ID.
  \param callData Caller data.
*/
void
DynamicRangeThresholdCallback::Execute(
                                       vtkObject * caller,
                                       unsigned long eventId,
                                       void * callData
                                       )
{
  vtkSliderWidget * const sldThr = reinterpret_cast<vtkSliderWidget *>(caller);

  if (NULL == sldThr) return;
  if (NULL == this->D) return;
  if (NULL == this->D->point_clouds) return;

  int const n = (int)( this->D->point_clouds->size() );
  assert(0 < n);
  if (0 == n) return;

  EnterCriticalSection( &(this->D->dataCS) );
  {
    int CloudID = this->D->CloudID;
    if (CloudID > n) CloudID = n - 1;
    if (CloudID < 0) CloudID = 0;
    assert( (0 <= CloudID) && (CloudID < n) );

    VTKpointclouddata_ * const points = ( *(this->D->point_clouds) )[CloudID];

    if (NULL != points)
      {
        std::vector<unsigned char> * const pMask = points->pMask;
        assert(NULL != pMask);
        if (NULL == pMask) return;

        int const N = (int)(pMask->size());
        unsigned char const * const msk = &( pMask->front() );
        assert(NULL != msk);
        if (NULL == msk) return;

        vtkUnsignedCharArray * const ColorsMapped = points->ColorsMapped;
        assert(NULL != ColorsMapped);
        if (NULL == ColorsMapped) return;

        assert( N == ColorsMapped->GetNumberOfTuples() );
        assert( 4 == ColorsMapped->GetNumberOfComponents() );

        unsigned char * const dst = ColorsMapped->WritePointer(0, 0);

        double const thr_d = static_cast<vtkSliderRepresentation *>(sldThr->GetRepresentation())->GetValue();
        if ( isnanorinf_inline(thr_d) ) return;

        float const thr_f = (float)( thr_d );

        unsigned char const invisible = 0;
        unsigned char const visible = 255;

        switch (points->thresholdType)
          {
          default:
          case VTK_THRESHOLD_RANGE:
            {
              assert( (points->rangeMin <= thr_f) && (thr_f <= points->rangeMax) );
              points->rangeThr = thr_f;

              std::vector<float> * const pDynamicRange = points->pDynamicRange;
              assert( (NULL != pDynamicRange) && (N == (int)(pDynamicRange->size())) );
              if ( (NULL == pDynamicRange) || (N != (int)(pDynamicRange->size())) ) break;
              float const * const src = &( pDynamicRange->front() );

              int i = 0;
              int const i_max = N - 7;

              // Unroll the loop with step eight.
              for (; i < i_max; i += 8)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i    ] >= thr_f) && (0 == msk[i    ]) ) dst[adr     ] = visible; else dst[adr     ] = invisible;
                  if ( (src[i + 1] >= thr_f) && (0 == msk[i + 1]) ) dst[adr +  4] = visible; else dst[adr +  4] = invisible;
                  if ( (src[i + 2] >= thr_f) && (0 == msk[i + 2]) ) dst[adr +  8] = visible; else dst[adr +  8] = invisible;
                  if ( (src[i + 3] >= thr_f) && (0 == msk[i + 3]) ) dst[adr + 12] = visible; else dst[adr + 12] = invisible;
                  if ( (src[i + 4] >= thr_f) && (0 == msk[i + 4]) ) dst[adr + 16] = visible; else dst[adr + 16] = invisible;
                  if ( (src[i + 5] >= thr_f) && (0 == msk[i + 5]) ) dst[adr + 20] = visible; else dst[adr + 20] = invisible;
                  if ( (src[i + 6] >= thr_f) && (0 == msk[i + 6]) ) dst[adr + 24] = visible; else dst[adr + 24] = invisible;
                  if ( (src[i + 7] >= thr_f) && (0 == msk[i + 7]) ) dst[adr + 28] = visible; else dst[adr + 28] = invisible;
                }
              /* for */

              // Process remainding items.
              for (; i < N; ++i)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i] >= thr_f) && (0 == msk[i]) ) dst[adr] = visible; else dst[adr] = invisible;
                }
              /* for */
            }
            break;

          case VTK_THRESHOLD_RAY_DISTANCE:
            {
              assert( (points->rayDistanceMin <= thr_f) && (thr_f <= points->rayDistanceMax) );
              points->rayDistanceThr = thr_f;

              std::vector<float> * const pRayDistance = points->pRayDistance;
              assert( (NULL != pRayDistance) && (N == (int)(pRayDistance->size())) );
              if ( (NULL == pRayDistance) || (N != (int)(pRayDistance->size())) ) break;
              float const * const src = &( pRayDistance->front() );

              int i = 0;
              int const i_max = N - 7;

              // Unroll the loop with step eight.
              for (; i < i_max; i += 8)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i    ] <= thr_f) && (0 == msk[i    ]) ) dst[adr     ] = visible; else dst[adr     ] = invisible;
                  if ( (src[i + 1] <= thr_f) && (0 == msk[i + 1]) ) dst[adr +  4] = visible; else dst[adr +  4] = invisible;
                  if ( (src[i + 2] <= thr_f) && (0 == msk[i + 2]) ) dst[adr +  8] = visible; else dst[adr +  8] = invisible;
                  if ( (src[i + 3] <= thr_f) && (0 == msk[i + 3]) ) dst[adr + 12] = visible; else dst[adr + 12] = invisible;
                  if ( (src[i + 4] <= thr_f) && (0 == msk[i + 4]) ) dst[adr + 16] = visible; else dst[adr + 16] = invisible;
                  if ( (src[i + 5] <= thr_f) && (0 == msk[i + 5]) ) dst[adr + 20] = visible; else dst[adr + 20] = invisible;
                  if ( (src[i + 6] <= thr_f) && (0 == msk[i + 6]) ) dst[adr + 24] = visible; else dst[adr + 24] = invisible;
                  if ( (src[i + 7] <= thr_f) && (0 == msk[i + 7]) ) dst[adr + 28] = visible; else dst[adr + 28] = invisible;
                }
              /* for */

              // Process remainding items.
              for (; i < N; ++i)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i] <= thr_f) && (0 == msk[i]) ) dst[adr] = visible; else dst[adr] = invisible;
                }
              /* for */

            }
            break;

          case VTK_THRESHOLD_PHASE_DISTANCE:
            {
              assert( (points->phaseDistanceMin <= thr_f) && (thr_f <= points->phaseDistanceMax) );
              points->phaseDistanceThr = thr_f;

              std::vector<float> * const pPhaseDistance = points->pPhaseDistance;
              assert( (NULL != pPhaseDistance) && (N == (int)(pPhaseDistance->size())) );
              if ((NULL == pPhaseDistance) || (N != (int)(pPhaseDistance->size())) ) break;
              float const * const src = &( pPhaseDistance->front() );

              int i = 0;
              int const i_max = N - 7;

              // Unroll the loop with step eight.
              for (; i < i_max; i += 8)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i    ] <= thr_f) && (0 == msk[i    ]) ) dst[adr     ] = visible; else dst[adr     ] = invisible;
                  if ( (src[i + 1] <= thr_f) && (0 == msk[i + 1]) ) dst[adr +  4] = visible; else dst[adr +  4] = invisible;
                  if ( (src[i + 2] <= thr_f) && (0 == msk[i + 2]) ) dst[adr +  8] = visible; else dst[adr +  8] = invisible;
                  if ( (src[i + 3] <= thr_f) && (0 == msk[i + 3]) ) dst[adr + 12] = visible; else dst[adr + 12] = invisible;
                  if ( (src[i + 4] <= thr_f) && (0 == msk[i + 4]) ) dst[adr + 16] = visible; else dst[adr + 16] = invisible;
                  if ( (src[i + 5] <= thr_f) && (0 == msk[i + 5]) ) dst[adr + 20] = visible; else dst[adr + 20] = invisible;
                  if ( (src[i + 6] <= thr_f) && (0 == msk[i + 6]) ) dst[adr + 24] = visible; else dst[adr + 24] = invisible;
                  if ( (src[i + 7] <= thr_f) && (0 == msk[i + 7]) ) dst[adr + 28] = visible; else dst[adr + 28] = invisible;
                }
              /* for */

              // Process remainding items.
              for (; i < N; ++i)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i] <= thr_f) && (0 == msk[i]) ) dst[adr] = visible; else dst[adr] = invisible;
                }
              /* for */
            }
            break;

          case VTK_THRESHOLD_PHASE_DEVIATION:
            {
              assert( (points->phaseDeviationMin <= thr_f) && (thr_f <= points->phaseDeviationMax) );
              points->phaseDeviationThr = thr_f;

              std::vector<float> * const pPhaseDeviation = points->pPhaseDeviation;
              assert( (NULL != pPhaseDeviation) && (N == (int)(pPhaseDeviation->size())) );
              if ((NULL == pPhaseDeviation) || (N != (int)(pPhaseDeviation->size())) ) break;
              float const * const src = &( pPhaseDeviation->front() );

              int i = 0;
              int const i_max = N - 7;

              // Unroll the loop with step eight.
              for (; i < i_max; i += 8)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i    ] <= thr_f) && (0 == msk[i    ]) ) dst[adr     ] = visible; else dst[adr     ] = invisible;
                  if ( (src[i + 1] <= thr_f) && (0 == msk[i + 1]) ) dst[adr +  4] = visible; else dst[adr +  4] = invisible;
                  if ( (src[i + 2] <= thr_f) && (0 == msk[i + 2]) ) dst[adr +  8] = visible; else dst[adr +  8] = invisible;
                  if ( (src[i + 3] <= thr_f) && (0 == msk[i + 3]) ) dst[adr + 12] = visible; else dst[adr + 12] = invisible;
                  if ( (src[i + 4] <= thr_f) && (0 == msk[i + 4]) ) dst[adr + 16] = visible; else dst[adr + 16] = invisible;
                  if ( (src[i + 5] <= thr_f) && (0 == msk[i + 5]) ) dst[adr + 20] = visible; else dst[adr + 20] = invisible;
                  if ( (src[i + 6] <= thr_f) && (0 == msk[i + 6]) ) dst[adr + 24] = visible; else dst[adr + 24] = invisible;
                  if ( (src[i + 7] <= thr_f) && (0 == msk[i + 7]) ) dst[adr + 28] = visible; else dst[adr + 28] = invisible;
                }
              /* for */

              // Process remainding items.
              for (; i < N; ++i)
                {
                  int const adr = i * 4 + 3;
                  if ( (src[i] <= thr_f) && (0 == msk[i]) ) dst[adr] = visible; else dst[adr] = invisible;
                }
              /* for */
            }
            break;
          }
        /* switch */

        // Mark color data updated.
        ColorsMapped->DataChanged();
        ColorsMapped->Modified();
      }
    /* if */

  }
  LeaveCriticalSection( &(this->D->dataCS) );
}
/* DynamicRangeThresholdCallback::Execute */



//! Static new method to create class instances.
vtkStandardNewMacro(AlignedLineRepresentation);



//! Override interaction method.
/*!
  Overrides default widget interaction method.

  \param e Event coordinates.
*/
void
AlignedLineRepresentation::StartWidgetInteraction(
                                                  double e[2]
                                                  )
{
  vtkLineRepresentation::StartWidgetInteraction(e);

  // Store the starting line centerpoint.
  this->start_pt[0] = this->ln_pt[0];
  this->start_pt[1] = this->ln_pt[1];
  this->start_pt[2] = this->ln_pt[2];
}
/* AlignedLineRepresentation::StartWidgetInteraction */



//! Override interaction method.
/*!
  Overrides default widget interaction method.

  \param e Event coordinates.
*/
void
AlignedLineRepresentation::WidgetInteraction(
                                             double e[2]
                                             )
{
  if ( ( this->InteractionState == vtkLineRepresentation::OnLine ) ||
       ( this->InteractionState == vtkLineRepresentation::TranslatingP1 ) ||
       ( this->InteractionState == vtkLineRepresentation::TranslatingP2 )
       )
    {
      double x[3], delta[3];

      if ( this->InteractionState == vtkLineRepresentation::OnLine )
        {
          this->LineHandleRepresentation->GetWorldPosition(x);
          delta[0] = x[0] - this->StartLineHandle[0];
          delta[1] = x[1] - this->StartLineHandle[1];
          delta[2] = x[2] - this->StartLineHandle[2];
        }
      else if ( this->InteractionState == vtkLineRepresentation::TranslatingP1 )
        {
          this->Point1Representation->GetWorldPosition(x);
          delta[0] = x[0] - this->StartP1[0];
          delta[1] = x[1] - this->StartP1[1];
          delta[2] = x[2] - this->StartP1[2];
        }
      else if ( this->InteractionState == vtkLineRepresentation::TranslatingP2 )
        {
          this->Point1Representation->GetWorldPosition(x);
          delta[0] = x[0] - this->StartP2[0];
          delta[1] = x[1] - this->StartP2[1];
          delta[2] = x[2] - this->StartP2[2];
        }
      /* if */

      double const len = delta[0] * this->move_vec[0] + delta[1] * this->move_vec[1] + delta[2] * this->move_vec[2];

      this->ln_pt[0] = this->start_pt[0] + len * this->move_vec[0];
      this->ln_pt[1] = this->start_pt[1] + len * this->move_vec[1];
      this->ln_pt[2] = this->start_pt[2] + len * this->move_vec[2];

      this->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
    }
  else if ( this->InteractionState == vtkLineRepresentation::Scaling )
    {
      // Gobble-up scaling!
    }
  else
    {
      // Other states may be passed through.
      //vtkLineRepresentation::WidgetInteraction(e);
    }
  /* if */

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}
/* AlignedLineRepresentation::WidgetInteraction */



//! Stretch line to cover viewport.
/*!
  Stretches line to cover the viewport.

  \param new_plane_crd New fixed coordinate of an aligned plane.
*/
void
AlignedLineRepresentation::StretchLineToCoverViewport(
                                                      double new_plane_crd
                                                      )
{
  bool invoke_user_event = false;

  bool const isnan = isnan_inline(new_plane_crd);
  if (false == isnan)
    {
      if (new_plane_crd == this->plane_crd) return;
      if (VTK_PLANE_AXIAL == this->plane_type)
        {
          invoke_user_event = (this->ln_pt[1] != new_plane_crd);
          this->ln_pt[1] = new_plane_crd;
        }
      /* if */
      if (VTK_PLANE_CORONAL == this->plane_type)
        {
          invoke_user_event = (this->ln_pt[2] != new_plane_crd);
          this->ln_pt[2] = new_plane_crd;
        }
      /* if */
      if (VTK_PLANE_SAGITTAL == this->plane_type)
        {
          invoke_user_event = (this->ln_pt[0] != new_plane_crd);
          this->ln_pt[0] = new_plane_crd;
        }
      /* if */
    }
  /* if */

  double const dx = this->bounds[1] - this->bounds[0];
  double const dy = this->bounds[3] - this->bounds[2];
  double const dz = this->bounds[5] - this->bounds[4];
  double const length = 2.0 * sqrt( dx * dx + dy * dy + dz * dz );

  double r[3], nr[3];
  r[0] = length * this->ln_vec[0];
  r[1] = length * this->ln_vec[1];
  r[2] = length * this->ln_vec[2];
  nr[0] = -r[0];
  nr[1] = -r[1];
  nr[2] = -r[2];

  double pw1[3], pd1[3];
  pw1[0] = this->ln_pt[0] - r[0];
  pw1[1] = this->ln_pt[1] - r[1];
  pw1[2] = this->ln_pt[2] - r[2];

  double pw2[3], pd2[3];
  pw2[0] = this->ln_pt[0] - nr[0];
  pw2[1] = this->ln_pt[1] - nr[1];
  pw2[2] = this->ln_pt[2] - nr[2];

  if (VTK_PLANE_AXIAL == this->plane_type)
    {
      invoke_user_event = this->plane_crd != this->ln_pt[1];
      this->plane_crd = this->ln_pt[1];
    }
  /* if */
  if (VTK_PLANE_CORONAL == this->plane_type)
    {
      invoke_user_event = (this->plane_crd != this->ln_pt[2]);
      this->plane_crd = this->ln_pt[2];
    }
  /* if */
  if (VTK_PLANE_SAGITTAL == this->plane_type)
    {
      invoke_user_event = (this->plane_crd != this->ln_pt[0]);
      this->plane_crd = this->ln_pt[0];
    }
  /* if */

  if (true == invoke_user_event) this->InvokeEvent(vtkCommand::UserEvent);

  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, pw1[0], pw1[1], pw1[2], pd1);
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, pw2[0], pw2[1], pw2[2], pd2);

  double * const viewport = this->Renderer->GetViewport();
  int * const size = this->Renderer->GetRenderWindow()->GetSize();

  double const xmin = (double)(size[0]) * viewport[0];
  double const xmax = (double)(size[0]) * viewport[2];
  double const ymin = (double)(size[1]) * viewport[1];
  double const ymax = (double)(size[1]) * viewport[3];

  double const vx = pd2[0] - pd1[0];
  double const vy = pd2[1] - pd1[1];

  double p1[3], p2[3];

  if ( (fabs(vx) < fabs(vy)) && (fabs(vx) < 0.001) )
    {
      // Vertical line.
      double pt[4];
      double const offset = (xmax - xmin) * 0.1;

      vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pd1[0], ymin - offset, 1, pt);
      p1[0] = pt[0];
      p1[1] = pt[1];
      p1[2] = pt[2];

      vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, pd2[0], ymax + offset, 1, pt);
      p2[0] = pt[0];
      p2[1] = pt[1];
      p2[2] = pt[2];
    }
  else if ( (fabs(vy) < fabs(vx)) && (fabs(vy) < 0.001) )
    {
      // Horizontal line.
      double pt[4];
      double const offset = (ymax - ymin) * 0.1;

      vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, xmin - offset, pd1[1], 1, pt);
      p1[0] = pt[0];
      p1[1] = pt[1];
      p1[2] = pt[2];

      vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, xmax + offset, pd2[1], 1, pt);
      p2[0] = pt[0];
      p2[1] = pt[1];
      p2[2] = pt[2];
    }
  else
    {
      // Neither vertical nor horizontal line. As this is derived representation that
      // should be used with aligned lines only this code segement should never execute.
      double t1, t2;
      char const intersect1 = vtkBox::IntersectBox(this->bounds, pw1, r, p1, t1);
      assert(0 != intersect1);
      char const intersect2 = vtkBox::IntersectBox(this->bounds, pw2, nr, p2, t2);
      assert(0 != intersect2);
    }
  /* if */

  this->Point1Representation->SetWorldPosition(p1);
  this->Point2Representation->SetWorldPosition(p2);
}
/* AlignedLineRepresentation::StretchLineToCoverViewport */



//! Stretch widget to bounding box.
/*!
  Stretches widget representation to the full bounding box.
*/
void
AlignedLineRepresentation::AdjustWidgetPlacement(
                                                 void
                                                 )
{
  double p1[3], p2[3], r[3], o[3], t;

  double const dx = this->bounds[1] - this->bounds[0];
  double const dy = this->bounds[3] - this->bounds[2];
  double const dz = this->bounds[5] - this->bounds[4];
  double const length = 2.0 * sqrt( dx * dx + dy * dy + dz * dz );

  r[0] = length * this->ln_vec[0];
  r[1] = length * this->ln_vec[1];
  r[2] = length * this->ln_vec[2];
  o[0] = this->ln_pt[0] - r[0];
  o[1] = this->ln_pt[1] - r[1];
  o[2] = this->ln_pt[2] - r[2];
  vtkBox::IntersectBox(this->bounds, o, r, p1, t);
  this->SetPoint1WorldPosition(p1);

  r[0] = -length * this->ln_vec[0];
  r[1] = -length * this->ln_vec[1];
  r[2] = -length * this->ln_vec[2];
  o[0] = this->ln_pt[0] - r[0];
  o[1] = this->ln_pt[1] - r[1];
  o[2] = this->ln_pt[2] - r[2];
  vtkBox::IntersectBox(this->bounds, o, r, p2, t);
  this->SetPoint2WorldPosition(p2);

  this->LineHandleRepresentation->SetWorldPosition(this->ln_pt);
}
/* AlignedLineRepresentation::AdjustWidgetPlacement */



//! Sets line parameters.
/*!
  Sets line parametrs.

  \param pt Line center-point.
  \param vec Line direction.
  \param move Allowed line translation.
  \param bds Bounding box.
  \param type Plane type may be axial, coronal, or sagittal.
*/
void
AlignedLineRepresentation::SetLineParameters(
                                             double pt[3],
                                             double vec[3],
                                             double move[3],
                                             double bds[6],
                                             SlicingPlane type
                                             )
{
  if (NULL != pt)
    {
      this->ln_pt[0] = pt[0];
      this->ln_pt[1] = pt[1];
      this->ln_pt[2] = pt[2];
    }
  /* if */

  if (NULL != vec)
    {
      this->ln_vec[0] = vec[0];
      this->ln_vec[1] = vec[1];
      this->ln_vec[2] = vec[2];
    }
  /* if */

  if (NULL != move)
    {
      this->move_vec[0] = move[0];
      this->move_vec[1] = move[1];
      this->move_vec[2] = move[2];
    }
  /* if */

  if (NULL != bds)
    {
      this->bounds[0] = bds[0];  this->bounds[1] = bds[1];
      this->bounds[2] = bds[2];  this->bounds[3] = bds[3];
      this->bounds[4] = bds[4];  this->bounds[5] = bds[5];
    }
  /* if */

  this->plane_type = type;

  if (type == VTK_PLANE_AXIAL) assert( (0.0 == move[0]) && (1.0 == move[1]) && (0.0 == move[2]) && (0.0 == vec[1]) );
  if (type == VTK_PLANE_CORONAL) assert( (0.0 == move[0]) && (0.0 == move[1]) && (1.0 == move[2]) && (0.0 == vec[2]) );
  if (type == VTK_PLANE_SAGITTAL) assert( (1.0 == move[0]) && (0.0 == move[1]) && (0.0 == move[2]) && (0.0 == vec[0]) );

  //this->AdjustWidgetPlacement();
  this->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
}
/* AlignedLineRepresentation::SetLineParameters */



//! Sets line color.
/*!
  Sets line color.

  \param r Red component.
  \param g Green component.
  \param b Blue component.
*/
void
AlignedLineRepresentation::SetLineColor(
                                        double r,
                                        double g,
                                        double b
                                        )
{
  if (NULL != this->EndPointProperty) this->EndPointProperty->SetColor(r, g, b);
  if (NULL != this->SelectedEndPointProperty) this->SelectedEndPointProperty->SetColor(r, g, b);

  if (NULL != this->EndPoint2Property) this->EndPoint2Property->SetColor(r, g, b);
  if (NULL != this->SelectedEndPoint2Property) this->SelectedEndPoint2Property->SetColor(r, g, b);

  if (NULL != this->LineProperty)
    {
      this->LineProperty->SetAmbientColor(r, g, b);
      this->LineProperty->SetLineWidth(2.0);
    }
  /* if */

  if (NULL != this->SelectedLineProperty)
    {
      this->SelectedLineProperty->SetAmbientColor(r, g, b);
      this->SelectedLineProperty->SetLineWidth(4.0);
    }
  /* if */
}
/* AlignedLineRepresentation::SetLineColor */



//! Testif mouse is over the line.
/*!
  Test if mouse pointer is over the line.

  \param X      Mouse x coordinate.
  \param Y      Mouse y coordinate.
  \param modify Not used.
  \return Returns object state.
*/
int
AlignedLineRepresentation::ComputeInteractionState(
                                                   int X,
                                                   int Y,
                                                   int modify
                                                   )
{
  // Check if we are within the viewport.
  double * const viewport = this->Renderer->GetViewport();
  int * const size = this->Renderer->GetRenderWindow()->GetSize();

  double const xmin = (double)(size[0]) * viewport[0];
  double const xmax = (double)(size[0]) * viewport[2];
  double const ymin = (double)(size[1]) * viewport[1];
  double const ymax = (double)(size[1]) * viewport[3];

  bool const outside = ((double)(X) < xmin) || (xmax < (double)(X)) || ((double)(Y) < ymin) || (ymax < (double)(Y));
  if (true == outside)
    {
      this->InteractionState = vtkLineRepresentation::Outside;
      this->SetRepresentationState(vtkLineRepresentation::Outside);
      return this->InteractionState;
    }
  /* if */

  // Either pass control to overloaded method or test if we are on line only.
  //return vtkLineRepresentation::ComputeInteractionState(X, Y, modify);

  // Check if we are on the line.
  double pos1[3], pos2[3];
  this->GetPoint1DisplayPosition(pos1);
  this->GetPoint2DisplayPosition(pos2);

  double p1[3], p2[3], xyz[3];
  double t, closest[3];
  xyz[0] = static_cast<double>(X);
  xyz[1] = static_cast<double>(Y);
  p1[0] = static_cast<double>(pos1[0]);
  p1[1] = static_cast<double>(pos1[1]);
  p2[0] = static_cast<double>(pos2[0]);
  p2[1] = static_cast<double>(pos2[1]);
  xyz[2] = p1[2] = p2[2] = 0.0;

  double const tol2 = this->Tolerance * this->Tolerance;

  bool const online = (vtkLine::DistanceToLine(xyz,p1,p2,t,closest) <= tol2);
  if ( (true == online) && (0.0 <= t) && (t <= 1.0) )
    {
      this->InteractionState = vtkLineRepresentation::OnLine;
      this->SetRepresentationState(vtkLineRepresentation::OnLine);
      this->GetPoint1WorldPosition(pos1);
      this->GetPoint2WorldPosition(pos2);

      this->LinePicker->Pick(X, Y, 0.0, this->Renderer);
      this->LinePicker->GetPickPosition(closest);
      this->LineHandleRepresentation->SetWorldPosition(closest);
    }
  else
    {
      this->InteractionState = vtkLineRepresentation::Outside;
      this->SetRepresentationState(vtkLineRepresentation::Outside);
    }
  /* if */

  return this->InteractionState;
}
/* AlignedLineRepresentation::ComputeInteractionState */



//! Callback to update the second line.
/*!
  Function updates the position of the second line which
  defines the slicing plane.

  \param caller Pointer to caller class.
  \param eventId Event ID.
  \param callData Caller data.
*/
void
AlignedLineCallback::Execute(
                             vtkObject * caller,
                             unsigned long eventId,
                             void * callData
                             )
{
  if (NULL == this->L) return;

  vtkLineWidget2 * const ptr = reinterpret_cast<vtkLineWidget2 *>(caller);
  if (NULL == ptr) return;

  AlignedLineRepresentation * const master = reinterpret_cast<AlignedLineRepresentation *>( ptr->GetRepresentation() );
  if (NULL == master) return;

  assert(this->L->plane_type == master->plane_type);

  this->L->StretchLineToCoverViewport(master->plane_crd);
}
/* AlignedLineCallback::Execute */



//! Callback to update all slicing planes.
/*!
  Function updates all slicing planes to cover the whole render viewport.

  \param caller Pointer to caller class.
  \param eventId Event ID.
  \param callData Caller data.
*/
void
AllAlignedLinesCallback::Execute(
                                 vtkObject * caller,
                                 unsigned long eventId,
                                 void * callData
                                 )
{
  if (NULL == W) return;

  if (NULL != W->representationCoronal1) W->representationCoronal1->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
  if (NULL != W->representationSagittal1) W->representationSagittal1->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
  if (NULL != W->representationAxial1) W->representationAxial1->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
  if (NULL != W->representationSagittal2) W->representationSagittal2->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
  if (NULL != W->representationAxial2) W->representationAxial2->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
  if (NULL != W->representationCoronal2) W->representationCoronal2->StretchLineToCoverViewport(BATCHACQUISITION_qNaN_dv);
}
/* AllAlignedLinesCallback::Execute */



//! Callback to update plane position.
/*!
  Function updates all slicing plane position.

  \param caller Pointer to caller class.
  \param eventId Event ID.
  \param callData Caller data.
*/
void
SlicingPlaneCallback::Execute(
                              vtkObject * caller,
                              unsigned long eventId,
                              void * callData
                              )
{
  if (NULL == this->P) return;

  AlignedLineRepresentation * const ptr = reinterpret_cast<AlignedLineRepresentation *>( caller );
  if (NULL == ptr) return;

  double pt[3] = {this->P->px, this->P->py, this->P->pz};
  bool update_plane = false;
  if (VTK_PLANE_AXIAL == ptr->plane_type)
    {
      update_plane = (pt[1] != ptr->ln_pt[1]);
      pt[1] = ptr->ln_pt[1];
    }
  /* if */
  if (VTK_PLANE_CORONAL == ptr->plane_type)
    {
      update_plane = (pt[2] != ptr->ln_pt[2]);
      pt[2] = ptr->ln_pt[2];
    }
  /* if */
  if (VTK_PLANE_SAGITTAL == ptr->plane_type)
    {
      update_plane = (pt[0] != ptr->ln_pt[0]);
      pt[0] = ptr->ln_pt[0];
    }
  /* if */

  if (true == update_plane)
    {
      VTKUpdateSlicingPlane_inline(this->P, NULL, pt, NULL);
    }
  /* if */
}
/* SlicingPlaneCallback::Execute */



/****** CALLBACKS ******/

//! Render event callback to add pushed actors to the scene.
/*!
  Actor manipulation for VTK must be done from the same thread that handles
  the window interactor and the renderer. To achieve this we use thread data
  storage structure where actors to be displayed can be stored temporarily.
  Every time display is updated this function is run via callback.
  Once called we check wheather new actor data is supplied.
  If it is then the renderer must be updated to reflect new data state.

  \param caller Pointer to VTK interactor.
  \param eventId        Received event ID.
  \param clientData     Pointer to clinet supplied data.
  \param callData       Pointer to data.
*/
void
VTKActorPushCallback(
                     vtkObject * caller,
                     long unsigned int eventId,
                     void * clientData,
                     void * callData
                     )
{
  assert(NULL != clientData);

  VTKdisplaythreaddata * const D = (VTKdisplaythreaddata *)(clientData);
  if (NULL == D) return;
  if (NULL == D->window) return;

  assert(D == D->myAddress);

  bool const have_jobs =
    (true == D->point_cloud_pushed) ||
    (true == D->projector_geometry_pushed) ||
    (true == D->camera_geometry_pushed) ||
    (true == D->camera_pushed);

  if (true == have_jobs)
    {
      /* Only one thread can manipulate the VTK data. */
      EnterCriticalSection( &(D->window->rendererCS) );
      EnterCriticalSection( &(D->pushCS) );
      EnterCriticalSection( &(D->dataCS) );
      {

        /* Update point clouds. */
        if (true == D->point_cloud_pushed)
          {
            bool data_changed = false;

            if ( (NULL != D->point_clouds) && (NULL != D->point_cloudsNEW) )
              {
                int const n = (int)( D->point_clouds->size() );
                int const nNEW = (int)( D->point_cloudsNEW->size() );
                for (int i = 0; i < nNEW; ++i)
                  {
                    assert(i < n);
                    if (i >= n) continue;

                    VTKpointclouddata_ * pointsNEW = ( *(D->point_cloudsNEW) )[i];
                    //assert(NULL != pointsNEW);
                    if (NULL == pointsNEW) continue;

                    data_changed = true;

                    /* We have new point cloud to add to the renderer. But first the old one must be removed! */
                    VTKpointclouddata_ * points = ( *(D->point_clouds) )[i];
                    //assert(NULL != points);
                    if (NULL != points)
                      {
                        bool const remove = VTKRemovePointCloudFromDisplayWindow_inline(D->window, points);
                        assert(true == remove);

                        VTKDeletePointCloudData( points );
                        points = NULL;
                        ( *(D->point_clouds) )[i] = NULL;
                      }
                    /* if */

                    bool const add = VTKAddActorToDisplayWindow(D->window, pointsNEW->Actor);
                    assert(true == add);

                    assert(NULL == ( *(D->point_clouds) )[i]);
                    ( *(D->point_clouds) )[i] = pointsNEW;
                    ( *(D->point_cloudsNEW) )[i] = NULL;

                    pointsNEW->Actor->Modified();

                    /* Set point cloud ID if there is none set. */
                    if (0 > D->CloudID) D->CloudID = i;

                    /* Update slider widget. */
                    pointsNEW->thresholdType = VTK_THRESHOLD_RANGE;
                    if (D->CloudID == i)
                      {
                        D->CloudID = -1; // To force update.
                        VTKSetActivePointCloud_inline(D, i);
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */

            if (true == data_changed)
              {
                /* Update volume slicing planes. */
                VTKUpdateAllPlaneWidgets_inline(D);
                VTKSetSlicingPlaneBounds_inline(D);

                /* Update statistics. */
                if (NULL != D->window->slicingStatistics) D->window->slicingStatistics->SetInput(gMsgClipStatisticsUpdateMessage);
              }
            /* if */

            /* Force redraw. */
            VTKUpdateDisplay(D);

            D->point_cloud_pushed = false;
          }
        else
          {
#ifdef _DEBUG
            /* All input data slots must contain a NULL pointer. */
            if (NULL != D->point_cloudsNEW)
              {
                int const nNEW = (int)( D->point_cloudsNEW->size() );
                for (int i = 0; i < nNEW; ++i)
                  {
                    assert(NULL == ( *(D->point_cloudsNEW) )[i] );
                  }
                /* for */
              }
            /* if */
#endif /* _DEBUG */
          }
        /* if */


        /* Update projector geometry. */
        if (true == D->projector_geometry_pushed)
          {
            if ( (NULL != D->projector_geometries) && (NULL != D->projector_geometriesNEW) )
              {
                int const n = (int)( D->projector_geometries->size() );
                int const nNEW = (int)( D->projector_geometriesNEW->size() );
                for (int i = 0; i < nNEW; ++i)
                  {
                    assert(i < n);
                    if (i >= n) continue;

                    ProjectiveGeometry_ * const geometryNEW = ( *(D->projector_geometriesNEW) )[i];
                    //assert(NULL != geometryNEW);
                    if (NULL == geometryNEW) continue;

                    /* Move geometry to regular storage. */
                    ( *(D->projector_geometriesNEW) )[i] = NULL;
                    SAFE_DELETE( ( *(D->projector_geometries) )[i] );
                    ( *(D->projector_geometries) )[i] = geometryNEW;

                    /* Update VTK camera if projector matches. */
                    if (D->ProjectorID == i)
                      {
                        assert(0 > D->CameraID);
                        VTKChangeCameraGeometry_inline(D, geometryNEW, false);
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */

            D->projector_geometry_pushed = false;
          }
        else
          {
#ifdef _DEBUG
            /* All input data slots must contain a NULL pointer. */
            if (NULL != D->projector_geometriesNEW)
              {
                int const nNEW = (int)( D->projector_geometriesNEW->size() );
                for (int i = 0; i < nNEW; ++i)
                  {
                    assert( NULL == ( *(D->projector_geometriesNEW) )[i] );
                  }
                /* for */
              }
            /* if */
#endif /* _DEBUG */
          }
        /* if */


        /* Update camera geometry. */
        if (true == D->camera_geometry_pushed)
          {
            if ( (NULL != D->camera_geometries) && (NULL != D->camera_geometriesNEW) )
              {
                int const n = (int)( D->camera_geometries->size() );
                int const nNEW = (int)( D->camera_geometriesNEW->size() );
                for (int i = 0; i < n; ++i)
                  {
                    assert(i < n);
                    if (i >= n) continue;

                    ProjectiveGeometry_ * const geometryNEW = ( *(D->camera_geometriesNEW) )[i];
                    //assert(NULL != geometryNEW);
                    if (NULL == geometryNEW) continue;

                    /* Move geometry to regular storage. */
                    ( *(D->camera_geometriesNEW) )[i] = NULL;
                    SAFE_DELETE( ( *(D->camera_geometries) )[i] );
                    ( *(D->camera_geometries) )[i] = geometryNEW;

                    /* Set camera ID if none is set. */
                    if ( (-1 == D->CameraID) && (-1 == D->ProjectorID) ) D->CameraID = i;

                    /* Update VTK camera if camera matches. */
                    if (D->CameraID == i)
                      {
                        assert(0 > D->ProjectorID);
                        VTKChangeCameraGeometry_inline(D, geometryNEW, false);
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */

            D->camera_geometry_pushed = false;
          }
        else
          {
#ifdef _DEBUG
            /* All input data slots must contain a NULL pointer. */
            if (NULL != D->camera_geometriesNEW)
              {
                int const nNEW = (int)( D->camera_geometriesNEW->size() );
                for (int i = 0; i < nNEW; ++i)
                  {
                    assert( NULL == ( *(D->camera_geometriesNEW) )[i] );
                  }
                /* for */
              }
            /* if */
#endif /* _DEBUG */
          }
        /* if */


        /* Update active camera. */
        if (true == D->camera_pushed)
          {
            assert(NULL != D->cameraNEW);
            if (NULL != D->cameraNEW)
              {
                VTKviewpoint * const tmp = D->camera;
                D->camera = D->cameraNEW;
                D->cameraNEW = tmp;

                VTKChangeCameraGeometry_inline(D, D->camera->geometry, false);

                D->window->ren3D->SetActiveCamera( D->camera->camera3D );
                D->window->renTop->SetActiveCamera( D->camera->cameraTop );
                D->window->renFront->SetActiveCamera( D->camera->cameraFront );
                D->window->renSide->SetActiveCamera( D->camera->cameraSide );
              }
            /* if */
            D->camera_pushed = false;
          }
        /* if */

      }
      LeaveCriticalSection( &(D->dataCS) );
      LeaveCriticalSection( &(D->pushCS) );
      LeaveCriticalSection( &(D->window->rendererCS) );
    }
  /* if */

  /* Terminate if needed. This must be done outside of the CS segment. */
  if (true == D->terminate) D->window->renWinInt->ExitCallback();
}
/* VTKActorPushCallback */



//! Render event callback to pop actors from the scene.
/*
  Actor manipulation for VTK must be done from the same thread that handles
  the window interactor and the renderer. This function implements actor
  removal.

  \param caller Pointer to VTK interactor.
  \param eventId        Received event ID.
  \param clientData     Pointer to clinet supplied data.
  \param callData       Pointer to data.
*/
void
VTKActorPopCallback(
                    vtkObject * caller,
                    long unsigned int eventId,
                    void * clientData,
                    void * callData
                    )
{
  assert(NULL != clientData);

  VTKdisplaythreaddata * const D = (VTKdisplaythreaddata *)(clientData);
  if (NULL == D) return;
  if (NULL == D->window) return;

  assert(D == D->myAddress);

  bool const have_jobs = (true == D->clear_all);

  if (true == have_jobs)
    {
      /* Only one thread can manipulate the VTK data. */
      EnterCriticalSection( &(D->window->rendererCS) );
      EnterCriticalSection( &(D->pushCS) );
      EnterCriticalSection( &(D->dataCS) );
      {
        if (true == D->clear_all)
          {
            if (NULL != D->point_clouds)
              {
                int const n = (int)( D->point_clouds->size() );
                for (int i = 0; i < n; ++i)
                  {
                    VTKpointclouddata_ * points = ( *(D->point_clouds) )[i];
                    if (NULL != points)
                      {
                        bool const remove = VTKRemovePointCloudFromDisplayWindow_inline(D->window, points);
                        assert(true == remove);

                        VTKDeletePointCloudData( points );
                        points = NULL;
                        ( *(D->point_clouds) )[i] = NULL;
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */

            D->CloudID = -1;
            VTKUpdateThresholdSliderWidget_inline(NULL, D->window);
            if ( (NULL != D->window) && (NULL != D->window->renWin) ) D->window->renWin->SetWindowName(gMsgWindowTitleNoData);

            if (NULL != D->projector_geometries)
              {
                int const n = (int)( D->projector_geometries->size() );
                for (int i = 0; i < n; ++i)
                  {
                    SAFE_DELETE( ( *(D->projector_geometries) )[i] );
                  }
                /* for */
              }
            /* if */

            D->ProjectorID = -1;

            if (NULL != D->camera_geometries)
              {
                int const n = (int)( D->camera_geometries->size() );
                for (int i = 0; i < n; ++i)
                  {
                    SAFE_DELETE( ( *(D->camera_geometries) )[i] );
                  }
                /* for */
              }
            /* if */

            D->CameraID = -1;

            D->clear_all = false;
          }
        /* if */
      }
      LeaveCriticalSection( &(D->dataCS) );
      LeaveCriticalSection( &(D->pushCS) );
      LeaveCriticalSection( &(D->window->rendererCS) );
    }
  /* if */

  /* Terminate if needed. This must be done outside of the CS segment. */
  if (true == D->terminate) D->window->renWinInt->ExitCallback();
}
/* VTKActorPopCallback */



//! Interactor event callback to handle keypresses.
/*!
  We want to change behaviour of the interactor for some of the
  keypresses, i.e. pressing c or p should reset viewpoint to the default aligned
  geometry for either (c)amera or (p)rojector.

  \param caller Pointer to VTK interactor.
  \param eventId        Received event ID.
  \param clientData     Pointer to clinet supplied data.
  \param callData       Pointer to data.
*/
void
VTKKeypressCallback(
                    vtkObject * caller,
                    long unsigned int eventId,
                    void * clientData,
                    void * callData
                    )
{
  VTKdisplaythreaddata * const D = (VTKdisplaythreaddata *)(clientData);
  if (NULL == D) return;

  assert(D == D->myAddress);
  assert(NULL != D->window);
  if (NULL == D->window) return;

  /* Only one thread can manipulate the VTK data. */
  EnterCriticalSection( &(D->window->rendererCS) );

  vtkRenderWindowInteractor * const iren = (vtkRenderWindowInteractor *)( caller );
  if (NULL == iren)
    {
      LeaveCriticalSection( &(D->window->rendererCS) );
      return;
    }
  /* if */

  assert(iren == D->window->renWinInt);

  /* Fetch pressed key. */
  std::string description = iren->GetKeySym();
  char const key = iren->GetKeyCode();
  int const shift = iren->GetShiftKey();
  int const control = iren->GetControlKey();

  EnterCriticalSection( &(D->dataCS) );
  {
    /* Fetch active point cloud. */
    VTKpointclouddata_ * points = NULL;
    int const CloudID = D->CloudID;
    if ( (NULL != D->point_clouds) && (0 <= CloudID) && (CloudID <= (int)(D->point_clouds->size())) )
      {
        points = ( *(D->point_clouds) )[CloudID];
      }
    /* if */

    switch ( key )
      {

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        {
          /* Set active point cloud. */
          int const num_key = (int)(key) - (int)('1');
          if (num_key != CloudID)
            {
              VTKSetActivePointCloud_inline(D, num_key);
            }
          else
            {
              VTKTogglePointCloudVisibility_inline(D, num_key);
            }
          /* if */
          VTKUpdateDisplay(D); // Force redraw.
        }
      break;

      case 't':
      case 'T':
      case (char)20: // CTRL+T
        {
          /* Cycle through available thresholds or reset treshold to default value (CTRL modifier). */
          bool const key_pressed = (0 == control) && ( ('t' == key) || ('T' == key) );
          bool const ctrl_key_pressed = (0 != control) && ((char)20 == key);
          if (NULL != points)
            {
              if (true == key_pressed)
                {
                  ThresholdControl const next = VTKNextThresholdControl_inline(points->thresholdType);
                  if (next != points->thresholdType)
                    {
                      VTKUpdateSelectionMask_inline(points);
                      points->thresholdType = next;
                      VTKUpdateThresholdSliderWidget_inline(points, D->window);
                    }
                  /* if */
                  VTKClearSelectionMask_inline(points);
                  VTKUpdateDisplay(D); // Force redraw.
                }
              else if (true == ctrl_key_pressed)
                {
                  VTKResetSelectionMask_inline(points);
                  VTKUpdateThresholdSliderWidget_inline(points, D->window);
                  VTKUpdateDisplay(D); // Force redraw.
                }
              /* if */
            }
          /* if */
        }
      break;

      case 'o':
      case 'O':
        {
          /* Toggle the outline of the active point cloud. */
          if ( (NULL != points) && (NULL != points->outline) )
            {
              bool const toggle = VTKToggleActorInDisplayWindow(D->window, points->outline->outlineActor);
              assert(true == toggle);
              if (true == toggle) VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
      break;

      case '+':
        {
          /* Increase brightness of the active point cloud. */
          if (NULL != points)
            {
              points->colorScale *= 1.05;
              VTKUpdatePointColors_inline(points);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
        break;

      case '-':
        {
          /* Decrease brightness of the active point cloud. */
          if (NULL != points)
            {
              points->colorScale *= 1.0/1.05;
              VTKUpdatePointColors_inline(points);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
        break;

      case '*':
        {
          /* Reset brightness to initial values. */
          if (NULL != points)
            {
              points->colorScale = 1.0;
              points->colorOffset = 0.0;
              VTKResetPointColors_inline(points);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
        break;

      case 'c':
      case 'C':
        {
          /* Slice active point cloud. */
          if ( (NULL != points) && (NULL != D->window->slicingStatistics) )
            {
              double total_axial = 0;
              double front_axial = 0;
              double back_axial = 0;
              VTKCountPointsInFrontOfSlicingPlanes_inline(D->window->planeAxial, points, &total_axial, &front_axial, &back_axial);
              double const scl_axial = 100.0 / total_axial;

              double total_coronal = 0;
              double front_coronal = 0;
              double back_coronal = 0;
              VTKCountPointsInFrontOfSlicingPlanes_inline(D->window->planeCoronal, points, &total_coronal, &front_coronal, &back_coronal);
              double const scl_coronal = 100.0 / total_coronal;

              double total_sagittal = 0;
              double front_sagittal = 0;
              double back_sagittal = 0;
              VTKCountPointsInFrontOfSlicingPlanes_inline(D->window->planeSagittal, points, &total_sagittal, &front_sagittal, &back_sagittal);
              double const scl_sagittal = 100.0 / total_sagittal;

              std::ostringstream stats;
              stats << std::fixed << std::setprecision(2) << std::setw(5);
              stats << "Axial (green, top vs bottom):  " << front_axial * scl_axial << "% vs " << back_axial * scl_axial << "%\n";
              stats << "Coronal (blue, front vs back):  " << front_coronal * scl_coronal << "% vs " << back_coronal * scl_coronal << "%\n";
              stats << "Sagittal (red, left vs right):  " << front_sagittal * scl_sagittal << "% vs " << back_sagittal * scl_sagittal << "%\n";
              stats << "Press C to update statistics!";

              std::string stats_str(stats.str());

              D->window->slicingStatistics->SetInput(stats_str.c_str());
              D->window->slicingStatistics->Modified();
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
      break;

      case 'p':
      case 'P':
        {
          /* Cycle through visibilities of the slicing planes. */
          VTKCycleSlicingPlaneOpacities_inline(D->window);
          VTKUpdateDisplay(D); // Force redraw.
        }
      break;

      case 'e':
      case 'E':
        {
          /* Export view using VTK functionality. */
          HRESULT const hr = VTKSaveRenderWindowToFile(D->window->renWin);
          //assert(S_OK == hr);
        }
      break;

      case 's':
      case 'S':
      case (char)19: // CTRL+S
        {
          /* Save point cloud to PLY file. */
          bool const key_pressed = (0 == control) && ( ('s' == key) || ('S' == key) );
          bool const ctrl_key_pressed = (0 != control) && ((char)19 == key);
          if (NULL != points)
            {
              if (true == key_pressed)
                {
                  VTKSavePointCloudToPLY_inline(points);
                }
              else if (true == ctrl_key_pressed)
                {
                  VTKSavePointCloudsToPLY_inline(D->point_clouds);
                }
              /* if */
            }
          /* if */
        }
      break;

      case 'm':
      case 'M':
      case (char)13: // CTRL+M
        {
          /* Change VTK camera to match the camera of the active point cloud (CTRL for orthographic). */
          bool const key_pressed = (0 == control) && ( ('m' == key) || ('M' == key) );
          bool const ctrl_key_pressed = (0 != control) && ((char)13 == key);
          if ( (key_pressed != ctrl_key_pressed) && (NULL != points) )
            {
              int const CameraID = points->CameraID;
              bool const parallel = (0 != control)? true : false;
              VTKSetActiveCamera_inline(D, CameraID, parallel);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
      break;

      case 'n':
      case 'N':
      case (char)14: // CTRL+N
        {
          /* Change VTK camera to match the projector of the active point cloud (CTRL for orthographic). */
          bool const key_pressed = (0 == control) && ( ('n' == key) || ('N' == key) );
          bool const ctrl_key_pressed = (0 != control) && ((char)14 == key);
          if ( (key_pressed != ctrl_key_pressed) && (NULL != points) )
            {
              int const ProjectorID = points->ProjectorID;
              bool const parallel = (0 != control)? true : false;
              VTKSetActiveProjector_inline(D, ProjectorID, parallel);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        };
      break;

      case 'l':
      case 'L':
        {
          /* Rotate camera left. */
          if ( (NULL != D->window) && (NULL != D->window->ren3D) )
            {
              D->window->ren3D->GetActiveCamera()->Roll(90);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
      break;

      case 'r':
      case 'R':
        {
          /* Rotate camera right. */
          if ( (NULL != D->window) && (NULL != D->window->ren3D) )
            {
              D->window->ren3D->GetActiveCamera()->Roll(-90);
              VTKUpdateDisplay(D); // Force redraw.
            }
          /* if */
        }
      break;

      default:
        // Nothing to do!
        break;
      }
    /* switch */

  }
  LeaveCriticalSection( &(D->dataCS) );

  LeaveCriticalSection( &(D->window->rendererCS) );

  /* Terminate if needed. This must be done outside of the CS segment. */
  if (true == D->terminate) D->window->renWinInt->ExitCallback();
}
/* VTKKeypressCallback */



/****** VTK DISPLAY WINDOW ******/

//! Close VTK display window.
/*!
  Closes VTK display window.

  \param P      Pointer to display window structure.
*/
void
VTKCloseDisplayWindow(
                      VTKwindowdata * const P
                      )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Only one thread can change variables. */
  EnterCriticalSection( &(P->rendererCS) );
  {
    bool const remove_axial = VTKRemoveActorFromRenderer_inline(P->ren3D, P->planeAxial->Actor);
    assert(true == remove_axial);

    bool const remove_coronal = VTKRemoveActorFromRenderer_inline(P->ren3D, P->planeCoronal->Actor);
    assert(true == remove_coronal);

    bool const remove_sagittal = VTKRemoveActorFromRenderer_inline(P->ren3D, P->planeSagittal->Actor);
    assert(true == remove_sagittal);

    bool const remove_text = VTKRemoveActorFromRenderer_inline(P->ren3D, P->slicingStatistics);
    assert(true == remove_text);

    SAFE_VTK_DELETE( P->sldThrCallback );
    SAFE_VTK_DELETE( P->sldThrRep );
    SAFE_VTK_DELETE( P->sldThr );

    SAFE_VTK_DELETE( P->callbackViewpointChange );

    SAFE_VTK_DELETE( P->callbackAxial );
    SAFE_VTK_DELETE( P->callbackCoronal );
    SAFE_VTK_DELETE( P->callbackSagittal );

    SAFE_VTK_DELETE( P->callbackAxial1 );
    SAFE_VTK_DELETE( P->callbackAxial2 );
    SAFE_VTK_DELETE( P->callbackCoronal1 );
    SAFE_VTK_DELETE( P->callbackCoronal2 );
    SAFE_VTK_DELETE( P->callbackSagittal1 );
    SAFE_VTK_DELETE( P->callbackSagittal2 );

    SAFE_VTK_DELETE( P->planeAxial1 );
    SAFE_VTK_DELETE( P->planeAxial2 );
    SAFE_VTK_DELETE( P->planeCoronal1 );
    SAFE_VTK_DELETE( P->planeCoronal2 );
    SAFE_VTK_DELETE( P->planeSagittal1 );
    SAFE_VTK_DELETE( P->planeSagittal2 );

    SAFE_VTK_DELETE( P->representationAxial1 );
    SAFE_VTK_DELETE( P->representationAxial2 );
    SAFE_VTK_DELETE( P->representationCoronal1 );
    SAFE_VTK_DELETE( P->representationCoronal2 );
    SAFE_VTK_DELETE( P->representationSagittal1 );
    SAFE_VTK_DELETE( P->representationSagittal2 );

    VTKDeleteSlicingPlaneData( P->planeAxial );
    VTKDeleteSlicingPlaneData( P->planeCoronal );
    VTKDeleteSlicingPlaneData( P->planeSagittal );

    SAFE_VTK_DELETE( P->slicingStatistics );

    SAFE_VTK_DELETE( P->renWinInt );
    SAFE_VTK_DELETE( P->renWinIntStyle );
    SAFE_VTK_DELETE( P->renWin );

    SAFE_VTK_DELETE( P->ren3D );
    SAFE_VTK_DELETE( P->renTop );
    SAFE_VTK_DELETE( P->renFront );
    SAFE_VTK_DELETE( P->renSide );

    SAFE_VTK_DELETE( P->pushCallback );
    SAFE_VTK_DELETE( P->popCallback );
    SAFE_VTK_DELETE( P->keypressCallback );
  }
  LeaveCriticalSection( &(P->rendererCS) );

  /* Destroy critical section. */
  DeleteCriticalSection( &(P->rendererCS) );

  VTKBlankWindowData_inline( P );

  free( P );
}
/* VTKCloseDisplayWindow */



//! Open VTK display window.
/*!
  Function opens VTK display window and prepares everything to start the window interactor.

  \param sx     Size of the window.
  \param sy     Size of the window.
  \param data Pointer to callback data structure.
  \return Function returns NULL if unsuccessfull, and pointer to data structure otherwise.
*/
VTKwindowdata *
VTKOpenDisplayWindow(
                     int const sx,
                     int const sy,
                     void * const data
                     )
{
  assert(0 < sx);
  assert(0 < sy);

  /* Allocate storage. */
  VTKwindowdata * const P = (VTKwindowdata *)malloc( sizeof(VTKwindowdata) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  VTKBlankWindowData_inline( P );

  /* Initialize critical section. */
  InitializeCriticalSection( &(P->rendererCS) );

  /* Pre-define required constants. */
  double const origin[3] = {0.0, 0.0, 0.0};

  double const vec_x[3] = {1.0, 0.0, 0.0};  double const vec_nx[3] = {-1.0,  0.0,  0.0};
  double const vec_y[3] = {0.0, 1.0, 0.0};  double const vec_ny[3] = { 0.0, -1.0,  0.0};
  double const vec_z[3] = {0.0, 0.0, 1.0};  double const vec_nz[3] = { 0.0,  0.0, -1.0};

  /* Try to create all necessary elements. */
  P->ren3D = vtkRenderer::New();
  assert(NULL != P->ren3D);

  P->renTop = vtkRenderer::New();
  assert(NULL != P->renTop);

  P->renFront = vtkRenderer::New();
  assert(NULL != P->renFront);

  P->renSide = vtkRenderer::New();
  assert(NULL != P->renSide);

  P->sldThr = vtkSliderWidget::New();
  assert(NULL != P->sldThr);

  P->sldThrRep = vtkSliderRepresentation2D::New();
  assert(NULL != P->sldThrRep);

  P->sldThrCallback = DynamicRangeThresholdCallback::New();
  assert(NULL != P->sldThrCallback);

  P->planeAxial1 = vtkLineWidget2::New();
  assert(NULL != P->planeAxial1);

  P->planeAxial2 = vtkLineWidget2::New();
  assert(NULL != P->planeAxial2);

  P->planeCoronal1 = vtkLineWidget2::New();
  assert(NULL != P->planeCoronal1);

  P->planeCoronal2 = vtkLineWidget2::New();
  assert(NULL != P->planeCoronal2);

  P->planeSagittal1 = vtkLineWidget2::New();
  assert(NULL != P->planeSagittal1);

  P->planeSagittal2 = vtkLineWidget2::New();
  assert(NULL != P->planeSagittal2);

  P->representationAxial1 = AlignedLineRepresentation::New();
  assert(NULL != P->representationAxial1);

  P->representationAxial2 = AlignedLineRepresentation::New();
  assert(NULL != P->representationAxial2);

  P->representationCoronal1 = AlignedLineRepresentation::New();
  assert(NULL != P->representationCoronal1);

  P->representationCoronal2 = AlignedLineRepresentation::New();
  assert(NULL != P->representationCoronal2);

  P->representationSagittal1 = AlignedLineRepresentation::New();
  assert(NULL != P->representationSagittal1);

  P->representationSagittal2 = AlignedLineRepresentation::New();
  assert(NULL != P->representationSagittal2);

  P->callbackAxial1 = AlignedLineCallback::New();
  assert(NULL != P->callbackAxial1);

  P->callbackAxial2 = AlignedLineCallback::New();
  assert(NULL != P->callbackAxial2);

  P->callbackCoronal1 = AlignedLineCallback::New();
  assert(NULL != P->callbackCoronal1);

  P->callbackCoronal2 = AlignedLineCallback::New();
  assert(NULL != P->callbackCoronal2);

  P->callbackSagittal1 = AlignedLineCallback::New();
  assert(NULL != P->callbackSagittal1);

  P->callbackSagittal2 = AlignedLineCallback::New();
  assert(NULL != P->callbackSagittal2);

  P->callbackViewpointChange = AllAlignedLinesCallback::New();
  assert(NULL != P->callbackViewpointChange);

  P->planeAxial = VTKCreateSlicingPlaneData(vec_y, origin, NULL);
  assert(NULL != P->planeAxial);

  P->planeCoronal = VTKCreateSlicingPlaneData(vec_z, origin, NULL);
  assert(NULL != P->planeCoronal);

  P->planeSagittal = VTKCreateSlicingPlaneData(vec_x, origin, NULL);
  assert(NULL != P->planeSagittal);

  P->callbackAxial = SlicingPlaneCallback::New();
  assert(NULL != P->callbackAxial);

  P->callbackCoronal = SlicingPlaneCallback::New();
  assert(NULL != P->callbackCoronal);

  P->callbackSagittal = SlicingPlaneCallback::New();
  assert(NULL != P->callbackSagittal);

  P->slicingStatistics = vtkTextActor::New();
  assert(NULL != P->slicingStatistics);

  P->renWin = vtkRenderWindow::New();
  assert(NULL != P->renWin);

  P->renWinInt = vtkRenderWindowInteractor::New();
  assert(NULL != P->renWinInt);

  P->renWinIntStyle = CustomInteractorStyle::New();
  assert(NULL != P->renWinIntStyle);

  P->pushCallback = vtkCallbackCommand::New();
  assert(NULL != P->pushCallback);

  P->popCallback = vtkCallbackCommand::New();
  assert(NULL != P->popCallback);

  P->keypressCallback = vtkCallbackCommand::New();
  assert(NULL != P->keypressCallback);

  if ( (NULL == P->ren3D) ||
       (NULL == P->renTop) ||
       (NULL == P->renFront) ||
       (NULL == P->renSide) ||
       (NULL == P->sldThr ) ||
       (NULL == P->sldThrRep) ||
       (NULL == P->sldThrCallback) ||
       (NULL == P->planeAxial1) ||
       (NULL == P->planeAxial2) ||
       (NULL == P->planeCoronal1) ||
       (NULL == P->planeCoronal2) ||
       (NULL == P->planeSagittal1) ||
       (NULL == P->planeSagittal2) ||
       (NULL == P->representationAxial1) ||
       (NULL == P->representationAxial2) ||
       (NULL == P->representationCoronal1) ||
       (NULL == P->representationCoronal2) ||
       (NULL == P->representationSagittal1) ||
       (NULL == P->representationSagittal2) ||
       (NULL == P->callbackAxial1) ||
       (NULL == P->callbackAxial2) ||
       (NULL == P->callbackCoronal1) ||
       (NULL == P->callbackCoronal2) ||
       (NULL == P->callbackSagittal1) ||
       (NULL == P->callbackSagittal2) ||
       (NULL == P->callbackViewpointChange) ||
       (NULL == P->planeAxial) ||
       (NULL == P->planeCoronal) ||
       (NULL == P->planeSagittal) ||
       (NULL == P->callbackAxial) ||
       (NULL == P->callbackCoronal) ||
       (NULL == P->callbackSagittal) ||
       (NULL == P->slicingStatistics) ||
       (NULL == P->renWin) ||
       (NULL == P->renWinInt) ||
       (NULL == P->renWinIntStyle) ||
       (NULL == P->pushCallback) ||
       (NULL == P->popCallback) ||
       (NULL == P->keypressCallback)
       )
    {
      VTKCloseDisplayWindow( P );
      return NULL;
    }
  /* if */

  /* To avoid any possible issues with multiple threads we add a callback routine
     to the renderer. Actors that are intended to be rendered and displayed must be
     completely prepared in advance and are then pushed to the display thread.
     The callback function VTKActorPushCallback collects them and adds them to the
     renderer. As it is run from the same thread as the renderer and the window
     interactior there is no possiblity for simultaneous access to VTK objects
     from multiple threads.
  */
  P->pushCallback->SetCallback( VTKActorPushCallback );
  P->pushCallback->SetClientData( data );

  P->popCallback->SetCallback( VTKActorPopCallback );
  P->popCallback->SetClientData( data );

  /* Add callback hook to control keyboard interaction. */
  P->keypressCallback->SetCallback( VTKKeypressCallback );
  P->keypressCallback->SetClientData( data );

  double const border_x = 0.7;

  P->ren3D->SetBackground(0.05, 0.05 ,0.05);
  P->ren3D->AddObserver( vtkCommand::StartEvent, P->pushCallback );
  P->ren3D->AddObserver( vtkCommand::StartEvent, P->popCallback );
  P->ren3D->SetViewport(0.0, 0.0, border_x, 1.0);

  P->renTop->SetBackground(0.05, 0.05 ,0.05);
  P->renTop->SetViewport(0.711, 0.674, 1.0, 1.000);

  bool const top = VTKSetOrthographicProjectionCamera(P->renTop->GetActiveCamera(), vec_y, vec_z, NULL, NULL, BATCHACQUISITION_qNaN_dv);
  assert(true == top);

  P->renFront->SetBackground(0.05, 0.05 ,0.05);
  P->renFront->SetViewport(0.711, 0.337, 1.0, 0.663);

  bool const front = VTKSetOrthographicProjectionCamera(P->renFront->GetActiveCamera(), vec_z, vec_ny, NULL, NULL, BATCHACQUISITION_qNaN_dv);
  assert(true == front);

  P->renSide->SetBackground(0.05, 0.05 ,0.05);
  P->renSide->SetViewport(0.711, 0.000, 1.0, 0.326);

  bool const side = VTKSetOrthographicProjectionCamera(P->renSide->GetActiveCamera(), vec_nx, vec_ny, NULL, NULL, BATCHACQUISITION_qNaN_dv);
  assert(true == side);

  P->renWin->AddRenderer(P->ren3D);
  P->renWin->AddRenderer(P->renTop);
  P->renWin->AddRenderer(P->renFront);
  P->renWin->AddRenderer(P->renSide);

  P->renWin->SetSize(sx, sy);

  P->renWinIntStyle->limit_to_2D = false;
  P->renWinIntStyle->border_x = border_x;

  P->renWinInt->SetRenderWindow(P->renWin);
  P->renWinInt->SetStillUpdateRate( 5 );
  P->renWinInt->SetDesiredUpdateRate( 15 );
  P->renWinInt->SetInteractorStyle( P->renWinIntStyle );
  P->renWinInt->AddObserver(vtkCommand::KeyPressEvent, P->keypressCallback);

  VTKUpdateThresholdSliderWidget_inline(NULL, P);
  P->sldThrRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
  P->sldThrRep->GetPoint1Coordinate()->SetValue(0.1, 0.1);
  P->sldThrRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
  P->sldThrRep->GetPoint2Coordinate()->SetValue(0.9, 0.1);

  P->sldThr->SetInteractor(P->renWinInt);
  P->sldThr->SetRepresentation(P->sldThrRep);
  //P->sldThr->SetAnimationModeToAnimate();
  P->sldThr->EnabledOn();
  P->sldThr->AddObserver(vtkCommand::InteractionEvent, P->sldThrCallback);

  P->planeAxial1->SetInteractor(P->renWinInt);
  P->planeAxial1->SetDefaultRenderer(P->renFront);
  P->planeAxial1->SetRepresentation(P->representationAxial1);
  P->representationAxial1->SetLineColor(0, 1, 0); // Axial plane is green.

  P->planeAxial2->SetInteractor(P->renWinInt);
  P->planeAxial2->SetDefaultRenderer(P->renSide);
  P->planeAxial2->SetRepresentation(P->representationAxial2);
  P->representationAxial2->SetLineColor(0, 1, 0); // Axial plane is green.

  P->callbackAxial1->L = P->representationAxial2;
  P->planeAxial1->AddObserver(vtkCommand::InteractionEvent, P->callbackAxial1);

  P->callbackAxial2->L = P->representationAxial1;
  P->planeAxial2->AddObserver(vtkCommand::InteractionEvent, P->callbackAxial2);

  P->planeCoronal1->SetInteractor(P->renWinInt);
  P->planeCoronal1->SetDefaultRenderer(P->renTop);
  P->planeCoronal1->SetRepresentation(P->representationCoronal1);
  P->representationCoronal1->SetLineColor(0, 0, 1); // Coronal plane is blue.

  P->planeCoronal2->SetInteractor(P->renWinInt);
  P->planeCoronal2->SetDefaultRenderer(P->renSide);
  P->planeCoronal2->SetRepresentation(P->representationCoronal2);
  P->representationCoronal2->SetLineColor(0, 0, 1); // Coronal plane is blue.

  P->callbackCoronal1->L = P->representationCoronal2;
  P->planeCoronal1->AddObserver(vtkCommand::InteractionEvent, P->callbackCoronal1);

  P->callbackCoronal2->L = P->representationCoronal1;
  P->planeCoronal2->AddObserver(vtkCommand::InteractionEvent, P->callbackCoronal2);

  P->planeSagittal1->SetInteractor(P->renWinInt);
  P->planeSagittal1->SetDefaultRenderer(P->renTop);
  P->planeSagittal1->SetRepresentation(P->representationSagittal1);
  P->representationSagittal1->SetLineColor(1, 0, 0); // Sagittal plane is red.

  P->planeSagittal2->SetInteractor(P->renWinInt);
  P->planeSagittal2->SetDefaultRenderer(P->renFront);
  P->planeSagittal2->SetRepresentation(P->representationSagittal2);
  P->representationSagittal2->SetLineColor(1, 0, 0); // Sagittal plane is red.

  P->callbackSagittal1->L = P->representationSagittal2;
  P->planeSagittal1->AddObserver(vtkCommand::InteractionEvent, P->callbackSagittal1);

  P->callbackSagittal2->L = P->representationSagittal1;
  P->planeSagittal2->AddObserver(vtkCommand::InteractionEvent, P->callbackSagittal2);

  P->callbackViewpointChange->W = P;
  P->renWin->AddObserver(vtkCommand::StartEvent, P->callbackViewpointChange);

  P->planeAxial->Actor->GetProperty()->SetColor(0, 1, 0); // Axial plane is green.
  bool const add_axial = VTKAddActorToRenderer_inline(P->ren3D, P->planeAxial->Actor);
  assert(true == add_axial);

  P->planeCoronal->Actor->GetProperty()->SetColor(0, 0, 1); // Coronal plane is blue.
  bool const add_coronal = VTKAddActorToRenderer_inline(P->ren3D, P->planeCoronal->Actor);
  assert(true == add_coronal);

  P->planeSagittal->Actor->GetProperty()->SetColor(1, 0, 0); // Sagittal plane is red.
  bool const add_sagittal = VTKAddActorToRenderer_inline(P->ren3D, P->planeSagittal->Actor);
  assert(true == add_sagittal);

  P->callbackAxial->P = P->planeAxial;
  P->representationAxial1->AddObserver(vtkCommand::UserEvent, P->callbackAxial);

  P->callbackCoronal->P = P->planeCoronal;
  P->representationCoronal1->AddObserver(vtkCommand::UserEvent, P->callbackCoronal);

  P->callbackSagittal->P = P->planeSagittal;
  P->representationSagittal1->AddObserver(vtkCommand::UserEvent, P->callbackSagittal);

  P->slicingStatistics->SetInput(gMsgNoDataAvailable);
  P->slicingStatistics->GetActualPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  P->slicingStatistics->SetPosition(0.01, 0.99);
  P->slicingStatistics->GetTextProperty()->SetColor(1, 1, 1);
  P->slicingStatistics->GetTextProperty()->SetJustificationToLeft();
  P->slicingStatistics->GetTextProperty()->SetVerticalJustificationToTop();
  P->slicingStatistics->GetTextProperty()->SetFontSize(18);

  bool const add_text = VTKAddActorToRenderer_inline(P->ren3D, P->slicingStatistics);
  assert(true == add_text);

  P->renWinInt->Initialize();

  P->planeCoronal1->On();
  P->planeSagittal1->On();

  P->planeAxial1->On();
  P->planeSagittal2->On();

  P->planeAxial2->On();
  P->planeCoronal2->On();

  return( P );
}
/* VTKOpenDisplayWindow */



/****** DISPLAY THREAD ******/


//! VTK display worker thread.
/*!
  This function implements the VTK display worker thread.

  \param parameter      Pointer to the worker thread data structure.
  \return Function returns error code.
*/
unsigned int
__stdcall
VTKDisplayThread(
                 void * parameter
                 )
{
  assert(NULL != parameter);
  if (NULL == parameter) return EXIT_FAILURE;

  SetThreadNameForMSVC(-1, "VTKDisplayThread");

  VTKdisplaythreaddata * const D = (VTKdisplaythreaddata *)(parameter);
  assert(D == D->myAddress);

  assert(NULL == D->window);

  assert( (NULL != D->point_clouds) && (1 <= D->point_clouds->size()) );
  assert( (NULL != D->point_cloudsNEW) && (1 <= D->point_cloudsNEW->size()) );

  assert( (NULL != D->projector_geometries) && (1 <= D->projector_geometries->size()) );
  assert( (NULL != D->projector_geometriesNEW) && (1 <= D->projector_geometriesNEW->size()) );

  assert( (NULL != D->camera_geometries) && (1 <= D->camera_geometries->size()) );
  assert( (NULL != D->camera_geometriesNEW) && (1 <= D->camera_geometriesNEW->size()) );

  assert(false == D->camera_pushed);
  assert(false == D->point_cloud_pushed);
  assert(false == D->projector_geometry_pushed);
  assert(false == D->camera_geometry_pushed);

  /* Open display window. */
  D->window = VTKOpenDisplayWindow(
                                   768, // X size.
                                   768, // Y size.
                                   D // Default thread datastore.
                                   );
  assert(NULL != D->window);

  if (NULL == D->window) return EXIT_FAILURE;

  unsigned int result = EXIT_SUCCESS;

  /* Add default actors. */
  if (NULL != D->point_clouds)
    {
      int const CloudID = D->CloudID;
      int const n = (int)( D->point_clouds->size() );
      if ( (0 <= CloudID) && (CloudID < n) )
        {
          VTKpointclouddata_ * points = ( *(D->point_clouds) )[CloudID];
          if (NULL != points)
            {
              bool const add = VTKAddActorToDisplayWindow(D->window, points->Actor);
              assert(true == add);
              if (true != add) result = EXIT_FAILURE;
            }
          /* if */
        }
      /* if */
    }
  /* if */

  if (NULL != D->camera_geometries)
    {
      int const CameraID = D->CameraID;
      int const n = (int)( D->camera_geometries->size() );
      if ( (0 <= CameraID) && (CameraID < n) )
        {
          ProjectiveGeometry_ * const geometry = ( *(D->camera_geometries) )[CameraID];
          VTKChangeCameraGeometry_inline(D, geometry, false);
        }
      /* if */
    }
  /* if */

  /* Enable callbacks routines. */
  D->window->sldThrCallback->D = D;

  /* Update plane widgets. */
  VTKUpdateAllPlaneWidgets_inline( D );

  /* Get and store thread ID. */
  D->threadID = GetCurrentThreadId();

  /* Disable close button. */
  BOOL const disable_close = EnumThreadWindows(D->threadID, VTKDisableCloseCommandHelper, 0);
  assert(0 != disable_close);

  /* Set window title. */
  D->window->renWin->SetWindowName(gMsgWindowTitleNoData);

  /* Start window interactor. */
  EnterCriticalSection( &(D->window->rendererCS) );
  {
    D->window->interactorRunning = true;
    D->window->renWinInt->Start();
    D->window->interactorRunning = false;
  }
  LeaveCriticalSection( &(D->window->rendererCS) );

  /* Remove default actors. */
  if (NULL != D->point_clouds)
    {
      EnterCriticalSection( &(D->dataCS) );
      {
        int const n = (int)( D->point_clouds->size() );
        for (int i = 0; i < n; ++i)
          {
            VTKpointclouddata_ * points = ( *(D->point_clouds) )[i];
            if (NULL != points)
              {
                bool const remove = VTKRemovePointCloudFromDisplayWindow_inline(D->window, points);
                assert(true == remove);
                if (true != remove) result = EXIT_FAILURE;

                VTKDeletePointCloudData( points );
                points = NULL;
                ( *(D->point_clouds) )[i] = NULL;
              }
            /* if */
          }
        /* for */
      }
      LeaveCriticalSection( &(D->dataCS) );
    }
  /* if */

  if (NULL != D->point_cloudsNEW)
    {
      EnterCriticalSection( &(D->pushCS) );
      {
        int const n = (int)( D->point_cloudsNEW->size() );
        for (int i = 0; i < n; ++i)
          {
            VTKpointclouddata_ * pointsNEW = ( *(D->point_cloudsNEW) )[i];
            if (NULL != pointsNEW)
              {
                VTKDeletePointCloudData( pointsNEW );
                pointsNEW = NULL;
                ( *(D->point_cloudsNEW) )[i] = NULL;
              }
            /* if */
          }
        /* for */
      }
      LeaveCriticalSection( &(D->pushCS) );
    }
  /* if */

  /* Delete window data. */
  VTKCloseDisplayWindow(D->window);
  D->window = NULL;

  return( (unsigned int)( result ) );
}
/* VTKDisplayThread */



/****** OPEN AND CLOSE WINDOW ******/

//! Open VTK display thread.
/*!
  Function opens VTK display window and associated thread.
  Once the thread is spawned it will execute the VTK window interactor in the trackball mouse mode.

  \param points  A pointer to a matrix containing coordinates of 3D point cloud points.
  The matrix must have three columns that are x, y, and z coordinate. Number of rows is equal to total number of points.
  \param camera Camera geometry class.
  \param projector Projector geometry class.
  \return Function returns pointer to the thread data-store if successfull and NULL otherwise.
  The pointer may be used to add and remove actors from the scene.
*/
VTKdisplaythreaddata *
OpenVTKWindow(
              cv::Mat * const points,
              ProjectiveGeometry * const camera,
              ProjectiveGeometry * const projector
              )
{
  VTKdisplaythreaddata * const P = VTKCreateDisplayThreadData_inline();
  assert(NULL != P);
  if (NULL == P) return NULL;

  /* Try to create outline and point cloud actors. */
  if (NULL != points)
    {
      if ( (NULL != P->point_clouds) && (1 <= P->point_clouds->size()) )
        {
          P->CloudID = 0;
          assert( NULL == ( *(P->point_clouds) )[0] );
          ( *(P->point_clouds) )[0] = VTKCreatePointCloudData(points, NULL, NULL, -1, -1, NULL);
          assert( NULL != ( *(P->point_clouds) )[0] );
        }
      /* if */
    }
  /* if */

  /* Create default camera and projector views. */
  if (NULL != projector)
    {
      if ( (NULL != P->projector_geometries) && (1 <= P->projector_geometries->size()) )
        {
          P->ProjectorID = 0;
          assert( NULL == ( *(P->projector_geometries) )[0] );
          ( *(P->projector_geometries) )[0] = new ProjectiveGeometry_(*projector);
          assert( NULL != ( *(P->projector_geometries) )[0] );
        }
      /* if */
    }
  /* if */

  if (NULL != camera)
    {
      if ( (NULL != P->camera_geometries) && (1 <= P->camera_geometries->size()) )
        {
          P->CameraID = 0;
          assert( NULL == ( *(P->camera_geometries) )[0] );
          ( *(P->camera_geometries) )[0] = new ProjectiveGeometry_(*camera);
          assert( NULL != ( *(P->camera_geometries) )[0] );
        }
      /* if */
    }
  /* if */

  /* Spawn display window thread. */
  P->thread = (HANDLE)( _beginthreadex(
                                       NULL, // No security atributes.
                                       0, // Automatic stack size.
                                       VTKDisplayThread,
                                       (void *)( P ),
                                       0, // Thread starts immediately.
                                       NULL // Thread identifier not used.
                                       )
                        );
  assert( (HANDLE)( NULL ) != P->thread );

  return( P );
}
/* OpenVTKWindow */



//! Closes VTK display winow.
/*!
  Waits for display thread to terminate. Wait is blocking meaning this
  function will return only when VTK visualization thread terminates.
  To force termination set autoterminate flag to true.

  \param P      Pointer to thread data.
*/
void
CloseVTKWindow(
               VTKdisplaythreaddata * const P
               )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Set termination flag. */
  P->terminate = true;
  SleepEx(5, TRUE);

  /* Force the window to call the update callback. */
  if (true == P->terminate) VTKUpdateDisplay(P);

  /* Wait for the VTK window to be closed. */
  DWORD exitcode = EXIT_SUCCESS;
  if ( (HANDLE)( NULL ) != P->thread )
    {
      DWORD const VTKwait = WaitForSingleObjectEx(P->thread, INFINITE, TRUE);
      assert(WAIT_OBJECT_0 == VTKwait);

      if (WAIT_OBJECT_0 == VTKwait)
        {
          BOOL const VTKexitcodeget = GetExitCodeThread(P->thread, &exitcode);
          assert(0 != VTKexitcodeget);
          assert(EXIT_SUCCESS == exitcode);

          CloseHandle(P->thread);
          P->thread = (HANDLE)( NULL );
        }
      /* if */
    }
  /* if */

  assert( NULL == P->window );

  assert( (HANDLE)( NULL ) == P->thread );

  VTKDestroyDisplayThreadData_inline( P );

}
/* CloseVTKWindow */



/****** DATA PUSH ******/

//! Push point cloud to display thread.
/*!
  Function pushes point cloud to display thread. Only one point cloud
  may be pushed at a time. If more then one point cloud is pushed and
  the display thread does not call the required callback function
  in time only the last pushed cloud will be displayed.

  \see VTKCreatePointCloudData

  \param P      Pointer to display thread data.
  \param points    Pointer to 3D point coordinates.
  \param colors    Pointer to point colors; may be NULL.
  \param data      Pointer to additional data; may be NULL.
  \param CameraID  ID of the camera used to acquire point cloud.
  \param ProjectorID ID of the projector used to acquire point cloud.
  \param name   Name of the current acquisition. May be NULL.
  \return Function returns true if successfull.
*/
bool
VTKPushPointCloudToDisplayThread(
                                 VTKdisplaythreaddata * const P,
                                 cv::Mat * const points,
                                 cv::Mat * const colors,
                                 cv::Mat * const data,
                                 int const CameraID,
                                 int const ProjectorID,
                                 wchar_t const * const name
                                 )
{
  bool point_cloud_pushed = false;

  /* Check if VTK thread is running. */
  bool const is_running = isVTKthreadrunning_inline( P );
  assert(true == is_running);
  if (true != is_running) return point_cloud_pushed;

  /* Then create a point cloud data. */
  VTKpointclouddata * const VTKpoints = VTKCreatePointCloudData(points, colors, data, CameraID, ProjectorID, name);
  //assert(NULL != VTKpoints);
  if (NULL == VTKpoints) return point_cloud_pushed;

  /* Push the created VTK data to the display thread.
     As concurrent access is not allowed we block until the resource is free.
  */
  EnterCriticalSection( &(P->pushCS) );
  {
    // Resize arrays if needed.
    VTKResizeDisplayThreadData_inline(P, (size_t)(CameraID + 1), true, false, false);

    if ( (NULL != P->point_cloudsNEW) && (0 <= CameraID) && (CameraID < (int)(P->point_cloudsNEW->size())) )
      {
        VTKpointclouddata_ * pointsNEW = ( *(P->point_cloudsNEW) )[CameraID];
        assert(NULL == pointsNEW);
        if (NULL != pointsNEW)
          {
            VTKDeletePointCloudData( pointsNEW );
            pointsNEW = NULL;
          }
        /* if */

        pointsNEW = VTKpoints;
        P->point_cloud_pushed = P->point_cloud_pushed || (NULL != pointsNEW);

        ( *(P->point_cloudsNEW) )[CameraID] = pointsNEW;

        point_cloud_pushed = true;
      }
    /* if */
  }
  LeaveCriticalSection( &(P->pushCS) );

  if (false == point_cloud_pushed)
    {
      VTKDeletePointCloudData( VTKpoints );
    }
  /* if */

  return point_cloud_pushed;
}
/* VTKPushPointCloudToDisplayThread */



//! Push camera geometry to display thread.
/*!
  Function pushes camera geometry data to the display thread.
  Only one set can be pushed at a time.

  \param P      Pointer to display thread data.
  \param G      Pointer to new camera geometry.
  \param CameraID  ID of the camera.
  \return Function returns true if successfull.
*/
bool
VTKPushCameraGeometryToDisplayThread(
                                     VTKdisplaythreaddata * const P,
                                     ProjectiveGeometry * const G,
                                     int const CameraID
                                     )
{
  bool geometry_pushed = false;

  /* Check if VTK thread is running. */
  bool const is_running = isVTKthreadrunning_inline( P );
  assert(true == is_running);
  if (true != is_running) return geometry_pushed;

  assert(NULL != G);
  if (NULL == G) return geometry_pushed;

  /* Create copy of supplied geometry. */
  ProjectiveGeometry_ * geometry = new ProjectiveGeometry_( *G );
  assert(NULL != geometry);
  if (NULL == geometry) return geometry_pushed;

  /* Push crated geometry to the display thread.
     As concurrent access is not allowed we block until the resource is free.
  */
  EnterCriticalSection( &(P->pushCS) );
  {
    // Resize arrays if needed.
    VTKResizeDisplayThreadData_inline(P, (size_t)(CameraID + 1), false, false, true);

    if ( (NULL != P->camera_geometriesNEW) && (0 <= CameraID) && (CameraID < (int)(P->camera_geometriesNEW->size())) )
      {
        ProjectiveGeometry_ * geometryNEW = ( *(P->camera_geometriesNEW) )[CameraID];
        assert(NULL == geometryNEW);
        SAFE_DELETE( geometryNEW );

        geometryNEW = geometry;
        P->camera_geometry_pushed = P->camera_geometry_pushed || (NULL != geometryNEW);

        ( *(P->camera_geometriesNEW) )[CameraID] = geometryNEW;

        geometry_pushed = true;
      }
    /* if */
  }
  LeaveCriticalSection( &(P->pushCS) );

  if (false == geometry_pushed)
    {
      SAFE_DELETE( geometry );
    }
  /* if */

  return geometry_pushed;
}
/* VTKPushCameraGeometryToDisplayThread */



//! Push projector geometry to display thread.
/*!
  Function pushes projector geometry data to the display thread.
  Only one set can be pushed at a time.

  \param P      Pointer to display thread data.
  \param G      Pointer to new projector geometry.
  \param ProjectorID  ID of the projector.
  \return Function returns true if successfull.
*/
bool
VTKPushProjectorGeometryToDisplayThread(
                                        VTKdisplaythreaddata * const P,
                                        ProjectiveGeometry * const G,
                                        int const ProjectorID
                                        )
{
  bool geometry_pushed = false;

  /* Check if VTK thread is running. */
  bool const is_running = isVTKthreadrunning_inline( P );
  assert(true == is_running);
  if (true != is_running) return geometry_pushed;

  assert(NULL != G);
  if (NULL == G) return geometry_pushed;

  /* Create copy of supplied geometry. */
  ProjectiveGeometry_ * geometry = new ProjectiveGeometry_( *G );
  assert(NULL != geometry);
  if (NULL == geometry) return geometry_pushed;

  /* Push crated geometry to the display thread.
     As concurrent access is not allowed we block until the resource is free.
  */
  EnterCriticalSection( &(P->pushCS) );
  {
    // Resize arrays if needed.
    VTKResizeDisplayThreadData_inline(P, (size_t)(ProjectorID + 1), false, true, false);

    if ( (NULL != P->projector_geometriesNEW) && (0 <= ProjectorID) && (ProjectorID < (int)(P->projector_geometriesNEW->size())) )
      {
        ProjectiveGeometry_ * geometryNEW = ( *(P->projector_geometriesNEW) )[ProjectorID];
        assert(NULL == geometryNEW);
        SAFE_DELETE(geometryNEW);

        geometryNEW = geometry;
        P->projector_geometry_pushed = P->projector_geometry_pushed || (NULL != geometryNEW);

        ( *(P->projector_geometriesNEW) )[ProjectorID] = geometryNEW;

        geometry_pushed = true;
      }
    /* if */
  }
  LeaveCriticalSection( &(P->pushCS) );

  if (false == geometry_pushed)
    {
      SAFE_DELETE( geometry );
    }
  /* if */

  return geometry_pushed;
}
/* VTKPushProjectorGeometryToDisplayThread */



//! Clear all pushed data.
/*!
  Function pops all data from the display thread.

  \param P   Pointer to display thread data.
  \return Function returns true if successfull.
*/
bool
VTKClearAllPushedData(
                      VTKdisplaythreaddata * const P
                      )
{
  bool clear_all = false;

  /* Check if VTK thread is running. */
  bool const is_running = isVTKthreadrunning_inline( P );
  assert(true == is_running);
  if (true != is_running) return clear_all;

  clear_all = true;
  P->clear_all = clear_all;

  VTKUpdateDisplay(P);

  return clear_all;
}
/* VTKClearAllPushedData */



//! Update VTK display.
/*!
  To force the redraw we must redraw message to VTK thread.

  \param P     Pointer to VTK thread data.
*/
void
VTKUpdateDisplay(
                 VTKdisplaythreaddata * const P
                 )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Post repaint messages to thread windows. */
  if (0 != P->threadID)
    {
      BOOL const result = EnumThreadWindows(P->threadID, VTKUpdateDisplayHelper, 0);
      assert(0 != result);
    }
  /* if */
}
/* VTKUpdateDisplay */



#endif /* !__BATCHACQUISITIONVTK_CPP */
