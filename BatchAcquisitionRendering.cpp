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
  \file   BatchAcquisitionRendering.cpp
  \brief  Image rendering thread.

  \author Tomislav Petkovic
  \date   2017-01-27
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONRENDERING_CPP
#define __BATCHACQUISITIONRENDERING_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionRendering.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionImageRender.h"



/****** HELPER FUNCTIONS ******/

#pragma region // Event name from code

//! Event names.
/*!
  This static array contains all event names which rendering thread processes.
  See function RenderingThread for event details.
*/
static
TCHAR const * const RenderingThreadEventNames[] = {
  /* 0 */ L"DRAW_TERMINATE",
  /* 1 */ L"MAIN_PREPARE_DRAW",
  /* 2 */ L"MAIN_BEGIN",
  /* 3 */ L"DRAW_RENDER",
  /* 4 */ L"DRAW_PRESENT",
  /* 5 */ L"DRAW_VBLANK",
  /* 6 */ L"CAMERA_SYNC_TRIGGERS"
};


//! Get event name.
/*!
  Function returns pointer to a string which contains event name.

  \param hnr    Event code.
  \return Pointer to string or NULL pointer.
*/
inline
TCHAR const * const
GetRenderingThreadEventName_inline(
                                   int const hnr
                                   )
{
  switch (hnr)
    {
    case 0: return RenderingThreadEventNames[0];
    case 1: return RenderingThreadEventNames[1];
    case 2: return RenderingThreadEventNames[2];
    case 3: return RenderingThreadEventNames[3];
    case 4: return RenderingThreadEventNames[4];
    case 5: return RenderingThreadEventNames[5];
    case 6: return RenderingThreadEventNames[6];
    }
  /* switch */
  return NULL;
}
/* GetRenderingThreadEventName_inline */

#pragma endregion // Event name from code


#pragma region // Concurrent access to nth attached camera

//! Gets nth CameraID.
/*!
  Returns ID of nth camera in the corresponding camera ID storage vector.

  \param P      Pointer to rendering thread parameters.
  \param n      Index of camera to return.
*/
inline
int
nth_ID(
       RenderingParameters_ * const P,
       int const n
       )
{
  int CameraID = -1;

  assert(NULL != P);
  if (NULL == P) return CameraID;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return CameraID;

  AcquireSRWLockShared( &(P->sLockAcquisitions) );
  {
    int const n_max = (int)(P->pAcquisitions->size());
    //assert( (0 <= n) && (n < n_max) );
    if ( (0 <= n) && (n < n_max) )
      {
        AcquisitionParameters * const pAcquisition = ( *(P->pAcquisitions) )[n];
        CameraID = pAcquisition->CameraID;
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockAcquisitions) );

  return CameraID;
}
/* nth_ID */



//! Gets nth pointer to acquisition thread parameters.
/*!
  Returns pointer to acquisition thread parameters.

  \param P      Pointer to rendering thread parameters.
  \param n      Index of camera to return.
*/
inline
AcquisitionParameters *
nth_pAcquisition(
                 RenderingParameters_ * const P,
                 int const n
                 )
{
  AcquisitionParameters * pAcquisition = NULL;

  assert(NULL != P);
  if (NULL == P) return pAcquisition;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return pAcquisition;

  AcquireSRWLockShared( &(P->sLockAcquisitions) );
  {
    int const n_max = (int)(P->pAcquisitions->size());
    //assert( (0 <= n) && (n < n_max) );
    if ( (0 <= n) && (n < n_max) )
      {
        pAcquisition = ( *(P->pAcquisitions) )[n];
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockAcquisitions) );

  return pAcquisition;
}
/* nth_pAcquisition */



//! Gets nth pointer to rendering thread parameters.
/*!
  Returns pointer to rendering thread parameters.

  \param P      Pointer to rendering thread parameters.
  \param n      Index of camera to return.
*/
inline
RenderingParameters_ *
nth_pRendering(
               RenderingParameters_ * const P,
               int const n
               )
{
  RenderingParameters_ * pRendering = NULL;

  assert(NULL != P);
  if (NULL == P) return pRendering;

  assert(NULL != P->pRenderings);
  if (NULL == P->pRenderings) return pRendering;

  AcquireSRWLockShared( &(P->sLockRenderings) );
  {
    int const n_max = (int)(P->pRenderings->size());
    //assert( (0 <= n) && (n < n_max) );
    if ( (0 <= n) && (n < n_max) )
      {
        pRendering = ( *(P->pRenderings) )[n];
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockRenderings) );

  return pRendering;
}
/* nth_pRendering */

#pragma endregion // Concurrent access to nth attached camera


#pragma region // Blanking and destruction of RenderingParameters

//! Blanks rendering thread parameters.
/*!
  Blanks rendering thread parameters.

  \param P      Pointer to rendering thread parametes.
*/
inline
void
RenderingParametersBlank_inline(
                                RenderingParameters * const P
                                )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Blank structure. */
  P->tRendering = (HANDLE)(NULL);

  P->ProjectorID = -1;

  P->SyncInterval = 1; // Set to 0 for immediate present operation.

  P->delay_ms = -1.0;

  P->fActive = false;
  P->fWaiting = false;
  P->fBatch = false;
  P->fSavePNG = false;
  P->fSaveRAW = true;
  P->fSynchronize = false;

  P->num_prj = -1;

  P->pRenderings = NULL;
  ZeroMemory( &(P->sLockRenderings), sizeof(P->sLockRenderings) );

  P->pTriggers = NULL;

  P->pAcquisitions = NULL;
  ZeroMemory( &(P->sLockAcquisitions), sizeof(P->sLockAcquisitions) );

  P->pStatisticsRenderDuration = NULL;
  P->pStatisticsPresentDuration = NULL;
  P->pStatisticsPresentFrequency = NULL;
  P->pStatisticsWaitForVBLANKDuration = NULL;

  P->pSynchronization = NULL;
  P->pWindow = NULL;
  P->pImageDecoder = NULL;

  /* Set default present-to-trigger delay for blocking acquisition mode.
     Measured delay times:
	 Mitsubishi EW230U-ST 16.804ms for 3D frame sequential 1280x800@119.909Hz
	 Canon LV-WX310-ST 25.234ms for 3D frame sequential 1280x800@119.909Hz
	 Canon LV-WX310-ST around 70ms for HDMI 1280x800@59.81Hz
	 Acer S1383WHne 33.505ms for 3D frame sequential 1280x800@119.909Hz
  */

  //P->delay_ms = 17.0; // Mitsubishi EW230U-ST using HDMI connection at 120Hz refresh.
  //P->delay_ms = 45.0; // Acer X1260 using VGA connection at 60Hz refresh.
  P->delay_ms = 80.0;
}
/* RenderingParametersBlank_inline */



//! Releases rendering thread parameters structure.
/*!
  Releases resources allocated by rendering thread.

  \param P      Pointer to rendering parametes.
*/
inline
void
RenderingParametersRelease_inline(
                                  RenderingParameters * const P
                                  )
{
  assert(NULL != P);
  if (NULL == P) return;

  // Delete list of synchronized projectors.
  AcquireSRWLockExclusive( &(P->sLockRenderings) );
  {
    SAFE_DELETE( P->pRenderings );
  }
  ReleaseSRWLockExclusive( &(P->sLockRenderings) );

  // Delete trigger queue.
  SAFE_DELETE( P->pTriggers );

  // Delete list of attached cameras.
  AcquireSRWLockExclusive( &(P->sLockAcquisitions) );
  {
    SAFE_DELETE( P->pAcquisitions );
  }
  ReleaseSRWLockExclusive( &(P->sLockAcquisitions) );

  // Delete statistics.
  FrameStatisticsDelete( P->pStatisticsRenderDuration );
  FrameStatisticsDelete( P->pStatisticsPresentDuration );
  FrameStatisticsDelete( P->pStatisticsPresentFrequency );
  FrameStatisticsDelete( P->pStatisticsWaitForVBLANKDuration );

  RenderingParametersBlank_inline( P );

  free(P);
}
/* RenderingParametersRelease_inline */

#pragma endregion // Blanking and destruction of RenderingParameters



/****** TRIGGER QUEUE FOR NON-BLOCKING ACQUISITION ******/

#pragma region // Trigger queue

//! Blanks present and trigger times structure.
/*!
  Blanks present and trigger times structure.

  \param P     Pointer to PresentAndTriggerTimes structure.
*/
inline
void
PresentAndTriggerTimesBlank_inline(
                                   PresentAndTriggerTimes * const P
                                   )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->key = -1;
  P->present_counter = -1;
  P->vblank_counter = -1;

  P->vblank_counter_trigger_scheduled = -1;
  P->vblank_counter_next_scheduled = -1;
  P->vblank_counter_next_presented = -1;

  P->QPC_current_presented = -1;
  P->QPC_trigger_scheduled_RT = -1;
  P->QPC_trigger_scheduled_AT = -1;
  P->QPC_next_scheduled = -1;
  P->QPC_next_presented = -1;
}
/* PresentAndTriggerTimesBlank_inline */



//! Empties trigger queue.
/*!
  Empties trigger queue.

  \param pTriggers     Pointer to timing queue.
*/
inline
void
EmptyTriggerQueue_inline(
                         std::vector<PresentAndTriggerTimes> * const pTriggers
                         )
{
  assert(NULL != pTriggers);
  if (NULL == pTriggers) return;

  while (false == pTriggers->empty())
    {
      PresentAndTriggerTimes sTimes = pTriggers->back();
      pTriggers->pop_back();
      PresentAndTriggerTimesBlank_inline( &sTimes );
    }
  /* while */
}
/* EmptyTriggerQueue_inline */



//! Insert timing information into queue.
/*!
  Inserts timing information for the currently presented frame into the trigger queue.
  Function also updates the previous element with the information about the current frame.

  \param pTriggers      Pointer to timing queue.
  \param pWindow        Pointer to DisplayWindowParameters structure.
  \param key            A unique item identifier.
  \param present_counter Present counter value of the current frame.
  \param vblank_counter VBLANK counter value of the current frame.
  \param QPC_current_presented    QPC value at which the current frame was presented.
  \param fLast    Flag to indicate last frame to be presented. For this frame there is no next frame scheduled.
*/
inline
void
AddToTriggerQueue_inline(
                         std::vector<PresentAndTriggerTimes> * const pTriggers,
                         DisplayWindowParameters * const pWindow,
                         long int const key,
                         long int const present_counter,
                         long int const vblank_counter,
                         __int64 const QPC_current_presented,
                         bool const fLast
                         )
{
  assert(NULL != pTriggers);
  if (NULL == pTriggers) return;

  assert(NULL != pWindow);
  if (NULL == pWindow) return;

  // Concurrently fetch data from pWindow.
  long int delayTime_whole = -1;
  long int presentTime = -1;
  __int64 QPC_delay_for_trigger_scheduled_RT = -1;
  __int64 QPC_delay_for_trigger_scheduled_AT = -1;
  __int64 QPC_presentTime = -1;
  AcquireSRWLockShared( &(pWindow->sLockRT) );
  {
    delayTime_whole = pWindow->delayTime_whole;
    presentTime = pWindow->presentTime;
    QPC_delay_for_trigger_scheduled_RT = pWindow->QPC_delay_for_trigger_scheduled_RT;
    QPC_delay_for_trigger_scheduled_AT = pWindow->QPC_delay_for_trigger_scheduled_AT;
    QPC_presentTime = pWindow->QPC_presentTime;
  }
  ReleaseSRWLockShared( &(pWindow->sLockRT) );
  assert( 0 <= delayTime_whole );
  assert( 0 <= presentTime );
  assert( 0 <= QPC_delay_for_trigger_scheduled_RT );
  assert( 0 <= QPC_delay_for_trigger_scheduled_AT );
  assert( 0 <= QPC_presentTime );

  assert( 0 <= vblank_counter );

  // Compute timing information.
  PresentAndTriggerTimes sTimes;
  PresentAndTriggerTimesBlank_inline( &sTimes );

  sTimes.key = key;
  sTimes.present_counter = present_counter;
  sTimes.vblank_counter = vblank_counter;

  sTimes.vblank_counter_trigger_scheduled = vblank_counter + delayTime_whole;
  sTimes.vblank_counter_next_scheduled = vblank_counter + presentTime;
  assert( -1 == sTimes.vblank_counter_next_presented );

  sTimes.QPC_current_presented = QPC_current_presented;
  sTimes.QPC_trigger_scheduled_RT = QPC_current_presented + QPC_delay_for_trigger_scheduled_RT;
  sTimes.QPC_trigger_scheduled_AT = QPC_current_presented + QPC_delay_for_trigger_scheduled_AT;
  if (false == fLast)
    {
      sTimes.QPC_next_scheduled = QPC_current_presented + QPC_presentTime;
    }
  else
    {
      assert( -1 == sTimes.QPC_next_scheduled );
    }
  /* if */
  assert( -1 == sTimes.QPC_next_presented );

  assert( sTimes.QPC_current_presented <= sTimes.QPC_trigger_scheduled_RT );
  assert( sTimes.QPC_trigger_scheduled_RT <= sTimes.QPC_trigger_scheduled_AT );

  // Update previous element in the queue.
  if ( false == pTriggers->empty() )
    {
      // Consistency check.
      assert( key - 1 == pTriggers->back().key );
      assert( present_counter > pTriggers->back().present_counter );
      assert( vblank_counter > pTriggers->back().vblank_counter );
      assert( QPC_current_presented > pTriggers->back().QPC_next_presented );
      assert( sTimes.QPC_trigger_scheduled_RT > pTriggers->back().QPC_trigger_scheduled_RT );
      assert( sTimes.QPC_trigger_scheduled_AT > pTriggers->back().QPC_trigger_scheduled_AT );

      // Update.
      assert(-1 == pTriggers->back().vblank_counter_next_presented);
      pTriggers->back().vblank_counter_next_presented = vblank_counter;

      assert(-1 == pTriggers->back().QPC_next_presented);
      pTriggers->back().QPC_next_presented = QPC_current_presented;
    }
  /* if */

  // Insert timing information into queue.
  pTriggers->push_back( sTimes );
}
/* AddToTriggerQueue_inline */



//! Test if trigger time exists.
/*!
  Function tests if there exist a valid trigger time.

  \param pTriggers      Pointer to timing queue.
*/
inline
bool
HaveTriggerTime_inline(
                       std::vector<PresentAndTriggerTimes> * const pTriggers
                       )
{
  if (NULL == pTriggers) return false;
  if (true == pTriggers->empty()) return false;
  return true;
}
/* HaveTriggerTime_inline */



//! Pops first element in the queue.
/*!
  Removes first element from the queue and stores retrieved data at the specified address.

  \param pTriggers      Pointer to timing queue.
  \param pTimes Address where timing values will be stored. May be NULL.
  \return Returns true if successfull, false otherwise.
*/
inline
bool
PopTriggerTime_inline(
                      std::vector<PresentAndTriggerTimes> * const pTriggers,
                      PresentAndTriggerTimes * const pTimes
                      )
{
  assert(NULL != pTriggers);
  if (NULL == pTriggers) return false;

  if ( true == pTriggers->empty() )
    {
      if (NULL != pTimes) PresentAndTriggerTimesBlank_inline( pTimes );
      return false;
    }
  /* if */

  if (NULL != pTimes) *pTimes = pTriggers->front();
  pTriggers->erase( pTriggers->begin() );

  return true;
}
/* PopTriggerTime_inline */



//! Peeks at first element in the queue.
/*!
  Fetches first element from the queue and stores retrieved data at the specified address.
  Element is not removed from the queue.

  \param pTriggers      Pointer to timing queue.
  \param pTimes Address where timing values will be stored. May be NULL.
  \return Returns true if successfull, false otherwise.
*/
inline
bool
PeekTriggerTime_inline(
                       std::vector<PresentAndTriggerTimes> * const pTriggers,
                       PresentAndTriggerTimes * const pTimes
                       )
{
  assert(NULL != pTriggers);
  if (NULL == pTriggers) return false;

  if ( true == pTriggers->empty() )
    {
      if (NULL != pTimes) PresentAndTriggerTimesBlank_inline( pTimes );
      return false;
    }
  /* if */

  if (NULL != pTimes) *pTimes = pTriggers->front();

  return true;
}
/* PeekTriggerTime_inline */



//! Removes expired triggers.
/*!
  Removes all expired triggers from the trigger queue.

  Triggers are considered expired (in the past) if their's latest allowed trigger time is
  earlier then the current time. Note that we only remove triggers if the actual presentation
  time of the next frame is known.

  \param pTriggers     Pointer to timing queue.
  \param pWindow        Pointer to DisplayWindowParameters structure.
  \param ProjectorID   ID of the projector which handles the queue.
  \return Returns true if successfull, false otherwise.
*/
inline
bool
RemoveExpiredTriggers_inline(
                             std::vector<PresentAndTriggerTimes> * const pTriggers,
                             DisplayWindowParameters * const pWindow,
                             int const ProjectorID
                             )
{
  assert(NULL != pTriggers);
  if (NULL == pTriggers) return false;

  assert(NULL != pWindow);
  if (NULL == pWindow) return false;

  // Concurrently fetch data from pWindow.
  __int64 QPC_delayTime = -1;
  __int64 QPC_exposureTime = -1;
  long int vblank_counter = -1;
  AcquireSRWLockShared( &(pWindow->sLockRT) );
  {
    QPC_delayTime = pWindow->QPC_delayTime;
    QPC_exposureTime = pWindow->QPC_exposureTime;
    vblank_counter = pWindow->vblank_counter;
  }
  ReleaseSRWLockShared( &(pWindow->sLockRT) );
  assert( 0 <= QPC_delayTime );
  assert( 0 <= QPC_exposureTime );
  assert( 0 <= vblank_counter);

  // Fetch current time.
  LARGE_INTEGER QPC_now;
  QPC_now.QuadPart = (LONGLONG)(-1);

  BOOL const query_qpc = QueryPerformanceCounter( &QPC_now );
  assert(TRUE == query_qpc);

  if ( (TRUE != query_qpc) || ((LONGLONG)(-1) == QPC_now.QuadPart) ) return false;

  // Remove invalid triggers.
  bool done = false;
  do
    {
      PresentAndTriggerTimes sTimes;
      bool const peek = PeekTriggerTime_inline(pTriggers, &sTimes);
      if (true == peek)
        {
          // Get latest allowed trigger time.
          __int64 QPC_delay_after_next = QPC_delayTime - QPC_exposureTime;
          if (0 > QPC_delay_after_next) QPC_delay_after_next = 0;

          __int64 QPC_trigger_latest = -1;
          if (0 < sTimes.QPC_next_presented) QPC_trigger_latest = sTimes.QPC_next_presented;
          if (0 < QPC_trigger_latest) QPC_trigger_latest = QPC_trigger_latest + QPC_delay_after_next;
          if (0 <= QPC_trigger_latest)
            {
              QPC_trigger_latest = QPC_trigger_latest + QPC_delay_after_next;

              if ( QPC_trigger_latest > QPC_now.QuadPart )
                {
                  bool const pop = PopTriggerTime_inline(pTriggers, NULL);
                  assert(true == pop);

                  if (false == pop)
                    {
                      // Abort immediately if delete operation failed.
                      return false;
                    }
                  else
                    {
                      Debugfprintf(
                                   stderr,
                                   gDbgTriggerDropForMetadata,
                                   ProjectorID + 1,
                                   sTimes.key + 1, vblank_counter,
                                   __FILE__, __LINE__
                                   );
                    }
                  /* if */
                }
              else
                {
                  done = true;
                }
              /* if */
            }
          else
            {
              assert(-1 == sTimes.QPC_next_presented);
              done = true;
            }
          /* if */
        }
      else
        {
          done = true;
        }
      /* if */
    }
  while (false == done);

  return done;
}
/* RemovePastTriggerTimes_inline */

#pragma endregion // Trigger queue



/****** HELPER FUNCTIONS FOR SYNCHRONIZATION ******/

#pragma region // Wait for many cameras

//! Tests if all cameras are ready.
/*!
  Function tests if all cameras are ready.

  \param parameters     Pointer to rendering thread parameters structure.
  \param pSynchronization       Pointer to synchronization structure.
  \param num_cam        Number of attached cameras.
  \return Returns true if all cameras are ready; false otherwise.
*/
inline
bool
AreAllCamerasReady_inline(
                          RenderingParameters * const parameters,
                          SynchronizationEvents * const pSynchronization,
                          int const num_cam
                          )
{
  assert(NULL != parameters);
  if (NULL == parameters) return false;

  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return false;

  bool all_ready = true;

  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
      assert(NULL != pAcquisition);

      int const CameraID = pAcquisition->CameraID;
      assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

      DWORD const dwIsReadyResult = pSynchronization->EventWaitFor(CAMERA_READY, CameraID, (DWORD)0);
      bool const camera_ready = (WAIT_OBJECT_0 == dwIsReadyResult);
      if (false == camera_ready) return false;
      all_ready = all_ready && camera_ready;
    }
  /* for */

  return all_ready;
}
/* AreAllCamerasReady_inline */



//! Waits for all cameras to become ready.
/*!
  Function waits for all attached cameras to become ready.
  Function also listens to DRAW_TERMINATE and MAIN_PREPARE_DRAW events; if any of these two
  signals is signalled then function returns.

  \param parameters     Pointer to rendering thread parameters structure.
  \param pSynchronization       Pointer to synchronization structure.
  \param num_cam        Number of attached cameras.
  \param dwMilliseconds Wait time.
  \return Function returns wait result. Subtract the WAIT_OBJECT_0 to indentify which events occured:
  if result is 0 then DRAW_TERMINATE event was signalled;
  if result is 1 then MAIN_PREPARE_DRAW event was signalled; and
  if result is 2 then CAMERA_READY event was signalled for all cameras;
  otherwise a wait error occured.
*/
inline
DWORD
WaitForAllCamerasToBecomeReady_inline(
                                      RenderingParameters * const parameters,
                                      SynchronizationEvents * const pSynchronization,
                                      int const num_cam,
                                      DWORD const dwMilliseconds
                                      )
{
  assert(NULL != parameters);
  if (NULL == parameters) return WAIT_FAILED;

  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return WAIT_FAILED;

  std::vector<SynchronizationCodes> ID_any(2);
  ID_any[0] = DRAW_TERMINATE;
  ID_any[1] = MAIN_PREPARE_DRAW;
  std::vector<int> H_any(2, parameters->ProjectorID);

  std::vector<SynchronizationCodes> ID_all(num_cam, CAMERA_READY);
  std::vector<int> H_all(num_cam);
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
      assert(NULL != pAcquisition);

      int const CameraID = pAcquisition->CameraID;
      assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

      H_all[i] = CameraID;
    }
  /* for */

  DWORD const dwIsReadyResult = pSynchronization->EventWaitForAnyAndAll(ID_any, H_any, ID_all, H_all, dwMilliseconds);

  return dwIsReadyResult;
}
/* WaitForAllCamerasToBecomeReady_inline */



