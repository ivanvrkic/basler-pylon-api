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
  \file   BatchAcquisitionFlyCapture2.h
  \brief  Functions for PointGrey FlyCapture2 SDK.

  \author Tomislav Petkovic
  \date   2017-01-26
*/


#ifndef __BATCHACQUISITIONFLYCAPTURE2_H
#define __BATCHACQUISITIONFLYCAPTURE2_H


#include "BatchAcquisition.h"


struct AcquisitionParameters_;


//! Parameters of the PointGrey camera.
/*!
  All classes and information needed to control a PointGrey camera via
  FlyCapture2 SDK are stored in this structure.
*/
typedef
struct AcquisitionParametersFlyCapture2_
{

#ifdef HAVE_FLYCAPTURE2_SDK
  FlyCapture2::BusManager * pBusManager; //!< Bus manager class.
  FlyCapture2::PGRGuid * pCameraGUID; //!< Unique identifier of currently connected camera.
  FlyCapture2::Camera * pCamera; //!< Main camera class.
  FlyCapture2::TriggerMode * pTriggerMode; //!< Class to control triggering.
  FlyCapture2::TriggerDelay * pTriggerDelay; //!< Class to control trigger delay.
  FlyCapture2::FC2Config * pConfig; //!< General image acquisition parameters.
#ifdef USE_FLYCAPTURE2_GUI
  FlyCapture2::CameraControlDlg * pControlDialog; //!< Camera control dialog.
#else
  void * pControlDialog; //!< Dummy pointer to keep structure size the same.
#endif /* USE_FLYCAPTURE2_GUI */
#else
  void * pBusManager; //!< Dummy pointer to keep structure size the same.
  void * pCameraGUID; //!< Dummy pointer to keep structure size the same.
  void * pCamera; //!< Dummy pointer to keep structure size the same.
  void * pTriggerMode; //!< Dummy pointer to keep structure size the same.
  void * pTriggerDelay; //!< Dummy pointer to keep structure size the same.
  void * pConfig; //!< Dummy pointer to keep structure size the same.
  void * pControlDialog; //!< Dummy pointer to keep structure size the same.
#endif /* HAVE_FLYCAPTURE2_SDK */

  void * pAcquisitionThread; //!< Pointer to acquisition thread data.

} AcquisitionParametersFlyCapture2;


//! Open camera control dialog.
bool
AcquisitionParametersFlyCapture2ControlDialogOpen(
                                                  AcquisitionParametersFlyCapture2 * const,
                                                  int const
                                                  );

//! Close camera control dialog.
bool
AcquisitionParametersFlyCapture2ControlDialogClose(
                                                   AcquisitionParametersFlyCapture2 * const,
                                                   int const
                                                   );

//! Toggle state of camera control dialog.
bool
AcquisitionParametersFlyCapture2ControlDialogToggle(
                                                    AcquisitionParametersFlyCapture2 * const,
                                                    int const
                                                    );

//! Stop all pending transfers.
bool
AcquisitionParametersFlyCapture2StopTransfer(
                                             AcquisitionParametersFlyCapture2 * const,
                                             double const exposureTime = 5000000.0, // us
                                             int const nFrames = 18
                                             );

//! Start transfer.
bool
AcquisitionParametersFlyCapture2StartTransfer(
                                              AcquisitionParametersFlyCapture2 * const
                                              );

//! Release FlyCapture2 SDK classes.
void
AcquisitionParametersFlyCapture2Release(
                                        AcquisitionParametersFlyCapture2 * const
                                        );

//! Adjust camera exposure time.
bool
AcquisitionParametersFlyCapture2AdjustExposureTime(
                                                   AcquisitionParametersFlyCapture2 * const,
                                                   int const,
                                                   double const,
                                                   double * const
                                                   );

//! Adjust camera delay and exposure times.
bool
AcquisitionParametersFlyCapture2SetExposureAndDelayTimes(
                                                         AcquisitionParametersFlyCapture2 * const,
                                                         double * const,
                                                         double * const
                                                         );

//! Create FlyCapture2 SDK classes.
AcquisitionParametersFlyCapture2 *
AcquisitionParametersFlyCapture2Create(
                                       AcquisitionParameters_ * const,
                                       int const nFrames = 18,
                                       std::vector<std::wstring *> * const pConnectedCameras = NULL
                                       );

//! Get camera ID.
std::wstring *
AcquisitionParametersFlyCapture2GetCameraIdentifier(
                                                    AcquisitionParametersFlyCapture2 * const
                                                    );


#ifdef HAVE_FLYCAPTURE2_SDK

//! Get image data type.
ImageDataType
GetImageDataType(
                 FlyCapture2::Image * const,
                 FlyCapture2::Camera * const
                 );

//! Get pixel format.
FlyCapture2::PixelFormat
GetFlyCapture2PixelFormat(
                          ImageDataType const
                          );

//! Get Bayer format.
FlyCapture2::BayerTileFormat
GetFlyCapture2BayerTileFormat(
                              ImageDataType const
                              );

#endif /* HAVE_FLYCAPTURE2_SDK */



#endif /* __BATCHACQUISITIONFLYCAPTURE2_H */
