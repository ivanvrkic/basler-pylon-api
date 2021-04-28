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
  \file   BatchAcquisitionSapera.cpp
  \brief  Functions for Teledyne-Dalsa SaperaLT SDK.

  Functions and wrappers for Teledyne-Dalsa SaperaLS SDK.

  \author Tomislav Petkovic
  \date   2017-02-17
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONSAPERA_CPP
#define __BATCHACQUISITIONSAPERA_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionSapera.h"
#include "BatchAcquisitionSaperaCallbacks.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionKeyboard.h"



/****** HELPER FUNCTIONS ******/

//! Blank parameters structure.
/*!
  Blanks acquisition parameter structure for Teledyne Dalsa Sapera SDK.

  \param P      Pointer to parameters class.
*/
void
inline
AcquisitionParametersSaperaBlank_inline(
                                        AcquisitionParametersSapera * const P
                                        )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->selectedServer = -1;
  ZeroMemory( P->selectedServerName, sizeof(P->selectedServerName) );

  P->idxTriggerSoftware = -1;
  P->idxExposureAlignment = -1;

  P->pCamera = NULL;
  P->pFeature = NULL;
  P->pBuffer = NULL;
  P->pBayer = NULL;
  P->pTransfer = NULL;
}
/* AcquisitionParametersSaperaBlank_inline */



#ifdef HAVE_SAPERA_SDK

//! Names of servers for SaperaLT.
/*!
  SaperaLT SDK uses a concept of servers which supply image data. Servers may be
  an acquisition board (frame grabber) or a GigEVision camera. This array lists
  server names for SaperaLT SKD v8.20.
*/
static
char const * const SaperaLTServerTypeString[] = {
  /*  0 */ "None",
  /*  1 */ "System",
  /*  2 */ "Cobra",
  /*  3 */ "ViperRgb",
  /*  4 */ "ViperDigital",
  /*  5 */ "ViperQuad",
  /*  6 */ "ViperCamLink",
  /*  7 */ "BanditII",
  /*  8 */ "Bandit3MV",
  /*  9 */ "Bandit3CV",
  /* 10 */ "X64CL",
  /* 11 */ "X64LVDS",
  /* 12 */ "X64NS",
  /* 13 */ "X64Analog",
  /* 14 */ "X64ANQuad",
  /* 15 */ "X64AN2",
  /* 16 */ "X64ANLX1",
  /* 17 */ "X64CLiPRO",
  /* 18 */ "X64CLiPROe",
  /* 19 */ "X64CLExpress",
  /* 20 */ "X64CLGigE",
  /* 21 */ "X64CLLX4",
  /* 22 */ "X64CLPX4",
  /* 23 */ "X64CLVX4",
  /* 24 */ "X64LVDSPX4",
  /* 25 */ "X64LVDSVX4",
  /* 26 */ "X64XRICL",
  /* 27 */ "X64XRILVDS",
  /* 28 */ "PC2Vision",
  /* 29 */ "PC2Comp",
  /* 30 */ "PC2CamLink",
  /* 31 */ "Genie",
  /* 32 */ "Mamba",
  /* 33 */ "Anaconda",
  /* 34 */ "AnacondaCL",
  /* 35 */ "AnacondaLVDS",
  /* 36 */ "XriCL",
  /* 37 */ "XriLVDS"
};


//! Return description string.
/*!
  Returns pointer to description string for particular Sapera SDK server type.

  \param type   Server type.
  \return Pointer to a valid string or NULL pointer if server type is unknown.
*/
inline
char const * const
GetSaperaLTServerTypeString_inline(
                                   SapManager::ServerType const type
                                   )
{
  switch( type )
    {
    case SapManager::ServerSystem:       return SaperaLTServerTypeString[ 1];
    case SapManager::ServerCobra:        return SaperaLTServerTypeString[ 2];
    case SapManager::ServerViperRgb:     return SaperaLTServerTypeString[ 3];
    case SapManager::ServerViperDigital: return SaperaLTServerTypeString[ 4];
    case SapManager::ServerViperQuad:    return SaperaLTServerTypeString[ 5];
    case SapManager::ServerViperCamLink: return SaperaLTServerTypeString[ 6];
    case SapManager::ServerBanditII:     return SaperaLTServerTypeString[ 7];
    case SapManager::ServerBandit3MV:    return SaperaLTServerTypeString[ 8];
    case SapManager::ServerBandit3CV:    return SaperaLTServerTypeString[ 9];
    case SapManager::ServerX64CL:        return SaperaLTServerTypeString[10];
    case SapManager::ServerX64LVDS:      return SaperaLTServerTypeString[11];
    case SapManager::ServerX64NS:        return SaperaLTServerTypeString[12];
    case SapManager::ServerX64ANQuad:    return SaperaLTServerTypeString[14];
    case SapManager::ServerX64AN2:       return SaperaLTServerTypeString[15];
    case SapManager::ServerX64ANLX1:     return SaperaLTServerTypeString[16];
    case SapManager::ServerX64CLiPRO:    return SaperaLTServerTypeString[17];
    case SapManager::ServerX64CLExpress: return SaperaLTServerTypeString[19];
    case SapManager::ServerX64CLGigE:    return SaperaLTServerTypeString[20];
    case SapManager::ServerX64CLLX4:     return SaperaLTServerTypeString[21];
    case SapManager::ServerX64CLPX4:     return SaperaLTServerTypeString[22];
    case SapManager::ServerX64CLVX4:     return SaperaLTServerTypeString[23];
    case SapManager::ServerX64LVDSPX4:   return SaperaLTServerTypeString[24];
    case SapManager::ServerX64LVDSVX4:   return SaperaLTServerTypeString[25];
    case SapManager::ServerX64XRICL:     return SaperaLTServerTypeString[26];
    case SapManager::ServerX64XRILVDS:   return SaperaLTServerTypeString[27];
    case SapManager::ServerPC2Vision:    return SaperaLTServerTypeString[28];
    case SapManager::ServerPC2Comp:      return SaperaLTServerTypeString[29];
    case SapManager::ServerPC2CamLink:   return SaperaLTServerTypeString[30];
    case SapManager::ServerGenie:        return SaperaLTServerTypeString[31];
    case SapManager::ServerMamba:        return SaperaLTServerTypeString[32];
    case SapManager::ServerAnacondaCL:   return SaperaLTServerTypeString[34];
    case SapManager::ServerAnacondaLVDS: return SaperaLTServerTypeString[35];
    case SapManager::ServerXriCL:        return SaperaLTServerTypeString[36];
    case SapManager::ServerXriLVDS:      return SaperaLTServerTypeString[37];
    default:                             return NULL;
    }
  /* switch */
}
/* GetSaperaLTServerTypeString */



