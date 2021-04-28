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
  \file   BatchAcquisitionImageEncoder.cpp
  \brief  Decodes image from camera and encodes it for storing to file.

  Images are stored using OpenCV, Window Imaging Components, or specific camera SDK.

  For image processing and storage to disk we use a separate thread that maintains
  a queue of acquired images. This file contains code for the thread and for image
  processing and encoding.

  Each acquisition thread maintains its own image encoder queue.

  \author Tomislav Petkovic
  \date   2017-02-03
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGEENCODER_CPP
#define __BATCHACQUISITIONIMAGEENCODER_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionImageEncoder.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionImageConversion.h"
#include "BatchAcquisitionFileList.h"
#include "BatchAcquisitionPatternSolid.h"
#include "BatchAcquisitionSapera.h"
#include "BatchAcquisitionFlyCapture2.h"
#include "BatchAcquisitionDialogs.h"


#pragma warning(push)
#pragma warning(disable: 4005)

#include <shlwapi.h>

#pragma warning(pop)

#pragma comment(lib, "Shlwapi.lib")



/****** HELPER FUNCTIONS ******/

#pragma region // Inline helper functions

//! Blank data variables.
/*!
  Blanks data variables where image is stored.

  \param ptr    Pointer to QueuedEncoderImage class.
*/
inline
void
QueuedEncoderImageBlankData_inline(
                                   QueuedEncoderImage * const ptr
                                   )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->data = NULL;
  ptr->data_size = 0;
  ptr->data_type = IDT_UNKNOWN;
  ptr->data_width = 0;
  ptr->data_height = 0;
  ptr->data_stride = 0;
}
/* QueuedEncoderImageBlankData_inline */



//! Blank class variables.
/*!
  Blanks class variables.

  \param ptr    Pointer to QueuedEncoderImage class.
*/
inline
void
QueuedEncoderImageBlank_inline(
                               QueuedEncoderImage * const ptr
                               )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->count = -1;

  ptr->render_type = QI_UNKNOWN_TYPE;
  ptr->pattern_type = SL_PATTERN_INVALID;

  ptr->no = 0;
  ptr->index = -1;

  ptr->ProjectorID = -1;
  ptr->CameraID = -1;

  QueuedEncoderImageBlankData_inline(ptr);

  ptr->is_batch = false;
  ptr->save = false;
  ptr->save_to_png = false;
  ptr->save_to_raw = false;

  ptr->pFilename = NULL;

  ptr->red = 0.0f;
  ptr->green = 0.0f;
  ptr->blue = 0.0f;
  ptr->alpha = 1.0f;

  ptr->delay = 0.0;
  ptr->exposure = 0.0;

  ptr->QPC_before_trigger = -1.0;
  ptr->QPC_after_trigger = -1.0;
}
/* QueuedEncoderImageBlank_inline */



//! Blanks image encoder thread parameters.
/*!
  Blanks image encoder thread parameters.

  \param P      Pointer to image encoder thread parametes.
*/
inline
void
ImageEncoderParametersBlank_inline(
                                   ImageEncoderParameters * const P
                                   )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Blank structure. */
  P->tImageEncoder = (HANDLE)(NULL);

  P->pImageQueue = NULL;
  P->pStatistics = NULL;

  P->pAllImages = NULL;

  P->pDirectoryData = NULL;

  P->pSubdirectorySession = NULL;
  P->pSubdirectoryRecording = NULL;
  P->pSubdirectoryCamera = NULL;

  P->pSynchronization = NULL;
  P->pWICFactory = NULL;

  P->count = 0;
  P->maxItems = 18;
  P->minItems = 0;

  P->EncoderID = -1;
  P->CameraID = -1;

  P->roi_x = 0;
  P->roi_y = 0;
  P->roi_w = 0;
  P->roi_h = 0;

  P->num_batch = 0;
  P->fActive = false;
  P->fWaiting = false;

  ZeroMemory( &(P->sLockImageQueue), sizeof(P->sLockImageQueue) );
  ZeroMemory( &(P->sLockImageData), sizeof(P->sLockImageData) );
  ZeroMemory( &(P->sLockDirectory), sizeof(P->sLockDirectory) );
}
/* ImageEncoderParametersBlank_inline */

#pragma endregion // Inline helper functions



/****** IMAGE CONTAINER CLASS ******/

#pragma region // Image encoder queue item

//! Constructor.
/*!
  Creates image to be stored to file.
*/
QueuedEncoderImage::QueuedEncoderImage()
{
  QueuedEncoderImageBlank_inline(this);
}
/* QueuedEncoderImage::QueuedEncoderImage */



//! Destructor
/*!
  Releases all resources allocates by queued image.
*/
QueuedEncoderImage::~QueuedEncoderImage()
{
  SAFE_DELETE(this->pFilename);
  SAFE_FREE(this->data);
  QueuedEncoderImageBlank_inline(this);
}
/* QueuedEncoderImage::~QueuedEncoderImage */


#pragma region // Copy metadata and data

//! Copy metadata from image queue item.
bool
QueuedEncoderImage::CopyMetadataFrom(
                                     ImageMetadata const * const pData
                                     )
{
  assert(NULL != pData);
  if (NULL == pData) return false;

  assert(-1 == this->count);

  this->no = pData->no;
  this->index = pData->index;

  this->render_type = pData->render_type;
  this->pattern_type = pData->pattern_type;

  this->ProjectorID = pData->ProjectorID;
  this->CameraID = pData->CameraID;

  this->is_batch = pData->fBatch;
  this->save = pData->fSavePNG || pData->fSaveRAW;
  this->save_to_png = pData->fSavePNG;
  this->save_to_raw = pData->fSaveRAW;

  this->pFilename = pData->pFilename;

  this->red = pData->red;
  this->green = pData->green;
  this->blue = pData->blue;
  this->alpha = pData->alpha;

  this->delay = pData->delay;
  this->exposure = pData->exposure;

  this->QPC_before_trigger = pData->QPC_before_trigger;
  this->QPC_after_trigger = pData->QPC_after_trigger;

  return true;
}
/* QueuedEncoderImage::CopyMetadataFrom */



//! Copy data from buffer.
/*!
  Copies image data from input buffer.

  \param src_data       Pointer to buffer.
  \param src_dst_size   Size of the buffer in bytes.
  \param type   Image type.
  \param width  Image width.
  \param height Image height.
  \param stride Size of one image row in bytes.
  \return Function returns TRUE if successfull.
*/
BOOL
QueuedEncoderImage::CopyImageFrom(
                                  void const * const src_data,
                                  unsigned int const src_dst_size,
                                  ImageDataType const type,
                                  unsigned int const width,
                                  unsigned int const height,
                                  unsigned int const stride
                                  )
{
  if (NULL == src_data) return FALSE;

  BOOL result = TRUE;

  SAFE_FREE( this->data );
  QueuedEncoderImageBlankData_inline(this);

  // Allocate image storage.
  assert(NULL == this->data);
  this->data = malloc( src_dst_size );
  assert(NULL != this->data);

  if (NULL == this->data) result = FALSE;

  // Copy data.
  if (TRUE == result)
    {
      this->data_size = src_dst_size;
      this->data_type = type;
      this->data_width = width;
      this->data_height = height;
      this->data_stride = stride;

      assert(NULL != src_data);

      void * const dst = memcpy(this->data, src_data, src_dst_size);
      assert(dst == this->data);
    }
  /* if */

  return result;
}
/* QueuedEncoderImage::CopyImageFrom */



#ifdef HAVE_SAPERA_SDK

//! Copy data from Sapera Buffer.
/*!
  Copies image data from Sapera SapBuffer class.

  This function does not use Read and Write methods of SapBuffer class as they are too slow.

  \param src    Pointer to Sapera Buffer class.
  \param pAcqDevice Pointer to Sapera Acquisition Device class.
*/
BOOL
QueuedEncoderImage::CopyImageFrom(
                                  SapBuffer * const src,
                                  SapAcqDevice * const pAcqDevice
                                  )
{
  if (NULL == src) return FALSE;

  BOOL result = TRUE;

  SAFE_FREE( this->data );
  QueuedEncoderImageBlankData_inline(this);

  // Get image info.
  unsigned int const width = src->GetWidth();
  unsigned int const height = src->GetHeight();
  unsigned int const stride = src->GetPitch();

  unsigned int const src_size = width * height * src->GetBytesPerPixel();
  unsigned int const dst_size = stride * height * sizeof(BYTE);
  assert( src_size <= dst_size );

  // Allocate image storage.
  assert(NULL == this->data);
  this->data = malloc( dst_size );
  assert(NULL != this->data);

  if (NULL == this->data) result = FALSE;

  // Copy image metadata.
  if (TRUE == result)
    {
      this->data_size = dst_size;
      this->data_type = GetImageDataType(src, pAcqDevice);
      this->data_width = width;
      this->data_height = height;
      this->data_stride = stride;
    }
  /* if */

  // Acquire source data lock.
  void * src_data = NULL;
  if (TRUE == result)
    {
      result = src->GetAddress(&src_data);
      assert(TRUE == result);
      assert(NULL != src_data);
    }
  /* if */

  // Copy data and release source lock.
  if (TRUE == result)
    {
      assert(NULL != src_data);

      void * const dst = memcpy(this->data, src_data, dst_size);
      assert(dst == this->data);

      result = src->ReleaseAddress(src_data);
      assert(TRUE == result);
    }
  /* if */

  return result;

}
/* QueuedEncoderImage::CopyImageFrom */

