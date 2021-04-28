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
  \file   BatchAcquisitionEvents.h
  \brief  Events for thread synchronization.

  \author Tomislav Petkovic
  \date   2017-02-06
*/


#ifndef __BATCHACQUISITIONEVENTS_H
#define __BATCHACQUISITIONEVENTS_H


#include "BatchAcquisition.h"




//! Enumeration for synchronization events.
/*!
  Events are used to synchronize threads.
*/
typedef
enum SynchronizationCodes_
  {
    IMAGE_DECODER_QUEUE_FULL, /*!< Event to signal the image decoder queue is full. */
    IMAGE_DECODER_QUEUE_EMPTY, /*!< Event to signal the image decoder queue is empty. */
    IMAGE_DECODER_QUEUE_PROCESS, /*!< Event to signal the image decoder queue can be refilled. */
    IMAGE_DECODER_QUEUE_TERMINATE, /*!< Event to signal the image decoder queue thread shoud terminate. */
    IMAGE_DECODER_CHANGE_ID, /*!< Event to signal the image decoder thread should re-read event IDs. */

    IMAGE_ENCODER_QUEUE_FULL, /*!< Event to signal the image encoder queue is full. */
    IMAGE_ENCODER_QUEUE_EMPTY, /*!< Event to signal the image encoder queue is empty. */
    IMAGE_ENCODER_QUEUE_PROCESS, /*!< Event to signal the image encoder queue has items that need to be processed. */
    IMAGE_ENCODER_QUEUE_TERMINATE, /*!< Event to signal the image encoder queue thread shoud terminate. */
    IMAGE_ENCODER_CHANGE_ID, /*!< Event to signal the image encoder thread should re-read event IDs. */

    DRAW_PRESENT, /*!< Event to signal the draw thread should present last rendered frame. */
    DRAW_PRESENT_READY, /*!< Event to signal the drawing thread has ended pre-rendering the next frame and is ready to present it. */
    DRAW_RENDER, /*!< Event to signal the drawing thread should pre-render the next frame. */
    DRAW_RENDER_READY, /*!< Event to signal the drawing thread has presented the frame and is ready to pre-render the next frame. */
    DRAW_TERMINATE, /*!< Event to signal the drawing thread should terminate. */
    DRAW_VBLANK, /*!< Event to signal the drawing thread should wait for next VBLANK event. */
    DRAW_CHANGE_ID, /*!< Event to signal the drawing thread should re-read all event IDs. */

    CAMERA_SYNC_TRIGGERS, /*!< Event for triger synchronization of all cameras associated with one rednering thread. */

    MAIN_PREPARE_DRAW, /*!< Event to signal preparation for batch acquisition of fringe patterns. */
    MAIN_READY_DRAW, /*!< Event to signal rendering preparation is complete. */
    MAIN_BEGIN, /*!< Event to signal a batch acquisition of fringe patterns has started. */
    MAIN_END_DRAW, /*!< Event to signal the batch acquisition has ended in the draw thread. */
    MAIN_RESUME_DRAW, /*!< Event to signal the normal preview mode may continue after the batch acquisition has ended. */

    CAMERA_SEND_TRIGGER, /*!< Event to signal the camera may start the acquisition. */
    CAMERA_REPEAT_TRIGGER, /*!< Event to signal the camera did not trigger correctly. */
    CAMERA_EXPOSURE_BEGIN, /*!< Event to signal the image acquisition has begun. */
    CAMERA_EXPOSURE_END, /*!< Event to signal the image acquisition has ended. */
    CAMERA_READOUT_BEGIN, /*!< Event to signal the image acquisition has begun. */
    CAMERA_READOUT_END, /*!< Event to signal the image acquisition has ended. */
    CAMERA_TRANSFER_BEGIN, /*!< Event to signal the image transfer has begun. */
    CAMERA_TRANSFER_END, /*!< Event to signal the image transfer has ended. */
    CAMERA_TERMINATE, /*!< Event to signal the image acquisition thread shoud terminate. */
    CAMERA_READY, /*!< Event to signal the camera is ready to accept new trigger. */
    CAMERA_INVALID_TRIGGER, /*!< Event to signal the trigger is dropped. */
    CAMERA_CHANGE_ID, /*!< Event to signal the acquisition thread should re-read all event IDs. */

    MAIN_PREPARE_CAMERA, /*!< Event to signal preparation for batch acquisition of fringe patterns. */
    MAIN_READY_CAMERA, /*!< Event to signal acquisition preparation is complete. */
    MAIN_END_CAMERA, /*!< Event to signal the batch acquisition has ended in the draw thread. */

    DRAW_SYNC_PRESENT, /*!< Event to synchronize present operation across multiple rendering threads.*/
    DRAW_SYNC_VBLANK, /*!< Event to synchronize wait for VBLANK operation across multiple rendering threads. */
    DRAW_SYNC_TRIGGERS, /*!< Event to synchronize trigger operation between multiple rendering threads. */

    INVALID_SYNCHRONIZATION_CODE, /*!< Name of invalid synchronization code. */
  } SynchronizationCodes;



