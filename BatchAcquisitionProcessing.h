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
  \file   BatchAcquisitionProcessing.h
  \brief  General structured light processing.

  This file contains definitions if data structures and functions for general processing.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#ifndef __BATCHACQUISITIONPROCESSING_H
#define __BATCHACQUISITIONPROCESSING_H


/* The include order may vary so predeclare typedefs. */
struct ImageSet_;
struct ProjectiveGeometry_;


#include "BatchAcquisition.h"
#include "BatchAcquisitionVTK.h"


//! Structure to store image data.
/*!
  This structure holds all images acquired during projection of one structured light sequence.
  It definition almost verbatim repeated from the code by Tomislav Pribanic for backwards compatibility.
  Only difference is addition of the size of the allocated block so memory access past block boundaries
  may be detected and the change of the PixelFormat enumeration.

  The structure contains one continuous memory block where image are sequentially stored.
  Two steps used for data storage are korak for image rows and OneImageSize for images.
  All images must have same the same size and the same pixel type.

  Original values for PixelFormat enumeration and corresponding values of ImageDataType enumeration were:
  1 for 8-bit RGB that corresponds to IDT_8U_RGB;
  2 for 8-bit RGBA that corresponds to IDT_8U_RGBA;
  3 for UYVY 16-bit, 4:2:2 subsampled, that corresponds to IDT_8U_YUV422;
  4 for YUY2 16-bit, 4:2:2 subsampled, that currently has no corresponding new format (use IDT_UNKNOWN);
  5 for 8-bit monochromatic that correspons to IDT_8U_GRAY; and
  6 for 10-bit monochromatic that currently has no corresponding new format (use IDT_UNKNOWN).
*/
typedef
struct ImageSet_
{
  unsigned char * data; //!< Address of the data block.
  int num_images; //!< Number of images in the set.
  int width; //!< Image width in pixels.
  int height; //!< Image height in pixel.
  int row_step; //!< Size of one image row in bytes.
  int image_step; //!< Size of one image in bytes.
  ImageDataType PixelFormat; //!< Pixel format. See ImageDataType enumeration for available pixel formats.

  size_t buffer_size; //!< Size of the allocated contiguous memory block in bytes.
  std::vector<bool> * image_added; //!< A vector indicating positions where images were stored.

  int window_width; //!< Size of the display window in pixels.
  int window_height; //!< Size of the display window in pixels.
  RECT rcScreen; //!< Projector window in desktop coordinates.
  RECT rcWindow; //!< Display window in desktop coordinates.

  int CameraID; //!< ID of the camera.
  int ProjectorID; //!< ID of the projector.

  std::wstring * camera_name; //!< Unique camera identifier.
  std::wstring * projector_name; //! Unique projector identifier.
  std::wstring * acquisition_name; //!< Description of the current acquisition.

  CameraSDK acquisition_method; //!< Flag which indicates what SDK is used.

  //! Constructor.
  ImageSet_();

  //! Blank class variables.
  void Blank(void);

  //! Release allocated memory.
  void Release(void);

  //! Set camera ID.
  void SetCamera(int const, std::wstring * const, CameraSDK const);

  //! Set projector ID.
  void SetProjector(int const, std::wstring * const);

  //! Set acquisition name.
  void SetName(std::wstring * const);

  //! Reallocator.
  bool Reallocate(unsigned int const, unsigned int const, unsigned int const, unsigned int const, size_t const, ImageDataType const);

  //! Add image at specified position.
  bool AddImage(int const, unsigned int const, unsigned int const, unsigned int const, size_t const, ImageDataType const, void const * const);

  //! Add image at specified position.
  bool AddImage(int const, cv::Mat const * const);

  //! Get graylevel image at specified position.
  cv::Mat * GetImageGray(int const);

  //! Get single channel image at specified position.
  cv::Mat * GetImage1C(int const);

  //! Get BGR image at specified position.
  cv::Mat * GetImageBGR(int const);

  //! Reset added images flags.
  bool Reset(void);

  //! Check if we have any data.
  bool HaveAny(void);

  //! Check if all images were added.
  bool HaveAll(void);

  //! Check if at least N images were added.
  bool HaveFirstN(size_t const);

  //! Count added images.
  int CountValid(void);

  //! Check if display window was fullscreen.
  bool IsFullscreen(void);

  //! Check if input images are grayscale.
  bool IsGrayscale(void);

  //! Destructor.
  ~ImageSet_();

} ImageSet;



