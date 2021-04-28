/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImageRender.cpp
  \brief  Image rendering routines.

  Routines to render image from memory or from file to DXGI render surface
  using Direct2D. Routines here require DirectX 10.

  \author Tomislav Petkovic
  \date   2017-02-21
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGERENDER_CPP
#define __BATCHACQUISITIONIMAGERENDER_CPP


#include "BatchAcquisitionImageRender.h"



//! Renders bitmap to Direct 2D surface.
/*!
  Function renders preloaded WIC bitmap to Direct 2D rendering surface.

  \param pIBitmap    Pointer to Windows Imaging Component (WIC) bitmap.
  \param pRenderTarget  Pointer to ID2D1RenderTarget.
  \param pBlackBrush_in   Pointer to black brush associated with the render target. May be NULL.
  \return Returns S_OK if successfull.
*/
HRESULT
RenderBitmapFromIWICBitmap(
                           IWICBitmap * const pIBitmap,
                           ID2D1RenderTarget * const pRenderTarget,
                           ID2D1SolidColorBrush * const pBlackBrush_in
                           )
{
  assert(NULL != pIBitmap);
  if (NULL == pIBitmap) return E_INVALIDARG;

  assert(NULL != pRenderTarget);
  if (NULL == pRenderTarget) return E_INVALIDARG;

  bool const have_brush = (NULL != pBlackBrush_in);
  ID2D1SolidColorBrush * pBlackBrush = pBlackBrush_in;

  ID2D1Bitmap * pBitmap = NULL;

  HRESULT hr = S_OK;

  // Create solid color brush.
  if ( SUCCEEDED(hr) && (false == have_brush) )
    {
      assert(NULL == pBlackBrush);
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
                                                &pBlackBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Create Direct 2D bitmap and render it to rendering surface.
  if ( SUCCEEDED(hr) )
    {
      hr = pRenderTarget->CreateBitmapFromWicBitmap(
                                                    pIBitmap,
                                                    NULL,
                                                    &pBitmap
                                                    );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      D2D1_SIZE_F renderTargetSize = pRenderTarget->GetSize();
      D2D1_SIZE_F bitmapSize = pBitmap->GetSize();

      // Set destination rectangle for solid color brush to full screen.
      D2D1_RECT_F destinationRectangleBrush = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);

      // Scale destination rectangle to achieve best fit-to-screen effect preserving aspect ratio.
      D2D1_RECT_F destinationRectangleImage = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);
      if (bitmapSize.width > bitmapSize.height)
        {
          destinationRectangleImage.bottom = renderTargetSize.width * bitmapSize.height / bitmapSize.width;
          if (destinationRectangleImage.bottom > renderTargetSize.height)
            {
              destinationRectangleImage.right = renderTargetSize.height * bitmapSize.width / bitmapSize.height;
              destinationRectangleImage.bottom = renderTargetSize.height;

              float const offsetX = (renderTargetSize.width - destinationRectangleImage.right) * 0.5f;
              destinationRectangleImage.left += offsetX;
              destinationRectangleImage.right += offsetX;
            }
          else
            {
              float const offsetY = (renderTargetSize.height - destinationRectangleImage.bottom) * 0.5f;
              destinationRectangleImage.top += offsetY;
              destinationRectangleImage.bottom += offsetY;
            }
          /* if */
        }
      else
        {
          destinationRectangleImage.right = renderTargetSize.height * bitmapSize.width / bitmapSize.height;
          if (destinationRectangleImage.right > renderTargetSize.width)
            {
              destinationRectangleImage.right = renderTargetSize.width;
              destinationRectangleImage.bottom = renderTargetSize.width * bitmapSize.height / bitmapSize.width;

              float const offsetY = (renderTargetSize.height - destinationRectangleImage.bottom) * 0.5f;
              destinationRectangleImage.top += offsetY;
              destinationRectangleImage.bottom += offsetY;
            }
          else
            {
              float const offsetX = (renderTargetSize.width - destinationRectangleImage.right) * 0.5f;
              destinationRectangleImage.left += offsetX;
              destinationRectangleImage.right += offsetX;
            }
          /* if */
        }
      /* if */

      pRenderTarget->BeginDraw();

      pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
      pRenderTarget->FillRectangle(destinationRectangleBrush, pBlackBrush);
      pRenderTarget->DrawBitmap(
                                pBitmap,
                                destinationRectangleImage,
                                1.0,
                                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                                D2D1::RectF(0, 0, bitmapSize.width, bitmapSize.height)
                                );

      hr = pRenderTarget->EndDraw();
      assert( SUCCEEDED(hr) );      
    }
  /* if */

  SAFE_RELEASE(pBitmap);
  if (false == have_brush) SAFE_RELEASE(pBlackBrush);

  return hr;
}
/* RenderBitmapFromIWICBitmap */



