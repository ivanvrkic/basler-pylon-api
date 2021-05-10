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
  \file   BatchAcquisitionMain.cpp
  \brief  Test of synchronous acquisition.

  \author Tomislav Petkovic
  \date   2017-02-07
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONMAIN_CPP
#define __BATCHACQUISITIONMAIN_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisition.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionFileList.h"
#include "BatchAcquisitionWindowDisplay.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionImageDecoder.h"
#include "BatchAcquisitionImageEncoder.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionRendering.h"
#include "BatchAcquisitionSwapChain.h"
#include "BatchAcquisitionKeyboard.h"
#include "BatchAcquisitionVTK.h"
#include "BatchAcquisitionWindowStorage.h"

#include "conio.h"

#pragma warning(push)
#pragma warning(disable: 4005)

#include <shlwapi.h>

#pragma warning(pop)

#pragma comment(lib, "Shlwapi.lib")



/****** HELPER FUNCTIONS ******/

#pragma region // Start and stop continuous acquisition

//! Stops continuous acquisition.
/*!
  Function stops continuous acquisition for the corresponding rendering thread.
  This is accomplished by signalling MAIN_PREPARE_DRAW which causes the rendering
  thread and all attached acquisition threads to stop all pending tasks and to prepare
  for batch acquisition.

  \param pRendering     Pointer to rendering thread for which to stop the acquisition.
  \param MainID  Unique ID of the main thread.
*/
inline
void
MainStopContinuousAcquisition_inline(
                                     RenderingParameters * const pRendering,
                                     int const MainID
                                     )
{
  assert(NULL != pRendering);
  if (NULL == pRendering) return;

  // Fetch synchronization object.
  SynchronizationEvents * const pSynchronization = pRendering->pSynchronization;
  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return;

  // Fetch projector ID.
  int const ProjectorID = pRendering->ProjectorID;
  assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

  // Inform the user that continuous acquisition will be stopped.
  {
    int const cnt = wprintf(gMsgCyclingStop, ProjectorID + 1);
    assert(0 < cnt);
  }

  // Store current cycling status.
  bool cycle = true;
  {
    bool const get_cycle = RenderingThreadGetCycleFlagForImageDecoder(pRendering, &cycle);
    assert(true == get_cycle);
  }

  // Reset MAIN_* events except MAIN_*_CAMERA events.
  {
    BOOL const reset = pSynchronization->EventResetAllMain(MainID, ProjectorID, -1);
    assert(0 != reset);
  }

  // Stop the acquisition by raising the MAIN_PREPARE_DRAW signal.
  {
    BOOL const prepare = pSynchronization->EventSet(MAIN_PREPARE_DRAW, ProjectorID);
    assert(0 != prepare);
  }

  // Wait for the preparation to complete.
  {
    DWORD const wait = pSynchronization->EventWaitFor(MAIN_READY_DRAW, ProjectorID, INFINITE);
    assert( WAIT_OBJECT_0 == wait );
  }

  // Restore list cycling flag.
  {
    bool const set_cycle = RenderingThreadSetCycleFlagForImageDecoder(pRendering, cycle);
    assert(true == set_cycle);
  }

  // Inform the user that continuous acquisition has stopped.
  {
    int const cnt = wprintf(gMsgCyclingStopped, ProjectorID + 1);
    assert(0 < cnt);
  }
}
/* MainStopContinuousAcquisition_inline */



//! Starts continuous acquisition.
/*!
  Function restarts continuous acquisition which was stopped using MainStopContinuousAcquisition function.

  \param pRendering     Pointer to rendering thread for which to start the acquisition.
*/
inline
void
MainStartContinuousAcquisition_inline(
                                      RenderingParameters * const pRendering
                                      )
{
  assert(NULL != pRendering);
  if (NULL == pRendering) return;

  // Fetch synchronization object.
  SynchronizationEvents * const pSynchronization = pRendering->pSynchronization;
  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return;

  // Fetch image decoder object.
  ImageDecoderParameters * const pImageDecoder = pRendering->pImageDecoder;
  assert(NULL != pImageDecoder);
  if (NULL == pImageDecoder) return;

  // Fetch display window.
  DisplayWindowParameters * const pWindow = pRendering->pWindow;
  assert(NULL != pWindow);
  if (NULL == pWindow) return;

  // Fetch projector and decoder ID.
  int const ProjectorID = pRendering->ProjectorID;
  assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

  int const DecoderID = pImageDecoder->DecoderID;
  assert( (0 <= DecoderID) && (DecoderID < (int)(pSynchronization->ImageDecoder.size())) );
  assert( ProjectorID == pImageDecoder->ProjectorID );

  // Inform the user that continuous acquisition will restart.
  {
    int const cnt = wprintf(gMsgCyclingStart, ProjectorID + 1);
    assert(0 < cnt);
  }

  // Check event status.
  {
    assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );
    assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
    assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );
    assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
    assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
  }

  // Fill image decoder queue.
  bool queue_full = false;
  do
    {
      BOOL const decoder = pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, DecoderID);
      assert(0 != decoder);

      DWORD const full = pSynchronization->EventWaitFor(IMAGE_DECODER_QUEUE_FULL, DecoderID, 50);
      //assert( WAIT_OBJECT_0 == full );
      queue_full = ( WAIT_OBJECT_0 == full );
    }
  while (false == queue_full);

  // Restart present-acquire cycle.
  BOOL const set_render_ready = pSynchronization->EventSet(DRAW_RENDER_READY, ProjectorID);
  assert(0 != set_render_ready);

  // Conditions required for DRAW_RENDER to be correctly executed without starting the acquisition loop.
  assert(true == pWindow->fBlocking);
  assert(false == pWindow->fConcurrentDelay);

  // Start rendering next frame.
  BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
  assert(0 != set_render);

  // Enable live view.
  bool const enable_live_view = RenderingThreadSetLiveViewForAttachedCameras(pRendering, true);
  assert(true == enable_live_view);

  // Enable triggering.
  bool const set_all_ready = RenderingThreadSetCameraReadyForAttachedCameras(pRendering);
  assert(true == set_all_ready);

  // Kick-start present-acquire cycle; cycle will autostart for projectors with no cameras attached.
  if (true == RenderingThreadHaveCamera(pRendering))
    {
      // Wait for render operation to complete.
      DWORD const dwWaitResult = pSynchronization->EventWaitFor(DRAW_PRESENT_READY, ProjectorID, INFINITE);
      assert(WAIT_OBJECT_0 == dwWaitResult);

      // Re-start project-acquire loop.
      BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, ProjectorID);
      assert(0 != set_present);
    }
  /* if */

  // Inform the user that continuous acquisition has restarted.
  {
    int const cnt = wprintf(gMsgCyclingStarted, ProjectorID + 1);
    assert(0 < cnt);
  }
}
/* MainStartContinuousAcquisition_inline */

#pragma endregion // Start and stop continuous acquisition


#pragma region // Query user to select active projector or camera

//! Query user to select SDK.
/*!
  Function prints selection menu and waits for the user to select camera SDK.

  \param timeout_ms     Timeout in ms.
  \param allow_from_file        Set to true if dummy acquisition from file is allowed.
  \param hWndCommand  Handle to console window.
  \return Functions returns camera SDK indetifier.
*/
inline
CameraSDK
MainSelectCameraSDK_inline(
                           int const timeout_ms,
                           bool const allow_from_file,
                           HWND const hWndCommand
                           )
{
  {
    int const cnt = wprintf(L"\n");
    assert(0 < cnt);
  }

  if (true == allow_from_file)
    {
      int const cnt = wprintf(gMsgCameraSDK);
      assert(0 < cnt);
    }
  else
    {
      int const cnt = wprintf(gMsgCameraSDKExceptFromFile);
      assert(0 < cnt);
    }
  /* if */

  CameraSDK selected_camera_SDK = CAMERA_SDK_DEFAULT;

  int const pressed_key = TimedWaitForNumberKey(timeout_ms, 10, true, true, hWndCommand);
  if (1 == pressed_key)
    {
      selected_camera_SDK = CAMERA_SDK_FLYCAPTURE2;

      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgCameraSDKUseFlyCapture2);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  else if (2 == pressed_key)
    {
      selected_camera_SDK = CAMERA_SDK_SAPERA;

      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgCameraSDKUseSaperaLT);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  else if ( (3 == pressed_key) && (true == allow_from_file) )
    {
      selected_camera_SDK = CAMERA_SDK_FROM_FILE;

      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgCameraSDKUseFromFile);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  else
    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgCameraSDKUseDefault);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  /* if */

  return selected_camera_SDK;
}
/* MainSelectCameraSDK_inline */