//! Waits for all cameras to end batch.
/*!
  Function waits for all attached cameras to end batch acquisition.
  Function also listens to DRAW_TERMINATE and MAIN_PREPARE_DRAW events;
  if any of these two signals is signalled the function returns.

  \param parameters     Pointer to rendering thread parameters structure.
  \param pSynchronization       Pointer to synchronization structure.
  \param num_cam        Number of attached cameras.
  \param dwMilliseconds Wait time.
  \return Function returns wait result. Subtract the WAIT_OBJECT_0 to indentify which events occured:
  if result is 0 then DRAW_TERMINATE event was signalled;
  if result is 1 then MAIN_PREPARE_DRAW event was signalled; and
  if result is 2 then MAIN_END_CAMERA event was signalled for all cameras;
  otherwise a wait error occured.
*/
inline
DWORD
WaitForAllCamerasToEndBatch_inline(
                                   RenderingParameters * const parameters,
                                   SynchronizationEvents * const pSynchronization,
                                   int const num_cam,
                                   DWORD const dwMilliseconds
                                   )
{
  assert(NULL != parameters);
  if (NULL == parameters) return WAIT_FAILED;

  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return WAIT_FAILED;

  std::vector<SynchronizationCodes> ID_any(2);
  ID_any[0] = DRAW_TERMINATE;
  ID_any[1] = MAIN_PREPARE_DRAW;
  std::vector<int> H_any(2, parameters->ProjectorID);

  std::vector<SynchronizationCodes> ID_all(num_cam, MAIN_END_CAMERA);
  std::vector<int> H_all(num_cam);
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
      assert(NULL != pAcquisition);

      int const CameraID = pAcquisition->CameraID;
      assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

      H_all[i] = CameraID;
    }
  /* for */

  DWORD const dwIsReadyResult = pSynchronization->EventWaitForAnyAndAll(ID_any, H_any, ID_all, H_all, dwMilliseconds);

  return dwIsReadyResult;
}
/* WaitForAllCamerasToEndBatch_inline */

#pragma endregion // Wait for many cameras


#pragma region // Wait for delay to elapse

//! Sleeps for required delay.
/*!
  Function sleeps for required delay.

  The specified delay time is the time between last successfull VBLANK and the camera trigger,
  therefore we must first determine how much time has elpased from the last successfull VBLANK.
  Once this value is computed this function will execute SleepEx for requred number of milliseconds.
  If the time of last VBLANK is unkwnon (indicated by value 0) then function will sleep for
  full amout of delay time.

  \param parameters     Pointer to rendering thread parameters.
  \param pWindow        Pointer to display window parameters.
  \param QPC_after_VBLANK       QPC value after last VBLANK interrupt.
*/
inline
void
SleepForRequiredDelay_inline(
                             RenderingParameters * const parameters,
                             DisplayWindowParameters * const pWindow,
                             LARGE_INTEGER const QPC_after_VBLANK
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
  if ( (LONGLONG)0 < QPC_after_VBLANK.QuadPart )
    {
      BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_sleep );
      assert(TRUE == qpc_before);

      if ( (TRUE != qpc_before) || ((LONGLONG)(-1) == QPC_before_sleep.QuadPart) ) return;

      elapsed_ms = (double)(QPC_before_sleep.QuadPart - QPC_after_VBLANK.QuadPart) * pWindow->ticks_to_ms;
      assert(0 < elapsed_ms);
    }
  /*if  */

  // Compute remaining delay time.
  double remaining_ms = 0.0;
  if (0 < parameters->delay_ms)
    {
      remaining_ms = parameters->delay_ms - elapsed_ms;
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
    }
  /* if */

  // Total elapsed time must be larger than requested.
  {
    LONGLONG const stop = QPC_after_VBLANK.QuadPart + (LONGLONG)(parameters->delay_ms * pWindow->ms_to_ticks + 0.5);

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
/* SleepForRequiredDelay_inline */

#pragma  endregion // Wait for delay to elapse


#pragma region // Test multiple projector synchronization

//! Test multiple projector synchronization.
/*!
  Function tests if VBLANK and present counters have the same value between multiple projectors.

  \param parameters     Pointer to rendering thread values.
  \param vblank_counter VBLANK counter value.
  \param present_counter        Present counter value.
  \return Function returns true if all VBLANK and present counter values match.
*/
inline
bool
TestMultipleProjectorSynchronization_inline(
                                            RenderingParameters * const parameters,
                                            long int const vblank_counter,
                                            long int const present_counter
                                            )
{
  bool synchronized = true;

  int const max_i = (int)( parameters->pRenderings->size() );
  for (int i = 0; i < max_i; ++i)
    {
      RenderingParameters * const pRendering = nth_pRendering(parameters, i);
      assert( (NULL != pRendering) && (NULL != pRendering->pWindow) );
      if ( (NULL != pRendering) && (NULL != pRendering->pWindow) )
        {
          long int vblank_counter_value = -1;
          long int present_counter_value = -1;
          AcquireSRWLockShared( &(pRendering->pWindow->sLockRT) );
          {
            vblank_counter_value = pRendering->pWindow->vblank_counter;
            present_counter_value = pRendering->pWindow->present_counter;
          }
          ReleaseSRWLockShared( &(pRendering->pWindow->sLockRT) );

          bool const vblank_matches = (vblank_counter == vblank_counter_value);
          if (false == vblank_matches)
            {
              Debugfprintf(
                           stderr,
                           gDbgProjectorSynchronizationVBLANKCounterMismatch,
                           parameters->ProjectorID + 1,
                           pRendering->ProjectorID + 1,
                           vblank_counter_value,
                           vblank_counter
                           );
            }
          /* if */

          bool const present_matches = (present_counter == present_counter_value);
          if (false == present_matches)
            {
              Debugfprintf(
                           stderr,
                           gDbgProjectorSynchronizationPresentCounterMismatch,
                           parameters->ProjectorID + 1,
                           pRendering->ProjectorID + 1,
                           present_counter_value,
                           present_counter
                           );
            }
          /* if */

          synchronized = vblank_matches && present_matches;
        }
      /* if */
    }
  /* for */

  return synchronized;
}
/* TestMultipleProjectorSynchronization_inline */

#pragma endregion // Test multiple projector synchronization


#pragma region // Check duration of each event

//! Check duration of event.
/*!
  Function checks duration of each event and outputs message to the console if event takes longer than expected.

  \param event_code     Event code.
  \param event_duration_ms      Time spent processing the event.
  \param parameters     Pointer to rendering thread parameters.
  \param pWindow        Pointer to display window parameters.
*/
inline
void
CheckEventDuration_inline(
                          int const event_code,
                          double const event_duration_ms,
                          RenderingParameters * const parameters,
                          DisplayWindowParameters * const pWindow
                          )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(NULL != pWindow);
  if (NULL == pWindow) return;

  bool const fBlocking = pWindow->fBlocking; // True if acquisition is blocking.
  bool const fFixed = pWindow->fFixed; // True if fixed SL pattern is used.

  double expected_duration_ms = pWindow->refreshTime_ms;

  if ( (5 == event_code) && ( (true == fBlocking) || (true == fFixed) ) )
    {
      // Adjust expected time for DRAW_VBLANK event which executes waiting for requested delay time.
      expected_duration_ms = parameters->delay_ms;
      double offset = parameters->delay_ms * 0.1;
      if (offset < 10.0) offset = 10.0;
      expected_duration_ms += offset; // Add 10 percent of delay time or 10 ms.
    }
  /* if */

  if ( 4 == event_code )
    {
      // Adjust expected time for DRAW_PRESENT event which waits for next VBLANK.
      double offset = pWindow->refreshTime_ms * 0.1;
      if (offset < 4.0) offset = 4.0;
      expected_duration_ms += offset; // Add 10 precent of refresh or 4 ms.
    }
  /* if */

  if ( event_duration_ms > expected_duration_ms )
    {
      TCHAR const * const event_name = GetRenderingThreadEventName_inline(event_code);
      double const precentage = 100.0 * event_duration_ms / pWindow->refreshTime_ms;
      if (NULL != event_name)
        {
          Debugfwprintf(stderr, gDbgEventProcessingTooLong, parameters->ProjectorID + 1, event_name, precentage);
        }
      /* if */
    }
  /* if */
}
/* CheckEventDuration_inline */

#pragma endregion // Check duration of each event



/****** HELPER FUNCTIONS FOR DIRECT X ******/

#pragma region // Blank screen

//! Render one black screen.
/*!
  Renders completely black screen to the first back buffer.

  \param pWindow        Pointer to display window parameters.
  \param pD2DFactory    Pointer to Direct2D factory.
  \return Returns S_OK if successfull and error code otherwise.
*/
inline
HRESULT
BlankScreenRender_inline(
                         DisplayWindowParameters * const pWindow,
                         ID2D1Factory * const pD2DFactory
                         )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_POINTER;

  assert(NULL != pD2DFactory);
  if (NULL == pD2DFactory) return E_POINTER;

  HRESULT hr = S_OK;

  // Render and present one black image.
  EnterCriticalSection( &(pWindow->csRenderAndPresent) );
  {
    bool const fRenderAndPresent = pWindow->fRenderAndPresent;
    if (false == fRenderAndPresent) pWindow->fRenderAndPresent = true;
    {
      if (NULL != pWindow->pSwapChain)
        {
          if (NULL == pWindow->pRenderTarget)
            {
              hr = RecreateDirect2DRenderTarget(pWindow);
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if ( SUCCEEDED(hr) && (false == pWindow->fFreeze) )
            {
              hr = RenderBlankImage(pWindow);
              assert( SUCCEEDED(hr) );
            }
          /* if */
        }
      /* if (NULL != pWindow->pSwapChain) */
    }
    if (fRenderAndPresent != pWindow->fRenderAndPresent) pWindow->fRenderAndPresent = fRenderAndPresent;
  }
  LeaveCriticalSection( &(pWindow->csRenderAndPresent) );

  return hr;
}
/* BlankScreenRender_inline */



//! Present buffer.
/*!
  Function presents rendered buffer.

  \param pWindow        Pointer to display window parameters.
  \param QPC_before_present     Reference to QPC counter where QPC value before present will be stored.
  \param QPC_after_present      Reference to QPC counter where QPC value after present will be stored.
  \param present_immediately    Flag which controls synchronization.
  If true then the output is presented immediately; otherwise output is synchronized to VBLANK.
  \return Returns S_OK if successfull and error code otherwise.
*/
inline
HRESULT
BlankScreenPresent_inline(
                          DisplayWindowParameters * const pWindow,
                          LARGE_INTEGER & QPC_before_present,
                          LARGE_INTEGER & QPC_after_present,
                          bool const present_immediately
                          )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_POINTER;

  HRESULT hr = S_OK;

  EnterCriticalSection( &(pWindow->csRenderAndPresent) );
  {
    bool const fFreeze = pWindow->fFreeze;
    bool const fRenderAndPresent = pWindow->fRenderAndPresent;
    if (false == fRenderAndPresent) pWindow->fRenderAndPresent = true;
    {
      if (NULL != pWindow->pSwapChain)
        {
          if ( SUCCEEDED(hr) )
            {
              BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_present );
              assert(TRUE == qpc_before);

              // Present at next VBLANK (synchronized to VSYNC).
              if (false == fFreeze)
                {
                  hr = pWindow->pSwapChain->Present((true == present_immediately)? 0 : 1, 0);
                  assert( SUCCEEDED(hr) );
                }
              /* if */

              BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_present );
              assert(TRUE == qpc_after);
            }
          /* if */

          if ( SUCCEEDED(hr) )
            {
              HRESULT const get_stats = pWindow->pSwapChain->GetFrameStatistics( &(pWindow->sStatisticsPresent) );
              //assert( SUCCEEDED(get_stats) ); // Works only in full-screen mode.
            }
          /* if */
        }
      /* if (NULL != pWindow->pSwapChain) */
    }
    if (fRenderAndPresent != pWindow->fRenderAndPresent) pWindow->fRenderAndPresent = fRenderAndPresent;
  }
  LeaveCriticalSection( &(pWindow->csRenderAndPresent) );

  return hr;
}
/* BlankScreenPresent_inline */



//! Wait for VBLANK.
/*!
  Waits for next VBLANK interrupt.

  \param pWindow        Pointer to display window parameters.
  \param QPC_before_VBLANK      Reference to QPC counter where QPC value before VBLANK will be stored.
  \param QPC_after_VBLANK       Reference to QPC counter where QPC value after VBLANK will be stored.
  \return Returns S_OK if successfull and error code otherwise.
*/
inline
HRESULT
BlankScreenWaitForVBlank_inline(
                                DisplayWindowParameters * const pWindow,
                                LARGE_INTEGER & QPC_before_VBLANK,
                                LARGE_INTEGER & QPC_after_VBLANK
                                )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_POINTER;

  HRESULT hr = S_OK;

  EnterCriticalSection( &(pWindow->csWaitForVBLANK) );
  {
    assert(false == pWindow->fWaitForVBLANK);
    pWindow->fWaitForVBLANK = true;
    {
      if (NULL != pWindow->pOutput)
        {
          if ( SUCCEEDED(hr) )
            {
              BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_VBLANK );
              assert(TRUE == qpc_before);

              // Wait for VBLANK interrupt.
              hr = pWindow->pOutput->WaitForVBlank();
              assert( SUCCEEDED(hr) );

              BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_VBLANK );
              assert(TRUE == qpc_after);
            }
          /* if */
        }
      /* if (NULL != pWindow->pOutput) */
    }
    pWindow->fWaitForVBLANK = false;
  }
  LeaveCriticalSection( &(pWindow->csWaitForVBLANK) );

  return hr;
}
/* BlankScreenWaitForVBlank_inline */



