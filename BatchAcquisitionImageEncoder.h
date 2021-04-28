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
  \file   BatchAcquisitionImageEncoder.h
  \brief  Decodes image from camera and encodes it for storing to file.

  \author Tomislav Petkovic
  \date   2017-02-03
*/


#ifndef __BATCHACQUISITIONIMAGEENCODER_H
#define __BATCHACQUISITIONIMAGEENCODER_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionProcessing.h"



//! Image to be encoded and metadata.
/*!
  This structure holds image to be encoded and its metadata.
*/
class QueuedEncoderImage
{

 public:

  long int count; //!< Queue counter; always increases during application runtime.

  QueuedImageType render_type; //!< Image type.
  StructuredLightPatternType pattern_type; //!< Pattern type. Pattern numbers identifies the pattern type.

  unsigned int no; //!< Image number (unique frame identifier).
  int index; //!< Image index. It defines the order in the specific structured light sequence.

  int ProjectorID; //!< Unique projector index.
  int CameraID; //!< Unique camera index.

  void * data; //!< Base data pointer.
  size_t data_size; //!< Size of allocated memory block.
  ImageDataType data_type; //!< Image data type.
  unsigned int data_width; //!< Image width.
  unsigned int data_height; //!< Image height.
  unsigned int data_stride; //!< Image stride in bytes.

  bool is_batch; //!< Flag to indicate image is acquired in batch acquisition.
  bool save; //!< Flag to indicate image should be stored to file.
  bool save_to_raw; //!< Flag to indicate raw data should be stored.
  bool save_to_png; //!< Flag to indicate PNG image should be stored.

  std::wstring * pFilename; //!< Filename (if available).

  float red; //!< Red color for solid pattern.
  float green; //!< Green color for solid pattern.
  float blue; //!< Blue color for solid pattern.
  float alpha; //!< Color opacity. Default is 1.0 for fully opaque color.

  double delay; //!< Trigger to exposure delay (in ms).
  double exposure; //!< Exposure time (in ms).

  __int64 QPC_before_trigger; //!< QPC value before API call to trigger the camera.
  __int64 QPC_after_trigger; //!< QPC counter value after API call to trigger the camera.

  // Constructor.
  QueuedEncoderImage();

  // Destructor.
  ~QueuedEncoderImage();

  //! Copy metadata from image queue item.
  bool CopyMetadataFrom(ImageMetadata const * const);

  //! Copy data from buffer.
  BOOL CopyImageFrom(
                     void const * const,
                     unsigned int const,
                     ImageDataType const,
                     unsigned int const,
                     unsigned int const,
                     unsigned int const
                     );

#ifdef HAVE_SAPERA_SDK

  //! Copy data from Sapera Buffer.
  BOOL CopyImageFrom(SapBuffer * const, SapAcqDevice * const);

#endif /* HAVE_SAPERA_SDK */

#ifdef HAVE_FLYCAPTURE2_SDK

  //! Copy data from FlyCapture2 buffer.
  BOOL CopyImageFrom(FlyCapture2::Image * const, FlyCapture2::Camera * const);

#endif /* HAVE_FLYCAPTURE2_SDK */

  //! Gets cv::Mat bitmap.
  cv::Mat * QueuedEncoderImage::GetCVMat(void);

  //! Gets WIC bitmap.
  HRESULT GetIWICBitmap(IWICImagingFactory * const, IWICBitmap ** const);

  //! Store image to PNG file.
  bool StoreToPNGFile(std::wstring * const, IWICImagingFactory * const);

  //! Store image raw data.
  bool StoreToRawFile(std::wstring * const);

};



//! Parameters of the image encoder thread.
/*!
  Image encoder thread encodes images and queues them for storage to disk.
*/
typedef
struct ImageEncoderParameters_
{
  HANDLE tImageEncoder; //!< Handle to a thread running the image encoder.
  
  std::vector<QueuedEncoderImage *> * pImageQueue; //!< Image queue.

  std::vector<PixelStatistics> * pStatistics; //!< Pixel statistics for acquired images.
  ImageSet * pAllImages; //!< Structure holding all images from one structured light pattern set.

  std::wstring * pDirectoryData; //!< Data directory where all acquisitions are stored.

  std::wstring * pSubdirectorySession; //!< Session subdirectory in data directory where recordings are stored. Must be set by user.
  std::wstring * pSubdirectoryRecording; //!< Recording subdirectory where one acquisition is stored. Always starts with a timestamp.
  std::wstring * pSubdirectoryCamera; //!< Camera subdirectory in recording directory where images acquired by one camera are stored.
  
  SynchronizationEvents * pSynchronization; //!< Pointer to synchronization structure.
  IWICImagingFactory * pWICFactory; //!< Windows Imaging Component (WIC) factory.

