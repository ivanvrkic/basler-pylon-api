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
  \file   BatchAcquisitionAcquisition.cpp
  \brief  Image acquisition thread.

  \author Tomislav Petkovic
  \date   2017-02-13
*/

#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONACQUISITION_CPP
#define __BATCHACQUISITIONACQUISITION_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionFlyCapture2Registers.h"
#include "BatchAcquisitionFromFile.h"
#include "BatchAcquisitionDebug.h"



/****** HELPER FUNCTIONS ******/

#pragma region // Event name from code

//! Event names.
/*
  This static array contains all event names which acquisition thread processes.
  See function AcquisitionThread for event details.
*/
static
TCHAR const * const AcquisitionThreadEventNames[] = {
  /* 0 */ L"CAMERA_TERMINATE",
  /* 1 */ L"MAIN_PREPARE_CAMERA",
  /* 2 */ L"CAMERA_SEND_TRIGGER",
  /* 3 */ L"CAMERA_REPEAT_TRIGGER",
  /* 4 */ L"CAMERA_EXPOSURE_END",
  /* 5 */ L"CAMERA_TRANSFER_END",
  /* 6 */ L"CAMERA_CHANGE_ID",
  /* 7 */ L"hTimerExposureTimeout"
};



//! Get event name.
/*!
  Function returns pointer to a string which contains event name.

  \param hnr    Event code.
  \return Pointer to string or NULL pointer.
*/
inline
TCHAR const * const
GetAcquisitionThreadEventName_inline(
                                     int const hnr
                                     )
{
  switch (hnr)
    {
    case 0: return AcquisitionThreadEventNames[0];
    case 1: return AcquisitionThreadEventNames[1];
    case 2: return AcquisitionThreadEventNames[2];
    case 3: return AcquisitionThreadEventNames[3];
    case 4: return AcquisitionThreadEventNames[4];
    case 5: return AcquisitionThreadEventNames[5];
    case 6: return AcquisitionThreadEventNames[6];
    case 7: return AcquisitionThreadEventNames[7];
    }
  /* switch */
  return NULL;
}
/* GetAcquisitionThreadEventName_inline */

#pragma endregion // Event name from code


#pragma region // Blanking and destruction of AcquisitionParameters

//! Blanks acquisition thread parameters.
/*!
  Blanks acquisition thread parameters.

  \param P      Pointer to acquisition thread parametes.
*/
inline
void
AcquisitionParametersBlank_inline(
                                  AcquisitionParameters * const P
                                  )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Blank structure. */
  P->tAcquisition = (HANDLE)(NULL);

  P->CameraID = -1;
  P->ProjectorID = -1;

  P->fActive = false;
  P->fWaiting = false;
  P->fView = true;
  P->fExposureInProgress = false;
  P->fThrottleDown = false;
  P->timeout = 50;

  P->pStatisticsTriggerDuration = NULL;
  P->pStatisticsTriggerFrequency = NULL;
  P->pStatisticsAcquisitionDuration = NULL;

  P->pSynchronization = NULL;
  P->pWindow = NULL;
  P->pView = NULL;
  P->pImageEncoder = NULL;
  P->pImageDecoder = NULL;

  P->pMetadataQueue = NULL;

  P->trigger_counter = 0;

  P->vblank_counter_before_trigger_RT = -1;
  P->present_counter_before_trigger_RT = -1;

  P->key = -1;

  P->QPC_before_trigger_RT.QuadPart = (LONGLONG)0;
  P->QPC_after_trigger_RT.QuadPart = (LONGLONG)0;
  P->QPC_before_trigger_AT.QuadPart = (LONGLONG)0;
  P->QPC_after_trigger_AT.QuadPart = (LONGLONG)0;
  P->QPC_exposure_start.QuadPart = (LONGLONG)0;
  P->QPC_exposure_end_scheduled.QuadPart = (LONGLONG)0;

  P->pFilenameAT = NULL;

  ImageMetadataBlank( &(P->sImageMetadataAT) );

  ZeroMemory( &(P->sLockAT), sizeof(P->sLockAT) );

  P->pFlyCapture2SDK = NULL;
  P->pSaperaSDK = NULL;
  P->pPylonSDK = NULL;
  P->pFromFile = NULL;

  P->exposureTime_QPC = -1;
  P->exposureTime_requested_us = BATCHACQUISITION_qNaN_dv;
  P->exposureTime_achieved_us = BATCHACQUISITION_qNaN_dv;
  P->k = 1.0;
}
/* AcquisitionParametersBlank_inline */



//! Stops pending transfers.
/*!
  Stops pending image transfers.

  \param P      Pointer to acquisition thread parameters.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
StopPendingTransfers_inline(
                            AcquisitionParameters * const P
                            )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const stop_flycapture2 = AcquisitionParametersFlyCapture2StopTransfer(P->pFlyCapture2SDK, P->exposureTime_achieved_us, P->nFrames);
  assert(true == stop_flycapture2);

  bool const stop_sapera = AcquisitionParametersSaperaStopTransfer(P->pSaperaSDK, P->exposureTime_achieved_us, P->nFrames);
  assert(true == stop_sapera);

  bool const stop_pylon = AcquisitionParametersPylonStopTransfer(P->pPylonSDK, P->exposureTime_achieved_us, P->nFrames);
  assert(true == stop_pylon);

  bool const stop_fromfile = AcquisitionParametersFromFileStopTransfer(P->pFromFile);
  assert(true == stop_fromfile);

  bool const stop = stop_flycapture2 && stop_sapera && stop_fromfile;

  return stop;
}
/* StopPendingTransfers_inline */



//! Starts image transfers.
/*!
  Starts image transfers.

  \param P      Pointer to acquisition thread parameters.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
StartImageTransfers_inline(
                           AcquisitionParameters * const P
                           )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const start_flycapture2 = AcquisitionParametersFlyCapture2StartTransfer(P->pFlyCapture2SDK);
  assert(true == start_flycapture2);

  bool const start_sapera = AcquisitionParametersSaperaStartTransfer(P->pSaperaSDK);
  assert(true == start_sapera);

  bool const start_pylon = AcquisitionParametersPylonStartTransfer(P->ppylonSDK);
  assert(true == start_pylon);

  bool const start_fromfile = AcquisitionParametersFromFileStartTransfer(P->pFromFile, NULL);
  assert(true == start_fromfile);

  bool const start = start_flycapture2 && start_sapera && start_pylon && start_fromfile;

  return start;
}
/* StartImageTransfers_inline */



//! Releases acquisition thread parameters structure.
/*!
  Releases resources allocated by acquistion thread.

  \param P      Pointer to acquisition parametes.
*/
inline
void
AcquisitionParametersRelease_inline(
                                    AcquisitionParameters * const P
                                    )
{
  assert(NULL != P);
  if (NULL == P) return;

  // Stop pending transfers.
  bool const stop_transfers = StopPendingTransfers_inline(P);
  assert(true == stop_transfers);

  // Release FlyCapture2 SDK classes.
  AcquisitionParametersFlyCapture2Release(P->pFlyCapture2SDK);

  // Release Sapera SDK classes.
  AcquisitionParametersSaperaRelease(P->pSaperaSDK);

  // Release Pylon SDK classes.
  AcquisitionParametersPylonRelease(P->pPylonSDK);

  // Release dummy camera classes.
  AcquisitionParametersFromFileRelease(P->pFromFile);

  // Delete image metadata.
  SAFE_DELETE( P->pMetadataQueue );

  AcquireSRWLockExclusive( &(P->sLockAT) );
  {
    SAFE_DELETE( P->pFilenameAT );
    ImageMetadataRelease( &(P->sImageMetadataAT) );
  }
  ReleaseSRWLockExclusive( &(P->sLockAT) );

  // Delete statistics.
  FrameStatisticsDelete( P->pStatisticsTriggerDuration );
  FrameStatisticsDelete( P->pStatisticsTriggerFrequency );
  FrameStatisticsDelete( P->pStatisticsAcquisitionDuration );

  AcquisitionParametersBlank_inline( P );

  free(P);
}
/* AcquisitionParametersRelease_inline */

#pragma endregion // Blanking and destruction of AcquisitionParameters


#pragma region // Adjust exposure time

//! Adjust camera exposure time.
/*!
  Camera exposure time should be tied to the display refresh rate. This function
  compares last set camera exposure time to

  \param P      Pointer to AcquisitionParameters structure.
  \param override If true then camera refresh rate will be updated always.
  If false then the value will be sent to camera only when necessary.
*/
void
AdjustCameraExposureTime_inline(
                                AcquisitionParameters * const P,
                                bool const override
                                )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pWindow);
  if (NULL == P->pWindow) return;

  // Compute new exposure time.
  double const exposureTime_requested_us = CameraExposureTimeFromRefreshRate(P);
  bool const invalidNew = isnan_inline(exposureTime_requested_us);
  if (true == invalidNew) return;

  // If unchanged skip adjustment.
  if ( (false == override) && (exposureTime_requested_us == P->exposureTime_requested_us) )
    {
      assert(0 < P->exposureTime_QPC);
      assert(0 < P->exposureTime_requested_us);
      assert(0 < P->exposureTime_achieved_us);
      return;
    }
  /* if */

  double exposureTime_achieved_us = BATCHACQUISITION_qNaN_dv;
  if (NULL != P->pFlyCapture2SDK)
    {
      bool const set_flycapture2 = AcquisitionParametersFlyCapture2AdjustExposureTime(P->pFlyCapture2SDK, P->CameraID, exposureTime_requested_us, &exposureTime_achieved_us);
      assert(true == set_flycapture2);
      if (true == set_flycapture2)
        {
          double const exposureTime_max_us = (exposureTime_achieved_us > exposureTime_requested_us)? exposureTime_achieved_us : exposureTime_requested_us;
          P->exposureTime_QPC = (__int64)( exposureTime_max_us * P->pWindow->us_to_ticks + 0.5 );
          P->exposureTime_requested_us = exposureTime_requested_us;
          P->exposureTime_achieved_us = exposureTime_achieved_us;
        }
      /* if */
    }
  else if (NULL != P->pSaperaSDK)
    {
      bool const set_sapera = AcquisitionParametersSaperaAdjustExposureTime(P->pSaperaSDK, P->CameraID, exposureTime_requested_us, &exposureTime_achieved_us);
      assert(true == set_sapera);
      if (true == set_sapera)
        {
          double const exposureTime_max_us = (exposureTime_achieved_us > exposureTime_requested_us)? exposureTime_achieved_us : exposureTime_requested_us;
          P->exposureTime_QPC = (__int64)( exposureTime_max_us * P->pWindow->us_to_ticks + 0.5 );
          P->exposureTime_requested_us = exposureTime_requested_us;
          P->exposureTime_achieved_us = exposureTime_achieved_us;
        }
      /* if */
    }
  else if (NULL != P->pPylonSDK)
    {
      bool const set_pylon = AcquisitionParametersPylonAdjustExposureTime(P->pPylonSDK, P->CameraID, exposureTime_requested_us, &exposureTime_achieved_us);
      assert(true == set_pylon);
      if (true == set_pylon)
        {
          double const exposureTime_max_us = (exposureTime_achieved_us > exposureTime_requested_us)? exposureTime_achieved_us : exposureTime_requested_us;
          P->exposureTime_QPC = (__int64)( exposureTime_max_us * P->pWindow->us_to_ticks + 0.5 );
          P->exposureTime_requested_us = exposureTime_requested_us;
          P->exposureTime_achieved_us = exposureTime_achieved_us;
        }
      /* if */
    }
  else if (NULL != P->pFromFile)
    {
      bool const set_fromfile = AcquisitionParametersFromFileAdjustExposureTime(P->pFromFile, exposureTime_requested_us, &exposureTime_achieved_us);
      assert(true == set_fromfile);
      if (true == set_fromfile)
        {
          double const exposureTime_max_us = (exposureTime_achieved_us > exposureTime_requested_us)? exposureTime_achieved_us : exposureTime_requested_us;
          P->exposureTime_QPC = (__int64)( exposureTime_max_us * P->pWindow->us_to_ticks + 0.5 );
          P->exposureTime_requested_us = exposureTime_requested_us;
          P->exposureTime_achieved_us = exposureTime_achieved_us;
        }
      /* if */
    }
  else
    {
      // No camera attached.
      P->exposureTime_QPC = -1;
      P->exposureTime_requested_us = BATCHACQUISITION_qNaN_dv;
      P->exposureTime_achieved_us = BATCHACQUISITION_qNaN_dv;
    }
  /* if */
}
/* AdjustCameraExposureTime_inline */

#pragma endregion // Adjust exposure time



/****** HELPER FUNCTIONS FOR SYNCHRONIZATION ******/

#pragma region // Waitable timer for exposure timeout

//! Sets waitable timer for exposure timeout.
/*!
  On low-end network cards return event from the camera may be lost in transmission.
  So we use a timer to time-out image acquisition, and if the acquisition event is
  not received then we retrigger the camera thus restarting the acquisition.

  \param hTimer Timer handle.
  \param P      Pointer to acquisition thread parameters.
*/
inline
void
StartExposureTimeout_inline(
                            HANDLE const hTimer,
                            AcquisitionParameters * const P
                            )
{
  assert((HANDLE)(NULL) != hTimer);
  if ((HANDLE)(NULL) == hTimer) return;

  assert(NULL != P);
  if (NULL == P) return;

  if ( (NULL == P->pSaperaSDK) && (NULL == P->pFlyCapture2SDK) && (NULL == P->pPylonSDK) ) return;
  assert(NULL == P->pFromFile);

  assert(NULL != P->pWindow);
  if (NULL == P->pWindow) return;

  double const numerator = (double)(P->pWindow->sRefreshRate.Numerator);
  double const denominator = (double)(P->pWindow->sRefreshRate.Denominator);
  double const displayFrequency = numerator / denominator; // Hz (Hertz)
  double const exposureTime_s = P->k / displayFrequency; // s (seconds).
  double const waitTime = 10.0 * (10000000.0 * exposureTime_s); // in 100 ns (nanoseconds)
  assert(0 < waitTime);

  LONGLONG const minimalWaitTime = 50000000LL; // 5 seconds.
  LARGE_INTEGER liDueTime;
  if ( (waitTime > minimalWaitTime) && (0 < denominator) && (0 < numerator) && (0 < P->k) )
    {
      // Wait for a multiple of exposure time.
      liDueTime.QuadPart = -(LONGLONG)(waitTime);
    }
  else
    {
      // Wait for minimal wait time.
      liDueTime.QuadPart = -minimalWaitTime;
    }
  /* if */

  BOOL const timer = SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
  assert(TRUE == timer);
}
/* StartExposureTimeout_inline */



//! Stops exposure timeout and reset timer signal.
/*!
  There is no API call to stop and reset the timeout timer so once the exposure is complete
  we simply restart the timer with a maximal possible timeout time which is around 2^63 ticks.

  \param hTimer Timer handle.
*/
inline
void
StopExposureTimeout_inline(
                           HANDLE const hTimer
                           )
{
  assert((HANDLE)(NULL) != hTimer);
  if ((HANDLE)(NULL) == hTimer) return;

  LARGE_INTEGER liDueTime;
  liDueTime.QuadPart = LLONG_MIN;

  BOOL const timer = SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
  assert(TRUE == timer);
}
/* StopExposureTimeout_inline */

#pragma endregion // Waitable timer for exposure timeout


#pragma  region Wait for exposure end