//! Blanks screen.
/*!
  Paints screen black.

  \param pWindow      Pointer to display window parameters.
  \param pD2DFactory    Pointer to Direct2D factory.
  \param QPC_before_present     Reference to QPC counter where QPC value before present will be stored.
  \param QPC_after_present      Reference to QPC counter where QPC value after present will be stored.
  \param QPC_before_VBLANK      Reference to QPC counter where QPC value before VBLANK will be stored.
  \param QPC_after_VBLANK       Reference to QPC counter where QPC value after VBLANK will be stored.
  \return Returns S_OK if successfull and error code otherwise.
*/
inline
HRESULT
BlankScreen_inline(
                   DisplayWindowParameters * const pWindow,
                   ID2D1Factory * const pD2DFactory,
                   LARGE_INTEGER & QPC_before_present,
                   LARGE_INTEGER & QPC_after_present,
                   LARGE_INTEGER & QPC_before_VBLANK,
                   LARGE_INTEGER & QPC_after_VBLANK
                   )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_POINTER;

  assert(NULL != pD2DFactory);
  if (NULL == pD2DFactory) return E_POINTER;

  HRESULT hr = S_OK;

  // Render and present one black image.
  EnterCriticalSection( &(pWindow->csRenderAndPresent) );
  {
    assert(false == pWindow->fRenderAndPresent);
    pWindow->fRenderAndPresent = true;
    {
      if (NULL != pWindow->pSwapChain)
        {
          if ( SUCCEEDED(hr) )
            {
              hr = BlankScreenRender_inline(pWindow, pD2DFactory);
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if ( SUCCEEDED(hr) )
            {
              hr = BlankScreenPresent_inline(pWindow, QPC_before_present, QPC_after_present, false);
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if ( SUCCEEDED(hr) )
            {
              hr = BlankScreenWaitForVBlank_inline(pWindow, QPC_before_VBLANK, QPC_after_VBLANK);
              assert( SUCCEEDED(hr) );
            }
          /* if */
        }
      /* if */
    }
    pWindow->fRenderAndPresent = false;
  }
  LeaveCriticalSection( &(pWindow->csRenderAndPresent) );

  return hr;
}
/* BlankScreen_inline */

#pragma endregion // Blank screen



/****** RENDERING THREAD ******/

//! Rendering thread.
/*!
  Rendering thread.

  \param parameters_in Pointer to structure holding rendering thread parameters.
  \return Returns 0 if successfull.
*/
unsigned int
__stdcall
RenderingThread(
                void * parameters_in
                )
{

#pragma region // Initialization

  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  RenderingParameters * const parameters = (RenderingParameters *)parameters_in;

  SetThreadNameAndIDForMSVC(-1, "RenderingThread", parameters->ProjectorID);

  // Fetch parameters.
  SynchronizationEvents * const pSynchronization = parameters->pSynchronization;
  assert(NULL != pSynchronization);

  DisplayWindowParameters * const pWindow = parameters->pWindow;
  assert(NULL != pWindow);

  ID2D1Factory * const pD2DFactory = (NULL != pWindow)? pWindow->pD2DFactory : NULL;
  assert(NULL != pD2DFactory);

  ImageDecoderParameters * const pImageDecoder = parameters->pImageDecoder;
  assert(NULL != pImageDecoder);

  FrameStatistics * const pStatisticsRenderDuration = parameters->pStatisticsRenderDuration;
  assert(NULL != pStatisticsRenderDuration);

  FrameStatistics * const pStatisticsPresentDuration = parameters->pStatisticsPresentDuration;
  assert(NULL != pStatisticsPresentDuration);

  FrameStatistics * const pStatisticsPresentFrequency = parameters->pStatisticsPresentFrequency;
  assert(NULL != pStatisticsPresentFrequency);

  FrameStatistics * const pStatisticsWaitForVBLANKDuration = parameters->pStatisticsWaitForVBLANKDuration;
  assert(NULL != pStatisticsWaitForVBLANKDuration);

  std::vector<PresentAndTriggerTimes> * const pTriggers = parameters->pTriggers;
  assert(NULL != pTriggers);

  int ProjectorID = parameters->ProjectorID;
  assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

  int DecoderID = pImageDecoder->DecoderID;
  assert( (0 <= DecoderID) && (DecoderID < (int)(pSynchronization->ImageDecoder.size())) );
  assert( ProjectorID == pImageDecoder->ProjectorID );

  int const MainID = 0; // There is only one main thread.

  // Initialize variables.
  bool continueLoop = true;

  QueuedDecoderImage * pImage = NULL;

  ImageMetadata sImageMetadata;
  ImageMetadataBlank( &sImageMetadata );

  PastEvents * const pEvents = PastEventsCreate();

  LARGE_INTEGER QPC_before_present;
  QPC_before_present.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_after_present;
  QPC_after_present.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_before_VBLANK;
  QPC_before_VBLANK.QuadPart = (LONGLONG)0;

  LARGE_INTEGER QPC_after_VBLANK;
  QPC_after_VBLANK.QuadPart = (LONGLONG)0;

  HRESULT hr = S_OK;

  long int vblank_counter = -1; // Local copy of VBLANK counter.
  long int present_counter = -1; // Local copy of present counter.

  long int vblanks_to_present = -1; // Number of VBLANKs to next present operation.

  long int frame_counter = -1; // Frame counter.
  long int key; // Key which uniquely identifies image metadata.

  bool fFirst = true; // Flag to indicate first image in batch acquisition.
  bool fLast = false; // Flag to indicate last image in batch acquisition.

  bool fSendPresentEvent = false; // Flag to indicate thread should send DRAW_PRESENT event to itself.

  // Raise thread priority.
  BOOL const priority = SetThreadPriority(parameters->tRendering, THREAD_PRIORITY_HIGHEST);
  assert( TRUE == priority );

  parameters->fActive = true;

#pragma endregion // Initialization

  /* Events are processed in an infinite loop. */
  do
    {
      if ( (NULL != pSynchronization) &&
           (NULL != pWindow) &&
           (NULL != pD2DFactory) &&
           (NULL != pImageDecoder)
           )
        {
          assert(false == parameters->fWaiting);
          parameters->fWaiting = true;

          /* If event ordering is changed here then event processing code which uses hnr,
             static array RenderingThreadEventNames, and function GetRenderingThreadEventName_inline
             must be updated as well.
          */
          DWORD const dwWaitResult =
            pSynchronization->EventWaitForAny(
                                              DRAW_TERMINATE,       ProjectorID, // 0
                                              MAIN_PREPARE_DRAW,    ProjectorID, // 1
                                              MAIN_BEGIN,           ProjectorID, // 2
                                              DRAW_RENDER,          ProjectorID, // 3
                                              DRAW_PRESENT,         ProjectorID, // 4
                                              DRAW_VBLANK,          ProjectorID, // 5
                                              CAMERA_SYNC_TRIGGERS, ProjectorID, // 6
                                              DRAW_CHANGE_ID,       ProjectorID, // 7
                                              INFINITE // Wait forever.
                                              );
          int const hnr_received = dwWaitResult - WAIT_OBJECT_0;
          assert( (0 <= hnr_received) && (hnr_received < 8) );
          int hnr = hnr_received;
          AddEvent(pEvents, hnr);

          parameters->fWaiting = false;

          /* DESCRIPTION OF THE RENDERING THREAD EVENT PROCESSING LOGIC

             The rendering thread processes an event immediately after it is signalled.
             Events always occur in a cycle (loop) so there is no need for event processing queue.
             Immediate processing of signalled event is (almost always) ensured by the thread
             priority which is set to THREAD_PRIORITY_HIGHEST.
             If no events are signalled then thread is idle and does not consume processor time.

             Rendering thread processes the following events:
             1) DRAW_TERMINATE - the rendering thread should terminate,
             2) MAIN_PREPARE_DRAW - the rendering thread should stop current actions and prepare for batch acquisition,
             3) MAIN_BEGIN - the rendering thread should start the batch acquisition,
             4) DRAW_RENDER - next SL pattern must be rendered,
             5) DRAW_PRESENT - previously rendered SL pattern must be presented,
             6) DRAW_VBLANK - execute waiting for projector delay to elapse or for the next VBLANK interrupt to occur,
             7) CAMERA_SYNC_TRIGGERS - synchronize all slaved camera triggers, and
             8) DRAW_CHANGE_ID - changes event IDs.

             The order in which events appear depends on the configured acquisition mode.
             There are several flags which control the acquisition mode and the type of
             the SL pattern which is used. These flags are:
             1) fBlocking - indicates if acquisition is blocking or non-blocking,
             2) fFixed - indicates if one image SL pattern is used,
             3) fConcurrentDelay - indicates if delay time is larger or shorter than camera exposure,
             4) fSynchronize - indicates that multiple projectors must be synchronized.

             Every acquisition mode has its cycle of events which is defined by aforementioned
             flags and which is executed by combined action of rendering and acquisition threads.
             Here we describe the logic of the rendering thread; for a description of acquisition
             thread logic see comments in the file BatchAcquisitionAcquisition.cpp which implements
             the acquisition thread.


             BLOCKING ACQUISITION MODE

             The blocking acquisition mode uses a causal sequence of events which requires that all previous
             operations are successfully completed before the next operation is executed. Due to such
             hard constraint any delay in program execution simply extends the run time; no frames will be dropped.

             Blocking acquisition mode is indicated by the true value of fBlocking flag.

             There are two cycles of events in the blocking acquisition mode which depend
             on the value of the delay time and of the camera exposure time; this relationship is
             indicated by the fConcurrentDelay flag which is set to true if delay time is larger
             than camera exposure time.

             If the delay time is larger than the camera exposure time (fConcurrentDelay is true)
             then the causal event loop is
             ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->...

             If the delay time is shorter than the camera exposure time (fConcurrentDelay is false)
             then the causal event loop is
             ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->...
             Here the DRAW_RENDER event is fired immediately after CAMERA_SEND_TRIGGER is successfully completed.
             To maintain causality of the cycle CAMERA_TRANSFER_END event will always wait on DRAW_RENDER to complete before
             signalling DRAW_PRESENT event; this is realized by waiting on DRAW_RENDER_READY event.


             NON-BLOCKING ACQUISITION MODE

             In non-blocking acquisition mode there is no loop between rendering and acquisition threads;
             instead the rendering thread operates independently and tries to trigger the camera when necessary.
             The camera trigger operation may fail if the camera is busy or if there is some unexpected
             delay in the program execution, therefore frames may be dropped.

             IMPORTANT: This acquisition mode has no guarantee that all frames will be acquired.

             Non-blocking acquisition mode is indicated by the false value of fBlocking flag.
             Flag fConcurrentDelay has no effect in non-blocking acquisition mode.

             In non-blocking acquisition mode there exists one cycle of events for the rendring thread only:
             ...->DRAW_PRESENT->DRAW_RENDER->(DRAW_VBLANK)->...
             where DRAW_VBLANK event is repeated a predefined number of times.

             In this acquisition mode after each event of the rendering thread is processed the code for
             processing CAMERA_SYNC_TRIGGERS events is run. The code segment for processing CAMERA_SYNC_TRIGGERS
             tests the time remaining to the next trigger and fires CAMERA_SEND_TRIGGER events as needed.
             Note that there is no testing if the trigger succedded or if the frame was successfully acquired.


             FIXED SL PATTERN

             Fixed SL pattern uses only one image which may then be recored as many times as requested.

             When a fixed SL pattern is used it is sufficient to render the SL pattern once; then
             the camera may be triggered as fast as possible as there is no need for synchronization.

             When fixed SL pattern is used the acquisition always start using the event sequence
             MAIN_PREPARE_DRAW->MAIN_BEGIN->DRAW_RENDER->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->...
             afer which the rendering thread has nothing to render.

             After cameras are triggered for the first time event cycles used for fixed SL pattern differ
             depending on the value of the fBlocking flag. In blocking acquisition mode the cameras
             will be triggered after the image is transfered to the PC while in non-blocking mode
             cameras will be triggered immediately after exposure completes.

             For blocking acquisition the event cycle is
             ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->CAMERA_SYNC_TRIGGERS->...

             For non-blocking acquisition the event cycle is
             ...->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_SYNC_TRIGGERS->...

             Therefore, for the rendering thread only event which is processed is CAMERA_SYNC_TRIGGERS.


             RENDERING ONLY MODE

             If there are no cameras attached the thread runs in rendering-only mode.
             This mode is comprised of the simplest possible event cycle DRAW_RENDER->DRAW_PRESENT
             which is repeated indefinitely.


             SYNCHRONIZATION BETWEEN MULTIPLE PROJECTORS

             When multiple projectors are used there exist two acquisition mode, sequential and simultaneous.

             In sequential acquisition mode each projector projects its own set of images which and
             acquisition is performed only on the cameras attached to the particular projector.

             In simultaneous acquisition mode all projectors simultaneously project images which are then
             acquired by all cameras. Therefore simultaneous acquisition mode requires that all devices
             are synchronized. This may be achieved by synchronizing projectors.

             Code to synchronize projectors is controlled by the fSynchronize flag.
             If the flag value is true then projector synchronization code is executed.

             Projectors are synchronized via three conditional synchronization events which behave like
             semaphores: DRAW_SYNC_PRESENT, DRAW_SYNC_VBLANK, and DRAW_SYNC_TRIGGERS.
             Each of these event is conditionally raised by all projector threads until the event counter
             reaches the number of projectors in the system. Event is then signalled which enables all
             threads to simultaneously continue with the execution of the appropriate task.

             Note that tasks which are synchronized depend on the acquisition mode.

             For blocking acquisition and for a fixed SL pattern all three events are used as follows:
             a) DRAW_SYNC_PRESENT is used to synchronize the start of the present operation;
             b) DRAW_SYNC_VBLANK is used to synchronize the end of the wait between the present and
             the camera trigger; and
             c) DRAW_SYNC_TRIGGERS is used to synchronize the camera triggering.

             For non-blocking acquisition mode cameras are triggered asynchronously according to
             the schedule, hence the DRAW_SYNC_TRIGGERS event cannot be used to synchronize the
             triggers. Other two events are used to synchronize the presentation as follows:
             a) DRAW_SYNC_PRESENT is used to synchronize the start of the present operation; and
             b) DRAW_SYNC_VBLANK is used to synchronize the start of the wait for VBLANK operation.
             Therefore, in non-blocking mode projectors are synchronized but cameras are not synchronized.


             STARTING AND STOPPING THE CYCLE

             All listed event cycles do not include the start-up and stopping sequence.
             During start-up sequence first frame must be rendered and output;
             after the first frame is rendered one of event cycles may be run indefinitely.

             To render the first frame the DRAW_RENDER event must be processed first.
             It is followed immedialtely by the DRAW_PRESENT event.
             When executed in sequence they output the first frame of the SL pattern.

             Note that not all event cycles have DRAW_RENDER->DRAW_PRESENT combination as building block
             therefore to implemented the start-up sequence we need additional flags to control the
             behaviour of event processing code during start-up sequence.
             These include flags fFirst and fLast are used which indicate the first and the last
             frame in a SL sequence are projected and fSendPresentEvent which
             may be set to true to force that DRAW_RENDER always raises DRAW_PRESENT event.

             To prepare for acquisition there exist two additional events, MAIN_PREPARE_DRAW and
             MAIN_BEGIN. Event MAIN_PREPARE_DRAW may be raised at any time. It effectively resets
             the state of the rendering thread to initial state. Event MAIN_BEGIN should be issued
             only after both rendering and acquisition threads indicate they are ready to start the
             acquisition (see BatchAcquisitionAcquisition.cpp for details regarding the acquistion
             thread).

             Overall, to start the batch acquisition the sequence of events for the rendering thread is as follows:
             MAIN_PREPARE_DRAW->MAIN_BEGIN->DRAW_RENDER->DRAW_PRESENT
             This start-up sequence is then followed by one of cycles listed above until all
             frames of the SL pattern are projected.
             Note that MAIN_BEGIN event must be issued only after the MAIN_READY_DRAW event is signalled.

             To signal the end of the batch acquistion started via MAIN_BEGIN two events are used,
             MAIN_END_DRAW and MAIN_END_CAMERA. The main thread needs only to wait for MAIN_END_DRAW
             as the rendering thread will wait on MAIN_END_CAMERA events before signalling MAIN_END_DRAW.
             For a more detailed description see comment in BatchAcquisitionAcquisition.cpp.


             ADDITIONAL NOTES ABOUT DIRECT X

             1) Rendering

             This tread renders SL patters which are displayed in an associated DirectX display window
             for which the message pump is run on a different thread.

             DirectX objects such as DXGI swap chain and DirectX output device are therefore used
             concurrently from at least two different threads which must synchronize their access.
             Synchronization is achieved by using a critical section kernel objects, specifically,
             pWindow->csRenderAndPresent and pWindow->csWaitForVBLANK.

             To avoid deadlocking some events which change DXGI swap chain which are processed by
             the message pump thread must be dropped/postponed if the rendering thread is executing
             the Present method of the DXGI swap chain. Critical message is WM_SIZE which causes
             the DXGI swap chain to resize; the resizing is deferred until rendering thread completes
             the Present.

             2) Presenting

             The Present method of DXGI SwapChain class operates in a non-blocking mode, i.e. even when
             we request that the presentation is synchronized to the next VBLANK the Present method will
             prepare everything and will return control immediately after all commands are properly queued
             to the GPU. Specifically, the moment the Present method returns is in no relation to the
             time VBLANK occures for a specific video display.

             Therefore, although Present method ensures vertical synchronization it does not provide the exact time
             at which the next VBLANK interrupt will occur. Two methods may be used to synchronize the program
             execution to the VBLANK interrupt:
             (a) WaitForVBlank method which blocks the execution until VBLANK occures, and
             (b) GetFrameStatistics method which returns DirectX timing information which includes precise QPC time at which the VBLANK interrupt occured.

             The method (a) is preferred method as if always works.
             The method (b) works only if the DXGI swap chain is operated in full-screen mode or if the flip presentation model
             (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) is used. If we are using windowed display mode without frame
             flipping then frame statistics is not available. Furthermore, frame statistics returned by the
             GetFrameStatistcs method may not be the most recent one and may exhibit lag up to several
             VBLANKs as indicated by SyncQPCTime field which provides precise QPC time at which the statistics was last updated.
          */

#pragma region // Get acquisition state

          int const num_cam = (int)( parameters->pAcquisitions->size() );
          assert(0 <= num_cam);

          bool const fSynchronize = parameters->fSynchronize; // True if multiple projectors must be synchronized.
          int const num_prj = parameters->num_prj;

          bool const fBlocking = pWindow->fBlocking; // True if acquisition is blocking.
          bool const fFixed = pWindow->fFixed; // True if fixed SL pattern is used.
          bool const fConcurrentDelay = pWindow->fConcurrentDelay; // True if delay is larger than exposure.

          UINT const SyncInterval = parameters->SyncInterval;

#pragma endregion // Get acquisition state

          if (0 == hnr)
            {
              // We received terminate event.
              continueLoop = false;
            }
          else if (1 == hnr)
            {
              /****** PREPARE FOR BATCH ACQUISITION ******/

              /* The preparation for batch acquisition is the same for all acquisition modes.

                 To prepare the acquisition threads we raise corresponding MAIN_PREPARE_CAMERA events
                 and wait for MAIN_READY_CAMERA to be armed.

                 After the preparation step is completed we signal the readiness to start
                 the batch acquistion by raising MAIN_READY_DRAW event. To start the acquisition
                 MAIN_BEGIN event must be signalled, however note that this event may be signalled
                 only after acquisition thread has raised MAIN_READY_CAMERA event.
              */

#pragma region // Process MAIN_PREPARE_DRAW event

              // Disarm MAIN_PREPARE_DRAW, DRAW_RENDER_READY, and DRAW_PRESENT_READY events.
              {
                assert( false == DebugIsSignalled(pSynchronization, MAIN_BEGIN, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, MAIN_READY_DRAW, ProjectorID) );

                BOOL const reset_prepare_draw = pSynchronization->EventReset(MAIN_PREPARE_DRAW, ProjectorID);
                assert(0 != reset_prepare_draw);


                BOOL const reset_render_ready = pSynchronization->EventReset(DRAW_RENDER_READY, ProjectorID);
                assert(0 != reset_render_ready);

                BOOL const reset_present_ready = pSynchronization->EventReset(DRAW_PRESENT_READY, ProjectorID);
                assert(0 != reset_present_ready);
              }


#pragma region // Signal to attached cameras to prepare

              // Disarm all MAIN_*_CAMERA and CAMERA_SEND_TRIGGER events; arm MAIN_PREPARE_CAMERA events.
              for (int i = 0; i < num_cam; ++i)
                {
                  int const CameraID = nth_ID(parameters, i);
                  assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                  assert( false == DebugIsSignalled(pSynchronization, MAIN_READY_CAMERA, CameraID) );

                  BOOL const reset_camera = pSynchronization->EventResetAllMain(-1, -1, CameraID);
                  assert(0 != reset_camera);

                  BOOL const reset_trigger = pSynchronization->EventReset(CAMERA_SEND_TRIGGER, CameraID);
                  assert(0 != reset_trigger);

                  BOOL const prepare_camera = pSynchronization->EventSet(MAIN_PREPARE_CAMERA, CameraID);
                  assert(0 != prepare_camera);
                }
              /* for */

#pragma endregion // Signal to attached cameras to prepare


#pragma region // Empty all queues

              // Empty all queues associated with this thread.
              {
                /* We have to empty several queues:

                   1) decoder queue which supplies images to the rendering thread,
                   2) trigger queue which contains triggerin information, and
                   3) image metadata queues.

                   1) IMAGE DEOCDER QUEUE

                   Image decoder queue is filled by the image decoder thread.
                   To empty the queue we must first prohibit the cycling through the
                   associated image list and rewind this list to the end.
                   Then we simply consume all queued images withouth actually processing them.
                   The queue will be re-filled by the main control thread once it receives
                   both the MAIN_READY_DRAW and MAIN_READY_CAMERA events.

                   2) TRIGGER QUEUE

                   Trigger queue is used to store trigger information for non-blocking acquisition.

                   3) IMAGE METADATA QUEUES

                   Image metadata queues are associated with acquisition threads. Depedning on
                   the acquisition mode they are filled either by the rendering thread or by
                   the acquisition thread. Due to this image metadata queues are emptied twice,
                   once by the rendering thread and once by the acquisition thread.

                */

                // Rewind file list to the end.
                if (NULL != pImageDecoder->pImageList)
                  {
                    bool const stop = pImageDecoder->pImageList->ToEndAndStopCycling();
                    assert(true == stop);
                  }
                /* if */

                // Wait for decoder thread to stop processing.
                {
                  bool busy = false;
                  DWORD dwIsBusyResult = WAIT_FAILED;
                  do
                    {
                      if (true == busy) SleepEx(1, TRUE);
                      dwIsBusyResult = pSynchronization->EventWaitFor(IMAGE_DECODER_QUEUE_PROCESS, DecoderID, (DWORD)0);
                      busy = (WAIT_OBJECT_0 == dwIsBusyResult);
                    }
                  while (true == busy);
                  assert(WAIT_TIMEOUT == dwIsBusyResult);
                }

                // Consume all queued images.
                while (true == ImageDecoderHaveNext(pImageDecoder))
                  {
                    QueuedDecoderImage * pNextImage = ImageDecoderFetchImage(pImageDecoder, true);
                    SAFE_DELETE(pNextImage);
                  }
                /* while */

                // Empty trigger times queue.
                EmptyTriggerQueue_inline(pTriggers);

                // Empty image metadata queues.
                for (int i = 0; i < num_cam; ++i)
                  {
                    AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                    assert(NULL != pAcquisition);

                    ImageMetadataQueue * const pMetadataQueue = pAcquisition->pMetadataQueue;
                    assert(NULL != pMetadataQueue);

                    EmptyImageMetadataQueue(pMetadataQueue);
                  }
                /* for */

                assert( true == ImageDecoderAllFilesQueued(pImageDecoder) );
                assert( 0 == ImageDecoderNumOfQueuedItems(pImageDecoder) );
              }

#pragma endregion // Empty all queues


              // Reset flags.
              fFirst = false;
              fLast = false;

              // Reset statistics.
              FrameStatisticsReset(pStatisticsRenderDuration);
              FrameStatisticsReset(pStatisticsPresentDuration);
              FrameStatisticsReset(pStatisticsPresentFrequency);
              FrameStatisticsReset(pStatisticsWaitForVBLANKDuration);


#pragma region // Wait for attached cameras

              // Wait for attached cameras to complete preparation.
              for (int i = 0; i < num_cam; ++i)
                {
                  int const CameraID = nth_ID(parameters, i);
                  assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                  DWORD const wait_camera = pSynchronization->EventWaitFor(MAIN_READY_CAMERA, CameraID, INFINITE);
                  int const idx_camera = wait_camera - WAIT_OBJECT_0;
                  assert(0 == idx_camera);
                  if (0 == idx_camera)
                    {
                      BOOL const reset_ready = pSynchronization->EventReset(MAIN_READY_CAMERA, CameraID);
                      assert(0 != reset_ready);
                    }
                  /* if */

                  BOOL const reset_camera = pSynchronization->EventResetAllCameraExceptTriggerReady(CameraID);
                  assert(0 != reset_camera);
                }
              /* for */

#pragma endregion // Wait for attached cameras


              // Reset all draw events including CAMERA_SYNC_TRIGGERS.
              {
                BOOL const reset_sync_trigger = pSynchronization->EventReset(CAMERA_SYNC_TRIGGERS, ProjectorID);
                assert(0 != reset_sync_trigger);

                BOOL const reset_draw = pSynchronization->EventResetAllDraw(ProjectorID);
                assert(0 != reset_draw);
              }

              // Set event counters.
              {
                BOOL const set_counter_sync_triggers = pSynchronization->SetStartCounterValue(CAMERA_SYNC_TRIGGERS, ProjectorID, num_cam, true);
                assert(0 != set_counter_sync_triggers);

                BOOL const set_counter_render = pSynchronization->SetStartCounterValue(DRAW_RENDER, ProjectorID, num_cam, true);
                assert(0 != set_counter_render);

                BOOL const set_counter_present = pSynchronization->SetStartCounterValue(DRAW_PRESENT, ProjectorID, num_cam, true);
                assert(0 != set_counter_present);
              }

#ifdef _DEBUG
              // Check status of camera events.
              for (int i = 0; i < num_cam; ++i)
                {
                  int const CameraID = nth_ID(parameters, i);
                  assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                }
              /* for */
#endif /* _DEBUG */


#pragma region // Refill input queue

              // Refill input queue.
              {
#ifdef _DEBUG
                // Image file list must be at the end.
                {
                  bool const have_next = ImageDecoderHaveNext(pImageDecoder);
                  bool const all_queued = ImageDecoderAllFilesQueued(pImageDecoder);
                  assert( (true == all_queued) && (false == have_next) );
                  assert( 0 == ImageDecoderNumOfQueuedItems(pImageDecoder) );
                }
#endif /* _DEBUG */

                // Wait for decoder thread to stop processing.
                {
                  bool busy = false;
                  DWORD dwIsBusyResult = WAIT_FAILED;
                  do
                    {
                      if (true == busy) SleepEx(1, TRUE);
                      dwIsBusyResult = pSynchronization->EventWaitFor(IMAGE_DECODER_QUEUE_PROCESS, DecoderID, (DWORD)0);
                      busy = (WAIT_OBJECT_0 == dwIsBusyResult);
                    }
                  while (true == busy);
                  assert(WAIT_TIMEOUT == dwIsBusyResult);
                }

                // Rewind file list to begining.
                if (NULL != pImageDecoder->pImageList)
                  {
                    /* Here we directly rewind the file list to its start.
                       Such operation is allowed only if the decoder thread is idling which
                       is indicated by disarmed IMAGE_DECODER_QUEUE_PROCESS signal.
                       We have waited for the decoder thread to stop processing
                       before rewinding the list so we know that the thread is idling.
                    */
                    assert( true == DebugIsSignalled(pSynchronization, IMAGE_DECODER_QUEUE_EMPTY, DecoderID) );
                    assert( false == DebugIsSignalled(pSynchronization, IMAGE_DECODER_QUEUE_FULL, DecoderID) );
                    assert( false == DebugIsSignalled(pSynchronization, IMAGE_DECODER_QUEUE_PROCESS, DecoderID) );

                    assert(false == pImageDecoder->pImageList->cycle);
                    bool const rewind = pImageDecoder->pImageList->Rewind();
                    assert(true == rewind);
                  }
                /* if */

                // Start processing.
                BOOL const set_process = pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, DecoderID);
                assert(0 != set_process);

                // Wait for processing to end.
                DWORD const dwIsDoneResult = pSynchronization->EventWaitFor(IMAGE_DECODER_QUEUE_FULL, DecoderID, INFINITE);
                assert( WAIT_OBJECT_0 == dwIsDoneResult );
              }

#pragma endregion // Refill input queue

              // Set projector ID for memory buffers.
              {
                std::wstring * ProjectorUID = GetUniqueProjectorIdentifier(parameters);
                for (int i = 0; i < num_cam; ++i)
                  {
                    AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                    assert(NULL != pAcquisition);
                    if (NULL == pAcquisition) continue;

                    ImageEncoderParameters * const pImageEncoder = pAcquisition->pImageEncoder;
                    assert(NULL != pImageEncoder);
                    if (NULL == pImageEncoder) continue;

                    assert(NULL != pImageEncoder->pAllImages);
                    if (NULL == pImageEncoder->pAllImages) continue;

                    pImageEncoder->pAllImages->SetProjector(ProjectorID, ProjectorUID);
                  }
                /* for */
                SAFE_DELETE(ProjectorUID);
              }

              // Render and present one black image.
              {
                HRESULT const hr_blank =
                  BlankScreen_inline(
                                     pWindow, pD2DFactory,
                                     QPC_before_present, QPC_after_present,
                                     QPC_before_VBLANK, QPC_after_VBLANK
                                     );
                assert( SUCCEEDED(hr_blank) );
              }

              // Reset frame counters.
              {
                present_counter = -1;
                vblank_counter = -1;

                AcquireSRWLockExclusive( &(pWindow->sLockRT) );
                {
                  pWindow->present_counter = present_counter;
                  pWindow->vblank_counter = vblank_counter;
                }
                ReleaseSRWLockExclusive( &(pWindow->sLockRT) );
              }

              // Singal to the main thread that we are prepared for the batch acquisition.
              {
                assert( false == DebugIsSignalled(pSynchronization, MAIN_READY_DRAW, ProjectorID) );

                BOOL const set_ready = pSynchronization->EventSet(MAIN_READY_DRAW, ProjectorID);
                assert(0 != set_ready);
              }

#pragma endregion // Process MAIN_PREPARE_DRAW event

            }
          else if (2 == hnr)
            {
              /****** START BATCH ACQUISITION ******/

              /* Re-initialize state variables depending on the requested acqusition mode and then
                 start the acquisition cycle.

                 Before starting the cycle one black image is always rendered and presented;
                 this ensures the DXGI SwapChain is properly reset so next render operation
                 in DRAW_RENDER events renders to the front of the swap chain.
              */

#pragma region // Process MAIN_BEGIN event

              // Disarm MAIN_BEGIN event.
              {
                assert( false == DebugIsSignalled(pSynchronization, MAIN_PREPARE_DRAW, ProjectorID) );

                BOOL const reset_begin = pSynchronization->EventReset(MAIN_BEGIN, ProjectorID);
                assert(0 != reset_begin);
              }

              // At least one camera must be attached.
              assert(0 < num_cam);

#ifdef _DEBUG
              // Check state of camera events.
              {
                int hnr_prev = -1;
                bool const getprev = GetPreviousEvent(pEvents, &hnr_prev, NULL, NULL, NULL);
                assert(true == getprev);
                assert(1 == hnr_prev); // Previous event must be MAIN_PREPARE_DRAW event.

                for (int i = 0; i < num_cam; ++i)
                  {
                    int const CameraID = nth_ID(parameters, 0);
                    assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                    assert( false == DebugIsSignalled(pSynchronization, MAIN_PREPARE_CAMERA, CameraID) );
                    assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );
                    assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                  }
                /* for */
              }
#endif /* _DEBUG */


              // Render and present one black image.
              {
                HRESULT const hr_blank =
                  BlankScreen_inline(
                                     pWindow, pD2DFactory,
                                     QPC_before_present, QPC_after_present,
                                     QPC_before_VBLANK, QPC_after_VBLANK
                                     );
                assert( SUCCEEDED(hr_blank) );
              }

              // Clear image metadata.
              ImageMetadataRelease( &sImageMetadata );

              // Indicate next frame is the first frame.
              fFirst = true;

              // Clear last frame flag.
              fLast = false;

              // Set frame, present and VBLANK counters.
              {
                /* Present counter is used to assign unique keys to image metadata so acquisition
                   thread is able to fetch the correct metadata from the image queue.
                   Additionally, in non-blocking acquisition mode present and VBLANK counters are used
                   to track when the next frame must be displayed. Note that counters are not used
                   to track when the camera trigger is scheduled as they are not reliable measurement
                   of absolute elapsed time; QPC counters are used to track when camera must be triggered.

                   The start-up sequence of events is MAIN_PREPARE_DRAW->MAIN_BEGIN->DRAW_RENDER->DRAW_PRESENT
                   after which an event cycle depends on the acquisition mode. Here we also set appropriate
                   flags which ensure this starting event sequence:

                   a) For blocking acquisition both fSendPresentEvent and fFirst flags must be set to true.

                   b) For non-blocking acquisition mode fSendPresentEvent flag must be set to false and
                   vblanks_to_present counter must be set to 0.
                */
                frame_counter = -1;
                key = -1;

                if (true == fBlocking)
                  {
                    vblanks_to_present = -1; // Set to invalid value; counter is not used.
                    fSendPresentEvent = true; // Indicate the thread should self-signal DRAW_PRESENT once.
                    assert(true == fFirst); // Normal event dispatch must be turned off.
                  }
                else
                  {
                    vblanks_to_present = 0; // Present first frame immediately.
                    fSendPresentEvent = false; // Do not raise DRAW_PRESENT twice.
                  }
                /* if */

                assert(-1 == present_counter);
                assert(-1 == vblank_counter);

                AcquireSRWLockExclusive( &(pWindow->sLockRT) );
                {
                  assert(pWindow->present_counter == present_counter);
                  assert(pWindow->vblank_counter == vblank_counter);
                }
                ReleaseSRWLockExclusive( &(pWindow->sLockRT) );
              }

              // Set event counters.
              {
                BOOL const set_counter_render = pSynchronization->SetStartCounterValue(DRAW_RENDER, ProjectorID, num_cam, true);
                assert(0 != set_counter_render);

                BOOL const set_counter_present = pSynchronization->SetStartCounterValue(DRAW_PRESENT, ProjectorID, num_cam, true);
                assert(0 != set_counter_present);
              }

              if (true == fFixed)
                {
                  BOOL const set_counter_sync_trigger = pSynchronization->SetStartCounterValue(CAMERA_SYNC_TRIGGERS, ProjectorID, num_cam, true);
                  assert(0 != set_counter_sync_trigger);
                }
              /* if */

              // Arm all *_READY events.
              {
                assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );

                BOOL const set_render_ready = pSynchronization->EventSet(DRAW_RENDER_READY, ProjectorID);
                assert(0 != set_render_ready);

                for (int i = 0; i < num_cam; ++i)
                  {
                    int const CameraID = nth_ID(parameters, i);
                    assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                    assert( false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );

                    BOOL const set_camera_ready = pSynchronization->EventSet(CAMERA_READY, CameraID);
                    assert(0 != set_camera_ready);
                  }
                /* for */
              }

              // Start batch acquisition by raising DRAW_RENDER_READY event.
              {
                assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                assert(0 != set_render);
              }

#pragma endregion // Process MAIN_BEGIN event

            }
          else if (3 == hnr)
            {
              /****** RENDER NEXT BUFFER ******/

              /* The DRAW_RENDER event is fired when next frame in SL sequenece must be presented.
                 The code for this event will pre-render the SL frame to the DirectX swap chain
                 so it is ready for presenting.

                 There also exists a DRAW_RENDER_READY event which signals the state of the rendering thread:
                 it is in non-signalled state from the moment the DRAW_RENDER event is processed and is
                 reset only after the rendered frame is actually presented in DRAW_PRESENT event.
                 Note that in normal operation DRAW_RENDER event should be raised only if DRAW_RENDER_READY
                 event is in signalled state; this may be done by waiting on DRAW_RENDER_READY event.
              */

#pragma region // Process DRAW_RENDER event

              // Start timer for DRAW_RENDER event.
              FrameStatisticsTic(pStatisticsRenderDuration);

              // Disarm DRAW_RENDER_READY and reset DRAW_RENDER events.
              {
                assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                BOOL const reset_render_ready = pSynchronization->EventReset(DRAW_RENDER_READY, ProjectorID);
                assert(0 != reset_render_ready);

                BOOL const reset_render = pSynchronization->EventResetAndSetCounterSet(DRAW_RENDER, ProjectorID, num_cam);
                assert(0 != reset_render);
              }

              // Reset image metadata; updated image metadata will carry over to the next DRAW_PRESENT event.
              ImageMetadataRelease( &sImageMetadata );

              // Reset rendering status; updated rendering status will carry over to the next DRAW_PRESENT event.
              hr = S_OK;


#pragma region // Render frame

              // Render the next frame.
              if (false == pWindow->fModeChange)
                {
                  bool copy_metadata = false;

                  if (NULL != pWindow->pSwapChain)
                    {
                      // Fetch image if there is none.
                      if (NULL == pImage) pImage = ImageDecoderFetchImage(pImageDecoder, true);
                    }
                  /* if */

                  EnterCriticalSection( &(pWindow->csRenderAndPresent) );
                  {
                    assert(false == pWindow->fRenderAndPresent);
                    pWindow->fRenderAndPresent = true;
                    {
                      if (NULL != pWindow->pSwapChain)
                        {
                          if (NULL == pWindow->pRenderTarget)
                            {
                              hr = RecreateDirect2DRenderTarget(pWindow);
                              assert( SUCCEEDED(hr) );
                            }
                          /* if */

                          // Render image if we have one; otherwise blank screen except if a fixed SL pattern is used.
                          if (NULL != pImage)
                            {
                              hr = RenderQueuedImage(pWindow, pImage);
                              assert( SUCCEEDED(hr) );

                              if ( SUCCEEDED(hr) )
                                {
                                  assert(false == copy_metadata);
                                  copy_metadata = true; // Indicate metadata should be copied.
                                  assert(NULL != pImage);
                                }
                              else
                                {
                                  Debugfprintf(stderr, gDbgFrameRenderFailed, ProjectorID + 1, frame_counter + 2);
                                }
                              /* if */
                            }
                          else // !(NULL != pImage)
                            {
                              if ( (false == fFixed) && (false == pWindow->fFreeze) )
                                {
                                  hr = RenderBlankImage(pWindow);
                                  assert( SUCCEEDED(hr) );
                                }
                              /* if */

                              assert(NULL == sImageMetadata.pFilename);
                              assert(false == sImageMetadata.fBatch);
                            }
                          /* if (NULL != pImage) */
                        }
                      /* if (NULL != pWindow->pSwapChain) */
                    }
                    pWindow->fRenderAndPresent = false;
                  }
                  LeaveCriticalSection( &(pWindow->csRenderAndPresent) );

                  if (true == copy_metadata)
                    {
                      assert(NULL != pImage);

                      sImageMetadata.no = pImage->no;
                      sImageMetadata.render_type = pImage->render_type;
                      sImageMetadata.pattern_type = pImage->pattern_type;

                      assert(-1 == sImageMetadata.key);
                      assert(-1 == sImageMetadata.present_counter);
                      assert(-1 == sImageMetadata.vblank_counter);

                      assert(-1 == sImageMetadata.QPC_current_presented);
                      assert(-1 == sImageMetadata.QPC_trigger_scheduled_RT);
                      assert(-1 == sImageMetadata.QPC_trigger_scheduled_AT);
                      assert(-1 == sImageMetadata.QPC_next_scheduled);
                      assert(-1 == sImageMetadata.QPC_next_presented);
                      assert(-1 == sImageMetadata.QPC_before_trigger);
                      assert(-1 == sImageMetadata.QPC_after_trigger);

                      assert(NULL == sImageMetadata.pFilename);
                      sImageMetadata.pFilename = pImage->pFilename;
                      pImage->pFilename = NULL;

                      sImageMetadata.red = pImage->red;
                      sImageMetadata.green = pImage->green;
                      sImageMetadata.blue = pImage->blue;
                      sImageMetadata.alpha = pImage->alpha;

                      sImageMetadata.delay = pImage->delay;
                      sImageMetadata.exposure = pImage->exposure;

                      sImageMetadata.index = pImage->index;
                      sImageMetadata.retry = pImage->retry;

                      assert(ProjectorID == pImage->projectorID);
                      sImageMetadata.ProjectorID = pImage->projectorID;
                      assert(-1 == sImageMetadata.CameraID);

                      sImageMetadata.fBatch = parameters->fBatch;
                      sImageMetadata.fBlocking = fBlocking;
                      sImageMetadata.fFixed = fFixed;
                      sImageMetadata.fSavePNG = parameters->fSavePNG;
                      sImageMetadata.fSaveRAW = parameters->fSaveRAW;
                      sImageMetadata.fLast = (false == fFixed)? !ImageDecoderHaveNext(pImageDecoder) : false;
                      assert(false == sImageMetadata.fTrigger);
                      sImageMetadata.fSkipAcquisition = pImage->fSkipAcquisition;
                    }
                  /* if */

#ifdef _DEBUG
                  /* In batch acquisition mode image fetch should not fail. */
                  if ( (true == parameters->fBatch) && (false == fLast) )
                    {
                      assert(true == copy_metadata);
                      assert(NULL != pImage);
                    }
                  /* if */
#endif /* _DEBUG */

                  // Store rendered image.
                  {
                    QueuedDecoderImage * pImageWindow = NULL;
                    AcquireSRWLockExclusive( &(pWindow->sLockImage) );
                    {
                      pImageWindow = pWindow->pImage;
                      pWindow->pImage = pImage;
                    }
                    ReleaseSRWLockExclusive( &(pWindow->sLockImage) );
                    pImage = NULL;
                    SAFE_DELETE(pImageWindow);
                  }
                }
              /* if (false == pWindow->fModeChange) */

#pragma endregion // Render frame


#pragma region // Event dispatch

              // Signal appropriate event depedning on the acquisition mode.
              if (0 < num_cam)
                {
                  if (true == fBlocking)
                    {
                      if (false == fFixed)
                        {
                          if (false == fFirst)
                            {
                              /* For blocking acquisition mode the first frame is indicated by fFirst flag and is handled
                                 differently as DRAW_RENDER event must be raised. Therefore for blocking acquisition
                                 the event dispatching is disabled when fFirst flag is set.

                                 For all subsequent frames we either proceed to wait for required delay by raising
                                 DRAW_VBLANK or we do nothing as the DRAW_RENDER event was called from the acquisition
                                 thread.
                              */
                              if (true == fConcurrentDelay)
                                {
                                  /* The event cycle is:
                                     ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->...

                                     Therefore, raise DRAW_VBLANK event which will wait for the required delay to elapse.
                                  */
                                  assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );
                                  assert( false == fSendPresentEvent );

                                  BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                                  assert(0 != set_vblank);
                                }
                              else // !(true == fConcurrentDelay)
                                {
                                  /* The event cycle is:
                                     ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->(CAMERA_REPEAT_TRIGGER)->CAMERA_EXPOSURE_END->CAMERA_TRANSFER_END->...
                                     where the DRAW_RENDER is fired immediately after CAMERA_SEND_TRIGGER is successfully completed.

                                     We have nothing to do here. Acquisition thread will fire DRAW_PRESENT after
                                     this event arms DRAW_PRESENT_READY signal.
                                  */
                                }
                              /* if */
                            }
                          /* if (true == fConcurrentDelay) */
                        }
                      else // !(false == fFixed)
                        {
                          /* For a fixed SL pattern rendering code of DRAW_RENDER event is executed only once. */
                          assert(true == fFirst);
                          assert(true == fSendPresentEvent);
                        }
                      /* if (false == fFixed) */
                    }
                  else // !(true == fBlocking)
                    {
                      if (false == fFixed)
                        {
                          /* The event cycle is:
                             ...->DRAW_PRESENT->DRAW_RENDER->(DRAW_VBLANK)->...
                             where DRAW_VBLANK event is repeated a predefined number of times.

                             Depedning on the value of vblanks_to_present raise either DRAW_VBLANK
                             or DRAW_PRESENT event signal. We also execute CAMERA_SYNC_TRIGGERS
                             via fallthrough by changing hnr code.
                          */
                          assert( false == fSendPresentEvent );
                          assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
                          assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                          assert(0 <= vblanks_to_present);
                          if (0 < vblanks_to_present)
                            {
                              BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                              assert(0 != set_vblank);
                            }
                          else
                            {
                              assert(false == fLast);

                              BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, ProjectorID);
                              assert(0 != set_present);
                            }
                          /* if */

                          // Execute CAMERA_SYNC_TRIGGERS event immediately by changing the hnr code.
                          if (true == HaveTriggerTime_inline(pTriggers))
                            {
                              hnr = 6;
                              AddEvent(pEvents, hnr);
                            }
                          /* if */
                        }
                      else // !(false == fFixed)
                        {
                          /* For a fixed SL pattern rendering code of DRAW_RENDER event is executed only once.
                             We dispatch DRAW_PRESENT event by setting the fSendPresentEvent flag to true.
                          */
                          fSendPresentEvent = true;

                          assert(true == fFirst);
                          assert(true == fSendPresentEvent);
                        }
                      /* if (false == fFixed) */
                    }
                  /* if (true == fBlocking) */
                }
              else // !(0 < num_cam)
                {
                  /* If there are no cameras attached the event cycle is DRAW_RENDER->DRAW_PRESENT.
                     Dispatch DRAW_PRESENT event by setting the fSendPresentEvent flag to true.
                  */
                  fSendPresentEvent = true;
                }
              /* if (0 < num_cam) */



              // Send DRAW_PRESENT event if requested.
              if (true == fSendPresentEvent)
                {
                  // Reset flag.
                  fSendPresentEvent = false;

                  // Signal DRAW_PRESENT event.
                  {
                    assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
                    assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                    BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, ProjectorID);
                    assert(0 != set_present);
                  }
                }
              /* if */

