/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2016-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2016-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionImageQueue.h
  \brief  Image metadata queue.

  Header for image metadata queue.

  \author Tomislav Petkovic
  \date   2017-01-19
*/



#ifndef __BATCHACQUISITIONIMAGEQUEUE_H
#define __BATCHACQUISITIONIMAGEQUEUE_H


#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionWindowDisplay.h"



//! Image metadata queue (list).
/*!
  Image metadata queue (list) holds image metadata of displayed images and is used to match displayed and
  acquired images.
*/
typedef
struct ImageMetadataQueue_
{
  std::vector<ImageMetadata> * pMetadataQueue; //!< Image metadata queue.
  SRWLOCK sQueueLock; //!< Image metadata queue lock.

  //! Constructor.
  ImageMetadataQueue_();

  //! Blanks structure variables.
  void Blank(void);

  //! Release allocated memory and resources.
  void Release(void);

  //! Destructor.
  ~ImageMetadataQueue_();

  //! Queue size.
  size_t Size(void);

  //! Check if image queue is empty.
  bool IsEmpty(void);

  //! Check if all queued images have the same type.
  bool AreAllImagesOfType(QueuedImageType const, StructuredLightPatternType const);

  //! Push image metadata into queue.
  bool PushBackImageMetadataToQueue(ImageMetadata * const, bool const);

  //! Pop first image metadata item from queue.
  bool PopFrontImageMetadataFromQueue(ImageMetadata * const, bool const triggered = true);

  //! Pop image metadata matching selected key from queue.
  bool PopImageMetadataFromQueue(ImageMetadata * const, long int const);

  //! Peek into image metadata.
  bool PeekImageMetadataInQueue(ImageMetadata * const, long int const);

  //! Peek into image metadata.
  bool PeekFrontImageMetadataInQueue(ImageMetadata * const, bool const triggered = true);

  //! Peek into image metadata.
  bool PeekBackImageMetadataInQueue(ImageMetadata * const, bool const triggered = true);

  //! Adjust metadata values from rendering thread.
  bool AdjustImageMetadataRendering(long int const, __int64 const);

  //! Adjust metadata values from acquistion thread.
  bool AdjustImageMetadataAcquisition(long int const, double const, double const, __int64 const, __int64 const, bool const, bool const);

  //! Invalidate first/oldest item in queue.
  bool InvalidateFirst(void);

} ImageMetadataQueue;




/****** IMAGE QUEUE ******/

//! Push image metadata to queue.
bool
PushBackImageMetadataToQueue(
                             ImageMetadataQueue * const,
                             ImageMetadata * const,
                             bool const
                             );

//! Pop image metadata from queue.
bool
PopFrontImageMetadataFromQueue(
                               ImageMetadataQueue * const,
                               ImageMetadata * const,
                               bool const triggered = true
                               );

//! Peek into image metadata.
bool
PeekFrontImageMetadataInQueue(
                              ImageMetadataQueue * const,
                              ImageMetadata * const,
                              bool const triggered = true
                              );

//! Empties image queue.
void
EmptyImageMetadataQueue(
                        ImageMetadataQueue * const
                        );

//! Remove missed images.
int
RemoveMissedImages(
                   ImageMetadataQueue * const,
                   __int64 const
                   );


#endif /* !__BATCHACQUISITIONIMAGEQUEUE_H */
