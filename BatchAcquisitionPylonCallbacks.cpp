/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2021 UniZG, Zagreb. All rights reserved.
 * (c) 2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionPylonCallbacks.cpp
  \brief  Callback functions for Basler Pylon.

  \author Ivan Vrkic
  \author Tomislav Petkovic
  \date   2021-06-25
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPYLONCALLBACKS_CPP
#define __BATCHACQUISITIONPYLONCALLBACKS_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionPylonCallbacks.h"
#include "BatchAcquisitionEvents.h"


#ifdef HAVE_PYLON_SDK

#include <iostream>


using namespace Pylon;

//! OnAttach event handler.
/*!
  Handles OnAttach event.

  \param camera Reference to camera class.
*/
void
CCustomConfigurationEventHandler::OnAttach(
                                           CInstantCamera & camera
                                           )
{
  Debugfprintf(stderr, "OnAttach event");
}
/* CCustomConfigurationEventHandler::OnAttach */



//! OnAttached event handler.
/*!
  Handles OnAttached event.

  \param camera Reference to camera class.
*/
void
CCustomConfigurationEventHandler::OnAttached(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnAttached, camera.GetDeviceInfo().GetModelName().c_str());
}
/* CCustomConfigurationEventHandler::OnAttached */


//! OnOpen event handler.
/*!
  Handles OnOpen event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnOpen(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnOpen, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnOpen */

//! OnOpened event handler.
/*!
  Handles OnOpened event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnOpened(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnOpened, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnOpened */

//! OnGrabStart event handler.
/*!
  Handles OnGrabStart event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnGrabStart(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnGrabStart, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnGrabStart */

//! OnGrabStarted event handler.
/*!
  Handles OnGrabStarted event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnGrabStarted(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnGrabStarted, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnGrabStarted */

//! OnGrabStop event handler.
/*!
  Handles OnGrabStop event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnGrabStop(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnGrabStop, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnGrabStop */

//! OnGrabStopped event handler.
/*!
  Handles OnGrabStopped event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnGrabStopped(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnGrabStopped, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnGrabStopped */

//! OnClose event handler.
/*!
  Handles OnClose event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnClose(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnClose, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnClose */

//! OnClosed event handler.
/*!
  Handles OnClosed event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnClosed(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnClosed, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnClosed */

//! OnDestroy event handler.
/*!
  Handles OnDestroy event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnDestroy(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnDestroy, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnDestroy */

//! OnDestroyed event handler.
/*!
  Handles OnDestroyed event.

  \param Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnDestroyed(CInstantCamera& /*camera*/)
{
  Debugfwprintf(stderr, gDbgEventOnDestroyed);
}
/* CCustomConfigurationEventHandler::OnDestroyed */

//! OnDetach event handler.
/*!
  Handles OnDetach event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnDetach(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnDetach, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnDetach */

//! OnDetached event handler.
/*!
  Handles OnDetached event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnDetached(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnDetached, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnDetached */

//! OnGrabError event handler.
/*!
  Handles OnGrabError event.

  \param camera Reference to camera class.
  \param camera Pointer to error message.
*/
void CCustomConfigurationEventHandler::OnGrabError(CInstantCamera& camera, const char* errorMessage)
{
  Debugfwprintf(stderr, gDbgEventOnGrabError, camera.GetDeviceInfo().GetModelName());
  Debugfwprintf(stderr, gDbgEventOnGrabErrorMessage, errorMessage);
}
/* CCustomConfigurationEventHandler::OnGrabError */

//! OnCameraDeviceRemoved event handler.
/*!
  Handles OnCameraDeviceRemoved event.

  \param camera Reference to camera class.
*/
void CCustomConfigurationEventHandler::OnCameraDeviceRemoved(CInstantCamera& camera)
{
  Debugfwprintf(stderr, gDbgEventOnCameraDeviceRemoved, camera.GetDeviceInfo().GetModelName());
}
/* CCustomConfigurationEventHandler::OnCameraDeviceRemoved */