#pragma endregion // Event dispatch


              // Arm DRAW_PRESENT_READY event; DRAW_RENDER_READY will be armed after DRAW_PRESENT is processed.
              {
                assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                BOOL const set_present_ready = pSynchronization->EventSet(DRAW_PRESENT_READY, ProjectorID);
                assert(0 != set_present_ready);
              }

              // Stop timer for DRAW_RENDER event.
              FrameStatisticsToc(pStatisticsRenderDuration);

#pragma endregion // Process DRAW_RENDER event

            }
          else if (4 == hnr)
            {
              /****** PRESENT BUFFER ******/

              /* The DRAW_PRESENT event is fired when the prepared SL frame should be output to display.
                 The code for this event calls Present method of the DXGI SwapChain class and requests that
                 present operation is synchronized to VBLANK.

                 There also exists a DRAW_PRESENT_READY event which signals the state of the rendering
                 thread: it is in non-signalled state while the DRAW_PRESENT event is processed.
                 Note that in normal operation DRAW_PRESENT event should be raised only if DRAW_PRESENT_READY
                 event is in signalled state which may be done by waiting on DRAW_PRESENT_READY.


                 NOTES ABOUT PRESENTING

                 The state hr and image metadata sImageMetadata are retained from the previous DRAW_RENDER event.
                 Note that overlapping nature of DRAW_RENDER_READY and DRAW_PRESENT_READY signals together
                 with appropriate signalling logic ensures that each DRAW_RENDER is always followed by DRAW_PRESENT
                 so information in sImageMetadata cannot be lost.

                 Frame will be presented only if the DirectX display window is ready,
                 which is tested before calling the Present method by examining fModeChange flag.
                 The fModeChange flag is set to true when DXGI SwapChain is unavailable;
                 most common reason is the transition between windowed and exclusive full-screen mode
                 which takes significat time. Only if this flag is false we proceed by requesting
                 the exclusive access to DXGI SwapChain followed by calling the Present method.

                 Note that the Present method is non-blocking with regard to the VBLANK interrupt;
                 to actually synchronize program execution to the VBLANK interrupt we must wait for it.
              */

#pragma region // Process DRAW_PRESENT event

              // Disarm DRAW_PRESENT_READY and reset DRAW_PRESENT events.
              {
                assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                assert( true == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                BOOL const reset_present_ready = pSynchronization->EventReset(DRAW_PRESENT_READY, ProjectorID);
                assert(0 != reset_present_ready);

                BOOL const reset_present = pSynchronization->EventResetAndSetCounterSet(DRAW_PRESENT, ProjectorID, num_cam);
                assert(0 != reset_present);
              }

              // Set flags.
              bool frame_presented = false; // Flag will be changed to true if frame is presented.
              bool vblank_occured = false; // Flag will be changed to true if wait on VBLANK succeeds.
              bool got_stats = false; // Flag will be changed to true if DXGI frame statistics is successfully retrieved.


#pragma region // Present frame

              // Synchronize presentation between multiple projectors.
              int hnr_sync_present = -1;
              if (true == fSynchronize)
                {
                  assert( 1 < num_prj );

                  DWORD dwIsBusyResult = WAIT_FAILED;
                  DWORD dwWaitTime = 0;
                  do
                    {
                      dwIsBusyResult = pSynchronization->EventWaitForAny(
                                                                         DRAW_TERMINATE,    ProjectorID, // 0
                                                                         MAIN_PREPARE_DRAW, ProjectorID, // 1
                                                                         DRAW_SYNC_PRESENT, MainID,      // 2
                                                                         dwWaitTime
                                                                         );
                      int const hnr_is_busy = dwIsBusyResult - WAIT_OBJECT_0;
                      if (0 == hnr_is_busy) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizePresentDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (1 == hnr_is_busy) // MAIN_PREPARE_DRAW
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizePresentDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (2 == hnr_is_busy) // DRAW_SYNC_PRESENT
                        {
                          if (0 == dwWaitTime)
                            {
                              Debugfprintf(stderr, gDbgUnexpectedStallDuringSynchronizePresent, ProjectorID + 1, __FILE__, __LINE__);
                              dwWaitTime = 1;
                            }
                          /* if */
                        }
                      else
                        {
                          assert(WAIT_TIMEOUT == dwIsBusyResult);
                        }
                      /* if */
                    }
                  while (WAIT_OBJECT_0 + 2 == dwIsBusyResult);

                  // Signal the thread is ready to sync.
                  assert( false == DebugIsSignalled(pSynchronization, DRAW_SYNC_PRESENT, MainID) );

                  BOOL const set_sync = pSynchronization->EventSetConditional(DRAW_SYNC_PRESENT, MainID);
                  assert(0 != set_sync);

                  // Compare present and VBLANK counters.
                  bool const sync_ok = TestMultipleProjectorSynchronization_inline(parameters, vblank_counter, present_counter);

                  // Wait for confirmation.
                  DWORD const dwAllReady = pSynchronization->EventWaitForAny(
                                                                             DRAW_SYNC_PRESENT, MainID,      // 0
                                                                             DRAW_TERMINATE,    ProjectorID, // 1
                                                                             MAIN_PREPARE_DRAW, ProjectorID, // 2
                                                                             INFINITE
                                                                             );
                  hnr_sync_present = dwAllReady - WAIT_OBJECT_0;

                  if (0 == hnr_sync_present) assert(true == sync_ok);
                }
              /* if (true == fSynchronize) */

              // Present frame.
              bool const fModeChange_present = pWindow->fModeChange;
              if ( SUCCEEDED(hr) && (false == fModeChange_present) )
                {
                  EnterCriticalSection( &(pWindow->csRenderAndPresent) );
                  {
                    assert(false == pWindow->fRenderAndPresent);
                    pWindow->fRenderAndPresent = true; // Indicate the present operation is in progress.
                    {
                      if (NULL != pWindow->pSwapChain)
                        {
                          BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_present );
                          assert(TRUE == qpc_before);

                          // Present at next VBLANK (synchronized to VSYNC).
                          if (false == pWindow->fFreeze)
                            {
                              hr = pWindow->pSwapChain->Present(SyncInterval, 0);
                              assert( SUCCEEDED(hr) );
                            }
                          /* if */

                          BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_present );
                          assert(TRUE == qpc_after);

                          // Get result of present operation and increase present counter.
                          frame_presented = SUCCEEDED(hr);
                          if (true == frame_presented)
                            {
                              AcquireSRWLockExclusive( &(pWindow->sLockRT) );
                              {
                                present_counter = ++(pWindow->present_counter);
                              }
                              ReleaseSRWLockExclusive( &(pWindow->sLockRT) );
                            }
                          /* if (true == frame_presented) */

                          // Get frame statistics.
                          if ( (true == frame_presented) && (NULL != pWindow->pOutput) )
                            {
                              HRESULT const get_stats = pWindow->pOutput->GetFrameStatistics( &(pWindow->sStatisticsPresent) );
                              got_stats = SUCCEEDED(get_stats);
                            }
                          /* if */
                        }
                      /* if (NULL != pWindow->pSwapChain) */
                    }
                    pWindow->fRenderAndPresent = false; // Indicate the present operation is done.
                  }
                  LeaveCriticalSection( &(pWindow->csRenderAndPresent) );
                }
              /* if ( SUCCEEDED(hr) && (false == fModeChange_present) ) */

              // Increase frame counter.
              ++frame_counter;

              // Update present statistics or reset QCP values if present operation failed.
              if (true == frame_presented)
                {
                  FrameStatisticsAddMeasurement(pStatisticsPresentDuration, QPC_before_present, QPC_after_present);
                  FrameStatisticsAddFrame(pStatisticsPresentFrequency);
                }
              else
                {
                  QPC_before_present.QuadPart = (LONGLONG)0;
                  QPC_after_present.QuadPart = (LONGLONG)0;

                  if (false == fModeChange_present)
                    {
                      Debugfprintf(stderr, gDbgFramePresentFailed, ProjectorID + 1, frame_counter + 1);
                    }
                  /* if */
                }
              /* if */

              // Test if this is the last frame and set appropriate flag.
              if (true == sImageMetadata.fLast)
                {
                  fLast = true;
                  assert(false == ImageDecoderHaveNext(pImageDecoder));
                  assert(false == fFixed);
                }
              /* if */

              // Synchronize presentation between multiple projectors.
              if (true == fSynchronize)
                {
                  if (0 == hnr_sync_present) // DRAW_SYNC_PRESENT
                    {
                      assert( true == DebugIsSignalled(pSynchronization, DRAW_SYNC_PRESENT, MainID) );

                      BOOL const reset_sync = pSynchronization->EventResetConditional(DRAW_SYNC_PRESENT, MainID);
                      assert(0 != reset_sync);
                    }
                  else if (1 == hnr_sync_present) // DRAW_TERMINATE
                    {
                      Debugfprintf(stderr, gDbgAbortSynchronizePresentDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else if (2 == hnr_sync_present) // MAIN_READY_DRAW
                    {
                      Debugfprintf(stderr, gDbgAbortSynchronizePresentDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else
                    {
                      Debugfprintf(stderr, gDbgAbortSynchronizePresent, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  /* if */
                }
              /* if (true == fSynchronize) */

#pragma endregion // Present frame


              // For non-blocing acquisition some short tasks may be executed here.
              if (false == fBlocking)
                {
                  /* After the frame is presented we have to wait for the next VBLANK interrupt.
                     If the time till next VBLANK interrupt is long then it may be advantageous
                     to perform some selected tasks here to save up time.
                  */
                }
              /* if */


#pragma region // Wait for VBLANK interrupt

              // Wait for next VBLANK interrupt.
              bool const fModeChange_VBLANK = pWindow->fModeChange;
              if ( SUCCEEDED(hr) && (false == fModeChange_VBLANK) )
                {
                  EnterCriticalSection( &(pWindow->csWaitForVBLANK) );
                  {
                    pWindow->fWaitForVBLANK = true;
                    {
                      if (NULL != pWindow->pOutput)
                        {
                          BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_VBLANK );
                          assert(TRUE == qpc_before);

                          // Wait for VBLANK interrupt.
                          hr = pWindow->pOutput->WaitForVBlank();
                          assert( SUCCEEDED(hr) );

                          BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_VBLANK );
                          assert(TRUE == qpc_after);

                          // Get result of wait operation and increase VBLANK counter.
                          vblank_occured = SUCCEEDED(hr);
                          if (true == vblank_occured)
                            {
                              AcquireSRWLockExclusive( &(pWindow->sLockRT) );
                              {
                                vblank_counter = ++(pWindow->vblank_counter);
                              }
                              ReleaseSRWLockExclusive( &(pWindow->sLockRT) );
                            }
                          /* if (true == vblank_occured) */
                        }
                      /* if (NULL != pWindow->pOutput) */
                    }
                    pWindow->fWaitForVBLANK = false;
                  }
                  LeaveCriticalSection( &(pWindow->csWaitForVBLANK) );
                }
              /* if ( SUCCEEDED(hr) && (false == fModeChange_VBLANK) ) */

              // Update VBLANK statistics or reset QCP values if wait operation failed.
              if (true == vblank_occured)
                {
                  FrameStatisticsAddMeasurement(pStatisticsWaitForVBLANKDuration, QPC_before_VBLANK, QPC_after_VBLANK);
                }
              else
                {
                  QPC_before_VBLANK.QuadPart = (LONGLONG)0;
                  QPC_after_VBLANK.QuadPart = (LONGLONG)0;

                  if (false == fModeChange_VBLANK)
                    {
                      Debugfprintf(stderr, gDbgWaitForVBLANKFailed, ProjectorID + 1, frame_counter + 1);
                    }
                  /* if */
                }
              /* if */

              // Clear frame statistics if they were not retrieved successfully.
              if ( false == got_stats ) ZeroMemory( &(pWindow->sStatisticsPresent), sizeof(pWindow->sStatisticsPresent) );

              // Set VBLANK interval to next present for non-blocking acquisition.
              if ( false == fBlocking )
                {
                  /* The vblanks_to_present counter is always zero when the DRAW_PRESENT event
                     is processed so we must reset it to the requested value.

                     If there exist additional frames to process then then vblanks_to_present
                     counter is set to the number of VBLANKs requested by the user. However,
                     if there are no additional frames to be presented (current frame is the last
                     frame) then this thread must count as many VBLANKs as is needed to send
                     the triggers to the camera so all frames are captured.
                  */
                  assert(0 == vblanks_to_present);
                  if (false == fLast)
                    {
                      // Re-start VBLANK down counter for present operation.
                      assert(false == sImageMetadata.fLast);
                      vblanks_to_present = pWindow->presentTime - 1;
                    }
                  else
                    {
                      // Ensure camera will be triggerd for all presented frames.
                      assert(true == sImageMetadata.fLast);
                      vblanks_to_present = pWindow->delayTime_whole + 1;
                    }
                  /* if */
                  assert(0 <= vblanks_to_present);
                }
              else
                {
                  assert(-1 == vblanks_to_present);
                }
              /* if */

#pragma endregion // Wait for VBLANK interrupt


#pragma region // Update timing information and queue image metadata

              // Update timing information.
              AcquireSRWLockExclusive( &(pWindow->sLockRT) );
              {
                long int const vblank_counter_value = pWindow->vblank_counter;
                long int const present_counter_value = pWindow->present_counter;
                assert(vblank_counter_value == vblank_counter);
                assert(present_counter_value == present_counter);

                pWindow->vblank_counter_after_present_RT = vblank_counter_value;
                pWindow->present_counter_after_present_RT = present_counter_value;

                assert(0 <= frame_counter);
                key = frame_counter;
                sImageMetadata.key = key;

                if (true == frame_presented)
                  {
                    sImageMetadata.vblank_counter = vblank_counter_value;
                    sImageMetadata.present_counter = present_counter_value;

                    sImageMetadata.QPC_current_presented = QPC_after_VBLANK.QuadPart;
                    sImageMetadata.QPC_trigger_scheduled_RT = QPC_after_VBLANK.QuadPart + pWindow->QPC_delay_for_trigger_scheduled_RT;
                    sImageMetadata.QPC_trigger_scheduled_AT = QPC_after_VBLANK.QuadPart + pWindow->QPC_delay_for_trigger_scheduled_AT;
                    if ( (false == fBlocking) && (false == fLast) )
                      {
                        sImageMetadata.QPC_next_scheduled = QPC_after_VBLANK.QuadPart + pWindow->QPC_presentTime;
                      }
                    else
                      {
                        assert(-1 == sImageMetadata.QPC_next_scheduled);
                      }
                    /* if */
                    assert( -1 == sImageMetadata.QPC_next_presented );
                    assert( -1 == sImageMetadata.QPC_before_trigger );
                    assert( -1 == sImageMetadata.QPC_after_trigger );
                  }
                /* if */
              }
              ReleaseSRWLockExclusive( &(pWindow->sLockRT) );


              // For non-blocking acquisition mode add trigger time to the trigger queue.
              if ( (false == fBlocking) && (0 < num_cam) )
                {
                  // Add trigger information to trigger queue.
                  AddToTriggerQueue_inline(pTriggers, pWindow, key, present_counter, vblank_counter, QPC_after_VBLANK.QuadPart, fLast);
                }
              /* if */

              /* Transfer image metadata to image queues in acquisition thread(s).

                 The image metadata was set in DRAW_RENDER event and was updated with timing information
                 after the present operation completed. This metadata must now be transferred to the
                 acquisition thread. Each acquisition thread maintains one image metadata queue and
                 one copy of the metadata of last presented image for fast access.
                 Handling of the metadata differs depending on the type of the SL pattern:

                 1) Non-fixed SL pattern (flag fFixed has value false)

                 The rendering thread inserts image metadata directly into metadata queues of each
                 acquisition thread. The metadata of the last image is also directly copied into
                 sImageMetadataAT field of the pAcquisition structure. Note that for blocking acquisition
                 the metadata queue should always contain only one item; for non-blocking acquisition
                 the number of queued items depends on the trigger delay time.

                 2) Fixed SL pattern (flag fFixed has value true)

                 For non-fixed SL pattern the image metadata is same for all images which makes
                 copying the data to metadata queue unnecessary; the image metadata is copied only
                 to the sImageMetadataAT field of the pAcquisition structure. Each acquisition
                 thread will the use sImageMetadataAT field of the pAcquisition structure as
                 the template for data which has to be inserted into the metadata queue.

                 Note that image filename stored in pFilename is a pointer to std::wstring, therefore
                 the acutal std::wstring container must be duplicated for each acquisition thread.
              */
              assert( (NULL != sImageMetadata.pFilename) ||
                      ( (QI_UNKNOWN_TYPE == sImageMetadata.render_type) != (QI_REPEAT_PRESENT == sImageMetadata.render_type) )
                      );
              if (QI_REPEAT_PRESENT != sImageMetadata.render_type)
                {
                  if (0 < num_cam)
                    {
                      if (false == fFixed)
                        {
                          int const key_previous = sImageMetadata.key - 1;
                          bool const update_previous = (0 <= key_previous) && (false == fBlocking);
                          for (int i = 0; i < num_cam; ++i)
                            {
                              AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                              assert(NULL != pAcquisition);

                              sImageMetadata.CameraID = pAcquisition->CameraID;

                              ImageMetadataQueue * const pMetadataQueue = pAcquisition->pMetadataQueue;
                              assert(NULL != pMetadataQueue);

                              // Add information to the metadata of the previous frame.
                              if (true == update_previous)
                                {
                                  bool const adjusted = pMetadataQueue->AdjustImageMetadataRendering(key_previous, QPC_after_VBLANK.QuadPart);
                                  //assert(true == adjusted);
                                }
                              /* if */

                              // Push image metadata to queue.
                              bool const push = PushBackImageMetadataToQueue(pMetadataQueue, &sImageMetadata, true);
                              assert(true == push);

                              AcquireSRWLockExclusive( &(pAcquisition->sLockAT) );
                              {
                                assert(NULL == pAcquisition->sImageMetadataAT.pFilename);
                                pAcquisition->sImageMetadataAT = sImageMetadata; // Copy image metadata data to acquisition thread.
                                pAcquisition->sImageMetadataAT.pFilename = NULL;
                              }
                              ReleaseSRWLockExclusive( &(pAcquisition->sLockAT) );
                            }
                          /* for */
                        }
                      else // !(false == fFixed)
                        {
                          sImageMetadata.QPC_next_scheduled = -1;

                          for (int i = 0; i < num_cam; ++i)
                            {
                              AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                              assert(NULL != pAcquisition);

                              sImageMetadata.CameraID = pAcquisition->CameraID;

                              // Duplicate filename for each acquisition thread.
                              std::wstring * const pFilenameAT = (NULL != sImageMetadata.pFilename)? new std::wstring(*(sImageMetadata.pFilename)) : NULL;

                              AcquireSRWLockExclusive( &(pAcquisition->sLockAT) );
                              {
                                SAFE_DELETE(pAcquisition->pFilenameAT);
                                pAcquisition->pFilenameAT = pFilenameAT;

                                assert(NULL == pAcquisition->sImageMetadataAT.pFilename);
                                pAcquisition->sImageMetadataAT = sImageMetadata; // Copy image metadata data to acquisition thread.
                                pAcquisition->sImageMetadataAT.pFilename = NULL;
                              }
                              ReleaseSRWLockExclusive( &(pAcquisition->sLockAT) );
                            }
                          /* for */
                        }
                      /* if (false == fFixed) */
                    }
                  else // !(0 < num_cam)
                    {
                      // Nothing to queue as there are no cameras attached!
                    }
                  /* if (0 < num_cam) */

#ifdef _DEBUG
                  /* In batch acquisition mode all queued images must have the same type. */
                  if (true == parameters->fBatch)
                    {
                      assert(0 < num_cam);

                      QueuedImageType const render_type = sImageMetadata.render_type;
                      StructuredLightPatternType const pattern_type = sImageMetadata.pattern_type;

                      for (int i = 0; i < num_cam; ++i)
                        {
                          AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                          assert(NULL != pAcquisition);

                          ImageMetadataQueue * const pMetadataQueue = pAcquisition->pMetadataQueue;
                          assert(NULL != pMetadataQueue);

                          assert(true == pMetadataQueue->AreAllImagesOfType(render_type, pattern_type));
                        }
                      /* for */
                    }
                  /* if */
#endif /* _DEBUG */

                  // Clear metadata to indicate it was queued.
                  sImageMetadata.CameraID = -1;
                  SAFE_DELETE(sImageMetadata.pFilename);
                  sImageMetadata.render_type = QI_REPEAT_PRESENT;

                }
              /* if */

#pragma endregion // Update timing information and queue image metadata


#pragma region // Event dispatch

              // Signal appropriate event depedning on the acquisition mode.
              if (0 < num_cam)
                {
                  if (false == fFixed)
                    {
                      if (true == fBlocking)
                        {
                          if (true == fConcurrentDelay)
                            {
                              /* The event cycle is:
                                 ...->DRAW_PRESENT->DRAW_RENDER->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->DRAW_PRESENT->...

                                 The delay time is larger than the exposure time so we immediately pre-render
                                 the next frame after by signalling DRAW_RENDER event. The remaining delay time before
                                 camera trigger will then elapse in DRAW_VBLANK event after which the cameras will
                                 be triggered in CAMERA_SYNC_TRIGGERS event.
                              */
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );

                              BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                              assert(0 != set_render);
                            }
                          else // !(true == fConcurrentDelay)
                            {
                              /* The event cycle is:
                                 ...->DRAW_PRESENT->DRAW_VBLANK->CAMERA_SYNC_TRIGGERS->CAMERA_SEND_TRIGGER->CAMERA_EXPOSURE_END->DRAW_PRESENT->...
                                 where the DRAW_RENDER is fired immediately after CAMERA_SEND_TRIGGER is completed.

                                 The delay time is shorter than the exposure time so we immediately proceed
                                 to DRAW_VBLANK event where the delay is timed after wich the cameras will be
                                 triggered in CAMERA_SYNC_TRIGGERS event. DRAW_RENDER will be signalled by the
                                 acquisition threads once cameras are successfully triggered.
                              */
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                              BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                              assert(0 != set_vblank);
                            }
                          /* if (true == fConcurrentDelay) */
                        }
                      else // !(true == fBlocking)
                        {
                          /* The event cycle is:
                             ...->DRAW_PRESENT->DRAW_RENDER->(DRAW_VBLANK)->...
                             where DRAW_VBLANK event is repeated a predefined number of times.

                             Always signal DRAW_RENDER event to prepare the next image except when
                             ending the acquisition when DRAW_VBLANK is signalled.
                             We also execute CAMERA_SYNC_TRIGGERS via fallthrough by chagning hnr code.
                          */
                          assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
                          assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                          if (false == fLast)
                            {
                              assert( 0 <= vblanks_to_present );

                              BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                              assert(0 != set_render);
                            }
                          else
                            {
                              assert( 0 < vblanks_to_present );

                              BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                              assert(0 != set_vblank);
                            }
                          /* if */

                          // Execute CAMERA_SYNC_TRIGGERS event immediately by changing the hnr code.
                          if (true == HaveTriggerTime_inline(pTriggers))
                            {
                              hnr = 6;
                              AddEvent(pEvents, hnr);
                            }
                          /* if */
                        }
                      /* if (true == fBlocking) */
                    }
                  else // !(false == fFixed)
                    {
                      /* For a fixed SL pattern DRAW_PRESENT occures once at the start of acquisition,
                         therefore this code path is only reachable for the first frame.
                         Before triggering the cameras we have to wait for required delay time in DRAW_VBLANK event.
                      */
                      assert(true == fFirst);

                      assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );

                      BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                      assert(0 != set_vblank);

                      // Reset vblanks_to_present to non-initialized value.
                      if (false == fBlocking) vblanks_to_present = -1;
                    }
                  /* if (false == fFixed) */
                }
              else // !(0 < num_cam)
                {
                  /* If there are no cameras attached the event cycle is DRAW_RENDER->DRAW_PRESENT.
                     Dispatch DRAW_RENDER event.
                  */

                  assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
                  assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                  BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                  assert(0 != set_render);
                }
              /* if (0 < num_cam) */

#pragma endregion // Event dispatch


              // Reset fFirst flag.
              if (true == fFirst) fFirst = false;

              // Arm DRAW_RENDER_READY event; DRAW_PRESENT_READY will be armed after DRAW_RENDER is processed.
              {
                assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                BOOL const set_render_ready = pSynchronization->EventSet(DRAW_RENDER_READY, ProjectorID);
                assert(0 != set_render_ready);
              }

#pragma endregion // Process DRAW_PRESENT event

            }
          else if (5 == hnr)
            {
              /****** WAIT FOR NEXT V-BLANK  ******/

              /* For non-blocking acquisition mode this event is used to count VBLANK interrupts
                 via calls to the WaitForVBlank method in the cases when each SL frame has to
                 be presented for more than one screen refresh interval.

                 In the blocking acquisition mode this event is used to wait for the required delay.
              */

#pragma region // Process DRAW_VBLANK event

              // Disarm DRAW_VBLANK event.
              {
                BOOL const reset_vblank = pSynchronization->EventReset(DRAW_VBLANK, ProjectorID);
                assert(0 != reset_vblank);
              }

              // At least one camera must be attached.
              assert(0 < num_cam);

              if ( (true == fBlocking) || (true == fFixed) )
                {
                  /* In blocking acquisition mode we always have to wait regardless of the
                     value of other flags.

                     This waiting code is also used for a fixed SL pattern.
                  */

#ifdef _DEBUG
                  // Previous event must be one of DRAW_RENDER or DRAW_PRESENT events.
                  {
                    int hnr_prev = -1;
                    bool const getprev = GetPreviousEvent(pEvents, &hnr_prev, NULL, NULL, NULL);
                    assert(true == getprev);
                    if (true == parameters->fBatch) assert( (3 == hnr_prev) != (4 == hnr_prev) );
                  }
#endif /* _DEBUG */

                  // Wait for required delay.
                  SleepForRequiredDelay_inline(parameters, pWindow, QPC_after_VBLANK);


#pragma region // Synchronize multiple projectors

                  // Synchronize waiting when multiple projectors are used.
                  if (true == fSynchronize)
                    {
                      assert( 1 < num_prj );

                      DWORD dwIsBusyResult = WAIT_FAILED;
                      DWORD dwWaitTime = 0;
                      do
                        {
                          dwIsBusyResult = pSynchronization->EventWaitForAny(
                                                                             DRAW_TERMINATE,    ProjectorID, // 0
                                                                             MAIN_PREPARE_DRAW, ProjectorID, // 1
                                                                             DRAW_SYNC_VBLANK,  MainID,      // 2
                                                                             dwWaitTime
                                                                             );
                          int const hnr_is_busy = dwIsBusyResult - WAIT_OBJECT_0;
                          if (0 == hnr_is_busy) // DRAW_TERMINATE
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (1 == hnr_is_busy) // MAIN_PREPARE_DRAW
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (2 == hnr_is_busy) // DRAW_SYNC_VBLANK
                            {
                              if (0 == dwWaitTime)
                                {
                                  Debugfprintf(stderr, gDbgUnexpectedStallDuringSynchronizeVBLANK, ProjectorID + 1, __FILE__, __LINE__);
                                  dwWaitTime = 1;
                                }
                              /* if */
                            }
                          else
                            {
                              assert(WAIT_TIMEOUT == dwIsBusyResult);
                            }
                          /* if */
                        }
                      while (WAIT_OBJECT_0 + 2 == dwIsBusyResult);

                      // Signal the thread is ready to sync.
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_SYNC_VBLANK, MainID) );

                      BOOL const set_sync = pSynchronization->EventSetConditional(DRAW_SYNC_VBLANK, MainID);
                      assert(0 != set_sync);

                      // Compare present and VBLANK counters.
                      bool const sync_ok = TestMultipleProjectorSynchronization_inline(parameters, vblank_counter, present_counter);

                      // Wait for confirmation.
                      DWORD const dwAllReady = pSynchronization->EventWaitForAny(
                                                                                 DRAW_SYNC_VBLANK,  MainID,      // 0
                                                                                 DRAW_TERMINATE,    ProjectorID, // 1
                                                                                 MAIN_PREPARE_DRAW, ProjectorID, // 2
                                                                                 INFINITE
                                                                                 );
                      int const hnr_sync_vblank = dwAllReady - WAIT_OBJECT_0;

                      if (0 == hnr_sync_vblank) // DRAW_SYNC_VBLANK
                        {
                          assert( true == sync_ok );
                          assert( true == DebugIsSignalled(pSynchronization, DRAW_SYNC_VBLANK, MainID) );

                          BOOL const reset_sync = pSynchronization->EventResetConditional(DRAW_SYNC_VBLANK, MainID);
                          assert(0 != reset_sync);
                        }
                      else if (1 == hnr_sync_vblank) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (2 == hnr_sync_vblank) // MAIN_PREPARE_DRAW
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANK, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      /* if */
                    }
                  /* if (true == fSynchronize) */

#pragma endregion // Synchronize multiple projectors

                  // Execute CAMERA_SYNC_TRIGGERS event immediately by changing the hnr code.
                  hnr = 6;
                  AddEvent(pEvents, hnr);

                }
              else // !( (true == fBlocking) || (true == fFixed) )
                {
                  /* In the non-blocking acquisition mode we wait for the next VBLANK
                     to occur and we update the counters as needed.

                     This code cannot be used for a fixed SL pattern.
                  */

                  assert(false == fFixed);

                  // Set flags.
                  bool vblank_occured = false; // Flag will be changed to true if wait on VBLANK succeeds.

#pragma region // Wait for VBLANK interrupt

                  // Synchronize VBLANK between multiple projectors.
                  int hnr_sync_vblank = -1;
                  if (true == fSynchronize)
                    {
                      assert( 1 < num_prj );

                      DWORD dwIsBusyResult = WAIT_FAILED;
                      DWORD dwWaitTime = 0;
                      do
                        {
                          dwIsBusyResult = pSynchronization->EventWaitForAny(
                                                                             DRAW_TERMINATE,    ProjectorID, // 0
                                                                             MAIN_PREPARE_DRAW, ProjectorID, // 1
                                                                             DRAW_SYNC_VBLANK,  MainID,      // 2
                                                                             dwWaitTime
                                                                             );
                          int const hnr_is_busy = dwIsBusyResult - WAIT_OBJECT_0;
                          if (0 == hnr_is_busy) // DRAW_TERMINATE
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (1 == hnr_is_busy) // MAIN_PREPARE_DRAW
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (2 == hnr_is_busy) // DRAW_SYNC_VBLANK
                            {
                              if (0 == dwWaitTime)
                                {
                                  Debugfprintf(stderr, gDbgUnexpectedStallDuringSynchronizeVBLANK, ProjectorID + 1, __FILE__, __LINE__);
                                  dwWaitTime = 1;
                                }
                              /* if */
                            }
                          else
                            {
                              assert(WAIT_TIMEOUT == dwIsBusyResult);
                            }
                          /* if */
                        }
                      while (WAIT_OBJECT_0 + 2 == dwIsBusyResult);

                      // Signal the thread is ready to sync.
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_SYNC_VBLANK, MainID) );

                      BOOL const set_sync = pSynchronization->EventSetConditional(DRAW_SYNC_VBLANK, MainID);
                      assert(0 != set_sync);

                      // Compare present and VBLANK counters.
                      bool const sync_ok = TestMultipleProjectorSynchronization_inline(parameters, vblank_counter, present_counter);

                      // Wait for confirmation.
                      DWORD const dwAllReady = pSynchronization->EventWaitForAny(
                                                                                 DRAW_SYNC_VBLANK,  MainID,      // 0
                                                                                 DRAW_TERMINATE,    ProjectorID, // 1
                                                                                 MAIN_PREPARE_DRAW, ProjectorID, // 2
                                                                                 INFINITE
                                                                                 );
                      hnr_sync_vblank = dwAllReady - WAIT_OBJECT_0;

                      if (0 == hnr_sync_vblank) assert(true == sync_ok);
                    }
                  /* if (true == fSynchronize) */

                  // Wait for next VBLANK interrupt.
                  bool const fModeChange_VBLANK = pWindow->fModeChange;
                  if ( SUCCEEDED(hr) && (false == fModeChange_VBLANK) )
                    {
                      EnterCriticalSection( &(pWindow->csWaitForVBLANK) );
                      {
                        pWindow->fWaitForVBLANK = true;
                        {
                          if (NULL != pWindow->pOutput)
                            {
                              BOOL const qpc_before = QueryPerformanceCounter( &QPC_before_VBLANK );
                              assert(TRUE == qpc_before);

                              // Wait for VBLANK interrupt.
                              hr = pWindow->pOutput->WaitForVBlank();
                              assert( SUCCEEDED(hr) );

                              BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_VBLANK );
                              assert(TRUE == qpc_after);

                              // Get result of wait operation and increase VBLANK counter.
                              vblank_occured = SUCCEEDED(hr);
                              if (true == vblank_occured)
                                {
                                  AcquireSRWLockExclusive( &(pWindow->sLockRT) );
                                  {
                                    vblank_counter = ++(pWindow->vblank_counter);
                                  }
                                  ReleaseSRWLockExclusive( &(pWindow->sLockRT) );
                                }
                              /* if (true == vblank_occured) */
                            }
                          /* if (NULL != pWindow->pOutput) */
                        }
                        pWindow->fWaitForVBLANK = false;
                      }
                      LeaveCriticalSection( &(pWindow->csWaitForVBLANK) );
                    }
                  /* if ( SUCCEEDED(hr) && (false == fModeChange_VBLANK) ) */

                  // Update VBLANK statistics or reset QCP values if wait operation failed.
                  if (true == vblank_occured)
                    {
                      FrameStatisticsAddMeasurement(pStatisticsWaitForVBLANKDuration, QPC_before_VBLANK, QPC_after_VBLANK);
                    }
                  else
                    {
                      QPC_before_VBLANK.QuadPart = (LONGLONG)0;
                      QPC_after_VBLANK.QuadPart = (LONGLONG)0;

                      if (false == fModeChange_VBLANK)
                        {
                          Debugfprintf(stderr, gDbgWaitForVBLANKFailed, ProjectorID + 1, frame_counter + 1);
                        }
                      /* if */
                    }
                  /* if */

                  // Synchronize wait for VBLANK between multiple projectors.
                  if (true == fSynchronize)
                    {
                      if (0 == hnr_sync_vblank) // DRAW_SYNC_VBLANK
                        {
                          assert( true == DebugIsSignalled(pSynchronization, DRAW_SYNC_VBLANK, MainID) );

                          BOOL const reset_sync = pSynchronization->EventResetConditional(DRAW_SYNC_VBLANK, MainID);
                          assert(0 != reset_sync);
                        }
                      else if (1 == hnr_sync_vblank) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (2 == hnr_sync_vblank) // MAIN_READY_DRAW
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANKDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeVBLANK, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      /* if */
                    }
                  /* if (true == fSynchronize) */

                  // Decrease the number of VBLANKs to next DRAW_PRESENT event.
                  {
                    assert(0 < vblanks_to_present);
                    --vblanks_to_present; // Decrease present counter.
                    assert(0 <= vblanks_to_present);
                  }

