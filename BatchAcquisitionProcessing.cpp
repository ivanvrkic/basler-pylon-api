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
  \file   BatchAcquisitionProcessing.cpp
  \brief  General structured light processing.

  This file contains functions for general structured light processing.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSING_CPP
#define __BATCHACQUISITIONPROCESSING_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionProcessing.h"
#include "BatchAcquisitionProcessingPhaseShift.h"
#include "BatchAcquisitionProcessingPixelSelector.h"
#include "BatchAcquisitionProcessingDistortion.h"
#include "BatchAcquisitionProcessingTriangulation.h"
#include "BatchAcquisitionImageConversion.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionProcessingXML.h"
#include "BatchAcquisitionProcessingDynamicRange.h"


#pragma warning(push)
#pragma warning(disable: 4005)

#include <shlwapi.h>

#pragma warning(pop)

#pragma comment(lib, "Shlwapi.lib")



/****** IMAGE SET ******/


//! Constructor.
/*!
  Default constructor. It blanks all variables.
*/
ImageSet_::ImageSet_()
{
  this->Blank();
}
/* ImageSet_::ImageSet_ */



//! Blank class variables.
/*!
  Sets all class variables to default values.
*/
void
ImageSet_::Blank(
                 void
                 )
{
  this->data = NULL;
  this->num_images = 0;
  this->width = 0;
  this->height = 0;
  this->row_step = 0;
  this->image_step = 0;
  this->PixelFormat = IDT_UNKNOWN;
  this->buffer_size = 0;
  this->image_added = NULL;
  this->window_width = -1;
  this->window_height = -1;
  this->CameraID = -1;
  this->ProjectorID = -1;
  this->camera_name = NULL;
  this->projector_name = NULL;
  this->acquisition_name = NULL;
  this->acquisition_method = CAMERA_SDK_UNKNOWN;

  ZeroMemory( &(this->rcScreen), sizeof(this->rcScreen) );
  ZeroMemory( &(this->rcWindow), sizeof(this->rcWindow) );
}
/* ImageSet_::Blank */



//! Release allocated memory.
/*!
  Releases allocated memory.
*/
void
ImageSet_::Release(
                   void
                   )
{
  SAFE_FREE(this->data);
  SAFE_DELETE(this->image_added);
  SAFE_DELETE(this->camera_name);
  SAFE_DELETE(this->projector_name);
  SAFE_DELETE(this->acquisition_name);
}
/* ImageSet_::Release */



//! Set camera ID.
/*!
  Sets camera ID and its unique identifier string.

  \param CameraID       ID of the acquisition thread.
  \param name   Unique name of the camera.
*/
void
ImageSet_::SetCamera(
                     int const CameraID,
                     std::wstring * const name,
                     CameraSDK const acquisition_method
                     )
{
  this->CameraID = CameraID;
  this->acquisition_method = acquisition_method;

  if (NULL != name)
    {
      if (NULL == this->camera_name) this->camera_name = new std::wstring();
      assert(NULL != this->camera_name);
      if (NULL != this->camera_name) *(this->camera_name) = *name;
    }
  else
    {
      SAFE_DELETE(this->camera_name);
    }
  /* if */
}
/* ImageSet_::SetCamera */



//! Set projector ID.
/*!
  Sets projector ID and its identifier string.

  \param ProjectorID    ID of the rendering thread.
  \param name   Unique name of the projector.
*/
void
ImageSet_::SetProjector(
                        int const ProjectorID,
                        std::wstring * const name
                        )
{
  this->ProjectorID = ProjectorID;

  if (NULL != name)
    {
      if (NULL == this->projector_name) this->projector_name = new std::wstring();
      assert(NULL != this->projector_name);
      if (NULL != this->projector_name) *(this->projector_name) = *name;
    }
  else
    {
      SAFE_DELETE(this->projector_name);
    }
  /* if */
}
/* ImageSet_::SetProjector */



//! Set acquisition name.
/*!
  Function sets acquisition name.

  \param name   Acquisition name.
*/
void
ImageSet_::SetName(
                   std::wstring * const name
                   )
{
  if (NULL != name)
    {
      if (NULL == this->acquisition_name) this->acquisition_name = new std::wstring();
      assert(NULL != this->acquisition_name);
      if (NULL != this->acquisition_name) *(this->acquisition_name) = *name;
    }
  else
    {
      SAFE_DELETE(this->acquisition_name);
    }
  /* if */
}
/* ImageSet_::SetName */



//! Reallocator.
/*!
  Reallocates memory for image storage if needed.

  \param N      Number of images to store.
  \param width  Width of every image.
  \param height Height of every image.
  \param stride Size of one image row in bytes.
  \param size   Size of one image in bytes.
  \param type   Pixel format; should be one of ImageDatyType enumerations.
  \return Returns true if successfull, false otherwise.
  If false is returned any data that was previously allocated is valid.
*/
bool
ImageSet_::Reallocate(
                      unsigned int const N,
                      unsigned int const width,
                      unsigned int const height,
                      unsigned int const stride,
                      size_t const size,
                      ImageDataType const type
                      )
{
  assert(0 < N);
  if (0 == N) return false;

  assert(0 < width);
  if (0 == width) return false;

  assert(0 < height);
  if (0 == height) return false;

  assert(0 < stride);
  if (0 == stride) return false;

  assert(0 < size);
  if (0 == size) return false;

  assert((size_t)(height * stride) <= size);
  if ((size_t)(height * stride) > size) return false;

  size_t const minimal_image_size = (size_t)( width * height * PixelSizeInBitsFromImageDataType_inline(type) ) / 8;
  assert(minimal_image_size <= size);
  if (minimal_image_size > size) return false;

  /* Compute new buffer size. */
  size_t const buffer_size = N * size;

  /* Reallocate buffer if needed. */
  if (buffer_size > this->buffer_size)
    {
      void * const buffer = realloc(this->data, buffer_size);
      assert(NULL != buffer);
      if (NULL == buffer)
        {
          return false;
        }
      else
        {
          this->data = (unsigned char *)buffer;
          this->buffer_size = buffer_size;
        }
      /* if */
    }
  /* if */

  /* Adjust flags. */
  if (NULL == this->image_added)
    {
      this->image_added = new std::vector<bool>(N, false);
      assert(NULL != this->image_added);
    }
  /* if */

  assert(NULL != this->image_added);
  if ( (NULL != this->image_added) && (N != this->image_added->size()) )
    {
      this->image_added->resize(N, false);
    }
  /* if */

  /* Update class variables. */
  assert(NULL != this->data);
  this->num_images = N;
  this->width = (int)width;
  this->height = (int)height;
  this->row_step = (int)stride;
  this->image_step = (int)size;
  this->PixelFormat = type;
  assert(0 < this->buffer_size);
  assert(buffer_size <= this->buffer_size);

  return true;
}
/* ImageSet_::Reallocate */



//! Add image at specified position.
/*!
  Adds image at specified position.

  \param i      Image position.
  \param width  Width of every image.
  \param height Height of every image.
  \param stride Size of one image row in bytes.
  \param type   Pixel format; should be one of ImageDatyType enumerations.
  \param size   Size of the image in bytes.
  \param data   Pointer to image data.
  \return Returns true if data is copied successfully.
*/
bool
ImageSet_::AddImage(
                    int const i,
                    unsigned int width,
                    unsigned int height,
                    unsigned int stride,
                    size_t const size,
                    ImageDataType const type,
                    void const * const data
                    )
{
  assert( (0 <= i) && (i < this->num_images) );
  if ( (0 > i) || (i >= this->num_images) ) return false;

  assert(width == this->width);
  if (width != this->width) return false;

  assert(height = this->height);
  if (height != this->height) return false;

  assert(stride == this->row_step);
  if (stride != this->row_step) return false;

  assert((int)size <= this->image_step);
  if ((int)size > this->image_step) return false;

  assert(type == this->PixelFormat);
  if (type != this->PixelFormat) return false;

  //assert(NULL != data);
  if (NULL == data) return false;

  void * const slot_i = this->data + i * this->image_step;
  void * const dst = memcpy(slot_i, data, size);
  assert(dst == slot_i);

  if (NULL != this->image_added)
    {
      assert(this->num_images == (int)this->image_added->size());
      (*(this->image_added))[i] = true;
    }
  /* if */

  return true;
}
/* ImageSet_::AddImage */



//! Add image at specified position.
/*!
  Adds image at specified position.

  \param i      Image position.
  \param pImage Pointer to cv::Mat image. Image must be of same format as one used while allocating.
  \return Returns true if data is copied successfully.
*/
bool
ImageSet_::AddImage(
                    int const i,
                    cv::Mat const * const pImage
                    )
{
  assert( (0 <= i) && (i < this->num_images) );
  if ( (0 > i) || (i >= this->num_images) ) return false;

  //assert(NULL != pImage);
  if (NULL == pImage) return false;

  assert(pImage->cols == this->width);
  if (pImage->cols != this->width) return false;

  assert(pImage->rows == this->height);
  if (pImage->rows != this->height) return false;

  assert(pImage->step[0] == this->row_step);
  if (pImage->step[0] != this->row_step) return false;

  size_t image_size = pImage->step[0] * pImage->rows;
  assert(image_size <= (size_t)this->image_step);
  if (image_size > (size_t)this->image_step) return false;

  void * const slot_i = this->data + i * this->image_step;
  void * const dst = memcpy(slot_i, pImage->data, image_size);
  assert(dst == slot_i);

  if (NULL != this->image_added)
    {
      assert(this->num_images == (int)this->image_added->size());
      (*(this->image_added))[i] = true;
    }
  /* if */

  return true;
}
/* ImageSet_::AddImage */



//! Get graylevel image at specified position.
/*!
  Gets graylevel image at position i.

  \param i Image position.
  \return Returns cv::Mat or NULL if image does not exist or cannot be converted.
*/
cv::Mat *
ImageSet_::GetImageGray(
                        int const i
                        )
{
  assert( (0 <= i) && (i < this->num_images) );
  if ( (0 > i) || (i >= this->num_images) ) return NULL;

  // Get starting address.
  void * const src = (void *)( (BYTE *)this->data + this->image_step * i );

  // Create shallow copy is possible.
  cv::Mat * shallow_copy = NULL;
  switch (this->PixelFormat)
    {
    case IDT_8U_BINARY:
    case IDT_8U_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_8UC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_16U_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_16UC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_8S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_8SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_16S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_16SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_32S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_32SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;
    }
  /* switch */

  // Return shallow copy.
  if (NULL != shallow_copy) return shallow_copy;
  SAFE_DELETE( shallow_copy );

  // Make deep copy.
  cv::Mat * const deep_copy = RawBufferToGraycvMat(this->PixelFormat, this->width, this->height, this->row_step, src);
  return deep_copy;
}
/* ImageSet_::GetImageGray */



