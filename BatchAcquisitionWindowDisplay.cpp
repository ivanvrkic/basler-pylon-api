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
  \file   BatchAcquisitionWindowDisplay.cpp
  \brief  Empty display window.

  Create empty display window and run associated message pump.
  The message pump will run in a separate thread.

  \author Tomislav Petkovic
  \date   2017-03-03
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONWINDOWDISPLAY_CPP
#define __BATCHACQUISITIONWINDOWDISPLAY_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionWindowDisplay.h"
#include "BatchAcquisitionWindowStorage.h"
#include "BatchAcquisitionSwapChain.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionImageRender.h"
#include "BatchAcquisitionPatternSolid.h"
#include "BatchAcquisitionRendering.h"



/****** INLINE HELPER FUNCTIONS ******/

#pragma region // Blank and release

//! Blank DisplayWindowParameters structure.
/*!
  Blanks structure.

  \param ptr Pointer to DisplayWindowParameters structure.
*/
inline
void
DisplayWindowParametersBlank_inline(
                                    DisplayWindowParameters * const ptr
                                    )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->hInstance = NULL;
  ptr->hPrevInstance = NULL;

  ZeroMemory( ptr->szTitle, sizeof(ptr->szTitle) );
  ZeroMemory( ptr->szWindowClass, sizeof(ptr->szWindowClass) );

  ptr->nCmdShow = SW_SHOW;
  ptr->hWnd = NULL;
  ptr->hWndParent = NULL;
  ptr->hWndCommand = NULL;
  ptr->tWindow = NULL;
  ptr->ProjectorID = -1;
  ptr->width = 0;
  ptr->height = 0;
  ptr->pMsg = NULL;
  ptr->fActive = false;
  ptr->fFullscreen = false;
  ptr->fModeChange = true;
  ptr->fRecreated = true;
  ptr->fResized = true;
  ptr->fFreeze = false;
  ptr->fRenderAndPresent = false;
  ptr->fWaitForVBLANK = false;
  ptr->fBlocking = true;
  ptr->fConcurrentDelay = false;
  ptr->fFixed = false;
  ptr->num_acquire = -1;
  ptr->pAdapter = NULL;
  ptr->pOutput = NULL;
  ptr->pDevice = NULL;
  ptr->pDeviceContext = NULL;
  ptr->pSwapChain = NULL;
  ptr->pBackBuffer = NULL;
  ptr->pRenderTarget = NULL;
  ptr->pBlackBrush = NULL;
  ptr->pYellowBrush = NULL;
  ptr->pTextFormat = NULL;
  ptr->hSwapChainMonitor = NULL;
  ptr->pDXGIFactory1 = NULL;
  ptr->pD2DFactory = NULL;

  ZeroMemory( &(ptr->sSwapChainDesc), sizeof(ptr->sSwapChainDesc) );
  ZeroMemory( &(ptr->sRefreshRate), sizeof(ptr->sRefreshRate) );
  ZeroMemory( &(ptr->sCurrentMode), sizeof(ptr->sCurrentMode) );
  ZeroMemory( &(ptr->sWindowMode), sizeof(ptr->sWindowMode) );
  ZeroMemory( &(ptr->sFullScreenMode), sizeof(ptr->sFullScreenMode) );
  ZeroMemory( &(ptr->sStatisticsPresent), sizeof(ptr->sStatisticsPresent) );

  ptr->frequency.QuadPart = (LONGLONG)0;
  ptr->inv_frequency = BATCHACQUISITION_qNaN_dv;

  ptr->ticks_to_us = BATCHACQUISITION_qNaN_dv;
  ptr->us_to_ticks = BATCHACQUISITION_qNaN_dv;
  ptr->ticks_to_ms = BATCHACQUISITION_qNaN_dv;
  ptr->ms_to_ticks = BATCHACQUISITION_qNaN_dv;

  ptr->us_to_vblanks = BATCHACQUISITION_qNaN_dv;
  ptr->vblanks_to_us = BATCHACQUISITION_qNaN_dv;
  ptr->ticks_to_vblanks = BATCHACQUISITION_qNaN_dv;
  ptr->vblanks_to_ticks = BATCHACQUISITION_qNaN_dv;

  ptr->presentTime = -1;
  ptr->presentTime_us = BATCHACQUISITION_qNaN_dv;
  ptr->refreshTime_ms = BATCHACQUISITION_qNaN_dv;
  ptr->QPC_presentTime = -1;
  ptr->QPC_refreshTime = -1;

  ptr->delayTime_ms = -1;
  ptr->delayTime_us = BATCHACQUISITION_qNaN_dv;
  ptr->delayTime_fraction_us = BATCHACQUISITION_qNaN_dv;
  ptr->delayTime_whole = -1;
  ptr->QPC_delayTime = -1;
  ptr->QPC_delayTime_whole = -1;
  ptr->QPC_delay_for_trigger_scheduled_RT = -1;
  ptr->QPC_delay_for_trigger_scheduled_AT = -1;
  ptr->QPC_delayDelta = -1;

  ptr->exposureTime_whole = -1;
  ptr->QPC_exposureTime = -1;

  ptr->vblank_counter = -1;
  ptr->present_counter = -1;

  ptr->vblank_counter_after_present_RT = -1;
  ptr->present_counter_after_present_RT = -1;

  ptr->pRendering = NULL;
  ptr->pImage = NULL;

  ZeroMemory( &(ptr->sLockRT), sizeof(ptr->sLockRT) );
  ZeroMemory( &(ptr->sLockImage), sizeof(ptr->sLockImage) );

  ZeroMemory( &(ptr->csRenderAndPresent), sizeof(ptr->csRenderAndPresent) );
  ZeroMemory( &(ptr->csWaitForVBLANK), sizeof(ptr->csWaitForVBLANK) );


  /* Set default present and delay time for non-blocking acquisition mode. */

  ptr->presentTime = 4; // Default is four VBlank intervals.

  //ptr->delayTime_ms = 16.7; // Mitsubishi EW230U-ST using HDMI connection at 120Hz refresh.
  //ptr->delayTime_ms = 49.0; // Acer X1260 using VGA connection at 60Hz refresh.
  ptr->delayTime_ms = 45.0;

}
/* DisplayWindowParametersBlank_inline */



//! Releases display window resources.
/*!
  Releases resources allocated by display window thread.

  \param ptr    Pointer to DisplayWindowParameters structure.
*/
inline
void
DisplayWindowParametersRelease_inline(
                                      DisplayWindowParameters * const ptr
                                      )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  // Delete DXGI swap chain.
  DeleteDirectXDeviceAndSwapChain( ptr );

  DeleteCriticalSection( &(ptr->csRenderAndPresent) );
  DeleteCriticalSection( &(ptr->csWaitForVBLANK) );

  PastMessagesDelete(ptr->pMsg);

  AcquireSRWLockExclusive( &(ptr->sLockImage) );
  {
    SAFE_DELETE( ptr->pImage );
  }
  ReleaseSRWLockExclusive( &(ptr->sLockImage) );

  DisplayWindowParametersBlank_inline( ptr );

  free( ptr );
}
/* DisplayWindowParametersRelease_inline */

#pragma endregion // Blank and release


#pragma region // Window message tracking

//! Stores message into list.
/*!
  Function stored message into a list.

  \param hWnd   A handle to the window procedure that received the message.
  \param message        Received message.
  \param wParam  Additional message information.
  \param lParam Additional message information.
*/
inline
void
AddMessageToList_inline(
                        HWND const hWnd,
                        UINT const message,
                        WPARAM const wParam,
                        LPARAM const lParam
                        )
{
  DisplayWindowParameters * const ptr = (DisplayWindowParameters *)GetWindowData(hWnd);
  if (NULL == ptr) return;
  AddMessage(ptr->pMsg, message, wParam, lParam);
  //PrintWindowMessageToConsole(hWnd, message, wParam, lParam);
}
/* AddMessageToList_inline */

#pragma endregion // Window message tracking


#pragma region // Set window title

//! Update window title.
/*!
  Function updates window title.

  \param ptr    Pointer to DisplayWindowParameters structure.
*/
inline
void
UpdateCurrentWindowTitle_inline(
                                DisplayWindowParameters * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  wchar_t const * pDirectory = NULL;
  if (NULL != ptr->pRendering)
    {
      pDirectory = RenderingThreadGetInputDirectory( (RenderingParameters *)(ptr->pRendering) );
      assert(NULL != pDirectory);
    }
  /* if */

  if (NULL != pDirectory)
    {
      int const cnt = swprintf_s(ptr->szTitle, MAX_LOADSTRING, _T("[PRJ %d] %s"), ptr->ProjectorID + 1, pDirectory);
      assert(0 < cnt);
      ptr->szTitle[MAX_LOADSTRING] = 0;      
    }
  else
    {
      int const cnt = swprintf_s(ptr->szTitle, MAX_LOADSTRING, _T("[PRJ %d] %s"), ptr->ProjectorID + 1, gNameWindowDisplay);
      assert(0 < cnt);
      ptr->szTitle[MAX_LOADSTRING] = 0;
    }
  /* if */

  BOOL const set_title = SetWindowText(ptr->hWnd, ptr->szTitle);
  assert(0 != set_title);
}
/* UpdateCurrentWindowTitle_inline */

#pragma endregion // Set window title


#pragma region // DXGI swap chain inline helpers

//! Update current display mode.
/*!
  Updates target display mode to match current one of active swap chain.

  Note that this function should only be used in code blocks protected
  by critical sections csRenderAndPresent and/or csWaitForVBLANK of DisplayWindowParameters
  structure.

  \param ptr    Pointer to DisplayWindowParameters structure.
*/
inline
void
UpdateCurrentDisplayMode_inline(
                                DisplayWindowParameters * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return;

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  ZeroMemory( &swap_chain_desc, sizeof(swap_chain_desc) );

  HRESULT const hr = ptr->pSwapChain->GetDesc( &swap_chain_desc );
  assert( SUCCEEDED(hr) );
  if ( SUCCEEDED(hr) ) ptr->sCurrentMode = swap_chain_desc.BufferDesc;
}
/* UpdateCurrentDisplayMode_inline */



