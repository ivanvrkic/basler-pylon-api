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
  \file   BatchAcquisitionWindowPreview.cpp
  \brief  Camera live preview window.

  Functions to create live preview window for the camera.

  \author Tomislav Petkovic
  \date   2017-02-28
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONWINDOWPREVIEW_CPP
#define __BATCHACQUISITIONWINDOWPREVIEW_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionWindowPreview.h"
#include "BatchAcquisitionWindowStorage.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionSwapChain.h"
#include "BatchAcquisitionAcquisition.h"
#include "BatchAcquisitionImageConversion.h"




/****** INLINE HELPER FUNCTIONS ******/

//! Blank PreviewWindowParameters structure.
/*!
  Blanks structure.

  \param ptr Pointer to PreviewWindowParameters structure.
*/
inline
void
BlankPreviewWindowParameters_inline(
                                    PreviewWindowParameters * const ptr
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
  ptr->tWindow = NULL;
  ptr->pMsg = NULL;

  ptr->fActive = false;
  ptr->fModeChange = true;
  ptr->fProcessingImage = false;
  ptr->fRenderAgain = false;
  ptr->fDialogShown = false;

  ptr->pAdapter = NULL;
  ptr->pOutput = NULL;
  ptr->pDevice = NULL;
  ptr->pDeviceContext = NULL;
  ptr->pSwapChain = NULL;
  ptr->pBackBuffer = NULL;
  ptr->pRenderTarget = NULL;
  ptr->hSwapChainMonitor = NULL;

  ptr->pDXGIFactory1 = NULL;
  ptr->pD2DFactory = NULL;

  ZeroMemory( &(ptr->sSwapChainDesc), sizeof(ptr->sSwapChainDesc) );

  ptr->pAcquisitions = NULL;
  ptr->pAcquisitionsLock = NULL;
  ptr->CameraID = -1;
  ptr->pAcquisition = NULL;

  ptr->pData = NULL;
  ptr->data_size = 0;
  ptr->data_type = IDT_UNKNOWN;
  ptr->data_height = 0;
  ptr->data_width = 0;
  ptr->data_stride = 0;
  ptr->pImageTMP = NULL;
  ptr->pImageBGR = NULL;
  ptr->pImageBGRA = NULL;

  ZeroMemory( &(ptr->ptMouse), sizeof(ptr->ptMouse) );

  ptr->scaleX = 1.0f;
  ptr->scaleY = 1.0f;
  ptr->sImageTransform = D2D1::Matrix3x2F::Identity();

  ptr->QPC_last_push = -1;
  ptr->QPC_max_present_interval = -1;

  ZeroMemory( &(ptr->csRenderAndPresent), sizeof(ptr->csRenderAndPresent) );
  ZeroMemory( &(ptr->csCamera), sizeof(ptr->csCamera) );
  ZeroMemory( &(ptr->csData), sizeof(ptr->csData) );
  ZeroMemory( &(ptr->csTransform), sizeof(ptr->csTransform) );
}
/* BlankPreviewWindowParameters_inline */



/****** CAMERA CONTROL DIALOG HANDLERS ******/