//! Sleep until exposure ends.
/*!
  Function sleeps for required time until exposure ends.

  The exposure time starts from the last successfull camera trigger,
  therefore to wait till exposure ends we must first determine how much
  time has elpased from the trigger. Once this value is computed this
  function will execute SleepEx for requred number of milliseconds.
  If the time of the last trigger is unknown (indicated by value 0)
  then function will sleep for a full amout of exposure time.

  \param parameters     Pointer to acquisition thread parameters.
  \param pWindow        Pointer to display window parameters.
  \param QPC_after_trigger       QPC value after last successfull trigger.
*/
inline
void
SleepUntilExposureEnds_inline(
                              AcquisitionParameters * const parameters,
                              DisplayWindowParameters * const pWindow,
                              LARGE_INTEGER const QPC_after_trigger
                              )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(NULL != pWindow);
  if (NULL == pWindow) return;

  LARGE_INTEGER QPC_before_sleep;
  QPC_before_sleep.QuadPart = (LONGLONG)(-1);

  // Compute elapsed time.
  double elapsed_ms = 0.0;
  if ( (LONGLONG)0 < QPC_after_trigger.QuadPart )
    {
      BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_sleep );
      assert(TRUE == qpc_before);

      if ( (TRUE != qpc_before) || ((LONGLONG)(-1) == QPC_before_sleep.QuadPart) ) return;

      elapsed_ms = (double)(QPC_before_sleep.QuadPart - QPC_after_trigger.QuadPart) * pWindow->ticks_to_ms;
      assert(0 < elapsed_ms);
    }
  /*if  */

  // Compute remaining delay time.
  double remaining_ms = 0.0;
  double const exposureTime_max_us =
    (parameters->exposureTime_achieved_us > parameters->exposureTime_requested_us)?
    parameters->exposureTime_achieved_us : parameters->exposureTime_requested_us;
  double const exposureTime_max_ms = exposureTime_max_us * 0.001;
  if (0.0 < exposureTime_max_ms)
    {
      remaining_ms = exposureTime_max_ms - elapsed_ms;
      if (0.0 > remaining_ms) remaining_ms = 0.0;
    }
  /* if */

  // Sleep for required delay.
  if (0.0 < remaining_ms)
    {
      // Consider using spinlock timer for short delays.
      DWORD const sleep_time = (DWORD)( remaining_ms + 0.5 );
      SleepEx(sleep_time, TRUE);
    }
  else
    {
      assert(0.0 == remaining_ms);
      return;
    }
  /* if */

  // Total elapsed time must be larger than requested.
  {
    LONGLONG const stop = QPC_after_trigger.QuadPart + parameters->exposureTime_QPC;

    LARGE_INTEGER QPC_after_sleep;
    QPC_after_sleep.QuadPart = (LONGLONG)(-1);

    {
      BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_sleep );
      assert(TRUE == qpc_after);
    }

    if (QPC_after_sleep.QuadPart < stop)
      {
        do
          {
            BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_sleep );
            assert(TRUE == qpc_after);
          }
        while (QPC_after_sleep.QuadPart < stop);
      }
    /* if */

    assert(QPC_after_sleep.QuadPart >= stop);
  }
}
/* SleepUntilExposureEnds_inline */

#pragma  endregion // Wait for exposure end


#pragma region // Check duration of each event

//! Check duration of event.
/*!
  Function checks duration of each event and outputs message to the console if event takes longer than expected.

  \param event_code     Event code.
  \param event_duration_ms      Time spent processing the event.
  \param parameters     Pointer to acquisition thread parameters.
*/
inline
void
CheckEventDuration_inline(
                          int const event_code,
                          double const event_duration_ms,
                          AcquisitionParameters * const parameters
                          )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  bool const have_FlyCapture2SDK = (NULL != parameters->pFlyCapture2SDK);

  double const exposureTime_ms = parameters->exposureTime_requested_us * 0.001 ;

  double expected_duration_ms = exposureTime_ms;
  if ( (true == have_FlyCapture2SDK) && (4 == event_code) )
    {
      // Adjust exposure time for CAMERA_EXPOSURE_END event which executes wait operation for FlyCapture2 SDK.
      double offset = exposureTime_ms * 1.5;
      if (offset < 100.0) offset = 100.0;
      expected_duration_ms += offset;
    }
  /* if */

  if (event_duration_ms > expected_duration_ms)
    {
      TCHAR const * const event_name = GetAcquisitionThreadEventName_inline(event_code);
      double const precentage = 100.0 * event_duration_ms / exposureTime_ms;
      if (NULL != event_name)
        {
          Debugfwprintf(stderr, gDbgEventProcessingTooLong, parameters->CameraID + 1, event_name, precentage);
        }
      /* if */
    }
  /* if */
}
/* CheckEventDuration_inline */

#pragma endregion // Check duration of each event



/****** HELPER FUNCTIONS FOR EVENT DISPATCH ******/

#pragma region // Event dispatcher for CAMERA_SEND_TRIGGER and CAMERA_REPEAT_TRIGGER events