//! Makes swap chain fullscreen.
/*!
  Forces swap chain to fullscreen mode.

  Note that this function does not contain critical section block so it may crash DXGI.
  It shall be called only if enclosed inside the critical section blocks csRenderAndPresent and/or
  csWaitForVBLANK of DisplayWindowParameters structure.

  \param hr     Result of the operation. Initial value must be S_OK, otherwise function will fail.
  \param ptr Pointer to window data structure.
*/
inline
void
GoFullscreen_inline(
                    HRESULT & hr,
                    DisplayWindowParameters * const ptr
                    )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(NULL != ptr->pOutput);
  if (NULL == ptr->pOutput) return;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return;

  /* We are switching to the fullscreen mode.
     First try to find a closest match to the preferred display mode as the preferred
     mode may not be supported. Then activate the closest match and toggle fullscreen state.
  */

  DXGI_MODE_DESC sFullScreenMode = ptr->sFullScreenMode;

  ptr->fFullscreen = true;

  if ( SUCCEEDED(hr) )
    {
      hr = FindBestMatchingModeForDXGIOutput(ptr->pOutput, &(ptr->sFullScreenMode), &(sFullScreenMode), ptr->pDevice);
      assert( SUCCEEDED(hr) );
      if ( SUCCEEDED(hr) )
        {
          ptr->sCurrentMode = sFullScreenMode;

          double const frequency_requested =
            (double)(ptr->sFullScreenMode.RefreshRate.Numerator) /
            (double)(ptr->sFullScreenMode.RefreshRate.Denominator);
          int const cnt1 = wprintf(
                                   gMsgFullscreenModeRequested,
                                   ptr->ProjectorID + 1,
                                   ptr->sFullScreenMode.Width,
                                   ptr->sFullScreenMode.Height,
                                   frequency_requested
                                   );
          assert(0 < cnt1);

          double const frequency_achieved =
            (double)(sFullScreenMode.RefreshRate.Numerator) /
            (double)(sFullScreenMode.RefreshRate.Denominator);
          int const cnt2 = wprintf(
                                   gMsgFullscreenModeAchieved,
                                   ptr->ProjectorID + 1,
                                   sFullScreenMode.Width,
                                   sFullScreenMode.Height,
                                   frequency_achieved
                                   );
          assert(0 < cnt2);
        }
      /* if */
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = ptr->pSwapChain->ResizeTarget(&sFullScreenMode);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = ptr->pSwapChain->SetFullscreenState(TRUE, ptr->pOutput);
      //assert( SUCCEEDED(hr) );
      ptr->fFullscreen = SUCCEEDED(hr);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      /* Note: MSDN articles about DXGI suggest that ResizeTarget should be called before
         SetFullscreenState to avoid unneeded WM_SIZE messages. Additionally, to avoid
         problems with refresh rates another call to ResizeTarget after SetFullscreenState
         with RefreshRate field of DXGI_MODE_DESC set to 0/0 is recommended.

         For test setup using DALSA Genie c1601 and Mitshubishi EW230U-ST on analog VGA
         seting RefreshRate to 0/0 usually reduces refresh rate to either 50 or 60 Hz.
         A same effect may be expected as Windows will tend to prefere the refresh rate
         of the primary monitor.
      */
      //sFullScreenMode.RefreshRate.Numerator = 0;
      //sFullScreenMode.RefreshRate.Denominator = 0;
      hr = ptr->pSwapChain->ResizeTarget(&sFullScreenMode);
      assert( SUCCEEDED(hr) );
    }
  /* if */
}
/* GoFullscreen_inline */



//! Makes swap chain windowed.
/*!
  Forces swap chain to window mode.

  Note that this function does not contain critical section block so it may crash DXGI.
  It shall be called only if enclosed inside the critical section blocks csRenderAndPresent and/or
  csWaitForVBLANK of DisplayWindowParameters structure.

  \param hr     Result of the operation. Initial value must be S_OK, otherwise function will fail.
  \param ptr Pointer to window data structure.
*/
inline
void
GoWindow_inline(
                HRESULT & hr,
                DisplayWindowParameters * const ptr
                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(NULL != ptr->pOutput);
  if (NULL == ptr->pOutput) return;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return;

  /* We are switching to the windowed mode.
     First try to find a closest match to the preferred display mode as the preferred
     mode may not be supported. Then activate the closest match and toggle fullscreen state.
  */

  DXGI_MODE_DESC sWindowMode = ptr->sWindowMode;

  ptr->fFullscreen = false;

  if ( SUCCEEDED(hr) )
    {
      hr = FindBestMatchingModeForDXGIOutput(ptr->pOutput, &(ptr->sWindowMode), &(sWindowMode), ptr->pDevice);
      assert( SUCCEEDED(hr) );
      if ( SUCCEEDED(hr) )
        {
          ptr->sCurrentMode = sWindowMode;

          double const frequency =
            (double)(sWindowMode.RefreshRate.Numerator) /
            (double)(sWindowMode.RefreshRate.Denominator);
          int const cnt = wprintf(
                                  gMsgWindowedModeAchieved,
                                  ptr->ProjectorID + 1,
                                  sWindowMode.Width,
                                  sWindowMode.Height,
                                  frequency
                                  );
          assert(0 < cnt);
        }
      /* if */
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = ptr->pSwapChain->SetFullscreenState(FALSE, NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = ptr->pSwapChain->ResizeTarget(&sWindowMode);
      assert( SUCCEEDED(hr) );
      ptr->fFullscreen = !SUCCEEDED(hr);
    }
  /* if */
}
/* GoWindow_inline */

#pragma endregion // DXGI swap chain inline helpers


#pragma region // Delay adjustment and exposure to refresh rate matching

//! Adjust trigger delays.
/*!
  Function adjusts scheduled trigger delays for rendering and acquisition thread.

  \param hr     Result of the operation (reference).
  Initial value must be S_OK, otherwise the function will fail.
  \param ptr    Pointer to window data structure.
  \param exposureDuration_us    Camera exposure or frame duration, in microseconds.
  \param k   Exposure multiplier; default value is 1.0.
*/
inline
void
AdjustTriggerDelays_inline(
                           HRESULT & hr,
                           DisplayWindowParameters * const ptr,
                           double const exposureDuration_us,
                           double const k = 1.0
                           )
{
  // Check input values.
  assert( 0.0 <= exposureDuration_us );
  if ( !(0.0 <= exposureDuration_us) ) hr = E_UNEXPECTED;
  if ( !SUCCEEDED(hr) ) return;

  assert( 0.0 <= k );
  if ( !(0.0 <= k) ) hr = E_UNEXPECTED;
  if ( !SUCCEEDED(hr) ) return;

  // Get duration of one frame.
  double const frameDuration_us = FrameDurationFromRefreshRate( ptr ); // us (microseconds)
  assert( !isnanorinf_inline(frameDuration_us) );
  if ( isnanorinf_inline(frameDuration_us) ) hr = E_UNEXPECTED;
  if ( !SUCCEEDED(hr) ) return;

  AcquireSRWLockExclusive( &(ptr->sLockRT) );
  {
    // Set exposure time.
    long int const exposureTime_whole = (long int)(exposureDuration_us * ptr->us_to_vblanks + 0.5);
    __int64 const QPC_exposureTime = (__int64)(exposureDuration_us * ptr->us_to_ticks + 0.5);

    ptr->exposureTime_whole = exposureTime_whole;
    ptr->QPC_exposureTime = QPC_exposureTime;

    // Get requested delay and present time.
    __int64 const QPC_delayTime_whole = ptr->QPC_delayTime;
    __int64 const QPC_delayTime = ptr->QPC_delayTime;
    __int64 const QPC_presentTime = ptr->QPC_presentTime;

    assert( 0 <= QPC_delayTime_whole );
    assert( 0 <= QPC_delayTime );
    assert( 0 <= QPC_presentTime );
    assert( QPC_delayTime_whole <= QPC_delayTime );
    assert( QPC_presentTime >= QPC_exposureTime );

    // Set scheduled delay times.
    __int64 QPC_delayDelta = (QPC_presentTime - QPC_exposureTime) / 2;
    if (0 > QPC_delayDelta) QPC_delayDelta = 0;

    ptr->QPC_delay_for_trigger_scheduled_RT = ptr->QPC_delayTime_whole;
    ptr->QPC_delay_for_trigger_scheduled_AT = ptr->QPC_delayTime;

    ptr->QPC_delayDelta = QPC_delayDelta;

    assert(ptr->QPC_delay_for_trigger_scheduled_RT <= ptr->QPC_delay_for_trigger_scheduled_AT);
  }
  ReleaseSRWLockExclusive( &(ptr->sLockRT) );
}
/* AdjustTriggerDelays_inline */



//! Updates delay time.
/*!
  Computes new decomposition of set delay time into whole part measured in VBLANK intervals and fractional
  part measured in us.

  \param hr     Result of the operation. Initial value must be S_OK, otherwise function will fail.
  \param ptr Pointer to window data structure.
*/
inline
void
AdjustPresentAndDelayTimes_inline(
                                  HRESULT & hr,
                                  DisplayWindowParameters * const ptr
                                  )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  // Fetch and check user defined parameters.
  double const delayTime_ms = ptr->delayTime_ms;
  long int const presentTime = ptr->presentTime;

  assert( 0 <= delayTime_ms );
  if ( !(0 <= delayTime_ms) ) hr = E_UNEXPECTED;

  assert( 0 <= presentTime );
  if (0 > presentTime) hr = E_UNEXPECTED;

  if ( !SUCCEEDED(hr) ) return;

  double const frameDuration_us = FrameDurationFromRefreshRate(ptr); // us (microseconds)
  assert( !isnanorinf_inline(frameDuration_us) );

  if ( isnanorinf_inline(frameDuration_us) ) hr = E_UNEXPECTED;

  if ( !SUCCEEDED(hr) ) return;

  // Invert frame duration.
  double const frameDuration_us_inv = 1.0 / frameDuration_us;

  // Compute conversion factors.
  double const us_to_vblanks = frameDuration_us_inv;
  double const vblanks_to_us = frameDuration_us;
  double const ticks_to_vblanks = ptr->ticks_to_us * frameDuration_us_inv;
  double const vblanks_to_ticks = ptr->us_to_ticks * frameDuration_us;

  // Convert requested present time from VBLANKs.
  double const presentTime_us = presentTime * vblanks_to_us;
  __int64 const QPC_presentTime = (__int64)( presentTime * vblanks_to_ticks + 0.5 );

  // Convert screen refresh time to QPCs.
  double const refreshTime_ms = frameDuration_us * 0.001;
  __int64 const QPC_refreshTime = (__int64)( frameDuration_us * ptr->us_to_ticks + 0.5 );

  // Convert requested present-to-trigger delay from ms.
  double const delayTime_us = delayTime_ms * 1000; // us (microseconds)
  assert(0 <= delayTime_us);

  double const delayTime_whole = (int)( delayTime_us * frameDuration_us_inv );
  double const delayTime_fraction_us = delayTime_us - (double)( delayTime_whole ) * frameDuration_us;

  double const QPC_delayTime_whole = (__int64)( delayTime_whole * frameDuration_us + 0.5 );
  double const QPC_delayTime = (__int64)( delayTime_us * ptr->us_to_ticks + 0.5 );

  // Update data in storage.
  AcquireSRWLockExclusive( &(ptr->sLockRT) );
  {
    ptr->us_to_vblanks = frameDuration_us_inv;
    ptr->vblanks_to_us = frameDuration_us;
    ptr->ticks_to_vblanks = ptr->ticks_to_us * frameDuration_us_inv;
    ptr->vblanks_to_ticks = ptr->us_to_ticks * frameDuration_us;

    assert(presentTime == ptr->presentTime);
    ptr->presentTime_us = presentTime_us;
    ptr->refreshTime_ms = refreshTime_ms;
    ptr->QPC_presentTime = QPC_presentTime;
    ptr->QPC_refreshTime = QPC_refreshTime;

    assert(delayTime_ms == ptr->delayTime_ms);
    ptr->delayTime_us = delayTime_us;
    ptr->delayTime_fraction_us = delayTime_fraction_us;
    ptr->delayTime_whole = delayTime_whole;
    ptr->QPC_delayTime_whole = QPC_delayTime_whole;
    ptr->QPC_delayTime = QPC_delayTime;
  }
  ReleaseSRWLockExclusive( &(ptr->sLockRT) );

  // Adjust scheduled trigger delays (assume exposure time is equal to present time).
  AdjustTriggerDelays_inline(hr, ptr, frameDuration_us);
  assert( SUCCEEDED(hr) );

}
/* AdjustPresentAndDelayTimes_inline */

