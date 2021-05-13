/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014-2021 UniZG, Zagreb. All rights reserved.
 * (c) 2014-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionSwapChain.cpp
  \brief  DXGI swap chain creation and deletion.

  \author Tomislav Petkovic
  \date   2021-04-21
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONSWAPCHAIN_CPP
#define __BATCHACQUISITIONSWAPCHAIN_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionSwapChain.h"
#include "BatchAcquisitionKeyboard.h"
#include "BatchAcquisitionDebug.h"



/****** HELPER FUNCTIONS ******/

//! Frequency from refresh rate.
/*!
  Returns display frequency (in Hz) from DXGI refresh rate.

  \param RefreshRate    Pointer to DXGI refresh rate structure.
  \return Returns display frequency.
*/
inline
double
FrequencyFromRefreshRate_inline(
                                DXGI_RATIONAL const * const RefreshRate
                                )
{
  assert(NULL != RefreshRate);
  if (NULL == RefreshRate) return 0.0;

  if (0 == RefreshRate->Denominator) return 0.0;

  double const frequency = (double)(RefreshRate->Numerator) / (double)(RefreshRate->Denominator);

  return frequency;
}
/* FrequencyFromRefreshRate_inline */



/****** DXGI FUNCTIONS ******/

//! DirectX adapter and output associated with window handle.
/*!
  Function enumerates all DirectX adapters and outputs using DXGI and
  selects ones associated with the window idenitifed by hWnd handle.
  Function also returns matching display mode with highest refresh rate.

  \param hWnd   Window handle.
  \param pDXGIFactory1   Pointer to DXGI factory class.
  \param ppAdapter_out   Memory address where pointer to DXGI adapter will be stored.
  \param ppOutput_out    Memory address where pointer to DXGI output will be stored.
  \param pDisplayMode_out       Pointer to DXGI display mode structure.
  \return Function returns S_OK if successfull.
*/
HRESULT
GetDXGIAdapterAndOutputFromWindowHandle(
                                        HWND const hWnd,
                                        IDXGIFactory1 * const pDXGIFactory1,
                                        IDXGIAdapter ** const ppAdapter_out,
                                        IDXGIOutput ** const ppOutput_out,
                                        DXGI_MODE_DESC * const pDisplayMode_out
                                        )
{
  assert(NULL != pDXGIFactory1);
  if (NULL == pDXGIFactory1) return E_INVALIDARG;

  HRESULT hr = S_OK;
  BOOL br = TRUE;

  // Get current monitor handle and device mode. We will use this to initialize default swap chain.
  HMONITOR const hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
  assert(NULL != hMonitor);
  if (NULL == hMonitor) return E_HANDLE;

  DEVMODE deviceMode;
  ZeroMemory( &deviceMode, sizeof(deviceMode) );
  {
    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    if (TRUE == br)
      {
        br = GetMonitorInfo(hMonitor, &monitorInfo);
        assert(TRUE == br);
      }
    /* if */

    if (TRUE == br)
      {
        br = EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &deviceMode);
        assert(TRUE == br);
      }
    /* if */
  }

  if (TRUE != br) return E_ABORT;

  // Enumerate all adapters (video cards) and outputs (monitors) connected to host PC.
  // Then find one that matches parent window handle and figure out default display mode.
  // Always select mode with the highest possible refresh rate.
  // We will use this mode to (re)initialize DirectX swap chain.

  IDXGIAdapter * pAdapter = NULL;
  IDXGIOutput * pOutput = NULL;

  int idxAdapter = -1;
  int idxOutput = -1;

  DXGI_MODE_DESC sDisplayMode;
  ZeroMemory( &sDisplayMode, sizeof(sDisplayMode) );

  // Create list of all adapters.
  std::vector<IDXGIAdapter *> vAdapters;
  IDXGIAdapter * tmpAdapter = NULL;
  int i = 0;
  while ( DXGI_ERROR_NOT_FOUND != pDXGIFactory1->EnumAdapters((UINT)i, &tmpAdapter) )
    {
      vAdapters.push_back(tmpAdapter);
      ++i;
    }
  /* while */

  // Cycle through all adapters.
  int const maxi = (int)vAdapters.size();
  for (i = 0; i < maxi; ++i)
    {
      // Create list of all outputs.
      std::vector<IDXGIOutput *> vOutputs;
      IDXGIOutput * tmpOutput = NULL;
      int j = 0;
      while ( DXGI_ERROR_NOT_FOUND != vAdapters[i]->EnumOutputs((UINT)j, &tmpOutput) )
        {
          vOutputs.push_back(tmpOutput);
          ++j;
        }
      /* while */

      // Cycle through all outputs.
      int const maxj = (int)vOutputs.size();
      for (j = 0; j < maxj; ++j)
        {
          if ( (-1 == idxAdapter) && (-1 == idxOutput) )
            {
              assert(NULL == pAdapter);
              assert(NULL == pOutput);

              DXGI_OUTPUT_DESC outputDescription;
              hr = vOutputs[j]->GetDesc(&outputDescription);
              assert( SUCCEEDED(hr) );

              if ( SUCCEEDED(hr) && (hMonitor == outputDescription.Monitor) )
                {
                  // Get the number of available modes for selected format.
                  // NOTE: Format must not be changed; or if changed must be one compatible with Direct 2D.
                  DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
                  UINT numModes = 0;

                  HRESULT const hr = vOutputs[j]->GetDisplayModeList(format, 0, &numModes, NULL);
                  assert( SUCCEEDED(hr) );

                  if ( SUCCEEDED(hr) )
                    {
                      // Get the list of modes.
                      DXGI_MODE_DESC * const displayModes = new DXGI_MODE_DESC[numModes];
                      HRESULT const hr = vOutputs[j]->GetDisplayModeList(format, 0, &numModes, displayModes);
                      assert( SUCCEEDED(hr) );
                      if ( SUCCEEDED(hr) )
                        {
                          // Cycle through all display modes.
                          for (UINT k = 0; k < numModes; ++k)
                            {
                              double const freqk = FrequencyFromRefreshRate_inline( &(displayModes[k].RefreshRate) );
                              double const freq = FrequencyFromRefreshRate_inline( &(sDisplayMode.RefreshRate) );

                              if ( (displayModes[k].Width == deviceMode.dmPelsWidth) &&
                                   (displayModes[k].Height == deviceMode.dmPelsHeight) &&
                                   (freqk >= freq)
                                   )
                                {
                                  pAdapter = vAdapters[i];
                                  idxAdapter = i;
                                  pOutput = vOutputs[j];
                                  idxOutput = j;
                                  sDisplayMode = displayModes[k];
                                }
                              /* if */
                            }
                          /* for */
                        }
                      /* if */

                      // Free resources.
                      delete[] displayModes;
                    }
                  /* if */
                }
              /* if */
            }
          /* if */

          if (idxOutput != j) SAFE_RELEASE( vOutputs[j] );
        }
      /* for */
      idxOutput = -1; // So vOutputs[j] may be deleted in the next iterations of outer loop.

      if (idxAdapter != i) SAFE_RELEASE( vAdapters[i] );
    }
  /* for */

  if (NULL != ppAdapter_out)
    {
      *ppAdapter_out = pAdapter;
    }
  else
    {
      SAFE_RELEASE( pAdapter );
    }
  /* if */

  if (NULL != ppOutput_out)
    {
      *ppOutput_out = pOutput;
    }
  else
    {
      SAFE_RELEASE( pOutput );
    }
  /* if */

  if (NULL != pDisplayMode_out) *pDisplayMode_out = sDisplayMode;

  return hr;
}
/* GetDXGIAdapterAndOutputFromWindowHandle */



