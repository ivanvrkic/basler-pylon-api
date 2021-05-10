/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2016-2021 UniZG, Zagreb. All rights reserved.
 * (c) 2016-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisition.h
  \brief  Main include header.

  \author Tomislav Petkovic
  \date   2021-04-20
*/


#ifndef __BATCHACQUISITION_H
#define __BATCHACQUISITION_H



#ifndef STRING_LENGTH
//! Maximum string length used by Sapera SDK.
#define STRING_LENGTH 256
#endif /* !STRING_LENGTH */



/****** MATRICES ******/

/* Alignment for either 16 or 32. */
#define BATCHACQUISITION_ALIGN16 __declspec(align(16))
#define BATCHACQUISITION_ALIGN32 __declspec(align(32))


//! Three element vector.
/*!
  Three element vector of the float type (single precision)
  that is aligned on a 16 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN16 float float_a_V3[3];

//! Four element vector.
/*!
  Four element vector of the float type (single precision)
  that is aligned on a 16 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN16 float float_a_V4[4];



//! Three element vector.
/*!
  Three element vector of the double type (double precision)
  that is aligned on a 32 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 double double_a_V3[3];

//! Four element vector.
/*!
  Four element vector of the double type (double precision)
  that is aligned on a 32 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 double double_a_V4[4];



//! 3x3 matrix.
/*!
  Three times three matrix of the float type (single precision)
  that is aligned on 16 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN16 float float_a_M33[3][3];

//! 4x4 matrix.
/*!
  Four times four matrix of the float type (single precision)
  that is aligned on 32 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 float float_a_M44[4][4];



//! 3x3 matrix.
/*!
  Three times three matrix of the double type (double precision)
  that is aligned on 16 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 double double_a_M33[3][3];

//! 3x4 matrix.
/*!
  Three times four matrix of the double type (double precision)
  that is aligned on 16 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 double double_a_M34[3][4];

//! 4x4 matrix.
/*!
  Four times four matrix of the double type (double precision)
  that is aligned on 32 bytes boundary.
*/
typedef BATCHACQUISITION_ALIGN32 double double_a_M44[4][4];



/****** CAMERA SDK ******/

//! Selects which camera SDK to use.
/*!
  There are several camera SDK's available. Each one is locked to
  specific camera type. Currently we support Sapera SDK and FlyCapture2 SDK.
  Default SDK is FlyCapture2.
*/
typedef
enum CameraSDK_
  {
    CAMERA_SDK_DEFAULT, /*!< Default camera SDK. */
    CAMERA_SDK_FLYCAPTURE2, /*!< PointGrey FlyCapture2 SDK. */
    CAMERA_SDK_SAPERA, /*!< Teledyne Dalsa Sapera SDK. */
    CAMERA_SDK_FROM_FILE, /*!< Dummy acquisition from file. */
    CAMERA_SDK_PYLON, /*!< Basler Pylon SDK. */
    CAMERA_SDK_SPINNAKER, /*!< Flir Spinnaker SDK. */
    CAMERA_SDK_UNKNOWN /*!< Unknown camera SDK. */
  } CameraSDK;



/****** STRUCTURED LIGHT PATTERN TYPE ******/

//! Identifies projected structured light pattern.
/*!
  There are several structured light patterns we use. Some are user defined, some are
  used for calibration etc. Negative values denote unknown pattern types.
*/
typedef
enum StructuredLightPatternType_
  {
    SL_PATTERN_INVALID = -1, //!< Enumeration for undefined pattern.
    SL_PATTERN_FROM_FILE = 1, //!< User defined pattern which is read from file.
    SL_PATTERN_BLACK, //!< All black SL pattern.
    SL_PATTERN_FRINGE_HORIZONTAL, //!< Horizontal sinusoidal fringe.
    SL_PATTERN_FRINGE_VERTICAL, //!< Vertical sinusoidal fringe.
    SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY, //!< Repeating color pattern that requires delay change after every acquisition.
    SL_PATTERN_DLP_WHEEL_SOFTWARE_DELAY, //!< Repeating color pattern that requires delay change after every acquisition.
    SL_PATTERN_RED_CHANNEL_TRANSFER, //!< Color pattern for measuring red channel transfer function.
    SL_PATTERN_GREEN_CHANNEL_TRANSFER, //!< Color pattern for measuring green channel transfer function.
    SL_PATTERN_BLUE_CHANNEL_TRANSFER, //!< Color pattern for measuring blue channel transfer function.
    SL_PATTERN_GRAY_CHANNEL_TRANSFER, //!< Color pattern for measuring gray channel transfer function.
    SL_PATTERN_CYAN_CHANNEL_TRANSFER, //!< Color pattern for measuring cyan channel transfer function.
    SL_PATTERN_YELLOW_CHANNEL_TRANSFER, //!< Color pattern for measuring yellow channel transfer function.
    SL_PATTERN_MAGENTA_CHANNEL_TRANSFER, //!< Color pattern for measuring magenta channel transfer function.
    SL_PATTERN_FIXED, //!< One image pattern.
    SL_PATTERN_DELAY_MEASUREMENT, //!< Repeated image for delay measurement.
    SL_PATTERN_DELAY_MEASUREMENT_WHITE, //!< All white pattern for delay measurement.
    SL_PATTERN_DELAY_MEASUREMENT_BLACK, //!< All black pattern for delay measurement.
    SL_PATTERN_DELAY_MEASUREMENT_WHITE_TO_BLACK, //!< White-to-black transition for delay measurement.
    SL_PATTERN_DELAY_MEASUREMENT_BLACK_TO_WHITE, //!< White-to-black transition for delay measurement.
  } StructuredLightPatternType;