#pragma endregion // Delay adjustment and exposure to refresh rate matching



/****** DXGI AND DIRECT 2D/3D  ******/

#pragma region // Create and destroy DXGI swap chain

//! Releases swap chain and Direct 3D device.
/*!
  Releases allocated resources.

  \param ptr    Pointer to display window data.
*/
void
DeleteDirectXDeviceAndSwapChain(
                                DisplayWindowParameters * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->fModeChange = true; // Flag may be reset only if the swap chain is recreated.

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    SAFE_RELEASE(ptr->pTextFormat);
    SAFE_RELEASE(ptr->pYellowBrush);
    SAFE_RELEASE(ptr->pBlackBrush);
    SAFE_RELEASE(ptr->pRenderTarget);
    SAFE_RELEASE(ptr->pBackBuffer);

    if (NULL != ptr->pSwapChain)
      {
        HRESULT const hr = ptr->pSwapChain->SetFullscreenState(FALSE, NULL);
        assert( SUCCEEDED(hr) );

        ptr->pSwapChain->Release();
      }
    /* if */
    ptr->pSwapChain = NULL;

    EnterCriticalSection( &(ptr->csWaitForVBLANK) );
    {
      SAFE_RELEASE(ptr->pDeviceContext);
      SAFE_RELEASE(ptr->pDevice);
      SAFE_RELEASE(ptr->pOutput);
      SAFE_RELEASE(ptr->pAdapter);
    }
    LeaveCriticalSection( &(ptr->csWaitForVBLANK) );
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );
}
/* DeleteDirectXDeviceAndSwapChain */



//! Recreate Direct2D render target.
/*!
  Function recreates Direct2D render target.

  \param ptr    Pointer to display window structure.
  \return Function returns S_OK if successfull and error code otherwise.
*/
HRESULT
RecreateDirect2DRenderTarget(
                             DisplayWindowParameters * const ptr
                             )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pD2DFactory);
  if (NULL == ptr->pD2DFactory) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_INVALIDARG;

  HRESULT hr = S_OK;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    /* Release old context and render target. */
    SAFE_RELEASE( ptr->pYellowBrush );
    SAFE_RELEASE( ptr->pBlackBrush );
    SAFE_RELEASE( ptr->pRenderTarget );
    SAFE_RELEASE( ptr->pBackBuffer );

    /* Create new context and render target. */
    assert(NULL == ptr->pBackBuffer);
    assert(NULL == ptr->pRenderTarget);
    assert(NULL == ptr->pBlackBrush);
    assert(NULL == ptr->pYellowBrush);
    if ( SUCCEEDED(hr) )
      {
        hr = RenderTargetCreate(
                                ptr->pD2DFactory,
                                ptr->pSwapChain,
                                &( ptr->pBackBuffer ),
                                &( ptr->pRenderTarget ),
                                &( ptr->pBlackBrush ),
                                &( ptr->pYellowBrush )
                                );
        assert( SUCCEEDED(hr) );
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  return hr;
}
/* RecreateDirect2DRenderTarget */



//! Recreate Direct 3D device and swap chain.
/*!
  Function checks if swap chain exists and if output device changed. If any
  of those conditions are met swap chain is recreated.

  \param ptr    Pointer to display window structure.
  \return Function returns S_OK if successfull and error code otherwise.
*/
HRESULT
RecreateDirectXDeviceAndSwapChain(
                                  DisplayWindowParameters * const ptr
                                  )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pDXGIFactory1);
  if (NULL == ptr->pDXGIFactory1) return E_INVALIDARG;

  assert(NULL != ptr->pD2DFactory);
  if (NULL == ptr->pD2DFactory) return E_INVALIDARG;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;
  ptr->fRecreated = false;

  BOOL enteredRenderAndPresent = TryEnterCriticalSection( &(ptr->csRenderAndPresent) );
  if (FALSE == enteredRenderAndPresent)
    {
      if (true == ptr->fRenderAndPresent)
        {
          //Debugfwprintf(stderr, gDbgSwapChainRecreationSkipped, ptr->ProjectorID + 1);
          ptr->fModeChange = fModeChange;
          assert(false == ptr->fRecreated);
          return S_OK;
        }
      /* if */
      EnterCriticalSection( &(ptr->csRenderAndPresent) );
      enteredRenderAndPresent = TRUE;
    }
  /* if */

  /* Check if containing output changed. */
  HMONITOR const hOld = ptr->hSwapChainMonitor;
  HMONITOR const hCurrent = SwapChainGetMonitorHandle(ptr->pSwapChain);

#ifdef DEBUG
  if (NULL != hCurrent)
    {
      HMONITOR const hWindow = MonitorFromWindow(ptr->hWnd, MONITOR_DEFAULTTOPRIMARY);
      assert(hWindow == hCurrent);
    }
  /* if */
#endif /* DEBUG */

  bool const recreate_swap_chain = (hOld != hCurrent) || (NULL == hOld) || (NULL == ptr->pSwapChain);
  if (false == recreate_swap_chain)
    {
      if (FALSE != enteredRenderAndPresent) LeaveCriticalSection( &(ptr->csRenderAndPresent) );
      ptr->fModeChange = fModeChange;
      ptr->fRecreated = true;
      return S_OK;
    }
  /* if */

  BOOL enteredVBLANK = TryEnterCriticalSection( &(ptr->csWaitForVBLANK) );
  if (FALSE == enteredVBLANK)
    {
      if (true == ptr->fWaitForVBLANK)
        {
          if (FALSE != enteredRenderAndPresent) LeaveCriticalSection( &(ptr->csRenderAndPresent) );
          assert(true == recreate_swap_chain);
          Debugfwprintf(stderr, gDbgSwapChainRecreationSkipped, ptr->ProjectorID + 1);
          ptr->fModeChange = fModeChange;
          assert(false == ptr->fRecreated);
          return S_OK;
        }
      /* if */
      EnterCriticalSection( &(ptr->csWaitForVBLANK) );
      enteredVBLANK = TRUE;
    }
  /* if */

  /* If output device changed then recreate the swap chain. */
  if ( true == recreate_swap_chain )
    {
      assert(FALSE != enteredRenderAndPresent);
      assert(FALSE != enteredVBLANK);

      /* First delete previous swap chain. */
      DeleteDirectXDeviceAndSwapChain(ptr);

      /* Then create new one. */
      assert(NULL == ptr->pAdapter);
      assert(NULL == ptr->pOutput);
      assert(NULL == ptr->pDevice);
      assert(NULL == ptr->pDeviceContext);
      assert(NULL == ptr->pSwapChain);
      hr = SwapChainCreate(
                           ptr->hWnd,
                           ptr->pDXGIFactory1,
                           &( ptr->sFullScreenMode ),
                           &( ptr->pAdapter ),
                           &( ptr->pOutput ),
                           &( ptr->pDevice ),
                           &( ptr->pDeviceContext ),
                           &( ptr->pSwapChain )
                           );
      assert( SUCCEEDED(hr) );

      // Dissassociate ALT+ENTER and DXGI for the window.
      // We want to respond to full-screen request ourself and set the mode to calibrated projector resolution.
      if ( SUCCEEDED(hr) )
        {
          hr = ptr->pDXGIFactory1->MakeWindowAssociation(ptr->hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_PRINT_SCREEN);
          assert( SUCCEEDED(hr) );
        }
      /* if */

      /* Update stored swap chain data. */
      if ( SUCCEEDED(hr) )
        {
          ptr->hSwapChainMonitor = hCurrent;

          hr = SwapChainGetRefreshRate(ptr->pSwapChain, &(ptr->sRefreshRate));
          assert( SUCCEEDED(hr) );

          AdjustPresentAndDelayTimes_inline(hr, ptr);

          hr = ptr->pSwapChain->GetDesc( &(ptr->sSwapChainDesc) );
          assert( SUCCEEDED(hr) );

          UpdateCurrentDisplayMode_inline( ptr );
        }
      /* if */

      /* Create new context and render target. */
      if ( SUCCEEDED(hr) )
        {
          hr = RecreateDirect2DRenderTarget(ptr);
          assert( SUCCEEDED(hr) );
        }
      /* if */
    }
  /* if */

  assert(FALSE != enteredRenderAndPresent);
  assert(FALSE != enteredVBLANK);

  if (FALSE != enteredVBLANK) LeaveCriticalSection( &(ptr->csWaitForVBLANK) );
  if (FALSE != enteredRenderAndPresent) LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  if ( SUCCEEDED(hr) )
    {
      ptr->fRecreated = true;
      ptr->fResized = true;

      Debugfwprintf(stderr, gDbgSwapChainRecreated, ptr->ProjectorID + 1);
    }
  else
    {
      assert(false == ptr->fRecreated);
    }
  /* if */

  ptr->fModeChange = fModeChange;

  return hr;
}
/* RecreateDirectXDeviceAndSwapChain */