//! Logic for event dispatch after triggering.
/*!
  This inline function contains logic for dispatching events after triggering.
  It is called from event processing code for CAMERA_SEND_TRIGGER and CAMERA_REPEAT_TRIGGER events.

  \param parameters     Pointer to acquisition thread parameters structure.
  \param pSynchronization       Pointer to event synchronization structure.
  \param fBlocking      Flag which indicates if acquisition mode is blocking or non-blocking.
  \param fFixed   Flag which indicates if fixed or non-fixed SL pattern is used.
  \param fConcurrentDelay       Flag which indicates if delay time is larger than camera exposure time.
  \param triggered      Flag which indicates if camera was successfully triggered.
*/
void
DispatchEventsAfterTrigger_inline(
                                  AcquisitionParameters * const parameters,
                                  SynchronizationEvents * const pSynchronization,
                                  bool const fBlocking,
                                  bool const fFixed,
                                  bool const fConcurrentDelay,
                                  bool const triggered
                                  )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return;

  bool const have_FlyCapture2SDK = (NULL != parameters->pFlyCapture2SDK);
  bool const have_SaperaSDK = (NULL != parameters->pSaperaSDK);
  bool const have_PylonSDK = (NULL != parameters->pPylonSDK);
  bool const have_FromFile = (NULL != parameters->pFromFile);

  int const CameraID = parameters->CameraID;
  int const ProjectorID = parameters->ProjectorID;

  // Signal appropriate event depending on the acquisition mode and trigger status.
  if (false == fFixed)
    {
      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
      assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

      if (true == fBlocking)
        {
          /* If triggering failed in blocking mode then we always send CAMERA_REPEAT_TRIGGER event to re-trigger the camera.

             If the triggering succedded we always signal CAMERA_EXPOSURE_BEGIN and CAMERA_EXPOSURE_END events
             together with one of DRAW_PRESENT or DRAW_RENDER events depending on the value of fConcurrentDelay flag.

             For Sapera SDK CAMERA_EXPOSURE_BEGIN and CAMERA_EXPOSURE_END events are dispatched by callback functions;
             otherwise they are dispatched here.
          */
          if (true == triggered)
            {
              if (false == have_SaperaSDK)
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );

                  BOOL const set_exposure_begin = pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, CameraID);
                  assert(0 != set_exposure_begin);
                }
              else // !(false == have_SaperaSDK)
                {
                  // For Sapera SDK event should already be dispatched by the callback functions.
                  assert( true == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
                }
              /* if (false == have_SaperaSDK) */

              if (true == fConcurrentDelay)
                {
                  /* Event cycle is
                     ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->...
                     so the next events are DRAW_PRESENT in the cycle and CAMERA_EXPOSURE_END as a branch.
                     Before signalling DRAW_PRESENT we have to wait for DRAW_PRESENT_READY.
                  */

                  DWORD const dwIsReadyResult = pSynchronization->EventWaitForAny(
                                                                                  DRAW_PRESENT_READY,  ProjectorID, // 0
                                                                                  CAMERA_TERMINATE,    CameraID,    // 1
                                                                                  MAIN_PREPARE_CAMERA, CameraID,    // 2
                                                                                  INFINITE // Wait forever.
                                                                                  );
                  int const hnr_ready = dwIsReadyResult - WAIT_OBJECT_0;
                  if (0 == hnr_ready) // DRAW_PRESENT_READY
                    {
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                      BOOL const set_present = pSynchronization->EventSetConditional(DRAW_PRESENT, ProjectorID);
                      assert(0 != set_present);
                    }
                  else if (1 == hnr_ready) // CAMERA_TERMINATE
                    {
                      Debugfprintf(stderr, dDbgDropPresentForProjectorDueToCameraTerminate, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else if (2 == hnr_ready) // MAIN_PREPARE_CAMERA
                    {
                      Debugfprintf(stderr, dDbgDropPresentForProjectorDueToMainPrepareCamera, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else
                    {
                      Debugfprintf(stderr, dDbgDropPresentForProjector, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  /* if */
                }
              else // !(true == fConcurrentDelay)
                {
                  /* Event cycle is
                     ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->...
                     so the next events are CAMERA_EXPOSURE_END in the cycle and DRAW_RENDER as a branch.
                     Before signalling DRAW_RENDER we have to wait for DRAW_RENDER_READY.
                  */

                  DWORD const dwIsReadyResult = pSynchronization->EventWaitForAny(
                                                                                  DRAW_RENDER_READY,   ProjectorID, // 0
                                                                                  CAMERA_TERMINATE,    CameraID,    // 1
                                                                                  MAIN_PREPARE_CAMERA, CameraID,    // 2
                                                                                  INFINITE // Wait forever.
                                                                                  );
                  int const hnr_ready = dwIsReadyResult - WAIT_OBJECT_0;
                  if (0 == hnr_ready) // DRAW_RENDER_READY
                    {
                      assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                      BOOL const set_render = pSynchronization->EventSetConditional(DRAW_RENDER, ProjectorID);
                      assert(0 != set_render);
                    }
                  else if (1 == hnr_ready)
                    {
                      Debugfprintf(stderr, dDbgDropRenderForProjectorDueToCameraTerminate, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else if (2 == hnr_ready)
                    {
                      Debugfprintf(stderr, dDbgDropRenderForProjectorDueToMainPrepareCamera, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else
                    {
                      Debugfprintf(stderr, dDbgDropRenderForProjector, CameraID + 1, ProjectorID + 1,  __FILE__, __LINE__);
                    }
                  /* if */
                }
              /* if (true == fConcurrentDelay) */


              if (false == have_SaperaSDK)
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

                  BOOL const set_exposure_end = pSynchronization->EventSet(CAMERA_EXPOSURE_END, CameraID);
                  assert(0 != set_exposure_end);

                }
              else // !(false == have_SaperaSDK)
                {
                  // For Sapera SDK event is dispatched by callback function.
                }
              /* if (false == have_SaperaSDK) */
            }
          else // !(true == triggered)
            {
              assert( false == parameters->fExposureInProgress );
              assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

              BOOL const set_repeat_trigger = pSynchronization->EventSet(CAMERA_REPEAT_TRIGGER, CameraID);
              assert(0 != set_repeat_trigger);
            }
          /* if (true == triggered) */
        }
      else // !(true == fBlocking)
        {
          /* For non-blocking acquisition there are no DRAW_* events which must be signalled.
             If the trigger succeded we signal CAMERA_EXPOSURE_END either here or in a callback
             function for Sapera SDK. If the trigger failed then we simply signal CAMERA_READY.
          */
          if (true == triggered)
            {
              if (false == have_SaperaSDK)
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );

                  BOOL const set_exposure_begin = pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, CameraID);
                  assert(0 != set_exposure_begin);

                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

                  BOOL const set_exposure_end = pSynchronization->EventSet(CAMERA_EXPOSURE_END, CameraID);
                  assert(0 != set_exposure_end);
                }
              else // !(false == have_SaperaSDK)
                {
                  // For Sapera SDK the CAMERA_EXPOSURE_BEGIN and CAMERA_EXPOSURE_END events are dispatched by callback functions.
                }
              /* if (false == have_SaperaSDK) */
            }
          else // !(true == triggered)
            {
              /* Frame is dropped so CAMERA_READY may be raised immediately. */
              assert( false == parameters->fExposureInProgress );
              assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

              BOOL const set_ready = pSynchronization->EventSet(CAMERA_READY, CameraID);
              assert(0 != set_ready);
            }
          /* if (true == triggered) */
        }
      /* if (true == fBlocking) */
    }
  else // !(false == fFixed)
    {
      /* Two event cycles for a fixed SL pattern depending on the value of fBlocking flag are
         ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->CAMERA_SYNC_TRIGGERS->...
         and
         ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_SYNC_TRIGGERS->...

         Regardless of the value of fBlocking flag the next event is either CAMERA_REPEAT_TRIGGER
         or CAMERA_EXPOSURE_END depending on trigger success.
      */

      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
      assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

      if (true == triggered)
        {
          if (false == have_SaperaSDK)
            {
              assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );

              BOOL const set_exposure_begin = pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, CameraID);
              assert(0 != set_exposure_begin);

              assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

              BOOL const set_exposure_end = pSynchronization->EventSet(CAMERA_EXPOSURE_END, CameraID);
              assert(0 != set_exposure_end);
            }
          else // !(false == have_SaperaSDK)
            {
              // For Sapera SDK the CAMERA_EXPOSURE_BEGIN and CAMERA_EXPOSURE_END events are dispatched by callback functions.
            }
          /* if (false == have_SaperaSDK) */
        }
      else // !(true == triggered)
        {
          assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

          BOOL const set_repeat_trigger = pSynchronization->EventSet(CAMERA_REPEAT_TRIGGER, CameraID);
          assert(0 != set_repeat_trigger);
        }
      /* if (true == triggered) */
    }
  /* if (false == fFixed) */
}
/* DispatchEventsAfterTrigger_inline */

#pragma endregion // Event dispatcher for CAMERA_SEND_TRIGGER and CAMERA_REPEAT_TRIGGER events



/****** ACQUISITION THREAD ******/

//! Acquisition thread.
/*!
  Acquisition thread.

  \param parameters_in Pointer to structure holding acquisition thread parameters.
  \return Returns 0 if successfull.
*/
unsigned int
__stdcall
AcquisitionThread(
                  void * parameters_in
                  )
{

#pragma region // Initialization

  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  AcquisitionParameters * const parameters = (AcquisitionParameters *)parameters_in;

  SetThreadNameAndIDForMSVC(-1, "AcquisitionThread", parameters->CameraID);

  // Fetch parameters.
  SynchronizationEvents * const pSynchronization = parameters->pSynchronization;
  assert(NULL != pSynchronization);

  DisplayWindowParameters * const pWindow = parameters->pWindow;
  assert(NULL != pWindow);

  ImageEncoderParameters * const pImageEncoder = parameters->pImageEncoder;
  assert(NULL != pImageEncoder);

  ImageMetadataQueue * const pMetadataQueue = parameters->pMetadataQueue;
  assert(NULL != pMetadataQueue);

  FrameStatistics * const pStatisticsTriggerDuration = parameters->pStatisticsTriggerDuration;
  assert(NULL != pStatisticsTriggerDuration);

  FrameStatistics * const pStatisticsTriggerFrequency = parameters->pStatisticsTriggerFrequency;
  assert(NULL != pStatisticsTriggerFrequency);

  FrameStatistics * const pStatisticsAcquisitionDuration = parameters->pStatisticsAcquisitionDuration;
  assert(NULL != pStatisticsAcquisitionDuration);

  AcquisitionParametersFlyCapture2 * const pFlyCapture2SDK = parameters->pFlyCapture2SDK;
  //assert(NULL != pFlyCapture2SDK);

  AcquisitionParametersSapera * const pSaperaSDK = parameters->pSaperaSDK;
  //assert(NULL != pSaperaSDK);
  AcquisitionParametersPylon * const pPylonSDK = parameters->pPylonSDK;
  //assert(NULL != pPylonSDK);

  AcquisitionParametersFromFile * const pFromFile = parameters->pFromFile;
  //assert(NULL != pFromFile);

  bool const have_FlyCapture2SDK = (NULL != pFlyCapture2SDK); // True if FlyCapture2 SKD is used.
  bool const have_SaperaSDK = (NULL != pSaperaSDK); // True if Sapera SDK is used.
  bool const have_PylonSDK = (NULL != pPylonSDK); // True if Pylon SDK is used.
  bool const have_FromFile = (NULL != pFromFile); // True if dummy from file acquisition is used.
  
  assert( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) || (true == have_PylonSDK) || (true == have_FromFile) );

  int CameraID = parameters->CameraID;
  assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

  int ProjectorID = parameters->ProjectorID;
  assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

  int EncoderID = pImageEncoder->EncoderID;
  assert( (0 <= EncoderID) && (EncoderID <= (int)(pSynchronization->ImageEncoder.size())) );
  assert( CameraID == pImageEncoder->CameraID );

  PastEvents * const pEvents = PastEventsCreate();
  assert(NULL != pEvents);

  // Initialize variables.
  bool continueLoop = true;

  ImageMetadata sImageMetadata;
  ImageMetadataBlank( &sImageMetadata );

  LARGE_INTEGER QPC_before_trigger;
  QPC_before_trigger.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_after_trigger;
  QPC_after_trigger.QuadPart = (LONGLONG)0;

  long int trigger_counter = 0; // Counter of triggers sent to camera.

  std::wstring * pFileSuffix = NULL;

  // Create spinlock timer.
  SpinlockTimer * pTimer = SpinlockTimerCreate();
  assert(NULL != pTimer);

  LARGE_INTEGER QPC_spinlock_start;
  QPC_spinlock_start.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_spinlock_stop;
  QPC_spinlock_stop.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_spinlock_limit;
  QPC_spinlock_limit.QuadPart = (LONGLONG)0;

  bool wait_for_vblank = false;
  bool use_absolute_timing = false;

  bool use_software_delay = false;
  bool use_hardware_delay = false;

  double hardware_delay_ms = BATCHACQUISITION_qNaN_dv;

  // Create waitable timer.
  HANDLE const hTimerExposureTimeout = CreateWaitableTimer(NULL, FALSE, NULL);
  assert((HANDLE)(NULL) != hTimerExposureTimeout);

  // Raise thread priority.
  BOOL const priority = SetThreadPriority(parameters->tAcquisition, THREAD_PRIORITY_HIGHEST);
  assert( TRUE == priority );

  parameters->fActive = true;

#pragma endregion // Initialization


  /* Events are processed in an infinite loop. */
  do
    {
      assert(NULL != pSynchronization);
      if ( (NULL != pSynchronization) &&
           (NULL != pWindow) &&
           (NULL != pImageEncoder)
           )
        {
          assert(false == parameters->fWaiting);
          parameters->fWaiting = true;

          /* If event ordering is changed here then event processing code which uses hnr,
             static array AcquisitionThreadEventNames, and function GetAcquisitionThreadEventName_inline
             must be updated as well.
          */
          DWORD const dwWaitResult =
            pSynchronization->EventWaitForAny(
                                              pSynchronization->GetEventHandle(CAMERA_TERMINATE,      CameraID), // 0
                                              pSynchronization->GetEventHandle(MAIN_PREPARE_CAMERA,   CameraID), // 1
                                              pSynchronization->GetEventHandle(CAMERA_SEND_TRIGGER,   CameraID), // 2
                                              pSynchronization->GetEventHandle(CAMERA_REPEAT_TRIGGER, CameraID), // 3
                                              pSynchronization->GetEventHandle(CAMERA_EXPOSURE_END,   CameraID), // 4
                                              pSynchronization->GetEventHandle(CAMERA_TRANSFER_END,   CameraID), // 5
                                              pSynchronization->GetEventHandle(CAMERA_CHANGE_ID,      CameraID), // 6
                                              hTimerExposureTimeout, // 7
                                              INFINITE // Wait forever.
                                              );
          int const hnr = dwWaitResult - WAIT_OBJECT_0;
          assert( (0 <= hnr) && (hnr < 8) );
          AddEvent(pEvents, hnr);

          parameters->fWaiting = false;

          /* DESCRIPTION OF THE ACQUISITION THREAD EVENT PROCESSING

             The acquisition thread processes an event immediately after it is signalled.
             Events are always signalled by the rendering thread which waits for acquisition to end.
             Therefore there is no need to maintain a queue of events.
             Immediate processing is (almost always) ensured by the thread priority
             which is set to THREAD_PRIORITY_HIGHEST. If no events are signalled then
             thread is idle and does not consume processor time.

             Acquisition thread processes the following events:
             1) CAMERA_TERMINATE - the acquistion thread shuld terminate,
             2) MAIN_PREPARE_CAMERA - the acquisition thread should stop current actions and prepare for batch acquisition,
             3) CAMERA_SEND_TRIGGER - the acquisition thread should trigger the camera,
             4) CAMERA_REPEAT_TRIGGER - the acquisition thread should retrigger the camera,
             5) CAMERA_EXPOSURE_END - the acquisition thread should indicate the camera is ready for next trigger,
             6) CAMERA_TRANSFER_END - the acquisition thread should indicate the image data was transferred from the camera,
             7) CAMERA_CHANGE_ID - changes event IDs, and
             8) hTimerExposureTimeout - the acquisition thread should perform camera diagnostic and try to restart it.

             The order in which events are signalled depends on the selected acquisition mode.
             There are several flags which control the acquisition mode and the type of the SL pattern.
             These are:
             1) fBlocking - indicates if acquisition is blocking or non-blocking,
             2) fFixed - indicates if one image SL pattern is used,
             3) fConcurrentDelay - indicates if delay time is larger or shorter than camera exposure.

             Every acquisition mode has its cycle of events which is defined by aforementioned
             flags and is executed by acquisition and rendering threads. Here we describe the
             event processing logic for the acquistion thread; for the description of
             the rendering thread logic see comments in the file BatchAcquisitionRendering.cpp.


             BLOCKING ACQUISITION MODE

             The blocking acquisition mode uses a causal sequence of events which requires that all previous
             operations complete successfully before the next operation is executed. Due to such constraint
             any delay in program execution simply extends the run time; no frames will be dropped.

             Blocking acquisition mode is indicated by the true value of fBlocking flag.

             There are two cycles of events in the blocking acquisition mode which depend on the value
             of the delay time and of the camera exposure time; this relationship is indicated by the
             fConcurrentDelay flag which is true if delay time is larger then camera exposure time.

             If the delay time is larger than the camera exposure time (fConcurrentDelay is true)
             the the causal event loop is
             ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->...
             Due to causal constraint the acquisition thread sends DRAW_PRESENT only after the
             camera confirms the trigger operation was successfull. If the trigger operation fails
             then the acquisition thread will signal to itself CAMERA_REPEAT_TRIGGER until the
             camera is successfully triggered. Presenting the frame immediately after the trigger
             succedded is allowed as the exposure time is shorter than delay time so frames cannot
             mix during the exposure.

             If the delay time is shorter than the camera exposure time (fConcurrentDelay is false)
             then the causal event loop is
             ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->...
             Here the camera sends DRAW_RENDER event immediately after a successfull trigger and then waits
             for the image acquisition and image transfer to end. Only after the image is acquired and transfered
             is the DRAW_PRESENT signalled. Again, this ensures adjacent frames cannot mix during the exposure.


             NON-BLOCKING ACQUISITION MODE

             In non-blocking acquisition mode the acquisition thread receives the CAMERA_SEND_TRIGGER signal
             from the rendering thread. When this signal is received the event sequence of the acquisition thread
             is a side-branch of the event cycle and is
             CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END
             where CAMERA_REPEAT_TRIGGER may be invoked only if there is sufficient time remaining for the
             exposure to complete. During the execution of this sequence of events the CAMERA_READY event is not armed;
             that prevents the rendering thread from triggering the camera while it is busy. The rendering thread will
             raise the CAMERA_SEND_TRIGGER event only if CAMERA_READY is signalled;
             therefore if camera is not ready when it need to be triggered then frames may be dropped.

             Non-blocking acquisition mode is indicated by the false value of fBlocking flag.
             Flag fConcurrentDelay has no effect in non-blocking acquisition mode.


             FIXED SL PATTERN

             Fixed SL pattern uses only one image which may be recored as many times as necessary.
             When a fixed SL pattern is used it is sufficient to render the the pattern once;
             camera then may be triggered as fast as possible as synchronization is unnecessary.

             When a fixed SL pattern is used the acquisition always starts by the event sequence
             MAIN_PREPARE_DRAW->MAIN_BEGIN->DRAW_RENDER->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->...
             afer which the rendering thread has nothing to render and present.

             After cameras are triggered for the first time the event cycles depend on the value
             of the fBlocking flag. In blocking acquisition mode the cameras will be triggered after the
             image is transfered to the PC while in non-blocking mode cameras will be triggered
             immediately after exposures are complete.

             For blocking acquisition the event cycle is
             ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->CAMERA_SYNC_TRIGGERS->...

             For non-blocking acquisition the event cycle is
             ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_SYNC_TRIGGERS->...


             STARTING AND STOPPING THE CYCLE

             All listed event cycles do not include the start-up sequence. The start-up sequence
             is controled in the rendering thread via events MAIN_PREPARE_DRAW and MAIN_BEGIN.

             Any particular acquisition thread is always slaved to exactly one rendering thread.
             That particular rendering thread controls the acquisition thread and upon receiving
             the MAIN_PREPARE_DRAW event will forward a corresponding MAIN_PREPARE_CAMERA event
             and will then wait for the acquisition thread to acknowledge the event was completed
             and the thread is ready via the MAIN_READY_CAMERA event.


             SIGNALLING THE END OF BATCH ACQUISITION

             To signal the end of the batch acquistion which was started via MAIN_BEGIN event
             two events are used, MAIN_END_CAMERA and MAIN_END_DRAW. The main thread which issued
             the MAIN_BEGIN event needs only to wait on MAIN_END_DRAW of the corresponding rendering
             thread as MAIN_END_CAMERA events are tested for internally, either in the
             rendering or in the acquistion thread. The exact place where MAIN_END_DRAW and
             MAIN_END_CAMERA events are raised depends on the acqisition mode.


             1) Ending the blocking acquisition mode

             In blocking acquistion mode all projected frames are always captured by design.
             The event MAIN_END_DRAW is therefore raised in the rendering thread once all
             MAIN_END_CAMERA events are raised in the callback transfer functions after the
             last frame is acquired and successfully transfered from the camera.


             2) Ending the non-blocking acquisition mode

             In non-blocking acquisition mode some projected frames may be dropped. If the last
             frame in the sequence is dropped then the MAIN_END_CAMERA event cannot be raised at all.
             Therefore, in non-blocking acquisition mode only reliable place where the MAIN_END_DRAW
             can be signalled is after all images are presented by the rendering thread. However,
             at that time all images are not yet captured so the rendering thread does not immediately
             signal MAIN_END_DRAW but instead it waits for a pre-specified time for MAIN_END_CAMERA
             events to be signalled. If all MAIN_END_CAMERA events are signalled then the last
             frame was successfully captured, otherwise the last frame was dropped for at least one
             camera.


             3) Ending the acquisition for a fixed SL pattern

             When a fixed SL pattern is used the rendering thread has nothing particular to do once
             the frame is rendered as almost all work is performed by the acquisition thread.
             The only event executed by the rendering thread is CAMERA_SYNC_TRIGGERS to ensure
             multiple cameras are synchronously triggered. In this case we use both MAIN_END_CAMERA
             and MAIN_END_DRAW events. First, MAIN_END_CAMERA events will be signalled from the
             image transfer callbacks after the last frame is acquired. After triggering the camera
             for the last requested frame the acquisition threads will wait for MAIN_END_CAMERA events.
             The normal event cycle for a fixed SL pattern will than continue with the
             CAMERA_SYNC_TRIGGERS event which will raise the MAIN_END_DRAW event and stop
             the acquisition.


             DIFFERENCES BETWEEN CAMERA DRIVES

             There exist several different camera drives which may be used. These are:

             1) FlyCapture2 API (PointGrey's cameras),
             2) Sapera API (Teledyne Dalsa's cameras),
             2) Pylon API (Basler's cameras), and
             3) dummy acquisition from file.

             A normal sequence of events for each camera for the acquisition of one frame is:
             CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END


             1) FlyCapture2 API

             FlyCapture2 API does not provide access to internal state of the attached camera during frame acquisition;
             the CAMERA_EXPOSURE_END event therefore cannot be signalled by the API and must be simulated.
             For FlyCapture2 API after the CAMERA_SEND_TRIGGER event is processed we immediately
             raise the CAMERA_EXPOSURE_END event in which we wait for the expected exposure time to elapse.
             The CAMERA_READY event may be raised as we may assume the exposure is complete.

             Fortunately, when triggering the camera the FlyCapture2 API provides the return information
             about trigger success immediately if the attached camera is of type IEEE1394 IIDC or USB3Vision.
             This means we do not need a separate trigger timeout routine to catch missed triggers.

             Regarding the data transfer from the camera the FlyCapture2 API uses a callback function
             which is executed once the data transfer is complete. This callback is used to signal the
             CAMERA_TRANSFER_END event. Note that image data transfer and triggering is not synchronous
             for the FlyCapture2 API meaning that CAMERA_TRANSFER_END event may be raised significantly
             later after the exposure is completed; e.g. in non-blocking mode CAMERA_TRANSFER_END
             events may be signalled only after two or more additional images are acquired.
             Therefore the CAMERA_TRANSFER_END event is usable only in the blocking acquisition mode.

             2) Sapera API

             Sapera API allowes the user to define callback functions which will be called once
             a specific event occures. This enables us to raise CAMERA_REPEAT_TRIGGER, CAMERA_EXPOSURE_BEGIN,
             CAMERA_EXPOSURE_END, and CAMERA_TRANSFER_END events via callback functions making the event
             logic of the acquisition thread simpler than for FlyCapture2 API.
             All callbacks are defined in the file BatchAcquisitionSaperaCallbacks.cpp.

             Unfortunately, if GigEVision cameras are used then the success of trigger operation only
             means that the triggering command was successfully sent over the network; the trigger
             command may yet fail after it was sent. The sapera API provides two named callbacks for
             this situation, "InvalidFrameTrigger" and "FrameSkipped", however, in certain situations
             none of them will be signalled even if the trigger failed. Therefore we have to use an
             additional timer event hTimerExposureTimeout which will be signalled after a pre-specified
             time for a successfull trigger elapses. Depending on the camera status we may then try
             to re-trigger the camera or abort the acquisition.

             3) Pylon API
             TODO

             4) Dummy acquisition from file

             This is a simple driver which reads the image data from a file. When acquisition from file is used
             the CAMERA_SEND_TRIGGER events immediately raises the CAMERA_EXPOSURE_END event.
             During the CAMERA_EXPOSURE_END event the image data is read from file.
          */


#pragma region // Get acquisition state

          bool const fBlocking = pWindow->fBlocking; // True if acquisition is blocking.
          bool const fFixed = pWindow->fFixed; // True if fixed SL pattern is used.
          bool const fConcurrentDelay = pWindow->fConcurrentDelay; // True if delay is larger than exposure.

          long int const key = parameters->key; // Present counter value of the current frame.
          assert(0 <= key);

#pragma endregion // Get acquisition state


          if (0 == hnr)
            {
              // We received terminate event.
              continueLoop = false;
            }
          else if (1 == hnr)
            {
              /****** PREPARE FOR BATCH ACQUISITION ******/

              /* Preparation for batch acquisition is the same for all acquisition modes.

                 After the preparation is completed we raise the MAIN_READY_CAMERA event to signal this.
                 Event MAIN_READY_CAMERA is consumed by the rendering thread which will in turn
                 signal the MAIN_READY_DRAW after both the acquisition and the rendering thread are ready.
              */

#pragma region // Process MAIN_PREPARE_CAMERA event

              // Disarm MAIN_PREPARE_CAMERA and CAMERA_READY events.
              {
                assert( false == DebugIsSignalled(pSynchronization, MAIN_BEGIN, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, MAIN_READY_CAMERA, CameraID) );

                BOOL const reset_prepare_camera = pSynchronization->EventReset(MAIN_PREPARE_CAMERA, CameraID);
                assert(0 != reset_prepare_camera);

                BOOL const reset_camera_ready = pSynchronization->EventReset(CAMERA_READY, CameraID);
                assert(0 != reset_camera_ready);
              }

              // Signal to image encoder to process all images.
              {
                BOOL const set_process = pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_PROCESS, EncoderID);
                assert(0 != set_process);
              }

              // Complete all pending transfers.
              {
                bool const stop_transfers = StopPendingTransfers_inline(parameters);
                assert(true == stop_transfers);
              }

              // Stop exposure timeout timer.
              StopExposureTimeout_inline(hTimerExposureTimeout);

              // Reset thread state.
              {
                ImageMetadataRelease( &sImageMetadata );

                QPC_before_trigger.QuadPart = (LONGLONG)0;
                QPC_after_trigger.QuadPart = (LONGLONG)0;

                trigger_counter = 0;

                SAFE_DELETE( pFileSuffix );

                QPC_spinlock_start.QuadPart = (LONGLONG)0;
                QPC_spinlock_stop.QuadPart = (LONGLONG)0;
                QPC_spinlock_limit.QuadPart = (LONGLONG)0;

                wait_for_vblank = false;
                use_absolute_timing = false;
                use_software_delay = false;
                use_hardware_delay = false;
                hardware_delay_ms = BATCHACQUISITION_qNaN_dv;

                FrameStatisticsReset(pStatisticsTriggerDuration);
                FrameStatisticsReset(pStatisticsTriggerFrequency);
                FrameStatisticsReset(pStatisticsAcquisitionDuration);

                parameters->fExposureInProgress = false;
                parameters->fView = false;

                AcquireSRWLockExclusive( &(parameters->sLockAT) );
                {
                  ImageMetadataRelease( &(parameters->sImageMetadataAT) );
                  assert( QI_UNKNOWN_TYPE == parameters->sImageMetadataAT.render_type );
                  assert( false == parameters->sImageMetadataAT.fBatch );
                  assert( NULL == parameters->sImageMetadataAT.pFilename );

                  parameters->trigger_counter = trigger_counter;

                  parameters->QPC_before_trigger_AT.QuadPart = (LONGLONG)0;
                  parameters->QPC_after_trigger_AT.QuadPart = (LONGLONG)0;
                  parameters->QPC_exposure_start.QuadPart = (LONGLONG)0;
                  parameters->QPC_exposure_end_scheduled.QuadPart = (LONGLONG)-1;
                }
                ReleaseSRWLockExclusive( &(parameters->sLockAT) );
              }

              // Restart image transfers.
              {
                bool const start_transfers = StartImageTransfers_inline(parameters);
                assert(true == start_transfers);
              }

              // Adjust timings.
              {
                // Force new exposure time.
                AdjustCameraExposureTime_inline(parameters, true);

                // Re-compute trigger delays for non-blocking acquisition mode.
                double exposureTime_us = parameters->exposureTime_achieved_us;
                if (true == isnan_inline(exposureTime_us)) exposureTime_us = parameters->exposureTime_achieved_us;
                assert(0.0 < exposureTime_us);
                HRESULT const hr = AdjustTriggerDelays(pWindow, exposureTime_us, parameters->k);
                assert( SUCCEEDED(hr) );

                // Set default delay of the spinlock timer.
                SpinlockTimerSetWaitIntervalInMicroseconds(pTimer, pWindow->delayTime_fraction_us);
              }

              // Test if cameras are ready.
              {
                if (true == have_FlyCapture2SDK)
                  {
#ifdef HAVE_FLYCAPTURE2_SDK
                    assert(0 < parameters->exposureTime_QPC);
                    bool const trigger_ready = WaitForTriggerReady(pFlyCapture2SDK->pCamera, 10 * parameters->exposureTime_QPC);
                    assert(true == trigger_ready);
#endif /* HAVE_FLYCAPTURE2_SDK */
                  }
                else if (true == have_SaperaSDK)
                  {
                    // There is no API call to test if camera is ready!
                  }
                else if (true== have_PylonSDK)
                {
                  //TODO
                }
                else if (true == have_FromFile)
                  {
                    // Nothing to do!
                  }
                else
                  {
                    // Nothing to do!
                  }
                /* if */
              }

              // Reset all camera events.
              {
                BOOL const reset_camera = pSynchronization->EventResetAllCamera(CameraID, ProjectorID);
                assert(0 != reset_camera);
              }

              // Wait for image encoder thread to stop processing.
              {
                bool empty = true;
                bool processing = false;
                DWORD dwIsProcessingResult = WAIT_FAILED;
                DWORD dwIsEmptyResult = WAIT_FAILED;
                do
                  {
                    BOOL const set_process = pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_PROCESS, EncoderID);
                    assert(0 != set_process);

                    do
                      {
                        if (true == processing) SleepEx(1, TRUE);
                        dwIsProcessingResult = pSynchronization->EventWaitFor(IMAGE_ENCODER_QUEUE_PROCESS, EncoderID, (DWORD)0);
                        processing = (WAIT_OBJECT_0 == dwIsProcessingResult);
                      }
                    while (true == processing);
                    assert( false == DebugIsSignalled(pSynchronization, IMAGE_ENCODER_QUEUE_PROCESS, EncoderID) );

                    dwIsEmptyResult = pSynchronization->EventWaitFor(IMAGE_ENCODER_QUEUE_EMPTY, EncoderID, (DWORD)0);
                    empty = (WAIT_OBJECT_0 == dwIsEmptyResult);
                  }
                while (false == empty);
                assert( true == DebugIsSignalled(pSynchronization, IMAGE_ENCODER_QUEUE_EMPTY, EncoderID) );
                assert( false == DebugIsSignalled(pSynchronization, IMAGE_ENCODER_QUEUE_PROCESS, EncoderID) );
              }

              // Set camera ID for memory buffer.
              if (NULL != pImageEncoder->pAllImages)
                {
                  std::wstring * CameraUID = GetUniqueCameraIdentifier(parameters);
                  CameraSDK const acquisition_method = GetAcquisitionMethod(parameters);
                  pImageEncoder->pAllImages->SetCamera(CameraID, CameraUID, acquisition_method);
                  SAFE_DELETE(CameraUID);
                }
              /* if */

              // Signal to the rendering thread we are ready for acquisition.
              {
                assert( false == DebugIsSignalled(pSynchronization, MAIN_READY_CAMERA, CameraID) );

                BOOL const set_ready = pSynchronization->EventSet(MAIN_READY_CAMERA, CameraID);
                assert(0 != set_ready);
              }

              // Check state of camera events.
              {
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_READOUT_BEGIN, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_READOUT_END, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_BEGIN, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_END, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
              }

#pragma endregion // Process MAIN_PREPARE_CAMERA event

            }
          else if (2 == hnr)
            {
              /****** SEND SOFTWARE TRIGGER ******/

              /* The CAMERA_SEND_TRIGGER event is fired when exposure must start.
                 The code for this event will first fetch frame information and will then
                 proceed with the timed triggering depending on the acquisition mode.

                 There also exists a CAMERA_READY event which signals the state of the camera:
                 if it is in the signalled state then camera may be triggered via CAMERA_SEND_TRIGGER
                 event, othewise it indicates camera is currently not ready for triggering.
                 Therfore during normal operation the CAMERA_SEND_TRIGGER event should only be
                 raised if CAMERA_READY is signalled; this may be achieved by waiting on CAMERA_READY event.
              */

#pragma region // Process CAMERA_SEND_TRIGGER event

              // Trigger cannot occur during exposure.
              assert(false == parameters->fExposureInProgress);

              // Disarm CAMERA_READY event and reset CAMERA_SEND_TRIGGER event.
              {
                assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

                BOOL const reset_ready = pSynchronization->EventReset(CAMERA_READY, CameraID);
                assert(0 != reset_ready);

                assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

                BOOL const reset_trigger = pSynchronization->EventReset(CAMERA_SEND_TRIGGER, CameraID);
                assert(0 != reset_trigger);
              }


#pragma region // Fetch image metadata

              // Fetch image metadata.
              {
                /* Each acquisition thread maintains its image queue. Items are added into the queue by
                   the rendering thread for all acquisition modes except when a fixed SL pattern is used;
                   for a fixed SL pattern items are created by the acquisition thread. Items are removed
                   from the queue by the image transfer callback function once transfer from the camera
                   completes.

                   Each item in the queue has its unique key which is simply the number of the frame
                   in the pattern.
                */

                if (false == fFixed)
                  {
                    // Fetch image metadata from queue.
                    bool const peek = pMetadataQueue->PeekImageMetadataInQueue(&sImageMetadata, key);
                    assert(true == peek);
                    if (true == peek)
                      {
                        sImageMetadata.pFilename = NULL;
                      }
                    else
                      {
                        ImageMetadataRelease( &sImageMetadata );
                      }
                    /* if */

#ifdef _DEBUG
                    /* In blocking acquisition mode fetched and stored metadata must match. */
                    if (true == fBlocking)
                      {
                        assert( true == ImageMetadataCompare( &sImageMetadata, &(parameters->sImageMetadataAT) ) );
                      }
                    /* if */
#endif /* _DEBUG */

                  }
                else // !(false == fFixed)
                  {
                    // Copy template metadata to local storage.
                    sImageMetadata = parameters->sImageMetadataAT;
                    assert(key == trigger_counter);
                    assert(key == sImageMetadata.key);
                    assert(NULL == sImageMetadata.pFilename);
                    assert(CameraID == sImageMetadata.CameraID);
                    assert(true == sImageMetadata.fFixed);

                    // Create file suffix if none exists.
                    if ( (NULL == pFileSuffix) && (NULL != parameters->pFilenameAT) )
                      {
                        AcquireSRWLockExclusive( &(parameters->sLockAT) );
                        try
                          {
                            pFileSuffix = new std::wstring();
                            assert(NULL != pFileSuffix);
                            if (NULL != pFileSuffix)
                              {
                                pFileSuffix->reserve( parameters->pFilenameAT->size() );
                                std::wstring::const_iterator ci = parameters->pFilenameAT->cbegin();
                                for (; (ci != parameters->pFilenameAT->cend()) && (L'.' != *ci); ++ci)
                                  {
                                    pFileSuffix->push_back(*ci);
                                  }
                                /* for */
                              }
                            /* if */
                          }
                        catch (...)
                          {
                            assert(NULL == pFileSuffix);
                          }
                        /* try */
                        ReleaseSRWLockExclusive( &(parameters->sLockAT) );
                      }
                    /* if */

                    // Create and assign output filename.
                    {
                      int const size = 2048;
                      wchar_t filename[size + 1];
                      filename[size] = 0;
                      if (NULL != pFileSuffix)
                        {
                          int const count = swprintf_s(filename, size, L"frame_%05ld_%s.png", key, pFileSuffix->c_str());
                          assert(count < size);
                        }
                      else
                        {
                          int const count = swprintf_s(filename, size, L"frame_%05ld.png", key);
                          assert(count < size);
                        }
                      /* if */

                      sImageMetadata.pFilename = new std::wstring(filename);
                    }

                    // Check if image is the last image of the sequence.
                    {
                      assert(trigger_counter < pWindow->num_acquire);
                      if ( (int)(trigger_counter) + 1 == pWindow->num_acquire )
                        {
                          sImageMetadata.fLast = true;
                        }
                      /* if */
                    }

                    // Push created image metadata into the queue.
                    bool const push = PushBackImageMetadataToQueue(pMetadataQueue, &sImageMetadata, false);
                    assert(true == push);

                    sImageMetadata.pFilename = NULL;
                  }
                /* if (false == fFixed) */

                assert(key == sImageMetadata.key);
                assert(NULL == sImageMetadata.pFilename);
              }

#pragma endregion // Fetch image metadata


              // Set flags.
              bool trigger_ready = true; // Assume camera is ready.
              bool trigger_on_time = true; // Assume trigger is on-time.
              bool triggered = false; // Assume triggering failed.

              // Reset previous state.
              wait_for_vblank = false; // Assume we do not have to wait for VBLANK.
              use_absolute_timing = false; // Assume spinlock timer is not used.
              use_software_delay = false; // Assume software delay is not used.
              use_hardware_delay = false; // Assume hardware delay is not used.
              hardware_delay_ms = BATCHACQUISITION_qNaN_dv; // Assume hardwer delay on camera is not used.


#pragma region // Set-up delays and absolute timing

              // Set-up delays and absolute timing.
              if (true == fBlocking)
                {
                  /* In blocking mode the spinlock timer is normally not used except for a special cases
                     when we record projector's transfer function and when we measuring DLP wheel characteristic.
                     For such situations the start of exposure must be delayed for some pre-set time
                     after the VBLANK interrupt. All such situations are easily indicated by the
                     QI_PATTERN_SOLID frame types.
                  */
                  if (QI_PATTERN_SOLID == sImageMetadata.render_type)
                    {

                      // Set timer and VBLANK wait when measuring DLP wheel characteristic.
                      if ( ((int)(SL_PATTERN_DLP_WHEEL_SOFTWARE_DELAY) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY) == sImageMetadata.pattern_type)
                           )
                        {
                          /* There are two ways to precisely delay the trigger with regard to the VBLANK interrupt:
                             first is a pure software delay implemted by using a spinlock timer and
                             second is a pure hardware delay by using a bulit-in trigger delay timer on the camera itself.
                             Which delay to use is indicated by the type of the SL pattern.

                             As the software delay using a spinlock timer cannot fail it will be used as a default solution;
                             therefore if the user has requested hardware delay then we try to configure the camera as requested
                             and then fallback to the spinlock timer only if something fails.

                             For both types of delay the actual delay time may be different then requested.
                             When a spinlock timer is used this is caused by granularity of the loop which queries the timer.
                             When a hardware delay timer is used this is caused by limitations of the timing hardware in the camera itself.
                             True waited time for the spinlock timer will be returned by the timer after the wait operation.
                             Configured hardware delay time will be stored in the hardware_delay variable.
                          */

                          use_hardware_delay = ((int)(SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY) == sImageMetadata.pattern_type);

                          if (true == use_hardware_delay) hardware_delay_ms = sImageMetadata.delay;

                          bool adjust = false;
                          if (true == have_FlyCapture2SDK)
                            {
                              adjust = AcquisitionParametersFlyCapture2SetExposureAndDelayTimes(
                                                                                                pFlyCapture2SDK,
                                                                                                (true == use_hardware_delay)? &hardware_delay_ms : NULL,
                                                                                                &( sImageMetadata.exposure )
                                                                                                );
                              assert(true == adjust);
                            }
                          else if (true == have_SaperaSDK)
                            {
                              adjust = AcquisitionParametersSaperaSetExposureAndDelayTimes(
                                                                                           pSaperaSDK,
                                                                                           (true == use_hardware_delay)? &hardware_delay_ms : NULL,
                                                                                           &( sImageMetadata.exposure )
                                                                                           );
                              assert(true == adjust);
                            }
                          else if (true == have_PylonSDK)
                            {
                              adjust = AcquisitionParametersPylonSetExposureAndDelayTimes(
                                                                                           pPylonSDK,
                                                                                           (true == use_hardware_delay)? &hardware_delay_ms : NULL,
                                                                                           &( sImageMetadata.exposure )
                                                                                           );
                              assert(true == adjust);
                            }
                          /* if */

                          // Set which delay to use.
                          if ( (false == adjust) && (true == use_hardware_delay) ) use_hardware_delay = false;
                          if (false == use_hardware_delay) use_software_delay = true;

                          // Prepare software timer.
                          if (true == use_software_delay) SpinlockTimerSetWaitIntervalInMilliseconds(pTimer, sImageMetadata.delay);

                          // Indicate we have to wait for VBLANK interrupt.
                          wait_for_vblank = true;
                        }
                      /* if */

                      // Set timer and VBLANK wait when measuring projector transfer functions.
                      if ( ((int)(SL_PATTERN_RED_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_GREEN_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_BLUE_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_GRAY_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_CYAN_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_YELLOW_CHANNEL_TRANSFER) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_MAGENTA_CHANNEL_TRANSFER) == sImageMetadata.pattern_type)
                           )
                        {
                          wait_for_vblank = true;
                          assert(false == use_software_delay);
                          assert(false == use_hardware_delay);
                        }
                      /* if */

                      // Set timer and VBLANK wait when mesuring projector delay time.
                      if ( ((int)(SL_PATTERN_DELAY_MEASUREMENT) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_DELAY_MEASUREMENT_WHITE) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_DELAY_MEASUREMENT_BLACK) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_DELAY_MEASUREMENT_WHITE_TO_BLACK) == sImageMetadata.pattern_type) ||
                           ((int)(SL_PATTERN_DELAY_MEASUREMENT_BLACK_TO_WHITE) == sImageMetadata.pattern_type)
                           )
                        {
                          assert(false == wait_for_vblank);
                          assert(false == use_software_delay);
                          assert(false == use_hardware_delay);

                        }
                      /* if */
                    }
                  else // !(QI_PATTERN_SOLID == sImageMetadata.render_type)
                    {
                      assert(false == wait_for_vblank);
                      assert(false == use_software_delay);
                      assert(false == use_hardware_delay);
                    }
                  /* if (QI_PATTERN_SOLID == sImageMetadata.render_type) */

                  // Absolute timing is never used in blocking mode.
                  assert(false == use_absolute_timing);
                }
              else // !(true == fBlocking)
                {
                  /* In non-blocking mode the spinlock timer is always used in absolute timing mode
                     if its data is set. We have already fetched the correct image metadata for the
                     current frame so only have to copy the timing information from the metadata to
                     the local variables which control the spinlock timer.

                     Note that spinlock timer is not set only if a fixed SL pattern is used.
                  */

                  if (-1 != sImageMetadata.QPC_trigger_scheduled_AT)
                    {
                      assert(0 <= sImageMetadata.QPC_current_presented);
                      QPC_spinlock_start.QuadPart = sImageMetadata.QPC_current_presented;

                      assert(0 <= sImageMetadata.QPC_trigger_scheduled_AT);
                      QPC_spinlock_stop.QuadPart = sImageMetadata.QPC_trigger_scheduled_AT;

                      __int64 QPC_delayTime = -1;
                      AcquireSRWLockShared( &(pWindow->sLockRT) );
                      {
                        QPC_delayTime = pWindow->QPC_delayTime;
                      }
                      ReleaseSRWLockShared( &(pWindow->sLockRT) );
                      assert( 0 <= QPC_delayTime );

                      if (-1 != sImageMetadata.QPC_next_presented)
                        {
                          assert(0 <= sImageMetadata.QPC_next_presented);
                          QPC_spinlock_limit.QuadPart = sImageMetadata.QPC_next_presented + QPC_delayTime - parameters->exposureTime_QPC;
                        }
                      else if (-1 != sImageMetadata.QPC_next_scheduled)
                        {
                          assert(0 <= sImageMetadata.QPC_next_scheduled);
                          QPC_spinlock_limit.QuadPart = sImageMetadata.QPC_next_scheduled + QPC_delayTime - parameters->exposureTime_QPC;
                        }
                      else
                        {
                          assert(-1 == sImageMetadata.QPC_next_scheduled);
                          assert(-1 == sImageMetadata.QPC_next_presented);
                          QPC_spinlock_limit.QuadPart = LLONG_MAX;
                        }
                      /* if */

                      assert(QPC_spinlock_start.QuadPart <= QPC_spinlock_stop.QuadPart);
                      //assert(QPC_spinlock_stop.QuadPart <= QPC_spinlock_limit.QuadPart);

                      use_absolute_timing = (NULL != pTimer) && (QPC_spinlock_start.QuadPart <= QPC_spinlock_stop.QuadPart);
                    }
                  else
                    {
                      assert(true == fFixed);
                    }
                  /* if */

                  assert(false == use_hardware_delay);
                }
              /* if (true == fBlocking) */