//! Find best matching mode for DXGI output.
/*!
  Function returns best display mode for selected output and desired target mode.

  \param pOutput        Pointer to DXGI output.
  \param pTargetMode    Pointer to target display mode.
  \param pFoundMode     Found display mode.
  \param pConcernedDevice       A device used for rendering.
  \return Returns S_OK if successfull.
*/
HRESULT
FindBestMatchingModeForDXGIOutput(
                                  IDXGIOutput * const pOutput,
                                  DXGI_MODE_DESC * const pTargetMode,
                                  DXGI_MODE_DESC * const pFoundMode,
                                  IUnknown * const pConcernedDevice
                                  )
{
  assert(NULL != pOutput);
  if (NULL == pOutput) return E_INVALIDARG;

  assert(NULL != pTargetMode);
  if (NULL == pTargetMode) return E_INVALIDARG;

  HRESULT hr = S_OK;

  DXGI_MODE_DESC sFoundMode;
  ZeroMemory( &sFoundMode, sizeof(sFoundMode) );

  UINT numModes = 0;

  hr = pOutput->GetDisplayModeList(pTargetMode->Format, 0, &numModes, NULL);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      DXGI_MODE_DESC * const displayModes = new DXGI_MODE_DESC[numModes];
      hr = pOutput->GetDisplayModeList(pTargetMode->Format, 0, &numModes, displayModes);
      assert( SUCCEEDED(hr) );

      if ( SUCCEEDED(hr) )
        {
          UINT const targetWidth = pTargetMode->Width;
          UINT const targetHeight = pTargetMode->Height;
          double const targetFreq = FrequencyFromRefreshRate_inline( &(pTargetMode->RefreshRate) );

          double delta = DBL_MAX;
          bool selected = false;

          /* First try to find an exact match in resolution. */
          for (UINT k = 0; k < numModes; ++k)
            {
              UINT const width = displayModes[k].Width;
              UINT const height = displayModes[k].Height;
              double const freq = FrequencyFromRefreshRate_inline( &(displayModes[k].RefreshRate) );
              double const deltak = fabs(targetFreq - freq);
              if ( (width == targetWidth) && (height == targetHeight) && (delta > deltak) )
                {
                  delta = deltak;
                  sFoundMode = displayModes[k];
                  selected = true;
                }
              /* if */
            }
          /* for */

          /* If no suitable match is found then fallback to the DXGI selection. */
          if (false == selected)
            {
              hr = pOutput->FindClosestMatchingMode(pTargetMode, &(sFoundMode), pConcernedDevice);
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if (false == selected) hr = E_NOINTERFACE;
        }
      /* if */

      // Free resources.
      delete[] displayModes;
    }
  /* if */

  if (NULL != pFoundMode) *pFoundMode = sFoundMode;

  return hr;
}
/* FindBestMatchingModeForDXGIOutput */