//! Renders bitmap to Direct 2D surface.
/*!
  Function loads image from file and renders it to Direct 2D rendering surface.

  \param pIWICFactory   Pointer to Windows Imaging Component (WIC) factory.
  \param URI    Filename or web address.
  \param pRenderTarget  Pointer to ID2D1RenderTarget.
  \param pBlackBrush_in   Pointer to black brush associated with the render target. May be NULL.
  \return Returns S_OK if successfull.
*/
HRESULT
RenderBitmapFromFile(
                     IWICImagingFactory * const pIWICFactory,
                     PCWSTR const URI,
                     ID2D1RenderTarget * const pRenderTarget,
                     ID2D1SolidColorBrush * const pBlackBrush_in
                     )
{
  assert(NULL != pIWICFactory);
  if (NULL == pIWICFactory) return E_INVALIDARG;

  assert(NULL != pRenderTarget);
  if (NULL == pRenderTarget) return E_INVALIDARG;

  bool const have_brush = (NULL != pBlackBrush_in);
  ID2D1SolidColorBrush * pBlackBrush = pBlackBrush_in;

  IWICBitmapDecoder * pDecoder = NULL;
  IWICBitmapFrameDecode * pSource = NULL;
  IWICFormatConverter * pConverter = NULL;
  ID2D1Bitmap * pBitmap = NULL;

  HRESULT hr = S_OK;

  // Create solid color brush.
  if ( SUCCEEDED(hr) && (false == have_brush) )
    {
      assert(NULL == pBlackBrush);
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
                                                &pBlackBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Load and convert the image.
  if ( SUCCEEDED(hr) )
    {
      hr = pIWICFactory->CreateDecoderFromFilename(
                                                   URI,
                                                   NULL,
                                                   GENERIC_READ,
                                                   WICDecodeMetadataCacheOnLoad,
                                                   &pDecoder
                                                   );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pDecoder->GetFrame(0, &pSource);
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
                                  pSource,
                                  DEFAULT_WIC_PIXEL_FORMAT, // Defined in TestSycnrhonization.h
                                  WICBitmapDitherTypeNone,
                                  NULL,
                                  0.0,
                                  WICBitmapPaletteTypeMedianCut
                                  );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Create Direct 2D bitmap and render it to rendering surface.
  if ( SUCCEEDED(hr) )
    {
      hr = pRenderTarget->CreateBitmapFromWicBitmap(
                                                    pConverter,
                                                    NULL,
                                                    &pBitmap
                                                    );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      D2D1_SIZE_F const renderTargetSize = pRenderTarget->GetSize();
      D2D1_SIZE_F bitmapSize = pBitmap->GetSize();

      // Set destination rectangle for solid color brush to full screen.
      D2D1_RECT_F destinationRectangleBrush = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);

      // Scale destination rectangle to achieve best fit-to-screen effect preserving aspect ratio.
      D2D1_RECT_F destinationRectangleImage = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);
      if (bitmapSize.width > bitmapSize.height)
        {
          destinationRectangleImage.bottom = renderTargetSize.width * bitmapSize.height / bitmapSize.width;
          if (destinationRectangleImage.bottom > renderTargetSize.height)
            {
              destinationRectangleImage.right = renderTargetSize.height * bitmapSize.width / bitmapSize.height;
              destinationRectangleImage.bottom = renderTargetSize.height;

              float const offsetX = (renderTargetSize.width - destinationRectangleImage.right) * 0.5f;
              destinationRectangleImage.left += offsetX;
              destinationRectangleImage.right += offsetX;
            }
          else
            {
              float const offsetY = (renderTargetSize.height - destinationRectangleImage.bottom) * 0.5f;
              destinationRectangleImage.top += offsetY;
              destinationRectangleImage.bottom += offsetY;
            }
          /* if */
        }
      else
        {
          destinationRectangleImage.right = renderTargetSize.height * bitmapSize.width / bitmapSize.height;
          if (destinationRectangleImage.right > renderTargetSize.width)
            {
              destinationRectangleImage.right = renderTargetSize.width;
              destinationRectangleImage.bottom = renderTargetSize.width * bitmapSize.height / bitmapSize.width;

              float const offsetY = (renderTargetSize.height - destinationRectangleImage.bottom) * 0.5f;
              destinationRectangleImage.top += offsetY;
              destinationRectangleImage.bottom += offsetY;
            }
          else
            {
              float const offsetX = (renderTargetSize.width - destinationRectangleImage.right) * 0.5f;
              destinationRectangleImage.left += offsetX;
              destinationRectangleImage.right += offsetX;
            }
          /* if */
        }
      /* if */

      pRenderTarget->BeginDraw();

      pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
      pRenderTarget->FillRectangle(destinationRectangleBrush, pBlackBrush);
      pRenderTarget->DrawBitmap(
                                pBitmap,
                                destinationRectangleImage,
                                1.0,
                                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                                D2D1::RectF(0, 0, bitmapSize.width, bitmapSize.height)
                                );

      hr = pRenderTarget->EndDraw();
      assert( SUCCEEDED(hr) );
    }
  /* if */

  SAFE_RELEASE(pBitmap);
  SAFE_RELEASE(pConverter);
  SAFE_RELEASE(pSource);
  SAFE_RELEASE(pDecoder);
  if (false == have_brush) SAFE_RELEASE(pBlackBrush);

  return hr;
}
/* RenderBitmapFromFile */



