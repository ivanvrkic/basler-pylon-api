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
  \file   BatchAcquisitionImageDecoder.cpp
  \brief  Image load functions.

  Defines image loading functions that use Window Imaging Components and Direct 2D.

  Images are loaded in a separate thread that maintains one decoder image queue.
  Loaded images are decoded to memory using a image format compatible with
  DirectX so when image must be rendered the only operation is a bitmap block
  transfer to the output buffer. Length of the decoded image queue and minimal
  number of decoded images are set at compile time as static members of structure
  ImageDecoderParameters.

  Each rendering thread maintains its own image decoder queue.

  \author Tomislav Petkovic
  \date   2017-03-01
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONIMAGEDECODER_CPP
#define __BATCHACQUISITIONIMAGEDECODER_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionImageDecoder.h"
#include "BatchAcquisitionDebug.h"




/****** HELPER FUNCTIONS ******/

#pragma region // Inline helper functions

//! Blanks class variables.
/*!
  Blanks class variables.

  \param P      Pointer to class.
*/
inline
void
BlankQueuedDecoderImage_inline(
                               QueuedDecoderImage * const P
                               )
{
  assert(NULL != P);
  if (NULL == P) return;

  P->no = 0;
  P->render_type = QI_UNKNOWN_TYPE;

  P->pattern_type = SL_PATTERN_INVALID;
  P->index = -1;
  P->retry = 0;

  P->projectorID = -1;

  P->pBitmap = NULL;
  P->pURI = NULL;
  P->pFilename = NULL;

  P->red = 0.0f;
  P->green = 0.0f;
  P->blue = 0.0f;
  P->alpha = 1.0f;

  P->delay = 0.0;
  P->exposure = 0.0;

  P->fSkipAcquisition = false;
}
/* BlankQueuedDecoderImage_inline */



//! Blanks image decoder parameters.
/*!
  Blanks image decoder parameters.

  \param P      Pointer to image decoder parametes.
*/
inline
void
ImageDecoderParametersBlank_inline(
                                   ImageDecoderParameters * const P
                                   )
{
  assert(NULL != P);
  if (NULL == P) return;

  /* Blank structure. */
  P->pImageQueue = NULL;
  P->count = 0;
  P->maxItems = 18;
  P->minItems = 9;
  P->DecoderID = -1;
  P->ProjectorID = -1;
  P->tImageDecoder = (HANDLE)(NULL);
  P->fActive = false;
  P->fWaiting = false;
  P->pImageList = NULL;
  P->pSynchronization = NULL;
  P->pWICFactory = NULL;

  ZeroMemory( &(P->sLockImageQueue), sizeof(P->sLockImageQueue) );
}
/* ImageDecoderParametersBlank_inline */



//! Releases image decoder parameters structure.
/*!
  Releases resources allocated by image decoder.

  \param P      Pointer to image decoder parametes.
*/
inline
void
ImageDecoderParametersRelease_inline(
                                     ImageDecoderParameters * const P
                                     )
{
  assert(NULL != P);
  if (NULL == P) return;

  AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        while ( !P->pImageQueue->empty() )
          {
            QueuedDecoderImage * item = ImageDecoderFetchImage(P, false);
            SAFE_DELETE( item );
          }
        /* while */
        assert( P->pImageQueue->empty() );
      }
    /* if */

    SAFE_DELETE(P->pImageQueue);
    assert(NULL == P->pImageQueue);
  }
  ReleaseSRWLockExclusive( &(P->sLockImageQueue) );

  ImageDecoderParametersBlank_inline( P );

  free(P);
}
/* ImageDecoderParametersRelease_inline */

#pragma endregion // Inline helper functions


/****** IMAGE CONTAINER CLASS ******/

#pragma region // Image decoder queue item

