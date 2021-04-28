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
  \file   BatchAcquisitionImageQueue.cpp
  \brief  Image metadata queue.

  Functions required for image metadata queue.

  \author Tomislav Petkovic
  \date   2017-01-19
*/

#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGEQUEUE_CPP
#define __BATCHACQUISITIONIMAGEQUEUE_CPP


#include "BatchAcquisitionImageQueue.h"
#include "BatchAcquisitionDebug.h"




/****** HELPER FUNCTIONS ******/


#pragma region // Queue sorting

//! Compares two items.
/*!
  Compares two image metadata items by comparing rendering times.

  \param first  Pointer to first item.
  \param second Pointer to second item.
  \return Returns true if first item was displayed and presented before the second item, false otherwise.
*/
bool
compare_by_render_time(
                       const ImageMetadata& first,
                       const ImageMetadata& second
                       )
{
  return ( first.QPC_current_presented < second.QPC_current_presented );
}
/* compare_by_render_time */



//! Sorts image queue.
/*!
  Sorts image queue using VBLANK synchronization QPC value.
  Items are sorted in ascending order using QPC presentation time.

  Note: This function does not acquire an exclusive SRW lock.
  The lock must be acquired prior to calling this function and must
  be released after the function completes.

  \param P      Pointer to ImageMetadataQueue structure.
*/
inline
void
SortImageMetadataQueue_inline(
                              ImageMetadataQueue * const P
                              )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return;

  if (false == P->pMetadataQueue->empty())
    {
      std::sort( P->pMetadataQueue->begin(), P->pMetadataQueue->end(), compare_by_render_time );
    }
  /* if */
}
/* SortImageMetadataQueue_inline */

#pragma endregion // Queue sorting


#pragma region // Push, pop, and peek item

//! Push image metadata to queue.
/*!
  Function stores image metadata into metadata queue as the last element.

  Note: Function acquires and releases an exclusive SRW lock.
  As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to image metadata.
  \param duplicate  Flag to indicate string object to which pFilename points should be duplicated.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
PushBackImageMetadataToQueue_inline(
                                    ImageMetadataQueue * const P,
                                    ImageMetadata * const pData,
                                    bool const duplicate
                                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return false;

  assert(NULL != pData);
  if (NULL == pData) return false;

  ImageMetadata sData = *pData;

  if (true == duplicate)
    {
      if (NULL != sData.pFilename) sData.pFilename = new std::wstring( *(pData->pFilename) );
    }
  /* if */

  AcquireSRWLockExclusive( &(P->sQueueLock) );
  {
    P->pMetadataQueue->push_back(sData);
  }
  ReleaseSRWLockExclusive( &(P->sQueueLock) );

  return true;
}
/* PushBackImageMetadataToQueue_inline */



//! Pop image metadata from queue.
/*!
  Function retrieves image metadata from the front of the queue.

  Note: Function acquires and releases an exclusive SRW lock.
  As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to structure where image metadata will be stored.
  If pData is NULL item will be poped from the queue so its contents will be lost.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
PopFrontImageMetadataFromQueue_inline(
                                      ImageMetadataQueue * const P,
                                      ImageMetadata * const pData,
                                      bool const triggered
                                      )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return false;

  assert(NULL != pData);

  bool retrieved = false;

  AcquireSRWLockExclusive( &(P->sQueueLock) );
  {
    if (false == P->pMetadataQueue->empty())
      {
        if (false == triggered)
          {
            ImageMetadata sData = P->pMetadataQueue->front();
            P->pMetadataQueue->erase( P->pMetadataQueue->begin() );
            retrieved = true;

            if (NULL != pData)
              {
                *pData = sData;
              }
            else
              {
                ImageMetadataRelease( &sData );
              }
            /* if */
          }
        else
          {
            std::vector<ImageMetadata>::iterator it;
            for (it = P->pMetadataQueue->begin(); it != P->pMetadataQueue->end(); ++it)
              {
                if (true == (*it).fTrigger)
                  {
                    ImageMetadata sData = *it;
                    P->pMetadataQueue->erase( it );
                    retrieved = true;

                    if (NULL != pData)
                      {
                        *pData = sData;
                      }
                    else
                      {
                        ImageMetadataRelease( &sData );
                      }
                    /* if */

                    break;
                  }
                /* if */
              }
            /* for */
          }
        /* if */
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sQueueLock) );

  return retrieved;
}
/* PopFrontImageMetadataFromQueue_inline */