//! Find best refresh rate for DXGI output.
/*!
  Function returns best refresh rate for selected output and desired target mode.

  \param pTargetMode   Target mode.
  \param pOutput        Pointer to DXGI output.
  \param pRefreshRate   Pointer to DXGI_RATIONAL where best available refresh rate will be stored.
  \return Function returns S_OK if successfull.
*/
HRESULT
FindBestRefreshRateForDXGIOutput(
                                 DXGI_MODE_DESC * const pTargetMode,
                                 IDXGIOutput * const pOutput,
                                 DXGI_RATIONAL * const pRefreshRate
                                 )
{
  assert(NULL != pTargetMode);
  if (NULL == pTargetMode) return E_INVALIDARG;

  //assert(NULL != pOutput);
  if (NULL == pOutput) return E_INVALIDARG;

  HRESULT hr = S_OK;

  DXGI_RATIONAL sRefreshRate;
  ZeroMemory( &sRefreshRate, sizeof(sRefreshRate) );

  DXGI_FORMAT format = DEFAULT_DIRECT_X_PIXEL_FORMAT; // Defined in BatchAcquisition.h
  UINT numModes = 0;

  hr = pOutput->GetDisplayModeList(format, 0, &numModes, NULL);
  assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      DXGI_MODE_DESC * const displayModes = new DXGI_MODE_DESC[numModes];
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, displayModes);
      assert( SUCCEEDED(hr) );

      if ( SUCCEEDED(hr) )
        {
          UINT const targetWidth = pTargetMode->Width;
          UINT const targetHeight = pTargetMode->Height;
          double const targetFreq = FrequencyFromRefreshRate_inline( &(pTargetMode->RefreshRate) );

          double delta = DBL_MAX;
          bool selected = false;

          /* First try to find proper match. */
          for (UINT k = 0; k < numModes; ++k)
            {
              UINT const width = displayModes[k].Width;
              UINT const height = displayModes[k].Height;
              double const freq = FrequencyFromRefreshRate_inline( &(displayModes[k].RefreshRate) );
              double const deltak = fabs(targetFreq - freq);
              if ( (width == targetWidth) && (height == targetHeight) && (delta > deltak) )
                {
                  delta = deltak;
                  sRefreshRate = displayModes[k].RefreshRate;
                  selected = true;
                }
              /* if */
            }
          /* for */

          if (false == selected)
            {
              /* Then find any larger match. */
              UINT const targetSize = targetWidth * targetHeight;
              for (UINT k = 0; k < numModes; ++k)
                {
                  UINT const size = displayModes[k].Width * displayModes[k].Height;
                  double const freq = FrequencyFromRefreshRate_inline( &(displayModes[k].RefreshRate) );
                  double const deltak = fabs(targetFreq - freq);
                  if ( (size >= targetSize) && (delta > deltak) )
                    {
                      delta = deltak;
                      sRefreshRate = displayModes[k].RefreshRate;
                      selected = true;
                    }
                  /* if */
                }
              /* for */
            }
          /* if */

          if (false == selected) hr = E_NOINTERFACE;
        }
      /* if */

      // Free resources.
      delete[] displayModes;
    }
  /* if */

  if (NULL != pRefreshRate) *pRefreshRate = sRefreshRate;

  return hr;
}
/* FindBestRefreshRateForDXGIOutput */



