/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2017 UniZG, Zagreb. All rights reserved.
 * (c) 2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionTiming.cpp
  \brief  Timers and timing routines.

  This file contains functions for precise timing which use Windows QPC API.

  \author Tomislav Petkovic
  \date   2017-02-27
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONTIMING_CPP
#define __BATCHACQUISITIONTIMING_CPP



#include "BatchACquisitionTiming.h"



/****** SPINLOCK TIMER ******/


//! Clear SpinlockTimer data.
/*!
  Clears SpinlockTimer data.

  \param ptr    Pointer to SpinlockTimer structure.
*/
inline
void
SpinlockTimerClearData_inline(
                              SpinlockTimer * const ptr
                              )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->frequency.QuadPart = (LONGLONG)0;

  ptr->ticks_in_us = 0.0;
  ptr->ticks_to_ms = 0.0;

  ptr->start.QuadPart = (LONGLONG)0;
  ptr->stop.QuadPart = (LONGLONG)0;

  ptr->delta.QuadPart = (LONGLONG)0;
}
/* SpinlockTimerClearData_inline */



//! Delete microsecond spinlock timer.
/*!
  Deletes SpinlockTimer structure and frees allocated memory.

  \param ptr    Pointer to SpinlockTimer structure.
*/
void
SpinlockTimerDelete(
                    SpinlockTimer * const ptr
                    )
{
  SpinlockTimerClearData_inline( ptr );
  if (NULL != ptr) free(ptr);
}
/* SpinlockTimerDelete */



//! Create microsecond spinlock timer.
/*!
  Creates and initialized microsecond spinlock timer.

  \return Returns pointer to timer structure if successfull, NULL pointer otherwise.
*/
SpinlockTimer *
SpinlockTimerCreate(
                    void
                    )
{
  SpinlockTimer * const ptr = (SpinlockTimer *)malloc( sizeof(SpinlockTimer) );
  assert(NULL != ptr);
  if (NULL == ptr) return ptr;

  SpinlockTimerClearData_inline( ptr );

  BOOL const res = QueryPerformanceFrequency( &(ptr->frequency) );
  assert(FALSE != res);

  ptr->ticks_in_us = (double)(ptr->frequency.QuadPart) / 1000000.0;
  assert(1 < ptr->ticks_in_us); // Must have better than 1 us resolution.

  ptr->ticks_in_ms = (double)(ptr->frequency.QuadPart) / 1000.0;

  ptr->ticks_to_ms = 1000.0 / (double)(ptr->frequency.QuadPart);

  return ptr;
}
/* SpinlockTimerCreate */



//! Set waiting time.
/*!
  Sets waiting interval in us (microseconds).

  \param ptr    Pointer to SpinlockTimer structure.
  \param delay  Delay time in us (microseconds). Must be positive.
*/
void
SpinlockTimerSetWaitIntervalInMicroseconds(
                                           SpinlockTimer * const ptr,
                                           int const delay
                                           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(0 < delay);
  if (0 >= delay)
    {
      ptr->delta.QuadPart = (LONGLONG)0;
      return;
    }
  /* if */

  double const delta = floor( (double)(delay) * ptr->ticks_in_us );
  ptr->delta.QuadPart = (LONGLONG)delta;
}
/* SpinlockTimerSetWaitIntervalInMicroseconds */



//! Set waiting time.
/*!
  Sets waiting interval in us (microseconds).

  \param ptr    Pointer to SpinlockTimer structure.
  \param delay  Delay time in us (microseconds). Must be positive.
*/
void
SpinlockTimerSetWaitIntervalInMicroseconds(
                                           SpinlockTimer * const ptr,
                                           double const delay
                                           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(0 <= delay);
  if (0 >= delay)
    {
      ptr->delta.QuadPart = (LONGLONG)0;
      return;
    }
  /* if */

  double const delta = floor( delay * ptr->ticks_in_us );
  ptr->delta.QuadPart = (LONGLONG)delta;
}
/* SpinlockTimerSetWaitIntervalInMicroseconds */



