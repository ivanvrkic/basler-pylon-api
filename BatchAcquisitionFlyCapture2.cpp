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
  \file   BatchAcquisitionFlyCapture2.cpp
  \brief  Functions for PointGrey FlyCapture2 SDK.

  Functions and wrappers for PointGrey FlyCapture2 SDK.

  \author Tomislav Petkovic
  \date   2017-02-15
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONFLYCAPTURE2_CPP
#define __BATCHACQUISITIONFLYCAPTURE2_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionFlyCapture2.h"
#include "BatchAcquisitionFlyCapture2Registers.h"
#include "BatchAcquisition.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionKeyboard.h"
#include "BatchAcquisitionWindowPreview.h"




/****** HELPER FUNCTIONS ******/

//! Blank parameters structure.
/*!
  Blanks acquisition parameter structure for PointGrey FlyCapture2 SDK.

  \param P      Pointer to parameters class.
*/
void
inline
AcquisitionParametersFlyCapture2Blank_inline(
                                             AcquisitionParametersFlyCapture2 * const P
                                             )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->pBusManager = NULL;
  P->pCameraGUID = NULL;
  P->pCamera = NULL;
  P->pTriggerMode = NULL;
  P->pTriggerDelay = NULL;
  P->pConfig = NULL;
  P->pControlDialog = NULL;

  P->pAcquisitionThread = NULL;
}
/* AcquisitionParametersFlyCapture2Blank_inline */



#ifdef HAVE_FLYCAPTURE2_SDK

//! Turn off auto adjustment.
/*!
  PointGrey cameras by default may have auto adjustment for certain parameters turned on.
  This function turns off auto adjustments.

  \param P      Pointer to parameters structure.
  \param type   Type to turn off.
*/
inline
void
TurnOffAutoAdjustmentForProperty_inline(
                                        AcquisitionParametersFlyCapture2 * const P,
                                        FlyCapture2::PropertyType const type
                                        )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pCamera);
  if (NULL == P->pCamera) return;

  FlyCapture2::Error error;
  FlyCapture2::Property prop;
  prop.type = type;

  error = P->pCamera->GetProperty( &prop );

  assert(error == FlyCapture2::PGRERROR_OK);
  if (error != FlyCapture2::PGRERROR_OK) return;

  //assert(true == prop.present);
  if (true != prop.present) return;

  prop.onePush = false;
  prop.autoManualMode = false;

  error = P->pCamera->SetProperty( &prop );
  assert(error == FlyCapture2::PGRERROR_OK);
}
/* TurnOffAutoAdjustmentForProperty_inline */



//! Turn off both auto adjustment and the property.
/*!
  PointGrey cameras by default may have auto adjustment for certain parameters turned on.
  This function turns off auto adjustments.

  \param P      Pointer to parameters structure.
  \param type   Type to turn off.
*/
inline
void
TurnOffProperty_inline(
                       AcquisitionParametersFlyCapture2 * const P,
                       FlyCapture2::PropertyType const type
                       )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pCamera);
  if (NULL == P->pCamera) return;

  FlyCapture2::Error error;
  FlyCapture2::Property prop;
  prop.type = type;

  error = P->pCamera->GetProperty( &prop );

  assert(error == FlyCapture2::PGRERROR_OK);
  if (error != FlyCapture2::PGRERROR_OK) return;

  //assert(true == prop.present);
  if (true != prop.present) return;

  prop.onOff = false;
  prop.onePush = false;
  prop.autoManualMode = false;

  error = P->pCamera->SetProperty( &prop );
  assert(error == FlyCapture2::PGRERROR_OK);
}
/* TurnOffProperty_inline */


//! Set property to value.
/*!
  This function sets property to relative value.

  \param P      Pointer to parameters structure.
  \param type   Property type to set.
  \param valueA Property value.
  \param valueB Property value.
*/
inline
void
SetPropertyToRelativeValue_inline(
                                  AcquisitionParametersFlyCapture2 * const P,
                                  FlyCapture2::PropertyType const type,
                                  unsigned int const valueA,
                                  unsigned int const valueB
                                  )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pCamera);
  if (NULL == P->pCamera) return;

  FlyCapture2::Error error;
  FlyCapture2::Property prop;
  prop.type = type;

  error = P->pCamera->GetProperty( &prop );

  assert(error == FlyCapture2::PGRERROR_OK);
  if (error != FlyCapture2::PGRERROR_OK) return;

  //assert(true == prop.present);
  if (true != prop.present) return;

  bool const absControl = prop.absControl;

  prop.absControl = false;
  prop.valueA = valueA;
  prop.valueB = valueB;

  error = P->pCamera->SetProperty( &prop );
  assert(error == FlyCapture2::PGRERROR_OK);

  // Restore property flag.
  if (true == absControl)
    {
      error = P->pCamera->GetProperty( &prop );
      assert(error == FlyCapture2::PGRERROR_OK);

      prop.absControl = absControl;

      error = P->pCamera->SetProperty( &prop );
      assert(error == FlyCapture2::PGRERROR_OK);
    }
  /* if */
}
/* SetPropertyToRelativeValue_inline */



//! Set property to value.
/*!
  This function sets property to absolute value.

  \param P      Pointer to parameters structure.
  \param type   Property type.
  \param onePush     Flag to indicate one-push status.
  \param onOff  Flag to indicate property activity.
  \param autoManualMode Flag to indicate auto or manual mode.
  \param absValue       Property value.
  \param absValue_out   Address where actually set value will be stored.
  \return Function returns true if successfull.
*/
inline
bool
SetPropertyToAbsoluteValue_inline(
                                  AcquisitionParametersFlyCapture2 * const P,
                                  FlyCapture2::PropertyType const type,
                                  bool const onePush,
                                  bool const onOff,
                                  bool const autoManualMode,
                                  float const absValue,
                                  float * const absValue_out
                                  )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pCamera);
  if (NULL == P->pCamera) return false;

  FlyCapture2::Error error;
  FlyCapture2::Property prop;
  prop.type = type;

  error = P->pCamera->GetProperty( &prop );

  assert(error == FlyCapture2::PGRERROR_OK);
  if (error != FlyCapture2::PGRERROR_OK) return false;

  //assert(true == prop.present);
  if (true != prop.present) return false;

  bool const absControl = prop.absControl;

  prop.absControl = true;
  prop.onePush = onePush;
  prop.onOff = onOff;
  prop.autoManualMode = autoManualMode;
  prop.absValue = absValue;

  error = P->pCamera->SetProperty( &prop );
  assert(error == FlyCapture2::PGRERROR_OK);
  bool result = (error == FlyCapture2::PGRERROR_OK);

  // Get new value.
  if ( (false == absControl) || (NULL != absValue_out) )
    {
      error = P->pCamera->GetProperty( &prop );
      assert(error == FlyCapture2::PGRERROR_OK);

      if (NULL != absValue_out)
        {
          *absValue_out = prop.absValue;
          result = result && (error == FlyCapture2::PGRERROR_OK);
        }
      /* if */
    }
  /* if */

  // Restore property flag.
  if (false == absControl)
    {
      prop.absControl = absControl;

      error = P->pCamera->SetProperty( &prop );
      assert(error == FlyCapture2::PGRERROR_OK);
    }
  /* if */

  return result;
}
/* SetPropertyToAbsoluteValue_inline */