#endif /* HAVE_SAPERA_SDK */



#ifdef HAVE_FLYCAPTURE2_SDK

//! Copy data from FlyCapture2 Image class.
/*!
  Copies image data from FlyCapture2 Image class.

  \param src    Pointer to image container class.
  \param pCamera Pointer to camera container class.
*/
BOOL
QueuedEncoderImage::CopyImageFrom(
                                  FlyCapture2::Image * const src,
                                  FlyCapture2::Camera * const pCamera
                                  )
{
  if (NULL == src) return FALSE;

  BOOL result = TRUE;

  SAFE_FREE( this->data );
  QueuedEncoderImageBlankData_inline(this);

  // Get image info.
  unsigned int const width = src->GetCols();
  unsigned int const height = src->GetRows();
  unsigned int const stride = src->GetStride();

  unsigned int const src_size = src->GetDataSize();
  unsigned int const dst_size = stride * height * sizeof(BYTE);
  assert( src_size == dst_size );

  // Allocate image storage.
  assert(NULL == this->data);
  this->data = malloc( dst_size );
  assert(NULL != this->data);

  if (NULL == this->data) result = FALSE;

  // Copy image metadata.
  if (TRUE == result)
    {
      this->data_size = dst_size;
      this->data_type = GetImageDataType(src, pCamera);
      this->data_width = width;
      this->data_height = height;
      this->data_stride = stride;
    }
  /* if */

  // Get source data.
  void const * const src_data = src->GetData();
  if (NULL == src_data) result = FALSE;

  // Copy image data.
  if (TRUE == result)
    {
      assert(NULL != src_data);

      void * const dst = memcpy(this->data, src_data, dst_size);
      assert(dst == this->data);
    }
  /* if */

  return result;
}
/* QueuedEncoderImage::CopyImageFrom */

#endif /* HAVE_FLYCAPTURE2_SDK */

#pragma endregion // Copy metadata and data


#pragma region // Get image data

//! Gets cv::Mat bitmap.
/*!
  Constructs 8-bit/16-bit/32-bit BGR bitmap.

  \return Returns pointer to cv::Mat or NULL if data conversion is not possible.
*/
cv::Mat *
QueuedEncoderImage::GetCVMat(
                             void
                             )
{
  return RawBufferToBGRcvMat(this->data_type, this->data_width, this->data_height, this->data_stride, this->data);
}
/* QueuedEncoderImage::GetCVMat */



//! Gets WIC bitmap.
/*!
  Constructs WIC bitmap from acquired image data.

  \param pIWICFactory   Pointer to WIC factory.
  \param ppBitmap Pointer to location where IWICBitmap pointer will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
QueuedEncoderImage::GetIWICBitmap(
                                  IWICImagingFactory * const pIWICFactory,
                                  IWICBitmap ** const ppBitmap
                                  )
{
  if (NULL != ppBitmap) *ppBitmap = NULL;

  assert(NULL != pIWICFactory);
  if (NULL == pIWICFactory) return E_INVALIDARG;

  assert(NULL != ppBitmap);
  if (NULL == ppBitmap) return E_INVALIDARG;

  assert(NULL != this->data);
  if (NULL == this->data) return E_POINTER;

  HRESULT hr = E_NOTIMPL;
  IWICBitmap * pBitmap = NULL;

  switch (this->data_type)
    {
    case IDT_UNKNOWN:
    case IDT_8U_BINARY:
      break;

    case IDT_8U_GRAY:
      {
        hr = ConvertMono8uToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_12U_GRAY_Packed:
      break;

    case IDT_16U_GRAY:
      {
        hr = ConvertMono16uToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_16U_GRAY_BigEndian:
    case IDT_32U_GRAY:
    case IDT_8S_GRAY:
    case IDT_16S_GRAY:
    case IDT_16S_GRAY_BigEndian:
    case IDT_32S_GRAY:
    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
    case IDT_12U_BayerGR_Packed:
    case IDT_12U_BayerRG_Packed:
    case IDT_12U_BayerGB_Packed:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerGR_BigEndian:
    case IDT_16U_BayerRG_BigEndian:
    case IDT_16U_BayerGB_BigEndian:
    case IDT_16U_BayerBG_BigEndian:
      break;

    case IDT_8U_RGB:
      {
        hr = ConvertRGB8ToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_8U_RGB_Planar:
    case IDT_8U_RGBA:
      break;

    case IDT_8U_BGR:
      {
        hr = ConvertBGR8ToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_16U_BGR:
    case IDT_8U_BGRA:
    case IDT_8U_YUV411:
      break;

    case IDT_8U_YUV422:
      {
        hr = ConvertYUV422ToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_8U_YUV422_BT601:
      {
        hr = ConvertYUV422BT601ToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_8U_YUV422_BT709:
      {
        hr = ConvertYUV422BT709ToBGR8(this->data_width, this->data_height, this->data_stride, this->data, pIWICFactory, &pBitmap);
        assert( SUCCEEDED(hr) );
      }
      break;

    case IDT_8U_YUV444:
    case IDT_8U_UYV444:
    default:
      break;
    }
  /* switch */

  if ( SUCCEEDED(hr) )
    {
      *ppBitmap = pBitmap;
    }
  else
    {
      SAFE_RELEASE( pBitmap );
    }
  /* if */

  return hr;
}
/* QueuedEncoderImage::GetIWICBitmap */

#pragma endregion // Get image data


#pragma region // Save to file

//! Store image to PNG file.
/*!
  Saves acquired image data to file.

  \param pDirectory     Directory where the data will be stored.
  \param pIWICFactory   Pointer to Windows Imaging Component (WIC) factory. May be NULL.
*/
bool
QueuedEncoderImage::StoreToPNGFile(
                                   std::wstring * const pDirectory,
                                   IWICImagingFactory * const pIWICFactory = NULL
                                   )
{
  //assert(NULL != this->pFilename);
  if (NULL == this->pFilename) return false;

  bool stored = false;


  // Create output filename sans extension.
  std::wstring URI = std::wstring(_T(""));
  if (NULL != pDirectory)
    {
      URI += *pDirectory;
      int const idx = (int)( URI.length() ) - 1;
      if ( (0 <= idx) && (_T('\\') != URI[idx]) ) URI += std::wstring(_T("\\"));
    }
  /* if */
  URI += *(this->pFilename);

  {
    int const idx = (int)(URI.length()) - 4;
    if ( (0 <= idx) && (_T('.') == URI[idx]) ) URI.erase(idx, 4);
  }


  // Note that default filename and directory name is stored as unicode string so it must be transcoded to work with ANSI C functions.
  char buffer[MAX_PATH + 1];
  buffer[MAX_PATH] = 0;
  bool ansi_valid = true;
  if (false == stored)
    {
      int const numch = WideCharToMultiByte(
                                            CP_ACP, // Transcode for ANSI fopen/fclose function.
                                            0,
                                            URI.c_str(), // Input string.
                                            -1,
                                            buffer, // Output string.
                                            MAX_PATH,
                                            NULL,
                                            NULL
                                            );
      assert( (0 < numch) && (numch < MAX_PATH) );

      if ( (0 >= numch) || (MAX_PATH <= numch) )
        {
          DWORD const error_code = GetLastError();
          assert(ERROR_SUCCESS == error_code);
          ansi_valid = false;
        }
      /* if */
    }
  /* if */


  // Try to convert to cv::Mat and store it.
  if ( (false == stored) && (true == ansi_valid) )
    {
      cv::Mat * pImage = this->GetCVMat();
      //assert(NULL != pImage);
      if (NULL != pImage)
        {
          char filename[MAX_PATH + 1];
          int i = 0;
          for (; buffer[i] && (i < MAX_PATH); ++i) filename[i] = buffer[i];
          if (i < MAX_PATH) filename[i++] = '.';
          if (i < MAX_PATH) filename[i++] = 'p';
          if (i < MAX_PATH) filename[i++] = 'n';
          if (i < MAX_PATH) filename[i++] = 'g';
          if (i < MAX_PATH) filename[i] = 0;
          assert(MAX_PATH + 1 > i);
          filename[MAX_PATH] = 0;

          bool const saved = cv::imwrite(filename, *(pImage));
          assert(true == saved);
          if (true == saved) stored = true;
        }
      /* if */

      SAFE_DELETE(pImage);
    }
  /* if */


  // Try to convet image to WIC bitmap and store it.
  if ( (false == stored) && (NULL != pIWICFactory) )
    {
      IWICBitmap * pBitmap = NULL;
      HRESULT hr = this->GetIWICBitmap(pIWICFactory, &pBitmap);
      //assert( SUCCEEDED(hr) );
      if ( SUCCEEDED(hr) )
        {
          std::wstring filename = URI + std::wstring(L".png");
          hr = ImageSaveToPNG(pIWICFactory, pBitmap, filename.c_str());
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) ) stored = true;
        }
      /* if */
      SAFE_RELEASE(pBitmap);
    }
  /* if */


