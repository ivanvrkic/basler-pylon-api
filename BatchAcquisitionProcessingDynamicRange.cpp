/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2017 UniZG, Zagreb. All rights reserved.
 * (c) 2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionProcessingDynamicRange.cpp
  \brief  Dynamic range and texture estimation methods.

  Function for computing dynamic range and texture for
  phase shifting.

  \author Tomislav Petkovic
  \date   2017-06-13
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGDYNAMICRANGE_CPP
#define __BATCHACQUISITIONPROCESSINGDYNAMICRANGE_CPP


#include "BatchAcquisitionProcessingDynamicRange.h"
#include "BatchAcquisitionImage.h"
#include "BatchAcquisitionImageConversion.h"



/****** HELPER FUNCTIONS ******/

//! Validate inputs.
/*!
  Validates inputs.

  \param AllImages Class containing all acquired images.
  \param first  Index of first image for processing.
  \param last   Index of last image for processing.
  \return Function returns true if input parameters are valid.
*/
inline
bool
ValidateInputs_inline(
                      ImageSet const * const AllImages,
                      int const first,
                      int const last
                      )
{
  assert(NULL != AllImages);
  if (NULL == AllImages) return false;

  assert(NULL != AllImages->data);
  if (NULL == AllImages->data) return false;

  assert( last >= first );
  if ( last < first ) return false;

  assert( (0 <= first) && (first < AllImages->num_images) );
  if ( (first < 0) || (AllImages->num_images <= first) ) return false;

  assert( (0 <= last) && (last < AllImages->num_images) );
  if ( (last < 0) || (AllImages->num_images <= last) ) return false;

  return true;
}
/* ValidateInputs_inline */



/****** DYNAMIC RANGE AND TEXTURE ESTIMATION ******/


//! Dynamic range estimation (single precision).
/*!
  Function estimates grayscale dynamic range of phase shifted images.

  First selected image is assumed to have the phase zero.
  Last selected image is assumed to have the phase 2*pi*(last-first)/(last-first+1).
  All other images have the phase equidistantly spread between the first and the last phase.

  \param AllImages      Pointer to class containing all acquired images.
  \param first  Index of the first image.
  \param last   Index of the last image (inclusive).
  \return Function returns a pointer to valid cv::Mat or NULL if unsuccessfull.
*/
cv::Mat *
EstimateDynamicRange(
                     ImageSet * const AllImages,
                     int const first,
                     int const last
                     )
{
  cv::Mat * dynamic_range = NULL; // Dynamic range.
  cv::Mat * gray_max = NULL; // Maximum value.
  cv::Mat * gray_min = NULL; // Minimum value.

  bool const inputs_valid = ValidateInputs_inline(AllImages, first, last);
  if (false == inputs_valid) return dynamic_range;

  int const num_images = last - first + 1;
  assert(0 < num_images);

  // Allocate data storage for accumulators and end result.
  int const cols = AllImages->width;
  int const rows = AllImages->height;

  dynamic_range = new cv::Mat(rows, cols, CV_32FC1, 0.0f);
  assert(NULL != dynamic_range);

  gray_max = new cv::Mat(rows, cols, CV_32FC1, BATCHACQUISITION_nINF_fv);
  gray_min = new cv::Mat(rows, cols, CV_32FC1, BATCHACQUISITION_pINF_fv);
  assert(NULL != gray_max);
  assert(NULL != gray_min);

  if ( (NULL == dynamic_range) || (NULL == gray_max) || (NULL == gray_min) ) goto EstimateDynamicRange_EXIT;

  // Find minimum and maximum value.
  for (int i = first; i <= last; ++i)
    {
      cv::Mat * gray = AllImages->GetImageGray(i);
      assert(NULL != gray);
      if (NULL == gray) goto EstimateDynamicRange_EXIT;

      // Re-use preallocated storage.
      gray->convertTo(*dynamic_range, CV_32FC1);

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          float const * const row_gray = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
          float       * const row_gray_max = (float *)( (BYTE *)(gray_max->data) + gray_max->step[0] * y );
          float       * const row_gray_min = (float *)( (BYTE *)(gray_min->data) + gray_min->step[0] * y );

          // Unrolled for loop with step 4.
          int x = 0;
          int const max_x = cols - 3;
          for (; x < max_x; x += 4)
            {
              if (row_gray[x    ] > row_gray_max[x    ]) row_gray_max[x    ] = row_gray[x    ];
              if (row_gray[x + 1] > row_gray_max[x + 1]) row_gray_max[x + 1] = row_gray[x + 1];
              if (row_gray[x + 2] > row_gray_max[x + 2]) row_gray_max[x + 2] = row_gray[x + 2];
              if (row_gray[x + 3] > row_gray_max[x + 3]) row_gray_max[x + 3] = row_gray[x + 3];

              if (row_gray[x    ] < row_gray_min[x    ]) row_gray_min[x    ] = row_gray[x    ];
              if (row_gray[x + 1] < row_gray_min[x + 1]) row_gray_min[x + 1] = row_gray[x + 1];
              if (row_gray[x + 2] < row_gray_min[x + 2]) row_gray_min[x + 2] = row_gray[x + 2];
              if (row_gray[x + 3] < row_gray_min[x + 3]) row_gray_min[x + 3] = row_gray[x + 3];
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              if (row_gray[x] > row_gray_max[x]) row_gray_max[x] = row_gray[x];
              if (row_gray[x] < row_gray_min[x]) row_gray_min[x] = row_gray[x];
            }
          /* for */
        }
      /* for */

      SAFE_DELETE(gray);
    }
  /* for */

  // Compute dynamic range.
  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      float       * const row_dynamic_range = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
      float const * const row_gray_max = (float *)( (BYTE *)(gray_max->data) + gray_max->step[0] * y );
      float const * const row_gray_min = (float *)( (BYTE *)(gray_min->data) + gray_min->step[0] * y );

      // Unrolled for loop with step 4.
      int x = 0;
      int const max_x = cols - 3;
      for (; x < max_x; x += 4)
        {
          row_dynamic_range[x    ] = row_gray_max[x    ] - row_gray_min[x    ];
          row_dynamic_range[x + 1] = row_gray_max[x + 1] - row_gray_min[x + 1];
          row_dynamic_range[x + 2] = row_gray_max[x + 2] - row_gray_min[x + 2];
          row_dynamic_range[x + 3] = row_gray_max[x + 3] - row_gray_min[x + 3];
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_dynamic_range[x] = row_gray_max[x] - row_gray_min[x];
        }
      /* for */
    }
  /* for */


 EstimateDynamicRange_EXIT:

  SAFE_DELETE( gray_max );
  SAFE_DELETE( gray_min );

  return dynamic_range;
}
/* EstimateDynamicRange */