#endif /* HAVE_FLYCAPTURE2_SDK */



/****** IMAGE TRANSFER CALLBACK ******/


#ifdef HAVE_FLYCAPTURE2_SDK

//! Queue acquired image for processing.
/*!
  Transfer callback function is called each time a complete frame is transferred.
  Default action is to queue acquired image to image encoder for processing and storage.
  It may also be used to update the display etc.

  \param pImage Pointer to captured image.
  \param pCallbackData Pointer to callback data.
*/
void
OnImageGrabbed(
               FlyCapture2::Image * pImage,
               const void * pCallbackData
               )
{
  AcquisitionParameters * const P = (AcquisitionParameters *)pCallbackData;
  assert(NULL != P);
  if (NULL == P) return;

  // Fetch timestamp.
  LARGE_INTEGER QPC_after_transfer;
  QPC_after_transfer.QuadPart = (LONGLONG)0;

  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_transfer );
  assert(TRUE == qpc_after);

  // Signal that data transfer has ended.
  if (NULL != P->pSynchronization)
    {
      BOOL const set_transfer_end = P->pSynchronization->EventSet(CAMERA_TRANSFER_END, P->CameraID);
      assert(0 != set_transfer_end);
    }
  /* if */

  // Fetch flags.
  bool fBlocking = true;
  bool fFixed = false;
  if (NULL != P->pWindow)
    {
      fBlocking = P->pWindow->fBlocking;
      fFixed = P->pWindow->fFixed;
    }
  /* if */

  // Fetch image metadata.
  ImageMetadata sData;
  ImageMetadataBlank( &sData );
  bool const pop = PopFrontImageMetadataFromQueue(P->pMetadataQueue, &sData);
  if (true == pop)
    {
      LARGE_INTEGER QPC_before_trigger;
      QPC_before_trigger.QuadPart = sData.QPC_before_trigger;

      FrameStatisticsAddMeasurement(P->pStatisticsAcquisitionDuration, QPC_before_trigger, QPC_after_transfer);

#ifdef _DEBUG
      if (NULL != P->pWindow)
        {
          if (true == fBlocking) assert( true == sData.fBlocking );
          if (true == fFixed) assert( true == sData.fFixed );
        }
      /* if */
#endif /* _DEBUG */
    }
  /* if */

  // Fetch SDK pointer.
  AcquisitionParametersFlyCapture2 * const pFlyCapture2 = P->pFlyCapture2SDK;
  assert(NULL != pFlyCapture2);
  if (NULL == pFlyCapture2)
    {
      ImageMetadataRelease( &sData );
      return;
    }
  /* if */

  // Test if buffer is acquired correctly.
  bool acquired = true;
  if (NULL != pFlyCapture2)
    {
      // TODO!
    }
  /* if */

  // Queue last successfully acquired frame into image encoder queue.
  if ( (true == acquired) && (NULL != P->pImageEncoder) && (NULL != pFlyCapture2) )
    {
      QueuedEncoderImage * item = new QueuedEncoderImage();
      assert(NULL != item);
      if (NULL != item)
        {
          bool const copy_metadata = item->CopyMetadataFrom( &sData );
          assert(true == copy_metadata);

          BOOL const copy_image = item->CopyImageFrom(pImage, pFlyCapture2->pCamera);
          assert(TRUE == copy_image);

          bool const queue = ImageEncoderQueueImage(P->pImageEncoder, item);
          assert(true == queue);
          if (true != queue) SAFE_DELETE(item);
        }
      /* if */
    }
  else
    {
      ImageMetadataRelease( &sData );
    }
  /* if */

  // Signal the batch acquisition has ended.
  if ( (NULL != P->pSynchronization) &&
       (NULL != P->pImageDecoder) && (NULL != P->pImageDecoder->pImageList) && (false == P->pImageDecoder->pImageList->cycle) &&
       (true == pop) && (true == sData.fLast)
       )
    {
      assert( false == DebugIsSignalled(P->pSynchronization, MAIN_END_CAMERA, P->CameraID) );

      BOOL const set_end_camera = P->pSynchronization->EventSet(MAIN_END_CAMERA, P->CameraID);
      assert(0 != set_end_camera);
    }
  /* if */

  // Display frames.
  if ( (true == P->fView) && (NULL != P->pView) )
    {
      PushImage(P->pView, P->CameraID, pImage, pFlyCapture2->pCamera);
    }
  /* if */
}
/* OnImageGrabbed */

#endif /* HAVE_FLYCAPTURE2_SDK */




/****** EXPORTED FUNCTIONS ******/

