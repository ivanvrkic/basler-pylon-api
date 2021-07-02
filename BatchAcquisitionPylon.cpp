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
  \file   BatchAcquisitionPylon.cpp
  \brief  Functions for Basler's Pylon SDK.

  Functions and wrappers for Baslers's Pylon SDK.
  
  \author Tomislav Petkovic
  \author Luka Dosen
  \author Ivan Vrkic
  \date   2021-06-09
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPYLON_CPP
#define __BATCHACQUISITIONPYLON_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionPylon.h"

#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionKeyboard.h"


using namespace Pylon;


/****** HELPER FUNCTIONS ******/

//! Blank parameters structure.
/*!
  Blanks acquisition parameter structure for Baslers's Pylon SDK.

  \param P      Pointer to parameters class.
*/
void
inline
AcquisitionParametersPylonBlank_inline(
                                       AcquisitionParametersPylon * const P
                                       )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->pCameraArray = NULL;
  P->pCamera = NULL;
  P->pCameraEventHandler = NULL;
  P->pImageEventHandler = NULL;

}
/* AcquisitionParametersPylonBlank_inline */



/****** IMAGE TRANSFER CALLBACK ******/


#ifdef HAVE_PYLON_SDK

// TODO: Add image tranfer callback functions here if required.
// NOTE: Image transfer callback is defined in BatchAcquisitionPylonCallbacks.cpp

#endif /* HAVE_PYLON_SDK */




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
AcquisitionParametersPylonStopTransfer(
                                       AcquisitionParametersPylon * const P,
                                       double const exposureTime,
                                       int const nFrames
                                       )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_PYLON_SDK

  // TODO: Add code here!
  
#endif /* HAVE_PYLON_SDK */

  return result;
}
/* AcquisitionParametersPylonStopTransfer */



//! Start transfer.
/*!
  Starts image transfer.

  \param P      Pointer to parameters structure.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersPylonStartTransfer(
                                        AcquisitionParametersPylon * const P
                                        )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  bool result = true;

#ifdef HAVE_PYLON_SDK

  // TODO: Add code here!
  
#endif /* HAVE_PYLON_SDK */

  return result;
}
/* AcquisitionParametersPylonStartTransfer */



//! Release Pylon SDK classes.
/*!
  Releases Pylon SDK classes.

  \param P      Pointer to parameters class.
*/
void
AcquisitionParametersPylonRelease(
                                  AcquisitionParametersPylon * const P
                                  )
{
  //assert(NULL != P);
  if (NULL == P) return;

#ifdef HAVE_PYLON_SDK
  {
    // Stop acquisition.
    bool const stop = AcquisitionParametersPylonStopTransfer(P);
    assert(true == stop);


    SAFE_DELETE( P->pImageEventHandler );
    SAFE_DELETE( P->pCameraEventHandler );
    SAFE_DELETE( P->pCamera );
    SAFE_DELETE( P->pCameraArray );


  }
#endif /* HAVE_PYLON_SDK */

  AcquisitionParametersPylonBlank_inline( P );
  free( P );
}
/* AcquisitionParametersPylonRelease */



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
AcquisitionParametersPylonAdjustExposureTime(
                                             AcquisitionParametersPylon * const P,
                                             int const CameraID,
                                             double const exposureTime_requested,
                                             double * const exposureTime_achieved
                                             )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool result = true;

#ifdef HAVE_PYLON_SDK

  // TODO: Add code here!
  
#endif /* HAVE_PYLON_SDK */

  return result;
}
/* AcquisitionParametersPylonAdjustExposureTime */



//! Adjust camera delay and exposure times.
/*!
  Sets camera exposure and delay times.

  \param P      Pointer to parameters structure.
  \param t_delay_ms        Desired delay time in milliseconds.
  \param t_exp_ms   Desired exposure time in milliseconds.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersPylonSetExposureAndDelayTimes(
                                                   AcquisitionParametersPylon * const P,
                                                   double * const t_delay_ms,
                                                   double * const t_exp_ms
                                                   )
{
  assert(NULL != P);
  if (NULL == P) return false;
  
  bool result = true;

#ifdef HAVE_PYLON_SDK

  // TODO: Add code here!
  
#endif /* HAVE_PYLON_SDK */

  return result;
}
/* AcquisitionParametersPylonSetExposureAndDelayTimes */