//! Set waiting time.
/*!
  Sets waiting interval in us (milliseconds).

  \param ptr    Pointer to SpinlockTimer structure.
  \param delay  Delay time in ms (milliseconds). Must be positive.
*/
void
SpinlockTimerSetWaitIntervalInMilliseconds(
                                           SpinlockTimer * const ptr,
                                           int const delay
                                           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(0 <= delay);
  if (0 >= delay)
    {
      ptr->delta.QuadPart = (LONGLONG)0;
      return;
    }
  /* if */

  double const delta = floor( (double)(delay) * ptr->ticks_in_ms );
  ptr->delta.QuadPart = (LONGLONG)delta;
}
/* SpinlockTimerSetWaitIntervalInMilliseconds */



//! Set waiting time.
/*!
  Sets waiting interval in us (milliseconds).

  \param ptr    Pointer to SpinlockTimer structure.
  \param delay  Delay time in ms (milliseconds). Must be positive.
*/
void
SpinlockTimerSetWaitIntervalInMilliseconds(
                                           SpinlockTimer * const ptr,
                                           double const delay
                                           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(0 <= delay);
  if (0 >= delay)
    {
      ptr->delta.QuadPart = (LONGLONG)0;
      return;
    }
  /* if */

  double const delta = floor( delay * ptr->ticks_in_ms );
  ptr->delta.QuadPart = (LONGLONG)delta;
}
/* SpinlockTimerSetWaitIntervalInMilliseconds */



//! Wait for specified time.
/*!
  Waits for specified time by spinlocking.
  Function returns on next tick after requested time.

  \param ptr    Pointer to SpinlockTimer structure.
*/
void SpinlockTimerWait(
                       SpinlockTimer * const ptr
                       )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  BOOL const res = QueryPerformanceCounter( &(ptr->start) );
  assert(TRUE == res);

  if (0 == ptr->delta.QuadPart)
    {
      ptr->stop = ptr->start;
      return;
    }
  /* if */

  LONGLONG stop = ptr->start.QuadPart + ptr->delta.QuadPart;

  do
    {
      BOOL const res = QueryPerformanceCounter( &(ptr->stop) );
      assert(TRUE == res);
    }
  while (ptr->stop.QuadPart < stop);
}
/* SpinlockTimerWait */



//! Wait for specified time form specified tick count.
/*!
  Waits for specified time by spinlocking.
  The waiting time starts from the specified tick count.
  Function returns on next tick after the requested time has elapsed.

  \param ptr    Pointer to SpinlockTimer structure.
  \param start  Starting QPC value for the spinlock timer.
*/
void SpinlockTimerWaitFrom(
                           SpinlockTimer * const ptr,
                           LARGE_INTEGER const start
                           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

#ifdef _DEBUG
  {
    LARGE_INTEGER current;

    BOOL const res = QueryPerformanceCounter( &current );
    assert(TRUE == res);

    assert( start.QuadPart <= current.QuadPart ); // Starting time should be in the past.
  }
#endif

  ptr->start = start; // Store starting time.

  LONGLONG const stop = ptr->start.QuadPart + ptr->delta.QuadPart;

  do
    {
      BOOL const res = QueryPerformanceCounter( &(ptr->stop) );
      assert(TRUE == res);
    }
  while (ptr->stop.QuadPart < stop);
}
/* SpinlockTimerWaitFrom */