#ifdef HAVE_FLYCAPTURE2_SDK
  if ( (false == stored) && (true == ansi_valid) )
    {
      // If not saved then use save function of FlyCapture2 SDK.
      // Use encapsulating buffer so image data is not copied.

      FlyCapture2::PixelFormat const pixel_format = GetFlyCapture2PixelFormat(this->data_type);
      FlyCapture2::BayerTileFormat const bayer_format = GetFlyCapture2BayerTileFormat(this->data_type);
      if (FlyCapture2::UNSPECIFIED_PIXEL_FORMAT != pixel_format)
        {
          FlyCapture2::Image * pImage = new FlyCapture2::Image(
                                                               this->data_height,
                                                               this->data_width,
                                                               this->data_stride,
                                                               (unsigned char *)( this->data ),
                                                               (unsigned int)( this->data_size ),
                                                               pixel_format,
                                                               bayer_format
                                                               );
          assert(NULL != pImage);
          if (NULL != pImage)
            {
              char filename[MAX_PATH + 1];
              int i = 0;
              for (; buffer[i] && (i < MAX_PATH); ++i) filename[i] = buffer[i];
              if (i < MAX_PATH) filename[i++] = '.';
              if (i < MAX_PATH) filename[i++] = 'p';
              if (i < MAX_PATH) filename[i++] = 'n';
              if (i < MAX_PATH) filename[i++] = 'g';
              if (i < MAX_PATH) filename[i] = 0;
              assert(MAX_PATH + 1 > i);
              filename[MAX_PATH] = 0;

              FlyCapture2::Error error;

              if (FlyCapture2::NONE == bayer_format)
                {
                  error = pImage->Save(filename);
                  assert(error == FlyCapture2::PGRERROR_OK);
                  if (error == FlyCapture2::PGRERROR_OK) stored = true;
                }
              else
                {
                  FlyCapture2::Image pImageBGR;

                  error = pImage->SetColorProcessing(FlyCapture2::RIGOROUS);
                  assert(error == FlyCapture2::PGRERROR_OK);

                  error = pImage->Convert(FlyCapture2::PIXEL_FORMAT_BGR, &pImageBGR);
                  assert(error == FlyCapture2::PGRERROR_OK);

                  error = pImageBGR.Save(filename);
                  assert(error == FlyCapture2::PGRERROR_OK);
                  if (error == FlyCapture2::PGRERROR_OK) stored = true;
                }
              /* if */
            }
          /* if */

          SAFE_DELETE(pImage);
        }
      /* if */
    }
  /* if */
#endif /* HAVE_FLYCAPTURE2_SDK */


#ifdef HAVE_SAPERA_SDK
  if ( (false == stored) && (true == ansi_valid) )
    {
      // If not saved then use save function of Sapera LT.
      // We use an encapsulating buffer so image data is not copied, only SapBuffer object is created.

      SapFormat const pixel_format = GetSaperaPixelFormat(this->data_type);
      if (SapFormatUnknown != pixel_format)
        {
          void * virtAddress[1];
          virtAddress[0] = this->data;

          SapBuffer * pBuffer = new SapBuffer(1, virtAddress, this->data_width, this->data_height, pixel_format, SapBuffer::TypeVirtual);
          SafeCreate<SapBuffer>( pBuffer );

          if ( (NULL != pBuffer) && (pBuffer->GetPitch() == this->data_stride) )
            {
              char filename[MAX_PATH + 1];
              int i = 0;
              for (; buffer[i] && (i < MAX_PATH); ++i) filename[i] = buffer[i];
              if (i < MAX_PATH) filename[i++] = '.';
              if (i < MAX_PATH) filename[i++] = 'b';
              if (i < MAX_PATH) filename[i++] = 'm';
              if (i < MAX_PATH) filename[i++] = 'p';
              if (i < MAX_PATH) filename[i] = 0;
              assert(MAX_PATH + 1 > i);
              filename[MAX_PATH] = 0;

              BOOL const save = pBuffer->Save(filename, "-format bmp");
              assert(TRUE == save);
              if (TRUE == save) stored = true;
            }
          /* if */

          SafeDestroy<SapBuffer>( pBuffer );
        }
      /* if */
    }
  /* if */
#endif /* HAVE_SAPERA_SDK */


  return stored;
}
/* QueuedEncoderImage::StoreToPNGFile */



//! Store image raw data.
/*!
  Saves acquired image data to raw file.

  \param pDirectory     Directory where the data will be stored.
*/
bool
QueuedEncoderImage::StoreToRawFile(
                                   std::wstring * const pDirectory
                                   )
{
  //assert(NULL != this->pFilename);
  if (NULL == this->pFilename) return false;

  assert(NULL != this->data);
  if (NULL == this->data) return false;

  bool stored = false;


  // Create output filename sans extension.
  std::wstring URI = std::wstring(_T(""));
  if (NULL != pDirectory)
    {
      URI += *pDirectory;
      int const idx = (int)( URI.length() ) - 1;
      if ( (0 <= idx) && (_T('\\') != URI[idx]) ) URI += std::wstring(_T("\\"));
    }
  /* if */
  URI += *(this->pFilename);

  {
    int const idx = (int)(URI.length()) - 4;
    if ( (0 <= idx) && (_T('.') == URI[idx]) ) URI.erase(idx, 4);
  }

  std::wstring filename_raw = URI + L".raw";
  FILE * FP_raw = NULL;
  errno_t const open_raw = _wfopen_s(&FP_raw, filename_raw.c_str(), L"wb");
  assert( (0 == open_raw) && (NULL != FP_raw) );
  if (NULL != FP_raw)
    {
      size_t const file_size = fwrite(this->data, this->data_size, 1, FP_raw);
      assert(1 == file_size);
      stored = (1 == file_size);
      fclose(FP_raw);
      FP_raw = NULL;
    }
  /* if */

  if (true == stored)
    {
      std::wstring filename_xml = URI + L".xml";
      FILE * FP_xml = NULL;
      errno_t const open_xml = _wfopen_s(&FP_xml, filename_xml.c_str(), L"w, ccs=UTF-8");
      assert( (0 == open_xml) && (NULL != FP_xml) );
      if (NULL != FP_xml)
        {
          wchar_t const * const data_type = StringFromImageDataType_inline(this->data_type);
          int const numc =
            fwprintf(
                     FP_xml,
                     L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                     L"<ImageMetadata xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
                     L"  <BufferSize>%u</BufferSize>\n"
                     L"  <PixelFormat>%s</PixelFormat>\n"
                     L"  <Width>%u</Width>\n"
                     L"  <Height>%u</Height>\n"
                     L"  <Stride>%u</Stride>\n"
                     L"  <QPCBeforeTrigger>%I64d</QPCBeforeTrigger>\n"
                     L"  <QPCAfterTrigger>%I64d</QPCAfterTrigger>\n"
                     L"</ImageMetadata>",
                     this->data_size, data_type,
                     this->data_width, this->data_height,
                     this->data_stride,
                     this->QPC_before_trigger, this->QPC_after_trigger
                     );
          assert(0 < numc);
          fclose(FP_xml);
          FP_xml = NULL;
        }
      /* if */
    }
  /* if */

  return stored;
}
/* QueuedEncoderImage::StoreToRawFile */

#pragma endregion // Save to file


#pragma endregion // Image encoder queue item


/****** IMAGE ENCODER THREAD ******/

//! Image encoder thread.
/*!
  Image encoder thread.

  \param parameters_in Pointer to structure holding image encoder thread parameters.
  \return Returns 0 if successfull.
*/
unsigned int
__stdcall
ImageEncoderThread(
                   void * parameters_in
                   )
{

#pragma region // Initialization

  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  ImageEncoderParameters * const parameters = (ImageEncoderParameters *)parameters_in;

  SetThreadNameAndIDForMSVC(-1, "ImageEncoderThread", parameters->EncoderID);

  SynchronizationEvents * const pSynchronization = parameters->pSynchronization;
  assert(NULL != pSynchronization);

  int EncoderID = parameters->EncoderID;
  assert( (0 <= EncoderID) && (EncoderID < (int)(pSynchronization->ImageEncoder.size())) );

  int CameraID = parameters->CameraID;
  assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );

  PastEvents * const pEvents = PastEventsCreate();
  assert(NULL != pEvents);

  // Initialize variables.
  bool continueLoop = true;

  int count = 0; // Number of items in the encoder queue.

  // Prepare image queue list.
  int const c0 = ImageEncoderEmptyQueue(parameters);
  assert( c0 == 0 );
  count += c0;

  // Lower thread priority.
  BOOL const priority = SetThreadPriority(parameters->tImageEncoder, THREAD_PRIORITY_LOWEST);
  assert( TRUE == priority );

  parameters->fActive = true;