//! Peek into image metadata in queue.
/*!
  Function retrieves image metadata from the queue without removing it from the queue.

  Note: Function acquires and releases a shared SRW lock.
  As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to structure where image metadata will be copied.
  \param key    A key which uniquely identifies the item.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
PeekImageMetadataInQueue_inline(
                                ImageMetadataQueue * const P,
                                ImageMetadata * const pData,
                                long int const key
                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return false;

  bool retrieved = false;

  AcquireSRWLockShared( &(P->sQueueLock) );
  {
    if (false == P->pMetadataQueue->empty())
      {
        std::vector<ImageMetadata>::iterator it;
        for (it = P->pMetadataQueue->begin(); it != P->pMetadataQueue->end(); ++it)
          {
            if ( key == (*it).key )
              {
                retrieved = true;
                if (NULL != pData) *pData = *it;
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sQueueLock) );

  return retrieved;
}
/* PeekImageMetadataInQueue_inline */



//! Peek into image metadata in queue.
/*!
  Function retrieves image metadata from the queue without removing it from the queue.

  Note: Function acquires and releases a shared SRW lock.
  As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to structure where image metadata will be copied.
  \param front  If true the front/first item is returned, if false the back/last item is returned.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
PeekImageMetadataInQueue_inline(
                                ImageMetadataQueue * const P,
                                ImageMetadata * const pData,
                                bool const front,
                                bool const triggered = true
                                )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return false;

  bool retrieved = false;

  AcquireSRWLockShared( &(P->sQueueLock) );
  {
    if (false == P->pMetadataQueue->empty())
      {
        if (true == front)
          {
            if (false == triggered)
              {
                ImageMetadata const sData = P->pMetadataQueue->front();
                if (NULL != pData) *pData = sData;
                retrieved = true;
              }
            else
              {
                std::vector<ImageMetadata>::iterator it;
                for (it = P->pMetadataQueue->begin(); it != P->pMetadataQueue->end(); ++it)
                  {
                    if (true == (*it).fTrigger)
                      {
                        if (NULL != pData) *pData = *it;
                        retrieved = true;
                        break;
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */
          }
        else
          {
            if (false == triggered)
              {
                ImageMetadata const sData = P->pMetadataQueue->back();
                if (NULL != pData) *pData = sData;
                retrieved = true;
              }
            else
              {
                std::vector<ImageMetadata>::reverse_iterator rit;
                for (rit = P->pMetadataQueue->rbegin(); rit != P->pMetadataQueue->rend(); ++rit)
                  {
                    if (true == (*rit).fTrigger)
                      {
                        if (NULL != pData) *pData = *rit;
                        retrieved = true;
                        break;
                      }
                    /* if */
                  }
                /* for */
              }
            /* if */
          }
        /* if */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sQueueLock) );

  return retrieved;
}
/* PeekImageMetadataInQueue_inline */

#pragma endregion // Push, pop, and peek item


#pragma region // Clear and empty queue

//! Removes items with invalid VBLANK values.
/*!
  Function removes items with VBLANK values that are outside of the specified range.

  Note: Function acquires and releases SRW lock. As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
  \param vblank_counter_min    Minimal allowed VBLANK value.
  \param vblank_counter_max    Maximal allowed VBLANK value.
*/
inline
void
ClearImageMetadataQueue_inline(
                               ImageMetadataQueue * const P,
                               long int const vblank_counter_min,
                               long int const vblank_counter_max
                               )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return;

  assert(vblank_counter_min < vblank_counter_max);

  std::vector<ImageMetadata> * const pQueue = new std::vector<ImageMetadata>;
  assert(NULL != pQueue);
  if (NULL == pQueue) return;

  std::vector<ImageMetadata>::iterator it;
  std::vector<ImageMetadata> * pTmp = NULL;

  /* Instead of deleting elements we simply create a new std::vector and copy
     valid elements into the new vector. The vectors are swapped and old one
     is deallocated.
  */
  AcquireSRWLockExclusive( &(P->sQueueLock) );
  {
    pQueue->reserve( P->pMetadataQueue->capacity() );

    for (it = P->pMetadataQueue->begin(); it != P->pMetadataQueue->end(); ++it)
      {
        long int const vblank_counter = (*it).vblank_counter;
        if ( (vblank_counter_min <= vblank_counter) && (vblank_counter <= vblank_counter_max) )
          {
            pQueue->push_back( *it );
            ImageMetadataBlank( &(*it) );
          }
        /* if */
      }
    /* for */

    pTmp = P->pMetadataQueue;
    P->pMetadataQueue = pQueue;
  }
  ReleaseSRWLockExclusive( &(P->sQueueLock) );


  while ( false == pTmp->empty() )
    {
      ImageMetadata sData = pTmp->back();
      pTmp->pop_back();
      ImageMetadataRelease( &sData );
    }
  /* while */
  SAFE_DELETE( pTmp );

}
/* ClearImageMetadataQueue_inline */