//! Structure to hold camera parameters.
/*!
  Structure holds projective geometry parameters for either camera or projector.
  The model is ideal pinhole camera. Parameters are split into extrinsic and
  intrinsic parameters. There are four intrinsic parameters, focus and centerpoint pairs,
  and two extrinsic parameters, rotation matrix and camera center.
*/
typedef
struct ProjectiveGeometry_
{
  double fx; //!< Focus along x direction.
  double fy; //!< Focus along y direction.
  double cx; //!< Image center in x direction.
  double cy; //!< Image center in y direction.

  double k0; //!< First parameter for radial distortion; multiplies r^2.
  double k1; //!< Second parameter for radial distortion; multiplies r^4.

  double w; //!< Sensor width in pixels.
  double h; //!< Sensor height in pixels.

  double_a_M34 projection; //!< Full perspective projection matrix.

  double_a_M33 rotation; //!< Rotation matrix from world to camera/projector coordinate system.
  double_a_V3 center; //!< Viewpoint center.

  std::wstring * name; //!< Unique name which identifies camera or projector.

  //! Constructor.
  ProjectiveGeometry_();

  //! Copy constructor.
  ProjectiveGeometry_(const ProjectiveGeometry_ &);

  //! Assingment operator.
  ProjectiveGeometry_ & operator = (const ProjectiveGeometry_ &);

  //! Blank class variables.
  void Blank(void);

  //! Update extrinsic parameters.
  void UpdateExtrinsicParameters(void);

  //! Set screen size.
  void SetScreenSize(double const, double const);

  //! Get view angle.
  double GetViewAngle(void);

  //! Get scale.
  double GetScale(void);

  //! Initialize.
  void Initialize(cv::Mat * const, cv::Mat * const);

  //! Read from file.
  void ReadFromRAWFile(wchar_t const * const, wchar_t const * const);

  //! Read from file.
  void ReadFromRAWFile(char const * const, char const * const);

  //! Read from XML file.
  HRESULT ReadFromXMLFile(wchar_t const * const, wchar_t const * const);

  //! Destructor.
  ~ProjectiveGeometry_();

} ProjectiveGeometry;



/****** LOAD/SAVE cv::Mat ******/

//! Reads cv::Mat from RAW file.
cv::Mat * ReadcvMatFromRAWFile(wchar_t const * const, wchar_t const * const, int * const, int * const);

//! Reads CvMat from RAW file.
CvMat * ReadCvMatFromRAWFile(char const * const, char const * const, int * const, int * const);

//! Writes cv::Mat to RAW file.
int WritecvMatToRAWFile(wchar_t const * const, cv::Mat * const);

//! Writes CvMat to RAW file.
int WriteCvMatToRAWFile(char const * const, CvMat * const);



/****** 3D RECONSTRUCTION ******/

//! Process acquired images.
bool
ProcessAcquiredImages(
                      ImageSet * const,
                      wchar_t const *,
                      wchar_t const *,
                      VTKdisplaythreaddata_ * const,
                      double const,
                      double const
                      );


/****** INLINE FUNCTIONS ******/


//! 16bit data.
/*!
  Union for simple access to upper and lower byte of 16-bit data.
*/
typedef union data_16bit_
{
  UINT16 u16; //!< Unsigned little-endian 16 bit data.
  INT16 s16; //!< Signed little-endian 16 bit data.
  UINT8 u8[2]; //!< In-memory order byte access.
} data_16bit;