//! Synchronization events for image decoder thread.
/*!
  Synchronization events for image decoder thread.
  This structure holds event handles and down-counters for conditional signalling and resetting of events.
*/
typedef
struct SynchronizationEventsImageDecoder_
{
  HANDLE ImageDecoderQueueFull; /*!< Event to signal the image decoder queue is full. */
  HANDLE ImageDecoderQueueEmpty; /*!< Event to signal the image decoder queue is empty. */
  HANDLE ImageDecoderQueueProcess; /*!< Event to signal the image decoder queue can be refilled. */
  HANDLE ImageDecoderQueueTerminate; /*!< Event to signal the image decoder queue should terminate. */
  HANDLE ImageDecoderChangeID; /*!< Event to signal the image decoder thread should re-read event IDs. */

  int CounterEventSetQueueFull; /*!< Counter for conditional signalling of the queue full event. */
  int CounterEventSetQueueEmpty; /*!< Counter for conditional signalling of the queue empty event. */
  int CounterEventSetQueueProcess; /*!< Counter for conditional signalling of the queue process event. */
  int CounterEventSetQueueTerminate; /*!< Counter for conditional signalling of the queue terminate event. */
  int CounterEventSetChangeID; /*!< Counter for conditional signalling of the change ID event. */

  int CounterEventResetQueueFull; /*!< Counter for conditional resetting of the queue full event. */
  int CounterEventResetQueueEmpty; /*!< Counter for conditional resetting of the queue empty event. */
  int CounterEventResetQueueProcess; /*!< Counter for conditional resetting of the queue process event. */
  int CounterEventResetQueueTerminate; /*!< Counter for conditional resetting of the queue terminate event. */
  int CounterEventResetChangeID; /*!< Counter for conditional resetting of the change ID event. */

  int StartCounterValueQueueFull; /*!< Starting counter value for queue full event. */
  int StartCounterValueQueueEmpty; /*!< Starting counter value for queue empty event. */
  int StartCounterValueQueueProcess; /*!< Starting counter value for queue process event. */
  int StartCounterValueQueueTerminate; /*!< Starting counter value for queue terminate event. */
  int StartCounterValueChangeID; /*!< Starting counter value for the change ID event. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Default constructor.
  SynchronizationEventsImageDecoder_();

  //! Copy constructor.
  SynchronizationEventsImageDecoder_(const SynchronizationEventsImageDecoder_ &);

  //! Default destructor.
  ~SynchronizationEventsImageDecoder_();

} SynchronizationEventsImageDecoder;



//! Synchronization events for image encoder thread.
/*!
  Synchronization events for image encoder thread.
  This structure holds event handles and down-counters for conditional signalling and resetting of events.
*/
typedef
struct SynchronizationEventsImageEncoder_
{
  HANDLE ImageEncoderQueueFull; /*!< Event to signal the image encoder queue is full. */
  HANDLE ImageEncoderQueueEmpty; /*!< Event to signal the image encoder queue is empty. */
  HANDLE ImageEncoderQueueProcess; /*!< Event to signal the image encoder queue can be emptied. */
  HANDLE ImageEncoderQueueTerminate; /*!< Event to signal the image encoder queue should terminate. */
  HANDLE ImageEncoderChangeID; /*!< Event to signal the image encoder thread should re-read event IDs. */

