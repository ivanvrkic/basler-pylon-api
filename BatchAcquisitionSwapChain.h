/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2014-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionSwapChain.h
  \brief  DXGI swap chain creation and deletion.

  \author Tomislav Petkovic
  \date   2017-02-21
*/


#ifndef __BATCHACQUISITIONSWAPCHAIN_H
#define __BATCHACQUISITIONSWAPCHAIN_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionWindowDisplay.h"


//! DirectX adapter and output associated with window handle.
HRESULT
GetDXGIAdapterAndOutputFromWindowHandle(
                                        HWND const,
                                        IDXGIFactory1 * const,
                                        IDXGIAdapter ** const,
                                        IDXGIOutput ** const,
                                        DXGI_MODE_DESC * const
                                        );

//! Find best matching mode for DXGI output.
HRESULT
FindBestMatchingModeForDXGIOutput(
                                  IDXGIOutput * const,
                                  DXGI_MODE_DESC * const,
                                  DXGI_MODE_DESC * const,
                                  IUnknown * const
                                  );

//! Find best refresh rate for DXGI output.
HRESULT
FindBestRefreshRateForDXGIOutput(
                                 DXGI_MODE_DESC * const,
                                 IDXGIOutput * const,
                                 DXGI_RATIONAL * const
                                 );

//! Create swap chain.
HRESULT
SwapChainCreate(
                HWND const,
                IDXGIFactory1 * const,
                DXGI_MODE_DESC * const,
                IDXGIAdapter ** const,
                IDXGIOutput ** const,
                ID3D11Device ** const,
                ID3D11DeviceContext ** const,
                IDXGISwapChain ** const
                );

//! Create render target.
HRESULT
RenderTargetCreate(
                   ID2D1Factory * const,
                   IDXGISwapChain * const,
                   IDXGISurface ** const,
                   ID2D1RenderTarget ** const,
                   ID2D1SolidColorBrush ** const,
                   ID2D1SolidColorBrush ** const
                   );

//! Get refresh rate.
HRESULT
SwapChainGetRefreshRate(
                        IDXGISwapChain * const,
                        DXGI_RATIONAL * const
                        );

//! Gets monitor handle.
HMONITOR
SwapChainGetMonitorHandle(
                          IDXGISwapChain * const
                          );

//! Get user selected mode.
HRESULT
QueryUserToSelectDisplayMode(
                             DisplayWindowParameters * const,
                             int const,
                             DXGI_MODE_DESC * const
                             );


#endif /* !__BATCHACQUISITIONSWAPCHAIN_H */