//! Queries user to select projector.
/*!
  Function queries the user to select a projector to use.

  \param num_prj        Number of projectors attached.
  \param DefaultProjectorID     Default choice.
  \param timeout_ms     Timeout in ms for which to wait for user interaction.
  \param hWndCommand  Handle to console window.
  \return Function returns a valid projector ID.
*/
inline
int
MainSelectProjectorID_inline(
                             int const num_prj,
                             int const DefaultProjectorID,
                             int const timeout_ms,
                             HWND const hWndCommand
                             )
{
  assert( 1 <= num_prj );

  // If there is only one projector return immediately.
  if (1 >= num_prj)
    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectProjectorDefaultChoice, DefaultProjectorID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
      assert( (0 <= DefaultProjectorID) && (DefaultProjectorID < num_prj) );

      return DefaultProjectorID;
    }
  /* if */

  // Output projector menu.
  {
    int const cnt1 = wprintf(L"\n");
    int const cnt2 = wprintf(gMsgSelectProjectorQuery);
    assert( (0 < cnt1) && (0 < cnt2) );
  }
  for (int i = 0; i < num_prj; ++i)
    {
      int const ProjectorID = i;
      if (ProjectorID == DefaultProjectorID)
        {
          int const cnt = wprintf(gMsgSelectProjectorItemDefault, i + 1, ProjectorID + 1);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgSelectProjectorItem, i + 1, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */

  int ProjectorID = TimedWaitForNumberKey(timeout_ms, 10, true, true, hWndCommand) - 1;
  if ( (0 > ProjectorID) || (num_prj <= ProjectorID) )
    {
      ProjectorID = DefaultProjectorID;

      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectProjectorDefaultChoice, DefaultProjectorID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  else
    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectProjectorUserChoice, ProjectorID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  /* if */

  assert( (0 <= ProjectorID) && (ProjectorID < num_prj) );

  return ProjectorID;
}
/* MainSelectProjectorID_inline */



//! Queries user to select camera.
/*!
  Function queries the user to select a camera to use.

  \param num_cam        Number of cameras attached.
  \param DefaultCameraID     Default choice.
  \param timeout_ms     Timeout in ms for which to wait for user interaction.
  \param hWndCommand  Handle to console window.
  \return Function returns a valid camera ID.
*/
inline
int
MainSelectCameraID_inline(
                          int const num_cam,
                          int const DefaultCameraID,
                          int const timeout_ms,
                          HWND const hWndCommand
                          )
{
  assert( 1 <= num_cam );

  // If there is only one camera return immediately.
  if (1 >= num_cam)
    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectCameraDefaultChoice, DefaultCameraID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
      assert( (0 <= DefaultCameraID) && (DefaultCameraID < num_cam) );

      return DefaultCameraID;
    }
  /* if */

  // Output camera menu.
  {
    int const cnt1 = wprintf(L"\n");
    int const cnt2 = wprintf(gMsgSelectCameraQuery);
    assert( (0 < cnt1) && (0 < cnt2) );
  }
  for (int i = 0; i < num_cam; ++i)
    {
      int const CameraID = i;
      if (CameraID == DefaultCameraID)
        {
          int const cnt = wprintf(gMsgSelectCameraItemDefault, i + 1, CameraID + 1);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgSelectCameraItem, i + 1, CameraID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */

  int CameraID = TimedWaitForNumberKey(timeout_ms, 10, true, true, hWndCommand) - 1;
  if ( (0 > CameraID) || (num_cam <= CameraID) )
    {
      CameraID = DefaultCameraID;

      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectCameraDefaultChoice, DefaultCameraID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  else
    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgSelectCameraUserChoice, CameraID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
  /* if */

  assert( (0 <= CameraID) && (CameraID < num_cam) );

  return CameraID;
}
/* MainSelectCameraID_inline */

#pragma endregion // Query user to select active projector or camera


#pragma region // Print status information for attached projectors and cameras

//! Print batch configuration parameters.
/*!
  Function outputs batch configuration to console.

  \param fBlocking      Flag which indicates if acquisition is blocking or non-blocking.
  \param fConcurrentDelay       Flag which indicats what type of delay is used.
  \param fFixed    Flag which indicates a fixed SL pattern is used.
  \param num_acquire     Number of frames to acquire when fixed SL pattern is used.
*/
inline
void
MainPrintBatchConfiguration_inline(
                                   bool const fBlocking,
                                   bool const fConcurrentDelay,
                                   bool const fFixed,
                                   int const num_acquire
                                   )
{
  int const cnt1 = wprintf(
                           gMsgBatchConfigurationBlockingModePrint,
                           (true == fBlocking)? gMsgStringEnabled : gMsgStringDisabled
                           );

  int const cnt2 = wprintf(
                           gMsgBatchConfigurationConcurrentDelayPrint,
                           (true == fConcurrentDelay)? gMsgStringEnabled : gMsgStringDisabled
                           );

  int const cnt3 = wprintf(
                           gMsgBatchConfigurationFixedSLPatternPrint,
                           (true == fFixed)? gMsgStringEnabled : gMsgStringDisabled
                           );

  int const cnt4 = wprintf(gMsgBatchConfigurationNumAcquirePrint, num_acquire);

  assert( (0 < cnt1) && (0 < cnt2) && (0 < cnt3) && (0 < cnt4) );
}
/* MainPrintBatchConfiguration_inline */



//! Print output directory.
/*!
  Function prints output directory for the default image encoder.

  \param sImageEncoder  Reference to array of all image encoders.
  \param pThreadStorageLock     Storage lock.
  \param DefaultEncoderID       ID of the default encoder.
  \param tag    Pointer to acquisition name tag. May be NULL which indicates there is no tag.
*/
inline
void
MainPrintOutputDirectory_inline(
                                std::vector<ImageEncoderParameters *> & sImageEncoder,
                                SRWLOCK * const pThreadStorageLock,
                                int const DefaultEncoderID,
                                std::wstring * const tag
                                )
{
  int const num_enc = (int)( sImageEncoder.size() );
  if (0 >= num_enc) return;

  ImageEncoderParameters * const pDefaultImageEncoder = get_ptr_inline(sImageEncoder, DefaultEncoderID, pThreadStorageLock);
  //assert(NULL != pDefaultImageEncoder);

  if (NULL != pDefaultImageEncoder)
    {
      int const EncoderID = pDefaultImageEncoder->EncoderID;
      wchar_t const * const directory = ImageEncoderGetDirectory( pDefaultImageEncoder );
      if (NULL != directory)
        {
          int const cnt = wprintf(gMsgOutputDirectoryPrint, EncoderID + 1, directory);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgOutputDirectoryInvalid, EncoderID + 1);
          assert(0 < cnt);
        }
      /* if */

      std::wstring * session = ImageEncoderGetSubdirectorySession( pDefaultImageEncoder );
      if (NULL != session)
        {
          int const cnt = wprintf(gMsgSessionSubdirectoryPrint, EncoderID + 1, session->c_str());
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgSessionSubdirectoryInvalid, EncoderID + 1);
          assert(0 < cnt);
        }
      /* if */
      SAFE_DELETE( session );

      if (NULL != tag)
        {
          int const cnt = wprintf(gMsgAcquisitionTagPrint, EncoderID + 1, tag->c_str());
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgAcquisitionTagInvalid, EncoderID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* if */
}
/* MainPrintOutputDirectory_inline */



//! Print all exposure multipliers.
/*!
  Function outputs all exposure multipliers and expected shutter speeds to console.

  \param sAcquisition   Reference to array of all acquisition thread parameters.
  \param pThreadStorageLock     Storage lock.
*/
inline
void
MainPrintAllExposureMultipliers_inline(
                                       std::vector<AcquisitionParameters *> & sAcquisition,
                                       SRWLOCK * const pThreadStorageLock
                                       )
{
  int const num_cam = (int)( sAcquisition.size() );

  for (int CameraID = 0; CameraID < num_cam; ++CameraID)
    {
      AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, pThreadStorageLock);
      assert(NULL != pAcquisition);
      if (NULL != pAcquisition)
        {
          assert(CameraID == pAcquisition->CameraID);
          double const multiplier = pAcquisition->k;
          double const exposureTime = CameraExposureTimeFromRefreshRate(pAcquisition);
          int const cnt = wprintf(gMsgExposureMultiplierPrint, CameraID + 1, multiplier, exposureTime);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidCamera, CameraID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllExposureMultipliers_inline */



//! Print all fullscreen resolutions.
/*!
  Function outputs fullscreen resolutions for each projector.

  \param sRendering    Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
*/
inline
void
MainPrintAllResolutions_inline(
                               std::vector<RenderingParameters *> & sRendering,
                               SRWLOCK * const pThreadStorageLock
                               )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          DisplayWindowParameters * const pWindow = pRendering->pWindow;
          assert(NULL != pWindow);
          if (NULL != pWindow)
            {
              double const num = (double)( pWindow->sFullScreenMode.RefreshRate.Numerator );
              double const den = (double)( pWindow->sFullScreenMode.RefreshRate.Denominator );
              double const freq = num / den;

              int const cnt = wprintf(
                                      gMsgProjectorFullscreenMode,
                                      ProjectorID + 1,
                                      pWindow->sFullScreenMode.Width,
                                      pWindow->sFullScreenMode.Height,
                                      freq
                                      );
              assert(0 < cnt);
            }
          else
            {
              int const cnt = wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllResolutions_inline */



//! Print all containing displays.
/*!
  Function outputs containing displays for each projector.

  \param sRendering    Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
*/
inline
void
MainPrintAllContainingDisplays_inline(
                                      std::vector<RenderingParameters *> & sRendering,
                                      SRWLOCK * const pThreadStorageLock
                                      )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          DisplayWindowParameters * const pWindow = pRendering->pWindow;
          assert(NULL != pWindow);
          if (NULL != pWindow)
            {
              HMONITOR const hMonitor = MonitorFromWindow(pWindow->hWnd, MONITOR_DEFAULTTONULL);
              assert(NULL != hMonitor);
              if (NULL != hMonitor)
                {
                  MONITORINFOEX monitorInfo;
                  ZeroMemory( &monitorInfo, sizeof(MONITORINFOEX) );
                  monitorInfo.cbSize = sizeof(MONITORINFOEX);

                  BOOL const get_info = GetMonitorInfo(hMonitor, &monitorInfo);
                  assert(TRUE == get_info);
                  if (TRUE == get_info)
                    {
                      int const cnt = wprintf(gMsgProjectorMonitorName, ProjectorID + 1, monitorInfo.szDevice);
                      assert(0 < cnt);
                    }
                  else
                    {
                      int const cnt = wprintf(gMsgProjectorMonitorUnknown);
                      assert(0 < cnt);
                    }
                  /* if */
                }
              else
                {
                  int const cnt = wprintf(gMsgProjectorMonitorUnknown);
                  assert(0 < cnt);
                }
              /* if */
            }
          else
            {
              int const cnt = wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllContainingDisplays_inline */



//! Print all input directories.
/*!
  Function prints all input directories.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
*/
inline
void
MainPrintAllInputDirectories_inline(
                                    std::vector<RenderingParameters *> & sRendering,
                                    SRWLOCK * const pThreadStorageLock
                                    )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          bool const rescan = RenderingThreadRescanInputDirectory(pRendering);
          //assert(true == rescan);
          if (true == rescan)
            {
              assert(NULL != pRendering->pImageDecoder);
              assert(NULL != pRendering->pImageDecoder->pImageList);

              TCHAR const * const directory = pRendering->pImageDecoder->pImageList->GetDirectory();
              assert(NULL != directory);
              if (NULL != directory)
                {
                  int const num_images = (int)( pRendering->pImageDecoder->pImageList->Size() );

                  if (0 < num_images)
                    {
                      int const cnt = wprintf(gMsgProjectorInputDirectory, ProjectorID + 1, num_images, directory);
                      assert(0 < cnt);
                    }
                  else
                    {
                      int const cnt = wprintf(gMsgProjectorInputDirectoryEmpty, ProjectorID + 1, directory);
                      assert(0 < cnt);
                    }
                  /* if */
                }
              else
                {
                  int const cnt = wprintf(gMsgProjectorInputDirectoryInvalid, ProjectorID + 1);
                  assert(0 < cnt);
                }
              /* if */
            }
          else
            {
              int const cnt = wprintf(gMsgInvalidProjectorImageDecoder, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllInputDirectories_inline */



//! Print all attached cameras.
/*!
  Function outputs all attached cameras for each projector.

  \param sRendering    Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
*/
inline
void
MainPrintAllAttachedCameras_inline(
                                   std::vector<RenderingParameters *> & sRendering,
                                   SRWLOCK * const pThreadStorageLock
                                   )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          bool const have_camera = RenderingThreadHaveCamera(pRendering);
          if (true == have_camera)
            {
              assert(NULL != pRendering->pAcquisitions);

              int const num_cam = (int)( pRendering->pAcquisitions->size() );
              {
                int const cnt = wprintf(gMsgAttachedCamerasListStart, ProjectorID + 1, num_cam);
                assert(0 < cnt);
              }
              for (int i = 0; i < num_cam; ++i)
                {
                  AcquisitionParameters * const pAcquisition = get_ptr_inline(*(pRendering->pAcquisitions), i, &(pRendering->sLockAcquisitions));
                  assert(NULL != pAcquisition);
                  if (NULL != pAcquisition)
                    {
                      int const CameraID = pAcquisition->CameraID;
                      int const cnt = wprintf(gMsgAttachedCamerasListItemValid, CameraID + 1);
                      assert(0 < cnt);
                    }
                  else
                    {
                      int const cnt = wprintf(gMsgAttachedCamerasListItemInvalid);
                      assert(0 < cnt);
                    }
                  /* if */
                  if (i + 1 < num_cam)
                    {
                      int const cnt = wprintf(gMsgAttachedCamerasListSeparator);
                      assert(0 < cnt);
                    }
                  /* if */
                }
              /* for */
              {
                int const cnt = wprintf(gMsgAttachedCamerasListEnd);
                assert(0 < cnt);
              }
              for (int i = 0; i < num_cam; ++i)
                {
                  AcquisitionParameters * const pAcquisition = get_ptr_inline(*(pRendering->pAcquisitions), i, &(pRendering->sLockAcquisitions));
                  assert(NULL != pAcquisition);
                  if (NULL != pAcquisition)
                    {
                      int const CameraID = pAcquisition->CameraID;
                      std::wstring * CameraUID = GetUniqueCameraIdentifier(pAcquisition);
                      if (NULL != CameraUID)
                        {
                          int const cnt = wprintf(gMsgAttachedCamerasListCameraUID, CameraID + 1, CameraUID->c_str());
                          assert(0 < cnt);
                        }
                      /* if */
                      SAFE_DELETE(CameraUID);
                    }
                  /* if */
                }
              /* for */
            }
          else // !(true == have_camera)
            {
              int const cnt = wprintf(gMsgAttachedCamerasListNone, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if (true == have_camera) */
        }
      else // !(NULL != pRendering)
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if (NULL != pRendering) */
    }
  /* for */

}
/* MainPrintAllAttachedCameras_inline */



//! Prints save-to-file options.
/*!
  Function prints chosen save-to-file options for all attached projectors.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Storage lock.
*/

inline
void
MainPrintAllSaveToFile_inline(
                              std::vector<RenderingParameters *> & sRendering,
                              SRWLOCK * const pThreadStorageLock
                              )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          bool const fSavePNG = pRendering->fSavePNG;
          bool const fSaveRAW = pRendering->fSaveRAW;
          if ( (true == fSavePNG) && (true == fSaveRAW) )
            {
              int const cnt = wprintf(gMsgImageSaveToPNGAndRaw, ProjectorID + 1);
              assert(0 < cnt);
            }
          else if ( (true == fSavePNG) && (false == fSaveRAW) )
            {
              int const cnt = wprintf(gMsgImageSaveToPNG, ProjectorID + 1);
              assert(0 < cnt);
            }
          else if ( (false == fSavePNG) && (true == fSaveRAW) )
            {
              int const cnt = wprintf(gMsgImageSaveToRAW, ProjectorID + 1);
              assert(0 < cnt);
            }
          else if ( (false == fSavePNG) && (false == fSaveRAW) )
            {
              int const cnt = wprintf(gMsgImageSaveToNone, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllSaveToFile_inline */



//! Print all blocking delays.
/*!
  Print blocking delays for all attached projectors.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Storage lock.
*/
inline
void
MainPrintAllBlockingDelays_inline(
                                  std::vector<RenderingParameters *> & sRendering,
                                  SRWLOCK * const pThreadStorageLock
                                  )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          int const cnt = wprintf(gMsgDelayTimeBlockingPrint, ProjectorID + 1, pRendering->delay_ms);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllBlockingDelays_inline */



//! Print all non-blocking delays.
/*!
  Print non-blocking delays for all attached projectors.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Storage lock.
*/
inline
void
MainPrintAllNonBlockingDelays_inline(
                                     std::vector<RenderingParameters *> & sRendering,
                                     SRWLOCK * const pThreadStorageLock
                                     )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          DisplayWindowParameters * const pWindow = pRendering->pWindow;
          assert(NULL != pWindow);
          if (NULL != pWindow)
            {
              double const delayTime_ms = pWindow->delayTime_ms;
              long int const delayTime_whole = pWindow->delayTime_whole;
              double const delayTime_fraction_us = pWindow->delayTime_fraction_us;
              int const cnt = wprintf(gMsgDelayTimeNonBlockingPrint, ProjectorID + 1, delayTime_ms, delayTime_whole, delayTime_fraction_us);
              assert(0 < cnt);
            }
          else
            {
              int const cnt = wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllNonBlockingDelays_inline */



//! Print all non-blocking present times.
/*!
  Print non-blocking present times for all attached projectors.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Storage lock.
*/
inline
void
MainPrintAllNonBlockingPresentTimes_inline(
                                           std::vector<RenderingParameters *> & sRendering,
                                           SRWLOCK * const pThreadStorageLock
                                           )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          DisplayWindowParameters * const pWindow = pRendering->pWindow;
          assert(NULL != pWindow);
          if (NULL != pWindow)
            {
              long int const presentTime = pWindow->presentTime;
              int const cnt = wprintf(gMsgPresentTimeNonBlockingPrint, ProjectorID + 1, presentTime);
              assert(0 < cnt);
            }
          else
            {
              int const cnt = wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllNonBlockingPresentTimes_inline */



//! Print all collected timing information.
/*!
  Function prints out collected timing information which incude achieved FPS and average duration of operations.

  \param sRendering     Reference to array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
*/
inline
void
MainPrintAllTimingStatistics_inline(
                                    std::vector<RenderingParameters *> & sRendering,
                                    SRWLOCK * const pThreadStorageLock
                                    )
{
  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          // Fetch and print projector statistics.
          {
            double const render_time_min = FrameStatisticsGetMin(pRendering->pStatisticsRenderDuration);
            double const render_time_mu = FrameStatisticsGetMean(pRendering->pStatisticsRenderDuration);
            double const render_time_dev = FrameStatisticsGetDeviation(pRendering->pStatisticsRenderDuration);
            double const render_time_max = FrameStatisticsGetMax(pRendering->pStatisticsRenderDuration);

            if ( (false == isnan_inline(render_time_min)) &&
                 (false == isnan_inline(render_time_mu)) &&
                 (false == isnan_inline(render_time_dev)) &&
                 (false == isnan_inline(render_time_max))
                 )
              {
                int const cnt = wprintf(
                                        gMsgTimingStatisticsRenderTime,
                                        ProjectorID + 1,
                                        render_time_min, render_time_mu, render_time_dev, render_time_max
                                        );
                assert(0 < cnt);
              }
            /* if */

            double const present_time_min = FrameStatisticsGetMin(pRendering->pStatisticsPresentDuration);
            double const present_time_mu = FrameStatisticsGetMean(pRendering->pStatisticsPresentDuration);
            double const present_time_dev = FrameStatisticsGetDeviation(pRendering->pStatisticsPresentDuration);
            double const present_time_max = FrameStatisticsGetMax(pRendering->pStatisticsPresentDuration);

            if ( (false == isnan_inline(present_time_min)) &&
                 (false == isnan_inline(present_time_mu)) &&
                 (false == isnan_inline(present_time_dev)) &&
                 (false == isnan_inline(present_time_max))
                 )
              {
                int const cnt = wprintf(
                                        gMsgTimingStatisticsPresentTime,
                                        ProjectorID + 1,
                                        present_time_min, present_time_mu, present_time_dev, present_time_max
                                        );
                assert(0 < cnt);
              }
            /* if */

            double const vblank_time_min = FrameStatisticsGetMin(pRendering->pStatisticsWaitForVBLANKDuration);
            double const vblank_time_mu = FrameStatisticsGetMean(pRendering->pStatisticsWaitForVBLANKDuration);
            double const vblank_time_dev = FrameStatisticsGetDeviation(pRendering->pStatisticsWaitForVBLANKDuration);
            double const vblank_time_max = FrameStatisticsGetMax(pRendering->pStatisticsWaitForVBLANKDuration);

            if ( (false == isnan_inline(vblank_time_min)) &&
                 (false == isnan_inline(vblank_time_mu)) &&
                 (false == isnan_inline(vblank_time_dev)) &&
                 (false == isnan_inline(vblank_time_max))
                 )
              {
                int const cnt = wprintf(
                                        gMsgTimingStatisticsVBLANKTime,
                                        ProjectorID + 1,
                                        vblank_time_min, vblank_time_mu, vblank_time_dev, vblank_time_max
                                        );
                assert(0 < cnt);
              }
            /* if */

            double const present_total_time = FrameStatisticsGetTotalTime(pRendering->pStatisticsPresentFrequency);

            if ( false == isnan_inline(present_total_time) )
              {
                int const cnt = wprintf(gMsgTimingStatisticsTotalTimeProjector, ProjectorID + 1, present_total_time);
                assert(0 < cnt);
              }
            /* if */

            double const projector_FPS = FrameStatisticsGetFPS(pRendering->pStatisticsPresentFrequency);

            if ( false == isnan_inline(projector_FPS) )
              {
                int const cnt = wprintf(gMsgTimingStatisticsFPSProjector, ProjectorID + 1, projector_FPS);
                assert(0 < cnt);
              }
            /* if */
          }

          bool const have_camera = RenderingThreadHaveCamera(pRendering);
          if (true == have_camera)
            {
              assert(NULL != pRendering->pAcquisitions);

              int const num_cam = (int)( pRendering->pAcquisitions->size() );
              for (int i = 0; i < num_cam; ++i)
                {
                  AcquisitionParameters * const pAcquisition = get_ptr_inline(*(pRendering->pAcquisitions), i, &(pRendering->sLockAcquisitions));
                  assert(NULL != pAcquisition);
                  if (NULL != pAcquisition)
                    {
                      int const CameraID = pAcquisition->CameraID;

                      // Fetch and print camera statistics
                      {
                        double const trigger_time_min = FrameStatisticsGetMin(pAcquisition->pStatisticsTriggerDuration);
                        double const trigger_time_mu = FrameStatisticsGetMean(pAcquisition->pStatisticsTriggerDuration);
                        double const trigger_time_dev = FrameStatisticsGetDeviation(pAcquisition->pStatisticsTriggerDuration);
                        double const trigger_time_max = FrameStatisticsGetMax(pAcquisition->pStatisticsTriggerDuration);

                        if ( (false == isnan_inline(trigger_time_min)) &&
                             (false == isnan_inline(trigger_time_mu)) &&
                             (false == isnan_inline(trigger_time_dev)) &&
                             (false == isnan_inline(trigger_time_max))
                             )
                          {
                            int const cnt = wprintf(
                                                    gMsgTimingStatisticsTriggerTime,
                                                    CameraID + 1,
                                                    trigger_time_min, trigger_time_mu, trigger_time_dev, trigger_time_max
                                                    );
                            assert(0 < cnt);
                          }
                        /* if */

                        double const acquisition_time_min = FrameStatisticsGetMin(pAcquisition->pStatisticsAcquisitionDuration);
                        double const acquisition_time_mu = FrameStatisticsGetMean(pAcquisition->pStatisticsAcquisitionDuration);
                        double const acquisition_time_dev = FrameStatisticsGetDeviation(pAcquisition->pStatisticsAcquisitionDuration);
                        double const acquisition_time_max = FrameStatisticsGetMax(pAcquisition->pStatisticsAcquisitionDuration);
                        if ( (false == isnan_inline(acquisition_time_min)) &&
                             (false == isnan_inline(acquisition_time_mu)) &&
                             (false == isnan_inline(acquisition_time_dev)) &&
                             (false == isnan_inline(acquisition_time_max))
                             )
                          {
                            int const cnt = wprintf(
                                                    gMsgTimingStatisticsAcquisitionTime,
                                                    CameraID + 1,
                                                    acquisition_time_min, acquisition_time_mu, acquisition_time_dev, acquisition_time_max
                                                    );
                            assert(0 < cnt);
                          }
                        /* if */

                        double const acquisition_total_time = FrameStatisticsGetTotalTime(pAcquisition->pStatisticsTriggerFrequency);

                        if ( false == isnan_inline(acquisition_total_time) )
                          {
                            int const cnt = wprintf(gMsgTimingStatisticsTotalTimeCamera, CameraID + 1, acquisition_total_time);
                            assert(0 < cnt);
                          }
                        /* if */

                        double const camera_FPS = FrameStatisticsGetFPS(pAcquisition->pStatisticsTriggerFrequency);

                        if ( false == isnan_inline(camera_FPS) )
                          {
                            int const cnt = wprintf(gMsgTimingStatisticsFPSCamera, CameraID + 1, camera_FPS);
                            assert(0 < cnt);
                          }
                        /* if */
                      }
                    }
                  /* if */
                }
              /* for */
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgInvalidProjector, ProjectorID + 1);
          assert(0 < cnt);
        }
      /* if */
    }
  /* for */
}
/* MainPrintAllTimingStatistics_inline */



//! Print out the number of remaining items for processing.
/*!
  Function counts the number of batch items currently queued in all image encoder queues
  and outputs corresponding message to the console to inform the user about how many batch
  images are still queued. No message is output if all images are processed.

  \param sImageEncoder  Array of all image encoders.
  \param pThreadStorageLock     Access lock.
  \param pRemaining Address where the previous number.
  \return Returns true if the number of remaining images is zero.
*/
inline
bool
MainPrintRemainingItemsForBatchProcessing_inline(
                                                 std::vector<ImageEncoderParameters *> & sImageEncoder,
                                                 SRWLOCK * const pThreadStorageLock,
                                                 int * const pRemaining
                                                 )
{
  bool print_message = false;
  int remaining = 0;

  int const num_enc = (int)( sImageEncoder.size() );

  for (int EncoderID = 0; EncoderID < num_enc; ++ EncoderID)
    {
      ImageEncoderParameters * const pImageEncoder = get_ptr_inline(sImageEncoder, EncoderID, pThreadStorageLock);
      assert(NULL != pImageEncoder);
      if (NULL != pImageEncoder)
        {
          remaining += ImageEncoderBatchItemsRemaining(pImageEncoder);
        }
      /* if */
    }
  /* for */

  if (NULL == pRemaining)
    {
      print_message = (0 < remaining);
    }
  else
    {
      if (0 > *pRemaining)
        {
          *pRemaining = remaining;
          print_message = (0 < remaining);
        }
      else
        {
          int const dst = 5;
          if ( 0 < (*pRemaining/dst) - (remaining/dst) )
            {
              *pRemaining = remaining;
              print_message = (0 < remaining);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  if (true == print_message)
    {
      int const cnt = wprintf(gMsgBatchItemsRemaining, remaining);
      assert(0 < cnt);
    }
  /* if */

  return (0 == remaining);
}
/* MainPrintRemainingItemsForBatchProcessing_inline */

#pragma endregion // Print status information for attached projectors and cameras


#pragma region // Configure image encoders

//! Set output directory for default image encoder.
/*!
  Function sets output directory for default image encoder.

  \param pImageEncoder  Pointer to image encoder.
  \return Function returns true if successfull.
*/
bool
inline
MainSetInitialOutputDirectoryForImageEncoder_inline(
                                                    ImageEncoderParameters * const pImageEncoder
                                                    )
{
  bool savedir = false;

  assert(NULL != pImageEncoder);
  if (NULL == pImageEncoder) return savedir;

  // Test if supplied image encoder is default (first) one.
  assert(0 == pImageEncoder->EncoderID);

  if (false == savedir) savedir = ImageEncoderTrySetDirectory(pImageEncoder, _T("C:\\Output"));
  if (false == savedir) savedir = ImageEncoderTrySetDirectory(pImageEncoder, _T("D:\\Output"));
  if (false == savedir) savedir = ImageEncoderSetDirectory(pImageEncoder, _T("E:\\Output"), NULL);
  
  return savedir;
}
/* MainSetInitialOutputDirectoryForImageEncoder_inline */



//! Set output directory for all image encoders.
/*!
  All images acquired during batch acquisition are stored in a subdirectory of the main
  output directory. All image encoders must use the same subdirectory. This function sets
  the output subdirectory of all image encoders to the same value which is equal to current
  timestamp.

  \param sImageEncoder  Reference to array holding all image encoders.
  \param sAcquisition   Reference to array holding all acquisition threads.
  \param sRendering     Reference to array holding all rendering threads.
  \param pThreadStorageLock     Access lock.
  \param tag    Pointer to acquisition name tag. May be NULL which indicates there is no tag.
  \return Function returns true if successfull, false otherwise.
*/
bool
inline
MainSetOutputDirectoryForImageEncoders_inline(
                                              std::vector<ImageEncoderParameters *> & sImageEncoder,
                                              std::vector<AcquisitionParameters *> & sAcquisition,
                                              std::vector<RenderingParameters *> & sRendering,
                                              SRWLOCK * const pThreadStorageLock,
                                              std::wstring * const tag
                                              )
{
  bool all_set = true;

  ImageEncoderParameters * pFirstImageEncoder = NULL;
  std::wstring * full_path = NULL;

  int const num_enc = (int)( sImageEncoder.size() );
  if (0 >= num_enc) return all_set;

  assert(1 <= num_enc);

  for (int EncoderID = 0; EncoderID < num_enc; ++ EncoderID)
    {
      ImageEncoderParameters * const pImageEncoder = get_ptr_inline(sImageEncoder, EncoderID, pThreadStorageLock);
      assert(NULL != pImageEncoder);
      if (NULL != pImageEncoder)
        {
          bool create_directories = true;

          int const CameraID = pImageEncoder->CameraID;
          AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, pThreadStorageLock);
          assert(NULL != pAcquisition);
          if (NULL != pAcquisition)
            {
              int const ProjectorID = pAcquisition->ProjectorID;
              RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
              assert(NULL != pRendering);
              if (NULL != pRendering)
                {
                  create_directories = pRendering->fSavePNG || pRendering->fSaveRAW;
                }
              /* if */
            }
          /* if */

          if (NULL != pFirstImageEncoder)
            {
              bool const directory_set = ImageEncoderCopyOutputDirectoryNames(pImageEncoder, pFirstImageEncoder);
              assert(true == directory_set);
              all_set = all_set && directory_set;
              if ( (true == directory_set) && (true == create_directories) )
                {
                  std::wstring * tmp_path = ImageEncoderGetOutputDirectory(pImageEncoder, create_directories, false);
                  SAFE_DELETE(tmp_path);
                }
              /* if */
            }
          else
            {
              bool const default_set = ImageEncoderSetSubdirectoryRecordingToTimestamp( pImageEncoder );
              assert(true == default_set);

              bool const tag_set = ImageEncoderAppendToSubdirectoryRecording(pImageEncoder, tag);
              assert(true == tag_set);

              all_set = all_set && default_set && tag_set;

              if (true == default_set)
                {
                  pFirstImageEncoder = pImageEncoder;
                  if (true == create_directories)
                    {
                      std::wstring * tmp_path = ImageEncoderGetOutputDirectory(pImageEncoder, create_directories, false);
                      SAFE_DELETE(tmp_path);
                    }
                  /* if */
                }
              /* if */

              if (true == all_set)
                {
                  assert(NULL == full_path);
                  full_path = ImageEncoderGetOutputDirectory(pImageEncoder, false, true);
                  assert(NULL != full_path);
                  all_set = all_set && (NULL != full_path);
                }
              /* if */
            }
          /* if */
        }
      /* if */

      if (false == all_set) break;
    }
  /* for */

  if (true == all_set)
    {
      if (NULL != full_path)
        {
          int const cnt = wprintf(gMsgBatchOutputDirectory, full_path->c_str());
          assert(0 < cnt);
        }
      /* if */
    }
  /* if */

  SAFE_DELETE(full_path);

  return all_set;
}
/* MainSetOutputDirectoryForImageEncoders_inline */



//! Sets projector resolution.
/*!
  Function copies current projector resolution to all image encoder threads.

  \param sRendering     Array of all rendering thread parameters.
  \param pThreadStorageLock     Access lock.
  \return Function returns true if successfull, false otherwise.
*/
bool
inline
MainSetProjectorSizeForImageEncoders_inline(
                                            std::vector<RenderingParameters *> & sRendering,
                                            SRWLOCK * const pThreadStorageLock
                                            )
{
  bool all_set = true;

  int const num_prj = (int)( sRendering.size() );
  assert( 1 <= num_prj );

  for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, pThreadStorageLock);
      assert(NULL != pRendering);
      if (NULL != pRendering)
        {
          DisplayWindowParameters * const pWindow = pRendering->pWindow;
          assert(NULL != pWindow);
          if (NULL != pWindow)
            {
              int wnd_width = -1;
              int wnd_height = -1;
              RECT rcScreen, rcWindow;

              HRESULT const getres = GetDisplayWindowSize(pWindow, &wnd_width, &wnd_height, &rcScreen, &rcWindow);
              assert( SUCCEEDED(getres) );
              all_set = all_set && SUCCEEDED(getres);

              if (false == all_set) break;

              bool const setres = RenderingThreadSetProjectorSizeForImageEncoders(pRendering, wnd_width, wnd_height, rcScreen, rcWindow);
              assert(true == setres);
              all_set = all_set && setres;
            }
          /* if */
        }
      /* if */

      if (false == all_set) break;
    }
  /* for */

  return all_set;
}
/* MainSetProjectorSizeForImageEncoders_inline */

#pragma endregion // Cofigure image encoders


#pragma region // Cofigure image decoders

//! Extend input queues with black images.
/*!
  Function checks if all input queues have the same number of images and extend them
  with pure black images if needed.

  \param sRenderingWithCamera   Array of all active projectors.
*/
inline
void
MainExtendImageDecoderQueues_inline(
                                    std::vector<RenderingParameters *> & sRenderingWithCamera
                                    )
{
  int const max_i = (int)( sRenderingWithCamera.size() );
  if (0 == max_i) return;

  int num_images_max = INT_MIN;
  int num_images_first = -1;
  bool all_same = true;
  for (int i = 0; i < max_i; ++i)
    {
      RenderingParameters_ * const pRendering = sRenderingWithCamera[i];
      assert(NULL != pRendering);
      if ( (NULL != pRendering) &&
           (NULL != pRendering->pImageDecoder) &&
           (NULL != pRendering->pImageDecoder->pImageList)
           )
        {
          int const num_images = (int)( pRendering->pImageDecoder->pImageList->Size() );
          assert(0 <= num_images);

          if (num_images_max < num_images) num_images_max = num_images;

          if (-1 == num_images_first) num_images_first = num_images;
          all_same = all_same && (num_images_first == num_images);
        }
      /* if */
    }
  /* for */

  if (false == all_same)
    {
      wprintf(gWarningUnequalNumberOfProjectorImages, num_images_max);

      for (int i = 0; i < max_i; ++i)
        {
          RenderingParameters_ * const pRendering = sRenderingWithCamera[i];
          assert(NULL != pRendering);
          if ( (NULL != pRendering) &&
               (NULL != pRendering->pImageDecoder) &&
               (NULL != pRendering->pImageDecoder->pImageList)
               )
            {
              bool const extend = pRendering->pImageDecoder->pImageList->ExtendWithBlackSLPatterns(num_images_max);
              assert(true == extend);
            }
          /* if */
        }
      /* for */
    }
  /* if */
}
/* MainExtendImageDecoderQueues_inline */



//! Removes black images form input queues.
/*!
  Function removes added black SL frames from input queues.

  \param sRenderingWithCamera   Array of all active projectors.
*/
inline
void
MainRestoreImageDecoderQueues_inline(
                                     std::vector<RenderingParameters *> & sRenderingWithCamera
                                     )
{
  int const max_i = (int)( sRenderingWithCamera.size() );
  if (0 == max_i) return;

  for (int i = 0; i < max_i; ++i)
    {
      RenderingParameters_ * const pRendering = sRenderingWithCamera[i];
      assert(NULL != pRendering);
      if ( (NULL != pRendering) &&
           (NULL != pRendering->pImageDecoder) &&
           (NULL != pRendering->pImageDecoder->pImageList)
           )
        {
          bool const remove = pRendering->pImageDecoder->pImageList->RemoveAllBlackSLPatterns();
          assert(true == remove);
        }
      /* if */
    }
  /* for */
}
/* MainRestoreImageDecoderQueues_inline */

#pragma endregion // Cofigure image decoders




/****** MAIN ******/

//! Main function for synchronous acquisition test.
/*!
  This is the main function for synchronous acquisition test.

  \param argc   Number of input arguments.
  \param argv   Array of input arguments strings.
*/
int
_tmain(
       int argc,
       _TCHAR* argv[]
       )
{

  /****** INITIALIZATION ******/

#pragma region // Initialize operating system components

  // Initialize COM library.
  {
    HRESULT const hr = CoInitialize(NULL);
    assert( SUCCEEDED(hr) );
    if ( FAILED(hr) ) return EXIT_FAILURE;
  }

  // Create WIC factory (will be shared between threads).
  IWICImagingFactory * pWICFactory = NULL;
  {
    HRESULT const hr = CoCreateInstance(
                                        CLSID_WICImagingFactory,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_IWICImagingFactory,
                                        (LPVOID*)&pWICFactory
                                        );
    assert( SUCCEEDED(hr) );
    if ( FAILED(hr) ) return EXIT_FAILURE;
  }
  assert(NULL != pWICFactory);

  // Create DXGI factory (will be shared between threads).
  IDXGIFactory1 * pDXGIFactory1 = NULL;
  {
    HRESULT const hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pDXGIFactory1));
    assert( SUCCEEDED(hr) );
    if ( FAILED(hr) ) return EXIT_FAILURE;
  }
  assert(NULL != pDXGIFactory1);

  // Create Direct 2D factory (will be shared between threads).
  ID2D1Factory * pD2DFactory = NULL;
  {
    HRESULT const hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2DFactory);
    assert( SUCCEEDED(hr) );
    if ( FAILED(hr) ) return EXIT_FAILURE;
  }
  assert(NULL != pD2DFactory);

#pragma endregion // Initialize operating system components


  /****** STARTUP ******/

#pragma region // Startup

  // Set thread name.
  SetThreadNameForMSVC(-1, "_tmain");

  // Set console name.
  {
    BOOL const set = SetConsoleTitle(gNameCommandWindow);
    assert(TRUE == set);
  }

  // Create global data storage.
  CreateWindowDataStorage();

  // Create synchronization events.
  SynchronizationEvents * const pSynchronization = CreateSynchronizationEventsStructure();
  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return EXIT_FAILURE;

  // There is only one main thread.
  int const MainID = 0;

  // Create vectors to store pointers to thread data.
  // Batch acquisition application will have several threads
  // depending on the number of connected projectors and cameras.
  std::vector<ImageFileList *> sImageList;
  std::vector<ImageDecoderParameters *> sImageDecoder;
  std::vector<DisplayWindowParameters *> sWindowDisplay;
  std::vector<RenderingParameters *> sRendering;
  std::vector<ImageEncoderParameters *> sImageEncoder;
  std::vector<AcquisitionParameters *> sAcquisition;

  std::vector<std::wstring *> sConnectedCameras;

  SRWLOCK ThreadStorageLock; //!< Storage lock to control concurrent acces to storage parameters.
  InitializeSRWLock( &(ThreadStorageLock) );

  // Print welcome message.
  wprintf(gMsgWelcomeMessage);
  wprintf(L"\n");

  // Get command window handle.
  HWND const hWndCommand = GetForegroundWindow();

#pragma endregion // Startup


  /****** XML CONFIGURATION ******/

  /* Load configuration from XML.
     Here we load both scanner geometry information and general program configuration.
     If configuration cannot be loaded we terminate the program.
  */

#pragma region // Load configuration from XML

  // TODO: Load program configuration from XML.

  // Find geometry configuration.
  std::wstring fname_geometry;
  {
    BOOL fname_exists = FALSE;

    if (FALSE == fname_exists)
      {
        fname_geometry = L"scanner_geometry.xml";
        fname_exists = PathFileExists(fname_geometry.c_str());
      }
    /* if */

    if (FALSE == fname_exists)
      {
        fname_geometry = L"D:\\3DTS\\Data\\scanner_geometry.xml";
        fname_exists = PathFileExists(fname_geometry.c_str());
      }
    /* if */

    if (FALSE == fname_exists)
      {
        fname_geometry = L"C:\\3DTS\\Data\\scanner_geometry.xml";
        fname_exists = PathFileExists(fname_geometry.c_str());
      }
    /* if */

    assert(TRUE == fname_exists);
    if (TRUE != fname_exists) return EXIT_FAILURE;
  }


#pragma endregion // Load configuration from XML


  /****** PREVIEW WINDOW ******/

#pragma region // Create camera preview window

  /* Open camera live preview window. There is only one live preview window per application.
     The message pump for the preview window will be run on a separate thread.
     One DirectX swap chain will be associated with this window.
  */

  PreviewWindowParameters * const pWindowPreview = OpenPreviewWindow(GetModuleHandle(NULL), gNameWindowPreview, _T("D3DCPW"), SW_SHOWNA, NULL);
  assert(NULL != pWindowPreview);
  if (NULL == pWindowPreview) return EXIT_FAILURE;

  // Wait for message pump to start.
  while (false == pWindowPreview->fActive) SleepEx(10, TRUE);

  // Create DirectX device and swap chain associated with the preview window.
  {
    assert(true == pWindowPreview->fActive);
    HRESULT const hr = CreateDirectXDeviceAndSwapChain(pWindowPreview, pDXGIFactory1, pD2DFactory);
    assert( SUCCEEDED(hr) );

    BOOL const pos = SetWindowPos(pWindowPreview->hWnd, HWND_TOP, 100, 100, 700, 500, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW);
    assert(TRUE == pos);
  }

#pragma endregion // Create camera preview window


  /****** VTK WINDOW ******/

#pragma region // Create VTK window

  /* Open VTK rendering window. There is only one VTK rendering window per application.
     The window is used to visualize results of the 3D acquisition.
     The message pump for the VTK rendering window will be run on a separate thread.
  */
  VTKdisplaythreaddata * const pWindowVTK = OpenVTKWindow(NULL, NULL, NULL);
  assert(NULL != pWindowVTK);
  if (NULL == pWindowVTK) return EXIT_FAILURE;

#pragma endregion // Create VTK window


  /****** IMAGE DECODER, RENDER WINDOW, AND REDNERING THREADS ******/

#pragma region // Create first projector

  /* For every attached projector we create one image decoder thread, one DirectX
     rendering window, and one rendering thread. Image decoder thread decodes
     images from pre-selected directory and prepares them for the rendering thread.
     Rendering thread pulls images from the decoder thread, presents them, and
     outputs signals to attached image acquisition threads.
  */

  // First create required events for the threads.
  int const DefaultDecoderID = AddImageDecoderToSynchronizationEventsStructure(pSynchronization);
  assert(0 == DefaultDecoderID);

  int const DefaultProjectorID = AddProjectorToSynchronizationEventsStructure(pSynchronization);
  assert(0 == DefaultProjectorID);

  // IMAGE DECODER THREAD

  /* Create the list of images to display.
     First test in order if one of predefined image directories exist.
     If none exist then query the user to select the directory.
     If no directory is selected abort the program.
  */
  {
    ImageFileList * const pImageList = new ImageFileList();
    assert(NULL != pImageList);
    if (NULL == pImageList) return EXIT_FAILURE;

    bool readdir = false;

    // First try gamma corrected MPS 20:21:25 pattern.
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("C:\\Input\\1280x800 MPS 20+21+25 (all), gamma 2.18")); 
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("E:\\Input\\1280x800 MPS 20+21+25 (all), gamma 2.18"));
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("D:\\Input\\1280x800 MPS 20+21+25 (all), gamma 2.18"));
    
    // Then try gamma corrected MPS 15:19 pattern.
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("C:\\Input\\1280x800 MPS 15+19 (all), gamma 2.18"));
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("E:\\Input\\1280x800 MPS 15+19 (all), gamma 2.18"));
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("D:\\Input\\1280x800 MPS 15+19 (all), gamma 2.18"));

    // Then try GC+PS gamma corrected pattern.
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("C:\\Input\\1280x800 GC+PS (all), gamma 2.18"));
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("E:\\Input\\1280x800 GC+PS (all), gamma 2.18"));
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("D:\\Input\\1280x800 GC+PS (all), gamma 2.18"));

    // Finally, try some obsolete patterns.
    if (false == readdir) readdir = pImageList->TrySetDirectory(_T("C:\\Input\\1280x800 GC+PS (all)"));
    
    assert(true == readdir);
    if (true != readdir) return EXIT_FAILURE;

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sImageList.push_back(pImageList);
      assert( DefaultDecoderID + 1 == sImageList.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );
  }

  // Start image decoder thread and wait for it to become active.
  {
    ImageDecoderParameters * const pImageDecoder =
      ImageDecoderStart(
                        get_ptr_inline(sImageList, DefaultDecoderID, &ThreadStorageLock),
                        pSynchronization,
                        pWICFactory,
                        DefaultDecoderID,
                        DefaultProjectorID
                        );
    assert(NULL != pImageDecoder);
    if (NULL == pImageDecoder) return EXIT_FAILURE;

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sImageDecoder.push_back(pImageDecoder);
      assert( DefaultDecoderID + 1 == sImageDecoder.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

    while (false == pImageDecoder->fActive) SleepEx(10, TRUE);
  }

  // RENDER WINDOW

  /* The message pump for the render window will be run on a separate thread.
     One DirectX swap chain will be associated with this window.
     Open display window for structured light pattern rendering and wait for it to become active.
     Then create DirectX device and swap chain associated with the display window.
  */
  {
    DisplayWindowParameters * const pWindowDisplay = OpenDisplayWindow(GetModuleHandle(NULL), DefaultProjectorID, SW_SHOWNA, NULL, hWndCommand);
    assert(NULL != pWindowDisplay);
    if (NULL == pWindowDisplay) return EXIT_FAILURE;

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sWindowDisplay.push_back(pWindowDisplay);
      assert( DefaultProjectorID + 1 == sWindowDisplay.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

    while (false == pWindowDisplay->fActive) SleepEx(10, TRUE);

    assert(true == pWindowDisplay->fActive);
    HRESULT const hr = CreateDirectXDeviceAndSwapChain(pWindowDisplay, pDXGIFactory1, pD2DFactory);
    assert( SUCCEEDED(hr) );

    BOOL const pos = SetWindowPos(pWindowDisplay->hWnd, HWND_TOP, 50, 50, 800, 600, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW);
    assert(TRUE == pos);
  }

  // RENDERING THREAD

  // Start image rendering thread and wait for it to become active.
  {
    RenderingParameters * const pRendering =
      RenderingThreadStart(
                           pSynchronization,
                           get_ptr_inline(sWindowDisplay, DefaultProjectorID, &ThreadStorageLock),
                           get_ptr_inline(sImageDecoder, DefaultDecoderID, &ThreadStorageLock),
                           DefaultProjectorID
                           );
    assert(NULL != pRendering);
    if (NULL == pRendering) return EXIT_FAILURE;

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sRendering.push_back(pRendering);
      assert( DefaultProjectorID + 1 == sRendering.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

    while (false == pRendering->fActive) SleepEx(10, TRUE);

    DisplayWindowUpdateTitle(pRendering->pWindow);
  }

#pragma endregion // Create first projector


  /****** IMAGE ENCODER AND ACQUISITION THREADS ******/

#pragma region // Create first camera

  /* For each attached camera we create one image encoder and one image acquistion threads.
     The acquisition thread is attached (slaved) to the rendering thread.
  */

  // First create required events for the threads.
  int const DefaultEncoderID = AddImageEncoderToSynchronizationEventsStructure(pSynchronization);
  assert(0 == DefaultEncoderID);

  int const DefaultCameraID = AddCameraToSynchronizationEventsStructure(pSynchronization);
  assert(0 == DefaultCameraID);

  // IMAGE ENCODER

  // Start image encoder thread, set output directory and wait for the thread to become active.
  {
    ImageEncoderParameters * const pImageEncoder = ImageEncoderStart(pSynchronization, pWICFactory, DefaultEncoderID, DefaultCameraID);
    assert(NULL != pImageEncoder);
    if (NULL == pImageEncoder) return EXIT_FAILURE;

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sImageEncoder.push_back(pImageEncoder);
      assert( DefaultEncoderID + 1 == sImageEncoder.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

    bool const savedir = MainSetInitialOutputDirectoryForImageEncoder_inline(pImageEncoder);
    assert(true == savedir);
    if (true != savedir) return EXIT_FAILURE;

    while (false == pImageEncoder->fActive) SleepEx(10, TRUE);
  }

  // ACQUISITION THREAD

  // Activate command window.
  {
    BOOL const top = BringWindowToTop(hWndCommand);
    assert(TRUE == top);
  }

  // Query user to select camera SDK.
  CameraSDK selected_camera_SDK = MainSelectCameraSDK_inline(30000, true, hWndCommand);

  // Start image acquisition thread and wait for it to become active.
  {
    AcquisitionParameters * const pAcquisition =
      AcquisitionThreadStart(
                             pSynchronization,
                             get_ptr_inline(sWindowDisplay, DefaultProjectorID, &ThreadStorageLock),
                             pWindowPreview,
                             get_ptr_inline(sImageEncoder, DefaultEncoderID, &ThreadStorageLock),
                             get_ptr_inline(sImageDecoder, DefaultDecoderID, &ThreadStorageLock),
                             selected_camera_SDK,
                             DefaultCameraID,
                             DefaultProjectorID,
                             &sConnectedCameras,
                             true // Allow fallback to acquisition from file.
                             );
    assert(NULL != pAcquisition);
    if (NULL == pAcquisition) return EXIT_FAILURE;

    // Get unique camera identifier.
    if ( true == IsAcquisitionLive(pAcquisition) )
      {
        std::wstring * const pCameraName = GetUniqueCameraIdentifier(pAcquisition);
        assert(NULL != pCameraName);
        if (NULL != pCameraName) sConnectedCameras.push_back(pCameraName);
      }
    /* if */

    AcquireSRWLockExclusive( &(ThreadStorageLock) );
    {
      sAcquisition.push_back(pAcquisition);
      assert( DefaultCameraID + 1 == sAcquisition.size() );
    }
    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

    while (false == pAcquisition->fActive) SleepEx(10, TRUE);
  }

  // Add camera to the rendering thread.
  {
    bool const add_camera =
      RenderingThreadAddCamera(
                               get_ptr_inline(sRendering, DefaultProjectorID, &ThreadStorageLock),
                               get_ptr_inline(sAcquisition, DefaultCameraID, &ThreadStorageLock)
                               );
    assert(true == add_camera);
  }

  // Set directory for acquisition from file.
  {
    ImageFileList * const pImageList = get_ptr_inline(sImageList, DefaultDecoderID, &ThreadStorageLock);
    assert(NULL != pImageList);
    if (NULL != pImageList)
      {
        bool const matchdir =
          RenderingThreadSetFromFileInputDirectory(
                                                   get_ptr_inline(sRendering, DefaultProjectorID, &ThreadStorageLock),
                                                   pImageList->GetDirectory()
                                                   );
        assert(true == matchdir);
      }
    /* if */
  }

  // Connect acquisition threads to preview window.
  ConnectToAcquisitionThreads(pWindowPreview, &sAcquisition, &ThreadStorageLock, DefaultCameraID);

#pragma endregion // Create first camera


  /****** PREPARE THREADS ******/

#pragma region // Start project-acquire loop

  /* To start the synchronized project-acquire loop we have to raise signals
     to indicate the camera is ready followed by signals to render the first
     structured light pattern.
  */
  {
    BOOL const set_ready = pSynchronization->EventSet(CAMERA_READY, DefaultCameraID);
    assert(0 != set_ready);

    BOOL const set_render_ready = pSynchronization->EventSet(DRAW_RENDER_READY, DefaultProjectorID);
    assert(0 != set_render_ready);

    // Conditions required for DRAW_RENDER to be executed without starting the acquisition loop.
    {
      DisplayWindowParameters * const pWindowDisplay = get_ptr_inline(sWindowDisplay, DefaultProjectorID, &ThreadStorageLock);
      assert(NULL != pWindowDisplay);
      assert(true == pWindowDisplay->fBlocking);
      assert(false == pWindowDisplay->fConcurrentDelay);
    }

    BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, DefaultProjectorID);
    assert(0 != set_render);

    DWORD const dwWaitResult = pSynchronization->EventWaitFor(DRAW_PRESENT_READY, DefaultProjectorID, INFINITE);
    assert(WAIT_OBJECT_0 == dwWaitResult);
  }

  // Disable saving for dummy acquisition.
  if (CAMERA_SDK_FROM_FILE == GetAcquisitionMethod(get_ptr_inline(sAcquisition, DefaultCameraID, &ThreadStorageLock)))
    {
      RenderingParameters * const pRendering = get_ptr_inline(sRendering, DefaultProjectorID, &ThreadStorageLock);
      assert(NULL != pRendering);

      pRendering->fSavePNG = false;
      pRendering->fSaveRAW = false;
    }
  /* if */

  // Start present-acquire cycle.
  BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, DefaultProjectorID);
  assert(0 != set_present);

  // Flag which indicates the present-acquire cycle is active.
  bool continuous_acquisition_active = true;

#pragma endregion // Start project-acquire loop


  /****** USER INTERACTION LOOP ******/

#pragma region // Initialize loop status variables

  // Flag which indicate the statuc of batch acquisition.
  bool batch_active = false; //!< Flag to indicate batch acquisition is active.

  int batch_remaining = 0; //!< Number of images remaining.
  bool batch_all_processed = true; //!< Flag to indicate the end message was displayed.

  // Acquisition parameters.
  bool cfg_fBlocking = true; //!< Flag to indicate we are using blocking acquisition.
  bool cfg_fConcurrentDelay = true; //!< Flag to indicate delay wait and camera exposure are concurrent events.
  bool cfg_fFixed = false; //!< Flag to indicate we are using fixed SL pattern making synchronization unnecessary.

  int cfg_num_acquire = 20; //!< Number of images to acquire when using the fixed SL pattern acquisition.

  bool cfg_save_to_PNG = false; //!< Flag which controls default choice for save to PNG option.
  bool cfg_save_to_RAW = true; //!< Flag which controls default choice for save to RAW option.
  {
    RenderingParameters * const pRendering = get_ptr_inline(sRendering, DefaultProjectorID, &ThreadStorageLock);
    assert(NULL != pRendering);
    if (NULL != pRendering)
      {
        cfg_save_to_PNG = pRendering->fSavePNG;
        cfg_save_to_RAW = pRendering->fSaveRAW;
      }
    /* if */
  }

  std::wstring * pAcquisitionTag = NULL; //!< Acquisition name tag.

  // Parameters for 3D reconstruction.
  double rel_thr = 0.02;
  double dst_thr = 25.0;

  // Print main menu.
  wprintf(L"\n");
  wprintf(gMsgMainMenu);

#pragma endregion // Initialize loop status variables


  // Loop unit user requests exit. Loop is time-sliced; we periodically check if a key is pressed.
  HANDLE rhnd = GetStdHandle(STD_INPUT_HANDLE);
  {
      BOOL const flush = FlushConsoleInputBuffer(rhnd);
      assert(0 != flush);
  }

  wint_t key = 0;
  bool ctrl = false;
  bool exit = false;
  do
    {

#pragma region // Check if user pressed some key

      // Check if a key has been pressed.
      {
        bool key_pressed = false;
        bool ctrl_pressed = false;

        INPUT_RECORD event_buffer;
        DWORD num_read = 0;
        BOOL get_event = FALSE;

        BOOL const peek_event = PeekConsoleInput(rhnd, &event_buffer, 1, &num_read);
        if ( (TRUE == peek_event) && (0 < num_read) )
          {
            get_event = ReadConsoleInput(rhnd, &event_buffer, 1, &num_read);
            //assert(TRUE == get_event);
          }
        /* if */

        if ( (TRUE == get_event) && (1 == num_read) )
          {
            key_pressed = (KEY_EVENT == event_buffer.EventType) && (TRUE == event_buffer.Event.KeyEvent.bKeyDown);
            ctrl_pressed = key_pressed && (0 != ( (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) & event_buffer.Event.KeyEvent.dwControlKeyState ));
          }
        /* if */

        if (false == key_pressed)
          {
            key = 0;
            ctrl = false;
          }
        else
          {
            key = event_buffer.Event.KeyEvent.wVirtualKeyCode;            
            ctrl = (true == ctrl_pressed);

            // Translate numeric keys.
            switch (key)
              {
              case VK_NUMPAD0: key = '0'; break;
              case VK_NUMPAD1: key = '1'; break;
              case VK_NUMPAD2: key = '2'; break;
              case VK_NUMPAD3: key = '3'; break;
              case VK_NUMPAD4: key = '4'; break;
              case VK_NUMPAD5: key = '5'; break;
              case VK_NUMPAD6: key = '6'; break;
              case VK_NUMPAD7: key = '7'; break;
              case VK_NUMPAD8: key = '8'; break;
              case VK_NUMPAD9: key = '9'; break;
              }
            /* switch */
          }
        /* if */
      }

#pragma endregion // Check if user pressed some key


      // Execute appropriate action.
      switch (key)
        {

          //-----------------------------------------------------------------------------------------------------------------
          // Sleep until next keypress.
        default:
          {
            bool const none_remaining = MainPrintRemainingItemsForBatchProcessing_inline(sImageEncoder, &ThreadStorageLock, &batch_remaining);
            if ( (true == none_remaining) && (false == batch_all_processed) )
              {
                batch_remaining = 0;
                batch_all_processed = true;
                wprintf(gMsgBatchItemsAllProcessed);

                // Play sound.
                PlaySound((LPCTSTR)SND_ALIAS_SYSTEMASTERISK, NULL, SND_ASYNC | SND_ALIAS_ID);
              }
            /* if */

            SleepEx(50, TRUE);
          }
          break;

        case (wint_t)(' '):
          //-----------------------------------------------------------------------------------------------------------------
          // Start sequential acquisition.

#pragma region // Start sequential acquisition
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgBatchSequentialNoAttachedCameras);
                break;
              }
            /* if */

            // Close camera configuration dialog.
            CloseCameraConfigurationDialog(pWindowPreview);

            // Indicate batch acquisition is active.
            assert(false == batch_active);
            batch_active = true;

            // Fetch number of projectors.
            int const num_prj = (int)( sRendering.size() );
            assert( 1 <= num_prj );

            // Prepare all projectors.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                // Reset MAIN_* events except MAIN_*_CAMERA events.
                {
                  BOOL const reset = pSynchronization->EventResetAllMain(MainID, ProjectorID, -1);
                  assert(0 != reset);
                }

                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                //assert(NULL != pRendering);

                // Proceed only if projector exists.
                if (NULL == pRendering) continue;

                // Raise MAIN_PREPARE_DRAW signal to start preparation for acquisition.
                {
                  BOOL const prepare = pSynchronization->EventSet(MAIN_PREPARE_DRAW, ProjectorID);
                  assert(0 != prepare);
                }
              }
            /* for */

            // Wait for all projectors to prepare; then set acquisition flags.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                //assert(NULL != pRendering);

                // Proceed only if projector exists.
                if (NULL == pRendering) continue;

                // Wait for preparation to complete.
                {
                  DWORD const wait = pSynchronization->EventWaitFor(MAIN_READY_DRAW, ProjectorID, INFINITE);
                  assert( WAIT_OBJECT_0 == wait );
                }

                bool const have_camera = RenderingThreadHaveCamera(pRendering);

                // Proceed with configuration only if projector has cameras attached.
                if (false == have_camera) continue;

                DisplayWindowParameters * const pWindow = pRendering->pWindow;
                assert(NULL != pWindow);
                if (NULL == pWindow) continue;

                // Set batch acquisition flag; this flag must be reset in the main thread after batch is completed.
                assert(false == pRendering->fBatch);
                pRendering->fBatch = true;

                // Set acquisition mode flags; these flags are auto-reset by the rendering thread once batch is completed.
                pWindow->fBlocking = cfg_fBlocking;
                if (true == cfg_fConcurrentDelay)
                  {
                    double const exposureTime = RenderingThreadGetMaxExposureTimeForAttachedCameras(pRendering) * 0.001; // Convert us to ms.
                    if (exposureTime < pRendering->delay_ms)
                      {
                        pWindow->fConcurrentDelay = true;
                      }
                    else
                      {
                        assert(false == pWindow->fConcurrentDelay);
                      }
                    /* if */
                  }
                /* if */
                pWindow->fFixed = cfg_fFixed;
                pWindow->num_acquire = cfg_num_acquire;
              }
            /* for */

            // Indicate the sequential batch acquisition is starting.
            {
              PlaySound((LPCTSTR)SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ASYNC | SND_ALIAS_ID);

              wprintf(L"\n");
              wprintf(gMsgBatchSequentialBegin);
            }

            // Set output directory and projector sizes for all image encoders.
            {
              wprintf(L"\n");

              bool const set_dir = MainSetOutputDirectoryForImageEncoders_inline(sImageEncoder, sAcquisition, sRendering, &ThreadStorageLock, pAcquisitionTag);
              assert(true == set_dir);

              bool const set_prj = MainSetProjectorSizeForImageEncoders_inline(sRendering, &ThreadStorageLock);
              assert(true == set_prj);
            }

            if (0 < num_prj) wprintf(L"\n");

            // Perform sequential batch acquisition.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                assert(NULL != pRendering);

                bool const have_camera = RenderingThreadHaveCamera(pRendering);
                if (true == have_camera)
                  {
                    DisplayWindowParameters * const pWindow = pRendering->pWindow;
                    assert(NULL != pWindow);
                    if (NULL == pWindow) continue;

                    int num_to_project = 0;
                    int num_to_acquire = 0;
                    bool const getnum = RenderingThreadGetNumberOfImagesToProjectAndAcquire(pRendering, &num_to_project, &num_to_acquire);
                    assert(true == getnum);

                    // Output start message and projector info to console.
                    wprintf(gMsgBatchSequentialProjectorBegin, ProjectorID + 1);
                    if ( (true == pWindow->fBlocking) && (true == pWindow->fConcurrentDelay) )
                      {
                        wprintf(gMsgBatchUsingConcurrentDelay, ProjectorID + 1);
                      }
                    /* if */
                    wprintf(gMsgBatchSequentialProjectorNumberOfImages, ProjectorID + 1, num_to_project, num_to_acquire);

                    // Check signal status.
                    assert( false == DebugIsSignalled(pSynchronization, MAIN_BEGIN, ProjectorID) );
                    assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );
                    assert( false == DebugIsSignalled(pSynchronization, MAIN_RESUME_DRAW, ProjectorID) );

                    // Raise begin signal.
                    BOOL const set_begin = pSynchronization->EventSet(MAIN_BEGIN, ProjectorID);
                    assert(0 != set_begin);

                    // Wait for the batch acquisition to complete.
                    DWORD const wait_end = pSynchronization->EventWaitFor(MAIN_END_DRAW, ProjectorID, INFINITE);
                    assert(WAIT_OBJECT_0 == wait_end);

                    // Disarm ending signal.
                    BOOL const reset_end = pSynchronization->EventReset(MAIN_END_DRAW, ProjectorID);
                    assert(0 != reset_end);

                    // Output end message to console.
                    wprintf(gMsgBatchSequentialProjectorEnd, ProjectorID + 1);
                  }
                else
                  {
                    wprintf(gMsgBatchSequentialProjectorSkip, ProjectorID + 1);
                  }
                /* if */
              }
            /* for */

            // Indicate the sequential batch acquisiton has ended.
            {
              PlaySound((LPCTSTR)SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ASYNC | SND_ALIAS_ID);

              wprintf(L"\n");
              wprintf(gMsgBatchSequentialEnd);
            }

            // Print statistics.
            {
              wprintf(L"\n");
              MainPrintAllTimingStatistics_inline(sRendering, &ThreadStorageLock);
            }

            // Resume project-acquire cycle.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                if (NULL == pRendering) continue;

                // Enable cycling; cycling is always disabled by MAIN_PREPARE_DRAW event.
                bool const set_cycle = RenderingThreadSetCycleFlagForImageDecoder(pRendering, true);
                assert(true == set_cycle);

                bool const have_camera = RenderingThreadHaveCamera(pRendering);
                if (true == have_camera)
                  {
                    // Disarm batch acquisition.
                    assert(true == pRendering->fBatch);
                    pRendering->fBatch = false;

                    // Enable live view; live view is always deactivated by MAIN_PREPARE_DRAW event.
                    bool const enable_live_view = RenderingThreadSetLiveViewForAttachedCameras(pRendering, true);
                    assert(true == enable_live_view);

                    // Send MAIN_RESUME_DRAW to restart project-acquire cycle.
                    assert( false == DebugIsSignalled(pSynchronization, MAIN_RESUME_DRAW, ProjectorID) );

                    BOOL const set_resume = pSynchronization->EventSet(MAIN_RESUME_DRAW, ProjectorID);
                    assert(0 != set_resume);
                  }
                else
                  {
                    // Projectors with no cameras are restarted normally.
                    MainStartContinuousAcquisition_inline(pRendering);
                  }
                /* if */
              }
            /* for */

            // Project-acquire cycle is now active.
            continuous_acquisition_active = true;

            // Indicate the batch acquisition is not active.
            assert(true == batch_active);
            batch_active = false;

            // Check if all acquired images were processed.
            batch_remaining = -1;
            batch_all_processed = MainPrintRemainingItemsForBatchProcessing_inline(sImageEncoder, &ThreadStorageLock, &batch_remaining);
            if (true == batch_all_processed)
              {
                assert(0 == batch_remaining);
                wprintf(gMsgBatchItemsAllProcessed);
              }
            /* if */
          }
