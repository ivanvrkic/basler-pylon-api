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
  \file   BatchAcquisitionWindowDisplay.h
  \brief  Empty display window for DirectX rendering.

  Header for display window functions.

  \author Tomislav Petkovic
  \date   2017-01-13
*/


#ifndef __BATCHACQUISITIONWINDOWDISPLAY_H
#define __BATCHACQUISITIONWINDOWDISPLAY_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionImageDecoder.h"


// Unique command numbers.
#define DISPLAY_WINDOW_EXIT 100 /*!< Exit command. */
#define DISPLAY_WINDOW_FULLSCREEN 101 /*!< If not fullscreen then go to fullscreen mode. */
#define DISPLAY_WINDOW_WINDOW 102 /*!< If not windowed then go to window mode. */
#define DISPLAY_WINDOW_CHANGE_RESOLUTION 103 /*!< Desired fullscreen resolution was changed so update swap chain. */
#define DISPLAY_WINDOW_ALT_ENTER 104 /*!< If not windowed then go to window mode. */
#define DISPLAY_WINDOW_UPDATE_TITLE 405 /*!< Update window title. */
#define DISPLAY_WINDOW_FREEZE 406 /*!< Freezes currently rendered image. */

#define MAX_LOADSTRING 1024 /*!< Maximum static string length. */



//! Window parameters.
/*!
  This structure stores all of display window parameters.

  This structure is associated with its display window thread and rendering threads.
  Every acquisition thread attached to the rendering thread may also access this structure.

  This structure therefore stores all acquisition flags which control how both rendering
  and acquisition threads behave. In addition the structure stores general timing information
  for non-blocking acquisition mode. Note that present-to-display delay for blocking acquisition mode
  is defined in RenderingParameters_ structure.
*/
typedef
struct DisplayWindowParameters_
{
  HINSTANCE hInstance; //!< A handle to the current instance of the application.
  HINSTANCE hPrevInstance; //!< A handle to the previous instance of the application.

  TCHAR szTitle[MAX_LOADSTRING + 1]; //!< The title bar text.
  TCHAR szWindowClass[MAX_LOADSTRING + 1]; //!< The main window class name.

  int nCmdShow; //!< Controls how the window is to be shown.

  HWND hWnd; //!< Handle to the created window.
  HWND hWndParent; //!< Handle to the parent window.
  HWND hWndCommand; //!< Handle of the command window.

  HANDLE tWindow; //!< Handle to a thread running the window message pump.

  int ProjectorID; //!< Unique ID of the projector associated with this window.

  UINT width; //!< Last width from WM_SIZE window message.
  UINT height; //!< Last height from WM_SIZE window message.

  PastMessages * pMsg; //!< A list of past messages handled by the message pump.

  volatile bool fActive; //!< Flag to indicate background thread is active.
  volatile bool fFullscreen; //!< Flag indicating full screen mode.
  volatile bool fModeChange; //!< Flag to indicate we are processing messages that affect the swap chain.
  volatile bool fRecreated; //!< Flag to indicate the DXGI swap chain was recreated.
  volatile bool fResized; //!< Flag to indicate the DXGI swap chain was resized when WM_SIZE message was received.

  /* Acquisition parameters. */

  volatile bool fFreeze; //!< Flag to indicate the present operation of the rendering thread should be skipped.
  volatile bool fRenderAndPresent; //!< Flag to indicate the rendering thread is rendering or presenting next frame.
  volatile bool fWaitForVBLANK; //!< Flag to indicate the rendering thread is waiting for VBLANK.
  volatile bool fBlocking; //!< Flag to indicate we are using blocking acquisition.
  volatile bool fConcurrentDelay; //!< Flag to indicate delay wait and camera exposure are concurrent events.
  volatile bool fFixed; //!< Flag to indicate we are using fixed SL pattern making synchronization unnecessary.

  volatile int num_acquire; //!< Number of images to acquire when using the fixed SL pattern acquisition.

  /* DirectX variables. */

  IDXGIAdapter * pAdapter; //!< DXGI adapter assigned to the window.
  IDXGIOutput * pOutput; //!< DXGI output assigned to the window.
  ID3D11Device * pDevice; //!< Direct 3D 11 device assigned to the window.
  ID3D11DeviceContext * pDeviceContext; //!< Direct 3D 11 device context.

