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
  \file   BatchAcquisitionFlyCapture2Registers.cpp
  \brief  Functions to poll registers status.

  All PointGrey cameras expose unified set of registers thorugh which camera
  features and status may be checked. Not all registers are used by all cameras
  so functions defined here should be checked agains specific model reference
  manual.

  Note that bit at position 0 is always the most significant bit of the register value;
  e.g. mask for reading bit 0 is 2^31 = 2147483648 = 0x80000000.

  \author Tomislav Petkovic
  \date   2017-01-27
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONFLYCAPTURE2REGISTERS_CPP
#define __BATCHACQUISITIONFLYCAPTURE2REGISTERS_CPP


#include "BatchAcquisitionFlyCapture2Registers.h"



#ifdef HAVE_FLYCAPTURE2_SDK

/****** HELPER FUNCTIONS ******/


//! Reads register value.
/*!
  Function reads specified bit from register at specified address.

  \param pCam   Pointer to camera class.
  \param address        Register address.
  \param mask   Bit mask.
  \param value_out      Pointer to where read value should be stored. If value_out is zero
  then specified bit was not set; if value is equal to mask then specified bit was set.
  \return Returns true if read operation succedded, false otherwise.
*/
inline
bool
ReadBitValue_inline(
                    FlyCapture2::Camera * const pCam,
                    unsigned int const address,
                    unsigned int const mask,
                    unsigned int * const value_out
                    )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  assert(NULL != value_out);
  if (NULL == value_out) return false;

  FlyCapture2::Error error;
  unsigned int value = 0;

  error = pCam->ReadRegister( address, &value );
  assert(error == FlyCapture2::PGRERROR_OK);

  if (error != FlyCapture2::PGRERROR_OK) return false;

  *value_out = value & mask;

  return true;
}
/* ReadBitValue_inline */



/****** EXPORTED FUNCTIONS ******/


//! Checks if software trigger is available.
/*!
  Checks if software trigger is available.

  To check for availability function reads out bit 15 of the TRIGGER_INQ
  camera register. If the bit is zero software trigger is not available.

  \param pCam   Pointer to camera.
  \return Returns true if software trigger is available.
*/
bool
IsSoftwareTriggerAvailable(
                           FlyCapture2::Camera * const pCam
                           )
{
  unsigned int const address = 0x530; // TRIGGER_INQ register address
  unsigned int const mask = 0x00010000; // Software_Trigger_Inq field mask

  unsigned int value = 0;
  bool const read = ReadBitValue_inline(pCam, address, mask, &value);
  if (false == read) return read;

  return mask == value;
}
/* IsSoftwareTriggerAvailable */



//! Checks if trigger mode 14 is available.
/*!
  Checks if trigger mode 14 is available.

  \param pCam   Pointer to camera.
  \return Returns true if software trigger is available.
*/
bool
IsMode14Available(
                  FlyCapture2::Camera * const pCam
                  )
{
  unsigned int const address = 0x530; // TRIGGER_INQ register address
  unsigned int const mask = 0x00000002; // Trigger_Mode14_Inq field mask

  unsigned int value = 0;
  bool const read = ReadBitValue_inline(pCam, address, mask, &value);
  if (false == read) return read;

  return mask == value;
}
/* IsMode14Available */



//! Checks if trigger mode 15 is available.
/*!
  Checks if trigger mode 15 is available.

  \param pCam   Pointer to camera.
  \return Returns true if software trigger is available.
*/
bool
IsMode15Available(
                  FlyCapture2::Camera * const pCam
                  )
{
  unsigned int const address = 0x530; // TRIGGER_INQ register address
  unsigned int const mask = 0x00000001; // Trigger_Mode15_Inq field mask

  unsigned int value = 0;
  bool const read = ReadBitValue_inline(pCam, address, mask, &value);
  if (false == read) return read;

  return mask == value;
}
/* IsMode15Available */