//! Combine dynamic ranges (single precision).
/*!
  Combines dynamic ranges by keeping the lowest dynamic range.

  \param range_1        First dynamic range image; must be single precision.
  \param range_2        Second dynamic range image; must be single precision.
  \return Returns pointer to combined dynamic range image or NULL if unsuccessfull.
*/
cv::Mat *
CombineDynamicRanges(
                     cv::Mat const * const range_1,
                     cv::Mat const * const range_2
                     )
{
  assert( (NULL != range_1) && (NULL != range_1->data) );
  if ( (NULL == range_1) || (NULL == range_1->data) ) return NULL;

  assert( (NULL != range_2) && (NULL != range_2->data) );
  if ( (NULL == range_2) || (NULL == range_2->data) ) return NULL;

  assert( (CV_MAT_DEPTH(range_1->type()) == CV_32F) && (CV_MAT_CN(range_1->type()) == 1) );
  if ( (CV_MAT_DEPTH(range_1->type()) != CV_32F) || (CV_MAT_CN(range_1->type()) != 1) ) return NULL;

  assert( (CV_MAT_DEPTH(range_2->type()) == CV_32F) && (CV_MAT_CN(range_2->type()) == 1) );
  if ( (CV_MAT_DEPTH(range_2->type()) != CV_32F) || (CV_MAT_CN(range_2->type()) != 1) ) return NULL;

  int const cols = range_1->cols;
  int const rows = range_1->rows;

  assert( (cols == range_2->cols) && (rows == range_2->rows) );
  if ( (cols != range_2->cols) || (rows != range_2->rows) ) return NULL;

  cv::Mat * dynamic_range = new cv::Mat(rows, cols, CV_32FC1, 0.0f);
  assert(NULL != dynamic_range);
  if (NULL == dynamic_range) return NULL;

  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      float const * const row_range_1 = (float *)( (BYTE *)(range_1->data) + range_1->step[0] * y );
      float const * const row_range_2 = (float *)( (BYTE *)(range_2->data) + range_2->step[0] * y );
      float       * const row_dynamic_range = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );

      // Unrolled for loop with step 4.
      int x = 0;
      int const max_x = cols - 3;
      for (; x < max_x; x += 4)
        {
          row_dynamic_range[x    ] = (row_range_1[x    ] < row_range_2[x    ])? row_range_1[x    ] : row_range_2[x    ];
          row_dynamic_range[x + 1] = (row_range_1[x + 1] < row_range_2[x + 1])? row_range_1[x + 1] : row_range_2[x + 1];
          row_dynamic_range[x + 2] = (row_range_1[x + 2] < row_range_2[x + 2])? row_range_1[x + 2] : row_range_2[x + 2];
          row_dynamic_range[x + 3] = (row_range_1[x + 3] < row_range_2[x + 3])? row_range_1[x + 3] : row_range_2[x + 3];
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_dynamic_range[x] = (row_range_1[x] < row_range_2[x])? row_range_1[x] : row_range_2[x];
        }
      /* for */
    }
  /* for */

  return dynamic_range;
}
/* CombineDynamicRange */