//! Get single channel image at specified position.
/*!
  Gets single channel image at specified position.
  A single channel image is grayscale or RAW Bayer image, i.e.
  if the underlaying image is in color (RGB, YUV, etc.) than it is converted to grayscale,
  if the underlaying image is Bayer then it is returned as is,
  and if the underlaying image is grayscale then it is returned as is.
  This approach makes operations like phase estimation, dynamic range estimation, and
  texture computation simpler.

  \param i      Image position.
  \return Returns single channel cv::Mat or NULL if image does not exist or cannot be converted.
*/
cv::Mat *
ImageSet_::GetImage1C(
                      int const i
                      )
{
  assert( (0 <= i) && (i < this->num_images) );
  if ( (0 > i) || (i >= this->num_images) ) return NULL;

  // Get starting address.
  void * const src = (void *)( (BYTE *)this->data + this->image_step * i );

  // Create shallow copy is possible.
  cv::Mat * shallow_copy = NULL;
  switch (this->PixelFormat)
    {
    case IDT_8U_BINARY:
    case IDT_8U_GRAY:
    case IDT_8U_BayerGR:
    case IDT_8U_BayerRG:
    case IDT_8U_BayerGB:
    case IDT_8U_BayerBG:
      shallow_copy = new cv::Mat(this->height, this->width, CV_8UC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_16U_GRAY:
    case IDT_16U_BayerGR:
    case IDT_16U_BayerRG:
    case IDT_16U_BayerGB:
    case IDT_16U_BayerBG:
      shallow_copy = new cv::Mat(this->height, this->width, CV_16UC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_8S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_8SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_16S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_16SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;

    case IDT_32S_GRAY:
      shallow_copy = new cv::Mat(this->height, this->width, CV_32SC1, src, this->row_step);
      assert(NULL != shallow_copy);
      break;
    }
  /* switch */

  // Return shallow copy.
  if (NULL != shallow_copy) return shallow_copy;
  SAFE_DELETE( shallow_copy );

  // Make deep copy.
  cv::Mat * const deep_copy = RawBufferTo1CcvMat(this->PixelFormat, this->width, this->height, this->row_step, src);
  return deep_copy;
}
/* ImageSet_::GetImage1C */



//! Get BGR image at specified position.
/*!
  Gets BGR image at position i.

  \param i Image position.
  \return Returns cv::Mat or NULL if image does not exist or cannot be converted.
*/
cv::Mat *
ImageSet_::GetImageBGR(
                       int const i
                       )
{
  assert( (0 <= i) && (i < this->num_images) );
  if ( (0 > i) || (i >= this->num_images) ) return NULL;

  // Get starting address.
  void * const src = (void *)( (BYTE *)this->data + this->image_step * i );

  // Create shallow copy if possible.
  cv::Mat * shallow_copy = NULL;
  switch (this->PixelFormat)
    {
    case IDT_8U_BGR:
      shallow_copy = new cv::Mat(this->height, this->width, CV_8UC3, src, this->row_step);
      assert(NULL != shallow_copy);
      break;
    }
  /* switch */

  // Return shallow copy.
  if (NULL != shallow_copy) return shallow_copy;
  SAFE_DELETE( shallow_copy );

  // Make deep copy.
  cv::Mat * const deep_copy = RawBufferToBGRcvMat(this->PixelFormat, this->width, this->height, this->row_step, src);
  return deep_copy;
}
/* ImageSet_::GetImageBGR */



//! Reset added images flags.
/*!
  Resets added image flags.

  \return Returns true if successfull.
*/
bool
ImageSet_::Reset(
                 void
                 )
{
  //assert(NULL != this->image_added);
  if (NULL == this->image_added) return true;

  size_t const N = this->image_added->size();
  for (size_t i = 0; i < N; ++i) (*(this->image_added))[i] = false;

  assert( (int)N == this->num_images );
  return (int)N == this->num_images;
}
/* ImageSet_::Reset */



//! Check if we have any data.
/*!
  Checks if any data was added.

  \return Returns true if at least one image were added, false otherwise.
*/
bool
ImageSet_::HaveAny(
                   void
                   )
{
  //assert(NULL != this->ptImageStart);
  if (NULL == this->data) return false;

  assert(NULL != this->image_added);
  if (NULL == this->image_added) return false;

  bool have_any = false;

  size_t const N = this->image_added->size();
  assert( N == this->num_images );
  for (size_t i = 0; i < N; ++i) have_any = have_any || (*(this->image_added))[i];

  return have_any;
}
/* ImageSet_::HaveAny */



//! Check if all images were added.
/*!
  Checks if all images were added.

  \return Returns true if all images were added, false otherwise.
*/
bool
ImageSet_::HaveAll(
                   void
                   )
{
  //assert(NULL != this->data);
  if (NULL == this->data) return false;

  assert(NULL != this->image_added);
  if (NULL == this->image_added) return false;

  bool have_all = true;

  size_t const N = this->image_added->size();
  assert( N == this->num_images);
  for (size_t i = 0; i < N; ++i) have_all = have_all && (*(this->image_added))[i];

  return have_all;
}
/* ImageSet_::HaveAll */



//! Check if at least N images were added.
/*!
  Checks if at least N images were added.

  \param N      Expected number of acquired images.
  \return Returns true if N images is acquired, false otherwise.
*/
bool
ImageSet_::HaveFirstN(
                      size_t const N
                      )
{
  //assert(NULL != this->data);
  if (NULL == this->data) return false;

  assert(NULL != this->image_added);
  if (NULL == this->image_added) return false;

  if (N > this->image_added->size()) return false;

  bool have_all = true;
  assert( (int)N <= this->num_images );
  for (size_t i = 0; i < N; ++i) have_all = have_all && (*(this->image_added))[i];

  return have_all;
}
/* ImageSet_::HaveFirstN */



//! Count added images.
/*!
  Counts added images.

  \return Returns number of images added or -1 if error occures.
*/
int
ImageSet_::CountValid(
                      void
                      )
{
  //assert(NULL != this->data);
  if (NULL == this->data) return -1;

  assert(NULL != this->image_added);
  if (NULL == this->image_added) return -1;

  int count = 0;

  size_t const N = this->image_added->size();
  assert( N == this->num_images);
  for (size_t i = 0; i < N; ++i) if (true == (*(this->image_added))[i]) ++count;

  return count;
}
/* ImageSet_::CountValid */



//! Check if display window was fullscreen.
/*!
  Checks if display window used to display the structured light pattern was fullscreen.

  \return Returns true if the window was fullscreen.
*/
bool
ImageSet_::IsFullscreen(
                        void
                        )
{
  bool const is_positive = (0 < this->window_width) && (0 < this->window_height);
  bool const is_valid =
    ( (int)(this->rcWindow.right - this->rcWindow.left) == this->window_width ) &&
    ( (int)(this->rcWindow.bottom - this->rcWindow.top) == this->window_height );
  bool const is_fullscreen =
    ( (int)(this->rcScreen.right - this->rcScreen.left) == this->window_width ) &&
    ( (int)(this->rcScreen.bottom - this->rcScreen.top) == this->window_height );

  return is_positive && is_valid && is_fullscreen;
}
/* ImageSet_::IsFullscreen */



//! Check if recorded images are grayscale.
/*!
  Checks if recorded images are grayscale.

  \return Returns true if images are grayscale.
*/
bool
ImageSet_::IsGrayscale(
                       void
                       )
{
  bool const is_grayscale = ImageDataTypeIsGrayscale_inline(this->PixelFormat);
  return is_grayscale;
}
/* ImageSet_::IsGrayscale */



//! Destructor.
/*!
  Blanks class variables.
*/
ImageSet_::~ImageSet_()
{
  this->Release();
  this->Blank();
}
/* ImageSet_::~ImageSet_ */



/****** PROJECTIVE GEOMETRY ******/

//! Constructor.
/*!
  Default constructor. It blanks all variables.
*/
ProjectiveGeometry_::ProjectiveGeometry_()
{
  this->Blank();
}
/* ProjectiveGeometry_::ProjectiveGeometry_ */


//! Copy constructor.
/*!
  Creates a copy of geometry.

  \param P Reference to object to copy.
*/
ProjectiveGeometry_::ProjectiveGeometry_(
                                         const ProjectiveGeometry_ & P
                                         )
{
  this->Blank();

  this->fx = P.fx;
  this->fy = P.fy;
  this->cx = P.cx;
  this->cy = P.cy;
  this->k0 = P.k0;
  this->k1 = P.k1;

  this->w = P.w;
  this->h = P.h;

  memcpy( &(this->projection), &(P.projection), sizeof(this->projection) );
  memcpy( &(this->rotation), &(P.rotation), sizeof(this->rotation) );
  memcpy( &(this->center), &(P.center), sizeof(this->center) );

  if (NULL != P.name)
    {
      if (NULL != this->name)
        {
          *(this->name) = *(P.name);
        }
      else
        {
          this->name = new std::wstring( *(P.name) );
        }
      /* if */
    }
  /* if */
}
/* ProjectiveGeometry_::ProjectiveGeometry_ */



//! Assingment operator.
/*!
  Creates a copy of geometry.

  \param P Reference to an object to copy.
*/
ProjectiveGeometry_ &
ProjectiveGeometry_::operator = (
                                 const ProjectiveGeometry_ & P
                                 )
{
  /* Return immediately if requested to copy itself. */
  if (this == &P)
    {
      assert( this->name == P.name );
      return * this;
    }
  /* if */

  this->fx = P.fx;
  this->fy = P.fy;
  this->cx = P.cx;
  this->cy = P.cy;
  this->k0 = P.k0;
  this->k1 = P.k1;

  this->w = P.w;
  this->h = P.h;

  memcpy( &(this->projection), &(P.projection), sizeof(this->projection) );
  memcpy( &(this->rotation), &(P.rotation), sizeof(this->rotation) );
  memcpy( &(this->center), &(P.center), sizeof(this->center) );

  if (NULL != P.name)
    {
      if (NULL != this->name)
        {
          *(this->name) = *(P.name);
        }
      else
        {
          this->name = new std::wstring( *(P.name) );
        }
      /* if */
    }
  else
    {
      SAFE_DELETE(this->name);
    }
  /* if */

  return * this;
}
/* ProjectiveGeometry_::operator = */



//! Blank class variables.
/*!
  Sets all class variables to default values.
*/
void
ProjectiveGeometry_::Blank(void)
{
  this->fx = 1.0;
  this->fy = 1.0;
  this->cx = 0.0;
  this->cy = 0.0;
  this->k0 = 0.0;
  this->k1 = 0.0;

  this->w = BATCHACQUISITION_qNaN_dv;
  this->h = BATCHACQUISITION_qNaN_dv;

  ZeroMemory( &(this->projection), sizeof(this->projection) );
  this->projection[0][0] = 1.0;
  this->projection[1][1] = 1.0;
  this->projection[2][3] = 1.0;

  ZeroMemory( &(this->rotation), sizeof(this->rotation) );
  this->rotation[0][0] = 1.0;
  this->rotation[1][1] = 1.0;
  this->rotation[2][2] = 1.0;

  ZeroMemory( &(this->center), sizeof(this->center) );

  this->name = NULL;
}
/* ProjectiveGeometry_::Blank */



//! Update extrinsic parameters.
/*!
  Function computes camera extrinsic paremeters from camera intrinsic parameters and full perspective projection matrix.
*/
void
ProjectiveGeometry_::UpdateExtrinsicParameters(
                                               void
                                               )
{
  double_a_M33 K_data = {{this->fx, 0.0, this->cx}, {0.0, this->fx, this->cy}, {0.0, 0.0, 1.0}};

  // Make shallow cv::Mat headers.
  cv::Mat K(3, 3, CV_64F, &(K_data[0][0]), 3 * sizeof(double));
  cv::Mat KR(3, 3, CV_64F, &(this->projection[0][0]), 4 * sizeof(double));
  cv::Mat P4(3, 1, CV_64F, &(this->projection[0][3]), 4 * sizeof(double));
  cv::Mat R(3, 3, CV_64F, &(this->rotation[0][0]), 3 * sizeof(double));
  cv::Mat C(3, 1, CV_64F, &(this->center[0]), 1 * sizeof(double));

  // Compute rotation matrix.
  R = K.inv(cv::DECOMP_SVD) * KR;

  // Compute camera position.
  C = ( -KR.inv(cv::DECOMP_SVD) ) * P4;
}
/* ProjectiveGeometry_::UpdateExtrinsicParameters */



//! Set screen size.
/*!
  Set screen size.

  \param w      Screen width in pixels.
  \param h      Screen height in pixels.
*/
void
ProjectiveGeometry_::SetScreenSize(
                                   double const w,
                                   double const h
                                   )
{
  this->w = w;
  this->h = h;
  assert( false == isnan_inline(this->w) );
  assert( false == isnan_inline(this->h) );
}
/* ProjectiveGeometry_::SetScreenSize */



//! Get view angle.
/*!
  Returns camera view angle.

  \return Camera view angle in radians or NaN if view angle cannot be computed.
*/
double
ProjectiveGeometry_::GetViewAngle(
                                  void
                                  )
{
  if ( true == isnan_inline(this->w) ) return BATCHACQUISITION_qNaN_dv;
  if ( true == isnan_inline(this->h) ) return BATCHACQUISITION_qNaN_dv;

  double const dx1 = abs(this->cx - 1.0);
  double const dx2 = abs(this->cx - this->w);
  double const dx = (dx1 > dx2)? dx1 : dx2;
  double const angx = 2.0 * atan(dx / this->fx);
  assert(0.0 <= angx);

  double const dy1 = abs(this->cy - 1.0);
  double const dy2 = abs(this->cy - this->h);
  double const dy = (dy1 > dy2)? dy1 : dy2;
  double const angy = 2.0 * atan(dy / this->fy);
  assert(0.0 <= angy);

  double const ang = (angx > angy)? angx : angy;

  return ang;
}
/* ProjectiveGeometry_::GetViewAngle */



//! Get scale.
/*!
  Returns camera scale in pixels.

  \return Camera scale in pixels or NaN if the scale cannot be computed.
*/
double
ProjectiveGeometry_::GetScale(
                              void
                              )
{
  if ( true == isnan_inline(this->w) ) return BATCHACQUISITION_qNaN_dv;
  if ( true == isnan_inline(this->h) ) return BATCHACQUISITION_qNaN_dv;

  double const dx1 = abs(this->cx - 1.0);
  double const dx2 = abs(this->cx - this->w);
  double const dx = (dx1 > dx2)? dx1 : dx2;
  double const scalex = 2.0 * dx;
  assert(0.0 <= scalex);

  double const dy1 = abs(this->cy - 1.0);
  double const dy2 = abs(this->cy - this->h);
  double const dy = (dy1 > dy2)? dy1 : dy2;
  double const scaley = 2.0 * dy;
  assert(0.0 <= scaley);

  double const scale = (scalex > scaley)? scalex : scaley;

  return scale;
}
/* ProjectiveGeometry_::GetScale */



//! Initialize.
/*!
  Copies data from two matrices holding intrinsic camera parameters and a full projection matrix.

  \param intParam   Intrinsic camera parameters stored in 6 element vector.
  \param proMatrix   Full perspective projection matrix; stored as 3x4 matrix.
*/
void
ProjectiveGeometry_::Initialize(
                                cv::Mat * const intParam,
                                cv::Mat * const proMatrix
                                )
{
  int const intType = CV_MAT_DEPTH(intParam->type());
  assert( (NULL != intParam) && (NULL != intParam->data) && (CV_64F == intType) );
  if ( (NULL != intParam) && (NULL != intParam->data) && (CV_64F == intType) )
    {
      double const * const IP = (double *)(intParam->data);
      int const size = intParam->rows * intParam->cols;
      if (0 < size) this->fx = IP[0];
      if (1 < size) this->fy = IP[1];
      if (2 < size) this->cx = IP[2];
      if (3 < size) this->cy = IP[3];
      if (4 < size) this->k0 = IP[4];
      if (5 < size) this->k1 = IP[5];
    }
  /* if */

  int const proType = CV_MAT_DEPTH(proMatrix->type());
  assert( (NULL != proMatrix) && (NULL != proMatrix->data) && (CV_64F == proType) );
  if ( (NULL != proMatrix) && (NULL != proMatrix->data) && (CV_64F == proType) )
    {
      int const size = proMatrix->rows * proMatrix->cols;
      assert(12 == size);
      if (12 == size)
        {
          double const * const PM = (double *)(proMatrix->data);
          this->projection[0][0] = PM[ 0];
          this->projection[0][1] = PM[ 1];
          this->projection[0][2] = PM[ 2];
          this->projection[0][3] = PM[ 3];
          this->projection[1][0] = PM[ 4];
          this->projection[1][1] = PM[ 5];
          this->projection[1][2] = PM[ 6];
          this->projection[1][3] = PM[ 7];
          this->projection[2][0] = PM[ 8];
          this->projection[2][1] = PM[ 9];
          this->projection[2][2] = PM[10];
          this->projection[2][3] = PM[11];
        }
      /* if */
    }
  /* if */

  this->UpdateExtrinsicParameters();
}
/* ProjectiveGeometry_::Initialize */



//! Read from file.
/*!
  Reads geometry data from raw file.

  \param intName        File storing intrinsic camera parameters.
  \param proName        File storing perspective projection matrix.
*/
void
ProjectiveGeometry_::ReadFromRAWFile(
                                     wchar_t const * const intName,
                                     wchar_t const * const proName
                                     )
{
  int intCols = 0;
  int intRows = 0;
  cv::Mat * intParam = ReadcvMatFromRAWFile(intName, L"double", &intCols, &intRows);
  assert(NULL != intParam);
  assert(6 <= intCols * intRows);

  int proCols = 0;
  int proRows = 0;
  cv::Mat * proMatrix = ReadcvMatFromRAWFile(proName, L"double", &proCols, &proRows);
  assert(NULL != proMatrix);
  assert( (4 == proCols) && (3 == proRows) );

  this->Initialize(intParam, proMatrix);

  SAFE_DELETE( intParam );
  SAFE_DELETE( proMatrix );
}
/* ProjectiveGeometry_::ReadFromRAWFile */



//! Read from file.
/*!
  Reads geometry data from raw file.

  \param intName_in        File storing intrinsic camera parameters.
  \param proName_in        File storing perspective projection matrix.
*/
void
ProjectiveGeometry_::ReadFromRAWFile(
                                     char const * const intName_in,
                                     char const * const proName_in
                                     )
{
  std::wstring intName = L"";
  for (int i = 0; 0 != intName_in[i]; ++i) intName.push_back( intName_in[i] );

  std::wstring proName = L"";
  for (int i = 0; 0 != proName_in[i]; ++i) proName.push_back( proName_in[i] );

  this->ReadFromRAWFile(intName.c_str(), proName.c_str());
}
/* ReadFromRAWFile */



//! Read from XML file.
/*!
  Read geometry data from XML file.

  \param filename       Input XML file.
  \param name           Camera or projector name.
*/
HRESULT
ProjectiveGeometry_::ReadFromXMLFile(
                                     wchar_t const * const filename,
                                     wchar_t const * const name
                                     )
{
  assert(NULL != filename);
  if (NULL == filename) return E_POINTER;

  assert(NULL != name);
  if (NULL == name) return E_POINTER;

  HRESULT hr = S_OK;

  IStream * pFileStream = NULL;
  IXmlReader * pReader = NULL;

  if ( SUCCEEDED(hr) )
    {
      hr = SHCreateStreamOnFileEx(filename, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &pFileStream);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pReader->SetInput(pFileStream);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  // Assume reading from XML failed.
  bool resolution_read = false;
  bool intrinsics_read = false;
  bool extrinsics_read = false;
  bool projection_matrix_read = false;

  cv::Mat resParam(1, 3, CV_64F);
  cv::Mat intParam(1, 6, CV_64F);
  cv::Mat extParam(1, 6, CV_64F);
  cv::Mat proMatrix(3, 4, CV_64F);

  XmlNodeType nodeType = XmlNodeType_None;
  HRESULT have_node = S_OK;
  while (S_OK == have_node)
    {
      have_node = pReader->Read(&nodeType);
      if (S_OK != have_node) break;
      if (XmlNodeType_Element != nodeType) continue;

      WCHAR const * pStartName = NULL;
      hr = pReader->GetLocalName(&pStartName, NULL);
      assert(S_OK == hr);

      UINT start_depth = 0;
      hr = pReader->GetDepth(&start_depth);
      assert(S_OK == hr);

      bool const is_camera = ( 0 == _wcsicmp(L"camera", pStartName) );
      bool const is_projector = ( 0 == _wcsicmp(L"projector", pStartName) );

      if ( (false == is_camera) && (false == is_projector) ) continue;

      HRESULT const have_attribute = pReader->MoveToFirstAttribute();
      if ( !SUCCEEDED(have_attribute ) ) continue;

      WCHAR const * pAttribute = NULL;
      hr = pReader->GetLocalName(&pAttribute, NULL);
      assert(S_OK == hr);

      WCHAR const * pValue = NULL;
      hr = pReader->GetValue(&pValue, NULL);
      assert( SUCCEEDED(hr) );

      bool const is_match = (0 == _wcsicmp(pValue, name));
      if (false == is_match) continue;

      bool break_while_loop = false;
      XmlNodeType inside_node_type = XmlNodeType_None;
      while ( S_OK == (have_node = pReader->Read(&inside_node_type)) )
        {
          switch (inside_node_type)
            {
            case XmlNodeType_Element:
              {
                WCHAR const * pName = NULL;
                hr = pReader->GetLocalName(&pName, NULL);
                assert(S_OK == hr);

                UINT depth = 0;
                hr = pReader->GetDepth(&depth);
                assert(S_OK == hr);

                if (0 == _wcsicmp(L"resolution", pName))
                  {
                    assert(false == resolution_read);
                    HRESULT const get_resolution = ProcessingXMLParseResolution(pReader, depth, (double *)resParam.data);
                    resolution_read = SUCCEEDED(get_resolution);
                  }
                /* if */

                if (0 == _wcsicmp(L"intrinsics", pName))
                  {
                    assert(false == intrinsics_read);
                    HRESULT const get_intrinsics = ProcessingXMLParseIntrinsics(pReader, depth, (double *)intParam.data);
                    intrinsics_read = SUCCEEDED(get_intrinsics);
                  }
                /* if */

                if (0 == _wcsicmp(L"extrinsics", pName))
                  {
                    assert(false == extrinsics_read);
                    HRESULT const get_extrinsics = ProcessingXMLParseExtrinsics(pReader, depth, (double *)extParam.data);
                    extrinsics_read = SUCCEEDED(get_extrinsics);
                  }
                /* if */

                if (0 == _wcsicmp(L"projection_matrix", pName))
                  {
                    assert(false == projection_matrix_read);
                    HRESULT const get_projection_matrix = ProcessingXMLParseProjectionMatrix(pReader, depth, (double *)proMatrix.data);
                    projection_matrix_read = SUCCEEDED(get_projection_matrix);
                  }
                /* if */
              }
              break;

            case XmlNodeType_EndElement:
              {
                WCHAR const * pEndName = NULL;
                hr = pReader->GetLocalName(&pEndName, NULL);
                assert(S_OK == hr);

                UINT end_depth = 0;
                hr = pReader->GetDepth(&end_depth);
                assert(S_OK == hr);

                break_while_loop = (start_depth + 1 == end_depth) && (0 == _wcsicmp(pStartName, pEndName));
              }
              break;
            }
          /* switch */

          if (true == break_while_loop) break;
        }
      /* while */
    }
  /* while */

  SAFE_RELEASE(pReader);
  SAFE_RELEASE(pFileStream);

  if ( (true == intrinsics_read) && (true == projection_matrix_read) && (true == resolution_read) )
    {
      SAFE_DELETE(this->name);
      this->name = new std::wstring(name);

      this->Initialize(&intParam, &proMatrix);

      this->w = resParam.at<double>(0, 0);
      this->h = resParam.at<double>(0, 1);
    }
  else
    {
      hr = E_FAIL;
    }
  /* if */

  return hr;
}
/* ProjectiveGeometry_::ReadFromXMLFile */



//! Destructor.
/*!
  Blanks class variables.
*/
ProjectiveGeometry_::~ProjectiveGeometry_()
{
  SAFE_DELETE( this->name );
  this->Blank();
}
/* ProjectiveGeometry_::~ProjectiveGeometry_ */



/****** LOAD/SAVE cv::Mat ******/


//! OpenCV depth from string.
/*!
  Returns OpenCV data depth from string containing a standard C/C++ or Windows datatype.
  Function is case-insensitive.

  \param datatype       String describing the datatype.
  \return Returns one of OpenCV supported datatypes or CV_USRTYPE1 if input datatype is not supported.
*/
int
DepthFromDatatypeString_inline(
                               wchar_t const * const datatype
                               )
{
  int depth = CV_USRTYPE1;

  assert(NULL != datatype);
  if (NULL == datatype) return depth;

  if (0 == _wcsicmp(datatype, L"UINT8")) depth = CV_8U; // Windows specific.
  if (0 == _wcsicmp(datatype, L"unsigned char"))
    {
      assert(1 == sizeof(unsigned char));
      depth = CV_8U;
    }
  /* if */
  if (0 == _wcsicmp(datatype, L"__int8")) depth = CV_8S; // MSVC specific.
  if (0 == _wcsicmp(datatype, L"INT8")) depth = CV_8S; // Windows specific.
  if (0 == _wcsicmp(datatype, L"signed char"))
    {
      assert(1 == sizeof(signed char));
      depth = CV_8U;
    }
  /* if */
  if (0 == _wcsicmp(datatype, L"UINT16")) depth = CV_16U; // Windows specific.
  if (0 == _wcsicmp(datatype, L"unsigned short"))
    {
      assert(2 == sizeof(unsigned short));
      depth = CV_16U;
    }
  /* if */
  if (0 == _wcsicmp(datatype, L"__int16")) depth = CV_16S; // MSVC specific.
  if (0 == _wcsicmp(datatype, L"INT16")) depth = CV_16S; // Windows specific.
  if (0 == _wcsicmp(datatype, L"signed short"))
    {
      assert(2 == sizeof(signed short));
      depth = CV_16S;
    }
  /* if */
  if (0 == _wcsicmp(datatype, L"__int32")) depth = CV_32S; // MSVC specific.
  if (0 == _wcsicmp(datatype, L"INT32")) depth = CV_32S; // Windows specific.
  if (0 == _wcsicmp(datatype, L"signed int"))
    {
      assert(4 == sizeof(signed short));
      depth = CV_32S;
    }
  /* if */
  if (0 == _wcsicmp(datatype, L"float")) depth = CV_32F;
  if (0 == _wcsicmp(datatype, L"double")) depth = CV_64F;

  return depth;
}
/* DepthFromDatatypeString_inline */



//! Size from OpenCV depth.
/*!
  Returns size of OpenCV datatype in bytes.

  \param depth   OpenCV datatype.
  \return Returns size in bytes or 0 if size is unknown.
*/
size_t
inline
SizeFromDepth_inline(
                     int const depth
                     )
{
  if (depth == CV_8U) return 1;
  if (depth == CV_8S) return 1;
  if (depth == CV_16U) return 2;
  if (depth == CV_16S) return 2;
  if (depth == CV_32S) return 4;
  if (depth == CV_32F) return 4;
  if (depth == CV_64F) return 8;
  return 0;
}
/* SizeFromDepth_inline */



//! Reads cv::Mat from RAW file.
/*!
  Reads cv::Mat from RAW binary file.
  The file is structured as follows:
  1) first 4 bytes encode the number of columns in the matrix as little endian 32bit integer,
  2) next 4 bytes encode the number of rows in the matrix as little endian 32bit integer,
  3) remaining bytes contain matrix data.

  \see CVTypeFromDatatypeString_inline

  \param filename       Filename where the data is stored.
  \param datatype       Datatype string, e.g. INT8, UINT8,... etc.
  \param cols_out       Address where number of columns will be stored. May be NULL.
  \param rows_out       Address where number of rows will be stored. May be NULL.
  \return Returns pointer to cv::Mat or NULL if data cannot be read.
*/
cv::Mat *
ReadcvMatFromRAWFile(
                     wchar_t const * const filename,
                     wchar_t const * const datatype,
                     int * const cols_out = NULL,
                     int * const rows_out = NULL
                     )
{
  cv::Mat * matrix = NULL;

  assert(NULL != filename);
  if (NULL == filename) return matrix;

  // Get datatype.
  int const depth = DepthFromDatatypeString_inline(datatype);

  // Try to open file.
  FILE * fid = NULL;
  errno_t const open_raw = _wfopen_s(&fid, filename, L"rb");
  assert( (0 == open_raw) && (NULL != fid) );
  if ( (0 != open_raw) || (NULL == fid) ) return matrix;

  // Get file size.
  int const seek_end = fseek(fid, 0, SEEK_END);
  assert( 0 == seek_end );

  long const file_size = ftell(fid);
  int const header_size = 2 * sizeof(int);
  int const data_size = file_size - header_size;

  int const seek_set = fseek(fid, 0, SEEK_SET);
  assert( 0 == seek_set );

  // Try to load header.
  assert( header_size < file_size );
  if ( header_size >= file_size) goto ReadcvMatFromRAWFile_EXIT;

  int matrix_size[2] = {-1, -1};
  size_t const read_header = fread(&matrix_size, header_size, 1, fid);
  assert(1 == read_header);
  assert( (0 < matrix_size[0]) & (0 < matrix_size[1]) );
  if ( (0 >= matrix_size[0]) || (0 >= matrix_size[1]) ) goto ReadcvMatFromRAWFile_EXIT;

  // Check if file header and datatype are consistent.
  int const element_size = (int)SizeFromDepth_inline(depth);
  int const row_size = element_size * matrix_size[0];
  bool const data_valid = (matrix_size[1] * row_size == data_size);
  assert(true == data_valid);
  if (true != data_valid) goto ReadcvMatFromRAWFile_EXIT;

  // Allocate cv::Mat.
  matrix = new cv::Mat(matrix_size[1], matrix_size[0], CV_MAKETYPE(depth, 1));
  assert(NULL != matrix);

  // Load data.
  if (NULL != matrix)
    {
      assert( matrix_size[0] == matrix->cols );
      assert( matrix_size[1] == matrix->rows );
      assert( NULL != matrix->data );
      assert( (int)(matrix->step[0]) * matrix->rows >= data_size );
      if ( element_size * matrix_size[0] == (int)(matrix->step[0]) )
        {
          // cv::Mat is densly packed.
          size_t const read_all = fread(matrix->data, data_size, 1, fid);
          assert(1 == read_all);
        }
      else
        {
          // Rows are aligned.
          for (int i = 0; i < matrix_size[1]; ++i)
            {
              size_t const read_row = fread(matrix->data + i * matrix->step[0], row_size, 1, fid);
              assert(1 == read_row);
            }
          /* for */
        }
      /* if */
    }
  /* if */

  if (NULL != cols_out) *cols_out = matrix->cols;
  if (NULL != rows_out) *rows_out = matrix->rows;

 ReadcvMatFromRAWFile_EXIT:

  // Close file.
  int const closed = fclose(fid);
  assert(0 == closed);
  if (0 == closed) fid = NULL;

  return matrix;
}
/* ReadcvMatFromRAWFile */



//! Reads CvMat from RAW file.
/*!
  Reads CvMat from RAW binary file.

  \see ReadcvMatFromRAWFile

  \param filename_in       Filename where the data is stored.
  \param datatype_in       Datatype string, e.g. INT8, UINT8,... etc.
  \param cols_out       Address where number of columns will be stored. May be NULL.
  \param rows_out       Address where number of rows will be stored. May be NULL.
  \return Returns pointer to cv::Mat or NULL if data cannot be read.
*/
CvMat *
ReadCvMatFromRAWFile(
                     char const * const filename_in,
                     char const * const datatype_in,
                     int * const cols_out = NULL,
                     int * const rows_out = NULL
                     )
{
  assert(NULL != filename_in);
  if (NULL == filename_in) return NULL;

  assert(NULL != datatype_in);
  if (NULL == datatype_in) return NULL;

  std::wstring filename = L"";
  for (int i = 0; 0 != filename_in[i]; ++i) filename.push_back( filename_in[i] );

  std::wstring datatype = L"";
  for (int i = 0; 0 != datatype_in[i]; ++i) datatype.push_back( datatype_in[i] );

  cv::Mat * matrix_in = ReadcvMatFromRAWFile(filename.c_str(), datatype.c_str(), cols_out, rows_out);
  assert(NULL != matrix_in);
  if (NULL == matrix_in) return NULL;

  CvMat matrix_tmp = * matrix_in;
  CvMat * const matrix_out = cvCloneMat(&matrix_tmp);
  assert(NULL != matrix_out);

  SAFE_DELETE(matrix_in);

  return matrix_out;
}
/* ReadCvMatFromRAWFile */



//! Writes cv::Mat to RAW file.
/*!
  Writes cv::Mat to RAW file.
  Only 1-channel data is supported.

  For file structure see ReadcvMatFromRAWFile.

  \see ReadcvMatFromRAWFile

  \param filename       Filename where the matrix should be dumped.
  \param matrix Pointer to cv::Mat object.
  \return Returns number of bytes written or -1 if unsuccessfull.
*/
int
WritecvMatToRAWFile(
                    wchar_t const * const filename,
                    cv::Mat * const matrix
                    )
{
  int result = -1;
  size_t bytes_written = 0;

  assert(NULL != filename);
  if (NULL == filename) return result;

  assert(NULL != matrix);
  if (NULL == matrix) return result;

  assert(NULL != matrix->data);
  if (NULL == matrix->data) return result;

  assert(0 < matrix->rows);
  if (0 >= matrix->rows) return result;

  assert(0 < matrix->cols);
  if (0 >= matrix->cols) return result;

  // Get datatype and number of channels.
  int const type = matrix->type();
  int const depth = CV_MAT_DEPTH(type);
  int const cn = CV_MAT_CN(type);

  // Only one channel data is supported.
  //assert(1 == cn);
  //if (1 != cn) return result;

  // Try to open file.
  FILE * fid = NULL;
  errno_t const open_raw = _wfopen_s(&fid, filename, L"wb");
  assert( (0 == open_raw) && (NULL != fid) );
  if ( (0 != open_raw) || (NULL == fid) ) return result;

  // Store width and height.
  int const matrix_size[2] = {matrix->cols * cn, matrix->rows};
  size_t const header_size = 2 * sizeof(int);

  size_t const write_header = fwrite(matrix_size, header_size, 1, fid);
  assert(1 == write_header);
  if (1 != write_header) goto WritecvMatToRAWFile_EXIT;

  bytes_written += write_header * header_size;

  // Store data.
  int const element_size = (int)SizeFromDepth_inline(depth);
  int const row_size = element_size * matrix_size[0];
  int const data_size = row_size * matrix_size[1];
  if ( row_size == matrix->step[0] )
    {
      // cv::Mat is densly packed.
      size_t const write_all = fwrite(matrix->data, data_size, 1, fid);
      assert(1 == write_all);
      if (1 != write_all) goto WritecvMatToRAWFile_EXIT;

      bytes_written += write_all * data_size;
    }
  else
    {
      // Rows are aligned.
      for (int i = 0; i < matrix_size[1]; ++i)
        {
          size_t const write_row = fwrite(matrix->data + i * matrix->step[0], row_size, 1, fid);
          assert(1 == write_row);
          if (1 != write_row) goto WritecvMatToRAWFile_EXIT;

          bytes_written += write_row * row_size;
        }
      /* for */
    }
  /* if */

  result = (int)bytes_written;


 WritecvMatToRAWFile_EXIT:

  // Close file.
  int const closed = fclose(fid);
  assert(0 == closed);
  if (0 == closed) fid = NULL;

  return result;
}
/* WritecvMatToRAWFile */



//! Writes CvMat to RAW file.
/*!
  Writes CvMat to RAW file. Only 1-channel data is supported.

  \see WritecvMatToRAWFile

  \param filename_in       Filename where the matrix should be dumped.
  \param matrix_in   Pointer to CvMat structure.
  \return Returns number of bytes written or -1 if unsuccessfull.
*/
int
WriteCvMatToRAWFile(
                    char const * const filename_in,
                    CvMat * const matrix_in
                    )
{
  assert(NULL != matrix_in);
  if (NULL == matrix_in) return -1;

  std::wstring filename = L"";
  for (int i = 0; 0 != filename_in[i]; ++i) filename.push_back( filename_in[i] );

  cv::Mat matrix = cv::cvarrToMat(matrix_in);
  int const result = WritecvMatToRAWFile(filename.c_str(), &matrix);

  return result;
}
/* WritecvMatToRAWFile */



/****** 3D RECONSTRUCTION ******/

//! Process acquired images.
/*!
  Function processes all acquired images, computes 3D point cloud reconstruction, and pushes
  computed point cloud to VTK for visualization.

  \param AllImages       Pointer to structure holding all acquired images.
  \param method          Type of SL used.
  \param fname_geometry  Filename of XML configuration which holds projector and camera geometry.
  \param pWindowVTK      Pointer to VTK visualization window.
  \param rel_thr         Relative threshold to determine illuminated pixels. Must be in [0,1] range.
  \param dst2_thr        Absolute threshold to determine quality of 3D reconstruction. Usually in mm. Should be positive.
*/
bool
ProcessAcquiredImages(
                      ImageSet * const AllImages,
                      wchar_t const * method,
                      wchar_t const * fname_geometry,
                      VTKdisplaythreaddata * const pWindowVTK,
                      double const rel_thr,
                      double const dst2_thr
                      )
{
  assert(NULL != AllImages);
  if (NULL == AllImages) return false;

  assert(NULL != method);
  if (NULL == method) return false;

  assert(NULL != fname_geometry);
  if (NULL == fname_geometry) return false;

  // Get projector and camera ID.
  int const CameraID = AllImages->CameraID;
  int const ProjectorID = AllImages->ProjectorID;

  assert( (0 <= CameraID) && (0 <= ProjectorID) );
  if ( (0 > CameraID) || (0 > ProjectorID) ) return false;

  // Pre-define variables.

  DEBUG_TIMER * const debug_timer = DebugTimerInit(); // Debug timer.

  ProjectiveGeometry camera; // Camera geometry.
  ProjectiveGeometry projector; // Projector geometry.

  cv::Mat * abs_phase_col = NULL; // Unwrapped normalized phase image for projector column.
  cv::Mat * abs_phase_col_distance = NULL; // Distance to constellation for projector column.
  cv::Mat * abs_phase_col_order = NULL; // Order of unwrapped phase for projector column.
  cv::Mat * abs_phase_col_deviation = NULL; // Standard deviation of unwrapped phase  for projector column.
  cv::Mat * abs_phase_row = NULL; // Unwrapped normalized phase image for projector row.
  cv::Mat * abs_phase_row_distance = NULL; // Distance to constellation for projector row.
  cv::Mat * abs_phase_row_order = NULL; // Order of unwrapped phase for projector row.
  cv::Mat * abs_phase_row_deviation = NULL; // Standard deviation of unwrapped phase for projector row.
  cv::Mat * abs_phase_distance = NULL; // Combined distance to constellation.
  cv::Mat * abs_phase_deviation = NULL; // Combined phase deviation.
  cv::Mat * dynamic_range = NULL; // Lowest observed dynamic range for each pixel.
  cv::Mat * crd_x_image = NULL; // Image column indices for valid pixels.
  cv::Mat * crd_y_image = NULL; // Image row indices for valid pixels.
  cv::Mat * range_image = NULL; // Dynamic range for valid pixels.
  cv::Mat * crd_x_camera = NULL; // Undistorted camera x coordinate.
  cv::Mat * crd_y_camera = NULL; // Undistorted camera y coordinate.
  cv::Mat * projector_col = NULL; // Index of projector column.
  cv::Mat * projector_row = NULL; // Index of projector row.
  cv::Mat * projector_col_est = NULL; // Estimated projector column.
  cv::Mat * projector_row_est = NULL; // Estimated projector row.
  cv::Mat * crd_x_projector = NULL; // Undistorted projector x coordinate.
  cv::Mat * crd_y_projector = NULL; // Undistorted projector y coordinate.
  cv::Mat * x_3D = NULL; // Reconstructed x coordinate.
  cv::Mat * y_3D = NULL; // Reconstructed y coordinate.
  cv::Mat * z_3D = NULL; // Reconstructed z coordinate.
  cv::Mat * dst2_3D = NULL; // Distance between rays from two views.
  cv::Mat * points_3D = NULL; // Final set of 3D points.
  cv::Mat * colors_3D = NULL; // Point color information.
  cv::Mat * data_3D = NULL; // Additional point data.

  cv::Mat * texture = NULL; // Texture image data.
  int texture_n = 0; // Number of summed textures.
  int texture_idx = -1; // Texture image index.

  bool failed = false; // Assume success.

  // Load projective geometry for camera and projector.
  {
    int const count = Debugfwprintf(stderr, gMsgProcessingLoadGeometry, CameraID + 1, ProjectorID + 1);
    assert(0 < count);
  }

  {
    if (NULL == AllImages->camera_name)
      {
        int const cnt = wprintf(gMsgProcessingCannotLoadCameraGeometryNoName);
        assert(0 < cnt);

        failed = true;
        goto ProcessAcquiredImages_EXIT;
      }
    else
      {
        HRESULT const read = camera.ReadFromXMLFile(fname_geometry, AllImages->camera_name->c_str());
        //assert( SUCCEEDED(read) );
        if ( !SUCCEEDED(read) )
          {
            int const cnt = wprintf(gMsgProcessingCannotLoadCameraGeometry, AllImages->camera_name->c_str());
            assert(0 < cnt);

            failed = true;
            goto ProcessAcquiredImages_EXIT;
          }
        /* if */
      }
    /* if */

    if ( ((double)(AllImages->width) != camera.w) || ((double)(AllImages->height) != camera.h) )
      {
        int const cnt = wprintf(
                                gMsgProcessingCameraResolutionMismatch,
                                AllImages->CameraID + 1, AllImages->width, AllImages->height,
                                camera.w, camera.h
                                );
        assert(0 < cnt);
      }
    /* if */
  }

  {
    if (NULL == AllImages->projector_name)
      {
        int const cnt = wprintf(gMsgProcessingCannotLoadProjectorGeometryNoName);
        assert(0 < cnt);

        failed = true;
        goto ProcessAcquiredImages_EXIT;
      }
    else
      {
        HRESULT const read = projector.ReadFromXMLFile(fname_geometry, AllImages->projector_name->c_str());
        //assert( SUCCEEDED(read) );
        if ( !SUCCEEDED(read) )
          {
            int const cnt = wprintf(gMsgProcessingCannotLoadProjectorGeometry, AllImages->projector_name->c_str());
            assert(0 < cnt);

            failed = true;
            goto ProcessAcquiredImages_EXIT;
          }
        /* if */
      }
    /* if */

    if ( ((double)(AllImages->window_width) != projector.w) || ((double)(AllImages->window_height) != projector.h) )
      {
        int const cnt = wprintf(
                                gMsgProcessingProjectorResolutionMismatch,
                                AllImages->ProjectorID + 1, AllImages->window_width, AllImages->window_height,
                                projector.w, projector.h
                                );
        assert(0 < cnt);
      }
    /* if */
  }

  // Set parameters.
  double pr_width = (double)( AllImages->window_width);
  double pr_height = (double)( AllImages->window_height );
  if ( CAMERA_SDK_FROM_FILE == AllImages->acquisition_method )
    {
      pr_width = projector.w;
      pr_height = projector.h;
    }
  /* if */

  // Get absolute threshold from realtive threshold.
  double abs_thr = 0.0;
  if (false == failed)
    {
      abs_thr = GetAbsoluteThreshold(AllImages, rel_thr);
      assert( false == isnan_inline(abs_thr) );
      failed = isnan_inline(abs_thr);
    }
  /* if */

  if (false == failed)
    {
      double const duration = DebugTimerQueryLast( debug_timer );
      Debugfwprintf(stderr, gMsgProcessingLoadGeometryDuration, CameraID + 1, ProjectorID + 1, duration);

      int const count = Debugfwprintf(stderr, gMsgProcessingDecodeSLCode, CameraID + 1, ProjectorID + 1, method);
      assert(0 < count);
    }
  /* if */

  // Set flags based on input method.
  // TODO: Implement regexp parser for PS and MPS.
  bool const ps_gc_col = ( 0 == _wcsicmp(method, L"PS+GC 8PS+(4+4)GC+B+W column") );
  bool const ps_gc_row = ( 0 == _wcsicmp(method, L"PS+GC 8PS+(4+4)GC+B+W row") );
  bool const ps_gc_all = ( 0 == _wcsicmp(method, L"PS+GC 8PS+(4+4)GC+B+W+8PS+(4+4)GC column row") );

  bool const mps_two_col = ( 0 == _wcsicmp(method, L"MPS 8PS(n15)+8PS(n19) column") );
  bool const mps_two_row = ( 0 == _wcsicmp(method, L"MPS 8PS(n15)+8PS(n19) row") );
  bool const mps_two_all = ( 0 == _wcsicmp(method, L"MPS 8PS(n15)+8PS(n19) column row") );

  bool const mps_three_col = ( 0 == _wcsicmp(method, L"MPS 3PS(n20)+3PS(n21)+3PS(n25) column") );
  bool const mps_three_row = ( 0 == _wcsicmp(method, L"MPS 3PS(n20)+3PS(n21)+3PS(n25) row") );
  bool const mps_three_all = ( 0 == _wcsicmp(method, L"MPS 3PS(n20)+3PS(n21)+3PS(n25) column row") );

  double const elapsed_to_decoding = DebugTimerQueryStart( debug_timer );

  // Decode projector coordinate.
  if ( (true == ps_gc_col) || (true == ps_gc_row) || (true == ps_gc_all) )
    {
      /****** Phase shift and Gray code ******/

      int const ps_col_begin = 0; // First eight images are phase shifted sine.
      int const ps_col_end = 7;

      int const gc1_col_begin = 8; // Next four images are Gray code aligned with the sine.
      int const gc1_col_end = 11;
      int const gc2_col_begin = 12; // Next four images are Gray code shifted by half-period.
      int const gc2_col_end = 15;

      int const black = 16; // Projector dimmed.
      int const white = 17; // Scene under white projector illumination.

      int const ps_row_begin = 18; // Next eight imags are phase shifted sine.
      int const ps_row_end = 25;

      int const gc1_row_begin = 26; // Next four images are Gray code aligned with the sine.
      int const gc1_row_end = 29;
      int const gc2_row_begin = 30; // Next four images are Gray code shifted by half-period.
      int const gc2_row_end = 33;

      cv::Mat * rel_phase_col = NULL; // Wrapped phase image for projector column.
      cv::Mat * gray_code_1_col = NULL; // Decoded in-phase Gray code for projector column.
      cv::Mat * gray_code_2_col = NULL; // Decoded half-period shifted Gray code for projector column.

      cv::Mat * rel_phase_row = NULL; // Wrapped phase image for projector row.
      cv::Mat * gray_code_1_row = NULL; // Decoded in-phase Gray code for projector row.
      cv::Mat * gray_code_2_row = NULL; // Decoded half-period shifted Gray code for projector row.

      texture_idx = white; // Select texture image.

      assert(false == failed);

      // Compute relative phase.
      if (false == failed)
        {
          rel_phase_col = EstimateRelativePhase(AllImages, ps_col_begin, ps_col_end);
          assert(NULL != rel_phase_col);
          failed = (NULL == rel_phase_col);
        }
      /* if */

      if (false == failed)
        {
          double const duration = DebugTimerQueryLast( debug_timer );
          Debugfwprintf(stderr, gMsgProcessingPSGCPhaseEstimationDuration, CameraID + 1, ProjectorID + 1, duration);
        }
      /* if */

      // Estimate dynamic range and texture.
      if (false == failed)
        {
          bool const update = UpdateDynamicRangeAndTexture(AllImages, ps_col_begin, ps_col_end, &dynamic_range, NULL);
          assert( (true == update) && (NULL != dynamic_range) );
          failed = (false == update);
          if ( (true == update) && (NULL != texture) ) ++texture_n;
        }
      /* if */

      if (false == failed)
        {
          double const duration = DebugTimerQueryLast( debug_timer );
          Debugfwprintf(stderr, gMsgProcessingPSGCDynamicRangeComputationDuration, CameraID + 1, ProjectorID + 1, duration);
        }
      /* if */

      // Unwrap relative phase.
      if (false == failed)
        {
          abs_phase_col = UnwrapPhasePSAndGC(
                                             AllImages,
                                             gc1_col_begin, gc1_col_end,
                                             gc2_col_begin, gc2_col_end,
                                             black, white,
                                             rel_phase_col,
                                             &gray_code_1_col, &gray_code_2_col
                                             );
          assert(NULL != abs_phase_col);
          assert(NULL != gray_code_1_col);
          assert(NULL != gray_code_2_col);
          failed = (NULL == abs_phase_col) || (NULL == gray_code_1_col) || (NULL == gray_code_2_col);
        }
      /* if */

      if (false == failed)
        {
          double const duration = DebugTimerQueryLast( debug_timer );
          Debugfwprintf(stderr, gMsgProcessingPSGCPhaseUnwrappingDuration, CameraID + 1, ProjectorID + 1, duration);
        }
      /* if */

      // Swap row and column data if only row pattern is recorded.
      if (true == ps_gc_row)
        {
          assert(false == ps_gc_col);

          SWAP_ONE_VALID_PTR( rel_phase_row, rel_phase_col );
          SWAP_ONE_VALID_PTR( gray_code_1_row, gray_code_1_col);
          SWAP_ONE_VALID_PTR( gray_code_2_row, gray_code_2_col);

          SWAP_ONE_VALID_PTR( abs_phase_row, abs_phase_col);
        }
      /* if */

      // Decode row data if both column and row pattern is recorded.
      if (true == ps_gc_all)
        {
          // Compute relative phase.
          if (false == failed)
            {
              rel_phase_row = EstimateRelativePhase(AllImages, ps_row_begin, ps_row_end);
              assert(NULL != rel_phase_row);
              failed = (NULL == rel_phase_row);
            }
          /* if */

          if (false == failed)
            {
              double const duration = DebugTimerQueryLast( debug_timer );
              Debugfwprintf(stderr, gMsgProcessingPSGCPhaseEstimationDuration, CameraID + 1, ProjectorID + 1, duration);
            }
          /* if */

          // Estimate dynamic range and texture.
          if (false == failed)
            {
              bool const update = UpdateDynamicRangeAndTexture(AllImages, ps_row_begin, ps_row_end, &dynamic_range, NULL);
              assert( (true == update) && (NULL != dynamic_range) );
              failed = (false == update);
              if ( (true == update) && (NULL != texture) ) ++texture_n;
            }
          /* if */

          if (false == failed)
            {
              double const duration = DebugTimerQueryLast( debug_timer );
              Debugfwprintf(stderr, gMsgProcessingPSGCDynamicRangeComputationDuration, CameraID + 1, ProjectorID + 1, duration);
            }
          /* if */

          // Unwrap relative phase.
          if (false == failed)
            {
              abs_phase_row = UnwrapPhasePSAndGC(
                                                 AllImages,
                                                 gc1_row_begin, gc1_row_end,
                                                 gc2_row_begin, gc2_row_end,
                                                 black, white,
                                                 rel_phase_row,
                                                 &gray_code_1_row, &gray_code_2_row
                                                 );
              assert(NULL != abs_phase_row);
              assert(NULL != gray_code_1_row);
              assert(NULL != gray_code_2_row);
              failed = (NULL == abs_phase_row) || (NULL == gray_code_1_row) || (NULL == gray_code_2_row);
            }
          /* if */

          if (false == failed)
            {
              double const duration = DebugTimerQueryLast( debug_timer );
              Debugfwprintf(stderr, gMsgProcessingPSGCPhaseUnwrappingDuration, CameraID + 1, ProjectorID + 1, duration);
            }
          /* if */
        }
      else
        {
          assert(NULL != dynamic_range);
        }
      /* if */

      // Get pixel coordinates.
      if (false == failed)
        {
          bool const res = GetValidPixelCoordinates(dynamic_range, abs_thr, &crd_x_image, &crd_y_image, &range_image);
          assert(true == res);
          failed = (true != res);
        }
      /* if */

      SAFE_DELETE( rel_phase_col );
      SAFE_DELETE( gray_code_1_col );
      SAFE_DELETE( gray_code_2_col );

      SAFE_DELETE( rel_phase_row );
      SAFE_DELETE( gray_code_1_row );
      SAFE_DELETE( gray_code_2_row );
    }
  else if ( (true == mps_two_col) || (true == mps_two_row) || (true == mps_two_all) ||
            (true == mps_three_col) || (true == mps_three_row) || (true == mps_three_all)
            )
    {
      /****** Multiple phase-shift ******/

      std::vector<double> counts; // Number of period counts per whole screen.
      std::vector<int> ps_begin; // Starting image index for each frequency.
      std::vector<int> ps_end; // Ending image index for each frequency.

      int n_frq = 0; // Number of frequencies.
      int n_phs = 0; // Number of relative phases.

      if ( (true == mps_two_col) || (true == mps_two_row) || (true == mps_two_all) )
        {
          n_frq = 2;
          n_phs = 2 * n_frq; // We assume both column and row codes are projected.

          counts.reserve( (size_t)n_frq ); // Number of periods per screen for each frequency.
          counts.push_back(15.0); // Number of periods per screen for the first frequency.
          counts.push_back(19.0); // Number of periods per screen for the second frequency.

          ps_begin.reserve( (size_t)n_phs ); // Starting image index for each frequency; first column then row code.
          ps_begin.push_back(0);
          ps_begin.push_back(8);
          ps_begin.push_back(16);
          ps_begin.push_back(24);

          ps_end.reserve( (size_t)n_phs ); // Ending image index for each frequency; first column then row code.
          ps_end.push_back(7);
          ps_end.push_back(15);
          ps_end.push_back(23);
          ps_end.push_back(31);

          texture_idx = -1; // There is no special texture image; texture is computed from phase shifted images instead.

          assert(false == failed);
        }
      else if ( (true == mps_three_col) || (true == mps_three_row) || (true == mps_three_all) )
        {
          n_frq = 3; // Number of frequencies.
          n_phs = 2 * n_frq; // We assume both column and row codes are projected.

          counts.reserve( (size_t)n_frq );
          counts.push_back(20.0);
          counts.push_back(21.0);
          counts.push_back(25.0);

          ps_begin.reserve( (size_t)n_phs ); // Starting image index for each frequency; first column then row code.
          ps_begin.push_back(0);
          ps_begin.push_back(3);
          ps_begin.push_back(6);
          ps_begin.push_back(9);
          ps_begin.push_back(12);
          ps_begin.push_back(15);

          ps_end.reserve( (size_t)n_phs ); // Ending image index for each frequency; first column then row code.
          ps_end.push_back(2);
          ps_end.push_back(5);
          ps_end.push_back(8);
          ps_end.push_back(11);
          ps_end.push_back(14);
          ps_end.push_back(17);

          texture_idx = -1; // There is no special texture image; texture is computed from phase shifted images instead.

          assert(false == failed);
        }
      else
        {
          failed = true;
        }
      /* if */

      assert( ps_begin.size() == ps_end.size() );

      std::vector<cv::Mat *> WP_col; // Relative phase images for columns.
      WP_col.reserve( (size_t)n_frq );

      std::vector<cv::Mat *> WP_row; // Relative phase images for rows.
      WP_row.reserve( (size_t)n_frq );

      cv::Mat * abs_phase_col_idx = NULL; // Index into period-order vector array.
      cv::Mat * abs_phase_row_idx = NULL; // Index into period-order vector array.

      cv::Mat * O = NULL; // Orthographic projection matrix.
      cv::Mat * Xk = NULL; // Regular constellation points.
      cv::Mat * kk = NULL; // Regular period-order vectors.
      cv::Mat * Xw = NULL; // Wrapped constellation points.
      cv::Mat * kw = NULL; // Wrapped period-order vectors.
      cv::Mat * X = NULL; // All constellation points.
      cv::Mat * K = NULL; // All period-order vectors.

      KDTreeRoot * tree = NULL; // KD tree.

      std::vector<int> * k_max = NULL; // Maximal period number for each wavelength.
      std::vector<double> * wgt = NULL; // Vector of weights used to combine unwrapped phases.
      std::vector<double> * lambda = NULL; // Vector of wavelengths.

      // Pre-compute unwrapping parameters.
      double width = std::numeric_limits<double>::quiet_NaN();
      if (false == failed)
        {
          // Get wavelengths and width from period counts.
          bool const get = mps_periods_from_fringe_counts(counts, width, &lambda, &width);
          assert(true == get);
          failed = (false == get) && (NULL != lambda);
        }
      /* if */

      if (false == failed)
        {
          // Get orthographic projection matrix and centers.
          bool const get = mps_get_projection_matrix_and_centers(*lambda, width, &O, &Xk, &kk, &Xw, &kw, NULL, &width);
          assert(true == get);
          failed = (false == get);
        }
      /* if */

      if (false == failed)
        {
          // Construct KD tree.
          bool const get = mps_get_kd_tree(Xk, kk, Xw, kw, &X, &K, &k_max, &tree);
          assert(true == get);
          failed = (false == get);

          // Release unneeded storage.
          SAFE_DELETE( Xk );
          SAFE_DELETE( kk );
          SAFE_DELETE( Xw );
          SAFE_DELETE( kw );
        }
      /* if */

      if (false == failed)
        {
          // Get standard weights.
          bool const get = mps_get_weights(*lambda, &wgt);
          assert(true == get);
          failed = (false == get);
        }
      /* if */

      if (false == failed)
        {
          double const duration = DebugTimerQueryLast( debug_timer );
          Debugfwprintf(stderr, gMsgProcessingMPSPreparationDuration, CameraID + 1, ProjectorID + 1, duration);
        }
      /* if */

      // Process column frames.
      {
        double duration_phase = 0.0;
        double duration_dynamic_range_and_texture = 0.0;

        for (int i = 0; i < n_frq; ++i)
          {
            int const idx_begin = ps_begin[i]; // Starting frame index.
            int const idx_end = ps_end[i]; // Ending frame index.

            // Compute relative phases.
            if (false == failed)
              {
                DebugTimerQueryTic( debug_timer );

                cv::Mat * const rel_phase = EstimateRelativePhase(AllImages, idx_begin, idx_end);
                assert(NULL != rel_phase);
                failed = (NULL == rel_phase);

                WP_col.push_back(rel_phase);

                duration_phase += DebugTimerQueryToc( debug_timer );
              }
            /* if */

            // Compute dynamic ranges and texture.
            if (false == failed)
              {
                DebugTimerQueryTic( debug_timer );

                bool const update = UpdateDynamicRangeAndTexture(AllImages, idx_begin, idx_end, &dynamic_range, &texture);
                assert( (true == update) && (NULL != dynamic_range) && (NULL != texture) );
                failed = (false == update);
                if ( (true == update) && (NULL != texture) ) ++texture_n;

                duration_dynamic_range_and_texture += DebugTimerQueryToc( debug_timer );
              }
            /* if */
          }
        /* for */

        if (false == failed)
          {
            double const duration = DebugTimerQueryLast( debug_timer );

            Debugfwprintf(stderr, gMsgProcessingMPSPhaseEstimationDuration, CameraID + 1, ProjectorID + 1, duration_phase);
            Debugfwprintf(stderr, gMsgProcessingMPSDynamicRangeAndTextureComputationDuration, CameraID + 1, ProjectorID + 1, duration_dynamic_range_and_texture);
          }
        /* if */
      }

      // Unwrap phase.
      if (false == failed)
        {
          std::vector<double> n; // Maximal fringe counts for each wavelength.
          n.reserve( k_max->size() );
          int const i_max = (int)k_max->size();
          for (int i = 0; i < i_max; ++i) n.push_back((double)((*k_max)[i] + 1));

          bool const unwrap = mps_unwrap_phase(WP_col, O, X, K, tree, n, *wgt, &abs_phase_col_idx, &abs_phase_col_distance, &abs_phase_col);
          assert(true == unwrap);
          failed = (false == unwrap);
        }
      /* if */

      if (false == failed)
        {
          double const duration = DebugTimerQueryLast( debug_timer );
          Debugfwprintf(stderr, gMsgProcessingMPSPhaseUnwrappingDuration, CameraID + 1, ProjectorID + 1, duration);
        }
      /* if */

      // Swap row and column data if only row pattern is recorded.
      if ( (true == mps_two_row) || (true == mps_three_row) )
        {
          assert(false == mps_two_col);
          assert(false == mps_three_col);

          std::swap(WP_col, WP_row);

          SWAP_ONE_VALID_PTR( abs_phase_row_idx, abs_phase_col_idx );
          SWAP_ONE_VALID_PTR( abs_phase_row_distance, abs_phase_col_distance );
          SWAP_ONE_VALID_PTR( abs_phase_row, abs_phase_col );
        }
      /* if */

      // Decode row data if both column and row pattern is recorded.
      if ( (true == mps_two_all) || (true == mps_three_all) )
        {
          // Process row frames.
          {
            double duration_phase = 0.0;
            double duration_dynamic_range_and_texture = 0.0;

            for (int i = 0; i < n_frq; ++i)
              {
                int const offset = n_frq;
                int const idx_begin = ps_begin[offset + i]; // Starting frame index.
                int const idx_end = ps_end[offset + i]; // Ending frame index.

                // Compute relative phases.
                if (false == failed)
                  {
                    DebugTimerQueryTic( debug_timer );

                    cv::Mat * const rel_phase = EstimateRelativePhase(AllImages, idx_begin, idx_end);
                    assert(NULL != rel_phase);
                    failed = (NULL == rel_phase);

                    WP_row.push_back(rel_phase);

                    duration_phase += DebugTimerQueryToc( debug_timer );
                  }
                /* if */

                // Compute dynamic ranges and texture.
                if (false == failed)
                  {
                    DebugTimerQueryTic( debug_timer );

                    bool const update = UpdateDynamicRangeAndTexture(AllImages, idx_begin, idx_end, &dynamic_range, &texture);
                    assert( (true == update) && (NULL != dynamic_range) && (NULL != texture) );
                    failed = (false == update);
                    if ( (true == update) && (NULL != texture) ) ++texture_n;

                    duration_dynamic_range_and_texture += DebugTimerQueryToc( debug_timer );
                  }
                /* if */
              }
            /* for */

            if (false == failed)
              {
                double const duration = DebugTimerQueryLast( debug_timer );

                Debugfwprintf(stderr, gMsgProcessingMPSPhaseEstimationDuration, CameraID + 1, ProjectorID + 1, duration_phase);
                Debugfwprintf(stderr, gMsgProcessingMPSDynamicRangeAndTextureComputationDuration, CameraID + 1, ProjectorID + 1, duration_dynamic_range_and_texture);
              }
            /* if */
          }

          // Unwrap phase.
          if (false == failed)
            {
              std::vector<double> n; // Maximal fringe counts for each wavelength.
              n.reserve( k_max->size() );
              int const i_max = (int)k_max->size();
              for (int i = 0; i < i_max; ++i) n.push_back((double)((*k_max)[i] + 1));

              bool const unwrap = mps_unwrap_phase(WP_row, O, X, K, tree, n, *wgt, &abs_phase_row_idx, &abs_phase_row_distance, &abs_phase_row);
              assert(true == unwrap);
              failed = (false == unwrap);
            }
          /* if */

          if (false == failed)
            {
              double const duration = DebugTimerQueryLast( debug_timer );
              Debugfwprintf(stderr, gMsgProcessingMPSPhaseUnwrappingDuration, CameraID + 1, ProjectorID + 1, duration);
            }
          /* if */
        }
      /* if */

      // Get pixel coordinates.
      if (false == failed)
        {
          bool const res = GetValidPixelCoordinates(dynamic_range, abs_thr, &crd_x_image, &crd_y_image, &range_image);
          assert(true == res);
          failed = (true != res);
        }
      /* if */

      // Release memory.
      {
        int const max_i = (int)WP_col.size();
        for (int i = 0; i < max_i; ++i) SAFE_DELETE( WP_col[i] );
      }

      {
        int const max_i = (int)WP_row.size();
        for (int i = 0; i < max_i; ++i) SAFE_DELETE( WP_row[i] );
      }

      SAFE_DELETE( abs_phase_col_idx );
      SAFE_DELETE( abs_phase_row_idx );

      SAFE_DELETE( O );
      SAFE_DELETE( Xk );
      SAFE_DELETE( kk );
      SAFE_DELETE( Xw );
      SAFE_DELETE( kw );
      SAFE_DELETE( X );
      SAFE_DELETE( K );

      SAFE_DELETE( tree );

      SAFE_DELETE( k_max );
      SAFE_DELETE( wgt );
      SAFE_DELETE( lambda );
    }
  else
    {
      failed = true;
    }
  /* if */

  // Prepare texture. Failure of this operation does not affect further processing.
  if (false == failed)
    {
      {
        double const duration = DebugTimerQueryStart( debug_timer ) - elapsed_to_decoding;
        Debugfwprintf(stderr, gMsgProcessingDecodeSLCodeDuration, CameraID + 1, ProjectorID + 1, duration);

        int const count = Debugfwprintf(stderr, gMsgProcessingPrepareTexture, CameraID + 1, ProjectorID + 1);
        assert(0 < count);
      }

      if (NULL != texture)
        {
          cv::Mat * tmp = ScaleAndDeBayerTexture(texture, AllImages->PixelFormat, texture_n);
          assert(NULL != tmp);
          if (NULL != tmp)
            {
              SAFE_DELETE( texture );
              SWAP_ONE_VALID_PTR( texture, tmp );
            }
          /* if */
          SAFE_DELETE( tmp );
        }
      else
        {
          texture = FetchTexture(AllImages, texture_idx);
          assert(NULL != texture);
        }
      /* if */
    }
  /* if */

  // Get phase statistics. Failure of this operation does not affect further processing.
  if (false == failed)
    {
      {
        double const duration = DebugTimerQueryLast( debug_timer );
        Debugfwprintf(stderr, gMsgProcessingPrepareTextureDuration, CameraID + 1, ProjectorID + 1, duration);

        int const count = Debugfwprintf(stderr, gMsgProcessingPrepareDataForFiltering, CameraID + 1, ProjectorID + 1);
        assert(0 < count);
      }

      // Get statistics for columns.
      if (NULL != abs_phase_col)
        {
          bool const res = GetAbsolutePhaseOrderAndDeviation(abs_phase_col, 5, 5, &abs_phase_col_order, &abs_phase_col_deviation);
          assert(true == res);
        }
      /* if */

      // Get statistics for rows.
      if (NULL != abs_phase_row)
        {
          bool const res = GetAbsolutePhaseOrderAndDeviation(abs_phase_row, 5, 5, &abs_phase_row_order, &abs_phase_row_deviation);
          assert(true == res);
        }
      /* if */

      // Combine phase deviations.
      if ( (NULL != abs_phase_col_deviation) && (NULL != abs_phase_row_deviation) )
        {
          abs_phase_deviation = CombinePhaseDeviationOrDistance(abs_phase_col_deviation, abs_phase_row_deviation);
          assert(NULL != abs_phase_deviation);
        }
      else if ( (NULL != abs_phase_col_deviation) && (NULL == abs_phase_row_deviation) )
        {
          SWAP_ONE_VALID_PTR( abs_phase_deviation, abs_phase_col_deviation );
        }
      else if ( (NULL == abs_phase_col_deviation) && (NULL != abs_phase_row_deviation) )
        {
          SWAP_ONE_VALID_PTR( abs_phase_deviation, abs_phase_row_deviation );
        }
      /* if */

      // Combine distances to constellation.
      if ( (NULL != abs_phase_col_distance) && (NULL != abs_phase_row_distance) )
        {
          abs_phase_distance = CombinePhaseDeviationOrDistance(abs_phase_col_distance, abs_phase_row_distance);
          assert(NULL != abs_phase_distance);
        }
      else if ( (NULL != abs_phase_col_distance) && (NULL != abs_phase_row_distance) )
        {
          SWAP_ONE_VALID_PTR( abs_phase_distance, abs_phase_col_distance );
        }
      else if ( (NULL != abs_phase_col_distance) && (NULL != abs_phase_row_distance) )
        {
          SWAP_ONE_VALID_PTR( abs_phase_distance, abs_phase_row_distance );
        }
      else
        {
          // Substituted distance to constellation with phase order.
          if ( (NULL != abs_phase_col_order) && (NULL != abs_phase_row_order) )
            {
              abs_phase_distance = CombinePhaseDeviationOrDistance(abs_phase_col_order, abs_phase_row_order);
              assert(NULL != abs_phase_distance);
            }
          else if ( (NULL != abs_phase_col_order) && (NULL == abs_phase_row_order) )
            {
              SWAP_ONE_VALID_PTR( abs_phase_distance, abs_phase_col_order );
            }
          else if ( (NULL == abs_phase_col_order) && (NULL != abs_phase_row_order) )
            {
              SWAP_ONE_VALID_PTR( abs_phase_distance, abs_phase_row_order ); 
            }
          /* if */
        }
      /* if */      

      // Release un-needed memory.
      SAFE_DELETE( abs_phase_col_distance );
      SAFE_DELETE( abs_phase_col_order );
      SAFE_DELETE( abs_phase_col_deviation );
      SAFE_DELETE( abs_phase_row_distance );
      SAFE_DELETE( abs_phase_row_order );
      SAFE_DELETE( abs_phase_row_deviation );      
    }
  /* if */

  if (false == failed)
    {
      double const duration = DebugTimerQueryLast( debug_timer );
      Debugfwprintf(stderr, gMsgProcessingPrepareDataForFilteringDuration, CameraID + 1, ProjectorID + 1, duration);

      int const count = Debugfwprintf(stderr, gMsgProcessing3DReconstruction, CameraID + 1, ProjectorID + 1);
      assert(0 < count);
    }
  /* if */

  // Undistort pixel coordinates.
  if (false == failed)
    {
      bool const res =
        UndistortImageCoordinatesForRadialDistorsion(
                                                     crd_x_image, crd_y_image, // OpenCV image coordinates.
                                                     1, 1, // Shift to get Matlab coordinates from OpenCV coordinates.
                                                     camera.fx, camera.fy, // Internal camera parameters.
                                                     camera.cx, camera.cy,
                                                     camera.k0, camera.k1,
                                                     &crd_x_camera, &crd_y_camera // Undistorted camera coordinates.
                                                     );
      assert(true == res);
      failed = (true != res);
    }
  /* if */

  // Test what coordinates we have.
  bool const have_col = (NULL != abs_phase_col);
  bool const have_row = (NULL != abs_phase_row);
  bool const have_both = (have_col && have_row);

  // Get projector coordinates.
  if (false == failed)
    {
      if (true == have_col)
        {
          bool const res = GetProjectorCoordinate(crd_x_image, crd_y_image, abs_phase_col, pr_width, &projector_col);
          assert(true == res);
          failed = (true != res);
        }
      /* if */

      if (true == have_row)
        {
          bool const res = GetProjectorCoordinate(crd_x_image, crd_y_image, abs_phase_row, pr_height, &projector_row);
          assert(true == res);
          failed = (true != res);
        }
      /* if */

      if (true == have_both)
        {
          assert(NULL != projector_col);
          assert(NULL != projector_row);
        }
      /* if */
    }
  /* if */

  // Triangulate views using one projector coordinate only and reproject points to projector plane.
  if ( false == have_both )
    {
      if (true == have_col)
        {
          assert(false == have_row);
          if (false == failed)
            {
              bool const res = TriangulateTwoViews(
                                                   &camera,
                                                   crd_x_camera, crd_y_camera, // Undistorted camera coordinates.
                                                   &projector,
                                                   projector_col, NULL, // Projector column index, distorted.
                                                   &x_3D, &y_3D, &z_3D, // Triangulated points.
                                                   NULL // Distance is not required.
                                                   );
              assert(true == res);
              failed = (true != res);
            }
          /* if */
        }
      /* if */

      if (true == have_row)
        {
          assert(false == have_col);
          if (false == failed)
            {
              bool const res = TriangulateTwoViews(
                                                   &camera,
                                                   crd_x_camera, crd_y_camera, // Undistorted camera coordinates.
                                                   &projector,
                                                   NULL, projector_row, // Projector row index, distorted.
                                                   &x_3D, &y_3D, &z_3D, // Triangulated points.
                                                   NULL // Distance is not required.
                                                   );
              assert(true == res);
              failed = (true != res);
            }
          /* if */
        }
      /* if */

      // Re-project to projector plane.
      if (false == failed)
        {
          bool const res = ProjectPoints(&projector, x_3D, y_3D, z_3D, &projector_col_est, &projector_row_est);
          assert(true == res);
          failed = (true != res);
        }
      /* if */
    }
  /* if */


  // Undistort projector coordinates.
  if (false == failed)
    {
      if (true == have_both)
        {
          bool const res =
            UndistortImageCoordinatesForRadialDistorsion(
                                                         projector_col, projector_row, // OpenCV image coordinates.
                                                         projector.fx, projector.fy, // Internal projector parameters.
                                                         projector.cx, projector.cy,
                                                         projector.k0, projector.k1,
                                                         &crd_x_projector, &crd_y_projector // Undistorted projector coordinates.
                                                         );
          assert(true == res);
          failed = (true != res);
        }
      else
        {
          if (true == have_col)
            {
              assert( (false == have_row) && (NULL == projector_row) );
              bool const res =
                UndistortImageCoordinatesForRadialDistorsion(
                                                             projector_col, projector_row_est, // OpenCV image coordinates.
                                                             projector.fx, projector.fy, // Internal projector parameters.
                                                             projector.cx, projector.cy,
                                                             projector.k0, projector.k1,
                                                             &crd_x_projector, &crd_y_projector // Undistorted projector coordinates.
                                                             );
              assert(true == res);
              failed = (true != res);
            }
          /* if */

          if (true == have_row)
            {
              assert( (false == have_col) && (NULL == projector_col) );
              bool const res =
                UndistortImageCoordinatesForRadialDistorsion(
                                                             projector_col_est, projector_row, // OpenCV image coordinates.
                                                             projector.fx, projector.fy, // Internal projector parameters.
                                                             projector.cx, projector.cy,
                                                             projector.k0, projector.k1,
                                                             &crd_x_projector, &crd_y_projector // Undistorted projector coordinates.
                                                             );
              assert(true == res);
              failed = (true != res);
            }
          /* if */
        }
      /* if */
    }
  /* if */

  // Re-triangulate views.
  if (false == failed)
    {
      SAFE_DELETE( x_3D );
      SAFE_DELETE( y_3D );
      SAFE_DELETE( z_3D );

      bool const res = TriangulateTwoViews(
                                           &camera,
                                           crd_x_camera, crd_y_camera, // Undistorted camera coordinates.
                                           &projector,
                                           crd_x_projector, crd_y_projector, // Undistorted projector coordinates.
                                           &x_3D, &y_3D, &z_3D, // Triangulated points.
                                           &dst2_3D
                                           );
      assert(true == res);
      failed = (true != res);
    }
  /* if */

  if (false == failed)
    {
      double const duration = DebugTimerQueryLast( debug_timer );
      Debugfwprintf(stderr, gMsgProcessing3DReconstructionDuration, CameraID + 1, ProjectorID + 1, duration);

      int const count = Debugfwprintf(stderr, gMsgProcessingPrepareDataForVTK, CameraID + 1, ProjectorID + 1);
      assert(0 < count);
    }
  /* if */

  // Create data for VTK.
  if (false == failed)
    {
      bool const res = SelectValidPointsAndAssembleDataForVTK(
                                                              x_3D, y_3D, z_3D, // Triangulated points.
                                                              dst2_3D, // Triangulation uncertainty.
                                                              dst2_thr, // Uncertainty threshold.
                                                              crd_x_image, crd_y_image, // Camera image coordinates.
                                                              range_image, // Dynamic range.
                                                              AllImages, // Image buffers.
                                                              texture, // Texture image.
                                                              abs_phase_distance, // Distance to constellation of unwrapped phase.
                                                              abs_phase_deviation, // Standard deviation of unwrapped phase.
                                                              &points_3D, // Point cloud for VTK.
                                                              &colors_3D, // Color information for VTK.
                                                              &data_3D // Additional data for VTK.
                                                              );
      assert(true == res);
      failed = (true != res);
    }
  /* if */

  wchar_t const * acquisition_name = NULL;
  if (NULL != AllImages->acquisition_name)
    {
      acquisition_name = AllImages->acquisition_name->c_str();
    }
  /* if */

  // Push data to VTK thread.
  bool const push_camera = VTKPushCameraGeometryToDisplayThread(pWindowVTK, &camera, CameraID);
  assert(true == push_camera);

  bool const push_projector = VTKPushProjectorGeometryToDisplayThread(pWindowVTK, &projector, ProjectorID);
  assert(true == push_projector);

  bool const push_points = VTKPushPointCloudToDisplayThread(pWindowVTK, points_3D, colors_3D, data_3D, CameraID, ProjectorID, acquisition_name);
  //assert(true == push_points);

  // Force redraw!
  VTKUpdateDisplay(pWindowVTK);

  if (false == failed)
    {
      double const duration = DebugTimerQueryLast( debug_timer );
      Debugfwprintf(stderr, gMsgProcessingPrepareDataForVTKDuration, CameraID + 1, ProjectorID + 1, duration);

      int const count = Debugfwprintf(stderr, gMsgProcessingDone, CameraID + 1, ProjectorID + 1);
      assert(0 < count);
    }
  /* if */


 ProcessAcquiredImages_EXIT:

  // Deallocate storage.
  SAFE_DELETE( abs_phase_col );
  SAFE_DELETE( abs_phase_col_distance );
  SAFE_DELETE( abs_phase_col_order );
  SAFE_DELETE( abs_phase_col_deviation );
  SAFE_DELETE( abs_phase_row );
  SAFE_DELETE( abs_phase_row_distance );
  SAFE_DELETE( abs_phase_row_order );
  SAFE_DELETE( abs_phase_row_deviation );
  SAFE_DELETE( abs_phase_distance );
  SAFE_DELETE( abs_phase_deviation );
  SAFE_DELETE( dynamic_range );
  SAFE_DELETE( crd_x_image );
  SAFE_DELETE( crd_y_image );
  SAFE_DELETE( range_image );
  SAFE_DELETE( crd_x_camera );
  SAFE_DELETE( crd_y_camera );
  SAFE_DELETE( projector_col );
  SAFE_DELETE( projector_row );
  SAFE_DELETE( projector_col_est );
  SAFE_DELETE( projector_row_est );
  SAFE_DELETE( crd_x_projector );
  SAFE_DELETE( crd_y_projector );
  SAFE_DELETE( x_3D );
  SAFE_DELETE( y_3D );
  SAFE_DELETE( z_3D );
  SAFE_DELETE( dst2_3D );
  SAFE_DELETE( points_3D );
  SAFE_DELETE( colors_3D );
  SAFE_DELETE( data_3D );
  SAFE_DELETE( texture );

  DebugTimerDestroy( debug_timer );

  return !failed;
}
/* ProcessAcquiredImages */



#endif /* !__BATCHACQUISITIONPROCESSING_CPP */