//! Constructor.
/*!
  Creates queued image from file.

  \param pIWICFactory   Pointer to Windows Imaging Component (WIC) factory.
  \param URI       Filename or URI of the image.
*/
QueuedDecoderImage::QueuedDecoderImage(
                                       IWICImagingFactory * const pIWICFactory,
                                       wchar_t const * const URI
                                       )
{
  /* Blank class variables. */
  BlankQueuedDecoderImage_inline(this);

  /* Load image from file. */
  HRESULT const hr = ImageLoadFromFile(pIWICFactory, URI, &(this->pBitmap));
  //assert( SUCCEEDED(hr) );

  /* Initialize class. */
  if ( SUCCEEDED(hr) )
    {
      this->render_type = QI_BGRA_BITMAP;
      if (NULL != URI) this->pURI = new std::wstring(URI);
    }
  else
    {
      assert(QI_UNKNOWN_TYPE == this->render_type);
    }
  /* if */
}
/* QueuedDecoderImage::QueuedDecoderImage */



//! Constructor.
/*!
  Creates queued image from color.

  \param color Color of a solid structured light pattern.
*/
QueuedDecoderImage::QueuedDecoderImage(
                                       D3DCOLORVALUE const color
                                       )
{
  /* Blank class variables. */
  BlankQueuedDecoderImage_inline(this);

  this->red = color.r;
  this->green = color.g;
  this->blue = color.b;
  this->alpha = color.a;
  this->render_type = QI_PATTERN_SOLID;
}
/* QueuedDecoderImage::QueuedDecoderImage */



//! Destructor
/*!
  Releases all resources allocates by queued image.
*/
QueuedDecoderImage::~QueuedDecoderImage()
{
  SAFE_RELEASE(this->pBitmap);
  SAFE_DELETE(this->pURI);
  SAFE_DELETE(this->pFilename);

  BlankQueuedDecoderImage_inline(this);
}
/* QueuedDecoderImage::~QueuedDecoderImage */

#pragma endregion // Image decoder queue item


/****** IMAGE DECODER THREAD ******/

//! Image decoder thread.
/*!
  Image decoder thread.

  \param parameters_in Pointer to structure holding image decoder thread parameters.
  \return Returns 0 if successfull.
*/
unsigned int
__stdcall
ImageDecoderThread(
                   void * parameters_in
                   )
{

#pragma region // Initialization

  assert(NULL != parameters_in);
  if (NULL == parameters_in) return EXIT_FAILURE;

  ImageDecoderParameters * const parameters = (ImageDecoderParameters *)parameters_in;

  SetThreadNameAndIDForMSVC(-1, "ImageDecoderThread", parameters->DecoderID);

  SynchronizationEvents * const pSynchronization = parameters->pSynchronization;
  assert(NULL != pSynchronization);

  int DecoderID = parameters->DecoderID;
  assert( (0 <= DecoderID) && (DecoderID < (int)(pSynchronization->ImageDecoder.size())) );

  int ProjectorID = parameters->ProjectorID;
  assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );

  /* Fill image queue list. */
  int count = 0;
  int const c0 = ImageDecoderFillQueue(parameters);
  assert( c0 == parameters->maxItems );
  count += c0;

  PastEvents * const pEvents = PastEventsCreate();

  parameters->fActive = true;

  bool continueLoop = true;

