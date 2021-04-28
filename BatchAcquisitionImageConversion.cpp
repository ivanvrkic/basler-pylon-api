/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015 UniZG, Zagreb. All rights reserved.
 * (c) 2015 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImageConversion.cpp
  \brief  Image conversions.

  This file contains converters from acquisition image format to BGR8 format
  that is required for saving and displaying images.
  Output uses WIC image container format.

  Main document describing possible conversions is GenICam
  Pixel Format Naming Convention (PFNC) available at
  http://www.emva.org/cms/upload/Standards/GenICam_Downloads/GenICam_PFNC_2_0.pdf

  \author Tomislav Petkovic
  \date   2015-05-27
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGECONVERSION_CPP
#define __BATCHACQUISITIONIMAGECONVERSION_CPP


#include "BatchAcquisitionImageConversion.h"
#include "BatchAcquisitionDebug.h"



/****** HELPER FUNCTIONS ******/

//! Clamps value to range [0,255].
/*!
  Function clamps value to  range [0,255].

  \param in     Input value.
  \return Clamped value.
*/
template <class T>
inline
T
ClampTo_0_255_inline(T in)
{
  if ((T)(255) < in) return (T)(255);
  if ((T)(  0) > in) return (T)(  0);
  return in;
}
/* ClampTo_0_255_inline */



//! Clamps value to range [16,235].
/*!
  Function clamps value to range [16,235].

  \param in     Input value.
  \return Clamped value.
*/
template <class T>
inline
T
ClampTo_16_235_inline(T in)
{
  if ((T)(235) < in) return (T)(235);
  if ((T)( 16) > in) return (T)( 16);
  return in;
}
/* ClampTo_16_235_inline */



//! Checks inputs.
/*!
  Checks image inputs.

  \param width  Image width.
  \param height Image height.
  \param stride Image stride.
  \param src    Pointer to image data.
  \return Returns S_OK or appropriate error code.
*/
inline
HRESULT
CheckImageInputs_inline(
                        unsigned int const width,
                        unsigned int const height,
                        unsigned int const stride,
                        void const * const src
                        )
{
  assert(0 < width);
  if (0 >= width) return E_INVALIDARG;

  assert(0 < height);
  if (0 >= height) return E_INVALIDARG;

  assert(0 < stride);
  if (0 >= stride) return E_INVALIDARG;

  assert(NULL != src);
  if (NULL == src) return E_POINTER;

  return S_OK;
}
/* CheckImageInputs_inline */



//! Checks inputs.
/*!
  Function checks input parameters.

  \param width  Image width. Must be positive.
  \param height Image height. Must be positive.
  \param stride Image stride. Must be positive.
  \param bpp Bytes per pixel.
  \param src    Source data pointer.
  \param pIWICFactory   WIC factory. Must not be NULL.
  \param ppDst Pointer to location where WIC bitmap pointer will be stored. Must not be NULL.
  \return Returns true if input parameters are valid.
*/
bool
CheckInputs_inline(
                   unsigned int const width,
                   unsigned int const height,
                   unsigned int const stride,
                   unsigned int const bpp,
                   void const * const src,
                   IWICImagingFactory * const pIWICFactory,
                   IWICBitmap ** ppDst
                   )
{
  assert(0 < width);
  if (0 >= width) return false;

  assert(0 < height);
  if (0 >= height) return false;

  assert(0 < stride);
  if (0 >= stride) return false;

  assert(0 < bpp);
  if (0 >= bpp) return false;

  assert(stride >= width * bpp);
  if (stride < width * bpp) return false;

  assert(NULL != src);
  if (NULL == src) return false;

  assert(NULL != pIWICFactory);
  if (NULL == pIWICFactory) return false;

  assert(NULL != ppDst);
  if (NULL == ppDst) return false;

  return true;
}
/* CheckInputs_inline */



/****** CONVERTERS ******/

//! Convert monochromatic 8-bit image to BGR8.
/*!
  Converts monochromatic 8-bit image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertMono8uToBGR8(
                    unsigned int const width,
                    unsigned int const height,
                    unsigned int const src_stride,
                    void const * const src,
                    IWICImagingFactory * const pIWICFactory,
                    IWICBitmap ** ppDst
                    )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 1, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  UINT8 * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; ++i)
            {
              UINT8 const value = srcrow[i];
              dstrow[3*i    ] = value;
              dstrow[3*i + 1] = value;
              dstrow[3*i + 2] = value;
            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertMono8uToBGR8 */



//! Convert monochromatic 16-bit image to BGR8.
/*!
  Converts monochromatic 16-bit image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertMono16uToBGR8(
                     unsigned int const width,
                     unsigned int const height,
                     unsigned int const src_stride,
                     void const * const src,
                     IWICImagingFactory * const pIWICFactory,
                     IWICBitmap ** ppDst
                     )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT16 const * const srcrow = (UINT16 *)( (BYTE *)(src) + src_stride * j );
          UINT8        * const dstrow = (UINT8  *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; ++i)
            {
              UINT8 const value = (UINT8)( (srcrow[i]) >> 8 );
              dstrow[3*i    ] = value;
              dstrow[3*i + 1] = value;
              dstrow[3*i + 2] = value;
            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertMono16uToBGR8 */



