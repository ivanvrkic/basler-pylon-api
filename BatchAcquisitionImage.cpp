/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImage.cpp
  \brief  Basic image manipulation procedures.

  Image load and save functions, etc.
  All functions use Windows Imaging Components (WIC).

  \author Tomislav Petkovic
  \date   2021-04-20
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGE_CPP
#define __BATCHACQUISITIONIMAGE_CPP


#include "BatchAcquisitionImage.h"



//! Blanks image metadata.
/*!
  Blanks image metadata.

  \param pData  Pointer to image metadata.
*/
void
ImageMetadataBlank(
                   ImageMetadata * const pData
                   )
{
  assert(NULL != pData);
  if (NULL == pData) return;

  pData->no = 0;
  pData->render_type = QI_UNKNOWN_TYPE;
  pData->pattern_type = SL_PATTERN_INVALID;
  pData->key = -1;
  pData->present_counter = -1;
  pData->vblank_counter = -1;
  pData->QPC_current_presented = -1;
  pData->QPC_trigger_scheduled_RT = -1;
  pData->QPC_trigger_scheduled_AT = -1;
  pData->QPC_next_scheduled = -1;
  pData->QPC_next_presented = -1;
  pData->QPC_before_trigger = -1;
  pData->QPC_after_trigger = -1;
  pData->pFilename = NULL;
  pData->red = 0.0f;
  pData->green = 0.0f;
  pData->blue = 0.0f;
  pData->alpha = 1.0f;
  pData->delay = 0.0;
  pData->exposure = 0.0;
  pData->index = -1;
  pData->retry = 0;
  pData->ProjectorID = -1;
  pData->CameraID = -1;
  pData->fBatch = false;
  pData->fBlocking = true;
  pData->fFixed = false;
  pData->fSavePNG = false;
  pData->fSaveRAW = false;
  pData->fLast = false;
  pData->fTrigger = false;
  pData->fSkipAcquisition = false;
}
/* ImageMetadataBlank */



//! Releases image metadata.
/*!
  Frees memory of image metadata structure (if any).

  \param pData  Pointer to image metadata.
*/
void
ImageMetadataRelease(
                     ImageMetadata * const pData
                     )
{
  assert(NULL != pData);
  if (NULL == pData) return;
  SAFE_DELETE(pData->pFilename);
  ImageMetadataBlank(pData);
}
/* ImageMetadataRelease */



//! Copy image metadata.
/*!
  Copies image metadata from source to destination.

  All variables stored in the ImageMetadata structure are simple except one pointer to a filename.
  When copying the data all fields may be copied verbatim so bot pSrc and pDst share the filename
  (shallow_copy is true) or filename string may be dupicated (shallow_copy is false).

  \param pSrc   Pointer to source structure.
  \param pDst   Pointer to destination structure. Destination must be previously initialized.
  \param shallow_copy Flag to indicate the type of copy operation. If true then data is copied verbatime.
*/
void
ImageMetadataCopy(
                  ImageMetadata * const pSrc,
                  ImageMetadata * const pDst,
                  bool const shallow_copy
                  )
{
  assert( (NULL != pSrc) && (NULL != pDst) );
  if ( (NULL == pSrc) || (NULL == pDst) ) return;

  SAFE_DELETE(pDst->pFilename);
  *pSrc = *pDst;

  if ( (false == shallow_copy) && (NULL != pSrc->pFilename) )
    {
      pDst->pFilename = new std::wstring( *(pSrc->pFilename) );
    }
  /* if */
}
/* ImageMetadataCopy */