//! Prints GenICam string node.
/*!
  Function prints GenICam string node to the standard output.

  \param pCamera        Pointer to camera class associated with requested feature to print.
  \param pFeature       Pointer to feature class.
  \param pFeatureName   Name of the string feature to print.
  \param pFormatString  Format string.
  \return Returns true if successfull (node is printed), false otherwise (node does not exist or print operation failed).
*/
bool
inline
PrintGenICamStringNode_inline(
                              SapAcqDevice * const pCamera,
                              SapFeature * const pFeature,
                              char const * const pFeatureName,
                              char const * const pFormatString
                              )
{
  bool printed = false;

  assert(NULL != pCamera);
  if (NULL == pCamera) return printed;

  assert(NULL != pFeature);
  if (NULL == pFeature) return printed;

  assert(NULL != pFeatureName);
  if (NULL == pFeatureName) return printed;

  assert(NULL != pFormatString);
  if (NULL == pFormatString) return printed;

  BOOL isAvailable = FALSE;

  BOOL const checkAvailability = pCamera->IsFeatureAvailable(pFeatureName, &isAvailable);
  assert(TRUE ==checkAvailability);

  if ( (TRUE == checkAvailability) && (TRUE == isAvailable) )
    {
      BOOL const get_feature = pCamera->GetFeatureInfo(pFeatureName, pFeature);
      assert(TRUE == get_feature);

      SapFeature::Type type = SapFeature::TypeUndefined;
      BOOL const get_type = pFeature->GetType(&type);
      assert(TRUE == get_type);
      assert(SapFeature::TypeString == type);
      if (SapFeature::TypeString == type)
        {
          char featureStringValue[1024];
          BOOL const get_string = pCamera->GetFeatureValue(pFeatureName, featureStringValue, 1024);
          assert(TRUE == get_string);
          if (TRUE == get_string)
            {
              int const cnt = printf(pFormatString, featureStringValue);
              assert(0 < cnt);
              printed = (0 < cnt);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  return printed;
}
/* PrintGenICamStringNode_inline */



//! Get GenICam pixel format.
/*!
  Returns GenICam pixel format.

  \param pCamera        Pointer to Sapera camera class.
  \param pixelFormat    Pointer where pixel format value will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
inline
GetGenICamPixelFormat_inline(
                             SapAcqDevice * const pCamera,
                             unsigned int * const pixelFormat
                             )
{
  assert(NULL != pCamera);
  if (NULL == pCamera) return false;

  assert(NULL != pixelFormat);
  if (NULL == pixelFormat) return false;

  BOOL isAvailable = FALSE;
  BOOL const check = pCamera->IsFeatureAvailable("PixelFormat", &isAvailable);
  assert(TRUE == check);
  if ( (TRUE != check) || (TRUE != isAvailable) ) return false;

  BOOL const get = pCamera->GetFeatureValue("PixelFormat", pixelFormat);
  assert(TRUE == get);
  if (TRUE != get) return false;

  return true;
}
/* GetGenICamPixelFormat_inline */



//! Sets GenICam feature to value.
/*!
  Sets GenICam feature to desired value.

  \param P      Pointer to acquisition parameters value.
  \param name   Name of the feature to set.
  \param value  Desired value.
  \param value_out      Address where achieved value will be stored. May be NULL.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
SetGenICamPropertyToValue_inline(
                                 AcquisitionParametersSapera * const P,
                                 char const * const name,
                                 double const value,
                                 double * const value_out
                                 )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pCamera);
  if (NULL == P->pCamera) return false;

  assert(NULL != name);
  if (NULL == name) return false;

  bool result = true;

  BOOL status = TRUE;
  BOOL isAvailable = FALSE;

  if (true == result)
    {
      status = P->pCamera->IsFeatureAvailable(name, &isAvailable);
      assert(TRUE == status);
      result = (TRUE == status) && (TRUE == isAvailable);
    }
  /* if */

  if (true == result)
    {
      status = P->pCamera->SetFeatureValue(name, value);
      assert(TRUE == status);
      result = (TRUE == status);
    }
  /* if */

  if ( (true == result) && (NULL != value_out) )
    {
      status = P->pCamera->GetFeatureValue(name, value_out);
      assert(TRUE == status);
      result = (TRUE == status);
    }
  /* if */

  return result;
}
/* SetGenICamPropertyToValue_inline */


#endif /* HAVE_SAPERA_SDK */



/****** IMAGE TRANSFER CALLBACK ******/


#ifdef HAVE_SAPERA_SDK

//! Queue acquired image for processing.
/*!
  Transfer callback function is called each time a complete frame is transferred.
  Default action is to queue acquired image to image encoder for processing and storage.
  It may also be used to update the display etc.

  \param pInfo  Callback info.
*/
void
XferCallback(
             SapXferCallbackInfo * pInfo
             )
{
  AcquisitionParameters * const P = (AcquisitionParameters *)pInfo->GetContext();
  assert(NULL != P);
  if (NULL == P) return;

  // Fetch timestamp.
  LARGE_INTEGER QPC_after_transfer;
  QPC_after_transfer.QuadPart = (LONGLONG)0;

  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_transfer );
  assert(TRUE == qpc_after);

  // Signal data transfer has ended.
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
          if (true == P->pWindow->fBlocking) assert( true == sData.fBlocking );
          if (true == P->pWindow->fFixed) assert( true == sData.fFixed );
        }
      /* if */
#endif /* _DEBUG */
    }
  /* if */

  // Fetch SDK pointer.
  AcquisitionParametersSapera * const pSapera = P->pSaperaSDK;
  assert(NULL != pSapera);
  if (NULL == pSapera)
    {
      ImageMetadataRelease( &sData );
      return;
    }
  /* if */

  // Test if buffer is acquired correctly.
  bool acquired = true;
  if ( (NULL != pSapera) && (NULL != pSapera->pBuffer) )
    {
      SapBuffer::State state = SapBuffer::StateEmpty;
      BOOL const get = pSapera->pBuffer->GetState(&state);
      if (TRUE == get)
        {
          if (0 != (state & SapBuffer::StateOverflow))
            {
              // Acquisition failed.
              acquired = false;
              if ( (true == sData.fBatch) && (false == sData.fFixed) )
                {
                  int const CameraID = P->CameraID;
                  int const index = sData.index;
                  int const ProjectorID = sData.ProjectorID;
                  unsigned int const retry = sData.retry + 1;
                  TCHAR const * const pFilename = sData.pFilename->c_str();

                  Debugfwprintf(stderr, gDbgImageTransferFailed, CameraID + 1, sData.key + 1);
                  Debugfwprintf(stderr, gDbgRequeueSLPattern, CameraID + 1, pFilename, retry, 2);

                  // Requeue image.
                  if ( (NULL != P->pImageDecoder) &&
                       (NULL != P->pImageDecoder->pImageList) &&
                       (3 > retry)
                       )
                    {
                      ImageFileList * const pImageList = P->pImageDecoder->pImageList;
                      assert(NULL != pImageList);

                      QueuedDecoderImage * item = NULL;

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
                              item = new QueuedDecoderImage(P->pImageDecoder->pWICFactory, fullname);
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
                          
                          bool const inserted = ImageDecoderQueueImage(P->pImageDecoder, item);
                          if (false == inserted) SAFE_DELETE(item);
                        }
                      /* if (NULL != item) */
                    }
                  /* if */
                }
              /* if */
            }
          else if ( (0 == (state & SapBuffer::StateOverflow)) &&
                    (0 != (state & SapBuffer::StateFull))
                    )
            {
              // Acquisition succeeded. Nothing to do!
            }
          /* if */
        }
      /* if */
    }
  /* if */

  // Queue last successfully acquired frame into image encoder queue.
  if ( (true == acquired) && (NULL != P->pImageEncoder) && (NULL != pSapera) )
    {
      QueuedEncoderImage * item = new QueuedEncoderImage();
      assert(NULL != item);
      if (NULL != item)
        {
          bool const copy_metadata = item->CopyMetadataFrom( &sData );
          assert(true == copy_metadata);

          BOOL const copy_image = item->CopyImageFrom(pSapera->pBuffer, pSapera->pCamera);
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

      BOOL const set_end = P->pSynchronization->EventSet(MAIN_END_CAMERA, P->CameraID);
      assert(0 != set_end);
    }
  /* if */

  // Display frames.
  if ( (true == P->fView) && (NULL != P->pView) )
    {
      PushImage(P->pView, P->CameraID, pSapera->pBuffer, pSapera->pCamera);
    }
  /* if */
}
/* XferCallback */

