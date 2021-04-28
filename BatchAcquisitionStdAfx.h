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
  \file   BatchAcquisitionStdAfx.h
  \brief  Use this file to include precompiled headers.

  \author Tomislav Petkovic
  \date   2014-12-10
*/


#ifndef __BATCHACQUISITIONSTDAFX_H
#define __BATCHACQUISITIONSTDAFX_H

#pragma once

#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif /* _DEBUG */


#include <windows.h>

#include "BatchAcquisitionTargetVer.h"


/* Microsoft libraries. */

#pragma warning(push)
#pragma warning(disable: 4995)

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <process.h>
#include <math.h>

#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>
#include <list>
#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <numeric>

#include <DXGI.h>
#include <D2D1.h>
#include <D3D11.h>
#include <Dwrite.h>

#include <Wincodec.h>
#include <Mmsystem.h>
#include <Windowsx.h>
#include <Shobjidl.h>
#include <xmllite.h>

#pragma warning(pop)

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "D3D11.lib")

#pragma comment(lib, "Windowscodecs.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "xmllite.lib")


/* Teledyne DALSA Sapera LT SDK */
#define HAVE_SAPERA_SDK
#ifdef HAVE_SAPERA_SDK

#pragma warning(push)
#pragma warning(disable: 4005)

#include <sapclassbasic.h>
#include <corapi.h>

#pragma warning(pop)

#pragma comment(lib, "SapClassBasic.lib")
#pragma comment(lib, "corapi.lib")

#else

#include <strsafe.h>

#endif /* HAVE_SAPERA_SDK */


/* PointGrey FlyCapture2 SDK */
#define HAVE_FLYCAPTURE2_SDK
#ifdef HAVE_FLYCAPTURE2_SDK

#pragma warning(push)
#pragma warning(disable: 4005)

#include <FlyCapture2.h>

#pragma warning(pop)

#define USE_FLYCAPTURE2_GUI
#ifdef USE_FLYCAPTURE2_GUI
#include <FlyCapture2GUI.h>
#endif /* USE_FLYCAPTURE2_GUI */

#ifdef DEBUG

#if _MSC_VER == 1700
// Visual studio 2012, MSVC 11.0
#pragma comment(lib, "FlyCapture2d_v110.lib")
#pragma comment(lib, "FlyCapture2GUId_v110.lib")
#elif _MSC_VER == 1600
// Visual studio 2010, MSVC 10.0
#pragma comment(lib, "FlyCapture2d_v100.lib")
#pragma comment(lib, "FlyCapture2GUId_v100.lib")
#elif _MSC_VER == 1500
// Visual studio 2005, MSVC 9.0
#pragma comment(lib, "FlyCapture2d_v90.lib")
#pragma comment(lib, "FlyCapture2GUId_v90.lib")
#else
#pragma comment(lib, "FlyCapture2d.lib")
#pragma comment(lib, "FlyCapture2GUId.lib")
#endif

#else

#if _MSC_VER == 1700
// Visual studio 2012, MSVC 11.0
#pragma comment(lib, "FlyCapture2_v110.lib")
#pragma comment(lib, "FlyCapture2GUI_v110.lib")
#elif _MSC_VER == 1600
// Visual studio 2010, MSVC 10.0
#pragma comment(lib, "FlyCapture2_v100.lib")
#pragma comment(lib, "FlyCapture2GUI_v100.lib")
#elif _MSC_VER == 1500
// Visual studio 2005, MSVC 9.0
#pragma comment(lib, "FlyCapture2_v90.lib")
#pragma comment(lib, "FlyCapture2GUId_v90.lib")
#else
#pragma comment(lib, "FlyCapture2.lib")
#pragma comment(lib, "FlyCapture2GUI.lib")
#endif

#endif /* DEBUG */

#endif /* HAVE_FLYCAPTURE2_SDK */


/* OpenCV 3.0.0-RC1. */
#include <opencv2/opencv.hpp>

#ifdef DEBUG

