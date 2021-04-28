/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImage.h
  \brief  Basic image manipulation procedures.

  \author Tomislav Petkovic
  \date   2016-04-05
*/


#ifndef __BATCHACQUISITIONIMAGE_H
#define __BATCHACQUISITIONIMAGE_H


#include "BatchAcquisition.h"



//! Enumeration for queued images.
/*!
  Images queued for display may have different types. Depending on the
  image type render procedure will be different. Two major classes
  are bitmap images and structured light patterns. For bitmap images
  entire pixel data is queued in format ready for displaying so bitmap
  block transfer may be used. For structured light patterns type and parameters
  of the pattern are stored so image must be rendered from scratch.
*/
typedef
enum QueuedImageType_
  {
    QI_UNKNOWN_TYPE, //!< Unknown image type. Will not be rendered.
    QI_REPEAT_PRESENT, //!< Repeated Present call with unknown image type.
    QI_BGRA_BITMAP, //!< Raw bitmap data in linear BGRA format with 8 bits per channel. This is default format for Direct2D.
    QI_PATTERN_SOLID //!< Solid color. Entire screen will have the same color.
  } QueuedImageType;



//! Acquired image metadata.
/*!
  Images are acquired sequentially. For images to be processed correctly we must know image metadata.
  As images are acquired in order for each image we queue the metadata defined by this structure.
*/
typedef
struct ImageMetadata_
{
  unsigned int no; //!< Image number (unique frame identifier).

  QueuedImageType render_type; //!< Image type.
  StructuredLightPatternType pattern_type; //!< Pattern type. Negative values denote unknown pattern types.

  long int key; //!< Unique number which identifies a frame.
  long int present_counter; //!< Number of present calls; corresponds to number of presented images. Negative value denotes undefined/not set value.
  long int vblank_counter; //!< Number of VBLANKs counted through DXGI VBLANK events at image redner. Negative value denotes undefined/not set value.

  __int64 QPC_current_presented; //!< QPC value at the time frame was presented.
  __int64 QPC_trigger_scheduled_RT; //!< Expected QPC value when trigger was scheduled in the rendering thread.
  __int64 QPC_trigger_scheduled_AT; //!< Expected QPC value when trigger was scheduled in the acquisition thread.
  __int64 QPC_next_scheduled; //!< Expected QPC value when next frame was scheduled.
  __int64 QPC_next_presented; //!< QPC value when next frame was presented.
  __int64 QPC_before_trigger; //!< QPC value before API call to trigger the camera.
  __int64 QPC_after_trigger; //!< QPC counter value after API call to trigger the camera.

  std::wstring * pFilename; //!< Fringe pattern filename. This filename will be used to store the acquired image.

  float red; //!< Red color.
  float green; //!< Green color.
  float blue; //!< Blue color.
  float alpha; //!< Color opacity.

  double delay; //!< Trigger to exposure delay (in ms). Applied only if larger than zero and if pattern_type is SOLID_PATTERN_DLP_WHEEL.
  double exposure; //!< Exposure time (in ms). Applied only if larger than zero and if pattern_type is SOLID_PATTERN_DLP_WHEEL.

  int index; //!< File index in the file list. Required for image requeing if acquisition fails.
  unsigned int retry; //!< Image retry count. If acquisition fails image will be requeued. To avoid infinite looping there exists a limit on number of retrys.

  int ProjectorID; //!< Projector ID.
  int CameraID; //!< Camera ID.

  volatile bool fBatch; //!< Flag to indicate image was acquired during batch processing.
  volatile bool fBlocking; //!< Flag to indicate acquisition is in blocking/causal mode.
  volatile bool fFixed; //!< Flag to indicate acquisition uses fixed SL pattern.
  volatile bool fSavePNG; //!< Flag to indicate image should be saved to disk in PNG format.
  volatile bool fSaveRAW; //!< Flag to indicate image should be saved to disk in RAW format.
  volatile bool fLast; //!< Flag to indicate this is last image in the batch.
  volatile bool fTrigger; //!< Flag to indicate the camera was triggered for this image.
  volatile bool fSkipAcquisition; //!< Flag to indicate image acquisition should be skipped.
} ImageMetadata;



//! Pixel statistics for whole image.
/*!
  When calibrating the projector pixel statistics must be computer for every acquired input image.
  This structure holds statistics data for one frame.
*/
typedef
struct PixelStatistics_
{
  int pattern_type; //!< Pattern type identifier.

  double sum[3]; //!< Sum of channel values.
  double mean[3]; //!< Mean pixel value.
  double dev[3]; //!< Pixel deviation.
  double min[3]; //!< Minimal pixel value.
  double max[3]; //!< Maximal pixel value.

  double t_exp; //!< Exposure time.
  double t_del; //!< Delay time.
} PixelStatistics;



//! Blanks image metadata.
void
ImageMetadataBlank(
                   ImageMetadata * const
                   );

//! Releases image metadata.
void
ImageMetadataRelease(
                     ImageMetadata * const
                     );

//! Copy image metadata.
void
ImageMetadataCopy(
                  ImageMetadata * const,
                  ImageMetadata * const,
                  bool const shallow_copy = true
                  );

//! Compare image metadata.
bool
ImageMetadataCompare(
                     ImageMetadata * const,
                     ImageMetadata * const,
                     bool const shallow_compare = true
                     );

//! Blanks pixel statistics.
void
PixelStatisticsBlank(
                     PixelStatistics * const
                     );

//! Loads image from file.
HRESULT
ImageLoadFromFile(
                  IWICImagingFactory * const,
                  wchar_t const * const,
                  IWICBitmap **
                  );

//! Stores image to file.
HRESULT
ImageSaveToPNG(
               IWICImagingFactory * const,
               IWICBitmap *,
               wchar_t const * const
               );

//! Computes image statistics.
HRESULT
ImageStatistics(
                IWICBitmap *,
                PixelStatistics * const
                );

//! Computes image statistics.
HRESULT
ImageStatistics(
                cv::Mat * const,
                PixelStatistics * const
                );

//! Return corresponding cv::Mat data type.
ImageDataType
GetImageDataType(
                 cv::Mat * const
                 );

//! Finds best matching cv::Mat data type.
bool
GetBestMatchingcvMatFlags(
                          ImageDataType const,
                          ImageDataType * const,
                          int * const
                          );

//! Return cv::Mat pixel size.
int
GetImagePixelSizeInBytes(
                         cv::Mat * const
                         );

//! Return place of the MSB bit.
double
GetImagePixelMSBPosition(
                         int const
                         );

//! Return place of the MSB bit.
double
GetImagePixelMSBPosition(
                         cv::Mat * const
                         );

//! Create shallow copy ROI.
cv::Mat *
GetcvMatROI(
            cv::Mat * const,
            int const,
            int const,
            int const,
            int const
            );



#endif /* !__BATCHACQUISITIONIMAGE_H */