#pragma endregion // Initialization

  /* Events are processed in an infinite loop. */
  do
    {
      assert(NULL != pSynchronization);
      if (NULL != pSynchronization)
        {
          assert(false == parameters->fWaiting);
          parameters->fWaiting = true;

          DWORD const dwWaitResult =
            pSynchronization->EventWaitForAny(
                                              IMAGE_DECODER_QUEUE_TERMINATE, DecoderID, // 0
                                              IMAGE_DECODER_QUEUE_PROCESS,   DecoderID, // 1
                                              IMAGE_DECODER_CHANGE_ID,       DecoderID, // 2
                                              INFINITE // Wait forewer.
                                              );
          int const hnr = dwWaitResult - WAIT_OBJECT_0;
          assert( (0 <= hnr) && (hnr < 3) );
          AddEvent(pEvents, hnr);

          parameters->fWaiting = false;

          if (0 == hnr)
            {
              // We received terminate event.
              continueLoop = false;
            }
          else if (1 == hnr)
            {
              // Process items.
              int const c1 = ImageDecoderFillQueue(parameters);
              count += c1;

              // The following assertion is valid only if items are inserted exclusively from this thread.
              //assert(count == parameters->count);

              // Reset processing signal only after the processing is done.
              BOOL const reset_process = pSynchronization->EventReset(IMAGE_DECODER_QUEUE_PROCESS, DecoderID);
              assert(0 != reset_process);
            }
          else if (2 == hnr)
            {
              // Store old event ID.
              int const DecoderIDOld = DecoderID;

              // Output event ID change message.
              if (DecoderIDOld != parameters->DecoderID)
                {
                  Debugfprintf(stderr, gDbgImageDecoderIDChanged, DecoderIDOld + 1, DecoderIDOld + 1, parameters->DecoderID + 1);

                  SetThreadNameAndIDForMSVC(-1, "ImageDecoderThread", parameters->DecoderID);
                }
              else
                {
                  Debugfprintf(stderr, gDbgImageDecoderIDNotChanged, DecoderIDOld + 1);
                }
              /* if */
              
              // Fetch new event ID values.
              {
                DecoderID = parameters->DecoderID;
                assert( (0 <= DecoderID) && (DecoderID < (int)(pSynchronization->ImageDecoder.size())) );

                ProjectorID = parameters->ProjectorID;
                assert( (0 <= ProjectorID) && (ProjectorID < (int)(pSynchronization->Draw.size())) );                
              }
              
              // Update queue items.
              {
                int const c1 = ImageDecoderUpdateProjectorID(parameters);
                assert(0 <= c1);
              }

              // Reset signal; note that we have to use the old ID.
              {
                BOOL const reset_change_id = pSynchronization->EventReset(IMAGE_DECODER_CHANGE_ID, DecoderIDOld);
                assert(0 != reset_change_id);
              }
            }
          else
            {
              // We received unknown event!
            }
          /* if */
        }
      else
        {
          continueLoop = false;
        }
      /* if */
    }
  while ( true == continueLoop );

  PastEventsDelete( pEvents );

  {
    BOOL const reset_terminate = pSynchronization->EventReset(IMAGE_DECODER_QUEUE_TERMINATE, DecoderID);
    assert(0 != reset_terminate);
  }

  parameters->fActive = false;

  return 0;
}
/* ImageDecoderThread */



/****** DECODER QUEUE ******/

#pragma region // Fill image decoder queue

//! Queue image to decoder.
/*!
  Queues image to image decoder queue.

  \param P      Pointer to image decoder thread parameters.
  \param item     Pointer to image to queue.
  \return Return true if successfull, false otherwise.
*/
bool
ImageDecoderQueueImage(
                       ImageDecoderParameters * const P,
                       QueuedDecoderImage * const item
                       )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != item);
  if (NULL == item) return false;

  //assert(QI_UNKNOWN_TYPE != item->render_type);
  if (QI_UNKNOWN_TYPE == item->render_type) return false;

  int size = 0;
  bool empty = false;

  AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        // Maintain proper item count.
        int const no = (P->count)++;
        item->no = no;

        // Insert item.
        P->pImageQueue->push_back( item );

        // Get queue state.
        size = (int)( P->pImageQueue->size() );
        empty = P->pImageQueue->empty();
      }
    /* if */
  }
  ReleaseSRWLockExclusive( &(P->sLockImageQueue) );

  if (NULL != P->pSynchronization)
    {
      if (size >= P->maxItems)
        {
          // If the queue has more than the preset maximum number of items then signal the queue is full.
          // Consumer threads may use this signal to adjust the consumption speed.
          BOOL const set_full = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_FULL, P->DecoderID);
          assert(0 != set_full);

          BOOL const reset_process = P->pSynchronization->EventReset(IMAGE_DECODER_QUEUE_PROCESS, P->DecoderID);
          assert(0 != reset_process);
        }
      /* if */

      if ( (true == empty) || (size <= P->minItems) )
        {
          // If the queue does not have enough items signal more images should be decoded.
          BOOL const set_process = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, P->DecoderID);
          assert(0 != set_process);
        }
      /* if */

      if (false == empty)
        {
          BOOL const reset_empty = P->pSynchronization->EventReset(IMAGE_DECODER_QUEUE_EMPTY, P->DecoderID);
          assert(0 != reset_empty);
        }
      /* if */
    }
  /* if */

  return true;
}
/* ImageDecoderQueueImage */



