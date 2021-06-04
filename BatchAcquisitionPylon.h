/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 * 
 * (c) 2021 UniZG, Zagreb. All rights reserved.
 * (c) 2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionPylon.h
  \brief  Functions for Basler's Pylon SDK.

  \author Tomislav Petkovic.
  \date   2021-05-10
*/


#ifndef __BATCHACQUISITIONPYLON_H
#define __BATCHACQUISITIONPYLON_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionPylonCallbacks.h"

struct AcquisitionParameters_;


//! Parameters of the Pylon camera.
/*!
  All classes and information needed to control a Pylon SDK camera
  are stored in this structure.
*/
typedef
struct AcquisitionParametersPylon_
{

#ifdef HAVE_PYLON_SDK
  // Members if SDK is available go here.
    Pylon::CInstantCamera * pCamera;
    Pylon::CCustomCameraEventHandler * pCameraEventHandler;
    Pylon::CCustomImageEventHandler * pImageEventHandler;

#else
  // Dummy members which take the same amount of memory if SDK is not avalable go here.
    void* pCamera;
    void* pCameraEventHandler;
    void* pImageEventHandler;
#endif /* HAVE_PYLON_SDK */

} AcquisitionParametersPylon;



//! Stop all pending transfers.
bool
AcquisitionParametersPylonStopTransfer(
                                       AcquisitionParametersPylon * const,
                                       double const exposureTime = 5000000.0, // us
                                       int const nFrames = 18
                                       );

//! Start transfer.
bool
AcquisitionParametersPylonStartTransfer(
                                        AcquisitionParametersPylon * const
                                        );

//! Release Pylon SDK classes.
void
AcquisitionParametersPylonRelease(
                                  AcquisitionParametersPylon * const
                                  );

//! Adjust camera exposure time.
bool
AcquisitionParametersPylonAdjustExposureTime(
                                             AcquisitionParametersPylon * const,
                                             int const,
                                             double const,
                                             double * const
                                             );

//! Adjust camera delay and exposure times.
bool
AcquisitionParametersPylonSetExposureAndDelayTimes(
                                                   AcquisitionParametersPylon * const,
                                                   double * const,
                                                   double * const
                                                   );

//! Create Pylon SDK classes.
AcquisitionParametersPylon *
AcquisitionParametersPylonCreate(
                                 AcquisitionParameters_ * const,
                                 int const nFrames = 18,
                                 std::vector<std::wstring *> * const pConnectedCameras = NULL
                                 );

//! Get camera ID.
std::wstring *
AcquisitionParametersPylonGetCameraIdentifier(
                                              AcquisitionParametersPylon * const
                                              );

#ifdef HAVE_PYLON_SDK

//! Get image data type.
ImageDataType
GetImageDataType(
                 int * const
                 );

//! Get pixel format.
void
GetPylonPixelFormat(
                    ImageDataType const
                    );

//! Get Bayer alignment mode.
void
GetPylonBayerAlignmentMode(
                           ImageDataType const
                           );

#endif /* HAVE_PYLON_SDK */



#endif /* __BATCHACQUISITIONPYLON_H */