#pragma endregion // Start sequential acquisition

          break;

        case (wint_t)(13):
          //-----------------------------------------------------------------------------------------------------------------
          // Start simultaneous acquisition.

#pragma region // Start simultaneous acquisition
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgBatchSimultaneousNoAttachedCameras);
                break;
              }
            /* if */

            // Close camera configuration dialog.
            CloseCameraConfigurationDialog(pWindowPreview);

            // Indicate batch acquisition is active.
            assert(false == batch_active);
            batch_active = true;

            // Fetch number of projectors.
            int const num_prj = (int)( sRendering.size() );
            assert( 1 <= num_prj );

            // Prepare all projectors.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                // Reset MAIN_* events except MAIN_*_CAMERA events.
                {
                  BOOL const reset = pSynchronization->EventResetAllMain(MainID, ProjectorID, -1);
                  assert(0 != reset);
                }

                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                //assert(NULL != pRendering);

                // Proceed only if projector exists.
                if (NULL == pRendering) continue;

                // Raise MAIN_PREPARE_DRAW signal to start preparation for acquisition.
                {
                  BOOL const prepare = pSynchronization->EventSet(MAIN_PREPARE_DRAW, ProjectorID);
                  assert(0 != prepare);
                }
              }
            /* for */

            // Wait for all projectors to prepare and collect parameters required for configuration.
            int num_prj_with_camera = 0; // Count projectors with active camera.
            double exposureTime_max = -DBL_MAX; // Get largest exposure time.
            double delay_ms_min = DBL_MAX; // Get shortest delay time.
            std::vector<RenderingParameters *> sRenderingWithCamera;
            sRenderingWithCamera.reserve(num_prj);
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                //assert(NULL != pRendering);

                // Proceed only if projector exists.
                if (NULL == pRendering) continue;

                // Wait for preparation to complete.
                {
                  DWORD const wait = pSynchronization->EventWaitFor(MAIN_READY_DRAW, ProjectorID, INFINITE);
                  assert( WAIT_OBJECT_0 == wait );
                }

                bool const have_camera = RenderingThreadHaveCamera(pRendering);

                // Proceed with configuration only if projector has cameras attached.
                if (false == have_camera) continue;

                DisplayWindowParameters * const pWindow = pRendering->pWindow;
                assert(NULL != pWindow);
                if (NULL == pWindow) continue;

                // Count valid projectors which control at least one camera.
                ++num_prj_with_camera;
                sRenderingWithCamera.push_back(pRendering);

                // Get maximal exposure time and minimal delay time.
                double const exposureTime = RenderingThreadGetMaxExposureTimeForAttachedCameras(pRendering) * 0.001; // Convert us to ms.
                if (exposureTime > exposureTime_max) exposureTime_max = exposureTime;

                double const delay_ms = pRendering->delay_ms;
                if (delay_ms < delay_ms_min) delay_ms_min = delay_ms;
              }
            /* for */

            // Indicate the simultaneous batch acquisition is starting.
            {
              PlaySound((LPCTSTR)SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ASYNC | SND_ALIAS_ID);

              wprintf(L"\n");
              wprintf(gMsgBatchSimultaneousBegin);
            }

            // Set output directory and projector sizes for all image encoders.
            {
              wprintf(L"\n");

              bool const set_dir = MainSetOutputDirectoryForImageEncoders_inline(sImageEncoder, sAcquisition, sRendering, &ThreadStorageLock, pAcquisitionTag);
              assert(true == set_dir);

              bool const set_prj = MainSetProjectorSizeForImageEncoders_inline(sRendering, &ThreadStorageLock);
              assert(true == set_prj);
            }

            if (0 < num_prj) wprintf(L"\n");

            // Configure acquisition flags.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                bool const have_camera = RenderingThreadHaveCamera(pRendering);

                // Proceed with configuration only if projector has cameras attached.
                if (false == have_camera) continue;

                DisplayWindowParameters * const pWindow = pRendering->pWindow;
                assert(NULL != pWindow);
                if (NULL == pWindow) continue;

                // Set rendering parameters; these must be reset in the main thread after batch is completed.
                assert(false == pRendering->fBatch);
                pRendering->fBatch = true;

                assert(false == pRendering->fSynchronize);
                pRendering->fSynchronize = (1 < num_prj_with_camera);

                pRendering->num_prj = num_prj_with_camera;

                bool const add_projectors = RenderingThreadAddProjectors(pRendering, &sRenderingWithCamera);
                assert(true == add_projectors);

                // Set acquisition mode flags; these flags are auto-reset by the rendering thread once batch is completed.
                pWindow->fBlocking = cfg_fBlocking;
                if (true == cfg_fConcurrentDelay)
                  {
                    if (exposureTime_max < delay_ms_min)
                      {
                        pWindow->fConcurrentDelay = true;
                      }
                    else
                      {
                        assert(false == pWindow->fConcurrentDelay);
                      }
                    /* if */
                  }
                /* if */
                pWindow->fFixed = cfg_fFixed;
                pWindow->num_acquire = cfg_num_acquire;

                int num_to_project = 0;
                int num_to_acquire = 0;
                bool const getnum = RenderingThreadGetNumberOfImagesToProjectAndAcquire(pRendering, &num_to_project, &num_to_acquire);
                assert(true == getnum);

                // Output projector info to console.
                if ( (true == pWindow->fBlocking) && (true == pWindow->fConcurrentDelay) )
                  {
                    wprintf(gMsgBatchUsingConcurrentDelay, ProjectorID + 1);
                  }
                /* if */
                wprintf(gMsgBatchSimultaneousProjectorNumberOfImages, ProjectorID + 1, num_to_project, num_to_acquire);
              }
            /* for */

            // Set start counter values.
            {
              BOOL const set_counter_sync_present = pSynchronization->SetStartCounterValue(DRAW_SYNC_PRESENT, MainID, num_prj_with_camera, true);
              assert(TRUE == set_counter_sync_present);

              BOOL const set_counter_sync_vblank = pSynchronization->SetStartCounterValue(DRAW_SYNC_VBLANK, MainID, num_prj_with_camera, true);
              assert(TRUE == set_counter_sync_vblank);

              BOOL const set_counter_sync_triggers = pSynchronization->SetStartCounterValue(DRAW_SYNC_TRIGGERS, MainID, num_prj_with_camera, true);
              assert(TRUE == set_counter_sync_triggers);
            }

            // Extend input queues for all active projectors.
            MainExtendImageDecoderQueues_inline(sRenderingWithCamera);

            // Dispatch starting signals to all active projectors.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                bool const have_camera = RenderingThreadHaveCamera(pRendering);

                // Proceed with configuration only if projector has cameras attached.
                if (false == have_camera) continue;

                // Check signal status.
                assert( false == DebugIsSignalled(pSynchronization, MAIN_BEGIN, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, MAIN_RESUME_DRAW, ProjectorID) );

                // Raise begin signal.
                BOOL const set_begin = pSynchronization->EventSet(MAIN_BEGIN, ProjectorID);
                assert(0 != set_begin);
              }
            /* for */

            // Wait for all active projectors to acquire images.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                bool const have_camera = RenderingThreadHaveCamera(pRendering);

                // Proceed with configuration only if projector has cameras attached.
                if (false == have_camera) continue;

                // Wait for the batch acquisition to complete.
                DWORD const wait_end = pSynchronization->EventWaitFor(MAIN_END_DRAW, ProjectorID, INFINITE);
                assert(WAIT_OBJECT_0 == wait_end);

                // Disarm ending signal.
                BOOL const reset_end = pSynchronization->EventReset(MAIN_END_DRAW, ProjectorID);
                assert(0 != reset_end);

                bool const remove_projectors = RenderingThreadRemoveProjectors(pRendering);
                assert(true == remove_projectors);
              }
            /* for */

            // Indicate the simultaneous batch acquisiton has ended.
            {
              PlaySound((LPCTSTR)SND_ALIAS_SYSTEMEXCLAMATION, NULL, SND_ASYNC | SND_ALIAS_ID);

              wprintf(L"\n");
              wprintf(gMsgBatchSimultaneousEnd);
            }

            // Restore input queues.
            MainRestoreImageDecoderQueues_inline(sRenderingWithCamera);

            // Print statistics.
            {
              wprintf(L"\n");
              MainPrintAllTimingStatistics_inline(sRendering, &ThreadStorageLock);
            }

            // Resume project-acquire cycle.
            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                if (NULL == pRendering) continue;

                // Enable cycling; cycling is always disabled by MAIN_PREPARE_DRAW event.
                bool const set_cycle = RenderingThreadSetCycleFlagForImageDecoder(pRendering, true);
                assert(true == set_cycle);

                bool const have_camera = RenderingThreadHaveCamera(pRendering);
                if (true == have_camera)
                  {
                    // Reset rendering parameters.
                    (true == pRendering->fBatch);
                    pRendering->fBatch = false;
                    pRendering->fSynchronize = false;
                    pRendering->num_prj = -1;

                    // Enable live view; live view is always deactivated by MAIN_PREPARE_DRAW event.
                    bool const enable_live_view = RenderingThreadSetLiveViewForAttachedCameras(pRendering, true);
                    assert(true == enable_live_view);

                    // Send MAIN_RESUME_DRAW to restart project-acquire cycle.
                    assert( false == DebugIsSignalled(pSynchronization, MAIN_RESUME_DRAW, ProjectorID) );

                    BOOL const set_resume = pSynchronization->EventSet(MAIN_RESUME_DRAW, ProjectorID);
                    assert(0 != set_resume);
                  }
                else
                  {
                    // Projectors with no cameras are restarted normally.
                    MainStartContinuousAcquisition_inline(pRendering);
                  }
                /* if */
              }
            /* for */

            // Project-acquire cycle is now active.
            continuous_acquisition_active = true;

            // Indicate the batch acquisition is not active.
            assert(true == batch_active);
            batch_active = false;

            // Check if all acquired images were processed.
            batch_remaining = -1;
            batch_all_processed = MainPrintRemainingItemsForBatchProcessing_inline(sImageEncoder, &ThreadStorageLock, &batch_remaining);
            if (true == batch_all_processed)
              {
                assert(0 == batch_remaining);
                wprintf(gMsgBatchItemsAllProcessed);
              }
            /* if */
          }