//! Open camera control dialog.
/*!
  Opens camera control dialog.
  Note that this function must be called from a thread that runs a window message pump.

  \param P      Pointer to parameters structure.
  \param CameraID Unique camera ID.
  \return Returns true if camera control dialog is visible.
*/
bool
AcquisitionParametersFlyCapture2ControlDialogOpen(
                                                  AcquisitionParametersFlyCapture2 * const P,
                                                  int const CameraID
                                                  )
{
  bool is_visible = false;

  //assert(NULL != P);
  if (NULL == P) return is_visible;

#if defined(HAVE_FLYCAPTURE2_SDK) && defined(USE_FLYCAPTURE2_GUI)
  assert(NULL != P->pControlDialog);
  if ( (NULL != P->pControlDialog) && (NULL != P->pCamera) && (true == P->pCamera->IsConnected()) )
    {
      try
        {
          if (false == P->pControlDialog->IsVisible())
            {
              P->pControlDialog->Connect(P->pCamera);
              P->pControlDialog->Show();
              if (0 <= CameraID)
                {
                  Debugfwprintf(stderr, gDbgCameraControlDialogOpen, CameraID + 1);
                }
              /* if */
            }
          /* if */
          is_visible = P->pControlDialog->IsVisible();
        }
      catch (...)
        {
        }
      /* try */
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return is_visible;
}
/* AcquisitionParametersFlyCapture2ControlDialogOpen */



//! Close camera control dialog.
/*!
  Close camera control dialog.
  Note that this function must be called from a thread that runs a window message pump.

  \param P      Pointer to parameters structure.
  \param CameraID Unique camera ID.
  \return Returns true if camera control dialog is not visible.
*/
bool
AcquisitionParametersFlyCapture2ControlDialogClose(
                                                   AcquisitionParametersFlyCapture2 * const P,
                                                   int const CameraID
                                                   )
{
  bool is_hidden = true;

  //assert(NULL != P);
  if (NULL == P) return is_hidden;

#if defined(HAVE_FLYCAPTURE2_SDK) && defined(USE_FLYCAPTURE2_GUI)
  assert(NULL != P->pControlDialog);
  if ( NULL != P->pControlDialog )
    {
      try
        {
          if (true == P->pControlDialog->IsVisible())
            {
              P->pControlDialog->Hide();
              P->pControlDialog->Disconnect();
              if (0 <= CameraID)
                {
                  Debugfwprintf(stderr, gDbgCameraControlDialogClose, CameraID + 1);
                }
              /* if */
            }
          /* if */
          is_hidden = !P->pControlDialog->IsVisible();
        }
      catch (...)
        {
        }
      /* try */
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return is_hidden;
}
/* AcquisitionParametersFlyCapture2ControlDialogClose */



//! Toggle state of camera control dialog.
/*!
  Toggles the state of the camera control dialog.
  Note that this function must be called from a thread that runs a window message pump.

  \param P      Pointer to parameters structure.
  \param CameraID Unique camera ID.
  \return Returns true is dialog is visible, false otherwise.
*/
bool
AcquisitionParametersFlyCapture2ControlDialogToggle(
                                                    AcquisitionParametersFlyCapture2 * const P,
                                                    int const CameraID
                                                    )
{
  bool is_visible = false;

  //assert(NULL != P);
  if (NULL == P) return is_visible;

#if defined(HAVE_FLYCAPTURE2_SDK) && defined(USE_FLYCAPTURE2_GUI)
  assert(NULL != P->pControlDialog);
  if (NULL != P->pControlDialog)
    {
      try
        {
          if (true == P->pControlDialog->IsVisible())
            {
              is_visible = !AcquisitionParametersFlyCapture2ControlDialogClose(P, CameraID);
            }
          else
            {
              is_visible = AcquisitionParametersFlyCapture2ControlDialogOpen(P, CameraID);
            }
          /* if */
        }
      catch (...)
        {
        }
      /* try */
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return is_visible;
}
/* AcquisitionParametersFlyCapture2ControlDialogToggle */



//! Stop all pending transfers.
/*!
  Stop all pending data transfers. Function waits for
  exposure time multiplied by number of queued frames increased
  by 5 seconds.

  \param P      Pointer to parameters structure.
  \param exposureTime   Exposure time in microseconds (us).
  \param nFrames        Number of frames/buffers used for acquisition.
  \return Function returns true if successfull.
*/
bool
AcquisitionParametersFlyCapture2StopTransfer(
                                             AcquisitionParametersFlyCapture2 * const P,
                                             double const exposureTime,
                                             int const nFrames
                                             )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_FLYCAPTURE2_SDK
  assert(NULL != P->pCamera);
  if (NULL != P->pCamera)
    {
      FlyCapture2::Error error;

      if (true == P->pCamera->IsConnected())
        {
          // Stop capture.
          error = P->pCamera->StopCapture();
          assert( (error == FlyCapture2::PGRERROR_OK) ||
                  (error == FlyCapture2::PGRERROR_ISOCH_NOT_STARTED)
                  );
          if ( (error != FlyCapture2::PGRERROR_OK) &&
               (error != FlyCapture2::PGRERROR_ISOCH_NOT_STARTED)
               )
            {
              result = false;
            }
          /* if */
        }
      /* if */
    }
  else
    {
      result = false;
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return result;
}
/* AcquisitionParametersFlyCapture2StopTransfer */



//! Start transfer.
/*!
  Starts image transfer.

  \param P      Pointer to parameters structure.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFlyCapture2StartTransfer(
                                              AcquisitionParametersFlyCapture2 * const P
                                              )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_FLYCAPTURE2_SDK
  if (NULL != P->pCamera)
    {
      FlyCapture2::Error error;

      assert(true == P->pCamera->IsConnected());
      if (true == P->pCamera->IsConnected())
        {
          // Poll for trigger ready to ensure camera is ready for acquisition.
          bool const ready = WaitForTriggerReady( P->pCamera, 1000.0 );
          assert(true == ready);

          // Start capture.
          error = P->pCamera->StartCapture(OnImageGrabbed, P->pAcquisitionThread);
          assert( (error == FlyCapture2::PGRERROR_OK) ||
                  (error == FlyCapture2::PGRERROR_ISOCH_ALREADY_STARTED)
                  );
          if ( (error != FlyCapture2::PGRERROR_OK) &&
               (error != FlyCapture2::PGRERROR_ISOCH_ALREADY_STARTED)
               )
            {
              result = false;
            }
          /* if */
        }
      else
        {
          result = false;
        }
      /* if */
    }
  else
    {
      result = false;
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return result;
}
/* AcquisitionParametersFlyCapture2StartTransfer */



//! Release FlyCapture2 SDK classes.
/*!
  Releases FlyCapture2 SDK classes.

  \param P      Pointer to parameters class.
*/
void
AcquisitionParametersFlyCapture2Release(
                                        AcquisitionParametersFlyCapture2 * const P
                                        )
{
  //assert(NULL != P);
  if (NULL == P) return;

#ifdef HAVE_FLYCAPTURE2_SDK
  {
    FlyCapture2::Error error;

    if (NULL != P->pControlDialog)
      {
#ifdef USE_FLYCAPTURE2_GUI
        P->pControlDialog->Hide();
        P->pControlDialog->Disconnect();
#endif /* USE_FLYCAPTURE2_GUI */
      }
    /* if */

    if (NULL != P->pCamera)
      {
        // Stop capture.
        bool const stop = AcquisitionParametersFlyCapture2StopTransfer(P);
        assert(true == stop);

        // Turn off trigger mode.
        if ( (NULL != P->pTriggerMode) && (true == P->pCamera->IsConnected()) )
          {
            P->pTriggerMode->onOff = false;
            error = P->pCamera->SetTriggerMode( P->pTriggerMode );
            assert(error == FlyCapture2::PGRERROR_OK);
          }
        /* if */

        // Disconnect camera.
        if (true == P->pCamera->IsConnected())
          {
            error = P->pCamera->Disconnect();
            assert(error == FlyCapture2::PGRERROR_OK);
          }
        /* if */
      }
    /* if */

    SAFE_DELETE(P->pControlDialog);
    SAFE_DELETE(P->pConfig);
    SAFE_DELETE(P->pTriggerDelay);
    SAFE_DELETE(P->pTriggerMode);
    SAFE_DELETE(P->pCamera);
    SAFE_DELETE(P->pCameraGUID);
    SAFE_DELETE(P->pBusManager);
  }
#endif /* HAVE_FLYCAPTURE2_SDK */

  AcquisitionParametersFlyCapture2Blank_inline( P );
  free( P );
}
/* AcquisitionParametersFlyCapture2Release */



//! Adjust camera exposure time.
/*!
  Adjusts camera exposure time.

  \param P Pointer to parameters structure.
  \param CameraID Unique camera ID.
  \param exposureTime_requested Exposure time in microseconds (us).
  \param exposureTime_achieved Address where achieved exposure time in microseconds (us) will be stored. May be NULL.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFlyCapture2AdjustExposureTime(
                                                   AcquisitionParametersFlyCapture2 * const P,
                                                   int const CameraID,
                                                   double const exposureTime_requested,
                                                   double * const exposureTime_achieved
                                                   )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = true;

#ifdef HAVE_FLYCAPTURE2_SDK
  assert(NULL != P->pCamera);
  if (NULL != P->pCamera)
    {
      result = false; // Assume failure.

      // Turn off delay.
      bool const set = SetPropertyToAbsoluteValue_inline(P, FlyCapture2::TRIGGER_DELAY, false, false, false, 0.0f, NULL);
      assert(true == set);

      // Then adjust shutter.
      FlyCapture2::Error error;

      FlyCapture2::Property shutter;
      shutter.type = FlyCapture2::SHUTTER;

      float exposureTime_f = (float)( 0.001 * exposureTime_requested ); // us to ms

      error = P->pCamera->GetProperty( &shutter );
      assert(error == FlyCapture2::PGRERROR_OK);
      assert(true == shutter.present);
      if ( (error == FlyCapture2::PGRERROR_OK) && (true == shutter.present) )
        {
          shutter.absControl = true;
          shutter.onePush = false;
          shutter.onOff = true;
          shutter.autoManualMode = false;
          shutter.absValue = exposureTime_f;

          error = P->pCamera->SetProperty( &shutter );
          assert(error == FlyCapture2::PGRERROR_OK);
          if (error == FlyCapture2::PGRERROR_OK)
            {
              result = true;

              error = P->pCamera->GetProperty( &shutter );
              assert(error == FlyCapture2::PGRERROR_OK);
              if (error == FlyCapture2::PGRERROR_OK)
                {
                  double const exposureTimeFromCamera = (double)(shutter.absValue) * 1000.0; // ms to us
                  if (NULL != exposureTime_achieved) *exposureTime_achieved = exposureTimeFromCamera;
                  wprintf(gMsgExposureTimeSet, CameraID + 1, exposureTimeFromCamera);

                  float const relative_difference = fabs( (exposureTime_f - shutter.absValue) / exposureTime_f );
                  if (0.005f <= relative_difference)
                    {
                      wprintf(gMsgExposureTimeSetLargeDifference, CameraID + 1);
                    }
                  /* if */

                  exposureTime_f = shutter.absValue;
                }
              else
                {
                  wprintf(gMsgExposureTimeReadError);
                }
              /* if */
            }
          /* if */
        }
      /* if */

      FlyCapture2::Property frame_rate;
      frame_rate.type = FlyCapture2::FRAME_RATE;

      error = P->pCamera->GetProperty( &frame_rate );
      assert(error == FlyCapture2::PGRERROR_OK);
      assert(true == frame_rate.present);
      if ( (error == FlyCapture2::PGRERROR_OK) &&
           (true == frame_rate.present) && (true == frame_rate.onOff) &&
           (true == result)
           )
        {
          frame_rate.absControl = true;
          frame_rate.onePush = false;
          frame_rate.onOff = false;
          frame_rate.autoManualMode = false;
          frame_rate.absValue = 1000.0f / (exposureTime_f + 1.0f);

          error = P->pCamera->SetProperty( &frame_rate );
          assert(error == FlyCapture2::PGRERROR_OK);
        }
      /* if */
    }
  else
    {
      result = false;
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return result;
}
/* AcquisitionParametersFlyCapture2AdjustExposureTime */



