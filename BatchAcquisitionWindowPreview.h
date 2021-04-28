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
  \file   BatchAcquisitionWindowPreview.h
  \brief  Image preview window.

  Header for image preview window.

  \author Tomislav Petkovic
  \date   2017-02-28
*/


#ifndef __BATCHACQUISITIONWINDOWPREVIEW_H
#define __BATCHACQUISITIONWINDOWPREVIEW_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionDebug.h"


struct AcquisitionParameters_;


// Unique command numbers.
#define PREVIEW_WINDOW_EXIT 100 /*!< Exit command. */
#define PREVIEW_WINDOW_IMAGE_PUSHED 101 /*!< Image was pushed for display. */
#define PREVIEW_WINDOW_RESET_IMAGE_TRANSFORM 102 /*!< Resets image transformation matrix. */
#define PREVIEW_WINDOW_CCD_TOGGLE 200 /*!< Toggles visibility state of the camera control dialog. */
#define PREVIEW_WINDOW_CCD_OPEN 201 /*!< Shows the camera control dialog. */
#define PREVIEW_WINDOW_CCD_CLOSE 202 /*!< Hides the camera control dialog. */
#define PREVIEW_WINDOW_CCD_CLOSE_ALL 203 /*!< Hides all control dialogs. */
#define PREVIEW_WINDOW_CAMERA_0 300 /*!< Selects camera with ID 0 for display. */
#define PREVIEW_WINDOW_CAMERA_1 301 /*!< Selects camera with ID 1 for display. */
#define PREVIEW_WINDOW_CAMERA_2 302 /*!< Selects camera with ID 2 for display. */
#define PREVIEW_WINDOW_CAMERA_3 303 /*!< Selects camera with ID 3 for display. */
#define PREVIEW_WINDOW_CAMERA_4 304 /*!< Selects camera with ID 4 for display. */
#define PREVIEW_WINDOW_CAMERA_5 305 /*!< Selects camera with ID 5 for display. */
#define PREVIEW_WINDOW_CAMERA_6 306 /*!< Selects camera with ID 6 for display. */
#define PREVIEW_WINDOW_CAMERA_7 307 /*!< Selects camera with ID 7 for display. */
#define PREVIEW_WINDOW_CAMERA_8 308 /*!< Selects camera with ID 8 for display. */
#define PREVIEW_WINDOW_CLEAR_CAMERA 400 /*!< Clears acquisition thread pointer. */
#define PREVIEW_WINDOW_RESTORE_CAMERA 401 /*!< Restores acquisition thread pointer (if possible). */
#define PREVIEW_WINDOW_UPDATE_TITLE 500 /*!< Updates window title. */

#define MAX_LOADSTRING 1024 /*!< Maximum static string length. */


//! Preview window parameters.
/*!
  This structure stores all of preview window parameters.
*/
typedef
struct PreviewWindowParameters_
{
  HINSTANCE hInstance; //!< A handle to the current instance of the application.
  HINSTANCE hPrevInstance; //!< A handle to the previous instance of the application.

  TCHAR szTitle[MAX_LOADSTRING + 1]; //!< The title bar text.
  TCHAR szWindowClass[MAX_LOADSTRING + 1]; //!< The main window class name.

  int nCmdShow; //!< Controls how the window is to be shown.

  HWND hWnd; //!< Handle to the created window.
  HWND hWndParent; //!< Handle to the parent window.

  HANDLE tWindow; //!< Handle to a thread running the window message pump.

  PastMessages * pMsg; //!< A list of past messages handled by the message pump.

  volatile bool fActive; //!< Flag to indicate background thread is active.
  volatile bool fModeChange; //!< Flag to indicate we are processing messages that affect the swap chain.
  volatile bool fProcessingImage; //!< Flag to indicate we are processing last image pushed for display.
  volatile bool fRenderAgain; //!< Flag to indicate the scene should be rendered again.
  volatile bool fDialogShown; //!< Flag to indicate camera control dialog was shown.

  IDXGIAdapter * pAdapter; //!< DXGI adapter assigned to the window.
  IDXGIOutput * pOutput; //!< DXGI output assigned to the window.
  ID3D11Device * pDevice; //!< Direct 3D 11 device assigned to the window.
  ID3D11DeviceContext * pDeviceContext; //!< Direct 3D 11 device context.

  IDXGISwapChain * pSwapChain; //!< DXGI swap chain assigned to the window.

  IDXGISurface * pBackBuffer; //!< Back buffer of the swap chain.
  ID2D1RenderTarget * pRenderTarget; //!< Direct2D render target associated with the DXGI swap chain back buffer.

  HMONITOR hSwapChainMonitor; //!< Handle to monitor associated with swap chain.