#pragma endregion // Initialization

  /* Events are processed in an infinite loop. */
  do
    {
      assert(NULL != pSynchronization);
      if (NULL != pSynchronization)
        {
          assert(false == parameters->fWaiting);
          parameters->fWaiting = true;

          DWORD const dwWaitResult =
            pSynchronization->EventWaitForAny(
                                              IMAGE_ENCODER_QUEUE_TERMINATE, EncoderID, // 0
                                              IMAGE_ENCODER_QUEUE_PROCESS,   EncoderID, // 1
                                              IMAGE_ENCODER_CHANGE_ID,       EncoderID, // 2
                                              INFINITE // Wait forever.
                                              );
          int const hnr = dwWaitResult - WAIT_OBJECT_0;
          assert( (0 <= hnr) && (hnr < 3) );
          AddEvent(pEvents, hnr);

          parameters->fWaiting = false;

          if (0 == hnr)
            {
              // We received terminate event.
              continueLoop = false;
            }
          else if (1 == hnr)
            {
              // Process items.
              int const c1 = ImageEncoderEmptyQueue(parameters);
              count += c1;
              assert(count <= parameters->count);

              // Reset processing signal only after the processing is done.
              BOOL const reset_process = pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_PROCESS, EncoderID);
              assert(0 != reset_process);
            }
          else if (2 == hnr)
            {
              // Store old event ID.
              int const EncoderIDOld = EncoderID;

              // Output change ID message.
              if (EncoderIDOld != parameters->EncoderID)
                {
                  Debugfprintf(stderr, gDbgImageEncoderIDChanged, EncoderIDOld + 1, EncoderIDOld + 1, parameters->EncoderID + 1);

                  SetThreadNameAndIDForMSVC(-1, "ImageEncoderThread", parameters->EncoderID);
                }
              else
                {
                  Debugfprintf(stderr, gDbgImageEncoderIDNotChanged, EncoderIDOld + 1);
                }
              /* if */

              // Fetch new event ID values.
              {
                EncoderID = parameters->EncoderID;
                assert( (0 <= EncoderID) && (EncoderID < (int)(pSynchronization->ImageEncoder.size())) );

                CameraID = parameters->CameraID;
                assert( (0 <= CameraID) && (CameraID < (int)(pSynchronization->Camera.size())) );
              }

              // Set new output directory.
              {
                std::wstring * pSubdirectoryCameraOld = NULL;

                std::wstring * pSubdirectoryCameraNew = new std::wstring( std::to_wstring((long long)(CameraID)) );
                assert(NULL != pSubdirectoryCameraNew);

                AcquireSRWLockExclusive( &(parameters->sLockDirectory) );
                {
                  pSubdirectoryCameraOld = parameters->pSubdirectoryCamera;
                  assert(NULL != pSubdirectoryCameraOld);

                  if (NULL != pSubdirectoryCameraNew)
                    {
                      parameters->pSubdirectoryCamera = pSubdirectoryCameraNew;
                    }
                  else
                    {
                      parameters->pSubdirectoryCamera = NULL;
                    }
                  /* if */
                }
                ReleaseSRWLockExclusive( &(parameters->sLockDirectory) );

                SAFE_DELETE(pSubdirectoryCameraOld);
              }

              // Reset signal; note that we have to use the old ID.
              {
                BOOL const reset_change_id = pSynchronization->EventReset(IMAGE_ENCODER_CHANGE_ID, EncoderIDOld);
                assert(0 != reset_change_id);
              }
            }
          else
            {
              // We received unknown event!
            }
          /* if */
        }
      else
        {
          continueLoop = false;
        }
      /* if */
    }
  while ( true == continueLoop );

  PastEventsDelete( pEvents );

  {
    BOOL const reset_terminate = pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_TERMINATE, EncoderID);
    assert(0 != reset_terminate);
  }

  parameters->fActive = false;

  return 0;
}
/* ImageEncoderThread */



/****** ENCODER QUEUE ******/

#pragma region // Fetch and release inline functions

//! Fetches image to encode.
/*!
  Fetches next image from the image encoder queue.

  \param P      Pointer to structure holding image encoder thread parameters.
  \param acquire_SRW_lock Flag to indicate if lock must be acquired.
  If true the function will wait on SRW lock; otherwise it assumes lock is already acquired.
  \return Returns pointer to image container class or NULL if unsuccessfull.
*/
inline
QueuedEncoderImage *
ImageEncoderFetchImage_inline(
                              ImageEncoderParameters * const P,
                              bool const acquire_SRW_lock
                              )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return NULL;

  QueuedEncoderImage * item = NULL;
  int size = 0;
  bool empty = true;

  if (true == acquire_SRW_lock) AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        // Fetch image from queue.
        size = (int)( P->pImageQueue->size() );
        if (0 < size)
          {
            item = P->pImageQueue->front();
            P->pImageQueue->erase( P->pImageQueue->begin() );
          }
        /* if */
        empty = P->pImageQueue->empty();
      }
    /* if */

    // Update batch item counter.
    if ( (NULL != item) && (true == item->is_batch) )
      {
        P->num_batch -= 1;
      }
    /* if */
  }
  if (true == acquire_SRW_lock) ReleaseSRWLockExclusive( &(P->sLockImageQueue) );


  if (NULL != P->pSynchronization)
    {
      if (true == empty)
        {
          // Signal the queue is empty and reset process signal.
          BOOL const set_empty = P->pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_EMPTY, P->EncoderID);
          assert(0 != set_empty);

          BOOL const reset_process = P->pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_PROCESS, P->EncoderID);
          assert(0 != reset_process);
        }
      /* if */

      if (size < P->minItems)
        {
          // If the number of items goes below a set minimum then reset processing signal.
          BOOL const reset_process = P->pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_PROCESS, P->EncoderID);
          assert(0 != reset_process);
        }
      /* if */

      if (size < P->maxItems)
        {
          // Signal the queue is no longer full.
          BOOL const reset_full = P->pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_FULL, P->EncoderID);
          assert(0 != reset_full);
        }
      /* if */
    }
  /* if */

  return item;
}
/* ImageEncoderFetchImage_inline */



//! Releases image encoder thread parameters structure.
/*!
  Releases resources allocated by image encoder thread.

  \param P      Pointer to image encoder parametes.
*/
inline
void
ImageEncoderParametersRelease_inline(
                                     ImageEncoderParameters * const P
                                     )
{
  assert(NULL != P);
  if (NULL == P) return;

  AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        while ( !P->pImageQueue->empty() )
          {
            QueuedEncoderImage * item = ImageEncoderFetchImage_inline(P, false);
            SAFE_DELETE( item );
          }
        /* while */
        assert( P->pImageQueue->empty() );
      }
    /* if */

    SAFE_DELETE(P->pImageQueue);
    assert(NULL == P->pImageQueue);
  }
  ReleaseSRWLockExclusive( &(P->sLockImageQueue) );

  AcquireSRWLockExclusive( &(P->sLockImageData) );
  {
    SAFE_DELETE(P->pAllImages);
    SAFE_DELETE(P->pStatistics);
  }
  ReleaseSRWLockExclusive( &(P->sLockImageData) );

  AcquireSRWLockExclusive( &(P->sLockDirectory) );
  {
    SAFE_DELETE(P->pDirectoryData);
    SAFE_DELETE(P->pSubdirectorySession);
    SAFE_DELETE(P->pSubdirectoryRecording);
    SAFE_DELETE(P->pSubdirectoryCamera);
  }
  ReleaseSRWLockExclusive( &(P->sLockDirectory) );

  ImageEncoderParametersBlank_inline( P );

  free(P);
}
/* ImageEncoderParametersRelease_inline */

#pragma endregion // Fetch and release inline functions


#pragma region // Empty image encoder queue

//! Fetches image to encode.
/*!
  Fetches next image from the image encoder queue.

  \param P      Pointer to structure holding image encoder thread parameters.
  \param acquire_SRW_lock Flag to indicate if lock must be acquired.
  If true the function will wait on SRW lock; otherwise it assumes lock is already acquired.
  \return Returns pointer to image container class or NULL if unsuccessfull.
*/
QueuedEncoderImage *
ImageEncoderFetchImage(
                       ImageEncoderParameters * const P,
                       bool const acquire_SRW_lock
                       )
{
  return ImageEncoderFetchImage_inline(P, acquire_SRW_lock);
}
/* ImageEncoderFetchImage */



//! Empty image queue.
/*!
  Stores queued images to files.

  \param P   Pointer to image encoder thread parameters.
  \return Number of items removed from the queue.
*/
int
ImageEncoderEmptyQueue(
                       ImageEncoderParameters * const P
                       )
{
  int num_deleted = 0;

  assert(NULL != P);
  if (NULL == P) return num_deleted;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return num_deleted;

  bool empty = false;
  do
    {
      QueuedEncoderImage * item = ImageEncoderFetchImage_inline(P, true);
      if (NULL != item)
        {
          assert( (item->CameraID == P->CameraID) != (-1 == item->CameraID) );

          /****** PROCESSING ******/

          // Only process images acquired during batch acquisition.
          IWICBitmap * pBitmap = NULL;

          if (true == item->is_batch)
            {
              if (NULL != item->data)
                {
                  int const nFrames = 34;

                  // Reallocate storage if needed.
                  AcquireSRWLockExclusive( &(P->sLockImageData) );
                  {
                    bool const reallocate = P->pAllImages->Reallocate(
                                                                      nFrames,
                                                                      item->data_width,
                                                                      item->data_height,
                                                                      item->data_stride,
                                                                      item->data_size,
                                                                      item->data_type
                                                                      );
                    assert(true == reallocate);
                  }
                  ReleaseSRWLockExclusive( &(P->sLockImageData) );

                  int const slot = item->index;
                  if ( (0 <= slot) && (slot < nFrames) )
                    {
                      AcquireSRWLockExclusive( &(P->sLockImageData) );
                      {
                        bool const insert = P->pAllImages->AddImage(
                                                                    slot,
                                                                    item->data_width,
                                                                    item->data_height,
                                                                    item->data_stride,
                                                                    item->data_size,
                                                                    item->data_type,
                                                                    item->data
                                                                    );
                        assert(true == insert);
                      }
                      ReleaseSRWLockExclusive( &(P->sLockImageData) );
                    }
                  /* if */
                }
              /* if */

            }
          /* if */

          // Save image to disk if required.
          if ( (true == item->is_batch) && (true == item->save) )
            {
              std::wstring * pDirectory = ImageEncoderGetOutputDirectory(P, true, false);
              assert(NULL != pDirectory);

              if (true == item->save_to_raw)
                {
                  bool const store_raw = item->StoreToRawFile(pDirectory);
                  assert(true == store_raw);
                }
              /* if */

              if (true == item->save_to_png)
                {
                  bool const store_png = item->StoreToPNGFile(pDirectory, P->pWICFactory);
                  assert(true == store_png);

                  if ( (NULL != pBitmap) && (false == store_png) )
                    {
                      // Create output filename sans extension.
                      std::wstring URI = std::wstring(_T(""));
                      if (NULL != pDirectory)
                        {
                          URI += *pDirectory;
                          int const idx = (int)( URI.length() ) - 1;
                          if ( (0 <= idx) && (_T('\\') != URI[idx]) ) URI += std::wstring(_T("\\"));
                        }
                      /* if */
                      URI += *(item->pFilename);

                      {
                        int const idx = (int)(URI.length()) - 4;
                        if ( (0 <= idx) && (_T('.') == URI[idx]) ) URI.erase(idx, 4);
                      }

                      URI += std::wstring(L".png");

                      HRESULT const hr = ImageSaveToPNG(P->pWICFactory, pBitmap, URI.c_str());
                      assert( SUCCEEDED(hr) );
                    }
                  /* if */
                }
              /* if */

              SAFE_DELETE( pDirectory );
            }
          /* if */

          SAFE_RELEASE( pBitmap );
          SAFE_DELETE( item );
          ++num_deleted;
        }
      else
        {
          empty = true;
        }
      /* if */
      assert(NULL == item);
    }
  while (false == empty);

  return num_deleted;
}
/* ImageEncoderEmptyQueue */

