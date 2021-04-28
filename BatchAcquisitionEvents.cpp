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
  \file   BatchAcquisitionEvents.cpp
  \brief  Events for thread synchronization.

  Functions handling events for thread synchronization are defined here.

  \author Tomislav Petkovic
  \date   2017-02-06
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONEVENTS_CPP
#define __BATCHACQUISITIONEVENTS_CPP


#include "BatchAcquisitionEvents.h"




/****** HELPER FUNCTIONS ******/

//! Returns address of nth element.
/*!
  Returns address of nth element.

  \param v      Pointer to std::vector containing elements of type T.
  \param H      Element index.
*/
template <typename T>
inline T *
nth_ptr(
        std::vector<T> & v,
        int const H
        )
{
  if (true == v.empty()) return NULL;
  assert( (0 <= H) && (H < (int)(v.size())) );
  if ( (H < 0) || ((int)(v.size()) <= H) ) return NULL;
  return &(v[H]);
}
/* nth_ptr */



//! Creates synchronization event.
/*!
  Creates synchronization event.
  Event will not be created if the hnd_out is NULL.

  If the format is NULL event will not be named.
  If the format is non NULL then a named event is created.
  Note that each event must have unique name as if the event with the same name exists
  the OS will return a new handle to the existing event. To ensure per process unique event
  names a PID may be added to the name; then there are no conficts if the application is
  rune more than once. Note that only one instance of named events is available per application,
  that is a second call to this function will return handles to existing events.

  \param hnd_out Pointer to variable were the event handle for created event will be stored.
  \param format Format specifier for the event name.
  \return Function returns last error code, normally ERROR_SUCCESS if successfull.
*/
DWORD
CreateSynchronizationEvent(
                           HANDLE * const hnd_out,
                           TCHAR const * const format,
                           ...
                           )
{
  va_list argptr;

  int const bsize = 4096;
  TCHAR buffer1[bsize];
  TCHAR * buffer = buffer1;

  va_start(argptr, format);
  if (NULL != format)
    {
      int const count = vswprintf_s(buffer, bsize, format, argptr);
      assert(0 < count);
    }
  else
    {
      buffer = NULL;
    }
  /* if */
  va_end(argptr);

  SetLastError(ERROR_SUCCESS);

  if (NULL != hnd_out)
    {
      HANDLE const hnd =
        CreateEvent(
                    NULL,
                    TRUE, // Event must be manually reset.
                    FALSE, // State is set to nonsignaled.
                    buffer
                    );

      assert((HANDLE)(NULL) == *hnd_out);
      *hnd_out = hnd;
    }
  /* if */

  DWORD const error = GetLastError();
  assert(ERROR_INVALID_HANDLE != error);
  assert(ERROR_SUCCESS == error);

  return error;
}
/* CreateSynchronizationEvent */



/****** IMAGE DECODER THREAD ******/

//! Blanks events structure for image decoder thread.
/*!
  Function blanks SynchronizationEventsImageDecoder structure.

  \param P      Pointer to SynchronizationEventsImageDecoder structure.
*/
inline
void
SynchronizationEventsImageDecoderBlank_inline(
                                              SynchronizationEventsImageDecoder * const P
                                              )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->ImageDecoderQueueFull = (HANDLE)(NULL);
  P->ImageDecoderQueueEmpty = (HANDLE)(NULL);
  P->ImageDecoderQueueProcess = (HANDLE)(NULL);
  P->ImageDecoderQueueTerminate = (HANDLE)(NULL);
  P->ImageDecoderChangeID = (HANDLE)(NULL);

  P->CounterEventSetQueueFull = 0;
  P->CounterEventSetQueueEmpty = 0;
  P->CounterEventSetQueueProcess = 0;
  P->CounterEventSetQueueTerminate = 0;
  P->CounterEventSetChangeID = 0;

  P->CounterEventResetQueueFull = 0;
  P->CounterEventResetQueueEmpty = 0;
  P->CounterEventResetQueueProcess = 0;
  P->CounterEventResetQueueTerminate = 0;
  P->CounterEventResetChangeID = 0;

  P->StartCounterValueQueueFull = 0;
  P->StartCounterValueQueueEmpty = 0;
  P->StartCounterValueQueueProcess = 0;
  P->StartCounterValueQueueTerminate = 0;
  P->StartCounterValueChangeID = 0;
}
/* SynchronizationEventsImageDecoderBlank_inline */



//! Creates all events of the image decoder thread.
/*!
  Function creates named events and stores then in SynchronizationEventsImageDecoder structure.

  \param P      Pointer to SynchronizationEventsImageDecoder structure.
  \param ID     Current process ID.
  \param H      Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsImageDecoderCreate_inline(
                                               SynchronizationEventsImageDecoder * const P,
                                               int const ID,
                                               int const H
                                               )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  assert( 0 <= H );
  if (0 > H) return E_INVALIDARG;

  AcquireSRWLockExclusive( &(P->lock) );

  DWORD const ce00 = CreateSynchronizationEvent(&(P->ImageDecoderQueueFull), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_DECODER_QUEUE_FULL"), ID, H);
  DWORD const ce01 = CreateSynchronizationEvent(&(P->ImageDecoderQueueEmpty), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_DECODER_QUEUE_EMPTY"), ID, H);
  DWORD const ce02 = CreateSynchronizationEvent(&(P->ImageDecoderQueueProcess), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_DECODER_QUEUE_PROCESS"), ID, H);
  DWORD const ce03 = CreateSynchronizationEvent(&(P->ImageDecoderQueueTerminate), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_DECODER_QUEUE_TERMINATE"), ID, H);
  DWORD const ce04 = CreateSynchronizationEvent(&(P->ImageDecoderChangeID), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_DECODER_CHANGE_ID"), ID, H);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail0 = (ERROR_SUCCESS != ce00) || (ERROR_SUCCESS != ce01) || (ERROR_SUCCESS != ce02) || (ERROR_SUCCESS != ce03) || (ERROR_SUCCESS != ce04);
  assert(false == fail0);
  if (true == fail0) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsImageDecoderCreate_inline */




//! Closes all events of the image decoder thread.
/*!
  Function closes events stored in SynchronizationEventsImageDecoder structure.

  \param P      Pointer to SynchronizationEventsImageDecoder structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsImageDecoderClose_inline(
                                              SynchronizationEventsImageDecoder * const P
                                              )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  AcquireSRWLockExclusive( &(P->lock) );

  BOOL const ch00 = ( (HANDLE)(NULL) != P->ImageDecoderQueueFull ) ? CloseHandle(P->ImageDecoderQueueFull) : TRUE;
  BOOL const ch01 = ( (HANDLE)(NULL) != P->ImageDecoderQueueEmpty ) ? CloseHandle(P->ImageDecoderQueueEmpty) : TRUE;
  BOOL const ch02 = ( (HANDLE)(NULL) != P->ImageDecoderQueueProcess ) ? CloseHandle(P->ImageDecoderQueueProcess) : TRUE;
  BOOL const ch03 = ( (HANDLE)(NULL) != P->ImageDecoderQueueTerminate ) ? CloseHandle(P->ImageDecoderQueueTerminate) : TRUE;
  BOOL const ch04 = ( (HANDLE)(NULL) != P->ImageDecoderChangeID ) ? CloseHandle(P->ImageDecoderChangeID) : TRUE;

  P->ImageDecoderQueueFull = (HANDLE)(NULL);
  P->ImageDecoderQueueEmpty = (HANDLE)(NULL);
  P->ImageDecoderQueueProcess = (HANDLE)(NULL);
  P->ImageDecoderQueueTerminate = (HANDLE)(NULL);
  P->ImageDecoderChangeID = (HANDLE)(NULL);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail0 = (!ch00) || (!ch01) || (!ch02) || (!ch03) || (!ch04);
  assert(false == fail0);
  if (true == fail0) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsImageDecoderClose_inline */



//! Default constructor.
/*!
  Sets all event handles to NULL.
*/
SynchronizationEventsImageDecoder_::SynchronizationEventsImageDecoder_()
{
  SynchronizationEventsImageDecoderBlank_inline(this);
  InitializeSRWLock( &(this->lock) );
}
/* SynchronizationEventsImageDecoder_::SynchronizationEventsImageDecoder_ */



