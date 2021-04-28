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
  \file   BatchAcquisitionImageDecoder.h
  \brief  Image load functions.

  Defines image loading functions that use Window Imaging Components.

  \author Tomislav Petkovic
  \date   2017-03-01
*/


#ifndef __BATCHACQUISITIONIMAGEDECODER_H
#define __BATCHACQUISITIONIMAGEDECODER_H


#include "BatchAcquisition.h"
#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionEvents.h"
#include "BatchAcquisitionFileList.h"


#include <Wincodec.h>
#include <D2D1.h>



//! Decoded image and metadata.
/*!
  This structure holds decoded image and its metadata.
*/
class QueuedDecoderImage
{

 public:

  unsigned int no; //!< Image number.
  QueuedImageType render_type; //!< Image render type.

  StructuredLightPatternType pattern_type; //!< Pattern type.
  int index; //!< Filename index in the file list/pattern generator.
  unsigned int retry; //!< Image retry count.

  int projectorID; //!< Projector ID.

  IWICBitmap * pBitmap; //!< WIC image.
  std::wstring * pURI; //!< URI (if available).
  std::wstring * pFilename; //!< Filename (if available).

  float red; //!< Red color for solid pattern.
  float green; //!< Green color for solid pattern.
  float blue; //!< Blue color for solid pattern.
  float alpha; //!< Color opacity. Default is 1.0 for fully opaque color.

  double delay; //!< Trigger to exposure delay (in ms). Applied only if pattern type is SOLID_PATTERN_DLP_WHEEL.
  double exposure; //!< Exposure time (in ms). Applied only if pattern type is SOLID_PATTERN_DLP_WHEEL.

  bool fSkipAcquisition; //!< Flag to indicate imaga acquisition should be skipped.

  //! Constructor.
  QueuedDecoderImage(IWICImagingFactory * const, wchar_t const * const);

  //! Constructor.
  QueuedDecoderImage(D3DCOLORVALUE const);

  // Destructor.
  ~QueuedDecoderImage();
};



//! Parameters of the image decoder thread.
/*!
  Image decoder thread decodes images from file and stores them in the image queue.
*/
typedef
struct ImageDecoderParameters_
{
  std::deque<QueuedDecoderImage *> * pImageQueue; //!< Image queue.

  int count; //!< Total number of queued images.
  int maxItems; //!< Number of items in the queue when we stop decoding.
  int minItems; //!< Number of items in the queue when we start decoding

  int DecoderID; //!< Thread ID.
  int ProjectorID; //!< Projector ID.

  HANDLE tImageDecoder; //!< Handle to a thread running the image decoder.

  volatile bool fActive; //!< Flag to indicate image decoder thread is active.
  volatile bool fWaiting; //!< Flag to indicate image decoder is waiting for an event to be signalled.

  ImageFileList * pImageList; //!< Image file list.

  SynchronizationEvents * pSynchronization; //!< Pointer to synchronization structure.
  IWICImagingFactory * pWICFactory; //!< Windows Imaging Component (WIC) factory.

  SRWLOCK sLockImageQueue; //!< Lock to control access to image queue.
} ImageDecoderParameters;



/****** DECODER QUEUE ******/

//! Queue image to decoder.
bool
ImageDecoderQueueImage(
                       ImageDecoderParameters * const,
                       QueuedDecoderImage * const
                       );

//! Fill image queue.
int
ImageDecoderFillQueue(
                      ImageDecoderParameters * const
                      );

//! Update projector event ID.
int
ImageDecoderUpdateProjectorID(
                              ImageDecoderParameters * const
                              );

//! Check if next image is available.
bool
ImageDecoderHaveNext(
                     ImageDecoderParameters * const
                     );

//! Fetch next image to display.
QueuedDecoderImage *
ImageDecoderFetchImage(
                       ImageDecoderParameters * const,
                       bool const
                       );


//! Test if all images are queued.
bool
ImageDecoderAllFilesQueued(
                           ImageDecoderParameters * const
                           );

//! Count items.
int
ImageDecoderNumOfQueuedItems(
                             ImageDecoderParameters * const
                             );



/****** DECODER THREAD ******/

//! Create image decoder parameters and start decoder thread.
ImageDecoderParameters *
ImageDecoderStart(
                  ImageFileList * const,
                  SynchronizationEvents * const,
                  IWICImagingFactory * const,
                  int const,
                  int const
                  );

//! Stop image decoder thread.
void
ImageDecoderStop(
                 ImageDecoderParameters * const
                 );


#endif /* !__BATCHACQUISITIONIMAGEDECODER_H */