//! Fill image queue.
/*!
  Function fills image decoder queue.

  \param P      Pointer to structure holding image decoder thread parameters.
  \return Returns number of items inserted in the queue.
*/
int
ImageDecoderFillQueue(
                      ImageDecoderParameters * const P
                      )
{
  int num_inserted = 0;

  assert(NULL != P);
  if (NULL == P) return num_inserted;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return num_inserted;

  assert(NULL != P->pImageList);
  if (NULL == P->pImageList) return num_inserted;

  int size = 0;
  int count = 0;

  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        size = (int)( P->pImageQueue->size() );
        count = P->count;
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  for (int i = size; i < P->maxItems; ++i)
    {
      QueuedDecoderImage * item = NULL;

      StructuredLightPatternType pattern_type = SL_PATTERN_INVALID;

      double const delay = 0.0;
      double const exposure = 0.0;
      bool const skip_acquisition = false;

      wchar_t filename[MAX_PATH];
      bool const get_filename = P->pImageList->GetFileName(filename, MAX_PATH);

      int const index = P->pImageList->GetFileIndex();
      bool const get_index = (-1 != index);

      bool const have_filename = P->pImageList->HaveFileName();
      if (true == have_filename)
        {
          wchar_t fullname[MAX_PATH];
          bool const get_URI = P->pImageList->GetFullFileName(fullname, MAX_PATH);

          pattern_type = SL_PATTERN_FROM_FILE;

          bool const get = get_URI && get_filename && get_index;
          if (true != get)
            {
              bool const next = P->pImageList->Next();
              continue;
            }
          /* if */

          assert(NULL == item);
          item = new QueuedDecoderImage(P->pWICFactory, fullname);
        }
      else
        {
          pattern_type = SL_PATTERN_BLACK;

          bool const get = get_filename && get_index;
          if (true != get)
            {
              bool const next = P->pImageList->Next();
              continue;
            }
          /* if */

          D3DCOLORVALUE color_black;
          color_black.r = 0.0f;
          color_black.g = 0.0f;
          color_black.b = 0.0f;
          color_black.a = 1.0f;

          assert(NULL == item);
          item = new QueuedDecoderImage(color_black);
          assert(NULL != item);
        }
      /* if */

      if (NULL != item)
        {
          item->pattern_type = pattern_type;
          item->index = index;
          item->projectorID = P->ProjectorID;
          SAFE_DELETE(item->pFilename);
          item->pFilename = new std::wstring(filename);
          item->delay = delay;
          item->exposure = exposure;
          item->fSkipAcquisition = skip_acquisition;
        }
      /* if */

      bool const inserted = ImageDecoderQueueImage(P, item);
      if (true == inserted)
        {
          // Following assertion is valid only if items are inserted exclusively from ImageDecoderThread thread.
          //assert(count + num_inserted == item->no);

          ++num_inserted;
        }
      else
        {
          SAFE_DELETE(item);
        }
      /* if */

      bool const next = P->pImageList->Next();
    }
  /* for */

  if (NULL != P->pSynchronization)
    {
      if (size >= P->maxItems)
        {
          BOOL const set_full = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_FULL, P->DecoderID);
          assert(0 != set_full);
        }
      else if ( (num_inserted < P->maxItems) && (false == P->pImageList->cycle) && (num_inserted == P->pImageList->Size()) )
        {
          // When starting the acquisition the number of input images may be insufficient to raise the
          // IMAGE_DECODER_QUEUE_FULL signal automatically. We raise the signal here for such cases.
          BOOL const set_full = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_FULL, P->DecoderID);
          assert(0 != set_full);
        }
      /* if */
    }
  /* if */

  return num_inserted;
}
/* ImageDecoderFillQueue */