//! Checks if camera is ready for triggering.
/*!
  Checks if camera is ready for triggering.

  To check this function reads out bit 0 of the SOFTWARE_TRIGGER register.
  Value 0 indicates camera is not ready while value 1 indicates camera is ready.

  \param pCam   Pointer to camera.
  \return Returns true if camera is ready for triggering.
*/
bool
CheckTriggerReady(
                  FlyCapture2::Camera * const pCam
                  )
{
  unsigned int const address = 0x62C; // SOFTWARE_TRIGGER register address
  unsigned int const mask = 0x80000000; // Software_Trigger field mask

  unsigned int value = 0;
  bool const read = ReadBitValue_inline(pCam, address, mask, &value);
  if (false == read) return read;

  bool const ready = ( 0 == (value >> 31) );

  return ready;
}
/* CheckTriggerReady */



//! Wait for trigger ready.
/*!
  Function waits for trigger to become ready.
  Trigger status is pooled once each ms until alloted time elapses.
  Function will return true as soon as trigger is ready.

  \param pCam   Pointer to camera.
  \param wait_time_QPC   Allowed time for waiting in QPC ticks. If negative then function will wait forever.
  \return Returns true if camera is ready and false otherwise.
*/
bool
WaitForTriggerReady(
                    FlyCapture2::Camera * const pCam,
                    __int64 const wait_time_QPC
                    )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  unsigned int const address = 0x62C; // SOFTWARE_TRIGGER register address
  unsigned int const mask = 0x80000000; // Software_Trigger field mask

  LARGE_INTEGER start, current;
  start.QuadPart = 0;
  current.QuadPart = 0;

  BOOL const res_start = QueryPerformanceCounter( &start );
  assert(TRUE == res_start);

  __int64 const stop = (0 <= wait_time_QPC)? start.QuadPart + wait_time_QPC : LLONG_MAX;
  
  bool ready = false;
  do
    {
      unsigned int value = 0;
      bool const read = ReadBitValue_inline(pCam, address, mask, &value);
      if (false == read) return false;

      ready = ( 0 == (value >> 31) );

      if (true == ready) return true;

      SleepEx(1, TRUE);

      BOOL const res_current = QueryPerformanceCounter( &current );
      assert(TRUE == res_current);
    }
  while (stop > current.QuadPart);

  return ready;
}
/* WaitForTriggerReady */



//! Wait for trigger ready.
/*!
  Function waits for trigger to become ready.
  Trigger status is pooled once each ms until alloted time elapses.
  Function will return true as soon as trigger is ready.

  \param pCam   Pointer to camera.
  \param wait_time_ms   Allowed time for waiting in ms (milliseconds). If negative then function will wait forever.
  \return Returns true if camera is ready and false otherwise.
*/
bool
WaitForTriggerReady(
                    FlyCapture2::Camera * const pCam,
                    double const wait_time_ms
                    )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  LARGE_INTEGER frequency;
  frequency.QuadPart = 0;

  BOOL const res_frequency = QueryPerformanceFrequency( &frequency );
  assert(TRUE == res_frequency);

  double const ms_to_ticks = (double)(frequency.QuadPart) * 0.001; // Conversion factor from ms to ticks.
  __int64 const wait_time_QPC = (0.0 <= wait_time_ms)? (__int64)(wait_time_ms * ms_to_ticks + 0.5) : -1;
  
  bool const ready = WaitForTriggerReady(pCam, wait_time_QPC);

  return ready;
}
/* WaitForTriggerReady */



//! Wait for trigger not-ready.
/*!
  Function waits for trigger to become not-ready.
  Trigger status is pooled once each ms until alloted time elapses.
  Function will return true as soon as trigger is not-ready.

  \param pCam   Pointer to camera.
  \param wait_time_QPC   Allowed time for waiting in QPC ticks. If negative then function will wait forever.
  \return Returns true if camera is ready and false otherwise.
*/
bool
WaitForTriggerNotReady(
                       FlyCapture2::Camera * const pCam,
                       __int64 const wait_time_QPC
                       )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  unsigned int const address = 0x62C; // SOFTWARE_TRIGGER register address
  unsigned int const mask = 0x80000000; // Software_Trigger field mask

  LARGE_INTEGER start, current;
  start.QuadPart = 0;
  current.QuadPart = 0;
  
  BOOL const res_start = QueryPerformanceCounter( &start );
  assert(TRUE == res_start);

  LONGLONG const stop = (0 <= wait_time_QPC)? start.QuadPart + wait_time_QPC : LLONG_MAX;
  
  bool not_ready = false;
  do
    {
      unsigned int value = 0;
      bool const read = ReadBitValue_inline(pCam, address, mask, &value);
      if (false == read) return false;

      not_ready = ( 0 != (value >> 31) );

      if (true == not_ready) return true;

      SleepEx(1, TRUE);

      BOOL const res_current = QueryPerformanceCounter( &current );
      assert(TRUE == res_current);
    }
  while (stop > current.QuadPart);

  return not_ready;
}
/* WaitForTriggerNotReady */