//! Compares image metadata.
/*!
  Compares image metadata from two structures.

  \param p1     Pointer to first structure.
  \param p2     Pointer to second structure.
  \param shallow_compare  Flag to indicate the type of compare operation. If true then everything but the filename is compared.
*/
bool
ImageMetadataCompare(
                     ImageMetadata * const p1,
                     ImageMetadata * const p2,
                     bool const shallow_compare
                     )
{
  assert( (NULL != p1) && (NULL != p2) );
  if ( (NULL == p1) || (NULL == p2) ) return false;

  bool equal = (p1->no == p2->no);
  equal = equal && (p1->render_type == p2->render_type);
  equal = equal && (p1->pattern_type == p2->pattern_type);

  equal = equal && (p1->key == p2->key);
  equal = equal && (p1->present_counter == p2->present_counter);
  equal = equal && (p1->vblank_counter == p2->vblank_counter);

  equal = equal && (p1->QPC_current_presented == p2->QPC_current_presented);
  equal = equal && (p1->QPC_trigger_scheduled_RT == p2->QPC_trigger_scheduled_RT);
  equal = equal && (p1->QPC_trigger_scheduled_AT == p2->QPC_trigger_scheduled_AT);
  equal = equal && (p1->QPC_next_scheduled == p2->QPC_next_scheduled);
  equal = equal && (p1->QPC_next_presented == p2->QPC_next_presented);
  equal = equal && (p1->QPC_before_trigger == p2->QPC_before_trigger);
  equal = equal && (p1->QPC_after_trigger == p2->QPC_after_trigger);

  equal = equal && (p1->red == p2->red);
  equal = equal && (p1->green == p2->green);
  equal = equal && (p1->blue == p2->blue);
  equal = equal && (p1->alpha == p2->alpha);

  equal = equal && (p1->delay == p2->delay);
  equal = equal && (p1->exposure == p2->exposure);

  equal = equal && (p1->index == p2->index);
  equal = equal && (p1->retry == p2->retry);

  equal = equal && (p1->ProjectorID == p2->ProjectorID);
  equal = equal && (p1->CameraID == p2->CameraID);

  equal = equal && (p1->fBatch == p2->fBatch);
  equal = equal && (p1->fBlocking == p2->fBlocking);
  equal = equal && (p1->fFixed == p2->fFixed);
  equal = equal && (p1->fSavePNG == p2->fSavePNG);
  equal = equal && (p1->fSaveRAW == p2->fSaveRAW);
  equal = equal && (p1->fLast == p2->fLast);
  equal = equal && (p1->fTrigger == p2->fTrigger);
  equal = equal && (p1->fSkipAcquisition == p2->fSkipAcquisition);

  if (false == shallow_compare)
    {
      if (p1->pFilename != p2->pFilename)
        {
          if ( (NULL != p1->pFilename) && (NULL != p2->pFilename) )
            {
              equal = equal && ( 0 == p1->pFilename->compare(*(p2->pFilename)) );
            }
          else
            {
              equal = false;
            }
          /* if */
        }
      /* if */
    }
  /* if */

  return equal;
}
/* ImageMetadataCompare */



//! Blanks pixel statistics.
/*!
  Blanks pixel statistics.

  \param P      Pointer to pixel statistics.
*/
void
PixelStatisticsBlank(
                     PixelStatistics * const P
                     )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->pattern_type = (int)(SL_PATTERN_INVALID);

  P->sum[0] = 0.0f;
  P->sum[1] = 0.0f;
  P->sum[2] = 0.0f;

  P->mean[0] = 0.0;
  P->mean[1] = 0.0;
  P->mean[2] = 0.0;

  P->dev[0] = 0.0;
  P->dev[1] = 0.0;
  P->dev[2] = 0.0;

  P->min[0] = 0.0;
  P->min[1] = 0.0;
  P->min[2] = 0.0;

  P->max[0] = 0.0;
  P->max[1] = 0.0;
  P->max[2] = 0.0;

  P->t_exp = 0.0;
  P->t_del = 0.0;
}
/* PixelStatisticsBlank */



