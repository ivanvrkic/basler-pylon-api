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
  \file   BatchAcquisitionSapera.h
  \brief  Functions for Teledyne-Dalsa Sapera SDK.

  \author Tomislav Petkovic
  \date   2017-01-26
*/


#ifndef __BATCHACQUISITIONSAPERA_H
#define __BATCHACQUISITIONSAPERA_H


#include "BatchAcquisition.h"


#ifndef CORSERVER_MAX_STRLEN
//! Maximum length of image acquisition server description string (taken from Sapera documentation/examples).
#define CORSERVER_MAX_STRLEN 30
#endif /* CORSERVER_MAX_STRLEN */


struct AcquisitionParameters_;


//! Parameters of the Sapera camera.
/*!
  All classes and information needed to control a Teledyne Dalsa Sapera SDK camera
  are stored in this structure.
*/
typedef
struct AcquisitionParametersSapera_
{
  int selectedServer; //!< Selected server.
  char selectedServerName[CORSERVER_MAX_STRLEN]; //!< Selected server name.

  int idxTriggerSoftware; //!< Index of GenICam software trigger execute node.
  int idxExposureAlignment; //!< Index of GenICam exposure alignment node.

#ifdef HAVE_SAPERA_SDK
  SapAcqDevice * pCamera; //!< Sapera LT camera class.
  SapFeature * pFeature; //!< Sapera LT camera feature class.
  SapBuffer * pBuffer; //!< Sapera LT image buffer.
  SapBayer * pBayer; //!< Sapera LT Bayer conversion.
  SapAcqDeviceToBuf * pTransfer; //!< Sapera LT data transfer class.
#else
  void * pCamera;
  void * pFeature;
  void * pBuffer;
  void * pBayer;
  void * pTransfer;
#endif /* HAVE_SAPERA_SDK */

} AcquisitionParametersSapera;



//! Stop all pending transfers.
bool
AcquisitionParametersSaperaStopTransfer(
                                        AcquisitionParametersSapera * const,
                                        double const exposureTime = 5000000.0, // us
                                        int const nFrames = 18
                                        );

//! Start transfer.
bool
AcquisitionParametersSaperaStartTransfer(
                                         AcquisitionParametersSapera * const
                                         );

//! Release Sapera SDK classes.
void
AcquisitionParametersSaperaRelease(
                                   AcquisitionParametersSapera * const
                                   );

//! Adjust camera exposure time.
bool
AcquisitionParametersSaperaAdjustExposureTime(
                                              AcquisitionParametersSapera * const,
                                              int const,
                                              double const,
                                              double * const
                                              );

//! Adjust camera delay and exposure times.
bool
AcquisitionParametersSaperaSetExposureAndDelayTimes(
                                                    AcquisitionParametersSapera * const,
                                                    double * const,
                                                    double * const
                                                    );

//! Create Sapera SDK classes.
AcquisitionParametersSapera *
AcquisitionParametersSaperaCreate(
                                  AcquisitionParameters_ * const,
                                  int const nFrames = 18,
                                  std::vector<std::wstring *> * const pConnectedCameras = NULL
                                  );

//! Get camera ID.
std::wstring *
AcquisitionParametersSaperaGetCameraIdentifier(
                                               AcquisitionParametersSapera * const
                                               );

#ifdef HAVE_SAPERA_SDK

//! Get image data type.
ImageDataType
GetImageDataType(
                 SapBuffer * const,
                 SapAcqDevice * const
                 );

//! Get pixel format.
SapFormat
GetSaperaPixelFormat(
                     ImageDataType const
                     );

//! Get Bayer alignment mode.
SapBayer::Align
GetSaperaBayerAlignmentMode(
                            ImageDataType const
                            );

#endif /* HAVE_SAPERA_SDK */



#endif /* __BATCHACQUISITIONSAPERA_H */