//! Creates Direct 3D device and swap chain.
/*!
  Function creates Direct 3D device and swap chain and associates it with
  the display window. Current code is for Windows 7 and above and DirectX 10 and above.

  \param ptr    Pointer to display window data.
  \param pDXGIFactory1   Pointer to DXGI factory.
  \param pD2DFactory   Pointer to Direct 2D factory.
  \return Returns S_OK if successfull.
*/
HRESULT
CreateDirectXDeviceAndSwapChain(
                                DisplayWindowParameters * const ptr,
                                IDXGIFactory1 * const pDXGIFactory1,
                                ID2D1Factory * const pD2DFactory
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != pDXGIFactory1);
  if (NULL == pDXGIFactory1) return E_INVALIDARG;

  assert(NULL != pD2DFactory);
  if (NULL == pD2DFactory) return E_INVALIDARG;

  HRESULT hr = S_OK;

  // Copy factory pointers.
  ptr->pDXGIFactory1 = pDXGIFactory1;
  ptr->pD2DFactory = pD2DFactory;

  assert(true == ptr->fModeChange);

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    EnterCriticalSection( &(ptr->csWaitForVBLANK) );
    {
      // Set preferred fullscreen mode.
      {
        DXGI_MODE_DESC displayMode;
        ZeroMemory( &displayMode, sizeof(displayMode) );
        displayMode.Width = 1280;
        displayMode.Height = 800;
        displayMode.RefreshRate.Numerator = 120;
        displayMode.RefreshRate.Denominator = 1;
        displayMode.Format = DEFAULT_DIRECT_X_PIXEL_FORMAT; // Defined in BatchAcquisition.h
        displayMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        displayMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        ptr->sFullScreenMode = displayMode;
      }

      // Set preferred windowed mode.
      {
        DXGI_MODE_DESC displayMode;
        ZeroMemory( &displayMode, sizeof(displayMode) );
        displayMode.Width = 1024;
        displayMode.Height = 768;
        displayMode.RefreshRate.Numerator = 0;
        displayMode.RefreshRate.Denominator = 0;
        displayMode.Format = DEFAULT_DIRECT_X_PIXEL_FORMAT; // Defined in BatchAcquisition.h
        displayMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        displayMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        ptr->sWindowMode = displayMode;
      }

      // Create swap chain.
      if ( SUCCEEDED(hr) )
        {
          hr = RecreateDirectXDeviceAndSwapChain(ptr);
          assert( SUCCEEDED(hr) );
        }
      /* if */
    }
    LeaveCriticalSection( &(ptr->csWaitForVBLANK) );
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  // Set window data for callback routine.
  SetWindowData(ptr, ptr->hWnd);

  ptr->fModeChange = false;

  return hr;
}
/* CreateDirectXDeviceAndSwapChain */

#pragma endregion // Create and destroy DXGI swap chain


#pragma region // Resize DXGI swap chain

//! Resizes swap chain.
/*!
  Function resizes swap chain.

  \param ptr    Pointer to window data structure.
  \param width_in  New width of the swap chain.
  \param height_in New height of the swap chain.
  \return Returns S_OK if successfull.
*/
HRESULT
ResizeSwapChain(
                DisplayWindowParameters * const ptr,
                UINT const width_in,
                UINT const height_in
                )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  // Store new size and set resize status to false.
  {
    ptr->width = width_in;
    ptr->height = height_in;
    ptr->fResized = false;
  }

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;

  BOOL enteredRenderAndPresent = TryEnterCriticalSection( &(ptr->csRenderAndPresent) );
  if (FALSE == enteredRenderAndPresent)
    {
      if (true == ptr->fRenderAndPresent)
        {
          //Debugfwprintf(stderr, gDbgSwapChainResizeDeferred, ptr->ProjectorID + 1, width_in, height_in);
          ptr->fModeChange = fModeChange;
          assert(false == ptr->fResized);
          return E_FAIL;
        }
      /* if */
      EnterCriticalSection( &(ptr->csRenderAndPresent) );
      enteredRenderAndPresent = TRUE;
    }
  /* if */

  UINT BufferCount = ptr->sSwapChainDesc.BufferCount;
  UINT width = width_in;
  UINT height = height_in;
  DXGI_FORMAT format = ptr->sSwapChainDesc.BufferDesc.Format;
  UINT Flags = ptr->sSwapChainDesc.Flags;

  if ( SUCCEEDED(hr) )
    {
      assert(FALSE != enteredRenderAndPresent);

      /* DXGI swap chain cannot be resized if its back buffer is referenced. We therefore have
         to release Direct2D render target and associated buffers.
      */
      SAFE_RELEASE(ptr->pYellowBrush);
      SAFE_RELEASE(ptr->pBlackBrush);
      SAFE_RELEASE(ptr->pRenderTarget);
      SAFE_RELEASE(ptr->pBackBuffer);

      hr = ptr->pSwapChain->ResizeBuffers(BufferCount, width, height, format, Flags);
      assert( SUCCEEDED(hr) != (hr == DXGI_ERROR_DEVICE_REMOVED) );

      if ( SUCCEEDED(hr) )
        {
          hr = RecreateDirect2DRenderTarget(ptr);
          assert( SUCCEEDED(hr) );
        }
      else if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
          BOOL enteredVBLANK = TryEnterCriticalSection( &(ptr->csWaitForVBLANK) );
          if (FALSE == enteredVBLANK)
            {
              if (true == ptr->fWaitForVBLANK)
                {
                  if (FALSE != enteredRenderAndPresent) LeaveCriticalSection( &(ptr->csRenderAndPresent) );
                  //Debugfwprintf(stderr, gDbgSwapChainResizeDeferred, ptr->ProjectorID + 1, width_in, height_in);
                  ptr->fModeChange = fModeChange;
                  assert(false == ptr->fResized);
                  return E_FAIL;
                }
              /* if */
              EnterCriticalSection( &(ptr->csWaitForVBLANK) );
              enteredVBLANK = TRUE;
            }
          /* if */

          assert(FALSE != enteredVBLANK);

          hr = RecreateDirectXDeviceAndSwapChain(ptr);
          assert( SUCCEEDED(hr) );

          if (FALSE != enteredVBLANK) LeaveCriticalSection( &(ptr->csWaitForVBLANK) );
        }
      /* if */

      assert(false == ptr->fResized);
      ptr->fResized = SUCCEEDED(hr);
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = SwapChainGetRefreshRate(ptr->pSwapChain, &(ptr->sRefreshRate));
      assert( SUCCEEDED(hr) );
    }
  /* if */

  AdjustPresentAndDelayTimes_inline(hr, ptr);

  if (FALSE != enteredRenderAndPresent) LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  if ( SUCCEEDED(hr) )
    {
      assert(true == ptr->fResized);

      //Debugfwprintf(stderr, gDbgSwapChainResized, ptr->ProjectorID + 1, ptr->width, ptr->height);
    }
  else
    {
      assert(false == ptr->fResized);
    }
  /* if */

  ptr->fModeChange = fModeChange;

  return hr;
}
/* ResizeSwapChain */

#pragma endregion // Resize DXGI swap chain


#pragma region // Fullscreen support functions

//! Update fullscreen state.
/*!
  Swap chain may drop out of fullscreen if another window is dragged to its screen.
  This function checks actual fullscreen state and corrects internal flag as needed.

  \param ptr    Pointer to window data structure.
  \return Returns S_OK if successfull.
*/
HRESULT
UpdateSwapChainFullscreenStatus(
                                DisplayWindowParameters * const ptr
                                )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  HRESULT hr = S_OK;

  BOOL const ready = TryEnterCriticalSection( &(ptr->csRenderAndPresent) );
  if (FALSE == ready) return hr;

  BOOL fullscreen = FALSE;
  IDXGIOutput * pOutput = NULL;

  hr = ptr->pSwapChain->GetFullscreenState(&fullscreen, &pOutput);
  assert( SUCCEEDED(hr) );
  if ( SUCCEEDED(hr) ) ptr->fFullscreen = (TRUE == fullscreen);

  SAFE_RELEASE(pOutput);

  if (FALSE != ready) LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  return hr;
}
/* UpdateSwapChainFullscreenStatus */