//! Create Pylon SDK classes.
/*!
  Create Pylon SDK classes, connect to camera, and configure camera for
  software triggering in overlapped acquisition mode.

  \param parameters Pointer to acquisition thread parameters structure.
  \param nFrames Number of buffers to allocate for temporary image storage.
  \param pConnectedCameras A vector of pointers to strings which uniquely identifiy prohibited cameras. May be NULL.
  \return Returns pointer to parameters structure or NULL if unsuccessfull.
*/
AcquisitionParametersPylon *
AcquisitionParametersPylonCreate(
                                 AcquisitionParameters_ * const parameters,
                                 int const nFrames,
                                 std::vector<std::wstring *> * const pConnectedCameras
                                 )
{
  AcquisitionParametersPylon * const P = (AcquisitionParametersPylon *)malloc(sizeof(AcquisitionParametersPylon));
  assert(NULL != P);
  if (NULL == P) return P;

  AcquisitionParametersPylonBlank_inline( P );

  BOOL status = TRUE;

#ifdef HAVE_PYLON_SDK

  DeviceInfoList_t devices;


  /****** PRINT SDK INFO******/

  // TODO: Add SDK info dump here!
  
  

  /****** SELECT CAMERA ******/

  // TODO: Add camera selection code here!
  //transport layer factory
  CTlFactory & tlFactory = CTlFactory::GetInstance();

  assert(NULL == P->pCamera);
  P->pCamera = new CInstantCamera();
  assert(NULL != P->pCamera);
  if (NULL == P->pCamera)
  {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT;
  }
  // Get all attached devices and exit application if no device is found.

  
  if (tlFactory.EnumerateDevices(devices) == 0)
  {
      
      status = FALSE;
      goto ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT;
  }
  
  assert(NULL == P->pCameraArray);
  P->pCameraArray = new CInstantCameraArray(devices.size());
  assert(NULL != P->pCameraArray);
  if (NULL == P->pCameraArray)
  {
      status = FALSE;
      goto ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT;
  }
  

  // Create and attach all Pylon Devices.
  for (size_t i = 0; i < P->pCameraArray->GetSize(); ++i)
  {
      ( *(P->pCameraArray) )[i].Attach(tlFactory.CreateDevice(devices[i]));

      // Print the model name of the camera.
      cout << "Using device " << (*(P->pCameraArray))[i].GetDeviceInfo().GetModelName() << endl;
      // Print the camera serial number in case of same model cameras
      //cout << "Using device " << cameras[i].GetDeviceInfo().GetSerialNumber << endl;
  }

  
  if (tlFactory.EnumerateDevices(devices) == 0)
  {
      
      status = FALSE;
      goto ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT;
  }
  


  // Create and attach all Pylon Devices.
  for (size_t i = 0; i < cameras.GetSize(); ++i)
  {
      cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

      // Print the model name of the camera.
      cout << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
  }


  /****** CONFIGURE CAMERA ******/

  // TODO: Add camera configuration code here!

  for (size_t i = 0; i < P->pCameraArray->GetSize(); ++i)
  {

      // Open the camera for accessing the parameters.
      (*(P->pCameraArray))[i].Open();

      GenApi::INodeMap& nodemap = (*(P->pCameraArray))[i].GetNodeMap();

      // Get camera device information.
      cout << "Camera Device Information" << endl
          << "=========================" << endl;
      cout << "Vendor           : "
          << CStringParameter(nodemap, "DeviceVendorName").GetValue() << endl;
      cout << "Model            : "
          << CStringParameter(nodemap, "DeviceModelName").GetValue() << endl;
      cout << "Firmware version : "
          << CStringParameter(nodemap, "DeviceFirmwareVersion").GetValue() << endl << endl;

      // Camera settings.
      cout << "Camera Device Settings" << endl
          << "======================" << endl;


      // Get the parameters for setting the image area of interest (Image AOI).
      CIntegerParameter width(nodemap, "Width");
      CIntegerParameter height(nodemap, "Height");
      CIntegerParameter offsetX(nodemap, "OffsetX");
      CIntegerParameter offsetY(nodemap, "OffsetY");

      //offsetX.TrySetToMinimum();
      //offsetY.TrySetToMinimum();
   

      // Maximize the Image AOI.
      offsetX.TrySetToMinimum(); // Set to minimum if writable.
      offsetY.TrySetToMinimum(); // Set to minimum if writable.
      width.SetToMaximum();
      height.SetToMaximum();

      // Set the pixel data format.
      CEnumParameter(nodemap, "PixelFormat").SetValue("Mono8");

      // Access the PixelFormat enumeration type node.
      CEnumParameter pixelFormat(nodemap, "PixelFormat");

      // Remember the current pixel format.
      String_t oldPixelFormat = pixelFormat.GetValue();
      cout << "Old PixelFormat  : " << oldPixelFormat << endl;

      // Set the pixel format to Mono8 if available.
      if (pixelFormat.CanSetValue("Mono8"))
      {
          pixelFormat.SetValue("Mono8");
          cout << "New PixelFormat  : " << pixelFormat.GetValue() << endl;
      }


      // Set the new gain to 50% ->  Min + ((Max-Min) / 2).
      CEnumParameter gainAuto(nodemap, "GainAuto");
      gainAuto.TrySetValue("Off");



          // Access the GainRaw integer type node. This node is available for IIDC 1394 and GigE camera devices.
          CIntegerParameter gainRaw(nodemap, "GainRaw");
          gainRaw.SetValuePercentOfRange(50.0);
          cout << "Gain (50%)       : " << gainRaw.GetValue() << " (Min: " << gainRaw.GetMin() << "; Max: " << gainRaw.GetMax() << "; Inc: " << gainRaw.GetInc() << ")" << endl;


      // Restore the old pixel format.
      pixelFormat.SetValue(oldPixelFormat);

      // Close the camera.
      (*(P->pCameraArray))[i].Close();
  }
  
  
  /****** CREATE REQUIRED ADDITIONAL CLASSES ******/

  // TODO: Add initialization code here!

  assert(NULL != P->pCameraEventHandler);

  P->pCameraEventHandler = new CCustomCameraEventHandler(parameters);
  assert(NULL != P->pCameraEventHandler);
  
  assert(NULL != P->pImageEventHandler);
  P->pImageEventHandler = new CCustomImageEventHandler(parameters);
  assert(NULL != P->pImageEventHandler);  

  
  /****** REGISTER CALLBACKS ******/

  // TODO: If callbacks are used add callback registration here!

  P->pCamera->RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
  ////P->pCamera.RegisterConfiguration(new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete); // Camera use.
  P->pCamera->RegisterImageEventHandler(P->pImageEventHandler, RegistrationMode_Append, Cleanup_Delete);

  P->pCamera->GrabCameraEvents = true;
  P->pCamera->Open();
  //assert(P->pCamera->EventSelector->IsWritable());
  //  // Cameras based on SFNC 2.0 or later, e.g., USB cameras
  //  if (P->pCamera->GetSfncVersion() >= Sfnc_2_0_0)
  //  {
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "EventExposureEndData", ExposureEndEventId, RegistrationMode_ReplaceAll, Cleanup_None);
  //  }
  //  else
  //  {
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "ExposureEndEventData", ExposureEndEventId, RegistrationMode_ReplaceAll, Cleanup_None);
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "EventOverrunEventData", EventOverrunEventId, RegistrationMode_Append, Cleanup_None);
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "EventFrameStartWaitData", FrameStartWaitEventId, RegistrationMode_Append, Cleanup_None);
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "EventFrameTriggerMissedData", FrameTriggerMissedEventId, RegistrationMode_Append, Cleanup_None);
  //      P->pCamera->RegisterCameraEventHandler(P->pCameraEventHandler, "FrameStartEventData", FrameStartEventId, RegistrationMode_Append, Cleanup_None);
  //  }
  //P->pCamera->EventSelector->SetValue(EventSelector_ExposureEnd);
  //if (!P->pCamera->EventNotification.TrySetValue(EventNotification_On))
  //{
  //    P->pCamera.EventNotification.SetValue(EventNotification_GenICamEvent);
  //}
  //if (P->pCamera.EventSelector.TrySetValue(EventSelector_EventOverrun))
  //{
  //    if (!P->pCamera.EventNotification.TrySetValue(EventNotification_On))
  //    {
  //        P->pCamera.EventNotification.SetValue(EventNotification_GenICamEvent);
  //    }
  //}  


  /****** START ACQUISITION ******/

  bool const pylon_start = AcquisitionParametersPylonStartTransfer(P);
  assert(true == pylon_start);
  if (true != pylon_start)
    {
      status = false;
      goto ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT;
    }
  /* if */

 ACQUISITION_PARAMETERS_PYLON_CREATE_EXIT:

#endif /* HAVE_PYLON_SDK */
  

  if (TRUE != status)
    {
      AcquisitionParametersPylonRelease(P);
      return NULL;
    }
  /* if */

  return P;
}
/* AcquisitionParametersPylonCreate */



//! Get camera ID.
/*!
  Returns unique camera identifier.
  For Pylon SDK the function returns ??? as string.
  The returned pointer must be deleted after returned
  unique camera identifier is no longer needed.

  \param P Pointer to the acquisition thread parameters.
  \return Returns pointer to the unique camera name.
*/
std::wstring *
AcquisitionParametersPylonGetCameraIdentifier(
                                              AcquisitionParametersPylon * const P
                                              )
{
  std::wstring * name = NULL;
  
  assert(NULL != P);
  if (NULL == P) return name;
  
#ifdef HAVE_PYLON_SDK

  // TODO: Add ID retrieval code here!
  
#endif /* HAVE_PYLON_SDK */

  return name;
}
/* AcquisitionParametersPylonGetCameraIdentifier */



#ifdef HAVE_PYLON_SDK

//! Get image data type.
/*!
  Returns image data type.
  
  \return Returns image data type.
*/
ImageDataType
GetImageDataType(
                 int * const dummy
                 )
{
  ImageDataType type = IDT_UNKNOWN;

  // TODO: Add code for image data type identification!

  return type;
}
/* GetImageDataType */