#pragma endregion // Set-up delays and absolute timing


#pragma region // Wait for VBLANK interrupt

              // Wait for VBLANK interrupt.
              if (true == wait_for_vblank)
                {
                  /* Wait for VBLANK interrupt is normally not required as it is done in the rendering thread;
                     it is never used for non-blocking acquisition. This code should only activate for special
                     cases such as DLP wheel measurements.
                  */
                  assert(true == fBlocking);

                  if (false == pWindow->fModeChange)
                    {
                      EnterCriticalSection( &(pWindow->csWaitForVBLANK) );
                      {
                        assert(false == pWindow->fWaitForVBLANK);
                        pWindow->fWaitForVBLANK = true;
                        {
                          if (NULL != pWindow->pOutput)
                            {
                              HRESULT const hr = pWindow->pOutput->WaitForVBlank();
                              assert( SUCCEEDED(hr) );
                            }
                          /* if (NULL != pWindow->pOutput) */
                        }
                        pWindow->fWaitForVBLANK = false;
                      }
                      LeaveCriticalSection( &(pWindow->csWaitForVBLANK) );
                    }
                  /* if */
                }
              /* if */

#pragma endregion // Wait for VBLANK interrupt


#pragma region // Reset events.
              {
                BOOL const reset_readout_begin = pSynchronization->EventReset(CAMERA_READOUT_BEGIN, CameraID);
                assert(0 != reset_readout_begin);

                BOOL const reset_readout_end = pSynchronization->EventReset(CAMERA_READOUT_END, CameraID);
                assert(0 != reset_readout_end);

                if ( (true == fBlocking) && (false == fConcurrentDelay) )
                  {
                    assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_BEGIN, CameraID) );
                    assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_END, CameraID) );
                  }
                /* if */
              }