  int CounterEventSetQueueFull; /*!< Counter for conditional signalling of the queue full event. */
  int CounterEventSetQueueEmpty; /*!< Counter for conditional signalling of the queue empty event. */
  int CounterEventSetQueueProcess; /*!< Counter for conditional signalling of the queue process event. */
  int CounterEventSetQueueTerminate; /*!< Counter for conditional signalling of the queue terminate event. */
  int CounterEventSetChangeID; /*!< Counter for conditional signalling of the change ID event. */

  int CounterEventResetQueueFull; /*!< Counter for conditional resetting of the queue full event. */
  int CounterEventResetQueueEmpty; /*!< Counter for conditional resetting of the queue empty event. */
  int CounterEventResetQueueProcess; /*!< Counter for conditional resetting of the queue process event. */
  int CounterEventResetQueueTerminate; /*!< Counter for conditional resetting of the queue terminate event. */
  int CounterEventResetChangeID; /*!< Counter for conditional resetting of the change ID event. */

  int StartCounterValueQueueFull; /*!< Starting counter value for queue full event. */
  int StartCounterValueQueueEmpty; /*!< Starting counter value for queue empty event. */
  int StartCounterValueQueueProcess; /*!< Starting counter value for queue process event. */
  int StartCounterValueQueueTerminate; /*!< Starting counter value for queue terminate event. */
  int StartCounterValueChangeID; /*!< Starting counter value for the change ID event. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Default constructor.
  SynchronizationEventsImageEncoder_();

  //! Copy constructor.
  SynchronizationEventsImageEncoder_(const SynchronizationEventsImageEncoder_ &);

  //! Default destructor.
  ~SynchronizationEventsImageEncoder_();

} SynchronizationEventsImageEncoder;



//! Synchronization events for drawing thread.
/*!
  Synchronization events for drawing thread.
  This structure holds event handles and down-counters for conditional signalling and resetting of events.
*/
typedef
struct SynchronizationEventsDraw_
{
  HANDLE DrawPresent; /*!< Event to signal the draw thread should present last rendered frame. */
  HANDLE DrawPresentReady; /*!< Event to signal the drawing thread has ended pre-rendering the next frame and is ready to present it. */
  HANDLE DrawRender; /*!< Event to signal the drawing thread should pre-render the next frame. */
  HANDLE DrawRenderReady; /*!< Event to signal the drawing thread has presented the frame and is ready to pre-render the next frame. */
  HANDLE DrawTerminate; /*!< Event to signal the drawing thread should terminate. */
  HANDLE DrawVBlank; /*!< Event to signal the drawing thread should wait for next VBLANK event. */
  HANDLE DrawChangeID; /*!< Event to signal the drawing thread should re-read all event IDs. */

  HANDLE CameraSyncTriggers; /*!< Event to synchronize triggers when fixed SL pattern is used. */

  HANDLE MainPrepareDraw; /*!< Event to signal preparation for batch acquisition of fringe patterns. */
  HANDLE MainReadyDraw; /*!< Event to signal rendering preparation is complete. */
  HANDLE MainBegin; /*!< Event to signal a batch acquisition of fringe patterns has started. */
  HANDLE MainEndDraw; /*!< Event to signal the batch acquisition has ended. */
  HANDLE MainResumeDraw; /*!< Event to signal the normal preview mode may continue after the batch acquisition has ended. */

  int CounterEventSetPresent; /*!< Counter for conditional signalling of the present event. */
  int CounterEventSetPresentReady; /*!< Counter for conditional signalling of the present ready event. */
  int CounterEventSetRender; /*!< Counter for conditional signalling of the redner event. */
  int CounterEventSetRenderReady; /*!< Counter for conditional signalling of the render ready event. */
  int CounterEventSetTerminate; /*!< Counter for conditional signalling of the termination event. */
  int CounterEventSetVBlank; /*!< Counter for conditional signalling of the wait for VBLANK event. */
  int CounterEventSetChangeID; /*!< Counter for conditional signalling of the change ID event. */

  int CounterEventSetSyncTriggers; /*!< Counter for conditional signalling of the camera trigger synchronization event. */