//! Changes resolution in fullscreen mode.
/*!
  Changes resolution in fullscreen mode.

  \param ptr    Pointer to window data structure.
  \return Returns S_OK if successfull.
*/
HRESULT
ChangeFullScreenResolution(
                           DisplayWindowParameters * const ptr
                           )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    EnterCriticalSection( &(ptr->csWaitForVBLANK) );
    {
      if ( SUCCEEDED(hr) )
        {
          hr = UpdateSwapChainFullscreenStatus( ptr );
          assert( SUCCEEDED(hr) );
        }
      /* if */

      /* Apply changes only if we are in fullscreen mode. */
      if (true == ptr->fFullscreen)
        {
          if ( SUCCEEDED(hr) )
            {
              /* Uncomment to force swap chain to always recreate iteself. */
              //ptr->hSwapChainMonitor = NULL;

              hr = RecreateDirectXDeviceAndSwapChain( ptr );
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if ( SUCCEEDED(hr) )
            {
              hr = UpdateSwapChainFullscreenStatus( ptr );
              assert( SUCCEEDED(hr) );
            }
          /* if */

          if ( SUCCEEDED(hr) )
            {
              if (true == ptr->fFullscreen)
                {
                  hr = ptr->pSwapChain->ResizeTarget( &(ptr->sFullScreenMode) );
                  assert( SUCCEEDED(hr) );

                  if ( SUCCEEDED(hr) )
                    {
                      UINT const width = ptr->sFullScreenMode.Width;
                      UINT const height = ptr->sFullScreenMode.Height;
                      hr = ResizeSwapChain(ptr, width, height);
                      //assert( SUCCEEDED(hr) );
                    }
                  /* if */
                }
              else
                {
                  GoFullscreen_inline(hr, ptr);
                  assert( SUCCEEDED(hr) );
                }
              /* if */
            }
          /* if */
        }
      /* if */
    }
    LeaveCriticalSection( &(ptr->csWaitForVBLANK) );
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* ChangeFullScreenResolution */

#pragma endregion // Fullscreen support functions


#pragma region // Windowed to fullscreen transitions

//! Toggle full screen.
/*!
  Sets or reset the full-screen mode.

  \param ptr    Pointer to window data structure.
  \return Returns S_OK if successfull.
*/
HRESULT
ToggleFullScreen(
                 DisplayWindowParameters * const ptr
                 )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  assert(NULL != ptr->pOutput);
  if (NULL == ptr->pOutput) return E_POINTER;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    hr = RecreateDirectXDeviceAndSwapChain( ptr );
    assert( SUCCEEDED(hr) );

    if ( SUCCEEDED(hr) )
      {
        hr = UpdateSwapChainFullscreenStatus( ptr );
        assert( SUCCEEDED(hr) );
      }
    /* if */

    if ( SUCCEEDED(hr) && (false == ptr->fFullscreen) )
      {
        GoFullscreen_inline(hr, ptr);
        assert( SUCCEEDED(hr) );
      }
    else if ( SUCCEEDED(hr) && (true == ptr->fFullscreen) )
      {
        GoWindow_inline(hr, ptr);
        assert( SUCCEEDED(hr) );
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        hr = SwapChainGetRefreshRate(ptr->pSwapChain, &(ptr->sRefreshRate));
        assert( SUCCEEDED(hr) );
      }
    /* if */

    AdjustPresentAndDelayTimes_inline(hr, ptr);
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* ToggleFullScreen */



//! Makes swap chain fullscreen.
/*!
  Makes swap chain fullscreen.

  \param ptr    Pointer to window data structure.
  \return Returns S_OK if successfull.
*/
HRESULT
GoFullscreen(
             DisplayWindowParameters * const ptr
             )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  assert(NULL != ptr->pOutput);
  if (NULL == ptr->pOutput) return E_POINTER;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    hr = RecreateDirectXDeviceAndSwapChain( ptr );
    assert( SUCCEEDED(hr) );

    if ( SUCCEEDED(hr) )
      {
        hr = UpdateSwapChainFullscreenStatus( ptr );
        assert( SUCCEEDED(hr) );
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        if (false == ptr->fFullscreen)
          {
            GoFullscreen_inline(hr, ptr);
            assert( SUCCEEDED(hr) );
          }
        else
          {
            Debugfwprintf(stderr, gDbgSwapChainIsFullscreen, ptr->ProjectorID + 1);
          }
        /* if */
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        hr = SwapChainGetRefreshRate(ptr->pSwapChain, &(ptr->sRefreshRate));
        assert( SUCCEEDED(hr) );
      }
    /* if */

    AdjustPresentAndDelayTimes_inline(hr, ptr);
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* GoFullscreen */



//! Makes swap chain windowed.
/*!
  Makes swap chain render to window.

  \param ptr    Pointer to window data structure.
  \return Returns S_OK if successfull.
*/
HRESULT
GoWindow(
         DisplayWindowParameters * const ptr
         )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_POINTER;

  assert(NULL != ptr->pOutput);
  if (NULL == ptr->pOutput) return E_POINTER;

  HRESULT hr = S_OK;

  bool const fModeChange = ptr->fModeChange;
  ptr->fModeChange = true;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    hr = RecreateDirectXDeviceAndSwapChain( ptr );
    assert( SUCCEEDED(hr) );

    if ( SUCCEEDED(hr) )
      {
        hr = UpdateSwapChainFullscreenStatus( ptr );
        assert( SUCCEEDED(hr) );
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        if (true == ptr->fFullscreen)
          {
            GoWindow_inline(hr, ptr);
            assert( SUCCEEDED(hr) );
          }
        else
          {
            Debugfwprintf(stderr, gDbgSwapChainIsWindowed, ptr->ProjectorID + 1);
          }
        /* if */
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        hr = SwapChainGetRefreshRate(ptr->pSwapChain, &(ptr->sRefreshRate));
        assert( SUCCEEDED(hr) );
      }
    /* if */

    AdjustPresentAndDelayTimes_inline(hr, ptr);
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* GoWindow */

#pragma endregion // Windowed to fullscreen transitions


#pragma region // Rendering

//! Render image.
/*!
  Renders image to DXGI swap chain. Image is not presented.

  \param pWindow        Pointer to display window parameters.
  \param pImage   Pointer to window data.
  \return Returns S_OK if successfull, and error code otherwise.
*/
HRESULT
RenderQueuedImage(
                  DisplayWindowParameters * const pWindow,
                  QueuedDecoderImage * const pImage
                  )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_INVALIDARG;

  assert(NULL != pImage);
  if (NULL == pImage) return E_INVALIDARG;

  HRESULT hr = S_OK;

  switch (pImage->render_type)
    {
    case QI_BGRA_BITMAP:
      {
        hr = RenderBitmapFromIWICBitmap(pImage->pBitmap, pWindow->pRenderTarget, pWindow->pBlackBrush);
        assert( SUCCEEDED(hr) != (D2DERR_RECREATE_TARGET == hr) );

        if (D2DERR_RECREATE_TARGET == hr)
          {
            Debugfwprintf(stderr, gDbgRecreatingRenderTarget, pWindow->ProjectorID + 1);

            hr = RecreateDirect2DRenderTarget(pWindow);
            assert( SUCCEEDED(hr) );
            if ( SUCCEEDED(hr) )
              {
                hr = RenderBitmapFromIWICBitmap(pImage->pBitmap, pWindow->pRenderTarget, pWindow->pBlackBrush);
                assert( SUCCEEDED(hr) );
              }
            /* if */
          }
        /* if */
      }
      break;

    case QI_PATTERN_SOLID:
      {
        hr = RenderSolidPattern(pImage->red, pImage->green, pImage->blue, pImage->alpha, pWindow->pRenderTarget);
        assert( SUCCEEDED(hr) != (D2DERR_RECREATE_TARGET == hr) );

        if (D2DERR_RECREATE_TARGET == hr)
          {
            Debugfwprintf(stderr, gDbgRecreatingRenderTarget, pWindow->ProjectorID + 1);

            hr = RecreateDirect2DRenderTarget(pWindow);
            assert( SUCCEEDED(hr) );
            if ( SUCCEEDED(hr) )
              {
                hr = RenderSolidPattern(pImage->red, pImage->green, pImage->blue, pImage->alpha, pWindow->pRenderTarget);
                assert( SUCCEEDED(hr) );
              }
            /* if */
          }
        /* if */
      }
      break;

    case QI_UNKNOWN_TYPE:
    default:
      hr = E_INVALIDARG;
    }
  /* switch */

  return hr;
}
/* RenderQueuedImage */



//! Blank window.
/*!
  Renders black image over whole DXGI swap chain area.

  \param pWindow        Pointer to display window parameters.
  \return Returns S_OK if successfull, error code othrewise.
*/
HRESULT
RenderBlankImage(
                 DisplayWindowParameters * const pWindow
                 )
{
  assert(NULL != pWindow);
  if (NULL == pWindow) return E_INVALIDARG;

  HRESULT hr = S_OK;

  hr = BlankRenderTarget(pWindow->pRenderTarget, pWindow->pBlackBrush);
  assert( SUCCEEDED(hr) != (D2DERR_RECREATE_TARGET == hr) );

  if (D2DERR_RECREATE_TARGET == hr)
    {
      Debugfwprintf(stderr, gDbgRecreatingRenderTarget, pWindow->ProjectorID + 1);

      hr = RecreateDirect2DRenderTarget(pWindow);
      assert( SUCCEEDED(hr) );
      if ( SUCCEEDED(hr) )
        {
          hr = BlankRenderTarget(pWindow->pRenderTarget, pWindow->pBlackBrush);
          assert( SUCCEEDED(hr) );
        }
      /* if */
    }
  /* if */

  return hr;
}
/* RenderBlankImage */

#pragma endregion // Rendering


/****** WINDOW THREAD AND MESSAGE PUMP ******/

#pragma region // Window message handler