//! Create swap chain.
/*!
  Creates DXGI swap chain associated with the window having handle hWnd.

  \param hWnd   Window handle.
  \param pDXGIFactory1   Pointer to DXGI factory class.
  \param pRequestedMode  Requested display mode.
  \param ppAdapter_out DXGI adapter the swap chain is running on.
  \param ppOutput_out   DXGI output the swap chain is running on.
  \param ppDevice_out  D3D device the swap chain is running on.
  \param ppDeviceContext_out D3D device context.
  \param ppSwapChain_out        DXGI swap chain.
  \return Returns S_OK if successfull, or error code otherwise.
*/
HRESULT
SwapChainCreate(
                HWND const hWnd,
                IDXGIFactory1 * const pDXGIFactory1,
                DXGI_MODE_DESC * const pRequestedMode,
                IDXGIAdapter ** const ppAdapter_out,
                IDXGIOutput ** const ppOutput_out,
                ID3D11Device ** const ppDevice_out,
                ID3D11DeviceContext ** const ppDeviceContext_out,
                IDXGISwapChain ** const ppSwapChain_out
                )
{
  HRESULT hr = S_OK;

  // Find matching adapter and output.
  IDXGIAdapter * pAdapter = NULL;
  IDXGIOutput * pOutput = NULL;

  DXGI_MODE_DESC sDisplayMode;
  ZeroMemory( &sDisplayMode, sizeof(sDisplayMode) );

  hr = GetDXGIAdapterAndOutputFromWindowHandle(hWnd, pDXGIFactory1, &pAdapter, &pOutput, &sDisplayMode);
  assert( SUCCEEDED(hr) );

  // Always zero-out DXGI structures as forgetting to set some parameter may lead to errors.
  // Zeroing out ensures default and/or neutral values for all members.
  DXGI_SWAP_CHAIN_DESC sSwapChainDescription;
  ZeroMemory( &sSwapChainDescription, sizeof(sSwapChainDescription) );

  sSwapChainDescription.BufferDesc.Width = 0; // If 0 then width and height will be inferred from the application window.
  sSwapChainDescription.BufferDesc.Height = 0;
  sSwapChainDescription.BufferDesc.RefreshRate = sDisplayMode.RefreshRate;
  sSwapChainDescription.BufferDesc.Format = DEFAULT_DIRECT_X_PIXEL_FORMAT; // Defined in BatchAcquisition.h
  sSwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
  sSwapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
  sSwapChainDescription.SampleDesc.Count = 1; // The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0.
  sSwapChainDescription.SampleDesc.Quality = 0;
  sSwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sSwapChainDescription.BufferCount = 1 + 1; // Front buffer included.
  sSwapChainDescription.OutputWindow = hWnd; // Bind to display window opened before.
  sSwapChainDescription.Windowed = TRUE; // DXGI documentation recommend that initialization is always done with this flag set to TRUE.
  sSwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
  sSwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  assert(DXGI_MAX_SWAP_CHAIN_BUFFERS > sSwapChainDescription.BufferCount); // There is upper limit to number of buffers!

  if ( SUCCEEDED(hr) && (NULL != pRequestedMode) )
    {
      DXGI_RATIONAL sRefreshRate;
      HRESULT const hr_refresh_rate = FindBestRefreshRateForDXGIOutput(pRequestedMode, pOutput, &sRefreshRate);
      //assert( SUCCEEDED(hr_refresh_rate) );
      if ( SUCCEEDED(hr_refresh_rate) )
        {
          sSwapChainDescription.BufferDesc.Width = pRequestedMode->Width;
          sSwapChainDescription.BufferDesc.Height = pRequestedMode->Height;
          sSwapChainDescription.BufferDesc.RefreshRate = sRefreshRate;
        }
      /* if */
    }
  /* if */

  /* Here we first create a D3D device and then we create the swap chain.
     If any error is thrown during the creation phase examine the debugger output
     window where the error message will be displayed. Also convert the return
     value hr to a Dword HEX value and then search for that error code in MSDN.

     Remarks:

     1) When creating D3D 11 device on known IDXGIAdapter (not NULL) the flag
     parameter to the create function must be D3D_DRIVER_TYPE_UNKNOWN;
     if D3D_DRIVER_TYPE_HARDWARE is passed then create function will always
     return E_INVALIDARG. This is in sharp contrast with previous DirectX
     versions (10 and earlier) where D3D_DRIVER_TYPE_HARDWARE is allowed.

     2) For interoperability with Direct2D one of the flags for D3D device
     must be D3D10_CREATE_DEVICE_BGRA_SUPPORT.
  */

  IDXGISwapChain * pSwapChain = NULL;
  ID3D11Device * pDevice = NULL;
  ID3D11DeviceContext * pDeviceContext = NULL;

  D3D_FEATURE_LEVEL const pD3DLevels[] =
    {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0
    };
  int const num_levels = 3; // We have defined three levels!

  D3D_FEATURE_LEVEL sFeatureLevel = D3D_FEATURE_LEVEL_11_0;

  UINT Flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT; // Required for interoperability with Direct 2D!
#ifdef _DEBUG
  //Flags = Flags | D3D11_CREATE_DEVICE_DEBUG; // Request extended debug information.
#endif /* !_DEBUG */

  if ( SUCCEEDED(hr) )
    {
      hr = D3D11CreateDeviceAndSwapChain(
                                         pAdapter,
                                         D3D_DRIVER_TYPE_UNKNOWN,
                                         NULL, // Not required for hardware device.
                                         Flags,
                                         pD3DLevels,
                                         num_levels,
                                         D3D11_SDK_VERSION,
                                         &sSwapChainDescription,
                                         &pSwapChain,
                                         &pDevice,
                                         &sFeatureLevel,
                                         &pDeviceContext
                                         );
      assert( SUCCEEDED(hr) );
      assert( NULL != pSwapChain );
      assert( NULL != pDevice );
      assert( NULL != pDeviceContext );
    }
  /* if */

  if ( SUCCEEDED(hr) && (NULL != pSwapChain) && (NULL == pOutput) )
    {
      assert(NULL == pOutput);
      hr = pSwapChain->GetContainingOutput(&pOutput);
      //assert( SUCCEEDED(hr) );
      if ( !SUCCEEDED(hr) )
        {
          /* For some reason the adapter used to create DXGI swap chain cannot access
             the output device which is used; one reason may be the monitor was attached
             to the system after the application started. For such cases the DXGI factory
             must be recreated (at the application level) and only then the swap chain
             may be recreated successfully.
          */

          Debugfwprintf(stderr, gDbgCannotGetContainingOutput);

          int const cnt = wprintf(gMsgRestartApplication);
          assert(0 < cnt);

          // TODO!
        }
      /* if */
    }
  /* if */

#ifdef DEBUG
  if ( SUCCEEDED(hr) )
    {
      HMONITOR const hMon1 = SwapChainGetMonitorHandle(pSwapChain);
      HMONITOR const hMon2 = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
      assert(hMon1 == hMon2);
    }
  /* if */
#endif /* DEBUG */

  if (NULL != ppAdapter_out)
    {
      *ppAdapter_out = pAdapter;
    }
  else
    {
      SAFE_RELEASE( pAdapter );
    }
  /* if */

  if (NULL != ppOutput_out)
    {
      *ppOutput_out = pOutput;
    }
  else
    {
      SAFE_RELEASE( pOutput );
    }
  /* if */

  if (NULL != ppSwapChain_out)
    {
      *ppSwapChain_out = pSwapChain;
    }
  else
    {
      SAFE_RELEASE( pSwapChain );
    }
  /* if */

  if (NULL != ppDevice_out)
    {
      *ppDevice_out = pDevice;
    }
  else
    {
      SAFE_RELEASE( pDevice );
    }
  /* if */

  if (NULL != ppDeviceContext_out)
    {
      *ppDeviceContext_out = pDeviceContext;
    }
  else
    {
      SAFE_RELEASE( pDeviceContext );
    }
  /* if */

  return hr;
}
/* SwapChainCreate */