//! Wait for trigger not-ready.
/*!
  Function waits for trigger to become not-ready.
  Trigger status is pooled once each ms until alloted time elapses.
  Function will return true as soon as trigger is not-ready.

  \param pCam   Pointer to camera.
  \param wait_time_ms   Allowed time for waiting. If negative then function will wait forever.
  \return Returns true if camera is ready and false otherwise.
*/
bool
WaitForTriggerNotReady(
                       FlyCapture2::Camera * const pCam,
                       double const wait_time_ms
                       )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  LARGE_INTEGER frequency;
  frequency.QuadPart = 0;

  BOOL const res_frequency = QueryPerformanceFrequency( &frequency );
  assert(TRUE == res_frequency);

  double const ms_to_ticks = (double)(frequency.QuadPart) * 0.001; // Conversion factor from ms to ticks.
  __int64 const wait_time_QPC = (0.0 <= wait_time_ms)? (__int64)(wait_time_ms * ms_to_ticks + 0.5) : -1;
 
  bool const not_ready = WaitForTriggerNotReady(pCam, wait_time_QPC);

  return not_ready;
}
/* WaitForTriggerNotReady */



//! Polls camera until it is ready for triggering.
/*!
  Polls camera until it is ready for triggering.

  \param pCam   Pointer to camera.
  \param max_wait_time Minimum time for the camera to become ready in ms (milliseconds).
  If negative then function will wait forever. If non-negative the function will not return before the specified time elapses.
  If zero the camera will be queried once.
  \return Returns true if camera is ready for triggering.
*/
bool
PollForTriggerReadyX(
                    FlyCapture2::Camera * const pCam,
                    double const max_wait_time
                    )
{
  unsigned int const address = 0x62C; // SOFTWARE_TRIGGER register address
  unsigned int const mask = 0x80000000; // Software_Trigger field mask

  LARGE_INTEGER start, stop, frequency;
  start.QuadPart = 0;
  stop.QuadPart = 0;
  frequency.QuadPart = 0;
  double inv_frequency = 0.0;

  bool ready = false;
  do
    {
      unsigned int value = 0;
      bool const read = ReadBitValue_inline(pCam, address, mask, &value);
      if (false == read) return read;

      ready = ( 0 == (value >> 31) );

      if ( (!ready) && (0 <= max_wait_time) )
        {
          if (0 == start.QuadPart)
            {
              BOOL const res_start = QueryPerformanceCounter( &start );
              assert(TRUE == res_start);
            }
          /* if */

          if (0 == frequency.QuadPart)
            {
              BOOL const res_frequency = QueryPerformanceFrequency( &frequency );
              assert(TRUE == res_frequency);

              inv_frequency = 1000.0 / (double)(frequency.QuadPart); // Conversion factor from ticks to ms.
            }
          /* if */

          BOOL const res_stop = QueryPerformanceCounter( &stop );
          assert(TRUE == res_stop);

          double const elapsed = (double)(stop.QuadPart - start.QuadPart) * inv_frequency;
          if (elapsed > max_wait_time)
            {
              return ready;
            }
          /* if */
        }
      /* if */
    }
  while ( !ready );

  return ready;
}
/* PollForTriggerReady */



//! Fire software trigger.
/*!
  Function fires software trigger.

  \param pCam   Pointer to camera class.
  \return Returns true if successfull, false otherwise.
*/
bool
FireSoftwareTrigger(
                    FlyCapture2::Camera * const pCam
                    )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  unsigned int const address = 0x62C; // SOFTWARE_TRIGGER register address
  unsigned int const mask = 0x80000000; // Software_Trigger field mask

  FlyCapture2::Error error;

  error = pCam->WriteRegister( address, mask );
  if (error != FlyCapture2::PGRERROR_OK) return false;

  return true;
}
/* FireSoftwareTrigger */



