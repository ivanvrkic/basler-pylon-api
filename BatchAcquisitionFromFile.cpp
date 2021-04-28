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
  \file   BatchAcquisitionFromFile.cpp
  \brief  Functions for dummy acquisition from file.

  Functions for dummy acquisition from file.

  \author Tomislav Petkovic
  \date   2017-01-26
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONFROMFILE_CPP
#define __BATCHACQUISITIONFROMFILE_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionFromFile.h"
#include "BatchAcquisitionAcquisition.h"



/****** HELPER FUNCTIONS ******/

//! Blank parameters structure.
/*!
  Blanks acquisition parameter structure for dummy acquisition.

  \param P      Pointer to parameters class.
*/
void
inline
AcquisitionParametersFromFileBlank_inline(
                                          AcquisitionParametersFromFile * const P
                                          )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->pFileList = NULL;
  P->pAcquisitionThread = NULL;

  P->external_list = false;
}
/* AcquisitionParametersFromFileBlank_inline */



/****** IMAGE TRANSFER ******/

//! Queue image for processing.
/*!
  Function loads next file from disk and pushes it into processing queue.

  \param P Pointer to image acquisition structure.
*/
void
DispatchNextImageFromFile(
                          AcquisitionParameters_ * const P
                          )
{
  assert(NULL != P);
  if (NULL == P) return;

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
#ifdef _DEBUG
  if ( (true == pop) && (NULL != P->pWindow) )
    {
      if (true == P->pWindow->fBlocking) assert( true == sData.fBlocking );
      if (true == P->pWindow->fFixed) assert( true == sData.fFixed );
    }
  /* if */
#endif /* _DEBUG */

  // Fetch SDK pointer.
  AcquisitionParametersFromFile * const pFromFile = P->pFromFile;
  assert(NULL != pFromFile);
  if (NULL == pFromFile)
    {
      ImageMetadataRelease( &sData );
      return;
    }
  /* if */

  // Acquire buffer.
  bool acquired = false;
  cv::Mat * pImage = NULL;
  switch ( sData.render_type )
    {
    case QI_BGRA_BITMAP:
      {
        if ( (NULL != pFromFile) && (NULL != pFromFile->pFileList) )
          {
            pImage = pFromFile->pFileList->ReadImage((true == pop)? sData.index : -1);
            acquired = (NULL != pImage);
          }
        /* if */
      }
      break;

    case QI_PATTERN_SOLID:
      {
        int width = 0;
        int height = 0;
        HRESULT const get_size = GetDisplayWindowSize(P->pWindow, &width, &height, NULL, NULL);
        //assert( SUCCEEDED(get_size) );
        if ( SUCCEEDED(get_size) )
          {
            unsigned char const red = (unsigned char)(sData.red * 255.0f);
            unsigned char const green = (unsigned char)(sData.green * 255.0f);
            unsigned char const blue = (unsigned char)(sData.blue * 255.0f);
            pImage = new cv::Mat(height, width, CV_8UC3, cv::Scalar(red, green, blue));
            acquired = (NULL != pImage);
          }
        else
          {
            assert( false == acquired );
          }
        /* if */
      }
      break;

    case QI_UNKNOWN_TYPE:
    default:
      assert(NULL == pImage);
      break;
    }
  /* switch */

  // Fetch timestamp.
  LARGE_INTEGER QPC_after_transfer;
  QPC_after_transfer.QuadPart = (LONGLONG)0;

  BOOL const qpc_after = QueryPerformanceCounter( &QPC_after_transfer );
  assert(TRUE == qpc_after);

  // Add acquisition time to statistics.
  if (true == pop)
    {
      LARGE_INTEGER QPC_before_trigger;
      QPC_before_trigger.QuadPart = sData.QPC_before_trigger;

      FrameStatisticsAddMeasurement(P->pStatisticsAcquisitionDuration, QPC_before_trigger, QPC_after_transfer);
    }
  /* if */

  // Queue last successfully acquired frame into image encoder queue.
  if ( (true == acquired) && (NULL != P->pImageEncoder) && (NULL != pFromFile) )
    {
      QueuedEncoderImage * item = new QueuedEncoderImage();
      assert(NULL != item);
      if (NULL != item)
        {
          bool const copy_metadata = item->CopyMetadataFrom( &sData );
          assert(true == copy_metadata);

          void const * const src_data = pImage->data;
          ImageDataType const src_type = GetImageDataType(pImage);
          unsigned int const src_width = pImage->cols;
          unsigned int const src_height = pImage->rows;
          unsigned int const src_stride = (unsigned int)(pImage->step[0]);
          unsigned int const src_size = src_stride * src_height;

          BOOL const copy_image = item->CopyImageFrom(src_data, src_size, src_type, src_width, src_height, src_stride);
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
  if ( (true == acquired) && (true == P->fView) && (NULL != P->pView) )
    {
      void const * const src_data = pImage->data;
      ImageDataType const src_type = GetImageDataType(pImage);
      unsigned int const src_width = pImage->cols;
      unsigned int const src_height = pImage->rows;
      unsigned int const src_stride = (unsigned int)(pImage->step[0]);
      unsigned int const src_size = src_stride * src_height;

      PushImage(P->pView, P->CameraID, src_width, src_height, src_stride, src_type, src_data);
    }
  /* if */

  SAFE_DELETE( pImage );
}
/* DispatchNextImageFromFile */



/****** EXPORTED FUNCTIONS ******/

//! Set input directory.
/*!
  Sets input directory for from file acquisition.

  \param P      Pointer to parameters structure.
  \param directory      Pointer to directory name. If NULL user will be queried for valid directory.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFromFileSetDirectory(
                                          AcquisitionParametersFromFile * const P,
                                          wchar_t const * const directory
                                          )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  assert(NULL != P->pFileList);
  if (NULL == P->pFileList) return false;

  wchar_t const * pszTitle = NULL;
  int const sz = 1024;
  wchar_t szTitle[sz + 1];
  szTitle[sz] = 0;
  if (NULL != P->pAcquisitionThread)
    {
      int const CameraID = ((AcquisitionParameters_ *)(P->pAcquisitionThread))->CameraID;
      if (0 <= CameraID)
        {
          int const cnt = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForCamera, CameraID + 1);
          assert(0 < cnt);
          pszTitle = szTitle;
        }
      /* if */
    }
  /* if */

  bool const result = P->pFileList->SetDirectory(directory, pszTitle);
  //assert(true == result);

  return result;
}
/* AcquisitionParametersFromFileSetDirectory */