#pragma endregion // Wait for VBLANK interrupt


#pragma region // Event dispatch

                  // Signal appropriate event depedning on the thread state.
                  {
                    assert( false == fSendPresentEvent );
                    assert( false == DebugIsSignalled(pSynchronization, DRAW_VBLANK, ProjectorID) );
                    assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                    if (false == fLast)
                      {
                        /* The event cycle is:
                           ...->DRAW_PRESENT->DRAW_RENDER->(DRAW_VBLANK)->...
                           where DRAW_VBLANK event is repeated a predefined number of times.

                           Depedning on the value of vblanks_to_present raise either DRAW_VBLANK
                           or DRAW_PRESENT event signal. We also execute CAMERA_SYNC_TRIGGERS
                           via fallthrough by changing hnr code.
                        */
                        assert(0 <= vblanks_to_present);
                        if (0 < vblanks_to_present)
                          {
                            BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                            assert(0 != set_vblank);
                          }
                        else
                          {
                            assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                            assert( true == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                            BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, ProjectorID);
                            assert(0 != set_present);
                          }
                        /* if */

                        // Execute CAMERA_SYNC_TRIGGERS event immediately by changing the hnr code.
                        if (true == HaveTriggerTime_inline(pTriggers))
                          {
                            hnr = 6;
                            AddEvent(pEvents, hnr);
                          }
                        /* if */
                      }
                    else // !(false == fLast)
                      {

#pragma region // Signal end of the batch acquisition

                        /* All images are displayed and we are executing the tail event sequence:
                           ...->DRAW_VBLANK->DRAW_VBLANK->DRAW_VBLANK->DRAW_VBLANK->MAIN_END_DRAW
                           which ends the acquisition.

                           Again, depending on the value of the vblanks_to_present we call either
                           DRAW_VBLANK or we end the acquisition by calling MAIN_END_DRAW.
                        */

                        if ( (0 == vblanks_to_present) && (true == HaveTriggerTime_inline(pTriggers)) )
                          {
                            vblanks_to_present = 1;
                          }
                        /* if */

                        if (0 < vblanks_to_present)
                          {
                            BOOL const set_vblank = pSynchronization->EventSet(DRAW_VBLANK, ProjectorID);
                            assert(0 != set_vblank);

                            // Execute CAMERA_SYNC_TRIGGERS event immediately by changing the hnr code.
                            if (true == HaveTriggerTime_inline(pTriggers))
                              {
                                hnr = 6;
                                AddEvent(pEvents, hnr);
                              }
                            /* if */
                          }
                        else // !(0 <= vblanks_to_present)
                          {
                            assert(false == HaveTriggerTime_inline(pTriggers));

                            assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );
                            assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                            assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                            assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );

                            fLast = false;

                            /* Before signalling MAIN_END_DRAW wait for all attached cameras to signal MAIN_END_CAMERA.
                               The event MAIN_END_CAMERA is signalled from the image transfer callback function so
                               if the trigger fails it will not be signalled. Therefore we also impose a hard limit
                               on wait time after which the MAIN_END_DRAW will be signalled.
                            */
                            DWORD const wait_time_ms = 15000; // Wait for 15 seconds.
                            DWORD const dwIsEndResult = WaitForAllCamerasToEndBatch_inline(parameters, pSynchronization, num_cam, wait_time_ms);
                            int const hnr_end = dwIsEndResult - WAIT_OBJECT_0;

                            if (2 == hnr_end) // MAIN_END_CAMERA
                              {
                                // Nothing to do!
                              }
                            else if (1 == hnr_end) // MAIN_PREPARE_DRAW
                              {
                                Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            else if (0 == hnr_end) // DRAW_TERMINATE
                              {
                                Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            else
                              {
                                Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            /* if */

                            // Blank screen.
                            {
                              assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                              HRESULT const hr_blank =
                                BlankScreen_inline(
                                                   pWindow, pD2DFactory,
                                                   QPC_before_present, QPC_after_present,
                                                   QPC_before_VBLANK, QPC_after_VBLANK
                                                   );
                              assert( SUCCEEDED(hr_blank) );
                            }

                            // Signal acquisition end.
                            BOOL const set_end = pSynchronization->EventSet(MAIN_END_DRAW, ProjectorID);
                            assert(0 != set_end);

                            // Wait for MAIN_RESUME_DRAW before continuing.
                            DWORD const dwIsResumeResult = pSynchronization->EventWaitForAny(
                                                                                             MAIN_RESUME_DRAW,  ProjectorID, // 0
                                                                                             DRAW_TERMINATE,    ProjectorID, // 1
                                                                                             MAIN_PREPARE_DRAW, ProjectorID, // 2
                                                                                             INFINITE // Wait forever.
                                                                                             );
                            int const hnr_resume = dwIsResumeResult - WAIT_OBJECT_0;

                            assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );

                            if (0 == hnr_resume) // MAIN_RESUME_DRAW
                              {
                                // Check rendering thread parameters.
                                assert(false == parameters->fBatch);
                                assert(false == parameters->fSynchronize);
                                assert(0 > parameters->num_prj);

                                // Reset acquisition mode to blocking with non-concurrent delay.
                                pWindow->fBlocking = true;
                                pWindow->fFixed = false;
                                pWindow->fConcurrentDelay = false;

                                // Reset all events.
                                BOOL const reset_sync_trigger = pSynchronization->EventResetAndSetCounterSet(CAMERA_SYNC_TRIGGERS, ProjectorID, num_cam);
                                assert(0 != reset_sync_trigger);

                                BOOL const reset_draw = pSynchronization->EventResetAllDrawExceptRenderAndPresentReady(ProjectorID);
                                assert(0 != reset_draw);

                                for (int i = 0; i < num_cam; ++i)
                                  {
                                    int const CameraID = nth_ID(parameters, i);
                                    assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                                    BOOL const reset_camera = pSynchronization->EventResetAllCameraExceptTriggerReady(CameraID);
                                    assert(0 != reset_camera);

                                    BOOL const reset_main = pSynchronization->EventResetAllMain(-1, -1, CameraID);
                                    assert(0 != reset_main);

                                    assert( false == DebugIsSignalled(pSynchronization, MAIN_END_CAMERA, CameraID) );
                                    assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                                  }
                                /* for */

                                // Kick-start preview.
                                vblanks_to_present = -1;
                                fSendPresentEvent = true;
                                fFirst = true;

                                assert( 6 != hnr );
                                assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                                assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                                assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                                BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                                assert(0 != set_render);
                              }
                            else if (1 == hnr_resume) // DRAW_TERMINATE
                              {
                                Debugfprintf(stderr, gDbgAbortPreviewDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            else if (2 == hnr_resume) // MAIN_PREPARE_DRAW
                              {
                                Debugfprintf(stderr, gDbgAbortPreviewDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            else
                              {
                                Debugfprintf(stderr, gDbgAbortPreview, ProjectorID + 1, __FILE__, __LINE__);
                              }
                            /* if */

                          }
                        /* if (0 <= vblanks_to_present) */

#pragma endregion // Signal end of the batch acquisition

                      }
                    /* if (false == fLast) */
                  }

#pragma endregion // Event dispatch

                }
              /* if ( (true == fBlocking) || (true == fFixed) ) */


#pragma endregion // Process DRAW_VBLANK event

            }
          else if (6 == hnr)
            {
              // This event is processed later to enable fallthrough via change to hnr.
            }
          else if (7 == hnr)
            {
              /****** CHANGE ID ******/

              /* Event identifiers may be changed during program execution, e.g. when projector is deleted.
                 This event is used to facilitate event ID change for the rendering thread, the image decoder
                 thread, and the associated DirectX display window. Event is normally dispatched by calling
                 RenderingThreadSetNewProjectorIDAndDecoderID function.
              */

#pragma region // Change event ID

              // Store old event ID.
              int const ProjectorIDOld = ProjectorID;

              // Output message.
              if (ProjectorIDOld != parameters->ProjectorID)
                {
                  Debugfwprintf(stderr, gDbgProjectorIDChanged, ProjectorIDOld + 1, ProjectorIDOld + 1, parameters->ProjectorID + 1);

                  SetThreadNameAndIDForMSVC(-1, "RenderingThread", parameters->ProjectorID);
                }
              else
                {
                  Debugfwprintf(stderr, gDbgProjectorIDNotChanged, ProjectorIDOld + 1);
                }
              /* if */

              // Fetch new event ID values.
              {
                ProjectorID = parameters->ProjectorID;
                assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

                DecoderID = pImageDecoder->DecoderID;
                assert( (0 <= DecoderID) && (DecoderID < (int)(pSynchronization->ImageDecoder.size())) );
                assert( ProjectorID == pImageDecoder->ProjectorID );
              }

              // Set projector ID for memory buffers.
              {
                std::wstring * ProjectorUID = GetUniqueProjectorIdentifier(parameters);
                for (int i = 0; i < num_cam; ++i)
                  {
                    AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                    assert(NULL != pAcquisition);
                    if (NULL == pAcquisition) continue;

                    ImageEncoderParameters * const pImageEncoder = pAcquisition->pImageEncoder;
                    assert(NULL != pImageEncoder);
                    if (NULL == pImageEncoder) continue;

                    assert(NULL != pImageEncoder->pAllImages);
                    if (NULL == pImageEncoder->pAllImages) continue;

                    pImageEncoder->pAllImages->SetProjector(ProjectorID, ProjectorUID);
                  }
                /* for */
                SAFE_DELETE(ProjectorUID);
              }

              // Disarm event; note that we have to use the old event ID.
              {
                BOOL const reset_change_id = pSynchronization->EventReset(DRAW_CHANGE_ID, ProjectorIDOld);
                assert(0 != reset_change_id);
              }

#pragma endregion // Change event ID

            }
          else
            {
              // We received an unknown event!
            }
          /* if */


          if (6 == hnr)
            {
              /****** TRIGGER ALL CAMERAS ******/

              /* This event is used to trigger the camera(s) in all acquisition modes.
                 Event may be reached in two ways, the first is by rasing the CAMERA_SYNC_TRIGGERS
                 event and the second is by code fallthrough when hnr is changed to 6 in event
                 processing code.
              */

#pragma region // Process CAMERA_SYNC_TRIGGERS event

              // Disarm CAMERA_SYNC_TRIGGERS event.
              if (hnr_received == hnr)
                {
                  BOOL const reset_sync_trigger = pSynchronization->EventResetAndSetCounterSet(CAMERA_SYNC_TRIGGERS, ProjectorID, num_cam);
                  assert(0 != reset_sync_trigger);
                }
              else
                {
                  assert( false == DebugIsSignalled(pSynchronization, CAMERA_SYNC_TRIGGERS, ProjectorID) );
                }
              /* if */

              // At least one camera must be attached.
              assert(0 < num_cam);

              if ( (true == fBlocking) || (true == fFixed) )
                {
                  /* In blocking acquisition mode we trigger all cameras in sequence but only after they are ready.
                     So first we test if all cameras are ready, and then we sequentially trigger them.

                     This triggering code is also used for a fixed SL pattern.
                  */

#pragma region // Synchronize multiple projectors

                  // Synchronize triggers when multiple projectors are used.
                  if (true == fSynchronize)
                    {
                      assert( 1 < num_prj );

                      DWORD dwIsBusyResult = WAIT_FAILED;
                      DWORD dwWaitTime = 0;
                      do
                        {
                          dwIsBusyResult = pSynchronization->EventWaitForAny(
                                                                             DRAW_TERMINATE,     ProjectorID, // 0
                                                                             MAIN_PREPARE_DRAW,  ProjectorID, // 1
                                                                             DRAW_SYNC_TRIGGERS, MainID,      // 2
                                                                             dwWaitTime
                                                                             );
                          int const hnr_is_busy = dwIsBusyResult - WAIT_OBJECT_0;
                          if (0 == hnr_is_busy) // DRAW_TERMINATE
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeTriggersDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (1 == hnr_is_busy) // MAIN_PREPARE_DRAW
                            {
                              Debugfprintf(stderr, gDbgAbortSynchronizeTriggersDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                            }
                          else if (2 == hnr_is_busy) // DRAW_SYNC_TRIGGERS
                            {
                              if (0 == dwWaitTime)
                                {
                                  Debugfprintf(stderr, gDbgUnexpectedStallDuringSynchronizeTriggers, ProjectorID + 1, __FILE__, __LINE__);
                                  dwWaitTime = 1;
                                }
                              /* if */
                            }
                          else
                            {
                              assert(WAIT_TIMEOUT == dwIsBusyResult);
                            }
                          /* if */
                        }
                      while (WAIT_OBJECT_0 + 2 == dwIsBusyResult);

                      // Signal the thread is ready to sync.
                      assert( false == DebugIsSignalled(pSynchronization, DRAW_SYNC_TRIGGERS, MainID) );

                      BOOL const set_sync = pSynchronization->EventSetConditional(DRAW_SYNC_TRIGGERS, MainID);
                      assert(0 != set_sync);

                      // Compare present and VBLANK counters.
                      bool const sync_ok = TestMultipleProjectorSynchronization_inline(parameters, vblank_counter, present_counter);

                      // Wait for confirmation.
                      DWORD const dwAllReady = pSynchronization->EventWaitForAny(
                                                                                 DRAW_SYNC_TRIGGERS, MainID,      // 0
                                                                                 DRAW_TERMINATE,     ProjectorID, // 1
                                                                                 MAIN_PREPARE_DRAW,  ProjectorID, // 2
                                                                                 INFINITE
                                                                                 );
                      int const hnr_sync_triggers = dwAllReady - WAIT_OBJECT_0;

                      if (0 == hnr_sync_triggers) // DRAW_SYNC_TRIGGERS
                        {
                          assert( true == sync_ok );
                          assert( true == DebugIsSignalled(pSynchronization, DRAW_SYNC_TRIGGERS, MainID) );

                          BOOL const reset_sync = pSynchronization->EventResetConditional(DRAW_SYNC_TRIGGERS, MainID);
                          assert(0 != reset_sync);
                        }
                      else if (1 == hnr_sync_triggers) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeTriggersDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (2 == hnr_sync_triggers) // MAIN_PREPARE_DRAW
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeTriggersDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else
                        {
                          Debugfprintf(stderr, gDbgAbortSynchronizeTriggers, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      /* if */
                    }
                  /* if (true == fSynchronize) */

#pragma endregion // Synchronize multiple projectors

                  assert(-1 == vblanks_to_present);

                  // Wait for all cameras to become ready.
                  DWORD const dwIsReadyResult = WaitForAllCamerasToBecomeReady_inline(parameters, pSynchronization, num_cam, INFINITE);
                  int const hnr_ready = dwIsReadyResult - WAIT_OBJECT_0;


#pragma region // Test if all frames are acquired for a fixed SL pattern

                  // For a fixed SL pattern all acquisition threads must acquire the same number of frames.
                  if (true == fFixed)
                    {
                      bool all_ended = true;
                      long int trigger_counter = -1;
                      long int ith_trigger_counter = -1;
                      for (int i = 0; i < num_cam; ++i)
                        {
                          AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                          assert(NULL != pAcquisition);
                          AcquireSRWLockShared( &(pWindow->sLockRT) );
                          {
                            ith_trigger_counter = pAcquisition->trigger_counter;
                          }
                          ReleaseSRWLockShared( &(pWindow->sLockRT) );
                          all_ended = all_ended && ( (int)(ith_trigger_counter) + 1 == pWindow->num_acquire );
                          if (0 == i)
                            {
                              trigger_counter = ith_trigger_counter;
                            }
                          else
                            {
                              assert(ith_trigger_counter == trigger_counter);
                            }
                          /* if */
                        }
                      /* for */

                      if (true == all_ended)
                        {
                          fLast = true;
                          assert( (int)(trigger_counter) + 1 == pWindow->num_acquire );
                        }
                      /* if */
                    }
                  /* if */

#pragma endregion // Test if all frames are acquired for a fixed SL pattern


#pragma region // Trigger cameras for blocking acquisition

                  // Trigger cameras sequentially.
                  bool triggered = true;
                  if (2 == hnr_ready) // CAMERA_READY
                    {
                      for (int i = 0; i < num_cam; ++i)
                        {
                          AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                          assert(NULL != pAcquisition);

                          int const CameraID = pAcquisition->CameraID;
                          assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                          /* Update counter values and query QPC timer. */
                          AcquireSRWLockExclusive( &(pAcquisition->sLockAT) );
                          {
                            pAcquisition->vblank_counter_before_trigger_RT = vblank_counter;
                            pAcquisition->present_counter_before_trigger_RT = present_counter;
                            if (false == fFixed)
                              {
                                pAcquisition->key = key;
                              }
                            else
                              {
                                pAcquisition->key = pAcquisition->trigger_counter;
                                pAcquisition->sImageMetadataAT.key = pAcquisition->trigger_counter;
                              }
                            /* if */
                          }
                          ReleaseSRWLockExclusive( &(pAcquisition->sLockAT) );

                          assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                          assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );

                          BOOL const qpc_before = QueryPerformanceCounter( &(pAcquisition->QPC_before_trigger_RT) );
                          assert(TRUE == qpc_before);

                          BOOL const set_trigger = pSynchronization->EventSet(CAMERA_SEND_TRIGGER, CameraID);
                          assert(0 != set_trigger);

                          BOOL const qpc_after = QueryPerformanceCounter( &(pAcquisition->QPC_after_trigger_RT) );
                          assert(TRUE == qpc_after);

                          if (0 != set_trigger)
                            {
                              triggered = triggered && true;
                            }
                          else
                            {
                              triggered = false;
                              Debugfprintf(stderr, gDbgTriggerDropForCamera, ProjectorID + 1, CameraID + 1, __FILE__, __LINE__);
                            }
                          /* if */
                        }
                      /* for */
                    }
                  else if (1 == hnr_ready) // MAIN_PREPARE_DRAW
                    {
                      triggered = false;
                      Debugfprintf(stderr, gDbgTriggerDropDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else if (0 == hnr_ready) // DRAW_TERMINATE
                    {
                      triggered = false;
                      Debugfprintf(stderr, gDbgTriggerDropDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                    }
                  else
                    {
                      Debugfprintf(stderr, gDbgTriggerDrop, ProjectorID + 1, __FILE__, __LINE__);
                      triggered = false;
                    }
                  /* if */

#pragma endregion // Trigger cameras for blocking acquisition


#pragma region // Signal end of the batch acquisition

                  // Signal acquisition end.
                  if (true == fLast)
                    {
                      fLast = false;

                      /* All images are displayed and all cameras are triggered for the last image.
                         Wait for all acquisition threads to signal MAIN_END_CAMERA and then raise MAIN_END_DRAW event.
                      */
                      DWORD const wait_time_ms = 15000; // Wait for 15 seconds.
                      DWORD const dwIsEndResult = WaitForAllCamerasToEndBatch_inline(parameters, pSynchronization, num_cam, wait_time_ms);
                      int const hnr_end = dwIsEndResult - WAIT_OBJECT_0;

                      if (2 == hnr_end) // MAIN_END_CAMERA
                        {
                          // Nothing to do!
                        }
                      else if (1 == hnr_end) // MAIN_PREPARE_DRAW
                        {
                          Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (0 == hnr_end) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else
                        {
                          Debugfprintf(stderr, gDbgDidNotReceiveMainEndCamera, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      /* if */

                      // Blank screen.
                      {
                        if ( (false == fFixed) && (true == fConcurrentDelay) )
                          {
                            assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                            assert( true == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );

                            HRESULT const hr_blank_present = BlankScreenPresent_inline(pWindow, QPC_before_present, QPC_after_present, true);
                            assert( SUCCEEDED(hr_blank_present) );
                          }
                        else
                          {
                            assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                            assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                          }
                        /* if */

                        HRESULT const hr_blank =
                          BlankScreen_inline(
                                             pWindow, pD2DFactory,
                                             QPC_before_present, QPC_after_present,
                                             QPC_before_VBLANK, QPC_after_VBLANK
                                             );
                        assert( SUCCEEDED(hr_blank) );
                      }

                      // Signal acquisition end.
                      assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );

                      BOOL const set_end = pSynchronization->EventSet(MAIN_END_DRAW, ProjectorID);
                      assert(0 != set_end);

                      // Wait for MAIN_RESUME_DRAW before continuing.
                      DWORD const dwIsResumeResult = pSynchronization->EventWaitForAny(
                                                                                       MAIN_RESUME_DRAW,  ProjectorID, // 0
                                                                                       DRAW_TERMINATE,    ProjectorID, // 1
                                                                                       MAIN_PREPARE_DRAW, ProjectorID, // 2
                                                                                       INFINITE // Wait forever.
                                                                                       );
                      int const hnr_resume = dwIsResumeResult - WAIT_OBJECT_0;

                      assert( false == DebugIsSignalled(pSynchronization, MAIN_END_DRAW, ProjectorID) );

                      if (0 == hnr_resume) // MAIN_RESUME_DRAW
                        {
                          // Check rendering thread parameters.
                          assert(false == parameters->fBatch);
                          assert(false == parameters->fSynchronize);
                          assert(0 > parameters->num_prj);

                          // Reset acquisition mode to blocking with non-concurrent delay.
                          pWindow->fBlocking = true;
                          pWindow->fFixed = false;
                          pWindow->fConcurrentDelay = false;

                          // Reset all events.
                          BOOL const reset_sync_trigger = pSynchronization->EventResetAndSetCounterSet(CAMERA_SYNC_TRIGGERS, ProjectorID, num_cam);
                          assert(0 != reset_sync_trigger);

                          BOOL const reset_draw = pSynchronization->EventResetAllDrawExceptRenderAndPresentReady(ProjectorID);
                          assert(0 != reset_draw);

                          for (int i = 0; i < num_cam; ++i)
                            {
                              int const CameraID = nth_ID(parameters, i);
                              assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                              BOOL const reset_camera = pSynchronization->EventResetAllCameraExceptTriggerReady(CameraID);
                              assert(0 != reset_camera);

                              BOOL const reset_main = pSynchronization->EventResetAllMain(-1, -1, CameraID);
                              assert(0 != reset_main);

                              assert( false == DebugIsSignalled(pSynchronization, MAIN_END_CAMERA, CameraID) );
                              assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                            }
                          /* for */

                          // Kick-start preview.
                          if (false == fFixed)
                            {
                              if (true == fConcurrentDelay)
                                {
                                  HRESULT const hr_blank_render = BlankScreenRender_inline(pWindow, pD2DFactory);
                                  assert( SUCCEEDED(hr_blank_render) );

                                  // Send DRAW_PRESENT event.
                                  assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                                  assert( true == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                                  assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT, ProjectorID) );

                                  BOOL const set_present = pSynchronization->EventSet(DRAW_PRESENT, ProjectorID);
                                  assert(0 != set_present);
                                }
                              else // !(true == fConcurrentDelay)
                                {
                                  // Send DRAW_RENDER event.
                                  assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                                  assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                                  assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                                  BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                                  assert(0 != set_render);
                                }
                              /* if (true == fConcurrentDelay) */
                            }
                          else // !(false == fFixed)
                            {
                              vblanks_to_present = -1;
                              fSendPresentEvent = true;
                              fFirst = true;

                              // Send DRAW_RENDER event.
                              assert( true == DebugIsSignalled(pSynchronization, DRAW_RENDER_READY, ProjectorID) );
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_PRESENT_READY, ProjectorID) );
                              assert( false == DebugIsSignalled(pSynchronization, DRAW_RENDER, ProjectorID) );

                              BOOL const set_render = pSynchronization->EventSet(DRAW_RENDER, ProjectorID);
                              assert(0 != set_render);
                            }
                          /* if (false == fFixed) */
                        }
                      else if (1 == hnr_resume) // DRAW_TERMINATE
                        {
                          Debugfprintf(stderr, gDbgAbortPreviewDueToDrawTerminate, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else if (2 == hnr_resume) // MAIN_PREPARE_DRAW
                        {
                          Debugfprintf(stderr, gDbgAbortPreviewDueToMainPrepareDraw, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      else
                        {
                          Debugfprintf(stderr, gDbgAbortPreview, ProjectorID + 1, __FILE__, __LINE__);
                        }
                      /* if */
                    }
                  /* if (true == fLast) */

#pragma endregion // Signal end of the batch acquisition

                }
              else // !(true == fBlocking)
                {
                  /* In non-blocking acquisition mode we must fetch next trigger information and test if
                     the time to trigger has arrived. If so then we trigger the cameras if they are ready,
                     otherwise the trigger is discarded if maximal allowed wait time elapsed.

                     Triggering information is stored in a queue in the order in which the frames are presented.
                     The first element in the queue may be obsolete by the time the triggering is scheduled
                     to run so the first trigger in the queue is always tested and removed if needed. In this
                     way we ensure that triggering code will always consider a trigger which is on time.

                     This code cannot be used for a fixed SL pattern as the trigger queue is only accessible
                     from the rendering thread so acquisition threads cannot add to it.
                  */

                  assert(false == fFixed);

#pragma region // Trigger cameras for non-blocking acquisition

                  // Remove expired triggers.
                  {
                    bool const remove = RemoveExpiredTriggers_inline(pTriggers, pWindow, ProjectorID);
                    assert(true == remove);
                  }

                  PresentAndTriggerTimes sTimes;
                  bool const peek = PeekTriggerTime_inline(pTriggers, &sTimes);

                  if (true == peek)
                    {
                      // Concurrently fetch data from pWindow.
                      __int64 QPC_delayTime = -1;
                      __int64 QPC_exposureTime = -1;
                      AcquireSRWLockShared( &(pWindow->sLockRT) );
                      {
                        QPC_delayTime = pWindow->QPC_delayTime;
                        QPC_exposureTime = pWindow->QPC_exposureTime;
                      }
                      ReleaseSRWLockShared( &(pWindow->sLockRT) );
                      assert( 0 <= QPC_delayTime );
                      assert( 0 <= QPC_exposureTime );

                      // Fetch current QPC time.
                      LARGE_INTEGER QPC_now;
                      QPC_now.QuadPart = (LONGLONG)(-1);

                      BOOL const query_qpc = QueryPerformanceCounter( &QPC_now );
                      assert(TRUE == query_qpc);
                      assert((LONGLONG)(-1) != QPC_now.QuadPart);

                      // Get latest allowed trigger time.
                      __int64 QPC_delay_after_next = QPC_delayTime - QPC_exposureTime;
                      if (0 > QPC_delay_after_next) QPC_delay_after_next = 0;

                      __int64 QPC_trigger_latest = -1;
                      if (0 < sTimes.QPC_next_scheduled) QPC_trigger_latest = sTimes.QPC_next_scheduled;
                      if (0 < sTimes.QPC_next_presented) QPC_trigger_latest = sTimes.QPC_next_presented;
                      if (0 < QPC_trigger_latest) QPC_trigger_latest = QPC_trigger_latest + QPC_delay_after_next;
                      if (0 > QPC_trigger_latest)
                        {
                          assert(-1 == sTimes.QPC_next_scheduled);
                          assert(-1 == sTimes.QPC_next_presented);
                          QPC_trigger_latest = LLONG_MAX;
                        }
                      /* if */

                      // Test if the trigger is on-time.
                      bool const on_time_RT = sTimes.QPC_trigger_scheduled_RT < QPC_now.QuadPart;
                      bool const on_time_AT = sTimes.QPC_trigger_scheduled_AT >= QPC_now.QuadPart;
                      bool const on_time_absolute = QPC_trigger_latest >= QPC_now.QuadPart;

                      // Test if all cameras are ready.
                      bool const cameras_ready = AreAllCamerasReady_inline(parameters, pSynchronization, num_cam);

                      // Trigger the cameras if we are on time.
                      if ( (true == on_time_RT) && (true == on_time_absolute) && (true == cameras_ready) )
                        {
                          bool triggered = true;

                          for (int i = 0; i < num_cam; ++i)
                            {
                              AcquisitionParameters * const pAcquisition = nth_pAcquisition(parameters, i);
                              assert(NULL != pAcquisition);

                              int const CameraID = pAcquisition->CameraID;
                              assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

                              /* Update counter values and query QPC timer. */
                              AcquireSRWLockExclusive( &(pAcquisition->sLockAT) );
                              {
                                pAcquisition->vblank_counter_before_trigger_RT = vblank_counter;
                                pAcquisition->present_counter_before_trigger_RT = present_counter;
                                pAcquisition->key = sTimes.key;
                              }
                              ReleaseSRWLockExclusive( &(pAcquisition->sLockAT) );

                              assert( true == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID) );
                              assert( false == DebugIsSignalled(pSynchronization, CAMERA_SEND_TRIGGER, CameraID) );

                              BOOL const qpc_before = QueryPerformanceCounter( &(pAcquisition->QPC_before_trigger_RT) );
                              assert(TRUE == qpc_before);

                              BOOL const set_trigger = pSynchronization->EventSet(CAMERA_SEND_TRIGGER, CameraID);
                              assert(0 != set_trigger);

                              BOOL const qpc_after = QueryPerformanceCounter( &(pAcquisition->QPC_after_trigger_RT) );
                              assert(TRUE == qpc_after);

                              if (0 != set_trigger)
                                {
                                  triggered = triggered && true;
                                }
                              else
                                {
                                  triggered = false;
                                  Debugfprintf(stderr, gDbgTriggerDropForCamera, ProjectorID + 1, CameraID + 1, __FILE__, __LINE__);
                                }
                              /* if */
                            }
                          /* for */

                          bool const pop = PopTriggerTime_inline(pTriggers, NULL);
                          assert(true == pop);
                        }
                      else if (true == on_time_RT)
                        {
                          if (false == on_time_absolute)
                            {
                              Debugfprintf(
                                           stderr,
                                           gDbgTriggerDropForMetadata,
                                           ProjectorID + 1,
                                           sTimes.key + 1, vblank_counter,
                                           __FILE__, __LINE__
                                           );

                              long int const key_value = sTimes.key;
                              bool const pop = PopTriggerTime_inline(pTriggers, &sTimes);
                              assert(true == pop);
                              assert(key_value == sTimes.key);
                              assert(-1 != sTimes.QPC_next_scheduled);
                            }
                          else if (false == on_time_AT)
                            {
                              double const unexpected_delay_ms = (double)(QPC_now.QuadPart - sTimes.QPC_trigger_scheduled_AT) * pWindow->ticks_to_ms;
                              assert(0.0 <= unexpected_delay_ms);

                              Debugfprintf(
                                           stderr,
                                           gDbgTriggerDelayKnownMetadata,
                                           ProjectorID + 1,
                                           sTimes.key + 1, unexpected_delay_ms, vblank_counter,
                                           __FILE__, __LINE__
                                           );
                            }
                          /* if */
                        }
                      /* if */

                      // Remove expired triggers.
                      {
                        bool const remove = RemoveExpiredTriggers_inline(pTriggers, pWindow, ProjectorID);
                        assert(true == remove);
                      }

                    }
                  /* if (true == peek) */

#pragma endregion // Trigger cameras for non-blocking acquisition

                }
              /* if (true == fBlocking) */

#pragma endregion // Process CAMERA_SYNC_TRIGGERS event


              // Update processing time of this and of the previous event.
              PreviousEventProcessed(pEvents);

#ifdef _DEBUG
              // Print event processing time in percentage of screen refresh interval.
              {
                int event_code = -1;
                double event_duration_ms = -1.0;

                bool const get_event = GetPreviousEvent(pEvents, &event_code, &event_duration_ms, NULL, NULL);
                assert(true == get_event);

                //if (true == get_event) CheckEventDuration_inline(event_code, event_duration_ms, parameters, pWindow);
              }
#endif /* _DEBUG */

              EventProcessed(pEvents);

            }
          else
            {
              // Update processing time.
              EventProcessed(pEvents);
            }
          /* if */

#ifdef _DEBUG
          // Print event processing time in percentage of screen refresh interval.
          {
            int event_code = -1;
            double event_duration_ms = -1.0;

            bool const get_event = GetCurrentEvent(pEvents, &event_code, &event_duration_ms, NULL, NULL);
            assert(true == get_event);

            //if (true == get_event) CheckEventDuration_inline(event_code, event_duration_ms, parameters, pWindow);
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

  ImageMetadataRelease( &sImageMetadata );

  PastEventsDelete( pEvents );

  {
    BOOL const set_terminate = pSynchronization->EventReset(DRAW_TERMINATE, ProjectorID);
    assert(0 != set_terminate);
  }

  parameters->fActive = false;

  return 0;

#pragma endregion // Cleanup

}
/* RenderingThread */



/****** START/STOP THREAD ******/

#pragma region // Start/stop rendering thread

//! Create rendering parameters and start rendering thread.
/*!
  Spawns rendering thread.

  \param pSynchronization       Pointer to a structure holding all required syncrhonization events.
  \param pWindow Pointer to opened display window.
  \param pImageDecoder   Pointer to image decoder thread structure.
  \param ProjectorID Unique thread identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \return Returns pointer to rendering thread parameters or NULL if unsuccessfull.
*/
RenderingParameters *
RenderingThreadStart(
                     SynchronizationEvents * const pSynchronization,
                     DisplayWindowParameters * const pWindow,
                     ImageDecoderParameters * const pImageDecoder,
                     int const ProjectorID
                     )
{
  RenderingParameters * const P = (RenderingParameters *)malloc( sizeof(RenderingParameters) );
  assert(NULL != P);
  if (NULL == P) return P;

  RenderingParametersBlank_inline( P );

  /* Initialize variables. */
  InitializeSRWLock( &(P->sLockAcquisitions) );
  InitializeSRWLock( &(P->sLockRenderings) );

  assert(NULL == P->pRenderings);
  P->pRenderings = new std::vector<RenderingParameters_ *>;
  assert(NULL != P->pRenderings);

  if (NULL == P->pRenderings) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pTriggers);
  P->pTriggers = new std::vector<PresentAndTriggerTimes>;
  assert(NULL != P->pTriggers);

  if (NULL == P->pTriggers) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pAcquisitions);
  P->pAcquisitions = new std::vector<AcquisitionParameters *>;
  assert(NULL != P->pAcquisitions);

  if (NULL == P->pAcquisitions) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsRenderDuration);
  P->pStatisticsRenderDuration = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsRenderDuration);

  if (NULL == P->pStatisticsRenderDuration) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsPresentDuration);
  P->pStatisticsPresentDuration = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsPresentDuration);

  if (NULL == P->pStatisticsPresentDuration) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsPresentFrequency);
  P->pStatisticsPresentFrequency = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsPresentFrequency);

  if (NULL == P->pStatisticsPresentFrequency) goto RENDERING_THREAD_START_EXIT;

  assert(NULL == P->pStatisticsWaitForVBLANKDuration);
  P->pStatisticsWaitForVBLANKDuration = FrameStatisticsCreate();
  assert(NULL != P->pStatisticsWaitForVBLANKDuration);

  if (NULL == P->pStatisticsWaitForVBLANKDuration) goto RENDERING_THREAD_START_EXIT;

  /* Copy parameters. */
  assert(NULL == P->pSynchronization);
  P->pSynchronization = pSynchronization;
  assert(NULL != P->pSynchronization);

  assert(NULL == P->pWindow);
  P->pWindow = pWindow;
  assert(NULL != P->pWindow);

  assert(NULL == P->pImageDecoder);
  P->pImageDecoder = pImageDecoder;
  assert(NULL != P->pImageDecoder);

  assert(-1 == P->ProjectorID);
  P->ProjectorID = ProjectorID;
  assert( (0 <= P->ProjectorID) && (P->ProjectorID < (int)(P->pSynchronization->Draw.size())) );

  /* Start rendering thread. */
  P->tRendering =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             RenderingThread,
                             (void *)( P ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != P->tRendering );
  if ( (HANDLE)( NULL ) == P->tRendering )
    {
    RENDERING_THREAD_START_EXIT:

      RenderingParametersRelease_inline( P );
      return NULL;
    }
  /* if */

  if (NULL != P->pWindow)
    {
      assert( (-1 == P->pWindow->ProjectorID) != (ProjectorID == P->pWindow->ProjectorID) );
      P->pWindow->ProjectorID = ProjectorID;

      assert(NULL == P->pWindow->pRendering);
      P->pWindow->pRendering = P;
    }
  /* if */
  
  return P;
}
/* RenderingStart */



//! Stop rendering thread.
/*!
  Stops image rendering thread.

  \param P      Pointer to rendering thread parameters.
*/
void
RenderingThreadStop(
                    RenderingParameters * const P
                    )
{
  //assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pSynchronization);
  if (NULL != P->pSynchronization)
    {
      DWORD const result = WaitForSingleObject(P->tRendering, 0);

      if ( (WAIT_OBJECT_0 != result) && (true == P->fActive) )
        {
          // The thread is alive so signal terminate event and wait for confirmation.
          BOOL const sm = P->pSynchronization->EventSet(DRAW_TERMINATE, P->ProjectorID);
          assert(0 != sm);

          if (0 != sm)
            {
              DWORD const confirm = WaitForSingleObject(P->tRendering, INFINITE);
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

  assert( WAIT_OBJECT_0 == WaitForSingleObject(P->tRendering, 0) );
  assert( false == P->fActive );

  RenderingParametersRelease_inline( P );
}
/* RenderingThreadStop */

#pragma endregion // Start/stop rendering thread



/****** AUXILIARY FUNCTIONS ******/

#pragma region // Add and remove projectors

//! Add projectors which should be synchronized.
/*!
  Function adds a list of projectors which must be synchronized.

  \param P     Pointer to rendering thread parameters.
  \param pRenderings     Pointer to list of projectors to add.
  \return Returns true if successfull; false otherwise.
*/
bool
RenderingThreadAddProjectors(
                             RenderingParameters * const P,
                             std::vector<RenderingParameters_ *> * const pRenderings
                             )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pRenderings);
  if (NULL == P->pRenderings) return false;

  assert(NULL != pRenderings);
  if (NULL == pRenderings) return false;

  int const max_i = (int)( pRenderings->size() );
  assert(0 < max_i);
  if (0 == max_i) return true;

  AcquireSRWLockExclusive( &(P->sLockRenderings) );
  {
    if (NULL != P->pRenderings)
      {
        for (int i = 0; i < max_i; ++i)
          {
            RenderingParameters_ * const ith_ptr = (*pRenderings)[i];
            P->pRenderings->push_back(ith_ptr);
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockRenderings) );

  return true;
}
/* RenderingThreadAddProjectors */



//! Remove all projectors which should be synchronized.
/*!
  Function removes all projectors which should be synchronized.

  \param P     Pointer to rendering thread parameters.
  \return Returns true if successfull; false otherwise.
*/
bool
RenderingThreadRemoveProjectors(
                                RenderingParameters * const P
                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pRenderings);
  if (NULL == P->pRenderings) return false;

  AcquireSRWLockExclusive( &(P->sLockRenderings) );
  {
    if (NULL != P->pRenderings) P->pRenderings->clear();
  }
  ReleaseSRWLockExclusive( &(P->sLockRenderings) );

  return true;
}
/* RenderingThreadRemoveProjectors */

#pragma endregion // Add and remove projectors


#pragma region // Add, remove or swap camera

//! Add camera to rendering thread.
/*!
  Slaves acquisition thread to the rendering thread.
  After this camera associated with the acquistion thread is slaved to the rendering thread.

  \param P      Pointer to rendering thread parameters.
  \param pAcquisition Pointer to acquisition thread to add.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadAddCamera(
                         RenderingParameters * const P,
                         AcquisitionParameters * const pAcquisition
                         )
{
  bool added = false;

  assert(NULL != pAcquisition);
  if (NULL == pAcquisition) return added;

  assert(NULL != P);
  if (NULL == P) return added;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return added;

  int const CameraID = pAcquisition->CameraID;
  assert( (0 <= CameraID) && (CameraID < (int)(P->pSynchronization->Camera.size())) );

  assert(true == P->fWaiting);
  assert(true == pAcquisition->fWaiting);

  AcquireSRWLockExclusive( &(P->sLockAcquisitions) );
  {
    if (NULL != P->pAcquisitions)
      {
        assert(P->ProjectorID == pAcquisition->ProjectorID);
        P->pAcquisitions->push_back(pAcquisition);

        added = true;
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockAcquisitions) );

  assert(true == P->fWaiting);
  assert(true == pAcquisition->fWaiting);

  return added;
}
/* RenderingThreadAddCamera */



//! Remove camera from rendering thread.
/*!
  Removec camera from the rendering thread.

  \param P      Pointer to rendering thread parameters.
  \param pAcquisition   Pointer to acquisition thread to remove.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadRemoveCamera(
                            RenderingParameters * const P,
                            AcquisitionParameters * const pAcquisition
                            )
{
  bool removed = false;

  assert(NULL != pAcquisition);
  if (NULL == pAcquisition) return removed;

  assert(NULL != P);
  if (NULL == P) return removed;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return removed;

  int const CameraID = pAcquisition->CameraID;
  assert( (0 <= CameraID) && (CameraID < (int)(P->pSynchronization->Camera.size())) );

  assert(true == P->fWaiting);
  assert(true == pAcquisition->fWaiting);

  AcquireSRWLockExclusive( &(P->sLockAcquisitions) );
  {
    if (NULL != P->pAcquisitions)
      {
        int const num_cam = (int)( P->pAcquisitions->size() );
        assert(0 <= num_cam);

        for (int i = 0; i < num_cam; ++i)
          {
            AcquisitionParameters * const ith_pAcquisition = ( *(P->pAcquisitions) )[i];
            assert(NULL != ith_pAcquisition);
            if (NULL != ith_pAcquisition)
              {
                int const ith_CameraID = ith_pAcquisition->CameraID;
                if (ith_CameraID == CameraID)
                  {
                    assert(ith_pAcquisition == pAcquisition);
                    P->pAcquisitions->erase(P->pAcquisitions->begin() + i);
                    removed = true;
                    break;
                  }
                /* if */
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockAcquisitions) );

  assert(true == P->fWaiting);
  assert(true == pAcquisition->fWaiting);

  return removed;
}
/* RenderingThreadRemoveCamera */