  int CounterEventSetPrepareDraw; /*!< Counter for conditional signalling of the drawing preparation event. */
  int CounterEventSetReadyDraw; /*!< Counter for conditional signalling of the drawing ready event. */
  int CounterEventSetBegin; /*!< Counter for conditional signalling of the acquisition begin event. */
  int CounterEventSetEndDraw; /*!< Counter for conditional signalling of the drawing completed event. */
  int CounterEventSetResumeDraw; /*!< Counter for conditional signalling of the drawing resume event. */

  int CounterEventResetPresent; /*!< Counter for conditional resetting of the present event. */
  int CounterEventResetPresentReady; /*!< Counter for conditional resetting of the present ready event. */
  int CounterEventResetRender; /*!< Counter for conditional resetting of the redner event. */
  int CounterEventResetRenderReady; /*!< Counter for conditional resetting of the render ready event. */
  int CounterEventResetTerminate; /*!< Counter for conditional resetting of the termination event. */
  int CounterEventResetVBlank; /*!< Counter for conditional resetting of the wait for VBLANK event. */
  int CounterEventResetChangeID; /*!< Counter for conditional resetting of the change ID event. */

  int CounterEventResetSyncTriggers; /*!< Counter for conditional resetting of the camera trigger synchronization event. */

  int CounterEventResetPrepareDraw; /*!< Counter for conditional resetting of the drawing preparation event. */
  int CounterEventResetReadyDraw; /*!< Counter for conditional resetting of the drawing ready event. */
  int CounterEventResetBegin; /*!< Counter for conditional resetting of the acquisition begin event. */
  int CounterEventResetEndDraw; /*!< Counter for conditional resetting of the drawing completed event. */
  int CounterEventResetResumeDraw; /*!< Counter for conditional resetting of the drawing resume event. */

  int StartCounterValuePresent; /*!< Starting counter value for the present event. */
  int StartCounterValuePresentReady; /*!< Starting counter value for the present ready event. */
  int StartCounterValueRender; /*!< Starting counter value for the redner event. */
  int StartCounterValueRenderReady; /*!< Starting counter value for the render ready event. */
  int StartCounterValueTerminate; /*!< Starting counter value for the termination event. */
  int StartCounterValueVBlank; /*!< Starting counter value for the wait for VBLANK event. */
  int StartCounterValueChangeID; /*!< Starting counter value for the change ID event. */

  int StartCounterValueSyncTriggers; /*!< Starting counter value for camera trigger synchronization event. */

  int StartCounterValuePrepareDraw; /*!< Starting counter value for drawing preparation event. */
  int StartCounterValueReadyDraw; /*!< Starting counter value for drawing ready event. */
  int StartCounterValueBegin; /*!< Starting counter value for acquisition begin event. */
  int StartCounterValueEndDraw; /*!< Starting counter value for drawing completed event. */
  int StartCounterValueResumeDraw; /*!< Starting counter value for drawing resume event. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Default constructor.
  SynchronizationEventsDraw_();

  //! Copy constructor.
  SynchronizationEventsDraw_(const SynchronizationEventsDraw_ &);

  //! Default destructor.
  ~SynchronizationEventsDraw_();

} SynchronizationEventsDraw;



//! Synchronization events for acquisition thread.
/*!
  Synchronization events for acquisition thread.
  This structure holds event handles and down-counters for conditional signalling and resetting of events.
*/
typedef
struct SynchronizationEventsCamera_
{
  HANDLE CameraSendTrigger; /*!< Event to signal the camera may start the acquisition. */
  HANDLE CameraRepeatTrigger; /*!< Event to signal the camera did not trigger correctly. */
  HANDLE CameraExposureBegin; /*!< Event to signal the sensor exposure has begun. */
  HANDLE CameraExposureEnd; /*!< Event to signal the sensor exposure has ended. */
  HANDLE CameraReadoutBegin; /*!< Event to signal the sensor readout has begun. */
  HANDLE CameraReadoutEnd; /*!< Event to signal the sensor readout has ended. */
  HANDLE CameraTransferBegin; /*!< Event to signal the image transfer has begun. */
  HANDLE CameraTransferEnd; /*!< Event to signal the image transfer has ended. */
  HANDLE CameraTerminate; /*!< Event to signal the image acquisition thread should terminate. */
  HANDLE CameraReady; /*!< Event to signal the camera is ready to accept new trigger. */
  HANDLE CameraInvalidTrigger; /*!< Event to signal the trigger is dropped. */
  HANDLE CameraChangeID; /*!< Event to signal the acquisition thread should re-read all event IDs. */