//! Get input directory.
/*!
  Gets input directory for from file acquisition.

  \param P      Pointer to parameters structure.
  \return Returns pointer to directory name if successfull and NULL pointer otherwise.
  Returned pointer is valid until the next call to AcquisitionParametersFromFileSetDirectory function.
*/
wchar_t const * const
AcquisitionParametersFromFileGetDirectory(
                                          AcquisitionParametersFromFile * const P
                                          )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  assert(NULL != P->pFileList);
  if (NULL == P->pFileList) return NULL;

  return P->pFileList->GetDirectory();
}
/* AcquisitionParametersFromFileGetDirectory */



//! Stop all pending transfers.
/*!
  Stop all pending data transfers.

  \param P      Pointer to parameters structure.
  \return Function returns true if successfull.
*/
bool
AcquisitionParametersFromFileStopTransfer(
                                          AcquisitionParametersFromFile * const P
                                          )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  assert(NULL != P->pFileList);
  if (NULL == P->pFileList) return false;

  bool result = true;

  bool const rewind = P->pFileList->Rewind();
  //assert(true == rewind);
  if (true != rewind) result = false;

  return result;
}
/* AcquisitionParametersFromFileStopTransfer */



//! Start transfer.
/*!
  Starts image transfer.

  \param P      Pointer to parameters structure.
  \param directory  Pointer to a string containing directory name where input images are stored.
  If NULL then previously set directory will be used.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFromFileStartTransfer(
                                           AcquisitionParametersFromFile * const P,
                                           wchar_t const * const directory
                                           )
{
  //assert(NULL != P);
  if (NULL == P) return true;

  assert(NULL != P->pFileList);
  if (NULL == P->pFileList) return false;

  bool result = true;

  if (NULL != directory)
    {
      wchar_t const * pszTitle = NULL;
      int const sz = 1024;
      wchar_t szTitle[sz + 1];
      szTitle[sz] = 0;
      if (NULL != P->pAcquisitionThread)
        {
          int const CameraID = ((AcquisitionParameters_ *)(P->pAcquisitionThread))->CameraID;
          if (0 <= CameraID)
            {
              int const cnt = swprintf_s(szTitle, sz, gMsgQueryInputDirectoryForCamera, CameraID + 1);
              assert(0 < cnt);
              pszTitle = szTitle;
            }
          /* if */
        }
      /* if */

      bool const set = P->pFileList->SetDirectory(directory, pszTitle);
      assert(true == set);
      if (true != set) result = false;
    }
  /* if */

  bool const rewind = P->pFileList->Rewind();
  //assert(true == rewind);
  if (true != rewind) result = false;

  return result;
}
/* AcquisitionParametersFromFileStartTransfer */