//! Test if projector has attached cameras.
/*!
  Function tests if projector has attached cameras.

  \param P      Pointer to rendering thread parameters.
  \return Returns true if there is at least one attached camera.
*/
bool
RenderingThreadHaveCamera(
                          RenderingParameters * const P
                          )
{
  //assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  size_t num_cam = 0;
  AcquireSRWLockShared( &(P->sLockAcquisitions) );
  {
    if (NULL != P->pAcquisitions) num_cam = P->pAcquisitions->size();
  }
  ReleaseSRWLockShared( &(P->sLockAcquisitions) );
  bool const have_camera = (0 < num_cam);

  return have_camera;
}
/* RenderingThreadHaveCamera */

#pragma endregion // Add, remove or swap camera


#pragma region // Get, set, and change camera parameters

//! Return maximal exposure time.
/*!
  Function returns maximal exposure time of all attached acquisition threads (cameras).

  \param P      Pointer to rendering thread parameters.
  \return Returns exposure time in us (microseconds) or NaN if unsuccessfull.
*/
double
RenderingThreadGetMaxExposureTimeForAttachedCameras(
                                                    RenderingParameters * const P
                                                    )
{
  double exposureTime_max_us = BATCHACQUISITION_qNaN_dv;

  assert(NULL != P);
  if (NULL == P) return exposureTime_max_us;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return exposureTime_max_us;

  int const num_cam = (int)( P->pAcquisitions->size() );
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
      assert(NULL != pAcquisition);

      double exposureTime_us = pAcquisition->exposureTime_achieved_us;
      if (true == isnan_inline(exposureTime_us)) exposureTime_us = pAcquisition->exposureTime_requested_us;
      if (false == isnan_inline(exposureTime_max_us))
        {
          if (exposureTime_max_us > exposureTime_us) exposureTime_max_us = exposureTime_us;
        }
      else
        {
          exposureTime_max_us = exposureTime_us;
        }
      /* if */
    }
  /* for */

  return exposureTime_max_us;
}
/* RenderingThreadGetMaxExposureTimeForAttachedCameras */



