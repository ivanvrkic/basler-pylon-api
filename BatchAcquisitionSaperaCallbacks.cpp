/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2014-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionSaperaCallbacks.cpp
  \brief  Callback functions for Teledyne Dalsa Sapera SDK.

  \author Tomislav Petkovic
  \date   2017-02-17
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONSAPERACALLBACKS_CPP
#define __BATCHACQUISITIONSAPERACALLBACKS_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionSaperaCallbacks.h"
#include "BatchAcquisitionEvents.h"



#ifdef HAVE_SAPERA_SDK

//! Prints event information.
/*!
  Prints event information for DEBUG build of the application.

  \param pInfo  Pointer to event information.
*/
inline
void
PrintCallbackInfo_inline(
                         SapAcqDeviceCallbackInfo * const pInfo
                         )
{

#ifdef DEBUG
  if (NULL != pInfo)
    {
      // Retrieve count, index and name of the received event.
      int eventCount = 0;
      int eventIndex = 0;
      char eventName[STRING_LENGTH];
      BOOL status = TRUE;
      LARGE_INTEGER performanceCounter;

      status = QueryPerformanceCounter( &performanceCounter );
      assert(TRUE == status);

      status = pInfo->GetEventCount(&eventCount);
      assert(TRUE == status);

      status = pInfo->GetEventIndex(&eventIndex);
      assert(TRUE == status);

      status = pInfo->GetAcqDevice()->GetEventNameByIndex(eventIndex, eventName, sizeof(eventName));
      assert(TRUE == status);

      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          int const CameraID = ptr->CameraID;
          Debugfprintf(stderr, gDbgCallbackInformationKnownCamera, CameraID + 1, eventName, eventIndex, eventCount, performanceCounter.QuadPart);
        }
      else
        {
          Debugfprintf(stderr, gDbgCallbackInformation, eventName, eventIndex, eventCount, performanceCounter.QuadPart);
        }
      /* if */
    }
  /* if */
#endif /* DEBUG */

}
/* PrintCallbackInfo_inline */



//! Register callback.
/*!
  Registers event callback. If there is previously registered callback it is
  unregistered prior to registering new callback.

  \param pDevice        Device pointer.
  \param eventName      Event name.
  \param callback       Callback function.
  \param context        Callback context.
  \return Returns TRUE if successfull, FALSE otherwise.
*/
BOOL
RegisterCallback(
                 SapAcqDevice * const pDevice,
                 char const * const eventName,
                 SapAcqDeviceCallback callback,
                 void * const context)
{
  assert(NULL != pDevice);
  if (NULL == pDevice) return FALSE;

  BOOL status = TRUE;

  /* Unregister any previously registered callback. */
  BOOL bIsRegistered = FALSE;

  if (TRUE == status)
    {
      status = pDevice->IsCallbackRegistered(eventName, &bIsRegistered);
      assert(TRUE == status);
    }
  /* if */

  if ( (TRUE == status) && (TRUE == bIsRegistered) )
    {
      status = pDevice->UnregisterCallback(eventName);
      assert(TRUE == status);
    }
  /* if */

  /* Register new callback. */
  if (TRUE == status)
    {
      status = pDevice->RegisterCallback(eventName, callback, context);
      assert(TRUE == status);
    }
  /* if */

  return status;
}
/* RegisterCallback */



//! Unregister all callbacks.
/*!
  Unregister all registered callbacks.

  \param pDevice        Pointer to device.
  \return Returns TRUE if successfull, FALSE otherwise.
*/
BOOL
UnregisterAllCallbacks(
                       SapAcqDevice * const pDevice
                       )
{
  assert(NULL != pDevice);
  if (NULL == pDevice) return FALSE;

  BOOL status = TRUE;

  int eventCount = 0;
  status = pDevice->GetEventCount(&eventCount);
  assert(TRUE == status);

  if (TRUE == status)
    {
      for (int eventIndex = 0; eventIndex < eventCount; ++eventIndex)
        {
          BOOL isRegistered = FALSE;

          status = pDevice->IsCallbackRegistered(eventIndex, &isRegistered);
          assert(TRUE == status);
          if (TRUE != status) break;

          if (TRUE ==  isRegistered)
            {
              status = pDevice->UnregisterCallback(eventIndex);
              assert(TRUE == status);
              if (TRUE != status) break;
            }
          /* if */
        }
      /* for */
    }
  /* if */

  return status;
}
/* UnregisterAllCallbacks */



//! Exposure begin callback.
/*!
  Callback will execute once sensor exposure for the current frame has begun.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackExposureBegin(
                            SapAcqDeviceCallbackInfo * pInfo
                            )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          assert(false == ptr->fExposureInProgress);
          ptr->fExposureInProgress = true;

          if (NULL != ptr->pSynchronization)
            {
              int const CameraID = ptr->CameraID;

              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READY, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

              BOOL const set_exposure_begin = ptr->pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, CameraID);
              assert(0 != set_exposure_begin);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  //PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackExposureBegin */



//! Exposure end callback.
/*!
  Callback will execute once sensor exposure for the current frame has ended.

  According to Genie documentation once exposure has ended new trigger may be sent
  to the camera (see pg.132 in Genie Operational Reference). However, there seems
  to be a delay when software trigger is ignored. Therefore we use the
  trigger failed callback to resend the trigger until triggering is successfull.

  Normal usage would be to send the camera trigger from the render thread
  and then to wait for the frame acquisition acknowledgment in the render thread.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackExposureEnd(
                          SapAcqDeviceCallbackInfo * pInfo
                          )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          assert(true == ptr->fExposureInProgress);
          ptr->fExposureInProgress = false;

          if (NULL != ptr->pSynchronization)
            {
              int const CameraID = ptr->CameraID;

              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READY, CameraID) );
              assert( true == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

              BOOL const set_exposure_end = ptr->pSynchronization->EventSet(CAMERA_EXPOSURE_END, CameraID);
              assert(0 != set_exposure_end);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  //PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackExposureEnd */



//! Readout begin callback.
/*!
  Callback will execute once readout of the current frame has begun.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackReadoutBegin(
                           SapAcqDeviceCallbackInfo * pInfo
                           )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if ( (NULL != ptr) && (NULL != ptr->pSynchronization) )
        {
          int const CameraID = ptr->CameraID;

          assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READOUT_BEGIN, CameraID) );
          assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READOUT_END, CameraID) );

          BOOL const set_readout_begin = ptr->pSynchronization->EventSet(CAMERA_READOUT_BEGIN, CameraID);
          assert(0 != set_readout_begin);
        }
      /* if */
    }
  /* if */

  PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackReadoutBegin */



//! Readout end callback.
/*!
  Callback will execute once readout of the current frame has ended.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackReadoutEnd(
                         SapAcqDeviceCallbackInfo * pInfo
                         )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if ( (NULL != ptr) && (NULL != ptr->pSynchronization) )
        {
          int const CameraID = ptr->CameraID;

          assert( true == DebugIsSignalled(ptr->pSynchronization, CAMERA_READOUT_BEGIN, CameraID) );
          assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READOUT_END, CameraID) );

          BOOL const set_readout_end = ptr->pSynchronization->EventSet(CAMERA_READOUT_END, CameraID);
          assert(0 != set_readout_end);
        }
      /* if */
    }
  /* if */

  PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackReadoutEnd */



//! Acquisition end callback
/*!
  Callback will execute once acquisition of the current frame has ended.
  Event will occur once Snap() method of Sapera LT has completed grabbing
  the requested number of frames or once Freeze() method of Sapera LT has
  stopped the continous acquisition.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackAcquisitionEnd(
                             SapAcqDeviceCallbackInfo * pInfo
                             )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if ( (NULL != ptr) && (NULL != ptr->pSynchronization) )
        {
          // Nothing to do!
        }
      /* if */
    }
  /* if */

  PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackAcquisitionEnd */



//! Invalid frame trigger callback.
/*!
  Callback will execute if invalid frame trigger is received.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackInvalidFrameTrigger(
                                  SapAcqDeviceCallbackInfo * pInfo
                                  )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          assert(false == ptr->fExposureInProgress);

          if (NULL != ptr->pSynchronization)
            {
              int const CameraID = ptr->CameraID;

              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_READY, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_EXPOSURE_END, CameraID) );
              assert( false == DebugIsSignalled(ptr->pSynchronization, CAMERA_INVALID_TRIGGER, CameraID) );

              BOOL const set_invalid_trigger = ptr->pSynchronization->EventSet(CAMERA_INVALID_TRIGGER, CameraID);
              assert(0 != set_invalid_trigger);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  //PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackInvalidFrameTrigger */



//! Frame skipped callback.
/*!
  Callback will execute if frame is skipped.

  \param pInfo  Pointer to event information.
*/
void
CameraCallbackFrameSkipped(
                           SapAcqDeviceCallbackInfo * pInfo
                           )
{
  assert(NULL != pInfo);
  if (NULL != pInfo)
    {
      AcquisitionParameters * const ptr = (AcquisitionParameters *)pInfo->GetContext();
      assert(NULL != ptr);
      if ( (NULL != ptr) && (NULL != ptr->pWindow) )
        {
          bool const fBlocking = ptr->pWindow->fBlocking; // True if acquisition is blocking.
          bool const fFixed = ptr->pWindow->fFixed; // True if fixed SL pattern is used.
          bool const fConcurrentDelay = ptr->pWindow->fConcurrentDelay; // True if delay is larger than exposure.

          int const CameraID = ptr->CameraID;

          Debugfprintf(stderr, gDbgCameraFrameSkipped, CameraID, __FILE__, __LINE__);

          if (true == fBlocking)
            {
              if (true == ptr->fThrottleDown)
                {
                  ptr->timeout += 50; // Increase timeout in 50ms steps.
                  Debugfprintf(stderr, gDbgCameraIncreaseTimeout, CameraID, ptr->timeout, __FILE__, __LINE__);
                }
              /* if */
              ptr->fThrottleDown = true;
            }
          /* if */
        }
      /* if */
    }
  /* if */

  PrintCallbackInfo_inline(pInfo);
}
/* CameraCallbackFrameSkipped */

#endif /* HAVE_SAPERA_SDK */



#endif /* !__BATCHACQUISITIONSAPERACALLBACKS_CPP */