  HANDLE MainPrepareCamera; /*!< Event to signal preparation for batch acquisition of fringe patterns. */
  HANDLE MainReadyCamera; /*!< Event to signal acquisition preparation is complete. */
  HANDLE MainEndCamera; /*!< Event to signal the batch acquisition has ended. */

  int CounterEventSetSendTrigger; /*!< Counter for conditional signalling of the trigger event. */
  int CounterEventSetRepeatTrigger; /*!< Counter for conditional signalling of the repeat trigger event. */
  int CounterEventSetExposureBegin; /*!< Counter for conditional signalling of the exposure begin event. */
  int CounterEventSetExposureEnd; /*!< Counter for conditional signalling of the exposure end event. */
  int CounterEventSetReadoutBegin; /*!< Counter for conditional signalling of the readout begin event. */
  int CounterEventSetReadoutEnd; /*!< Counter for conditional signalling of the readout end event. */
  int CounterEventSetTransferBegin; /*!< Counter for conditional signalling of the transfer begin event. */
  int CounterEventSetTransferEnd; /*!< Counter for conditional signalling of the transfer end event. */
  int CounterEventSetTerminate; /*!< Counter for conditional signalling of the termination event. */
  int CounterEventSetReady; /*!< Counter for conditional signalling of the camera ready event. */
  int CounterEventSetInvalidTrigger; /*!< Counter for conditional signalling of the invalid trigger event. */
  int CounterEventSetChangeID; /*!< Counter for conditional signalling of the change ID event. */

  int CounterEventSetPrepareCamera; /*!< Counter for conditional signalling of the acquisition preparation event. */
  int CounterEventSetReadyCamera; /*!< Counter for conditional signalling of the acquisition ready event. */
  int CounterEventSetEndCamera; /*!< Counter for conditional signalling of the acquisition completed event. */

  int CounterEventResetSendTrigger; /*!< Counter for conditional resetting of the trigger event. */
  int CounterEventResetRepeatTrigger; /*!< Counter for conditional resetting of the repeat trigger event. */
  int CounterEventResetExposureBegin; /*!< Counter for conditional resetting of the exposure begin event. */
  int CounterEventResetExposureEnd; /*!< Counter for conditional resetting of the exposure end event. */
  int CounterEventResetReadoutBegin; /*!< Counter for conditional resetting of the readout begin event. */
  int CounterEventResetReadoutEnd; /*!< Counter for conditional resetting of the readout end event. */
  int CounterEventResetTransferBegin; /*!< Counter for conditional resetting of the transfer begin event. */
  int CounterEventResetTransferEnd; /*!< Counter for conditional resetting of the transfer end event. */
  int CounterEventResetTerminate; /*!< Counter for conditional resetting of the termination event. */
  int CounterEventResetReady; /*!< Counter for conditional resetting of the camera ready event. */
  int CounterEventResetInvalidTrigger; /*!< Counter for conditional resetting of the invalid trigger event. */
  int CounterEventResetChangeID; /*!< Counter for conditional resetting of the change ID event. */

  int CounterEventResetPrepareCamera; /*!< Counter for conditional resetting of the acquisition preparation event. */
  int CounterEventResetReadyCamera; /*!< Counter for conditional resetting of the acquisition ready event. */
  int CounterEventResetEndCamera; /*!< Counter for conditional resetting of the acquisition completed event. */

  int StartCounterValueSendTrigger; /*!< Starting counter value for trigger event. */
  int StartCounterValueRepeatTrigger; /*!< Starting counter value for repeat trigger event. */
  int StartCounterValueExposureBegin; /*!< Starting counter value for exposure begin event. */
  int StartCounterValueExposureEnd; /*!< Starting counter value for exposure end event. */
  int StartCounterValueReadoutBegin; /*!< Starting counter value for readout begin event. */
  int StartCounterValueReadoutEnd; /*!< Starting counter value for readout end event. */
  int StartCounterValueTransferBegin; /*!< Starting counter value for transfer begin event. */
  int StartCounterValueTransferEnd; /*!< Starting counter value for transfer end event. */
  int StartCounterValueTerminate; /*!< Starting counter value for termination event. */
  int StartCounterValueReady; /*!< Starting counter value for camera ready event. */
  int StartCounterValueInvalidTrigger; /*!< Starting counter value for invalid trigger event. */
  int StartCounterValueChangeID; /*!< Starting counter value for the change ID event. */