//! Wait for specified time form specified tick count.
/*!
  Waits for specified time by spinlocking.
  The waiting time starts from the specified tick count.
  Function returns on next tick after the requested time has elapsed.

  \param ptr    Pointer to SpinlockTimer structure.
  \param start  Starting QPC value for the spinlock timer.
  \param stop  QPC value to wait for.
*/
void SpinlockTimerWaitFromTo(
                             SpinlockTimer * const ptr,
                             LARGE_INTEGER const start,
                             LARGE_INTEGER const stop
                             )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  LARGE_INTEGER current;

  BOOL const res = QueryPerformanceCounter( &current );
  assert(TRUE == res);

  // Due to QPC fetch overhead current time may be slightly off in regard to start and stop times.
  //assert( start.QuadPart <= current.QuadPart ); // Starting time should be in the past.
  //assert( current.QuadPart < stop.QuadPart ); // Stopping time should be in the future.
  assert( start.QuadPart <= stop.QuadPart ); // Stopping time should be after starting time.

  if ( current.QuadPart > stop.QuadPart )
    {
      ptr->start = start; // Store starting time.
      ptr->stop = current;
      return; // Skip waiting for a moment in the past.
    }
  else if (start.QuadPart > stop.QuadPart)
    {
      ptr->start = current;
      ptr->stop = current;
      return; // Skip waiting if times are in wrong order.
    }
  /* if */

  ptr->start = start; // Store starting time.

  do
    {
      BOOL const res = QueryPerformanceCounter( &(ptr->stop) );
      assert(TRUE == res);
    }
  while (ptr->stop.QuadPart < stop.QuadPart);
}
/* SpinlockTimerWaitFromTo */



//! Wait for specified tick count.
/*!
  Waits for specified time by spinlocking.
  Function returns on next tick after the QPC counter exceeds given value.

  \param ptr    Pointer to SpinlockTimer structure.
  \param stop  QPC value to wait for.
*/
void SpinlockTimerWaitTo(
                         SpinlockTimer * const ptr,
                         LARGE_INTEGER const stop
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  BOOL const res = QueryPerformanceCounter( &(ptr->start) );
  assert(TRUE == res);

  assert( ptr->start.QuadPart < stop.QuadPart );
  if (ptr->start.QuadPart > stop.QuadPart)
    {
      ptr->stop = stop;
      return; // Skip waiting for moment in the past.
    }
  /* if */

  do
    {
      BOOL const res = QueryPerformanceCounter( &(ptr->stop) );
      assert(TRUE == res);
    }
  while (ptr->stop.QuadPart < stop.QuadPart);
}
/* SpinlockTimerWaitTo */



//! Duration of last wait.
/*!
  Function returns duration of last wait interval in ms.

  \param ptr    Pointer to SpinlockTimer structure.
  \return Returns duration in ms or NaN if unsuccessfull.
*/
double
SpinlockTimerLastWaitDuration(
                              SpinlockTimer * const ptr
                              )
{
  assert(NULL != ptr);
  if (NULL == ptr) return BATCHACQUISITION_qNaN_dv;

  double const wait_time = (double)(ptr->stop.QuadPart - ptr->start.QuadPart) * ptr->ticks_to_ms;

  return wait_time;
}
/* SpinlockTimerLastWaitDuration */



/****** ACQUISITION STATISTICS ******/

//! Clears FrameStatistics data.
/*!
  Clears FrameStatistics data (resets to initial state).

  \see FrameStatisticsBlank_inline

  \param ptr    Pointer to FrameStatistics structure.
*/
inline
void
FrameStatisticsClearData_inline(
                                FrameStatistics * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->start.QuadPart = (LONGLONG)0;
  ptr->stop.QuadPart = (LONGLONG)0;
  ptr->tic.QuadPart = (LONGLONG)0;
  ptr->toc.QuadPart = (LONGLONG)0;
  ptr->min = 0.0;
  ptr->max = 0.0;
  ptr->length = 0.0;
  ptr->mean = 0.0;
  ptr->M2 = 0.0;
  ptr->n_events = 0.0;
  ptr->initialized = false;
  ptr->tictoc = false;
}
/* FrameStatisticsClearData_inline */