//! Return pixel gray value.
/*!
  Returns pixel gray value.

  This function should be used only for debugging or for fetching specific pixel values.
  If an operation is performed on all pixes of a graylevel image then it is more efficient
  to allocate extra memory and convert the whole image instead of fetching the
  gray values through this function.

  \param type   Image data type.
  \param row_ptr        Row pointer.
  \param x      Column index.
*/
float
inline
PixelGrayValueAsFloat_inline(
                             ImageDataType const type,
                             void const * const row_ptr,
                             int const x
                             )
{
  assert(NULL != row_ptr);
  if (NULL == row_ptr) return BATCHACQUISITION_qNaN_fv;

  float const R = 0.298936021293776f;
  float const G = 0.587043074451121f;
  float const B = 0.114020904255103f;

  switch (type)
    {
    case (int)IDT_UNKNOWN: break;
    case (int)IDT_8U_BINARY: return (float)( ( (UINT8 *)row_ptr )[x] );
    case (int)IDT_8U_GRAY: return (float)( ( (UINT8 *)row_ptr )[x] );
    case (int)IDT_10U_GRAY: (float)( ( (UINT16 *)row_ptr )[x] );
    case (int)IDT_12U_GRAY_Packed: break;
    case (int)IDT_16U_GRAY: return (float)( ( (UINT16 *)row_ptr )[x] );

    case (int)IDT_16U_GRAY_BigEndian:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 2 * x;
        data_16bit pixel;
        pixel.u8[0] = ptr_8u[idx + 1];
        pixel.u8[1] = ptr_8u[idx    ];
        float const gray = (float)( pixel.u16 );
        return gray;
      }
      break;

    case (int)IDT_32U_GRAY: return (float)( ( (UINT32 *)row_ptr )[x] );
    case (int)IDT_8S_GRAY: return (float)( ( (INT8 *)row_ptr )[x] );
    case (int)IDT_16S_GRAY: return (float)( ( (INT16 *)row_ptr )[x] );

    case (int)IDT_16S_GRAY_BigEndian:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 2 * x;
        data_16bit pixel;
        pixel.u8[0] = ptr_8u[idx + 1];
        pixel.u8[1] = ptr_8u[idx    ];
        float const gray = (float)( pixel.s16 );
        return gray;
      }
      break;

    case (int)IDT_32S_GRAY: return (float)( ( (INT32 *)row_ptr )[x] );
    case (int)IDT_8U_BayerGR: break;
    case (int)IDT_8U_BayerRG: break;
    case (int)IDT_8U_BayerGB: break;
    case (int)IDT_8U_BayerBG: break;
    case (int)IDT_10U_BayerGR: break;
    case (int)IDT_10U_BayerRG: break;
    case (int)IDT_10U_BayerGB: break;
    case (int)IDT_10U_BayerBG: break;
    case (int)IDT_12U_BayerGR_Packed: break;
    case (int)IDT_12U_BayerRG_Packed: break;
    case (int)IDT_12U_BayerGB_Packed: break;
    case (int)IDT_12U_BayerBG_Packed: break;
    case (int)IDT_16U_BayerGR: break;
    case (int)IDT_16U_BayerRG: break;
    case (int)IDT_16U_BayerGB: break;
    case (int)IDT_16U_BayerBG: break;
    case (int)IDT_16U_BayerGR_BigEndian: break;
    case (int)IDT_16U_BayerRG_BigEndian: break;
    case (int)IDT_16U_BayerGB_BigEndian: break;
    case (int)IDT_16U_BayerBG_BigEndian: break;

    case (int)IDT_8U_RGB:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 3 * x;
        float const gray = R * ptr_8u[idx] + G * ptr_8u[idx + 1] + B * ptr_8u[idx + 2];
        return gray;
      }
      break;

    case (int)IDT_8U_RGB_Planar: break;

    case (int)IDT_8U_RGBA:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 4 * x;
        float const gray = R * ptr_8u[idx] + G * ptr_8u[idx + 1] + B * ptr_8u[idx + 2];
        return gray;
      }
      break;

    case (int)IDT_8U_BGR:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 3 * x;
        float const gray = B * ptr_8u[idx] + G * ptr_8u[idx + 1] + R * ptr_8u[idx + 2];
        return gray;
      }
      break;

    case (int)IDT_8U_BGRA:
      {
        UINT8 const * const ptr_8u = (UINT8 *)row_ptr;
        int const idx = 4 * x;
        float const gray = B * ptr_8u[idx] + G * ptr_8u[idx + 1] + R * ptr_8u[idx + 2];
        return gray;
      }
      break;

    case (int)IDT_8U_YUV411: return (float)( ( (UINT8 *)row_ptr )[((3 * x) / 2) + 1] );
    case (int)IDT_8U_YUV422: return (float)( ( (UINT8 *)row_ptr )[2 * x + 1] );
    case (int)IDT_8U_YUV422_BT601: return 1.16438f * ( (float)( ( (UINT8 *)row_ptr )[2 * x + 1] ) - 16.0f );
    case (int)IDT_8U_YUV422_BT709: return 1.16438f * ( (float)( ( (UINT8 *)row_ptr )[2 * x + 1] ) - 16.0f );
    case (int)IDT_8U_YUV444: return (float)( ( (UINT8 *)row_ptr )[3 * x    ] );
    case (int)IDT_8U_UYV444: return (float)( ( (UINT8 *)row_ptr )[3 * x + 1] );
    default: break;
    }
  /* switch */

  return BATCHACQUISITION_qNaN_fv;
}
/* PixelGrayValueAsFloat_inline */



#endif /* !__BATCHACQUISITIONPROCESSING */
