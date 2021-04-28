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
  \file   BatchAcquisitionTiming.h
  \brief  Timers and timing routines.

  \author Tomislav Petkovic
  \date   2017-02-27
*/


#ifndef __BATCHACQUISITIONTIMING_H
#define __BATCHACQUISITIONTIMING_H


#include "BatchAcquisition.h"



/****** SPINLOCK TIMER ******/

//! Structure to store data required for microsecond timer.
/*!
  This structure stores data required for microsecond timer.
  Microsecond timer is QPC based and assumes it runs on an
  uniterruptable thread.
*/
typedef struct SpinlockTimer_
{
  LARGE_INTEGER frequency; /*!< CPU frequency. */

  double ticks_in_us; /*!< Number of CPU clocks in one microsecond. */
  double ticks_in_ms; /*!< Number of CPU clocks in one millisecond. */

  double ticks_to_ms; /*!< Muliplication factor to convert ticks to ms. */

  LARGE_INTEGER start; /*!< Tick count on timer start. */
  LARGE_INTEGER stop; /*!< Tick count on counter stop. */

  LARGE_INTEGER delta; /*!< Minimum difference for tick counter. */
} SpinlockTimer;


//! Delete microsecond spinlock timer.
void SpinlockTimerDelete(SpinlockTimer * const);

//! Create microsecond spinlock timer.
SpinlockTimer * SpinlockTimerCreate(void);

//! Set waiting time.
void SpinlockTimerSetWaitIntervalInMicroseconds(SpinlockTimer * const, int const);

//! Set waiting time.
void SpinlockTimerSetWaitIntervalInMicroseconds(SpinlockTimer * const, double const);

//! Set waiting time.
void SpinlockTimerSetWaitIntervalInMilliseconds(SpinlockTimer * const, int const);

//! Set waiting time.
void SpinlockTimerSetWaitIntervalInMilliseconds(SpinlockTimer * const, double const);

//! Wait for specified time.
void SpinlockTimerWait(SpinlockTimer * const);

//! Wait for specified time form specified tick count.
void SpinlockTimerWaitFrom(SpinlockTimer * const, LARGE_INTEGER const);

//! Wait for specified time form specified tick count to specified tick count.
void SpinlockTimerWaitFromTo(SpinlockTimer * const, LARGE_INTEGER const, LARGE_INTEGER const);

//! Wait for specified tick count.
void SpinlockTimerWaitTo(SpinlockTimer * const, LARGE_INTEGER const);

//! Duration of last wait.
double SpinlockTimerLastWaitDuration(SpinlockTimer * const);



/****** ACQUISITION STATISTICS ******/

//! Structure to store timing measurements.
/*!
  For both rendering and camera acquisition we would like to measure durations between
  two frames and compute immediate and average framerate. This structure stores the
  measured data. It can be used in two scenarios:

  1) Calling FrameStatisticsAddFrame for every frame. In this secenario duration
  between two consecutive calls to FrameStatisticsAddFrame is measured.

  2) Calling FrameStatisticsTic and FrameStatisticsToc for some action. Here duration
  between Tic and Toc calls is measured. However, FPS is not computed from this
  duration and is instead computed from durations between first Tic call and last
  Toc call. Therefore, output FPS and computed mean processing duration values
  are not directly related.
*/
typedef struct _FrameStatistics
{
  LARGE_INTEGER frequency; /*!< CPU frequency. */
  double invfrq; /*!< Duration of one CPU clock. */

  SRWLOCK sStatisticsLock; //!< Access lock.

  LARGE_INTEGER start; /*!< First time AddFrame was called. */
  LARGE_INTEGER stop; /*!< Last time AddFrame was called. */

  LARGE_INTEGER tic; /*!< Last time tic was called. */
  LARGE_INTEGER toc; /*!< Last time toc was called. */

  double min; /*!< Minimal measured frame time. */
  double max; /*!< Maximal measured frame time. */

  double length; /*!< Number of measured values. */
  double mean; /*!< Mean measured value. */
  double M2; /*!< Intermediate storage for on-line variance computation. */

  double n_events; /*!< Number of events in measurement interval. */

  bool initialized; /*!< Flag to indicate start time was initialized. */
  bool tictoc; /*!< Flag to indicate whether tic or toc was called last; true value denotes last call was tic, false denotes toc. */
} FrameStatistics;


//! Delete frame statistics.
void FrameStatisticsDelete(FrameStatistics * const);

//! Create frame statistics.
FrameStatistics * FrameStatisticsCreate(void);

//! Add new time measurement.
void FrameStatisticsAddFrame(FrameStatistics * const);

//! Mark beginning of time measurement.
void FrameStatisticsTic(FrameStatistics * const);

//! Mark end of time measurement.
void FrameStatisticsToc(FrameStatistics * const);

//! Returns last tic-toc interval.
LARGE_INTEGER FrameStatisticsLastTicTocInterval(FrameStatistics * const);

//! Add measurement.
void FrameStatisticsAddMeasurement(FrameStatistics * const, LARGE_INTEGER const, LARGE_INTEGER const);

//! Reset statistics.
void FrameStatisticsReset(FrameStatistics * const);

//! Get mean.
double FrameStatisticsGetMean(FrameStatistics * const);

//! Get max.
double FrameStatisticsGetMax(FrameStatistics * const);

//! Get min.
double FrameStatisticsGetMin(FrameStatistics * const);

//! Get deviation.
double FrameStatisticsGetDeviation(FrameStatistics * const);

//! Get FPS.
double FrameStatisticsGetFPS(FrameStatistics * const);

//! Get duration.
double FrameStatisticsGetTotalTime(FrameStatistics * const);

//! Combines two frame statistics.
FrameStatistics * FrameStatisticsCombine(FrameStatistics * const, FrameStatistics * const);



#endif /* !__BATCHACQUISITIONTIMING_H */