//! Get output directory.
/*!
  Returns pointer to output directory of the first attached camera.

  \param P      Pointer to rendering thread parameters.
  \return Returns pointer to C string or NULL if unsuccessfull.
*/
TCHAR const *
RenderingThreadGetImageEncoderDirectory(
                                        RenderingParameters * const P
                                        )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return NULL;

  AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, 0);
  //assert(NULL != pAcquisition);
  if (NULL == pAcquisition) return NULL;

  TCHAR const * const pImageDirectory = ImageEncoderGetDirectory( pAcquisition->pImageEncoder );
  assert(NULL != pImageDirectory);

  return pImageDirectory;
}
/* RenderingThreadGetImageEncoderDirectory */



//! Set live preview for attached cameras.
/*!
  Function enables or disables live view for all attached cameras.

  \param P      Pointer to rendering thread parameters.
  \param fView  Flag which controls live preview. Set to true to enable, false to disable live view.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetLiveViewForAttachedCameras(
                                             RenderingParameters * const P,
                                             bool const fView
                                             )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  int const num_cam = (int)( P->pAcquisitions->size() );
  bool all_set = true;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
      assert(NULL != pAcquisition);
      if (NULL != pAcquisition)
        {
          pAcquisition->fView = fView;
          all_set = all_set && true;
        }
      else
        {
          all_set = false;
        }
      /* if */
    }
  /* for */

  return all_set;
}
/* RenderingThreadSetLiveViewForAttachedCameras */