#endif /* HAVE_SAPERA_SDK */




/****** EXPORTED FUNCTIONS ******/

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
AcquisitionParametersSaperaStopTransfer(
                                        AcquisitionParametersSapera * const P,
                                        double const exposureTime,
                                        int const nFrames
                                        )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_SAPERA_SDK
  //assert(NULL != P->pTransfer);
  if (NULL != P->pTransfer)
    {
      BOOL const freeze = P->pTransfer->Freeze();
      assert(TRUE == freeze);
      if (TRUE != freeze) result = false;

      DWORD const dwMilliseconds = (DWORD)( 0.001 * exposureTime );
      SleepEx(dwMilliseconds, TRUE);

      BOOL const wait = P->pTransfer->Wait(nFrames * dwMilliseconds + 5000);
      assert(TRUE == wait);
      if (TRUE != wait) result = false;
    }
  else
    {
      result = (NULL == P->pTransfer) && (NULL == P->pCamera) && (NULL == P->pBuffer);
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */

  return result;
}
/* AcquisitionParametersSaperaStopTransfer */



//! Start transfer.
/*!
  Starts image transfer.

  \param P      Pointer to parameters structure.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersSaperaStartTransfer(
                                         AcquisitionParametersSapera * const P
                                         )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_SAPERA_SDK
  assert(NULL != P->pTransfer);
  if (NULL != P->pTransfer)
    {
      BOOL const grab = P->pTransfer->Grab();
      assert(TRUE == grab);
      if (TRUE != grab) result = false;
    }
  else
    {
      result = false;
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */

  return result;
}
/* AcquisitionParametersSaperaStartTransfer */



//! Release Sapera SDK classes.
/*!
  Releases Sapera SDK classes.

  \param P      Pointer to parameters class.
*/
void
AcquisitionParametersSaperaRelease(
                                   AcquisitionParametersSapera * const P
                                   )
{
  //assert(NULL != P);
  if (NULL == P) return;

#ifdef HAVE_SAPERA_SDK
  {
    // Stop acquisition.
    bool const stop = AcquisitionParametersSaperaStopTransfer(P);
    assert(true == stop);

    // Remove all callbacks.
    if (NULL != P->pCamera)
      {
        BOOL const deregister = UnregisterAllCallbacks(P->pCamera);
        assert(TRUE == deregister);
      }
    /* if */

    // Release resources for all objects.
    SafeDestroy<SapAcqDeviceToBuf>( P->pTransfer );
    SafeDestroy<SapBayer>( P->pBayer );
    SafeDestroy<SapBuffer>( P->pBuffer );
    SafeDestroy<SapFeature>( P->pFeature );
    SafeDestroy<SapAcqDevice>( P->pCamera );
  }
#endif /* HAVE_SAPERA_SDK */

  AcquisitionParametersSaperaBlank_inline( P );
  free( P );
}
/* AcquisitionParametersSaperaRelease */



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
AcquisitionParametersSaperaAdjustExposureTime(
                                              AcquisitionParametersSapera * const P,
                                              int const CameraID,
                                              double const exposureTime_requested,
                                              double * const exposureTime_achieved
                                              )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = true;