//! Message handler.
/*!
  Processes messages for display window.

  \param hWnd   A handle to the window procedure that received the message.
  \param message        Received message.
  \param wParam  Additional message information. The content of this parameter depends on the value of the Msg parameter.
  \param lParam Additional message information. The content of this parameter depends on the value of the Msg parameter.
  \return The return value is the result of the message processing and depends on the message (mostly 0 if successfull).
*/
LRESULT
CALLBACK
WndProcDisplay(
               HWND hWnd,
               UINT message,
               WPARAM wParam,
               LPARAM lParam
               )
{
  /* Get window data pointer from global storage. */
  DisplayWindowParameters * const ptr = (DisplayWindowParameters *)GetWindowData(hWnd);

#ifdef _DEBUG
  AddMessageToList_inline(hWnd, message, wParam, lParam);
#endif /* _DEBUG */

  /* Some actions on DXGI swap chain must be deferred. */
  if ( (NULL != ptr) && (false == ptr->fModeChange) )
    {
      if (false == ptr->fRecreated)
        {
          HRESULT const hr = RecreateDirectXDeviceAndSwapChain(ptr);
          //assert( SUCCEEDED(hr) );
        }
      else if ( (false == ptr->fResized) && (WM_SIZE != message) )
        {
          HRESULT const hr = ResizeSwapChain(ptr, ptr->width, ptr->height);
          //assert( SUCCEEDED(hr) );
        }
      /* if */
    }
  /* if */

  switch (message)
    {

    case WM_COMMAND:
      {
        /* Commandy may be mapped to key combinations. This is done through the accelerator table that
           is defined in DisplayWindowThread function. Key combination is mapped to a number that
           must be unique. To faciliate this all used nambers are defined as constants in
           header file BatchAcquisitionWindowDisplay.h.
        */
        int wmId    = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);
        switch (wmId)
          {
          case DISPLAY_WINDOW_EXIT:
            {
              BOOL const res = DestroyWindow(hWnd);
              if (TRUE == res) return 0;
            }
            break;

          case DISPLAY_WINDOW_FULLSCREEN:
            {
              HRESULT const hr = GoFullscreen(ptr);
              assert( SUCCEEDED(hr) );
              if ( SUCCEEDED(hr) )
                {
                  if (NULL != ptr) ptr->fFreeze = false;
                  return 0;
                }
              /* if */
            }
            break;

          case DISPLAY_WINDOW_WINDOW:
            {
              HRESULT const hr = GoWindow(ptr);
              assert( SUCCEEDED(hr) );
              if ( SUCCEEDED(hr) ) return 0;
            }
            break;

          case DISPLAY_WINDOW_CHANGE_RESOLUTION:
            {
              HRESULT const hr = ChangeFullScreenResolution(ptr);
              assert( SUCCEEDED(hr) );
              if ( SUCCEEDED(hr) ) return 0;
            }
            break;

          case DISPLAY_WINDOW_ALT_ENTER:
            {
              HRESULT const hr = ToggleFullScreen(ptr);
              if ( SUCCEEDED(hr) ) return 0;
            }
            break;

          case DISPLAY_WINDOW_UPDATE_TITLE:
            {
              UpdateCurrentWindowTitle_inline(ptr);
              return 0;
            }
            break;

          case DISPLAY_WINDOW_FREEZE:
            {
              if (NULL != ptr)
                {
                  ptr->fFreeze = !ptr->fFreeze;
                  if (true == ptr->fFreeze)
                    {
                      int const cnt = wprintf(gMsgPresentSuspended, ptr->ProjectorID + 1);
                      assert(0 < cnt);
                    }
                  else
                    {
                      int const cnt = wprintf(gMsgPresentResumed, ptr->ProjectorID + 1);
                      assert(0 < cnt);
                    }
                  /* if */
                }
              /* if */
              return 0;
            }
            break;

          default:
            return DefWindowProc(hWnd, message, wParam, lParam);
          }
        /* switch */
      }
      break;

    case WM_PAINT:
      {
        /* Rendering and presenting is normally done on a separate thread. Only in the case
           of fixed SL pattern when the frame is rendered and presented once we repeat the
           rendering process here.
        */
        if ( (NULL != ptr) && (true == ptr->fFixed) )
          {
            HRESULT hr = S_OK;
            QueuedDecoderImage * pImage = NULL;
            AcquireSRWLockExclusive( &(ptr->sLockImage) );
            {
              pImage = ptr->pImage;
              ptr->pImage = NULL;
            }
            ReleaseSRWLockExclusive( &(ptr->sLockImage) );

            if ( (NULL != pImage) && (NULL != ptr->pSwapChain) && (NULL != ptr->pRenderTarget) )
              {
                bool const fModeChange = ptr->fModeChange;

                ptr->fModeChange = true;
                EnterCriticalSection( &(ptr->csRenderAndPresent) );
                {
                  if ( SUCCEEDED(hr) )
                    {
                      hr = RenderQueuedImage(ptr, pImage);
                      //assert( SUCCEEDED(hr) );
                    }
                  /* if */

                  if ( SUCCEEDED(hr) && (NULL != ptr->pSwapChain) )
                    {
                      hr = ptr->pSwapChain->Present(0, 0);
                      assert( SUCCEEDED(hr) );
                    }
                  /* if */
                }
                LeaveCriticalSection( &(ptr->csRenderAndPresent) );
                ptr->fModeChange = fModeChange;

                AcquireSRWLockExclusive( &(ptr->sLockImage) );
                {
                  if (NULL == ptr->pImage)
                    {
                      ptr->pImage = pImage;
                      pImage = NULL;
                    }
                  /* if */
                }
                ReleaseSRWLockExclusive( &(ptr->sLockImage) );

                SAFE_DELETE(pImage);
              }
            /* if */
          }
        /* if */
        return DefWindowProc(hWnd, message, wParam, lParam);
      }

    case WM_GETMINMAXINFO:
      {
        /* In case of multiple monitors default message processing code may not work correctly
           as it uses primary monitor to limit application window sizes. If we are going fullscreen
           on any monitor other than primary then default values must be replaced by ones
           requested by the ToggleFullScreen function.
        */
        MINMAXINFO * const pMINMAXINFO = (MINMAXINFO *)lParam;
        if ( (NULL != ptr) && (true == ptr->fFullscreen) )
          {
            int const maxx = pMINMAXINFO->ptMaxSize.x;
            int const maxy = pMINMAXINFO->ptMaxSize.y;
            int const requestedx = ptr->sCurrentMode.Width + 16;
            int const requestedy = ptr->sCurrentMode.Height + 16;
            if (maxx < requestedx) pMINMAXINFO->ptMaxSize.x = requestedx;
            if (maxy < requestedy) pMINMAXINFO->ptMaxSize.y = requestedy;
            return 0;
          }
        else
          {
            return DefWindowProc(hWnd, message, wParam, lParam);
          }
        /* if */
      }
      break;

    case WM_SIZE:
      {
        /* Per MSDN article about DXGI states that when WM_SIZE is called
           then the swap chain buffers should be resized to match the window size.
           If resizing fails then default to DefWindowProc.
        */
        UINT const width = LOWORD(lParam);
        UINT const height = HIWORD(lParam);
        HRESULT const hr = ResizeSwapChain(ptr, width, height);
        if ( SUCCEEDED(hr) )
          {
            return 0;
          }
        else
          {
            return DefWindowProc(hWnd, message, wParam, lParam);
          }
        /* if */
      }
      break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      {
        /* ALT+Enter key combination is used to toggle fullscreen On/Off. It may
           be handled by DXGI or by the application. We chose to handle ALT+Enter ourself
           so we are able to precisely choose the target resolution and the refresh rate.
           For this to work MakeWindowAssociation function with option DXGI_MWA_NO_ALT_ENTER
           must be called when swap chain is (re)created
           (we do this in function RecreateDirectXDeviceAndSwapChain).
        */
        if ( (wParam == VK_RETURN) && (HIWORD(lParam) & KF_ALTDOWN) )
          {
            HRESULT const hr = ToggleFullScreen(ptr);
            if ( SUCCEEDED(hr) ) return 0;
          }
        else
          {
            return DefWindowProc(hWnd, message, wParam, lParam);
          }
        /* if */
      }
      break;

    case WM_DESTROY:
      {
        PostQuitMessage(0);
        return 0;
      }
      break;

    case WM_GETTITLEBARINFOEX:
      {
        assert(0 == wParam);
        TITLEBARINFOEX * const ptinfo = (TITLEBARINFOEX *)lParam;

        if (sizeof(TITLEBARINFO) <= ptinfo->cbSize)
          {
            ptinfo->rcTitleBar.left = 0;
            ptinfo->rcTitleBar.top = 0;
            ptinfo->rcTitleBar.right = 0;
            ptinfo->rcTitleBar.bottom = 0;
            bool const has_extended_data = sizeof(TITLEBARINFOEX) <= ptinfo->cbSize;
            for (int i = 0; i < CCHILDREN_TITLEBAR + 1; ++i)
              {
                ptinfo->rgstate[i] = STATE_SYSTEM_UNAVAILABLE | STATE_SYSTEM_OFFSCREEN;
                if (true == has_extended_data)
                  {
                    ptinfo->rgrect[i].left = 0;
                    ptinfo->rgrect[i].top = 0;
                    ptinfo->rgrect[i].right = 0;
                    ptinfo->rgrect[i].bottom = 0;
                  }
                /* if */
              }
            /* for */
          }
        /* if */

        return 0;
      }
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }
  /* switch */

  // Normal return value is zero when the message was processed so we return 1 if message was not processed.
  return 1;
}
/* WndProcDisplay */

#pragma endregion // Window message handler


#pragma region // Display window thread