#pragma endregion // Start simultaneous acquisition

          break;

        case (wint_t)('0'):
          //-----------------------------------------------------------------------------------------------------------------
          // Print system configuration.

#pragma region // Print system configuration
          {
            // Fetch number of projectors.
            int const num_prj = (int)( sRendering.size() );
            assert( 1 <= num_prj );

            int const num_cam = (int)( sAcquisition.size() );
            assert( 0 <= num_cam );

            int const num_enc = (int)( sImageEncoder.size() );
            assert( 0 <= num_enc);

            wprintf(L"\n");
            wprintf(gMsgSystemConfiguration);

            wprintf(L"\n");
            MainPrintBatchConfiguration_inline(cfg_fBlocking, cfg_fConcurrentDelay, cfg_fFixed, cfg_num_acquire);

            wprintf(L"\n");
            MainPrintAllContainingDisplays_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllResolutions_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllInputDirectories_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllSaveToFile_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllBlockingDelays_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllNonBlockingDelays_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllNonBlockingPresentTimes_inline(sRendering, &ThreadStorageLock);

            if (1 < num_prj) wprintf(L"\n");
            MainPrintAllAttachedCameras_inline(sRendering, &ThreadStorageLock);

            if (0 < num_cam) wprintf(L"\n");
            MainPrintAllExposureMultipliers_inline(sAcquisition, &ThreadStorageLock);

            if (0 < num_enc) wprintf(L"\n");
            MainPrintOutputDirectory_inline(sImageEncoder, &ThreadStorageLock, DefaultEncoderID, pAcquisitionTag);

          }
