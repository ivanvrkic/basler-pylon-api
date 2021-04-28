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
  \file   BatchAcquisitionAcquisition.h
  \brief  Image acquisition thread.

  \author Tomislav Petkovic
  \date   2017-01-25
*/


#ifndef __BATCHACQUISITIONACQUISITION_H
#define __BATCHACQUISITIONACQUISITION_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionTiming.h"
#include "BatchAcquisitionImageQueue.h"
#include "BatchAcquisitionImageEncoder.h"
#include "BatchAcquisitionImageDecoder.h"
#include "BatchAcquisitionWindowDisplay.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionSapera.h"
#include "BatchAcquisitionFlyCapture2.h"
#include "BatchAcquisitionFromFile.h"



//! Parameters of the acquisition thread.
/*!
  Image acquisition thread acquires images and queues them into image encoder thread for storage.
*/
typedef
struct AcquisitionParameters_
{
  HANDLE tAcquisition; //!< Handle to image acquisition thread.

  int CameraID; //!< Camera ID.
  int ProjectorID; //!< Projector ID.

  volatile bool fActive; //!< Flag to indicate image acquisition thread is active.
  volatile bool fWaiting; //!< Flag to indicate image acquisition thread is waiting for an event to be signalled.
  volatile bool fView; //!< Flag to indicate image view window is enabled.
  volatile bool fExposureInProgress; //!< Flag to indicate exposure is in progress.
  volatile bool fThrottleDown; //!< Flag to indicate we must slow down the acquisition.
  int timeout; //!< Timeout in ms.

  FrameStatistics * pStatisticsTriggerDuration; //!< Statistics for tracking the average software trigger time.
  FrameStatistics * pStatisticsTriggerFrequency; //!< Statistics for tracking the frequency of software triggers.
  FrameStatistics * pStatisticsAcquisitionDuration; //!< Statistics for tracking the average total acquisition time for blocking acquisition mode only.

  SynchronizationEvents * pSynchronization; //!< Pointer to synchronization structure.
  DisplayWindowParameters * pWindow; //!< Display window.
  PreviewWindowParameters * pView; //!< Live preview window.
  ImageDecoderParameters * pImageDecoder; //!< Image decoder.
  ImageEncoderParameters * pImageEncoder; //!< Image encoder.

  ImageMetadataQueue * pMetadataQueue; //!< Image metadata queue.

  long int trigger_counter; //!< Value of trigger counter at last camera trigger in acquisition thread.

  long int vblank_counter_before_trigger_RT; //!< Value of VBLANK counter at last camera trigger in rendering thread.
  long int present_counter_before_trigger_RT; //!< Value of present counter at last camera trigger in rendering thread.

  long int key; //!< Present counter value of the current frame.

  LARGE_INTEGER QPC_before_trigger_RT; /*!< QPC value immediately before CAMERA_SEND_TRIGGER event is sent from the rendering thread. */
  LARGE_INTEGER QPC_after_trigger_RT; /*!< QPC value immediately after CAMERA_SEND_TRIGGER event is dispatched from the rendering thread. */
  LARGE_INTEGER QPC_before_trigger_AT; /*!< QPC value immediately before camera is triggered in acquisition thread. */
  LARGE_INTEGER QPC_after_trigger_AT; /*!< QPC value immediately after camera is triggered in acquisition thread. */
  LARGE_INTEGER QPC_exposure_start; //!< QPC time when camera exposure started.
  LARGE_INTEGER QPC_exposure_end_scheduled; //!< QPC time at which we expect the exposure to end.

  std::wstring * pFilenameAT; //!< Filename of the image.
  ImageMetadata sImageMetadataAT; //!< Metadata of currently presented image (contains filename, index, batch flag etc.).

  SRWLOCK sLockAT; //!< Slim lock for acquisition thread in exclusive mode and other threads in shared mode.

  AcquisitionParametersSapera * pSaperaSDK; //!< Pointer to camera parameters if Teledyne Dalsa Sapera SDK is used.
  AcquisitionParametersFlyCapture2 * pFlyCapture2SDK; //!< Pointer to camera parameters if PointGrey FlyCapture2 SDK is used.
  AcquisitionParametersFromFile * pFromFile; //!< Pointer to camera parameters if dummy acquisition from file is used.

  static int const nFrames = 18; //!< Number of frame buffers.

  __int64 exposureTime_QPC; //!< Exposure time in QPC units.
  double exposureTime_requested_us; //!< Requested exposure time in us (microseconds).
  double exposureTime_achieved_us; //!< Achieved exposure time in us (microseconds).
  double k; //!< Number of frames we integrate.
} AcquisitionParameters;



/****** START/STOP THREAD ******/

//! Create acquisition parameters and start acquisition thread.
AcquisitionParameters *
AcquisitionThreadStart(
                       SynchronizationEvents * const,
                       DisplayWindowParameters * const,
                       PreviewWindowParameters * const,
                       ImageEncoderParameters * const,
                       ImageDecoderParameters * const,
                       CameraSDK const,
                       int const,
                       int const,
                       std::vector<std::wstring *> * const,
                       bool const
                       );

//! Stop acquisition thread.
void
AcquisitionThreadStop(
                      AcquisitionParameters * const
                      );


/****** AUXILIARY FUNCTIONS ******/

//! Restart image transfers.
bool
AcquisitionThreadRestartCameraTransfers(
                                        AcquisitionParameters * const
                                        );

//! Camera exposure time.
double
CameraExposureTimeFromRefreshRate(
                                  AcquisitionParameters * const
                                  );

//! Get acquisition method.
CameraSDK
GetAcquisitionMethod(
                     AcquisitionParameters * const
                     );

//! Test if acquisition is live.
bool
IsAcquisitionLive(
                  AcquisitionParameters * const
                  );

//! Get unique camera identifier.
std::wstring *
GetUniqueCameraIdentifier(
                          AcquisitionParameters * const
                          );

//! Test if all acquisition methods are from file.
bool
AreAllAcquisitionMethodsFromFile(
                                 std::vector<AcquisitionParameters *> &,
                                 PSRWLOCK const
                                 );

//! Test if any acquisition method is from file.
bool
IsAnyAcquisitionMethodFromFile(
                               std::vector<AcquisitionParameters *> &,
                               PSRWLOCK const
                               );

//! Rescan input directory.
bool
AcquisitionThreadRescanInputDirectory(
                                      AcquisitionParameters * const
                                      );

//! Set new projector ID.
bool
AcquisitionThreadSetNewProjectorID(
                                   AcquisitionParameters * const,
                                   int const
                                   );

//! Set new camera and encoder ID.
bool
AcquisitionThreadSetNewCameraIDAndEncoderID(
                                            AcquisitionParameters * const,
                                            int const,
                                            int const
                                            );

#endif /* !__BATCHACQUISITIONACQUISITION_H */