//! Create render target.
/*!
  Creates DXGI render target associated with the swap chain.

  \param pD2DFactory  Pointer to Direct2D factory object.
  \param pSwapChain     Pointer to swap chain.
  \param ppBackBuffer_out       Back buffer of the swap chain.
  \param ppRenderTarget_out     Direct2D render target associated with the swap chain.
  \param ppBlackBrush_out       Black brush associated with the Direct2D render target.
  \param ppYellowBrush_out       Yellow brush associated with the Direct2D render target.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
RenderTargetCreate(
                   ID2D1Factory * const pD2DFactory,
                   IDXGISwapChain * const pSwapChain,
                   IDXGISurface ** const ppBackBuffer_out,
                   ID2D1RenderTarget ** const ppRenderTarget_out,
                   ID2D1SolidColorBrush ** const ppBlackBrush_out,
                   ID2D1SolidColorBrush ** const ppYellowBrush_out
                   )
{
  assert(NULL != pD2DFactory);
  if (NULL == pD2DFactory) return E_POINTER;

  assert(NULL != pSwapChain);
  if (NULL == pSwapChain) return E_POINTER;

  HRESULT hr = S_OK;

  DXGI_SURFACE_DESC BackBufferDesc;
  DXGI_SWAP_CHAIN_DESC SwapChainDesc;

  IDXGISurface * pBackBuffer = NULL;
  ID2D1RenderTarget * pRenderTarget = NULL;
  ID2D1SolidColorBrush * pBlackBrush = NULL;
  ID2D1SolidColorBrush * pYellowBrush = NULL;

  // Get a DXGI surface from the swap chain.
  if ( SUCCEEDED(hr) )
    {
      hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
      assert( SUCCEEDED(hr) );
      assert(NULL != pBackBuffer);
    }
  /* if */

  // Get a DXGI swap chain description.
  if ( SUCCEEDED(hr) )
    {
      hr = pSwapChain->GetDesc(&SwapChainDesc);
      assert( SUCCEEDED(hr) );
    }
  /* if */
  
  // Get a DXGI surface description.
  if (SUCCEEDED(hr))
  {
      hr = pBackBuffer->GetDesc(&BackBufferDesc);
      assert(SUCCEEDED(hr));
  }
  /* if */

  // Create the DXGI Surface Render Target.
  if ( SUCCEEDED(hr) )
    {
      FLOAT dpiX = 96.0f;
      FLOAT dpiY = 96.0f;
      
      UINT const dpi = GetDpiForWindow(SwapChainDesc.OutputWindow);
      assert(0 != dpi);
      if (0 < dpi)
        {
          dpiX = (float)dpi;
          dpiY = dpiX;
        }
      /* if */

      D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
                                     D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                     D2D1::PixelFormat(BackBufferDesc.Format, D2D1_ALPHA_MODE_IGNORE),
                                     dpiX,
                                     dpiY,
                                     D2D1_RENDER_TARGET_USAGE_NONE,
                                     D2D1_FEATURE_LEVEL_10
                                     );

      hr = pD2DFactory->CreateDxgiSurfaceRenderTarget(
                                                      pBackBuffer,
                                                      props,
                                                      &pRenderTarget
                                                      );
      assert( SUCCEEDED(hr) );
      assert(NULL != pRenderTarget);
    }
  /* if */

  // Create solid color brushes.
  if ( SUCCEEDED(hr) && (NULL != ppBlackBrush_out) )
    {
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
                                                &pBlackBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) && (NULL != ppYellowBrush_out) )
    {
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f),
                                                &pYellowBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if (NULL != ppBackBuffer_out)
    {
      *ppBackBuffer_out = pBackBuffer;
    }
  else
    {
      SAFE_RELEASE(pBackBuffer);
    }
  /* if */

  if (NULL != ppRenderTarget_out)
    {
      *ppRenderTarget_out = pRenderTarget;
    }
  else
    {
      SAFE_RELEASE(pRenderTarget);
    }
  /* if */

  if (NULL != ppBlackBrush_out)
    {
      *ppBlackBrush_out = pBlackBrush;
    }
  else
    {
      SAFE_RELEASE(pBlackBrush);
    }
  /* if */

  if (NULL != ppYellowBrush_out)
    {
      *ppYellowBrush_out = pYellowBrush;
    }
  else
    {
      SAFE_RELEASE(pYellowBrush);
    }
  /* if */

  return hr;
}
/* RenderTargetCreate */