#pragma endregion // Reset events.


#pragma region // Spinlock timer

              // Use spinlock timer.
              {
                /* There are two modes in which a spinlock timer may be used:

                   1) First is the relative mode where we wait for a pre-set time interval to elapse.
                   This mode is indicated by use_software_delay flag which must be set to true.
                   In normal operation the value of use_software_delay flag should be set true
                   only if the pattern type is either SL_PATTERN_DLP_WHEEL_SOFTWARE_DELAY or
                   SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY.

                   2) Second is the absolute mode where we wait until pre-set time is achieved.
                   This mode is indicated by use_absolute_timing flag which must be set to true.
                   In normal operation the value of use_absolute_timing should be set set to true
                   only for non-blocking delay and non-fixed SL pattern (fBlocking and fFixed are
                   both false).
                */

                if (true == use_software_delay)
                  {
                    assert(false == use_absolute_timing);
                    assert(true == trigger_on_time);

                    SpinlockTimerWait( pTimer );
                  }
                /* if */

                if (true == use_absolute_timing)
                  {
                    assert(false == use_software_delay);

                    SpinlockTimerWaitFromTo( pTimer, QPC_spinlock_start, QPC_spinlock_stop );
                    trigger_on_time = (pTimer->stop.QuadPart <= QPC_spinlock_limit.QuadPart);

                    if (false == trigger_on_time)
                      {
                        assert(false == sImageMetadata.fLast);
                        Debugfprintf(stderr, dDbgTriggerDropKnownMetadata, CameraID + 1, key + 1, __FILE__, __LINE__);
                      }
                    /* if */
                  }
                /* if */
              }

#pragma endregion // Spinlock timer


#pragma region // Send trigger

              // Trigger the camera.
              if (true == have_FlyCapture2SDK)
                {
#ifdef HAVE_FLYCAPTURE2_SDK
                  /* FlyCapture2 SDK provides a method to test if camera is ready for triggering.
                     As any trigger is sent only after CAMERA_READY event is armed we have to
                     ensure the camera is ready at the moment we arm the CAMERA_READY event.
                     Here we only poll the camera to ensure it is ready.
                  */
                  trigger_ready = CheckTriggerReady(pFlyCapture2SDK->pCamera);
#endif /* HAVE_FLYCAPTURE2_SDK */
                  assert(true == trigger_ready);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  if (true == trigger_on_time)
                    {
#ifdef HAVE_FLYCAPTURE2_SDK
                      assert(NULL != pFlyCapture2SDK->pCamera);
                      FlyCapture2::Error error;
                      error = pFlyCapture2SDK->pCamera->FireSoftwareTrigger();
                      triggered = (error == FlyCapture2::PGRERROR_OK);
                      //assert(true == triggered);
                      if (true == triggered)
                        {
                          bool const trigger_status = CheckTriggerReady(pFlyCapture2SDK->pCamera);
                          if (true == trigger_status)
                            {
                              bool const trigger_not_ready = WaitForTriggerNotReady(pFlyCapture2SDK->pCamera, parameters->exposureTime_QPC);
                              //assert(true == trigger_not_ready);
                            }
                          /* if */
                        }
                      /* if */
#else /* HAVE_FLYCAPTURE2_SDK */
                      assert(false == triggered);
#endif /* HAVE_FLYCAPTURE2_SDK */
                    }
                  /* if */

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              else if (true == have_SaperaSDK)
                {
                  assert(true == trigger_ready);

                  // Camera must support software triggering.
                  assert(-1 != pSaperaSDK->idxTriggerSoftware);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  if (true == trigger_on_time)
                    {
                      if (-1 != pSaperaSDK->idxTriggerSoftware)
                        {
#ifdef HAVE_SAPERA_SDK
                          /* Sapera SDK does not provide a method to test if a GenICam camera is ready for triggering.
                             Instead, we execute GenICam command node for software triggering and check the return value.
                             If the return value is true then trigger may or may not be sent; failure will be indicated
                             via separate event and we will signal CAMERA_REPEAT_TRIGGER from that event callback routine.

                             For GigEVision cameras the software trigger is a GenICam execute note which
                             for Sapera LT API is idenitifed as boolean type node that is only writeable.
                             Writing TRUE to such node sends execute command to camera. Return value of TRUE
                             only indicates the command was successfully sent to the camera; it does not indicate
                             the trigger was accepted or executed by the camera. There is no need to reset the node
                             value to FALSE as it is an execute only node (and not a value node) so it resets automatically.
                          */
                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_INVALID_TRIGGER, CameraID) );

                          BOOL const trigger = pSaperaSDK->pCamera->SetFeatureValue(pSaperaSDK->idxTriggerSoftware, TRUE);
                          triggered = (TRUE == trigger);
                          //assert(true == triggered);
                          if (true == triggered)
                            {
                              DWORD const dwWaitTime = (DWORD)( parameters->exposureTime_requested_us * 0.001 ) + 15000;
                              DWORD const dwIsTriggeredResult = pSynchronization->EventWaitForAny(
                                                                                                  CAMERA_INVALID_TRIGGER, CameraID, // 0
                                                                                                  CAMERA_EXPOSURE_BEGIN,  CameraID, // 1
                                                                                                  dwWaitTime
                                                                                                  );
                              int const hnr_triggered = dwIsTriggeredResult - WAIT_OBJECT_0;
                              if (0 == hnr_triggered) // CAMERA_INVALID_TRIGGER
                                {
                                  triggered = false;
                                  Debugfprintf(stderr, dDbgInvalidTriggerForCamera, CameraID + 1, sImageMetadata.key + 1);

                                  BOOL const reset_invalid_trigger = pSynchronization->EventReset(CAMERA_INVALID_TRIGGER, CameraID);
                                  assert(0 != reset_invalid_trigger);
                                }
                              else if (1 == hnr_triggered) // CAMERA_EXPOSURE_BEGIN
                                {
                                  assert(true == triggered);
                                }
                              else
                                {
                                  triggered = false;
                                  Debugfprintf(stderr, dDbgTriggerConfirmationTimeoutExpiredForCamera, CameraID + 1, sImageMetadata.key + 1);
                                }
                              /* if */
                            }
                          /* if */
#else /* HAVE_SAPERA_SDK */
                          assert(false == triggered);
#endif /* HAVE_SAPERA_SDK */
                        }
                      else
                        {
                          assert(false); // Always break!
                        }
                      /* if */
                    }
                  /* if */

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              else if (true == have_PylonSDK)
                {
                  //todo
                }
              else if (true == have_FromFile)
                {
                  assert(NULL != pFromFile);
                  assert(true == trigger_ready);
                  assert(true == trigger_on_time);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  /* File will be read from disk while processing CAMERA_EXPOSURE_END event.
                     The procedure is the same for all types of acquisition modes.
                  */
                  triggered = true;

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              else
                {
                  assert(true == trigger_ready);
                  assert(true == trigger_on_time);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  triggered = true;

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              /* if */

              // Test if trigger completed on time.
              if (true == use_absolute_timing)
                {
                  trigger_on_time = trigger_on_time && (QPC_after_trigger.QuadPart <= QPC_spinlock_limit.QuadPart);

                  if ( (false == trigger_on_time) && (true == triggered) )
                    {
                      assert(false == sImageMetadata.fLast);
                      Debugfprintf(stderr, dDbgTriggerStallKnownMetadata, CameraID + 1, key + 1, __FILE__, __LINE__);
                    }
                  /* if */
                }
              else
                {
                  assert(true == trigger_on_time);
                }
              /* if */

#pragma endregion // Send trigger


#pragma region // Update trigger information and image metadata

              // Update trigger information and image metadata.
              if (true == triggered)
                {
                  // Increase trigger counter.
                  ++trigger_counter;

                  // Update timing information.
                  AcquireSRWLockExclusive( &(parameters->sLockAT) );
                  {
                    parameters->trigger_counter = trigger_counter;

                    parameters->QPC_before_trigger_AT = QPC_before_trigger;
                    parameters->QPC_after_trigger_AT = QPC_after_trigger;
                    parameters->QPC_exposure_start = QPC_after_trigger;
                    parameters->QPC_exposure_end_scheduled.QuadPart = QPC_after_trigger.QuadPart + parameters->exposureTime_QPC;
                  }
                  ReleaseSRWLockExclusive( &(parameters->sLockAT) );

                  // Update trigger statistics.
                  FrameStatisticsAddMeasurement(pStatisticsTriggerDuration, QPC_before_trigger, QPC_after_trigger);
                  FrameStatisticsAddFrame(pStatisticsTriggerFrequency);

                  // Update image metadata.
                  {
                    double delay_ms = sImageMetadata.delay;
                    double exposure_ms = sImageMetadata.exposure;

                    if (true == use_software_delay)
                      {
                        assert( false == use_absolute_timing );
                        assert( false == use_hardware_delay );
                        delay_ms = SpinlockTimerLastWaitDuration( pTimer );
                      }
                    /* if */

                    if (true == use_hardware_delay)
                      {
                        assert( false == use_absolute_timing );
                        assert( false == use_software_delay );
                        assert( false == isnan_inline(hardware_delay_ms) );
                        delay_ms = hardware_delay_ms;
                      }
                    /* if */

                    if (0.0 >= exposure_ms)
                      {
                        exposure_ms = parameters->exposureTime_achieved_us * 0.001; // Convert us to ms.
                        if (true == isnan_inline(exposure_ms)) exposure_ms = parameters->exposureTime_requested_us * 0.001;
                        assert(0.0 < exposure_ms);
                      }
                    /* if */

#ifdef _DEBUG
                    /* For a fixed SL pattern frame key always lags one step behind the trigger_counter value. */
                    if (true == fFixed) assert( key + 1 == trigger_counter );
#endif /* _DEBUG */

                    // Update local copy.
                    sImageMetadata.delay = delay_ms;
                    sImageMetadata.exposure = exposure_ms;
                    sImageMetadata.QPC_before_trigger = QPC_before_trigger.QuadPart;
                    sImageMetadata.QPC_after_trigger = QPC_after_trigger.QuadPart;
                    sImageMetadata.fTrigger = triggered;
                    if (false == trigger_on_time) sImageMetadata.fBatch = false;

                    // Update image metadata in queue.
                    bool const update =
                      pMetadataQueue->AdjustImageMetadataAcquisition(
                                                                     key,
                                                                     delay_ms,
                                                                     exposure_ms,
                                                                     QPC_before_trigger.QuadPart,
                                                                     QPC_after_trigger.QuadPart,
                                                                     triggered,
                                                                     trigger_on_time
                                                                     );
                    assert(true == update);
                  }
                }
              else // !(true == triggered)
                {
                  Debugfprintf(stderr, gDbgTriggerFailedForFrame, CameraID + 1, sImageMetadata.key + 1);

                  /* If trigger failed and we are in a non-blocking mode then image metadata
                     must be deleted from the metadata queue as the frame is dropped.
                  */
                  if ( (false == fBlocking) && (false == fFixed) )
                    {
                      ImageMetadata sImageMetadataPopped;
                      ImageMetadataBlank( &sImageMetadataPopped );

                      bool const pop = pMetadataQueue->PopImageMetadataFromQueue(&sImageMetadataPopped, key);
                      assert(true == pop);
                      if (true == pop)
                        {
                          assert( true == ImageMetadataCompare( &sImageMetadata, &sImageMetadataPopped ) );
                          assert( false == sImageMetadataPopped.fTrigger );
                          ImageMetadataRelease( &sImageMetadataPopped );
                        }
                      /* if */
                    }
                  /* if */
                }
              /* if (true == triggered) */

#pragma endregion // Update trigger information and image metadata


#pragma region // Event dispatch

              // Dispatch events after processing is done.
              {
                /* Event dispatch logic for CAMERA_SEND_TRIGGER and CAMERA_REPEAT_TRIGGER is the same and
                   is realized as an inline function to avoid code duplication.
                */
                DispatchEventsAfterTrigger_inline(
                                                  parameters,
                                                  pSynchronization,
                                                  fBlocking, fFixed, fConcurrentDelay,
                                                  triggered
                                                  );
              }

#pragma endregion // Event dispatch


              // Mark exposure as started and begin exposure timeout.
              if (true == triggered)
                {
                  if (false == have_SaperaSDK) parameters->fExposureInProgress = true;
                  StartExposureTimeout_inline(hTimerExposureTimeout, parameters);
                }
              /* if */

#pragma endregion // Process CAMERA_SEND_TRIGGER event

            }
          else if (3 == hnr)
            {
              /****** REPEAT SOFTWARE TRIGGER ******/

              /* The CAMERA_REPEAT_TRIGGER event is fired only if camera needs re-triggering. It should be fired
                 exclusively from the acquisition thread or camera API after the CAMERA_SEND_TRIGGER event.
                 The code for this event tries to trigger the camera until it is successfull.
              */

#pragma region // Process CAMERA_REPEAT_TRIGGER event

              // Trigger cannot occur during exposure.
              assert(false == parameters->fExposureInProgress);

              // Disarm CAMERA_REPEAT_TRIGGER event
              {
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID) );

                BOOL const reset_repeat = pSynchronization->EventReset(CAMERA_REPEAT_TRIGGER, CameraID);
                assert(0 != reset_repeat);
              }

              Debugfprintf(stderr, gDbgRepeatTriggerForFrame, CameraID + 1, sImageMetadata.key + 1);

#pragma region // Fetch image metadata

              // Fetch image metadata.
              {
                /* Image metadata should aready be in the sImageMetadata structure of the thread
                   as this event occures after the CAMERA_SEND_TRIGGER event is executed.
                */
                ImageMetadata sImageMetadataPeeked;
                ImageMetadataBlank( &sImageMetadataPeeked );

                bool const peek = pMetadataQueue->PeekImageMetadataInQueue(&sImageMetadataPeeked, key);
                assert(true == peek);
                assert(true == ImageMetadataCompare( &sImageMetadata, &sImageMetadataPeeked ));
              }

#pragma endregion // Fetch image metadata


              // Set flags.
              bool trigger_ready = true; // Assume camera is ready.
              bool trigger_on_time = true; // Assume trigger is on-time.
              bool triggered = false; // Assume triggering failed.


#pragma region // Wait for VBLANK interrupt

              // Wait for VBLANK interrupt; wait status is inherited from the previous CAMERA_SEND_TRIGGER event.
              if (true == wait_for_vblank)
                {
                  if (false == pWindow->fModeChange)
                    {
                      EnterCriticalSection( &(pWindow->csWaitForVBLANK) );
                      {
                        assert(false == pWindow->fWaitForVBLANK);
                        pWindow->fWaitForVBLANK = true;
                        {
                          if (NULL != pWindow->pOutput)
                            {
                              HRESULT const hr = pWindow->pOutput->WaitForVBlank();
                              assert( SUCCEEDED(hr) );
                            }
                          /* if (NULL != pWindow->pOutput) */
                        }
                        pWindow->fWaitForVBLANK = false;
                      }
                      LeaveCriticalSection( &(pWindow->csWaitForVBLANK) );
                    }
                  /* if */
                }
              /* if */

