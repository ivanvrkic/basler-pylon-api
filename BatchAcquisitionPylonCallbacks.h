/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014 UniZG, Zagreb. All rights reserved.
 * (c) 2014 FER, Zagreb. All rights reserved.
 */

 /*!
   \file   BatchAcquisitionPylonCallbacks.h
   \brief  Callback functions.

   \author Ivan Vrkic
   \date   2014-05-20
 */


#ifndef __BATCHACQUISITIONPYLONCALLBACKS_H
#define __BATCHACQUISITIONPYLONCALLBACKS_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionWindowDisplay.h"
 // Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>




#ifndef STRING_LENGTH
#define STRING_LENGTH 256
#endif /* !STRING_LENGTH */


#ifdef HAVE_PYLON_SDK

// Namespace for using pylon objects.


// Namespace for using pylon universal instant camera parameters.
using namespace Basler_UniversalCameraParams;

// Namespace for using cout.
using namespace std;


namespace Pylon {
    class CInstantCamera;

    class CCustomConfigurationEventHandler : public CConfigurationEventHandler
    {
    public:
        void OnAttach(CInstantCamera& /*camera*/);
        void OnAttached(CInstantCamera& camera);
        void OnOpen(CInstantCamera& camera);
        void OnOpened(CInstantCamera& camera);
        void OnGrabStart(CInstantCamera& camera);
        void OnGrabStarted(CInstantCamera& camera);
        void OnGrabStop(CInstantCamera& camera);
        void OnGrabStopped(CInstantCamera& camera);
        void OnClose(CInstantCamera& camera);
        void OnClosed(CInstantCamera& camera);
        void OnDestroy(CInstantCamera& camera);
        void OnDestroyed(CInstantCamera& /*camera*/);
        void OnDetach(CInstantCamera& camera);
        void OnDetached(CInstantCamera& camera);
        void OnGrabError(CInstantCamera& camera, const char* errorMessage);
        void OnCameraDeviceRemoved(CInstantCamera& camera);
    };

    //Example of an image event handler.
    class CCustomImageEventHandler : public CImageEventHandler
    {
    private:
        DisplayWindowParameters* pWindow; //!< Display window.
        int timeout; //!< Timeout in ms.
        int CameraID;
        volatile bool fThrottleDown; //!< Flag to indicate we must slow down the acquisition.
    public:
        virtual void OnImageGrabbed(CInstantCamera& /*camera*/, const CGrabResultPtr& ptrGrabResult);
        virtual void OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages);
    };

    
    // Example handler for camera events.
    class CCustomCameraEventHandler : public CBaslerUniversalCameraEventHandler
    {
    private:
        bool fExposureInProgress;
        SynchronizationEvents * pSynchronization;
        int CameraID;

    public:
        CCustomCameraEventHandler(): CBaslerUniversalCameraEventHandler() {}

        // Only very short processing tasks should be performed by this method. Otherwise, the event notification will block the
        // processing of images.
        virtual void OnCameraEvent(CBaslerUniversalInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */);
    };
}


#endif /* HAVE_PYLON_SDK */s


#endif /* !__BATCHACQUISITIONPYLONCALLBACKS_H */