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
  \file   BatchAcquisitionPylonCallbacks.h
  \brief  Callback functions.

  \author Ivan Vrkic
  \autor  Tomislav Petkovic
  \date   2021-06-09
*/


#ifndef __BATCHACQUISITIONPYLONCALLBACKS_H
#define __BATCHACQUISITIONPYLONCALLBACKS_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionWindowDisplay.h"


struct AcquisitionParameters_;


//! Enumeration of handled events.
/*!
  Each of handled events must have an unique ID assigned.
  We let the compiler assign numbers to names of the events.
*/
typedef
enum CustomPylonEvents_
  {
    ExposureEndEventID, /*!< */
    EventOverrunEventID, /*!< */
    FrameStartEventID, /*!< */
    FrameTriggerMissedEventID, /*!< */
    FrameStartWaitEventID /*!< */
  } CustomPylonEvents;



#ifdef HAVE_PYLON_SDK


// Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>


//! Configuration event handler class.
/*!
  Derived class to hold all configuration event handlers.
*/
class CCustomConfigurationEventHandler : public Pylon::CConfigurationEventHandler
{
  
public:
  
  void OnAttach(Pylon::CInstantCamera &);
  void OnAttached(Pylon::CInstantCamera &);
  void OnOpen(Pylon::CInstantCamera &);
  void OnOpened(Pylon::CInstantCamera &);
  void OnGrabStart(Pylon::CInstantCamera &);
  void OnGrabStarted(Pylon::CInstantCamera &);
  void OnGrabStop(Pylon::CInstantCamera &);
  void OnGrabStopped(Pylon::CInstantCamera &);
  void OnClose(Pylon::CInstantCamera &);
  void OnClosed(Pylon::CInstantCamera &);
  void OnDestroy(Pylon::CInstantCamera &);
  void OnDestroyed(Pylon::CInstantCamera &);
  void OnDetach(Pylon::CInstantCamera &);
  void OnDetached(Pylon::CInstantCamera &);
  void OnGrabError(Pylon::CInstantCamera &, char const *);
  void OnCameraDeviceRemoved(Pylon::CInstantCamera &);
  
};



//! Image event handler class.
/*!
  Derived class to hold all image event handlers.
*/
class CCustomImageEventHandler : public Pylon::CImageEventHandler
{
  
private:
  
  AcquisitionParameters_ * pAcquisition = NULL; //!< Pointer to the acquisition parameters structure.

public:

  //! Constructor.
  /*!
    Constructs the event handler class.
    
    \param ptr Pointer to the structure which holds the state of the acquisition thread.
  */  
  CCustomImageEventHandler(
                           AcquisitionParameters_ * const ptr
                           )
    : Pylon::CImageEventHandler()
  {
    assert(NULL != ptr);
    this->pAcquisition = ptr;
  };
  /* CCustomImageEventHandler */

  
  virtual void OnImageGrabbed(Pylon::CInstantCamera &, const Pylon::CGrabResultPtr &);
  virtual void OnImagesSkipped(Pylon::CInstantCamera &, size_t);
  
};



//! Universal event handler class.
/*!
  Derived class to hold all image event handlers.
*/
class CCustomCameraEventHandler : public Pylon::CBaslerUniversalCameraEventHandler
{
  
private:
  
  AcquisitionParameters_ * pAcquisition = NULL; //!< Pointer to acquisition parameters structure.

public:

  //! Constructor.
  /*!
    Constructs the event handler class.

    \param ptr	Pointer to the structure which holds the state of the acquisition thread.
  */
  CCustomCameraEventHandler(
                            AcquisitionParameters_ * const ptr
                            )
    : Pylon::CBaslerUniversalCameraEventHandler()
  {
    assert(NULL != ptr);
    this->pAcquisition = ptr;
  };
  /* CCustomCameraEventHandler */

  
  virtual void OnCameraEvent(Pylon::CBaslerUniversalInstantCamera &, intptr_t, GenApi::INode *);
  
};


#endif /* HAVE_PYLON_SDK */


#endif /* !__BATCHACQUISITIONPYLONCALLBACKS_H */