  int StartCounterValuePrepareCamera; /*!< Starting counter value for acquisition preparation event. */
  int StartCounterValueReadyCamera; /*!< Starting counter value for acquisition ready event. */
  int StartCounterValueEndCamera; /*!< Starting counter value for acquisition completed event. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Default constructor.
  SynchronizationEventsCamera_();

  //! Copy constructor.
  SynchronizationEventsCamera_(const SynchronizationEventsCamera_ &);

  //! Default destructor.
  ~SynchronizationEventsCamera_();

} SynchronizationEventsCamera;



//! Synchronization events for main thread.
/*!
  Synchronization events for main thread.
  This structure holds event handles and down-counters for conditional signalling and resetting of events.

  Note: All older MAIN_* events are no longer declared here but are instead moved to the corresponding
  SynchronizationEventsDraw or SynchronizationEventsCamera structured as they have to be unique to
  each concrete instance of rendering or acquisition threads. Despite the move events retain the MAIN_*
  in the event name as they were intended to be raised by the main thread and to be consumbed by the
  rendering and acquisition threads (early application desing which allowed for one rendering and one
  acquisition thread). Current design has one main thread which controlls all rendering threads which
  control all acquisition threads so MAIN_*_DRAW events are still exclusively signalled by the main
  thread and consumed by the selected rendering thread, however all MAIN_*_CAMERA events are signalled
  by a rendering thread which owns the corresponding acquisition thread.
*/
typedef
struct SynchronizationEventsMain_
{
  HANDLE DrawSyncPresent; /*!< Event to synchronize present operation across multiple rendering threads.*/
  HANDLE DrawSyncVBlank; /*!< Event to synchronize wait for VBLANK operation across multiple rendering threads. */
  HANDLE DrawSyncTriggers; /*!< Event to synchronize trigger operation between multiple rendering threads. */

  int CounterEventSetSyncPresent; /*!< Counter for conditional signalling to synchronize present operation. */
  int CounterEventSetSyncVBlank; /*!< Counter for conditional signalling to synchronize wait for VBLANK operation. */
  int CounterEventSetSyncTriggers; /*!< Counter for conditional signalling to synchronize camera triggering across rendering threads. */

  int CounterEventResetSyncPresent; /*!< Counter for conditional resetting of the present synchronization signal. */
  int CounterEventResetSyncVBlank; /*!< Counter for conditional resetting of the wait for VBLANK synchronization signal. */
  int CounterEventResetSyncTriggers; /*!< Counter for conditional resetting of the trigger synchronization signal. */

  int StartCounterValueSyncPresent; /*!< Starting counter value for present synchronization event. */
  int StartCounterValueSyncVBlank; /*!< Starting counter value for wait for VBLANK synchronization event. */
  int StartCounterValueSyncTriggers; /*!< Starting counter value for trigger synchronization event. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Default constructor.
  SynchronizationEventsMain_();

  //! Copy constructor.
  SynchronizationEventsMain_(const SynchronizationEventsMain_ &);

  //! Default destructor.
  ~SynchronizationEventsMain_();
} SynchronizationEventsMain;



//! Synchronization events.
/*!
  Events are used to synchronize between threads.
*/
typedef
struct SynchronizationEvents_
{
  std::vector<SynchronizationEventsImageDecoder> ImageDecoder; /*!< Events for image decoder thread. */
  std::vector<SynchronizationEventsImageEncoder> ImageEncoder; /*!< Events for image encoder thread. */
  std::vector<SynchronizationEventsDraw> Draw; /*!< Events for image drawing thread. */
  std::vector<SynchronizationEventsCamera> Camera; /*!< Events for image acquisition thread. */
  std::vector<SynchronizationEventsMain> Main; /*!< Events for main thread. */

  SRWLOCK lock; /*!< Slim lock to control concurrent access. */

  //! Signal specified event.
  BOOL EventSet(SynchronizationCodes const, int const H = 0);

  //! Reset specified event to non-signaled state.
  BOOL EventReset(SynchronizationCodes const, int const H = 0);