#pragma endregion // Print system configuration

          break;

        case (wint_t)('1'):
          //-----------------------------------------------------------------------------------------------------------------
          // Configure acquisition.

#pragma region // Configure acquisition
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

          MENU_ITEM_CHANGE_ACQUISITION_PARAMETERS:

            int const timeout_ms = 10000;

            // Query user for action.
            wprintf(L"\n");
            wprintf(
                    gMsgBatchConfigurationSubmenu,
                    (true == cfg_fBlocking)? gMsgStringEnabled : gMsgStringDisabled,
                    (true == cfg_fConcurrentDelay)? gMsgStringEnabled : gMsgStringDisabled,
                    (true == cfg_fFixed)? gMsgStringEnabled : gMsgStringDisabled,
                    cfg_num_acquire
                    );

            int const pressed_key = TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL);

            wprintf(L"\n");

            if (1 == pressed_key)
              {
                cfg_fBlocking = !cfg_fBlocking;
                if (true == cfg_fBlocking)
                  {
                    wprintf(gMsgBatchConfigurationBlockingModeEnabled);
                  }
                else
                  {
                    wprintf(gMsgBatchConfigurationBlockingModeDisabled);
                    wprintf(gWarningBatchFrameDropPossible);
                  }
                /* if */

                goto MENU_ITEM_CHANGE_ACQUISITION_PARAMETERS;
              }
            else if (2 == pressed_key)
              {
                cfg_fConcurrentDelay = !cfg_fConcurrentDelay;
                if (true == cfg_fConcurrentDelay)
                  {
                    wprintf(gMsgBatchConfigurationConcurrentDelayEnabled);
                  }
                else
                  {
                    wprintf(gMsgBatchConfigurationConcurrentDelayDisabled);
                  }
                /* if */

                goto MENU_ITEM_CHANGE_ACQUISITION_PARAMETERS;
              }
            else if (3 == pressed_key)
              {
                cfg_fFixed = !cfg_fFixed;
                if (true == cfg_fFixed)
                  {
                    wprintf(gMsgBatchConfigurationFixedSLPatternEnabled);
                  }
                else
                  {
                    wprintf(gMsgBatchConfigurationFixedSLPatternDisabled);
                  }
                /* if */

                goto MENU_ITEM_CHANGE_ACQUISITION_PARAMETERS;
              }
            else if (4 == pressed_key)
              {
                int const cfg_num_acquire_old = cfg_num_acquire;

                wprintf(gMsgBatchConfigurationNumAcquirePrint, cfg_num_acquire_old);

                wprintf(gMsgBatchConfigurationNumAcquireQuery);
                int cfg_num_acquire_new = cfg_num_acquire_old;
                int const scan = scanf_s("%d", &cfg_num_acquire_new);
                if ( (1 == scan) && (0 < cfg_num_acquire_new) && (cfg_num_acquire_old != cfg_num_acquire_new) )
                  {
                    cfg_num_acquire = cfg_num_acquire_new;
                    wprintf(gMsgBatchConfigurationNumAcquireChanged, cfg_num_acquire_old, cfg_num_acquire_new);
                  }
                else
                  {
                    wprintf(gMsgBatchConfigurationNumAcquireNotChanged, cfg_num_acquire_old);
                  }
                /* if */

                goto MENU_ITEM_CHANGE_ACQUISITION_PARAMETERS;
              }
            else
              {
                wprintf(gMsgBatchConfigurationNoChange);

                wprintf(L"\n");
                wprintf(gMsgMainMenu);
              }
            /* if */
          }
#pragma endregion // Configure acquisition

          break;


        case (wint_t)('2'):
          //-----------------------------------------------------------------------------------------------------------------
          // Change input directory for selected projector (key 2) or for selected camera (key CTRL+2).

#pragma region // Select input directory
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            if (false == ctrl)
              {
                int const ProjectorID = MainSelectProjectorID_inline((int)(sRendering.size()), DefaultProjectorID, 10000, hWndCommand);

                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                assert(NULL != pRendering);

                wprintf(L"\n");

                if (NULL != pRendering)
                  {
                    bool const set = RenderingThreadAskUserToSetInputDirectory(pRendering);
                    //assert(true == set);
                    if (true == set)
                      {
                        TCHAR const * const directory = pRendering->pImageDecoder->pImageList->GetDirectory();
                        assert(NULL != directory);
                        if (NULL != directory)
                          {
                            int const num_images = (int)( pRendering->pImageDecoder->pImageList->Size() );

                            wprintf(gMsgSetInputDirectoryForProjector, ProjectorID + 1, directory, ProjectorID + 1, num_images);

                            bool const matchdir = RenderingThreadSetFromFileInputDirectory(pRendering, directory);
                            assert(true == matchdir);
                          }
                        /* if */
                      }
                    /* if */

                    DisplayWindowUpdateTitle(pRendering->pWindow);
                    PreviewWindowUpdateTitle(pWindowPreview);
                  }
                else
                  {
                    wprintf(gMsgInvalidProjector, ProjectorID + 1);
                  }
                /* if */
              }
            else
              {
                int const CameraID = MainSelectCameraID_inline((int)(sAcquisition.size()), DefaultCameraID, 10000, hWndCommand);

                AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock);
                assert(NULL != pAcquisition);

                wprintf(L"\n");

                if (NULL != pAcquisition)
                  {
                    if (NULL != pAcquisition->pFromFile)
                      {
                        bool const set = AcquisitionParametersFromFileSetDirectory(pAcquisition->pFromFile, NULL);
                        //assert(true == set);

                        wchar_t const * const directory = AcquisitionParametersFromFileGetDirectory(pAcquisition->pFromFile);
                        assert(NULL != directory);

                        if (true == set)
                          {
                            int const num_images = (int)( pAcquisition->pFromFile->pFileList->Size() );

                            wprintf(gMsgSetInputDirectoryForCamera, CameraID + 1, directory, CameraID + 1, num_images);
                          }
                        else
                          {
                            wprintf(gMsgSetInputDirectoryForCameraUnchanged, CameraID + 1, directory);
                          }
                        /* if */

                        PreviewWindowUpdateTitle(pWindowPreview);
                      }
                    else
                      {
                        wprintf(gMsgSetInputDirectoryForCameraNotFromFile, CameraID + 1);
                      }
                    /* if */
                  }
                else
                  {
                    wprintf(gMsgInvalidCamera, CameraID + 1);
                  }
                /* if */
              }
            /* if */
          }
#pragma endregion // Select input directory

          break;


        case (wint_t)('i'):
        case (wint_t)('I'):
          //-----------------------------------------------------------------------------------------------------------------
          // Rescan all input directories.

#pragma region // Rescan input directories
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            if (false == ctrl)
              {
                int const num_prj = (int)( sRendering.size() );
                assert( 1 <= num_prj );

                if (0 < num_prj) wprintf(L"\n");

                for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
                  {
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    if (NULL != pRendering)
                      {
                        bool const rescan = RenderingThreadRescanInputDirectory(pRendering);
                        //assert(true == rescan);
                        if (true == rescan)
                          {
                            TCHAR const * const directory = pRendering->pImageDecoder->pImageList->GetDirectory();
                            assert(NULL != directory);
                            if (NULL != directory)
                              {
                                int const num_images = (int)( pRendering->pImageDecoder->pImageList->Size() );

                                wprintf(gMsgRescanInputDirectoryProjector, ProjectorID + 1, num_images, directory);
                              }
                            /* if */
                          }
                        /* if */
                      }
                    else
                      {
                        wprintf(gMsgInvalidProjector, ProjectorID + 1);
                      }
                    /* if */
                  }
                /* for */
              }
            else
              {
                int const num_cam = (int)( sAcquisition.size() );

                if (0 < num_cam) wprintf(L"\n");

                for (int CameraID = 0; CameraID < num_cam; ++CameraID)
                  {
                    AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock);
                    assert(NULL != pAcquisition);
                    if (NULL == pAcquisition)
                      {
                        wprintf(gMsgInvalidCamera, CameraID + 1);
                        continue;
                      }
                    /* if */

                    CameraSDK const camera_sdk = GetAcquisitionMethod(pAcquisition);
                    if (camera_sdk != CAMERA_SDK_FROM_FILE)
                      {
                        wprintf(gMsgSetInputDirectoryForCameraNotFromFile, CameraID + 1);
                        continue;
                      }
                    /* if */

                    bool const rescan = AcquisitionThreadRescanInputDirectory(pAcquisition);
                    assert(true == rescan);
                    if (true == rescan)
                      {
                        TCHAR const * const directory = pAcquisition->pFromFile->pFileList->GetDirectory();
                        assert(NULL != directory);
                        if (NULL != directory)
                          {
                            int const num_images = (int)( pAcquisition->pFromFile->pFileList->Size() );

                            wprintf(gMsgRescanInputDirectoryCamera, CameraID + 1, num_images, directory);
                          }
                        /* if */
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */
          }
#pragma endregion // Rescan input directories

        break;


        case (wint_t)('3'):
          //-----------------------------------------------------------------------------------------------------------------
          // Set session directory (key 3) or change output output directory (key CTRL+3).

#pragma region // Select output directory
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgOutputDirectoryNoAttachedCameras);
                break;
              }
            /* if */

            /* All decoders share the same output directory. Therefore we query the user once to select
               the output directory for the default encoder. If the choice is valid we propagate that
               choice to all other decoders.
            */

            ImageEncoderParameters * const pDefaultImageEncoder = get_ptr_inline(sImageEncoder, DefaultEncoderID, &ThreadStorageLock);
            //assert(NULL != pDefaultImageEncoder);
            if (NULL == pDefaultImageEncoder)
              {
                wprintf(gMsgOutputDirectoryNoAttachedCameras);
                break;
              }
            /* if */

            if (false == ctrl)
              {
                std::wstring * pSubdirectorySessionOld = ImageEncoderGetSubdirectorySession(pDefaultImageEncoder);
                std::wstring * pSubdirectorySessionNew = NULL;
                bool session_changed = false;

                wprintf(L"\n");

                if (NULL != pSubdirectorySessionOld)
                  {
                    wprintf(gMsgSetSessionSubdirectoryPrintDefined, pSubdirectorySessionOld->c_str());
                  }
                else
                  {
                    wprintf(gMsgSetSessionSubdirectoryPrintUndefined);
                  }
                /* if */

                wprintf(gMsgSetSessionSubdirectoryQuery);
                int const buffer_sz = 1024;
                wchar_t buffer[buffer_sz + 1];
                wchar_t * const scan = _getws_s(buffer, (unsigned)_countof(buffer));
                if (NULL != scan)
                  {
                    buffer[buffer_sz] = 0;

                    // Copy user input to string and trim whitespaces and tabs.
                    // TODO: Test input string for invalid characters which are prohibited in directory names.
                    assert(NULL == pSubdirectorySessionNew);
                    pSubdirectorySessionNew = new std::wstring(buffer);
                    assert(NULL != pSubdirectorySessionNew);

                    pSubdirectorySessionNew->erase(0, pSubdirectorySessionNew->find_first_not_of(L" \t"));
                    pSubdirectorySessionNew->erase(pSubdirectorySessionNew->find_last_not_of(L" \t") + 1);

                    bool const is_empty = (0 == pSubdirectorySessionNew->size());
                    bool are_equal = false;
                    if (NULL == pSubdirectorySessionOld)
                      {
                        if (true == is_empty) are_equal = true;
                      }
                    else
                      {
                        are_equal = (0 == _wcsicmp(pSubdirectorySessionNew->c_str(), pSubdirectorySessionOld->c_str()));
                      }
                    /* if */

                    // Compare to old value.
                    if (false == are_equal)
                      {
                        if (true == is_empty)
                          {
                            SAFE_DELETE( pSubdirectorySessionNew );
                          }
                        /* if */

                        bool const set_default = ImageEncoderSetSubdirectorySession(pDefaultImageEncoder, pSubdirectorySessionNew);
                        assert(true == set_default);
                        session_changed = set_default;

                        int const num_enc = (int)( sImageEncoder.size() );
                        assert(1 <= num_enc);
                        for (int i = 0; i < num_enc; ++i)
                          {
                            if (i != DefaultEncoderID)
                              {
                                ImageEncoderParameters * const pImageEncoder = get_ptr_inline(sImageEncoder, i, &ThreadStorageLock);
                                bool const copydir = ImageEncoderCopyOutputDirectoryNames(pImageEncoder, pDefaultImageEncoder);
                                assert(true == copydir);
                                session_changed = session_changed && copydir;
                              }
                            /* if */
                          }
                        /* for */
                      }
                    /* if */
                  }
                /* if */

                if (true == session_changed)
                  {
                    if ( (NULL != pSubdirectorySessionOld) && (NULL != pSubdirectorySessionNew) )
                      {
                        wprintf(gMsgSetSessionSubdirectoryChanged, pSubdirectorySessionOld->c_str(), pSubdirectorySessionNew->c_str());
                      }
                    else if ( (NULL != pSubdirectorySessionOld) && (NULL == pSubdirectorySessionNew) )
                      {
                        wprintf(gMsgSetSessionSubdirectoryChangedNoDestination, pSubdirectorySessionOld->c_str());
                      }
                    else if ( (NULL == pSubdirectorySessionOld) && (NULL != pSubdirectorySessionNew) )
                      {
                        wprintf(gMsgSetSessionSubdirectoryChangedNoSource, pSubdirectorySessionNew->c_str());
                      }
                    /* if */
                  }
                else
                  {
                    if (NULL != pSubdirectorySessionOld)
                      {
                        wprintf(gMsgSetSessionSubdirectoryUnchangedDefined, pSubdirectorySessionOld->c_str());
                      }
                    else
                      {
                        wprintf(gMsgSetSessionSubdirectoryUnchangedUndefined);
                      }
                    /* if */
                  }
                /* if */

                SAFE_DELETE( pSubdirectorySessionOld );
                SAFE_DELETE( pSubdirectorySessionNew );

              }
            else
              {
                bool const defaultsavedir = ImageEncoderSetDirectory(pDefaultImageEncoder, NULL, NULL);
                //assert(true == defaultsavedir);

                if ( (true == defaultsavedir) && (NULL != pDefaultImageEncoder) )
                  {
                    wchar_t const * const directory = ImageEncoderGetDirectory(pDefaultImageEncoder);
                    if (NULL != directory)
                      {
                        int const num_enc = (int)( sImageEncoder.size() );
                        assert(1 <= num_enc);
                        for (int i = 0; i < num_enc; ++i)
                          {
                            if (i != DefaultEncoderID)
                              {
                                ImageEncoderParameters * const pImageEncoder = get_ptr_inline(sImageEncoder, i, &ThreadStorageLock);
                                bool const savedir = ImageEncoderSetDirectory(pImageEncoder, directory, NULL);
                                assert(true == savedir);
                              }
                            /* if */
                          }
                        /* for */

                        wprintf(L"\n");
                        wprintf(gMsgOutputDirectoryChanged, directory);

                      }
                    /* if */
                  }
                /* if */
              }
            /* if (false == ctrl) */
          }
#pragma endregion // Select output directory

          break;


        case (wint_t)('4'):
          //-----------------------------------------------------------------------------------------------------------------
          // Change fullscreen display resolution and refresh rate.

#pragma region // Select display mode
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Query user to select which projector to configure.
            int const ProjectorID = MainSelectProjectorID_inline((int)(sRendering.size()), DefaultProjectorID, 10000, hWndCommand);

            RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
            assert(NULL != pRendering);
            if (NULL != pRendering)
              {
                DisplayWindowParameters * const pWindow = pRendering->pWindow;
                assert(NULL != pWindow);
                if (NULL != pWindow)
                  {
                    HRESULT const hr = QueryUserToSelectDisplayMode(pWindow, ProjectorID, NULL);
                    //assert( SUCCEEDED(hr) );
                  }
                else
                  {
                    wprintf(L"\n");
                    wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
                  }
                /* if */
              }
            else
              {
                wprintf(L"\n");
                wprintf(gMsgInvalidProjector, ProjectorID + 1);
              }
            /* if */
          }
#pragma endregion // Select display mode

          break;


        case (wint_t)('5'):
          //-----------------------------------------------------------------------------------------------------------------
          // Change exposure time multiplier.

#pragma region // Change exposure time
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgExposureNoAttachedCameras);
                break;
              }
            /* if */

            // Query user to select which camera to configure.
            int const CameraID = MainSelectCameraID_inline((int)(sAcquisition.size()), DefaultCameraID, 10000, hWndCommand);

            // Change exposure multiplier for selected camera.
            {
              AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock);
              assert(NULL != pAcquisition);

              wprintf(L"\n");

              if (NULL != pAcquisition)
                {
                  assert(CameraID == pAcquisition->CameraID);
                  double const multiplier_old = pAcquisition->k;
                  double const exposureTime_old = CameraExposureTimeFromRefreshRate(pAcquisition);
                  wprintf(gMsgExposureMultiplierPrint, CameraID + 1, multiplier_old, exposureTime_old);

                  wprintf(gMsgExposureMultiplierQuery, CameraID + 1);
                  double multiplier = 1.0;
                  int const scan = scanf_s("%lf", &multiplier);
                  if ( (1 == scan) && (0 < multiplier) && (multiplier_old != multiplier) )
                    {
                      pAcquisition->k = multiplier;
                      double const multiplier_new = pAcquisition->k;
                      wprintf(gMsgExposureMultiplierChanged, CameraID + 1, multiplier_old, multiplier_new);
                    }
                  /* if */
                }
              else
                {
                  wprintf(gMsgInvalidCamera, CameraID + 1);
                }
              /* if */
            }

            wprintf(L"\n");
            MainPrintAllExposureMultipliers_inline(sAcquisition, &ThreadStorageLock);
          }
#pragma endregion // Change exposure time

          break;


        case (wint_t)('6'):
          //-----------------------------------------------------------------------------------------------------------------
          // Change delay and present times.