#pragma endregion // Empty image encoder queue


#pragma region // Push image to encoder queue

//! Queues next image to store.
/*!
  Queues next image to store to file.

  \param P      Pointer to image encoder thread parameters.
  \param item     Pointer to image to queue.
  \return Number of queued images, or a negative number if unsuccessfull.
*/
bool
ImageEncoderQueueImage(
                       ImageEncoderParameters * const P,
                       QueuedEncoderImage * const item
                       )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != item);
  if (NULL == item) return false;

  assert( (item->CameraID == P->CameraID) != (-1 == item->CameraID) );

  int size = 0;
  bool empty = true;

  AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        // Insert item.
        int const count = (P->count)++;
        item->count = count;
        P->pImageQueue->push_back( item );

        // Update counters.
        if (true == item->is_batch) P->num_batch += 1;

        // Get queue state.
        size = (int)( P->pImageQueue->size() );
        empty = P->pImageQueue->empty();
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockImageQueue) );


  if (NULL != P->pSynchronization)
    {
      if ( (false == empty) && (size >= P->minItems) )
        {
          // If the queue has enough items signal the processing may start.
          BOOL const set_process = P->pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_PROCESS, P->EncoderID);
          assert(0 != set_process);
        }
      /* if */

      if (size >= P->maxItems)
        {
          // If the queue has more than the preset maximum of items then signal the queue is full.
          // Producer threads may use this signal to adjust the production speed.
          BOOL const set_full = P->pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_FULL, P->EncoderID);
          assert(0 != set_full);

          BOOL const set_process = P->pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_PROCESS, P->EncoderID);
          assert(0 != set_process);
        }
      /* if */

      if (false == empty)
        {
          BOOL const reset_empty = P->pSynchronization->EventReset(IMAGE_ENCODER_QUEUE_EMPTY, P->EncoderID);
          assert(0 != reset_empty);
        }
      /* if */
    }
  /* if  */

  return true;
}
/* ImageEncoderQueueImage*/

#pragma endregion // Push image to encoder queue


#pragma region // Test status and count items

//! Count remaining items from batch.
/*!
  Encoder queue may be quite slow.
  This function returns number of queueud images having batch flag set.

  \param P      Pointer to image encoder thread parameters.
  \return Number of items in the queue having is_batch flag set to true.
*/
int
ImageEncoderBatchItemsRemaining(
                                ImageEncoderParameters * const P
                                )
{
  assert(NULL != P);
  if (NULL == P) return 0;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return 0;

  int num_batch = 0;
  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    num_batch = P->num_batch;
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  return num_batch;
}
/* ImageEncoderBatchItemsRemaining */



//! Count total remaining items.
/*!
  Encoder queue may be quite slow.
  This function returns total number of queueud images.

  \param P      Pointer to image encoder thread parameters.
  \return Number of items in the queue.
*/
int
ImageEncoderTotalItemsRemaining(
                                ImageEncoderParameters * const P
                                )
{
  assert(NULL != P);
  if (NULL == P) return 0;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return 0;

  int size = 0;
  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue) size = (int)( P->pImageQueue->size() );
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  return size;
}
/* ImageEncoderTotalItemsRemaining */

#pragma endregion // Test status and count items


#pragma region // Get next item metadata

//! Get image dimensions.
/*!
  Gets image dimensions of the first image in the image encoder queue.

  \param P      Pointer to image encoder thread parameters.
  \param width  Width of the acquired image.
  \param height Height of the acquired image.
  \return True if successfull, false otherwise.
*/
bool
ImageEncoderGetImageDimensions(
                               ImageEncoderParameters * const P,
                               int * const width,
                               int * const height
                               )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return false;

  bool have_image = false;

  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        int const size = (int)( P->pImageQueue->size() );
        if (0 < size)
          {
            QueuedEncoderImage const * const item = P->pImageQueue->front();
            if (NULL != item->data)
              {
                have_image = true;
                if (NULL != width) *width = (int)( item->data_width );
                if (NULL != height) *height = (int)( item->data_height );
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  return have_image;
}
/* ImageEncoderGetImageDimensions */

#pragma endregion // Get next item metadata



/****** START/STOP THREAD ******/

#pragma region // Encoder start and stop

//! Create image encoder parameters and start image encoder thread.
/*!
  Spawns image encoder thread.

  \param pSynchronization       Pointer to a structure holding all required syncrhonization events.
  \param pWICFactory   Pointer to Window Image Component factory.
  \param EncoderID Unique thread identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \param CameraID Unique camera identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \return Returns pointer to image encoder thread parameters or NULL if unsuccessfull.
*/
ImageEncoderParameters *
ImageEncoderStart(
                  SynchronizationEvents * const pSynchronization,
                  IWICImagingFactory * const pWICFactory,
                  int const EncoderID,
                  int const CameraID
                  )
{
  ImageEncoderParameters * const P = (ImageEncoderParameters *)malloc( sizeof(ImageEncoderParameters) );
  assert(NULL != P);
  if (NULL == P) return P;

  ImageEncoderParametersBlank_inline( P );

  /* Initialize variables. */
  InitializeSRWLock( &(P->sLockImageQueue) );
  InitializeSRWLock( &(P->sLockImageData) );
  InitializeSRWLock( &(P->sLockDirectory) );

  assert(NULL == P->pImageQueue);
  P->pImageQueue = new std::vector<QueuedEncoderImage *>();
  assert(NULL != P->pImageQueue);

  if (NULL == P->pImageQueue) goto IMAGE_ENCODER_THREAD_START_EXIT;

  assert(NULL == P->pAllImages);
  P->pAllImages = new ImageSet();
  assert(NULL != P->pAllImages);

  if (NULL == P->pAllImages) goto IMAGE_ENCODER_THREAD_START_EXIT;

  assert(NULL == P->pSubdirectoryCamera);
  P->pSubdirectoryCamera = new std::wstring( std::to_wstring((long long)(CameraID)) );
  assert(NULL != P->pSubdirectoryCamera);

  if (NULL == P->pSubdirectoryCamera) goto IMAGE_ENCODER_THREAD_START_EXIT;

  /* Copy parameters. */
  assert(NULL == P->pSynchronization);
  P->pSynchronization = pSynchronization;
  assert(NULL != P->pSynchronization);

  assert(NULL == P->pWICFactory);
  P->pWICFactory = pWICFactory;
  assert(NULL != P->pWICFactory);

  assert(-1 == P->EncoderID);
  P->EncoderID = EncoderID;
  assert( (0 <= P->EncoderID) && (P->EncoderID < (int)(P->pSynchronization->ImageEncoder.size())) );

  assert(-1 == P->CameraID);
  P->CameraID = CameraID;
  P->pAllImages->CameraID = CameraID;
  assert( (0 <= P->CameraID) && (P->CameraID < (int)(P->pSynchronization->Camera.size())) );

  /* Reserve space for 64 elements. */
  P->pImageQueue->reserve(64);

  /* Start image encoder thread. */
  P->tImageEncoder =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             ImageEncoderThread,
                             (void *)( P ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != P->tImageEncoder );
  if ( (HANDLE)( NULL ) == P->tImageEncoder )
    {

    IMAGE_ENCODER_THREAD_START_EXIT:
      ImageEncoderParametersRelease_inline( P );
      return NULL;

    }
  /* if */

  return P;
}
/* ImageEncoderStart */



//! Stop image encoder thread.
/*!
  Stops image encoder.

  \param P      Pointer to image encoder parameters.
*/
void
ImageEncoderStop(
                 ImageEncoderParameters * const P
                 )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SynchronizationEvents * const pSynchronization = P->pSynchronization;
  int const EncoderID = P->EncoderID;

  assert(NULL != pSynchronization);
  if (NULL != pSynchronization)
    {
      DWORD const result = WaitForSingleObject(P->tImageEncoder, 0);

      if ( (WAIT_OBJECT_0 != result) && (true == P->fActive) )
        {
          // The thread is alive so signal terminate event and wait for confirmation.
          BOOL const set_terminate = pSynchronization->EventSet(IMAGE_ENCODER_QUEUE_TERMINATE, EncoderID);
          assert(0 != set_terminate);
          if (0 != set_terminate)
            {
              DWORD const confirm = WaitForSingleObject(P->tImageEncoder, INFINITE);
              assert(WAIT_OBJECT_0 == confirm);
            }
          /* if */
        }
      else
        {
          // The thread has already terminated.
        }
      /* if */
    }
  /* if */

  assert( WAIT_OBJECT_0 == WaitForSingleObject(P->tImageEncoder, 0) );
  assert( false == P->fActive );

  ImageEncoderParametersRelease_inline( P );

  if (NULL != pSynchronization)
    {
      BOOL const reset_encoder = pSynchronization->EventResetAllImageEncoder(EncoderID);
      assert(0 != reset_encoder);
    }
  /* if */
}
/* ImageEncoderStop */