//! Get pixel format.
/*!
  Function returns pixel format corresponding to the image data type.

  \param type   Image data type.
  \return A valid pixel format for Pylon SDK.
*/
void
GetPylonPixelFormat(
                    ImageDataType const type
                    )
{
  // TODO: Add output variable and populate the switch statement!
  
  switch (type)
    {
    default:
    case IDT_UNKNOWN:
      
      break;

    case IDT_8U_BINARY:
      
      break;

    case IDT_8U_GRAY:

      break;

    case IDT_12U_GRAY_Packed:

      break;

    case IDT_16U_GRAY:

      break;

    case IDT_16U_GRAY_BigEndian:

      break;

    case IDT_32U_GRAY:

      break;

    case IDT_8S_GRAY:

      break;

    case IDT_16S_GRAY:

      break;

    case IDT_16S_GRAY_BigEndian:

      break;

    case IDT_32S_GRAY:

      break;

    case IDT_8U_BayerGR:

      break;

    case IDT_8U_BayerRG:

      break;

    case IDT_8U_BayerGB:

      break;

    case IDT_8U_BayerBG:

      break;

    case IDT_12U_BayerGR_Packed:

      break;

    case IDT_12U_BayerRG_Packed:

      break;

    case IDT_12U_BayerGB_Packed:

      break;

    case IDT_12U_BayerBG_Packed:

      break;

    case IDT_16U_BayerGR:

      break;

    case IDT_16U_BayerRG:

      break;

    case IDT_16U_BayerGB:

      break;

    case IDT_16U_BayerBG:

      break;

    case IDT_16U_BayerGR_BigEndian:

      break;

    case IDT_16U_BayerRG_BigEndian:

      break;

    case IDT_16U_BayerGB_BigEndian:

      break;

    case IDT_16U_BayerBG_BigEndian:

      break;

    case IDT_8U_RGB:

      break;

    case IDT_8U_RGB_Planar:

      break;

    case IDT_8U_RGBA:

      break;

    case IDT_8U_BGR:

      break;

    case IDT_8U_BGRA:

      break;

    case IDT_8U_YUV411:

      break;

    case IDT_8U_YUV422:

      break;

    case IDT_8U_YUV422_BT601:

      break;

    case IDT_8U_YUV422_BT709:

      break;

    case IDT_8U_YUV444:

      break;

    case IDT_8U_UYV444:

      break;
    }
  /* switch */

}
/* GetPylonPixelFormat */



//! Get Bayer alignment mode.
/*!
  Returns Bayer tile format.

  \param type   Image data type.
  \return Returns Bayer tile format.
*/
void
GetPylonBayerAlignmentMode(
                           ImageDataType const type
                           )
{
  // TODO: Add output variable and populate the switch statement!
  
  switch (type)
    {
    case IDT_8U_BayerGR:
    case IDT_12U_BayerGR_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerGR_BigEndian:

      break;

    case IDT_8U_BayerRG:
    case IDT_12U_BayerRG_Packed:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerRG_BigEndian:

      break;

    case IDT_8U_BayerGB:
    case IDT_12U_BayerGB_Packed:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerGB_BigEndian:

      break;

    case IDT_8U_BayerBG:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerBG_BigEndian:

      break;

    default:

      break;
    }
  /* switch */

}
/* GetPylonBayerAlignmentMode */

#endif /* HAVE_PYLON_SDK */



#endif /* __BATCHACQUISITIONSPYLON_CPP */