//! Empties image queue.
/*!
  Removes all queued filenames.

  Note: Function acquires and releases SRW lock. As SRW locking cannot be nested if SRW lock is held by
  the calling thread then it must be released prior to calling this function.

  \param P      Pointer to ImageMetadataQueue structure.
*/
inline
void
EmptyImageMetadataQueue_inline(
                               ImageMetadataQueue * const P
                               )
{
  assert(NULL != P);
  if (NULL == P) return;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return;

  AcquireSRWLockExclusive( &(P->sQueueLock) );
  {
    while ( false == P->pMetadataQueue->empty() )
      {
        ImageMetadata sData = P->pMetadataQueue->back();
        P->pMetadataQueue->pop_back();
        ImageMetadataRelease( &sData );
      }
    /* while */

    SortImageMetadataQueue_inline( P );
  }
  ReleaseSRWLockExclusive( &(P->sQueueLock) );
}
/* EmptyImageMetadataQueue_inline */

#pragma endregion // Clear and empty queue




/****** IMAGE METADATA QUEUE ******/

#pragma region // Constructor and destructor

//! Constructor.
/*!
  Creates ImageMetadataQueue structure and allocates storage list and SRW lock.
*/
ImageMetadataQueue_::ImageMetadataQueue_()
{
  this->Blank();

  this->pMetadataQueue = new std::vector<ImageMetadata>;
  assert(NULL != this->pMetadataQueue);

  InitializeSRWLock( &(this->sQueueLock) );
}
/* ImageMetadataQueue_::ImageMetadataQueue_ */



//! Blanks structure variables.
/*!
  Sets all class variables to default values.
*/
void
ImageMetadataQueue_::Blank(
                           void
                           )
{
  this->pMetadataQueue = NULL;
  ZeroMemory( &(this->sQueueLock), sizeof(this->sQueueLock) );
}
/* ImageMetadataQueue_::Blank */



//! Release allocated memory and resources.
/*!
  Releases allocated memory.
*/
void
ImageMetadataQueue_::Release(
                             void
                             )
{
  EmptyImageMetadataQueue_inline( this );

  AcquireSRWLockExclusive( &(this->sQueueLock) );
  {
    SAFE_DELETE( this->pMetadataQueue );
  }
  ReleaseSRWLockExclusive( &(this->sQueueLock) );

  this->Blank();
}
/* ImageMetadataQueue_::Release */



//! Destructor.
/*!
  Releases allocated resources and blanks class variables.
*/
ImageMetadataQueue_::~ImageMetadataQueue_()
{
  this->Release();
  this->Blank();
}
/* ImageMetadataQueue_::~ImageMetadataQueue_ */

#pragma endregion // Constructor and destructor


#pragma region // Check queue state

//! Queue size.
/*!
  Returns size of the queue.

  \return Returns size of the queue.
*/
size_t
ImageMetadataQueue_::Size(
                          void
                          )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return 0;

  size_t sz = 0;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    sz = this->pMetadataQueue->size();
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return sz;
}
/* ImageMetadataQueue_::Size */



//! Check if image queue is empty.
/*!
  Checks if queue is empty.

  \return Returns true if queue is empty.
*/
bool
ImageMetadataQueue_::IsEmpty(
                             void
                             )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return true;

  bool empty = true;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    empty = this->pMetadataQueue->empty();
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return empty;
}
/* ImageMetadataQueue_::IsEmpty */