#pragma endregion // Encoder start and stop


#pragma region // Configuration of output directory

//! Set output data directory.
/*!
  Sets data directory where recorded data will be stored.

  This function tries to set the data directory to the supplied directory name.
  However, if the supplied name is not a proper directory then an UI
  dialog is open and the user is queried to select the output directory.
  Directory is never created if it does not exist.

  \param P      Pointer to image encoder parameters.
  \param directory      Image directory.
  \param title          Dialog title. May be NULL.
  \return Returns true if successfull.
*/
bool
ImageEncoderSetDirectory(
                         ImageEncoderParameters * const P,
                         wchar_t const * const directory,
                         wchar_t const * const title
                         )
{
  //assert(NULL != P);
  if (NULL == P) return false;

  bool initialized = false;

  // Assume requested directory is valid.
  std::wstring directory_in_out;
  directory_in_out.reserve(MAX_PATH + 1);
  if (NULL != directory)
    {
      directory_in_out = std::wstring(directory);
    }
  /* if */

  // Set dialog title.
  wchar_t const * pszTitle = title;
  if (NULL == pszTitle)
    {
      pszTitle = gMsgImageEncoderSetDataDirectory;
    }
  /* if */

  // Query user for a valid directory if the input directory is invalid.
  if ( FALSE == PathIsDirectory(directory_in_out.c_str()) )
    {
      /* Query the user to select input directory if none is
         given or if given directory does not exist.
      */
      AcquireSRWLockShared( &(P->sLockDirectory) );
      {
        if ( (NULL == directory) && (NULL != P->pDirectoryData) )
          {
            directory_in_out = *(P->pDirectoryData);
          }
        /* if */
      }
      ReleaseSRWLockShared( &(P->sLockDirectory) );

      HRESULT const hr = SelectFolderDialog(directory_in_out, pszTitle);
      assert( SUCCEEDED(hr) != (0x800704C7 == hr) );
    }
  /* if */

  // If successfull store new directory name.
  if (FALSE != PathIsDirectory(directory_in_out.c_str()))
    {
      AcquireSRWLockExclusive( &(P->sLockDirectory) );
      {
        if (NULL == P->pDirectoryData) P->pDirectoryData = new std::wstring(_T(""));
        if (NULL != P->pDirectoryData) *(P->pDirectoryData) = directory_in_out;
      }
      ReleaseSRWLockExclusive( &(P->sLockDirectory) );

      initialized = true;
    }
  /* if */

  return initialized;
}
/* ImageEncoderSetDirectory */



//! Set output data directory.
/*!
  Sets output data directory where recorded data will be stored.

  It differs from ImageEncoderSetDirectory function in behaviour:
  if the supplied string is not a valid directory then the function will
  not display an UI for the user to select a valid directory.

  \param P      Pointer to image encoder parameters.
  \param directory      Image directory.
  \return Returns true if successfull.
*/
bool
ImageEncoderTrySetDirectory(
                            ImageEncoderParameters * const P,
                            wchar_t const * const directory
                            )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != directory);
  if (NULL == directory) return false;

  if ( FALSE == PathIsDirectory(directory) ) return false;

  return ImageEncoderSetDirectory(P, directory, NULL);
}
/* ImageEncoderTrySetDirectory */



//! Get data directory.
/*!
  Returns output directory.

  The returned pointer is valid until next call to
  ImageEncoderSetDirectory or to ImageEncoderTrySetDirectory.

  \param P      Pointer to image encoder parameters.
  \return Returns pointer to directory name or NULL if unsuccessfull.
  Pointer is valid until next call to ImageEncoderSetDirectory.
*/
wchar_t const *
ImageEncoderGetDirectory(
                         ImageEncoderParameters * const P
                         )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  wchar_t const * directory = NULL;
  AcquireSRWLockShared( &(P->sLockDirectory) );
  {
    if (NULL != P->pDirectoryData) directory = P->pDirectoryData->c_str();
  }
  ReleaseSRWLockShared( &(P->sLockDirectory) );

  return directory;
}
/* ImageEncoderGetDirectory */



//! Set session subdirectory.
/*!
  Sets session subdirectory.

  \param P      Pointer to image encoder parameters.
  \param pSubdirectorySession   Pointer to string storing session name or NULL pointer if the session is unnamed.
  \return Returns true if successfull, false otherwise.
*/
bool
ImageEncoderSetSubdirectorySession(
                                   ImageEncoderParameters * const P,
                                   std::wstring * const pSubdirectorySession
                                   )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const have_src = (NULL != pSubdirectorySession);
  bool const have_dst = (NULL != P->pSubdirectorySession);

  std::wstring * ptr = NULL;
  if ( (true == have_src) && (false == have_dst) )
    {
      ptr = new std::wstring(_T(""));
      assert(NULL != ptr);
      ptr->reserve(MAX_PATH);
    }
  /* if */

  AcquireSRWLockExclusive( &(P->sLockDirectory) );
  {
    if (true == have_src)
      {
        if (NULL == P->pSubdirectorySession)
          {
            assert( (false == have_dst) && (NULL != ptr) );
            P->pSubdirectorySession = ptr;
            ptr = NULL;
          }
        /* if */

        assert(NULL != P->pSubdirectorySession);
        if (NULL != P->pSubdirectorySession) *(P->pSubdirectorySession) = *pSubdirectorySession;
      }
    else
      {
        assert(NULL == ptr);
        ptr = P->pSubdirectorySession;
        P->pSubdirectorySession = NULL;
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockDirectory) );

  SAFE_DELETE( ptr );

  return true;
}
/* ImageEncoderSetSubdirectorySession */



//! Get session subdirectory.
/*!
  Returns the name of session subdirectory where recordings will be stored.

  Once it is no longer needed the returned string should be destroyed by delete operator.

  \param P      Pointer to image encoder parameters.
  \return Returns pointer to a string containing sub directory name or NULL pointer if directory does not exist or if the function is unsuccessfull.
*/
std::wstring *
ImageEncoderGetSubdirectorySession(
                                   ImageEncoderParameters * const P
                                   )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  std::wstring * pSubdirectorySession = new std::wstring();
  assert(NULL != pSubdirectorySession);
  if (NULL == pSubdirectorySession) return pSubdirectorySession;

  pSubdirectorySession->reserve(MAX_PATH);

  AcquireSRWLockShared( &(P->sLockDirectory) );
  {
    if (NULL != P->pSubdirectorySession)
      {
        *pSubdirectorySession += *(P->pSubdirectorySession);
      }
    else
      {
        SAFE_DELETE(pSubdirectorySession);
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockDirectory) );

  return pSubdirectorySession;
}
/* ImageEncoderGetSubdirectorySession */



//! Set recording subdirectory.
/*!
  Sets active recording subdirectory to the given string.

  \param P      Pointer to image encoder parameters.
  \param pSubdirectoryRecording  Pointer to string storing subdirectory name.
  \return Returns true if successfull.
*/
bool
ImageEncoderSetSubdirectoryRecording(
                                     ImageEncoderParameters * const P,
                                     std::wstring * const pSubdirectoryRecording
                                     )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const have_src = (NULL != pSubdirectoryRecording);
  bool const have_dst = (NULL != P->pSubdirectoryRecording);

  std::wstring * ptr = NULL;
  if ( (true == have_src) && (false == have_dst) )
    {
      ptr = new std::wstring(_T(""));
      assert(NULL != ptr);
      ptr->reserve(MAX_PATH);
    }
  /* if */

  AcquireSRWLockExclusive( &(P->sLockDirectory) );
  {
    if (true == have_src)
      {
        if (NULL == P->pSubdirectoryRecording)
          {
            assert( (false == have_dst) && (NULL != ptr) );
            P->pSubdirectoryRecording = ptr;
            ptr = NULL;
          }
        /* if */

        assert(NULL != P->pSubdirectoryRecording);
        if (NULL != P->pSubdirectoryRecording) *(P->pSubdirectoryRecording) = *pSubdirectoryRecording;
      }
    else
      {
        assert(NULL == ptr);
        ptr = P->pSubdirectoryRecording;
        P->pSubdirectoryRecording = NULL;
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockDirectory) );

  SAFE_DELETE( ptr );

  return true;
}
/* ImageEncoderSetSubdirectoryRecording */