#pragma region // Change delays
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Query user to select which projector to configure.
            int const ProjectorID = MainSelectProjectorID_inline((int)(sRendering.size()), DefaultProjectorID, 10000, hWndCommand);

            RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
            assert(NULL != pRendering);
            if (NULL != pRendering)
              {
                DisplayWindowParameters * const pWindow = pRendering->pWindow;
                assert(NULL != pWindow);
                if (NULL != pWindow)
                  {

                  MENU_ITEM_CHANGE_DELAY_AND_PRESENT_TIMES:

                    // Fetch current delay value for blocking acquisition mode.
                    double const delay_ms_old = pRendering->delay_ms;

                    // Fetch current delay value for non-blocking aquisition mode.
                    double const delayTime_ms_old = pWindow->delayTime_ms;
                    long int const delayTime_whole_old = pWindow->delayTime_whole;
                    double const delayTime_fraction_us_old = pWindow->delayTime_fraction_us;

                    // Fetch current present interval for non-blocking acquisition mode.
                    long int const presentTime_old = pWindow->presentTime;

                    int const timeout_ms = 10000;

                    // Query user for action.
                    wprintf(L"\n");
                    wprintf(gMsgDelayTimesSubmenu, ProjectorID, delay_ms_old, delayTime_ms_old, presentTime_old);

                    int const pressed_key = TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL);

                    wprintf(L"\n");

                    if (1 == pressed_key)
                      {
                        wprintf(gMsgDelayTimeBlockingPrint, ProjectorID + 1, delay_ms_old);

                        wprintf(gMsgDelayTimeBlockingQuery, ProjectorID + 1);
                        double delay_ms = delay_ms_old;
                        int const scan = scanf_s("%lf", &delay_ms);
                        if ( (1 == scan) && (0 <= delay_ms) && (delay_ms_old != delay_ms) )
                          {
                            pRendering->delay_ms = delay_ms;

                            double const delay_ms_new = pRendering->delay_ms;
                            wprintf(gMsgDelayTimeBlockingChanged, ProjectorID + 1, delay_ms_old, delay_ms_new);
                          }
                        else
                          {
                            wprintf(gMsgDelayTimeBlockingNotChanged, ProjectorID + 1, delay_ms_old);
                          }
                        /* if */

                        wprintf(L"\n");
                        MainPrintAllBlockingDelays_inline(sRendering, &ThreadStorageLock);

                        goto MENU_ITEM_CHANGE_DELAY_AND_PRESENT_TIMES;
                      }
                    else if (2 == pressed_key)
                      {
                        wprintf(gMsgDelayTimeNonBlockingPrint, ProjectorID + 1, delayTime_ms_old, delayTime_whole_old, delayTime_fraction_us_old);

                        wprintf(gMsgDelayTimeNonBlockingQuery, ProjectorID + 1);
                        double delayTime_ms = delayTime_ms_old;
                        int const scan = scanf_s("%lf", &delayTime_ms);
                        if ( (1 == scan) && (0 <= delayTime_ms) && (delayTime_ms_old != delayTime_ms) )
                          {
                            HRESULT const hr = SetDisplayAndDelayTimes(pWindow, presentTime_old, delayTime_ms);
                            assert( SUCCEEDED(hr) );

                            double const delayTime_us_new = pWindow->delayTime_us;
                            long int const delayTime_whole_new = pWindow->delayTime_whole;
                            double const delayTime_fraction_us_new = pWindow->delayTime_fraction_us;
                            wprintf(
                                    gMsgDelayTimeNonBlockingChanged,
                                    ProjectorID + 1,
                                    delayTime_ms_old,
                                    delayTime_us_new, delayTime_whole_new, delayTime_fraction_us_new
                                    );
                          }
                        else
                          {
                            wprintf(
                                    gMsgDelayTimeNonBlockingNotChanged,
                                    ProjectorID + 1,
                                    delayTime_ms_old, delayTime_whole_old, delayTime_fraction_us_old
                                    );
                          }
                        /* if */

                        wprintf(L"\n");
                        MainPrintAllNonBlockingDelays_inline(sRendering, &ThreadStorageLock);

                        goto MENU_ITEM_CHANGE_DELAY_AND_PRESENT_TIMES;
                      }
                    else if (3 == pressed_key)
                      {
                        wprintf(gMsgPresentTimeNonBlockingPrint, ProjectorID + 1, presentTime_old);

                        wprintf(gMsgPresentTimeNonBlockingQuery, ProjectorID + 1);
                        long int presentTime = presentTime_old;
                        int const scan = scanf_s("%ld", &presentTime);
                        if ( (1 == scan) && (0 < presentTime) && (presentTime_old != presentTime) )
                          {
                            HRESULT const hr = SetDisplayAndDelayTimes(pWindow, presentTime, delayTime_ms_old);
                            assert( SUCCEEDED(hr) );

                            long int const presentTime_new = pWindow->presentTime;
                            wprintf(gMsgPresentTimeNonBlockingChanged, ProjectorID + 1, presentTime_old, presentTime_new);
                          }
                        else
                          {
                            wprintf(gMsgPresentTimeNonBlockingNotChanged, ProjectorID + 1, presentTime_old);
                          }
                        /* if */

                        wprintf(L"\n");
                        MainPrintAllNonBlockingPresentTimes_inline(sRendering, &ThreadStorageLock);

                        goto MENU_ITEM_CHANGE_DELAY_AND_PRESENT_TIMES;
                      }
                    else
                      {
                        wprintf(gMsgDelayTimesNoChange, ProjectorID + 1);

                        wprintf(L"\n");
                        wprintf(gMsgMainMenu);
                      }
                    /* if */
                  }
                else
                  {
                    wprintf(L"\n");
                    wprintf(gMsgInvalidProjectorWindow, ProjectorID + 1);
                  }
                /* if */
              }
            else
              {
                wprintf(L"\n");
                wprintf(gMsgInvalidProjector, ProjectorID + 1);
              }
            /* if */
          }
#pragma endregion // Change delays

          break;


        case (wint_t)('7'):
          //-----------------------------------------------------------------------------------------------------------------
          // Enable/disable image saving in PNG format.

#pragma region // Toggle save to PNG
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int ProjectorID = -1; // Apply to all projectors.
            bool fSavePNG = false;

          MENU_ITEM_TOGGLE_SAVE_TO_PNG:

            int const num_prj = (int)( sRendering.size() );

            if (1 == num_prj)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, 0, &ThreadStorageLock);
                assert(NULL != pRendering);
                fSavePNG = (NULL != pRendering)? pRendering->fSavePNG : cfg_save_to_PNG;
              }
            else
              {
                wprintf(L"\n");
                if (-1 == ProjectorID)
                  {
                    wprintf(gMsgSubMenuSaveToPNGForAll);
                    fSavePNG = cfg_save_to_PNG;
                  }
                else
                  {
                    wprintf(gMsgSubMenuSaveToPNGForProjector, ProjectorID + 1);
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    fSavePNG = (NULL != pRendering)? pRendering->fSavePNG : cfg_save_to_PNG;
                  }
                /* if */
                wprintf(gMsgSubMenuSaveToPNGReturnToMainMenu);
                wprintf(gMsgSubMenuSaveToPNGSelectProjector);
                if (true == fSavePNG)
                  {
                    wprintf(gMsgSubMenuSaveToPNGDeactivate);
                  }
                else
                  {
                    wprintf(gMsgSubMenuSaveToPNGActivate);
                  }
                /* if */
              }
            /* if */

            int const timeout_ms = 10000;
            int const pressed_key = (1 != num_prj)? TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL) : -1;
            if (0 == pressed_key)
              {
                wprintf(gMsgSubMenuSaveToPNGNoChange);

                wprintf(L"\n");
                wprintf(gMsgMainMenu);
              }
            else if (1 == pressed_key)
              {
                ProjectorID =
                  MainSelectProjectorID_inline(
                                               (int)(sRendering.size()),
                                               (-1 == ProjectorID)? DefaultProjectorID : ProjectorID,
                                               10000,
                                               hWndCommand
                                               );
                goto MENU_ITEM_TOGGLE_SAVE_TO_PNG;
              }
            else
              {
                assert( 1 <= num_prj );

                if (0 < num_prj) wprintf(L"\n");

                for (int i = 0; i < num_prj; ++i)
                  {
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, i, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    if ( (-1 == ProjectorID) || (i == ProjectorID) )
                      {
                        if (NULL != pRendering)
                          {
                            pRendering->fSavePNG = !fSavePNG;
                            if (true == pRendering->fSavePNG)
                              {
                                wprintf(gMsgImageSavePNGEnabled, ProjectorID + 1);
                              }
                            else
                              {
                                wprintf(gMsgImageSavePNGDisabled, ProjectorID + 1);
                              }
                            /* if */
                          }
                        else
                          {
                            wprintf(gMsgInvalidProjector, ProjectorID + 1);
                          }
                        /* if */
                      }
                    /* if */
                  }
                /* for */

                if (-1 == ProjectorID) cfg_save_to_PNG = !fSavePNG;
              }
            /* if */

          }
#pragma endregion // Toggle save to PNG

          break;


        case (wint_t)('8'):
          //-----------------------------------------------------------------------------------------------------------------
          // Enable/disable image saving in RAW format.

#pragma region // Toggle save to RAW
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int ProjectorID = -1;
            bool fSaveRAW = false;

          MENU_ITEM_TOGGLE_SAVE_TO_RAW:

            int const num_prj = (int)( sRendering.size() );

            if (1 == num_prj)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, 0, &ThreadStorageLock);
                assert(NULL != pRendering);
                fSaveRAW = (NULL != pRendering)? pRendering->fSaveRAW : cfg_save_to_RAW;
              }
            else
              {
                wprintf(L"\n");
                if (-1 == ProjectorID)
                  {
                    wprintf(gMsgSubMenuSaveToRAWForAll);
                    fSaveRAW = cfg_save_to_RAW;
                  }
                else
                  {
                    wprintf(gMsgSubMenuSaveToRAWForProjector, ProjectorID + 1);
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    fSaveRAW = (NULL != pRendering)? pRendering->fSaveRAW : cfg_save_to_RAW;
                  }
                /* if */
                wprintf(gMsgSubMenuSaveToRAWReturnToMainMenu);
                wprintf(gMsgSubMenuSaveToRAWSelectProjector);
                if (true == fSaveRAW)
                  {
                    wprintf(gMsgSubMenuSaveToRAWDeactivate);
                  }
                else
                  {
                    wprintf(gMsgSubMenuSaveToRAWActivate);
                  }
                /* if */
              }
            /* if */

            int const timeout_ms = 10000;
            int const pressed_key = (1 != num_prj)? TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL) : -1;
            if (0 == pressed_key)
              {
                wprintf(gMsgSubMenuSaveToRAWNoChange);

                wprintf(L"\n");
                wprintf(gMsgMainMenu);
              }
            else if (1 == pressed_key)
              {
                ProjectorID =
                  MainSelectProjectorID_inline(
                                               (int)(sRendering.size()),
                                               (-1 == ProjectorID)? DefaultProjectorID : ProjectorID,
                                               10000,
                                               hWndCommand
                                               );
                goto MENU_ITEM_TOGGLE_SAVE_TO_RAW;
              }
            else
              {
                int const num_prj = (int)( sRendering.size() );
                assert( 1 <= num_prj );

                if (0 < num_prj) wprintf(L"\n");

                for (int i = 0; i < num_prj; ++i)
                  {
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, i, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    if ( (-1 == ProjectorID) || (i == ProjectorID) )
                      {
                        if (NULL != pRendering)
                          {
                            pRendering->fSaveRAW = !fSaveRAW;
                            if (true == pRendering->fSaveRAW)
                              {
                                wprintf(gMsgImageSaveRAWEnabled, ProjectorID + 1);
                              }
                            else
                              {
                                wprintf(gMsgImageSaveRAWDisabled, ProjectorID + 1);
                              }
                            /* if */
                          }
                        else
                          {
                            wprintf(gMsgInvalidProjector, ProjectorID + 1);
                          }
                        /* if */
                      }
                    /* if */
                  }
                /* for */

                if (-1 == ProjectorID) cfg_save_to_RAW = !fSaveRAW;
              }
            /* if */
          }
#pragma endregion // Toggle save to RAW

          break;


        case (wint_t)('v'):
        case (wint_t)('V'):
          //-----------------------------------------------------------------------------------------------------------------
          // Toggle live view.

#pragma region // Toggle camera preview
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_prj = (int)( sRendering.size() );
            assert( 1 <= num_prj );

            if (0 < num_prj) wprintf(L"\n");

            for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
              {
                RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                assert(NULL != pRendering);
                if (NULL != pRendering)
                  {
                    bool all_on = false;
                    bool all_off = false;

                    bool const toggle = RenderingThreadToggleLiveViewForAttachedCameras(pRendering, &all_on, &all_off);
                    assert(true == toggle);

                    if ( (true == all_on) && (false == all_off) )
                      {
                        wprintf(gMsgLiveViewEnabled, ProjectorID + 1);
                      }
                    else if ( (false == all_on) && (true == all_off) )
                      {
                        wprintf(gMsgLiveViewDisabled, ProjectorID + 1);
                      }
                    else
                      {
                        wprintf(gMsgLiveViewInvalid, ProjectorID + 1);
                      }
                    /* if */
                  }
                else
                  {
                    wprintf(gMsgInvalidProjector, ProjectorID + 1);
                  }
                /* if */
              }
            /* for */
          }
#pragma endregion // Toggle camera preview

        break;


        case (wint_t)('f'):
        case (wint_t)('F'):
          //-----------------------------------------------------------------------------------------------------------------
          // Go fullscreen.

#pragma region // Go fullscreen for all projectors
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_wnd = (int)( sWindowDisplay.size() );
            assert( 1 <= num_wnd );

            for (int i = 0; i < num_wnd; ++i)
              {
                DisplayWindowParameters * const pWindowDisplay = get_ptr_inline(sWindowDisplay, i, &ThreadStorageLock);
                assert(NULL != pWindowDisplay);
                if (NULL != pWindowDisplay)
                  {
                    SetFullscreenStatusOfDisplayWindow(pWindowDisplay, true);
                  }
                else
                  {
                    wprintf(gMsgInvalidProjectorWindow, i + 1);
                  }
                /* if */
              }
            /* for */
          }
#pragma endregion // Go fullscreen for all projectors

        break;


        case (wint_t)('w'):
        case (wint_t)('W'):
          //-----------------------------------------------------------------------------------------------------------------
          // Go windowed.

#pragma region // Go windowed for all projectors
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_wnd = (int)( sWindowDisplay.size() );
            assert( 1 <= num_wnd );

            for (int i = 0; i < num_wnd; ++i)
              {
                DisplayWindowParameters * const pWindowDisplay = get_ptr_inline(sWindowDisplay, i, &ThreadStorageLock);
                assert(NULL != pWindowDisplay);
                if (NULL != pWindowDisplay)
                  {
                    SetFullscreenStatusOfDisplayWindow(pWindowDisplay, false);
                  }
                else
                  {
                    wprintf(gMsgInvalidProjectorWindow, i + 1);
                  }
                /* if */
              }
            /* for */
          }
#pragma endregion // Go windowed for all projectors

        break;


        case (wint_t)('s'):
        case (wint_t)('S'):
          //-----------------------------------------------------------------------------------------------------------------
          // Stop/start continuous acquisition.

#pragma region // Start/stop present-acquire cycle
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            if (true == continuous_acquisition_active)
              {
                int const num_prj = (int)( sRendering.size() );
                assert( 1 <= num_prj );

                if (0 < num_prj) wprintf(L"\n");

                for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
                  {
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    if (NULL != pRendering)
                      {
                        MainStopContinuousAcquisition_inline(pRendering, MainID);
                      }
                    else
                      {
                        wprintf(gMsgInvalidProjector, ProjectorID + 1);
                      }
                    /* if */
                  }
                /* for */

                // Indicate the project-acquire cycle is now stopped.
                assert(true == continuous_acquisition_active);
                continuous_acquisition_active = false;
              }
            else
              {
                int const num_prj = (int)( sRendering.size() );
                assert( 1 <= num_prj );

                if (0 < num_prj) wprintf(L"\n");

                for (int ProjectorID = 0; ProjectorID < num_prj; ++ProjectorID)
                  {
                    RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pRendering);
                    if (NULL != pRendering)
                      {
                        MainStartContinuousAcquisition_inline(pRendering);
                      }
                    else
                      {
                        wprintf(gMsgInvalidProjector, ProjectorID + 1);
                      }
                    /* if */
                  }
                /* for */

                // Indicate the project-acquire cycle has re-started.
                assert(false == continuous_acquisition_active);
                continuous_acquisition_active = true;
              }
            /* if */
          }
#pragma endregion // Start/stop present-acquire cycle

        break;


        case (wint_t)('c'):
        case (wint_t)('C'):
          //-----------------------------------------------------------------------------------------------------------------
          // Toggle camera configuration dialog for active camera.

#pragma region // Open/close camera configuration dialog
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgCameraConfigurationDialogNoAttachedCameras);
                break;
              }
            /* if */

            ToggleCameraConfigurationDialog(pWindowPreview);
          }
#pragma endregion // Open/close camera configuration dialog

        break;


        case (wint_t)('d'):
        case (wint_t)('D'):
          //-----------------------------------------------------------------------------------------------------------------
          // Add camera to selected projector.

#pragma region // Add camera
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Query user to select which projector to use.
            int const ProjectorID = MainSelectProjectorID_inline((int)(sRendering.size()), DefaultProjectorID, 10000, hWndCommand);

            RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
            assert(NULL != pRendering);
            if (NULL == pRendering)
              {
                wprintf(L"\n");
                wprintf(gMsgInvalidProjector, ProjectorID + 1);
                break;
              }
            /* if */

            // Assume success.
            bool result = true;

            // Query user for which camera SDK to use.
            selected_camera_SDK = MainSelectCameraSDK_inline(30000, true, hWndCommand);

            // Stop continuous acquisition if needed.
            if (true == continuous_acquisition_active)
              {
                MainStopContinuousAcquisition_inline(pRendering, MainID);
              }
            /* if */

            // Create synchronization events.
            int NewEncoderID = -1;
            if (true == result)
              {
                NewEncoderID = AddImageEncoderToSynchronizationEventsStructure(pSynchronization);
                assert(0 <= NewEncoderID);
                result = (-1 != NewEncoderID);
              }
            /* if */

            int NewCameraID = -1;
            if (true == result)
              {
                NewCameraID = AddCameraToSynchronizationEventsStructure(pSynchronization);
                assert(0 <= NewCameraID);
                result = (-1 != NewCameraID);
              }
            /* if */

            // Create image encoder.
            ImageEncoderParameters * pImageEncoder = NULL;
            if (true == result)
              {
                pImageEncoder = ImageEncoderStart(pSynchronization, pWICFactory, NewEncoderID, NewCameraID);
                assert(NULL != pImageEncoder);
                result = (NULL != pImageEncoder);
              }
            /* if */

            // Copy output directory from default image encoder.
            if (true == result)
              {
                ImageEncoderParameters * const pDefaultImageEncoder = get_ptr_inline(sImageEncoder, DefaultEncoderID, &ThreadStorageLock);
                //assert(NULL != pDefaultImageEncoder);
                if (NULL != pDefaultImageEncoder)
                  {
                    TCHAR const * const pImageDirectory = ImageEncoderGetDirectory(pDefaultImageEncoder);
                    assert(NULL != pImageDirectory);
                    if (NULL != pImageDirectory)
                      {
                        bool const savedir = ImageEncoderTrySetDirectory(pImageEncoder, pImageDirectory);
                        assert(true == savedir);

                        result = (NULL != savedir);
                      }
                    else
                      {
                        result = false;
                      }
                    /* if */
                  }
                else if (0 == (int)(sImageEncoder.size()))
                  {
                    bool const savedir = MainSetInitialOutputDirectoryForImageEncoder_inline(pImageEncoder);
                    //assert(true == savedir);
                    result = (NULL != savedir);
                  }
                else
                  {
                    result = false;
                  }
                /* if */
              }
            /* if */

            // Create acquisition thread.
            AcquisitionParameters * pAcquisition = NULL;
            if (true == result)
              {
                pAcquisition =
                  AcquisitionThreadStart(
                                         pSynchronization,
                                         pRendering->pWindow,
                                         pWindowPreview,
                                         pImageEncoder,
                                         pRendering->pImageDecoder,
                                         selected_camera_SDK,
                                         NewCameraID,
                                         pRendering->ProjectorID,
                                         &sConnectedCameras,
                                         false // Prohibit fallback to dummy from file acquisition.
                                         );
                //assert(NULL != pAcquisition);
                result = (NULL != pAcquisition);
              }
            /* if */

            // Wait for all threads to start.
            if (true == result)
              {
                while (false == pImageEncoder->fActive) SleepEx(10, TRUE);
                while (false == pAcquisition->fActive) SleepEx(10, TRUE);
              }
            /* if */

            // Connect created threads.
            if (true == result)
              {
                // Store unique camera identifier.
                if ( true == IsAcquisitionLive(pAcquisition) )
                  {
                    std::wstring * const pCameraName = GetUniqueCameraIdentifier(pAcquisition);
                    assert(NULL != pCameraName);
                    if (NULL != pCameraName) sConnectedCameras.push_back(pCameraName);
                  }
                /* if */

                // Store created threads.
                AcquireSRWLockExclusive( &(ThreadStorageLock) );
                {
                  sImageEncoder.push_back(pImageEncoder);
                  assert( NewEncoderID + 1 == sImageEncoder.size() );

                  sAcquisition.push_back(pAcquisition);
                  assert( NewCameraID + 1 == sAcquisition.size() );
                }
                ReleaseSRWLockExclusive( &(ThreadStorageLock) );

                // Connect to rendering thread.
                bool const add_camera = RenderingThreadAddCamera(pRendering, pAcquisition);
                assert(true == add_camera);

                // Set directory for acquisition from file.
                bool const setdir = AcquisitionParametersFromFileSetDirectory(pAcquisition->pFromFile, RenderingThreadGetInputDirectory(pRendering));
                assert(true == setdir);
              }
            else
              {
                AcquisitionThreadStop( pAcquisition );
                pAcquisition = NULL;

                ImageEncoderStop( pImageEncoder );
                pImageEncoder = NULL;

                HRESULT const remove_encoder = RemoveImageEncoderFromSynchronizationEventsStructure(pSynchronization, NewEncoderID);
                assert( SUCCEEDED(remove_encoder) );

                HRESULT const remove_camera = RemoveCameraFromSynchronizationEventsStructure(pSynchronization, NewCameraID);
                assert( SUCCEEDED(remove_camera) );
              }
            /* if */

            // Re-start continuous acquisition.
            if (true == continuous_acquisition_active)
              {
                MainStartContinuousAcquisition_inline(pRendering);
              }
            /* if */
          }