//! Blanks FrameStatistics structure.
/*!
  Blanks FrameStatistics structure.

  \param ptr    Pointer to FrameStatistics structure.
*/
inline
void
FrameStatisticsBlank_inline(
                            FrameStatistics * const ptr
                            )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->frequency.QuadPart = (LONGLONG)0;
  ptr->invfrq = 0.0;
  ZeroMemory( &(ptr->sStatisticsLock), sizeof(ptr->sStatisticsLock) );

  FrameStatisticsClearData_inline(ptr);
}
/* FrameStatisticsBlank_inline */



//! Delete frame statistics.
/*!
  Deletes frame statistics structure.

  \param ptr    Pointer to FrameStatistics structure.
*/
void
FrameStatisticsDelete(
                      FrameStatistics * const ptr
                      )
{
  FrameStatisticsBlank_inline(ptr);
  if (NULL != ptr) free(ptr);
}
/* FrameStatisticsDelete */



//! Create frame statistics.
/*!
  Creates frame statistics structure.

  \return Returns pointer to FrameStatistics structure or NULL if unsuccessfull.
*/
FrameStatistics *
FrameStatisticsCreate(
                      void
                      )
{
  FrameStatistics * const ptr = (FrameStatistics *)malloc(sizeof(FrameStatistics));
  assert(NULL != ptr);
  if (NULL == ptr) return ptr;

  FrameStatisticsBlank_inline(ptr);

  InitializeSRWLock( &(ptr->sStatisticsLock) );

  BOOL const res = QueryPerformanceFrequency( &(ptr->frequency) );
  assert(FALSE != res);

  ptr->invfrq = 1000.0 / (double)(ptr->frequency.QuadPart);

  return ptr;
}
/* FrameStatisticsCreate */



//! Add new measurement.
/*!
  Adds new measurement value and updates state.

  \param ptr    Pointer to FrameStatistics structure.
  \param value  Measurement value to add.
*/
void
FrameStatisticsAddValue_inline(
                               FrameStatistics * const ptr,
                               double const value
                               )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  /* Track extremal data values. */
  if (0.0 < ptr->length)
    {
      if (ptr->min > value) ptr->min = value;
      if (ptr->max < value) ptr->max = value;
    }
  else
    {
      ptr->min = value;
      ptr->max = value;
    }
  /* if */

  /* Increase counter by one. */
  ptr->length += 1.0;
  ptr->n_events += 1.0;

  /* Update statistics. */
  double const delta = value - ptr->mean;
  ptr->mean = ptr->mean  + delta / ptr->length;
  ptr->M2 = ptr->M2 + delta * (value - ptr->mean);
}
/* FrameStatisticsAddValue_inline */



//! Add new time measurement.
/*!
  Adds new time measurement. This function should be called after frame is displayed or acquired.
  It automatically queries performance counter and updates statistics. Minimum of two
  calls is required to have well defined mean value, and minimum of three calls is required
  to have well defined deviation.

  \param ptr    Pointer to FrameStatistics structure.
*/
void
FrameStatisticsAddFrame(
                        FrameStatistics * const ptr
                        )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return;

  AcquireSRWLockExclusive( &(ptr->sStatisticsLock) );

  if (true == ptr->initialized)
    {
      /* Compute duration. */
      double const value = (double)(ticks.QuadPart - ptr->stop.QuadPart) * ptr->invfrq;
      FrameStatisticsAddValue_inline(ptr, value);

      ptr->stop = ticks;
    }
  else
    {
      ptr->start = ticks;
      ptr->stop = ticks;
      ptr->initialized = true;
    }
  /* if */

  ReleaseSRWLockExclusive( &(ptr->sStatisticsLock) );
}
/* FrameStatisticsAddFrame */



