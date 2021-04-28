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
  \file   BatchAcquisitionDebug.h
  \brief  Helper functions for easier debugging.

  \author Tomislav Petkovic
  \date   2017-01-03
*/


#ifndef __BATCHACQUISITIONDEBUG_H
#define __BATCHACQUISITIONDEBUG_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"


//! Exception ID to push the thread info to the MS Visual Studio debugger.
const DWORD MS_VC_EXCEPTION=0x406D1388;


#pragma pack(push,8)

//! Thread information for the MS Visual Studio debugger.
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; //!< Must be 0x1000.
  LPCSTR szName; //!< Pointer to name (in user addr space).
  DWORD dwThreadID; //!< Thread ID (-1=caller thread).
  DWORD dwFlags; //!< Reserved for future use, must be zero.
} THREADNAME_INFO;

#pragma pack(pop)


//! Set thread name for MSVC.
void SetThreadNameForMSVC(DWORD const, char const * const);

//! Set thread name and ID for MSVC.
void SetThreadNameAndIDForMSVC(DWORD const, char const * const, int const);



/****** THREAD STATE ******/

//! Past event data.
/*!
  All threads are implemented as state machines that react to specific events.
  This structure holds data which describes one processed event.
*/
typedef struct PastEventData_
{
  int code; //!< Event code.

  double duration; //!< Event processing time.
  double elapsed; //!< Elapsed time in ms from the previous event.

  LONGLONG QPC_added; //!< QPC value at the time event was added.
  LONGLONG QPC_processed; //!< QPC value at the time event was processed.
} PastEventData;


//! Structure to store past events.
/*!
  All threads are implemented as state machine that react to specific events. Depeding
  on the event sequence in time the state of the thread will be different. This structure
  may be used to store past states and facilitate simpler debugging.
*/
typedef struct PastEvents_
{
  static int const num_codes = 100; //!< Number of states we may store.

  double ticks_to_ms; /*!< Muliplication factor to convert ticks to ms. */

  int idx; //!< Index where to store next state.
  PastEventData event_data[num_codes]; //!< Array holding event data.
} PastEvents;


//! Deletes past events storage.
void PastEventsDelete(PastEvents * const);

//! Creates past events storage.
PastEvents * PastEventsCreate();

//! Add event to storage.
void AddEvent(PastEvents * const, int const);

//! Add event processing time.
void EventProcessed(PastEvents * const);

//! Add event processing time.
void PreviousEventProcessed(PastEvents * const);

//! Get current event.
bool GetCurrentEvent(PastEvents * const, int * const, double * const, LONGLONG * const, LONGLONG * const);

//! Get previous event.
bool GetPreviousEvent(PastEvents * const, int * const, double * const, LONGLONG * const, LONGLONG * const);



/****** WINDOW MESSAGES ******/

//! Structure to store past messages.
/*!
  This structure is used to store past messages that were given to a
  window message processing function.
*/
typedef struct _PastMessages
{
  static int const num_messages = 100; //!< Number of messages we may store.

  int idx; //!< Index where to store next message.
  UINT message[num_messages]; //!< Array holding past messages.
  WPARAM wParam[num_messages]; //!< Array holding parameters.
  LPARAM lParam[num_messages]; //!< Array holding parameters.
} PastMessages;

#if defined(_DEBUG) || defined(DEBUG) || defined(__BATCHACQUISITIONDEBUG_CPP)

//! Outputs received window message.
void PrintWindowMessageToConsole(HWND const, UINT const, WPARAM const, LPARAM const);

//! Deletes past messages storage.
void PastMessagesDelete(PastMessages * const);

//! Deletes past messages storage.
PastMessages * PastMessagesCreate(void);

//! Add message to storage.
void AddMessage(PastMessages * const, UINT const, WPARAM const, LPARAM const);

#else

//! Replacement macro for release target to remove debug functions.
#define PrintWindowMessageToConsole(...)

//! Replacement macro for release target to remove debug functions.
#define PastMessagesDelete(...)

//! Replacement macro for release target to remove debug functions.
#define PastMessagesCreate(...) NULL