/****** RECONSTRUCTION METHOD ******/

//! Selects which reconstruction method to use.
/*!
  There are several reconstruction methods that are tied to the structured light pattern.
*/
typedef
enum ReconstructionMethod_
  {
    RECONSTRUCTION_DEFAULT, /*!< Default reconstruction method. */
    RECONSTRUCTION_PSGC_COL, /*!< Gray code and phase shifting using column code. */
    RECONSTRUCTION_PSGC_ROW, /*!< Gray code and phase shifting using row code. */
    RECONSTRUCTION_PSGC_ALL, /*!< Gray code and phase shifting using both column and row code. */
    RECONSTRUCTION_MPS2_COL, /*!< Two-frequency multiple phase shifting using column code. */
    RECONSTRUCTION_MPS2_ROW, /*!< Two-frequency multiple phase shifting using row code. */
    RECONSTRUCTION_MPS2_ALL, /*!< Two-frequency multiple phase shifting using both column and row code. */
    RECONSTRUCTION_MPS3_COL, /*!< Three-frequency multiple phase shifting using column code. */
    RECONSTRUCTION_MPS3_ROW, /*!< Three-frequency multiple phase shifting using row code. */
    RECONSTRUCTION_MPS3_ALL, /*!< Three-frequency multiple phase shifting using both column and row code. */
    RECONSTRUCTION_CONFIGURE_PARAMETERS, /*!< Special tag to indicate reconstruction parameters have to be changed. */
  } ReconstructionMethod;



/****** PIXELS ******/

/* Default pixel values for DirectX and WIC.
   BGR is supposed to be faster then RBG, however, MSDN documentation
   suggests RGB has better compatibility with older DirectX capable hardware.
   OpenCV uses BGR by default so this is the format we use.
*/

//! Default format for DirectX bitmaps. We use BGR for compatibility with OpenCV.
#define DEFAULT_DIRECT_X_PIXEL_FORMAT DXGI_FORMAT_B8G8R8A8_UNORM

//! Default format for WIC bitmap. We use BGR for compatibility with OpenCV.
#define DEFAULT_WIC_PIXEL_FORMAT GUID_WICPixelFormat32bppPBGRA

// RGB formats. Using these requires compatibility layer that swaps BGR of OpenCV to RGB.
//#define DEFAULT_DIRECT_X_PIXEL_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
//#define DEFAULT_WIC_PIXEL_FORMAT GUID_WICPixelFormat32bppPRGBA