//! Dynamic range and texture estimation (single precision).
/*!
  Function estimates grayscale dynamic range of phase shifted images.
  For texture estimation to be successfull all phase shifted images must
  sum to a constant.

  \param AllImages      Pointer to class containing all acquired images.
  \param first  Index of the first image.
  \param last   Index of the last image (inclusive).
  \param dynamic_range_in_out Address where a pointer to cv::Mat which holds dynamic range image is stored.
  Image data must be of CV_32FC1 type. Pointer may be NULL in which case the image will be created.
  \param texture_in_out Address where a pointer to cv::Mat which holds texture image is stored.
  Image data must be of CV_32FC1 type. Pointer may be NULL in which case the image will be created.
  \return Function returns true if successfull and false otherwise.
*/
bool
UpdateDynamicRangeAndTexture(
                             ImageSet * const AllImages,
                             int const first,
                             int const last,
                             cv::Mat * * const dynamic_range_in_out,
                             cv::Mat * * const texture_in_out
                             )
{
  bool result = true; // Assume success.

  cv::Mat * dynamic_range = NULL; // Dynamic range and temporary storage.
  cv::Mat * texture = NULL; // Texture.
  cv::Mat * texture_tmp = NULL; // Temporary texture storage.
  cv::Mat * gray_max = NULL; // Maximum value.
  cv::Mat * gray_min = NULL; // Minimum value.

  bool const inputs_valid = ValidateInputs_inline(AllImages, first, last);
  if (false == inputs_valid)
    {
      result = false;
      goto UpdateDynamicRangeAndTexture_EXIT;
    }
  /* if */

  int const num_images = last - first + 1;
  assert(0 < num_images);

  int const cols = AllImages->width;
  int const rows = AllImages->height;

  // Check input datatypes (if any).
  bool const compute_dynamic_range = (NULL != dynamic_range_in_out);
  if ( (true == compute_dynamic_range) && (NULL != *dynamic_range_in_out) )
    {
      // Dynamic range image must be 1 channel single precision image.
      assert( (NULL != (*dynamic_range_in_out)->data) &&
              (CV_MAT_DEPTH((*dynamic_range_in_out)->type()) == CV_32F) &&
              (CV_MAT_CN((*dynamic_range_in_out)->type()) == 1)
              );
      if ( (NULL == (*dynamic_range_in_out)->data) ||
           (CV_MAT_DEPTH((*dynamic_range_in_out)->type()) != CV_32F) ||
           (CV_MAT_CN((*dynamic_range_in_out)->type()) != 1)
           )
        {
          result = false;
          goto UpdateDynamicRangeAndTexture_EXIT;
        }
      /* if */
    }
  /* if */

  bool const compute_texture = (NULL != texture_in_out);
  bool const is_1_channel = ImageDataTypeIs1C_inline(AllImages->PixelFormat);
  if ( (true == compute_texture) && (NULL != *texture_in_out) )
    {
      // Texture image must be 1 channel or BGR single precision image.
      assert( (NULL != (*texture_in_out)->data) &&
              (CV_MAT_DEPTH((*texture_in_out)->type()) == CV_32F) &&
              ( (true == is_1_channel) ?
                (CV_MAT_CN((*texture_in_out)->type()) == 1) :
                (CV_MAT_CN((*texture_in_out)->type()) == 3)
                )
              );
      if ( (NULL == (*texture_in_out)->data) ||
           (CV_MAT_DEPTH((*texture_in_out)->type()) != CV_32F) ||
           ( (true == is_1_channel)?
             (CV_MAT_CN((*texture_in_out)->type()) != 1) :
             (CV_MAT_CN((*texture_in_out)->type()) != 3)
             )
           )
        {
          result = false;
          goto UpdateDynamicRangeAndTexture_EXIT;
        }
      /* if */
    }
  /* if */

  // Allocate memory on heap. Note that dynamic range image is used
  // as temporary buffer and must be always be allocated.

  // Allocate storage for dynamic range.
  dynamic_range = new cv::Mat(rows, cols, CV_32FC1);
  assert( (NULL != dynamic_range) && (NULL != dynamic_range->data) );

  if ( (NULL == dynamic_range) || (NULL == dynamic_range->data) )
    {
      result = false;
      goto UpdateDynamicRangeAndTexture_EXIT;
    }
  /* if */

  if (true == compute_dynamic_range)
    {
      // Allocate temporary storage for minimum and maximum.
      gray_max = new cv::Mat(rows, cols, CV_32FC1, cv::Scalar(BATCHACQUISITION_nINF_fv));
      assert( (NULL != gray_max) && (NULL != gray_max->data) );

      gray_min = new cv::Mat(rows, cols, CV_32FC1, cv::Scalar(BATCHACQUISITION_pINF_fv));
      assert( (NULL != gray_min) && (NULL != gray_min->data) );

      if ( (NULL == gray_max) || (NULL == gray_max->data) ||
           (NULL == gray_min) || (NULL == gray_min->data)
           )
        {
          result = false;
          goto UpdateDynamicRangeAndTexture_EXIT;
        }
      /* if */
    }
  /* if */

  if (true == compute_texture)
    {
      // Allocate storage for texture.
      texture = new cv::Mat(rows, cols, (true == is_1_channel)? CV_32FC1 : CV_32FC3, cv::Scalar(0.0f));
      assert( (NULL != texture) && (NULL != texture->data) );

      if ( (NULL == texture) || (NULL == texture->data) )
        {
          result = false;
          goto UpdateDynamicRangeAndTexture_EXIT;
        }
      /* if */

      if (false == is_1_channel)
        {
          texture_tmp = new cv::Mat(rows, cols, CV_32FC3);
          assert( (NULL != texture_tmp) && (NULL != texture_tmp->data) );
          
          if ( (NULL == texture_tmp) || (NULL == texture_tmp->data) )
            {
              result = false;
              goto UpdateDynamicRangeAndTexture_EXIT;
            }
          /* if */
        }
      /* if */
    }
  /* if */
  
  // First find the minimum and the maximum value for each pixel.
  for (int i = first; i <= last; ++i)
    {
      cv::Mat * img1C = AllImages->GetImage1C(i);
      assert(NULL != img1C);
      if (NULL == img1C) goto UpdateDynamicRangeAndTexture_EXIT;

      if (true == compute_dynamic_range)
        {
          // Re-use preallocated storage for conversion to single precision.
          img1C->convertTo(*dynamic_range, CV_32FC1);

          for (int y = 0; y < rows; ++y)
            {
              // Get row addresses.
              float const * const row_gray = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
              float       * const row_gray_max = (float *)( (BYTE *)(gray_max->data) + gray_max->step[0] * y );
              float       * const row_gray_min = (float *)( (BYTE *)(gray_min->data) + gray_min->step[0] * y );
              
              // Unrolled for loop with step 8.
              int x = 0;
              int const max_x = cols - 7;
              for (; x < max_x; x += 8)
                {
                  if (row_gray[x    ] > row_gray_max[x    ]) row_gray_max[x    ] = row_gray[x    ];
                  if (row_gray[x + 1] > row_gray_max[x + 1]) row_gray_max[x + 1] = row_gray[x + 1];
                  if (row_gray[x + 2] > row_gray_max[x + 2]) row_gray_max[x + 2] = row_gray[x + 2];
                  if (row_gray[x + 3] > row_gray_max[x + 3]) row_gray_max[x + 3] = row_gray[x + 3];
                  if (row_gray[x + 4] > row_gray_max[x + 4]) row_gray_max[x + 4] = row_gray[x + 4];
                  if (row_gray[x + 5] > row_gray_max[x + 5]) row_gray_max[x + 5] = row_gray[x + 5];
                  if (row_gray[x + 6] > row_gray_max[x + 6]) row_gray_max[x + 6] = row_gray[x + 6];
                  if (row_gray[x + 7] > row_gray_max[x + 7]) row_gray_max[x + 7] = row_gray[x + 7];

                  if (row_gray[x    ] < row_gray_min[x    ]) row_gray_min[x    ] = row_gray[x    ];
                  if (row_gray[x + 1] < row_gray_min[x + 1]) row_gray_min[x + 1] = row_gray[x + 1];
                  if (row_gray[x + 2] < row_gray_min[x + 2]) row_gray_min[x + 2] = row_gray[x + 2];
                  if (row_gray[x + 3] < row_gray_min[x + 3]) row_gray_min[x + 3] = row_gray[x + 3];
                  if (row_gray[x + 4] < row_gray_min[x + 4]) row_gray_min[x + 4] = row_gray[x + 4];
                  if (row_gray[x + 5] < row_gray_min[x + 5]) row_gray_min[x + 5] = row_gray[x + 5];
                  if (row_gray[x + 6] < row_gray_min[x + 6]) row_gray_min[x + 6] = row_gray[x + 6];
                  if (row_gray[x + 7] < row_gray_min[x + 7]) row_gray_min[x + 7] = row_gray[x + 7];
                }
              /* for */

              // Complete to end.
              for (; x < cols; ++x)
                {
                  if (row_gray[x] > row_gray_max[x]) row_gray_max[x] = row_gray[x];
                  if (row_gray[x] < row_gray_min[x]) row_gray_min[x] = row_gray[x];
                }
              /* for */
            }
          /* for */
        }
      /* if (true == compute_dynamic_range) */

      if (true == compute_texture)
        {
          if (true == is_1_channel)
            {
              // Convert to single precision.
              if (false == compute_dynamic_range) img1C->convertTo(*dynamic_range, CV_32FC1);

              for (int y = 0; y < rows; ++y)
                {
                  // Get row addresses.
                  float const * const row_gray = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
                  float       * const row_texture = (float *)( (BYTE *)(texture->data) + texture->step[0] * y );

                  // Unrolled for loop with step 8.
                  int x = 0;
                  int const max_x = cols - 7;
                  for (; x < max_x; x += 8)
                    {
                      row_texture[x    ] += row_gray[x    ];
                      row_texture[x + 1] += row_gray[x + 1];
                      row_texture[x + 2] += row_gray[x + 2];
                      row_texture[x + 3] += row_gray[x + 3];
                      row_texture[x + 4] += row_gray[x + 4];
                      row_texture[x + 5] += row_gray[x + 5];
                      row_texture[x + 6] += row_gray[x + 6];
                      row_texture[x + 7] += row_gray[x + 7];
                    }
                  /* for */

                  // Complete to end.
                  for (; x < cols; ++x)
                    {
                      row_texture[x] += row_gray[x];
                    }
                  /* for */
                }
              /* for */
            }
          else // !(true == is_1_channel)
            {
              cv::Mat * img3C = AllImages->GetImageBGR(i);
              assert(NULL != img3C);
              if (NULL == img3C)
                {
                  SAFE_DELETE(img1C);
                  goto UpdateDynamicRangeAndTexture_EXIT;
                }
              /* if */

              // Re-use preallocated storage.
              assert(NULL != texture_tmp);
              img3C->convertTo(*texture_tmp, CV_32FC3);

              int const end_x = 3 * cols;
              for (int y = 0; y < rows; ++y)
                {
                  // Get row addresses.
                  float const * const row_texture_tmp = (float *)( (BYTE *)(texture_tmp->data) + texture_tmp->step[0] * y );
                  float       * const row_texture = (float *)( (BYTE *)(texture->data) + texture->step[0] * y );

                  // Unrolled for loop with step 8.
                  int x = 0;
                  int const max_x = end_x - 7;
                  for (; x < max_x; x += 8)
                    {
                      row_texture[x    ] += row_texture_tmp[x    ];
                      row_texture[x + 1] += row_texture_tmp[x + 1];
                      row_texture[x + 2] += row_texture_tmp[x + 2];
                      row_texture[x + 3] += row_texture_tmp[x + 3];
                      row_texture[x + 4] += row_texture_tmp[x + 4];
                      row_texture[x + 5] += row_texture_tmp[x + 5];
                      row_texture[x + 6] += row_texture_tmp[x + 6];
                      row_texture[x + 7] += row_texture_tmp[x + 7];
                    }
                  /* for */

                  // Complete to end.
                  for (; x < end_x; ++x)
                    {
                      row_texture[x] += row_texture_tmp[x];
                    }
                  /* for */
                }
              /* for */

              SAFE_DELETE(img3C);
            }
          /* if (true == is_1_channel) */
        }
      /* if (true == compute_texture) */

      SAFE_DELETE(img1C);
    }
  /* for */

  // Compute dynamic range.
  if (true == compute_dynamic_range)
    {
      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          float       * const row_dynamic_range = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
          float const * const row_gray_max = (float *)( (BYTE *)(gray_max->data) + gray_max->step[0] * y );
          float const * const row_gray_min = (float *)( (BYTE *)(gray_min->data) + gray_min->step[0] * y );

          // Unrolled for loop with step 8.
          int x = 0;
          int const max_x = cols - 7;
          for (; x < max_x; x += 8)
            {
              row_dynamic_range[x    ] = row_gray_max[x    ] - row_gray_min[x    ];
              row_dynamic_range[x + 1] = row_gray_max[x + 1] - row_gray_min[x + 1];
              row_dynamic_range[x + 2] = row_gray_max[x + 2] - row_gray_min[x + 2];
              row_dynamic_range[x + 3] = row_gray_max[x + 3] - row_gray_min[x + 3];
              row_dynamic_range[x + 4] = row_gray_max[x + 4] - row_gray_min[x + 4];
              row_dynamic_range[x + 5] = row_gray_max[x + 5] - row_gray_min[x + 5];
              row_dynamic_range[x + 6] = row_gray_max[x + 6] - row_gray_min[x + 6];
              row_dynamic_range[x + 7] = row_gray_max[x + 7] - row_gray_min[x + 7];
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_dynamic_range[x] = row_gray_max[x] - row_gray_min[x];
            }
          /* for */
        }
      /* for */
    }
  /* if (true == compute_dynamic_range) */

  // Merge data with existing dynamic range if one exists.
  if ( (true == compute_dynamic_range) && (NULL != *dynamic_range_in_out) )
    {
      cv::Mat * const srcdst = *dynamic_range_in_out;
      for (int y = 0; y < rows; ++y)
        {
          float const * const row_dynamic_range = (float *)( (BYTE *)(dynamic_range->data) + dynamic_range->step[0] * y );
          float * const row_srcdst = (float *)( (BYTE *)(srcdst->data) + srcdst->step[0] * y );

          // Unrolled for loop with step 8
          int x = 0;
          int const max_x = cols - 7;
          for (; x < max_x; x += 8)
            {
              row_srcdst[x    ] = (row_srcdst[x    ] < row_dynamic_range[x    ])? row_srcdst[x    ] : row_dynamic_range[x    ];
              row_srcdst[x + 1] = (row_srcdst[x + 1] < row_dynamic_range[x + 1])? row_srcdst[x + 1] : row_dynamic_range[x + 1];
              row_srcdst[x + 2] = (row_srcdst[x + 2] < row_dynamic_range[x + 2])? row_srcdst[x + 2] : row_dynamic_range[x + 2];
              row_srcdst[x + 3] = (row_srcdst[x + 3] < row_dynamic_range[x + 3])? row_srcdst[x + 3] : row_dynamic_range[x + 3];
              row_srcdst[x + 4] = (row_srcdst[x + 4] < row_dynamic_range[x + 4])? row_srcdst[x + 4] : row_dynamic_range[x + 4];
              row_srcdst[x + 5] = (row_srcdst[x + 5] < row_dynamic_range[x + 5])? row_srcdst[x + 5] : row_dynamic_range[x + 5];
              row_srcdst[x + 6] = (row_srcdst[x + 6] < row_dynamic_range[x + 6])? row_srcdst[x + 6] : row_dynamic_range[x + 6];
              row_srcdst[x + 7] = (row_srcdst[x + 7] < row_dynamic_range[x + 7])? row_srcdst[x + 7] : row_dynamic_range[x + 7];
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_srcdst[x] = (row_srcdst[x] < row_dynamic_range[x])? row_srcdst[x] : row_dynamic_range[x];
            }
          /* for */
        }
      /* for */
    }
  else
    {
      SAFE_ASSIGN_PTR( dynamic_range, dynamic_range_in_out );
    }
  /* if */

  // Scale and output texture data.
  float const scl = (float)( 2.0 / (double)(num_images) );
  if ( (true == compute_texture) && (NULL != *texture_in_out) )
    {
      // If previous texture exists then add textures together.
      cv::Mat * const srcdst = *texture_in_out;
      int const end_x = (true == is_1_channel)? cols : 3 * cols;
      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          float const * const row_texture_src = (float *)( (BYTE *)(texture->data) + texture->step[0] * y );
          float * const row_srcdst = (float *)( (BYTE *)(srcdst->data) + srcdst->step[0] * y );

          // Unrolled for loop with step 8.
          int x = 0;
          int const max_x = end_x - 7;
          for (; x < max_x; x += 8)
            {
              row_srcdst[x    ] += scl * row_texture_src[x    ];
              row_srcdst[x + 1] += scl * row_texture_src[x + 1];
              row_srcdst[x + 2] += scl * row_texture_src[x + 2];
              row_srcdst[x + 3] += scl * row_texture_src[x + 3];
              row_srcdst[x + 4] += scl * row_texture_src[x + 4];
              row_srcdst[x + 5] += scl * row_texture_src[x + 5];
              row_srcdst[x + 6] += scl * row_texture_src[x + 6];
              row_srcdst[x + 7] += scl * row_texture_src[x + 7];
            }
          /* for */

          // Complete to end.
          for (; x < end_x; ++x)
            {
              row_srcdst[x] += scl * row_texture_src[x];
            }
          /* for */
        }
      /* for */
    }
  else
    {
      // If there is no previous texture scale the texture and assign it to output.
      if (true == compute_texture)
        {
          int const end_x = (true == is_1_channel)? cols : 3 * cols;
          for (int y = 0; y < rows; ++y)
            {
              // Get row addresses.
              float * const row_texture = (float *)( (BYTE *)(texture->data) + texture->step[0] * y );

              // Unrolled for loop with step 8.
              int x = 0;
              int const max_x = end_x - 7;
              for (; x < max_x; x += 8)
                {
                  row_texture[x    ] *= scl;
                  row_texture[x + 1] *= scl;
                  row_texture[x + 2] *= scl;
                  row_texture[x + 3] *= scl;
                  row_texture[x + 4] *= scl;
                  row_texture[x + 5] *= scl;
                  row_texture[x + 6] *= scl;
                  row_texture[x + 7] *= scl;
                }
              /* for */

              // Complete to end.
              for (; x < end_x; ++x)
                {
                  row_texture[x] *= scl;
                }
              /* for */
            }
          /* for */
        }
      /* if (true == compute_texture) */

      SAFE_ASSIGN_PTR( texture, texture_in_out );
    }
  /* if */


 UpdateDynamicRangeAndTexture_EXIT:

  SAFE_DELETE( dynamic_range );
  SAFE_DELETE( texture );
  SAFE_DELETE( texture_tmp );
  SAFE_DELETE( gray_max );
  SAFE_DELETE( gray_min );

  return result;
}
/* UpdateDynamicRangeAndTexture */