//! Toggle live preview for attached cameras.
/*!
  Function changes the state of live view for all attached cameras.

  \param P      Pointer to rendering thread parameters.
  \param ptr_all_on  Address where a value indicating if live view is enabled for all cameras will be stored.
  \param ptr_all_off  Address where a value indicating if live view is disabled for all cameras will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadToggleLiveViewForAttachedCameras(
                                                RenderingParameters * const P,
                                                bool * const ptr_all_on,
                                                bool * const ptr_all_off
                                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  int const num_cam = (int)( P->pAcquisitions->size() );
  bool all_set = true;
  bool all_on = true;
  bool all_off = true;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
      assert(NULL != pAcquisition);
      if (NULL != pAcquisition)
        {
          pAcquisition->fView = !(pAcquisition->fView);
          all_set = all_set && true;
          all_on = all_on && (true == pAcquisition->fView);
          all_off = all_off && (false == pAcquisition->fView);
        }
      else
        {
          all_set = false;
          all_on = false;
          all_off = false;
        }
      /* if */
    }
  /* for */

  assert( !(all_off && all_on) );

  if (NULL != ptr_all_on) *ptr_all_on = all_on;
  if (NULL != ptr_all_off) *ptr_all_off = all_off;

  return all_set;
}
/* RenderingThreadToggleLiveViewForAttachedCameras */



//! Set CAMERA_READY event for attached cameras.
/*!
  Function sets CAMERA_READY event for all attached cameras.

  \param P      Pointer to rendering thread parameters.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetCameraReadyForAttachedCameras(
                                                RenderingParameters * const P
                                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  assert(NULL != P->pSynchronization);
  if (NULL == P->pSynchronization) return false;

  int const num_cam = (int)( P->pAcquisitions->size() );
  bool all_set = true;
  for (int i = 0; i < num_cam; ++i)
    {
      int const CameraID = nth_ID(P, i);
      assert( (0 <= CameraID) && (CameraID < (int)(P->pSynchronization->Camera.size())) );

      if (-1 != CameraID)
        {
          BOOL const set_camera_ready = P->pSynchronization->EventSet(CAMERA_READY, CameraID);
          assert(0 != set_camera_ready);

          all_set = all_set && (0 != set_camera_ready);
        }
      else
        {
          all_set = false;
        }
      /* if */
    }
  /* for */

  return all_set;
}
/* RenderingThreadSetCameraReadyForAttachedCameras */



//! Set directory for from file acquisition.
/*!
  Function sets input directory for dummy from file acquisition for all attached cameras
  which use from file acquisition.

  \param P      Pointer to rendering thread parameters.
  \param directory      Pointer to input directory to set.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetFromFileInputDirectory(
                                         RenderingParameters * const P,
                                         TCHAR const * const directory
                                         )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  int const num_cam = (int)( P->pAcquisitions->size() );
  bool all_set = true;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
      assert(NULL != pAcquisition);
      if (NULL != pAcquisition)
        {
          bool const setdir = AcquisitionParametersFromFileSetDirectory(pAcquisition->pFromFile, directory);
          assert(true == setdir);
          all_set = all_set && setdir;
        }
      else
        {
          all_set = false;
        }
      /* if */
    }
  /* for */

  return all_set;
}
/* RenderingThreadSetFromFileInputDirectory */



//! Set projector size for image encoders.
/*!
  Function resets image storage structure for all attached image encoders and
  copies supplied projector data which is used for acquisition.

  \param P      Pointer to rendering thread parameters.
  \param wnd_width      Window width (projector width).
  \param wnd_height     Window height (projector height).
  \param rcScreen       Screen rectangle.
  \param rcWindow       Window rectangle.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetProjectorSizeForImageEncoders(
                                                RenderingParameters * const P,
                                                int const wnd_width,
                                                int const wnd_height,
                                                RECT const rcScreen,
                                                RECT const rcWindow
                                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pAcquisitions);
  if (NULL == P->pAcquisitions) return false;

  int const num_cam = (int)( P->pAcquisitions->size() );
  bool all_set = true;
  for (int i = 0; i < num_cam; ++i)
    {
      AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
      assert(NULL != pAcquisition);
      if (NULL == pAcquisition)
        {
          all_set = false;
          continue;
        }
      /* if */

#ifdef _DEBUG
      if (NULL != pAcquisition->pMetadataQueue) assert(true == pAcquisition->pMetadataQueue->IsEmpty());
#endif /* _DEBUG */

      ImageEncoderParameters * const pImageEncoder = pAcquisition->pImageEncoder;
      assert(NULL != pImageEncoder);
      if (NULL == pImageEncoder)
        {
          all_set = false;
          continue;
        }
      /* if */

      assert( 0 == ImageEncoderBatchItemsRemaining(pImageEncoder) );
      assert( 0 == ImageEncoderTotalItemsRemaining(pImageEncoder) );

      if (NULL != pImageEncoder->pAllImages)
        {
          bool const reset = pImageEncoder->pAllImages->Reset();
          assert(true == reset);
          all_set = all_set && reset;

          pImageEncoder->pAllImages->window_width = wnd_width;
          pImageEncoder->pAllImages->window_height = wnd_height;
          pImageEncoder->pAllImages->rcScreen = rcScreen;
          pImageEncoder->pAllImages->rcWindow = rcWindow;
        }
      /* if */

    }
  /* for */

  return all_set;
}
/* RenderingThreadSetProjectorSizeForImageEncoders */

#pragma endregion // Get, set, and change camera parameters


#pragma region // Get, set, and change image decoder parameters

//! Get cycling flag.
/*!
  Get cycling flag of the data source for image decoder thread which is attached to the rendering thread.

  \param P      Pointer to rendering thread parameters.
  \param cycle  Address where the flag value will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadGetCycleFlagForImageDecoder(
                                           RenderingParameters * const P,
                                           bool * const cycle
                                           )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return false;

  if (NULL != cycle) *cycle = P->pImageDecoder->pImageList->cycle;

  return true;
}
/* RenderingThreadGetCycleFlagForImageDecoder */



//! Set cycling flag.
/*!
  Set cycling flag of the data source for image decoder thread which is attached to the rendering thread.

  \param P      Pointer to rendering thread parameters.
  \param cycle  Value to set.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetCycleFlagForImageDecoder(
                                           RenderingParameters * const P,
                                           bool const cycle
                                           )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return false;

  P->pImageDecoder->pImageList->cycle = cycle;

  return true;
}
/* RenderingThreadSetCycleFlagForImageDecoder */

#pragma endregion // Get, set, and change image decoder parameters


#pragma region // Set and rescan input directory

//! Set input directory.
/*!
  Function asks user to select new input directory.
  Function the clears all queued images from the image queue so projection can immedately switch to new images.

  \param P      Pointer to rendering thread parameters.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadAskUserToSetInputDirectory(
                                          RenderingParameters * const P
                                          )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return false;

  int const sz = 1024;
  wchar_t szTitle[sz + 1];
  int const cnt1 = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForProjector, P->ProjectorID + 1);
  assert(0 < cnt1);
  szTitle[sz] = 0;

  bool const readdir = P->pImageDecoder->pImageList->SetDirectory(NULL, szTitle);
  //assert(true == readdir);
  if (true == readdir)
    {
      AcquireSRWLockExclusive( &(P->pImageDecoder->sLockImageQueue) );
      {
        if (NULL != P->pImageDecoder->pImageQueue)
          {
            while ( !P->pImageDecoder->pImageQueue->empty() )
              {
                QueuedDecoderImage * item = ImageDecoderFetchImage(P->pImageDecoder, false);
                SAFE_DELETE( item );
              }
            /* while */
            assert( P->pImageDecoder->pImageQueue->empty() );
          }
        /* if */
      }
      ReleaseSRWLockExclusive( &(P->pImageDecoder->sLockImageQueue) );
    }
  /* if */

  return readdir;
}
/* RenderingThreadAskUserToSetInputDirectory */



//! Get input directory.
/*!
  Gets pointer to current input directory.
  The pointer is valid until the directory is changed.

  \return Pointer to current directory or NULL if unsuccessfull.
*/
wchar_t const *
RenderingThreadGetInputDirectory(
                                 RenderingParameters * const P
                                 )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return NULL;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return NULL;

  return P->pImageDecoder->pImageList->GetDirectory();
}
/* RenderingThreadGetInputDirectory */



//! Rescan input directory.
/*!
  Rescan input directory.

  \param P      Pointer to rendering thread parameters.
  \return Returns true if successfull, false otherwise.
*/
bool
RenderingThreadRescanInputDirectory(
                                    RenderingParameters * const P
                                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return false;

  if (NULL == P->pImageDecoder->pImageList->directory_name) return false;

  int const sz = 1024;
  wchar_t szTitle[sz + 1];
  int const cnt1 = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForProjector, P->ProjectorID + 1);
  assert(0 < cnt1);
  szTitle[sz] = 0;

  bool const rescan = P->pImageDecoder->pImageList->SetDirectory(P->pImageDecoder->pImageList->directory_name->c_str(), szTitle);
  assert(true == rescan);

  return rescan;
}
/* RenderingThreadRescanInputDirectory */

#pragma endregion // Set and rescan input directory


#pragma region // Get number of images to project

//! Get number of images to project.
/*!
  Returns number of images to project.

  \param P      Pointer to rendering thread parameters.
  \param pNumProject    Address where the number of images to project will be stored.
  \param pNumAcquire    Address where the number of images to acquire will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadGetNumberOfImagesToProjectAndAcquire(
                                                    RenderingParameters * const P,
                                                    int * const pNumProject,
                                                    int * const pNumAcquire
                                                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  int const num_cam = (NULL != P->pAcquisitions)? (int)( P->pAcquisitions->size() ) : 0;
  assert(0 <= num_cam);

  assert(NULL != P->pWindow);
  if (NULL == P->pWindow) return false;

  bool const fFixed = P->pWindow->fFixed; // True if fixed SL pattern is used.

  if (true == fFixed)
    {
      int const NumProject = 1;
      int const NumAcquire = num_cam * P->pWindow->num_acquire;
      if (NULL != pNumProject) *pNumProject = NumProject;
      if (NULL != pNumAcquire) *pNumAcquire = NumAcquire;
      return true;
    }
  /* if */

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pImageDecoder->pImageList);
  if (NULL == P->pImageDecoder->pImageList) return false;

  {
    int const NumProject = P->pImageDecoder->pImageList->Size();
    int const NumAcquire = num_cam * NumProject;
    if (NULL != pNumProject) *pNumProject = NumProject;
    if (NULL != pNumAcquire) *pNumAcquire = NumAcquire;
  }

  return true;
}
/* RenderingThreadGetNumberOfImagesToProject */

#pragma endregion // Get number of images to project


#pragma region // Change event IDs

//! Set new projector and encoder ID.
/*!
  Function sets new projector and encoder ID.
  Rendering and encoder threads must be in waiting state when this function is called;
  threads may be put into waiting state by signalling MAIN_PREPARE_DRAW to the rendering thread.

  \param P      Pointer to rendering thread parameters.
  \param ProjectorID    New projector ID.
  \param DecoderID      New decoder ID.
  \return Function returns true if successfull, false otherwise.
*/
bool
RenderingThreadSetNewProjectorIDAndDecoderID(
                                             RenderingParameters * const P,
                                             int const ProjectorID,
                                             int const DecoderID
                                             )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pSynchronization);
  if (NULL == P->pSynchronization) return false;

  assert(NULL != P->pImageDecoder);
  if (NULL == P->pImageDecoder) return false;

  assert(NULL != P->pWindow);
  if (NULL == P->pWindow) return false;

  assert(true == P->fWaiting);
  if (false == P->fWaiting) return false;

  int const ProjectorIDOld = P->ProjectorID;
  int const DecoderIDOld = P->pImageDecoder->DecoderID;

  assert( (0 <= ProjectorID) && (ProjectorID < (int)(P->pSynchronization->Draw.size())) );
  assert( (0 <= DecoderID) && (DecoderID <= (int)(P->pSynchronization->ImageDecoder.size())) );
  assert( ProjectorIDOld == P->pImageDecoder->ProjectorID );

  bool set = true;

  // Change event IDs.
  {
    P->ProjectorID = ProjectorID;
    P->pWindow->ProjectorID = ProjectorID;
    P->pImageDecoder->ProjectorID = ProjectorID;
    P->pImageDecoder->DecoderID = DecoderID;
  }

  // Signal to the threads to implement ID change.
  {
    assert(true == P->fWaiting);
    assert(true == P->pImageDecoder->fWaiting);

    BOOL const change_rendering = P->pSynchronization->EventSet(DRAW_CHANGE_ID, ProjectorIDOld);
    assert(0 != change_rendering);
    set == set && (0 != change_rendering);

    BOOL const change_decoder = P->pSynchronization->EventSet(IMAGE_DECODER_CHANGE_ID, DecoderIDOld);
    assert(0 != change_decoder);
    set == set && (0 != change_decoder);
  }

  // Wait for rendering thread to change event IDs.
  {
    bool rendering_changing = false;
    DWORD dwIsRenderingChangingResult = WAIT_FAILED;
    do
      {
        if (true == rendering_changing) SleepEx(1, TRUE);
        dwIsRenderingChangingResult = P->pSynchronization->EventWaitFor(DRAW_CHANGE_ID, ProjectorIDOld, (DWORD)0);
        rendering_changing = (WAIT_OBJECT_0 == dwIsRenderingChangingResult);
      }
    while (true == rendering_changing);
  }

  // Wait for decoder thread to change event IDs.
  {
    bool decoder_changing = false;
    DWORD dwIsDecoderChangingResult = WAIT_FAILED;
    do
      {
        if (true == decoder_changing) SleepEx(1, TRUE);
        dwIsDecoderChangingResult = P->pSynchronization->EventWaitFor(IMAGE_DECODER_CHANGE_ID, DecoderIDOld, (DWORD)0);
        decoder_changing = (WAIT_OBJECT_0 == dwIsDecoderChangingResult);
      }
    while (true == decoder_changing);
  }

  // Update projector ID for all slaved acquisition threads.
  assert(NULL != P->pAcquisitions);
  if (NULL != P->pAcquisitions)
    {
      int const num_cam = (int)( P->pAcquisitions->size() );
      for (int i = 0; i < num_cam; ++i)
        {
          AcquisitionParameters * const pAcquisition = nth_pAcquisition(P, i);
          assert(NULL != pAcquisition);

          bool const change_id = AcquisitionThreadSetNewProjectorID(pAcquisition, ProjectorID);
          assert(true == change_id);

          set = set && change_id;
        }
      /* for */
    }
  else
    {
      set = set && false;
    }
  /* if */

  return set;
}
/* RenderingThreadSetNewProjectorIDAndDecoderID */

#pragma endregion // Change event IDs


#pragma region // Auxiliary functions to query the rendering thread

//! Get unique projector (monitor) identifier.
/*!
  Gets string which uniquely identifies attached projector (monitor).
  Returned pointer must be deleted after unique projector identifer is no longer needed.

  \param P      Pointer to rendering parameters.
  \param Returns a pointer to wstring which uniquely identifies attached projector or NULL pointer otherwise.
*/
std::wstring *
GetUniqueProjectorIdentifier(
                             RenderingParameters * const P
                             )
{
  std::wstring * name = NULL;

  assert(NULL != P);
  if (NULL == P) return name;

  assert(NULL != P->pWindow);
  if (NULL == P->pWindow) return name;

  HMONITOR const hMonitor = MonitorFromWindow(P->pWindow->hWnd, MONITOR_DEFAULTTONULL);
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
          name = new std::wstring(monitorInfo.szDevice);
          assert(NULL != name);
        }
      /* if */
    }
  /* if */

  return name;
}
/* GetUniqueProjectorIdentifier */

#pragma endregion // Auxiliary functions to query the rendering thread


#endif /* !__BATCHACQUISITIONRENDERING_CPP */