  IDXGISwapChain * pSwapChain; //!< DXGI swap chain assigned to the window.

  IDXGISurface * pBackBuffer; //!< Back buffer of the swap chain.
  ID2D1RenderTarget * pRenderTarget; //!< Direct2D render target associated with the DXGI swap chain back buffer.
  ID2D1SolidColorBrush * pBlackBrush; //!< Black brush associated with Direct2D render targget.
  ID2D1SolidColorBrush * pYellowBrush; //!< Yellow brush associated with Direct2D render target.
  IDWriteTextFormat * pTextFormat; //!< Text format for projector ID string.

  HMONITOR hSwapChainMonitor; //!< Handle to monitor associated with swap chain.

  IDXGIFactory1 * pDXGIFactory1; //!< Copy of a pointer to DXGI factory.
  ID2D1Factory * pD2DFactory; //!< Copy of a pointer to Direct2D factory.

  DXGI_SWAP_CHAIN_DESC sSwapChainDesc; //!< Initial swap chain description.
  DXGI_RATIONAL sRefreshRate; //!< Swap chain refresh rate.
  DXGI_MODE_DESC sCurrentMode; //!< Currently selected display mode.
  DXGI_MODE_DESC sWindowMode; //!< Target display mode when the display window is not fullscreen.
  DXGI_MODE_DESC sFullScreenMode; //!< Target display mode when the display window is fullscreen.

  DXGI_FRAME_STATISTICS sStatisticsPresent; //!< DXGI statistics after present call.

  /* QPC timing and constants for unit conversion. */

  LARGE_INTEGER frequency; /*!< CPU frequency for QPC. */
  double inv_frequency; /*!< Duration of one QPC cycle. */

  double ticks_to_us; /*!< Multiplication factor to convert ticks to microseconds. */
  double us_to_ticks; /*!< Multiplication factor to convert microseconds to ticks. */
  double ticks_to_ms; /*!< Multiplication factor to convert ticks to milliseconds. */
  double ms_to_ticks; /*!< Multiplication factor to convert milliseconds to ticks. */

  double us_to_vblanks; /*!< Multiplication factor to convert microseconds to vblanks. */
  double vblanks_to_us; /*!< Multiplication factor to convert vblanks to microseconds. */
  double ticks_to_vblanks; /*!< Multiplication factor to convert ticks to VBLANKs. */
  double vblanks_to_ticks; /*!< Multiplication factor to convert VBLANKs to ticks. */

  /* Present time. */

  long int presentTime; //!< Frame display time in VBLANKS; used for non-blocking acquisition only. Set by user.
  double presentTime_us; //!< Frame display time in us (rounded). Derived from presentTime.
  double refreshTime_ms; //!< Screen refresh time in ms.
  __int64 QPC_presentTime; //!< Number of QPC ticks for one frame display (rounded). Derived from presentTime.
  __int64 QPC_refreshTime; //!< Number of QPC ticks for one screen refresh interval (rounded). Updated each time refresh rate changes.

  /* Present-to-trigger delays. */

  double delayTime_ms; //!< Total present-to-display delay time in ms (milliseconds) for non-blocking acquisition. Ignored for blocking acquisition. Set by user.
  double delayTime_us; //!< Total present-to-display delay time in us (microseconds) for non-blocking acquisition. Derived from delayTime_ms.
  double delayTime_fraction_us; //!< Fractional part of present-to-display delay in us (microseconds) for non-blocking acquisition. Derived from delayTime_ms.
  long int delayTime_whole; //!< Whole part of present-to-display delay in VBLANK units. Derived from delayTime_ms.
  __int64 QPC_delayTime_whole; //!< Number of QPC ticks for the whole part of present-to-display delay.
  __int64 QPC_delayTime; //!< Number of QPC ticks for total present-to-display delay for non-blocking acquisition. Derived from delayTime_ms.
  __int64 QPC_delay_for_trigger_scheduled_RT; //!< Number of QPC ticks for the minimal present-to-delay time.
  __int64 QPC_delay_for_trigger_scheduled_AT; //!< Expected number of QPC ticks for the optimal present-to-delay time.

  __int64 QPC_delayDelta; //!< Additional optimal delay which is allowed after QPC_delay_for_trigger_scheduled_AT elapses.