//! Fetch texture image.
/*!
  Function fetches indicated texture image from the set of all images and converts
  it to 8U image with automatic amplitude scaling.

  \param AllImages      Pointer to class containing all acquired images.
  \param texture_idx    Index of the texture image; indicated image should be acquired under all-white illumination.
  \return Function returns pointer to a valid cv::Mat or NULL pointer if unsuccessfull.
*/
cv::Mat *
FetchTexture(
             ImageSet * const AllImages,
             int const texture_idx
             )
{
  cv::Mat * texture = NULL;

  bool const inputs_valid = ValidateInputs_inline(AllImages, texture_idx, texture_idx);
  if (false == inputs_valid) return texture;

  // Pre-allocate texture header.
  texture = new cv::Mat();
  assert(NULL != texture);
  if (NULL == texture) return texture;

  // Get recorded image parameters.
  bool const is_grayscale = (true == AllImages->IsGrayscale());

  uint const nbits = MSBPositionInOpenCVFromImageDataType_inline(AllImages->PixelFormat);
  double const scale = 255.0 / ( pow( 2.0, double(nbits + 1) ) - 1.0 );

  // Copy and scale image data to output texture.
  if (true == is_grayscale)
    {
      cv::Mat * tmp = AllImages->GetImageGray(texture_idx);
      assert(NULL != tmp);
      if (NULL == tmp)
        {
          SAFE_DELETE( texture );
          return texture;
        }
      /* if */
      tmp->convertTo(*texture, CV_8UC1, scale);
      SAFE_DELETE( tmp );
    }
  else
    {
      cv::Mat * tmp = AllImages->GetImageBGR(texture_idx);
      assert(NULL != tmp);
      if (NULL == tmp)
        {
          SAFE_DELETE( texture );
          return texture;
        }
      /* if */
      tmp->convertTo(*texture, CV_8UC3, scale);
      SAFE_DELETE( tmp );
    }
  /* if */

  return texture;
}
/* FetchTexture */