//! Update projector event ID.
/*!
  Function updates projector ID for all already queued items.

  \param P      Pointer to image decoder thread.
  \return Function returns number of items updated.
*/
int
ImageDecoderUpdateProjectorID(
                              ImageDecoderParameters * const P
                              )
{
  int num_changed = 0;

  assert(NULL != P);
  if (NULL == P) return num_changed;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return num_changed;

  int const ProjectorID = P->ProjectorID;

  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        std::deque<QueuedDecoderImage *>::iterator it = P->pImageQueue->begin();
        for ( ; it != P->pImageQueue->end(); ++it)
          {
            QueuedDecoderImage * const item = *it;
            if ( (NULL != item) && (item->projectorID != ProjectorID) )
              {
                item->projectorID = ProjectorID;
                ++num_changed;
              }
            /* if */
          }
        /* for */
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );
  
  return num_changed;
}
/* ImageDecoderUpdateProjectorID */

#pragma endregion // Fill image decoder queue


#pragma region // Pop image from decoder queue

//! Check if next image is available.
/*!
  Checks if next image is available in the image queue.

  \param P      Pointer to structure holding image decoder thread parameters.
  \return Returns pointer to image container class or NULL if unsuccessfull.
*/
bool
ImageDecoderHaveNext(
                     ImageDecoderParameters * const P
                     )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(NULL != P->pImageList);
  if (NULL == P->pImageList) return false;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return false;

  bool cycle = false;
  bool have_next_in_list = false;
  bool have_next_in_queue = false;

  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageList)
      {
        cycle = P->pImageList->cycle; // Check if cycling is turned on.
        have_next_in_list = ! P->pImageList->AtEnd();
      }
    /* if */

    if (NULL != P->pImageQueue)
      {
        have_next_in_queue = ! P->pImageQueue->empty();
      }
    /* if */
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  // If the queue is empty then signal to the image decoder to start filling it.
  if (false == have_next_in_queue)
    {
      BOOL const set_empty = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_EMPTY, P->DecoderID);
      assert(0 != set_empty);

      BOOL const set_process = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, P->DecoderID);
      assert(0 != set_process);
    }
  /* if */

  // We cannot run out of images if cycling is turned on.
  if (true == cycle) return true;

  // If cycling is turned off then first the list pImageList must reach its end
  // after which the queue pImageQueue must become empty.
  bool const have_next = have_next_in_list || have_next_in_queue;
  return have_next;
}
/* ImageDecoderHaveNext */



