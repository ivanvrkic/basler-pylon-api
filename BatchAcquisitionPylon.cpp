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
  \date   2021-05-10
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPYLON_CPP
#define __BATCHACQUISITIONPYLON_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionPylon.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionKeyboard.h"



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

  // TODO: Add blanking code.
}
/* AcquisitionParametersPylonBlank_inline */



/****** IMAGE TRANSFER CALLBACK ******/


#ifdef HAVE_PYLON_SDK

// TODO: Add image tranfer callback functions here if required.

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

    // TODO: Add code here!
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


  /****** PRINT SDK INFO******/

  // TODO: Add SDK info dump here!
  

  /****** SELECT CAMERA ******/

  // TODO: Add camera selection code here!
  

  /****** CONFIGURE CAMERA ******/

  // TODO: Add camera configuration code here!

  
  /****** CREATE REQUIRED ADDITIONAL CLASSES ******/

  // TODO: Add initialization code here!

  
  /****** REGISTER CALLBACKS ******/

  // TODO: If callbacks are used add callback registration here!
  

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