//! Copy constructor.
/*!
  Creates a copy of each handle.

  \param P      Reference to object to copy.
*/
SynchronizationEventsImageDecoder_::SynchronizationEventsImageDecoder_(
                                                                       const SynchronizationEventsImageDecoder_ & P
                                                                       )
{
  SynchronizationEventsImageDecoderBlank_inline(this);
  InitializeSRWLock( &(this->lock) );

  HANDLE const hProc = GetCurrentProcess();

  AcquireSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  BOOL const dh00 = ( (HANDLE)(NULL) != P.ImageDecoderQueueFull ) && ( (HANDLE)(NULL) == this->ImageDecoderQueueFull ) ?
    DuplicateHandle(hProc, P.ImageDecoderQueueFull, hProc, &(this->ImageDecoderQueueFull), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh01 = ( (HANDLE)(NULL) != P.ImageDecoderQueueEmpty ) && ( (HANDLE)(NULL) == this->ImageDecoderQueueEmpty ) ?
    DuplicateHandle(hProc, P.ImageDecoderQueueEmpty, hProc, &(this->ImageDecoderQueueEmpty), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh02 = ( (HANDLE)(NULL) != P.ImageDecoderQueueProcess ) && ( (HANDLE)(NULL) == this->ImageDecoderQueueProcess ) ?
    DuplicateHandle(hProc, P.ImageDecoderQueueProcess, hProc, &(this->ImageDecoderQueueProcess), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh03 = ( (HANDLE)(NULL) != P.ImageDecoderQueueTerminate ) && ( (HANDLE)(NULL) == this->ImageDecoderQueueTerminate ) ?
    DuplicateHandle(hProc, P.ImageDecoderQueueTerminate, hProc, &(this->ImageDecoderQueueTerminate), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh04 = ( (HANDLE)(NULL) != P.ImageDecoderChangeID ) && ( (HANDLE)(NULL) == this->ImageDecoderChangeID ) ?
    DuplicateHandle(hProc, P.ImageDecoderChangeID, hProc, &(this->ImageDecoderChangeID), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  this->CounterEventSetQueueFull = P.CounterEventSetQueueFull;
  this->CounterEventSetQueueEmpty = P.CounterEventSetQueueEmpty;
  this->CounterEventSetQueueProcess = P.CounterEventSetQueueProcess;
  this->CounterEventSetQueueTerminate = P.CounterEventSetQueueTerminate;
  this->CounterEventSetChangeID = P.CounterEventSetChangeID;

  this->CounterEventResetQueueFull = P.CounterEventResetQueueFull;
  this->CounterEventResetQueueEmpty = P.CounterEventResetQueueEmpty;
  this->CounterEventResetQueueProcess = P.CounterEventResetQueueProcess;
  this->CounterEventResetQueueTerminate = P.CounterEventResetQueueTerminate;
  this->CounterEventResetChangeID = P.CounterEventResetChangeID;

  this->StartCounterValueQueueFull = P.StartCounterValueQueueFull;
  this->StartCounterValueQueueEmpty = P.StartCounterValueQueueEmpty;
  this->StartCounterValueQueueProcess = P.StartCounterValueQueueProcess;
  this->StartCounterValueQueueTerminate = P.StartCounterValueQueueTerminate;
  this->StartCounterValueChangeID = P.StartCounterValueChangeID;

  ReleaseSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  bool const fail0 = (!dh00) || (!dh01) || (!dh02) || (!dh03) || (!dh04);
  assert(false == fail0);
}
/* SynchronizationEventsImageDecoder_::SynchronizationEventsImageDecoder_ */



//! Default destructor.
/*!
  Closes all non NULL handles.
*/
SynchronizationEventsImageDecoder_::~SynchronizationEventsImageDecoder_()
{
  HRESULT const res = SynchronizationEventsImageDecoderClose_inline(this);
  SynchronizationEventsImageDecoderBlank_inline(this);
}
/* SynchronizationEventsImageDecoder_::SynchronizationEventsImageDecoder_*/



/****** IMAGE ENCODER THREAD ******/

//! Blanks events structure for image encoder thread.
/*!
  Function blanks SynchronizationEventsImageEncoder structure.

  \param P      Pointer to SynchronizationEventsImageEncoder structure.
*/
inline
void
SynchronizationEventsImageEncoderBlank_inline(
                                              SynchronizationEventsImageEncoder * const P
                                              )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->ImageEncoderQueueFull = (HANDLE)(NULL);
  P->ImageEncoderQueueEmpty = (HANDLE)(NULL);
  P->ImageEncoderQueueProcess = (HANDLE)(NULL);
  P->ImageEncoderQueueTerminate = (HANDLE)(NULL);
  P->ImageEncoderChangeID = (HANDLE)(NULL);

  P->CounterEventSetQueueFull = 0;
  P->CounterEventSetQueueEmpty = 0;
  P->CounterEventSetQueueProcess = 0;
  P->CounterEventSetQueueTerminate = 0;
  P->CounterEventSetChangeID = 0;

  P->CounterEventResetQueueFull = 0;
  P->CounterEventResetQueueEmpty = 0;
  P->CounterEventResetQueueProcess = 0;
  P->CounterEventResetQueueTerminate = 0;
  P->CounterEventResetChangeID = 0;

  P->StartCounterValueQueueFull = 0;
  P->StartCounterValueQueueEmpty = 0;
  P->StartCounterValueQueueProcess = 0;
  P->StartCounterValueQueueTerminate = 0;
  P->StartCounterValueChangeID = 0;
}
/* SynchronizationEventsImageEncoderBlank_inline */




//! Creates all events of the image encoder thread.
/*!
  Function creates named events and stores then in SynchronizationEventsImageEncoder structure.

  \param P      Pointer to SynchronizationEventsImageEncoder structure.
  \param ID     Current process ID.
  \param H      Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsImageEncoderCreate_inline(
                                               SynchronizationEventsImageEncoder * const P,
                                               int const ID,
                                               int const H
                                               )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  assert( 0 <= H );
  if (0 > H) return E_INVALIDARG;

  AcquireSRWLockExclusive( &(P->lock) );

  DWORD const ce10 = CreateSynchronizationEvent(&(P->ImageEncoderQueueFull), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_ENCODER_QUEUE_FULL"), ID, H);
  DWORD const ce11 = CreateSynchronizationEvent(&(P->ImageEncoderQueueEmpty), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_ENCODER_QUEUE_EMPTY"), ID, H);
  DWORD const ce12 = CreateSynchronizationEvent(&(P->ImageEncoderQueueProcess), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_ENCODER_QUEUE_PROCESS"), ID, H);
  DWORD const ce13 = CreateSynchronizationEvent(&(P->ImageEncoderQueueTerminate), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_ENCODER_QUEUE_TERMINATE"), ID, H);
  DWORD const ce14 = CreateSynchronizationEvent(&(P->ImageEncoderChangeID), _T("Local\\PID_%d_H_%d_EVENT_IMAGE_ENCODER_CHANGE_ID"), ID, H);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail1 = (ERROR_SUCCESS != ce10) || (ERROR_SUCCESS != ce11) || (ERROR_SUCCESS != ce12) || (ERROR_SUCCESS != ce13) || (ERROR_SUCCESS != ce14);
  assert(false == fail1);
  if (true == fail1) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsImageEncoderCreate_inline */



//! Closes all events of the image encoder thread.
/*!
  Function closes events stored in SynchronizationEventsImageEncoder structure.

  \param P      Pointer to SynchronizationEventsImageEncoder structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsImageEncoderClose_inline(
                                              SynchronizationEventsImageEncoder * const P
                                              )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  AcquireSRWLockExclusive( &(P->lock) );

  BOOL const ch10 = ( (HANDLE)(NULL) != P->ImageEncoderQueueFull )? CloseHandle(P->ImageEncoderQueueFull) : TRUE;
  BOOL const ch11 = ( (HANDLE)(NULL) != P->ImageEncoderQueueEmpty )? CloseHandle(P->ImageEncoderQueueEmpty) : TRUE;
  BOOL const ch12 = ( (HANDLE)(NULL) != P->ImageEncoderQueueProcess )? CloseHandle(P->ImageEncoderQueueProcess) : TRUE;
  BOOL const ch13 = ( (HANDLE)(NULL) != P->ImageEncoderQueueTerminate )? CloseHandle(P->ImageEncoderQueueTerminate) : TRUE;
  BOOL const ch14 = ( (HANDLE)(NULL) != P->ImageEncoderChangeID )? CloseHandle(P->ImageEncoderChangeID) : TRUE;

  P->ImageEncoderQueueFull = (HANDLE)(NULL);
  P->ImageEncoderQueueEmpty = (HANDLE)(NULL);
  P->ImageEncoderQueueProcess = (HANDLE)(NULL);
  P->ImageEncoderQueueTerminate = (HANDLE)(NULL);
  P->ImageEncoderChangeID = (HANDLE)(NULL);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail1 = (!ch10) || (!ch11) || (!ch12) || (!ch13) || (!ch14);
  assert(false == fail1);
  if (true == fail1) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsImageEncoderClose_inline */



//! Default constructor.
/*!
  Sets all handles to NULL.
*/
SynchronizationEventsImageEncoder_::SynchronizationEventsImageEncoder_()
{
  SynchronizationEventsImageEncoderBlank_inline(this);
  InitializeSRWLock( &(this->lock) );
}
/* SynchronizationEventsImageEncoder_::SynchronizationEventsImageEncoder_ */



//! Copy constructor.
/*!
  Creates a copy of each handle.

  \param P      Reference to object to copy.
*/
SynchronizationEventsImageEncoder_::SynchronizationEventsImageEncoder_(
                                                                       const SynchronizationEventsImageEncoder_ & P
                                                                       )
{
  SynchronizationEventsImageEncoderBlank_inline(this);
  InitializeSRWLock( &(this->lock) );

  HANDLE const hProc = GetCurrentProcess();

  AcquireSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  BOOL const dh10 = ( (HANDLE)(NULL) != P.ImageEncoderQueueFull ) && ( (HANDLE)(NULL) == this->ImageEncoderQueueFull ) ?
    DuplicateHandle(hProc, P.ImageEncoderQueueFull, hProc, &(this->ImageEncoderQueueFull), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh11 = ( (HANDLE)(NULL) != P.ImageEncoderQueueEmpty ) && ( (HANDLE)(NULL) == this->ImageEncoderQueueEmpty ) ?
    DuplicateHandle(hProc, P.ImageEncoderQueueEmpty, hProc, &(this->ImageEncoderQueueEmpty), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh12 = ( (HANDLE)(NULL) != P.ImageEncoderQueueProcess ) && ( (HANDLE)(NULL) == this->ImageEncoderQueueProcess ) ?
    DuplicateHandle(hProc, P.ImageEncoderQueueProcess, hProc, &(this->ImageEncoderQueueProcess), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh13 = ( (HANDLE)(NULL) != P.ImageEncoderQueueTerminate ) && ( (HANDLE)(NULL) == this->ImageEncoderQueueTerminate ) ?
    DuplicateHandle(hProc, P.ImageEncoderQueueTerminate, hProc, &(this->ImageEncoderQueueTerminate), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh14 = ( (HANDLE)(NULL) != P.ImageEncoderChangeID ) && ( (HANDLE)(NULL) == this->ImageEncoderChangeID ) ?
    DuplicateHandle(hProc, P.ImageEncoderChangeID, hProc, &(this->ImageEncoderChangeID), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  this->CounterEventSetQueueFull = P.CounterEventSetQueueFull;
  this->CounterEventSetQueueEmpty = P.CounterEventSetQueueEmpty;
  this->CounterEventSetQueueProcess = P.CounterEventSetQueueProcess;
  this->CounterEventSetQueueTerminate = P.CounterEventSetQueueTerminate;
  this->CounterEventSetChangeID = P.CounterEventSetChangeID;

  this->CounterEventResetQueueFull = P.CounterEventResetQueueFull;
  this->CounterEventResetQueueEmpty = P.CounterEventResetQueueEmpty;
  this->CounterEventResetQueueProcess = P.CounterEventResetQueueProcess;
  this->CounterEventResetQueueTerminate = P.CounterEventResetQueueTerminate;
  this->CounterEventResetChangeID = P.CounterEventResetChangeID;

  this->StartCounterValueQueueFull = P.StartCounterValueQueueFull;
  this->StartCounterValueQueueEmpty = P.StartCounterValueQueueEmpty;
  this->StartCounterValueQueueProcess = P.StartCounterValueQueueProcess;
  this->StartCounterValueQueueTerminate = P.StartCounterValueQueueTerminate;
  this->StartCounterValueChangeID = P.StartCounterValueChangeID;

  ReleaseSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  bool const fail1 = (!dh10) || (!dh11) || (!dh12) || (!dh13) || (!dh14);
  assert(false == fail1);
}
/* SynchronizationEventsImageEncoder_::SynchronizationEventsImageEncoder_ */



//! Default destructor.
/*!
  Closes all non NULL handles.
*/
SynchronizationEventsImageEncoder_::~SynchronizationEventsImageEncoder_()
{
  HRESULT const res = SynchronizationEventsImageEncoderClose_inline(this);
  SynchronizationEventsImageEncoderBlank_inline(this);
}
/* SynchronizationEventsImageEncoder_::~SynchronizationEventsImageEncoder_ */



/****** DRAWING THREAD ******/

//! Blanks events structure for drawing thread.
/*!
  Function blanks SynchronizationEventsDraw structure.

  \param P      Pointer to SynchronizationEventsDraw structure.
*/
inline
void
SynchronizationEventsDrawBlank_inline(
                                      SynchronizationEventsDraw * const P
                                      )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->DrawPresent = (HANDLE)(NULL);
  P->DrawPresentReady = (HANDLE)(NULL);
  P->DrawRender = (HANDLE)(NULL);
  P->DrawRenderReady = (HANDLE)(NULL);
  P->DrawTerminate = (HANDLE)(NULL);
  P->DrawVBlank = (HANDLE)(NULL);
  P->DrawChangeID = (HANDLE)(NULL);

  P->CameraSyncTriggers = (HANDLE)(NULL);

  P->MainPrepareDraw = (HANDLE)(NULL);
  P->MainReadyDraw = (HANDLE)(NULL);
  P->MainBegin = (HANDLE)(NULL);
  P->MainEndDraw = (HANDLE)(NULL);
  P->MainResumeDraw = (HANDLE)(NULL);

  P->CounterEventSetPresent = 0;
  P->CounterEventSetPresentReady = 0;
  P->CounterEventSetRender = 0;
  P->CounterEventSetRenderReady = 0;
  P->CounterEventSetTerminate = 0;
  P->CounterEventSetVBlank = 0;
  P->CounterEventSetChangeID = 0;

  P->CounterEventSetSyncTriggers = 0;

  P->CounterEventSetPrepareDraw = 0;
  P->CounterEventSetReadyDraw = 0;
  P->CounterEventSetBegin = 0;
  P->CounterEventSetEndDraw = 0;
  P->CounterEventSetResumeDraw = 0;

  P->CounterEventResetPresent = 0;
  P->CounterEventResetPresentReady = 0;
  P->CounterEventResetRender = 0;
  P->CounterEventResetRenderReady = 0;
  P->CounterEventResetTerminate = 0;
  P->CounterEventResetVBlank = 0;
  P->CounterEventResetChangeID = 0;

  P->CounterEventResetSyncTriggers = 0;

  P->CounterEventResetPrepareDraw = 0;
  P->CounterEventResetReadyDraw = 0;
  P->CounterEventResetBegin = 0;
  P->CounterEventResetEndDraw = 0;
  P->CounterEventResetResumeDraw = 0;

  P->StartCounterValuePresent = 0;
  P->StartCounterValuePresentReady = 0;
  P->StartCounterValueRender = 0;
  P->StartCounterValueRenderReady = 0;
  P->StartCounterValueTerminate = 0;
  P->StartCounterValueVBlank = 0;
  P->StartCounterValueChangeID = 0;

  P->StartCounterValueSyncTriggers = 0;

  P->StartCounterValuePrepareDraw = 0;
  P->StartCounterValueReadyDraw = 0;
  P->StartCounterValueBegin = 0;
  P->StartCounterValueEndDraw = 0;
  P->StartCounterValueResumeDraw = 0;
}
/* SynchronizationEventsDrawBlank_inline */



//! Creates all events of the drawing thread.
/*!
  Function creates named events and stores then in SynchronizationEventsDraw structure.

  \param P      Pointer to SynchronizationEventsDraw structure.
  \param ID     Current process ID.
  \param H      Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsDrawCreate_inline(
                                       SynchronizationEventsDraw * const P,
                                       int const ID,
                                       int const H
                                       )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  assert( 0 <= H );
  if (0 > H) return E_INVALIDARG;

  AcquireSRWLockExclusive( &(P->lock) );

  DWORD const ce20 = CreateSynchronizationEvent(&(P->DrawPresent), _T("Local\\PID_%d_H_%d_EVENT_DRAW_PRESENT"), ID, H);
  DWORD const ce21 = CreateSynchronizationEvent(&(P->DrawPresentReady), _T("Local\\PID_%d_H_%d_EVENT_DRAW_PRESENT_READY"), ID, H);
  DWORD const ce22 = CreateSynchronizationEvent(&(P->DrawRender), _T("Local\\PID_%d_H_%d_EVENT_DRAW_RENDER"), ID, H);
  DWORD const ce23 = CreateSynchronizationEvent(&(P->DrawRenderReady), _T("Local\\PID_%d_H_%d_EVENT_DRAW_RENDER_READY"), ID, H);
  DWORD const ce24 = CreateSynchronizationEvent(&(P->DrawTerminate), _T("Local\\PID_%d_H_%d_EVENT_DRAW_TERMINATE"), ID, H);
  DWORD const ce25 = CreateSynchronizationEvent(&(P->DrawVBlank), _T("Local\\PID_%d_H_%d_EVENT_DRAW_VBLANK"), ID, H);
  DWORD const ce26 = CreateSynchronizationEvent(&(P->DrawChangeID), _T("Local\\PID_%d_H_%d_EVENT_DRAW_CHANGE_ID"), ID, H);

  DWORD const ce3A = CreateSynchronizationEvent(&(P->CameraSyncTriggers), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_SYNC_TRIGGERS"), ID, H);

  DWORD const ce40 = CreateSynchronizationEvent(&(P->MainPrepareDraw), _T("Local\\PID_%d_H_%d_EVENT_MAIN_PREPARE_DRAW"), ID, H);
  DWORD const ce42 = CreateSynchronizationEvent(&(P->MainReadyDraw), _T("Local\\PID_%d_H_%d_EVENT_MAIN_READY_DRAW"), ID, H);
  DWORD const ce44 = CreateSynchronizationEvent(&(P->MainBegin), _T("Local\\PID_%d_H_%d_EVENT_MAIN_BEGIN"), ID, H);
  DWORD const ce45 = CreateSynchronizationEvent(&(P->MainEndDraw), _T("Local\\PID_%d_H_%d_EVENT_MAIN_END_DRAW"), ID, H);
  DWORD const ce47 = CreateSynchronizationEvent(&(P->MainResumeDraw), _T("Local\\PID_%d_H_%d_EVENT_MAIN_RESUME_DRAW"), ID, H);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail2 =
    (ERROR_SUCCESS != ce20) || (ERROR_SUCCESS != ce21) || (ERROR_SUCCESS != ce22) || (ERROR_SUCCESS != ce23) ||
    (ERROR_SUCCESS != ce24) || (ERROR_SUCCESS != ce25) || (ERROR_SUCCESS != ce26);
  assert(false == fail2);
  bool const fail3 =
    (ERROR_SUCCESS != ce3A);
  assert(false == fail3);
  bool const fail4 =
    (ERROR_SUCCESS != ce40) || (ERROR_SUCCESS != ce42) || (ERROR_SUCCESS != ce44) || (ERROR_SUCCESS != ce45) ||
    (ERROR_SUCCESS != ce47);
  assert(false == fail4);
  if ( (true == fail2) || (true == fail3) || (true == fail4) ) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsDrawCreate_inline */



//! Closes all events of the drawing thread.
/*!
  Function closes events stored in SynchronizationEventsDraw structure.

  \param P      Pointer to SynchronizationEventsDraw structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsDrawClose_inline(
                                      SynchronizationEventsDraw * const P
                                      )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  AcquireSRWLockExclusive( &(P->lock) );

  BOOL const ch20 = ( (HANDLE)(NULL) != P->DrawPresent )? CloseHandle(P->DrawPresent) : TRUE;
  BOOL const ch21 = ( (HANDLE)(NULL) != P->DrawPresentReady )? CloseHandle(P->DrawPresentReady) : TRUE;
  BOOL const ch22 = ( (HANDLE)(NULL) != P->DrawRender )? CloseHandle(P->DrawRender) : TRUE;
  BOOL const ch23 = ( (HANDLE)(NULL) != P->DrawRenderReady )? CloseHandle(P->DrawRenderReady) : TRUE;
  BOOL const ch24 = ( (HANDLE)(NULL) != P->DrawTerminate )? CloseHandle(P->DrawTerminate) : TRUE;
  BOOL const ch25 = ( (HANDLE)(NULL) != P->DrawVBlank )? CloseHandle(P->DrawVBlank) : TRUE;
  BOOL const ch26 = ( (HANDLE)(NULL) != P->DrawChangeID )? CloseHandle(P->DrawChangeID) : TRUE;

  BOOL const ch3A = ( (HANDLE)(NULL) != P->CameraSyncTriggers )? CloseHandle(P->CameraSyncTriggers) : TRUE;

  BOOL const ch40 = ( (HANDLE)(NULL) != P->MainPrepareDraw )? CloseHandle(P->MainPrepareDraw) : TRUE;
  BOOL const ch42 = ( (HANDLE)(NULL) != P->MainReadyDraw )? CloseHandle(P->MainReadyDraw) : TRUE;
  BOOL const ch44 = ( (HANDLE)(NULL) != P->MainBegin )? CloseHandle(P->MainBegin) : TRUE;
  BOOL const ch45 = ( (HANDLE)(NULL) != P->MainEndDraw )? CloseHandle(P->MainEndDraw) : TRUE;
  BOOL const ch47 = ( (HANDLE)(NULL) != P->MainResumeDraw )? CloseHandle(P->MainResumeDraw) : TRUE;

  P->DrawPresent = (HANDLE)(NULL);
  P->DrawPresentReady = (HANDLE)(NULL);
  P->DrawRender = (HANDLE)(NULL);
  P->DrawRenderReady = (HANDLE)(NULL);
  P->DrawTerminate = (HANDLE)(NULL);
  P->DrawVBlank = (HANDLE)(NULL);
  P->DrawChangeID = (HANDLE)(NULL);

  P->CameraSyncTriggers = (HANDLE)(NULL);

  P->MainPrepareDraw = (HANDLE)(NULL);
  P->MainReadyDraw = (HANDLE)(NULL);
  P->MainBegin = (HANDLE)(NULL);
  P->MainEndDraw = (HANDLE)(NULL);
  P->MainResumeDraw = (HANDLE)(NULL);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail2 = (!ch20) || (!ch21) || (!ch22) || (!ch23) || (!ch24) || (!ch25) || (!ch26);
  assert(false == fail2);
  bool const fail3 = (!ch3A);
  assert(false == fail3);
  bool const fail4 = (!ch40) || (!ch42) || (!ch44) || (!ch45) || (!ch47);
  assert(false == fail4);
  if ( (true == fail2) || (true == fail3) || (true == fail4) ) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsDrawClose_inline */



//! Default constructor.
/*!
  Sets all handles to NULL.
*/
SynchronizationEventsDraw_::SynchronizationEventsDraw_()
{
  SynchronizationEventsDrawBlank_inline(this);
  InitializeSRWLock( &(this->lock) );
}
/* SynchronizationEventsDraw_::SynchronizationEventsDraw_ */



//! Copy constructor.
/*!
  Creates a copy of each handle.

  \param P      Reference to object to copy.
*/
SynchronizationEventsDraw_::SynchronizationEventsDraw_(
                                                       const SynchronizationEventsDraw_ & P
                                                       )
{
  SynchronizationEventsDrawBlank_inline(this);
  InitializeSRWLock( &(this->lock) );

  HANDLE const hProc = GetCurrentProcess();

  AcquireSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  BOOL const dh20 = ( (HANDLE)(NULL) != P.DrawPresent ) && ( (HANDLE)(NULL) == this->DrawPresent ) ?
    DuplicateHandle(hProc, P.DrawPresent, hProc, &(this->DrawPresent), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh21 = ( (HANDLE)(NULL) != P.DrawPresentReady ) && ( (HANDLE)(NULL) == this->DrawPresentReady ) ?
    DuplicateHandle(hProc, P.DrawPresentReady, hProc, &(this->DrawPresentReady), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh22 = ( (HANDLE)(NULL) != P.DrawRender ) && ( (HANDLE)(NULL) == this->DrawRender ) ?
    DuplicateHandle(hProc, P.DrawRender, hProc, &(this->DrawRender), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh23 = ( (HANDLE)(NULL) != P.DrawRenderReady ) && ( (HANDLE)(NULL) == this->DrawRenderReady ) ?
    DuplicateHandle(hProc, P.DrawRenderReady, hProc, &(this->DrawRenderReady), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh24 = ( (HANDLE)(NULL) != P.DrawTerminate ) && ( (HANDLE)(NULL) == this->DrawTerminate ) ?
    DuplicateHandle(hProc, P.DrawTerminate, hProc, &(this->DrawTerminate), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh25 = ( (HANDLE)(NULL) != P.DrawVBlank ) && ( (HANDLE)(NULL) == this->DrawVBlank ) ?
    DuplicateHandle(hProc, P.DrawVBlank, hProc, &(this->DrawVBlank), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh26 = ( (HANDLE)(NULL) != P.DrawChangeID ) && ( (HANDLE)(NULL) == this->DrawChangeID ) ?
    DuplicateHandle(hProc, P.DrawChangeID, hProc, &(this->DrawChangeID), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  BOOL const dh3A = ( (HANDLE)(NULL) != P.CameraSyncTriggers ) && ( (HANDLE)(NULL) == this->CameraSyncTriggers ) ?
    DuplicateHandle(hProc, P.CameraSyncTriggers, hProc, &(this->CameraSyncTriggers), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  BOOL const dh40 = ( (HANDLE)(NULL) != P.MainPrepareDraw ) && ( (HANDLE)(NULL) == this->MainPrepareDraw ) ?
    DuplicateHandle(hProc, P.MainPrepareDraw, hProc, &(this->MainPrepareDraw), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh42 = ( (HANDLE)(NULL) != P.MainReadyDraw ) && ( (HANDLE)(NULL) == this->MainReadyDraw ) ?
    DuplicateHandle(hProc, P.MainReadyDraw, hProc, &(this->MainReadyDraw), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh44 = ( (HANDLE)(NULL) != P.MainBegin ) && ( (HANDLE)(NULL) == this->MainBegin ) ?
    DuplicateHandle(hProc, P.MainBegin, hProc, &(this->MainBegin), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh45 = ( (HANDLE)(NULL) != P.MainEndDraw ) && ( (HANDLE)(NULL) == this->MainEndDraw ) ?
    DuplicateHandle(hProc, P.MainEndDraw, hProc, &(this->MainEndDraw), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh47 = ( (HANDLE)(NULL) != P.MainResumeDraw ) && ( (HANDLE)(NULL) == this->MainResumeDraw ) ?
    DuplicateHandle(hProc, P.MainResumeDraw, hProc, &(this->MainResumeDraw), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  this->CounterEventSetPresent = P.CounterEventSetPresent;
  this->CounterEventSetPresentReady = P.CounterEventSetPresentReady;
  this->CounterEventSetRender = P.CounterEventSetRender;
  this->CounterEventSetRenderReady = P.CounterEventSetRenderReady;
  this->CounterEventSetTerminate = P.CounterEventSetTerminate;
  this->CounterEventSetVBlank = P.CounterEventSetVBlank;
  this->CounterEventSetChangeID = P.CounterEventSetChangeID;

  this->CounterEventSetSyncTriggers = P.CounterEventSetSyncTriggers;

  this->CounterEventSetPrepareDraw = P.CounterEventSetPrepareDraw;
  this->CounterEventSetReadyDraw = P.CounterEventSetReadyDraw;
  this->CounterEventSetBegin = P.CounterEventSetBegin;
  this->CounterEventSetEndDraw = P.CounterEventSetEndDraw;
  this->CounterEventSetResumeDraw = P.CounterEventSetResumeDraw;

  this->CounterEventResetPresent = P.CounterEventResetPresent;
  this->CounterEventResetPresentReady = P.CounterEventResetPresentReady;
  this->CounterEventResetRender = P.CounterEventResetRender;
  this->CounterEventResetRenderReady = P.CounterEventResetRenderReady;
  this->CounterEventResetTerminate = P.CounterEventResetTerminate;
  this->CounterEventResetVBlank = P.CounterEventResetVBlank;
  this->CounterEventResetChangeID = P.CounterEventResetChangeID;

  this->CounterEventResetSyncTriggers = P.CounterEventResetSyncTriggers;

  this->CounterEventResetPrepareDraw = P.CounterEventResetPrepareDraw;
  this->CounterEventResetReadyDraw = P.CounterEventResetReadyDraw;
  this->CounterEventResetBegin = P.CounterEventResetBegin;
  this->CounterEventResetEndDraw = P.CounterEventResetEndDraw;
  this->CounterEventResetResumeDraw = P.CounterEventResetResumeDraw;

  this->StartCounterValuePresent = P.StartCounterValuePresent;
  this->StartCounterValuePresentReady = P.StartCounterValuePresentReady;
  this->StartCounterValueRender = P.StartCounterValueRender;
  this->StartCounterValueRenderReady = P.StartCounterValueRenderReady;
  this->StartCounterValueTerminate = P.StartCounterValueTerminate;
  this->StartCounterValueVBlank = P.StartCounterValueVBlank;
  this->StartCounterValueChangeID = P.StartCounterValueChangeID;

  this->StartCounterValueSyncTriggers = P.StartCounterValueSyncTriggers;

  this->StartCounterValuePrepareDraw = P.StartCounterValuePrepareDraw;
  this->StartCounterValueReadyDraw = P.StartCounterValueReadyDraw;
  this->StartCounterValueBegin = P.StartCounterValueBegin;
  this->StartCounterValueEndDraw = P.StartCounterValueEndDraw;
  this->StartCounterValueResumeDraw = P.StartCounterValueResumeDraw;

  ReleaseSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  bool const fail2 = (!dh20) || (!dh21) || (!dh22) || (!dh23) || (!dh24) || (!dh25) || (!dh26);
  assert(false == fail2);
  bool const fail3 = (!dh3A);
  assert(false == fail3);
  bool const fail4 = (!dh40) || (!dh42) || (!dh44) || (!dh45) || (!dh47);
  assert(false == fail4);
}
/* SynchronizationEventsDraw_::SynchronizationEventsDraw_ */



//! Default destructor.
/*!
  Closes all non NULL handles.
*/
SynchronizationEventsDraw_::~SynchronizationEventsDraw_()
{
  HRESULT const res = SynchronizationEventsDrawClose_inline(this);
  SynchronizationEventsDrawBlank_inline(this);
}
/* SynchronizationEventsDraw_::~SynchronizationEventsDraw_ */



/****** ACQUISITION THREAD ******/

//! Blanks events structure for image acquisition thread.
/*!
  Function blanks SynchronizationEventsCamera structure.

  \param P      Pointer to SynchronizationEventsCamera structure.
*/
inline
void
SynchronizationEventsCameraBlank_inline(
                                        SynchronizationEventsCamera * const P
                                        )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->CameraSendTrigger = (HANDLE)(NULL);
  P->CameraRepeatTrigger = (HANDLE)(NULL);
  P->CameraExposureBegin = (HANDLE)(NULL);
  P->CameraExposureEnd = (HANDLE)(NULL);
  P->CameraReadoutBegin = (HANDLE)(NULL);
  P->CameraReadoutEnd = (HANDLE)(NULL);
  P->CameraTransferBegin = (HANDLE)(NULL);
  P->CameraTransferEnd = (HANDLE)(NULL);
  P->CameraTerminate = (HANDLE)(NULL);
  P->CameraReady = (HANDLE)(NULL);
  P->CameraInvalidTrigger = (HANDLE)(NULL);
  P->CameraChangeID = (HANDLE)(NULL);

  P->MainPrepareCamera = (HANDLE)(NULL);
  P->MainReadyCamera = (HANDLE)(NULL);
  P->MainEndCamera = (HANDLE)(NULL);

  P->CounterEventSetSendTrigger = 0;
  P->CounterEventSetRepeatTrigger = 0;
  P->CounterEventSetExposureBegin = 0;
  P->CounterEventSetExposureEnd = 0;
  P->CounterEventSetReadoutBegin = 0;
  P->CounterEventSetReadoutEnd = 0;
  P->CounterEventSetTransferBegin = 0;
  P->CounterEventSetTransferEnd = 0;
  P->CounterEventSetTerminate = 0;
  P->CounterEventSetReady = 0;
  P->CounterEventSetInvalidTrigger = 0;
  P->CounterEventSetChangeID = 0;

  P->CounterEventSetPrepareCamera = 0;
  P->CounterEventSetReadyCamera = 0;
  P->CounterEventSetEndCamera = 0;

  P->CounterEventResetSendTrigger = 0;
  P->CounterEventResetRepeatTrigger = 0;
  P->CounterEventResetExposureBegin = 0;
  P->CounterEventResetExposureEnd = 0;
  P->CounterEventResetReadoutBegin = 0;
  P->CounterEventResetReadoutEnd = 0;
  P->CounterEventResetTransferBegin = 0;
  P->CounterEventResetTransferEnd = 0;
  P->CounterEventResetTerminate = 0;
  P->CounterEventResetReady = 0;
  P->CounterEventResetInvalidTrigger = 0;
  P->CounterEventResetChangeID = 0;

  P->CounterEventResetPrepareCamera = 0;
  P->CounterEventResetReadyCamera = 0;
  P->CounterEventResetEndCamera = 0;

  P->StartCounterValueSendTrigger = 0;
  P->StartCounterValueRepeatTrigger = 0;
  P->StartCounterValueExposureBegin = 0;
  P->StartCounterValueExposureEnd = 0;
  P->StartCounterValueReadoutBegin = 0;
  P->StartCounterValueReadoutEnd = 0;
  P->StartCounterValueTransferBegin = 0;
  P->StartCounterValueTransferEnd = 0;
  P->StartCounterValueTerminate = 0;
  P->StartCounterValueReady = 0;
  P->StartCounterValueInvalidTrigger = 0;
  P->StartCounterValueChangeID = 0;

  P->StartCounterValuePrepareCamera = 0;
  P->StartCounterValueReadyCamera = 0;
  P->StartCounterValueEndCamera = 0;
}
/* SynchronizationEventsCameraBlank_inline */




//! Creates all events of the acquisition  thread.
/*!
  Function creates named events and stores then in SynchronizationEventsCamera structure.

  \param P      Pointer to SynchronizationEventsCamera structure.
  \param ID     Current process ID.
  \param H      Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsCameraCreate_inline(
                                         SynchronizationEventsCamera * const P,
                                         int const ID,
                                         int const H
                                         )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  assert( 0 <= H );
  if (0 > H) return E_INVALIDARG;

  AcquireSRWLockExclusive( &(P->lock) );

  DWORD const ce30 = CreateSynchronizationEvent(&(P->CameraSendTrigger), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_SEND_TRIGGER"), ID, H);
  DWORD const ce31 = CreateSynchronizationEvent(&(P->CameraRepeatTrigger), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_REPEAT_TRIGGER"), ID, H);
  DWORD const ce32 = CreateSynchronizationEvent(&(P->CameraExposureBegin), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_EXPOSURE_BEGIN"), ID, H);
  DWORD const ce33 = CreateSynchronizationEvent(&(P->CameraExposureEnd), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_EXPOSURE_END"), ID, H);
  DWORD const ce34 = CreateSynchronizationEvent(&(P->CameraReadoutBegin), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_READOUT_BEGIN"), ID, H);
  DWORD const ce35 = CreateSynchronizationEvent(&(P->CameraReadoutEnd), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_READOUT_END"), ID, H);
  DWORD const ce36 = CreateSynchronizationEvent(&(P->CameraTransferBegin), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_TRANSFER_BEGIN"), ID, H);
  DWORD const ce37 = CreateSynchronizationEvent(&(P->CameraTransferEnd), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_TRANSFER_END"), ID, H);
  DWORD const ce38 = CreateSynchronizationEvent(&(P->CameraTerminate), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_TERMINATE"), ID, H);
  DWORD const ce39 = CreateSynchronizationEvent(&(P->CameraReady), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_READY"), ID, H);
  // CAMERA_SYNC_TRIGGERS (3A) is located in SynchronizationEventsDraw
  DWORD const ce3B = CreateSynchronizationEvent(&(P->CameraInvalidTrigger), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_INVALID_TRIGGER"), ID, H);
  DWORD const ce3C = CreateSynchronizationEvent(&(P->CameraChangeID), _T("Local\\PID_%d_H_%d_EVENT_CAMERA_CHANGE_ID"), ID, H);

  DWORD const ce41 = CreateSynchronizationEvent(&(P->MainPrepareCamera), _T("Local\\PID_%d_H_%d_EVENT_MAIN_PREPARE_CAMERA"), ID, H);
  DWORD const ce43 = CreateSynchronizationEvent(&(P->MainReadyCamera), _T("Local\\PID_%d_H_%d_EVENT_MAIN_READY_CAMERA"), ID, H);
  DWORD const ce46 = CreateSynchronizationEvent(&(P->MainEndCamera), _T("Local\\PID_%d_H_%d_EVENT_MAIN_END_CAMERA"), ID, H);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail3 =
    (ERROR_SUCCESS != ce30) || (ERROR_SUCCESS != ce31) || (ERROR_SUCCESS != ce32) || (ERROR_SUCCESS != ce33) ||
    (ERROR_SUCCESS != ce34) || (ERROR_SUCCESS != ce35) || (ERROR_SUCCESS != ce36) || (ERROR_SUCCESS != ce37) ||
    (ERROR_SUCCESS != ce38) || (ERROR_SUCCESS != ce39) || (ERROR_SUCCESS != ce3B) || (ERROR_SUCCESS != ce3C);
  assert(false == fail3);
  bool const fail4 =
    (ERROR_SUCCESS != ce41) || (ERROR_SUCCESS != ce43) || (ERROR_SUCCESS != ce46);
  assert(false == fail4);
  if ( (true == fail3) || (true == fail4) ) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsCameraCreate_inline */



//! Closes all events of the acquisition  thread.
/*!
  Function closes events stored in SynchronizationEventsCamera structure.

  \param P      Pointer to SynchronizationEventsCamera structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsCameraClose_inline(
                                        SynchronizationEventsCamera * const P
                                        )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  AcquireSRWLockExclusive( &(P->lock) );

  BOOL const ch30 = ( (HANDLE)(NULL) != P->CameraSendTrigger )? CloseHandle(P->CameraSendTrigger) : TRUE;
  BOOL const ch31 = ( (HANDLE)(NULL) != P->CameraRepeatTrigger )? CloseHandle(P->CameraRepeatTrigger) : TRUE;
  BOOL const ch32 = ( (HANDLE)(NULL) != P->CameraExposureBegin )? CloseHandle(P->CameraExposureBegin) : TRUE;
  BOOL const ch33 = ( (HANDLE)(NULL) != P->CameraExposureEnd )? CloseHandle(P->CameraExposureEnd) : TRUE;
  BOOL const ch34 = ( (HANDLE)(NULL) != P->CameraReadoutBegin )? CloseHandle(P->CameraReadoutBegin) : TRUE;
  BOOL const ch35 = ( (HANDLE)(NULL) != P->CameraReadoutEnd )? CloseHandle(P->CameraReadoutEnd) : TRUE;
  BOOL const ch36 = ( (HANDLE)(NULL) != P->CameraTransferBegin )? CloseHandle(P->CameraTransferBegin) : TRUE;
  BOOL const ch37 = ( (HANDLE)(NULL) != P->CameraTransferEnd )? CloseHandle(P->CameraTransferEnd) : TRUE;
  BOOL const ch38 = ( (HANDLE)(NULL) != P->CameraTerminate )? CloseHandle(P->CameraTerminate) : TRUE;
  BOOL const ch39 = ( (HANDLE)(NULL) != P->CameraReady )? CloseHandle(P->CameraReady) : TRUE;
  // CAMERA_SYNC_TRIGGERS (3A) is located in SynchronizationEventsDraw
  BOOL const ch3B = ( (HANDLE)(NULL) != P->CameraInvalidTrigger )? CloseHandle(P->CameraInvalidTrigger) : TRUE;
  BOOL const ch3C = ( (HANDLE)(NULL) != P->CameraChangeID )? CloseHandle(P->CameraChangeID) : TRUE;

  BOOL const ch41 = ( (HANDLE)(NULL) != P->MainPrepareCamera )? CloseHandle(P->MainPrepareCamera) : TRUE;
  BOOL const ch43 = ( (HANDLE)(NULL) != P->MainReadyCamera )? CloseHandle(P->MainReadyCamera) : TRUE;
  BOOL const ch46 = ( (HANDLE)(NULL) != P->MainEndCamera )? CloseHandle(P->MainEndCamera) : TRUE;

  P->CameraSendTrigger = (HANDLE)(NULL);
  P->CameraRepeatTrigger = (HANDLE)(NULL);
  P->CameraExposureBegin = (HANDLE)(NULL);
  P->CameraExposureEnd = (HANDLE)(NULL);
  P->CameraReadoutBegin = (HANDLE)(NULL);
  P->CameraReadoutEnd = (HANDLE)(NULL);
  P->CameraTransferBegin = (HANDLE)(NULL);
  P->CameraTransferEnd = (HANDLE)(NULL);
  P->CameraTerminate = (HANDLE)(NULL);
  P->CameraReady = (HANDLE)(NULL);
  P->CameraInvalidTrigger = (HANDLE)(NULL);
  P->CameraChangeID = (HANDLE)(NULL);

  P->MainPrepareCamera = (HANDLE)(NULL);
  P->MainReadyCamera = (HANDLE)(NULL);
  P->MainEndCamera = (HANDLE)(NULL);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail3 = (!ch30) || (!ch31) || (!ch32) || (!ch33) || (!ch34) || (!ch35) || (!ch36) || (!ch37) || (!ch38) || (!ch39) || (!ch3B) || (!ch3C);
  assert(false == fail3);
  bool const fail4 = (!ch41) || (!ch43) || (!ch46);
  assert(false == fail4);
  if ( (true == fail3) || (true == fail4) ) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsCameraClose_inline */



//! Default constructor.
/*!
  Sets all handles to NULL.
*/
SynchronizationEventsCamera_::SynchronizationEventsCamera_()
{
  SynchronizationEventsCameraBlank_inline(this);
  InitializeSRWLock( &(this->lock) );
}
/* SynchronizationEventsCamera_::SynchronizationEventsCamera_ */



//! Copy constructor.
/*!
  Creates copies of all valid handles.

  \param P      Reference to object to copy.
*/
SynchronizationEventsCamera_::SynchronizationEventsCamera_(
                                                           const SynchronizationEventsCamera_ & P
                                                           )
{
  SynchronizationEventsCameraBlank_inline(this);
  InitializeSRWLock( &(this->lock) );

  HANDLE const hProc = GetCurrentProcess();

  AcquireSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  BOOL const dh30 = ( (HANDLE)(NULL) != P.CameraSendTrigger ) && ( (HANDLE)(NULL) == this->CameraSendTrigger ) ?
    DuplicateHandle(hProc, P.CameraSendTrigger, hProc, &(this->CameraSendTrigger), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh31 = ( (HANDLE)(NULL) != P.CameraRepeatTrigger ) && ( (HANDLE)(NULL) == this->CameraRepeatTrigger ) ?
    DuplicateHandle(hProc, P.CameraRepeatTrigger, hProc, &(this->CameraRepeatTrigger), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh32 = ( (HANDLE)(NULL) != P.CameraExposureBegin ) && ( (HANDLE)(NULL) == this->CameraExposureBegin ) ?
    DuplicateHandle(hProc, P.CameraExposureBegin, hProc, &(this->CameraExposureBegin), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh33 = ( (HANDLE)(NULL) != P.CameraExposureEnd ) && ( (HANDLE)(NULL) == this->CameraExposureEnd ) ?
    DuplicateHandle(hProc, P.CameraExposureEnd, hProc, &(this->CameraExposureEnd), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh34 = ( (HANDLE)(NULL) != P.CameraReadoutBegin ) && ( (HANDLE)(NULL) == this->CameraReadoutBegin ) ?
    DuplicateHandle(hProc, P.CameraReadoutBegin, hProc, &(this->CameraReadoutBegin), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh35 = ( (HANDLE)(NULL) != P.CameraReadoutEnd ) && ( (HANDLE)(NULL) == this->CameraReadoutEnd ) ?
    DuplicateHandle(hProc, P.CameraReadoutEnd, hProc, &(this->CameraReadoutEnd), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh36 = ( (HANDLE)(NULL) != P.CameraTransferBegin ) && ( (HANDLE)(NULL) == this->CameraTransferBegin ) ?
    DuplicateHandle(hProc, P.CameraTransferBegin, hProc, &(this->CameraTransferBegin), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh37 = ( (HANDLE)(NULL) != P.CameraTransferEnd ) && ( (HANDLE)(NULL) == this->CameraTransferEnd ) ?
    DuplicateHandle(hProc, P.CameraTransferEnd, hProc, &(this->CameraTransferEnd), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh38 = ( (HANDLE)(NULL) != P.CameraTerminate ) && ( (HANDLE)(NULL) == this->CameraTerminate ) ?
    DuplicateHandle(hProc, P.CameraTerminate, hProc, &(this->CameraTerminate), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh39 = ( (HANDLE)(NULL) != P.CameraReady ) && ( (HANDLE)(NULL) == this->CameraReady ) ?
    DuplicateHandle(hProc, P.CameraReady, hProc, &(this->CameraReady), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  // CAMERA_SYNC_TRIGGERS (3A) is located in SynchronizationEventsDraw
  BOOL const dh3B = ( (HANDLE)(NULL) != P.CameraInvalidTrigger ) && ( (HANDLE)(NULL) == this->CameraInvalidTrigger ) ?
    DuplicateHandle(hProc, P.CameraInvalidTrigger, hProc, &(this->CameraInvalidTrigger), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh3C = ( (HANDLE)(NULL) != P.CameraChangeID ) && ( (HANDLE)(NULL) == this->CameraChangeID ) ?
    DuplicateHandle(hProc, P.CameraChangeID, hProc, &(this->CameraChangeID), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  BOOL const dh41 = ( (HANDLE)(NULL) != P.MainPrepareCamera ) && ( (HANDLE)(NULL) == this->MainPrepareCamera ) ?
    DuplicateHandle(hProc, P.MainPrepareCamera, hProc, &(this->MainPrepareCamera), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh43 = ( (HANDLE)(NULL) != P.MainReadyCamera ) && ( (HANDLE)(NULL) == this->MainReadyCamera ) ?
    DuplicateHandle(hProc, P.MainReadyCamera, hProc, &(this->MainReadyCamera), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh46 = ( (HANDLE)(NULL) != P.MainEndCamera ) && ( (HANDLE)(NULL) == this->MainEndCamera ) ?
    DuplicateHandle(hProc, P.MainEndCamera, hProc, &(this->MainEndCamera), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  this->CounterEventSetSendTrigger = P.CounterEventSetSendTrigger;
  this->CounterEventSetRepeatTrigger = P.CounterEventSetRepeatTrigger;
  this->CounterEventSetExposureBegin = P.CounterEventSetExposureBegin;
  this->CounterEventSetExposureEnd = P.CounterEventSetExposureEnd;
  this->CounterEventSetReadoutBegin = P.CounterEventSetReadoutBegin;
  this->CounterEventSetReadoutEnd = P.CounterEventSetReadoutEnd;
  this->CounterEventSetTransferBegin = P.CounterEventSetTransferBegin;
  this->CounterEventSetTransferEnd = P.CounterEventSetTransferEnd;
  this->CounterEventSetTerminate = P.CounterEventSetTerminate;
  this->CounterEventSetReady = P.CounterEventSetReady;
  this->CounterEventSetReady = P.CounterEventSetInvalidTrigger;
  this->CounterEventSetChangeID = P.CounterEventSetChangeID;

  this->CounterEventSetPrepareCamera = P.CounterEventSetPrepareCamera;
  this->CounterEventSetReadyCamera = P.CounterEventSetReadyCamera;
  this->CounterEventSetEndCamera = P.CounterEventSetEndCamera;

  this->CounterEventResetSendTrigger = P.CounterEventResetSendTrigger;
  this->CounterEventResetRepeatTrigger = P.CounterEventResetRepeatTrigger;
  this->CounterEventResetExposureBegin = P.CounterEventResetExposureBegin;
  this->CounterEventResetExposureEnd = P.CounterEventResetExposureEnd;
  this->CounterEventResetReadoutBegin = P.CounterEventResetReadoutBegin;
  this->CounterEventResetReadoutEnd = P.CounterEventResetReadoutEnd;
  this->CounterEventResetTransferBegin = P.CounterEventResetTransferBegin;
  this->CounterEventResetTransferEnd = P.CounterEventResetTransferEnd;
  this->CounterEventResetTerminate = P.CounterEventResetTerminate;
  this->CounterEventResetReady = P.CounterEventResetReady;
  this->CounterEventResetInvalidTrigger = P.CounterEventResetInvalidTrigger;
  this->CounterEventResetChangeID = P.CounterEventResetChangeID;

  this->CounterEventResetPrepareCamera = P.CounterEventResetPrepareCamera;
  this->CounterEventResetReadyCamera = P.CounterEventResetReadyCamera;
  this->CounterEventResetEndCamera = P.CounterEventResetEndCamera;

  this->StartCounterValueSendTrigger = P.StartCounterValueSendTrigger;
  this->StartCounterValueRepeatTrigger = P.StartCounterValueRepeatTrigger;
  this->StartCounterValueExposureBegin = P.StartCounterValueExposureBegin;
  this->StartCounterValueExposureEnd = P.StartCounterValueExposureEnd;
  this->StartCounterValueReadoutBegin = P.StartCounterValueReadoutBegin;
  this->StartCounterValueReadoutEnd = P.StartCounterValueReadoutEnd;
  this->StartCounterValueTransferBegin = P.StartCounterValueTransferBegin;
  this->StartCounterValueTransferEnd = P.StartCounterValueTransferEnd;
  this->StartCounterValueTerminate = P.StartCounterValueTerminate;
  this->StartCounterValueReady = P.StartCounterValueReady;
  this->StartCounterValueInvalidTrigger = P.StartCounterValueInvalidTrigger;
  this->StartCounterValueChangeID = P.StartCounterValueChangeID;

  this->StartCounterValuePrepareCamera = P.StartCounterValuePrepareCamera;
  this->StartCounterValueReadyCamera = P.StartCounterValueReadyCamera;
  this->StartCounterValueEndCamera = P.StartCounterValueEndCamera;

  ReleaseSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  bool const fail3 = (!dh30) || (!dh31) || (!dh32) || (!dh33) || (!dh34) || (!dh35) || (!dh36) || (!dh37) || (!dh38) || (!dh39) || (!dh3B) || (!dh3C);
  assert(false == fail3);
  bool const fail4 = (!dh41) || (!dh43) || (!dh46);
  assert(false == fail4);
}
/* SynchronizationEventsCamera_::SynchronizationEventsCamera_ */



//! Default destructor.
/*!
  Closes all non NULL handles.
*/
SynchronizationEventsCamera_::~SynchronizationEventsCamera_()
{
  HRESULT const res = SynchronizationEventsCameraClose_inline(this);
  SynchronizationEventsCameraBlank_inline(this);
}
/* SynchronizationEventsCamera_::~SynchronizationEventsCamera_ */



/****** MAIN THREAD ******/

//! Blanks events structure.
/*!
  Function blanks SynchronizationEventsMain structure.

  \param P      Pointer to SynchronizationEventsMain structure.
*/
inline
void
SynchronizationEventsMainBlank_inline(
                                      SynchronizationEventsMain * const P
                                      )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->DrawSyncPresent = (HANDLE)(NULL);
  P->DrawSyncVBlank = (HANDLE)(NULL);
  P->DrawSyncTriggers = (HANDLE)(NULL);

  P->CounterEventSetSyncPresent = 0;
  P->CounterEventSetSyncVBlank = 0;
  P->CounterEventSetSyncTriggers = 0;

  P->CounterEventResetSyncPresent = 0;
  P->CounterEventResetSyncVBlank = 0;
  P->CounterEventResetSyncTriggers = 0;

  P->StartCounterValueSyncPresent = 0;
  P->StartCounterValueSyncVBlank = 0;
  P->StartCounterValueSyncTriggers = 0;
}
/* SynchronizationEventsMainBlank_inline */




//! Creates all events of the main thread.
/*!
  Function creates named events and stores then in SynchronizationEventsMain structure.

  \param P      Pointer to SynchronizationEventsMain structure.
  \param ID     Current process ID.
  \param H      Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsMainCreate_inline(
                                       SynchronizationEventsMain * const P,
                                       int const ID,
                                       int const H
                                       )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  assert( 0 <= H );
  if (0 > H) return E_INVALIDARG;

  AcquireSRWLockExclusive( &(P->lock) );

  // MAIN_PREPARE_DRAW (ce40) is located in SynchronizationEventsDraw
  // MAIN_PREPARE_CAMERA (ce41) is located in SynchronizationEventsCamera
  // MAIN_READY_DRAW (ce42) is located in SynchronizationEventsDraw
  // MAIN_READY_CAMERA (ce43) is located in SynchronizationEventsCamera
  // MAIN_BEGIN (ce44) is located in SynchronizationEventsCamera
  // MAIN_END_DRAW (ce45) is located in SynchronizationEventsDraw
  // MAIN_END_CAMERA (ce46) is located in SynchronizationEventsCamera
  // MAIN_RESUME_DRAW (ce47) is located in SynchronizationEventsDraw

  DWORD const ce48 = CreateSynchronizationEvent(&(P->DrawSyncPresent), _T("Local\\PID_%d_H_%d_EVENT_DRAW_SYNC_PRESENT"), ID, H);
  DWORD const ce49 = CreateSynchronizationEvent(&(P->DrawSyncVBlank), _T("Local\\PID_%d_H_%d_EVENT_DRAW_SYNC_VBLANK"), ID, H);
  DWORD const ce4A = CreateSynchronizationEvent(&(P->DrawSyncTriggers), _T("Local\\PID_%d_H_%d_EVENT_DRAW_SYNC_TRIGGERS"), ID, H);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail4 = (ERROR_SUCCESS != ce48) || (ERROR_SUCCESS != ce49) || (ERROR_SUCCESS != ce4A);
  assert(false == fail4);
  if (true == fail4) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsMainCreate_inline */



//! Closes all events of the main  thread.
/*!
  Function closes events stored in SynchronizationEventsMain structure.

  \param P      Pointer to SynchronizationEventsMain structure.
  \return Returns S_OK if successfull.
*/
inline
HRESULT
SynchronizationEventsMainClose_inline(
                                      SynchronizationEventsMain * const P
                                      )
{
  assert(NULL != P);
  if (NULL == P) return E_POINTER;

  AcquireSRWLockExclusive( &(P->lock) );

  // MAIN_PREPARE_DRAW (ce40) is located in SynchronizationEventsDraw
  // MAIN_PREPARE_CAMERA (ce41) is located in SynchronizationEventsCamera
  // MAIN_READY_DRAW (ce42) is located in SynchronizationEventsDraw
  // MAIN_READY_CAMERA (ce43) is located in SynchronizationEventsCamera
  // MAIN_BEGIN (ce44) is located in SynchronizationEventsCamera
  // MAIN_END_DRAW (ce45) is located in SynchronizationEventsDraw
  // MAIN_END_CAMERA (ce46) is located in SynchronizationEventsCamera
  // MAIN_RESUME_DRAW (ce47) is located in SynchronizationEventsDraw

  BOOL const ch48 = ( (HANDLE)(NULL) != P->DrawSyncPresent )? CloseHandle(P->DrawSyncPresent) : TRUE;
  BOOL const ch49 = ( (HANDLE)(NULL) != P->DrawSyncVBlank )? CloseHandle(P->DrawSyncVBlank) : TRUE;
  BOOL const ch4A = ( (HANDLE)(NULL) != P->DrawSyncTriggers )? CloseHandle(P->DrawSyncTriggers) : TRUE;

  P->DrawSyncPresent = (HANDLE)(NULL);
  P->DrawSyncVBlank = (HANDLE)(NULL);
  P->DrawSyncTriggers = (HANDLE)(NULL);

  ReleaseSRWLockExclusive( &(P->lock) );

  bool const fail4 = (!ch48) || (!ch49) || (!ch4A);
  assert(false == fail4);
  if (true == fail4) return E_FAIL;

  return S_OK;
}
/* SynchronizationEventsMainClose_inline */



//! Default constructor.
/*!
  Sets all handles to NULL.
*/
SynchronizationEventsMain_::SynchronizationEventsMain_()
{
  SynchronizationEventsMainBlank_inline(this);
  InitializeSRWLock( &(this->lock) );
}
/* SynchronizationEventsMain_::SynchronizationEventsMain_ */


//! Copy constructor.
/*!
  Creates copies of all valid handles.

  \param P      Reference to object to copy.
*/
SynchronizationEventsMain_::SynchronizationEventsMain_(
                                                       const SynchronizationEventsMain_ & P
                                                       )
{
  SynchronizationEventsMainBlank_inline(this);
  InitializeSRWLock( &(this->lock) );

  HANDLE const hProc = GetCurrentProcess();

  AcquireSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  // MAIN_PREPARE_DRAW (dh40) is located in SynchronizationEventsDraw
  // MAIN_PREPARE_CAMERA (dh41) is located in SynchronizationEventsCamera
  // MAIN_READY_DRAW (dh42) is located in SynchronizationEventsDraw
  // MAIN_READY_CAMERA (dh43) is located in SynchronizationEventsCamera
  // MAIN_BEGIN (dh44) is located in SynchronizationEventsCamera
  // MAIN_END_DRAW (dh45) is located in SynchronizationEventsDraw
  // MAIN_END_CAMERA (dh46) is located in SynchronizationEventsCamera
  // MAIN_RESUME_DRAW (dh47) is located in SynchronizationEventsDraw

  BOOL const dh48 = ( (HANDLE)(NULL) != P.DrawSyncPresent ) && ( (HANDLE)(NULL) == this->DrawSyncPresent ) ?
    DuplicateHandle(hProc, P.DrawSyncPresent, hProc, &(this->DrawSyncPresent), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh49 = ( (HANDLE)(NULL) != P.DrawSyncVBlank ) && ( (HANDLE)(NULL) == this->DrawSyncVBlank ) ?
    DuplicateHandle(hProc, P.DrawSyncVBlank, hProc, &(this->DrawSyncVBlank), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;
  BOOL const dh4A = ( (HANDLE)(NULL) != P.DrawSyncTriggers ) && ( (HANDLE)(NULL) == this->DrawSyncTriggers ) ?
    DuplicateHandle(hProc, P.DrawSyncTriggers, hProc, &(this->DrawSyncTriggers), 0, FALSE, DUPLICATE_SAME_ACCESS) : TRUE;

  this->CounterEventSetSyncPresent = P.CounterEventSetSyncPresent;
  this->CounterEventSetSyncVBlank = P.CounterEventSetSyncVBlank;
  this->CounterEventSetSyncTriggers = P.CounterEventSetSyncTriggers;

  this->CounterEventResetSyncPresent = P.CounterEventResetSyncPresent;
  this->CounterEventResetSyncVBlank = P.CounterEventResetSyncVBlank;
  this->CounterEventResetSyncTriggers = P.CounterEventResetSyncTriggers;

  this->StartCounterValueSyncPresent = P.StartCounterValueSyncPresent;
  this->StartCounterValueSyncVBlank = P.StartCounterValueSyncVBlank;
  this->StartCounterValueSyncTriggers = P.StartCounterValueSyncTriggers;

  ReleaseSRWLockExclusive( const_cast<PSRWLOCK>( &(P.lock) ) );

  bool const fail4 = (!dh48) || (!dh49) || (!dh4A);
  assert(false == fail4);
}
/* SynchronizationEventsMain_::SynchronizationEventsMain_ */



//! Default destructor.
/*!
  Closes all non NULL handles.
*/
SynchronizationEventsMain_::~SynchronizationEventsMain_()
{
  HRESULT const res = SynchronizationEventsMainClose_inline(this);
  SynchronizationEventsMainBlank_inline(this);
}
/* SynchronizationEventsMain_::~SynchronizationEventsMain_ */



/****** SYNCHRONIZATION EVENTS STRUCTURE ******/

//! Gets event handle.
/*!
  Returns handle associated with selected event.

  \param P      Pointer to SynchronizationEvents structure.
  \param ID     Event identifier.
  \param H Event number.
  \param hnd    Address where the event handle will be stored.
  \param counter_set Address where the address of the counter for conditional event signalling will be stored.
  \param counter_reset Address where the address of the counter for conditional event resetting will be stored.
  \param counter_start Address where the start value for conditional signalling and resetting will be stored.
  \param lock  Address where the SRW lock address will be stored.
  \return Returns handle or NULL if unsuccessfull.
*/
inline
bool
GetEventHandle_inline(
                      SynchronizationEvents * const P,
                      SynchronizationCodes const ID,
                      int const H,
                      HANDLE * const hnd,
                      int * * const counter_set,
                      int * * const counter_reset,
                      int * * const counter_start,
                      SRWLOCK * * const lock
                      )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = false;

  AcquireSRWLockShared( &(P->lock) );

  switch( ID )
    {
    case IMAGE_DECODER_QUEUE_FULL:
      {
        SynchronizationEventsImageDecoder_ * const ptr = nth_ptr(P->ImageDecoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageDecoderQueueFull;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueFull );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueFull );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueFull );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_DECODER_QUEUE_EMPTY:
      {
        SynchronizationEventsImageDecoder_ * const ptr = nth_ptr(P->ImageDecoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageDecoderQueueEmpty;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueEmpty );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueEmpty );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueEmpty );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_DECODER_QUEUE_PROCESS:
      {
        SynchronizationEventsImageDecoder_ * const ptr = nth_ptr(P->ImageDecoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageDecoderQueueProcess;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueProcess );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueProcess );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueProcess );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_DECODER_QUEUE_TERMINATE:
      {
        SynchronizationEventsImageDecoder_ * const ptr = nth_ptr(P->ImageDecoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageDecoderQueueTerminate;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueTerminate );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueTerminate );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueTerminate );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_DECODER_CHANGE_ID:
      {
        SynchronizationEventsImageDecoder_ * const ptr = nth_ptr(P->ImageDecoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageDecoderChangeID;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetChangeID );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetChangeID );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueChangeID );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_ENCODER_QUEUE_FULL:
      {
        SynchronizationEventsImageEncoder_ * const ptr = nth_ptr(P->ImageEncoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageEncoderQueueFull;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueFull );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueFull );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueFull );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_ENCODER_QUEUE_EMPTY:
      {
        SynchronizationEventsImageEncoder_ * const ptr = nth_ptr(P->ImageEncoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageEncoderQueueEmpty;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueEmpty );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueEmpty );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueEmpty );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_ENCODER_QUEUE_PROCESS:
      {
        SynchronizationEventsImageEncoder_ * const ptr = nth_ptr(P->ImageEncoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageEncoderQueueProcess;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueProcess );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueProcess );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueProcess );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_ENCODER_QUEUE_TERMINATE:
      {
        SynchronizationEventsImageEncoder_ * const ptr = nth_ptr(P->ImageEncoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageEncoderQueueTerminate;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetQueueTerminate );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetQueueTerminate );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueQueueTerminate );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case IMAGE_ENCODER_CHANGE_ID:
      {
        SynchronizationEventsImageEncoder_ * const ptr = nth_ptr(P->ImageEncoder, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->ImageEncoderChangeID;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetChangeID );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetChangeID );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueChangeID );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_PRESENT:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawPresent;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetPresent );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetPresent );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValuePresent );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_PRESENT_READY:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawPresentReady;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetPresentReady );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetPresentReady );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValuePresentReady );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_RENDER:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawRender;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetRender );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetRender );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueRender );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_RENDER_READY:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawRenderReady;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetRenderReady );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetRenderReady );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueRenderReady );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_TERMINATE:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawTerminate;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetTerminate );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetTerminate );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueTerminate );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_VBLANK:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawVBlank;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetVBlank );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetVBlank );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueVBlank );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_CHANGE_ID:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawChangeID;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetChangeID );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetChangeID );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueChangeID );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_SYNC_TRIGGERS:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraSyncTriggers;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetSyncTriggers );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetSyncTriggers );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueSyncTriggers );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_PREPARE_DRAW:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainPrepareDraw;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetPrepareDraw );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetPrepareDraw );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValuePrepareDraw );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_READY_DRAW:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainReadyDraw;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetReadyDraw );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetReadyDraw );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueReadyDraw );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_BEGIN:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainBegin;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetBegin );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetBegin );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueBegin );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_END_DRAW:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainEndDraw;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetEndDraw );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetEndDraw );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueEndDraw );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_RESUME_DRAW:
      {
        SynchronizationEventsDraw_ * const ptr = nth_ptr(P->Draw, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainResumeDraw;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetResumeDraw );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetResumeDraw );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueResumeDraw );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_SEND_TRIGGER:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraSendTrigger;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetSendTrigger );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetSendTrigger );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueSendTrigger );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_REPEAT_TRIGGER:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraRepeatTrigger;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetRepeatTrigger );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetRepeatTrigger );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueRepeatTrigger );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_EXPOSURE_BEGIN:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraExposureBegin;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetExposureBegin );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetExposureBegin );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueExposureBegin );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_EXPOSURE_END:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraExposureEnd;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetExposureEnd );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetExposureEnd );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueExposureEnd );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_READOUT_BEGIN:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraReadoutBegin;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetReadoutBegin );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetReadoutBegin );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueReadoutBegin );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_READOUT_END:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraReadoutEnd;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetReadoutEnd );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetReadoutEnd );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueReadoutEnd );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_TRANSFER_BEGIN:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraTransferBegin;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetTransferBegin );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetTransferBegin );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueTransferBegin );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_TRANSFER_END:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraTransferEnd;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetTransferEnd );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetTransferEnd );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueTransferEnd );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_TERMINATE:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraTerminate;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetTerminate );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetTerminate );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueTerminate );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_READY:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraReady;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetReady );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetReady );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueReady );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_INVALID_TRIGGER:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraInvalidTrigger;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetInvalidTrigger );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetInvalidTrigger );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueInvalidTrigger );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case CAMERA_CHANGE_ID:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->CameraChangeID;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetChangeID );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetChangeID );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueChangeID );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_PREPARE_CAMERA:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainPrepareCamera;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetPrepareCamera );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetPrepareCamera );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValuePrepareCamera );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_READY_CAMERA:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainReadyCamera;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetReadyCamera );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetReadyCamera );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueReadyCamera );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case MAIN_END_CAMERA:
      {
        SynchronizationEventsCamera_ * const ptr = nth_ptr(P->Camera, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->MainEndCamera;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetEndCamera );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetEndCamera );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueEndCamera );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_SYNC_PRESENT:
      {
        SynchronizationEventsMain_ * const ptr = nth_ptr(P->Main, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawSyncPresent;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetSyncPresent );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetSyncPresent );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueSyncPresent );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_SYNC_VBLANK:
      {
        SynchronizationEventsMain_ * const ptr = nth_ptr(P->Main, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawSyncVBlank;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetSyncVBlank );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetSyncVBlank );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueSyncVBlank );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;

    case DRAW_SYNC_TRIGGERS:
      {
        SynchronizationEventsMain_ * const ptr = nth_ptr(P->Main, H);
        if (NULL != ptr)
          {
            result = true;
            if (NULL != hnd) *hnd = ptr->DrawSyncTriggers;
            if (NULL != counter_set) *counter_set = &( ptr->CounterEventSetSyncTriggers );
            if (NULL != counter_reset) *counter_reset = &( ptr->CounterEventResetSyncTriggers );
            if (NULL != counter_start) *counter_start = &( ptr->StartCounterValueSyncTriggers );
            if (NULL != lock) *lock = &( ptr->lock );
          }
        /* if */
      }
      break;
    }
  /* switch */

  ReleaseSRWLockShared( &(P->lock) );

  return result;
}
/* GetEventHandle_inline */