//! Mark beginning of time measurement.
/*!
  Denotes start of measure interval. After calling this a call
  to FrameStatisticsToc is required.

  \param ptr Pointer to FrameStatistics structure.
*/
void
FrameStatisticsTic(
                   FrameStatistics * const ptr
                   )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return;

  AcquireSRWLockExclusive( &(ptr->sStatisticsLock) );

  assert(false == ptr->tictoc);
  ptr->tic = ticks;
  ptr->tictoc = true;

  if (false == ptr->initialized)
    {
      ptr->start = ticks;
      ptr->stop = ticks;
      ptr->initialized = true;
    }
  /* if */

  ReleaseSRWLockExclusive( &(ptr->sStatisticsLock) );
}
/* FrameStatisticsTic */



//! Mark end of time measurement.
/*!
  Denotes end of measure interval. After calling this a call
  to FrameStatisticsTic is required to start new measurement.

  \param ptr Pointer to FrameStatistics structure.
*/
void
FrameStatisticsToc(
                   FrameStatistics * const ptr
                   )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return;

  AcquireSRWLockExclusive( &(ptr->sStatisticsLock) );

  assert(true == ptr->tictoc);
  ptr->toc = ticks;
  ptr->tictoc = false;

  /* Compute duration. */
  double const value = (double)(ptr->toc.QuadPart - ptr->tic.QuadPart) * ptr->invfrq;
  FrameStatisticsAddValue_inline(ptr, value);

  ptr->stop = ticks;

  ReleaseSRWLockExclusive( &(ptr->sStatisticsLock) );
}
/* FrameStatisticsToc */



//! Returns last tic-toc interval.
/*!
  Returns last measured time interval.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns last tic-toc interval, or 0 if unsuccessfull.
*/
LARGE_INTEGER
FrameStatisticsLastTicTocInterval(
                                  FrameStatistics * const ptr
                                  )
{
  LARGE_INTEGER interval;
  interval.QuadPart = 0;

  assert(NULL != ptr);
  if (NULL == ptr) return interval;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    //assert(false == ptr->tictoc);
    interval.QuadPart = ptr->toc.QuadPart - ptr->tic.QuadPart;
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return interval;
}
/* FrameStatisticsLastTicTocInterval */



//! Add measurement.
/*!
  Adds time measurement.

  \param ptr    Pointer to FrameStatistics structure.
  \param tic    Time measurement start.
  \param toc    Time measurement stop.
*/
void
FrameStatisticsAddMeasurement(
                              FrameStatistics * const ptr,
                              LARGE_INTEGER const tic,
                              LARGE_INTEGER const toc
                              )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (0 >= tic.QuadPart) return;
  if (0 >= toc.QuadPart) return;

  assert(tic.QuadPart <= toc.QuadPart);

  AcquireSRWLockExclusive( &(ptr->sStatisticsLock) );

  /* Compute duration. */
  double const value = (double)(toc.QuadPart - tic.QuadPart) * ptr->invfrq;
  FrameStatisticsAddValue_inline(ptr, value);

  ReleaseSRWLockExclusive( &(ptr->sStatisticsLock) );
}
/* FrameStatisticsAddMeasurement */



//! Reset statistics.
/*!
  Resets collected statistics.

  \param ptr Pointer to FrameStatistics structure.
*/
void
FrameStatisticsReset(
                     FrameStatistics * const ptr
                     )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  AcquireSRWLockExclusive( &(ptr->sStatisticsLock) );

  LARGE_INTEGER const frequency = ptr->frequency;
  double const invfrq = ptr->invfrq;

  FrameStatisticsClearData_inline(ptr);

  ptr->frequency = frequency;
  ptr->invfrq = invfrq;

  ReleaseSRWLockExclusive( &(ptr->sStatisticsLock) );
}
/* FrameStatisticsReset */



//! Get mean.
/*!
  Returns mean value of collected timing data.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns mean value or NaN if unsuccessfull.
*/
double
FrameStatisticsGetMean(
                       FrameStatistics * const ptr
                       )
{
  double mean = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return mean;

  if (1.0 > ptr->length) return mean;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    assert(0.0 < ptr->length);
    mean = ptr->mean;
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return mean;
}
/* FrameStatisticsGetMean */