  //! Get conditional event counter for event signalling.
  BOOL GetEventSetCounter(SynchronizationCodes const, int const, int * const);

  //! Get conditional event counter for event reset.
  BOOL GetEventResetCounter(SynchronizationCodes const, int const, int * const);

  //! Get start counter value.
  BOOL GetStartCounterValue(SynchronizationCodes const, int const, int * const);

  //! Set conditional event counter for event signalling.
  BOOL SetEventSetCounter(SynchronizationCodes const, int const, int const);

  //! Set conditional event counter for event reset.
  BOOL SetEventResetCounter(SynchronizationCodes const, int const, int const);

  //! Set start counter value.
  BOOL SetStartCounterValue(SynchronizationCodes const, int const, int const, bool const);

  //! Signal specified event and set conditional counter.
  BOOL EventSetAndResetCounterSet(SynchronizationCodes const, int const, int const);

  //! Reset specified event to non-signaled state and set conditional counter.
  BOOL EventResetAndSetCounterSet(SynchronizationCodes const, int const, int const);

  //! Conditional event signal.
  BOOL EventSetConditional(SynchronizationCodes const, int const);

  //! Conditional event reset.
  BOOL EventResetConditional(SynchronizationCodes const, int const);

  //! Reset all events.
  BOOL EventResetAll(int const, int const, int const);

  //! Reset all image decoder events.
  BOOL EventResetAllImageDecoder(int const);

  //! Reset all image encoder events.
  BOOL EventResetAllImageEncoder(int const);

  //! Reset all draw events.
  BOOL EventResetAllDraw(int const);

  //! Reset almost all draw events.
  BOOL EventResetAllDrawExceptRenderAndPresentReady(int const);

  //! Reset all camera events.
  BOOL EventResetAllCamera(int const, int const);

  //! Reset all camera events except CAMERA_READY.
  BOOL EventResetAllCameraExceptTriggerReady(int const);

  //! Reset all main thread events.
  BOOL EventResetAllMain(int const, int const, int const);

  //! Wait for specified event.
  DWORD EventWaitFor(SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified event.
  DWORD EventWaitFor(SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAll(SynchronizationCodes const, SynchronizationCodes const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAll(SynchronizationCodes const, int const, SynchronizationCodes const, int const, DWORD const);

  //! Get event handle.
  HANDLE GetEventHandle(SynchronizationCodes const, int const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified handles.
  DWORD EventWaitForAny(HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, HANDLE const, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAny(std::vector<SynchronizationCodes> &, std::vector<int> &, DWORD const);

  //! Wait for specified events.
  DWORD EventWaitForAll(std::vector<SynchronizationCodes> &, std::vector<int> &, DWORD const);

  //! Waits for specified events.
  DWORD EventWaitForAnyAndAll(std::vector<SynchronizationCodes> &, std::vector<int> &, std::vector<SynchronizationCodes> &, std::vector<int> &, DWORD const);

} SynchronizationEvents;


//! Deletes SynchronizationEvents structure.
void DeleteSynchronizationEventsStructure(SynchronizationEvents * const);

//! Creates SynchronizationEvents structure.
SynchronizationEvents * CreateSynchronizationEventsStructure(void);

//! Add decoder.
int AddImageDecoderToSynchronizationEventsStructure(SynchronizationEvents * const);

//! Add encoder.
int AddImageEncoderToSynchronizationEventsStructure(SynchronizationEvents * const);

//! Add projector.
int AddProjectorToSynchronizationEventsStructure(SynchronizationEvents * const);

//! Add camera.
int AddCameraToSynchronizationEventsStructure(SynchronizationEvents * const);

//! Remove decoder.
HRESULT RemoveImageDecoderFromSynchronizationEventsStructure(SynchronizationEvents * const, int const);

//! Remove encoder.
HRESULT RemoveImageEncoderFromSynchronizationEventsStructure(SynchronizationEvents * const, int const);

//! Remove projector.
HRESULT RemoveProjectorFromSynchronizationEventsStructure(SynchronizationEvents * const, int const);

//! Remove camera.
HRESULT RemoveCameraFromSynchronizationEventsStructure(SynchronizationEvents * const, int const);



#endif /* !__BATCHACQUISITIONEVENTS_H */