//! Get refresh rate.
/*!
  Returns swap chain refresh rate.

  \param pSwapChain     Pointer to swap chain.
  \param pRational      Pointer to DXGI_RATIONAL structure.
  \return Function returns S_OK if successfull and error code otherwise.
*/
HRESULT
SwapChainGetRefreshRate(
                        IDXGISwapChain * const pSwapChain,
                        DXGI_RATIONAL * const pRational
                        )
{
  assert(NULL != pSwapChain);
  if (NULL == pSwapChain) return E_INVALIDARG;

  assert(NULL != pRational);
  if (NULL == pRational) return E_INVALIDARG;

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  ZeroMemory( &swap_chain_desc, sizeof(swap_chain_desc) );

  HRESULT hr = S_OK;

  hr = pSwapChain->GetDesc( &swap_chain_desc );
  assert( SUCCEEDED(hr) );

  DXGI_RATIONAL const sRefreshRate = swap_chain_desc.BufferDesc.RefreshRate;

  if ( SUCCEEDED(hr) && (0 < sRefreshRate.Numerator) && (0 < sRefreshRate.Denominator) )
    {
      *pRational = swap_chain_desc.BufferDesc.RefreshRate;
    }
  else
    {
      HMONITOR const hMonitor = SwapChainGetMonitorHandle(pSwapChain);
      assert(NULL != hMonitor);
      if (NULL != hMonitor)
        {
          BOOL br = TRUE;

          DEVMODE deviceMode;
          ZeroMemory( &deviceMode, sizeof(deviceMode) );

          MONITORINFOEX monitorInfo;
          monitorInfo.cbSize = sizeof(MONITORINFOEX);

          if (TRUE == br)
            {
              br = GetMonitorInfo(hMonitor, &monitorInfo);
              assert(TRUE == br);
            }
          /* if */

          if (TRUE == br)
            {
              br = EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &deviceMode);
              assert(TRUE == br);
            }
          /* if */

          if ( (TRUE == br) && (0 != deviceMode.dmDisplayFrequency) && (1 != deviceMode.dmDisplayFrequency) )
            {
              pRational->Numerator = deviceMode.dmDisplayFrequency;
              pRational->Denominator = 1;

              hr = S_OK;
            }
          else
            {
              hr = E_FAIL;
            }
          /* if */
        }
      else
        {
          hr = E_FAIL;
        }
      /* if */
    }
  /* if */

  return hr;
}
/* SwapChainGetRefreshRate */



//! Gets monitor handle.
/*!
  Gets handle of the monitor associated with the swap chain.

  \param pSwapChain     Pointer to swap chain.
  \return Returns monitor handle or NULL if unsuccessfull.
*/
HMONITOR
SwapChainGetMonitorHandle(
                          IDXGISwapChain * const pSwapChain
                          )
{
  HMONITOR hMonitor = NULL;

  //assert(NULL != pSwapChain);
  if (NULL == pSwapChain) return hMonitor;

  HRESULT hr = S_OK;

  IDXGIOutput * pOutput = NULL;
  hr = pSwapChain->GetContainingOutput( &pOutput );
  //assert( SUCCEEDED(hr) );

  if ( SUCCEEDED(hr) )
    {
      DXGI_OUTPUT_DESC outputDescription;
      hr = pOutput->GetDesc(&outputDescription);
      assert( SUCCEEDED(hr) );
      if ( SUCCEEDED(hr) ) hMonitor = outputDescription.Monitor;
    }
  /* if */

  SAFE_RELEASE( pOutput );

  return hMonitor;
}
/* SwapChainGetMonitorHandle */