//! Toggles state of camera control dialog.
/*!
  Toggles the state of camera control dialog.
  If the dialog is closed it is (re)opened.
  If the dialog is open then it is closed.

  \param ptr    Pointer to preview window parameters.
*/
void
inline
CameraControlDialogToggle_inline(
                                 PreviewWindowParameters * const ptr
                                 )
{
  static bool info_sapera = false;

  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (NULL == ptr->pAcquisition) return;

  EnterCriticalSection( &(ptr->csCamera) );
  {
    if (NULL != ptr->pAcquisition)
      {
        assert( ptr->CameraID == ptr->pAcquisition->CameraID );

        if (NULL != ptr->pAcquisition->pFlyCapture2SDK)
          {
            ptr->fDialogShown =
              AcquisitionParametersFlyCapture2ControlDialogToggle(
                                                                  ptr->pAcquisition->pFlyCapture2SDK,
                                                                  ptr->pAcquisition->CameraID
                                                                  );
          }
        else if (NULL != ptr->pAcquisition->pSaperaSDK)
          {
            if (false == info_sapera)
              {
                int const res = MessageBox(ptr->hWnd, gMsgCameraControlNotImplementedForSapera, gMsgInformationPopUpTitle, MB_OK | MB_ICONINFORMATION);
                info_sapera = true;
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csCamera) );
}
/* CameraControlDialogToggle_inline */



//! Open camera control dialog.
/*!
  Opens the camera control dialog.

  \param ptr    Pointer to preview window parameters.
*/
void
inline
CameraControlDialogOpen_inline(
                               PreviewWindowParameters * const ptr
                               )
{
  static bool info_sapera = false;

  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (NULL == ptr->pAcquisition) return;

  EnterCriticalSection( &(ptr->csCamera) );
  {
    if ( (false == ptr->fDialogShown) && (NULL != ptr->pAcquisition) )
      {
        assert( ptr->CameraID == ptr->pAcquisition->CameraID );

        if (NULL != ptr->pAcquisition->pFlyCapture2SDK)
          {
            ptr->fDialogShown =
              AcquisitionParametersFlyCapture2ControlDialogOpen(
                                                                ptr->pAcquisition->pFlyCapture2SDK,
                                                                ptr->pAcquisition->CameraID
                                                                );
          }
        else if (NULL != ptr->pAcquisition->pSaperaSDK)
          {
            if (false == info_sapera)
              {
                int const res = MessageBox(ptr->hWnd, gMsgCameraControlNotImplementedForSapera, gMsgInformationPopUpTitle, MB_OK | MB_ICONINFORMATION);
                info_sapera = true;
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csCamera) );
}
/* CameraControlDialogOpen_inline */



//! Close camera control dialog.
/*!
  Closes the camera control dialog.

  \param ptr    Pointer to preview window parameters.
*/
void
inline
CameraControlDialogClose_inline(
                                PreviewWindowParameters * const ptr
                                )
{
  static bool info_sapera = false;

  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (NULL == ptr->pAcquisition) return;

  EnterCriticalSection( &(ptr->csCamera) );
  {
    if ( (true == ptr->fDialogShown) && (NULL != ptr->pAcquisition) )
      {
        assert( ptr->CameraID == ptr->pAcquisition->CameraID );

        if (NULL != ptr->pAcquisition->pFlyCapture2SDK)
          {
            ptr->fDialogShown =
              !AcquisitionParametersFlyCapture2ControlDialogClose(
                                                                  ptr->pAcquisition->pFlyCapture2SDK,
                                                                  ptr->pAcquisition->CameraID
                                                                  );
          }
        else if (NULL != ptr->pAcquisition->pSaperaSDK)
          {
            if (false == info_sapera)
              {
                int const res = MessageBox(ptr->hWnd, gMsgCameraControlNotImplementedForSapera, gMsgInformationPopUpTitle, MB_OK | MB_ICONINFORMATION);
                info_sapera = true;
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csCamera) );
}
/* CameraControlDialogClose_inline */



/****** CAMERA SELECTION HANDLERS ******/


//! Set window title.
/*!
  Sets window title text.

  \param ptr    Pointer to preview window parameters.
  \param CameraID       Camera ID.
*/
inline
void
SetWindowTitle_inline(
                      PreviewWindowParameters * const ptr,
                      int const CameraID
                      )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  AcquisitionParameters_ * const pAcquisition = get_ptr_inline(*(ptr->pAcquisitions), CameraID, ptr->pAcquisitionsLock);
  if (NULL == pAcquisition) return;

  TCHAR szTitle[MAX_LOADSTRING + 1];  

  EnterCriticalSection( &(ptr->csCamera) );
  {
    int const CameraID = pAcquisition->CameraID;
    int const ProjectorID = pAcquisition->ProjectorID;
    std::wstring * CameraUID = GetUniqueCameraIdentifier(pAcquisition);

    if (NULL != CameraUID)
      {
        int const cnt = swprintf_s(szTitle, MAX_LOADSTRING, gNameWindowPreviewKnownCameraIDAndUID, CameraID + 1, ProjectorID + 1, CameraUID->c_str());
        assert(0 < cnt);
      }
    else
      {
        int const cnt = swprintf_s(szTitle, MAX_LOADSTRING, gNameWindowPreviewKnownCameraID, CameraID + 1, ProjectorID + 1);
        assert(0 < cnt);
      }
    /* if */
    szTitle[MAX_LOADSTRING] = 0;
    
    SAFE_DELETE(CameraUID);
  }
  LeaveCriticalSection( &(ptr->csCamera) );  
  
  BOOL const set_title = SetWindowText(ptr->hWnd, szTitle);
  assert(0 != set_title);
}
/* SetWindowTitle_inline */



//! Selects active camera.
/*!
  Sets camera with given ID as active.

  \param ptr    Pointer to preview window parameters.
  \param CameraID       Camera ID.
*/
inline
void
SelectActiveCamera_inline(
                          PreviewWindowParameters * const ptr,
                          int const CameraID
                          )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  AcquisitionParameters_ * const pAcquisition = get_ptr_inline(*(ptr->pAcquisitions), CameraID, ptr->pAcquisitionsLock);
  if (NULL != pAcquisition)
    {
      assert(CameraID == pAcquisition->CameraID);
      
      EnterCriticalSection( &(ptr->csCamera) );
      {
        if (true == ptr->fDialogShown) CameraControlDialogClose_inline(ptr);
        assert(false == ptr->fDialogShown);

        ptr->pAcquisition = pAcquisition;
        ptr->CameraID = CameraID;

        SetWindowTitle_inline(ptr, ptr->CameraID);
      }
      LeaveCriticalSection( &(ptr->csCamera) );
    }
  /* if */
}
/* SelectActiveCamera_inline */



//! Clears active camera.
/*!
  Clears active camera.

  \param ptr    Pointer to preview window parameters.
*/
inline
void
ClearActiveCamera_inline(
                         PreviewWindowParameters * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  EnterCriticalSection( &(ptr->csCamera) );
  {
    if (true == ptr->fDialogShown) CameraControlDialogClose_inline(ptr);
    assert(false == ptr->fDialogShown);

    ptr->pAcquisition = NULL;
  }
  LeaveCriticalSection( &(ptr->csCamera) );

  TCHAR szTitle[MAX_LOADSTRING + 1];

  int const cnt = swprintf_s(szTitle, MAX_LOADSTRING, gNameWindowPreviewNoCamera);
  assert(0 < cnt);

  szTitle[MAX_LOADSTRING] = 0;

  BOOL const set_title = SetWindowText(ptr->hWnd, szTitle);
  assert(0 != set_title);
}
/* ClearActiveCamera_inline */



/****** RENDER TARGET TRANSFORM ******/

//! Get render target transform matrix.
/*!
  Returns render target transform matrix.

  \param ptr    Pointer to preview window parameters.
  \return 3x2 transform matrix.
*/
inline
D2D1_MATRIX_3X2_F
GetRenderTargetTransform_inline(
                                PreviewWindowParameters * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return D2D1::Matrix3x2F::Identity();

  D2D1_MATRIX_3X2_F sTransformMatrix;

  EnterCriticalSection( &(ptr->csTransform) );
  {
    sTransformMatrix = ptr->sImageTransform;
  }
  LeaveCriticalSection( &(ptr->csTransform) );

  return sTransformMatrix;
}
/* GetRenderTargetTransform_inline */



//! Resets render target transform.
/*!
  Resets render target transform to identity matrix.

  \param ptr    Pointer to preview window parameters.
  \return Returns S_OK if successfull.
*/
HRESULT
inline
ResetRenderTargetTransform_inline(
                                  PreviewWindowParameters * const ptr
                                  )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  EnterCriticalSection( &(ptr->csTransform) );
  {
    ptr->sImageTransform = D2D1::Matrix3x2F::Identity();
  }
  LeaveCriticalSection( &(ptr->csTransform) );

  ptr->fRenderAgain = true;

  return S_OK;
}
/* ResetRenderTargetTransform_inline */



//! Adds transformation.
/*!
  Adds another transformation to the render target transform.

  \param ptr    Pointer to preview window parameters.
  \param sTransformMatrix Transformation matrix to add.
  \return Returns S_OK if successfull.
*/
HRESULT
inline
AddToRenderTargetTransform_inline(
                                  PreviewWindowParameters * const ptr,
                                  D2D1_MATRIX_3X2_F const sTransformMatrix
                                  )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  EnterCriticalSection( &(ptr->csTransform) );
  {
    ptr->sImageTransform = ptr->sImageTransform * sTransformMatrix;
  }
  LeaveCriticalSection( &(ptr->csTransform) );

  ptr->fRenderAgain = true;

  return S_OK;
}
/* AddToRenderTargetTransform_inline */



/****** PUSHED IMAGE HANDLERS ******/

//! Decodes pushed image.
/*!
  Function decodes pushed image into BGRA 8-bit format that is required by the Direct2D.

  \param ptr    Pointer to display window data.
*/
void
inline
DecodePushedImage_inline(
                         PreviewWindowParameters * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(NULL != ptr->pImageTMP);
  if (NULL == ptr->pImageTMP) return;

  assert(NULL != ptr->pImageBGR);
  if (NULL == ptr->pImageBGR) return;

  assert(NULL != ptr->pImageBGRA);
  if (NULL == ptr->pImageBGRA) return;

  //assert(true == ptr->fProcessingImage);

  EnterCriticalSection( &(ptr->csData) );
  {
    switch (ptr->data_type)
      {
      default:
      case IDT_UNKNOWN:
        break;

      case IDT_8U_BINARY:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.convertTo(*(ptr->pImageTMP), CV_8U, 256.0); // Saturate data.
          cv::cvtColor(*(ptr->pImageTMP), *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_8U_GRAY:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_10U_GRAY:
        {
          HRESULT const hr = Shrink16BitLSB10To8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_GRAY;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_12U_GRAY_Packed:
        {
          HRESULT const hr = Shrink12BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_GRAY;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_16U_GRAY:
        {
          HRESULT const hr = Shrink16BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_GRAY;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_16U_GRAY_BigEndian:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_GRAY;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_32U_GRAY:
        {
          HRESULT const hr = Shrink32BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_GRAY;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_8S_GRAY:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8SC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.convertTo(*(ptr->pImageTMP), CV_8UC1, 1.0, 128.0); // Affine scale to 8U; zero maps to 128.
          cv::cvtColor(*(ptr->pImageTMP), *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_16S_GRAY:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_16SC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.convertTo(*(ptr->pImageTMP), CV_8UC1, 1.0/256.0, 128.0);
          cv::cvtColor(*(ptr->pImageTMP), *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_16S_GRAY_BigEndian:
        {
          HRESULT const hr = SwapBytesMono16InPlace(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) ) ptr->data_type = IDT_16S_GRAY;

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_16SC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.convertTo(*(ptr->pImageTMP), CV_8UC1, 1.0/256.0, 128.0);
          cv::cvtColor(*(ptr->pImageTMP), *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_32S_GRAY:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_32SC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.convertTo(*(ptr->pImageTMP), CV_8UC1, 1.0/16777216.0, 128.0);
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_GRAY2BGRA);
        }
        break;

      case IDT_8U_BayerGR:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGB2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_BayerRG:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerBG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_BayerGB:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGR2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_BayerBG:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerRG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_10U_BayerGR:
        {
          HRESULT const hr = Shrink16BitLSB10To8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGR;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGB2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_10U_BayerRG:
        {
          HRESULT const hr = Shrink16BitLSB10To8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerRG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerBG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_10U_BayerGB:
        {
          HRESULT const hr = Shrink16BitLSB10To8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGB;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGR2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_10U_BayerBG:
        {
          HRESULT const hr = Shrink16BitLSB10To8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerBG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerRG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;


      case IDT_12U_BayerGR_Packed:
        {
          HRESULT const hr = Shrink12BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGR;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGB2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_12U_BayerRG_Packed:
        {
          HRESULT const hr = Shrink12BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerRG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerBG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_12U_BayerGB_Packed:
        {
          HRESULT const hr = Shrink12BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGB;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGR2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_12U_BayerBG_Packed:
        {
          HRESULT const hr = Shrink12BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerBG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerRG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerGR:
        {
          HRESULT const hr = Shrink16BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGR;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGB2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerRG:
        {
          HRESULT const hr = Shrink16BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerRG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerBG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerGB:
        {
          HRESULT const hr = Shrink16BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGB;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGR2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerBG:
        {
          HRESULT const hr = Shrink16BitTo8Bit(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerBG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerRG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerGR_BigEndian:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGR;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGB2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerRG_BigEndian:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerRG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerBG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerGB_BigEndian:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerGB;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerGR2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BayerBG_BigEndian:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BayerBG;
              ptr->data_stride = ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC1, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_BayerRG2BGR);
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_RGB:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_RGB2BGRA);
        }
        break;

      case IDT_8U_RGBA:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC4, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_RGBA2BGRA);
        }
        break;

      case IDT_8U_BGR:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_16U_BGR:
        {
          HRESULT const hr = Shrink16BitTo8BitBigEndian(3*ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, ptr->data_width, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) )
            {
              ptr->data_type = IDT_8U_BGR;
              ptr->data_stride = 3*ptr->data_width;
            }
          /* if */

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_BGRA:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC4, (void *)(ptr->pData), (int)(ptr->data_stride));
          msrc.copyTo( *(ptr->pImageBGRA) );
        }
        break;

      case IDT_8U_YUV411:
        {
          ptr->pImageBGR->create((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3);

          HRESULT const hr = ConvertYUV411ToBGR8(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, (unsigned int)(ptr->pImageBGR->step[0]), ptr->pImageBGR->data);
          assert( SUCCEEDED(hr) );

          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_YUV422:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC2, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGRA), CV_YUV2BGRA_UYVY);
        }
        break;

      case IDT_8U_YUV422_BT601:
        {
          ptr->pImageBGR->create((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3);

          HRESULT const hr = ConvertYUV422BT601ToBGR8(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, (unsigned int)(ptr->pImageBGR->step[0]), ptr->pImageBGR->data);
          assert( SUCCEEDED(hr) );

          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_YUV422_BT709:
        {
          ptr->pImageBGR->create((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3);

          HRESULT const hr = ConvertYUV422BT709ToBGR8(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData, (unsigned int)(ptr->pImageBGR->step[0]), ptr->pImageBGR->data);
          assert( SUCCEEDED(hr) );

          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_YUV444:
        {
          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_YUV2RGB); // We use RGB as YUV conversion is implemented incorrectly (Bug #4227).
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;

      case IDT_8U_UYV444:
        {
          HRESULT const hr = SwapUYV8ToYUV8InPlace(ptr->data_width, ptr->data_height, ptr->data_stride, ptr->pData);
          assert( SUCCEEDED(hr) );
          if ( SUCCEEDED(hr) ) ptr->data_type = IDT_8U_YUV444;

          cv::Mat msrc((int)(ptr->data_height), (int)(ptr->data_width), CV_8UC3, (void *)(ptr->pData), (int)(ptr->data_stride));
          cv::cvtColor(msrc, *(ptr->pImageBGR), CV_YUV2RGB); // We use RGB as YUV conversion is implemented incorrectly (Bug #4227).
          cv::cvtColor(*(ptr->pImageBGR), *(ptr->pImageBGRA), CV_BGR2BGRA);
        }
        break;
      }
    /* switch */

    ptr->fProcessingImage = false;
    ptr->fRenderAgain = true;
  }
  LeaveCriticalSection( &(ptr->csData) );
}
/* DecodePushedImage_inline */



//! Render pushed image.
/*!
  Function renders pushed image.

  \param ptr    Pointer to preview window parameters.
  \return Returns S_OK if successfull.
*/
HRESULT
RenderPushedImage_inline(
                         PreviewWindowParameters * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_INVALIDARG;

  assert(NULL != ptr->pImageBGRA);
  if (NULL == ptr->pImageBGRA) return E_INVALIDARG;

  if (true == ptr->fModeChange) return E_ACCESSDENIED;

  HRESULT hr = S_OK;

  ID2D1Bitmap * pBitmap = NULL;

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
    if (NULL != ptr->pSwapChain)
      {
        if (NULL == ptr->pRenderTarget)
          {
            hr = RecreateDirect2DRenderTarget(ptr);
            assert( SUCCEEDED(hr) );
          }
        /* if (NULL == ptr->pRenderTarget) */

        bool const have_all_elements = (NULL != ptr->pRenderTarget);
        if ( true == have_all_elements )
          {
            bool render_bitmap = true;
            if ( (0 < ptr->QPC_last_push) && (0 < ptr->QPC_max_present_interval) )
              {
                LARGE_INTEGER QPC_at_render;

                BOOL const qpc_query = QueryPerformanceCounter( &QPC_at_render );
                assert(TRUE == qpc_query);
                if (TRUE == qpc_query)
                  {
                    render_bitmap = (QPC_at_render.QuadPart <= ptr->QPC_last_push + ptr->QPC_max_present_interval);
                  }
                /* if */
              }
            /* if */

            // Create Direct 2D bitmap.
            if ( SUCCEEDED(hr) && (true == render_bitmap) )
              {
                BOOL const entered = TryEnterCriticalSection( &(ptr->csData) );
                if (TRUE == entered)
                  {
                    D2D1_SIZE_U size = D2D1::SizeU((unsigned int)(ptr->pImageBGRA->cols), (unsigned int)(ptr->pImageBGRA->rows));

                    D2D1_BITMAP_PROPERTIES properties;
                    ZeroMemory( &properties, sizeof(properties) );
                    properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
                    properties.dpiX = 96.0f;
                    properties.dpiY = 96.0f;

                    hr = ptr->pRenderTarget->CreateBitmap(
                                                          size,
                                                          ptr->pImageBGRA->data,
                                                          (unsigned int)( ptr->pImageBGRA->step[0] ),
                                                          properties,
                                                          &pBitmap
                                                          );
                    assert( SUCCEEDED(hr) );
                    assert( NULL != pBitmap );

                    LeaveCriticalSection( &(ptr->csData) );
                  }
                else
                  {
                    hr = E_ACCESSDENIED;
                  }
                /* if */
              }
            /* if ( SUCCEEDED(hr) ) */

            // Render bitmap to the rendering surface.
            if ( SUCCEEDED(hr) )
              {
                D2D1_SIZE_F renderTargetSize = ptr->pRenderTarget->GetSize();
                D2D1_SIZE_F bitmapSize = (true == render_bitmap)? pBitmap->GetSize() : renderTargetSize;

                // Scale destination rectangle to achieve best fit-to-screen effect preserving aspect ratio.
                D2D1_RECT_F destinationRectangleImage = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);
                if (true == render_bitmap)
                  {
                    assert(NULL != pBitmap);

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
                  }
                /* if */

                ptr->pRenderTarget->BeginDraw();

                ptr->pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Lime));
                if (true == render_bitmap)
                  {
                    assert(NULL != pBitmap);
                    ptr->pRenderTarget->SetTransform(GetRenderTargetTransform_inline(ptr));
                    ptr->pRenderTarget->DrawBitmap(
                                                   pBitmap,
                                                   destinationRectangleImage,
                                                   1.0,
                                                   D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                                                   D2D1::RectF(0, 0, bitmapSize.width, bitmapSize.height)
                                                   );
                  }
                /* if */

                hr = ptr->pRenderTarget->EndDraw();
                assert( SUCCEEDED(hr) );
              }
            /* if ( SUCCEEDED(hr) ) */
          }
        else // !( true == have_all_elements )
          {
            hr = E_FAIL;
          }
        /* if ( true == have_all_elements ) */

        SAFE_RELEASE(pBitmap);

        ptr->fRenderAgain = !SUCCEEDED(hr);

        if ( D2DERR_RECREATE_TARGET == hr )
          {
            Debugfwprintf(stderr, gDbgRecreatingRenderTarget);

            hr = RecreateDirect2DRenderTarget(ptr);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
    /* if (NULL != ptr->pSwapChain) */
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  return hr;
}
/* RenderPushedImage_inline */



//! Presents rendered image.
/*!
  Presents last rendered buffer from the swap chain.

  \param ptr    Pointer to preview window data.
  \return Returns S_OK if successfull.
*/
HRESULT
inline
PresentPushedImage_inline(
                          PreviewWindowParameters * const ptr
                          )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return E_INVALIDARG;

  assert(NULL != ptr->pSwapChain);
  if (NULL == ptr->pSwapChain) return E_INVALIDARG;

  BOOL const entered = TryEnterCriticalSection( &(ptr->csRenderAndPresent) );
  if (FALSE == entered)
    {
      return E_ACCESSDENIED;
    }
  /* if */

  HRESULT hr = S_OK;

  if ( SUCCEEDED(hr) )
    {
      hr = ptr->pSwapChain->Present(0, 0); // Present immediately!
      assert( SUCCEEDED(hr) );
    }
  /* if */

  assert(TRUE == entered);

  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  return hr;
}
/* PresentPushedImage_inline */



/****** MOUSE HANDLERS ******/

//! Converts PX to DIPs.
/*!
  Converts screen coordinates in pixels to Direct2D coordinates in DIPs.

  \param ptr  Pointer to preview window parameters.
  \param xPos   X position in screen pixels.
  \param yPos   Y position in screen pixels.
  \return Returns position in device independent pixels.
*/
inline
D2D1_POINT_2F
PixelToDIP_inline(
                  PreviewWindowParameters * const ptr,
                  int const xPos,
                  int const yPos
                  )
{
  assert(NULL != ptr);
  if (NULL != ptr)
    {
      return D2D1::Point2F(static_cast<float>(xPos) * ptr->scaleX, static_cast<float>(yPos) * ptr->scaleY);
    }
  /* if */
  return D2D1::Point2F(static_cast<float>(xPos), static_cast<float>(yPos));
}
/* PixelToDIP_inline */



//! Mouse left button down handler.
/*!
  Handles left button press. If the mouse is inside a valid area
  then try to capture it and initialize panning.

  \param ptr    Pointer to preview window data.
  \param xPos   X position of the mouse in screen pixels.
  \param yPos   Y position of the mouse in screen pixels.
  \return Returns true if the event is consumed.
*/
inline
bool
OnLButtonDown_inline(
                     PreviewWindowParameters * const ptr,
                     int const xPos,
                     int const yPos
                     )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return false;

  ptr->ptMouse = PixelToDIP_inline(ptr, xPos, yPos);
  HWND const hWnd = SetCapture(ptr->hWnd);
  assert(GetCapture() == ptr->hWnd);

  return true;
}
/* OnLButtonDown_inline */



//! Mouse move handler.
/*!
  Handles move event. If the mouse is captured then the move event is
  translated into image pan.

  \param ptr    Pointer to preview window data.
  \param xPos   X position of the mouse in screen pixels.
  \param yPos   Y position of the mouse in screen pixels.
  \param flags  Additional message information as received by message procedure.
  \return Returns true if the event is consumed.
*/
inline
bool
OnMouseMove_inline(
                   PreviewWindowParameters * const ptr,
                   int const xPos,
                   int const yPos,
                   DWORD const flags
                   )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return false;

  if ( (flags & MK_LBUTTON) && (GetCapture() == ptr->hWnd) )
    {
      D2D1_POINT_2F ptMouseMove = PixelToDIP_inline(ptr, xPos, yPos);
      FLOAT const dx = ptMouseMove.x - ptr->ptMouse.x;
      FLOAT const dy = ptMouseMove.y - ptr->ptMouse.y;
      ptr->ptMouse = ptMouseMove;

      HRESULT const hr = AddToRenderTargetTransform_inline(ptr, D2D1::Matrix3x2F::Translation(dx, dy));
      assert( SUCCEEDED(hr) );

      return true;
    }
  /* if */

  return false;
}
/* OnMouseMove_inline */



//! Mouse left buttun up handler.
/*!
  Handles left button depress event. If the mouse is captured
  then it is released.

  \param ptr    Pointer to preview window data.
  \param xPos   X position of the mouse in screen pixels.
  \param yPos   Y position of the mouse in screen pixels.
  \return Returns true if the event is consumed.
*/
inline
bool
OnLButtonUp_inline(
                   PreviewWindowParameters * const ptr,
                   int const xPos,
                   int const yPos
                   )
{
  assert(NULL != ptr);
  if (NULL == ptr) return false;

  if (GetCapture() == ptr->hWnd)
    {
      D2D1_POINT_2F ptMouseUp = PixelToDIP_inline(ptr, xPos, yPos);

      FLOAT const dx = ptMouseUp.x - ptr->ptMouse.x;
      FLOAT const dy = ptMouseUp.y - ptr->ptMouse.y;
      ptr->ptMouse = ptMouseUp;

      HRESULT const hr = AddToRenderTargetTransform_inline(ptr, D2D1::Matrix3x2F::Translation(dx, dy));
      assert( SUCCEEDED(hr) );

      BOOL const released = ReleaseCapture();
      assert(TRUE == released);

      return true;
    }
  /* if */

  return false;
}
/* OnLButtonUp_inline */



//! One mouse wheel handler.
/*!
  Handles the mouse wheel event.

  \param ptr    Pointer to preview window data.
  \param xPos   X position of the mouse in screen pixels.
  \param yPos   Y position of the mouse in screen pixels.
  \param zDelta Wheel delta.
  \return Returns true if the event is consumed.
*/
inline
bool
OnMouseWheel_inline(
                    PreviewWindowParameters * const ptr,
                    int const xPos,
                    int const yPos,
                    short const zDelta
                    )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return false;

  POINT ptMouseInPixels;
  ptMouseInPixels.x = xPos;
  ptMouseInPixels.y = yPos;

  BOOL const convert = ScreenToClient(ptr->hWnd, &ptMouseInPixels);
  assert(TRUE == convert);

  D2D1_POINT_2F ptMouse = PixelToDIP_inline(ptr, ptMouseInPixels.x, ptMouseInPixels.y);
  if (0 < zDelta)
    {
      HRESULT const hr = AddToRenderTargetTransform_inline(ptr, D2D1::Matrix3x2F::Scale(1.05f, 1.05f, ptMouse));
      assert( SUCCEEDED(hr) );
    }
  else
    {
      HRESULT const hr = AddToRenderTargetTransform_inline(ptr, D2D1::Matrix3x2F::Scale(0.95f, 0.95f, ptMouse));
      assert( SUCCEEDED(hr) );
    }
  /* if */

  return false;
}
/* OnMouseWheel_inline */





/****** DIRECT 2D/3D  ******/

//! Releases swap chain and Direct 3D device.
/*!
  Releases allocated resources.

  \param ptr    Pointer to preview window data.
*/
void
DeleteDirectXDeviceAndSwapChain(
                                PreviewWindowParameters * const ptr
                                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->fModeChange = true; // Flag may only be reset if swap chain is recreated.

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {
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

    SAFE_RELEASE(ptr->pDeviceContext);
    SAFE_RELEASE(ptr->pDevice);
    SAFE_RELEASE(ptr->pOutput);
    SAFE_RELEASE(ptr->pAdapter);
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );
}
/* DeleteDirectXDeviceAndSwapChain */




//! Recreate Direct 3D device and swap chain.
/*!
  Function checks if swap chain exists and if output device changed. If any
  of those conditions are met the swap chain is recreated.

  \param ptr    Pointer to preview window structure.
  \return Function returns S_OK if succesffull.
*/
HRESULT
RecreateDirectXDeviceAndSwapChain(
                                  PreviewWindowParameters * const ptr
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

  EnterCriticalSection( &(ptr->csRenderAndPresent) );
  {

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
        LeaveCriticalSection( &(ptr->csRenderAndPresent) );
        ptr->fModeChange = fModeChange;
        return S_OK;
      }
    /* if */

    /* If output device changed then recreate the swap chain. */
    if ( true == recreate_swap_chain )
      {
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
                             NULL,
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

            hr = ptr->pSwapChain->GetDesc( &(ptr->sSwapChainDesc) );
            assert( SUCCEEDED(hr) );
          }
        /* if */

        /* Update scaling factors. */
        {
          FLOAT dpiX = 0.0f;  FLOAT dpiY = 0.0f;
          ptr->pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
          ptr->scaleX = 96.0f / dpiX;
          ptr->scaleY = 96.0f / dpiY;
        }

        /* Create new context and render target. */
        if ( SUCCEEDED(hr) )
          {
            hr = RecreateDirect2DRenderTarget(ptr);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
    /* if */

  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* RecreateDirectXDeviceAndSwapChain */



//! Recreate Direct2D render target.
/*!
  Function recreates Direct2D render target.

  \param ptr    Pointer to display window structure.
  \return Function returns S_OK if successfull and error code otherwise.
*/
HRESULT
RecreateDirect2DRenderTarget(
                             PreviewWindowParameters * const ptr
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
    SAFE_RELEASE( ptr->pBackBuffer );
    SAFE_RELEASE( ptr->pRenderTarget );

    /* Create new context and render target. */
    assert(NULL == ptr->pBackBuffer);
    assert(NULL == ptr->pRenderTarget);
    if ( SUCCEEDED(hr) )
      {
        hr = RenderTargetCreate(
                                ptr->pD2DFactory,
                                ptr->pSwapChain,
                                &( ptr->pBackBuffer ),
                                &( ptr->pRenderTarget ),
                                NULL,
                                NULL
                                );
        assert( SUCCEEDED(hr) );
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  return hr;
}
/* RecreateDirect2DRenderTarget */



//! Creates Direct 3D device and swap chain.
/*!
  Function creates Direct 3D device and swap chain and associates it with
  the preview window. Current code is for Windows 7 and above and DirectX 10 and above.

  \param ptr    Pointer to preview window data.
  \param pDXGIFactory1   Pointer to DXGI factory.
  \param pD2DFactory   Pointer to Direct 2D factory.
  \return Returns S_OK if successfull.
*/
HRESULT
CreateDirectXDeviceAndSwapChain(
                                PreviewWindowParameters * const ptr,
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
    // Create swap chain.
    if ( SUCCEEDED(hr) )
      {
        hr = RecreateDirectXDeviceAndSwapChain(ptr);
        assert( SUCCEEDED(hr) );
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  // Set window data for callback routine.
  SetWindowData(ptr, ptr->hWnd);

  ptr->fModeChange = false;

  return hr;
}
/* CreateDirectXDeviceAndSwapChain */



//! Resizes swap chain.
/*!
  Function resizes swap chain.

  \param ptr    Pointer to preview window data structure.
  \param width_in  New width of the swap chain.
  \param height_in New height of the swap chain.
  \return Returns S_OK if successfull.
*/
HRESULT
ResizeSwapChain(
                PreviewWindowParameters * const ptr,
                UINT const width_in,
                UINT const height_in
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
    UINT BufferCount = ptr->sSwapChainDesc.BufferCount;
    UINT width = width_in;
    UINT height = height_in;
    DXGI_FORMAT format = ptr->sSwapChainDesc.BufferDesc.Format;
    UINT Flags = ptr->sSwapChainDesc.Flags;

    if ( SUCCEEDED(hr) )
      {
        /* DXGI swap chain cannot be resized if its back buffer is referenced. We therefore have
           to release Direct2D render target and associated buffers.
        */
        SAFE_RELEASE(ptr->pRenderTarget);
        SAFE_RELEASE(ptr->pBackBuffer);

        hr = ptr->pSwapChain->ResizeBuffers(BufferCount, width, height, format, Flags);
        assert( SUCCEEDED(hr) );

        if (hr == DXGI_ERROR_DEVICE_REMOVED)
          {
            hr = RecreateDirectXDeviceAndSwapChain(ptr);
            assert( SUCCEEDED(hr) );
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(ptr->csRenderAndPresent) );

  ptr->fModeChange = fModeChange;

  return hr;
}
/* ResizeSwapChain */



/****** WINDOW THREAD AND MESSAGE PUMP ******/

//! Message handler.
/*!
  Processes messages for preview window.

  \param hWnd   A handle to the window procedure that received the message.
  \param message        Received message.
  \param wParam  Additional message information. The content of this parameter depends on the value of the Msg parameter.
  \param lParam Additional message information. The content of this parameter depends on the value of the Msg parameter.
  \return The return value is the result of the message processing and depends on the message (mostly 0 if successfull).
*/
LRESULT
CALLBACK
WndProcPreview(
               HWND hWnd,
               UINT message,
               WPARAM wParam,
               LPARAM lParam
               )
{
  PreviewWindowParameters * const ptr = (PreviewWindowParameters *)GetWindowData(hWnd);

#ifdef _DEBUG
  {
    if (NULL != ptr) AddMessage(ptr->pMsg, message, wParam, lParam);
  }
#endif /* _DEBUG */

  switch (message)
    {

    case WM_COMMAND:
      {
        /* Commandy may be mapped to key combinations. This is done through the accelerator table that
           is defined in PreviewWindowThread function. Key combination is mapped to a number that
           must be unique. To faciliate this all used nambers are defined as constants in
           header file BatchAcquisitionWindowPreview.h.
        */
        int wmId    = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);
        switch (wmId)
          {
          case PREVIEW_WINDOW_EXIT:
            {
              CameraControlDialogClose_inline(ptr);
              BOOL const res = DestroyWindow(hWnd);
              if (TRUE == res) return 0;
            }
            break;

          case PREVIEW_WINDOW_CCD_TOGGLE:
            {
              CameraControlDialogToggle_inline(ptr);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CCD_OPEN:
            {
              CameraControlDialogOpen_inline(ptr);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CCD_CLOSE:
            {
              CameraControlDialogClose_inline(ptr);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CCD_CLOSE_ALL:
            {
              // Not yet implemented.
              return 0;
            }
            break;

          case PREVIEW_WINDOW_IMAGE_PUSHED:
            {
              DecodePushedImage_inline(ptr);
              HRESULT const render = RenderPushedImage_inline(ptr);
              if ( SUCCEEDED(render) )
                {
                  HRESULT const present = PresentPushedImage_inline(ptr);
                  assert( SUCCEEDED(present) );
                }
              /* if */
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_0:
            {
              SelectActiveCamera_inline(ptr, 0);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_1:
            {
              SelectActiveCamera_inline(ptr, 1);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_2:
            {
              SelectActiveCamera_inline(ptr, 2);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_3:
            {
              SelectActiveCamera_inline(ptr, 3);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_4:
            {
              SelectActiveCamera_inline(ptr, 4);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_5:
            {
              SelectActiveCamera_inline(ptr, 5);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_6:
            {
              SelectActiveCamera_inline(ptr, 6);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_7:
            {
              SelectActiveCamera_inline(ptr, 7);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CAMERA_8:
            {
              SelectActiveCamera_inline(ptr, 8);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_CLEAR_CAMERA:
            {
              ClearActiveCamera_inline(ptr);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_RESTORE_CAMERA:
            {
              SelectActiveCamera_inline(ptr, ptr->CameraID);
              return 0;
            }
            break;

          case PREVIEW_WINDOW_RESET_IMAGE_TRANSFORM:
            {
              HRESULT const hr = ResetRenderTargetTransform_inline(ptr);
              assert( SUCCEEDED(hr) );
              if ( SUCCEEDED(hr) ) return 0;
            }
            break;

          case PREVIEW_WINDOW_UPDATE_TITLE:
            {
              SetWindowTitle_inline(ptr, ptr->CameraID);
              return 0;
            }
            break;

          default:
            return DefWindowProc(hWnd, message, wParam, lParam);
          }
        /* switch */
      }
      break;

    case WM_LBUTTONDOWN:
      {
        int const xPos = GET_X_LPARAM(lParam);
        int const yPos = GET_Y_LPARAM(lParam);
        bool const consumed = OnLButtonDown_inline(ptr, xPos, yPos);
        if (false == consumed) return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
      }
      break;

    case WM_MOUSEMOVE:
      {
        int const xPos = GET_X_LPARAM(lParam);
        int const yPos = GET_Y_LPARAM(lParam);
        bool const consumed = OnMouseMove_inline(ptr, xPos, yPos, (DWORD)wParam);
        if (false == consumed) return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
      }
      break;

    case WM_LBUTTONDBLCLK:
      {
        HRESULT const hr = ResetRenderTargetTransform_inline(ptr);
        assert( SUCCEEDED(hr) );
        if ( !SUCCEEDED(hr) ) return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
      }
      /* break */

    case WM_LBUTTONUP:
      {
        int const xPos = GET_X_LPARAM(lParam);
        int const yPos = GET_Y_LPARAM(lParam);
        bool const consumed = OnLButtonUp_inline(ptr, xPos, yPos);
        if (false == consumed) return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
      }
      break;

    case WM_MOUSEWHEEL:
      {
        int const fwKeys = GET_KEYSTATE_WPARAM(wParam);
        short const zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int const xPos = GET_X_LPARAM(lParam);
        int const yPos = GET_Y_LPARAM(lParam);
        bool const consumed = OnMouseWheel_inline(ptr, xPos, yPos, zDelta);
        if (false == consumed) return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
      }
      break;

    case WM_SIZE:
      {
        /* Per MSDN article about DXGI when WM_SIZE is called swap chain
           buffers should be resized to match the window size. If resizing
           the buffers fails then default to DefWindowProc.
        */
        UINT const width = LOWORD(lParam);
        UINT const height = HIWORD(lParam);
        if (NULL != ptr) ptr->fRenderAgain = true;
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

    case WM_PAINT:
      {
        if (NULL != ptr) ptr->fRenderAgain = true;
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;

    case WM_DESTROY:
      {
        CameraControlDialogClose_inline(ptr);
        BOOL const res = DestroyWindow(hWnd);
      }
      PostQuitMessage(0);
      return 0;
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }
  /* switch */

  // Normal return value is zero when the message was processed so we return 1 if message was not processed.
  return 1;
}
/* WndProcPreview */



//! Window thread.
/*!
  Creates empty preview window and runs the message pump.

  \param parameters_in Pointer to a structure holding window data.
  \return Function returns EXIT_SUCCESS if successfull, and EXIT_FAILURE if unsuccessfull.
*/
unsigned int
__stdcall
PreviewWindowThread(
                    void * parameters_in
                    )
{
  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  PreviewWindowParameters * const parameters = (PreviewWindowParameters *)parameters_in;

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
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProcPreview;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = parameters->hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 255, 0));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = parameters->szWindowClass;
    wcex.hIconSm = NULL;

    RegisterClassEx(&wcex);
  }

  // Initialize the preview window.
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
  ACCEL AccelTable[13];

  AccelTable[0].fVirt = 0;
  AccelTable[0].key = '1';
  AccelTable[0].cmd = PREVIEW_WINDOW_CAMERA_0;

  AccelTable[1].fVirt = 0;
  AccelTable[1].key = '2';
  AccelTable[1].cmd = PREVIEW_WINDOW_CAMERA_1;

  AccelTable[2].fVirt = 0;
  AccelTable[2].key = '3';
  AccelTable[2].cmd = PREVIEW_WINDOW_CAMERA_2;

  AccelTable[3].fVirt = 0;
  AccelTable[3].key = '4';
  AccelTable[3].cmd = PREVIEW_WINDOW_CAMERA_3;

  AccelTable[4].fVirt = 0;
  AccelTable[4].key = '5';
  AccelTable[4].cmd = PREVIEW_WINDOW_CAMERA_4;

  AccelTable[5].fVirt = 0;
  AccelTable[5].key = '6';
  AccelTable[5].cmd = PREVIEW_WINDOW_CAMERA_5;

  AccelTable[6].fVirt = 0;
  AccelTable[6].key = '7';
  AccelTable[6].cmd = PREVIEW_WINDOW_CAMERA_6;

  AccelTable[7].fVirt = 0;
  AccelTable[7].key = '8';
  AccelTable[7].cmd = PREVIEW_WINDOW_CAMERA_7;

  AccelTable[8].fVirt = 0;
  AccelTable[8].key = '9';
  AccelTable[8].cmd = PREVIEW_WINDOW_CAMERA_8;

  AccelTable[9].fVirt = 0;
  AccelTable[9].key = 'c';
  AccelTable[9].cmd = PREVIEW_WINDOW_CCD_TOGGLE;

  AccelTable[10].fVirt = 0;
  AccelTable[10].key = 'C';
  AccelTable[10].cmd = PREVIEW_WINDOW_CCD_TOGGLE;

  AccelTable[11].fVirt = 0;
  AccelTable[11].key = 'i';
  AccelTable[11].cmd = PREVIEW_WINDOW_RESET_IMAGE_TRANSFORM;

  AccelTable[12].fVirt = 0;
  AccelTable[12].key = 'I';
  AccelTable[12].cmd = PREVIEW_WINDOW_RESET_IMAGE_TRANSFORM;

  HACCEL hAccelTable = CreateAcceleratorTable(AccelTable, 13);

  // Raise thread active flag.
  assert(false == parameters->fActive);
  parameters->fActive = true;

  // Main message loop.
  MSG msg_peek;
  MSG msg_get;
  BOOL done = FALSE;
  while (FALSE == done)
    {
      if ( PeekMessage(&msg_peek, NULL, 0, 0, PM_NOREMOVE) )
        {
          BOOL const bRet = GetMessage(&msg_get, NULL, 0, 0);

          if (WM_QUIT == msg_get.message) done = TRUE;

          if (-1 == bRet)
            {
              done = TRUE;
            }
          else if ( !TranslateAccelerator(msg_get.hwnd, hAccelTable, &msg_get) )
            {
              BOOL const tm = TranslateMessage( &msg_get );
              LRESULT const dm = DispatchMessage( &msg_get );
            }
          /* if */
        }
      /* if */

      if ( (0 < parameters->QPC_last_push) && (0 < parameters->QPC_max_present_interval) )
        {
          LARGE_INTEGER QPC_at_message;

          BOOL const qpc_query = QueryPerformanceCounter( &QPC_at_message );
          assert(TRUE == qpc_query);
          if (TRUE == qpc_query)
            {
              parameters->fRenderAgain = (QPC_at_message.QuadPart > parameters->QPC_last_push + parameters->QPC_max_present_interval);
            }
          /* if */
        }
      /* if */

      if (true == parameters->fRenderAgain)
        {
          HRESULT const render = RenderPushedImage_inline(parameters);
          if ( SUCCEEDED(render) )
            {
              HRESULT const present = PresentPushedImage_inline(parameters);
              assert( SUCCEEDED(present) );
            }
          /* if */
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
/* PreviewWindowThread */



/****** UPDATES ******/

//! Pushes image to display.
/*!
  Pushes image to preview thread for display.
  This function creates a deep copy of the acquired data and then pushes
  display message into the thread queue. Image data will be decoded and
  displayed once the message is processed. While the message is not processed
  any subsequently pushed images will be ignored.

  \param ptr Pointer to preview window parameters.
  \param CameraID ID of the active camera.
  \param width  Width of the image.
  \param height Height of the image.
  \param stride Row stride (size of one image row in bytes).
  \param type   Image data type.
  \param data   Pointer to image data.
  \return Function returns true if image is successfully pushed to the preview window.
*/
void
PushImage(
          PreviewWindowParameters * const ptr,
          int const CameraID,
          unsigned int const width,
          unsigned int const height,
          unsigned int const stride,
          ImageDataType const type,
          void const * const data
          )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  assert(NULL != data);
  if (NULL == data) return;

  if (CameraID != ptr->CameraID) return;

  // Lock access to image data.
  BOOL const entered = TryEnterCriticalSection( &(ptr->csData) );
  if (FALSE == entered)
    {
      // Return immediately if last pushed image is in decoding stage.
      if (true == ptr->fProcessingImage) return;

      // Return if the window is not active.
      if (false == ptr->fActive) return;

      EnterCriticalSection( &(ptr->csData) );
    }
  /* if */

  assert(true == ptr->fActive);
  ptr->fProcessingImage = true;

  // Reallocate memory if needed.
  unsigned int size = height * stride;
  if ( (NULL == ptr->pData) || (ptr->data_size < size) )
    {
      SAFE_FREE(ptr->pData);
      ptr->data_size = 0;

      assert(NULL == ptr->pData);
      ptr->pData = malloc(size);
      assert(NULL != ptr->pData);

      if (NULL != ptr->pData) ptr->data_size = size;
    }
  /* if */

  // Copy data. Any decoding is postponed and will be done by the display window thread.
  if (NULL != ptr->pData)
    {
      ptr->data_type = type;
      ptr->data_height = height;
      ptr->data_width = width;
      ptr->data_stride = stride;

      void * const dst = memcpy(ptr->pData, data, size);
      assert(dst == ptr->pData);
    }
  /* if */

  {
    LARGE_INTEGER QPC_at_push;

    BOOL const qpc_query = QueryPerformanceCounter( &QPC_at_push );
    assert(TRUE == qpc_query);
    if (TRUE == qpc_query) ptr->QPC_last_push = QPC_at_push.QuadPart;
  }

  assert(true == ptr->fProcessingImage);

  LeaveCriticalSection( &(ptr->csData) );


  // Push message to display window thread.
  BOOL const sent =
    PostMessage(
                ptr->hWnd,
                WM_COMMAND, // Message.
                MAKEWPARAM(PREVIEW_WINDOW_IMAGE_PUSHED, 0),
                MAKELPARAM(0, 0)
                );
  //assert(TRUE == sent);
}
/* PushImage */



#ifdef HAVE_SAPERA_SDK

//! Pushes image to display.
/*!
  Pushes image to preview thread.

  \param ptr    Pointer to preview thread data.
  \param CameraID ID of the active camera.
  \param pImage Pointer to image data.
  \param pCamera Pointer to camera class.
*/
void
PushImage(
          PreviewWindowParameters * const ptr,
          int const CameraID,
          SapBuffer * const pImage,
          SapAcqDevice * const pCamera
          )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (CameraID != ptr->CameraID) return;

  if (true == ptr->fProcessingImage) return;
  if (false == ptr->fActive) return;

  assert(NULL != pImage);
  if (NULL == pImage) return;

  void * pData = NULL;
  BOOL const lock = pImage->GetAddress(&pData);
  assert(TRUE == lock);
  if (TRUE == lock)
    {
      unsigned int const width = pImage->GetWidth();
      unsigned int const height = pImage->GetHeight();
      unsigned int const stride = pImage->GetPitch();
      ImageDataType const type = GetImageDataType(pImage, pCamera);

      PushImage(ptr, CameraID, width, height, stride, type, pData);

      BOOL const unlock = pImage->ReleaseAddress(pData);
      assert(TRUE == unlock);
    }
  /* if */
}
/* PushImage */

#endif /* HAVE_SAPERA_SDK */



#ifdef HAVE_FLYCAPTURE2_SDK

//! Pushes image to display.
/*!
  Pushes image to preview thread.

  \param ptr    Pointer to preview thread data.
  \param CameraID ID of the active camera.
  \param pImage Pointer to image data.
  \param pCamera Pointer to camera. May be NULL.
*/
void
PushImage(
          PreviewWindowParameters * const ptr,
          int const CameraID,
          FlyCapture2::Image * const pImage,
          FlyCapture2::Camera * const pCamera
          )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  if (CameraID != ptr->CameraID) return;

  if (true == ptr->fProcessingImage) return;
  if (false == ptr->fActive) return;

  assert(NULL != pImage);
  if (NULL == pImage) return;

  void const * const pData = pImage->GetData();
  unsigned int const width = pImage->GetCols();
  unsigned int const height = pImage->GetRows();
  unsigned int const stride = pImage->GetStride();
  ImageDataType const type = GetImageDataType(pImage, pCamera);

  PushImage(ptr, CameraID, width, height, stride, type, pData);
}
/* PushImage */

#endif /* HAVE_FLYCAPTURE2_SDK */



/****** OPEN/CLOSE PREVIEW WINDOW ******/

//! Opens preview window and starts message pump.
/*!
  Opens empty preview window and spawns new thread that runs the message pump.

  \see ClosePreviewWindow

  \param hInstance   A handle to the current instance of the application.
  \param szTitle     Window title.
  \param szWindowClass  Window class.
  \param nCmdShow       Controls how the window is to be shown.
  \param hWndParent     Handle to parent window.
  \return Returns NULL if unsuccessfull and pointer to PreviewWindowParameters structure otherwise.
*/
PreviewWindowParameters *
OpenPreviewWindow(
                  HINSTANCE const hInstance,
                  TCHAR const * const szTitle,
                  TCHAR const * const szWindowClass,
                  int const nCmdShow,
                  HWND const hWndParent
                  )
{
  PreviewWindowParameters * const parameters = (PreviewWindowParameters *)malloc( sizeof(PreviewWindowParameters) );
  assert(NULL != parameters);
  if (NULL == parameters) return parameters;

  BlankPreviewWindowParameters_inline(parameters);

  /* Create critical sections. */
  InitializeCriticalSection( &(parameters->csRenderAndPresent) );
  InitializeCriticalSection( &(parameters->csCamera) );
  InitializeCriticalSection( &(parameters->csData) );
  InitializeCriticalSection( &(parameters->csTransform) );

  /* Create cv::Mat containes. */
  assert(NULL == parameters->pImageTMP);
  parameters->pImageTMP = new cv::Mat();
  assert(NULL != parameters->pImageTMP);

  assert(NULL == parameters->pImageBGR);
  parameters->pImageBGR = new cv::Mat();
  assert(NULL != parameters->pImageBGR);

  assert(NULL == parameters->pImageBGRA);
  parameters->pImageBGRA = new cv::Mat();
  assert(NULL != parameters->pImageBGRA);

  /* Copy supplied data. */
  parameters->hInstance = hInstance;

  if (NULL != szTitle)
    {
      int i = 0;
      for (; (i < MAX_LOADSTRING) && (szTitle[i]); ++i) parameters->szTitle[i] = szTitle[i];
      parameters->szTitle[i] = 0;
    }
  /* if */

  if (NULL != szWindowClass)
    {
      int i = 0;
      for (; (i < MAX_LOADSTRING) && (szWindowClass[i]); ++i) parameters->szWindowClass[i] = szWindowClass[i];
      parameters->szWindowClass[i] = 0;
    }
  /* if */

  parameters->nCmdShow = nCmdShow;

  parameters->hWndParent = hWndParent;

  /* Create message storage. */
  assert(NULL == parameters->pMsg);
  parameters->pMsg = PastMessagesCreate();
  assert(NULL != parameters->pMsg);

  /* Set frame present interval. */
  {
    LARGE_INTEGER frequency;
    BOOL const res = QueryPerformanceFrequency( &frequency );
    assert(FALSE != res);

    parameters->QPC_max_present_interval = 10 * frequency.QuadPart; // Set timeout to 10 seconds.
  }

  /* Spawn preview window thread. */
  parameters->tWindow =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             PreviewWindowThread,
                             (void *)( parameters ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != parameters->tWindow );
  if ( (HANDLE)( NULL ) == parameters->tWindow )
    {
      free(parameters);
      return NULL;
    }
  /* if */

  return parameters;
}
/* OpenPreviewWindow */



//! Close preview window.
/*!
  Closes preview window. After calling this function parameters structure
  will be deallocated and must not be used.

  \see OpenPreviewWindow

  \param parameters     Pointer to parameters structure return by OpenPreviewWindow.
*/
void
ClosePreviewWindow(
                   PreviewWindowParameters * const parameters
                   )
{
  assert(NULL != parameters);
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
                           MAKEWPARAM(PREVIEW_WINDOW_EXIT, 0),
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

  parameters->fProcessingImage = true;

  EnterCriticalSection( &(parameters->csRenderAndPresent) );
  {
    EnterCriticalSection( &(parameters->csTransform) );
    {
      EnterCriticalSection( &(parameters->csCamera) );
      {
        EnterCriticalSection( &(parameters->csData) );
        {
          DeleteDirectXDeviceAndSwapChain( parameters );

          SAFE_FREE(parameters->pData);
          SAFE_DELETE(parameters->pImageTMP);
          SAFE_DELETE(parameters->pImageBGR);
          SAFE_DELETE(parameters->pImageBGRA);
        }
        LeaveCriticalSection( &(parameters->csData) );
      }
      LeaveCriticalSection( &(parameters->csCamera) );
    }
    LeaveCriticalSection( &(parameters->csTransform) );
  }
  LeaveCriticalSection( &(parameters->csRenderAndPresent) );

  DeleteCriticalSection( &(parameters->csCamera) );
  DeleteCriticalSection( &(parameters->csRenderAndPresent) );
  DeleteCriticalSection( &(parameters->csData) );
  DeleteCriticalSection( &(parameters->csTransform) );

  PastMessagesDelete(parameters->pMsg);

  BlankPreviewWindowParameters_inline( parameters);

  free( parameters );
}
/* ClosePreviewWindow */



/****** AUXILIARY FUNCTIONS ******/

//! Connect to acquisition threads.
/*!
  Connects preview window to acquisition threads.

  \param parameters     Pointer to preview window thread parameters.
  \param pAcquisitions  Pointer to acquisition threads parameters.
  \param pAcquisitionsLock  Pointer to SRW lock to control access to acquisition threads parameters.
  \param CameraID    ID of the camera which is active.
*/
void
ConnectToAcquisitionThreads(
                            PreviewWindowParameters * const parameters,
                            std::vector<AcquisitionParameters_ *> * const pAcquisitions,
                            SRWLOCK * const pAcquisitionsLock,
                            int const CameraID
                            )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(NULL != pAcquisitions);
  if (NULL == pAcquisitions) return;

  assert(NULL != pAcquisitionsLock);
  if (NULL == pAcquisitionsLock) return;

  EnterCriticalSection( &(parameters->csCamera) );
  {
    assert(NULL == parameters->pAcquisitions);
    assert(false == parameters->fDialogShown);

    parameters->pAcquisitions = pAcquisitions;
    parameters->pAcquisitionsLock = pAcquisitionsLock;

    SelectActiveCamera_inline(parameters, CameraID);
  }
  LeaveCriticalSection( &(parameters->csCamera) );
}
/* ConnectToAcquisitionThread */



//! Disconnect from acquisition thread.
/*!
  Disconnects preview window from acquisition thread.

  \param parameters     Pointer to preview window thread parameters.
*/
void
DisconnectFromAcquisitionThreads(
                                 PreviewWindowParameters * const parameters
                                 )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  assert(NULL != parameters->pAcquisitions);
  assert(NULL != parameters->pAcquisitionsLock);

  if (true == parameters->fDialogShown)
    {
      // First try to close camera control dialog (if any).
      BOOL const post = PostMessage(parameters->hWnd, WM_COMMAND, PREVIEW_WINDOW_CCD_CLOSE, 0);
      assert(TRUE == post);

      // Wait for image decoder thread to become active.
      while (true == parameters->fDialogShown) SleepEx(10, TRUE);
    }
  /* if */

  EnterCriticalSection( &(parameters->csCamera) );
  {
    assert(false == parameters->fDialogShown);

    parameters->pAcquisitions = NULL;
    parameters->pAcquisitionsLock = NULL;
    parameters->CameraID = -1;
    parameters->pAcquisition = NULL;
  }
  LeaveCriticalSection( &(parameters->csCamera) );
}
/* DisconnectFromAcquisitionThread */



//! Clear active camera.
/*!
  Function sends message to camera preview window to indicate
  the currently active camera should be deactivated.

  \param parameters     Pointer to preview window thread parameters.
*/
void
ClearActiveCamera(
                  PreviewWindowParameters * const parameters
                  )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  DWORD const result = WaitForSingleObject(parameters->tWindow, 0);

  if ( (WAIT_OBJECT_0 != result) && (true == parameters->fActive) )
    {
      DWORD_PTR dwResult;
      LRESULT const sm =
        SendMessageTimeout(
                           parameters->hWnd,
                           WM_COMMAND, // Message.
                           MAKEWPARAM(PREVIEW_WINDOW_CLEAR_CAMERA, 0),
                           MAKELPARAM(0, 0),
                           SMTO_NOTIMEOUTIFNOTHUNG,
                           1000, // Timeout in ms.
                           &dwResult
                           );
      assert(0 != sm);
    }
  /* if */
}
/* ClearActiveCamera */



//! Restores active camera.
/*!
  Function send message to camera preview window which indicates the window
  may try to restore the active camera and resume preview.

  \param parameters     Pointer to preview window thread parameters.
*/
void
RestoreActiveCamera(
                    PreviewWindowParameters * const parameters
                    )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  DWORD const result = WaitForSingleObject(parameters->tWindow, 0);

  if ( (WAIT_OBJECT_0 != result) && (true == parameters->fActive) )
    {
      DWORD_PTR dwResult;
      LRESULT const sm =
        SendMessageTimeout(
                           parameters->hWnd,
                           WM_COMMAND, // Message.
                           MAKEWPARAM(PREVIEW_WINDOW_RESTORE_CAMERA, 0),
                           MAKELPARAM(0, 0),
                           SMTO_NOTIMEOUTIFNOTHUNG,
                           1000, // Timeout in ms.
                           &dwResult
                           );
      assert(0 != sm);
    }
  /* if */
}
/* RestoreActiveCamera */



//! Close camera configuration dialog.
/*!
  Closes camera configuration dialog if it is open.

  \param parameters     Pointer to preview window parameters.
*/
void
CloseCameraConfigurationDialog(
                               PreviewWindowParameters * const parameters
                               )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  // Message pump of the preview window must be active.
  assert(true == parameters->fActive);
  if (false == parameters->fActive) return;

  // Send message to window.
  BOOL const post = PostMessage(parameters->hWnd, WM_COMMAND, PREVIEW_WINDOW_CCD_CLOSE, 0);
  assert(TRUE == post);
}
/* CloseCameraConfigurationDialog */



//! Toggle camera configuration dialog.
/*!
  Toggles the camera configuration dialog.

  \param parameters     Pointer to preview window parameters.
*/
void
ToggleCameraConfigurationDialog(
                                PreviewWindowParameters * const parameters
                                )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  // Message pump of the preview window must be active.
  assert(true == parameters->fActive);
  if (false == parameters->fActive) return;

  // Send message to window.
  BOOL const post = PostMessage(parameters->hWnd, WM_COMMAND, PREVIEW_WINDOW_CCD_TOGGLE, 0);
  assert(TRUE == post);
}
/* ToggleCameraConfigurationDialog */



//! Update window title.
/*!
  Updates window title.

  \param parameters     Pointer to preview window parameters.
*/
void
PreviewWindowUpdateTitle(
                         PreviewWindowParameters * const parameters
                         )
{
  assert(NULL != parameters);
  if (NULL == parameters) return;

  // Message pump of the preview window must be active.
  assert(true == parameters->fActive);
  if (false == parameters->fActive) return;

  // Send message to window.
  BOOL const post = PostMessage(parameters->hWnd, WM_COMMAND, PREVIEW_WINDOW_UPDATE_TITLE, 0);
  assert(TRUE == post);
}
/* PreviewWindowUpdateTitle */



#endif /* !__BATCHACQUISITIONPREVIEWWINDOWPREVIEW_CPP */