//! Convert texture image to 8-bit grayscale or BGR.
/*!
  Function converts texture to 8U depth and to either grayscale or BGR representation.
  Texture is grayscale if input images are grayscale and it BGR if input images are in color (Bayer, YUV, BGR, etc.).

  \param texture_in     Pointer to texture image; this is the texture_in_out output of the UpdateDynamicRangeAndTexture function.
  \param PixelFormat    Pixel format of captures image. Note that texture_in always uses CV_32F datatype.
  \param N      Number of times UpdateDynamicRangeAndTexture function was called, i.e. this is the number of individual texture images added together.
  \return Returns valid pointer to cv::Mat which contains the texture or NULL if unsuccessfull.
*/
cv::Mat *
ScaleAndDeBayerTexture(
                       cv::Mat * const texture_in,
                       ImageDataType const PixelFormat,
                       int const N
                       )
{
  cv::Mat * texture = NULL;

  assert( (NULL != texture_in) && (NULL != texture_in->data) );
  if ( (NULL == texture_in) || (NULL == texture_in->data) ) return texture;

  assert(0 < N);

  // Pre-allocate texture header.
  texture = new cv::Mat();
  assert(NULL != texture);
  if (NULL == texture) return texture;

  // Get texture image parameters.
  bool const is_grayscale = ImageDataTypeIsGrayscale_inline(PixelFormat);
  bool const is_bayer = ImageDataTypeIsBayer_inline(PixelFormat);

  if (true == is_grayscale)
    {
      // Get scaling factor.
      uint const nbits = MSBPositionInOpenCVFromImageDataType_inline(PixelFormat);
      double const scale = 255.0 / ( (double)(N) * ( pow( 2.0, double(nbits + 1) ) - 1.0 ) );

      // Copy and scale data.
      texture_in->convertTo(*texture, CV_8UC1, scale);
    }
  else if (true == is_bayer)
    {
      // Get pixel format.
      int flags = 0;
      ImageDataType type = IDT_UNKNOWN;
      bool const get_flags = GetBestMatchingcvMatFlags(PixelFormat, &type, &flags);
      assert( (true == get_flags) && (1 == CV_MAT_CN(flags)) && (true == ImageDataTypeIsBayer_inline(type)) );

      // Convert from float to best matching pixel format.
      cv::Mat tmp_bayer;
      texture_in->convertTo(tmp_bayer, flags, 1.0/(double)(N));

      // Convert from Bayer to BGR.
      int const width = tmp_bayer.cols;
      int const height = tmp_bayer.rows;
      int const stride = (int)( tmp_bayer.step[0] );
      void const * const data = tmp_bayer.data;
      cv::Mat * tmp_bgr = RawBufferToBGRcvMat(type, width, height, stride, data);
      assert(NULL != tmp_bgr);
      if (NULL == tmp_bgr)
        {
          SAFE_DELETE( texture );
          return texture;
        }
      /* if */

      // Get scaling factor.
      double const nbits = GetImagePixelMSBPosition(tmp_bgr);
      double const scale = (false == isnan_inline(nbits))? 255.0 / (pow(2.0, nbits + 1.0) - 1.0) : 1.0;

      // Copy and scale data.
      tmp_bgr->convertTo(*texture, CV_8UC1, scale);

      SAFE_DELETE( tmp_bgr );
    }
  else
    {
      // Get and check pixel format.
      int flags = 0;
      ImageDataType type = IDT_UNKNOWN;
      bool const get_flags = GetBestMatchingcvMatFlags(PixelFormat, &type, &flags);
      assert( (true == get_flags) && (3 == CV_MAT_CN(flags)) );

      // Get scaling factor.
      double const nbits = GetImagePixelMSBPosition( CV_MAT_DEPTH(flags) );
      double const scale = (false == isnan_inline(nbits))?
        255.0 / ( (double)(N) * ( pow(2.0, nbits + 1.0) - 1.0 ) ) :
        1.0 / (double)(N);

      // Convert from float to best matching BGR pixel format.
      texture_in->convertTo(*texture, CV_8UC1, 1.0/(double)(N));
    }
  /* if */

  return texture;
}
/* ScaleAndDeBayerTexture */



#endif /* !__BATCHACQUISITIONPROCESSINGDYNAMICRANGE_CPP */