//! Check if all queued images have the same type.
/*!
  Checks if all queued items have the same image type.

  \param render_type   Image render type to check against.
  \param pattern_type   SL pattern type to check against.
  \return Returns true if all items have the same type or if queue is empty, false otherwise.
*/
bool
ImageMetadataQueue_::AreAllImagesOfType(
                                        QueuedImageType const render_type,
                                        StructuredLightPatternType const pattern_type
                                        )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return true;

  bool all_same_type = true;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    if (false == this->pMetadataQueue->empty())
      {
        std::vector<ImageMetadata>::iterator it;
        for (it = this->pMetadataQueue->begin(); it != this->pMetadataQueue->end(); ++it)
          {
            all_same_type = all_same_type &&
              (render_type == it->render_type) &&
              (pattern_type == it->pattern_type);
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return all_same_type;
}
/* ImageMetadataQueue_::AreAllImagesOfType */

#pragma endregion // Check queue state


#pragma region // Push items into queue

//! Push image metadata into queue.
/*!
  Function stores image metadata into metadata queue.

  \param pData  Pointer to image metadata.
  \param duplicate  Flag to indicate string objects should be duplicated.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PushBackImageMetadataToQueue(
                                                  ImageMetadata * const pData,
                                                  bool const duplicate
                                                  )
{
  return PushBackImageMetadataToQueue_inline(this, pData, duplicate);
}
/* ImageMetadataQueue_::PushBackImageMetadataToQueue */

#pragma endregion // Push items into queue


#pragma region // Pop items from queue

//! Pop first image metadata item from queue.
/*!
  Function retrieves front image metadata from queue.
  If pData is NULL the item will be poped from the queue but its contents will be lost.

  \param pData  Pointer to structure where image metadata will be stored.
  \param triggered If set to true then the first item that has fTrigger flag set is returned; if set to false than the first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PopFrontImageMetadataFromQueue(
                                                    ImageMetadata * const pData,
                                                    bool const triggered
                                                    )
{
  return PopFrontImageMetadataFromQueue_inline(this, pData, triggered);
}
/* ImageMetadataQueue_::PopFrontImageMetadataFromQueue */



//! Pop image metadata matching selected key from queue.
/*!
  Function retrieves image metadata which matches the selected key from the queue.
  Matching item is one whose key matches the supplied key.
  A true value will be returned if such item exists; false will be returned if no such valu exists or if function failed.
  If pData is NULL the matching item will be poped from the queue but its contents will be lost.

  \param pData Pointer to structure where the image metadata will be stored.
  \param key    A key which uniquely identifies the item.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PopImageMetadataFromQueue(
                                               ImageMetadata * const pData,
                                               long int const key
                                               )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return false;

  assert(NULL != pData);

  bool retrieved = false;
  std::vector<ImageMetadata>::iterator it;

  AcquireSRWLockExclusive( &(this->sQueueLock) );
  {
    if (false == this->pMetadataQueue->empty())
      {
        for (it = this->pMetadataQueue->begin(); it != this->pMetadataQueue->end(); ++it)
          {
            if ( key == (*it).key )
              {
                ImageMetadata sData = *it;
                this->pMetadataQueue->erase(it);
                retrieved = true;

                if (NULL != pData)
                  {
                    *pData = sData;
                  }
                else
                  {
                    ImageMetadataRelease( &sData );
                  }
                /* if */

                SortImageMetadataQueue_inline( this );
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(this->sQueueLock) );

  return retrieved;
}
/* ImageMetadataQueue_::PopImageMetadataFromQueue */

#pragma endregion // Pop items from queue


#pragma region // Peek into queue

//! Peek into image metadata.
/*!
  Function retrieves image metadata with selected key from the queue without removing it.

  \param pData  Pointer to structure where image metadata will be copied.
  \param key    A key which uniquely identifies the item.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PeekImageMetadataInQueue(
                                              ImageMetadata * const pData,
                                              long int const key
                                              )
{
  return PeekImageMetadataInQueue_inline(this, pData, key);
}
/* ImageMetadataQueue_::PeekImageMetadataInQueue */



//! Peek into image metadata.
/*!
  Function retrieves image metadata from the front of the queue without removing it.

  \param pData  Pointer to structure where image metadata will be copied.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PeekFrontImageMetadataInQueue(
                                                   ImageMetadata * const pData,
                                                   bool const triggered
                                                   )
{
  return PeekImageMetadataInQueue_inline(this, pData, true, triggered);
}
/* ImageMetadataQueue_::PeekFrontImageMetadataInQueue */



//! Peek into image metadata.
/*!
  Function retrieves image metadata from the back of the queue without removing it.

  \param pData  Pointer to structure where image metadata will be copied.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::PeekBackImageMetadataInQueue(
                                                  ImageMetadata * const pData,
                                                  bool const triggered
                                                  )
{
  return PeekImageMetadataInQueue_inline(this, pData, false, triggered);
}
/* ImageMetadataQueue_::PeekBackImageMetadataInQueue */

