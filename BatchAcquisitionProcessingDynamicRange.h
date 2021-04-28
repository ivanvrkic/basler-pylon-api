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
  \file   BatchAcquisitionProcessingDynamicRange.h
  \brief  Dynamic range and texture estimation methods.

  Function for computing dynamic range and texture for
  phase shifting.

  \author Tomislav Petkovic
  \date   2017-06-13
*/


#ifndef __BATCHACQUISITIONPROCESSINGDYNAMICRANGE_H
#define __BATCHACQUISITIONPROCESSINGDYNAMICRANGE_H


#include "BatchAcquisitionProcessing.h"


//! Dynamic range estimation (single precision).
cv::Mat * EstimateDynamicRange(ImageSet * const, int const, int const);

//! Combine dynamic ranges (single precision).
cv::Mat * CombineDynamicRanges(cv::Mat const * const, cv::Mat const * const);

//! Dynamic range and texture estimation (single precision).
bool UpdateDynamicRangeAndTexture(ImageSet * const, int const, int const, cv::Mat * * const, cv::Mat * * const);

//! Fetch texture image.
cv::Mat * FetchTexture(ImageSet * const, int const);

//! Convert texture image to 8-bit BGR.
cv::Mat * ScaleAndDeBayerTexture(cv::Mat * const, ImageDataType const, int const);



#endif /* !__BATCHACQUISITIONPROCESSINGDYNAMICRANGE_H */