//! Image data type.
/*!
  Image data may be stored in various formats. Every camera SDK will have
  its own image codes that are usually not completely indentical nor compatible.
  OpenCV also has its own image data codes. Here we define a set of supported image
  datatypes for which the converters to grayscale and BGR formats are implemented.
  All implemented conversions preserve data depth.

  For multi-byte data normal storage order is little endian.
  If the storage is big endian then _BigEndian suffix is attached to particular enumeration name.

  Data is normally assumend to be unpacked so every pixel starts at byte boundary.
  If the data is packed so byte is shared between adjacent pixels then _Packed suffix is attached to particular enumeration name.

  Bayer formats are defined as in GenICam specification by the colors of first two pixels.
  Note that OpenCV defines Bayer formats by the colors of second and third pixel in the second row, e.g. GenICam GR becomes OpenCV GB etc.

  Note there exists significant confusion about YUV formats; a good explanation of various storage types may be found
  in GenICam "Pixel Format Naming Conventions" document. One must take careful notice of how the data is subsampled
  and what is the range as not all YUV formats utilize the full data range, e.g. YUV411 is different than YUV420 as
  411 subsampling uses 4x1 blocks and 420 subsampling uses 2x2 blocks etc.
  Also see https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx.
*/
typedef
enum ImageDataType_
  {
    IDT_UNKNOWN = 0, /*!< Unknown or unsupported image data type. */
    IDT_8U_BINARY, /*!< One channel 8 bit unsigned data with two levels only. */
    IDT_8U_GRAY, /*!< One channel 8 bit unsigned data; graylevel image. */
    IDT_10U_GRAY, /*!< One channel 10 bit unsigned data stored in two bytes where 6 MSB are zeros; graylevel image. */
    IDT_12U_GRAY_Packed, /*!< One channel 12 bit packed unsigned data where two pixels are stored in two bytes; graylevel image. */
    IDT_16U_GRAY, /*!< One channel 16 bit unsigned data; graylevel image. */
    IDT_16U_GRAY_BigEndian, /*!< One channel 16 bit unsigned data; graylevel image. */
    IDT_32U_GRAY, /*!< One channel 32 bit unsigned data; graylevel image. */
    IDT_8S_GRAY, /*!< One channel 8 bit signed data; graylevel image. */
    IDT_16S_GRAY, /*!< One channel 8 bit signed data; graylevel image. */
    IDT_16S_GRAY_BigEndian, /*!< One channel 8 bit signed data; graylevel image. */
    IDT_32S_GRAY, /*!< One channel 8 bit signed data; graylevel image. */
    IDT_8U_BayerGR, /*!< Bayer GR 8-bit data; the first two pixels in the first row are GR. */
    IDT_8U_BayerRG, /*!< Bayer RG 8-bit data; the first two pixels in the first row are RG. */
    IDT_8U_BayerGB, /*!< Bayer GB 8-bit data; the first two pixels in the first row are GB. */
    IDT_8U_BayerBG, /*!< Bayer BG 8-bit data; the first two pixels in the first row are BG. */
    IDT_10U_BayerGR, /*!< Bayer GR 10-bit data stored two bytes LSB aligned; the first two pixels in the first row are GR. */
    IDT_10U_BayerRG, /*!< Bayer RG 10-bit data stored two bytes LSB aligned; the first two pixels in the first row are RG. */
    IDT_10U_BayerGB, /*!< Bayer GB 10-bit data stored two bytes LSB aligned; the first two pixels in the first row are GB. */
    IDT_10U_BayerBG, /*!< Bayer BG 10-bit data stored two bytes LSB aligned; the first two pixels in the first row are BG. */
    IDT_12U_BayerGR_Packed, /*!< Bayer GR 12-bit packed data; the first two pixels in the first row are GR. */
    IDT_12U_BayerRG_Packed, /*!< Bayer RG 12-bit packed data; the first two pixels in the first row are RG. */
    IDT_12U_BayerGB_Packed, /*!< Bayer GB 12-bit packed data; the first two pixels in the first row are GB. */
    IDT_12U_BayerBG_Packed, /*!< Bayer BG 12-bit packed data; the first two pixels in the first row are BG. */
    IDT_16U_BayerGR, /*!< Bayer GR 16-bit data; the first two pixels in the first row are GR. */
    IDT_16U_BayerRG, /*!< Bayer RG 16-bit data; the first two pixels in the first row are RG. */
    IDT_16U_BayerGB, /*!< Bayer GB 16-bit data; the first two pixels in the first row are GB. */
    IDT_16U_BayerBG, /*!< Bayer BG 16-bit data; the first two pixels in the first row are BG. */
    IDT_16U_BayerGR_BigEndian, /*!< Bayer GR 8-bit data; the first two pixels in the first row are GR. */
    IDT_16U_BayerRG_BigEndian, /*!< Bayer RG 8-bit data; the first two pixels in the first row are RG. */
    IDT_16U_BayerGB_BigEndian, /*!< Bayer GB 8-bit data; the first two pixels in the first row are GB. */
    IDT_16U_BayerBG_BigEndian, /*!< Bayer BG 8-bit data; the first two pixels in the first row are BG. */
    IDT_8U_RGB, /*!< 8-bit RGB data. */
    IDT_8U_RGB_Planar, /*!< Planar 8-bit RGB data. */
    IDT_8U_RGBA, /*!< 8-bit RGBA data. */
    IDT_8U_BGR, /*!< 8-bit BGR data. */
    IDT_16U_BGR, /*!< 16-bit BGR data. */
    IDT_8U_BGRA, /*!< 8-bit BGRA data. */
    IDT_8U_YUV411, /*!< 8-bit YUV 4:1:1 subsampled data. All YUV are in [0,255] range. In increasing memory addresses the order is U0 Y0 Y1 V0 Y2 Y3 U4 Y4 Y5 V4 Y6 Y7 etc.*/
    IDT_8U_YUV422, /*!< 8-bit YUV 4:2:2 subsampled data. All YUV are in [0,255] range. In increasing memory addresses the order is U0 Y0 V0 Y1 U2 Y2 V2 Y3 U4 Y4 V4 etc. */
    IDT_8U_YUV422_BT601, /*!< 8-bit YUV 4:2:2 BT.601 subsampled data. Y is in [16,235] and UV are in [16,240] ranges. */
    IDT_8U_YUV422_BT709, /*!< 8-bit YUV 4:2:2 BT.709 subsampled data. Y is in [16,235] and UV are in [16,240] ranges. */
    IDT_8U_YUV444, /*!< 8-bit YUV 4:4:4 data. All YUV are in [0,255] range. In increasing memory addresses the order is Y0 U0 V0 Y1 U1 V1 etc. */
    IDT_8U_UYV444 /*!< 8-bit UYV 4:4:4 data. All YUV are in [0,255] range. In increaseing memory addresses the order is U0 Y0 V0 U1 Y1 V1. */
  } ImageDataType;