//! Window thread.
/*!
  Creates empty display window and runs the message pump.

  \param parameters_in Pointer to a structure holding window data.
  \return Function returns EXIT_SUCCESS if successfull, and EXIT_FAILURE if unsuccessfull.
*/
unsigned int
__stdcall
DisplayWindowThread(
                    void * parameters_in
                    )
{
  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  DisplayWindowParameters * const parameters = (DisplayWindowParameters *)parameters_in;

  // Set thread name.
  char sThreadName[MAX_PATH + 1];
  {
    int i = 0;
    for (; i < MAX_PATH; ++i) sThreadName[i] = (char)(parameters->szTitle[i]);
    sThreadName[i] = 0;
    SetThreadNameForMSVC(-1, sThreadName);
  }

  // Register the window class.
  {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
    wcex.lpfnWndProc = WndProcDisplay;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = parameters->hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = parameters->szWindowClass;
    wcex.hIconSm = NULL;

    RegisterClassEx(&wcex);
  }

  // Initialize the display window.
  assert(NULL == parameters->hWnd);
  parameters->hWnd =
    CreateWindow(
                 parameters->szWindowClass,
                 parameters->szTitle,
                 WS_OVERLAPPEDWINDOW,
                 CW_USEDEFAULT,
                 0, // Ignored due to previous CW_USEDEFAULT
                 CW_USEDEFAULT,
                 0, // Ignored due to previous CW_USEDEFAULT
                 parameters->hWndParent,
                 NULL,
                 parameters->hInstance,
                 NULL
                 );
  assert(NULL != parameters->hWnd);
  if (NULL == parameters->hWnd) return EXIT_FAILURE;

  BOOL const sw = ShowWindow(parameters->hWnd, parameters->nCmdShow);
  assert(0 == sw);

  BOOL const uw = UpdateWindow(parameters->hWnd);
  assert(TRUE == uw);

  // Create keyboar shorcuts accelerator table.
  ACCEL AccelTable[2];
  AccelTable[0].fVirt = 0;
  AccelTable[0].key = 'f';
  AccelTable[0].cmd = DISPLAY_WINDOW_FREEZE;

  AccelTable[1].fVirt = 0;
  AccelTable[1].key = 'F';
  AccelTable[1].cmd = DISPLAY_WINDOW_FREEZE;

  HACCEL hAccelTable = CreateAcceleratorTable(AccelTable, 2);

  // Raise thread active flag.
  assert(false == parameters->fActive);
  parameters->fActive = true;

  // Main message loop.
  MSG msg;
  BOOL bRet;
  while ( bRet = GetMessage(&msg, NULL, 0, 0) )
    {
      if ( !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
        {
          BOOL const tm = TranslateMessage( &msg );
          LRESULT const dm = DispatchMessage( &msg );
        }
      /* if */
    }
  /* while */

  // Lower thread active flag.
  parameters->fActive = false;

  // Release accelerator table.
  BOOL const dat = DestroyAcceleratorTable(hAccelTable);
  assert(0 != dat);

  return EXIT_SUCCESS;
}
/* DisplayWindowThread */

#pragma endregion // Display window thread


/****** OPEN/CLOSE DISPLAY WINDOW ******/

#pragma region // Open and close display window

//! Opens display window and starts message pump.
/*!
  Opens empty display window and spawns new thread that runs the message pump.

  \see CloseDisplayWindow

  \param hInstance   A handle to the current instance of the application.
  \param ProjectorID    Projector ID.
  \param nCmdShow       Controls how the window is to be shown.
  \param hWndParent     Handle to parent window.
  \return Returns NULL if unsuccessfull and pointer to DisplayWindowParameters structure otherwise.
*/
DisplayWindowParameters *
OpenDisplayWindow(
                  HINSTANCE const hInstance,
                  int const ProjectorID,
                  int const nCmdShow,
                  HWND const hWndParent,
                  HWND const hWndCommand
                  )
{
  DisplayWindowParameters * const parameters = (DisplayWindowParameters *)malloc( sizeof(DisplayWindowParameters) );
  assert(NULL != parameters);
  if (NULL == parameters) return parameters;

  DisplayWindowParametersBlank_inline(parameters);

  /* Create critical section. */
  InitializeCriticalSection( &(parameters->csRenderAndPresent) );
  InitializeCriticalSection( &(parameters->csWaitForVBLANK) );

  InitializeSRWLock( &(parameters->sLockRT) );
  InitializeSRWLock( &(parameters->sLockImage) );

  /* Copy supplied data. */
  parameters->ProjectorID = ProjectorID;
  parameters->hInstance = hInstance;
  parameters->nCmdShow = nCmdShow;
  parameters->hWndParent = hWndParent;
  parameters->hWndCommand = hWndCommand;

  /* Name window and window class. */
  int const cnt1 = swprintf_s(parameters->szTitle, MAX_LOADSTRING, _T("[PRJ %d] %s"), ProjectorID + 1, gNameWindowDisplay);
  assert(0 < cnt1);
  parameters->szTitle[MAX_LOADSTRING] = 0;

  int const cnt2 = swprintf_s(parameters->szWindowClass, MAX_LOADSTRING, _T("PRJ_DXGI"));
  assert(0 < cnt2);
  parameters->szWindowClass[MAX_LOADSTRING] = 0;

  /* Get QPC information. */
  BOOL const res = QueryPerformanceFrequency( &(parameters->frequency) );
  assert(FALSE != res);
  if (FALSE == res) goto OPEN_DISPLAY_WINDOW_EXIT;

  double const frequency = (double)(parameters->frequency.QuadPart);
  double const frequency_inv = 1.0 / frequency;
  parameters->inv_frequency = frequency_inv;

  parameters->ticks_to_us = frequency_inv * 1000000.0;
  parameters->us_to_ticks = frequency * 0.000001;
  parameters->ticks_to_ms = frequency_inv * 1000.0;
  parameters->ms_to_ticks = frequency * 0.001;

  /* Create message storage. */
  assert(NULL == parameters->pMsg);
  parameters->pMsg = PastMessagesCreate();
  assert(NULL != parameters->pMsg);

  /* Spawn display window thread. */
  parameters->tWindow =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             DisplayWindowThread,
                             (void *)( parameters ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != parameters->tWindow );
  if ( (HANDLE)( NULL ) == parameters->tWindow )
    {
    OPEN_DISPLAY_WINDOW_EXIT:

      DisplayWindowParametersRelease_inline( parameters );
      return NULL;
    }
  /* if */

  return parameters;
}
/* OpenDisplayWindow */



//! Close display window.
/*!
  Closes display window. After calling this function parameters structure
  will be deallocated and must not be used.

  \see OpenDisplayWindow

  \param parameters     Pointer to parameters structure return by OpenDisplayWindow.
*/
void
CloseDisplayWindow(
                   DisplayWindowParameters * const parameters
                   )
{
  //assert(NULL != parameters);
  if (NULL == parameters) return;

  DWORD const result = WaitForSingleObject(parameters->tWindow, 0);

  if ( (WAIT_OBJECT_0 != result) && (true == parameters->fActive) )
    {
      // The thread is alive so send terminate message and wait for confirmation.
      DWORD_PTR dwResult;
      LRESULT const sm =
        SendMessageTimeout(
                           parameters->hWnd,
                           WM_COMMAND, // Message.
                           MAKEWPARAM(DISPLAY_WINDOW_EXIT, 0),
                           MAKELPARAM(0, 0),
                           SMTO_NOTIMEOUTIFNOTHUNG,
                           30000, // Timeout in ms.
                           &dwResult
                           );
      assert(0 != sm);
      if (0 != sm)
        {
          DWORD const confirm = WaitForSingleObject(parameters->tWindow, INFINITE);
          assert(WAIT_OBJECT_0 == confirm);
        }
      /* if */
    }
  else
    {
      // The thread has already terminated.
    }
  /* if */

  assert( WAIT_OBJECT_0 == WaitForSingleObject(parameters->tWindow, 0) );
  assert(false == parameters->fActive);

  DisplayWindowParametersRelease_inline( parameters );

}
/* CloseDisplayWindow */

#pragma endregion // Open and close display window



/****** AUXILIARY FUNCTIONS ******/

#pragma region // Windowed to fullscreen transitions

//! Changes fullscreen status of display window.
/*!
  Changes fullscreen status of display window.

  \param parameters     Pointer to parameters structure return by OpenDisplayWindow.
*/
void
ToggleFullscreenStatusOfDisplayWindow(
                                      DisplayWindowParameters * const parameters
                                      )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(true == parameters->fActive);
  if (false == parameters->fActive) return;

  HWND const hWndPrevious = GetForegroundWindow();

  // Send command message to display window.
  DWORD_PTR dwResult;
  LRESULT const sm =
    SendMessageTimeout(
                       parameters->hWnd,
                       WM_COMMAND, // Message.
                       MAKEWPARAM(DISPLAY_WINDOW_ALT_ENTER, 0),
                       MAKELPARAM(0, 0),
                       SMTO_NOTIMEOUTIFNOTHUNG,
                       1000, // Timeout in ms.
                       &dwResult
                       );
  assert(0 != sm);

  // Return focus to calling window.
  if (0 != sm)
    {
      if (NULL != hWndPrevious)
        {
          HMONITOR const hWindowPrevious = MonitorFromWindow(hWndPrevious, MONITOR_DEFAULTTOPRIMARY);
          HMONITOR const hWindowCurrent = MonitorFromWindow(parameters->hWnd, MONITOR_DEFAULTTOPRIMARY);

          if (hWindowPrevious != hWindowCurrent)
            {
              BOOL const activate = SetForegroundWindow(hWndPrevious);
              assert(TRUE == activate);

              HWND hWndActive = SetActiveWindow(hWndPrevious);
            }
          /* if */
        }
      /* if */
    }
  /* if */
}
/* ToggleFullscreenStatusOfDisplayWindow */



//! Changes fullscreen status of display window.
/*!
  Changes fullscreen status of display window.

  \param parameters     Pointer to parameters structure return by OpenDisplayWindow.
  \param fullscreen     Flag to indicate fullscreen status.
  Set to true to make window fullscreen and false to make window windowed.
*/
void
SetFullscreenStatusOfDisplayWindow(
                                   DisplayWindowParameters * const parameters,
                                   bool const fullscreen
                                   )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(true == parameters->fActive);
  if (false == parameters->fActive) return;

  HWND const hWndPrevious = GetForegroundWindow();

  // Send command message to display window.
  DWORD_PTR dwResult;
  LRESULT sm = 0;
  if (true == fullscreen)
    {
      sm = SendMessageTimeout(
                              parameters->hWnd,
                              WM_COMMAND, // Message.
                              MAKEWPARAM(DISPLAY_WINDOW_FULLSCREEN, 0),
                              MAKELPARAM(0, 0),
                              SMTO_NOTIMEOUTIFNOTHUNG,
                              2000, // Timeout in ms.
                              &dwResult
                              );
      //assert(0 != sm);
    }
  else
    {
      sm = SendMessageTimeout(
                              parameters->hWnd,
                              WM_COMMAND, // Message.
                              MAKEWPARAM(DISPLAY_WINDOW_WINDOW, 0),
                              MAKELPARAM(0, 0),
                              SMTO_NOTIMEOUTIFNOTHUNG,
                              2000, // Timeout in ms.
                              &dwResult
                              );
      //assert(0 != sm);
    }
  /* if */

  // Return focus to calling window.
  if (0 != sm)
    {
      if (NULL != hWndPrevious)
        {
          HMONITOR const hWindowPrevious = MonitorFromWindow(hWndPrevious, MONITOR_DEFAULTTOPRIMARY);
          HMONITOR const hWindowCurrent = MonitorFromWindow(parameters->hWnd, MONITOR_DEFAULTTOPRIMARY);

          if (hWindowPrevious != hWindowCurrent)
            {
              BOOL const activate = SetForegroundWindow(hWndPrevious);
              assert(TRUE == activate);

              HWND hWndActive = SetActiveWindow(hWndPrevious);
            }
          /* if */
        }
      /* if */
    }
  /* if */
}
/* SetFullscreenStatusOfDisplayWindow */

#pragma endregion // Windowed to fullscreen transitions


#pragma region // Delay adjustment and exposure to refresh rate matching

//! Set display and delay times for non-blocking acquisition.
/*!
  Sets display and delay times for non-blocking acquisition.

  \param parameters      Pointer to display window parameters.
  \param presentTime    Display time in units of VBLANK intervals. Must be positive whole number.
  \param delayTime_ms      Present-to-display delay in microseconds. Must be non-negative.
  \return Returns S_OK if successfull.
*/
HRESULT
SetDisplayAndDelayTimes(
                        DisplayWindowParameters * const parameters,
                        long int const presentTime,
                        double const delayTime_ms
                        )
{
  assert(NULL != parameters);
  if (NULL == parameters) return E_POINTER;

  HRESULT hr = S_OK;

  assert(0 < presentTime);
  if ( SUCCEEDED(hr) )
    {
      if (0 < presentTime)
        {
          parameters->presentTime = presentTime;
        }
      else
        {
          hr = E_INVALIDARG;
        }
      /* if */
    }
  /* if */

  assert(0 <= delayTime_ms);
  if ( SUCCEEDED(hr) )
    {
      if (0 <= delayTime_ms)
        {
          parameters->delayTime_ms = delayTime_ms;
        }
      else
        {
          hr = E_INVALIDARG;
        }
      /* if */
    }
  /* if */

  AdjustPresentAndDelayTimes_inline(hr, parameters);

  return hr;
}
/* SetDisplayAndDelayTimes */



