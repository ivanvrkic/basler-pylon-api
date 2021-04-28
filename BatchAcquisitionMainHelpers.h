/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 * 
 * (c) 2017 UniZG, Zagreb. All rights reserved.
 * (c) 2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionMainHelpers.h
  \brief  Helper functions for main control loop.

  \author Tomislav Petkovic
  \date   2017-09-06
*/


#ifndef __BATCHACQUISITIONMAINHELPERS_H
#define __BATCHACQUISITIONMAINHELPERS_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionFileList.h"
#include "BatchAcquisitionWindowDisplay.h"
#include "BatchAcquisitionImageDecoder.h"
#include "BatchAcquisitionImageEncoder.h"
#include "BatchAcquisitionRendering.h"
#include "BatchAcquisitionAcquisition.h"



//! Scanner data.
/*!
  This structure stores pointers to all threads which are necessary for the scanning process.
  They are collected here to allow easy manipulation of projectors and cameras such as
  adding or removing projector or camera.
*/
typedef struct Scanner_
{
  SynchronizationEvents * pSynchronization; //!< Pointer to thread synchronization structure.
  bool internally_allocated; //!< Flag which indicates if synchronization data is allocated internally.
  
  std::vector<ImageFileList *> sImageList; //!< List of objects which provide lists of images to project.
  std::vector<ImageDecoderParameters *> sImageDecoder; //!< Threads which prepare images to project.
  std::vector<DisplayWindowParameters *> sWindowDisplay; //!< Windows which are used to display images to project.
  std::vector<RenderingParameters *> sRendering; //!< Threads which render images using DirectX.
  std::vector<ImageEncoderParameters *> sImageEncoder; //!< Threads which store acquired images to disk.
  std::vector<AcquisitionParameters *> sAcquisition; //!< Threads which trigger the cameras.
  
  std::vector<std::wstring *> sConnectedCameras; //!< Names of all connected cammeras.

  SRWLOCK StorageLock; //!< Storage lock to control concurrent acces to stored data.

} Scanner;




#endif /* !__BATCHACQUISITIONMAINHELPERS_H */