//! Loads image from file.
/*!
  Creates WIC bitmap from file.

  \param pIWICFactory   Pointer to WIC factory. Must not be NULL.
  \param URI            Filename or URI of the bitmap to load.
  \param ppBitmap       Address where a pointer to created WIC bitmap will be stored. Must not be NULL.
  \return Returns S_OK if successfull.
*/
HRESULT
ImageLoadFromFile(
                  IWICImagingFactory * const pIWICFactory,
                  wchar_t const * const URI,
                  IWICBitmap ** ppBitmap
                  )
{
  if (NULL != ppBitmap) *ppBitmap = NULL;

  assert(NULL != pIWICFactory);
  if (NULL == pIWICFactory) return E_INVALIDARG;

  assert(NULL != ppBitmap);
  if (NULL == ppBitmap) return E_INVALIDARG;

  assert(NULL != URI);
  if (NULL == URI) return E_INVALIDARG;

  HRESULT hr = S_OK;

  IWICBitmapDecoder * pDecoder = NULL;
  IWICBitmapFrameDecode * pDecoderFrame = NULL;
  IWICFormatConverter * pConverter = NULL;
  IWICBitmap * pBitmap = NULL;

  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateDecoderFromFilename(
                                                   URI,
                                                   &GUID_VendorMicrosoftBuiltIn,
                                                   GENERIC_READ,
                                                   WICDecodeMetadataCacheOnLoad,
                                                   &pDecoder
                                                   );
      //assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pDecoder->GetFrame(0, &pDecoderFrame);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateFormatConverter(&pConverter);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pConverter->Initialize(
                                  pDecoderFrame,
                                  DEFAULT_WIC_PIXEL_FORMAT, // Defined in TestSycnrhonization.h
                                  WICBitmapDitherTypeNone,
                                  NULL,
                                  0.0,
                                  WICBitmapPaletteTypeMedianCut
                                  );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmapFromSource(
                                                pConverter,
                                                WICBitmapCacheOnLoad,
                                                &pBitmap
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  SAFE_RELEASE(pConverter);
  SAFE_RELEASE(pDecoderFrame);
  SAFE_RELEASE(pDecoder);

  if ( SUCCEEDED(hr) )
    {
      *ppBitmap = pBitmap;
    }
  else
    {
      SAFE_RELEASE(pBitmap);
    }
  /* if */

  return hr;
}
/* ImageLoadFromFile */