/****** MEMBER FUNCTIONS ******/


//! Signal specified event.
/*!
  Signals specified event.

  \param ID     Event identifier.
  \param H      Event number. May be omitted; default value is 0.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventSet(
                                SynchronizationCodes const ID,
                                int const H
                                )
{
  BOOL result = FALSE;

  HANDLE hnd = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, NULL, NULL, NULL, NULL);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) )
    {
      result = SetEvent(hnd);
      assert(0 != result);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventSet */



//! Reset specified event to non-signaled state.
/*!
  Resets specified event.

  \param ID     Event identifier.
  \param H      Event number. May be omitted; default value is 0.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventReset(
                                  SynchronizationCodes const ID,
                                  int const H
                                  )
{
  BOOL result = 0;

  HANDLE hnd = (HANDLE)(NULL);
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, NULL, NULL, NULL, NULL);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) )
    {
      result = ResetEvent(hnd);
      assert(0 != result);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventReset */



//! Get conditional event counter for event signalling.
/*!
  Gets counter value for conditional event signalling.

  \param ID     Event identifier.
  \param H      Event number.
  \param adr    Address where the counter value will be returned.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::GetEventSetCounter(
                                          SynchronizationCodes const ID,
                                          int const H,
                                          int * const adr
                                          )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_set) && (NULL != lock) )
    {
      AcquireSRWLockShared(lock);
      if (NULL != adr) *adr = *counter_set;
      ReleaseSRWLockShared(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::GetEventSetCounter */