  int count; //!< Total number of queued images.
  int maxItems; //!< Number of items in the queue when we start encoding.
  int minItems; //!< Number of items in the queue when we stop encoding.

  int EncoderID; //!< Thread ID.
  int CameraID; //!< Camera ID.

  int roi_x; //!< Upper left corner of the ROI.
  int roi_y; //!< Upper left corner of the ROI.
  int roi_w; //!< Width of the ROI.
  int roi_h; //!< Height of the ROI.

  volatile int num_batch; //!< Number of items having batch flag set.
  volatile bool fActive; //!< Flag to indicate image encoder thread is active.
  volatile bool fWaiting; //!< Flag to indicate image encoder is waiting for an event to be signalled.

  SRWLOCK sLockImageQueue; //!< Lock to control access to image queue.
  SRWLOCK sLockImageData; //!< Lock to control access to image data structures.
  SRWLOCK sLockDirectory; //!< Lock to control access to directory strings.
} ImageEncoderParameters;



/****** ENCODER QUEUE ******/

//! Fetches image to encode.
QueuedEncoderImage *
ImageEncoderFetchImage(
                       ImageEncoderParameters * const,
                       bool const
                       );

//! Empty image encoder queue.
int
ImageEncoderEmptyQueue(
                       ImageEncoderParameters * const
                       );

//! Queues image to encode.
bool
ImageEncoderQueueImage(
                       ImageEncoderParameters * const,
                       QueuedEncoderImage * const
                       );

//! Count remaining items from batch.
int
ImageEncoderBatchItemsRemaining(
                                ImageEncoderParameters * const
                                );

//! Count all remaining items.
int
ImageEncoderTotalItemsRemaining(
                                ImageEncoderParameters * const
                                );

//! Get image dimensions.
bool
ImageEncoderGetImageDimensions(
                               ImageEncoderParameters * const,
                               int * const,
                               int * const
                               );



/****** ENCODER THREAD ******/

//! Create image encoder parameters and start encoder thread.
ImageEncoderParameters *
ImageEncoderStart(
                  SynchronizationEvents * const,
                  IWICImagingFactory * const,
                  int const,
                  int const
                  );

//! Stop image encoder thread.
void
ImageEncoderStop(
                 ImageEncoderParameters * const
                 );

//! Set output data directory.
bool
ImageEncoderSetDirectory(
                         ImageEncoderParameters * const,
                         wchar_t const * const,
                         wchar_t const * const
                         );

//! Set output data directory.
bool
ImageEncoderTrySetDirectory(
                            ImageEncoderParameters * const,
                            wchar_t const * const
                            );

//! Get output data directory.
wchar_t const *
ImageEncoderGetDirectory(
                         ImageEncoderParameters * const
                         );

//! Set session subdirectory.
bool
ImageEncoderSetSubdirectorySession(
                                   ImageEncoderParameters * const,
                                   std::wstring * const
                                   );

//! Get session subdirectory.
std::wstring *
ImageEncoderGetSubdirectorySession(
                                   ImageEncoderParameters * const
                                   );
                                 

//! Set recording subdirectory.
bool
ImageEncoderSetSubdirectoryRecording(
                                     ImageEncoderParameters * const,
                                     std::wstring * const
                                     );

//! Append to recording subdirectory.
bool
ImageEncoderAppendToSubdirectoryRecording(
                                          ImageEncoderParameters * const,
                                          std::wstring * const
                                          );

//! Set recodring subdirectory to current timestamp.
bool
ImageEncoderSetSubdirectoryRecordingToTimestamp(
                                                ImageEncoderParameters * const
                                                );

//! Get recording subdirectory.
std::wstring *
ImageEncoderGetSubdirectoryRecording(
                                     ImageEncoderParameters * const
                                     );

//! Get output directory.
std::wstring *
ImageEncoderGetOutputDirectory(
                               ImageEncoderParameters * const,
                               bool const,
                               bool const
                               );

//! Copy output directory names.
bool
ImageEncoderCopyOutputDirectoryNames(
                                     ImageEncoderParameters * const,
                                     ImageEncoderParameters * const
                                     );

//! Set ROI.
bool
ImageEncoderSetROI(
                   ImageEncoderParameters * const,
                   int const,
                   int const,
                   int const,
                   int const
                   );

//! Resets pixel statistics.
bool
ImageEncoderResetFrameData(
                           ImageEncoderParameters * const
                           );

//! Compute system delay time.
bool
ImageEncoderComputeDelay(
                         ImageEncoderParameters * const,
                         double * const,
                         double * const,
                         double * const
                         );


#endif /* !__BATCHACQUISITIONIMAGEENCODER_H */