//! 
/*!
  <long-description>

  Warning: Only very short processing tasks should be performed by this method.
  Otherwise, the event notification will block the processing of images.

  \param camera 
  \param userProvidedId 
  \param pNode  
*/
void
CCustomCameraEventHandler::OnCameraEvent(
                                         CBaslerUniversalInstantCamera & camera,
                                         intptr_t userProvidedId,
                                         GenApi::INode * pNode
                                         )
{
  assert(NULL != this->pAcquisition);
  if (NULL == this->pAcquisition) return;

  SynchronizationEvents * const pSynchronization = this->pAcquisition->pSynchronization;
  assert(NULL != pSynchronization);
  if (NULL == pSynchronization) return;

  int const CameraID = this->pAcquisition->CameraID;

  switch (userProvidedId)
    {
      
    case ExposureEndEventID:
      {
        assert(true == this->pAcquisition->fExposureInProgress);
        this->pAcquisition->fExposureInProgress = false;
        
        if (NULL != pSynchronization)
          {
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID));
            assert(true == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID));
            
            BOOL const set_exposure_end = (pSynchronization)->EventSet(CAMERA_EXPOSURE_END, CameraID);
            assert(0 != set_exposure_end);
          }
        /* if */
      }
      break;
      
    case FrameStartEventID:
      {
        assert(false == this->pAcquisition->fExposureInProgress);
        this->pAcquisition->fExposureInProgress = true;

        if (NULL != pSynchronization)
          {
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID));
            
            BOOL const set_exposure_begin = pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, CameraID);
            assert(0 != set_exposure_begin);
          }
        /* if */
      }
      break;
      
    case FrameTriggerMissedEventID:
      {
        assert(false == this->pAcquisition->fExposureInProgress);

        if (NULL != pSynchronization)
          {
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_READY, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_BEGIN, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_EXPOSURE_END, CameraID));
            assert(false == DebugIsSignalled(pSynchronization, CAMERA_INVALID_TRIGGER, CameraID));
            
            BOOL const set_invalid_trigger = pSynchronization->EventSet(CAMERA_INVALID_TRIGGER, CameraID);
            assert(0 != set_invalid_trigger);
          }
        /* if */
      }
      
      break;
      
    case FrameStartWaitEventID:
      {
        // TODO: Check when this events occurs.
      }
      break;
      
    case EventOverrunEventID:
      {
        Debugfprintf(stderr, "Event Overrun event. FrameID: ", camera.EventOverrunEventFrameID.GetValue());
      }      
      break;
      
    }
  /* switch */  
}
/* CCustomCameraEventHandler::OnCameraEvent */