//! Adjust camera delay and exposure times.
/*!
  Sets camera exposure and delay times.

  \param P      Pointer to parameters structure.
  \param t_delay_ms        Desired delay time in milliseconds.
  \param t_exp_ms   Desired exposure time in milliseconds.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFlyCapture2SetExposureAndDelayTimes(
                                                         AcquisitionParametersFlyCapture2 * const P,
                                                         double * const t_delay_ms,
                                                         double * const t_exp_ms
                                                         )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = true;

#ifdef HAVE_FLYCAPTURE2_SDK
  if ( (NULL != t_exp_ms) && (0 < *t_exp_ms) )
    {
      float t_exp_ms_f = (float)( *t_exp_ms );
      bool const set1 = SetPropertyToAbsoluteValue_inline(P, FlyCapture2::SHUTTER, false, true, false, t_exp_ms_f, &t_exp_ms_f);
      *t_exp_ms = (double)( t_exp_ms_f );
      result &= set1;
    }
  /* if */

  if ( (NULL != t_delay_ms) && (0 <= *t_delay_ms) )
    {
      float t_delay_s_f = (float)( 0.001 * (*t_delay_ms) ); // Convert to seconds.
      bool const set2 = SetPropertyToAbsoluteValue_inline(P, FlyCapture2::TRIGGER_DELAY, false, true, false, t_delay_s_f, &t_delay_s_f);
      *t_delay_ms = 1000.0 * (double)( t_delay_s_f ); // Convert back to milliseconds.
      result &= set2;
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return result;
}
/* AcquisitionParametersFlyCapture2SetExposureAndDelayTimes */