  IDXGIFactory1 * pDXGIFactory1; //!< Copy of a pointer to DXGI factory.
  ID2D1Factory * pD2DFactory; //!< Copy of a pointer to Direct2D factory.

  DXGI_SWAP_CHAIN_DESC sSwapChainDesc; //!< Initial swap chain description.

  std::vector<AcquisitionParameters_ *> * pAcquisitions; //!< Pointer to all acquisition parameters structures.
  SRWLOCK * pAcquisitionsLock; //!< Pointer to SRW lock.

  AcquisitionParameters_ * pAcquisition; //!< Pointer to acquisition parameters structure of the currently selected camera.
  int CameraID; //!< ID of the camera selected for preview.

  void * pData; //!< Pointer to raw image data.
  size_t data_size; //!< Size of allocated memory block.
  ImageDataType data_type; //!< Image data type.
  unsigned int data_height; //!< Image height.
  unsigned int data_width; //!< Image width.
  unsigned int data_stride; //!< Image stride (length of one row in bytes).

  cv::Mat * pImageTMP; //!< Temporary storage.
  cv::Mat * pImageBGR; //!< BGR image.
  cv::Mat * pImageBGRA; //!< BGRA image.

  D2D1_POINT_2F ptMouse; //!< Mouse position on first click.
  float scaleX; //!< Conversion factor from pixels to device independent pixels.
  float scaleY; //!< Conversion factor from pixels to device independent pixels.

  D2D1_MATRIX_3X2_F sImageTransform; //!< Image transformation matrix.

  __int64 QPC_last_push; //!< QPC value at last update.
  __int64 QPC_max_present_interval; //!< Maximal present time for one image in QPC ticks.

  CRITICAL_SECTION csRenderAndPresent; //!< Critical section for syncronizing access to Direct X.
  CRITICAL_SECTION csCamera; //!< Critical section for syncronizing access to camera parameters.
  CRITICAL_SECTION csData; //!< Critical section for syncronizing image push operations.
  CRITICAL_SECTION csTransform; //!< Critical section for syncronizing image push operations.
} PreviewWindowParameters;


/****** DIRECT 2D/3D  ******/

//! Releases swap chain and Direct 3D device.
void DeleteDirectXDeviceAndSwapChain(PreviewWindowParameters * const);

//! Recreate Direct2D render target.
HRESULT RecreateDirect2DRenderTarget(PreviewWindowParameters * const);

//! Recreate Direct 3D device and swap chain.
HRESULT RecreateDirectXDeviceAndSwapChain(PreviewWindowParameters * const);

//! Creates Direct 3D device and swap chain.
HRESULT CreateDirectXDeviceAndSwapChain(PreviewWindowParameters * const, IDXGIFactory1 * const, ID2D1Factory * const);

//! Resizes swap chain.
HRESULT ResizeSwapChain(PreviewWindowParameters * const, UINT const, UINT const);


/****** UPDATES ******/

// Pushes image to display.
void PushImage(PreviewWindowParameters * const, int const, unsigned int const, unsigned int const, unsigned int const, ImageDataType const, void const * const);


#ifdef HAVE_SAPERA_SDK

// Pushes image to display.
void PushImage(PreviewWindowParameters * const, int const, SapBuffer * const, SapAcqDevice * const);

#endif /* HAVE_SAPERA_SDK */


#ifdef HAVE_FLYCAPTURE2_SDK

// Pushes image to display.
void PushImage(PreviewWindowParameters * const, int const, FlyCapture2::Image * const, FlyCapture2::Camera * const);

#endif /* HAVE_FLYCAPTURE2_SDK */


/****** OPEN AND CLOSE WINDOW ******/

//! Opens display window and starts message pump.
PreviewWindowParameters * OpenPreviewWindow(HINSTANCE const, TCHAR const * const, TCHAR const * const, int const, HWND const);

//! Close display window.
void ClosePreviewWindow(PreviewWindowParameters * const);


/****** AUXILIARY FUNCTIONS ******/

//! Connect to acquisition thread.
void ConnectToAcquisitionThreads(PreviewWindowParameters * const, std::vector<AcquisitionParameters_ *> * const, SRWLOCK * const, int const);

//! Disconnect from acquisition thread.
void DisconnectFromAcquisitionThreads(PreviewWindowParameters * const);

//! Clear active camera.
void ClearActiveCamera(PreviewWindowParameters * const);

//! Restores active camera.
void RestoreActiveCamera(PreviewWindowParameters * const);

//! Close camera configuration dialog.
void CloseCameraConfigurationDialog(PreviewWindowParameters * const);

//! Toggle camera configuration dialog.
void ToggleCameraConfigurationDialog(PreviewWindowParameters * const);

//! Update window title.
void PreviewWindowUpdateTitle(PreviewWindowParameters * const);


#endif /* !__BATCHACQUISITIONWINDOWPREVIEW_H */
