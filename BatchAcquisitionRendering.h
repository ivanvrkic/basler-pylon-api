/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2016-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2016-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionRendering.h
  \brief  Image rendering thread.

  \author Tomislav Petkovic
  \date   2017-02-13
*/


#ifndef __BATCHACQUISITIONRENDERING_H
#define __BATCHACQUISITIONRENDERING_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionTiming.h"
#include "BatchAcquisitionWindowDisplay.h"
#include "BatchAcquisitionImageDecoder.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionAcquisition.h"



//! Present and trigger times for non-blocking mode.
/*!
  In non-blocking acquisition mode camera trigger must be delayed for a specified delay time from
  the moment the frame was presented. For such scheme to work we must keep precise timing information
  when past present events occured and when future present and trigger events are scheduled.

  This structure stores present and VBLANK counters and QPC timer values for the present operation of
  the current frame, for the trigger operation of the current frame, and for the present operation of
  the next frame.
*/
typedef
struct PresentAndTriggerTimes_
{
  long int key; //!< Unique number which identifies a frame.
  long int present_counter; //!< Present counter value of the current frame. This is a key which identifies particular SL pattern.
  long int vblank_counter; //!< VBLANK counter value at current frame present.

  long int vblank_counter_trigger_scheduled; //!< Expected VBlank counter value when rendering thread has to execute CAMERA_SYNC_TRIGGERS event.
  long int vblank_counter_next_scheduled; //!< Expected VBlank counter value at next frame present.
  long int vblank_counter_next_presented; //!< VBlank counter value at next frame present.

  __int64 QPC_current_presented; //!< QPC value at current frame present.
  __int64 QPC_trigger_scheduled_RT; //!< Expected QPC value when renedring thread has to execute CAMERA_SYNC_TRIGGERS event.
  __int64 QPC_trigger_scheduled_AT; //!< Expected QPC value when acquisition thread will trigger the camera.
  __int64 QPC_next_scheduled; //!< Expected QPC counter value when next frame will be presented.
  __int64 QPC_next_presented; //!< QPC counter value when next frame was presented (negative value indicates the next frame was not yet presented).
} PresentAndTriggerTimes;



//! Parameters of the rendering thread.
/*!
  Image rendering thread renders images and queues them in the DirectX swap chain for display.
*/
typedef
struct RenderingParameters_
{
  HANDLE tRendering; //!< Handle to image rendering thread.

  int ProjectorID; //!< Projector ID.

  volatile UINT SyncInterval; //!< Sync interval for DXGI present operation.

  volatile double delay_ms; //!< Trigger delay in ms for blocking acquisition. Ignored for non-blocking acquisition.

  volatile bool fActive; //!< Flag to indicate image rendering thread is active.
  volatile bool fWaiting; //!< Flag to indicate image rendering thread is waiting for an event to be signalled.
  volatile bool fBatch; //!< Flag to indicate batch acquisition is in progress.
  volatile bool fSavePNG; //!< Flag to indicate acquired images should be saved to disk in PNG format.
  volatile bool fSaveRAW; //!< Flag to indicate acquired images should be saved to disk in RAW format.
  volatile bool fSynchronize; //!< Flag to indicate projector syhnchronization should be enabled.

  volatile int num_prj; //!< Number of projectors to synchronize.

  std::vector<RenderingParameters_ *> * pRenderings; //!< Vector containing pointers to rendering threads data of projectors which work synchronously.
  SRWLOCK sLockRenderings; //!< Slim lock to control concurrent access.

  std::vector<PresentAndTriggerTimes> * pTriggers; //!< Vector containing future triggering data.

  std::vector<AcquisitionParameters *> * pAcquisitions; //!< Vector containing pointers to acquisition threads data.
  SRWLOCK sLockAcquisitions; //!< Slim lock to control concurrent access.

  FrameStatistics * pStatisticsRenderDuration; //!< Statistics for tracking the average rendering time for images.
  FrameStatistics * pStatisticsPresentDuration; //!< Statistics for tracking the average present time for images.
  FrameStatistics * pStatisticsPresentFrequency; //!< Statistics for tracking the frequency of present operation.
  FrameStatistics * pStatisticsWaitForVBLANKDuration; //!< Statistics for tracking the average time spent waiting for the next VBLANK.