//! Convert YUV411 image to BGR8.
/*!
  Input image is stored in UYYVYY order with 12 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 4).
  Y is in range [0,255]; U and V are in range [0,255].

  Output image is BGR with 8 bits per pixel. RGB values are in range [0,255].

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV411ToBGR8(
                    unsigned int const width,
                    unsigned int const height,
                    unsigned int const src_stride,
                    void const * const src,
                    unsigned int const dst_stride,
                    void * const dst
                    )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Convert data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          int k = 0;
          for (unsigned int i = 0; i < width; i += 4, k += 6)
            {
              // Generic full-scale Y'CbCr to R'G'B' as defined in GenICam PFNC v2.0
              // Input: Y is in range [0,255]; U and V are in range [0,255]
              // Output: RGB is in range [0,255]
              {
                float const U  = (float)(src_row[k    ]) - 128.0f;
                float const Y1 = (float)(src_row[k + 1]);
                float const Y2 = (float)(src_row[k + 2]);
                float const V  = (float)(src_row[k + 3]) - 128.0f;
                float const Y3 = (float)(src_row[k + 4]);
                float const Y4 = (float)(src_row[k + 5]);

                float const UVB = 1.77200f * U;
                float const UVG = 0.34414f * U + 0.71414f * V;
                float const UVR =                1.40200f * V;

                int const idx = 3 * i;

                dst_row[idx     ] = (UINT8)( ClampTo_0_255_inline(Y1 + UVB) );
                dst_row[idx +  1] = (UINT8)( ClampTo_0_255_inline(Y1 - UVG) );
                dst_row[idx +  2] = (UINT8)( ClampTo_0_255_inline(Y1 + UVR) );

                dst_row[idx +  3] = (UINT8)( ClampTo_0_255_inline(Y2 + UVB) );
                dst_row[idx +  4] = (UINT8)( ClampTo_0_255_inline(Y2 - UVG) );
                dst_row[idx +  5] = (UINT8)( ClampTo_0_255_inline(Y2 + UVR) );

                dst_row[idx +  6] = (UINT8)( ClampTo_0_255_inline(Y3 + UVB) );
                dst_row[idx +  7] = (UINT8)( ClampTo_0_255_inline(Y3 - UVG) );
                dst_row[idx +  8] = (UINT8)( ClampTo_0_255_inline(Y3 + UVR) );

                dst_row[idx +  9] = (UINT8)( ClampTo_0_255_inline(Y4 + UVB) );
                dst_row[idx + 10] = (UINT8)( ClampTo_0_255_inline(Y4 - UVG) );
                dst_row[idx + 11] = (UINT8)( ClampTo_0_255_inline(Y4 + UVR) );
              }
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* ConvertYUV411ToBGR8 */



//! Convert YUV422 image to BGR8.
/*!
  Input image is stored in UYVY order with 16 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 2).
  Y is in range [0,255]; U and V are in range [0,255].

  Output image is BGR with 8 bits per pixel. RGB values are in range [0,255].

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422ToBGR8(
                    unsigned int const width,
                    unsigned int const height,
                    unsigned int const src_stride,
                    void const * const src,
                    unsigned int const dst_stride,
                    void * const dst
                    )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Convert data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; i += 2)
            {
              // Generic full-scale Y'CbCr to R'G'B' as defined in GenICam PFNC v2.0
              // Input: Y is in range [0,255]; U and V are in range [0,255]
              // Output: RGB is in range [0,255]
              {
                float const U  = (float)(src_row[2*i    ]) - 128.0f;
                float const Y1 = (float)(src_row[2*i + 1]);
                float const V  = (float)(src_row[2*i + 2]) - 128.0f;
                float const Y2 = (float)(src_row[2*i + 3]);

                float const UVB = 1.77200f * U;
                float const UVG = 0.34414f * U + 0.71414f * V;
                float const UVR =                1.40200f * V;

                int const idx = 3 * i;

                dst_row[idx    ] = (UINT8)( ClampTo_0_255_inline(Y1 + UVB) );
                dst_row[idx + 1] = (UINT8)( ClampTo_0_255_inline(Y1 - UVG) );
                dst_row[idx + 2] = (UINT8)( ClampTo_0_255_inline(Y1 + UVR) );

                dst_row[idx + 3] = (UINT8)( ClampTo_0_255_inline(Y2 + UVB) );
                dst_row[idx + 4] = (UINT8)( ClampTo_0_255_inline(Y2 - UVG) );
                dst_row[idx + 5] = (UINT8)( ClampTo_0_255_inline(Y2 + UVR) );
              }
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* ConvertYUV422ToBGR8 */



//! Convert YUV422 image to BGR8.
/*!
  Converts YUV422 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422ToBGR8(
                    unsigned int const width,
                    unsigned int const height,
                    unsigned int const src_stride,
                    void const * const src,
                    IWICImagingFactory * const pIWICFactory,
                    IWICBitmap ** ppDst
                    )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      hr = ConvertYUV422ToBGR8(width, height, src_stride, src, dst_stride, dst);
      assert( SUCCEEDED(hr) );

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertYUV422ToBGR8 */



//! Convert YUV422 image to full-scale BGR8 (BT.601).
/*!
  Input image is stored in UYVY order with 16 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 2).
  Y is in range [16,235]; U and V are in range [16,240].

  Output image is BGR with 8 bits per pixel. RGB valuse are in range [0,255].

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT601ToBGR8(
                         unsigned int const width,
                         unsigned int const height,
                         unsigned int const src_stride,
                         void const * const src,
                         unsigned int const dst_stride,
                         void * const dst
                         )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Convert data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; i += 2)
            {
              // Y'CbCr601 8-bit to R'G'B' as defined in GenICam PFNC v2.0 (BT.601)
              // Input: Y is in range [16,235]; U and V are in range [16,240]
              // Output: RGB is in range [0,255]
              {
                float const U  =              (float)(srcrow[2*i    ]) - 128.0f;
                float const Y1 = 1.16438f * ( (float)(srcrow[2*i + 1]) -  16.0f );
                float const V  =              (float)(srcrow[2*i + 2]) - 128.0f;
                float const Y2 = 1.16438f * ( (float)(srcrow[2*i + 3]) -  16.0f );

                float const UVB = 2.01723f * U;
                float const UVG = 0.39176f * U + 0.81297f * V;
                float const UVR =                1.59603f * V;

                int const idx = 3 * i;

                dstrow[idx    ] = (UINT8)( ClampTo_0_255_inline(Y1 + UVB) );
                dstrow[idx + 1] = (UINT8)( ClampTo_0_255_inline(Y1 - UVG) );
                dstrow[idx + 2] = (UINT8)( ClampTo_0_255_inline(Y1 + UVR) );

                dstrow[idx + 3] = (UINT8)( ClampTo_0_255_inline(Y2 + UVB) );
                dstrow[idx + 4] = (UINT8)( ClampTo_0_255_inline(Y2 - UVG) );
                dstrow[idx + 5] = (UINT8)( ClampTo_0_255_inline(Y2 + UVR) );
              }
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT601ToBGR8 */