#pragma endregion // Wait for VBLANK interrupt


#pragma region // Spinlock timer

              // Use spinlock timer; timer status is inherited from the previous CAMERA_SEND_TRIGGER event.
              {
                if (true == use_software_delay)
                  {
                    assert(false == use_absolute_timing);
                    assert(true == trigger_on_time);

                    SpinlockTimerWait( pTimer );
                  }
                /* if */

                if (true == use_absolute_timing)
                  {
                    assert(false == use_software_delay);

                    SpinlockTimerWaitFromTo( pTimer, QPC_spinlock_start, QPC_spinlock_stop );
                    trigger_on_time = (pTimer->stop.QuadPart <= QPC_spinlock_limit.QuadPart);

                    if (false == trigger_on_time)
                      {
                        Debugfprintf(stderr, dDbgTriggerDropKnownMetadata, CameraID + 1, key + 1, __FILE__, __LINE__);
                      }
                    /* if */
                  }
                /* if */
              }

#pragma endregion // Spinlock timer


#pragma region // Repeat trigger

              // Trigger the camera.
              if (true == have_FlyCapture2SDK)
                {
#ifdef HAVE_FLYCAPTURE2_SDK
                  /* FlyCapture2 SDK provides a method to test if camera is ready for triggering.
                     As any trigger is sent only after CAMERA_READY event is armed we have to
                     ensure the camera is ready at the moment we arm the CAMERA_READY event.
                     Here we only poll the camera to ensure it is ready.
                  */
                  trigger_ready = CheckTriggerReady(pFlyCapture2SDK->pCamera);
#endif /* HAVE_FLYCAPTURE2_SDK */
                  assert(true == trigger_ready);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  if (true == trigger_on_time)
                    {
#ifdef HAVE_FLYCAPTURE2_SDK
                      assert(NULL != pFlyCapture2SDK->pCamera);
                      FlyCapture2::Error error;
                      error = pFlyCapture2SDK->pCamera->FireSoftwareTrigger();
                      triggered = (error == FlyCapture2::PGRERROR_OK);
                      //assert(true == triggered);
                      if (true == triggered)
                        {
                          bool const trigger_status = CheckTriggerReady(pFlyCapture2SDK->pCamera);
                          if (true == trigger_status)
                            {
                              bool const trigger_not_ready = WaitForTriggerNotReady(pFlyCapture2SDK->pCamera, parameters->exposureTime_QPC);
                              //assert(true == trigger_not_ready);
                            }
                          /* if */
                        }
                      /* if */
#else /* HAVE_FLYCAPTURE2_SDK */
                      assert(false == triggered);
#endif /* HAVE_FLYCAPTURE2_SDK */
                    }
                  /* if */

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              else if (true == have_SaperaSDK)
                {
                  assert(true == trigger_ready);

                  // Camera must support software triggering.
                  assert(-1 != pSaperaSDK->idxTriggerSoftware);

                  BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_trigger );
                  assert(TRUE == qpc_before);

                  if (true == trigger_on_time)
                    {
                      if (-1 != pSaperaSDK->idxTriggerSoftware)
                        {
#ifdef HAVE_SAPERA_SDK
                          /* Sapera SDK does not provide a method to test if a GenICam camera is ready for triggering.
                             Instead, we execute GenICam command node for software triggering and check the return value.
                             If the return value is true then trigger may or may not be sent; failure will be indicated
                             via separate event and we will signal CAMERA_REPEAT_TRIGGER from that event callback routine.

                             For GigEVision cameras the software trigger is a GenICam execute note which
                             for Sapera LT API is idenitifed as boolean type node that is only writeable.
                             Writing TRUE to such node sends execute command to camera. Return value of TRUE
                             only indicates the command was successfully sent to the camera; it does not indicate
                             the trigger was accepted or executed by the camera. There is no need to reset the node
                             value to FALSE as it is an execute only node (and not a value node) so it resets automatically.
                          */
                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_INVALID_TRIGGER, CameraID) );

                          BOOL const trigger = pSaperaSDK->pCamera->SetFeatureValue(pSaperaSDK->idxTriggerSoftware, TRUE);
                          triggered = (TRUE == trigger);
                          //assert(true == triggered);
                          if (true == triggered)
                            {
                              DWORD const dwWaitTime = (DWORD)( parameters->exposureTime_requested_us * 0.001 ) + 15000;
                              DWORD const dwIsTriggeredResult = pSynchronization->EventWaitForAny(
                                                                                                  CAMERA_INVALID_TRIGGER, CameraID, // 0
                                                                                                  CAMERA_EXPOSURE_BEGIN,  CameraID, // 1
                                                                                                  dwWaitTime
                                                                                                  );
                              int const hnr_triggered = dwIsTriggeredResult - WAIT_OBJECT_0;
                              if (0 == hnr_triggered) // CAMERA_INVALID_TRIGGER
                                {
                                  triggered = false;
                                  Debugfprintf(stderr, dDbgInvalidTriggerForCamera, CameraID + 1, sImageMetadata.key + 1);

                                  BOOL const reset_invalid_trigger = pSynchronization->EventReset(CAMERA_INVALID_TRIGGER, CameraID);
                                  assert(0 != reset_invalid_trigger);
                                }
                              else if (1 == hnr_triggered) // CAMERA_EXPOSURE_BEGIN
                                {
                                  assert(true == triggered);
                                }
                              else
                                {
                                  triggered = false;
                                  Debugfprintf(stderr, dDbgTriggerConfirmationTimeoutExpiredForCamera, CameraID + 1, sImageMetadata.key + 1);
                                }
                              /* if */
                            }
                          /* if */
#else /* HAVE_SAPERA_SDK */
                          assert(false == triggered);
#endif /* HAVE_SAPERA_SDK */
                        }
                      else
                        {
                          assert(false); // Always break!
                        }
                      /* if */
                    }
                  /* if */

                  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_trigger );
                  assert(TRUE == qpc_after);
                }
              else if (true == have_PylonSDK)
              {
                //TODO
              }
              else if (true == have_FromFile)
                {
                  assert(false); // Always break! This code should not be reachable unless there exists an error in signal dispatching.
                }
              else
                {
                  assert(false); // Always break! This code should not be reachable unless there exists an error in signal dispatching.
                }
              /* if */

              // Test if trigger completed on time.
              if (true == use_absolute_timing)
                {
                  trigger_on_time = trigger_on_time && (QPC_after_trigger.QuadPart <= QPC_spinlock_limit.QuadPart);

                  if ( (false == trigger_on_time) && (true == triggered) )
                    {
                      assert(false == sImageMetadata.fLast);
                      Debugfprintf(stderr, dDbgTriggerStallKnownMetadata, CameraID + 1, key + 1, __FILE__, __LINE__);
                    }
                  /* if */
                }
              else
                {
                  assert(true == trigger_on_time);
                }
              /* if */

#pragma endregion // Repeat trigger


#pragma region // Update trigger information and image metadata

              // Update trigger information and image metadata.
              if (true == triggered)
                {
                  // Increase trigger counter.
                  ++trigger_counter;

                  // Update timing information.
                  AcquireSRWLockExclusive( &(parameters->sLockAT) );
                  {
                    parameters->trigger_counter = trigger_counter;

                    parameters->QPC_before_trigger_AT = QPC_before_trigger;
                    parameters->QPC_after_trigger_AT = QPC_after_trigger;
                    parameters->QPC_exposure_start = QPC_after_trigger;
                    parameters->QPC_exposure_end_scheduled.QuadPart = QPC_after_trigger.QuadPart + parameters->exposureTime_QPC;
                  }
                  ReleaseSRWLockExclusive( &(parameters->sLockAT) );

                  // Update trigger statistics.
                  FrameStatisticsAddMeasurement(pStatisticsTriggerDuration, QPC_before_trigger, QPC_after_trigger);
                  FrameStatisticsAddFrame(pStatisticsTriggerFrequency);

                  // Update image metadata.
                  {
                    double delay_ms = sImageMetadata.delay;
                    double exposure_ms = sImageMetadata.exposure;

                    if (true == use_software_delay)
                      {
                        assert( false == use_absolute_timing );
                        assert( false == use_hardware_delay );
                        delay_ms = SpinlockTimerLastWaitDuration( pTimer );
                      }
                    /* if */

                    if (true == use_hardware_delay)
                      {
                        assert( false == use_absolute_timing );
                        assert( false == use_software_delay );
                        assert( false == isnan_inline(hardware_delay_ms) );
                        delay_ms = hardware_delay_ms;
                      }
                    /* if */

                    if (0.0 >= exposure_ms)
                      {
                        exposure_ms = parameters->exposureTime_achieved_us * 0.001; // Convert us to ms.
                        if (true == isnan_inline(exposure_ms)) exposure_ms = parameters->exposureTime_requested_us * 0.001;
                        assert(0.0 < exposure_ms);
                      }
                    /* if */

#ifdef _DEBUG
                    /* For a fixed SL pattern frame key always lags one step behind the trigger_counter value. */
                    if (true == fFixed) assert( key + 1 == trigger_counter );
#endif /* _DEBUG */

                    // Update local copy.
                    sImageMetadata.delay = delay_ms;
                    sImageMetadata.exposure = exposure_ms;
                    sImageMetadata.QPC_before_trigger = QPC_before_trigger.QuadPart;
                    sImageMetadata.QPC_after_trigger = QPC_after_trigger.QuadPart;
                    sImageMetadata.fTrigger = triggered;
                    if (false == trigger_on_time) sImageMetadata.fBatch = false;

                    // Update image metadata in queue.
                    bool const update =
                      pMetadataQueue->AdjustImageMetadataAcquisition(
                                                                     key,
                                                                     delay_ms,
                                                                     exposure_ms,
                                                                     QPC_before_trigger.QuadPart,
                                                                     QPC_after_trigger.QuadPart,
                                                                     triggered,
                                                                     trigger_on_time
                                                                     );
                    assert(true == update);
                  }
                }
              else // !(true == triggered)
                {
                  Debugfprintf(stderr, gDbgRepeatTriggerFailedForFrame, CameraID + 1, sImageMetadata.key + 1);

                  /* If trigger failed and we are in a non-blocking mode then image metadata
                     must be deleted from the metadata queue as the frame is dropped.
                  */
                  if ( (false == fBlocking) && (false == fFixed) )
                    {
                      ImageMetadata sImageMetadataPopped;
                      ImageMetadataBlank( &sImageMetadataPopped );

                      bool const pop = pMetadataQueue->PopImageMetadataFromQueue(&sImageMetadataPopped, key);
                      assert(true == pop);
                      if (true == pop)
                        {
                          assert( true == ImageMetadataCompare( &sImageMetadata, &sImageMetadataPopped ) );
                          assert( false == sImageMetadataPopped.fTrigger );
                          ImageMetadataRelease( &sImageMetadataPopped );
                        }
                      /* if */
                    }
                  /* if */
                }
              /* if (true == triggered) */

#pragma endregion // Update trigger information and image metadata


#pragma region // Event dispatch

              // Dispatch events after processing is done.
              {
                /* Event dispatch logic for CAMERA_SEND_TRIGGER and CAMERA_REPEAT_TRIGGER is the same and
                   is realized as an inline function to avoid code duplication.
                */
                DispatchEventsAfterTrigger_inline(
                                                  parameters,
                                                  pSynchronization,
                                                  fBlocking, fFixed, fConcurrentDelay,
                                                  triggered
                                                  );
              }

#pragma endregion // Event dispatch


              // Mark exposure as started and begin exposure timeout.
              if (true == triggered)
                {
                  if (false == have_SaperaSDK) parameters->fExposureInProgress = true;
                  StartExposureTimeout_inline(hTimerExposureTimeout, parameters);
                }
              /* if */


#pragma endregion // Process CAMERA_REPEAT_TRIGGER event

            }
          else if (4 == hnr)
            {
              /****** EXPOSURE COMPLETE ******/

              /* The CAMERA_EXPOSURE_EVENT is normally signalled by a callback function from a particular
                 camera SDK. Unfortunately, as all camera SDKs do not support such functionality CAMERA_EXPOSURE_EVENT
                 may also be signalled by the acquisition thread to itself.

                 The code for this event depends on the SDK used:

                 1) For FlyCapture2 SDK the event is signalled by the acquisition thread to itself as
                 the SDK does not provide functionality to observe camera state during acquisition.
                 Here we wait for the exposure time to ellapse and then we poll the camera to ensure
                 it is ready. Once we have the confirmation the camera is ready we raise the CAMERA_REAY event.
                 Other events are dispathed depending on the acquisition mode.

                 2) For Sapera SDK the event is signalled by a callback function.
                 We only have to dispatch events depending on the acquisition mode.

                 3) For acquisition from file the event is signalled by the acquisition thread to itself.
                 We have to read the image data from the file and then signal the camera is ready by
                 raising the CAMERA_READY event. Other events are dispathed depending on the acquisition mode.
              */

#pragma region // Process CAMERA_EXPOSURE_END event

              // Exposure must be in progress.
              if (false == have_SaperaSDK) assert( true == parameters->fExposureInProgress );

              // Reset CAMERA_EXPOSURE_END event.
              {
                assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                assert( true == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID) );

                BOOL const reset_exposure_begin = pSynchronization->EventReset(CAMERA_EXPOSURE_BEGIN, CameraID);
                assert(0 != reset_exposure_begin);

                BOOL const reset_exposure_end = pSynchronization->EventReset(CAMERA_EXPOSURE_END, CameraID);
                assert(0 != reset_exposure_end);
              }


#pragma region // Execute camera SDK specific code

              // Execute camera SDK specific code
              if (true == have_FlyCapture2SDK)
                {
                  // Sleep till exposure time elapses.
                  SleepUntilExposureEnds_inline(parameters, pWindow, QPC_after_trigger);

                  // Wait for the camera to become ready.
#ifdef HAVE_FLYCAPTURE2_SDK
                  assert(0 < parameters->exposureTime_QPC);
                  if (false == fBlocking)
                    {
                      bool const trigger_ready = WaitForTriggerReady(pFlyCapture2SDK->pCamera, parameters->exposureTime_QPC);
                      //assert(true == trigger_ready);
                    }
                  else
                    {
                      bool const trigger_ready = WaitForTriggerReady(pFlyCapture2SDK->pCamera, 10 * parameters->exposureTime_QPC);
                      //assert(true == trigger_ready);
                    }
                  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

                }
              else if (true == have_SaperaSDK)
                {
                  // Slow down acquisition.
                  if (true == parameters->fThrottleDown)
                    {
#ifdef HAVE_SAPERA_SDK
                      if (NULL != pSaperaSDK->pTransfer)
                        {
                          BOOL const wait = pSaperaSDK->pTransfer->Wait(parameters->timeout);
                          //assert(TRUE == wait);
                        }
                      /* if */
#endif /* HAVE_SAPERA_SDK */
                    }
                  /* if */
                }
              else if (true == have_PylonSDK) 
              {
                //TODO
              }
              else if (true == have_FromFile)
                {
                  // Fetch next image.
                  DispatchNextImageFromFile( parameters );
                }
              else
                {
                  // Nothing to do!
                }
              /* if */

#pragma endregion // Execute camera SDK specific code


              // Mark exposure complete and cancel timeout.
              {
                if (false == have_SaperaSDK) parameters->fExposureInProgress = false;
                StopExposureTimeout_inline(hTimerExposureTimeout);
              }

              // Assume metadata may be cleared here.
              bool clear_metadata = true;