//! Fetch next image to display.
/*!
  Fetches next image from the image queue.

  \param P      Pointer to structure holding image decoder thread parameters.
  \param acquire_SRW_lock Flag to indicate if lock must be acquired.
  If true the function will wait on SRW lock; otherwise it assumes lock is already acquired.
  \return Returns pointer to image container class or NULL if unsuccessfull.
*/
QueuedDecoderImage *
ImageDecoderFetchImage(
                       ImageDecoderParameters * const P,
                       bool const acquire_SRW_lock
                       )
{
  assert(NULL != P);
  if (NULL == P) return NULL;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return NULL;

  QueuedDecoderImage * item = NULL;
  int size = 0;
  bool empty = false;

  if (true == acquire_SRW_lock) AcquireSRWLockExclusive( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue)
      {
        // Fetch image from queue.
        size = (int)( P->pImageQueue->size() );
        if (0 < size)
          {
            item = P->pImageQueue->front();
            P->pImageQueue->pop_front();
          }
        /* if */
        empty = P->pImageQueue->empty();
      }
    /* if */
  }
  if (true == acquire_SRW_lock) ReleaseSRWLockExclusive( &(P->sLockImageQueue) );

  if (NULL != P->pSynchronization)
    {
      if (true == empty)
        {
          // Signal if the queue is empty.
          BOOL const set_empty = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_EMPTY, P->DecoderID);
          assert(0 != set_empty);

          BOOL const set_process = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, P->DecoderID);
          assert(0 != set_process);
        }
      /* if */

      if (size <= P->minItems)
        {
          // If the number of items falls below the preset minimum then signal to the image decoder
          // to start decoding images from a storage device and filling the queue.
          BOOL const set_process = P->pSynchronization->EventSet(IMAGE_DECODER_QUEUE_PROCESS, P->DecoderID);
          assert(0 != set_process);
        }
      /* if */

      if (size < P->maxItems)
        {
          BOOL const reset_full = P->pSynchronization->EventReset(IMAGE_DECODER_QUEUE_FULL, P->DecoderID);
          assert(0 != reset_full);
        }
      /* if */
    }
  /* if */

  return item;
}
/* ImageDecoderFetchImage */

#pragma endregion // Pop image from decoder queue


#pragma region // Test status and count items

//! Test if all images are queued.
/*!
  Returns true if all files were queued for processing. Note that if this function returns
  true it does not mean the files are actually processed; a number of unprocessed files
  may be stil queued.

  \param P      Pointer to structure holding image decoder thread parameters.
  \return Returns true if all files are processed.
*/
bool
ImageDecoderAllFilesQueued(
                           ImageDecoderParameters * const P
                           )
{
  assert(NULL != P);
  if (NULL == P) return true;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return true;

  assert(NULL != P->pImageList);
  if (NULL == P->pImageList) return true;

  bool cycle = false;
  int size = 0;
  bool atend = true;

  EnterCriticalSection( &(P->pImageList->csFileList) );
  {
    cycle = P->pImageList->cycle;
    size = P->pImageList->Size();
    atend = P->pImageList->AtEnd();
  }
  LeaveCriticalSection( &(P->pImageList->csFileList) );

  if (0 >= size) return true;
  if (true == cycle) return false;
  if (true == atend) return true;

  return false;
}
/* ImageDecoderAllFilesQueued */



//! Count items.
/*!
  Returns number of items in the queue.

  \param P      Pointer to structure holding image decoder thread parameters.
  \return Number of items in the queue.
*/
int
ImageDecoderNumOfQueuedItems(
                             ImageDecoderParameters * const P
                             )
{
  assert(NULL != P);
  if (NULL == P) return true;

  assert(NULL != P->pImageQueue);
  if (NULL == P->pImageQueue) return true;

  int size = 0;
  AcquireSRWLockShared( &(P->sLockImageQueue) );
  {
    if (NULL != P->pImageQueue) size = (int)( P->pImageQueue->size() );
  }
  ReleaseSRWLockShared( &(P->sLockImageQueue) );

  return size;
}
/* ImageDecoderNumOfQueuedItems */

#pragma endregion // Test status and count items



/****** START/STOP THREAD ******/

#pragma region // Decoder start and stop

