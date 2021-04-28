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
  \file   BatchAcquisitionSaperaCallbacks.h
  \brief  Callback functions.

  \author Tomislav Petkovic
  \date   2014-12-13
*/


#ifndef __BATCHACQUISITIONSAPERACALLBACKS_H
#define __BATCHACQUISITIONSAPERACALLBACKS_H


#include "BatchAcquisition.h"


#ifndef STRING_LENGTH
#define STRING_LENGTH 256
#endif /* !STRING_LENGTH */


#ifdef HAVE_SAPERA_SDK

//! Register callback.
BOOL RegisterCallback(SapAcqDevice * const, char const * const, SapAcqDeviceCallback, void * const);

//! Unregister all callbacks.
BOOL UnregisterAllCallbacks(SapAcqDevice * const);

//! Exposure begin callback.
void CameraCallbackExposureBegin(SapAcqDeviceCallbackInfo *);

//! Exposure end callback.
void CameraCallbackExposureEnd(SapAcqDeviceCallbackInfo *);

//! Readout begin callback.
void CameraCallbackReadoutBegin(SapAcqDeviceCallbackInfo *);

//! Readout end callback
void CameraCallbackReadoutEnd(SapAcqDeviceCallbackInfo *);

//! Acquisition begin callback.
void CameraCallbackAcquisitionBegin(SapAcqDeviceCallbackInfo *);

//! Acquisition end callback.
void CameraCallbackAcquisitionEnd(SapAcqDeviceCallbackInfo *);

//! Invalid frame trigger callback.
void CameraCallbackInvalidFrameTrigger(SapAcqDeviceCallbackInfo *);

//! Frame skipped callback.
void CameraCallbackFrameSkipped(SapAcqDeviceCallbackInfo *);

#endif /* HAVE_SAPERA_SDK */


#endif /* !__BATCHACQUISITIONSAPERACALLBACKS_H */