//! Convert YUV422 image to full-scale BGR8 (BT.601).
/*!
  Converts YUV422 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT601ToBGR8(
                         unsigned int const width,
                         unsigned int const height,
                         unsigned int const src_stride,
                         void const * const src,
                         IWICImagingFactory * const pIWICFactory,
                         IWICBitmap ** ppDst
                         )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      hr = ConvertYUV422BT601ToBGR8(width, height, src_stride, src, dst_stride, dst);
      assert( SUCCEEDED(hr) );

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT601ToBGR8 */



//! Convert YUV422 image to scaled-down BGR8 (BT.601).
/*!
  Converts YUV422 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  Input image is stored in UYVY order with 16 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 2).
  Y is in range [16,235]; U and V are in range [16,240].

  Output image is BGR with 8 bits per pixel. RGB values are in range [16,240].

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT601ToScaledDownBGR8(
                                   unsigned int const width,
                                   unsigned int const height,
                                   unsigned int const src_stride,
                                   void const * const src,
                                   IWICImagingFactory * const pIWICFactory,
                                   IWICBitmap ** ppDst
                                   )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; i += 2)
            {
              // Y'CbCr601 8-bit to rgb as defined in GenICam PFNC v2.0 (BT.601)
              // Input: Y is in range [16,235]; U and V are in range [16,240]
              // Output: RGB is in range [16,235]
              {
                float const U  =              (float)(srcrow[2*i    ]) - 128.0f;
                float const Y1 = 1.16438f * ( (float)(srcrow[2*i + 1]) -  16.0f );
                float const V  =              (float)(srcrow[2*i + 2]) - 128.0f;
                float const Y2 = 1.16438f * ( (float)(srcrow[2*i + 3]) -  16.0f );

                float const UVB = 1.73245f * U;
                float const UVG = 0.33645f * U + 0.69820f * V;
                float const UVR =                1.37071f * V;

                dstrow[3*i    ] = (UINT8)( ClampTo_16_235_inline(Y1 + UVB) );
                dstrow[3*i + 1] = (UINT8)( ClampTo_16_235_inline(Y1 - UVG) );
                dstrow[3*i + 2] = (UINT8)( ClampTo_16_235_inline(Y1 + UVR) );

                dstrow[3*i + 3] = (UINT8)( ClampTo_16_235_inline(Y2 + UVB) );
                dstrow[3*i + 4] = (UINT8)( ClampTo_16_235_inline(Y2 - UVG) );
                dstrow[3*i + 5] = (UINT8)( ClampTo_16_235_inline(Y2 + UVR) );
              }
            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT601ToScaledDownBGR8 */



//! Convert YUV422 image to full-scale BGR8 (BT.709).
/*!
  Input image is stored in UYVY order with 16 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 2).
  Y is in range [16,235]; U and V are in range [16,240].

  Output image is BGR with 8 bits per pixel. RGB values are in range [0,255].

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT709ToBGR8(
                         unsigned int const width,
                         unsigned int const height,
                         unsigned int const src_stride,
                         void const * const src,
                         unsigned int const dst_stride,
                         void * const dst
                         )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Convert data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; i += 2)
            {
              // Y'CbCr709 8-bit to R'G'B' as defined in GenICam PFNC v2.0 (BT.709)
              // Input: Y is in range [16,235]; U and V are in range [16,240]
              // Output: RGB is in range [0,255]
              {
                float const U  =              (float)(srcrow[2*i    ]) - 128.0f;
                float const Y1 = 1.16438f * ( (float)(srcrow[2*i + 1]) -  16.0f );
                float const V  =              (float)(srcrow[2*i + 2]) - 128.0f;
                float const Y2 = 1.16438f * ( (float)(srcrow[2*i + 3]) -  16.0f );

                float const UVB = 2.11240f * U;
                float const UVG = 0.21325f * U + 0.53291f * V;
                float const UVR =                1.79274f * V;

                dstrow[3*i    ] = (UINT8)( ClampTo_0_255_inline(Y1 + UVB) );
                dstrow[3*i + 1] = (UINT8)( ClampTo_0_255_inline(Y1 - UVG) );
                dstrow[3*i + 2] = (UINT8)( ClampTo_0_255_inline(Y1 + UVR) );

                dstrow[3*i + 3] = (UINT8)( ClampTo_0_255_inline(Y2 + UVB) );
                dstrow[3*i + 4] = (UINT8)( ClampTo_0_255_inline(Y2 - UVG) );
                dstrow[3*i + 5] = (UINT8)( ClampTo_0_255_inline(Y2 + UVR) );
              }
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT709ToBGR8 */



//! Convert YUV422 image to full-scale BGR8 (BT.709).
/*!
  Converts YUV422 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT709ToBGR8(
                         unsigned int const width,
                         unsigned int const height,
                         unsigned int const src_stride,
                         void const * const src,
                         IWICImagingFactory * const pIWICFactory,
                         IWICBitmap ** ppDst
                         )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      hr = ConvertYUV422BT709ToBGR8(width, height, src_stride, src, dst_stride, dst);
      assert( SUCCEEDED(hr) );

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT709ToBGR8 */