#ifdef HAVE_SAPERA_SDK
  assert(NULL != P->pCamera);
  if (NULL != P->pCamera)
    {
      // TODO: Exposure time node name may depend on camera model
      // so this fixed name should be replaced with dynamically generated one.
      char const * const featureName = "ExposureTimeAbs";

      BOOL status = TRUE;
      BOOL isAvailable = FALSE;
      status = P->pCamera->IsFeatureAvailable(featureName, &isAvailable);
      assert(TRUE == status);

      result = false; // Assume failure.

      if ( (TRUE == status) && (TRUE == isAvailable) )
        {
          status = P->pCamera->SetFeatureValue(featureName, exposureTime_requested);
          assert(TRUE == status);
          if (TRUE == status)
            {
              result = true;

              double exposureTimeFromCamera = 0.0;
              status = P->pCamera->GetFeatureValue(featureName, &exposureTimeFromCamera);
              assert(TRUE == status);
              if (TRUE == status)
                {
                  if (NULL != exposureTime_achieved) *exposureTime_achieved = exposureTimeFromCamera;
                  wprintf(gMsgExposureTimeSet, CameraID + 1, exposureTimeFromCamera);

                  double const relative_difference = fabs( (exposureTime_requested - exposureTimeFromCamera) / exposureTime_requested );
                  if (0.005 <= relative_difference)
                    {
                      wprintf(gMsgExposureTimeSetLargeDifference, CameraID + 1);
                    }
                  /* if */
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
    }
  else
    {
      result = false;
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */

  return result;
}
/* AcquisitionParametersSaperaAdjustExposureTime */



//! Adjust camera delay and exposure times.
/*!
  Sets camera exposure and delay times.

  \param P      Pointer to parameters structure.
  \param t_delay_ms        Desired delay time in milliseconds.
  \param t_exp_ms   Desired exposure time in milliseconds.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersSaperaSetExposureAndDelayTimes(
                                                    AcquisitionParametersSapera * const P,
                                                    double * const t_delay_ms,
                                                    double * const t_exp_ms
                                                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = true;

#ifdef HAVE_SAPERA_SDK
  if ( (NULL != t_exp_ms) && (0 < *t_exp_ms) )
    {
      double t_exp_us = 1000.0 * ( *t_exp_ms ); // Convert to microseconds.
      bool const set = SetGenICamPropertyToValue_inline(P, "ExposureTimeAbs", t_exp_us, &t_exp_us);
      *t_exp_ms = 0.001 * t_exp_us; // Convert back to milliseconds.
      result &= set;
    }
  /* if */

  if ( (NULL != t_delay_ms) && (0 < *t_delay_ms) )
    {
      double t_delay_us = 1000.0 * ( *t_delay_ms ); // Convert to microseconds.
      bool const set = SetGenICamPropertyToValue_inline(P, "TriggerDelayAbs", t_delay_us, &t_delay_us);
      *t_delay_ms = 0.001 * t_delay_us; // Convert back to milliseconds.
      result &= set;
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */

  return result;
}
/* AcquisitionParametersSaperaSetExposureAndDelayTimes */



//! Create Sapera SDK classes.
/*!
  Create Sapera SDK classes, connect to camera, and configure camera for
  software triggering in overlapped acquisition mode.

  \param parameters Pointer to acquisition thread parameters structure.
  \param nFrames Number of buffers to allocate for temporary image storage.
  \param pConnectedCameras A vector of pointers to strings which uniquely identifiy prohibited cameras. May be NULL.
  \return Returns pointer to parameters structure or NULL if unsuccessfull.
*/
AcquisitionParametersSapera *
AcquisitionParametersSaperaCreate(
                                  AcquisitionParameters_ * const parameters,
                                  int const nFrames,
                                  std::vector<std::wstring *> * const pConnectedCameras
                                  )
{
  AcquisitionParametersSapera * const P = (AcquisitionParametersSapera *)malloc(sizeof(AcquisitionParametersSapera));
  assert(NULL != P);
  if (NULL == P) return P;

  AcquisitionParametersSaperaBlank_inline( P );

  BOOL status = TRUE;

  std::vector<int> valid_servers;
  std::vector<int> prohibited_cameras;

#ifdef HAVE_SAPERA_SDK


  /****** PRINT SAPERA LT SDK INFO******/

  status = SapManager::Open();
  assert(TRUE == status);
  if (TRUE != status)
    {
      int const cnt = wprintf(gMsgSaperaLTLoadDLLFailed);
      assert(0 < cnt);
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  {
    SapManVersionInfo sVersionInfo;
    status = SapManager::GetVersionInfo(&sVersionInfo);
    assert(TRUE == status);
    if (TRUE == status)
      {
        int const major = sVersionInfo.GetMajor();
        int const minor = sVersionInfo.GetMinor();
        int const revision = sVersionInfo.GetRevision();
        int const build = sVersionInfo.GetBuild();

        int const cnt = wprintf(gMsgSaperaLTVersion, major, minor, revision, build);
        assert(0 < cnt);
      }
    else
      {
        int const cnt = wprintf(gMsgSaperaLTLoadDLLFailed);
        assert(0 < cnt);
        goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
      }
    /* if */
  }

  /****** SELECT SERVER ******/

  /* SaperaLT SDK uses concept of servers which supply image data. A server may be an acquisition board
     (a frame grabber) with multiple attached cameras or a simple camera such as GigEVision camera.
     Camera selection should therefore first enumerate all servers, list them to the user and
     then prompt the user to select the server. If the server is simple (GigEVision camera or
     frame grabber with one camera) then the selection is complete, otherwise the user must be queried
     to select which camera attached to the frame grabber should be used.

     Currently we only support servers which are simple and are comprised of one imaging device;
     such servers allow creation of SapAcqDevice class.
  */

  // Get total number of servers (acquisition boards/cameras) in the system.
  int const serverCount = SapManager::GetServerCount();
  if (0 < serverCount)
    {
      int const cnt = wprintf(gMsgServerDetectionSucceeded, serverCount);
      assert(0 < cnt);
    }
  else
    {
      int const cnt = wprintf(gMsgServerDetectionFailed);
      assert(0 < cnt);
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  /* There must exist at least one server with attached camera. */
  int selectedServer = -1;
  valid_servers.reserve(serverCount);
  for (int serverIndex = 0; serverIndex < serverCount; ++serverIndex)
    {
      bool valid = false;

      /* Enumerate available servers which use SapAcqDevice class (e.g. GigEVision cameras). */
      if (false == valid)
        {
          int const max_j = SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice);
          for (int j = 0; j < max_j; ++j)
            {
              // GigEVision cameras should have only one resource.
              assert(1 == max_j);

              BOOL const is_available = SapManager::IsResourceAvailable(serverIndex, SapManager::ResourceAcqDevice, j);
              if (TRUE == is_available)
                {
                  bool prohibited = false; // Assume camera is available.

                  if (NULL != pConnectedCameras)
                    {
                      char resourceName[SapManager::MaxLabelSize + 1];
                      resourceName[SapManager::MaxLabelSize] = 0;

                      BOOL const get_resource_name = SapManager::GetResourceName(serverIndex, SapManager::ResourceAcqDevice, 0, resourceName);
                      assert(TRUE == get_resource_name);
                      if (TRUE == get_resource_name)
                        {
                          wchar_t resourceName_w[SapManager::MaxLabelSize + 1];
                          int idx = -1;
                          do
                            {
                              ++idx;
                              resourceName_w[idx] = resourceName[idx];
                            }
                          while ( (0 != resourceName[idx]) && (idx < SapManager::MaxLabelSize) );
                          resourceName_w[SapManager::MaxLabelSize] = 0;

                          for (size_t i = 0; i < pConnectedCameras->size(); ++i)
                            {
                              if ( (NULL != (*pConnectedCameras)[i]) && (0 == (*pConnectedCameras)[i]->compare(resourceName_w)) )
                                {
                                  prohibited = true;
                                  break;
                                }
                              /* if */
                            }
                          /* for */
                        }
                      /* if (TRUE == get_resource_name) */
                    }
                  /* if (NULL != pConnectedCameras) */

                  valid = !prohibited;
                  break;
                }
              /* if (TRUE == is_available) */
            }
          /* for */
        }
      /* if (false == valid) */

      /* Enumerate available servers which use SapAcquisition class (e.g. acquisition boards/frame grabbers). */
      if (false == valid)
        {
          int const max_j = SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcq);
          for (int j = 0; j < max_j; ++j)
            {
              BOOL const is_available = SapManager::IsResourceAvailable(serverIndex, SapManager::ResourceAcq, j);
              if (TRUE == is_available)
                {
                  //valid = true; // TODO: Extend the code to use SapAcquisition interface.
                  break;
                }
              /* if */
            }
          /* for */
        }
      /* if (false == valid) */

      if (true == valid)
        {
          valid_servers.push_back(serverIndex); // Maintain list of valid server indices.
          if (-1 == selectedServer) selectedServer = serverIndex; // First valid server is the default choice.
        }
      /* if (true == valid) */
    }
  /* for */

  if (-1 == selectedServer)
    {
      int const cnt = wprintf(gMsgServerNoAttachedDevices);
      assert(0 < cnt);
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  int const validServerCount = (int)( valid_servers.size() );

  // Print server selection menu.
  bool list_details = false;
  if (1 < validServerCount)
    {

    ACQUISITION_PARAMETERS_SAPERA_SERVER_SELECTION_MENU:

      {
        int const cnt1 = wprintf(L"\n");
        assert(0 < cnt1);

        int const cnt2 = wprintf(gMsgServerSelectionMenu);
        assert(0 < cnt2);
      }
      if (false == list_details)
        {
          int const cnt = wprintf(gMsgServerSelectionListDetails);
          assert(0 < cnt);
        }
      else
        {
          int const cnt = wprintf(gMsgServerSelectionListNoDetails);
          assert(0 < cnt);
        }
      /* if */

      for (int serverIndex = 0; serverIndex < serverCount; ++serverIndex)
        {
          bool server_valid = false;
          for (int i = 0; i < validServerCount; ++i)
            {
              if (serverIndex == valid_servers[i])
                {
                  server_valid = true;
                  break;
                }
              /* if */
            }
          /* for */

          if (false == server_valid) continue;

          char serverName[CORSERVER_MAX_STRLEN + 1];
          serverName[CORSERVER_MAX_STRLEN] = 0;

          BOOL const get_server_name = SapManager::GetServerName(serverIndex, serverName, CORSERVER_MAX_STRLEN);
          assert(TRUE == get_server_name);

          SapManager::ServerType const type = SapManager::GetServerType(serverIndex);
          char const * const serverType = GetSaperaLTServerTypeString_inline(type);
          if (NULL == serverType) continue;

          if (1 == SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice))
            {
              char resourceName[SapManager::MaxLabelSize + 1];
              resourceName[SapManager::MaxLabelSize] = 0;

              BOOL const get_resource_name = SapManager::GetResourceName(serverIndex, SapManager::ResourceAcqDevice, 0, resourceName);
              assert(TRUE == get_resource_name);

              SapAcqDevice * pCamera = NULL;
              SapFeature * pFeature = NULL;

              if (true == list_details)
                {
                  pCamera = new SapAcqDevice(serverName, TRUE);
                  assert(NULL != pCamera);

                  if (NULL != pCamera)
                    {
                      {
                        BOOL const create = pCamera->Create();
                        assert(TRUE == create);
                        if (TRUE != create) SafeDestroy<SapAcqDevice>( pCamera );
                      }

                      pFeature = new SapFeature(serverName);
                      assert(NULL != pFeature);
                      if (NULL != pFeature)
                        {
                          BOOL const create = pFeature->Create();
                          assert(TRUE == create);
                          if (TRUE != create) SafeDestroy<SapFeature>( pFeature );
                        }
                      /* if */
                    }
                  /* if */
                }
              /* if */

              if (selectedServer == serverIndex)
                {
                  int const cnt = printf(gMsgServerSelectionMenuCameraItemDefault, serverIndex + 1, serverName, resourceName);
                  assert(0 < cnt);
                }
              else
                {
                  int const cnt = printf(gMsgServerSelectionMenuCameraItem, serverIndex + 1, serverName, resourceName);
                  assert(0 < cnt);
                }
              /* if */

              if ( (NULL != pCamera) && (NULL != pFeature) )
                {
                  bool const pSerialNumber = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceSerialNumber", gMsgServerSelectionMenuCameraSN);
                  if (false == pSerialNumber) bool const pID = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceID", gMsgServerSelectionMenuCameraSN);
                  bool const pModelName = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceModelName", gMsgServerSelectionMenuCameraModel);
                  bool const pVersion = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceVersion", gMsgServerSelectionMenuCameraVersion);
                  bool const pVendorName = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceVendorName", gMsgServerSelectionMenuCameraVendor);
                  bool const pFirmwareVersion = PrintGenICamStringNode_inline(pCamera, pFeature, "DeviceVendorName", gMsgServerSelectionMenuCameraFirmwareVersion);
                }
              /* if */

              SafeDestroy<SapFeature>( pFeature );
              SafeDestroy<SapAcqDevice>( pCamera );
            }
          else
            {
              if (selectedServer == serverIndex)
                {
                  int const cnt = printf(gMsgServerSelectionMenuItemDefault, serverIndex + 1, serverName);
                  assert(0 < cnt);
                }
              else
                {
                  int const cnt = printf(gMsgServerSelectionMenuItem, serverIndex + 1, serverName);
                  assert(0 < cnt);
                }
              /* if */
            }
          /* if */
        }
      /* for */

      int const pressed_key = TimedWaitForNumberKey(60000, 10, true, true, (HWND)NULL);
      if (0 == pressed_key)
        {
          list_details = !list_details;
          goto ACQUISITION_PARAMETERS_SAPERA_SERVER_SELECTION_MENU;
        }
      else if ( (1 <= pressed_key) && (pressed_key <= serverCount) )
        {
          int const requestedServer = pressed_key - 1;
          bool server_valid = false;
          for (int i = 0; i < validServerCount; ++i)
            {
              if (requestedServer == valid_servers[i])
                {
                  server_valid = true;
                  break;
                }
              /* if */
            }
          /* for */
          if (true == server_valid)
            {
              selectedServer = requestedServer;
            }
          else
            {
              int const cnt = wprintf(gMsgServerSelectionMenuRevertToDefault);
              assert(0 < cnt);
            }
          /* if */
        }
      else
        {
          int const cnt = wprintf(gMsgServerSelectionMenuRevertToDefault);
          assert(0 < cnt);
        }
      /* if */
    }
  else
    {
      int const cnt = wprintf(gMsgServerOneAvailable);
      assert(0 < cnt);
    }
  /* if */

  // Selected server must be valid.
  assert( (0 <= selectedServer) && (selectedServer < serverCount) );
  if ( (0 > selectedServer) || (selectedServer >= serverCount) )
    {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  // Store server data.
  P->selectedServer = selectedServer;
  {
    BOOL const get = SapManager::GetServerName(P->selectedServer, P->selectedServerName, CORSERVER_MAX_STRLEN);
    assert(TRUE == get);
  }

  // Selected server must have at least one attached device.
  if ( (0 == SapManager::GetResourceCount(P->selectedServer, SapManager::ResourceAcq)) &&
       (0 == SapManager::GetResourceCount(P->selectedServer, SapManager::ResourceAcqDevice))
       )
    {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  // Allocate acquisition object with currently active device configuration.
  P->pCamera = new SapAcqDevice(P->selectedServerName, FALSE);
  assert(NULL != P->pCamera);
  if (NULL == P->pCamera)
    {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

  status = P->pCamera->Create();
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;


  /****** CONFIGURE CAMERA ******/

  // Create an empty feature object (to receive information) and use it to set exposure time and software trigger.
  P->pFeature = new SapFeature(P->selectedServerName);
  assert(NULL != P->pFeature);
  if (NULL == P->pFeature)
    {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* */

  status = P->pFeature->Create();
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  // Set software triggering.
  {
    char const * const featureName = "TriggerMode";
    BOOL isAvailable = FALSE;
    status = P->pCamera->IsFeatureAvailable(featureName, &isAvailable);
    assert(TRUE == status);

    if ( (TRUE == status) && (TRUE == isAvailable) )
      {
        status = P->pCamera->GetFeatureInfo(featureName, P->pFeature);
        assert(TRUE == status);

        SapFeature::Type type = SapFeature::TypeUndefined;
        status = P->pFeature->GetType(&type);
        assert(TRUE == status);
        assert(SapFeature::TypeEnum == type);

        int enumCount = 0;
        int enumValue = -1;
        char enumString[STRING_LENGTH];

        status = P->pFeature->GetEnumCount(&enumCount);
        assert(TRUE == status);
        if (TRUE == status)
          {
            for (int i = 0; i < enumCount; ++i)
              {
                status = P->pFeature->GetEnumStringFromValue(i, enumString, STRING_LENGTH);
                assert(TRUE == status);
                if (0 == strcmp("On", enumString))
                  {
                    enumValue = i;
                    break;
                  }
              }
            /* for */
          }
        /* if */

        if (-1 != enumValue)
          {
            status = P->pCamera->SetFeatureValue(featureName, enumString);
            assert(TRUE == status);
          }
        /* if */

        status = P->pCamera->GetFeatureValue(featureName, enumString, STRING_LENGTH);
        assert(TRUE == status);
        if (TRUE == status)
          {
            int const cnt = printf(gMsgCameraSetTriggerMode, enumString);
            assert(0 < cnt);
          }
        /* if */
      }
    /* if */
  }

  {
    char const * const featureName = "TriggerSource";
    BOOL isAvailable = FALSE;
    status = P->pCamera->IsFeatureAvailable(featureName, &isAvailable);
    assert(TRUE == status);

    if ( (TRUE == status) && (TRUE == isAvailable) )
      {
        status = P->pCamera->GetFeatureInfo(featureName, P->pFeature);
        assert(TRUE == status);

        SapFeature::Type type = SapFeature::TypeUndefined;
        status = P->pFeature->GetType(&type);
        assert(TRUE == status);
        assert(SapFeature::TypeEnum == type);

        int enumCount = 0;
        int enumValue = -1;
        char enumString[STRING_LENGTH];

        status = P->pFeature->GetEnumCount(&enumCount);
        assert(TRUE == status);
        if (TRUE == status)
          {
            for (int i = 0; i < enumCount; ++i)
              {
                status = P->pFeature->GetEnumStringFromValue(i, enumString, STRING_LENGTH);
                assert(TRUE == status);
                if (0 == strcmp("Software", enumString))
                  {
                    enumValue = i;
                    break;
                  }
              }
            /* for */
          }
        /* if */

        if (-1 != enumValue)
          {
            status = P->pCamera->SetFeatureValue(featureName, enumString);
            assert(TRUE == status);
          }
        /* if */

        status = P->pCamera->GetFeatureValue(featureName, enumString, STRING_LENGTH);
        assert(TRUE == status);
        if (TRUE == status)
          {
            int const cnt = printf(gMsgCameraSetTriggerSource, enumString);
            assert(0 < cnt);
          }
        /* if */
      }
    /* if */
  }

  // Get software trigger command node ID.
  {
    char const * const featureName = "TriggerSoftware";
    BOOL isAvailable = FALSE;
    status = P->pCamera->IsFeatureAvailable(featureName, &isAvailable);
    assert(TRUE == status);

    if ( (TRUE == status) && (TRUE == isAvailable) )
      {
        status = P->pCamera->GetFeatureIndexByName(featureName, &(P->idxTriggerSoftware));
        assert(TRUE == status);
      }
    /* if */
  }

  // Get exposure alignment.
  // Sapera documentation indicates the exposure alignment should be set to synchronous
  // for fastest possible triggering (see pg.132 in Genie Operational Reference).
  {
    char const * const featureName = "ExposureAlignment";
    BOOL isAvailable = FALSE;
    status = P->pCamera->IsFeatureAvailable(featureName, &isAvailable);
    assert(TRUE == status);

    if ( (TRUE == status) && (TRUE == isAvailable) )
      {
        status = P->pCamera->GetFeatureInfo(featureName, P->pFeature);
        assert(TRUE == status);

        SapFeature::Type type = SapFeature::TypeUndefined;
        status = P->pFeature->GetType(&type);
        assert(TRUE == status);

        SapFeature::WriteMode writeMode = SapFeature::WriteUndefined;
        status = P->pFeature->GetWriteMode(&writeMode); // Exposure alignment seems to be read-only??
        assert(TRUE == status);

        SapFeature::AccessMode accessMode = SapFeature::AccessUndefined;
        status = P->pFeature->GetAccessMode(&accessMode);
        assert(TRUE == status);

        status = P->pCamera->GetFeatureIndexByName(featureName, &(P->idxExposureAlignment));
        assert(TRUE == status);

        char featureValue[STRING_LENGTH];
        status = P->pCamera->GetFeatureValue(P->idxExposureAlignment, featureValue, STRING_LENGTH);
        assert(TRUE == status);
        if (TRUE == status)
          {
            int const cnt = printf(gMsgCameraSetExposureAlignment, featureValue);
            assert(0 < cnt);
          }
        /* if */
      }
    /* if */
  }



  /****** CREATE SAPERA CLASSES ******/

  // Allocate buffer object, taking settings directly from the camera.
  P->pBuffer = new SapBuffer(nFrames, P->pCamera);

  // Allocate transfer object to link acquisition and buffer.
  P->pTransfer = new SapAcqDeviceToBuf(P->pCamera, P->pBuffer, XferCallback, parameters);
  status = P->pTransfer->SetStartMode( SapTransfer::StartSynchronous );
  assert(TRUE == status);

  // Create resources for all objects.
  SafeCreate<SapBuffer>( P->pBuffer );
  SafeCreate<SapAcqDeviceToBuf>( P->pTransfer );


  /****** REGISTER CALLBACKS ******/

  status = RegisterCallback(P->pCamera, "ExposureStart", CameraCallbackExposureBegin, parameters);
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  status = RegisterCallback(P->pCamera, "ExposureEnd", CameraCallbackExposureEnd, parameters);
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  //status = RegisterCallback(P->pCamera, "ReadoutStart", CameraCallbackReadoutBegin, parameters);
  //assert(TRUE == status);
  //if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  //status = RegisterCallback(P->pCamera, "ReadoutEnd", CameraCallbackReadoutEnd, parameters);
  //assert(TRUE == status);
  //if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  //status = RegisterCallback(P->pCamera, "AcquisitionEnd", CameraCallbackAcquisitionEnd, parameters);
  //assert(TRUE == status);
  //if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  status = RegisterCallback(P->pCamera, "InvalidFrameTrigger", CameraCallbackInvalidFrameTrigger, parameters);
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;

  status = RegisterCallback(P->pCamera, "FrameSkipped", CameraCallbackFrameSkipped, parameters);
  assert(TRUE == status);
  if (TRUE != status) goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;


  /****** START ACQUISITION ******/

  bool const sapera_start = AcquisitionParametersSaperaStartTransfer(P);
  assert(true == sapera_start);
  if (true != sapera_start)
    {
      status = false;
      goto ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT;
    }
  /* if */

 ACQUISITION_PARAMETERS_SAPERA_CREATE_EXIT:

#endif /* HAVE_SAPERA_SDK */


  if (TRUE != status)
    {
      AcquisitionParametersSaperaRelease(P);
      return NULL;
    }
  /* if */

  return P;
}
/* AcquisitionParametersSaperaCreate */



//! Get camera ID.
/*!
  Returns unique camera identifier.
  For SaperaLT API the function returns server resource name as string.
  Returned pointer must be deleted after unique camera identifier is no longer needed.

  \param P Pointer to acquisition thread parameters.
  \return Returns pointer to unique camera name.
*/
std::wstring *
AcquisitionParametersSaperaGetCameraIdentifier(
                                               AcquisitionParametersSapera * const P
                                               )
{
  std::wstring * name = NULL;

  assert(NULL != P);
  if (NULL == P) return name;

#ifdef HAVE_SAPERA_SDK
  if (NULL != P->pCamera)
    {
      int const n_acq_device = SapManager::GetResourceCount(P->selectedServerName, SapManager::ResourceAcqDevice);
      if (0 < n_acq_device)
        {
          char resourceName[SapManager::MaxLabelSize + 1];
          resourceName[SapManager::MaxLabelSize] = 0;

          BOOL const get_resource_name = SapManager::GetResourceName(P->selectedServerName, SapManager::ResourceAcqDevice, 0, resourceName);
          assert(TRUE == get_resource_name);
          if (TRUE == get_resource_name)
            {
              wchar_t resourceName_w[SapManager::MaxLabelSize + 1];
              int idx = -1;
              do
                {
                  ++idx;
                  resourceName_w[idx] = resourceName[idx];
                }
              while ( (0 != resourceName[idx]) && (idx < SapManager::MaxLabelSize) );
              resourceName_w[SapManager::MaxLabelSize] = 0;

              name = new std::wstring( resourceName_w );
            }
          /* if */
        }
      /* if */

      if (NULL == name)
        {
          int const n_acq = SapManager::GetResourceCount(P->selectedServerName, SapManager::ResourceAcq);
          if (0 < n_acq)
            {
              // TODO!
            }
          /* if */
        }
      /* if */
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */

  return name;
}
/* AcquisitionParametersSaperaGetCameraIdentifier */



#ifdef HAVE_SAPERA_SDK

//! Get image data type.
/*!
  Returns image data type.

  \param pImage Pointer to Sapera buffer class.
  \param pCamera Pointer to Sapera acquisition device class.
  \return Returns image data type.
*/
ImageDataType
GetImageDataType(
                 SapBuffer * const pImage,
                 SapAcqDevice * const pCamera
                 )
{
  assert(NULL != pImage);
  if (NULL == pImage) return IDT_UNKNOWN;

  ImageDataType type = IDT_UNKNOWN;

  switch ( pImage->GetFormat() )
    {
    default:
    case SapFormatUnknown:
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatMono8: // Unsigned monochrome, 8bits/pixel.
      //case SapFormatUint8: // Same as SapFormatMono8.
      {
        unsigned int pixelFormat = 0;
        bool const get_format = GetGenICamPixelFormat_inline(pCamera, &pixelFormat);
        assert(true == get_format);
        if (true == get_format)
          {
            switch ( pixelFormat )
              {
              case 0x01080008: // PFNC BayerGR8
                type = IDT_8U_BayerGR;
                break;

              case 0x01080009: // PFNC BayerRG8
                type = IDT_8U_BayerRG;
                break;

              case 0x0108000A: // PFNC BayerGB8
                type = IDT_8U_BayerGB;
                break;

              case 0x0108000B: // PFNC BayerBG8
                type = IDT_8U_BayerBG;
                break;

              default:
                type = IDT_8U_GRAY;
                break;
              }
            /* switch */
          }
        else
          {
            type = IDT_8U_GRAY;
          }
        /* if */
      }
      break;

    case SapFormatInt8: // Signed monochrome, 8bits/pixel.
      type = IDT_8S_GRAY;
      break;

    case SapFormatMono16: // Unsigned monochrome, 16bits/pixel.
      //case SapFormatUint16: // Same as SapFormatMono16.
      {
        unsigned int pixelFormat = 0;
        bool const get_format = GetGenICamPixelFormat_inline(pCamera, &pixelFormat);
        assert(true == get_format);
        if (true == get_format)
          {
            switch ( pixelFormat )
              {
              case 0x01100003: // PFNC Mono10
                type = IDT_10U_GRAY;
                break;

              case 0x0110000C: // PFNC BayerGR10
                type = IDT_10U_BayerGR;
                break;

              case 0x0110000D: // PFNC BayerRG10
                type = IDT_10U_BayerRG;
                break;

              case 0x0110000E: // PFNC BayerGB10
                type = IDT_10U_BayerGB;
                break;

              case 0x0110000F: // PFNC BayerBG10
                type = IDT_10U_BayerBG;
                break;

              case 0x0110002E: // PFNC BayerGR16
                type = IDT_16U_BayerGR;
                break;

              case 0x0110002F: // PFNC BayerRG16
                type = IDT_16U_BayerRG;
                break;

              case 0x01100030: // PFNC BayerGB16
                type = IDT_16U_BayerGB;
                break;

              case 0x01100031: // PFNC BayerBG16
                type = IDT_16U_BayerBG;
                break;

              default:
                type = IDT_16U_GRAY;
                break;
              }
            /* switch */
          }
        else
          {
            type = IDT_16U_GRAY;
          }
        /* if */
      }
      break;

    case SapFormatInt16: // Signed monochrome, 16bits/pixel.
      type = IDT_16S_GRAY;
      break;

    case SapFormatInt24:
    case SapFormatMono24:
      //case SapFormatUint24: // Same as SapFormatMono24.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatInt32: // Signed monochrome, 32bits/pixel.
      type = IDT_32S_GRAY;
      break;

    case SapFormatMono32: // Unsigned monochrome, 32bits/pixel.
      //case SapFormatUint32: // Same as SapFormatMono32.
      type = IDT_32U_GRAY;
      break;

    case SapFormatInt64:
    case SapFormatMono64:
      //case SapFormatUint64: // Same as SapFormatMono64.
    case SapFormatRGB5551: // 16-bit, 5 for each of red/green/blue, 1 for alpha.
    case SapFormatRGB565: // 16-bit, 5 for red, 6 for green, 5 for blue.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatRGB888: // 24-bit, 8 for red, 8 for green, 8 for blue, blue component is stored first (should be called BGR, not RGB).
      type = IDT_8U_BGR;
      break;

    case SapFormatRGBR888: // 24-bit, 8 for red, 8 for green, 8 for blue, red component is stored first.
      type = IDT_8U_RGB;
      break;

    case SapFormatRGB8888: // 32-bit, 8 for each of red/green/blue, 8 for alpha.
      type = IDT_8U_BGRA;
      break;

    case SapFormatRGB101010: // 32-bit, 10 for each of red/green/blue, 2 unused.
    case SapFormatRGB161616: // 48-bit, 16 for each of red/green/blue.
    case SapFormatRGB16161616: // 64-bit, 16 for each of red/green/blue/alpha.
    case SapFormatHSV: // 32-bit HSV, 8 for each component, 8 unused.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatUYVY: // 16-bit, 4:2:2 subsampled.
      type = IDT_8U_YUV422;
      break;

    case SapFormatYUY2: // 16-bit, 4:2:2 subsampled.
    case SapFormatYVYU: // 16-bit, 4:2:2 subsampled.
    case SapFormatYUYV: // 16-bit, 4:2:2 subsampled.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatY411: // 12-bit, 4:1:1 subsampled; also known as Y41P.
      //case SapFormatIYU1: // Same as SapFormatY411.
      type = IDT_8U_YUV411;
      break;

    case SapFormatY211: // 8-bit, 2:1:1 subsampled.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatYUV: // 32-bit (8 for each of Y/U/V, 8 for alpha).
      //case SapFormatAYU2: // Same as SapFormatYUV.
      type = IDT_8U_YUV444;
      break;

    case SapFormatIYU2:
    case SapFormatFloat: // 32-bit signed floating point.
    case SapFormatComplex: // Real and imaginary components
    case SapFormatPoint: // X and Y integer components; 64-bit, 32-bit signed integer for both X and Y components.
    case SapFormatFPoint: // X and Y float components; 64-bit, 32-bit signed floating-point for both X and Y components.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatMono1: // 1-bit monochrome
      //case SapFormatUint1: // Same as SapFormatMono1.
      //case SapFormatBinary: // Same as SapFormatMono1.
      type = IDT_8U_BINARY;
      break;

    case SapFormatHSI: // 32-bit HSI, 8 for each component, 8 unused.
    case SapFormatLAB: // 32-bit, 8 for each component, 8 unused.
    case SapFormatLAB16161616:
    case SapFormatLAB101010: // 32-bit, 10 for each of red/green/blue, 2 unused.
      assert(IDT_UNKNOWN == type);
      break;

    case SapFormatRGBP8: // 8-bit RGB
      type = IDT_8U_RGB_Planar;
      break;

    case SapFormatRGBP16: // 16-bit RGB
    case SapFormatYUVP8: // 8-bit YUV
    case SapFormatYUVP16: // 16-bit YUV
    case SapFormatHSVP8: // 8-bit HSV
      //case SapFormatLABP8: // Same as SapFormatHSVP8
    case SapFormatHSVP16: // 16-bit HSV
      //case SapFormatLABP16: // Same as SapFormatHSVP16
    case SapFormatHSIP8: // 8-bit HSI
    case SapFormatHSIP16: // 16-bit HSI
    case SapFormatMono9:
      //case SapFormatUint9: // Same as SapFormatMono9.
    case SapFormatMono10:
      //case SapFormatUint10: // Same as SapFormatMono10.
    case SapFormatMono11:
      //case SapFormatUint11: // Same as SapFormatMono11.
    case SapFormatMono12:
      //case SapFormatUint12: // Same as SapFormatMono12.
    case SapFormatMono13:
      //case SapFormatUint13: // Same as SapFormatMono13.
    case SapFormatMono14:
      //case SapFormatUint14: // Same as SapFormatMono14.
    case SapFormatMono15:
      //case SapFormatUint15: // Same as SapFormatMono15.
    case SapFormatInt9:
    case SapFormatInt10:
    case SapFormatInt11:
    case SapFormatInt12:
    case SapFormatInt13:
    case SapFormatInt14:
    case SapFormatInt15:
    case SapFormatColorI8:
    case SapFormatColorI9:
    case SapFormatColorI10:
    case SapFormatColorI11:
    case SapFormatColorI12:
    case SapFormatColorI13:
    case SapFormatColorI14:
    case SapFormatColorI15:
    case SapFormatColorI16:
    case SapFormatColorNI8:
    case SapFormatColorNI9:
    case SapFormatColorNI10:
    case SapFormatColorNI11:
    case SapFormatColorNI12:
    case SapFormatColorNI13:
    case SapFormatColorNI14:
    case SapFormatColorNI15:
    case SapFormatColorNI16:
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
  \return A valid pixel format or SapFormatUnknown.
*/
SapFormat
GetSaperaPixelFormat(
                     ImageDataType const type
                     )
{
  SapFormat format = SapFormatUnknown;

  switch (type)
    {
    default:
    case IDT_UNKNOWN:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_BINARY:
      format = SapFormatMono1;
      break;

    case IDT_8U_GRAY:
      format = SapFormatMono8;
      break;

    case IDT_12U_GRAY_Packed:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_GRAY:
      format = SapFormatMono16;
      break;

    case IDT_16U_GRAY_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_32U_GRAY:
      format = SapFormatMono32;
      break;

    case IDT_8S_GRAY:
      format = SapFormatInt8;
      break;

    case IDT_16S_GRAY:
      format = SapFormatInt16;
      break;

    case IDT_16S_GRAY_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_32S_GRAY:
      format = SapFormatInt32;
      break;

    case IDT_8U_BayerGR:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_BayerRG:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_BayerGB:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_BayerBG:
      assert(SapFormatUnknown == format);
      break;

    case IDT_12U_BayerGR_Packed:
      assert(SapFormatUnknown == format);
      break;

    case IDT_12U_BayerRG_Packed:
      assert(SapFormatUnknown == format);
      break;

    case IDT_12U_BayerGB_Packed:
      assert(SapFormatUnknown == format);
      break;

    case IDT_12U_BayerBG_Packed:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerGR:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerRG:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerGB:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerBG:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerGR_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerRG_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerGB_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_16U_BayerBG_BigEndian:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_RGB:
      format = SapFormatRGBR888;
      break;

    case IDT_8U_RGB_Planar:
      format = SapFormatRGBP8;
      break;

    case IDT_8U_RGBA:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_BGR:
      format = SapFormatRGB888;
      break;

    case IDT_8U_BGRA:
      format = SapFormatRGB8888;
      break;

    case IDT_8U_YUV411:
      format = SapFormatY411;
      break;

    case IDT_8U_YUV422:
      format = SapFormatUYVY;
      break;

    case IDT_8U_YUV422_BT601:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_YUV422_BT709:
      assert(SapFormatUnknown == format);
      break;

    case IDT_8U_YUV444:
      format = SapFormatYUV;
      break;

    case IDT_8U_UYV444:
      assert(SapFormatUnknown == format);
      break;
    }
  /* switch */

  return format;
}
/* GetSaperaPixelFormat */



//! Get Bayer alignment mode.
/*!
  Returns Bayer tile format.

  \param type   Image data type.
  \return Returns Bayer tile format.
*/
SapBayer::Align
GetSaperaBayerAlignmentMode(
                            ImageDataType const type
                            )
{
  SapBayer::Align align = SapBayer::AlignAll;

  switch (type)
    {
    case IDT_8U_BayerGR:
    case IDT_12U_BayerGR_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerGR_BigEndian:
      align = SapBayer::AlignGRBG;
      break;

    case IDT_8U_BayerRG:
    case IDT_12U_BayerRG_Packed:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerRG_BigEndian:
      align = SapBayer::AlignRGGB;
      break;

    case IDT_8U_BayerGB:
    case IDT_12U_BayerGB_Packed:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerGB_BigEndian:
      align = SapBayer::AlignGBRG;
      break;

    case IDT_8U_BayerBG:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerBG_BigEndian:
      align = SapBayer::AlignBGGR;
      break;

    default:
      assert(SapBayer::AlignAll == align);
      break;
    }
  /* switch */

  return align;
}
/* GetSaperaBayerAlignmentMode */

#endif /* HAVE_SAPERA_SDK */



#endif /* __BATCHACQUISITIONSAPERA_CPP */