static
wchar_t const * const ImageDataTypeNames[] = {
  L"IDT_UNKNOWN",
  L"IDT_8U_BINARY",
  L"IDT_8U_GRAY",
  L"IDT_10U_GRAY",
  L"IDT_12U_GRAY_Packed",
  L"IDT_16U_GRAY",
  L"IDT_16U_GRAY_BigEndian",
  L"IDT_32U_GRAY",
  L"IDT_8S_GRAY",
  L"IDT_16S_GRAY",
  L"IDT_16S_GRAY_BigEndian",
  L"IDT_32S_GRAY",
  L"IDT_8U_BayerGR",
  L"IDT_8U_BayerRG",
  L"IDT_8U_BayerGB",
  L"IDT_8U_BayerBG",
  L"IDT_10U_BayerGR",
  L"IDT_10U_BayerRG",
  L"IDT_10U_BayerGB",
  L"IDT_10U_BayerBG",
  L"IDT_12U_BayerGR_Packed",
  L"IDT_12U_BayerRG_Packed",
  L"IDT_12U_BayerGB_Packed",
  L"IDT_12U_BayerBG_Packed",
  L"IDT_16U_BayerGR",
  L"IDT_16U_BayerRG",
  L"IDT_16U_BayerGB",
  L"IDT_16U_BayerBG",
  L"IDT_16U_BayerGR_BigEndian",
  L"IDT_16U_BayerRG_BigEndian",
  L"IDT_16U_BayerGB_BigEndian",
  L"IDT_16U_BayerBG_BigEndian",
  L"IDT_8U_RGB",
  L"IDT_8U_RGB_Planar",
  L"IDT_8U_RGBA",
  L"IDT_8U_BGR",
  L"IDT_16U_BGR",
  L"IDT_8U_BGRA",
  L"IDT_8U_YUV411",
  L"IDT_8U_YUV422",
  L"IDT_8U_YUV422_BT601",
  L"IDT_8U_YUV422_BT709",
  L"IDT_8U_YUV444",
  L"IDT_8U_UYV444"
};



//! Returns image datatype string.
/*!
  Returns image datatype string.

  \param type   Image datatype.
  \return A pointer to wide string.
*/
inline
wchar_t const * const
StringFromImageDataType_inline(
                               ImageDataType const type
                               )
{
  int const value = (int)(type);
  if ( (0 <= type) && (type < sizeof(ImageDataTypeNames) ) ) return ImageDataTypeNames[(int)type];
  return ImageDataTypeNames[IDT_UNKNOWN];
}
/* StringFromImageDataType_inline */



//! Image data type from number.
/*!
  Returns image data type from its number.

  \param value  Number.
  \return Returns corresponding image data type or IDT_UNKNOWN is the corrspoding type does not exist.
*/
inline
ImageDataType
ImageDataTypeFromInt_inline(
                            int const value
                            )
{
  switch (value)
    {
    case (int)IDT_UNKNOWN: return IDT_UNKNOWN;
    case (int)IDT_8U_BINARY: return IDT_8U_BINARY;
    case (int)IDT_8U_GRAY: return IDT_8U_GRAY;
    case (int)IDT_10U_GRAY: return IDT_10U_GRAY;
    case (int)IDT_12U_GRAY_Packed: return IDT_12U_GRAY_Packed;
    case (int)IDT_16U_GRAY: return IDT_16U_GRAY;
    case (int)IDT_16U_GRAY_BigEndian: return IDT_16U_GRAY_BigEndian;
    case (int)IDT_32U_GRAY: return IDT_32U_GRAY;
    case (int)IDT_8S_GRAY: return IDT_8S_GRAY;
    case (int)IDT_16S_GRAY: return IDT_16S_GRAY;
    case (int)IDT_16S_GRAY_BigEndian: return IDT_16S_GRAY_BigEndian;
    case (int)IDT_32S_GRAY: return IDT_32S_GRAY;
    case (int)IDT_8U_BayerGR: return IDT_8U_BayerGR;
    case (int)IDT_8U_BayerRG: return IDT_8U_BayerRG;
    case (int)IDT_8U_BayerGB: return IDT_8U_BayerGB;
    case (int)IDT_8U_BayerBG: return IDT_8U_BayerBG;
    case (int)IDT_10U_BayerGR: return IDT_10U_BayerGR;
    case (int)IDT_10U_BayerRG: return IDT_10U_BayerRG;
    case (int)IDT_10U_BayerGB: return IDT_10U_BayerGB;
    case (int)IDT_10U_BayerBG: return IDT_10U_BayerBG;
    case (int)IDT_12U_BayerGR_Packed: return IDT_12U_BayerGR_Packed;
    case (int)IDT_12U_BayerRG_Packed: return IDT_12U_BayerRG_Packed;
    case (int)IDT_12U_BayerGB_Packed: return IDT_12U_BayerGB_Packed;
    case (int)IDT_12U_BayerBG_Packed: return IDT_12U_BayerBG_Packed;
    case (int)IDT_16U_BayerGR: return IDT_16U_BayerGR;
    case (int)IDT_16U_BayerRG: return IDT_16U_BayerRG;
    case (int)IDT_16U_BayerGB: return IDT_16U_BayerGB;
    case (int)IDT_16U_BayerBG: return IDT_16U_BayerBG;
    case (int)IDT_16U_BayerGR_BigEndian: return IDT_16U_BayerGR_BigEndian;
    case (int)IDT_16U_BayerRG_BigEndian: return IDT_16U_BayerRG_BigEndian;
    case (int)IDT_16U_BayerGB_BigEndian: return IDT_16U_BayerGB_BigEndian;
    case (int)IDT_16U_BayerBG_BigEndian: return IDT_16U_BayerBG_BigEndian;
    case (int)IDT_8U_RGB: return IDT_8U_RGB;
    case (int)IDT_8U_RGB_Planar: return IDT_8U_RGB_Planar;
    case (int)IDT_8U_RGBA: return IDT_8U_RGBA;
    case (int)IDT_8U_BGR: return IDT_8U_BGR;
    case (int)IDT_16U_BGR: return IDT_16U_BGR;
    case (int)IDT_8U_BGRA: return IDT_8U_BGRA;
    case (int)IDT_8U_YUV411: return IDT_8U_YUV411;
    case (int)IDT_8U_YUV422: return IDT_8U_YUV422;
    case (int)IDT_8U_YUV422_BT601: return IDT_8U_YUV422_BT601;
    case (int)IDT_8U_YUV422_BT709: return IDT_8U_YUV422_BT709;
    case (int)IDT_8U_YUV444: return IDT_8U_YUV444;
    case (int)IDT_8U_UYV444: return IDT_8U_UYV444;
    default: return IDT_UNKNOWN;
    }
  /* switch */
  return IDT_UNKNOWN;
}
/* ImageDataTypeFromInt_inline */