//! Get conditional event counter for event reset.
/*!
  Gets counter value for conditional event reset.

  \param ID     Event identifier.
  \param H      Event number.
  \param adr    Address where the counter value will be returned.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::GetEventResetCounter(
                                            SynchronizationCodes const ID,
                                            int const H,
                                            int * const adr
                                            )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_reset) && (NULL != lock) )
    {
      AcquireSRWLockShared(lock);
      if (NULL != adr) *adr = *counter_reset;
      ReleaseSRWLockShared(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::GetEventResetCounter */



//! Get start counter value.
/*!
  Gets start counter value for conditional event signalling and resetting.
  Returned value is used to auto initialize the counter after the condition for signalling or resettting if fulfilled.

  \param ID     Event identifier.
  \param H      Event number.
  \param adr    Address where the counter value will be returned.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::GetStartCounterValue(
                                            SynchronizationCodes const ID,
                                            int const H,
                                            int * const adr
                                            )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_start) && (NULL != lock) )
    {
      AcquireSRWLockShared(lock);
      if (NULL != adr) *adr = *counter_start;
      ReleaseSRWLockShared(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::GetStartCounterValue */



//! Set conditional event counter for event signalling.
/*!
  Sets counter value for conditional event signaling.

  \param ID     Event identifier.
  \param H      Event number.
  \param C      Event counter value.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::SetEventSetCounter(
                                          SynchronizationCodes const ID,
                                          int const H,
                                          int const C
                                          )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_set) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      *counter_set = C;
      ReleaseSRWLockExclusive(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::SetEventSetCounter */