//! Create FlyCapture2 SDK classes.
/*!
  Create FlyCapture2 SDK classes, connect to camera, and configure camera for
  software triggering in acquisition mode 1.

  \param parameters Pointer to acquisition thread parameters structure.
  \param nFrames Number of buffers to allocate for temporary image storage.
  \param pConnectedCameras A vector of pointers to strings which uniquely identifiy prohibited cameras. May be NULL.
  \return Returns pointer to parameters structure or NULL if unsuccessfull.
*/
AcquisitionParametersFlyCapture2 *
AcquisitionParametersFlyCapture2Create(
                                       AcquisitionParameters_ * const parameters,
                                       int const nFrames,
                                       std::vector<std::wstring *> * const pConnectedCameras
                                       )
{
  AcquisitionParametersFlyCapture2 * const P = (AcquisitionParametersFlyCapture2 *)malloc(sizeof(AcquisitionParametersFlyCapture2));
  assert(NULL != P);
  if (NULL == P) return P;

  AcquisitionParametersFlyCapture2Blank_inline( P );

  P->pAcquisitionThread = parameters;

  bool result = true;
  std::vector<int> prohibited_cameras;

#ifdef HAVE_FLYCAPTURE2_SDK

  FlyCapture2::Error error;


  /****** PRINT FLYCAPTURE2 SDK INFO ******/

  {
    FlyCapture2::FC2Version fc2Version;
    FlyCapture2::Utilities::GetLibraryVersion( &fc2Version );

    unsigned int const major = fc2Version.major;
    unsigned int const minor = fc2Version.minor;
    unsigned int const type = fc2Version.type;
    unsigned int const build = fc2Version.build;

    int const cnt = wprintf(gMsgFlyCapture2Version, major, minor, type, build);
    assert(0 < cnt);
  }

  /****** CREATE ALL CLASSES ******/

  FlyCapture2::TriggerModeInfo triggerModeInfo;

  assert(NULL == P->pBusManager);
  P->pBusManager = new FlyCapture2::BusManager();
  assert(NULL != P->pBusManager);

  assert(NULL == P->pCameraGUID);
  P->pCameraGUID = new FlyCapture2::PGRGuid();
  assert(NULL != P->pCameraGUID);

  assert(NULL == P->pCamera);
  P->pCamera = new FlyCapture2::Camera();
  assert(NULL != P->pCamera);

  assert(NULL == P->pTriggerMode);
  P->pTriggerMode = new FlyCapture2::TriggerMode();
  assert(NULL != P->pTriggerMode);

  assert(NULL == P->pTriggerDelay);
  P->pTriggerDelay = new FlyCapture2::TriggerDelay();
  assert(NULL != P->pTriggerDelay);

  assert(NULL == P->pConfig);
  P->pConfig = new FlyCapture2::FC2Config();
  assert(NULL != P->pConfig);

  assert(NULL == P->pControlDialog);
#ifdef USE_FLYCAPTURE2_GUI
  P->pControlDialog = new FlyCapture2::CameraControlDlg();
  assert(NULL != P->pControlDialog);
#endif /* USE_FLYCAPTURE2_GUI */

  if ( (NULL == P->pBusManager) ||
       (NULL == P->pCameraGUID) ||
       (NULL == P->pCamera) ||
       (NULL == P->pTriggerMode) ||
       (NULL == P->pTriggerDelay) ||
       (NULL == P->pConfig)
#ifdef USE_FLYCAPTURE2_GUI
       || (NULL == P->pControlDialog)
#endif /* USE_FLYCAPTURE2_GUI */
       )
    {
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */


  /****** ENUMERATE DEVICES ******/

  /* First enumerate all attached devices; if there are no devices attached then terminate.
     After devices are enumerated we will list device interfaces and camera information.
     For this test application we always connect to first found camera, but that may
     be changed by allowing user to selet which camera to connect to.
  */

  unsigned int numCameras = 0; // Number of cameras.
  error = P->pBusManager->GetNumOfCameras(&numCameras);
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  if ( 1 > numCameras )
    {
      int const cnt = wprintf(gMsgCameraDetectionFailed);
      assert(0 < cnt);
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  else
    {
      int const cnt = wprintf(gMsgCameraDetectionSucceeded, numCameras);
      assert(0 < cnt);
    }
  /* if */

  /* There must be at least one available camera. */
  int selected_camera = -1;
  if (NULL != pConnectedCameras)
    {
      bool all_prohibited = true;

      for (unsigned int i = 0; i < numCameras; ++i)
        {
          FlyCapture2::PGRGuid guid;
          FlyCapture2::Camera cam;
          FlyCapture2::CameraInfo camInfo;

          error = P->pBusManager->GetCameraFromIndex(i, &guid);
          assert(error == FlyCapture2::PGRERROR_OK);

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.Connect(&guid);
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.GetCameraInfo(&camInfo);
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              std::wstring serial( std::to_wstring( (long long)(camInfo.serialNumber) ) );
              bool prohibited = false; // Assume camera is available.
              for (size_t j = 0; j < pConnectedCameras->size(); ++j)
                {
                  if ( (NULL != (*pConnectedCameras)[j]) && (0 == (*pConnectedCameras)[j]->compare(serial)) )
                    {
                      prohibited = true;
                      prohibited_cameras.push_back(i);
                      break;
                    }
                  /* if */
                }
              /* for */
              all_prohibited = all_prohibited && prohibited;
              if ( (-1 == selected_camera) && (false == prohibited) ) selected_camera = i;
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.Disconnect();
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */
        }
      /* for */

      if ( true == all_prohibited )
        {
          int const cnt = wprintf(gMsgCameraDetectionNoFreeCameras);
          assert(0 < cnt);
          result = false;
          goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
        }
      /* if */
    }
  else
    {
      selected_camera = 0;
    }
  /* if */

  int const num_prohibited = (int)( prohibited_cameras.size() );
  int const num_available = (int)(numCameras) - num_prohibited;
  {
    int const cnt = wprintf(gMsgCameraDetectionAvailable, numCameras, num_available);
    assert(0 < cnt);
  }

  /* Print camera selection menu. */
  bool list_details = false;
  if (1 < num_available)
    {

    ACQUISITION_PARAMETERS_FLYCAPTURE2_CAMERA_SELECTION_MENU:

      {
        int const cnt1 = wprintf(L"\n");
        assert(0 < cnt1);

        int const cnt2 = wprintf(gMsgCameraSelectionMenu);
        assert(0 < cnt2);
      }
      if (false == list_details)
        {
          int const cnt = wprintf(gMsgCameraSelectionListDetails);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgCameraSelectionListNoDetails);
          assert(0 < cnt);
        }
      /* if */

      for (unsigned int i = 0; i < numCameras; ++i)
        {
          FlyCapture2::PGRGuid guid;
          FlyCapture2::Camera cam;
          FlyCapture2::CameraInfo camInfo;
          FlyCapture2::InterfaceType interfaceType;
          bool prohibited = false;

          error = P->pBusManager->GetCameraFromIndex(i, &guid);
          assert(error == FlyCapture2::PGRERROR_OK);

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.Connect(&guid);
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.GetCameraInfo(&camInfo);
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              for (int j = 0; j < num_prohibited; ++j)
                {
                  if (i == prohibited_cameras[j])
                    {
                      prohibited = true;
                      break;
                    }
                  /* if */
                }
              /* for */

              if (false == prohibited)
                {
                  if (false == list_details)
                    {
                      if (selected_camera == i)
                        {
                          int const cnt = printf(gMsgCameraSelectionMenuItemDefault, i + 1, camInfo.modelName, camInfo.serialNumber);
                          assert(0 < cnt);
                        }
                      else
                        {
                          int const cnt = printf(gMsgCameraSelectionMenuItem, i + 1, camInfo.modelName, camInfo.serialNumber);
                          assert(0 < cnt);
                        }
                      /* if */
                    }
                  else
                    {
                      if (selected_camera == i)
                        {
                          int const cnt = printf(
                                                 gMsgCameraSelectionMenuItemDetailsDefault,
                                                 i + 1, camInfo.modelName, camInfo.serialNumber,
                                                 camInfo.serialNumber,
                                                 camInfo.modelName,
                                                 camInfo.vendorName,
                                                 camInfo.sensorInfo,
                                                 camInfo.sensorResolution,
                                                 camInfo.driverName,
                                                 camInfo.firmwareVersion,
                                                 camInfo.firmwareBuildTime
                                                 );
                          assert(0 < cnt);
                        }
                      else
                        {
                          int const cnt = printf(
                                                 gMsgCameraSelectionMenuItemDetails,
                                                 i + 1, camInfo.modelName, camInfo.serialNumber,
                                                 camInfo.serialNumber,
                                                 camInfo.modelName,
                                                 camInfo.vendorName,
                                                 camInfo.sensorInfo,
                                                 camInfo.sensorResolution,
                                                 camInfo.driverName,
                                                 camInfo.firmwareVersion,
                                                 camInfo.firmwareBuildTime
                                                 );
                          assert(0 < cnt);
                        }
                      /* if */
                    }
                  /* if */
                }
              /* if */
            }
          /* if */

          if ( (true == list_details) && (false == prohibited) )
            {
              if (error == FlyCapture2::PGRERROR_OK)
                {
                  error = P->pBusManager->GetInterfaceTypeFromGuid( &guid, &interfaceType );
                  assert(error == FlyCapture2::PGRERROR_OK);
                }
              /* if */

              if (error == FlyCapture2::PGRERROR_OK)
                {
                  switch( interfaceType )
                    {
                    case FlyCapture2::INTERFACE_IEEE1394:
                      {
                        int const cnt = printf(gMsgCameraSelectionMenuItemDetailsConnectionIEEE1394);
                        assert(0 < cnt);
                      }
                      break;

                    case FlyCapture2::INTERFACE_USB2:
                      {
                        int const cnt = printf(gMsgCameraSelectionMenuItemDetailsConnectionUSB2);
                        assert(0 < cnt);
                      }
                      break;

                    case FlyCapture2::INTERFACE_USB3:
                      {
                        int const cnt = printf(gMsgCameraSelectionMenuItemDetailsConnectionUSB3);
                        assert(0 < cnt);
                      }
                      break;

                    case FlyCapture2::INTERFACE_GIGE:
                      {
                        int const cnt = printf(gMsgCameraSelectionMenuItemDetailsConnectionETH);
                        assert(0 < cnt);
                      }
                      break;

                    case FlyCapture2::INTERFACE_UNKNOWN:
                    case FlyCapture2::INTERFACE_TYPE_FORCE_32BITS:
                    default:
                      // Nothing to do!
                      break;
                    }
                  /* switch */
                }
              /* if */
            }
          /* if */

          if (error == FlyCapture2::PGRERROR_OK)
            {
              error = cam.Disconnect();
              assert(error == FlyCapture2::PGRERROR_OK);
            }
          /* if */
        }
      /* for */

      int const pressed_key = TimedWaitForNumberKey(60000, 10, false, true, (HWND)NULL);
      if (0 == pressed_key)
        {
          list_details = !list_details;
          goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CAMERA_SELECTION_MENU;
        }
      else if ( (-1 != pressed_key) && (1 <= pressed_key) && (pressed_key <= (int)(numCameras)) )
        {
          int const requested_camera = pressed_key - 1;
          bool prohibited = false;
          for (int i = 0; i < num_prohibited; ++i)
            {
              if (requested_camera == prohibited_cameras[i])
                {
                  prohibited = true;
                  break;
                }
              /* if */
            }
          /* if */
          if (false == prohibited)
            {
              selected_camera = requested_camera;
            }
          else
            {
              int const cnt = wprintf(gMsgCameraSelectionMenuRevertToDefault);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgCameraSelectionMenuRevertToDefault);
          assert(0 < cnt);
        }
      /* if */
    }
  else
    {
      int const cnt = wprintf(gMsgCameraDetectionOneAvailable);
      assert(0 < cnt);
    }
  /* if */

  assert( (0 <= selected_camera) && (selected_camera < (int)numCameras) );
  if ( (0 > selected_camera) || (selected_camera >= (int)numCameras) )
    {
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */


  /****** CONNECT TO SELECTED DEVICE ******/

  /* Connect to first found camera. Camera class is created on stack so destructor
     call is not needed as it will be automatically called as stack unwinds.
  */

  error = P->pBusManager->GetCameraFromIndex(selected_camera, P->pCameraGUID);
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  error = P->pCamera->Connect(P->pCameraGUID);
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  else
    {
      FlyCapture2::CameraInfo camInfo;

      if (error == FlyCapture2::PGRERROR_OK)
        {
          error = P->pCamera->GetCameraInfo(&camInfo);
          assert(error == FlyCapture2::PGRERROR_OK);
        }
      /* if */

      if (error == FlyCapture2::PGRERROR_OK)
        {
          int const cnt = printf(gMsgConnectedToCamera, camInfo.modelName, camInfo.serialNumber);
          assert(0 < cnt);
        }
      /* if */
    }
  /* if */

  /* Power on camera. */
  bool const power = PowerOnCamera( P->pCamera );
  assert(true == power);

  /* Check for software trigger support. This functionality may be checked for by
     requesting TriggerModeInfo structure or by directy reading appropriate camera
     registers.
  */

  bool const mode14 = IsMode14Available( P->pCamera ); // Mode 14 is overlapped exposure-readout mode.
  bool const mode15 = IsMode15Available( P->pCamera ); // Mode 15 is mult-shot trigger mode.

  bool const trigger_available = IsSoftwareTriggerAvailable( P->pCamera );

  error = P->pCamera->GetTriggerModeInfo( &triggerModeInfo );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  if ( true != triggerModeInfo.softwareTriggerSupported )
    {
      assert(true == trigger_available);
      int const cnt = printf(gMsgCameraDoesNotSupportSoftwareTrigger);
      assert(0 < cnt);
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  // Turn off trigger delay.
  error = P->pCamera->GetTriggerDelay( P->pTriggerDelay );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  P->pTriggerDelay->absControl = false;
  P->pTriggerDelay->onePush = false;
  P->pTriggerDelay->onOff = false;
  P->pTriggerDelay->valueA = 0;
  P->pTriggerDelay->valueB = 0;

  error = P->pCamera->SetTriggerDelay( P->pTriggerDelay );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  // Configure camera.
  error = P->pCamera->GetConfiguration( P->pConfig );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  P->pConfig->numBuffers = 18;
  P->pConfig->grabMode = FlyCapture2::BUFFER_FRAMES;
  P->pConfig->grabTimeout = 5000;

  error = P->pCamera->SetConfiguration( P->pConfig );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  // Turn off auto-adjustments or completely turn off some properties.
  // The goal is to set the camera in linear mode.
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::BRIGHTNESS);
  TurnOffProperty_inline(P, FlyCapture2::AUTO_EXPOSURE);
  TurnOffProperty_inline(P, FlyCapture2::SHARPNESS);
  TurnOffProperty_inline(P, FlyCapture2::WHITE_BALANCE);
  TurnOffProperty_inline(P, FlyCapture2::HUE);
  TurnOffProperty_inline(P, FlyCapture2::SATURATION);
  TurnOffProperty_inline(P, FlyCapture2::GAMMA);
  TurnOffProperty_inline(P, FlyCapture2::IRIS);
  TurnOffProperty_inline(P, FlyCapture2::FOCUS);
  TurnOffProperty_inline(P, FlyCapture2::ZOOM);
  TurnOffProperty_inline(P, FlyCapture2::PAN);
  TurnOffProperty_inline(P, FlyCapture2::TILT);
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::SHUTTER);
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::GAIN);
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::TRIGGER_MODE);
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::TRIGGER_DELAY);
  TurnOffAutoAdjustmentForProperty_inline(P, FlyCapture2::FRAME_RATE);

  SetPropertyToRelativeValue_inline(P, FlyCapture2::BRIGHTNESS, 0, 0);
  SetPropertyToRelativeValue_inline(P, FlyCapture2::GAIN, 0, 0);

  // Set trigger mode to software triggering.
  error = P->pCamera->GetTriggerMode( P->pTriggerMode );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  /* if */

  P->pTriggerMode->onOff = true;
  P->pTriggerMode->source = 7; // Software trigger.
  P->pTriggerMode->mode = (true == mode14)? 14 : 0; // Use overlapped trigger mode if available.
  P->pTriggerMode->parameter = 0;

  error = P->pCamera->SetTriggerMode( P->pTriggerMode );
  if (error != FlyCapture2::PGRERROR_OK)
    {
      error.PrintErrorTrace();
      result = false;
      goto ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT;
    }
  else
    {
      bool const trigger_ready = CheckTriggerReady( P->pCamera );
      //assert(true == trigger_ready);
    }
  /* if */


  /****** START ACQUISITION ******/

  bool const start_flycapture2 = AcquisitionParametersFlyCapture2StartTransfer(P);
  assert(true == start_flycapture2);


 ACQUISITION_PARAMETERS_FLYCAPTURE2_CREATE_EXIT:

#endif HAVE_FLYCAPTURE2_SDK

  if (true != result)
    {
      AcquisitionParametersFlyCapture2Release(P);
      return NULL;
    }
  /* if */

  return P;
}
/* AcquisitionParametersFlyCapture2Create */



//! Get camera ID.
/*!
  Returns unique camera identifier.
  For FlyCapture2 API the function returns serial number as string.
  Returned pointer must be deleted after camera name is no longer needed.

  \param P Pointer to acquisition thread parameters.
  \return Returns pointer to unique camera name.
*/
std::wstring *
AcquisitionParametersFlyCapture2GetCameraIdentifier(
                                                    AcquisitionParametersFlyCapture2 * const P
                                                    )
{
  std::wstring * name = NULL;

  assert(NULL != P);
  if (NULL == P) return name;

#ifdef HAVE_FLYCAPTURE2_SDK
  if (NULL != P->pCamera)
    {
      FlyCapture2::Error error;
      FlyCapture2::CameraInfo camInfo;

      assert(true == P->pCamera->IsConnected());
      if (true == P->pCamera->IsConnected())
        {
          error = P->pCamera->GetCameraInfo(&camInfo);
          assert(error == FlyCapture2::PGRERROR_OK);

          if (error == FlyCapture2::PGRERROR_OK)
            {
              name = new std::wstring( std::to_wstring( (long long)(camInfo.serialNumber) ) );
              assert(NULL != name);
            }
          /* if */
        }
      /* if */
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */

  return name;
}
/* AcquisitionParametersFlyCapture2GetCameraIdentifier */



#ifdef HAVE_FLYCAPTURE2_SDK

//! Get image data type.
/*!
  Returns image data type.

  \param pImage Pointer to FlyCapture2 image class.
  \param pCamera Pointer to FlyCapture2 camera class.
  \return Returns image data type.
*/
ImageDataType
GetImageDataType(
                 FlyCapture2::Image * const pImage,
                 FlyCapture2::Camera * const pCamera
                 )
{
  assert(NULL != pImage);
  if (NULL == pImage) return IDT_UNKNOWN;

  ImageDataType type = IDT_UNKNOWN;

  // Regarding FlyCapture2 documentation it is not clear if 10 and 12 bit formats are packed or not.
  switch (pImage->GetPixelFormat())
    {
    case FlyCapture2::PIXEL_FORMAT_MONO8: // 8 bits of mono information.
      {
        assert( FlyCapture2::NONE == pImage->GetBayerTileFormat() );
        type = IDT_8U_GRAY;
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_411YUV8: // YUV 4:1:1.
      type = IDT_8U_YUV411;
      break;

    case FlyCapture2::PIXEL_FORMAT_422YUV8: // YUV 4:2:2.
      type = IDT_8U_YUV422;
      break;

    case FlyCapture2::PIXEL_FORMAT_444YUV8: // YUV 4:4:4.
      type = IDT_8U_UYV444;
      break;

    case FlyCapture2::PIXEL_FORMAT_RGB8: // R = G = B = 8 bits.
      //case FlyCapture2::PIXEL_FORMAT_RGB: // 24 bit RGB, same as PIXEL_FORMAT_RGB8.
      type = IDT_8U_RGB;
      break;

    case FlyCapture2::PIXEL_FORMAT_MONO16: // 16 bits of mono information.
      {
        assert( FlyCapture2::NONE == pImage->GetBayerTileFormat() );
        bool const isbigendian = IsY16DataBigEndian(pCamera);
        if (true == isbigendian)
          {
            type = IDT_16U_GRAY_BigEndian;
          }
        else
          {
            type = IDT_16U_GRAY;
          }
        /* if */
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_RGB16: // R = G = B = 16 bits.
      assert(IDT_UNKNOWN == type);
      break;

    case FlyCapture2::PIXEL_FORMAT_S_MONO16: // 16 bits of signed mono information.
      {
        assert( FlyCapture2::NONE == pImage->GetBayerTileFormat() );
        bool const isbigendian = IsY16DataBigEndian(pCamera);
        if (true == isbigendian)
          {
            type = IDT_16S_GRAY_BigEndian;
          }
        else
          {
            type = IDT_16S_GRAY;
          }
        /* if */
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_S_RGB16: // R = G = B = 16 bits signed.
      assert(IDT_UNKNOWN == type);
      break;

    case FlyCapture2::PIXEL_FORMAT_RAW8: // 8 bit raw data output of sensor.
      {
        switch( pImage->GetBayerTileFormat() )
          {
          case FlyCapture2::RGGB: // Red-Green-Green-Blue.
            type = IDT_8U_BayerRG;
            break;

          case FlyCapture2::GRBG: // Green-Red-Blue-Green.
            type = IDT_8U_BayerGR;
            break;

          case FlyCapture2::GBRG: // Green-Blue-Red-Green.
            type = IDT_8U_BayerGB;
            break;

          case FlyCapture2::BGGR: // Blue-Green-Green-Red.
            type = IDT_8U_BayerBG;
            break;

          case FlyCapture2::NONE: // No Bayer tile format.
            type = IDT_8U_GRAY;
            break;

          default:
            assert(IDT_UNKNOWN == type);
            break;
          }
        /* switch */
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_RAW16: // 16 bit raw data output of sensor.
      {
        bool const isbigendian = IsY16DataBigEndian(pCamera);
        if (true == isbigendian)
          {
            switch( pImage->GetBayerTileFormat() )
              {
              case FlyCapture2::RGGB: // Red-Green-Green-Blue.
                type = IDT_16U_BayerRG_BigEndian;
                break;

              case FlyCapture2::GRBG: // Green-Red-Blue-Green.
                type = IDT_16U_BayerGR_BigEndian;
                break;

              case FlyCapture2::GBRG: // Green-Blue-Red-Green.
                type = IDT_16U_BayerGB_BigEndian;
                break;

              case FlyCapture2::BGGR: // Blue-Green-Green-Red.
                type = IDT_16U_BayerBG_BigEndian;
                break;

              case FlyCapture2::NONE: // No Bayer tile format.
                type = IDT_16U_GRAY_BigEndian;
                break;

              default:
                assert(IDT_UNKNOWN == type);
                break;
              }
            /* switch */
          }
        else
          {
            switch( pImage->GetBayerTileFormat() )
              {
              case FlyCapture2::RGGB: // Red-Green-Green-Blue.
                type = IDT_16U_BayerRG;
                break;

              case FlyCapture2::GRBG: // Green-Red-Blue-Green.
                type = IDT_16U_BayerGR;
                break;

              case FlyCapture2::GBRG: // Green-Blue-Red-Green.
                type = IDT_16U_BayerGB;
                break;

              case FlyCapture2::BGGR: // Blue-Green-Green-Red.
                type = IDT_16U_BayerBG;
                break;

              case FlyCapture2::NONE: // No Bayer tile format.
                type = IDT_16U_GRAY;
                break;

              default:
                assert(IDT_UNKNOWN == type);
                break;
              }
            /* switch */
          }
        /* if */
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_MONO12: // 12 bits of mono information.
      {
        assert( FlyCapture2::NONE == pImage->GetBayerTileFormat() );
        type = IDT_12U_GRAY_Packed;
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_RAW12: // 12 bit raw data output of sensor.
      {
        switch( pImage->GetBayerTileFormat() )
          {
          case FlyCapture2::RGGB: // Red-Green-Green-Blue.
            type = IDT_12U_BayerRG_Packed;
            break;

          case FlyCapture2::GRBG: // Green-Red-Blue-Green.
            type = IDT_12U_BayerGR_Packed;
            break;

          case FlyCapture2::GBRG: // Green-Blue-Red-Green.
            type = IDT_12U_BayerGB_Packed;
            break;

          case FlyCapture2::BGGR: // Blue-Green-Green-Red.
            type = IDT_12U_BayerBG_Packed;
            break;

          case FlyCapture2::NONE: // No Bayer tile format.
            type = IDT_12U_GRAY_Packed;
            break;

          default:
            assert(IDT_UNKNOWN == type);
            break;
          }
        /* switch */
      }
      break;

    case FlyCapture2::PIXEL_FORMAT_BGR: // 24 bit BGR.
      type = IDT_8U_BGR;
      break;

    case FlyCapture2::PIXEL_FORMAT_BGRU: // 32 bit BGRU.
      type = IDT_8U_BGRA;
      break;

    case FlyCapture2::PIXEL_FORMAT_RGBU: // 32 bit RGBU.
      type = IDT_8U_RGBA;
      break;

    case FlyCapture2::PIXEL_FORMAT_BGR16: // R = G = B = 16 bits.
    case FlyCapture2::PIXEL_FORMAT_BGRU16: // 64 bit BGRU.
    case FlyCapture2::PIXEL_FORMAT_422YUV8_JPEG: // JPEG compressed stream.

    case FlyCapture2::UNSPECIFIED_PIXEL_FORMAT: // Unspecified pixel format.

    default:
      assert(IDT_UNKNOWN == type);
      break;
    }
  /* switch */

  return type;
}
/* GetImageDataType */



//! Get pixel format.
/*!
  Function returns pixel format corresponding to the image data type.

  \param type   Image data type.
  \return A valid pixel format or FlyCapture2::UNSPECIFIED_PIXEL_FORMAT.
*/
FlyCapture2::PixelFormat
GetFlyCapture2PixelFormat(
                          ImageDataType const type
                          )
{
  FlyCapture2::PixelFormat format = FlyCapture2::UNSPECIFIED_PIXEL_FORMAT;

  switch (type)
    {
    default:
    case IDT_UNKNOWN:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_BINARY:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_GRAY:
      format = FlyCapture2::PIXEL_FORMAT_MONO8;
      break;

    case IDT_12U_GRAY_Packed:
      format = FlyCapture2::PIXEL_FORMAT_MONO12;
      break;

    case IDT_16U_GRAY:
    case IDT_16U_GRAY_BigEndian:
      format = FlyCapture2::PIXEL_FORMAT_MONO16;
      break;

    case IDT_32U_GRAY:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8S_GRAY:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_16S_GRAY:
    case IDT_16S_GRAY_BigEndian:
      format = FlyCapture2::PIXEL_FORMAT_S_MONO16;
      break;

    case IDT_32S_GRAY:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
      format = FlyCapture2::PIXEL_FORMAT_RAW8;
      break;

    case IDT_12U_BayerGR_Packed:
    case IDT_12U_BayerRG_Packed:
    case IDT_12U_BayerGB_Packed:
    case IDT_12U_BayerBG_Packed:
      format = FlyCapture2::PIXEL_FORMAT_RAW12;
      break;

    case IDT_16U_BayerGR:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerGR_BigEndian:
    case IDT_16U_BayerRG_BigEndian:
    case IDT_16U_BayerGB_BigEndian:
    case IDT_16U_BayerBG_BigEndian:
      format = FlyCapture2::PIXEL_FORMAT_RAW16;
      break;

    case IDT_8U_RGB:
      format = FlyCapture2::PIXEL_FORMAT_RGB8;
      break;

    case IDT_8U_RGB_Planar:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_RGBA:
      format = FlyCapture2::PIXEL_FORMAT_RGBU;
      break;

    case IDT_8U_BGR:
      format = FlyCapture2::PIXEL_FORMAT_BGR;
      break;

    case IDT_8U_BGRA:
      format = FlyCapture2::PIXEL_FORMAT_BGRU;
      break;

    case IDT_8U_YUV411:
      format = FlyCapture2::PIXEL_FORMAT_411YUV8;
      break;

    case IDT_8U_YUV422:
      format = FlyCapture2::PIXEL_FORMAT_422YUV8;
      break;

    case IDT_8U_YUV422_BT601:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_YUV422_BT709:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_YUV444:
      assert(FlyCapture2::UNSPECIFIED_PIXEL_FORMAT == format);
      break;

    case IDT_8U_UYV444:
      format = FlyCapture2::PIXEL_FORMAT_444YUV8;
      break;
    }
  /* switch */

  return format;
}
/* GetFlyCapture2PixelFormat */



//! Get Bayer format.
/*!
  Returns Bayer tile format.

  \param type   Image data type.
  \return Returns Bayer tile format.
*/
FlyCapture2::BayerTileFormat
GetFlyCapture2BayerTileFormat(
                              ImageDataType const type
                              )
{
  FlyCapture2::BayerTileFormat format = FlyCapture2::NONE;

  switch (type)
    {
    case IDT_8U_BayerGR:
    case IDT_12U_BayerGR_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerGR_BigEndian:
      format = FlyCapture2::GRBG;
      break;

    case IDT_8U_BayerRG:
    case IDT_12U_BayerRG_Packed:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerRG_BigEndian:
      format = FlyCapture2::RGGB;
      break;

    case IDT_8U_BayerGB:
    case IDT_12U_BayerGB_Packed:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerGB_BigEndian:
      format = FlyCapture2::GBRG;
      break;

    case IDT_8U_BayerBG:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerBG_BigEndian:
      format = FlyCapture2::BGGR;
      break;

    default:
      assert(FlyCapture2::NONE == format);
      break;
    }
  /* switch */

  return format;
}
/* GetFlyCapture2BayerTileFormat */



#endif /* HAVE_FLYCAPTURE2_SDK */



#endif /* __BATCHACQUISITIONFLYCAPTURE2_CPP */