//! Get frame duration in us from refresh rate.
/*!
  Computes frame duration in us (microseconds) from screen refresh rate.

  \param parameters      Pointer to display window parameters.
  \return Frame duration in us (microseconds) or NaN if frame rate cannot be determined.
*/
double
FrameDurationFromRefreshRate(
                             DisplayWindowParameters * const parameters
                             )
{
  double displayFrequency = BATCHACQUISITION_qNaN_dv;
  double frameDuration = BATCHACQUISITION_qNaN_dv;

  assert(NULL != parameters);
  if (NULL == parameters) return frameDuration;

  /* First try to fetch last stored refresh rate.
     Note that the refresh rate may be 0/0 which indicates automatic frequency selection.
  */
  DXGI_RATIONAL const * const pNewFrq = &(parameters->sRefreshRate);
  if ( (NULL != pNewFrq) && (0 < pNewFrq->Denominator) && (0 < pNewFrq->Numerator) )
    {
      displayFrequency = (double)(pNewFrq->Numerator) / (double)(pNewFrq->Denominator); // Hz (Hertz)
      assert(0 < displayFrequency);

      frameDuration = 1000000.0 / displayFrequency; // Convert Hz to us (microseconds).
      assert(0 < frameDuration);

      return frameDuration;
    }
  /* if */

  /* If stored frequency is invalid try to read DXGI swap chain description.
     Swap chain description may again return 0/0 which indicates the frequency is automatically set.
  */
  BOOL const enteredRenderAndPresent = TryEnterCriticalSection( &(parameters->csRenderAndPresent) );
  if (FALSE != enteredRenderAndPresent)
    {
      DXGI_SWAP_CHAIN_DESC swap_chain_desc;
      ZeroMemory( &swap_chain_desc, sizeof(swap_chain_desc) );

      HRESULT const hr = parameters->pSwapChain->GetDesc( &swap_chain_desc );
      assert( SUCCEEDED(hr) );

      if ( SUCCEEDED(hr) )
        {
          DXGI_RATIONAL const sRefreshRate = swap_chain_desc.BufferDesc.RefreshRate;
          if ( (0 < sRefreshRate.Numerator) && (0 < sRefreshRate.Denominator) )
            {
              displayFrequency = (double)(sRefreshRate.Numerator) / (double)(sRefreshRate.Denominator); // Hz (Hertz)
              frameDuration = 1000000.0 / displayFrequency; // Convert Hz to us (microseconds).
            }
          /* if */
        }
      /* if */

      LeaveCriticalSection( &(parameters->csRenderAndPresent) );
    }
  /* if */

  /* If frequency cannot be determined by querying DXGI swap chain then revert to Windows API
     and query the current display mode of the monitor which contains the window.
  */
  if (true == isnan_inline(frameDuration))
    {
      HMONITOR const hMonitor = MonitorFromWindow(parameters->hWnd, MONITOR_DEFAULTTOPRIMARY);
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
              displayFrequency = (double)(deviceMode.dmDisplayFrequency); // Hz (Hertz)
              frameDuration = 1000000.0 / displayFrequency; // Convert Hz to us (microseconds).
            }
          /* if */
        }
      /* if */
    }
  /* if */

  return frameDuration;
}
/* FrameDurationFromRefreshRate */



//! Adjust trigger delays.
/*!
  Function adjusts trigger delays.

  \see AdjustTriggerDelays_inline

  \param ptr    Pointer to window data structure.
  \param exposureDuration       Camera exposure or frame duration, in microseconds.
  \param microseconds   Exposure multiplier.
  \param k   Exposure multiplier.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
AdjustTriggerDelays(
                    DisplayWindowParameters * const ptr,
                    double const exposureDuration,
                    double const k
                    )
{
  HRESULT hr = S_OK;
  AdjustTriggerDelays_inline(hr, ptr, exposureDuration, k);
  return hr;
}
/* AdjustTriggerDelays */

#pragma endregion // Delay adjustment and exposure to refresh rate matching


#pragma region // Get and set display window properties

//! Get display window size.
/*!
  Gets size of the display window.

  \param parameters     Pointer to display window parameters.
  \param window_width_out   Display window width.
  \param window_height_out  Display window height.
  \param rcMonitor_out   Monitor size in virtual screen coordinates (in px, not DPI dependent).
  \param rcWindow_out    Window size in virtual screen coordinates (in px, not DPI dependent).
  \return Returns S_OK if successfull.
*/
HRESULT
GetDisplayWindowSize(
                     DisplayWindowParameters * const parameters,
                     int * const window_width_out,
                     int * const window_height_out,
                     RECT * const rcMonitor_out,
                     RECT * const rcWindow_out
                     )
{
  assert(NULL != parameters);
  if (NULL == parameters) return E_POINTER;

  assert(NULL != window_width_out);
  if (NULL == window_width_out) return E_POINTER;

  assert(NULL != window_height_out);
  if (NULL == window_height_out) return E_POINTER;

  assert(true == parameters->fActive);
  if (false == parameters->fActive) return E_ABORT;

  //assert(false == parameters->fModeChange);
  if (true == parameters->fModeChange) return E_ABORT;

  HRESULT hr = S_OK;

  int window_width = -1;
  int window_height = -1;

  BOOL fullscreen = FALSE;

  RECT rcMonitor;
  ZeroMemory( &rcMonitor, sizeof(rcMonitor) );

  RECT rcClient;
  ZeroMemory( &rcClient, sizeof(rcClient) );

  RECT rcWindow;
  ZeroMemory( &rcWindow, sizeof(rcWindow) );

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  ZeroMemory( &swap_chain_desc, sizeof(swap_chain_desc) );

  DXGI_OUTPUT_DESC output_desc;
  ZeroMemory( &output_desc, sizeof(output_desc) );

  MONITORINFOEX monitor_info;
  ZeroMemory( &monitor_info, sizeof(monitor_info) );
  monitor_info.cbSize = sizeof(MONITORINFOEX);

  EnterCriticalSection( &(parameters->csRenderAndPresent) );
  {
    assert(NULL != parameters->pSwapChain);
    if (NULL == parameters->pSwapChain) hr = E_POINTER;

    if ( SUCCEEDED(hr) )
      {
        hr = parameters->pSwapChain->GetDesc( &swap_chain_desc );
        assert( SUCCEEDED(hr) );
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        window_width = (int)swap_chain_desc.BufferDesc.Width;
        window_height = (int)swap_chain_desc.BufferDesc.Height;
      }
    /* if */

    if ( SUCCEEDED(hr) )
      {
        hr = parameters->pSwapChain->GetFullscreenState(&fullscreen, NULL);
        assert( SUCCEEDED(hr) );
      }
    /* if */

    // Get screen coordinates.
    {
      HMONITOR const hMonitor = MonitorFromWindow(parameters->hWnd, MONITOR_DEFAULTTONEAREST);

      BOOL const br = GetMonitorInfo(hMonitor, &monitor_info);
      assert(TRUE == br);
      if (TRUE == br)
        {
          rcMonitor = monitor_info.rcMonitor;
        }
      else
        {
          hr = E_FAIL;
        }
      /* if */
    }

    // Get window coordinates.
    {
      BOOL const br = GetClientRect(parameters->hWnd, &rcClient);
      assert(TRUE == br);
      if (TRUE == br)
        {
          POINT pt1, pt2;
          pt1.x = rcClient.left;  pt1.y = rcClient.top;
          pt2.x = rcClient.right;  pt2.y = rcClient.bottom;
          BOOL const tr1 = ClientToScreen(parameters->hWnd, &pt1);
          assert(TRUE == tr1);
          BOOL const tr2 = ClientToScreen(parameters->hWnd, &pt2);
          assert(TRUE == tr2);
          if ( (TRUE == tr1) && (TRUE == tr2) )
            {
              rcWindow.left = pt1.x;  rcWindow.top = pt1.y;
              rcWindow.right = pt2.x;  rcWindow.bottom = pt2.y;
            }
          /* if */
        }
      else
        {
          hr = E_FAIL;
        }
      /* if */
    }
  }
  LeaveCriticalSection( &(parameters->csRenderAndPresent) );


  if (TRUE == fullscreen)
    {
      assert( (rcWindow.left == rcMonitor.left) &&
              (rcWindow.top == rcMonitor.top) &&
              (rcWindow.right == rcMonitor.right) &&
              (rcWindow.bottom == rcMonitor.bottom)
              );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      assert( (int)(rcWindow.right - rcWindow.left) == window_width );
      assert( (int)(rcWindow.bottom - rcWindow.top) == window_height );
    }
  /* if */

  // Assign outputs.
  *window_width_out = window_width;
  *window_height_out = window_height;
  if (NULL != rcMonitor_out) *rcMonitor_out = rcMonitor;
  if (NULL != rcWindow_out) *rcWindow_out = rcWindow;

  return hr;
}
/* GetDisplayWindowSize */



//! Update window title.
/*!
  Updates display window title.

  \param ptr    Pointer to window data structure.
*/
void
DisplayWindowUpdateTitle(
                         DisplayWindowParameters * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  DWORD const result = WaitForSingleObject(ptr->tWindow, 0);

  if ( (WAIT_OBJECT_0 != result) && (true == ptr->fActive) )
    {
      DWORD_PTR dwResult;
      LRESULT const sm =
        SendMessageTimeout(
                           ptr->hWnd,
                           WM_COMMAND, // Message.
                           MAKEWPARAM(DISPLAY_WINDOW_UPDATE_TITLE, 0),
                           MAKELPARAM(0, 0),
                           SMTO_NOTIMEOUTIFNOTHUNG,
                           1000, // Timeout in ms.
                           &dwResult
                           );
      //assert(0 != sm);
    }
  /* if */
}
/* DisplayWindowUpdateTitle */

#pragma endregion // Get and set display window properties


#endif /* !__BATCHACQUISITIONWINDOWDISPLAY_CPP */