//! Return number of bits per pixel.
/*!
  Returns the number of bits per pixel

  \param type   Image data type.
  \return Number of bits per pixel.
*/
inline
unsigned int
PixelSizeInBitsFromImageDataType_inline(
                                        ImageDataType const type
                                        )
{
  switch (type)
    {
    case IDT_UNKNOWN: return 0;
    case IDT_8U_BINARY: return 8;
    case IDT_8U_GRAY: return 8;
    case IDT_10U_GRAY: return 16;
    case IDT_12U_GRAY_Packed: return 12;
    case IDT_16U_GRAY: return 16;
    case IDT_16U_GRAY_BigEndian: return 16;
    case IDT_32U_GRAY: return 32;
    case IDT_8S_GRAY: return 8;
    case IDT_16S_GRAY: return 16;
    case IDT_16S_GRAY_BigEndian: return 16;
    case IDT_32S_GRAY: return 32;
    case IDT_8U_BayerGR: return 8;
    case IDT_8U_BayerRG: return 8;
    case IDT_8U_BayerGB: return 8;
    case IDT_8U_BayerBG: return 8;
    case IDT_10U_BayerGR: return 16;
    case IDT_10U_BayerRG: return 16;
    case IDT_10U_BayerGB: return 16;
    case IDT_10U_BayerBG: return 16;
    case IDT_12U_BayerGR_Packed: return 12;
    case IDT_12U_BayerRG_Packed: return 12;
    case IDT_12U_BayerGB_Packed: return 12;
    case IDT_12U_BayerBG_Packed: return 12;
    case IDT_16U_BayerGR: return 16;
    case IDT_16U_BayerRG: return 16;
    case IDT_16U_BayerGB: return 16;
    case IDT_16U_BayerBG: return 16;
    case IDT_16U_BayerGR_BigEndian: return 16;
    case IDT_16U_BayerRG_BigEndian: return 16;
    case IDT_16U_BayerGB_BigEndian: return 16;
    case IDT_16U_BayerBG_BigEndian: return 16;
    case IDT_8U_RGB: return 3 * 8;
    case IDT_8U_RGB_Planar: return 3 * 8;
    case IDT_8U_RGBA: return 4 * 8;
    case IDT_8U_BGR: return 3 * 8;
    case IDT_16U_BGR: return 3 * 16;
    case IDT_8U_BGRA: return 4 * 8;
    case IDT_8U_YUV411: return 12;
    case IDT_8U_YUV422: return 16;
    case IDT_8U_YUV422_BT601: return 16;
    case IDT_8U_YUV422_BT709: return 16;
    case IDT_8U_YUV444: return 24;
    case IDT_8U_UYV444: return 24;
    }
  /* switch */
  return 0;
}
/* PixelSizeInBitsFromImageDataType_inline */