//! Get max.
/*!
  Returns maximal value of collected timing data.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns maxmimal value or NaN if unsuccessfull.
*/
double
FrameStatisticsGetMax(
                      FrameStatistics * const ptr
                      )
{
  double max = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return max;

  if (0.0 >= ptr->length) return max;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    assert(0.0 < ptr->length);
    max = ptr->max;
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return max;
}
/* FrameStatisticsGetMax */



//! Get min.
/*!
  Returns minimal value of collected timing data.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns minimal value or NaN if unsuccessfull.
*/
double
FrameStatisticsGetMin(
                      FrameStatistics * const ptr
                      )
{
  double min = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return min;

  if (0.0 >= ptr->length) return min;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    assert(0.0 < ptr->length);
    min = ptr->min;
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return min;
}
/* FrameStatisticsGetMin */


//! Get deviation.
/*!
  Returns standard deviation of collected timing data.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns standard deviation or NaN if unsuccessfull.
*/
double
FrameStatisticsGetDeviation(
                            FrameStatistics * const ptr
                            )
{
  double deviation = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return deviation;

  if (2.0 > ptr->length) return deviation;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    deviation = sqrt( ptr->M2 / (ptr->length - 1.0) );
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return deviation;
}
/* FrameStatisticsGetDeviation */



//! Get FPS.
/*!
  Returns average FPS for the duration of data acquistion.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns FPS value or NaN if unsuccessfull.
*/
double
FrameStatisticsGetFPS(
                      FrameStatistics * const ptr
                      )
{
  double FPS = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return FPS;

  if (1.0 >= ptr->n_events) return FPS;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    double const duration = (double)(ptr->stop.QuadPart - ptr->start.QuadPart) * ptr->invfrq;
    FPS = 1000.0 * (ptr->n_events - 1.0) / duration;
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return FPS;
}
/* FrameStatisticsGetFPS */



//! Get duration.
/*!
  Returns estimated total time of acquisition in seconds.

  Duration is time elapsed between first and last FrameStatisticsAddFrame
  calls increased for one average interframe duration.

  \param ptr    Pointer to FrameStatistics structure.
  \return Returns duration in seconds or NaN if unsuccessfull.
*/
double
FrameStatisticsGetTotalTime(
                            FrameStatistics * const ptr
                            )
{
  double duration = BATCHACQUISITION_qNaN_dv;

  assert(NULL != ptr);
  if (NULL == ptr) return duration;

  if (1.0 > ptr->length) return duration;

  AcquireSRWLockShared( &(ptr->sStatisticsLock) );
  {
    double const duration_ms = (double)(ptr->stop.QuadPart - ptr->start.QuadPart) * ptr->invfrq;
    duration = (duration_ms + ptr->mean) * 0.001; // Convert ms to s.
  }
  ReleaseSRWLockShared( &(ptr->sStatisticsLock) );

  return duration;
}
/* FrameStatisticsGetTotalTime */