//! Convert YUV422 image to scaled-down BGR8 (BT.709).
/*!
  Converts YUV422 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  Input image is stored in UYVY order with 16 bits per pixel
  and with 8 bits for each of Y, U, and V components (so U and V are subsampled with factor 2).
  Y is in range [16,235]; U and V are in range [16,240].

  Output image is BGR with 8 bits per pixel. RGB values are in range [16,235].

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertYUV422BT709ToScaledDownBGR8(
                                   unsigned int const width,
                                   unsigned int const height,
                                   unsigned int const src_stride,
                                   void const * const src,
                                   IWICImagingFactory * const pIWICFactory,
                                   IWICBitmap ** ppDst
                                   )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 2, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      /* Convert and copy data. */
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; i += 2)
            {
              // Y'CbCr709 8-bit to r'g'b' as defined in GenICam PFNC v2.0 (BT.709)
              // Input: Y is in range [16,235]; U and V are in range [16,240]
              // Output: RGB is in range [16,235]
              {
                float const U  =              (float)(srcrow[2*i    ]) - 128.0f;
                float const Y1 = 1.16438f * ( (float)(srcrow[2*i + 1]) -  16.0f );
                float const V  =              (float)(srcrow[2*i + 2]) - 128.0f;
                float const Y2 = 1.16438f * ( (float)(srcrow[2*i + 3]) -  16.0f );

                float const UVB = 1.81418f * U;
                float const UVG = 0.18314f * U + 0.45768f * V;
                float const UVR =                1.53965f * V;

                dstrow[3*i    ] = (UINT8)( ClampTo_16_235_inline(Y1 + UVB) );
                dstrow[3*i + 1] = (UINT8)( ClampTo_16_235_inline(Y1 - UVG) );
                dstrow[3*i + 2] = (UINT8)( ClampTo_16_235_inline(Y1 + UVR) );

                dstrow[3*i + 3] = (UINT8)( ClampTo_16_235_inline(Y2 + UVB) );
                dstrow[3*i + 4] = (UINT8)( ClampTo_16_235_inline(Y2 - UVG) );
                dstrow[3*i + 5] = (UINT8)( ClampTo_16_235_inline(Y2 + UVR) );
              }

            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertYUV422BT709ToScaledDownBGR8 */



//! Convert BGR8 image to BGR8.
/*!
  Converts (copies data) from BGR8 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored. Must not be NULL.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertBGR8ToBGR8(
                  unsigned int const width,
                  unsigned int const height,
                  unsigned int const src_stride,
                  void const * const src,
                  IWICImagingFactory * const pIWICFactory,
                  IWICBitmap ** ppDst
                  )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 3, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < 3 * width; ++i)
            {
              dstrow[i] = srcrow[i];
            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertBGR8ToBGR8 */



//! Convert RGB8 image to BGR8.
/*!
  Converts (copies data) from BGR8 image to BGR8 (GUID_WICPixelFormat24bppBGR).

  \param width  Source image width.
  \param height Source image height.
  \param src_stride Source image stride in bytes (number of bytes in one row).
  \param src    Pointer to source image data.
  \param pIWICFactory   Pointer to WIC factory.
  \param ppDst Address where a pointer to created IWICBitmap will be stored.
  \return Returns S_OK if successfull.
*/
HRESULT
ConvertRGB8ToBGR8(
                  unsigned int const width,
                  unsigned int const height,
                  unsigned int const src_stride,
                  void const * const src,
                  IWICImagingFactory * const pIWICFactory,
                  IWICBitmap ** ppDst
                  )
{
  // First blank outputs.
  if (NULL != ppDst) *ppDst = NULL;

  // Then check inputs.
  bool const check = CheckInputs_inline(width, height, src_stride, 3, src, pIWICFactory, ppDst);
  if (false == check) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Create destination bitmap.
  IWICBitmap * pDst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat24bppBGR, WICBitmapCacheOnLoad, &pDst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Lock bitmap.
  WICRect const rcLock = { 0, 0, width, height };
  IWICBitmapLock * pLock = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pDst->Lock(&rcLock, WICBitmapLockWrite, &pLock);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Get destination stride.
  UINT dst_stride = 0;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetStride(&dst_stride);
      assert( SUCCEEDED(hr) );
      assert((UINT)(src_stride) <= dst_stride);
    }
  /* if */

  // Get data pointer.
  UINT dst_size = 0;
  BYTE * dst = NULL;
  if ( SUCCEEDED(hr) )
    {
      hr = pLock->GetDataPointer(&dst_size, &dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy and convert the data.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const srcrow = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dstrow = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          for (unsigned int i = 0; i < width; ++i)
            {
              dstrow[3*i    ] = srcrow[3*i + 2];
              dstrow[3*i + 1] = srcrow[3*i + 1];
              dstrow[3*i + 2] = srcrow[3*i    ];
            }
          /* for */
        }
      /* for */

      // Release the bitmap lock.
      SAFE_RELEASE(pLock);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert(NULL != ppDst);
      assert(NULL == *ppDst);
      *ppDst = pDst;
    }
  else
    {
      SAFE_RELEASE(pDst);
    }
  /* if */

  return hr;
}
/* ConvertRGB8ToBGR8 */