//! Return place of the MSB bit.
/*!
  Returns index of the MSB bit, i.e. if pixel data is 16 bits but only
  10 bits are used for data storage then MSB bit may be any bit between 9 and 15.
  Note that this MSB position is for input RAW buffer.

  \param type   Image data type.
  \return Position of MSB bit.
*/
inline
unsigned int
MSBPositionInRAWFromImageDataType_inline(
                                         ImageDataType const type
                                         )
{
  switch (type)
    {
    case IDT_UNKNOWN: return 0;
    case IDT_8U_BINARY: return 7;
    case IDT_8U_GRAY: return 7;
    case IDT_10U_GRAY: return 9;
    case IDT_12U_GRAY_Packed: return 11;
    case IDT_16U_GRAY: return 15;
    case IDT_16U_GRAY_BigEndian: return 15;
    case IDT_32U_GRAY: return 31;
    case IDT_8S_GRAY: return 7;
    case IDT_16S_GRAY: return 15;
    case IDT_16S_GRAY_BigEndian: return 15;
    case IDT_32S_GRAY: return 30;
    case IDT_8U_BayerGR: return 7;
    case IDT_8U_BayerRG: return 7;
    case IDT_8U_BayerGB: return 7;
    case IDT_8U_BayerBG: return 7;
    case IDT_10U_BayerGR: return 9;
    case IDT_10U_BayerRG: return 9;
    case IDT_10U_BayerGB: return 9;
    case IDT_10U_BayerBG: return 9;
    case IDT_12U_BayerGR_Packed: return 11;
    case IDT_12U_BayerRG_Packed: return 11;
    case IDT_12U_BayerGB_Packed: return 11;
    case IDT_12U_BayerBG_Packed: return 11;
    case IDT_16U_BayerGR: return 15;
    case IDT_16U_BayerRG: return 15;
    case IDT_16U_BayerGB: return 15;
    case IDT_16U_BayerBG: return 15;
    case IDT_16U_BayerGR_BigEndian: return 15;
    case IDT_16U_BayerRG_BigEndian: return 15;
    case IDT_16U_BayerGB_BigEndian: return 15;
    case IDT_16U_BayerBG_BigEndian: return 15;
    case IDT_8U_RGB: return 7;
    case IDT_8U_RGB_Planar: return 7;
    case IDT_8U_RGBA: return 7;
    case IDT_8U_BGR: return 7;
    case IDT_16U_BGR: return 15;
    case IDT_8U_BGRA: return 7;
    case IDT_8U_YUV411: return 7;
    case IDT_8U_YUV422: return 7;
    case IDT_8U_YUV422_BT601: return 7;
    case IDT_8U_YUV422_BT709: return 7;
    case IDT_8U_YUV444: return 7;
    case IDT_8U_UYV444: return 7;
    }
  /* switch */
  return 0;
}
/* MSBPositionInRAWFromImageDataType_inline */



//! Return place of the MSB bit.
/*!
  Returns index of the MSB bit, i.e. if pixel data is 16 bits but only
  10 bits are used for data storage then MSB bit may be any bit between 9 and 15.
  Note that this MSB position is after RAW buffer was decoded to one
  of OpenCV types.

  \param type   Image data type.
  \return Position of MSB bit.
*/
inline
unsigned int
MSBPositionInOpenCVFromImageDataType_inline(
                                            ImageDataType const type
                                            )
{
  switch (type)
    {
    case IDT_UNKNOWN: return 0;
    case IDT_8U_BINARY: return 7;
    case IDT_8U_GRAY: return 7;
    case IDT_10U_GRAY: return 15;
    case IDT_12U_GRAY_Packed: return 15;
    case IDT_16U_GRAY: return 15;
    case IDT_16U_GRAY_BigEndian: return 15;
    case IDT_32U_GRAY: return 31;
    case IDT_8S_GRAY: return 7;
    case IDT_16S_GRAY: return 15;
    case IDT_16S_GRAY_BigEndian: return 15;
    case IDT_32S_GRAY: return 30;
    case IDT_8U_BayerGR: return 7;
    case IDT_8U_BayerRG: return 7;
    case IDT_8U_BayerGB: return 7;
    case IDT_8U_BayerBG: return 7;
    case IDT_10U_BayerGR: return 15;
    case IDT_10U_BayerRG: return 15;
    case IDT_10U_BayerGB: return 15;
    case IDT_10U_BayerBG: return 15;
    case IDT_12U_BayerGR_Packed: return 15;
    case IDT_12U_BayerRG_Packed: return 15;
    case IDT_12U_BayerGB_Packed: return 15;
    case IDT_12U_BayerBG_Packed: return 15;
    case IDT_16U_BayerGR: return 15;
    case IDT_16U_BayerRG: return 15;
    case IDT_16U_BayerGB: return 15;
    case IDT_16U_BayerBG: return 15;
    case IDT_16U_BayerGR_BigEndian: return 15;
    case IDT_16U_BayerRG_BigEndian: return 15;
    case IDT_16U_BayerGB_BigEndian: return 15;
    case IDT_16U_BayerBG_BigEndian: return 15;
    case IDT_8U_RGB: return 7;
    case IDT_8U_RGB_Planar: return 7;
    case IDT_8U_RGBA: return 7;
    case IDT_8U_BGR: return 7;
    case IDT_16U_BGR: return 15;
    case IDT_8U_BGRA: return 7;
    case IDT_8U_YUV411: return 7;
    case IDT_8U_YUV422: return 7;
    case IDT_8U_YUV422_BT601: return 7;
    case IDT_8U_YUV422_BT709: return 7;
    case IDT_8U_YUV444: return 7;
    case IDT_8U_UYV444: return 7;
    }
  /* switch */
  return 0;
}
/* MSBPositionInOpenCVFromImageDataType_inline */



//! Check if image type is grayscale.
/*!
  Returns true if image data type is grayscale.

  \param type   Image data type.
  \return True if image is grayscale, false otherwise.
*/
inline
bool
ImageDataTypeIsGrayscale_inline(
                                ImageDataType const type
                                )
{
  switch (type)
    {
    case IDT_8U_BINARY:
    case IDT_8U_GRAY:
    case IDT_10U_GRAY:
    case IDT_12U_GRAY_Packed:
    case IDT_16U_GRAY:
    case IDT_16U_GRAY_BigEndian:
    case IDT_32U_GRAY:
    case IDT_8S_GRAY:
    case IDT_16S_GRAY:
    case IDT_16S_GRAY_BigEndian:
    case IDT_32S_GRAY:
      return true;
    }
  /* switch */
  return false;
}
/* ImageDataTypeIsGrayscale_inline */



