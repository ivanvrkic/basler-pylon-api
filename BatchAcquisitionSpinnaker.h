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
  \file   BatchAcquisitionSpinnaker.h
  \brief  Functions for Flir's Spinnaker SDK.

  \author Tomislav Petkovic
  \date   2021-05-10
*/


#ifndef __BATCHACQUISITIONSPINNAKER_H
#define __BATCHACQUISITIONSPINNAKER_H


#include "BatchAcquisition.h"


struct AcquisitionParameters_;


//! Parameters of the Spinnaker camera.
/*!
  All classes and information needed to control a Spinnaker SDK camera
  are stored in this structure.
*/
typedef
struct AcquisitionParametersSpinnaker_
{

#ifdef HAVE_SPINNAKER_SDK
  // Members if SDK is available go here.
#else
  // Dummy members which take the same amount of memory if SDK is not avalable go here.
#endif /* HAVE_SPINNAKER_SDK */

} AcquisitionParametersSpinnaker;



//! Stop all pending transfers.
bool
AcquisitionParametersSpinnakerStopTransfer(
                                           AcquisitionParametersSpinnaker * const,
                                           double const exposureTime = 5000000.0, // us
                                           int const nFrames = 18
                                           );

//! Start transfer.
bool
AcquisitionParametersSpinnakerStartTransfer(
                                            AcquisitionParametersSpinnaker * const
                                            );

//! Release Spinnaker SDK classes.
void
AcquisitionParametersSpinnakerRelease(
                                      AcquisitionParametersSpinnaker * const
                                      );

//! Adjust camera exposure time.
bool
AcquisitionParametersSpinnakerAdjustExposureTime(
                                                 AcquisitionParametersSpinnaker * const,
                                                 int const,
                                                 double const,
                                                 double * const
                                                 );

//! Adjust camera delay and exposure times.
bool
AcquisitionParametersSpinnakerSetExposureAndDelayTimes(
                                                       AcquisitionParametersSpinnaker * const,
                                                       double * const,
                                                       double * const
                                                       );

//! Create Spinnaker SDK classes.
AcquisitionParametersSpinnaker *
AcquisitionParametersSpinnakerCreate(
                                     AcquisitionParameters_ * const,
                                     int const nFrames = 18,
                                     std::vector<std::wstring *> * const pConnectedCameras = NULL
                                     );

//! Get camera ID.
std::wstring *
AcquisitionParametersSpinnakerGetCameraIdentifier(
                                                  AcquisitionParametersSpinnaker * const
                                                  );

#ifdef HAVE_SPINNAKER_SDK

//! Get image data type.
ImageDataType
GetImageDataType(
                 void
                 );

//! Get pixel format.
void
GetSpinnakerPixelFormat(
                        ImageDataType const
                        );

//! Get Bayer alignment mode.
void
GetSpinnakerBayerAlignmentMode(
                               ImageDataType const
                               );

#endif /* HAVE_SPINNAKER_SDK */



#endif /* __BATCHACQUISITIONSPINNAKER_H */