//! 
/*!
  <long-description>

  \param camera 
  \param ptrGrabResult  
*/
void
CCustomImageEventHandler::OnImageGrabbed(
                                         CInstantCamera & camera,
                                         const CGrabResultPtr & ptrGrabResult
                                         )
{
  assert(NULL != this->pAcquisition);
  if (NULL == this->pAcquisition) return;
  
  // Fetch timestamp.
  LARGE_INTEGER QPC_after_transfer;
  QPC_after_transfer.QuadPart = (LONGLONG)0;

  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_transfer );
  assert(TRUE == qpc_after);

  // Signal data transfer has ended.
  if (NULL != this->pAcquisition->pSynchronization)
    {
      BOOL const set_transfer_end = this->pAcquisition->pSynchronization->EventSet(CAMERA_TRANSFER_END, this->pAcquisition->CameraID);
      assert(0 != set_transfer_end);
    }
  /* if */

  // Fetch flags.
  bool fBlocking = true;
  bool fFixed = false;
  if (NULL != this->pAcquisition->pWindow)
    {
      fBlocking = this->pAcquisition->pWindow->fBlocking;
      fFixed = this->pAcquisition->pWindow->fFixed;
    }
  /* if */

  // Fetch image metadata.
  ImageMetadata sData;
  ImageMetadataBlank(&sData);
  bool const pop = PopFrontImageMetadataFromQueue(this->pAcquisition->pMetadataQueue, &sData);
  if (true == pop)
    {
      LARGE_INTEGER QPC_before_trigger;
      QPC_before_trigger.QuadPart = sData.QPC_before_trigger;

      FrameStatisticsAddMeasurement(this->pAcquisition->pStatisticsAcquisitionDuration, QPC_before_trigger, QPC_after_transfer);

#ifdef _DEBUG
      if (NULL != this->pAcquisition->pWindow)
        {
          if (true == this->pAcquisition->pWindow->fBlocking) assert(true == sData.fBlocking);
          if (true == this->pAcquisition->pWindow->fFixed) assert(true == sData.fFixed);
        }
      /* if */
#endif /* _DEBUG */
    }
  /* if */

  // Fetch SDK pointer.
  AcquisitionParametersPylon * const pPylon = this->pAcquisition->pPylonSDK;
  assert(NULL != pPylon);
  if (NULL == pPylon)
    {
      ImageMetadataRelease(&sData);
      return;
    }
  /* if */

  // Test if buffer is acquired correctly.
  bool acquired = true;
  if (NULL != pPylon)
    {
      // TODO: Image buffer test code goes here!
      BOOL const get = ptrGrabResult->GrabSucceeded();
      if (FALSE == get)
      {
              // Acquisition failed.
              acquired = false;
              if ((true == sData.fBatch) && (false == sData.fFixed))
              {
                  int const CameraID = this->pAcquisition->CameraID;
                  int const index = sData.index;
                  int const ProjectorID = sData.ProjectorID;
                  unsigned int const retry = sData.retry + 1;
                  TCHAR const* const pFilename = sData.pFilename->c_str();

                  Debugfwprintf(stderr, gDbgImageTransferFailed, CameraID + 1, sData.key + 1);
                  Debugfwprintf(stderr, gDbgRequeueSLPattern, CameraID + 1, pFilename, retry, 2);

                  // Requeue image.
                  if ((NULL != this->pAcquisition->pImageDecoder) &&
                      (NULL != this->pAcquisition->pImageDecoder->pImageList) &&
                      (3 > retry)
                      )
                  {
                      ImageFileList* const pImageList = this->pAcquisition->pImageDecoder->pImageList;
                      assert(NULL != pImageList);

                      QueuedDecoderImage* item = NULL;

                      StructuredLightPatternType pattern_type = SL_PATTERN_INVALID;

                      double const delay = 0.0;
                      double const exposure = 0.0;
                      bool const skip_acquisition = false;

                      wchar_t filename[MAX_PATH];
                      bool const get_filename = pImageList->GetFileNameAt(index, filename, MAX_PATH);

                      bool const have_filename = pImageList->HaveFileNameAt(index);
                      if (true == have_filename)
                      {
                          wchar_t fullname[MAX_PATH];
                          bool const get_URI = pImageList->GetFullFileNameAt(index, fullname, MAX_PATH);

                          pattern_type = SL_PATTERN_FROM_FILE;

                          bool const get = get_URI && get_filename;
                          if (true == get)
                          {
                              assert(NULL == item);
                              item = new QueuedDecoderImage(this->pAcquisition->pImageDecoder->pWICFactory, fullname);
                              assert(NULL != item);
                          }
                          /* if */
                      }
                      else
                      {
                          pattern_type = SL_PATTERN_BLACK;

                          D3DCOLORVALUE color_black;
                          color_black.r = 0.0f;
                          color_black.g = 0.0f;
                          color_black.b = 0.0f;
                          color_black.a = 1.0f;

                          bool const get = get_filename;
                          if (true == get)
                          {
                              assert(NULL == item);
                              item = new QueuedDecoderImage(color_black);
                              assert(NULL != item);
                          }
                          /* if */
                      }
                      /* if */

                      if (NULL != item)
                      {
                          item->pattern_type = pattern_type;
                          item->index = index;
                          item->projectorID = ProjectorID;
                          item->retry = retry;
                          SAFE_DELETE(item->pFilename);
                          item->pFilename = new std::wstring(filename);
                          item->delay = delay;
                          item->exposure = exposure;
                          item->fSkipAcquisition = skip_acquisition;

                          bool const inserted = ImageDecoderQueueImage(this->pAcquisition->pImageDecoder, item);
                          if (false == inserted) SAFE_DELETE(item);
                      }
                      /* if (NULL != item) */
                  }
                  /* if */
              }
              /* if */
          }
     
        // Acquisition succeeded. Nothing to do!
    }
  /* if */

  // Queue last successfully acquired frame into image encoder queue.
  if ((true == acquired) && (NULL != this->pAcquisition->pImageEncoder) && (NULL != pPylon))
    {
      QueuedEncoderImage* item = new QueuedEncoderImage();
      assert(NULL != item);
      if (NULL != item)
        {
          // TODO: Update the following code which copies the image into the processing queue.
          //bool const copy_metadata = item->CopyMetadataFrom(&sData);
          //assert(true == copy_metadata);

          //BOOL const copy_image = item->CopyImageFrom(pSapera->pBuffer, pSapera->pCamera);
          //assert(TRUE == copy_image);

          //bool const queue = ImageEncoderQueueImage(this->pAcquisition->pImageEncoder, item);
          //assert(true == queue);
          //if (true != queue) SAFE_DELETE(item);
        }
      /* if */
    }
  else
    {
      ImageMetadataRelease(&sData);
    }
  /* if */

  // Signal the batch acquisition has ended.
  if ((NULL != this->pAcquisition->pSynchronization) &&
      (NULL != this->pAcquisition->pImageDecoder) && (NULL != this->pAcquisition->pImageDecoder->pImageList) && (false == this->pAcquisition->pImageDecoder->pImageList->cycle) &&
      (true == pop) && (true == sData.fLast)
      )
    {
      assert(false == DebugIsSignalled(this->pAcquisition->pSynchronization, MAIN_END_CAMERA, this->pAcquisition->CameraID));

      BOOL const set_end = this->pAcquisition->pSynchronization->EventSet(MAIN_END_CAMERA, this->pAcquisition->CameraID);
      assert(0 != set_end);
    }
  /* if */

  // Display frames.
  if ((true == this->pAcquisition->fView) && (NULL != this->pAcquisition->pView))
    {
      // TODO: Update to push the image data into the preview window.
      //PushImage(this->pAcquisition->pView, this->pAcquisition->CameraID, pSapera->pBuffer, pSapera->pCamera);
    }
  /* if */
}
/* CCustomImageEventHandler::OnImageGrabbed */