  SynchronizationEvents * pSynchronization; //!< Pointer to synchronization structure.
  DisplayWindowParameters * pWindow; //!< Display window.
  ImageDecoderParameters * pImageDecoder; //!< Image decoder.
} RenderingParameters;



/****** START/STOP THREAD ******/

//! Create rendering parameters and start rendering thread.
RenderingParameters *
RenderingThreadStart(
                     SynchronizationEvents * const,
                     DisplayWindowParameters * const,
                     ImageDecoderParameters * const,
                     int const
                     );

//! Stop rendering thread.
void
RenderingThreadStop(
                    RenderingParameters * const
                    );


/****** AUXILIARY FUNCTIONS ******/

//! Add projectors which should be synchronized.
bool
RenderingThreadAddProjectors(
                             RenderingParameters * const,
                             std::vector<RenderingParameters_ *> * const
                             );

//! Remove all projectors which should be synchronized.
bool
RenderingThreadRemoveProjectors(
                                RenderingParameters * const
                                );

//! Add camera to rendering thread.
bool
RenderingThreadAddCamera(
                         RenderingParameters * const,
                         AcquisitionParameters * const
                         );

//! Remove camera from rendering thread.
bool
RenderingThreadRemoveCamera(
                            RenderingParameters * const,
                            AcquisitionParameters * const
                            );

//! Test if projector has attached cameras.
bool
RenderingThreadHaveCamera(
                          RenderingParameters * const
                          );

//! Return maximal exposure time.
double
RenderingThreadGetMaxExposureTimeForAttachedCameras(
                                                    RenderingParameters * const
                                                    );

//! Get output directory.
TCHAR const *
RenderingThreadGetImageEncoderDirectory(
                                        RenderingParameters * const
                                        );

//! Set live preview for attached cameras.
bool
RenderingThreadSetLiveViewForAttachedCameras(
                                             RenderingParameters * const,
                                             bool const
                                             );

//! Toggle live preview for attached cameras.
bool
RenderingThreadToggleLiveViewForAttachedCameras(
                                                RenderingParameters * const,
                                                bool * const,
                                                bool * const
                                                );

//! Set CAMERA_READY event for attached cameras.
bool
RenderingThreadSetCameraReadyForAttachedCameras(
                                                RenderingParameters * const
                                                );

//! Set directory for from file acquisition.
bool
RenderingThreadSetFromFileInputDirectory(
                                         RenderingParameters * const,
                                         TCHAR const * const
                                         );

//! Set projector size for image encoders.
bool
RenderingThreadSetProjectorSizeForImageEncoders(
                                                RenderingParameters * const,
                                                int const,
                                                int const,
                                                RECT const,
                                                RECT const
                                                );
//! Get cycling flag.
bool
RenderingThreadGetCycleFlagForImageDecoder(
                                           RenderingParameters * const,
                                           bool * const
                                           );

//! Set cycling flag.
bool
RenderingThreadSetCycleFlagForImageDecoder(
                                           RenderingParameters * const,
                                           bool const
                                           );

//! Set input directory.
bool
RenderingThreadAskUserToSetInputDirectory(
                                          RenderingParameters * const
                                          );

//! Get input directory.
TCHAR const *
RenderingThreadGetInputDirectory(
                                 RenderingParameters * const
                                 );

//! Rescan input directory.
bool
RenderingThreadRescanInputDirectory(
                                    RenderingParameters * const
                                    );

//! Get number of images to project.
bool
RenderingThreadGetNumberOfImagesToProjectAndAcquire(
                                                    RenderingParameters * const,
                                                    int * const,
                                                    int * const
                                                    );

//! Set new projector and encoder ID.
bool
RenderingThreadSetNewProjectorIDAndDecoderID(
                                             RenderingParameters * const,
                                             int const,
                                             int const
                                             );

//! Get unique projector (monitor) identifier.
std::wstring *
GetUniqueProjectorIdentifier(
                             RenderingParameters * const
                             );


#endif /* !__BATCHACQUISITIONRENDERING_H */
