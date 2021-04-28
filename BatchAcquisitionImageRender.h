/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImageRender.h
  \brief  Image rendering routines.

  \author Tomislav Petkovic
  \date   2017-02-21
*/


#ifndef __BATCHACQUISITIONIMAGERENDER_H
#define __BATCHACQUISITIONIMAGERENDER_H


#include "BatchAcquisition.h"


//! Renders bitmap to Direct 2D surface.
HRESULT
RenderBitmapFromIWICBitmap(
                           IWICBitmap * const,
                           ID2D1RenderTarget * const,
                           ID2D1SolidColorBrush * const
                           );

//! Renders bitmap to Direct 2D surface.
HRESULT
RenderBitmapFromFile(
                     IWICImagingFactory * const,
                     PCWSTR const,
                     ID2D1RenderTarget * const,
                     ID2D1SolidColorBrush * const
                     );

//! Paints render target black.
HRESULT
BlankRenderTarget(
                  ID2D1RenderTarget * const,
                  ID2D1SolidColorBrush * const
                  );


#endif /* !__BATCHACQUISITIONIMAGERENDER_H */