#pragma comment(lib, "opencv_calib3d300d.lib")
#pragma comment(lib, "opencv_core300d.lib")
#pragma comment(lib, "opencv_features2d300d.lib")
#pragma comment(lib, "opencv_flann300d.lib")
#pragma comment(lib, "opencv_hal300d.lib")
#pragma comment(lib, "opencv_highgui300d.lib")
#pragma comment(lib, "opencv_imgcodecs300d.lib")
#pragma comment(lib, "opencv_imgproc300d.lib")
#pragma comment(lib, "opencv_ml300d.lib")
#pragma comment(lib, "opencv_objdetect300d.lib")
#pragma comment(lib, "opencv_photo300d.lib")
#pragma comment(lib, "opencv_shape300d.lib")
#pragma comment(lib, "opencv_stitching300d.lib")
#pragma comment(lib, "opencv_superres300d.lib")
#pragma comment(lib, "opencv_ts300d.lib")
#pragma comment(lib, "opencv_video300d.lib")
#pragma comment(lib, "opencv_videoio300d.lib")
#pragma comment(lib, "opencv_videostab300d.lib")

#else

#pragma comment(lib, "opencv_calib3d300.lib")
#pragma comment(lib, "opencv_core300.lib")
#pragma comment(lib, "opencv_features2d300.lib")
#pragma comment(lib, "opencv_flann300.lib")
#pragma comment(lib, "opencv_hal300.lib")
#pragma comment(lib, "opencv_highgui300.lib")
#pragma comment(lib, "opencv_imgcodecs300.lib")
#pragma comment(lib, "opencv_imgproc300.lib")
#pragma comment(lib, "opencv_ml300.lib")
#pragma comment(lib, "opencv_objdetect300.lib")
#pragma comment(lib, "opencv_photo300.lib")
#pragma comment(lib, "opencv_shape300.lib")
#pragma comment(lib, "opencv_stitching300.lib")
#pragma comment(lib, "opencv_superres300.lib")
#pragma comment(lib, "opencv_ts300.lib")
#pragma comment(lib, "opencv_video300.lib")
#pragma comment(lib, "opencv_videoio300.lib")
#pragma comment(lib, "opencv_videostab300.lib")

#endif /* _DEBUG */


/* VTK 6.2.0 */
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkContourFilter.h>
#include <vtkDataArray.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkInteractorStyleTrackballCamera.h> 
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkLine.h>
#include <vtkLineRepresentation.h>
#include <vtkLineWidget.h>
#include <vtkLineWidget2.h>
#include <vtkLookupTable.h>
#include <vtkMaskPoints.h>
#include <vtkOutlineFilter.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkQuad.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRenderer.h>
#include <vtkTexture.h>
#include <vtkTimeStamp.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkObjectFactory.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkSmartPointer.h>
#include <vtkVRMLExporter.h>
#include <vtkX3DExporter.h>
#include <vtkOBJExporter.h>
#include <vtkSTLWriter.h>