//! Replacement macro for release target to remove debug functions.
#define AddMessage(...)

#endif


/****** MESSAGE OUTPUT AND SINGAL TESTING ******/

#if defined(_DEBUG) || defined(DEBUG) || defined(__BATCHACQUISITIONDEBUG_CPP) || defined(__BATCHACQUISITIONPROCESSING_CPP)

//! Print debug message.
int
Debugfprintf(
             FILE *,
             char const * const,
             ...
             );

//! Print debug message.
int
Debugfwprintf(
              FILE *,
              wchar_t const * const,
              ...
              );

//! Check singla state.
bool
DebugIsSignalled(
                 SynchronizationEvents * const,
                 SynchronizationCodes const,
                 int const
                 );

#else

//! Replacement macro for release target to remove debug functions.
#define Debugfprintf(...) 0

//! Replacement macro for release target to remove debug functions.
#define Debugfwprintf(...) 0

//! Replacement macro for release target to remove debug functions.
#define DebugIsSiganlled(...) false

#endif



/****** TIME MEASUREMENT ******/

//! Timer storage.
/*!
  This structure is used to store times when measuring the perfomance.
*/
typedef
struct DEBUG_TIMER_
{
  LARGE_INTEGER frequency; /*!< CPU frequency. */
  double invfrq; /*!< Duration of one CPU clock. */

  LARGE_INTEGER clockStart; /*!< Start clock; this is set in DebugTimerInit only. */
  LARGE_INTEGER clockStop; /*!< Last time DebugTimerQueryStop was was called. */

  LARGE_INTEGER tic; /*!< Last time DebugTimerQueryTic was called. */
  LARGE_INTEGER toc; /*!< Last time DebugTimerQueryToc was called. */

  double elapsed; /*!< Elapsed time in ms. */
} DEBUG_TIMER;



#if defined(_DEBUG) || defined(DEBUG) || defined(__BATCHACQUISITIONDEBUG_CPP) || defined(__BATCHACQUISITIONPROCESSING_CPP)

//! Record current time.
DEBUG_TIMER *
DebugTimerInit(
               void
               );

//! Return elapsed time from start.
double
DebugTimerQueryStart(
                     DEBUG_TIMER * const
                     );

//! Return elapsed time from last query.
double
DebugTimerQueryLast(
                    DEBUG_TIMER * const
                    );

//! Store current QPC time.
void
DebugTimerQueryTic(
                   DEBUG_TIMER * const
                   );

//! Return elapsed time from last tic.
double
DebugTimerQueryToc(
                   DEBUG_TIMER * const
                   );

//! Destroy timer structure.
void
DebugTimerDestroy(
                  DEBUG_TIMER * const
                  );

#else

//! Replacement macro for release target to remove debug functions.
#define DebugTimerInit(...) NULL

//! Replacement macro for release target to remove debug functions.
#define DebugTimerQueryStart(...) 0

//! Replacement macro for release target to remove debug functions.
#define DebugTimerQueryLast(...) 0

//! Replacement macro for release target to remove debug functions.
#define DebugTimerDestroy(...)

#endif



/****** BREAK ON TIME-CRITICAL ACTIONS ******/


#if defined(_DEBUG) || defined(DEBUG)

//! Time the wait operation.
void DebugEnterCriticalSection(LPCRITICAL_SECTION, LARGE_INTEGER const, bool const);

#else

//! Time the wait operation.
/*!
  Inline variant for release build which does not time the operation.

  \param lpCriticalSection      A pointer to the critical section object.
  \param timeout        Allowed time in ticks to wait for ownership of the critical section object.
  \param stop_execution Flag to indicate debugger should be invoked if the time limit is exceeded.
*/
inline
void
DebugEnterCriticalSection(
                          LPCRITICAL_SECTION lpCriticalSection,
                          LARGE_INTEGER const timeout,
                          bool const stop_execution
                          )
{
  EnterCriticalSection(lpCriticalSection);
}
/* DebugEnterCriticalSection */


#endif



#endif /* !__BATCHACQUISITIONDEBUG_H */
