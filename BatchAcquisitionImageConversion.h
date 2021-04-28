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
  \file   BatchAcquisitionImageConversion.h
  \brief  Image conversions.

  \author Tomislav Petkovic
  \date   2015-05-27
*/


#ifndef __BATCHACQUISITIONIMAGECONVERSION_H
#define __BATCHACQUISITIONIMAGECONVERSION_H


#include "BatchAcquisition.h"



//! Convert monochromatic 8-bit image to BGR8.
HRESULT ConvertMono8uToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert monochromatic 16-bit image to BGR8.
HRESULT ConvertMono16uToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert YUV411 image to BGR8.
HRESULT ConvertYUV411ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert YUV422 image to BGR8.
HRESULT ConvertYUV422ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert YUV422 image to BGR8.
HRESULT ConvertYUV422ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert YUV422 image to full-scale BGR8 (BT.601).
HRESULT ConvertYUV422BT601ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert YUV422 image to full-scale BGR8 (BT.601).
HRESULT ConvertYUV422BT601ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert YUV422 image to scaled-down BGR8 (BT.601).
HRESULT ConvertYUV422BT601ToScaledDownBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert YUV422 image to full-scale BGR8 (BT.709).
HRESULT ConvertYUV422BT709ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert YUV422 image to full-scale BGR8 (BT.709).
HRESULT ConvertYUV422BT709ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert YUV422 image to scaled-down BGR8 (BT.709).
HRESULT ConvertYUV422BT709ToScaledDownBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert BGR8 image to BGR8.
HRESULT ConvertBGR8ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert RGB8 image to BGR8.
HRESULT ConvertRGB8ToBGR8(unsigned int const, unsigned int const, unsigned int const, void const * const, IWICImagingFactory * const, IWICBitmap **);

//! Convert by changing byte order in place.
HRESULT SwapBGR8ToRGB8InPlace(unsigned int const, unsigned int const, unsigned int const, void * const);

//! Convert UYV to YUV by changing byte order.
HRESULT SwapUYV8ToYUV8(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert UYV to YUV by changing byte order in place.
HRESULT SwapUYV8ToYUV8InPlace(unsigned int const, unsigned int const, unsigned int const, void * const);

//! Shift bits.
HRESULT ShiftLeftMono16(unsigned int const, unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Swap bytes.
HRESULT SwapBytesMono16(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Swap bytes in place.
HRESULT SwapBytesMono16InPlace(unsigned int const, unsigned int const, unsigned int const, void * const);

//! Expand 12 bit packed data to 16 bit unpacked.
HRESULT Expand12BitTo16Bit(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Shrink 12 bit packed data to 8 bit unpacked.
HRESULT Shrink12BitTo8Bit(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Shrink 10 bit unpacked data to 8 bit unpacked.
HRESULT Shrink16BitLSB10To8Bit(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Shrink 16 bit data to 8 bit unpacked.
HRESULT Shrink16BitTo8Bit(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Shrink 16 bit data to 8 bit unpacked.
HRESULT Shrink16BitTo8BitBigEndian(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Shrink 32 bit data to 8 bit unpacked.
HRESULT Shrink32BitTo8Bit(unsigned int const, unsigned int const, unsigned int const, void const * const, unsigned int const, void * const);

//! Convert raw buffer to BGR cv::Mat.
cv::Mat * RawBufferToBGRcvMat(ImageDataType const, unsigned int const, unsigned int const, unsigned int const, void const * const);

//! Convert raw buffer to grayscale cv::Mat.
cv::Mat * RawBufferToGraycvMat(ImageDataType const, unsigned int const, unsigned int const, unsigned int const, void const * const);

//! Convert raw buffer to single channel cv::Mat.
cv::Mat * RawBufferTo1CcvMat(ImageDataType const, unsigned int const, unsigned int const, unsigned int const, void const * const);



#endif /* !__BATCHACQUISITIONIMAGECONVERSION_H */