//! Set conditional event counter for event reset.
/*!
  Sets counter value for conditional event reset.

  \param ID     Event identifier.
  \param H      Event number.
  \param C      Event counter value.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::SetEventResetCounter(
                                            SynchronizationCodes const ID,
                                            int const H,
                                            int const C
                                            )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_reset) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      *counter_reset = C;
      ReleaseSRWLockExclusive(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::SetEventResetCounter */



//! Set start counter value.
/*!
  Sets start counter value for conditional event signalling and resetting.
  Supplied value is used to auto initialize event counters after the condition for signalling or resettting if fulfilled.

  \param ID     Event identifier.
  \param H      Event number.
  \param C      Event counter value.
  \param initialize If true then both set and reset counters are initialized to the starting value.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::SetStartCounterValue(
                                            SynchronizationCodes const ID,
                                            int const H,
                                            int const C,
                                            bool const initialize
                                            )
{
  BOOL result = TRUE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_start) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      *counter_start = C;
      if (true == initialize)
        {
          if (NULL != counter_set) *counter_set = C;
          if (NULL != counter_reset) *counter_reset = C;
        }
      /* if */
      ReleaseSRWLockExclusive(lock);
    }
  else
    {
      result = FALSE;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::SetEventResetCounter */



//! Signal specified event and set conditional counter.
/*!
  Signals specified event and sets event reset counter value to the specified value.

  \param ID     Event identifier.
  \param H      Event number.
  \param C      Counter value.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventSetAndResetCounterSet(
                                                  SynchronizationCodes const ID,
                                                  int const H,
                                                  int const C
                                                  )
{
  BOOL result = FALSE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_reset) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      result = SetEvent(hnd);
      assert(0 != result);
      *counter_reset = C;
      ReleaseSRWLockExclusive(lock);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventSetAndResetCounterSet*/



//! Reset specified event to non-signaled state and set conditional counter.
/*!
  Resets specified event and sets event set counter value to the specified value.

  \param ID     Event identifier.
  \param H      Event number.
  \param C      Counter value.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventResetAndSetCounterSet(
                                                  SynchronizationCodes const ID,
                                                  int const H,
                                                  int const C
                                                  )
{
  BOOL result = FALSE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_set) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      result = ResetEvent(hnd);
      assert(0 != result);
      *counter_set = C;
      ReleaseSRWLockExclusive(lock);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventResetAndSetCounterSet */



//! Conditional event signal.
/*!
  Function first decreases the event set counter by one.
  The event is signalled conditionaly once the set counter becomes equal or less-then zero.
  The set counter is then reset to the start value.

  \param ID     Event identifier.
  \param H      Event number.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventSetConditional(
                                           SynchronizationCodes const ID,
                                           int const H
                                           )
{
  BOOL result = FALSE;

  HANDLE hnd = NULL;
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_set) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      *counter_set = *counter_set - 1;
      if (0 >= *counter_set)
        {
          result = SetEvent( hnd );
          assert(0 != result);
          if (NULL != counter_start) *counter_set = *counter_start;
        }
      else
        {
          result = TRUE;
        }
      /* if */
      ReleaseSRWLockExclusive(lock);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventSetConditional */



//! Conditional event reset.
/*!
  Function first decreases the event reset counter by one.
  The event is signalled conditionaly once the reset counter becomes equal or less-then zero.
  The reset counter is then reset to the start value.

  \param ID     Event identifier.
  \param H      Event number.
  \return Returns 0 if unsuccessfull.
*/
BOOL
SynchronizationEvents::EventResetConditional(
                                             SynchronizationCodes const ID,
                                             int const H
                                             )
{
  BOOL result = FALSE;

  HANDLE hnd = (HANDLE)(NULL);
  int * counter_set = NULL;
  int * counter_reset = NULL;
  int * counter_start = NULL;
  SRWLOCK * lock = NULL;
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, &counter_set, &counter_reset, &counter_start, &lock);
  assert(true == get_handle);

  if ( (true == get_handle) && ( (HANDLE)(NULL) != hnd ) && (NULL != counter_reset) && (NULL != lock) )
    {
      AcquireSRWLockExclusive(lock);
      *counter_reset = *counter_reset - 1;
      if (0 >= *counter_reset)
        {
          result = ResetEvent( hnd );
          assert(0 != result);
          if (NULL != counter_start) *counter_reset = *counter_start;
        }
      else
        {
          result = TRUE;
        }
      /* if */
      ReleaseSRWLockExclusive(lock);
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventResetConditional */



//! Reset all events.
/*!
  Function resets all events.

  \param H_main Event number for MAIN_BEGIN event.
  \param H_draw Event number for all *_DRAW and IMAGE_DECODER_QUEUE_* events.
  \param H_camera Event number for all *_CAMERA and IMAGE_ENCODER_QUEUE_* events.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAll(
                                     int const H_main,
                                     int const H_draw,
                                     int const H_camera
                                     )
{
  BOOL const r0 = this->EventResetAllImageDecoder(H_draw);
  BOOL const r1 = this->EventResetAllImageEncoder(H_camera);
  BOOL const r2 = this->EventResetAllDraw(H_draw);
  BOOL const r3 = this->EventResetAllCamera(H_camera, H_draw);
  BOOL const r4 = this->EventResetAllMain(H_main, H_draw, H_camera);

  BOOL const result = r0 && r1 && r2 && r3 && r4;

  return result;
}
/* SynchronizationEvents::EventResetAll */



//! Reset all image decoder events.
/*!
  Function resets all image decoder events.

  \param H      Event number.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllImageDecoder(
                                                 int const H
                                                 )
{
  BOOL const r00 = this->EventReset(IMAGE_DECODER_QUEUE_FULL, H);
  BOOL const r01 = this->EventReset(IMAGE_DECODER_QUEUE_EMPTY, H);
  BOOL const r02 = this->EventReset(IMAGE_DECODER_QUEUE_PROCESS, H);
  BOOL const r03 = this->EventReset(IMAGE_DECODER_QUEUE_TERMINATE, H);
  BOOL const r04 = this->EventReset(IMAGE_DECODER_CHANGE_ID, H);

  BOOL const result = r00 && r01 && r02 && r03 && r04;
  
  return result;
}
/* SynchronizationEvents::EventResetAllImageDecoder */



//! Reset all image encoder events.
/*!
  Function resets all image encoder events.

  \param H      Event number.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllImageEncoder(
                                                 int const H
                                                 )
{
  BOOL const r10 = this->EventReset(IMAGE_ENCODER_QUEUE_FULL, H);
  BOOL const r11 = this->EventReset(IMAGE_ENCODER_QUEUE_EMPTY, H);
  BOOL const r12 = this->EventReset(IMAGE_ENCODER_QUEUE_PROCESS, H);
  BOOL const r13 = this->EventReset(IMAGE_ENCODER_QUEUE_TERMINATE, H);
  BOOL const r14 = this->EventReset(IMAGE_ENCODER_CHANGE_ID, H);
  
  BOOL const result = r10 && r11 && r12 && r13 && r14;

  return result;  
}
/* SynchronizationEvents::EventResetAllImageEncoder */


//! Reset all draw events.
/*!
  Function resets all draw events.

  \param H Event number.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllDraw(
                                         int const H
                                         )
{
  BOOL const e2 = this->EventResetAllDrawExceptRenderAndPresentReady(H);
  BOOL const r21 = this->EventReset(DRAW_PRESENT_READY, H);
  BOOL const r23 = this->EventReset(DRAW_RENDER_READY, H);

  BOOL const result = e2 && r21 && r23;

  return result;
}
/* SynchronizationEvents::EventResetAllDraw */



//! Reset almost all draw events.
/*!
  Function resets all draw events execpt DRAW_RENDER_READY and DRAW_PRESENT_READY.

  \param H Event number.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllDrawExceptRenderAndPresentReady(
                                                                    int const H
                                                                    )
{
  BOOL const r20 = this->EventReset(DRAW_PRESENT, H);
  // DRAW_PRESENT_READY
  BOOL const r22 = this->EventReset(DRAW_RENDER, H);
  // DRAW_RENDER_READY
  BOOL const r24 = this->EventReset(DRAW_TERMINATE, H);
  BOOL const r25 = this->EventReset(DRAW_VBLANK, H);
  BOOL const r26 = this->EventReset(DRAW_CHANGE_ID, H);

  BOOL const result = r20 && r22 && r24 && r25 && r26;

  return result;
}
/* SynchronizationEvents::EventResetAllDrawExceptRenderAndPresentReady */



//! Reset all camera events.
/*!
  Function resets all camera events.

  \param H_camera Event number for CAMERA_* events except CAMERA_SYNC_TRIGGERS.
  \param H_draw   Event number for CAMERA_SYNC_TRIGGERS event.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllCamera(
                                           int const H_camera,
                                           int const H_draw
                                           )
{
  BOOL result = TRUE;

  if (0 <= H_camera)
    {
      BOOL const e39 = this->EventResetAllCameraExceptTriggerReady(H_camera);
      BOOL const r39 = this->EventReset(CAMERA_READY, H_camera);
      result = result && e39 && r39;
    }
  /* if */

  if (0 <= H_draw)
    {
      BOOL const e3A = this->EventReset(CAMERA_SYNC_TRIGGERS, H_draw);
      result = result && e3A;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventResetAllCamera */



//! Reset all camera events except CAMERA_READY.
/*!
  Function resets all camera events except CAMERA_READY and CAMERA_SYNC_TRIGGERS.

  \param H Event number.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllCameraExceptTriggerReady(
                                                             int const H
                                                             )
{
  BOOL const r30 = this->EventReset(CAMERA_SEND_TRIGGER, H);
  BOOL const r31 = this->EventReset(CAMERA_REPEAT_TRIGGER, H);
  BOOL const r32 = this->EventReset(CAMERA_EXPOSURE_BEGIN, H);
  BOOL const r33 = this->EventReset(CAMERA_EXPOSURE_END, H);
  BOOL const r34 = this->EventReset(CAMERA_READOUT_BEGIN, H);
  BOOL const r35 = this->EventReset(CAMERA_READOUT_END, H);
  BOOL const r36 = this->EventReset(CAMERA_TRANSFER_BEGIN, H);
  BOOL const r37 = this->EventReset(CAMERA_TRANSFER_END, H);
  BOOL const r38 = this->EventReset(CAMERA_TERMINATE, H);
  // CAMERA_READY (39) is reset elsewhere
  // CAMERA_SYNC_TRIGGERS (3A) is located in SynchronizationEventsDraw
  BOOL const r3B = this->EventReset(CAMERA_INVALID_TRIGGER, H);
  BOOL const r3C = this->EventReset(CAMERA_CHANGE_ID, H);

  BOOL const result = r30 && r31 && r32 && r33 && r34 && r35 && r36 && r37 && r38 && r3B && r3C;

  return result;
}
/* SynchronizationEvents::EventResetAllCameraExceptTriggerReady */



//! Reset all main thread events.
/*!
  Function resets all main thread events.

  Note: Main thread events (named MAIN_*) are events which were expected to be signalled exclusively
  by the main thread. However, when the application was extended to support multiple rendering
  and acquisition threads all such events could not be shared between different rendering and
  acquisition threads; the events must be unique to each acquisition and rendering thread.
  Therefore all MAIN_* events were moved to corresponding SynchronizationEventsDraw and
  SynchronizationEventsCamera structures. See comment in SynchronizationEventsMain for more
  details.

  \param H_main Event number for DRAW_SYNC_* events; if negative (e.g. -1) then DRAW_SYNC_* events will not be reset.
  \param H_draw Event number for all MAIN_*_DRAW events; if negative (e.g. -1) then MAIN_*_DRAW events will not be reset.
  \param H_camera Event number for all MAIN_*_CAMERA events; if negative (e.g. -1) then MAIN_*_CAMERA events will not be reset.
  \return Returns 0 if successfull.
*/
BOOL
SynchronizationEvents::EventResetAllMain(
                                         int const H_main,
                                         int const H_draw,
                                         int const H_camera
                                         )
{
  BOOL result = TRUE;

  if (0 <= H_draw)
    {
      BOOL const r40 = this->EventReset(MAIN_PREPARE_DRAW, H_draw);
      BOOL const r42 = this->EventReset(MAIN_READY_DRAW, H_draw);
      BOOL const r44 = this->EventReset(MAIN_BEGIN, H_draw);
      BOOL const r45 = this->EventReset(MAIN_END_DRAW, H_draw);
      BOOL const r47 = this->EventReset(MAIN_RESUME_DRAW, H_draw);
      result = result && r40 && r42 && r44 && r45 && r47;
    }
  /* if */

  if (0 <= H_camera)
    {
      BOOL const r41 = this->EventReset(MAIN_PREPARE_CAMERA, H_camera);
      BOOL const r43 = this->EventReset(MAIN_READY_CAMERA, H_camera);
      BOOL const r46 = this->EventReset(MAIN_END_CAMERA, H_camera);
      result = result && r41 && r43 && r46;
    }
  /* if */

  if (0 <= H_main)
    {
      BOOL const r48 = this->EventReset(DRAW_SYNC_PRESENT, H_main);
      BOOL const r49 = this->EventReset(DRAW_SYNC_VBLANK, H_main);
      BOOL const r4A = this->EventReset(DRAW_SYNC_TRIGGERS, H_main);
      result = result && r48 && r49 && r4A;
    }
  /* if */

  return result;
}
/* SynchronizationEvents::EventResetAllMain */



//! Wait for specified event.
/*!
  Waits for specified signal.

  \param ID    Event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForSingleObjectEx operation.
*/
DWORD
SynchronizationEvents::EventWaitFor(
                                    SynchronizationCodes const ID,
                                    DWORD const dwMilliseconds
                                    )
{
  int const H = 0;
  return this->EventWaitFor(ID, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitFor */



//! Wait for specified events.
/*!
  Waits for specified signal.

  \param ID0    First event identifier.
  \param ID1    Second event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       DWORD const dwMilliseconds
                                       )
{
  int const H = 0;
  return this->EventWaitForAny(ID0, H, ID1, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param ID1     Event identifier.
  \param ID2     Event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       SynchronizationCodes const ID2,
                                       DWORD const dwMilliseconds
                                       )
{
  int const H = 0;
  return this->EventWaitForAny(ID0, H, ID1, H, ID2, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal.

  \param ID0     Event identifier.
  \param ID1     Event identifier.
  \param ID2     Event identifier.
  \param ID3     Event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       SynchronizationCodes const ID2,
                                       SynchronizationCodes const ID3,
                                       DWORD const dwMilliseconds
                                       )
{
  int const H = 0;
  return EventWaitForAny(ID0, H, ID1, H, ID2, H, ID3, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal.

  \param ID0     Event identifier.
  \param ID1     Event identifier.
  \param ID2     Event identifier.
  \param ID3     Event identifier.
  \param ID4     Event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       SynchronizationCodes const ID2,
                                       SynchronizationCodes const ID3,
                                       SynchronizationCodes const ID4,
                                       DWORD const dwMilliseconds
                                       )
{
  int const H = 0;
  return EventWaitForAny(ID0, H, ID1, H, ID2, H, ID3, H, ID4, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal.

  \param ID0     Event identifier.
  \param ID1     Event identifier.
  \param ID2     Event identifier.
  \param ID3     Event identifier.
  \param ID4     Event identifier.
  \param ID5     Event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       SynchronizationCodes const ID2,
                                       SynchronizationCodes const ID3,
                                       SynchronizationCodes const ID4,
                                       SynchronizationCodes const ID5,
                                       DWORD const dwMilliseconds
                                       )
{
  int const H = 0;
  return EventWaitForAny(ID0, H, ID1, H, ID2, H, ID3, H, ID4, H, ID5, H, dwMilliseconds);
}
/* SynchronizationEvents::EventWaitForAny */




//! Wait for specified event.
/*!
  Waits for specified signal.
  Call is wrapped to WaitForSingleObjectEx.

  \param ID    Event identifier.
  \param H     Unique index of event indentifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForSingleObjectEx operation.
*/
DWORD
SynchronizationEvents::EventWaitFor(
                                    SynchronizationCodes const ID,
                                    int const H,
                                    DWORD const dwMilliseconds
                                    )
{
  HANDLE hnd = (HANDLE)(NULL);

  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, NULL, NULL, NULL, NULL);
  assert(true == get_handle);

  DWORD result = 0;
  if ((HANDLE)(NULL) != hnd)
    {
      result = WaitForSingleObjectEx(hnd, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitFor */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0    First event identifier.
  \param H0     Unique index of first event identifier.
  \param ID1    Second event identifier.
  \param H1     Unique index of second event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[2] = {(HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1])
      )
    {
      result = WaitForMultipleObjectsEx(2, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[3] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2])
      )
    {
      result = WaitForMultipleObjectsEx(3, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param ID3     Event identifier.
  \param H3     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       SynchronizationCodes const ID3,
                                       int const H3,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[4] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  bool const get_handle_3 = GetEventHandle_inline(this, ID3, H3, hnd + 3, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);
  assert(true == get_handle_3);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2]) &&
      ((HANDLE)(NULL) != hnd[3])
      )
    {
      result = WaitForMultipleObjectsEx(4, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param ID3     Event identifier.
  \param H3     Unique index of event identifier.
  \param ID4     Event identifier.
  \param H4     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       SynchronizationCodes const ID3,
                                       int const H3,
                                       SynchronizationCodes const ID4,
                                       int const H4,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[5] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  bool const get_handle_3 = GetEventHandle_inline(this, ID3, H3, hnd + 3, NULL, NULL, NULL, NULL);
  bool const get_handle_4 = GetEventHandle_inline(this, ID4, H4, hnd + 4, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);
  assert(true == get_handle_3);
  assert(true == get_handle_4);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2]) &&
      ((HANDLE)(NULL) != hnd[3]) &&
      ((HANDLE)(NULL) != hnd[4])
      )
    {
      result = WaitForMultipleObjectsEx(5, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param ID3     Event identifier.
  \param H3     Unique index of event identifier.
  \param ID4     Event identifier.
  \param H4     Unique index of event identifier.
  \param ID5     Event identifier.
  \param H5     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       SynchronizationCodes const ID3,
                                       int const H3,
                                       SynchronizationCodes const ID4,
                                       int const H4,
                                       SynchronizationCodes const ID5,
                                       int const H5,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[6] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  bool const get_handle_3 = GetEventHandle_inline(this, ID3, H3, hnd + 3, NULL, NULL, NULL, NULL);
  bool const get_handle_4 = GetEventHandle_inline(this, ID4, H4, hnd + 4, NULL, NULL, NULL, NULL);
  bool const get_handle_5 = GetEventHandle_inline(this, ID5, H5, hnd + 5, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);
  assert(true == get_handle_3);
  assert(true == get_handle_4);
  assert(true == get_handle_5);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2]) &&
      ((HANDLE)(NULL) != hnd[3]) &&
      ((HANDLE)(NULL) != hnd[4]) &&
      ((HANDLE)(NULL) != hnd[5])
      )
    {
      result = WaitForMultipleObjectsEx(6, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param ID3     Event identifier.
  \param H3     Unique index of event identifier.
  \param ID4     Event identifier.
  \param H4     Unique index of event identifier.
  \param ID5     Event identifier.
  \param H5     Unique index of event identifier.
  \param ID6     Event identifier.
  \param H6     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       SynchronizationCodes const ID3,
                                       int const H3,
                                       SynchronizationCodes const ID4,
                                       int const H4,
                                       SynchronizationCodes const ID5,
                                       int const H5,
                                       SynchronizationCodes const ID6,
                                       int const H6,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[7] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  bool const get_handle_3 = GetEventHandle_inline(this, ID3, H3, hnd + 3, NULL, NULL, NULL, NULL);
  bool const get_handle_4 = GetEventHandle_inline(this, ID4, H4, hnd + 4, NULL, NULL, NULL, NULL);
  bool const get_handle_5 = GetEventHandle_inline(this, ID5, H5, hnd + 5, NULL, NULL, NULL, NULL);
  bool const get_handle_6 = GetEventHandle_inline(this, ID6, H6, hnd + 6, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);
  assert(true == get_handle_3);
  assert(true == get_handle_4);
  assert(true == get_handle_5);
  assert(true == get_handle_6);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2]) &&
      ((HANDLE)(NULL) != hnd[3]) &&
      ((HANDLE)(NULL) != hnd[4]) &&
      ((HANDLE)(NULL) != hnd[5]) &&
      ((HANDLE)(NULL) != hnd[6])
      )
    {
      result = WaitForMultipleObjectsEx(7, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0     Event identifier.
  \param H0     Unique index of event identifier.
  \param ID1     Event identifier.
  \param H1     Unique index of event identifier.
  \param ID2     Event identifier.
  \param H2     Unique index of event identifier.
  \param ID3     Event identifier.
  \param H3     Unique index of event identifier.
  \param ID4     Event identifier.
  \param H4     Unique index of event identifier.
  \param ID5     Event identifier.
  \param H5     Unique index of event identifier.
  \param ID6     Event identifier.
  \param H6     Unique index of event identifier.
  \param ID7     Event identifier.
  \param H7     Unique index of event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       SynchronizationCodes const ID2,
                                       int const H2,
                                       SynchronizationCodes const ID3,
                                       int const H3,
                                       SynchronizationCodes const ID4,
                                       int const H4,
                                       SynchronizationCodes const ID5,
                                       int const H5,
                                       SynchronizationCodes const ID6,
                                       int const H6,
                                       SynchronizationCodes const ID7,
                                       int const H7,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[8] = {(HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  bool const get_handle_2 = GetEventHandle_inline(this, ID2, H2, hnd + 2, NULL, NULL, NULL, NULL);
  bool const get_handle_3 = GetEventHandle_inline(this, ID3, H3, hnd + 3, NULL, NULL, NULL, NULL);
  bool const get_handle_4 = GetEventHandle_inline(this, ID4, H4, hnd + 4, NULL, NULL, NULL, NULL);
  bool const get_handle_5 = GetEventHandle_inline(this, ID5, H5, hnd + 5, NULL, NULL, NULL, NULL);
  bool const get_handle_6 = GetEventHandle_inline(this, ID6, H6, hnd + 6, NULL, NULL, NULL, NULL);
  bool const get_handle_7 = GetEventHandle_inline(this, ID7, H7, hnd + 7, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);
  assert(true == get_handle_2);
  assert(true == get_handle_3);
  assert(true == get_handle_4);
  assert(true == get_handle_5);
  assert(true == get_handle_6);
  assert(true == get_handle_7);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1]) &&
      ((HANDLE)(NULL) != hnd[2]) &&
      ((HANDLE)(NULL) != hnd[3]) &&
      ((HANDLE)(NULL) != hnd[4]) &&
      ((HANDLE)(NULL) != hnd[5]) &&
      ((HANDLE)(NULL) != hnd[6]) &&
      ((HANDLE)(NULL) != hnd[7])
      )
    {
      result = WaitForMultipleObjectsEx(8, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified events.
/*!
  Waits for all signals. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0    First event identifier.
  \param ID1    Second event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAll(
                                       SynchronizationCodes const ID0,
                                       SynchronizationCodes const ID1,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[2] = {(HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, 0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, 0, hnd + 1, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1])
      )
    {
      result = WaitForMultipleObjectsEx(2, hnd, TRUE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAll */



//! Wait for specified events.
/*!
  Waits for all signals. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID0    First event identifier.
  \param H0     Unique index of first event identifier.
  \param ID1    Second event identifier.
  \param H1     Unique index of second event identifier.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAll(
                                       SynchronizationCodes const ID0,
                                       int const H0,
                                       SynchronizationCodes const ID1,
                                       int const H1,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[2] = {(HANDLE)(NULL), (HANDLE)(NULL)};
  bool const get_handle_0 = GetEventHandle_inline(this, ID0, H0, hnd    , NULL, NULL, NULL, NULL);
  bool const get_handle_1 = GetEventHandle_inline(this, ID1, H1, hnd + 1, NULL, NULL, NULL, NULL);
  assert(true == get_handle_0);
  assert(true == get_handle_1);

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd[0]) &&
      ((HANDLE)(NULL) != hnd[1])
      )
    {
      result = WaitForMultipleObjectsEx(2, hnd, TRUE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAll */



//! Get event handle.
/*!
  Returns event handle.

  \see GetEventHandle_inline

  \param ID     Event identifier.
  \param H   Event number.
  \return Returns handle or NULL if unsuccessfull.
*/
HANDLE
SynchronizationEvents::GetEventHandle(
                                      SynchronizationCodes const ID,
                                      int const H
                                      )
{
  HANDLE hnd = (HANDLE)(NULL);
  bool const get_handle = GetEventHandle_inline(this, ID, H, &hnd, NULL, NULL, NULL, NULL);
  assert(true == get_handle);
  return hnd;
}
/* SynchronizationEvents::GetEventHandle */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Handle to first event.
  \param hnd1   Handle to second event.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[2];
  hnd[0] = hnd0;
  hnd[1] = hnd1;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1)
      )
    {
      result = WaitForMultipleObjectsEx(2, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Handle to first event.
  \param hnd1   Handle to second event.
  \param hnd2   Handle to third event.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[3];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2)
      )
    {
      result = WaitForMultipleObjectsEx(3, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Event handle.
  \param hnd1   Event handle.
  \param hnd2   Event handle.
  \param hnd3   Event handle.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       HANDLE const hnd3,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[4];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;
  hnd[3] = hnd3;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2) &&
      ((HANDLE)(NULL) != hnd3)
      )
    {
      result = WaitForMultipleObjectsEx(4, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Event handle.
  \param hnd1   Event handle.
  \param hnd2   Event handle.
  \param hnd3   Event handle.
  \param hnd4   Event handle.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       HANDLE const hnd3,
                                       HANDLE const hnd4,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[5];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;
  hnd[3] = hnd3;
  hnd[4] = hnd4;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2) &&
      ((HANDLE)(NULL) != hnd3) &&
      ((HANDLE)(NULL) != hnd4)
      )
    {
      result = WaitForMultipleObjectsEx(5, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Event handle.
  \param hnd1   Event handle.
  \param hnd2   Event handle.
  \param hnd3   Event handle.
  \param hnd4   Event handle.
  \param hnd5   Event handle.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       HANDLE const hnd3,
                                       HANDLE const hnd4,
                                       HANDLE const hnd5,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[6];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;
  hnd[3] = hnd3;
  hnd[4] = hnd4;
  hnd[5] = hnd5;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2) &&
      ((HANDLE)(NULL) != hnd3) &&
      ((HANDLE)(NULL) != hnd4) &&
      ((HANDLE)(NULL) != hnd5)
      )
    {
      result = WaitForMultipleObjectsEx(6, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Event handle.
  \param hnd1   Event handle.
  \param hnd2   Event handle.
  \param hnd3   Event handle.
  \param hnd4   Event handle.
  \param hnd5   Event handle.
  \param hnd6   Event handle.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       HANDLE const hnd3,
                                       HANDLE const hnd4,
                                       HANDLE const hnd5,
                                       HANDLE const hnd6,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[7];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;
  hnd[3] = hnd3;
  hnd[4] = hnd4;
  hnd[5] = hnd5;
  hnd[6] = hnd6;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2) &&
      ((HANDLE)(NULL) != hnd3) &&
      ((HANDLE)(NULL) != hnd4) &&
      ((HANDLE)(NULL) != hnd5) &&
      ((HANDLE)(NULL) != hnd6)
      )
    {
      result = WaitForMultipleObjectsEx(7, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Wait for specified handles.
/*!
  Waits for specified handles. Call is wrapped to WaitForMultipleObjectsEx.

  \param hnd0   Event handle.
  \param hnd1   Event handle.
  \param hnd2   Event handle.
  \param hnd3   Event handle.
  \param hnd4   Event handle.
  \param hnd5   Event handle.
  \param hnd6   Event handle.
  \param hnd7   Event handle.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       HANDLE const hnd0,
                                       HANDLE const hnd1,
                                       HANDLE const hnd2,
                                       HANDLE const hnd3,
                                       HANDLE const hnd4,
                                       HANDLE const hnd5,
                                       HANDLE const hnd6,
                                       HANDLE const hnd7,
                                       DWORD const dwMilliseconds
                                       )
{
  HANDLE hnd[8];
  hnd[0] = hnd0;
  hnd[1] = hnd1;
  hnd[2] = hnd2;
  hnd[3] = hnd3;
  hnd[4] = hnd4;
  hnd[5] = hnd5;
  hnd[6] = hnd6;
  hnd[7] = hnd7;

  DWORD result = 0;
  if (((HANDLE)(NULL) != hnd0) &&
      ((HANDLE)(NULL) != hnd1) &&
      ((HANDLE)(NULL) != hnd2) &&
      ((HANDLE)(NULL) != hnd3) &&
      ((HANDLE)(NULL) != hnd4) &&
      ((HANDLE)(NULL) != hnd5) &&
      ((HANDLE)(NULL) != hnd6) &&
      ((HANDLE)(NULL) != hnd7)
      )
    {
      result = WaitForMultipleObjectsEx(8, hnd, FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Waits for specified events.
/*!
  Waits for specified signal. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID    Reference to vector of event IDs.
  \param H     Reference to vector of event identifiers. Must have the same number of elements as ID.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAny(
                                       std::vector<SynchronizationCodes> & ID,
                                       std::vector<int> & H,
                                       DWORD const dwMilliseconds
                                       )
{
  assert( ID.size() == H.size() );
  if ( ID.size() != H.size() ) return WAIT_FAILED;

  size_t const sz = ID.size();
  assert( sz < MAXIMUM_WAIT_OBJECTS);
  if ( sz >= MAXIMUM_WAIT_OBJECTS ) return WAIT_FAILED;

  std::vector<HANDLE> hnd(sz, (HANDLE)(NULL));

  DWORD result = TRUE;
  for (int i = 0; i < (int)sz; ++i)
    {
      bool const get_handle = GetEventHandle_inline(this, ID[i], H[i], &(hnd[i]), NULL, NULL, NULL, NULL);
      assert(true == get_handle);
      assert((HANDLE)(NULL) != hnd[i]);
      if ( (false == get_handle) || ((HANDLE)(NULL) == hnd[i]) )
        {
          result = FALSE;
          break;
        }
      /* if */
    }
  /* for */

  if (TRUE == result)
    {
      result = WaitForMultipleObjectsEx((DWORD)(sz), &(hnd[0]), FALSE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAny */



//! Waits for specified events.
/*!
  Waits for all events. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID    Reference to vector of event IDs.
  \param H     Reference to vector of event identifiers. Must have the same number of elements as ID.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAll(
                                       std::vector<SynchronizationCodes> & ID,
                                       std::vector<int> & H,
                                       DWORD const dwMilliseconds
                                       )
{
  assert( ID.size() == H.size() );
  if ( ID.size() != H.size() ) return WAIT_FAILED;

  size_t const sz = ID.size();
  assert( sz < MAXIMUM_WAIT_OBJECTS);
  if ( sz >= MAXIMUM_WAIT_OBJECTS ) return WAIT_FAILED;

  std::vector<HANDLE> hnd(sz, (HANDLE)(NULL));

  DWORD result = TRUE;
  for (int i = 0; i < (int)sz; ++i)
    {
      bool const get_handle = GetEventHandle_inline(this, ID[i], H[i], &(hnd[i]), NULL, NULL, NULL, NULL);
      assert(true == get_handle);
      assert((HANDLE)(NULL) != hnd[i]);
      if ( (false == get_handle) || ((HANDLE)(NULL) == hnd[i]) )
        {
          result = FALSE;
          break;
        }
      /* if */
    }
  /* for */

  if (TRUE == result)
    {
      result = WaitForMultipleObjectsEx((DWORD)(sz), &(hnd[0]), TRUE, dwMilliseconds, TRUE);
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAll */



//! Waits for specified events.
/*!
  Waits for specified events. Call is wrapped to WaitForMultipleObjectsEx.

  \param ID_any    Reference to vector of event IDs of which any may be signalled.
  \param H_any     Reference to vector of event identifiers. Must have the same number of elements as ID_any.
  \param ID_all    Reference to vector of event IDs which must all be signalled.
  \param H_all     Reference to vector of event identifiers. Must have the same number of elements as ID_all.
  \param dwMilliseconds Time-out in milliseconds.
  \return Returns the result of WaitForMultipleObjectsEx operation.
*/
DWORD
SynchronizationEvents::EventWaitForAnyAndAll(
                                             std::vector<SynchronizationCodes> & ID_any,
                                             std::vector<int> & H_any,
                                             std::vector<SynchronizationCodes> & ID_all,
                                             std::vector<int> & H_all,
                                             DWORD const dwMilliseconds
                                             )
{
  assert( ID_any.size() == H_any.size() );
  if ( ID_any.size() != H_any.size() ) return WAIT_FAILED;

  assert( ID_all.size() == H_all.size() );
  if ( ID_all.size() != H_all.size() ) return WAIT_FAILED;

  int const n_any = (int)( ID_any.size() );
  int const n_all = (int)( ID_all.size() );
  int const sz = n_any + n_all;

  assert( sz < MAXIMUM_WAIT_OBJECTS);
  if ( sz >= MAXIMUM_WAIT_OBJECTS ) return WAIT_FAILED;

  // Assume success.
  DWORD result = TRUE;

  // Fetch handles.
  std::vector<HANDLE> hnd(sz, (HANDLE)(NULL));
  for (int i = 0; i < sz; ++i)
    {
      bool get_handle = true;
      if (i < n_any)
        {
          assert( (0 <= i) && (i < n_any) );
          get_handle = GetEventHandle_inline(this, ID_any[i], H_any[i], &(hnd[i]), NULL, NULL, NULL, NULL);
        }
      else
        {
          int const j = i - n_any;
          assert( (0 <= j) && (j < n_all) );
          get_handle = GetEventHandle_inline(this, ID_all[j], H_all[j], &(hnd[i]), NULL, NULL, NULL, NULL);
        }
      /* if */
      assert(true == get_handle);
      assert((HANDLE)(NULL) != hnd[i]);
      if ( (false == get_handle) || ((HANDLE)(NULL) == hnd[i]) )
        {
          result = FALSE;
          break;
        }
      /* if */
    }
  /* for */

  if (TRUE == result)
    {
      DWORD dwMillisecondsRemaining = dwMilliseconds;
      int n_all_not_signalled = n_all;

      bool id_any = false;
      bool id_all = false;
      do
        {
          assert(0 < n_all_not_signalled);
          int const n = n_any + n_all_not_signalled;

          result = WaitForMultipleObjectsEx((DWORD)(n), &(hnd[0]), FALSE, dwMillisecondsRemaining, TRUE);
          int const hnr = result - WAIT_OBJECT_0;
          if ( (0 <= hnr) && (hnr < n_any) )
            {
              id_any = true;
            }
          else if ( (n_any <= hnr) && (hnr < n) )
            {
              if (hnr + 1 != n)
                {
                  HANDLE const h1 = hnd[hnr];
                  hnd[hnr] = hnd[n - 1];
                  hnd[n - 1] = hnd[hnr];
                }
              /* if */
              n_all_not_signalled -= 1;
              if (0 == n_all_not_signalled) id_all = true;
            }
          else
            {
              break; // Wait error!
            }
          /* if */

          if (INFINITE != dwMillisecondsRemaining)
            {
              // TODO: Decrease waiting time!
            }
          /* if */
        }
      while ( (false == id_any) && (false == id_all) );
    }
  else
    {
      result = WAIT_FAILED;
    }
  /* if */

  return( result );
}
/* SynchronizationEvents::EventWaitForAnyAndAll */



/****** EXPORTED FUNCTIONS ******/

//! Deletes SynchronizationEvents structure.
/*!
  Releases all resources acquired for SynchronizationEvents structure.

  \param P    Pointer to SynchronizationEvents structure.
*/
void
DeleteSynchronizationEventsStructure(
                                     SynchronizationEvents * const P
                                     )
{
  assert(NULL != P);
  if(NULL == P) return;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    P->ImageDecoder.clear();
    P->ImageEncoder.clear();
    P->Draw.clear();
    P->Camera.clear();
    P->Main.clear();
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  delete P;
}
/* DeleteSynchronizationEventsStructure */



//! Creates SynchronizationEvents structure.
/*!
  Allocates memory and creates resources for SynchronizationEvents structure.

  \return Pointer to SynchronizationEvents structure or NULL if unsuccessfull.
*/
SynchronizationEvents *
CreateSynchronizationEventsStructure(
                                     void
                                     )
{
  SynchronizationEvents * const P = new SynchronizationEvents_;
  assert(NULL != P);
  if(NULL == P) return P;

  /* Create SRW lock. */
  InitializeSRWLock( &(P->lock) );

  /* Reserve space for 10 threads. */
  P->ImageDecoder.reserve(10);
  P->ImageEncoder.reserve(10);
  P->Draw.reserve(10);
  P->Camera.reserve(10);

  /* Create only events for the main thread. */
  int const ID = GetCurrentProcessId();
  int const H = 0;

  P->Main.resize(H + 1);
  HRESULT const hr = SynchronizationEventsMainCreate_inline(nth_ptr(P->Main, H), ID, H);
  assert( SUCCEEDED(hr) );

  if ( !SUCCEEDED(hr) )
    {
      DeleteSynchronizationEventsStructure( P );
      return NULL;
    }
  /* if */

  return P;
}
/* CreateSynchronizationEventsStructure */



//! Add decoder.
/*!
  Adds image decoder to synchronization events structure.

  \param P      Pointer to SynchronizationEvents structure.
  \return Returns camera ID or -1 if unsuccessfull.
*/
int
AddImageDecoderToSynchronizationEventsStructure(
                                                SynchronizationEvents * const P
                                                )
{
  assert(NULL != P);
  if(NULL == P) return -1;

  int const ID = GetCurrentProcessId();
  int H = -1;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    H = (int)( P->ImageDecoder.size() );
    P->ImageDecoder.resize(H + 1);

    SynchronizationEventsImageDecoder * const ptr = nth_ptr(P->ImageDecoder, H);
    assert(NULL != ptr);

    HRESULT const hr = SynchronizationEventsImageDecoderCreate_inline(ptr, ID, H);
    assert( SUCCEEDED(hr) );

    if ( !SUCCEEDED(hr) ) H = -1;
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return H;
}
/* AddImageDecoderToSynchronizationEventsStructure */



//! Add encoder.
/*!
  Adds image encoder to synchronization events structure.

  \param P      Pointer to SynchronizationEvents structure.
  \return Returns camera ID or -1 if unsuccessfull.
*/
int
AddImageEncoderToSynchronizationEventsStructure(
                                                SynchronizationEvents * const P
                                                )
{
  assert(NULL != P);
  if(NULL == P) return -1;

  int const ID = GetCurrentProcessId();
  int H = -1;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    H = (int)( P->ImageEncoder.size() );
    P->ImageEncoder.resize(H + 1);

    SynchronizationEventsImageEncoder * const ptr = nth_ptr(P->ImageEncoder, H);
    assert(NULL != ptr);

    HRESULT const hr = SynchronizationEventsImageEncoderCreate_inline(ptr, ID, H);
    assert( SUCCEEDED(hr) );

    if ( !SUCCEEDED(hr) ) H = -1;
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return H;
}
/* AddImageEncoderToSynchronizationEventsStructure */



//! Add projector.
/*!
  Adds projector to synchronization events structure.

  \param P      Pointer to SynchronizationEvents structure.
  \return Returns camera ID or -1 if unsuccessfull.
*/
int
AddProjectorToSynchronizationEventsStructure(
                                             SynchronizationEvents * const P
                                             )
{
  assert(NULL != P);
  if(NULL == P) return -1;

  int const ID = GetCurrentProcessId();
  int H = -1;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    H = (int)( P->Draw.size() );
    P->Draw.resize(H + 1);

    SynchronizationEventsDraw * const ptr = nth_ptr(P->Draw, H);
    assert(NULL != ptr);

    HRESULT const hr = SynchronizationEventsDrawCreate_inline(ptr, ID, H);
    assert( SUCCEEDED(hr) );

    if ( !SUCCEEDED(hr) ) H = -1;
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return H;
}
/* AddProjectorToSynchronizationEventsStructure */



//! Add camera.
/*!
  Adds camera to synchronization events structure.

  \param P      Pointer to SynchronizationEvents structure.
  \return Returns camera ID or -1 if unsuccessfull.
*/
int
AddCameraToSynchronizationEventsStructure(
                                          SynchronizationEvents * const P
                                          )
{
  assert(NULL != P);
  if(NULL == P) return -1;

  int const ID = GetCurrentProcessId();
  int H = -1;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    H = (int)( P->Camera.size() );
    P->Camera.resize(H + 1);

    SynchronizationEventsCamera * const ptr = nth_ptr(P->Camera, H);
    assert(NULL != ptr);

    HRESULT const hr = SynchronizationEventsCameraCreate_inline(ptr, ID, H);
    assert( SUCCEEDED(hr) );

    if ( !SUCCEEDED(hr) ) H = -1;
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return H;
}
/* AddCameraToSynchronizationEventsStructure */



//! Remove decoder.
/*!
  Removes image decoder from synchronization events structure.
  Note that the underlying vector storing events is changed only if the decoder to be removed is the last one.
  If decoder to be removed is in the middle of the vector then only events are deallocated.

  \param P      Pointer to SynchronizationEvents structure.
  \param H      ID of the decoder to remove.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
RemoveImageDecoderFromSynchronizationEventsStructure(
                                                     SynchronizationEvents * const P,
                                                     int const H
                                                     )
{
  assert(NULL != P);
  if(NULL == P) return E_POINTER;

  HRESULT hr = S_OK;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    SynchronizationEventsImageDecoder * const ptr = nth_ptr(P->ImageDecoder, H);
    assert(NULL != ptr);

    hr = SynchronizationEventsImageDecoderClose_inline(ptr);
    assert( SUCCEEDED(hr) );

    int const sz = (int)( P->ImageDecoder.size() );
    if (H + 1 == sz) P->ImageDecoder.pop_back();
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return hr;
}
/* RemoveImageDecoderFromSynchronizationEventsStructure */



//! Remove encoder.
/*!
  Removes image encoder from synchronization events structure.
  Note that the underlying vector storing events is changed only if the encoder to be removed is the last one.
  If encoder to be removed is in the middle of the vector then only events are deallocated.

  \param P      Pointer to SynchronizationEvents structure.
  \param H      ID of the encoder to remove.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
RemoveImageEncoderFromSynchronizationEventsStructure(
                                                     SynchronizationEvents * const P,
                                                     int const H
                                                     )
{
  assert(NULL != P);
  if(NULL == P) return E_POINTER;

  HRESULT hr = S_OK;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    SynchronizationEventsImageEncoder * const ptr = nth_ptr(P->ImageEncoder, H);
    assert(NULL != ptr);

    hr = SynchronizationEventsImageEncoderClose_inline(ptr);
    assert( SUCCEEDED(hr) );

    int const sz = (int)( P->ImageEncoder.size() );
    if (H + 1 == sz) P->ImageEncoder.pop_back();
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return hr;
}
/* RemoveImageEncoderFromSynchronizationEventsStructure */



//! Remove projector.
/*!
  Removes projector from synchronization events structure.
  Note that the underlying vector storing events is changed only if the projector to be removed is the last one.
  If projector to be removed is in the middle of the vector then only events are deallocated.

  \param P      Pointer to SynchronizationEvents structure.
  \param H      ID of the projector to remove.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
RemoveProjectorFromSynchronizationEventsStructure(
                                                  SynchronizationEvents * const P,
                                                  int const H
                                                  )
{
  assert(NULL != P);
  if(NULL == P) return E_POINTER;

  HRESULT hr = S_OK;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    SynchronizationEventsDraw * const ptr = nth_ptr(P->Draw, H);
    assert(NULL != ptr);

    hr = SynchronizationEventsDrawClose_inline(ptr);
    assert( SUCCEEDED(hr) );

    int const sz = (int)( P->Draw.size() );
    if (H + 1 == sz) P->Draw.pop_back();
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return hr;
}
/* RemoveProjectorFromSynchronizationEventsStructure */



//! Remove camera.
/*!
  Removes camera from synchronization events structure.
  Note that the underlying vector storing events is changed only if the camera to removed is the last one.
  If camera to be removed is in the middle of the vector then only events are deallocated.

  \param P      Pointer to SynchronizationEvents structure.
  \param H      ID of the camera to remove.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
RemoveCameraFromSynchronizationEventsStructure(
                                               SynchronizationEvents * const P,
                                               int const H
                                               )
{
  assert(NULL != P);
  if(NULL == P) return E_POINTER;

  HRESULT hr = S_OK;

  AcquireSRWLockExclusive( &(P->lock) );
  {
    SynchronizationEventsCamera * const ptr = nth_ptr(P->Camera, H);
    assert(NULL != ptr);

    hr = SynchronizationEventsCameraClose_inline(ptr);
    assert( SUCCEEDED(hr) );

    int const sz = (int)( P->Camera.size() );
    if (H + 1 == sz) P->Camera.pop_back();
  }
  ReleaseSRWLockExclusive( &(P->lock) );

  return hr;
}
/* RemoveCameraFromSynchronizationEventsStructure */



#endif /* !__BATCHACQUISITIONEVENTS_CPP */