//! Convert by changing byte order in place.
/*!
  Function effectively changes the data endianness.
  Calling the function twice restores the original buffer.

  \param width  Image width.
  \param height Image height.
  \param stride Image stride (length of one row in bytes).
  \param src_dst        Pointer to image.
  \return Returns S_OK if successfull.
*/
HRESULT
SwapBGR8ToRGB8InPlace(
                      unsigned int const width,
                      unsigned int const height,
                      unsigned int const stride,
                      void * const src_dst
                      )
{
  HRESULT hr = CheckImageInputs_inline(width, height, stride, src_dst);
  assert( SUCCEEDED(hr) );

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 * const src_dst_row = (UINT8 *)( (BYTE *)(src_dst) + stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const idx = 3 * i;

              UINT8 const byte0 = src_dst_row[idx    ];
              UINT8 const byte1 = src_dst_row[idx + 1];
              UINT8 const byte2 = src_dst_row[idx + 2];

              src_dst_row[idx    ] = byte2;
              src_dst_row[idx + 1] = byte1;
              src_dst_row[idx + 2] = byte0;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* SwapBGR8ToRGB8InPlace */



//! Convert by changing byte order.
/*!
  Function effectively changes UYV order to YUV order.
  Function is its own inverse.
  Operation may be performed in place.

  \param width  Image width.
  \param height Image height.
  \param src_stride     Length of one source row in bytes.
  \param src    Pointer to source data.
  \param dst_stride     Length of one destination row in bytes.
  \param dst    Pointer to destination data.
  \return Returns S_OK if successfull.
*/
HRESULT
SwapUYV8ToYUV8(
               unsigned int const width,
               unsigned int const height,
               unsigned int const src_stride,
               void const * const src,
               unsigned int const dst_stride,
               void * const dst
               )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const idx = 3 * i;

              UINT8 const byte0 = src_row[idx    ];
              UINT8 const byte1 = src_row[idx + 1];
              UINT8 const byte2 = src_row[idx + 2];

              dst_row[idx    ] = byte1;
              dst_row[idx + 1] = byte0;
              dst_row[idx + 2] = byte2;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* SwapUYV8ToYUV8 */



//! Convert by changing byte order in place.
/*!
  Function effectively changes UYV order to YUV order.
  Calling the function twice restores the original buffer.

  \param width  Image width.
  \param height Image height.
  \param stride Image stride (length of one row in bytes).
  \param src_dst        Pointer to image.
  \return Returns S_OK if successfull.
*/
HRESULT
SwapUYV8ToYUV8InPlace(
                      unsigned int const width,
                      unsigned int const height,
                      unsigned int const stride,
                      void * const src_dst
                      )
{
  HRESULT hr = CheckImageInputs_inline(width, height, stride, src_dst);
  assert( SUCCEEDED(hr) );

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 * const src_dst_row = (UINT8 *)( (BYTE *)(src_dst) + stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const idx = 3 * i;

              UINT8 const byte0 = src_dst_row[idx    ];
              UINT8 const byte1 = src_dst_row[idx + 1];

              src_dst_row[idx    ] = byte1;
              src_dst_row[idx + 1] = byte0;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* SwapUYV8ToYUV8InPlace */



//! Shift bits.
/*!
  Copies data from source to destination and shift bits thus effectively rescale the data.
  This function enables shifting LSB bits to MSB bits, e.g. if camera packs 10bit data
  into 16bits so 10bits are stored in 10 LSB bits then shifting by 6 gets the MSB of 10bit
  data into MSB of 16bit data so the data may be processed as regular 16 bit data.

  \param shift  Number of shifts.
  \param width  Image width.
  \param height Image height.
  \param src_stride     Length of one source row in bytes.
  \param src    Pointer to source data.
  \param dst_stride     Length of one destination row in bytes.
  \param dst    Pointer to destination data.
  \return Returns S_OK if successfull.
*/
HRESULT
ShiftLeftMono16(
                unsigned int const shift,
                unsigned int const width,
                unsigned int const height,
                unsigned int const src_stride,
                void const * const src,
                unsigned int const dst_stride,
                void * const dst
                )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Shift bits in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT16 const * const src_row = (UINT16 *)( (BYTE *)(src) + src_stride * j );
          UINT16       * const dst_row = (UINT16 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              dst_row[i] = src_row[i] << shift;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* ShiftLeftMono16 */



//! Swap bytes.
/*!
  Copies data from source to destination and swap bytes thus effectively changing the endianness of the data.

  \param width  Image width.
  \param height Image height.
  \param src_stride     Length of one source row in bytes.
  \param src    Pointer to source data.
  \param dst_stride     Length of one destination row in bytes.
  \param dst    Pointer to destination data.
  \return Returns S_OK if successfull.
*/
HRESULT
SwapBytesMono16(
                unsigned int const width,
                unsigned int const height,
                unsigned int const src_stride,
                void const * const src,
                unsigned int const dst_stride,
                void * const dst
                )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const idx = 2 * i;

              UINT8 const byte0 = src_row[idx    ];
              UINT8 const byte1 = src_row[idx + 1];

              dst_row[idx    ] = byte1;
              dst_row[idx + 1] = byte0;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* SwapBytesMono16 */



//! Swap bytes in place.
/*!
  Swaps pixel bytes in place thus effectively changing the endianness of the data.

  \param width  Image width.
  \param height Image height.
  \param stride Image stride (length of one row in bytes).
  \param src_dst        Pointer to image.
  \return Returns S_OK if successfull.
*/
HRESULT
SwapBytesMono16InPlace(
                       unsigned int const width,
                       unsigned int const height,
                       unsigned int const stride,
                       void * const src_dst
                       )
{
  HRESULT hr = CheckImageInputs_inline(width, height, stride, src_dst);
  assert( SUCCEEDED(hr) );

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 * const src_dst_row = (UINT8 *)( (BYTE *)(src_dst) + stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const idx = 2 * i;
              UINT8 const value = src_dst_row[idx];
              src_dst_row[idx] = src_dst_row[idx + 1];
              src_dst_row[idx + 1] = value;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* SwapBytesMono16InPlace */



//! Expand 12 bit packed data to 16 bit unpacked.
/*!
  Expands 12 bit packed data to 16 bit unpacked.
  Operation may be done in place if the buffer has enough space for the result.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Expand12BitTo16Bit(
                   unsigned int const width,
                   unsigned int const height,
                   unsigned int const src_stride,
                   void const * const src,
                   unsigned int const dst_stride,
                   void * const dst
                   )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Expand data. As expansion may be in-place we need to start from the end.
  if ( SUCCEEDED(hr) )
    {
      for (int j = (int)( height - 1 ); 0 <= j; --j)
        {
          UINT8 * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8 * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          int max_i = (int)( width );
          if (1 == max_i % 2)
            {
              --max_i;

              int const src_i = (max_i / 2) * 3;
              int const dst_i = max_i * 2;

              assert(1 + src_i < (int)(src_stride));
              assert(1 + dst_i < (int)(dst_stride));
              assert(src_i <= dst_i);

              UINT8 const b0 = src_row[src_i    ];
              UINT8 const b1 = src_row[src_i + 1];

              dst_row[dst_i    ] = b0;
              dst_row[dst_i + 1] = (b1 & 0xF0);
            }
          /* if */

          for (int i = max_i - 2; 0 <= i; i -= 2)
            {
              assert(0 == i % 2);
              int const src_i = (i / 2) * 3;
              int const dst_i = i * 2;

              assert(2 + src_i < (int)(src_stride));
              assert(3 + dst_i < (int)(dst_stride));
              assert(src_i <= dst_i);

              UINT8 const b0 = src_row[src_i    ];
              UINT8 const b1 = src_row[src_i + 1];
              UINT8 const b2 = src_row[src_i + 2];

              dst_row[dst_i    ] = (b1 & 0xF0);
              dst_row[dst_i + 1] = b0;
              dst_row[dst_i + 2] = (b1 & 0x0F) << 4;
              dst_row[dst_i + 3] = b2;
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* Expand12BitTo16Bit */



//! Shrink 12 bit packed data to 8 bit unpacked.
/*!
  Shirnks 12 bit packed data to 8 bit unpacked.
  Operation may be done in place if the buffer has enough space for the result.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Shrink12BitTo8Bit(
                  unsigned int const width,
                  unsigned int const height,
                  unsigned int const src_stride,
                  void const * const src,
                  unsigned int const dst_stride,
                  void * const dst
                  )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Shrink data.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8 * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );

          unsigned int i = 0;
          for (; i < width; i += 2)
            {
              assert(0 == i % 2);
              int const src_i = (i / 2) * 3;
              int const dst_i = i;

              assert(2 + src_i < (int)(src_stride));
              assert(1 + dst_i < (int)(dst_stride));
              assert(src_i >= dst_i);

              UINT8 const b0 = src_row[src_i    ];
              UINT8 const b1 = src_row[src_i + 1];
              UINT8 const b2 = src_row[src_i + 2];

              dst_row[dst_i    ] = b0;
              dst_row[dst_i + 1] = b2;
            }
          /* for */

          if (1 == width % 2)
            {
              int const last_i = width - 1;

              int const src_i = (last_i / 2) * 3;
              int const dst_i = last_i;

              assert(1 + src_i < (int)(src_stride));
              assert(dst_i < (int)(dst_stride));
              assert(src_i >= dst_i);

              UINT8 const b0 = src_row[src_i    ];
              UINT8 const b1 = src_row[src_i + 1];

              dst_row[dst_i] = b0;
            }
          else
            {
              assert(width == i);
            }
          /* if */
        }
      /* for */
    }
  /* if */

  return hr;
}
/* Shrink12BitTo8Bit */



//! Shrink 10 bit unpacked data to 8 bit unpacked.
/*!
  Shrinks 10 bit unpacked data stored as 10LSB bits in 16bits to 8 bit by leaving only most significant 8 bits.
  Operation may be performed in-place.
  Data is little endian.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Shrink16BitLSB10To8Bit(
                       unsigned int const width,
                       unsigned int const height,
                       unsigned int const src_stride,
                       void const * const src,
                       unsigned int const dst_stride,
                       void * const dst
                       )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Swap bytes in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              int const src_i = 2 * i;
              UINT8 const b0 = src_row[src_i    ];
              UINT8 const b1 = src_row[src_i + 1];

              dst_row[i] = (b0 >> 2) | (b1 << 6);
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/*  Shrink16BitLSB10To8Bit */



//! Shrink 16 bit data to 8 bit unpacked.
/*!
  Shrinks 16 bit data to 8 bit by leaving only most significant byte.
  Operation may be performed in-place.
  Data is little endian.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Shrink16BitTo8Bit(
                  unsigned int const width,
                  unsigned int const height,
                  unsigned int const src_stride,
                  void const * const src,
                  unsigned int const dst_stride,
                  void * const dst
                  )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Simply copy MSB byte in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              dst_row[i] = src_row[2*i + 1];
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/*  Shrink16BitTo8Bit */



//! Shrink 16 bit data to 8 bit unpacked.
/*!
  Shrinks 16 bit data to 8 bit by leaving only most significant byte.
  Operation may be performed in-place.
  Data is big endian.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Shrink16BitTo8BitBigEndian(
                           unsigned int const width,
                           unsigned int const height,
                           unsigned int const src_stride,
                           void const * const src,
                           unsigned int const dst_stride,
                           void * const dst
                           )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Simply copy LSB byte in-place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              dst_row[i] = src_row[2*i];
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/*  Shrink16BitTo8BitBigEndian */



//! Shrink 32 bit data to 8 bit unpacked.
/*!
  Shrinks 32 bit data to 8 bit by leaving only most significant byte.
  Operation may be performed in-place. Data is little endian.

  \param width  Source and destination image width.
  \param height Source and destination image height.
  \param src_stride Source image stride (length of one row in bytes).
  \param src        Pointer to source image.
  \param dst_stride Destination image stride (length of one row in bytes).
  \param dst        Pointer to destination image.
  \return Returns S_OK if successfull.
*/
HRESULT
Shrink32BitTo8Bit(
                  unsigned int const width,
                  unsigned int const height,
                  unsigned int const src_stride,
                  void const * const src,
                  unsigned int const dst_stride,
                  void * const dst
                  )
{
  HRESULT hr = CheckImageInputs_inline(width, height, src_stride, src);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      hr = CheckImageInputs_inline(width, height, dst_stride, dst);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Copy MSB byte in place.
  if ( SUCCEEDED(hr) )
    {
      for (unsigned int j = 0; j < height; ++j)
        {
          UINT8 const * const src_row = (UINT8 *)( (BYTE *)(src) + src_stride * j );
          UINT8       * const dst_row = (UINT8 *)( (BYTE *)(dst) + dst_stride * j );
          for (unsigned int i = 0; i < width; ++i)
            {
              dst_row[i] = src_row[4*i + 3];
            }
          /* for */
        }
      /* for */
    }
  /* if */

  return hr;
}
/*  Shrink32BitTo8Bit */



//! Convert raw buffer to BGR cv::Mat.
/*!
  Creates BGR cv::Mat from raw buffer data.
  Output matrix datatype will depend on input pixel format.
  Output matrix will always have three channels.

  \param type   Image pixel format.
  \param width  Image width.
  \param height Image height.
  \param stride Length of one image row in bytes.
  \param src    Pointer to raw image data.
  \return Returns pointer to cv::Mat or NULL if conversion cannot be performed.
*/
cv::Mat *
RawBufferToBGRcvMat(
                    ImageDataType const type,
                    unsigned int const width,
                    unsigned int const height,
                    unsigned int const stride,
                    void const * const src
                    )
{
  cv::Mat * mdst = NULL;

  HRESULT const hr = CheckImageInputs_inline(width, height, stride, src);
  assert( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return mdst;

  switch (type)
    {
    default:
    case IDT_UNKNOWN:
      {
        assert(NULL == mdst);
      }
      break;

    case IDT_8U_BINARY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_8U_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_10U_GRAY:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_12U_GRAY_Packed:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_16U_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_16U_GRAY_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_32U_GRAY:
      {
        // OpenCV does not support 32U datatype.
      }
      break;

    case IDT_8S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8S, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8SC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_16S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16S, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_16S_GRAY_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16S);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_32S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_32S, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_32SC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_GRAY2BGR);
      }
      break;

    case IDT_8U_BayerGR:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerGB2BGR);
      }
      break;

    case IDT_8U_BayerRG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerBG2BGR);
      }
      break;

    case IDT_8U_BayerGB:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerGR2BGR);
      }
      break;

    case IDT_8U_BayerBG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerRG2BGR);
      }
      break;

    case IDT_10U_BayerGR:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGB2BGR);
      }
      break;

    case IDT_10U_BayerRG:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerBG2BGR);
      }
      break;

    case IDT_10U_BayerGB:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGR2BGR);
      }
      break;

    case IDT_10U_BayerBG:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerRG2BGR);
      }
      break;

    case IDT_12U_BayerGR_Packed:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGB2BGR);
      }
      break;

    case IDT_12U_BayerRG_Packed:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerBG2BGR);
      }
      break;

    case IDT_12U_BayerGB_Packed:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGR2BGR);
      }
      break;

    case IDT_12U_BayerBG_Packed:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerRG2BGR);
      }
      break;

    case IDT_16U_BayerGR:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerGB2BGR);
      }
      break;

    case IDT_16U_BayerRG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerBG2BGR);
      }
      break;

    case IDT_16U_BayerGB:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerGR2BGR);
      }
      break;

    case IDT_16U_BayerBG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16U, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BayerRG2BGR);
      }
      break;

    case IDT_16U_BayerGR_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGB2BGR);
      }
      break;

    case IDT_16U_BayerRG_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerBG2BGR);
      }
      break;

    case IDT_16U_BayerGB_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerGR2BGR);
      }
      break;

    case IDT_16U_BayerBG_BigEndian:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_16U);
        HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_BayerRG2BGR);
      }
      break;

    case IDT_8U_RGB:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC3, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_RGB2BGR);
      }
      break;

    case IDT_8U_RGB_Planar:
      {
        // TODO!
      }
      break;

    case IDT_8U_RGBA:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC4, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_RGBA2BGR);
      }
      break;

    case IDT_8U_BGR:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC3, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16U_BGR:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16UC3, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC3);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_8U_BGRA:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC4, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_BGRA2BGR);
      }
      break;

    case IDT_8U_YUV411:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = ConvertYUV411ToBGR8(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
      break;

    case IDT_8U_YUV422:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC2, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_YUV2BGR_Y422);
      }
      break;

    case IDT_8U_YUV422_BT601:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = ConvertYUV422BT601ToBGR8(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
      break;

    case IDT_8U_YUV422_BT709:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = ConvertYUV422BT709ToBGR8(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
      break;

    case IDT_8U_YUV444:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC3, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(msrc, *(mdst), CV_YUV2RGB); // We use RGB as YUV conversion is implemented incorrectly (Bug #4227).
      }
      break;

    case IDT_8U_UYV444:
      {
        cv::Mat mtmp((int)(height), (int)(width), CV_8UC3);
        HRESULT const hr = SwapUYV8ToYUV8(width, height, stride, src, (unsigned int)(mtmp.step[0]), mtmp.data);
        assert( SUCCEEDED(hr) );

        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC3);
        assert(NULL != mdst);
        if (NULL != mdst) cv::cvtColor(mtmp, *(mdst), CV_YUV2RGB); // We use RGB as YUV conversion is implemented incorrectly (Bug #4227).
      }
      break;
    }
  /* switch */

  return mdst;
}
/* RawBufferToBGRcvMat */