//! Check if image type is Bayer.
/*!
  Returns true if image data type is Bayer.

  \param type   Image data type.
  \return True if image is Bayer, false otherwise.
*/
inline
bool
ImageDataTypeIsBayer_inline(
                            ImageDataType const type
                            )
{
  switch (type)
    {
    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
    case IDT_10U_BayerGR:
    case IDT_10U_BayerRG:
    case IDT_10U_BayerGB:
    case IDT_10U_BayerBG:
    case IDT_12U_BayerGR_Packed:
    case IDT_12U_BayerRG_Packed:
    case IDT_12U_BayerGB_Packed:
    case IDT_12U_BayerBG_Packed:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerBG:
    case IDT_16U_BayerGR_BigEndian:
    case IDT_16U_BayerRG_BigEndian:
    case IDT_16U_BayerGB_BigEndian:
    case IDT_16U_BayerBG_BigEndian:
      return true;
    }
  /* switch */
  return false;
}
/* ImageDataTypeIsBayer_inline */



//! Check if image is one-channel.
/*!
  Returns true if image data type is one-channel, i.e. it
  is either grayscale or Bayer.

  \param type   Image data type.
  \return Returns true if image is single channel image.
*/
inline
bool
ImageDataTypeIs1C_inline(
                         ImageDataType const type
                         )
{
  return ImageDataTypeIsGrayscale_inline(type) || ImageDataTypeIsBayer_inline(type);
}
/* ImageDataTypeIs1C_inline */



/****** AUXILIARY AND HELPER FUNCTIONS ******/

//! Creates Sapera classes.
/*!
  Most Sapera LT SDK classes require user to call Create method after
  class construction.

  \param ptr    Pointer to Sapera LT class.
*/
template <class T>
inline
void
SafeCreate(
           T * ptr
           )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return;

  BOOL const created = ptr->Create();
  assert(TRUE == created);
};



//! Deletes Sapera classes.
/*!
  Most Sapera LT SDK classes require user to call Destroy method prior
  to calling delete method. This template ensures proper destruction
  of Sapera LT SDK classes.

  \param ptr    Pointer to Sapera LT class (passed by reference).
*/
template <class T>
inline
void
SafeDestroy(
            T * & ptr
            )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return;

  BOOL const destroyed = ptr->Destroy();
  assert(TRUE == destroyed);

  if (TRUE == destroyed)
    {
      delete ptr;
      ptr = NULL;
    }
  /* if */
};



/* Safe deallocation macros. */

//! Safe release for all classes that inherit IUnknown interface. Use for DirectX and DXGI classes.
#define SAFE_RELEASE(p) { if( (p) ) (p)->Release(); (p) = NULL; }

//! Safe delete for all classes created with call to new.
#define SAFE_DELETE(a) { if( NULL != (a) ) delete (a); (a) = NULL; }

//! Safe delete[] for all classes created with call to new[].
#define SAFE_DELETE_ARRAY(a) { if( NULL != (a) ) delete[] (a); (a) = NULL; }

//! Safe free for memory blocks allocated with malloc.
#define SAFE_FREE(a) { if( NULL != (a) ) free( (a) ); (a) = NULL; }

//! Safe release CvMat.
#define SAFE_CV_RELEASE_MAT(a) { if( NULL != (a) ) cvReleaseMat(&a); (a) = NULL; }

//! Safe release IplImage.
#define SAFE_CV_RELEASE_IMAGE(a) { if( NULL != (a) ) cvReleaseImage(&a); (a) = NULL; }

//! Safe delete VTK classes.
#define SAFE_VTK_DELETE(a) { if( NULL != (a) ) (a)->Delete(); (a) = NULL; }


/* Safe assignment macro. */

//! Safe assign pointer to address.
#define SAFE_ASSIGN_PTR(ptr, adr) { if( NULL != (adr) ) { assert(NULL == *(adr)); *(adr) = (ptr); (ptr) = NULL; } }

//! Safe swap valid and NULL pointer.
#define SWAP_ONE_VALID_PTR(ptr1, ptr2) { assert( (NULL == ptr1) && (NULL != ptr2) ); ptr1 = ptr2; ptr2 = NULL; }


//! Returns nth element.
/*!
  Returns nth element of std::vector which stores pointers and which is protected by a SRW lock.
  If the requested element does not exist then a NULL pointer is returned.

  \param v      Pointer to std::vector.
  \param n      Index of element to return.
  \param lock   Pointer to SRW lock.
  \return Returns nth element.
*/
template <typename T>
inline
T *
get_ptr_inline(
               std::vector<T *> & v,
               int const n,
               SRWLOCK * const lock
               )
{
  T * ptr = NULL;

  assert(NULL != lock);
  if (NULL != lock) AcquireSRWLockShared( lock );
  {
    int const n_max = (int)(v.size());
    //assert( (0 <= n) && (n < n_max) );
    if ( (0 <= n) && (n < n_max) )
      {
        ptr = v[n];
      }
    /* if */
  }
  if (NULL != lock) ReleaseSRWLockShared( lock );

  return ptr;
}
/* get_ptr_inline */