//! Create image decoder parameters and start decoder thread.
/*!
  Spawns image decoder thread.

  \param pImageList     Pointer to file list.
  \param pSynchronization       Pointer to a structure holding all required syncrhonization events.
  \param pWICFactory   Pointer to Window Image Component factory.
  \param DecoderID  Unique thread identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \param ProjectorID   Unique projector identifier. Must be a non-negative number that indexes a corresponding slot in pSynchronization structure.
  \return Returns pointer to decoder thread parameters or NULL if unsuccessfull.
*/
ImageDecoderParameters *
ImageDecoderStart(
                  ImageFileList * const pImageList,
                  SynchronizationEvents * const pSynchronization,
                  IWICImagingFactory * const pWICFactory,
                  int const DecoderID,
                  int const ProjectorID
                  )
{
  ImageDecoderParameters * const P = (ImageDecoderParameters *)malloc( sizeof(ImageDecoderParameters) );
  assert(NULL != P);
  if (NULL == P) return P;

  ImageDecoderParametersBlank_inline( P );

  /* Initialize variables. */
  InitializeSRWLock( &(P->sLockImageQueue) );

  assert(NULL == P->pImageQueue);
  P->pImageQueue = new std::deque<QueuedDecoderImage *>();
  assert(NULL != P->pImageQueue);

  if (NULL == P->pImageQueue) goto IMAGE_DECODER_THREAD_START_EXIT;

  /* Copy parameters. */
  assert(NULL == P->pImageList);
  P->pImageList = pImageList;
  assert(NULL != P->pImageList);

  assert(NULL == P->pSynchronization);
  P->pSynchronization = pSynchronization;
  assert(NULL != P->pSynchronization);

  assert(NULL == P->pWICFactory);
  P->pWICFactory = pWICFactory;
  assert(NULL != P->pWICFactory);

  assert(-1 == P->DecoderID);
  P->DecoderID = DecoderID;
  assert( (0 <= P->DecoderID) && (P->DecoderID < (int)(P->pSynchronization->ImageDecoder.size())) );

  assert(-1 == P->ProjectorID);
  P->ProjectorID = ProjectorID;
  assert( (0 <= P->ProjectorID) && (P->ProjectorID < (int)(P->pSynchronization->Draw.size())) );

  /* Start decoder thread. */
  P->tImageDecoder =
    (HANDLE)( _beginthreadex(
                             NULL, // No security atributes.
                             0, // Automatic stack size.
                             ImageDecoderThread,
                             (void *)( P ),
                             0, // Thread starts immediately.
                             NULL // Thread identifier not used.
                             )
              );
  assert( (HANDLE)( NULL ) != P->tImageDecoder );
  if ( (HANDLE)( NULL ) == P->tImageDecoder )
    {

    IMAGE_DECODER_THREAD_START_EXIT:
      ImageDecoderParametersRelease_inline( P );
      return NULL;

    }
  /* if */

  return P;
}
/* ImageDecoderStart */



//! Stop image decoder thread.
/*!
  Stops image decoder thread.

  \param P      Pointer to image decoder thread parameters.
*/
void
ImageDecoderStop(
                 ImageDecoderParameters * const P
                 )
{
  //assert(NULL != P);
  if (NULL == P) return;

  SynchronizationEvents * const pSynchronization = P->pSynchronization;
  int const DecoderID = P->DecoderID;

  assert(NULL != pSynchronization);
  if (NULL != pSynchronization)
    {
      DWORD const result = WaitForSingleObject(P->tImageDecoder, 0);

      if ( (WAIT_OBJECT_0 != result) && (true == P->fActive) )
        {
          // The thread is alive so signal terminate event and wait for confirmation.
          BOOL const set_terminate = pSynchronization->EventSet(IMAGE_DECODER_QUEUE_TERMINATE, DecoderID);
          assert(0 != set_terminate);
          if (0 != set_terminate)
            {
              DWORD const confirm = WaitForSingleObject(P->tImageDecoder, INFINITE);
              assert(WAIT_OBJECT_0 == confirm);
            }
          /* if */
        }
      else
        {
          // The thread has already terminated.
        }
      /* if */
    }
  /* if */

  assert( WAIT_OBJECT_0 == WaitForSingleObject(P->tImageDecoder, 0) );
  assert( false == P->fActive );

  ImageDecoderParametersRelease_inline( P );

  if (NULL != pSynchronization)
    {
      BOOL const reset_decoder = pSynchronization->EventResetAllImageDecoder(DecoderID);
      assert(0 != reset_decoder);
    }
  /* if */
}
/* ImageDecoderStop */

#pragma endregion // Decoder start and stop


#endif /* !__BATCHACQUISITIONIMAGEDECODER_CPP */
