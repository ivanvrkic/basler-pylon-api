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
  \file   BatchAcquisitionFlyCapture2Registers.h
  \brief  Functions to poll registers status.

  Header file for register polling functions.

  \author Tomislav Petkovic
  \date   2017-01-27
*/


#ifndef __BATCHACQUISITIONFLYCAPTURE2REGISTERS_H
#define __BATCHACQUISITIONFLYCAPTURE2REGISTERS_H



#ifdef HAVE_FLYCAPTURE2_SDK


//! Checks if software trigger is available.
bool IsSoftwareTriggerAvailable(FlyCapture2::Camera * const);

//! Checks if trigger mode 14 is available.
bool IsMode14Available(FlyCapture2::Camera * const);

//! Checks if trigger mode 15 is available.
bool IsMode15Available(FlyCapture2::Camera * const);

//! Checks if camera is ready for triggering.
bool CheckTriggerReady(FlyCapture2::Camera * const);

//! Wait for trigger ready.
bool WaitForTriggerReady(FlyCapture2::Camera * const, __int64 const);

//! Wait for trigger ready.
bool WaitForTriggerReady(FlyCapture2::Camera * const, double const);

//! Wait for trigger not-ready.
bool WaitForTriggerNotReady(FlyCapture2::Camera * const, __int64 const);

//! Wait for trigger not-ready.
bool WaitForTriggerNotReady(FlyCapture2::Camera * const, double const);

//! Fire software trigger.
bool FireSoftwareTrigger(FlyCapture2::Camera * const);

//! Powers on camera.
bool PowerOnCamera(FlyCapture2::Camera * const);

//! Get endianess.
bool IsY16DataBigEndian(FlyCapture2::Camera * const);

//! Get trigger delay value.
int GetTriggerDelayRegisterValue(FlyCapture2::Camera * const);


#endif /* HAVE_FLYCAPTURE2_SDK */



#endif /* __BATCHACQUISITIONFLYCAPTURE2REGISTERS_H */
