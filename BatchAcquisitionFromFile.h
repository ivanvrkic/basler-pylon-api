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
  \file   BatchAcquisitionFromFile.h
  \brief  Functions for dummy acquisition from file.

  \author Tomislav Petkovic
  \date   2017-01-26
*/


#ifndef __BATCHACQUISITIONFROMFILE_H
#define __BATCHACQUISITIONFROMFILE_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionFileList.h"


struct AcquisitionParameters_;


//! Parameters of the dummy camera.
/*!
  All classes and information needed to control a dummy camera are stored in this structure.
*/
typedef
struct AcquisitionParametersFromFile_
{
  ImageFileList * pFileList; //!< Pointer to image file list.
  void * pAcquisitionThread; //!< Pointer to acquisition thread data.

  volatile bool external_list; //!< Flag to indicate file list is external.
} AcquisitionParametersFromFile;



/****** IMAGE TRANSFER ******/

//! Queue image for processing.
void
DispatchNextImageFromFile(
                          AcquisitionParameters_ * const P
                          );



/****** EXPORTED FUNCTIONS ******/

//! Set input directory.
bool
AcquisitionParametersFromFileSetDirectory(
                                          AcquisitionParametersFromFile * const,
                                          wchar_t const * const
                                          );

//! Get input directory.
wchar_t const * const
AcquisitionParametersFromFileGetDirectory(
                                          AcquisitionParametersFromFile * const
                                          );

//! Stop all pending transfers.
bool
AcquisitionParametersFromFileStopTransfer(
                                          AcquisitionParametersFromFile * const
                                          );

//! Start transfer.
bool
AcquisitionParametersFromFileStartTransfer(
                                           AcquisitionParametersFromFile * const,
                                           wchar_t const * const
                                           );

//! Release resources.
void
AcquisitionParametersFromFileRelease(
                                     AcquisitionParametersFromFile * const
                                     );

//! Adjust camera exposure time.
bool
AcquisitionParametersFromFileAdjustExposureTime(
                                                AcquisitionParametersFromFile * const,
                                                double const,
                                                double * const
                                                );

//! Create resources.
AcquisitionParametersFromFile *
AcquisitionParametersFromFileCreate(
                                    AcquisitionParameters_ * const,
                                    ImageFileList * const
                                    );


#endif /* __BATCHACQUISITIONFLYCAPTURE2_H */
