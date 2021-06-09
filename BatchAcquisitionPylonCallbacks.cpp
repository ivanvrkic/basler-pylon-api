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
  \date   2021-06-09
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
  Debugfwprintf(stderr, gDbgEventOnAttach, camera.GetDeviceInfo().GetModelName().c_str());
}
/* CCustomConfigurationEventHandler::OnAttached */



void CCustomConfigurationEventHandler::OnOpen(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnOpen event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnOpened(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnOpened event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnGrabStart(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnGrabStart event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnGrabStarted(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnGrabStarted event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnGrabStop(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnGrabStop event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnGrabStopped(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnGrabStopped event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnClose(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnClose event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnClosed(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnClosed event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnDestroy(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnDestroy event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnDestroyed(CInstantCamera& /*camera*/)
{
  Debugfprintf(stderr, "OnDestroyed event");
}

void CCustomConfigurationEventHandler::OnDetach(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnDetach event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnDetached(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnDetached event for device %s",camera.GetDeviceInfo().GetModelName());
}

void CCustomConfigurationEventHandler::OnGrabError(CInstantCamera& camera, const char* errorMessage)
{
  Debugfprintf(stderr, "OnGrabError event for device %s", camera.GetDeviceInfo().GetModelName());
  Debugfprintf(stderr, "Error Message: ", errorMessage);
}

void CCustomConfigurationEventHandler::OnCameraDeviceRemoved(CInstantCamera& camera)
{
  Debugfprintf(stderr, "OnCameraDeviceRemoved event for device %s", camera.GetDeviceInfo().GetModelName());
}



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

          //bool const queue = ImageEncoderQueueImage(P->pImageEncoder, item);
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
      //PushImage(P->pView, P->CameraID, pSapera->pBuffer, pSapera->pCamera);
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