//! Sets nth element.
/*!
  Function sets nth element of std::vector to supplied pointer which stores pointer.
  The std::vector is protected by a SRW lock.

  \param v      Pointer to std::vector.
  \param n      Index of element to return.
  \param lock   Pointer to SRW lock.
  \param ptr    Value to set.
  \return Returns true if successfull, false otherwise.
*/
template <typename T>
inline
bool
set_ptr_inline(
               std::vector<T *> & v,
               int const n,
               SRWLOCK * const lock,
               T * const ptr
               )
{
  bool set = false;

  assert(NULL != lock);
  if (NULL != lock) AcquireSRWLockShared( lock );
  {
    int const n_max = (int)(v.size());
    assert( (0 <= n) && (n < n_max) );
    if ( (0 <= n) && (n < n_max) )
      {
        v[n] = ptr;
        set = true;
      }
    /* if */
  }
  if (NULL != lock) ReleaseSRWLockShared( lock );

  return set;
}
/* set_ptr_inline */



/****** NaN AND Inf ******/

/* We define two constants that can be used as double values for NaN. */
static unsigned __int32 const BATCHACQUISITION_sNaN_d[2] = {0xffffffff, 0xfff7ffff}; //!< Double precision signaling NaN bitvalues.
static unsigned __int32 const BATCHACQUISITION_qNaN_d[2] = {0xffffffff, 0xffffffff}; //!< Double quiet signaling NaN bitvalues.
static unsigned __int32 const BATCHACQUISITION__IND_d[2] = {0x00000000, 0xfff80000}; //!< Double precision IND bitvalues.

double const BATCHACQUISITION_sNaN_dv = *( (double *)BATCHACQUISITION_sNaN_d ); //!< Double precision signaling NaN.
double const BATCHACQUISITION_qNaN_dv = *( (double *)BATCHACQUISITION_qNaN_d ); //!< Double quiet signaling NaN.
double const BATCHACQUISITION__IND_dv = *( (double *)BATCHACQUISITION__IND_d ); //!< Double precision IND.


/* We also define two constants that can be used as float values for NaN. */
static unsigned __int32 const BATCHACQUISITION_sNaN_f[1] = {0xffbfffff}; //!< Single precision signaling NaN bitvalues.
static unsigned __int32 const BATCHACQUISITION_qNaN_f[1] = {0xffffffff}; //!< Single quiet signaling NaN bitvalues.
static unsigned __int32 const BATCHACQUISITION__IND_f[1] = {0x7ffc0000}; //!< Single precision IND bitvalues.

float const BATCHACQUISITION_sNaN_fv = *( (float *)BATCHACQUISITION_sNaN_f ); //!< Single precision signaling NaN.
float const BATCHACQUISITION_qNaN_fv = *( (float *)BATCHACQUISITION_qNaN_f ); //!< Single quiet signaling NaN.
float const BATCHACQUISITION__IND_fv = *( (float *)BATCHACQUISITION__IND_f ); //!< Single precision IND.


/* We also have plus and minus infinity. */
static unsigned __int32 const BATCHACQUISITION_pINF_d[2] = {0x00000000, 0x7ff00000}; //!< Double precision positive infinity bitvalues.
static unsigned __int32 const BATCHACQUISITION_nINF_d[2] = {0x00000000, 0xfff00000}; //!< Double precision negative infinity bitvalues.

static unsigned __int32 const BATCHACQUISITION_pINF_f[1] = {0x7f800000}; //!< Single precision positive infinity bitvalues.
static unsigned __int32 const BATCHACQUISITION_nINF_f[1] = {0xff800000}; //!< Single precision negative infinity bitvalues.

double const BATCHACQUISITION_pINF_dv = *( (double *)BATCHACQUISITION_pINF_d ); //!< Double precision positive infinity.
double const BATCHACQUISITION_nINF_dv = *( (double *)BATCHACQUISITION_nINF_d ); //!< Double precision negative infinity.

float const BATCHACQUISITION_pINF_fv = *( (float *)BATCHACQUISITION_pINF_f ); //!< Single precision positive infinity.
float const BATCHACQUISITION_nINF_fv = *( (float *)BATCHACQUISITION_nINF_f ); //!< Single precision negative infinity.



//! Checks if number is NaN.
/*!
  Checks if double precision number is NaN.

  \param x      Double precision number.
  \return Returns true if number is NaN.
*/
inline
bool
isnan_inline(
             double const x
             )
{
  union { unsigned __int64 u; double f; } ieee754;
  ieee754.f = x;
  bool const isnan =
    ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
    ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
  return isnan;
}
/* isnan_inline */



//! Test for NaN or Inf values (double).
/*!
  We test for NaN or Inf values by extracting the exponent. If the
  exponent is all zeros then the number is either NaN or Inf.

  \param x      Double precision number.
  \return Returns true if number is NaN or Inf.
*/
inline
bool
isnanorinf_inline(
                  double const x
                  )
{
  union { unsigned __int64 u; double f; } ieee754;
  ieee754.f = x;
  unsigned __int64 maske = 0x7ff0000000000000;
  bool const isnanorinf = ( maske == (ieee754.u & maske) );
  return isnanorinf;
}
/* isnanorinf_inline */



#endif /* !__BATCHACQUISITION_H */