//! 
/*!
  <long-description>

  \param camera 
  \param countOfSkippedImages   
*/
void
CCustomImageEventHandler::OnImagesSkipped(
                                          CInstantCamera & camera,
                                          size_t countOfSkippedImages
                                          )
{
  if (NULL == this->pAcquisition) return;
  if (NULL == this->pAcquisition->pWindow) return;

  bool const fBlocking = this->pAcquisition->pWindow->fBlocking; // True if acquisition is blocking.
  bool const fFixed = this->pAcquisition->pWindow->fFixed; // True if fixed SL pattern is used.
  bool const fConcurrentDelay = this->pAcquisition->pWindow->fConcurrentDelay; // True if delay is larger than exposure.
  
  int const CameraID = this->pAcquisition->CameraID;
      
  Debugfprintf(stderr, gDbgCameraFrameSkipped, CameraID, __FILE__, __LINE__);
  
  if (true == fBlocking)
    {
      if (true == this->pAcquisition->fThrottleDown)
        {
          this->pAcquisition->timeout += 50; // Increase timeout in 50ms steps.
          Debugfprintf(stderr, gDbgCameraIncreaseTimeout, CameraID, this->pAcquisition->timeout, __FILE__, __LINE__);
        }
      /* if */
      this->pAcquisition->fThrottleDown = true;
    }
  /* if */
}
/* CCustomImageEventHandler::OnImagesSkipped */
  

#endif /* HAVE_PYLON_SDK */


#endif /* !__BATCHACQUISITIONPYLONCALLBACKS_CPP */