//! Combines two frame statistics.
/*!
  Combines two frame statistics.

  \param src_1 First statistics.
  \param src_2 Second statistics.
  \return Function returns pointer to new statistics.
*/
FrameStatistics *
FrameStatisticsCombine(
                       FrameStatistics * const src_1,
                       FrameStatistics * const src_2
                       )
{
  assert(NULL != src_1);
  if (NULL == src_1) return NULL;

  assert(NULL != src_2);
  if (NULL == src_2) return NULL;

  FrameStatistics * const dst = FrameStatisticsCreate();
  assert(NULL != dst);
  if (NULL == dst) return dst;

  LARGE_INTEGER start_1; start_1.QuadPart = (LONGLONG)0;
  LARGE_INTEGER stop_1; stop_1.QuadPart = (LONGLONG)0;

  double min_1 = BATCHACQUISITION_qNaN_dv;
  double max_1 = BATCHACQUISITION_qNaN_dv;

  double n_1 = BATCHACQUISITION_qNaN_dv;
  double mu_1 = BATCHACQUISITION_qNaN_dv;
  double M2_1 = BATCHACQUISITION_qNaN_dv;

  bool have_1 = false;

  AcquireSRWLockShared( &(src_1->sStatisticsLock) );
  {
    have_1 = (true == src_1->initialized) && (1.0 <= src_1->n_events);
    if (true == have_1)
      {
        start_1 = src_1->start;
        stop_1 = src_1->stop;

        min_1 = src_1->min;
        max_1 = src_1->max;

        n_1 = src_1->length;
        mu_1 = src_1->mean;
        M2_1 = src_1->M2;

        assert(n_1 == src_1->n_events);
      }
    /* if */
  }
  ReleaseSRWLockShared( &(src_1->sStatisticsLock) );

  LARGE_INTEGER start_2; start_2.QuadPart = (LONGLONG)0;
  LARGE_INTEGER stop_2; stop_2.QuadPart = (LONGLONG)0;

  double min_2 = BATCHACQUISITION_qNaN_dv;
  double max_2 = BATCHACQUISITION_qNaN_dv;

  double n_2 = BATCHACQUISITION_qNaN_dv;
  double mu_2 = BATCHACQUISITION_qNaN_dv;
  double M2_2 = BATCHACQUISITION_qNaN_dv;

  bool have_2 = false;

  AcquireSRWLockShared( &(src_2->sStatisticsLock) );
  {
    have_2 = (true == src_2->initialized) && (1.0 <= src_2->n_events);
    if (true == have_2)
      {
        start_2 = src_2->start;
        stop_2 = src_2->stop;

        min_2 = src_2->min;
        max_2 = src_2->max;

        n_2 = src_2->length;
        mu_2 = src_2->mean;
        M2_2 = src_2->M2;

        assert(n_2 == src_2->n_events);
      }
    /* if */
  }
  ReleaseSRWLockShared( &(src_2->sStatisticsLock) );


  if ( (true == have_1) && (true == have_2) )
    {
      // Combine data.
      double const n = n_1 + n_2;
      double const mu = ( n_1 * mu_1 + n_2 * mu_2 ) / n;
      double const delta = mu_1 - mu_2;
      double const M2 = M2_1 + M2_2 + delta * delta * n_1 * n_2 / n;

      dst->start.QuadPart = (start_1.QuadPart < start_2.QuadPart)? start_1.QuadPart : start_2.QuadPart;
      dst->stop.QuadPart = (stop_1.QuadPart > stop_2.QuadPart)? stop_1.QuadPart : stop_2.QuadPart;

      dst->min = (min_1 < min_2)? min_1 : min_2;
      dst->max = (max_1 > max_2)? max_1 : max_2;

      dst->length = n;
      dst->mean = mu;
      dst->M2 = M2;

      dst->n_events = n;

      dst->initialized = true;
    }
  else if ( (true == have_1) && (false == have_2) )
    {
      // Copy from first source.
      dst->start = start_1;
      dst->stop = stop_1;

      dst->min = min_1;
      dst->max = max_1;

      dst->length = n_1;
      dst->mean = mu_1;
      dst->M2 = M2_1;

      dst->n_events = n_1;

      dst->initialized = true;
    }
  else if ( (false == have_1) && (true == have_2) )
    {
      // Copy from second source.
      dst->start = start_2;
      dst->stop = stop_2;

      dst->min = min_2;
      dst->max = max_2;

      dst->length = n_2;
      dst->mean = mu_2;
      dst->M2 = M2_2;

      dst->n_events = n_2;

      dst->initialized = true;
    }
  /* if */

  return dst;
}
/* FrameStatisticsCombine */



#endif /* !__BATCHACQUISITIONTIMING_CPP */