//! Append to recording subdirectory.
/*!
  Appends tag to recording subdirectory name.

  \param P      Pointer to image encoder parameters.
  \param tag    Pointer to tag to append.
  \return Returns true if successfull.
*/
bool
ImageEncoderAppendToSubdirectoryRecording(
                                          ImageEncoderParameters * const P,
                                          std::wstring * const tag
                                          )
{
  assert(NULL != P);
  if (NULL == P) return false;

  bool const have_src = (NULL != tag);
  bool const have_dst = (NULL != P->pSubdirectoryRecording);

  std::wstring * ptr = NULL;
  if ( (true == have_src) && (false == have_dst) )
    {
      ptr = new std::wstring(_T(""));
      assert(NULL != ptr);
      ptr->reserve(MAX_PATH);
    }
  /* if */

  AcquireSRWLockExclusive( &(P->sLockDirectory) );
  {
    if (true == have_src)
      {
        if (NULL == P->pSubdirectoryRecording)
          {
            assert( (false == have_dst) && (NULL != ptr) );
            P->pSubdirectoryRecording = ptr;
            ptr = NULL;
          }
        /* if */

        assert(NULL != P->pSubdirectoryRecording);
        if (NULL != P->pSubdirectoryRecording)
          {
            *(P->pSubdirectoryRecording) += L" ";
            *(P->pSubdirectoryRecording) += *tag;
          }
        /* if */
      }
    else
      {
        assert(NULL == ptr);
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockDirectory) );  

  SAFE_DELETE( ptr );

  return true;  
}
/* ImageEncoderAppendToSubdirectoryRecording */



//! Set recodring subdirectory to current timestamp.
/*!
  Sets active recording subdirectory to the current timestamp.
  The subdirectory is not created by this function; onlt its name is set.
  This function is intended to be called before each batch acquisition
  is started so results of every batch acquisition end in a different
  subdirectory which contains recording timestamp.

  \param P      Pointer to image encoder parameters.
  \param pSubdirectoryRecording  Pointer to string storing subdirectory name.
  If NULL then subdirectory name will be constructed from the current time.
  \return Returns true if successfull.
*/
bool
ImageEncoderSetSubdirectoryRecordingToTimestamp(
                                                ImageEncoderParameters * const P
                                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  SYSTEMTIME sLocalTime;
  GetLocalTime( &sLocalTime );

  std::ostringstream time;
  time << std::setfill ('0') << std::setw (4) << sLocalTime.wYear;
  time << "-";
  time << std::setfill ('0') << std::setw (2) << sLocalTime.wMonth;
  time << "-";
  time << std::setfill ('0') << std::setw (2) << sLocalTime.wDay;
  time << "T";
  time << std::setfill ('0') << std::setw (2) << sLocalTime.wHour;
  time << "-";
  time << std::setfill ('0') << std::setw (2) << sLocalTime.wMinute;
  time << "-";
  time << std::setfill ('0') << std::setw (2) << sLocalTime.wSecond;

  std::string time_str(time.str());
  wchar_t * buffer = new wchar_t[time_str.size() + 1];
  for (size_t i = 0; i < time_str.size(); ++i) buffer[i] = time_str[i];
  buffer[time_str.size()] = 0;

  std::wstring SubdirectoryRecording(buffer);

  bool const set = ImageEncoderSetSubdirectoryRecording(P, &SubdirectoryRecording);
  assert(true == set);

  SAFE_DELETE_ARRAY( buffer );

  return set;
}
/* ImageEncoderSetSubdirectoryRecordingToTimestamp */



//! Get recording subdirectory.
/*!
  Returns the name of recording subdirectory where images will be stored.

  Once it is no longer needed the returned string should be destroyed by delete operator.

  \param P      Pointer to image encoder parameters.
  \return Returns pointer to a string containing sub directory name or NULL pointer if unsuccessfull.
*/
std::wstring *
ImageEncoderGetSubdirectoryRecording(
                                     ImageEncoderParameters * const P
                                     )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  std::wstring * pSubdirectoryRecording = new std::wstring();
  assert(NULL != pSubdirectoryRecording);
  if (NULL == pSubdirectoryRecording) return pSubdirectoryRecording;

  pSubdirectoryRecording->reserve(MAX_PATH);

  AcquireSRWLockShared( &(P->sLockDirectory) );
  {
    if (NULL != P->pSubdirectoryRecording)
      {
        *pSubdirectoryRecording += *(P->pSubdirectoryRecording);
      }
    else
      {
        SAFE_DELETE(pSubdirectoryRecording);
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockDirectory) );

  return pSubdirectoryRecording;
}
/* ImageEncoderGetSubdirectoryRecording */



//! Get output directory.
/*!
  Function returns the full name of the output directory which is comprised of:
  1) the name of the output directory;
  2) followed by the output subdirectory which is set to current timestamp; and
  3) followed by camera subdirectory if more than one camera is attached.

  The output directory is created as needed.

  \param P      Pointer to image encoder parameters.
  \param create_directories   If set to true then directories are created if they do not exist.
  If set to false then output directory name is constructed without checking if the name is valid.
  \param skip_camera_subdirectory If true then camera subdirectory is not appended.
  \return Function returns directory name or NULL if unsucceffull.
*/
std::wstring *
ImageEncoderGetOutputDirectory(
                               ImageEncoderParameters * const P,
                               bool const create_directories,
                               bool const skip_camera_subdirectory
                               )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  bool have_directory_data = false;
  bool have_subdirectory_session = false;
  bool have_subdirectory_recording = false;
  bool have_subdirectory_camera = false;

  AcquireSRWLockShared( &(P->sLockDirectory) );
  {
    have_directory_data = (NULL != P->pDirectoryData);
    have_subdirectory_session = (NULL != P->pSubdirectorySession);
    have_subdirectory_recording = (NULL != P->pSubdirectoryRecording);
    have_subdirectory_camera = (NULL != P->pSubdirectoryCamera);
  }
  ReleaseSRWLockShared( &(P->sLockDirectory) );

  // If there is no output directory then return immediately.
  if (false == have_directory_data) return NULL;

  std::wstring * pDirectory = NULL;
  pDirectory = new std::wstring();
  assert(NULL != pDirectory);
  if (NULL == pDirectory) return pDirectory;
  pDirectory->reserve(MAX_PATH);

  // Start with output directory.
  AcquireSRWLockShared( &(P->sLockDirectory) );
  {
    assert(NULL != P->pDirectoryData);
    (*pDirectory) += *(P->pDirectoryData);
  }
  ReleaseSRWLockShared( &(P->sLockDirectory) );

  // Then append session directory.
  if (true == have_subdirectory_session)
    {
      bool append_subdirectory_session = false;
      if (true == create_directories)
        {
          std::wstring tmp;
          tmp.reserve(MAX_PATH + 1);
          tmp += *(pDirectory);
          tmp += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectorySession);
            tmp += *(P->pSubdirectorySession);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );

          if (FALSE != PathIsDirectoryW( tmp.c_str() ) )
            {
              append_subdirectory_session = true;
            }
          else
            {
              BOOL const created = CreateDirectory(tmp.c_str(), NULL);
              //assert(TRUE == created);
              if (TRUE == created)
                {
                  append_subdirectory_session = true;
                }
              else
                {
                  DWORD const error_code = GetLastError();
                  if (ERROR_ALREADY_EXISTS == error_code)
                    {
                      append_subdirectory_session = true;
                    }
                  else
                    {
                      Debugfprintf(stderr, gDbgImageEncoderCannotCreateDirectory, P->EncoderID + 1);
                    }
                  /* if */
                }
              /* if */
            }
          /* if (FALSE != PathIsDirectoryW( tmp.c_str() ) ) */
        }
      else
        {
          append_subdirectory_session = true;
        }
      /* if */

      if (true == append_subdirectory_session)
        {
          (*pDirectory) += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectorySession);
            (*pDirectory) += *(P->pSubdirectorySession);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );
        }
      /* if (true == append_subdirectory_session) */
    }
  /* if (true == have_subdirectory_session) */

  // Then append recording subdirectory.
  if (true == have_subdirectory_recording)
    {
      bool append_subdirectory_recording = false;
      if (true == create_directories)
        {
          std::wstring tmp;
          tmp.reserve(MAX_PATH + 1);
          tmp += *(pDirectory);
          tmp += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectoryRecording);
            tmp += *(P->pSubdirectoryRecording);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );

          if (FALSE != PathIsDirectoryW( tmp.c_str() ) )
            {
              append_subdirectory_recording = true;
            }
          else
            {
              BOOL const created = CreateDirectory(tmp.c_str(), NULL);
              //assert(TRUE == created);
              if (TRUE == created)
                {
                  append_subdirectory_recording = true;
                }
              else
                {
                  DWORD const error_code = GetLastError();
                  if (ERROR_ALREADY_EXISTS == error_code)
                    {
                      append_subdirectory_recording = true;
                    }
                  else
                    {
                      Debugfprintf(stderr, gDbgImageEncoderCannotCreateDirectory, P->EncoderID + 1);
                    }
                  /* if */
                }
              /* if */
            }
          /* if */
        }
      else
        {
          append_subdirectory_recording = true;
        }
      /* if */

      if (true == append_subdirectory_recording)
        {
          (*pDirectory) += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectoryRecording);
            (*pDirectory) += *(P->pSubdirectoryRecording);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );
        }
      /* if (true == append_subdirectory_recording) */
    }
  /* (true == have_subdirectory_recording) */

  bool const have_multiple_cameras = (NULL != P->pSynchronization) && (1 < P->pSynchronization->Camera.size());
  if ( (false == skip_camera_subdirectory) && (true == have_multiple_cameras) && (true == have_subdirectory_camera) )
    {
      bool append_subdirectory_camera = false;
      if (true == create_directories)
        {
          std::wstring tmp;
          tmp.reserve(MAX_PATH + 1);
          tmp += *pDirectory;
          tmp += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectoryCamera);
            tmp += *(P->pSubdirectoryCamera);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );

          if (FALSE != PathIsDirectoryW( tmp.c_str() ) )
            {
              append_subdirectory_camera = true;
            }
          else
            {
              BOOL const created = CreateDirectory(tmp.c_str(), NULL);
              //assert(TRUE == created);
              if (TRUE == created)
                {
                  append_subdirectory_camera = true;
                }
              else
                {
                  DWORD const error_code = GetLastError();
                  if (ERROR_ALREADY_EXISTS == error_code)
                    {
                      append_subdirectory_camera = true;
                    }
                  else
                    {
                      Debugfprintf(stderr, gDbgImageEncoderCannotCreateCameraDirectory, P->EncoderID + 1, P->CameraID + 1);
                    }
                  /* if */
                }
              /* if */
            }
          /* if */
        }
      else
        {
          append_subdirectory_camera = true;
        }
      /* if */

      if (true == append_subdirectory_camera)
        {
          (*pDirectory) += L"\\";
          AcquireSRWLockShared( &(P->sLockDirectory) );
          {
            assert(NULL != P->pSubdirectoryCamera);
            (*pDirectory) += *(P->pSubdirectoryCamera);
          }
          ReleaseSRWLockShared( &(P->sLockDirectory) );
        }
      /* if (true == append_subdirectory_camera) */
    }
  /* if */

  return pDirectory;
}
/* ImageEncoderGetOutputDirectory */