#pragma endregion // Add camera

        break;


        case (wint_t)('x'):
        case (wint_t)('X'):
          //-----------------------------------------------------------------------------------------------------------------
          // Remove camera.

#pragma region // Remove camera
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            int const num_cam = (int)( sAcquisition.size() );
            if (0 >= num_cam)
              {
                wprintf(gMsgDeleteCameraNoAttachedCameras);
                break;
              }
            /* if */

            // Query user to select which camera to delete.
            int const CameraID = MainSelectCameraID_inline((int)(sAcquisition.size()), DefaultCameraID, 10000, hWndCommand);

            // Fetch acquisition thread pointer.
            AcquisitionParameters * pAcquisition = get_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock);
            assert(NULL != pAcquisition);

            // Fetch camera identifier.
            std::wstring * pCameraName = GetUniqueCameraIdentifier(pAcquisition);
            assert(NULL != pCameraName);

            // Fetch rendering thread pointer which controls camera to delete.
            int const ProjectorID = (NULL != pAcquisition)? pAcquisition->ProjectorID : -1;
            RenderingParameters * const pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
            assert(NULL != pRendering);

            // Fetch last acquisition thread.
            int const CameraIDLast = num_cam - 1;
            AcquisitionParameters * const pAcquisitionLast = get_ptr_inline(sAcquisition, CameraIDLast, &ThreadStorageLock);
            assert(NULL != pAcquisitionLast);

            // Fetch rendering thread pointer which controls last camera.
            int const ProjectorIDLast = (NULL != pAcquisitionLast)? pAcquisitionLast->ProjectorID : -1;
            RenderingParameters * const pRenderingLast = get_ptr_inline(sRendering, ProjectorIDLast, &ThreadStorageLock);
            assert(NULL != pRenderingLast);

            // We may proceed if all pointers are not NULL.
            bool result =
              (NULL != pAcquisition) && (NULL != pRendering) &&
              (NULL != pAcquisitionLast) && (NULL != pRenderingLast);

            // Delete camera.
            if (true == result)
              {
                // Stop preview.
                ClearActiveCamera(pWindowPreview);

                // Stop continuous acquisition if needed.
                if (true == continuous_acquisition_active)
                  {
                    MainStopContinuousAcquisition_inline(pRendering, MainID);
                    if (pRendering != pRenderingLast) MainStopContinuousAcquisition_inline(pRenderingLast, MainID);
                  }
                /* if */

                // Remove camera from the rendering thread.
                bool const remove = RenderingThreadRemoveCamera(pRendering, pAcquisition);
                assert(true == remove);
                if (true != remove) result = false;

                // Fetch image encoder associated with the camera to delete.
                int const EncoderID = (NULL != pAcquisition->pImageEncoder)? pAcquisition->pImageEncoder->EncoderID : CameraID;
                int const EncoderIDLast = (NULL != pAcquisitionLast->pImageEncoder)? pAcquisitionLast->pImageEncoder->EncoderID : CameraIDLast;

                if (true == result)
                  {
                    ImageEncoderParameters * pImageEncoder = get_ptr_inline(sImageEncoder, EncoderID, &ThreadStorageLock);
                    assert(NULL != pImageEncoder);

                    // Delete selected acquisition and encoder threads.
                    if (CameraID != CameraIDLast)
                      {
                        /* Selected acquisition and encoder threads are not last in the thread storage.
                           Due to application design unused slots are not allowed in the storage so the last thread must
                           be copied to the place of the deleted thread. This requires event ID change.
                        */
                        ImageEncoderParameters * const pImageEncoderLast = get_ptr_inline(sImageEncoder, EncoderIDLast, &ThreadStorageLock);
                        assert(NULL != pImageEncoderLast);

                        assert(EncoderID != EncoderIDLast);
                        assert(CameraIDLast + 1 == (int)(sAcquisition.size()));
                        assert(EncoderIDLast + 1 == (int)(sImageEncoder.size()));

                        // Copy last element to the place of the deleted element.
                        bool const set_acquisition = set_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock, pAcquisitionLast);
                        assert(true == set_acquisition);

                        bool const set_encoder = set_ptr_inline(sImageEncoder, EncoderID, &ThreadStorageLock, pImageEncoderLast);
                        assert(true == set_encoder);

                        bool const remove_acquisition = set_ptr_inline(sAcquisition, CameraIDLast, &ThreadStorageLock, (AcquisitionParameters *)NULL);
                        assert(true == remove_acquisition);

                        bool const remove_encoder = set_ptr_inline(sImageEncoder, EncoderIDLast, &ThreadStorageLock, (ImageEncoderParameters *)NULL);
                        assert(true == remove_encoder);

                        // Stop threads.
                        AcquisitionThreadStop(pAcquisition);
                        pAcquisition = NULL;

                        ImageEncoderStop(pImageEncoder);
                        pImageEncoder = NULL;

                        // Change event IDs.
                        bool const change_id = AcquisitionThreadSetNewCameraIDAndEncoderID(pAcquisitionLast, CameraID, EncoderID);
                        assert(true == change_id);

                        assert(CameraID == pAcquisitionLast->CameraID);
                        assert(CameraID == pImageEncoderLast->CameraID);
                        assert(EncoderID == pImageEncoderLast->EncoderID);
                      }
                    else
                      {
                        /* Selected aqcuisition and encoder threads are last in the thread storage and may be deleted immediately. */

                        // Remove threads from storage.
                        bool const remove_acquisition = set_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock, (AcquisitionParameters *)NULL);
                        assert(true == remove_acquisition);

                        bool const remove_encoder = set_ptr_inline(sImageEncoder, EncoderID, &ThreadStorageLock, (ImageEncoderParameters *)NULL);
                        assert(true == remove_encoder);

                        // Stop threads.
                        AcquisitionThreadStop(pAcquisition);
                        pAcquisition = NULL;

                        ImageEncoderStop(pImageEncoder);
                        pImageEncoder = NULL;
                      }
                    /* if */

                    // Pop NULL entries.
                    AcquireSRWLockExclusive( &(ThreadStorageLock) );
                    {
                      assert(NULL == sAcquisition.back());
                      sAcquisition.pop_back();

                      assert(NULL == sImageEncoder.back());
                      sImageEncoder.pop_back();

                      assert(CameraIDLast == EncoderIDLast);
                      assert(CameraIDLast == sAcquisition.size());
                      assert(EncoderIDLast == sImageEncoder.size());
                    }
                    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

                    // Remove extra event IDs.
                    HRESULT const remove_encoder = RemoveImageEncoderFromSynchronizationEventsStructure(pSynchronization, EncoderIDLast);
                    assert( SUCCEEDED(remove_encoder) );

                    HRESULT const remove_camera = RemoveCameraFromSynchronizationEventsStructure(pSynchronization, CameraIDLast);
                    assert( SUCCEEDED(remove_camera) );

                    // Remove camera identifier from the list of attached cameras.
                    if (NULL != pCameraName)
                      {
                        // First find the matching camera identifier.
                        int const sz = (int)( sConnectedCameras.size() );
                        int cam_idx = -1;
                        for (int i = 0; i < sz; ++i)
                          {
                            std::wstring * ptr = sConnectedCameras[i];
                            if ( (NULL != ptr) && (0 == pCameraName->compare(*ptr)) )
                              {
                                cam_idx = i;
                                break;
                              }
                            /* if */
                          }
                        /* for */

                        // Then put the last identifier into its place and delete it.
                        if (-1 != cam_idx)
                          {
                            std::wstring * ptr = sConnectedCameras[cam_idx];
                            if (cam_idx != sz - 1)
                              {
                                assert( (0 <= cam_idx) && (cam_idx < sz - 1) );
                                sConnectedCameras[cam_idx] = sConnectedCameras[sz - 1];
                                sConnectedCameras[sz - 1] = NULL;
                              }
                            /* if */
                            sConnectedCameras.pop_back();
                            SAFE_DELETE(ptr);
                          }
                        /* if */
                      }
                    /* if (NULL != pCameraName) */
                  }
                /* if (true == result) */

                // Restart continuous acquisition if needed.
                if (true == continuous_acquisition_active)
                  {
                    if (pRendering != pRenderingLast) MainStartContinuousAcquisition_inline(pRenderingLast);
                    MainStartContinuousAcquisition_inline(pRendering);
                  }
                /* if */

                // Resume preview.
                RestoreActiveCamera(pWindowPreview);

                // Output message.
                if (true == result)
                  {
                    wprintf(gMsgDeleteCameraSucceeded, CameraID + 1);
                  }
                else
                  {
                    wprintf(gMsgDeleteCameraError, CameraID + 1);
                  }
                /* if */
              }
            else
              {
                wprintf(gMsgDeleteCameraError, CameraID + 1);
              }
            /* if */

            SAFE_DELETE( pCameraName );

          }
#pragma endregion // Remove camera

        break;


        case (wint_t)('p'):
        case (wint_t)('P'):
          //-----------------------------------------------------------------------------------------------------------------
          // Add projector.

#pragma region // Add projector
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Assume success.
            bool result = true;

            // Create synchronization events.
            int NewDecoderID = -1;
            if (true == result)
              {
                NewDecoderID = AddImageDecoderToSynchronizationEventsStructure(pSynchronization);
                assert(0 < NewDecoderID);
                result = (-1 != NewDecoderID);
              }
            /* if */

            int NewProjectorID = -1;
            if (true == result)
              {
                NewProjectorID = AddProjectorToSynchronizationEventsStructure(pSynchronization);
                assert(0 < NewProjectorID);
                result = (-1 != NewProjectorID);
              }
            /* if */

            // Create image file list.
            ImageFileList * pImageList = NULL;
            if (true == result)
              {
                pImageList = new ImageFileList();
                assert(NULL != pImageList);
                result = (NULL != pImageList);
              }
            /* if */

            // Copy input directory form default image file list.
            if (true == result)
              {
                ImageFileList * const pDefaultImageList = get_ptr_inline(sImageList, DefaultDecoderID, &ThreadStorageLock);
                assert(NULL != pDefaultImageList);
                if (NULL != pDefaultImageList)
                  {
                    int const sz = 1024;
                    wchar_t szTitle[sz + 1];
                    int const cnt1 = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForProjector, NewProjectorID + 1);
                    assert(0 < cnt1);
                    szTitle[sz] = 0;

                    bool const setdir = pImageList->SetDirectory(pDefaultImageList->GetDirectory(), szTitle);
                    assert(true == setdir);
                    result = setdir;
                  }
                else
                  {
                    result = false;
                  }
                /* if */
              }
            /* if */

            // Create image decoder.
            ImageDecoderParameters * pImageDecoder = NULL;
            if (true == result)
              {
                pImageDecoder = ImageDecoderStart(pImageList, pSynchronization, pWICFactory, NewDecoderID, NewProjectorID);
                assert(NULL != pImageDecoder);
                result = (NULL != pImageDecoder);
              }
            /* if */

            // Create render window.
            DisplayWindowParameters * pWindow = NULL;
            if (true == result)
              {
                pWindow = OpenDisplayWindow(GetModuleHandle(NULL), NewProjectorID, SW_SHOWNA, NULL, hWndCommand);
                assert(NULL != pWindow);
              }
            /* if */

            // Create swap chain for the render window.
            if (true == result)
              {
                while (false == pWindow->fActive) SleepEx(10, TRUE);

                HRESULT const hr = CreateDirectXDeviceAndSwapChain(pWindow, pDXGIFactory1, pD2DFactory);
                assert( SUCCEEDED(hr) );
                result = SUCCEEDED(hr);

                BOOL const pos = SetWindowPos(pWindow->hWnd, HWND_TOP, 50, 50, 800, 600, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW);
                assert(TRUE == pos);
              }
            /* if */

            // Create rendering thread.
            RenderingParameters * pRendering = NULL;
            if (true == result)
              {
                pRendering = RenderingThreadStart(pSynchronization, pWindow, pImageDecoder, NewProjectorID);
                assert(NULL != pRendering);
                result = (NULL != pRendering);
              }
            /* if */

            // Start rendering.
            if (true == result)
              {
                // Wait for all threads to become active.
                while (false == pImageDecoder->fActive) SleepEx(10, TRUE);
                while (false == pRendering->fActive) SleepEx(10, TRUE);

                // Store created threads.
                AcquireSRWLockExclusive( &(ThreadStorageLock) );
                {
                  assert(NewDecoderID == NewProjectorID);

                  sImageList.push_back(pImageList);
                  assert( NewDecoderID + 1 == sImageList.size() );

                  sImageDecoder.push_back(pImageDecoder);
                  assert( NewDecoderID + 1 == sImageDecoder.size() );

                  sWindowDisplay.push_back(pWindow);
                  assert( NewProjectorID + 1 == sWindowDisplay.size() );

                  sRendering.push_back(pRendering);
                  assert( NewProjectorID + 1 == sRendering.size() );
                }
                ReleaseSRWLockExclusive( &(ThreadStorageLock) );

                // Kickstart rendering.
                BOOL const set_render_ready = pSynchronization->EventSet(DRAW_RENDER_READY, NewProjectorID);
                assert(0 != set_render_ready);

                BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, NewProjectorID);
                assert(0 != set_render);
              }
            else
              {
                RenderingThreadStop( pRendering );
                pRendering = NULL;

                CloseDisplayWindow( pWindow );
                pWindow = NULL;

                ImageDecoderStop( pImageDecoder );
                pImageDecoder = NULL;

                DeleteImageFileList( pImageList );
                pImageList = NULL;

                HRESULT const remove_decoder = RemoveImageDecoderFromSynchronizationEventsStructure(pSynchronization, NewDecoderID);
                assert( SUCCEEDED(remove_decoder) );

                HRESULT const remove_projector = RemoveProjectorFromSynchronizationEventsStructure(pSynchronization, NewProjectorID);
                assert( SUCCEEDED(remove_projector) );
              }
            /* if */

            // Activate command window.
            {
              BOOL const top = BringWindowToTop(hWndCommand);
              assert(TRUE == top);
            }
          }
#pragma endregion // Add projector

        break;


        case (wint_t)('l'):
        case (wint_t)('L'):
          //-----------------------------------------------------------------------------------------------------------------
          // Remove projector.

#pragma region // Remove projector
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Last projector cannot be deleted.
            int const num_prj = (int)( sRendering.size() );
            if (1 >= num_prj)
              {
                wprintf(gMsgDeleteProjectorOneProjectorRemaining);
                break;
              }
            /* if */

            // Query user to select which projector to delete.
            int const ProjectorID = MainSelectProjectorID_inline((int)(sRendering.size()), DefaultProjectorID, 10000, hWndCommand);

            // Fetch rendering thread pointer.
            RenderingParameters * pRendering = get_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock);
            assert(NULL != pRendering);

            // Projector may be deleted only if it has no cameras attached.
            bool const have_cameras = RenderingThreadHaveCamera(pRendering);
            if ( true == have_cameras )
              {
                wprintf(gMsgDeleteProjectorErrorHasCamerasAttached, ProjectorID + 1);
                break;
              }
            /* if */

            // Fetch last rendering thread pointer.
            int const ProjectorIDLast = num_prj - 1;
            RenderingParameters * pRenderingLast = get_ptr_inline(sRendering, ProjectorIDLast, &ThreadStorageLock);
            assert(NULL != pRenderingLast);

            // We may proceed if all pointers are not NULL.
            bool result = (NULL != pRendering) && (NULL != pRenderingLast);
            if (true == result)
              {
                // Stop continuous acquisition if needed.
                if (true == continuous_acquisition_active)
                  {
                    MainStopContinuousAcquisition_inline(pRendering, MainID);
                    if (pRendering != pRenderingLast) MainStopContinuousAcquisition_inline(pRenderingLast, MainID);
                  }
                /* if */

                // Fetch image decoder associated with the projector to delete.
                int const DecoderID = (NULL != pRendering->pImageDecoder)? pRendering->pImageDecoder->DecoderID : DecoderID;
                int const DecoderIDLast = (NULL != pRenderingLast->pImageDecoder)? pRenderingLast->pImageDecoder->DecoderID : ProjectorIDLast;

                assert(DecoderID == ProjectorID);
                assert(DecoderIDLast == ProjectorIDLast);

                if (true == result)
                  {
                    DisplayWindowParameters * pWindowDisplay = get_ptr_inline(sWindowDisplay, ProjectorID, &ThreadStorageLock);
                    assert(NULL != pWindowDisplay);

                    assert(pRendering->pWindow == pWindowDisplay);

                    ImageDecoderParameters * pImageDecoder = get_ptr_inline(sImageDecoder, DecoderID, &ThreadStorageLock);
                    assert(NULL != pImageDecoder);

                    ImageFileList * pImageList = get_ptr_inline(sImageList, DecoderID, &ThreadStorageLock);
                    assert(NULL != pImageList);

                    assert(pImageList == pImageDecoder->pImageList);

                    // Delete selected rendering and decoder threads.
                    if (ProjectorID != ProjectorIDLast)
                      {
                        /* Selected rendering and decoder threads are not last in the thread storage.
                           Due to application design constraints unused slots are not allowed in the storage so the last thread
                           must be copied to the place of the delted thread. This requires event ID change.
                        */
                        DisplayWindowParameters * const pWindowDisplayLast = get_ptr_inline(sWindowDisplay, ProjectorIDLast, &ThreadStorageLock);
                        assert(NULL != pWindowDisplayLast);

                        assert(pRenderingLast->pWindow == pWindowDisplayLast);

                        ImageDecoderParameters * const pImageDecoderLast = get_ptr_inline(sImageDecoder, DecoderIDLast, &ThreadStorageLock);
                        assert(NULL != pImageDecoderLast);

                        ImageFileList * const pImageListLast = get_ptr_inline(sImageList, DecoderIDLast, &ThreadStorageLock);
                        assert(NULL != pImageListLast);

                        assert(pImageListLast == pImageDecoderLast->pImageList);

                        assert(DecoderID != DecoderIDLast);
                        assert(ProjectorIDLast + 1 == (int)(sRendering.size()));
                        assert(ProjectorIDLast + 1 == (int)(sWindowDisplay.size()));
                        assert(DecoderIDLast + 1 == (int)(sImageDecoder.size()));
                        assert(DecoderIDLast + 1 == (int)(sImageList.size()));

                        // Copy last element to the place of the deleted element.
                        bool const set_rendering =  set_ptr_inline(sRendering, ProjectorID, &ThreadStorageLock, pRenderingLast);
                        assert(true == set_rendering);

                        bool const set_window = set_ptr_inline(sWindowDisplay, ProjectorID, &ThreadStorageLock, pWindowDisplayLast);
                        assert(true == set_window);

                        bool const set_decoder = set_ptr_inline(sImageDecoder, DecoderID, &ThreadStorageLock, pImageDecoderLast);
                        assert(true == set_decoder);

                        bool const set_list = set_ptr_inline(sImageList, DecoderID, &ThreadStorageLock, pImageListLast);
                        assert(true == set_list);

                        bool const remove_rendering = set_ptr_inline(sRendering, ProjectorIDLast, &ThreadStorageLock, (RenderingParameters *)NULL);
                        assert(true == remove_rendering);

                        bool const remove_window = set_ptr_inline(sWindowDisplay, ProjectorIDLast, &ThreadStorageLock, (DisplayWindowParameters *)NULL);
                        assert(true == remove_window);

                        bool const remove_decoder = set_ptr_inline(sImageDecoder, DecoderIDLast, &ThreadStorageLock, (ImageDecoderParameters *)NULL);
                        assert(true == remove_decoder);

                        bool const remove_list = set_ptr_inline(sImageList, DecoderIDLast, &ThreadStorageLock, (ImageFileList *)NULL);
                        assert(true == remove_list);

                        // Stop threads and close window.
                        RenderingThreadStop(pRendering);
                        pRendering = NULL;

                        CloseDisplayWindow(pWindowDisplay);
                        pWindowDisplay = NULL;

                        ImageDecoderStop(pImageDecoder);
                        pImageDecoder = NULL;

                        DeleteImageFileList(pImageList);
                        pImageList = NULL;

                        // Change event IDs.
                        bool const change_id = RenderingThreadSetNewProjectorIDAndDecoderID(pRenderingLast, ProjectorID, DecoderID);
                        assert(true == change_id);

                        assert(ProjectorID == pRenderingLast->ProjectorID);
                        assert(ProjectorID == pImageDecoderLast->ProjectorID);
                        assert(DecoderID == pImageDecoderLast->DecoderID);

                        // Update window.
                        DisplayWindowUpdateTitle(pWindowDisplayLast);
                      }
                    else
                      {
                        /* Selected rendering and decoder threads are last in the thread storage and may be deleted immediately. */

                        assert(pRendering = pRenderingLast);

                        // Remove threads from storage.
                        bool const remove_rendering = set_ptr_inline(sRendering, ProjectorIDLast, &ThreadStorageLock, (RenderingParameters *)NULL);
                        assert(true == remove_rendering);

                        bool const remove_window = set_ptr_inline(sWindowDisplay, ProjectorIDLast, &ThreadStorageLock, (DisplayWindowParameters *)NULL);
                        assert(true == remove_window);

                        bool const remove_decoder = set_ptr_inline(sImageDecoder, DecoderIDLast, &ThreadStorageLock, (ImageDecoderParameters *)NULL);
                        assert(true == remove_decoder);

                        bool const remove_list = set_ptr_inline(sImageList, DecoderIDLast, &ThreadStorageLock, (ImageFileList *)NULL);
                        assert(true == remove_list);

                        // Stop threads and close window.
                        RenderingThreadStop(pRendering);
                        pRendering = NULL;
                        pRenderingLast = NULL;

                        CloseDisplayWindow(pWindowDisplay);
                        pWindowDisplay = NULL;

                        ImageDecoderStop(pImageDecoder);
                        pImageDecoder = NULL;

                        DeleteImageFileList(pImageList);
                        pImageList = NULL;
                      }
                    /* if */

                    // Pop NULL entries.
                    AcquireSRWLockExclusive( &(ThreadStorageLock) );
                    {
                      assert(NULL == sRendering.back());
                      sRendering.pop_back();

                      assert(NULL == sWindowDisplay.back());
                      sWindowDisplay.pop_back();

                      assert(NULL == sImageDecoder.back());
                      sImageDecoder.pop_back();

                      assert(NULL == sImageList.back());
                      sImageList.pop_back();

                      assert(ProjectorIDLast == DecoderIDLast);
                      assert(ProjectorIDLast == sRendering.size());
                      assert(ProjectorIDLast == sWindowDisplay.size());
                      assert(DecoderIDLast == sImageDecoder.size());
                      assert(DecoderIDLast == sImageList.size());
                    }
                    ReleaseSRWLockExclusive( &(ThreadStorageLock) );

                    // Remove extra event IDs.
                    HRESULT const remove_decoder = RemoveImageDecoderFromSynchronizationEventsStructure(pSynchronization, DecoderIDLast);
                    assert( SUCCEEDED(remove_decoder) );

                    HRESULT const remove_projector = RemoveProjectorFromSynchronizationEventsStructure(pSynchronization, ProjectorIDLast);
                    assert( SUCCEEDED(remove_projector) );
                  }
                else
                  {
                    wprintf(gMsgDeleteProjectorError, ProjectorID + 1);
                  }
                /* if */

                // Restart continuous acquisition if needed.
                if (true == continuous_acquisition_active)
                  {
                    if (NULL != pRenderingLast) MainStartContinuousAcquisition_inline(pRenderingLast);
                  }
                /* if */

                // Output message.
                if (true == result)
                  {
                    wprintf(gMsgDeleteProjectorSucceeded, ProjectorID + 1);
                  }
                else
                  {
                    wprintf(gMsgDeleteProjectorError, ProjectorID + 1);
                  }
                /* if */
              }
            else
              {
                wprintf(gMsgDeleteProjectorError, ProjectorID + 1);
              }
            /* if */

          }