//! Release resources.
/*!
  Releases allocated resources.

  \param P      Pointer to parameters class.
*/
void
AcquisitionParametersFromFileRelease(
                                     AcquisitionParametersFromFile * const P
                                     )
{
  //assert(NULL != P);
  if (NULL == P) return;

  if (false == P->external_list) SAFE_DELETE(P->pFileList);

  AcquisitionParametersFromFileBlank_inline( P );

  free(P);
}
/* AcquisitionParametersFromFileRelease */



//! Adjust camera exposure time.
/*!
  As there is no exposure for dummy camera this function does nothing.

  \param P Pointer to parameters structure.
  \param exposureTime_requested Exposure time in microseconds (us).
  \param exposureTime_achieved Address where achieved exposure time in microseconds (us) will be stored. May be NULL.
  \return Returns true if successfull.
*/
bool
AcquisitionParametersFromFileAdjustExposureTime(
                                                AcquisitionParametersFromFile * const P,
                                                double const exposureTime_requested,
                                                double * const exposureTime_achieved
                                                )
{
  //assert(NULL != P);
  if (NULL != exposureTime_achieved) *exposureTime_achieved = exposureTime_requested;
  return true;
}
/* AcquisitionParametersFromFileAdjustExposureTime */



//! Create resources.
/*!
  Creates resources and asks user to select input directory.

  \param parameters Pointer to acquisition thread parameters structure.
  \param pFileList Pointer to file list.
  \return Returns pointer to parameters structure or NULL if unsuccessfull.
*/
AcquisitionParametersFromFile *
AcquisitionParametersFromFileCreate(
                                    AcquisitionParameters_ * const parameters,
                                    ImageFileList * const pFileList
                                    )
{
  AcquisitionParametersFromFile * const P = (AcquisitionParametersFromFile *)malloc( sizeof(AcquisitionParametersFromFile) );
  assert(NULL != P);
  if (NULL == P) return NULL;

  bool result = true;

  AcquisitionParametersFromFileBlank_inline( P );

  P->pAcquisitionThread = parameters;

  assert(NULL == P->pFileList);
  if (NULL == pFileList)
    {
      P->pFileList = new ImageFileList();
      assert(false == P->external_list);
    }
  else
    {
      P->pFileList = pFileList;
      P->external_list = true;
    }
  /* if */
  assert(NULL != P->pFileList);

  if (NULL == P->pFileList) result = false;

  bool const start_fromfile = AcquisitionParametersFromFileStartTransfer(P, NULL);
  //assert(true == start_fromfile);

  if (true != result)
    {
      AcquisitionParametersFromFileRelease( P );
      return NULL;
    }
  /* if */

  return P;
}
/* AcquisitionParametersFromFileCreate */



#endif /* !__BATCHACQUISITIONFROMFILE_CPP */