#pragma comment(lib, "vtkCommonColor-6.2.lib")
#pragma comment(lib, "vtkCommonComputationalGeometry-6.2.lib")
#pragma comment(lib, "vtkCommonCore-6.2.lib")
#pragma comment(lib, "vtkCommonDataModel-6.2.lib")
#pragma comment(lib, "vtkCommonExecutionModel-6.2.lib")
#pragma comment(lib, "vtkCommonMath-6.2.lib")
#pragma comment(lib, "vtkCommonMisc-6.2.lib")
#pragma comment(lib, "vtkCommonSystem-6.2.lib")
#pragma comment(lib, "vtkCommonTransforms-6.2.lib")
#pragma comment(lib, "vtkFiltersAMR-6.2.lib")
#pragma comment(lib, "vtkFiltersCore-6.2.lib")
#pragma comment(lib, "vtkFiltersExtraction-6.2.lib")
#pragma comment(lib, "vtkFiltersFlowPaths-6.2.lib")
#pragma comment(lib, "vtkFiltersGeneral-6.2.lib")
#pragma comment(lib, "vtkFiltersGeneric-6.2.lib")
#pragma comment(lib, "vtkFiltersGeometry-6.2.lib")
#pragma comment(lib, "vtkFiltersHybrid-6.2.lib")
#pragma comment(lib, "vtkFiltersHyperTree-6.2.lib")
#pragma comment(lib, "vtkFiltersImaging-6.2.lib")
#pragma comment(lib, "vtkFiltersModeling-6.2.lib")
#pragma comment(lib, "vtkFiltersParallel-6.2.lib")
#pragma comment(lib, "vtkFiltersParallelImaging-6.2.lib")
#pragma comment(lib, "vtkFiltersSelection-6.2.lib")
#pragma comment(lib, "vtkFiltersSMP-6.2.lib")
#pragma comment(lib, "vtkFiltersSources-6.2.lib")
#pragma comment(lib, "vtkFiltersStatistics-6.2.lib")
#pragma comment(lib, "vtkFiltersTexture-6.2.lib")
#pragma comment(lib, "vtkFiltersVerdict-6.2.lib")
#pragma comment(lib, "vtkImagingColor-6.2.lib")
#pragma comment(lib, "vtkImagingCore-6.2.lib")
#pragma comment(lib, "vtkImagingFourier-6.2.lib")
#pragma comment(lib, "vtkImagingGeneral-6.2.lib")
#pragma comment(lib, "vtkImagingHybrid-6.2.lib")
#pragma comment(lib, "vtkImagingMath-6.2.lib")
#pragma comment(lib, "vtkImagingMorphological-6.2.lib")
#pragma comment(lib, "vtkImagingSources-6.2.lib")
#pragma comment(lib, "vtkImagingStatistics-6.2.lib")
#pragma comment(lib, "vtkImagingStencil-6.2.lib")
#pragma comment(lib, "vtkInteractionImage-6.2.lib")
#pragma comment(lib, "vtkInteractionStyle-6.2.lib")
#pragma comment(lib, "vtkInteractionWidgets-6.2.lib")
#pragma comment(lib, "vtkIOAMR-6.2.lib")
#pragma comment(lib, "vtkIOCore-6.2.lib")
#pragma comment(lib, "vtkIOEnSight-6.2.lib")
#pragma comment(lib, "vtkIOExodus-6.2.lib")
#pragma comment(lib, "vtkIOExport-6.2.lib")
#pragma comment(lib, "vtkIOGeometry-6.2.lib")
#pragma comment(lib, "vtkIOImage-6.2.lib")
#pragma comment(lib, "vtkIOImport-6.2.lib")
#pragma comment(lib, "vtkIOInfovis-6.2.lib")
#pragma comment(lib, "vtkIOLegacy-6.2.lib")
#pragma comment(lib, "vtkIOLSDyna-6.2.lib")
#pragma comment(lib, "vtkIOMINC-6.2.lib")
#pragma comment(lib, "vtkIOMovie-6.2.lib")
#pragma comment(lib, "vtkIONetCDF-6.2.lib")
#pragma comment(lib, "vtkIOParallel-6.2.lib")
#pragma comment(lib, "vtkIOParallelXML-6.2.lib")
#pragma comment(lib, "vtkIOPLY-6.2.lib")
#pragma comment(lib, "vtkIOSQL-6.2.lib")
#pragma comment(lib, "vtkIOVideo-6.2.lib")
#pragma comment(lib, "vtkIOXML-6.2.lib")
#pragma comment(lib, "vtkIOXMLParser-6.2.lib")
#pragma comment(lib, "vtkParallelCore-6.2.lib")
#pragma comment(lib, "vtkRenderingAnnotation-6.2.lib")
#pragma comment(lib, "vtkRenderingContext2D-6.2.lib")
#pragma comment(lib, "vtkRenderingContextOpenGL-6.2.lib")
#pragma comment(lib, "vtkRenderingCore-6.2.lib")
#pragma comment(lib, "vtkRenderingFreeType-6.2.lib")
#pragma comment(lib, "vtkRenderingFreeTypeOpenGL-6.2.lib")
#pragma comment(lib, "vtkRenderingGL2PS-6.2.lib")
#pragma comment(lib, "vtkRenderingImage-6.2.lib")
#pragma comment(lib, "vtkRenderingLabel-6.2.lib")
#pragma comment(lib, "vtkRenderingLIC-6.2.lib")
#pragma comment(lib, "vtkRenderingLOD-6.2.lib")
#pragma comment(lib, "vtkRenderingOpenGL-6.2.lib")
#pragma comment(lib, "vtkRenderingVolume-6.2.lib")
#pragma comment(lib, "vtkRenderingVolumeOpenGL-6.2.lib")



#endif /* !__BATCHACQUISITIONSTDAFX_H */