  /* Exposure time. */

  long int exposureTime_whole; //!< Exposure time measured in VBLANK units (rounded towards infinity).
  __int64 QPC_exposureTime; //!< Number of QPC ticks for one full frame exposure (rounded towards infinity).

  /* Counters. */

  volatile long int vblank_counter; //!< Local counter for VBLANK events. Should be updated by rendering thread only.
  volatile long int present_counter; //!< Local counter for present calls. Should be updated by rendering thread only.

  long int vblank_counter_after_present_RT; //!< Value of VBLANK_counter at last present call.
  long int present_counter_after_present_RT; //!< Value of present counter at last present call.

  /* Rendered image. */

  void * pRendering; //!< Pointer to image rendering thread.
  QueuedDecoderImage * pImage; //!< Last rendered image.

  /* Concurrent access. */

  SRWLOCK sLockRT; //!< Slim lock for rendering thread in exclusive mode and other threads in shared mode. Used to control access to VBlank and present counters.
  SRWLOCK sLockImage; //!< Slim lock for rendered image data.

  CRITICAL_SECTION csRenderAndPresent; //!< Critical section for syncronizing full access to Direct 3D.
  CRITICAL_SECTION csWaitForVBLANK; //!< Critical section for syncronizing partial access to Direct 3D for WaitForVBlank method only.
} DisplayWindowParameters;


/****** DIRECT 2D/3D  ******/

//! Releases swap chain and Direct 3D device.
void DeleteDirectXDeviceAndSwapChain(DisplayWindowParameters * const);

//! Recreate Direct2D render target.
HRESULT RecreateDirect2DRenderTarget(DisplayWindowParameters * const);

//! Recreate Direct 3D device and swap chain.
HRESULT RecreateDirectXDeviceAndSwapChain(DisplayWindowParameters * const);

//! Creates Direct 3D device and swap chain.
HRESULT CreateDirectXDeviceAndSwapChain(DisplayWindowParameters * const, IDXGIFactory1 * const, ID2D1Factory * const);

//! Resizes swap chain.
HRESULT ResizeSwapChain(DisplayWindowParameters * const, UINT const, UINT const);

//! Update fullscreen state.
HRESULT UpdateSwapChainFullscreenStatus(DisplayWindowParameters * const);

//! Changes resolution in fullscreen mode.
HRESULT ChangeFullScreenResolution(DisplayWindowParameters * const);

//! Toggle full screen.
HRESULT ToggleFullScreen(DisplayWindowParameters * const);

//! Makes swap chain fullscreen.
HRESULT GoFullscreen(DisplayWindowParameters * const);

//! Makes swap chain windowed.
HRESULT GoWindow(DisplayWindowParameters * const);

//! Render image.
HRESULT RenderQueuedImage(DisplayWindowParameters * const, QueuedDecoderImage * const);

//! Blank window.
HRESULT RenderBlankImage(DisplayWindowParameters * const);


/****** OPEN AND CLOSE WINDOW ******/

//! Opens display window and starts message pump.
DisplayWindowParameters * OpenDisplayWindow(HINSTANCE const, int const, int const, HWND const, HWND const);

//! Close display window.
void CloseDisplayWindow(DisplayWindowParameters * const);


/****** AUXILIARY FUNCTIONS ******/

//! Changes fullscreen status of display window.
void ToggleFullscreenStatusOfDisplayWindow(DisplayWindowParameters * const);

//! Changes fullscreen status of display window.
void SetFullscreenStatusOfDisplayWindow(DisplayWindowParameters * const, bool const);

//! Set display and delay times for non-blocking acquisition.
HRESULT SetDisplayAndDelayTimes(DisplayWindowParameters * const, long int const, double const);

//! Get frame duration in us from refresh rate.
double FrameDurationFromRefreshRate(DisplayWindowParameters * const);

//! Adjust delay limits.
HRESULT AdjustTriggerDelays(DisplayWindowParameters * const, double const, double const);

//! Get display window size.
HRESULT GetDisplayWindowSize(DisplayWindowParameters * const, int * const, int * const, RECT * const, RECT * const);

//! Update window title.
void DisplayWindowUpdateTitle(DisplayWindowParameters * const);



#endif /* !__BATCHACQUISITIONWINDOWDISPLAY_H */