#pragma endregion // Remove projector

        break;


        case (wint_t)('r'):
        case (wint_t)('R'):
          //-----------------------------------------------------------------------------------------------------------------
          // 3D reconstruction

#pragma region // 3D reconstruction
          {
            ImageEncoderParameters * const pDefaultImageEncoder = get_ptr_inline(sImageEncoder, DefaultEncoderID, &ThreadStorageLock);
            //assert(NULL != pDefaultImageEncoder);

            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
              }
            else if ( (NULL == pDefaultImageEncoder) || (NULL == pDefaultImageEncoder->pAllImages) )
              {
                wprintf(gMsgReconstructionNoCamerasAttached);
              }
            else if (false == pDefaultImageEncoder->pAllImages->HaveAny())
              {
                wprintf(gMsgReconstructionNoImagesAcquired);
              }
            else
              {
                assert(NULL != pDefaultImageEncoder);
                assert(NULL != pDefaultImageEncoder->pAllImages);
                assert(true == pDefaultImageEncoder->pAllImages->HaveAny());

                // Clear any previous 3D reconstructions.
                {
                  bool const clear_previous = VTKClearAllPushedData(pWindowVTK);
                  assert(true == clear_previous);
                }

              MENU_ITEM_3D_RECONSTRUCTION:

                int const timeout_ms = 30000;

                {
                  int const cnt = wprintf(L"\n");
                  assert(0 < cnt);
                }

                // Print reconstruction method selection message.
                {
                  int const cnt = wprintf(gMsgReconstructionMenu);
                  assert(0 < cnt);
                }

                // Wait for the user to select 3D reconstruction type.
                ReconstructionMethod selected_method = RECONSTRUCTION_DEFAULT;
                {
                  int const pressed_key = TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL);
                  if (0 == pressed_key) selected_method = RECONSTRUCTION_CONFIGURE_PARAMETERS;
                  if (1 == pressed_key) selected_method = RECONSTRUCTION_PSGC_COL;
                  if (2 == pressed_key) selected_method = RECONSTRUCTION_PSGC_ROW;
                  if (3 == pressed_key) selected_method = RECONSTRUCTION_PSGC_ALL;
                  if (4 == pressed_key) selected_method = RECONSTRUCTION_MPS2_COL;
                  if (5 == pressed_key) selected_method = RECONSTRUCTION_MPS2_ROW;
                  if (6 == pressed_key) selected_method = RECONSTRUCTION_MPS2_ALL;
                  if (7 == pressed_key) selected_method = RECONSTRUCTION_MPS3_COL;
                  if (8 == pressed_key) selected_method = RECONSTRUCTION_MPS3_ROW;
                  if (9 == pressed_key) selected_method = RECONSTRUCTION_MPS3_ALL;
                }

                // Adjust reconstruction parameters if requested by the user.
                if (RECONSTRUCTION_CONFIGURE_PARAMETERS == selected_method)
                  {
                    {
                      int const cnt = wprintf(L"\n");
                      assert(0 < cnt);
                    }

                    {
                      int const cnt = wprintf(gMsgReconstructionMenuConfigurationParameters, rel_thr, dst_thr);
                      assert(0 < cnt);
                    }

                    int const pressed_key = TimedWaitForNumberKey(timeout_ms, 10, true, true, (HWND)NULL);

                    if (1 == pressed_key)
                      {
                        double const rel_thr_old = rel_thr;

                        wprintf(gMsgReconstructionConfigurationRelativeThresholdPrint, rel_thr_old);

                        wprintf(gMsgReconstructionConfigurationRelativeThresholdQuery);
                        double rel_thr_new = rel_thr_old;
                        int const scan = scanf_s("%lf", &rel_thr_new);
                        if ( (1 == scan) && (0.0 <= rel_thr_new) && (rel_thr_new < 1.0) && (rel_thr_old != rel_thr_new) )
                          {
                            rel_thr = rel_thr_new;
                            wprintf(gMsgReconstructionConfigurationRelativeThresholdChanged, rel_thr_old, rel_thr_new);
                          }
                        else
                          {
                            wprintf(gMsgReconstructionConfigurationRelativeThresholdNotChanged, rel_thr_old);
                          }
                        /* if */
                      }
                    else if (2 == pressed_key)
                      {
                        double const dst_thr_old = dst_thr;

                        wprintf(gMsgReconstructionConfigurationDistanceThresholdPrint, dst_thr_old);

                        wprintf(gMsgReconstructionConfigurationDistanceThresholdQuery);
                        double dst_thr_new = dst_thr_old;
                        int const scan = scanf_s("%lf", &dst_thr_new);
                        if ( (1 == scan) && (0.0 <= dst_thr_new) && (dst_thr_new != dst_thr_old) )
                          {
                            dst_thr = dst_thr_new;
                            wprintf(gMsgReconstructionConfigurationDistanceThresholdChanged, dst_thr_old, dst_thr_new);
                          }
                        else
                          {
                            wprintf(gMsgReconstructionConfigurationDistanceThresholdNotChanged, dst_thr_old);
                          }
                        /* if */
                      }
                    else
                      {
                        wprintf(gMsgReconstructionConfigurationNoChange);
                      }
                    /* if */

                    goto MENU_ITEM_3D_RECONSTRUCTION;
                  }
                /* if */

                // Set selected method description string.
                std::wstring method;
                int num_images = 0;
                switch ( selected_method )
                  {
                  case RECONSTRUCTION_PSGC_COL:
                    method = L"PS+GC 8PS+(4+4)GC+B+W column";
                    num_images = 18;
                    break;

                  case RECONSTRUCTION_PSGC_ROW:
                    method = L"PS+GC 8PS+(4+4)GC+B+W row";
                    num_images = 18;
                    break;

                  case RECONSTRUCTION_PSGC_ALL:
                    method = L"PS+GC 8PS+(4+4)GC+B+W+8PS+(4+4)GC column row";
                    num_images = 34;
                    break;

                  case RECONSTRUCTION_MPS2_COL:
                    method = L"MPS 8PS(n15)+8PS(n19) column";
                    num_images = 16;
                    break;

                  case RECONSTRUCTION_MPS2_ROW:
                    method = L"MPS 8PS(n15)+8PS(n19) row";
                    num_images = 16;
                    break;

                  case RECONSTRUCTION_MPS2_ALL:
                    method = L"MPS 8PS(n15)+8PS(n19) column row";
                    num_images = 32;
                    break;

                  case RECONSTRUCTION_MPS3_COL:
                    method = L"MPS 3PS(n20)+3PS(n21)+3PS(n25) column";
                    num_images = 9;
                    break;

                  case RECONSTRUCTION_MPS3_ROW:
                    method = L"MPS 3PS(n20)+3PS(n21)+3PS(n25) row";
                    num_images = 9;
                    break;

                  case RECONSTRUCTION_MPS3_ALL:
                  case RECONSTRUCTION_DEFAULT:
                  default:
                    method = L"MPS 3PS(n20)+3PS(n21)+3PS(n25) column row";
                    num_images = 18;
                    break;
                  }
                /* switch */

                // For each attached camera and projector perform the 3D reconstruction.
                for (int CameraID = 0; CameraID < (int)(sAcquisition.size()); ++CameraID)
                  {
                    AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, CameraID, &ThreadStorageLock);
                    assert(NULL != pAcquisition);
                    if (NULL == pAcquisition) continue;

                    ImageEncoderParameters * const pImageEncoder = pAcquisition->pImageEncoder;
                    assert(NULL != pImageEncoder);
                    if (NULL == pImageEncoder) continue;

                    {
                      int const cnt = wprintf(L"\n");
                      assert(0 < cnt);
                    }

                    int const ProjectorID = pAcquisition->ProjectorID;
                    {
                      int const cnt = wprintf(gMsgReconstructionForCameraStart, CameraID + 1, ProjectorID + 1);
                      assert(0 < cnt);
                    }

                    // All images must be acquired.
                    bool const have_all = pImageEncoder->pAllImages->HaveFirstN(num_images);
                    //assert(true == have_all);
                    if (false == have_all)
                      {
                        int const cnt = wprintf(gMsgReconstructionForCameraMissingImages, CameraID + 1);
                        assert(0 < cnt);
                        continue;
                      }
                    /* if */

                    // Check if run was fullscreen.
                    bool const is_fullscreen = pImageEncoder->pAllImages->IsFullscreen();
                    //assert(true == is_fullscreen);
                    if ( (CAMERA_SDK_FROM_FILE != GetAcquisitionMethod(pAcquisition)) && (false == is_fullscreen) )
                      {
                        int const cnt = wprintf(gMsgReconstructionForCameraNotFullscreen, ProjectorID + 1);
                        assert(0 < cnt);
                      }
                    /* if */

                    // Set default name.
                    pImageEncoder->pAllImages->SetName(pImageEncoder->pSubdirectoryRecording);

                    // Do 3D reconstruction.
                    bool const res = ProcessAcquiredImages(
                                                           pImageEncoder->pAllImages,
                                                           method.c_str(),
                                                           fname_geometry.c_str(),
                                                           pWindowVTK,
                                                           rel_thr,
                                                           dst_thr * dst_thr
                                                           );

                    if (true == res)
                      {
                        int const cnt = wprintf(gMsgReconstructionForCameraCompleted, CameraID + 1, ProjectorID + 1);
                        assert(0 < cnt);
                      }
                    else
                      {
                        int const cnt = wprintf(gMsgReconstructionForCameraFailed, CameraID + 1, ProjectorID + 1);
                        assert(0 < cnt);
                      }
                    /* if */
                  }
                /* for */

                // Inform user the reconstruction is completed.
                wprintf(L"\n");
                wprintf(gMsgReconstructionReturnToMainMenu);

                wprintf(L"\n");
                wprintf(gMsgMainMenu);
              }
            /* if */
          }
#pragma endregion // 3D reconstruction

        break;


        case (wint_t)('n'):
        case (wint_t)('N'):
          //-----------------------------------------------------------------------------------------------------------------
          // Set acquisition name tag.

#pragma region // Acquisition name tag
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            std::wstring * pAcquisitionTagOld = (NULL == pAcquisitionTag)? NULL : new std::wstring(*pAcquisitionTag);
            std::wstring * pAcquisitionTagNew = NULL;
            bool tag_changed = false;

            wprintf(L"\n");

            if (NULL != pAcquisitionTagOld)
              {
                wprintf(gMsgSetAcquisitionTagPrintDefined, pAcquisitionTagOld->c_str());
              }
            else
              {
                wprintf(gMsgSetAcquisitionTagPrintUndefined);
              }
            /* if */


            wprintf(gMsgSetAcquisitionTagQuery);
            int const buffer_sz = 1024;
            wchar_t buffer[buffer_sz + 1];
            wchar_t * const scan = _getws_s(buffer, (unsigned)_countof(buffer));
            if (NULL != scan)
              {
                buffer[buffer_sz] = 0;

                // Copy user input to string and trim whitespaces and tabs.
                // TODO: Test input string for invalid characters which are prohibited in directory names.
                assert(NULL == pAcquisitionTagNew);
                pAcquisitionTagNew = new std::wstring(buffer);
                assert(NULL != pAcquisitionTagNew);

                pAcquisitionTagNew->erase(0, pAcquisitionTagNew->find_first_not_of(L" \t"));
                pAcquisitionTagNew->erase(pAcquisitionTagNew->find_last_not_of(L" \t") + 1);

                bool const is_empty = (0 == pAcquisitionTagNew->size());
                bool are_equal = false;
                if (NULL == pAcquisitionTagOld)
                  {
                    if (true == is_empty) are_equal = true;
                  }
                else
                  {
                    are_equal = (0 == _wcsicmp(pAcquisitionTagNew->c_str(), pAcquisitionTagOld->c_str()));
                  }
                /* if */

                // Compare to old value.
                if (false == are_equal)
                  {
                    if (true == is_empty)
                      {
                        SAFE_DELETE( pAcquisitionTag );
                        SAFE_DELETE( pAcquisitionTagNew );
                      }
                    else
                      {
                        assert(NULL != pAcquisitionTagNew);
                        bool const have_dst = (NULL != pAcquisitionTag);

                        if (true == have_dst)
                          {
                            *pAcquisitionTag = *pAcquisitionTagNew;
                          }
                        else
                          {
                            pAcquisitionTag = new std::wstring(*pAcquisitionTagNew);
                          }
                        /* if */
                      }
                    /* if */
                    tag_changed = true;
                  }
                /* if */
              }
            /* if */

            if (true == tag_changed)
              {
                if ( (NULL != pAcquisitionTagOld) && (NULL != pAcquisitionTagNew) )
                  {
                    wprintf(gMsgSetAcquisitionTagChanged, pAcquisitionTagOld->c_str(), pAcquisitionTagNew->c_str());
                  }
                else if ( (NULL != pAcquisitionTagOld) && (NULL == pAcquisitionTagNew) )
                  {
                    wprintf(gMsgSetAcquisitionTagChangedNoDestination, pAcquisitionTagOld->c_str());
                  }
                else if ( (NULL == pAcquisitionTagOld) && (NULL != pAcquisitionTagNew) )
                  {
                    wprintf(gMsgSetAcquisitionTagChangedNoSource, pAcquisitionTagNew->c_str());
                  }
                /* if */
              }
            else
              {
                if (NULL != pAcquisitionTagOld)
                  {
                    wprintf(gMsgSetAcquisitionTagUnchangedDefined, pAcquisitionTagOld->c_str());
                  }
                else
                  {
                    wprintf(gMsgSetAcquisitionTagUnchangedUndefined);
                  }
                /* if */
              }
            /* if */

            SAFE_DELETE( pAcquisitionTagOld );
            SAFE_DELETE( pAcquisitionTagNew );
          }
#pragma endregion // Acquisition name tag

        break;


        case (wint_t)('m'):
        case (wint_t)('M'):
        case (wint_t)('h'):
        case (wint_t)('H'):
          //-----------------------------------------------------------------------------------------------------------------
          // Print menu.
          {
            wprintf(L"\n");
            wprintf(gMsgMainMenu);
          }
        break;


        case (wint_t)('q'):
        case (wint_t)('Q'):
        case (wint_t)(27):
          //-----------------------------------------------------------------------------------------------------------------
          // Exit the application.

#pragma region // Terminate program
          {
            if (true == batch_active)
              {
                wprintf(gMsgBatchCommandDisabled);
                break;
              }
            /* if */

            // Post messages to display windows to terminate.
            int const num_wnd = (int)( sWindowDisplay.size() );
            assert( 1 <= num_wnd );

            for (int i = 0; i < num_wnd; ++i)
              {
                DisplayWindowParameters * const pWindowDisplay = get_ptr_inline(sWindowDisplay, i, &ThreadStorageLock);
                assert(NULL != pWindowDisplay);
                if (NULL != pWindowDisplay)
                  {
                    BOOL const post = PostMessage(pWindowDisplay->hWnd, WM_COMMAND, DISPLAY_WINDOW_EXIT, 0);
                    assert(TRUE == post);
                  }
                /* if */
              }
            /* for */

            // Indicate exit from the command loop.
            exit = true;
          }
#pragma endregion // Terminate program

        break;

        }
      /* switch */
    }
  while (false == exit);


  /****** CLEANUP ******/

#pragma region // Cleanup

  /* Note the order of the clean-up is in general the opposite of the order of creation,
     e.g. a class/thread/resource that was created first is the last one to be destroyed etc.

     To prevent any dangling pointers all window data structures must be destroyed after
     all rendering and acquisition threads are stopped; however, note that actual windows
     may be closed (or invisible), only the datastructure is required as both rendering
     and image acquisition threads are pushing the data into rendering and live camera view windows.
     As it takes some time to close DXGI Swap Chains and exit exclusive fullscreen mode
     the DISPLAY_WINDOW_EXIT commands are posted first; this is allowed as window data
     structures are deleted after all rendering and acquisition threads are stopped.
  */

  for (int i = 0; i < (int)(sRendering.size()); ++i) RenderingThreadStop(get_ptr_inline(sRendering, i, &ThreadStorageLock));
  sRendering.clear();

  DisconnectFromAcquisitionThreads(pWindowPreview);

  for (int i = 0; i < (int)(sAcquisition.size()); ++i) AcquisitionThreadStop(get_ptr_inline(sAcquisition, i, &ThreadStorageLock));
  sAcquisition.clear();

  CloseVTKWindow(pWindowVTK);

  ClosePreviewWindow(pWindowPreview);

  for (int i = 0; i < (int)(sWindowDisplay.size()); ++i) CloseDisplayWindow(get_ptr_inline(sWindowDisplay, i, &ThreadStorageLock));
  sWindowDisplay.clear();

  for (int i = 0; i < (int)(sImageEncoder.size()); ++i) ImageEncoderStop(get_ptr_inline(sImageEncoder, i, &ThreadStorageLock));
  sImageEncoder.clear();

  for (int i = 0; i < (int)(sImageDecoder.size()); ++i) ImageDecoderStop(get_ptr_inline(sImageDecoder, i, &ThreadStorageLock));
  sImageDecoder.clear();

  for (int i = 0; i < (int)(sImageList.size()); ++i) DeleteImageFileList(get_ptr_inline(sImageList, i, &ThreadStorageLock));
  sImageList.clear();

  DeleteSynchronizationEventsStructure(pSynchronization);

  while ( !sConnectedCameras.empty() )
    {
      std::wstring * pCameraName = sConnectedCameras.back();
      sConnectedCameras.pop_back();
      SAFE_DELETE(pCameraName);
    }
  /* while */

  SAFE_DELETE(pAcquisitionTag);

  SAFE_RELEASE(pD2DFactory);
  SAFE_RELEASE(pDXGIFactory1);

  CoUninitialize();

#pragma endregion // Cleanup

  return EXIT_SUCCESS;
}
/* _tmain */



#endif /* !__BATCHACQUISITIONMAIN_CPP */