//! Convert raw buffer to grayscale cv::Mat.
/*!
  Creates cv::Mat from raw buffer data.
  Output matrix datatype will depend on input pixel format.
  Output matrix will always have one channel.

  \param type   Image pixel format.
  \param width  Image width.
  \param height Image height.
  \param stride Length of one image row in bytes.
  \param src    Pointer to raw image data.
  \return Returns pointer to cv::Mat or NULL if conversion cannot be performed.
*/
cv::Mat *
RawBufferToGraycvMat(
                     ImageDataType const type,
                     unsigned int const width,
                     unsigned int const height,
                     unsigned int const stride,
                     void const * const src
                     )
{
  cv::Mat * mdst = NULL;

  HRESULT const hr = CheckImageInputs_inline(width, height, stride, src);
  assert( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return mdst;

  // Do a direct conversion for graylevel inputs. For all other inputs do
  // an indirect conversion using intermediate BGR format by calling RawBufferToBGRcvMat.
  switch (type)
    {

    case IDT_8U_BINARY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_8U_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_10U_GRAY:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_12U_GRAY_Packed:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_16U_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16UC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16U_GRAY_BigEndian:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_32U_GRAY:
      {
        // OpenCV does not support 32U datatype.
      }
      break;

    case IDT_8S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16S_GRAY_BigEndian:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_32S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_32SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_32SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    default:
      {
        cv::Mat * mtmp = RawBufferToBGRcvMat(type, width, height, stride, src);
        if (NULL != mtmp)
          {
            int const type = mtmp->type();
            int const depth = CV_MAT_DEPTH(type);
            assert( 3 == CV_MAT_CN(type) );
            mdst = new cv::Mat((int)height, (int)(width), CV_MAKE_TYPE(depth, 1));
            assert(NULL != mdst);
            if (NULL != mdst) cv::cvtColor(*(mtmp), *(mdst), CV_BGR2GRAY);
          }
        /* if */
        SAFE_DELETE(mtmp);
      }
      break;

    }
  /* switch */

  return mdst;
}
/* RawBufferToGraycvMat */



//! Convert raw buffer to 1C cv::Mat.
/*!
  Creates cv::Mat from raw buffer data.
  Output matrix datatype will depend on input pixel format.
  Output matrix will always have one channel using the following rules:
  1) if the image is grayscale return it as is;
  2) if the image is RAW Bayer simply copy data a single channel cv::Mat without debayering; and
  3) if the image is in color (RGB, BGR, YUV, etc.) then convert it to grayscale.

  \param type   Image pixel format.
  \param width  Image width.
  \param height Image height.
  \param stride Length of one image row in bytes.
  \param src    Pointer to raw image data.
  \return Returns pointer to cv::Mat or NULL if conversion cannot be performed.
*/
cv::Mat *
RawBufferTo1CcvMat(
                   ImageDataType const type,
                   unsigned int const width,
                   unsigned int const height,
                   unsigned int const stride,
                   void const * const src
                   )
{
  cv::Mat * mdst = NULL;

  HRESULT const hr = CheckImageInputs_inline(width, height, stride, src);
  assert( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return mdst;

  // Do a direct conversion for graylevel inputs. For all other inputs do
  // an indirect conversion using intermediate BGR format by calling RawBufferToBGRcvMat.
  switch (type)
    {

    case IDT_8U_BINARY:
    case IDT_8U_GRAY:
    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8UC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8UC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_10U_GRAY:
    case IDT_10U_BayerGR:
    case IDT_10U_BayerRG:
    case IDT_10U_BayerGB:
    case IDT_10U_BayerBG:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = ShiftLeftMono16(6, width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_12U_GRAY_Packed:
    case IDT_12U_BayerGR_Packed:
    case IDT_12U_BayerRG_Packed:
    case IDT_12U_BayerGB_Packed:
    case IDT_12U_BayerBG_Packed:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = Expand12BitTo16Bit(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_16U_GRAY:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerBG:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16UC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16U_GRAY_BigEndian:
    case IDT_16U_BayerGR_BigEndian:
    case IDT_16U_BayerRG_BigEndian:
    case IDT_16U_BayerGB_BigEndian:
    case IDT_16U_BayerBG_BigEndian:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16UC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_32U_GRAY:
      {
        // OpenCV does not support 32U datatype.
      }
      break;

    case IDT_8S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_8SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_8SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_16SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    case IDT_16S_GRAY_BigEndian:
      {
        mdst = new cv::Mat((int)(height), (int)(width), CV_16SC1);
        assert(NULL != mdst);
        if (NULL != mdst)
          {
            HRESULT const hr = SwapBytesMono16(width, height, stride, src, (unsigned int)(mdst->step[0]), mdst->data);
            assert( SUCCEEDED(hr) );
            if ( !SUCCEEDED(hr) ) SAFE_DELETE(mdst);
          }
        /* if */
      }
      break;

    case IDT_32S_GRAY:
      {
        cv::Mat msrc((int)(height), (int)(width), CV_32SC1, (void *)(src), (int)(stride));
        mdst = new cv::Mat((int)(height), (int)(width), CV_32SC1);
        assert(NULL != mdst);
        if (NULL != mdst) msrc.copyTo( *(mdst) );
      }
      break;

    default:
      {
        cv::Mat * mtmp = RawBufferToBGRcvMat(type, width, height, stride, src);
        if (NULL != mtmp)
          {
            int const type = mtmp->type();
            int const depth = CV_MAT_DEPTH(type);
            assert( 3 == CV_MAT_CN(type) );
            mdst = new cv::Mat((int)height, (int)(width), CV_MAKE_TYPE(depth, 1));
            assert(NULL != mdst);
            if (NULL != mdst) cv::cvtColor(*(mtmp), *(mdst), CV_BGR2GRAY);
          }
        /* if */
        SAFE_DELETE(mtmp);
      }
      break;

    }
  /* switch */

  return mdst;
}
/* RawBufferTo1CcvMat */



#endif /* !__BATCHACQUISITIONIMAGECONVERSION_CPP */