//! Stores image to file.
/*!
  Stores WIC bitmap to PNG file.

  \param pIWICFactory   Pointer to WIC factory.
  \param pBitmap        Pointer to WIC bitmap.
  \param URI    Filename. Extension should be PNG.
  \return Returns S_OK if successfull.
*/
HRESULT
ImageSaveToPNG(
               IWICImagingFactory * const pIWICFactory,
               IWICBitmap * pBitmap,
               wchar_t const * const URI
               )
{
  assert(NULL != pIWICFactory);
  if (NULL == pIWICFactory) return E_INVALIDARG;

  assert(NULL != pBitmap);
  if (NULL == pBitmap) return E_INVALIDARG;

  assert(NULL != URI);
  if (NULL == URI) return E_INVALIDARG;

  HRESULT hr = S_OK;

  IWICStream * pStream = NULL;
  IWICBitmapEncoder * pEncoder = NULL;
  IWICBitmapFrameEncode * pBitmapFrame = NULL;

  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateStream(&pStream);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pStream->InitializeFromFilename(URI, GENERIC_WRITE);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pEncoder->CreateNewFrame(&pBitmapFrame, NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pBitmapFrame->Initialize(NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pBitmapFrame->WriteSource(pBitmap, NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pBitmapFrame->Commit();
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pEncoder->Commit();
      assert( SUCCEEDED(hr) );
    }
  /* if */

  SAFE_RELEASE( pBitmapFrame );
  SAFE_RELEASE( pEncoder );
  SAFE_RELEASE( pStream );

  return hr;
}
/* ImageSaveToPNG */



//! Computes image statistics.
/*!
  Computes image statistics.

  \param cols Number of columns.
  \param rows Number of rows.
  \param stride Length of one row in bytes.
  \param data Pointer to memory block holding image data.
  \param pStatistics    Pointer to output statistics.
  \return Function returns S_OK if successfull.
*/
template <class T>
inline
HRESULT
ImageStatisticsC3_inline(
                         int const cols,
                         int const rows,
                         size_t const stride,
                         void const * const data,
                         PixelStatistics * const pStatistics
                         )
{
  assert( (0 < cols) && (0 < rows) && (0 < stride) );
  if ( (0 >= cols) || (0 >= rows) || (0 >= stride) ) return E_INVALIDARG;

  assert(NULL != data);
  if (NULL == data) return E_INVALIDARG;

  assert(NULL != pStatistics);
  if (NULL == pStatistics) return E_INVALIDARG;

  T const * const src = (T *)(data);

  T b_max, b_min, g_max, g_min, r_max, r_min;
  {
    T const b = src[0];
    T const g = src[1];
    T const r = src[2];

    b_max = b; b_min = b;
    g_max = g; g_min = g;
    r_max = r; r_min = r;
  }

  double b_sum = 0.0;
  double g_sum = 0.0;
  double r_sum = 0.0;

  double b_mean = 0.0;
  double g_mean = 0.0;
  double r_mean = 0.0;

  double b_M2 = 0.0;
  double g_M2 = 0.0;
  double r_M2 = 0.0;

  double length = 0.0;

  for (int j = 0; j < rows; ++j)
    {
      T const * const srcrow = (T *)( (T *)(src) + stride * j );
      for (int i = 0; i < cols; ++i)
        {
          T const b = srcrow[3*i    ];
          T const g = srcrow[3*i + 1];
          T const r = srcrow[3*i + 2];

          if (b > b_max) b_max = b;
          if (g > g_max) g_max = g;
          if (r > r_max) r_max = r;

          if (b < b_min) b_min = b;
          if (g < g_min) g_min = g;
          if (r < r_min) r_min = r;

          double const b_value = (double)b;
          double const g_value = (double)g;
          double const r_value = (double)r;

          b_sum += b_value;
          g_sum += g_value;
          r_sum += r_value;

          double const b_delta = b_value - b_mean;
          double const g_delta = g_value - g_mean;
          double const r_delta = r_value - r_mean;

          length += 1.0;
          double const inv_length = 1.0 / length;

          b_mean += b_delta * inv_length;
          g_mean += g_delta * inv_length;
          r_mean += r_delta * inv_length;

          b_M2 += b_delta * (b_value - b_mean);
          g_M2 += g_delta * (g_value - g_mean);
          r_M2 += r_delta * (r_value - r_mean);
        }
      /* for */
    }
  /* for */

  assert(b_min <= b_max);
  assert(g_min <= g_max);
  assert(r_min <= r_max);

  double const inv_length_1 = 1.0 / (length - 1.0);

  double const b_dev = sqrt( b_M2 * inv_length_1 );
  double const g_dev = sqrt( g_M2 * inv_length_1 );
  double const r_dev = sqrt( r_M2 * inv_length_1 );

  if (NULL != pStatistics)
    {
      pStatistics->sum[0] = r_sum;
      pStatistics->sum[1] = g_sum;
      pStatistics->sum[2] = b_sum;

      pStatistics->mean[0] = r_mean;
      pStatistics->mean[1] = g_mean;
      pStatistics->mean[2] = b_mean;

      pStatistics->dev[0] = r_dev;
      pStatistics->dev[1] = g_dev;
      pStatistics->dev[2] = b_dev;

      pStatistics->min[0] = (double)r_min;
      pStatistics->min[1] = (double)g_min;
      pStatistics->min[2] = (double)b_min;

      pStatistics->max[0] = (double)r_max;
      pStatistics->max[1] = (double)g_max;
      pStatistics->max[2] = (double)b_max;
    }
  /* if */

  return S_OK;
}
/* ImageStatisticsC3_inline */



//! Computes image statistics.
/*!
  Computes image statistics.

  \param pBitmap        Pointer to image. Image must be in BGR8 format.
  \param pStatistics    Pointer to output statistics.
  \return Function returns S_OK if successfull.
*/
HRESULT
ImageStatistics(
                IWICBitmap * pBitmap,
                PixelStatistics * const pStatistics
                )
{
  assert(NULL != pBitmap);
  if (NULL == pBitmap) return E_INVALIDARG;

  assert(NULL != pStatistics);
  if (NULL == pStatistics) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Get image format and size.
  WICPixelFormatGUID PixelFormat = GUID_WICPixelFormatUndefined;

  UINT width = 0;
  UINT height = 0;

  if ( SUCCEEDED(hr) )
    {
      hr = pBitmap->GetPixelFormat( &PixelFormat );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pBitmap->GetSize(&width, &height);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if (GUID_WICPixelFormat24bppBGR != PixelFormat) return E_NOTIMPL;
  if (0 == width) return E_INVALIDARG;
  if (0 == height) return E_INVALIDARG;

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, (INT)width, (INT)height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get image stride.
  UINT src_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&src_stride);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get data pointer.
  UINT src_size = 0;
  BYTE * src = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&src_size, &src);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      hr = ImageStatisticsC3_inline<unsigned char>(width, height, src_stride, src, pStatistics);

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  return hr;
}
/* ImageStatistics */



//! Computes image statistics.
/*!
  Computes image statistics.

  \param pImg        Pointer to image. Image must be in BGR8 format.
  \param pStatistics    Pointer to output statistics.
  \return Function returns S_OK if successfull.
*/
HRESULT
ImageStatistics(
                cv::Mat * const pImg,
                PixelStatistics * const pStatistics
                )
{
  assert(NULL != pImg);
  if (NULL == pImg) return E_INVALIDARG;

  assert(NULL != pStatistics);
  if (NULL == pStatistics) return E_INVALIDARG;

  assert( CV_MAT_CN(pImg->type()) == 3 );
  if ( CV_MAT_CN(pImg->type()) != 3 ) return E_INVALIDARG;

  void const * const src = (void *)(pImg->data);
  assert(NULL != src);
  if (NULL == src) return E_INVALIDARG;

  HRESULT hr = S_OK;

  int const depth = CV_MAT_DEPTH(pImg->type());

  switch (depth)
    {
    case CV_8U:
      hr = ImageStatisticsC3_inline<UINT8>(pImg->cols, pImg->rows, pImg->step[0], pImg->data, pStatistics);
      break;

    case CV_8S:
      hr = ImageStatisticsC3_inline<INT8>(pImg->cols, pImg->rows, pImg->step[0], pImg->data, pStatistics);
      break;

    case CV_16U:
      hr = ImageStatisticsC3_inline<UINT16>(pImg->cols, pImg->rows, pImg->step[0], pImg->data, pStatistics);
      break;

    case CV_16S:
      hr = ImageStatisticsC3_inline<INT16>(pImg->cols, pImg->rows, pImg->step[0], pImg->data, pStatistics);
      break;

    case CV_32S:
      hr = ImageStatisticsC3_inline<INT32>(pImg->cols, pImg->rows, pImg->step[0], pImg->data, pStatistics);
      break;

    default:
      hr = E_NOTIMPL;
    }
  /* switch */

  return hr;
}
/* ImageStatistics */



//! Return corresponding cv::Mat data type.
/*!
  Function returns corresponding cv::Mat data type.
  As cv::Mat only encodes depth and channel information
  we identify all single channel matrices as grayscale and
  all three channel matrices as BGR.

  \param ptr    Pointer to cv::Mat.
  \return Returns pixel datatype.
*/
ImageDataType
GetImageDataType(
                 cv::Mat * const ptr
                 )
{
  assert(NULL != ptr);
  if (NULL == ptr) return IDT_UNKNOWN;

  ImageDataType type = IDT_UNKNOWN;

  int const flags = ptr->type();
  int const depth = CV_MAT_DEPTH(flags);
  int const cn = CV_MAT_CN(flags);

  switch (cn)
    {
    case 1:
      {
        switch (depth)
          {
          case CV_8U:
            type = IDT_8U_GRAY;
            break;

          case CV_8S:
            type = IDT_8S_GRAY;
            break;

          case CV_16U:
            type = IDT_16U_GRAY;
            break;

          case CV_16S:
            type = IDT_16S_GRAY;
            break;

          case CV_32S:
            type = IDT_32S_GRAY;
            break;

          case CV_32F:
          case CV_64F:
          default:
            assert(IDT_UNKNOWN == type);
            break;
          }
        /* switch */
      }
      break;

    case 3:
      {
        switch (depth)
          {
          case CV_8U:
            type = IDT_8U_BGR;
            break;

          case CV_16U:
            type = IDT_16U_BGR;
            break;

          case CV_8S:
          case CV_16S:
          case CV_32S:
          case CV_32F:
          case CV_64F:
          default:
            assert(IDT_UNKNOWN == type);
            break;
          }
        /* switch */
      }
      break;

    default:
      assert(IDT_UNKNOWN == type);
      break;
    }
  /* switch */

  return type;
}
/* GetImageDataType */



//! Finds best matching cv::Mat data type.
/*!
  Function returns cv::Mat flags and IDT enumeration which is
  a best match for input IDT enumeration without data loss, i.e.
  a buffer having IDT enumeration type_in may be converted to cv::Mat
  using the selected flags without loss of data. IDT enumeration
  type_out always matches number of channels and depth of the cv::Mat;
  it is necessary to retain Bayer information. Note as cv::Mat cannot
  have different endianness than the host machine, cannot
  store packed data, and cannot store 10 or 12 bit data the output
  IDT enumeration is only a subset of input IDT enumeration.
  
  \param type_in        Input IDT enumeration.
  \param type_out       Output IDT enumeration.
  \param flags_out      Output cv::Mat flags.
  \return Returns true if matching mode exists and false otherwise.
*/
bool
GetBestMatchingcvMatFlags(
                          ImageDataType const type_in,
                          ImageDataType * const type_out,
                          int * const flags_out
                          )
{
  bool have_match = false;
  ImageDataType type = IDT_UNKNOWN;
  int flags = 0;

  switch (type_in)
    {
    case IDT_UNKNOWN:
    case IDT_32U_GRAY:
    case IDT_8U_RGB_Planar:
      {
        assert(false == have_match);
      }
      break;

    case IDT_8U_BINARY:
    case IDT_8U_GRAY:
      {
        // Matches RawBufferTo1CcvMat and RawBufferToGraycvMat
        have_match = true;
        type = type_in; 
        flags = CV_8UC1;
      }
      break;

    case IDT_10U_GRAY:
    case IDT_12U_GRAY_Packed:
    case IDT_16U_GRAY:
    case IDT_16U_GRAY_BigEndian:
      {
        // Matches RawBufferTo1CcvMat and RawBufferToGraycvMat
        have_match = true;
        type = IDT_16U_GRAY; 
        flags = CV_16UC1;
      }
      break;
      
    case IDT_8S_GRAY:
      {
        // Matches RawBufferTo1CcvMat and RawBufferToGraycvMat
        type = type_in;
        flags = CV_8SC1;
      }
      break;

    case IDT_16S_GRAY:
    case IDT_16S_GRAY_BigEndian:
      {
        // Matches RawBufferTo1CcvMat and RawBufferToGraycvMat
        have_match = true;
        type = IDT_16S_GRAY;
        flags = CV_16SC1;
      }
      break;
      
    case IDT_32S_GRAY:
      {
        // Matches RawBufferTo1CcvMat and RawBufferToGraycvMat
        have_match = true;
        type = type_in;
        flags = CV_32SC1;
      }
      break;

    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
      {
        // Matches RawBufferTo1CcvMat
        have_match = true;
        type = type_in;
        flags = CV_8UC1;
      }
      break;

    case IDT_10U_BayerGR:
    case IDT_12U_BayerGR_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerGR_BigEndian:
      {
        // Matches RawBufferTo1CcvMat
        have_match = true;
        type = IDT_16U_BayerGR;
        flags = CV_16UC1;
      }
      break;

    case IDT_10U_BayerRG:
    case IDT_12U_BayerRG_Packed:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerRG_BigEndian:
      {
        // Matches RawBufferTo1CcvMat
        have_match = true;
        type = IDT_16U_BayerRG;
        flags = CV_16UC1;
      }
      break;

    case IDT_10U_BayerGB:
    case IDT_12U_BayerGB_Packed:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerGB_BigEndian:
      {
        // Matches RawBufferTo1CcvMat
        have_match = true;
        type = IDT_16U_BayerGB;
        flags = CV_16UC1;
      }
      break;

    case IDT_10U_BayerBG:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerBG_BigEndian:
      {
        // Matches RawBufferTo1CcvMat
        have_match = true;
        type = IDT_16U_BayerBG;
        flags = CV_16UC1;
      }
      break;

    case IDT_8U_RGB:
    case IDT_8U_BGR:
    case IDT_8U_RGBA:
    case IDT_8U_BGRA:
    case IDT_8U_YUV411:
    case IDT_8U_YUV422:
    case IDT_8U_YUV422_BT601:
    case IDT_8U_YUV422_BT709:
    case IDT_8U_YUV444:
    case IDT_8U_UYV444:
      {
        // Matches RawBufferToBGRcvMat
        have_match = true;
        type = IDT_8U_BGR;
        flags = CV_8UC3;
      }
      break;
      
    case IDT_16U_BGR:
      {
        // Matches RawBufferToBGRcvMat
        have_match = true;
        type = type_in;
        flags = CV_16UC3;
      }
      break;      
    }
  /* switch */

  if (NULL != type_out) *type_out = type;
  if (NULL != flags_out) *flags_out = flags;

  return have_match;  
}
/* GetBestMatchingcvMatFlags */
                 


//! Return cv::Mat pixel size.
/*!
  Function returns cv::Mat pixel size in bytes.

  \param ptr    Pointer to cv::Mat.
  \return Returns pixel size in bytes or -1 if pixel size cannot be determined.
*/
int
GetImagePixelSizeInBytes(
                         cv::Mat * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return -1;

  int size = -1;

  int const flags = ptr->type();
  int const depth = CV_MAT_DEPTH(flags);
  int const cn = CV_MAT_CN(flags);

  switch (depth)
    {
    case CV_8U:
    case CV_8S:
      size = 1 * cn;
      break;

    case CV_16U:
    case CV_16S:
      size = 2 * cn;
      break;

    case CV_32S:
      size = 4 * cn;
      break;

    case CV_32F:
      size = 4 * cn;
      break;

    case CV_64F:
      size = 8 * cn;
      break;

    default:
      assert(0 > size);
      break;
    }
  /* switch */

  return size;
}
/* GetImagePixelSizeInBytes */



//! Return place of the MSB bit.
/*!
  Returns place of the MSB bit.

  \param ptr    Pointer to cv::Mat image.
  \return Returns place of MSB bit or NaN if unsuccessfull.
*/
double
GetImagePixelMSBPosition(
                         int const depth
                         )
{
  double pos = BATCHACQUISITION_qNaN_dv;  
  switch (depth)
    {
    case CV_8U:
    case CV_8S:
      pos = 7.0;
      break;

    case CV_16U:
    case CV_16S:
      pos = 15.0;
      break;

    case CV_32S:
      pos = 30.0;
      break;

    case CV_32F:
      pos = BATCHACQUISITION_nINF_dv;
      break;

    case CV_64F:
      pos = BATCHACQUISITION_nINF_dv;
      break;

    default:
      assert(true == isnan_inline(pos));
      break;
    }
  /* switch */
  return pos;
}
/* GetImagePixelMSBPosition */



//! Return place of the MSB bit.
/*!
  Returns place of the MSB bit.

  \param ptr    Pointer to cv::Mat image.
  \return Returns place of MSB bit or NaN if unsuccessfull.
*/
double
GetImagePixelMSBPosition(
                         cv::Mat * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return -1;

  double pos = BATCHACQUISITION_qNaN_dv;

  int const flags = ptr->type();
  int const depth = CV_MAT_DEPTH(flags);

  return GetImagePixelMSBPosition(depth);
}
/* GetImagePixelMSBPosition */



//! Create shallow copy ROI.
/*!
  Function creates shallow copy cv::Mat class that contains the valid
  intersection of the requested ROI and input image.

  \param src    Pointer to cv::Mat image.
  \param x      Upper left corner of the ROI.
  \param y      Upper left corner of the ROI.
  \param w      Width of the ROI.
  \param h      Height of the ROI.
  \return Function returns pointer to shallow copy cv::Mat or NULL if unsuccessfull.
*/
cv::Mat *
GetcvMatROI(
            cv::Mat * const src,
            int const x,
            int const y,
            int const w,
            int const h
            )
{
  assert( NULL != src );
  if (NULL == src) return NULL;

  assert( NULL != src->data );
  if (NULL == src->data) return NULL;

  int const cols = src->cols;
  int const rows = src->rows;

  assert( (0 < cols) && (0 < rows) );
  if ( (0 >= cols) || (0 >= rows) ) return NULL;

  assert( (0 < w) && (0 < h) );
  if ( (0 >= w) || (0 >= h) ) return NULL;

  int x0 = x;
  int y0 = y;
  if (0 > x0) x0 = 0;
  if (0 > y0) y0 = 0;
  if (cols <= x0) x0 = cols - 1;
  if (rows <= y0) y0 = rows - 1;

  int w0 = w;
  int h0 = h;
  if (x0 + w0 > cols) w0 = cols - x0;
  if (y0 + h0 > rows) h0 = rows - y0;

  int const stepx = GetImagePixelSizeInBytes(src);
  int const stepy = (int)( src->step[0] );

  void * const data = (BYTE *)(src->data) + x0 * stepx + y0 * stepy;

  cv::Mat * const dst = new cv::Mat(h0, w0, src->type(), data, stepy);
  assert(NULL != dst);

  return dst;
}
/* getcvMatROI */



#endif /* !__BATCHACQUISITIONIMAGE_CPP */