#pragma region // Event dispatch after CAMERA_EXPOSURE_END

              // Exposure has completed.
              assert( false == parameters->fExposureInProgress );

              // Arm CAMERA_READY event except for blocking mode without concurrent delay.
              if ( (false == fBlocking) || (true == fFixed) || (true == fConcurrentDelay) )
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

                  BOOL const set_ready = pSynchronization->EventSet(CAMERA_READY, CameraID);
                  assert(0 != set_ready);
                }
              /* if */

              // Dispatch events after processing is done.
              if (true == fBlocking)
                {
                  if (false == fFixed)
                    {
                      if (true == fConcurrentDelay)
                        {
                          /* Event cycle is
                             ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->DRAW_PRESENT->...
                             where after successfull camera trigger CAMERA_EXPOSURE_END was executed
                             as a branch of the cycle simultaneously with DRAW_PRESENT.
                          */

                          // Nothing to do!
                        }
                      else // !(true == fConcurrentDelay)
                        {
                          /* Event cycle is
                             ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->DRAW_PRESENT->...
                             so the next event is CAMERA_TRANSFER_END.
                             Depending on the camera SDK we raise the CAMERA_TRANSFER_END event here or in the callback function.
                          */

                          if ( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) || (true == have_PylonSDK))
                            {
                              // Event CAMERA_TRANSFER_END is raised by the transfer callback function!
                            }
                          else // !( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) || (true == have_PylonSDK))
                            {
                              assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_END, CameraID) );

                              BOOL const set_transfer_end = pSynchronization->EventSet(CAMERA_TRANSFER_END, CameraID);
                              assert(0 != set_transfer_end);
                            }
                          /* if ( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) ) */

                          clear_metadata = false;
                        }
                      /* if (true == fConcurrentDelay) */
                    }
                  else // !(false == fFixed)
                    {
                      /* Event cycle is
                         ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->CAMERA_SYNC_TRIGGERS->...
                         so next event is CAMERA_TRANSFER_END.
                         Depending on the camera SDK we raise the CAMERA_TRANSFER_END event here or in the callback function.
                      */

                      if ( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) || (true == have_PylonSDK))
                        {
                          // Event CAMERA_TRANSFER_END is raised by the transfer callback function!
                        }
                      else // !( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) )
                        {
                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_TRANSFER_END, CameraID) );

                          BOOL const set_transfer_end = pSynchronization->EventSet(CAMERA_TRANSFER_END, CameraID);
                          assert(0 != set_transfer_end);
                        }
                      /* if ( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) ) */

                      clear_metadata = false;
                    }
                  /* if (false == fFixed) */
                }
              else // (true == fBlocking)
                {
                  if (true == fFixed)
                    {
                      /* Event cycle is
                         ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_SYNC_TRIGGERS->...
                         so the next event to be dispatched is CAMERA_SYNC_TRIGGERS.
                         Note the event is dispatched using conditional dispatch, i.e. the event will be
                         signalled only after all acquisition threads attached to the rendering thread set the signal.
                      */

                      assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SYNC_TRIGGERS, ProjectorID) );

                      BOOL const set_sync_trigger = pSynchronization->EventSetConditional(CAMERA_SYNC_TRIGGERS, ProjectorID);
                      assert(0 != set_sync_trigger);
                    }
                  else // !(true == fFixed)
                    {
                      // Nothing to do!
                    }
                  /* if (true == fFixed) */
                }
              /* if (true == fBlocking) */

#pragma endregion // Event dispatch after CAMERA_EXPOSURE_END


              // Adjust camera exposure time.
              {
                /* Exposure adjustment operation is lazy as indicated by the false flag:
                   exposure time will be updated only if user changed exposure factor multiplier or if
                   display refresh rate changed.
                */
                AdjustCameraExposureTime_inline(parameters, false);
              }

              // Clear image metadata.
              if (true == clear_metadata)
                {
                  ImageMetadataRelease( &sImageMetadata );
                }
              /* if */

#pragma endregion // Process CAMERA_EXPOSURE_END event

            }
          else if (5 == hnr)
            {
              /****** TRANSFER COMPLETE ******/

              /* The CAMERA_TRANSFER_END event is normally signalled by frame processing callback function
                 which may not out-of-sync with the camera triggering as occures at some later time.
                 In blocking acquisition mode it is always synchronous and is a part of the event cycle
                 and is used to dispatch events, however, in the non-blocking mode the event is almost
                 always out-of-sync and should not be used at all.
              */

#pragma region // Process CAMERA_TRANSFER_END event

              // Reset CAMERA_TRANSFER_END event.
              {
                BOOL const reset_transfer_end = pSynchronization->EventReset(CAMERA_TRANSFER_END, CameraID);
                assert(0 != reset_transfer_end);
              }

              // Assume metadata was cleared while processing CAMERA_EXPOSURE_END event.
              bool clear_metadata = false;


#pragma region // Event dispatch for CAMERA_TRANSFER_END

              // Arm CAMERA_READY event for blocking mode without concurrent delay.
              if ( (true == fBlocking) && (false == fFixed) && (false == fConcurrentDelay) )
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

                  BOOL const set_ready = pSynchronization->EventSet(CAMERA_READY, CameraID);
                  assert(0 != set_ready);
                }
              /* if */

              // Dispatch event after processing is done.
              if (true == fBlocking)
                {
                  if (false == fFixed)
                    {
                      if (false == fConcurrentDelay)
                        {
                          /* Event cycle is
                             ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->...
                             so the next event is DRAW_PRESENT.
                             Before signalling DRAW_PRESENT we have to wait for DRAW_PRESENT_READY.
                          */

                          DWORD const dwIsReadyResult = pSynchronization->EventWaitForAny(
                                                                                          DRAW_PRESENT_READY,  ProjectorID, // 0
                                                                                          CAMERA_TERMINATE,    CameraID,    // 1
                                                                                          MAIN_PREPARE_CAMERA, CameraID,    // 2
                                                                                          INFINITE // Wait forever.
                                                                                          );
                          int const hnr_ready = dwIsReadyResult - WAIT_OBJECT_0;
                          if (0 == hnr_ready) // DRAW_PRESENT_READY
                            {
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                              BOOL const set_present = pSynchronization->EventSetConditional(DRAW_PRESENT, ProjectorID);
                              assert(0 != set_present);
                            }
                          else if (1 == hnr_ready) // CAMERA_TERMINATE
                            {
                              Debugfprintf(stderr, dDbgDropPresentForProjectorDueToCameraTerminate, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (2 == hnr_ready) // MAIN_PREPARE_CAMERA
                            {
                              Debugfprintf(stderr, dDbgDropPresentForProjectorDueToMainPrepareCamera, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else
                            {
                              Debugfprintf(stderr, dDbgDropPresentForProjector, CameraID + 1, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          /* if */

                          clear_metadata = true;
                        }
                      else
                        {
                          // Nothing to do!
                        }
                      /* if */
                    }
                  else // !(false == fFixed)
                    {
                      /* Event cycle is
                         ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->CAMERA_SYNC_TRIGGERS->...
                         so the next event to be dispatched is CAMERA_SYNC_TRIGGERS.
                         Note the event is dispatched using conditional dispatch, i.e. the event will be
                         signalled only after all acquisition threads attached to the rendering thread set the signal.
                      */

                      assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );
                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_SYNC_TRIGGERS, ProjectorID) );

                      BOOL const set_sync_trigger = pSynchronization->EventSetConditional(CAMERA_SYNC_TRIGGERS, ProjectorID);
                      assert(0 != set_sync_trigger);

                      clear_metadata = true;
                    }
                  /* if (false == fFixed) */
                }
              else // !(true == fBlocking)
                {
                  // Nothing to do!
                }
              /* if (true == fBlocking) */

#pragma endregion // Event dispatch for CAMERA_TRANSFER_END


              // Clear image metadata.
              if (true == clear_metadata)
                {
                  ImageMetadataRelease( &sImageMetadata );
                }
              /* if */

#pragma endregion // Process CAMERA_TRANSFER_END event

            }
          else if (6 == hnr)
            {
              /****** CHANGE ID ******/

              /* Event identifiers may be changed during program execution, e.g. when camera is deleted.
                 This event is used to facilitate event ID change for the acquisition and image encoder threads.
              */

#pragma region // Change event ID

              // Store old event ID.
              int const CameraIDOld = CameraID;

              // Output message.
              if (CameraIDOld != parameters->CameraID)
                {
                  Debugfwprintf(stderr, gDbgCameraIDChanged, CameraIDOld + 1, CameraIDOld + 1, parameters->CameraID + 1);

                  SetThreadNameAndIDForMSVC(-1, "AcquisitionThread", parameters->CameraID);
                }
              else
                {
                  Debugfwprintf(stderr, gDbgCameraIDNotChanged, CameraIDOld + 1);
                }
              /* if */

              // Fetch new event ID values.
              {
                CameraID = parameters->CameraID;
                assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                ProjectorID = parameters->ProjectorID;
                assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

                EncoderID = pImageEncoder->EncoderID;
                assert( (0 <= EncoderID) && (EncoderID <= (int)(pSynchronization->ImageEncoder.size())) );
                assert( CameraID == pImageEncoder->CameraID );
              }

              // Set camera ID for memory buffer.
              if (NULL != pImageEncoder->pAllImages)
                {
                  std::wstring * CameraUID = GetUniqueCameraIdentifier(parameters);
                  CameraSDK const acquisition_method = GetAcquisitionMethod(parameters);
                  pImageEncoder->pAllImages->SetCamera(CameraID, CameraUID, acquisition_method);
                  SAFE_DELETE(CameraUID);
                }
              /* if */

              // Disarm event; note that we have to use the old event ID.
              {
                BOOL const reset_change_id = pSynchronization->EventReset(CAMERA_CHANGE_ID, CameraIDOld);
                assert(0 != reset_change_id);
              }

#pragma endregion // Change event ID

            }
          else if (7 == hnr)
            {
              /****** REPEAT ACQUISITION ******/

              /* For network cameras a result of trigger operation may be successfull and the trigger
                 may stil fail. For all acquisition modes such situation means the acquisition thread
                 will become deadlocked as it indefinitely waits for the image data transfer to complete.

                 To avoid deadlock we use a timer object whose timeout is set to some pre-specified time
                 after a successfull trigger. If the image data transfer completed during this time then
                 nothing is done and timer is reset, otherwise we retrigger the camera.
              */

#pragma region // Process acquisition timeout event

              Debugfprintf(stderr, gDbgTriggerTimeoutForFrame, CameraID + 1, sImageMetadata.key + 1);

              // Exposure timer is only used for real camera SDKs.
              assert( (true == have_FlyCapture2SDK) || (true == have_SaperaSDK) || (true == have_PylonSDK));

              // Timeout means exposure never completed.
              assert( true == parameters->fExposureInProgress );

              // Timeout means camera is not ready.
              assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

              // Disarm timer.
              StopExposureTimeout_inline(hTimerExposureTimeout);

              // Dispatch appropriate event to break the deadlock.
              if (true == parameters->fExposureInProgress)
                {
                  if (true == fBlocking)
                    {
                      /* For blocking acquisition send CAMERA_REPEAT_TRIGGER event.
                         Camera will then be triggered and timeout timer will be reset.
                      */

                      assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

                      BOOL const set_repeat_trigger = pSynchronization->EventSet(CAMERA_REPEAT_TRIGGER, CameraID);
                      assert(0 != set_repeat_trigger);
                    }
                  else // !(true == fBlocking)
                    {
                      /* For non-blocking acquisition mode either mark the camera ready or retrigger the
                         camera depending on the SL pattern type.
                      */
                      if (false == fFixed)
                        {
                          parameters->fExposureInProgress = false;

                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

                          BOOL const set_ready = pSynchronization->EventSet(CAMERA_READY, CameraID);
                          assert(0 != set_ready);
                        }
                      else // !(false == fFixed)
                        {
                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_REPEAT_TRIGGER, CameraID) );

                          BOOL const set_repeat_trigger = pSynchronization->EventSet(CAMERA_REPEAT_TRIGGER, CameraID);
                          assert(0 != set_repeat_trigger);
                        }
                      /* if (false == fFixed) */
                    }
                  /* if */
                }
              /* if */

#pragma endregion // Process acquisition timeout event

            }
          else
            {
              // We received an unknown event!
            }
          /* if */

          // Update processing time.
          EventProcessed(pEvents);

#ifdef _DEBUG
          // Print event processing time in percentage of screen refresh interval.
          {
            int event_code = -1;
            double event_duration_ms = -1.0;

            bool const get_event = GetCurrentEvent(pEvents, &event_code, &event_duration_ms, NULL, NULL);
            assert(true == get_event);

            //if (true == get_event) CheckEventDuration_inline(event_code, event_duration_ms, parameters);
          }
#endif /* _DEBUG */

        }
      else
        {
          continueLoop = false;
        }
      /* if */
    }
  while ( true == continueLoop );

#pragma region // Cleanup

  if ((HANDLE)(NULL) != hTimerExposureTimeout)
    {
      BOOL const close = CloseHandle(hTimerExposureTimeout);
      assert(TRUE == close);
    }
  /* if */

  SAFE_DELETE( pFileSuffix );

  SpinlockTimerDelete( pTimer );

  PastEventsDelete( pEvents );

  {
    BOOL const set_terminate = pSynchronization->EventReset(CAMERA_TERMINATE, CameraID);
    assert(0 != set_terminate);
  }

  parameters->fActive = false;

  return 0;

#pragma endregion // Cleanup

}
/* AcquisitionThread */



/****** START/STOP THREAD ******/

#pragma region // Start/stop acquisition thread

//! Create acquisition parameters and start acquisition thread.
/*!
  Spawns acquisition thread.

  \param pSynchronization       Pointer to a structure holding all required synchronization events.
  \param pWindow Pointer to opened display window.
  \param pView Pointer to opened preview window.
  \param pImageEncoder   Pointer to image encoder thread structure.
  \param pImageDecoder   Pointer to image decoder thread structure.
  \param selected_camera_SDK Selected camera SDK. Default SDK is PointGrey FlyCapture2.
  \param CameraID Unique thread identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \param ProjectorID Unique projector identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \param pConnectedCameras A vector of pointers to strings which uniquely identifiy prohibited cameras. May be NULL.
  \param fallback_to_from_file Flag which indicates if fallback to acquisition from file is allowed.
  \return Returns pointer to acquisition thread parameters or NULL if unsuccessfull.
*/
AcquisitionParameters *
AcquisitionThreadStart(
                       SynchronizationEvents * const pSynchronization,
                       DisplayWindowParameters * const pWindow,
                       PreviewWindowParameters * const pView,
                       ImageEncoderParameters * const pImageEncoder,
                       ImageDecoderParameters * const pImageDecoder,
                       CameraSDK const selected_camera_SDK,
                       int const CameraID,
                       int const ProjectorID,
                       std::vector<std::wstring *> * const pConnectedCameras,
                       bool const fallback_to_from_file
                       )
{
  AcquisitionParameters * const P = (AcquisitionParameters *)malloc( sizeof(AcquisitionParameters) );
  assert(NULL != P);
  if (NULL == P) return P;

  AcquisitionParametersBlank_inline( P );

  /* Initialize variables. */
  P->pMetadataQueue = new ImageMetadataQueue();
  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) goto ACQUISITION_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsTriggerDuration);
  P->pStatisticsTriggerDuration = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsTriggerDuration);

  if (NULL == P->pStatisticsTriggerDuration) goto ACQUISITION_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsTriggerFrequency);
  P->pStatisticsTriggerFrequency = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsTriggerFrequency);

  if (NULL == P->pStatisticsTriggerFrequency) goto ACQUISITION_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsAcquisitionDuration);
  P->pStatisticsAcquisitionDuration = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsAcquisitionDuration);

  if (NULL == P->pStatisticsAcquisitionDuration) goto ACQUISITION_THREAD_START_EXIT;

  InitializeSRWLock( &(P->sLockAT) );

  /* Copy parameters. */
  assert(NULL == P->pSynchronization);
  P->pSynchronization = pSynchronization;
  assert(NULL != P->pSynchronization);

  assert(NULL == P->pWindow);
  P->pWindow = pWindow;
  assert(NULL != P->pWindow);

  assert(NULL == P->pView);
  P->pView = pView;
  assert(NULL != P->pView);

  assert(NULL == P->pImageEncoder);
  P->pImageEncoder = pImageEncoder;
  assert(NULL != P->pImageEncoder);

  assert(NULL == P->pImageDecoder);
  P->pImageDecoder = pImageDecoder;
  assert(NULL != P->pImageDecoder);

  assert(-1 == P->CameraID);
  P->CameraID = CameraID;
  assert( (0 <= P->CameraID) && (P->CameraID < (int)(P->pSynchronization->Camera.size())) );

  assert(-1 == P->ProjectorID);
  P->ProjectorID = ProjectorID;
  assert( (0 <= P->ProjectorID) && (P->ProjectorID < (int)(P->pSynchronization->Draw.size())) );

  /* Attach camera. */
  switch( selected_camera_SDK )
    {
    case CAMERA_SDK_DEFAULT:
    case CAMERA_SDK_FLYCAPTURE2:
    default:
      {
        assert(NULL == P->pFlyCapture2SDK);
        P->pFlyCapture2SDK = AcquisitionParametersFlyCapture2Create(P, P->nFrames, pConnectedCameras);
        //assert(NULL != P->pFlyCapture2SDK);
        if ( (NULL == P->pFlyCapture2SDK) && (true == fallback_to_from_file) )
          {
            std::cout << std::endl << gMsgAcquisitionFlyCap2RevertToFromFile << std::endl;
            goto ACQUISITION_THREAD_START_SDK_FROM_FILE;
          }
        /* if */
      }
      break;

    case CAMERA_SDK_SAPERA:
      {
        assert(NULL == P->pSaperaSDK);
        P->pSaperaSDK = AcquisitionParametersSaperaCreate(P, P->nFrames, pConnectedCameras);
        //assert(NULL != P->pSaperaSDK);
        if ( (NULL == P->pSaperaSDK) && (true == fallback_to_from_file) )
          {
            std::cout << std::endl << gMsgAcquisitionSaperaLTRevertToFromFile << std::endl;
            goto ACQUISITION_THREAD_START_SDK_FROM_FILE;
          }
        /* if */
      }
      break;

    case CAMERA_SDK_PYLON:
      {
        assert(NULL == P->pPylonSDK);
        P->pPylonSDK = AcquisitionParametersPylonCreate(P, P->nFrames, pConnectedCameras);
        //assert(NULL != P->pPylonSDK);
        if ( (NULL == P->pPylonSDK) && (true == fallback_to_from_file) )
          {
            std::cout << std::endl << gMsgAcquisitionSaperaLTRevertToFromFile << std::endl;
            goto ACQUISITION_THREAD_START_SDK_FROM_FILE;
          }
        /* if */
      }
      break;

    case CAMERA_SDK_FROM_FILE:
      {
      ACQUISITION_THREAD_START_SDK_FROM_FILE:

        assert(NULL == P->pFromFile);
        P->pFromFile = AcquisitionParametersFromFileCreate(P, NULL);
        assert(NULL != P->pFromFile);
      }
      break;

    }
  /* switch */

  if ( (NULL == P->pFlyCapture2SDK) &&
       (NULL == P->pSaperaSDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      goto ACQUISITION_THREAD_START_EXIT;
    }
  /* if */

  AdjustCameraExposureTime_inline(P, true);

  /* Start acquisition thread. */
  P->tAcquisition =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             AcquisitionThread,
                             (void *)( P ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != P->tAcquisition );

  if ( (HANDLE)( NULL ) == P->tAcquisition )
    {

    ACQUISITION_THREAD_START_EXIT:
      AcquisitionParametersRelease_inline( P );
      return NULL;

    }
  /* if */

  return P;
}
/* AcquisitionStart */