//! Lists available Direct X fullscreen modes.
/*!
  Lists all available Direct X fullscreen modes and prompts user
  to select one.

  \param pWindowDisplay  Pointer to display window parameters.
  \param ProjectorID     Unique projector ID.
  \param pDisplayMode_out    Pointer to DXGI_MODE_DESC structure where chosen display mode will be stored. May be NULL
  \return Returns DXGI display mode structure. If user does not select
  display mode default one is used.
*/
HRESULT
QueryUserToSelectDisplayMode(
                             DisplayWindowParameters * const pWindowDisplay,
                             int const ProjectorID,
                             DXGI_MODE_DESC * const pDisplayMode_out
                             )
{
  assert(NULL != pWindowDisplay);
  if (NULL == pWindowDisplay) return E_INVALIDARG;

  IDXGISwapChain * pSwapChain = pWindowDisplay->pSwapChain;

  assert(NULL != pSwapChain);
  if (NULL == pSwapChain) return E_POINTER;

  DXGI_MODE_DESC displayMode;
  ZeroMemory( &displayMode, sizeof(displayMode) );

  MONITORINFOEX monitorInfo;
  ZeroMemory( &monitorInfo, sizeof(MONITORINFOEX) );
  monitorInfo.cbSize = sizeof(MONITORINFOEX);

  DEVMODE monitorMode;
  ZeroMemory( &monitorMode, sizeof(DEVMODE) );

  HMONITOR hMonitor = NULL;

  IDXGIOutput * pOutput = NULL;

  DXGI_MODE_DESC * displayModes = NULL;
  UINT numModes = 0;

  int selectedMode = -1;

  DXGI_FORMAT format = DEFAULT_DIRECT_X_PIXEL_FORMAT; // Defined in BatchAcquisition.h

  HRESULT hr = S_OK;

  /* First get containing DXGI output and construct a list of allowed video modes.
     A valid DXGI output is necessary to construct a list of supported video modes.
     Note that the containing DXGI output may be invalid if the monitor was removed;
     if so then we recreate the DXGI swap chain to use a valid DXGI output.
  */
  hr = pSwapChain->GetContainingOutput( &pOutput );
  //assert( SUCCEEDED(hr) );

  if (DXGI_ERROR_UNSUPPORTED == hr)
    {
      assert( pSwapChain == pWindowDisplay->pSwapChain );

      bool const fModeChange = pWindowDisplay->fModeChange;
      pWindowDisplay->fModeChange = true;

      EnterCriticalSection( &(pWindowDisplay->csRenderAndPresent) );
      {
        EnterCriticalSection( &(pWindowDisplay->csWaitForVBLANK) );
        {
          hr = RecreateDirectXDeviceAndSwapChain(pWindowDisplay);
          assert( SUCCEEDED(hr) );
        }
        LeaveCriticalSection( &(pWindowDisplay->csWaitForVBLANK) );
      }
      LeaveCriticalSection( &(pWindowDisplay->csRenderAndPresent) );

      pWindowDisplay->fModeChange = fModeChange;

      if ( SUCCEEDED(hr) )
        {
          pSwapChain = pWindowDisplay->pSwapChain;

          hr = pSwapChain->GetContainingOutput( &pOutput );
          assert( SUCCEEDED(hr) );
        }
      else
        {
          pSwapChain = NULL;
        }
      /* if */
    }
  /* if */

  if ( !SUCCEEDED(hr) || (NULL == pOutput) )
    {
      hr = E_FAIL;

      int const cnt1 = wprintf(gMsgCannotGetContainingOutput, ProjectorID + 1);
      assert(0 < cnt1);

      int const cnt2 = wprintf(gMsgCycleThroughFullScreenAndWindowedMode, ProjectorID + 1);
      assert(0 < cnt2);

      goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      displayModes = new DXGI_MODE_DESC[numModes];
      assert(NULL != displayModes);
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, displayModes);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( !SUCCEEDED(hr) || (0 == numModes) || (NULL == displayModes) )
    {
      hr = E_FAIL;

      int const cnt1 = wprintf(gMsgCannotGetDisplayModeList, ProjectorID + 1);
      assert(0 < cnt1);

      int const cnt2 = wprintf(gMsgCycleThroughFullScreenAndWindowedMode, ProjectorID + 1);
      assert(0 < cnt2);

      goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
    }
  /* if */


  /* Then get monitor handle. The monitor handle will be used to figure out
     currently active video mode for a particular display. This mode will
     be presented to the user as the default choice. If the mode is not
     appropriate then the user may list all available modes and select
     which one to use.
  */
  hMonitor = MonitorFromWindow(pWindowDisplay->hWnd, MONITOR_DEFAULTTOPRIMARY);
  assert(NULL != hMonitor);
  if (NULL == hMonitor)
    {
      hr = E_FAIL;

      int const cnt = wprintf(gMsgCannotGetMonitorHandle, ProjectorID + 1);
      assert(0 < cnt);

      goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
    }
  /* if */

  BOOL const get_info = GetMonitorInfo(hMonitor, &monitorInfo);
  assert(TRUE == get_info);
  if (TRUE != get_info)
    {
      hr = E_FAIL;

      int const cnt = wprintf(gMsgCannotGetMonitorData, ProjectorID + 1);
      assert(0 < cnt);

      goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
    }
  /* if */

  BOOL const get_mode = EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &monitorMode);
  assert(TRUE == get_mode);
  if (TRUE != get_mode)
    {
      hr = E_FAIL;

      int const cnt = wprintf(gMsgCannotGetMonitorData, ProjectorID + 1);
      assert(0 < cnt);

      goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
    }
  /* if */


  /* Get default display mode. */
  if ( SUCCEEDED(hr) )
    {
      LONG const monitor_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
      assert(0 < monitor_height);

      LONG const monitor_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
      assert(0 < monitor_width);

      DWORD const monitor_freq = monitorMode.dmDisplayFrequency;
      assert(0 < monitor_freq);

      /* First try to find an exact match. */
      for (unsigned int i = 0; i < numModes; ++i)
        {
          LONG const width_i = (LONG)( displayModes[i].Width );
          LONG const height_i = (LONG)( displayModes[i].Height );

          double const num = (double)( displayModes[i].RefreshRate.Numerator );
          double const den = (double)( displayModes[i].RefreshRate.Denominator );
          DWORD const freq_i = (DWORD)( num / den + 0.5 );

          if ( (monitor_width == width_i) &&
               (monitor_height == height_i) &&
               (monitor_freq == freq_i)
               )
            {
              selectedMode = i;
              break;
            }
          /* if */
        }
      /* for */

      /* If there is no exact match try to find approximate match. */
      if (-1 == selectedMode)
        {
          LONG selectedQuality = LONG_MAX;
          for (unsigned int i = 0; i < numModes; ++i)
            {
              LONG const width_i = (LONG)( displayModes[i].Width );
              LONG const height_i = (LONG)( displayModes[i].Height );

              double const num = (double)(displayModes[i].RefreshRate.Numerator);
              double const den = (double)(displayModes[i].RefreshRate.Denominator);
              DWORD const freq_i = (DWORD)(num / den + 0.5);

              LONG const quality_i = labs(width_i - monitor_width) + labs(height_i - monitor_height) + labs(freq_i - monitor_freq);
              if (quality_i < selectedQuality)
                {
                  selectedQuality = quality_i;
                  selectedMode = i;
                }
              /* if */
            }
          /* for */
        }
      /* if */
    }
  /* if */

  /* Print quick menu. */
  {
    assert( SUCCEEDED(hr) );

    int const cnt1 = wprintf(L"\n");
    assert(0 < cnt1);

    double const num = (double)(displayModes[selectedMode].RefreshRate.Numerator);
    double const den = (double)(displayModes[selectedMode].RefreshRate.Denominator);
    double const frequency = num / den;

    int const cnt2 = wprintf(
                             gMsgQuickResolutionMenu,
                             ProjectorID + 1,
                             displayModes[selectedMode].Width, displayModes[selectedMode].Height, frequency
                             );
    assert(0 < cnt2);

    int const pressed_key = TimedWaitForNumberKey(30000, 10, true, true, (HWND)NULL);
    if (0 == pressed_key) // Return to main menu.
      {
        hr = E_FAIL;
        goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
      }
    else if (2 == pressed_key) // List all display modes.
      {
        goto SWAP_CHAIN_SELECT_DISPLAY_MODE_FULL_MENU;
      }
    else // Default action.
      {
        assert( (0 <= selectedMode) && (selectedMode < (int)(numModes)) );
        displayMode = displayModes[selectedMode];
        goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
      }
    /* if */
  }

  /* Print full menu. */
  {

  SWAP_CHAIN_SELECT_DISPLAY_MODE_FULL_MENU:

    assert( SUCCEEDED(hr) );

    {
      int const cnt1 = wprintf(L"\n");
      int const cnt2 = wprintf(gMsgFullscreenModeMenu, ProjectorID + 1);
      assert( (0 < cnt1) && (0 < cnt2) );
    }
    for (unsigned int i = 0; i < numModes; ++i)
      {
        double const num = (double)(displayModes[i].RefreshRate.Numerator);
        double const den = (double)(displayModes[i].RefreshRate.Denominator);
        double const frequency = num / den;

        if (i == selectedMode)
          {
            int const cnt = wprintf(
                                    gMsgFullscreenModeMenuItemDefault,
                                    i + 1,
                                    displayModes[i].Width, displayModes[i].Height,
                                    frequency,
                                    num, den
                                    );
            assert(0 < cnt);
          }
        else
          {
            int const cnt = wprintf(
                                    gMsgFullscreenModeMenuItem,
                                    i + 1,
                                    displayModes[i].Width, displayModes[i].Height,
                                    frequency,
                                    num, den
                                    );
            assert(0 < cnt);
          }
        /* if */
      }
    /* for */

    {
      int const cnt = wprintf(gMsgFullscreenModeMenuQuery, ProjectorID + 1);
      int idx = -1;
      int const scan = scanf_s("%d", &idx);
      if (1 == scan)
        {
          idx -= 1;
          if ( (0 <= idx) && (idx < (int)(numModes)) )
            {
              displayMode = displayModes[idx];
            }
          else
            {
              assert( (0 <= selectedMode) && (selectedMode < (int)(numModes)) );
              displayMode = displayModes[selectedMode];
              int const cnt = wprintf(gMsgFullscreenModeInvalidResponse, ProjectorID + 1);
              assert(0 < cnt);
            }
          /* if */
          goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
        }
      else
        {
          hr = E_FAIL;
          goto SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT;
        }
      /* if */
    }

  }

 SWAP_CHAIN_SELECT_DISPLAY_MODE_EXIT:

  SAFE_RELEASE( pOutput );
  SAFE_DELETE_ARRAY( displayModes );

  if ( SUCCEEDED(hr) )
    {
      double const num_old = (double)( pWindowDisplay->sFullScreenMode.RefreshRate.Numerator );
      double const den_old = (double)( pWindowDisplay->sFullScreenMode.RefreshRate.Denominator );
      double const freq_old = num_old / den_old;

      double const num_new = (double)( displayMode.RefreshRate.Numerator );
      double const den_new = (double)( displayMode.RefreshRate.Denominator );
      double const freq_new = num_new / den_new;

      int const cnt1 = wprintf(L"\n");
      assert(0 < cnt1);

      int const cnt2 = wprintf(
                               gMsgFullscreenModeChanged,
                               ProjectorID + 1,
                               pWindowDisplay->sFullScreenMode.Width, pWindowDisplay->sFullScreenMode.Height, freq_old,
                               displayMode.Width, displayMode.Height, freq_new
                               );
      assert(0 < cnt2);

      if (NULL != pDisplayMode_out) *(pDisplayMode_out) = displayMode;
      pWindowDisplay->sFullScreenMode = displayMode;

      BOOL const post = PostMessage(pWindowDisplay->hWnd, WM_COMMAND, DISPLAY_WINDOW_CHANGE_RESOLUTION, 0);
      assert(TRUE == post);
    }
  else
    {
      double const num_old = (double)( pWindowDisplay->sFullScreenMode.RefreshRate.Numerator );
      double const den_old = (double)( pWindowDisplay->sFullScreenMode.RefreshRate.Denominator );
      double const freq_old = num_old / den_old;

      int const cnt1 = wprintf(L"\n");
      assert(0 < cnt1);

      int const cnt2 = wprintf(
                               gMsgFullscreenModeNotChanged,
                               ProjectorID + 1,
                               pWindowDisplay->sFullScreenMode.Width, pWindowDisplay->sFullScreenMode.Height, freq_old
                               );
      assert(0 < cnt2);
    }
  /* if */

  return hr;
}
/* QueryUserToSelectDisplayMode */




#endif /* !__BATCHACQUISITIONSWAPCHAIN_CPP */