//! Copy output directory names.
/*!
  Function copies output directory names from source to destination encoder.

  \param dst    Pointer to destination encoder.
  \param src    Pointer to source encoder.
  \return Function returns true if successfull and false otherwise.
*/
bool
ImageEncoderCopyOutputDirectoryNames(
                                     ImageEncoderParameters * const dst,
                                     ImageEncoderParameters * const src
                                     )
{
  assert( (NULL != dst) && (NULL != src) );
  if ( (NULL == dst) || (NULL == src) ) return false;

  wchar_t const * const pDirectoryData = ImageEncoderGetDirectory(src);
  std::wstring * pSubdirectorySession = ImageEncoderGetSubdirectorySession(src);
  std::wstring * pSubdirectoryRecording = ImageEncoderGetSubdirectoryRecording(src);

  bool set_data = ImageEncoderSetDirectory(dst, pDirectoryData, NULL);
  bool const set_session = ImageEncoderSetSubdirectorySession(dst, pSubdirectorySession);
  bool const set_recording = ImageEncoderSetSubdirectoryRecording(dst, pSubdirectoryRecording);

  bool const set = set_data && set_session && set_recording;

  SAFE_DELETE( pSubdirectorySession );
  SAFE_DELETE( pSubdirectoryRecording );

  return set;
}
/* ImageEncoderCopyOutputDirectoryNames */

#pragma endregion // Configuration of output directory


#pragma region // Set region of interest

//! Set ROI.
/*!
  Function sets rectangular region of interest.

  \param P      Pointer to image encoder parameters.
  \param x      X coordinate of the upper left corner of the ROI.
  \param y      Y coordinate of the upper left corner of the ROI.
  \param w      Width in pixels of the ROI.
  \param h      Height in pixels of the ROI.
  \return Function returns true if successfull.
*/
bool
ImageEncoderSetROI(
                   ImageEncoderParameters * const P,
                   int const x,
                   int const y,
                   int const w,
                   int const h
                   )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert( (0 < x) && (0 < y) && (0 < w) && (0 < h) );
  if ( (0 >= x) || (0 >= y) || (0 >= w) || (0 >= h) ) return false;

  P->roi_x = x;
  P->roi_y = y;
  P->roi_w = w;
  P->roi_h = h;

  return true;
}
/* ImageEncoderSetROI */

#pragma endregion // Set region of interest


#pragma region // Pixel statistics

//! Resets pixel statistics.
/*!
  Resets pixel statistics for all acquired frames.

  \param P     Pointer to image encoder parameters.
  \return Returns true if successfull.
*/
bool
ImageEncoderResetFrameData(
                           ImageEncoderParameters * const P
                           )
{
  assert(NULL != P);
  if (NULL == P) return false;

  std::vector<PixelStatistics> * pStatistics = new std::vector<PixelStatistics>();
  assert(NULL != pStatistics);
  if (NULL == pStatistics) return false;

  AcquireSRWLockExclusive( &(P->sLockImageData) );
  {
    SAFE_DELETE(P->pStatistics);
    P->pStatistics = pStatistics;
  }
  ReleaseSRWLockExclusive( &(P->sLockImageData) );

  return true;
}
/* ImageEncoderResetFrameData */

#pragma endregion // Pixel statistics


#pragma region // Delay time measurement

//! Compute system delay time.
/*!
  Computes system delay time.

  \param P      Pointer to image encoder parameters.
  \param delay_out   Address where the delay time will be returned. Note that delay_out is always the largest of all measured delays.
  \param delay_BW_out      Address where the delay time for black-to-white transition will be returned.
  \param delay_WB_out      Address where the delay time for white-to-black transition will be returned.
  \return Returns true if successfull.
*/
bool
ImageEncoderComputeDelay(
                         ImageEncoderParameters * const P,
                         double * const delay_out,
                         double * const delay_BW_out,
                         double * const delay_WB_out
                         )
{
  assert(NULL != P);
  if (NULL == P) return false;

  std::vector<PixelStatistics> * pStatistics = NULL;
  int N = 0;

  bool delay_computed = true;

  double t_exp = BATCHACQUISITION_sNaN_dv;
  double white = BATCHACQUISITION_sNaN_dv;
  double black = BATCHACQUISITION_sNaN_dv;
  double white_to_black = BATCHACQUISITION_sNaN_dv;
  double black_to_white = BATCHACQUISITION_sNaN_dv;

  AcquireSRWLockShared( &(P->sLockImageData) );
  {
    pStatistics = P->pStatistics;
    if (NULL != pStatistics)
      {
        N = (int)pStatistics->size();

        for (int i = 0; i < N; ++i)
          {
            PixelStatistics const data = (*pStatistics)[i];
            if (data.pattern_type == (int)(SL_PATTERN_DELAY_MEASUREMENT_WHITE))
              {
                white = data.sum[0] + data.sum[1] + data.sum[2];
                t_exp = data.t_exp;
                assert(0.0 == data.t_del);
                break;
              }
            /* if */
          }
        /* for */

        for (int i = 0; i < N; ++i)
          {
            PixelStatistics const data = (*pStatistics)[i];
            if (data.pattern_type == (int)(SL_PATTERN_DELAY_MEASUREMENT_BLACK))
              {
                black = data.sum[0] + data.sum[1] + data.sum[2];
                delay_computed = delay_computed & (t_exp == data.t_exp);
                assert(0.0 == data.t_del);
                break;
              }
            /* if */
          }
        /* for */

        for (int i = 0; i < N; ++i)
          {
            PixelStatistics const data = (*pStatistics)[i];
            if (data.pattern_type == (int)(SL_PATTERN_DELAY_MEASUREMENT_WHITE_TO_BLACK))
              {
                white_to_black = data.sum[0] + data.sum[1] + data.sum[2];
                delay_computed = delay_computed & (t_exp == data.t_exp);
                assert(0.0 == data.t_del);
                break;
              }
            /* if */
          }
        /* for */

        for (int i = 0; i < N; ++i)
          {
            PixelStatistics const data = (*pStatistics)[i];
            if (data.pattern_type == (int)(SL_PATTERN_DELAY_MEASUREMENT_BLACK_TO_WHITE))
              {
                black_to_white = data.sum[0] + data.sum[1] + data.sum[2];
                delay_computed = delay_computed & (t_exp == data.t_exp);
                assert(0.0 == data.t_del);
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockImageData) );

  if (NULL == pStatistics) return false;

  // Sanity check. If any of the folowing assertions failed then input data is corrupt.
  bool const cnd1 = white > black;
  bool const cnd2 = (white > black_to_white) && (black_to_white > black);
  bool const cnd3 = (white > white_to_black) && (white_to_black > black);
  bool const measurement_valid = cnd1 && cnd2 && cnd3;
  if (false == measurement_valid)
    {
      int const cnt = wprintf(gWarningImageEncoderDelayMeasurement);
      assert(0 < cnt);
    }
  /* if */

  delay_computed = delay_computed & measurement_valid;

  double const delay_BW = t_exp * (white - black_to_white) / (white - black);
  double const delay_WB = t_exp * (white_to_black - black) / (white - black);

  if (true == measurement_valid)
    {
      assert( 0 < delay_BW );
      assert( 0 < delay_WB );
    }
  /* if */

  double const delay = (delay_BW > delay_WB)? delay_BW : delay_WB;
  if (NULL != delay_out) *delay_out = delay;
  if (NULL != delay_BW_out) *delay_BW_out = delay_BW;
  if (NULL != delay_WB_out) *delay_WB_out = delay_WB;

  return delay_computed;
}
/* ImageEncoderComputeDelay */

#pragma endregion // Delay time measurement


#endif /* !__BATCHACQUISITIONIMAGEENCODER_CPP */