//! Paints render target black.
/*!
  Paints render target in black color.

  \param pRenderTarget  Pointer to ID2D1RenderTarget.
  \param pBlackBrush_in   Pointer to black brush associated with the render target. May be NULL.
  \return Returns S_OK if successfull.
*/
HRESULT
BlankRenderTarget(
                  ID2D1RenderTarget * const pRenderTarget,
                  ID2D1SolidColorBrush * const pBlackBrush_in
                  )
{
  assert(NULL != pRenderTarget);
  if (NULL == pRenderTarget) return E_INVALIDARG;

  bool const have_brush = (NULL != pBlackBrush_in);
  ID2D1SolidColorBrush * pBlackBrush = pBlackBrush_in;

  HRESULT hr = S_OK;

  // Create solid color brush.
  if ( SUCCEEDED(hr) && (false == have_brush) )
    {
      assert(NULL == pBlackBrush);
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
                                                &pBlackBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      D2D1_SIZE_F const renderTargetSize = pRenderTarget->GetSize();
      D2D1_RECT_F destinationRectangleBrush = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);

      pRenderTarget->BeginDraw();

      pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
      pRenderTarget->FillRectangle(destinationRectangleBrush, pBlackBrush);

      hr = pRenderTarget->EndDraw();
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if (false == have_brush) SAFE_RELEASE(pBlackBrush);

  return hr;
}
/* BlankRenderTarget */



#endif /* !__BATCHACQUISITIONIMAGERENDER_CPP */