//! Powers on camera.
/*!
  Requests the camera to turn on.

  \param pCam   Pointer to camera.
  \return Returns true if camera has powered on, false otherwise.
*/
bool
PowerOnCamera(
              FlyCapture2::Camera * const pCam
              )
{
  assert(NULL != pCam);
  if (NULL == pCam) return false;

  unsigned int const address = 0x610; // CAMERA_POWER register address
  unsigned int const mask = 0x80000000; // Cam_Pwr_Ctrl field mask

  FlyCapture2::Error error;

  error = pCam->WriteRegister( address, mask );
  if (error != FlyCapture2::PGRERROR_OK) return false;

  unsigned int const millisecondsToSleep = 100;

  unsigned int value = 0;
  unsigned int retries = 10;
  bool awake = false;

  // Wait for camera to complete power-up.
  do
    {
      SleepEx(millisecondsToSleep, TRUE);

      error = pCam->ReadRegister(address, &value);
      if (error == FlyCapture2::PGRERROR_TIMEOUT)
        {
          // Ignore timeout errors during power-up.
        }
      else if (error != FlyCapture2::PGRERROR_OK)
        {
          return false;
        }
      /* if */

      awake = ((mask & value) == mask);
      --retries;

    }
  while ( !awake && (0 < retries) );

  return awake;
}
/* PowerOnCamera */



//! Get endianess.
/*!
  Get endianess of Y16 data. If endianess cannot be read a default value of true is returned.
  Note that the register that controls the endianess depends on the IIDC version.

  \param pCam   Pointer to camera.
  \return Returns true if Y16 mode is big endian.
*/
bool
IsY16DataBigEndian(
                   FlyCapture2::Camera * const pCam
                   )
{
  bool is_big_endian = true; // Big endian mode is default on camera initialization.

  assert(NULL != pCam);
  if (NULL == pCam) return is_big_endian;

  FlyCapture2::Error error;
  FlyCapture2::CameraInfo camInfo;

  error = pCam->GetCameraInfo(&camInfo);
  assert(error == FlyCapture2::PGRERROR_OK);
  if (error != FlyCapture2::PGRERROR_OK) return is_big_endian;

  if (camInfo.iidcVer >= 132)
    {
      unsigned int const address = 0x630; // DATA_DEPTH register address
      unsigned int const mask = 0x00800000; // Little_Endian field mask
      unsigned int value = 0;

      error = pCam->ReadRegister( address, &value );
      assert(error == FlyCapture2::PGRERROR_OK);
      if (error != FlyCapture2::PGRERROR_OK) return is_big_endian;

      is_big_endian = ( 0 == (value & mask) );
    }
  else
    {
      unsigned int const address = 0x1048; // IMAGE_DATA_FORMAT
      unsigned int const mask = 0x00000001;

      unsigned int value = 0;

      error = pCam->ReadRegister( address, &value );
      assert(error == FlyCapture2::PGRERROR_OK);
      if (error != FlyCapture2::PGRERROR_OK) return is_big_endian;

      is_big_endian = ( 0 != (value & mask) );
    }
  /* if */

  return is_big_endian;
}
/* IsY16DataBigEndian */



//! Returns trigger delay value.
/*!
  Returns value of trigger delay.

  \param pCam   Pointer to camera.
  \return Returns value of trigger delay register or -1 if unsuccessfull.
*/
int
GetTriggerDelayRegisterValue(
                             FlyCapture2::Camera * const pCam
                             )
{
  int delay = -1; // Negative value indicates error.

  unsigned int const address = 0x834; // TRIGGER_DELAY register address
  unsigned int const mask = 0x00000FFF; // Value field mask

  unsigned int value = 0;
  bool const read = ReadBitValue_inline(pCam, address, mask, &value);
  if (true == read) delay = value;

  return delay;
}
/* GetTriggerDelayRegisterValue */


#endif /* HAVE_FLYCAPTURE2_SDK */



#endif /* !__BATCHACQUISITIONFLYCAPTURE2REGISTERS_CPP */
