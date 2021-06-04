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
   \file   BatchAcquisitionSaperaCallbacks.cpp
   \brief  Callback functions for Teledyne Dalsa Sapera SDK.

   \author Ivan Vrkic
   \date   2021-05-20
 */


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPYLONCALLBACKS_CPP
#define __BATCHACQUISITIONPYLONCALLBACKS_CPP

#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionPylonCallbacks.h"
#include <iostream>

#ifdef HAVE_PYLON_SDK

namespace Pylon
{
    class CInstantCamera;
    void CCustomConfigurationEventHandler::OnAttach(CInstantCamera& /*camera*/)
    {
        Debugfprintf(stderr, "OnAttach event");
    }

    void CCustomConfigurationEventHandler::OnAttached(CInstantCamera& camera)
    {
        Debugfprintf(stderr, "OnAttached event for device %s",camera.GetDeviceInfo().GetModelName());
    }

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

    // Namespace for using pylon universal instant camera parameters.
    using namespace Basler_UniversalCameraParams;
    // Namespace for using cout.
    using namespace std;

    enum MyEvents
    {
        ExposureEndEventId = 100,
        EventOverrunEventId = 200,
        FrameStartEventId = 300,
        FrameTriggerMissedEventId = 400,
        FrameStartWaitEventId=500
        // More events can be added here.
    };

    // Example handler for camera events.
    void CCustomCameraEventHandler::OnCameraEvent(CBaslerUniversalInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
        {
            switch (userProvidedId)
            {
            case ExposureEndEventId: // Exposure End event
                assert(true == this->fExposureInProgress);
                this->fExposureInProgress = false;

                if (NULL != this->pSynchronization)
                {
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_READY, this->CameraID));
                    assert(true == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_BEGIN, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_END, this->CameraID));

                    BOOL const set_exposure_end = (this->pSynchronization)->EventSet(CAMERA_EXPOSURE_END, this->CameraID);
                    assert(0 != set_exposure_end);
                }
                break;
            case FrameStartEventId: // Exposure End event
                assert(false == this->fExposureInProgress);
                this->fExposureInProgress = true;

                if (NULL != this->pSynchronization)
                {
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_READY, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_BEGIN, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_END, this->CameraID));

                    BOOL const set_exposure_begin = this->pSynchronization->EventSet(CAMERA_EXPOSURE_BEGIN, this->CameraID);
                    assert(0 != set_exposure_begin);
                }
                break;
            case FrameTriggerMissedEventId:
                assert(false == this->fExposureInProgress);

                if (NULL != this->pSynchronization)
                {
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_READY, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_BEGIN, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_EXPOSURE_END, this->CameraID));
                    assert(false == DebugIsSignalled(this->pSynchronization, CAMERA_INVALID_TRIGGER, this->CameraID));

                    BOOL const set_invalid_trigger = this->pSynchronization->EventSet(CAMERA_INVALID_TRIGGER, CameraID);
                    assert(0 != set_invalid_trigger);
                }
                break;
            case FrameStartWaitEventId:

                break;
            case EventOverrunEventId:  // Event Overrun event
                Debugfprintf(stderr, "Event Overrun event. FrameID: ", camera.EventOverrunEventFrameID.GetValue());
                break;
            }
        }


    //Example of an image event handler.
    void CCustomImageEventHandler::OnImageGrabbed(CInstantCamera& /*camera*/, const CGrabResultPtr& ptrGrabResult)
        {
        Debugfprintf(stderr,"CSampleImageEventHandler::OnImageGrabbed called.");

    #ifdef PYLON_WIN_BUILD


    #endif
        }
    void CCustomImageEventHandler::OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages) {
    #ifdef PYLON_WIN_BUILD
        if ( NULL != this->pWindow)
        {
            bool const fBlocking = this->pWindow->fBlocking; // True if acquisition is blocking.
            bool const fFixed = this->pWindow->fFixed; // True if fixed SL pattern is used.
            bool const fConcurrentDelay = this->pWindow->fConcurrentDelay; // True if delay is larger than exposure.
            Debugfprintf(stderr, "[CAM %d] Frame skipped in %s:%d.\n", this->CameraID, __FILE__, __LINE__);

            if (true == fBlocking)
            {
                if (true == this->fThrottleDown)
                {
                    this->timeout += 50; // Increase timeout in 50ms steps.
                    Debugfprintf(stderr, "[CAM %d] Increasing transfer timeout to %d ms in %s:%d.\n" , this->CameraID, this->timeout, __FILE__, __LINE__);
                }
                /* if */
                this->fThrottleDown = true;
            }
            /* if */
        }
    #endif
        }
}

#endif /* HAVE_PYLON_SDK */

#endif /* !__BATCHACQUISITIONPYLONCALLBACKS_CPP */