#pragma endregion // Peek into queue


#pragma region // Adjust metadata values form rendering

//! Adjust metadata values from rendering thread.
/*!
  Adjusts metadata values for one specific item in the queue which matches a provided key.
  
  \param key    Present counter value of the image.
  \param QPC_next_presented   QPC time when the next frame was presented.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::AdjustImageMetadataRendering(
                                                  long int const key,
                                                  __int64 const QPC_next_presented
                                                  )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return false;

  assert(0 < QPC_next_presented);

  bool adjusted = false;
  std::vector<ImageMetadata>::reverse_iterator rit;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    if (false == this->pMetadataQueue->empty())
      {
        for (rit = this->pMetadataQueue->rbegin(); rit != this->pMetadataQueue->rend(); ++rit)
          {
            if (key == rit->key)
              {
                assert(-1 == rit->QPC_next_presented);
                rit->QPC_next_presented = QPC_next_presented;

                adjusted = true;
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return adjusted;
}
/* ImageMetadataQueue_::AdjustImageMetadataRendering */

#pragma endregion // Adjust metadata values form rendering


#pragma region // Adjust metadata values form acquisition

//! Adjust metadata values from acquisition thread.
/*!
  Adjusts metadata values for one specific item in the queue which matches a provided key.

  \param key    A key which uniquely identifies the item.
  \param delay      Trigger delay value to set (in ms).
  \param exposure   Exposure value to set (in ms).
  \param QPC_before_trigger   QPC timer value exactly before the camera software trigger API call was made.
  \param QPC_after_trigger    QPC timer value exactly after the camera software trigger API call returned.
  \param triggered            Flag which indicates camera was triggered for this image.
  \param trigger_on_time      Flag which indicates the trigger was on time.
  \return Function returns true if successfull, false otherwise.
*/
bool
ImageMetadataQueue_::AdjustImageMetadataAcquisition(
                                                    long int const key,
                                                    double const delay,
                                                    double const exposure,
                                                    __int64 const QPC_before_trigger,
                                                    __int64 const QPC_after_trigger,
                                                    bool const triggered,
                                                    bool const trigger_on_time
                                                    )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return false;

  assert( 0 <= delay );
  assert( 0 < exposure );
  assert( QPC_before_trigger <= QPC_after_trigger );

  bool adjusted = false;
  std::vector<ImageMetadata>::reverse_iterator rit;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    if (false == this->pMetadataQueue->empty())
      {
        for (rit = this->pMetadataQueue->rbegin(); rit != this->pMetadataQueue->rend(); ++rit)
          {
            if (key == rit->key)
              {
                assert( 0.0 == rit->delay );
                rit->delay = delay;

                assert( 0.0 == rit->exposure );
                rit->exposure = exposure;

                assert( -1 == rit->QPC_before_trigger );
                rit->QPC_before_trigger = QPC_before_trigger;

                assert( -1 == rit->QPC_after_trigger );
                rit->QPC_after_trigger = QPC_after_trigger;

                rit->fTrigger = triggered;

                if (false == trigger_on_time) rit->fBatch = false;

                adjusted = true;
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return adjusted;
}
/* ImageMetadataQueue_::AdjustImageMetadataAcquisition */

#pragma endregion // Adjust metadata values from acquisition


#pragma region // Invalidate items

//! Invalidate first/oldest item in queue.
/*!
  Invalidates first/oldest item in queue which is not marked as triggered
  by marking it as triggered and unmarking it as batch.

  \return Returns true if successfull.
*/
bool
ImageMetadataQueue_::InvalidateFirst(
                                     void
                                     )
{
  assert(NULL != this->pMetadataQueue);
  if (NULL == this->pMetadataQueue) return false;

  bool invalidated = false;
  std::vector<ImageMetadata>::iterator it;

  AcquireSRWLockShared( &(this->sQueueLock) );
  {
    if (false == this->pMetadataQueue->empty())
      {
        for (it = this->pMetadataQueue->begin(); it != this->pMetadataQueue->end(); ++it)
          {
            if (false == it->fTrigger)
              {
                it->fTrigger = true;
                it->fBatch = false;
                invalidated = true;
                break;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(this->sQueueLock) );

  return invalidated;
}
/* ImageMetadataQueue_::InvalidateFirst */

#pragma endregion // Invalidate items



/****** IMAGE QUEUE ******/

#pragma region // Push, pop, and peek item

//! Push image metadata to queue.
/*!
  Function stores image metadata into queue.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to image metadata.
  \param duplicate  Flag to indicate string objects should be duplicated.
  \return Function returns true if successfull, false otherwise.
*/
bool
PushBackImageMetadataToQueue(
                             ImageMetadataQueue * const P,
                             ImageMetadata * const pData,
                             bool const duplicate
                             )
{
  return PushBackImageMetadataToQueue_inline(P, pData, duplicate);
}
/* PushBackImageMetadataToQueue */



//! Pop image metadata from queue.
/*!
  Function retrieves image metadata from queue.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to structure where image metadata will be stored.
  If pData is NULL item will be poped from the queue so its contents will be lost.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
bool
PopFrontImageMetadataFromQueue(
                               ImageMetadataQueue * const P,
                               ImageMetadata * const pData,
                               bool const triggered
                               )
{
  return PopFrontImageMetadataFromQueue_inline(P, pData, triggered);
}
/* PopFrontImageMetadataFromQueue */



//! Peek into image metadata in queue.
/*!
  Function retrieves image metadata from queue without removing it.

  \param P      Pointer to ImageMetadataQueue structure.
  \param pData  Pointer to structure where image metadata will be copied.
  \param triggered If set to true then first item that has fTrigger flag set is returned; if set to false than first item is returned.
  \return Function returns true if successfull, false otherwise.
*/
bool
PeekFrontImageMetadataInQueue(
                              ImageMetadataQueue * const P,
                              ImageMetadata * const pData,
                              bool const triggered
                              )
{
  return PeekImageMetadataInQueue_inline(P, pData, true, triggered);
}
/* PeekFrontImageMetadataInQueue */

#pragma endregion // Push, pop, and peek item


#pragma region // Remove items

//! Empties image queue.
/*!
  Removes all queued filenames.

  \param P      Pointer to ImageMetadataQueue structure.
*/
void
EmptyImageMetadataQueue(
                        ImageMetadataQueue * const P
                        )
{
  EmptyImageMetadataQueue_inline(P);
}
/* EmptyImageMetadataQueue */



//! Remove missed images.
/*!
  Removes missed images from image queue.

  \param P      Pointer to ImageMetadataQueue structure.
  \param QPC_current_presented_earliest  Maximal allowed render timestamp in QPC units.
  All items which are rendered before this timestamp and were not triggered are removed from the queue.
  \return Returns number of items removed or negative value if unsuccessfull.
*/
int
RemoveMissedImages(
                   ImageMetadataQueue * const P,
                   __int64 const QPC_current_presented_earliest
                   )
{
  assert(NULL != P);
  if (NULL == P) return -1;

  assert(NULL != P->pMetadataQueue);
  if (NULL == P->pMetadataQueue) return -1;

  std::vector<ImageMetadata> * const pQueue = new std::vector<ImageMetadata>;
  assert(NULL != pQueue);
  if (NULL == pQueue) return -1;

  std::vector<ImageMetadata> * pTmp = NULL;
  std::vector<ImageMetadata>::iterator it;

  assert( 0 < QPC_current_presented_earliest );
  int removed = 0;

  AcquireSRWLockExclusive( &(P->sQueueLock) );
  {
    pQueue->reserve( P->pMetadataQueue->capacity() );

    for (it = P->pMetadataQueue->begin(); it != P->pMetadataQueue->end(); ++it)
      {
        __int64 const QPC_current_presented = it->QPC_current_presented;
        bool const triggered = it->fTrigger;
        if ( (true == triggered) || (QPC_current_presented >= QPC_current_presented_earliest) )
          {
            pQueue->push_back( *it );
            ImageMetadataBlank( &(*it) );
          }
        else
          {
            removed++;
          }
        /* if */
      }
    /* for */

    pTmp = P->pMetadataQueue;
    P->pMetadataQueue = pQueue;
  }
  ReleaseSRWLockExclusive( &(P->sQueueLock) );


  while ( false == pTmp->empty() )
    {
      ImageMetadata sData = pTmp->back();
      pTmp->pop_back();
      ImageMetadataRelease( &sData );
    }
  /* while */
  SAFE_DELETE( pTmp );

  return removed;
}
/* RemoveMissedImages */

#pragma endregion // Remove items



#endif /* !__BATCHACQUISITIONIMAGEQUEUE_CPP */