//! Stop acquisition thread.
/*!
  Stops image acquisition thread.

  \param P      Pointer to acquisition thread parameters.
*/
void
AcquisitionThreadStop(
                      AcquisitionParameters * const P
                      )
{
  //assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pSynchronization);
  if (NULL != P->pSynchronization)
    {
      DWORD const result = WaitForSingleObject(P->tAcquisition, 0);

      if ( (WAIT_OBJECT_0 != result) && (true == P->fActive) )
        {
          // The thread is alive so signal terminate event and wait for confirmation.
          BOOL const sm = P->pSynchronization->EventSet(CAMERA_TERMINATE, P->CameraID);
          assert(0 != sm);

          if (0 != sm)
            {
              DWORD const confirm = WaitForSingleObject(P->tAcquisition, INFINITE);
              assert(WAIT_OBJECT_0 == confirm);
            }
          /* if */
        }
      else
        {
          // The thread has already terminated.
        }
      /* if */
    }
  /* if */

  assert( WAIT_OBJECT_0 == WaitForSingleObject(P->tAcquisition, 0) );
  assert( false == P->fActive );

  AcquisitionParametersRelease_inline( P );
}
/* AcquisitionThreadStop */

#pragma endregion // Start/stop acquisition thread



/****** AUXILIARY FUNCTIONS ******/


#pragma region // Restart image transfer

//! Restart image transfers.
/*!
  Restarts image transfers.

  \param P      Pointer to acquisition parameters.
  \return Returns true if image transfers are successfully restarted.
*/
bool
AcquisitionThreadRestartCameraTransfers(
                                        AcquisitionParameters * const P
                                        )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const stop = StopPendingTransfers_inline(P);
  bool const start = StartImageTransfers_inline(P);

  return stop && start;
}
/* AcquisitionThreadRestartCameraTransfers */

#pragma endregion // Restart image transfer


#pragma region // Auxiliary functions to query the acquisition thread

//! Camera exposure time.
/*!
  Computes camera exposure time. Exposure time is always a multiplier of refresh rate.
  This is required as we do not want to capture half frame.

  \param P      Pointer to acquisition parameters.
  \return Returns exposure time in us if successfull and NaN if unsuccessfull.
*/
double
CameraExposureTimeFromRefreshRate(
                                  AcquisitionParameters * const P
                                  )
{
  assert(NULL != P);
  if (NULL == P) return BATCHACQUISITION_qNaN_dv;

  double const frameDuration_us = FrameDurationFromRefreshRate(P->pWindow); // us (microseconds)
  assert( !isnanorinf_inline(frameDuration_us) );
  if ( isnanorinf_inline(frameDuration_us) ) return BATCHACQUISITION_qNaN_dv;

  double const exposureTime_us = P->k * frameDuration_us; // us (microseconds)
  assert(0 < exposureTime_us);

  return exposureTime_us;
}
/* CameraExposureTimeFromRefreshRate */



//! Get acquisition method.
/*!
  Returns acquisition method.

  \param P      Pointer to acquisition parameters.
  \return Returns indicator of which camera SDK is used.
*/
CameraSDK
GetAcquisitionMethod(
                     AcquisitionParameters * const P
                     )
{
  assert(NULL != P);
  if (NULL == P) return CAMERA_SDK_UNKNOWN;

  if ( (NULL != P->pSaperaSDK) &&
       (NULL == P->pFlyCapture2SDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return CAMERA_SDK_SAPERA;
    }
  /* if */

  if ( (NULL == P->pSaperaSDK) &&
       (NULL != P->pFlyCapture2SDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return CAMERA_SDK_FLYCAPTURE2;
    }
  /* if */
  
  if ( (NULL == P->pSaperaSDK) &&
       (NULL == P->pFlyCapture2SDK) &&
       (NULL != P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return CAMERA_SDK_PYLON;
    }
  /* if */

  if ( (NULL == P->pSaperaSDK) &&
       (NULL == P->pFlyCapture2SDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL != P->pFromFile)
       )
    {
      return CAMERA_SDK_FROM_FILE;
    }
  /* if */

  return CAMERA_SDK_UNKNOWN;
}
/* GetAcquisitionMethod */



//! Test if acquisition is live.
/*!
  Function tests if acquisition subsystem is using a live camera.

  \param P      Pointer to acquisition parameters.
  \return Returns true if a live camera is attached.
*/
bool
IsAcquisitionLive(
                  AcquisitionParameters * const P
                  )
{
  assert(NULL != P);
  if (NULL == P) return false;

  if ( (NULL != P->pSaperaSDK) &&
       (NULL == P->pFlyCapture2SDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return true;
    }
  /* if */

  if ( (NULL == P->pSaperaSDK) &&
       (NULL != P->pFlyCapture2SDK) &&
       (NULL == P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return true;
    }
  /* if */  

  
  if ( (NULL == P->pSaperaSDK) &&
       (NULL == P->pFlyCapture2SDK) &&
       (NULL != P->pPylonSDK) &&
       (NULL == P->pFromFile)
       )
    {
      return true;
    }
  /* if */

  return false;
}
/* IsAcquisitionLive */



//! Get unique camera identifier.
/*!
  Gets string which uniquely identifies attached camera.
  Returned pointer must be deleted after unique camera identifier is no longer needed.

  \param P      Pointer to acquisition parameters.
  \return Returns a pointer to wstring which uniquely identifies attached camera or NULL pointer otherwise.
*/
std::wstring *
GetUniqueCameraIdentifier(
                          AcquisitionParameters * const P
                          )
{
  std::wstring * name = NULL;

  assert(NULL != P);
  if (NULL == P) return name;

  if (NULL != P->pFlyCapture2SDK)
    {
      name = AcquisitionParametersFlyCapture2GetCameraIdentifier(P->pFlyCapture2SDK);
      assert(NULL != name);
    }
  else if (NULL != P->pSaperaSDK)
    {
      name = AcquisitionParametersSaperaGetCameraIdentifier(P->pSaperaSDK);
      assert(NULL != name);
    }
  else if (NULL != P->pPylonSDK)
    {
    name = AcquisitionParametersPylonGetCameraIdentifier(P->pPylonSDK);
    assert(NULL != name);
    }
  else if (NULL != P->pFromFile)
    {
      wchar_t const * const directory = AcquisitionParametersFromFileGetDirectory(P->pFromFile);
      name = new std::wstring( directory );
      assert(NULL != name);
    }
  else
    {
      assert(NULL == name);
    }
  /* if */

  return name;
}
/* GetUniqueCameraIdentifier */



//! Test if all acquisition methods are from file.
/*!
  Tests if all acquisition methods are from file.

  \param sAcquisition  Reference to vector of acquisition thread parameters.
  \param ThreadStorageLock      Address of SRW lock which controls access to sAcquisition.
  \return Returns true if all acquisitions use dummy from file acquisition.
*/
bool
AreAllAcquisitionMethodsFromFile(
                                 std::vector<AcquisitionParameters *> & sAcquisition,
                                 PSRWLOCK const ThreadStorageLock
                                 )
{
  assert(NULL != ThreadStorageLock);
  if (NULL == ThreadStorageLock) return false;

  int const num_cam = (int)( sAcquisition.size() );
  bool all_from_file = true;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, i, ThreadStorageLock);
      assert(NULL != pAcquisition);

      if (CAMERA_SDK_FROM_FILE != GetAcquisitionMethod(pAcquisition))
        {
          all_from_file = false;
          break;
        }
      /* if */
    }
  /* for */

  return all_from_file;
}
/* AreAllAcquisitionMethodsFromFile */



//! Test if any acquisition method is from file.
/*!
  Tests if any acquisition method is from file.

  \param sAcquisition  Reference to vector of acquisition thread parameters.
  \param ThreadStorageLock      Address of SRW lock which controls access to sAcquisition.
  \return Returns true if any acquisition uses dummy from file acquisition.
*/
bool
IsAnyAcquisitionMethodFromFile(
                               std::vector<AcquisitionParameters *> & sAcquisition,
                               PSRWLOCK const ThreadStorageLock
                               )
{
  assert(NULL != ThreadStorageLock);
  if (NULL == ThreadStorageLock) return false;

  int const num_cam = (int)( sAcquisition.size() );
  bool any_from_file = false;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = get_ptr_inline(sAcquisition, i, ThreadStorageLock);
      assert(NULL != pAcquisition);

      if (CAMERA_SDK_FROM_FILE == GetAcquisitionMethod(pAcquisition))
        {
          any_from_file = true;
          break;
        }
      /* if */
    }
  /* for */

  return any_from_file;
}
/* IsAnyAcquisitionMethodFromFile */


//! Rescan input directory.
/*!
  Rescans input directory.

  \param P      Pointer to acquisition parameters.
  \return Returns true if successfull, false otherwise.
*/
bool
AcquisitionThreadRescanInputDirectory(
                                      AcquisitionParameters * const P
                                      )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pFromFile);
  if (NULL == P->pFromFile) return false;
  
  assert(NULL != P->pFromFile->pFileList);
  if (NULL == P->pFromFile->pFileList) return false;

  if (NULL == P->pFromFile->pFileList->directory_name) return false;

  int const sz = 1024;
  wchar_t szTitle[sz + 1];
  int const cnt1 = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForCamera, P->CameraID + 1);
  assert(0 < cnt1);
  szTitle[sz] = 0; 

  bool const rescan = P->pFromFile->pFileList->SetDirectory(P->pFromFile->pFileList->directory_name->c_str(), szTitle);
  assert(true == rescan);

  return rescan;  
}
/* AcquisitionThreadRescanInputDirectory */

#pragma endregion // Auxiliary functions to query the acquisition thread


#pragma region // Change event IDs

//! Set new projector ID.
/*!
  Sets new projector ID. Acquisition thread must be in waiting state when this function
  is called; thread may be put into waiting state by signalling MAIN_PREPARE_CAMERA to the
  thread or MAIN_PREPARE_DRAW to the parent rendering thread.

  \param P      Pointer to acquisition thread parameters.
  \param ProjectorID    Projector ID to set.
  \return Returns true if successfull.
*/
bool
AcquisitionThreadSetNewProjectorID(
                                   AcquisitionParameters * const P,
                                   int const ProjectorID
                                   )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pSynchronization);
  if (NULL == P->pSynchronization) return false;

  assert(true == P->fWaiting);
  if (false == P->fWaiting) return false;

  int const CameraIDOld = P->CameraID;
  int const ProjectorIDOld = P->ProjectorID;

  if (ProjectorIDOld == ProjectorID) return true;

  assert( (0 <= ProjectorID) && (ProjectorID < (int)(P->pSynchronization->Draw.size())) );

  bool set = true;

  // Set new event ID.
  P->ProjectorID = ProjectorID;

  // Signal to the thread to implement ID change.
  {
    assert(true == P->fWaiting);

    BOOL const change_acquisition = P->pSynchronization->EventSet(CAMERA_CHANGE_ID, CameraIDOld);
    assert(0 != change_acquisition);
    set == set && (0 != change_acquisition);
  }

  // Wait for the acquisition thread to change ID.
  {
    bool acquisition_changing = false;
    DWORD dwIsAcquisitionChangingResult = WAIT_FAILED;
    do
      {
        if (true == acquisition_changing) SleepEx(1, TRUE);
        dwIsAcquisitionChangingResult = P->pSynchronization->EventWaitFor(CAMERA_CHANGE_ID, CameraIDOld, (DWORD)0);
        acquisition_changing = (WAIT_OBJECT_0 == dwIsAcquisitionChangingResult);
      }
    while (true == acquisition_changing);
  }

  return set;
}
/* AcquisitionThreadSetNewProjectorID */



//! Set new camera and encoder ID.
/*!
  Sets new camera and encoder IDs.  Acquisition and encoder threads must be in waiting state
  when this function is called; threads may be put into waiting state by signalling
  MAIN_PREPARE_CAMERA to the acquisition thread or MAIN_PREPARE_DRAW to the parent rendering thread.

  \param P      Pointer to acquisition thread parameters.
  \param CameraID       New camera ID to set.
  \param EncoderID   New encoder ID to set.
  \return Returns true if successfull.
*/
bool
AcquisitionThreadSetNewCameraIDAndEncoderID(
                                            AcquisitionParameters * const P,
                                            int const CameraID,
                                            int const EncoderID
                                            )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pSynchronization);
  if (NULL == P->pSynchronization) return false;

  assert(NULL != P->pImageEncoder);
  if (NULL == P->pImageEncoder) return false;

  assert(true == P->fWaiting);
  if (false == P->fWaiting) return false;

  int const CameraIDOld = P->CameraID;
  int const EncoderIDOld = P->pImageEncoder->EncoderID;

  assert( (0 <= CameraID) && (CameraID < (int)(P->pSynchronization->Camera.size())) );
  assert( (0 <= EncoderID) && (EncoderID <= (int)(P->pSynchronization->ImageEncoder.size())) );
  assert( CameraIDOld == P->pImageEncoder->CameraID );

  bool set = true;
  
  // Change event IDs.
  {
    P->CameraID = CameraID;
    P->pImageEncoder->CameraID = CameraID;
    P->pImageEncoder->EncoderID = EncoderID;
  }

  // Signal to the threads to implement ID change.
  {
    assert(true == P->fWaiting);
    assert(true == P->pImageEncoder->fWaiting);

    BOOL const change_acquisition = P->pSynchronization->EventSet(CAMERA_CHANGE_ID, CameraIDOld);
    assert(0 != change_acquisition);
    set == set && (0 != change_acquisition);

    BOOL const change_encoder = P->pSynchronization->EventSet(IMAGE_ENCODER_CHANGE_ID, EncoderIDOld);
    assert(0 != change_encoder);
    set == set && (0 != change_encoder);
  }

  // Wait for acquisition thread to change event IDs.
  {
    bool acquisition_changing = false;
    DWORD dwIsAcquisitionChangingResult = WAIT_FAILED;
    do
      {
        if (true == acquisition_changing) SleepEx(1, TRUE);
        dwIsAcquisitionChangingResult = P->pSynchronization->EventWaitFor(CAMERA_CHANGE_ID, CameraIDOld, (DWORD)0);
        acquisition_changing = (WAIT_OBJECT_0 == dwIsAcquisitionChangingResult);
      }
    while (true == acquisition_changing);
  }

  // Wait for encoder thread to change event IDs.
  {
    bool encoder_changing = false;
    DWORD dwIsEncoderChangingResult = WAIT_FAILED;
    do
      {
        if (true == encoder_changing) SleepEx(1, TRUE);
        dwIsEncoderChangingResult = P->pSynchronization->EventWaitFor(IMAGE_ENCODER_CHANGE_ID, EncoderIDOld, (DWORD)0);
        encoder_changing = (WAIT_OBJECT_0 == dwIsEncoderChangingResult);
      }
    while (true == encoder_changing);
  }
  
  return set;
}
/* AcquisitionThreadSetNewCameraIDAndEncoderID */

#pragma endregion // Change event IDs


#endif /* !__BATCHACQUISITIONACQUISITION_CPP */
