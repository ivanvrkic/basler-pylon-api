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
  \file   BatchAcquisitionProcessingPhaseShift.cpp
  \brief  Phase estimation methods.

  This file contains functions for both wrapped and unwrapped phase estimation.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2017-06-13
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPROCESSINGPHASESHIFT_CPP
#define __BATCHACQUISITIONPROCESSINGPHASESHIFT_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionProcessingPhaseShift.h"
#include "BatchAcquisitionDebug.h"
#include "BatchAcquisitionProcessingDynamicRange.h"


#pragma intrinsic(cos)
#pragma intrinsic(sin)
#pragma intrinsic(atan2)
#pragma intrinsic(sqrt)



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



/****** RELATIVE PHASE ESTIMATION ******/


//! Relative phase estimation (single precision).
/*!
  Function computes relative phase using the selected image span.

  First selected image is assumed to have the phase zero.
  Last selected image is assumed to have the phase 2*pi*(last-first)/(last-first+1).
  All other images have the phase equidistantly spread between the first and the last phase.

  See function relphsmap.m for a basic implementation in Matlab.

  \param AllImages      Pointer to class containing all acquired images.
  \param first  Index of the first image.
  \param last   Index of the last image (inclusive).
  \return Function returns a pointer to valid cv::Mat or NULL if unsuccessfull.
*/
cv::Mat *
EstimateRelativePhaseSingle(
                            ImageSet * const AllImages,
                            int const first,
                            int const last
                            )
{
  cv::Mat * rel_phase = NULL; // Relative phase.
  cv::Mat * acc_num = NULL; // Accumulator.
  cv::Mat * acc_den = NULL; // Accumulator.

  float * phi = NULL;
  float * weight_num = NULL;
  float * weight_den = NULL;

  bool const inputs_valid = ValidateInputs_inline(AllImages, first, last);
  if (false == inputs_valid) return rel_phase;

  int const num_images = last - first + 1;
  assert(0 < num_images);

  // Compute weight factors for numerator and denominator.
  phi = new float[num_images];
  assert(NULL != phi);

  weight_num = new float[num_images];
  weight_den = new float[num_images];
  assert(NULL != weight_num);
  assert(NULL != weight_den);

  if ( (NULL == phi) || (NULL == weight_num) || (NULL == weight_den) ) goto EstimateRelativePhase_EXIT;

  float const pi = 3.141592653589793238462643383279502884197169399375f;
  float const k = 2.0f * pi / (float)( num_images );

  for (int i = 0; i < num_images; ++i)
    {
      phi[i] = k * (float)(i);
      weight_num[i] = cosf( phi[i] );
      weight_den[i] = -sinf( phi[i] );
    }
  /* for */

  // Allocate data storage for accumulators and end result.
  int const cols = AllImages->width;
  int const rows = AllImages->height;

  rel_phase = new cv::Mat(rows, cols, CV_32FC1, 0.0f);
  assert(NULL != rel_phase);

  acc_num = new cv::Mat(rows, cols, CV_32FC1, 0.0f);
  acc_den = new cv::Mat(rows, cols, CV_32FC1, 0.0f);
  assert(NULL != acc_num);
  assert(NULL != acc_den);

  if ( (NULL == rel_phase) || (NULL == acc_num) || (NULL == acc_den) ) goto EstimateRelativePhase_EXIT;

  // Accumulate numerator and denominator.
  for (int i = first; i <= last; ++i)
    {
      cv::Mat * img1C = AllImages->GetImage1C(i);
      assert(NULL != img1C);
      if (NULL == img1C) goto EstimateRelativePhase_EXIT;

      // Re-use preallocated storage.
      img1C->convertTo(*rel_phase, CV_32FC1);

      // Pre-fetch weights.
      float const k_num = weight_num[i-first];
      float const k_den = weight_den[i-first];

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          float const * const row_gray = (float *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
          float       * const row_acc_num = (float *)( (BYTE *)(acc_num->data) + acc_num->step[0] * y );
          float       * const row_acc_den = (float *)( (BYTE *)(acc_den->data) + acc_den->step[0] * y );

          // Unrolled for loop with step 4.
          int x = 0;
          int const max_x = cols - 3;
          for (; x < max_x; x += 4)
            {
              row_acc_num[x    ] += k_num * row_gray[x    ];
              row_acc_num[x + 1] += k_num * row_gray[x + 1];
              row_acc_num[x + 2] += k_num * row_gray[x + 2];
              row_acc_num[x + 3] += k_num * row_gray[x + 3];

              row_acc_den[x    ] += k_den * row_gray[x    ];
              row_acc_den[x + 1] += k_den * row_gray[x + 1];
              row_acc_den[x + 2] += k_den * row_gray[x + 2];
              row_acc_den[x + 3] += k_den * row_gray[x + 3];
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_acc_num[x] += k_num * row_gray[x];
              row_acc_den[x] += k_den * row_gray[x];
            }
          /* for */
        }
      /* for */

      SAFE_DELETE(img1C);
    }
  /* for */

  // Compute relative phase.
  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      float       * const row_rel_phase = (float *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
      float const * const row_acc_num = (float *)( (BYTE *)(acc_num->data) + acc_num->step[0] * y );
      float const * const row_acc_den = (float *)( (BYTE *)(acc_den->data) + acc_den->step[0] * y );

      // Unrolled for loop with step 4.
      int x = 0;
      int const max_x = cols - 3;
      for (; x < max_x; x += 4)
        {
          row_rel_phase[x    ] = (float)atan2(row_acc_num[x    ], row_acc_den[x    ]) + pi;
          row_rel_phase[x + 1] = (float)atan2(row_acc_num[x + 1], row_acc_den[x + 1]) + pi;
          row_rel_phase[x + 2] = (float)atan2(row_acc_num[x + 2], row_acc_den[x + 2]) + pi;
          row_rel_phase[x + 3] = (float)atan2(row_acc_num[x + 3], row_acc_den[x + 3]) + pi;
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_rel_phase[x] = (float)atan2(row_acc_num[x], row_acc_den[x]) + pi;
        }
      /* for */
    }
  /* for */


 EstimateRelativePhase_EXIT:

  SAFE_DELETE( acc_num );
  SAFE_DELETE( acc_den );

  SAFE_DELETE_ARRAY( weight_num );
  SAFE_DELETE_ARRAY( weight_den );

  SAFE_DELETE_ARRAY( phi );

  return rel_phase;
}
/* EstimateRelativePhaseSingle */



//! Relative phase estimation (double precision).
/*!
  Function computes relative phase using the selected image span.

  First selected image is assumed to have the phase zero.
  Last selected image is assumed to have the phase 2*pi*(last-first)/(last-first+1).
  All other images have the phase equidistantly spread between the first and the last phase.
  Function assumes images are consecutively stored in AllImages starting
  from index first and ending with index last (inclusive).

  See function relphsmap.m for a basic implementation in Matlab.

  \param AllImages      Pointer to class containing all acquired images.
  \param first  Index of the first image.
  \param last   Index of the last image (inclusive).
  \return Function returns a pointer to valid cv::Mat or NULL if unsuccessfull.
*/
cv::Mat *
EstimateRelativePhase(
                      ImageSet * const AllImages,
                      int const first,
                      int const last
                      )
{
  cv::Mat * rel_phase = NULL; // Relative phase.
  cv::Mat * acc_num = NULL; // Accumulator.
  cv::Mat * acc_den = NULL; // Accumulator.

  double * phi = NULL;
  double * weight_num = NULL;
  double * weight_den = NULL;

  bool const inputs_valid = ValidateInputs_inline(AllImages, first, last);
  if (false == inputs_valid) return rel_phase;

  int const num_images = last - first + 1;
  assert(0 < num_images);

  // Compute weight factors for numerator and denominator.
  phi = new double[num_images];
  assert(NULL != phi);

  weight_num = new double[num_images];
  weight_den = new double[num_images];
  assert(NULL != weight_num);
  assert(NULL != weight_den);

  if ( (NULL == phi) || (NULL == weight_num) || (NULL == weight_den) ) goto EstimateRelativePhase_EXIT;

  double const pi = 3.141592653589793238462643383279502884197169399375;
  double const k = 2.0 * pi / (double)( num_images );

  for (int i = 0; i < num_images; ++i)
    {
      phi[i] = k * (double)(i);
      weight_num[i] = cos( phi[i] );
      weight_den[i] = -sin( phi[i] );
    }
  /* for */

  // Allocate data storage for accumulators and end result.
  int const cols = AllImages->width;
  int const rows = AllImages->height;

  rel_phase = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != rel_phase);

  acc_num = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  acc_den = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != acc_num);
  assert(NULL != acc_den);

  if ( (NULL == rel_phase) || (NULL == acc_num) || (NULL == acc_den) ) goto EstimateRelativePhase_EXIT;

  // Accumulate numerator and denominator.
  for (int i = first; i <= last; ++i)
    {
      cv::Mat * img1C = AllImages->GetImage1C(i);
      assert(NULL != img1C);
      if (NULL == img1C) goto EstimateRelativePhase_EXIT;

      // Re-use preallocated storage.
      img1C->convertTo(*rel_phase, CV_64FC1);

      // Pre-fetch weights.
      double const k_num = weight_num[i-first];
      double const k_den = weight_den[i-first];

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          double const * const row_gray = (double *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
          double       * const row_acc_num = (double *)( (BYTE *)(acc_num->data) + acc_num->step[0] * y );
          double       * const row_acc_den = (double *)( (BYTE *)(acc_den->data) + acc_den->step[0] * y );

          // Unrolled for loop with step 4.
          int x = 0;
          int const max_x = cols - 3;
          for (; x < max_x; x += 4)
            {
              row_acc_num[x    ] += k_num * row_gray[x    ];
              row_acc_num[x + 1] += k_num * row_gray[x + 1];
              row_acc_num[x + 2] += k_num * row_gray[x + 2];
              row_acc_num[x + 3] += k_num * row_gray[x + 3];

              row_acc_den[x    ] += k_den * row_gray[x    ];
              row_acc_den[x + 1] += k_den * row_gray[x + 1];
              row_acc_den[x + 2] += k_den * row_gray[x + 2];
              row_acc_den[x + 3] += k_den * row_gray[x + 3];
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_acc_num[x] += k_num * row_gray[x];
              row_acc_den[x] += k_den * row_gray[x];
            }
          /* for */
        }
      /* for */

      SAFE_DELETE(img1C);
    }
  /* for */

  // Compute relative phase.
  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      double       * const row_rel_phase = (double *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
      double const * const row_acc_num = (double *)( (BYTE *)(acc_num->data) + acc_num->step[0] * y );
      double const * const row_acc_den = (double *)( (BYTE *)(acc_den->data) + acc_den->step[0] * y );

      // Unrolled for loop with step 4.
      int x = 0;
      int const max_x = cols - 3;
      for (; x < max_x; x += 4)
        {
          row_rel_phase[x    ] = atan2(row_acc_num[x    ], row_acc_den[x    ]) + pi;
          row_rel_phase[x + 1] = atan2(row_acc_num[x + 1], row_acc_den[x + 1]) + pi;
          row_rel_phase[x + 2] = atan2(row_acc_num[x + 2], row_acc_den[x + 2]) + pi;
          row_rel_phase[x + 3] = atan2(row_acc_num[x + 3], row_acc_den[x + 3]) + pi;
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_rel_phase[x] = atan2(row_acc_num[x], row_acc_den[x]) + pi;
        }
      /* for */
    }
  /* for */


 EstimateRelativePhase_EXIT:

  SAFE_DELETE( acc_num );
  SAFE_DELETE( acc_den );

  SAFE_DELETE_ARRAY( weight_num );
  SAFE_DELETE_ARRAY( weight_den );

  SAFE_DELETE_ARRAY( phi );

  return rel_phase;
}
/* EstimateRelativePhase */



/****** GRAY CODE DECODING ******/


//! Computes Gray code weights (double precision).
/*!
  Function computes weights required to decode Gray code into projector column or row index.

  \param N      Number of bits reserved for the Gray code.
  \return Function returns a pointer to an array of doubles that has 2^N elements or NULL if unsuccessfull.
*/
double *
CreateGrayCodeWeights(
                      int const N
                      )
{
  assert(0 < N);
  if (0 >= N) return NULL;

  /* Pre-allocate storage. */
  int * const idx = (int *)malloc( sizeof(int) * N );
  assert(NULL != idx);

  int const total = 1 << N;

  int * const code = (int *)malloc( sizeof(int) * total );
  assert(NULL != code);

  double * const weight = (double *)malloc( sizeof(double) * total );
  assert(NULL != weight);

  if ( (NULL == idx) || (NULL == code) || (NULL == weight) )
    {
      if (NULL != idx) free(idx);
      if (NULL != code) free(code);
      if (NULL != weight) free(weight);
      return NULL;
    }
  /* if */

  /* Compute code. */
  for (int i = 0; i < N; ++i) idx[i] = 1 << i;

  code[0] = 0;
  for (int i = 0; i < N; ++i)
    {
      for (int j = idx[i]; j < 2 * idx[i]; ++j)
        {
          assert( (0 <= j) && (j < total) );
          int const k = j - 2 * (j - idx[i] + 1) + 1;
          assert( (0 <= k) && (k < total) );
          code[j] = code[k] + idx[i];
        }
      /* for */
    }
  /* for */

  double const total_inv = 1.0 / (double)(total);
  for (int i = 0; i < total; ++i)
    {
      int const j = code[i];
      assert( (0 <= j) && (j < total) );
      weight[j] = double(i) * total_inv;
    }
  /* for */

  /* Deallocate temporary storage. */
  free(idx);
  free(code);

  return weight;
}
/* CreateGrayCodeWeights */



//! Deletes Gray code weights (double precision).
/*!
  Function deletes Gray code weights.

  \param weight Pointer to Gray code weights.
*/
void
DeleteGrayCodeWeights(
                      double * const weight
                      )
{
  if (NULL != weight) free( weight );
}
/* DeleteGrayCodeWeights */



//! Decodes Gray code (double precision).
/*!
  Decodes Gray code and returns normalized projector coordinate.
  Function assumes images are consecutively stored in AllImages starting
  from index first and ending with index last (inclusive).

  \param AllImages      Pointer to class containing all acquired images.
  \param threshold      Pointer to threshold image. Must be of CV_64FC1 type.
  \param first  Index of the first Gray code image.
  \param last   Index of the last Gray code image.
  \return Function returns a pointer to valid cv::Mat or NULL if unsuccesffull.
*/
cv::Mat *
DecodeGrayCode(
               ImageSet * const AllImages,
               cv::Mat * threshold,
               int const first,
               int const last
               )
{
  cv::Mat * code = NULL; // Decoded Gray code.

  bool const inputs_valid = ValidateInputs_inline(AllImages, first, last);
  if (false == inputs_valid) return code;

  assert(NULL != threshold);
  if (NULL == threshold) return code;

  int const num_images = last - first + 1;
  assert(0 < num_images);

  // Fetch input image size.
  int const cols = AllImages->width;
  int const rows = AllImages->height;

  // Check size and type of the threshold image.
  assert( cols <= threshold->cols );
  assert( rows <= threshold->rows );
  if ( (cols > threshold->cols) || (rows > threshold->rows) ) return code;

  assert( (CV_MAT_DEPTH(threshold->type()) == CV_64F) && (CV_MAT_CN(threshold->type()) == 1) );
  if ( (CV_MAT_DEPTH(threshold->type()) != CV_64F) || (CV_MAT_CN(threshold->type()) != 1) ) return code;

  // Allocate data storage for temporray buffers and end result.
  code = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != code);

  cv::Mat * tmp_buffer = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != tmp_buffer);

  double * const weight = CreateGrayCodeWeights(num_images);
  assert(NULL != weight);

  int const total = 1 << num_images;

  if ( (NULL == code) || (NULL == tmp_buffer) || (NULL == weight) ) goto DecodeGrayCode_EXIT;

  // Decode Gray code.
  for (int i = first; i <= last; ++i)
    {
      cv::Mat * img1C = AllImages->GetImage1C(i);
      assert(NULL != img1C);
      if (NULL == img1C) goto DecodeGrayCode_EXIT;

      // Re-use preallocated storage.
      img1C->convertTo(*tmp_buffer, CV_64FC1);

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          double const * const row_threshold = (double *)( (BYTE *)(threshold->data) + threshold->step[0] * y );
          double const * const row_gray = (double *)( (BYTE *)(tmp_buffer->data) + tmp_buffer->step[0] * y );
          double       * const row_code = (double *)( (BYTE *)(code->data) + code->step[0] * y );

          // Unrolled for loop with step 4.
          int x = 0;
          int const max_x = cols - 3;
          for (; x < max_x; x += 4)
            {
              row_code[x    ] *= 2.0;
              row_code[x + 1] *= 2.0;
              row_code[x + 2] *= 2.0;
              row_code[x + 3] *= 2.0;

              if (row_gray[x    ] > row_threshold[x    ]) row_code[x    ] += 1.0;
              if (row_gray[x + 1] > row_threshold[x + 1]) row_code[x + 1] += 1.0;
              if (row_gray[x + 2] > row_threshold[x + 2]) row_code[x + 2] += 1.0;
              if (row_gray[x + 3] > row_threshold[x + 3]) row_code[x + 3] += 1.0;
            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_code[x] *= 2.0;
              if (row_gray[x] > row_threshold[x]) row_code[x] += 1.0;
            }
          /* for */
        }
      /* for */

      SAFE_DELETE(img1C);
    }
  /* for */

  // Normalize code.
  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      double * const row_code = (double *)( (BYTE *)(code->data) + code->step[0] * y );

      for (int x = 0; x < cols; ++x)
        {
          assert( (0 <= row_code[x]) && (row_code[x] < total) );
          row_code[x] = weight[ (int)(row_code[x]) ];
        }
      /* for */
    }
  /* for */

 DecodeGrayCode_EXIT:

  SAFE_DELETE(tmp_buffer);
  DeleteGrayCodeWeights(weight);

  return code;
}
/* DecodeGrayCode */



/****** ABSOLUTE PHASE ESTIMATION USING GC+PS ******/


//! Unwraps phase using Gray code.
/*!
  Function unwraps phase using Gray code.

  \param AllImages
  \param n1     First image of the normal Gray code set.
  \param n2     Last image of the normal Gray code set.
  \param m1     First image of the shifted Gray code set.
  \param m2     Last iamge of the shifted Gray code set.
  \param b  Index of the black image.
  \param w  Index of the white image.
  \param rel_phase      Pointer to wrapped phase image. Must be of type.
  \param gray_code_1_out    Pointer to address where decoded normal Gray code will be stored. May be NULL.
  \param gray_code_2_out    Pointer to address where decoded second Gray code will be stored. May be NULL.
  \return Function returns pointer to unwrapped phase image or NULL if unsuccessfull.
*/
cv::Mat *
UnwrapPhasePSAndGC(
                   ImageSet * const AllImages,
                   int const n1,
                   int const n2,
                   int const m1,
                   int const m2,
                   int const b,
                   int const w,
                   cv::Mat * const rel_phase,
                   cv::Mat * * const gray_code_1_out,
                   cv::Mat * * const gray_code_2_out
                   )
{
  cv::Mat * abs_phase = NULL; // Unwrapped (or absolute) phase.

  // Check inputs.
  assert( (0 <= b) && (b < AllImages->num_images) );
  if ( (b < 0) || (AllImages->num_images <= b) ) return abs_phase;

  assert( (0 <= w) && (w < AllImages->num_images) );
  if ( (w < 0) || (AllImages->num_images <= w) ) return abs_phase;

  assert( NULL != rel_phase );
  if (NULL == rel_phase) return abs_phase;

  assert( NULL != rel_phase->data );
  if (NULL == rel_phase->data) return abs_phase;

  // Fetch input image size.
  int const cols = AllImages->width;
  int const rows = AllImages->height;

  assert( cols <= rel_phase->cols );
  assert( rows <= rel_phase->rows );
  if ( (cols > rel_phase->cols) || (rows > rel_phase->rows) ) return abs_phase;

  assert( (CV_MAT_DEPTH(rel_phase->type()) == CV_64F) && (CV_MAT_CN(rel_phase->type()) == 1) );
  if ( (CV_MAT_DEPTH(rel_phase->type()) != CV_64F) || (CV_MAT_CN(rel_phase->type()) != 1) ) return abs_phase;

  // Preallocate storage.
  cv::Mat * black = AllImages->GetImage1C(b);
  assert(NULL != black);

  cv::Mat * white = AllImages->GetImage1C(w);
  assert(NULL != white);

  cv::Mat * gray_code_1 = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != gray_code_1);

  cv::Mat * gray_code_2 = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != gray_code_2);

  cv::Mat * threshold = new cv::Mat(rows, cols, CV_64FC1, 0.0f);
  assert(NULL != threshold);

  if ( (NULL == black) ||
       (NULL == white) ||
       (NULL == gray_code_1) ||
       (NULL == gray_code_2) ||
       (NULL == threshold)
       )
    {
      goto UnwrapPhasePSAndGC_EXIT;
    }
  /* if */

  // Compute threshold; we use gray_code_? as temporary buffers.
  black->convertTo(*gray_code_1, CV_64FC1);
  white->convertTo(*gray_code_2, CV_64FC1);

  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      double const * const row_gray_code_1 = (double *)( (BYTE *)(gray_code_1->data) + gray_code_1->step[0] * y );
      double const * const row_gray_code_2 = (double *)( (BYTE *)(gray_code_2->data) + gray_code_2->step[0] * y );
      double       * const row_threshold = (double *)( (BYTE *)(threshold->data) + threshold->step[0] * y );

      // Unrolled for loop with step 8.
      int x = 0;
      int const max_x = cols - 7;
      for (; x < max_x; x += 8)
        {
          row_threshold[x    ] = 0.5 * ( row_gray_code_1[x    ] + row_gray_code_2[x    ] );
          row_threshold[x + 1] = 0.5 * ( row_gray_code_1[x + 1] + row_gray_code_2[x + 1] );
          row_threshold[x + 2] = 0.5 * ( row_gray_code_1[x + 2] + row_gray_code_2[x + 2] );
          row_threshold[x + 3] = 0.5 * ( row_gray_code_1[x + 3] + row_gray_code_2[x + 3] );

          row_threshold[x + 4] = 0.5 * ( row_gray_code_1[x + 4] + row_gray_code_2[x + 4] );
          row_threshold[x + 5] = 0.5 * ( row_gray_code_1[x + 5] + row_gray_code_2[x + 5] );
          row_threshold[x + 6] = 0.5 * ( row_gray_code_1[x + 6] + row_gray_code_2[x + 6] );
          row_threshold[x + 7] = 0.5 * ( row_gray_code_1[x + 7] + row_gray_code_2[x + 7] );
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_threshold[x] = 0.5 * ( row_gray_code_1[x] + row_gray_code_2[x] );
        }
      /* for */
    }
  /* for */

  SAFE_DELETE( gray_code_1 );
  SAFE_DELETE( gray_code_2 );

  // Decode normal Gray code.
  gray_code_1 = DecodeGrayCode(AllImages, threshold, n1, n2);
  assert(NULL != gray_code_1);
  if (NULL == gray_code_1) goto UnwrapPhasePSAndGC_EXIT;

  // Decode shifted Gray code (may not be present).
  gray_code_2 = DecodeGrayCode(AllImages, threshold, m1, m2);

  // Reuse preallocated space for the output buffer.
  abs_phase = threshold;
  threshold = NULL;

  // Define pi.
  double const pi = 3.141592653589793238462643383279502884197169399375;

  // Decode wrapped phase.
  if (NULL == gray_code_2)
    {
      // Decode using only the normal Gray code set. This type of decoding
      // may produce errors on code boundaries.
      double const total = (double)( 1 << (n2 - n1 + 1) );
      double const c = 0.5 / (total * pi);

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          double const * const row_gray_code_1 = (double *)( (BYTE *)(gray_code_1->data) + gray_code_1->step[0] * y );
          double const * const row_rel_phase = (double *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
          double       * const row_abs_phase = (double *)( (BYTE *)(abs_phase->data) + abs_phase->step[0] * y );

          // Unrolled for loop with step 8.
          int x = 0;
          int const max_x = cols - 7;
          for (; x < max_x; x += 8)
            {
              row_abs_phase[x    ] = row_gray_code_1[x    ] + c * row_rel_phase[x    ];
              row_abs_phase[x + 1] = row_gray_code_1[x + 1] + c * row_rel_phase[x + 1];
              row_abs_phase[x + 2] = row_gray_code_1[x + 2] + c * row_rel_phase[x + 2];
              row_abs_phase[x + 3] = row_gray_code_1[x + 3] + c * row_rel_phase[x + 3];

              row_abs_phase[x + 4] = row_gray_code_1[x + 4] + c * row_rel_phase[x + 4];
              row_abs_phase[x + 5] = row_gray_code_1[x + 5] + c * row_rel_phase[x + 5];
              row_abs_phase[x + 6] = row_gray_code_1[x + 6] + c * row_rel_phase[x + 6];
              row_abs_phase[x + 7] = row_gray_code_1[x + 7] + c * row_rel_phase[x + 7];

            }
          /* for */

          // Complete to end.
          for (; x < cols; ++x)
            {
              row_abs_phase[x] = row_gray_code_1[x] + c * row_rel_phase[x];
            }
          /* for */

        }
      /* for */
    }
  else
    {
      double const total1 = (double)( 1 << (n2 - n1 + 1) );
      double const total2 = (double)( 1 << (m2 - m1 + 1) );
      assert(total1 == total2);

      double const c = 0.5 / pi;
      double const c1 = 1.0 / total1;
      double const c2 = 1.0 / total2;

      for (int y = 0; y < rows; ++y)
        {
          // Get row addresses.
          double const * const row_gray_code_1 = (double *)( (BYTE *)(gray_code_1->data) + gray_code_1->step[0] * y );
          double const * const row_gray_code_2 = (double *)( (BYTE *)(gray_code_2->data) + gray_code_2->step[0] * y );
          double const * const row_rel_phase = (double *)( (BYTE *)(rel_phase->data) + rel_phase->step[0] * y );
          double       * const row_abs_phase = (double *)( (BYTE *)(abs_phase->data) + abs_phase->step[0] * y );

          // Unwrap phase.
          int x = 0;
          for (; x < cols; ++x)
            {
              double const WP_norm = c * row_rel_phase[x];

              if ( (0.25 <= WP_norm) && (WP_norm < 0.75) )
                {
                  row_abs_phase[x] = row_gray_code_1[x] + c1 * WP_norm;
                }
              else
                {
                  if ( 0 != row_gray_code_1[x] )
                    {
                      double const WP_norm_shifted = WP_norm + ( (WP_norm < 0.5)? 0.5 : -0.5 );
                      row_abs_phase[x] = (row_gray_code_2[x] + c2 * 0.5) + c2 * WP_norm_shifted;
                    }
                  else
                    {
                      row_abs_phase[x] = c1 * WP_norm;
                    }
                  /* if */
                }
              /* if */
            }
          /* for */
        }
      /* for */

    }
  /* if */


  SAFE_ASSIGN_PTR( gray_code_1, gray_code_1_out );
  SAFE_ASSIGN_PTR( gray_code_2, gray_code_2_out );

 UnwrapPhasePSAndGC_EXIT:

  SAFE_DELETE( black );
  SAFE_DELETE( white );
  SAFE_DELETE( threshold );
  SAFE_DELETE( gray_code_1 );
  SAFE_DELETE( gray_code_2 );

  return abs_phase;
}
/* UnwrapPSAndGC */




/****** ABSOLUTE PHASE ESTIMATION USING MPS ******/

//! Test in all numbers are whole numbers.
/*!
  Function tests is all numbers in std::vector are whole.

  \param numbers_in     Reference to std::vector of double precision numbers.
  \return Function returns true if all elements in std::vector are whole numbers and false otherwise.
  If the vector is empty then true is returned.
*/
inline
bool
mps_all_whole_inline(
                     std::vector<double> const & numbers_in
                     )
{
  bool all_whole = true;

  int const i_max = (int)( numbers_in.size() );
  for (int i = 0; i < i_max; ++i)
    {
      __int64 const x = (__int64)( numbers_in[i] + 0.5 );
      bool const x_is_whole = ( (double)x == numbers_in[i] );
      all_whole = all_whole && x_is_whole;
    }
  /* for */

  return all_whole;
}
/* mps_all_whole_inline */



//! Compute greatest common divisor of two numbers.
/*!
  Function computes greatest common divisor of two numbers
  using Euclidean algorithm.

  \param a      First number. Input must be whole number.
  \param b      Second number. Input must be whole number different than zero.
  \return Function returns greatest common divisor if successfull or NaN if unsuccessfull.
*/
double
mps_gcd(
        double const a,
        double const b
        )
{
  // Test if input numbers are whole numbers.
  __int64 const A = (__int64)( a + 0.5 );
  __int64 const B = (__int64)( b + 0.5 );
  bool const a_is_whole = ( (double)A == a );
  bool const b_is_whole = ( (double)B == b );
  assert( (true == a_is_whole) && (true == b_is_whole) );
  if ( (false == a_is_whole) || (false == b_is_whole) )
    {
      Debugfwprintf(stderr, gDbgGCDInputsAreNotWholeNumbers);
    }
  /* if */

  // Test if input numbers have all digits.
  double const limit = pow(2.0, 53);
  assert( (a < limit) && (b < limit) );
  if ( (a >= limit) || (b >= limit) )
    {
      Debugfwprintf(stderr, gDbgGCDInputsHaveOverflow);
    }
  /* if */

  // Assume failure.
  double gcd = std::numeric_limits<double>::quiet_NaN();

  // The second input must be non-zero to avoid an infinite loop.
  // Note that the first input may be zero in which case the while loop terminates after first iteration.
  if (0 != B)
    {
      __int64 r0 = A;
      __int64 r1 = B;
      __int64 r = 1;

      while (0 != r)
        {
          r = r0 % r1;
          r0 = r1;
          r1 = r;
        }
      /* while */

      gcd = (double)( r0 );

      __int64 const GCD = (__int64)(gcd + 0.5);
      assert( (double)GCD == gcd );
    }
  /* if */

  return gcd;
}
/* mps_gcd */



//! Compute greatest common divisor of many numbers.
/*!
  Function computes greatest common divisor of arbitrary many numbers in
  sequential manner using associativity property of the greatest common divisor.

  \param vec    Reference to a vector of numbers.
  \return Function returns greatest common divisor if successfull or NaN if unsuccessfull.
*/
double
mps_gcd(
        std::vector<double> const & vec
        )
{
  // Assume failure.
  double gcd = std::numeric_limits<double>::quiet_NaN();

  if ( (false == vec.empty()) && (true == mps_all_whole_inline(vec)) )
    {
      gcd = vec.front();
      for (size_t i = 1; i < vec.size(); ++i)
        {
          gcd = mps_gcd(gcd, vec.at(i));
        }
      /* for */
    }
  /* if */

  return gcd;
}
/* mps_gcd */



//! Compute least common multiple of two numbers.
/*!
  Function computes least common multiple of two numbers
  using mps_gcd function.

  \param a      First number. Input must be whole number.
  \param b      Second number. Input must be whole number different than zero.
  \return Function returns least common multiple if successfull or NaN if unsuccessfull.
*/
double
mps_lcm(
        double const a,
        double const b
        )
{
  // Test if input numbers are whole numbers.
  __int64 const A = (__int64)( a + 0.5 );
  __int64 const B = (__int64)( b + 0.5 );
  bool const a_is_whole = ( (double)A == a );
  bool const b_is_whole = ( (double)B == b );
  assert( (true == a_is_whole) && (true == b_is_whole) );
  if ( (false == a_is_whole) || (false == b_is_whole) )
    {
      Debugfwprintf(stderr, gDbgLCMInputsAreNotWholeNumbers);
    }
  /* if */

  // Test if input numbers have all digits and if product overflows.
  double const limit = pow(2.0, 53);
  assert( (a < limit) && (b < limit) && (a * b <= limit) );
  if ( (a >= limit) || (b >= limit) || (a * b > limit) )
    {
      Debugfwprintf(stderr, gDbgLCMInputsHaveOverflow);
    }
  /* if */

  double const gcd = mps_gcd(a, b);
  double const lcm = fabs(a * b) / gcd;

  return lcm;
}
/* mps_lcm */



//! Compute least common multiple of many numbers.
/*!
  Function computes least common multiple of arbitrary many numbers
  in sequentiall manner using associativity property of the least common multiple.

  \param vec    Reference to a vector of numbers.
  \return Function returns least common multiple is successfull or NaN if unsuccessfull.
*/
double
mps_lcm(
        std::vector<double> const & vec
        )
{
  // Assume failure.
  double lcm = std::numeric_limits<double>::quiet_NaN();

  if ( (false == vec.empty()) && (true == mps_all_whole_inline(vec)) )
    {
      lcm = vec.front();
      for (size_t i = 1; i < vec.size(); ++i)
        {
          lcm = mps_lcm(lcm, vec.at(i));
        }
      /* for */
    }
  /* if */

  return lcm;
}
/* mps_lcm */



//! Return periods from fringe counts.
/*!
  Function returns a vector of period (wavelengts) which corresponds to given fringe counts.

  \param counts_in      Reference to a vector of fringe counts.
  \param width_in       Maximal unwrapped phase width; may be set to NaN in which case it is automatically computed from fringe counts.
  \param lambda_out     Address where the pointer to a vector of periods will be stored.
  \param width_out      Address where used maximal width will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
mps_periods_from_fringe_counts(
                               std::vector<double> const & counts_in,
                               double const width_in,
                               std::vector<double> * * const lambda_out,
                               double * const width_out
                               )
{
  assert( true == mps_all_whole_inline(counts_in) );

  // Set default width.
  double width = width_in;
  double const lcm_counts = mps_lcm(counts_in);
  double const gcd_counts = mps_gcd(counts_in);
  if ( 0 != _isnan(width_in) )
    {
      width = lcm_counts / gcd_counts;
      assert( 0 == _isnan(width) );
    }
  /* if */
  assert( (0 <= width) && (width <= lcm_counts) );

  bool result = false; // Assume failure.

  std::vector<double> * lambda = new std::vector<double>( counts_in.size() );
  assert(NULL != lambda);
  if (NULL != lambda)
    {      
      int const i_max = (int)( counts_in.size() );
      if (1.0 < gcd_counts)
        {
          Debugfwprintf(stderr, gDbgFringeCountsAreNotWholeNumbers);
          for (int i = 0; i < i_max; ++i) (*lambda)[i] = ( width * gcd_counts ) / counts_in[i];
        }
      else
        {
          assert(1.0 == gcd_counts);
          for (int i = 0; i < i_max; ++i) (*lambda)[i] = width / counts_in[i];
        }
      /* if */
      
      double const gcd_lambda = mps_gcd(*lambda);
      if (1.0 < gcd_lambda)
        {
          Debugfwprintf(stderr, gDbgPeriodsAreNotRelativelyPrime);
          double const scl = 1.0 / gcd_lambda;
          for (int i = 0; i < i_max; ++i) (*lambda)[i] = (*lambda)[i] * scl;
        }
      else
        {
          assert(1.0 == gcd_lambda);
        }
      /* if */

      result = true;
    }
  /* if */

  SAFE_ASSIGN_PTR( lambda, lambda_out );
  if (NULL != width_out) *width_out = width;

  SAFE_DELETE( lambda );

  return result;
}
/* mps_periods_from_fringe_counts */



//! Computes all valid period tuples.
/*!
  Function returns all valid period tuples k for input periods lambda.

  \param lambda_in      Pointer to wavelengths.
  \param width_in       Maximal unwrapped phase width; may be set to NaN in which case it is automaticall computed from wavelengths lambda.
  \param k_out          Address where pointer to cv::Mat which stores all period tuples will be placed. May be NULL.
  \param width_out      Address where used maximal width will be stored.
  \param w_min_out      Address where pointer to cv::Mat which stores starting boundaries of each segment will be placed. May be NULL.
  \param w_max_out      Address where pointer to cv::Mat which stores ending boundaries of each segment will be placed. May be NULL.
  \return Function returns true if successfull, false otherwise.
*/
bool
mps_get_period_tuples(
                      std::vector<double> const & lambda_in,
                      double const width_in,
                      cv::Mat * * const k_out,
                      double * const width_out,
                      cv::Mat * * const w_min_out,
                      cv::Mat * * const w_max_out
                      )
{
  // Set default width.
  double width = width_in;
  if ( 0 != _isnan(width_in) )
    {
      double const lcm = mps_lcm(lambda_in);
      double const gcd = mps_gcd(lambda_in);
      width = lcm / gcd;
      assert( 0 == _isnan(width) );
    }
  /* if */

  if ( 0 != _isnan(width) ) return false;

  bool result = true; // Assume success.

  // Allocate storage on stack.
  int const i_max = (int)( lambda_in.size() );

  int j = 1;
  int const j_step = 100;

  std::vector<double> next_boundary(lambda_in);
  std::vector<int> inc(lambda_in.size(), 0);

  // Find first minimal boundary.
  double x = *std::min_element(next_boundary.begin(), next_boundary.end());
  for (int i = 0; i < i_max; ++i) inc[i] = (next_boundary[i] <= x)? 1 : 0;

  // Allocate storage on heap.
  cv::Mat * k = new cv::Mat(j_step, i_max, CV_32SC1, cv::Scalar(0));
  assert( (NULL != k) && (NULL != k->data) );

  // Storage for borders will be allocated later.
  cv::Mat * w_min = NULL;
  cv::Mat * w_max = NULL;

  // Bail-out if allocation failed.
  if ( (NULL == k) || (NULL == k->data) )
    {
      result = false;
      goto mps_get_period_tuples_EXIT;
    }
  /* if */

  // Create valid tuples.
  while ( (x < width) &&
          (0 < *std::max_element(inc.begin(), inc.end()))
          )
    {
      // Get next boundary.
      for (int i = 0; i < i_max; ++i)
        {
          if (0 < inc[i]) next_boundary[i] += lambda_in[i];
        }
      /* for */

      // Increase storage if necessary.
      if (j >= k->rows) k->resize(k->rows + j_step, cv::Scalar(0));

      // Set next period-order tuple.
      int const j_prev = j - 1;
      for (int i = 0; i < i_max; ++i)
        {
          k->at<int>(j, i) = k->at<int>(j_prev, i) + inc[i];
        }
      /* for */
      j = j + 1;

      // Find next minimal boundary.
      x = *std::min_element(next_boundary.begin(), next_boundary.end());
      for (int i = 0; i < i_max; ++i) inc[i] = (next_boundary[i] <= x)? 1 : 0;
    }
  /* while */

  // Efficiently create new cv::Mat header for the output buffer.
  int const N = j;
  if (N != k->rows)
    {
      cv::Range r[2] = {cv::Range(0, N), cv::Range::all()};

      cv::Mat * k_tmp = new cv::Mat(*k, r);
      assert(NULL != k_tmp);
      if (NULL != k_tmp)
        {
          SAFE_DELETE( k );
          k = k_tmp;
          k_tmp = NULL;
        }
      /* if */
    }
  /* if */

  // Compute interval boundaries if requested.
  if ( (NULL != w_min_out) || (NULL != w_max_out) )
    {
      w_min = new cv::Mat(N, 1, CV_64FC1, 0.0);
      assert( (NULL != w_min) && (NULL != w_min->data) );

      w_max = new cv::Mat(N, 1, CV_64FC1, 0.0);
      assert( (NULL != w_max) && (NULL != w_max->data) );

      if ( (NULL == w_min) || (NULL == w_min->data) ||
           (NULL == w_max) || (NULL == w_max->data)
           )
        {
          result = false;
          goto mps_get_period_tuples_EXIT;
        }
      /* if */

      // Compute interval boundaries.
      double stop_max = -std::numeric_limits<double>::infinity();
      for (int j = 0; j < N; ++j)
        {
          stop_max = -std::numeric_limits<double>::infinity();
          for (int i = 0; i < i_max; ++i)
            {
              double const stop = (double)( k->at<int>(j, i) ) * lambda_in[i];
              if (stop > stop_max) stop_max = stop;
            }
          /* for */

          w_min->at<double>(j, 0) = stop_max;
          if (0 < j) w_max->at<double>(j - 1, 0) = stop_max;
        }
      /* for */
      if (0 < N) w_max->at<double>(N - 1, 0) = width;
    }
  /* if */


  SAFE_ASSIGN_PTR( k, k_out );
  if (NULL != width_out) *width_out = width;
  SAFE_ASSIGN_PTR( w_min, w_min_out );
  SAFE_ASSIGN_PTR( w_max, w_max_out );

 mps_get_period_tuples_EXIT:

  SAFE_DELETE( k );
  SAFE_DELETE( w_min );
  SAFE_DELETE( w_max );

  return result;
}
/* mps_get_period_tuples */



//! Return line equations for chosen set of wavelengths.
/*!
  Function computes line equations for a chosen set of wavelengths.
  Line equations are defined by line direction vector V and particular points X0 on each line.
  Line direction V is stored as row vector.
  Particular points X0 are stored as one point per row.

  \param lambda_in      Pointer to wavelengths.
  \param width_in       Maximal unwrapped phase width; may be set to NaN in which case it is automaticall computed from wavelengths lambda.
  \param V_out          Address where pointer to cv::Mat which stores line direction vector will be placed. May be NULL.
  \param X0_out         Address where pointer to cv::Mat which stores line points will be placed. May be NULL.
  \param k_out          Address where pointer to cv::Mat which stores all period tuples will be placed. May be NULL.
  \param width_out      Address where used maximal width will be stored.
  \param w_min          Address where pointer to cv::Mat which stores starting boundaries of each segment will be placed. May be NULL.
  \param w_max          Address where pointer to cv::Mat which stores ending boundaries of each segment will be placed. May be NULL.
  \return Function returns true if successfull, false otherwise.
*/
bool
mps_get_lines(
              std::vector<double> const & lambda_in,
              double const width_in,
              cv::Mat * * const V_out,
              cv::Mat * * const X0_out,
              cv::Mat * * const k_out,
              double * const width_out,
              cv::Mat * * const w_min,
              cv::Mat * * const w_max
              )
{
  double width = width_in;

  cv::Mat * V = NULL;
  cv::Mat * X0 = NULL;
  cv::Mat * k = NULL;

  // Get period tuples.
  bool result = mps_get_period_tuples(lambda_in, width_in, &k, &width, w_min, w_max);
  assert( (NULL != k) && (NULL != k->data) && (true == result) );

  // Return immediately if unsuccessfull.
  if ( (NULL == k) || (NULL == k->data) || (false == result) )
    {
      result = false;
      goto mps_get_lines_EXIT;
    }
  /* if */

  // Fetch period-tuple sizes.
  int const N = k->rows;
  int const M = k->cols;
  assert( M == (int)( lambda_in.size() ) );

  double const pi = 3.141592653589793238462643383279502884197169399375;
  double const tau = 2.0 * pi;

  // Allocate storage on heap.
  V = new cv::Mat(1, M, CV_64FC1);
  assert( (NULL != V) && (NULL != V->data) );

  X0 = new cv::Mat(N, M, CV_64FC1);
  assert( (NULL != X0) && (NULL != X0->data) );

  if ( (NULL == V) || (NULL == V->data) ||
       (NULL == X0) || (NULL == X0->data)
       )
    {
      result = false;
      goto mps_get_lines_EXIT;
    }
  /* if */

  // Compute points on lines.
  for (int j = 0; j < N; ++j)
    {
      int const * const srcrow = (int *)( (BYTE *)(k->data) + k->step[0] * j );
      double * const dstrow = (double *)( (BYTE *)(X0->data) + X0->step[0] * j );
      for (int i = 0; i < M; ++i)
        {
          dstrow[i] = -tau * srcrow[i];
        }
      /* for */
    }
  /* for */

  // Compute line direction vector.
  {
    double * const dstrow = (double *)( (BYTE *)(V->data) );
    for (int i = 0; i < M; ++i)
      {
        dstrow[i] = tau / lambda_in[i];
      }
    /* for */
  }


  SAFE_ASSIGN_PTR( V, V_out );
  SAFE_ASSIGN_PTR( X0, X0_out );
  SAFE_ASSIGN_PTR( k, k_out );

 mps_get_lines_EXIT:

  SAFE_DELETE( V );
  SAFE_DELETE( X0 );
  SAFE_DELETE( k );

  return result;
}
/* mps_get_lines */



//! Return wrapped tupples.
/*!
  Function computes all wrapped period tuples.

  \param lambda_in      Pointer to wavelengths.
  \param width_in       Maximal unwrapped phase width; may be set to NaN in which case it is automaticall computed from wavelengths lambda.
  \param k_out          Address where pointer to cv::Mat which stores all wrapped period tuples will be placed. May be NULL.
  \param width_out      Address where used maximal width will be stored.
  \param idx_out        Address where pointer to cv::Mat which stores indices into list of all unwrapped period tuples.
  \return Function returns true if successfull, false otherwise.
*/
bool
mps_get_wrapped_tuples(
                       std::vector<double> const & lambda_in,
                       double const width_in,
                       cv::Mat * * const k_out,
                       double * const width_out,
                       cv::Mat * * const idx_out
                       )
{
  // Set default width.
  double width = width_in;
  if ( 0 != _isnan(width_in) )
    {
      double const lcm = mps_lcm(lambda_in);
      double const gcd = mps_gcd(lambda_in);
      width = lcm / gcd;
      assert( 0 == _isnan(width) );
    }
  /* if */

  bool result = true; // Assume success.

  // Allocate storage on stack.
  int const i_max = (int)( lambda_in.size() );

  int j = 0;
  int j_tmp = 0;
  int const j_step = 100;

  // Allocate storage on heap.
  cv::Mat * k = new cv::Mat(j_step, i_max, CV_32SC1, cv::Scalar(0));
  assert( (NULL != k) && (NULL != k->data) );

  cv::Mat * k_tmp = new cv::Mat(j_step, i_max, CV_32SC1, cv::Scalar(0));
  assert( (NULL != k_tmp) && (NULL != k_tmp->data) );

  cv::Mat * idx = new cv::Mat(j_step, 1, CV_32SC1, cv::Scalar(0));
  assert( (NULL != idx) && (NULL != idx->data) );

  if ( (NULL == k) || (NULL == k->data) ||
       (NULL == k_tmp) || (NULL == k_tmp->data) ||
       (NULL == idx) || (NULL == idx->data)
       )
    {
      result = false;
      goto mps_get_wrapped_tuples_EXIT;
    }
  /* if */

  // Generate first wrapped tuple. The first wrapped tuple covers all
  // corners of a hyper-cube except ones whose all coordinates are 0 or 2*pi.
  {
    assert(i_max < 32);
    int b = 1;
    double const b_max_dbl = ldexp(1.0, i_max) - 1.0;
    assert( b_max_dbl < INT_MAX );
    int const b_max = (int)( b_max_dbl );
    while (b < b_max)
      {
        // Increase storage if necessary.
        if (j >= k->rows) k->resize(k->rows + j_step, cv::Scalar(0));
        if (j >= idx->rows) idx->resize(idx->rows +j_step, cv::Scalar(0));

        // Copy starting tupple and starting index.
        for (int i = 0; i < i_max; ++i)
          {
            k->at<int>(j, i) = k_tmp->at<int>(1, i);
          }
        /* for */
        idx->at<int>(j, 0) = 0;

        // Adjust tupple to obtain wrapped tuple.
        for (int i = 0; i < i_max; ++i)
          {
            int const bitmask = (1 << i);
            if ( 0 != (b & bitmask) )
              {
                k->at<int>(j, i) = k->at<int>(j, i) - 1;
              }
            /* if */
          }
        /* for */

        // Go to next wrapped tuple.
        j = j + 1;
        b = b + 1;
      }
    /* while */
  }

  // Create remainin wrapped tuples.
  // Note that we first generate regular tuples in k_tmp.
  {
    j_tmp = j_tmp + 1;

    std::vector<double> next_boundary(lambda_in);
    std::vector<int> inc(lambda_in.size(), 0);
    std::vector<int> I(lambda_in.size(), 0);

    double x = *std::min_element(next_boundary.begin(), next_boundary.end());
    for (int i = 0; i < i_max; ++i) inc[i] = (next_boundary[i] <= x)? 1 : 0;

    while ( (x < width) &&
            (0 < *std::max_element(inc.begin(), inc.end()))
            )
      {
        // Increase storage if necessary.
        if (j_tmp >= k_tmp->rows) k_tmp->resize(k_tmp->rows + j_step, cv::Scalar(0));

        // Fetch next regular tuple.
        int const j_tmp_prev = j_tmp - 1;
        for (int i = 0; i < i_max; ++i)
          {
            k_tmp->at<int>(j_tmp, i) = k_tmp->at<int>(j_tmp_prev, i) + inc[i];
          }
        /* for */

        // Get next boundary.
        int s = 0;
        I.clear();
        for (int i = 0; i < i_max; ++i)
          {
            if (0 < inc[i])
              {
                next_boundary[i] += lambda_in[i];
                ++s;
                I.push_back(i);
              }
            /* if */
          }
        /* for */

        // Generate wrapped tuple if boundary jump is at more than one axis.
        if (1 < s)
          {
            assert(s < 32);
            int b = 1;
            double const b_max_dbl = ldexp(1.0, s) - 1.0;
            assert( b_max_dbl < INT_MAX );
            int const b_max = (int)( b_max_dbl );
            while (b < b_max)
              {
                // Increase storage if necessary.
                if (j >= k->rows) k->resize(k->rows + j_step, cv::Scalar(0));
                if (j >= idx->rows) idx->resize(idx->rows +j_step, cv::Scalar(0));

                // Copy starting tupple and starting index.
                for (int i = 0; i < i_max; ++i)
                  {
                    k->at<int>(j, i) = k_tmp->at<int>(j_tmp, i);
                  }
                /* for */
                idx->at<int>(j, 0) = j_tmp;

                // Adjust tupple to obtain wrapped tuple.
                for (int i = 0; i < s; ++i)
                  {
                    int const bitmask = (1 << i);
                    if ( 0 != (b & bitmask) )
                      {
                        int const col = I[i];
                        k->at<int>(j, col) = k->at<int>(j, col) - 1;
                      }
                    /* if */
                  }
                /* for */

                // Go to next wrapped tuple.
                j = j + 1;
                b = b + 1;
              }
            /* while */
          }
        /* if */

        // Go to next regular tuple.
        j_tmp = j_tmp + 1;
        x = *std::min_element(next_boundary.begin(), next_boundary.end());
        for (int i = 0; i < i_max; ++i) inc[i] = (next_boundary[i] <= x)? 1 : 0;
      }
    /* while */
  }

  // Efficiently create new cv::Mat headers for output buffers.
  int const N = j;
  if (N != k->rows)
    {
      cv::Range r[2] = {cv::Range(0, N), cv::Range::all()};

      cv::Mat * ptr = new cv::Mat(*k, r);
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          SAFE_DELETE( k );
          k = ptr;
          ptr = NULL;
        }
      /* if */
    }
  /* if */

  int const N_tmp = j_tmp;
  if (N_tmp != k_tmp->rows)
    {
      cv::Range r[2] = {cv::Range(0, N_tmp), cv::Range::all()};

      cv::Mat * ptr = new cv::Mat(*k_tmp, r);
      assert(NULL != ptr);
      if (NULL != ptr)
        {
          SAFE_DELETE( k_tmp );
          k_tmp = ptr;
          ptr = NULL;
        }
      /* if */
    }
  /* if */


  SAFE_ASSIGN_PTR( k, k_out );
  if (NULL != width_out) *width_out = width;
  SAFE_ASSIGN_PTR( idx, idx_out);

 mps_get_wrapped_tuples_EXIT:

  SAFE_DELETE( k );
  SAFE_DELETE( k_tmp );
  SAFE_DELETE( idx );

  return result;
}
/* mps_get_wrapped_tuples */



//! Returns ortographic projection matrix and projected tuple center points.
/*!
  Function computes orthographic projection matrix O, tuple center points Xk,
  wrapped period-order t.

  \param lambda_in      Pointer to wavelengths.
  \param width_in       Maximal unwrapped phase width; may be set to NaN in which case it is automaticall computed from wavelengths lambda.
  \param O_out          Address where pointer to cv::Mat which stores orthographic projection matrix will be stored. May be NULL.
  \param Xk_out         Address where pointer to cv::Mat which stores all period-order tuples center points will be stored. May be NULL.
  \param kk_out         Address where pointer to cv::Mat which stores all period tuples will be placed. May be NULL.
  \param Xw_out         Address where pointer to cv::Mat which stores all wrapped period-order tuples center points will be stored. May be NULL.
  \param kw_out         Address where pointer to cv::Mat which stores all wrapped period tuples will be placed. May be NULL.
  \param idxw_out       Address where pointer to cv::Mat which stores indices into list of all unwrapped period tuples for each wrapped period tuple. May be NULL.
  \param width_out      Address where used maximal width will be stored.
  \return Function returns true if successfull, false otherwise.
*/
bool
mps_get_projection_matrix_and_centers(
                                      std::vector<double> const & lambda_in,
                                      double const width_in,
                                      cv::Mat * * const O_out,
                                      cv::Mat * * const Xk_out,
                                      cv::Mat * * const kk_out,
                                      cv::Mat * * const Xw_out,
                                      cv::Mat * * const kw_out,
                                      cv::Mat * * const idx_out,
                                      double * const width_out
                                      )
{
  double width = width_in;

  cv::Mat * O = NULL;
  cv::Mat * Xk = NULL;
  cv::Mat * kk = NULL;
  cv::Mat * Xw = NULL;
  cv::Mat * kw = NULL;
  cv::Mat * idx = NULL;

  cv::Mat * V = NULL;
  cv::Mat * X0 = NULL;
  cv::Mat * X1 = NULL;

  bool result = true; // Assume success.

  // Get lines.
  bool const get_lines = mps_get_lines(lambda_in, width, &V, &X0, &kk, &width, NULL, NULL);
  assert(true == get_lines);
  result = result & get_lines;

  if ( (false == get_lines) ||
       (NULL == V) || (NULL == V->data) ||
       (NULL == X0) || (NULL == X0->data) ||
       (NULL == kk) || (NULL == kk->data)
       )
    {
      result = false;
      goto mps_get_projection_matrix_and_centers_EXIT;
    }
  /* if */

  // Get wrapped-around phase vectors.
  bool const get_wrapped_tuples = mps_get_wrapped_tuples(lambda_in, width, &kw, &width, &idx);
  result = result & get_wrapped_tuples;

  if ( (false == get_wrapped_tuples) ||
       (NULL == kw) || (NULL == kw->data) ||
       (NULL == idx) || (NULL == idx->data)
       )
    {
      result = false;
      goto mps_get_projection_matrix_and_centers_EXIT;
    }
  /* if */

  int const N0 = kk->rows;
  int const N1 = kw->rows;
  int const D = kk->cols;
  assert( D == kw->cols );
  assert( D == (int)( lambda_in.size() ) );

  // Allocate space for orthographic projection matrix.
  O = new cv::Mat(D-1, D, CV_64FC1);
  assert( (NULL != O) && (NULL != O->data) );
  if ( (NULL == O) || (NULL == O->data) )
    {
      result = false;
      goto mps_get_projection_matrix_and_centers_EXIT;
    }
  /* if */

  // Get projection matrix.
  {
    cv::Mat w, u, vt;
    cv::SVD::compute(*V, w, u, vt, cv::SVD::FULL_UV);

    double const s = w.at<double>(0,0);
    double const tol = (double)(V->cols) * s * DBL_EPSILON;
    int const r = ( (s > tol)? 1 : 0 );

    for (int j = 0; j < D - 1; ++j)
      {
        assert(j + r < vt.rows);
        double const * const srcrow = (double *)( (BYTE *)vt.data + vt.step[0] * (j + r) );
        double * const dstrow = (double *)( (BYTE *)O->data + O->step[0] * j );
        for (int i = 0; i < D; ++i)
          {
            dstrow[i] = srcrow[i];
          }
        /* for */
      }
    /* for */
  }

  // Allocate space for projected lines.
  Xk = new cv::Mat(N0, D - 1, CV_64FC1);
  assert( (NULL != Xk) && (NULL != Xk->data) );
  if ( (NULL == Xk) || (NULL == Xk->data) )
    {
      result = false;
      goto mps_get_projection_matrix_and_centers_EXIT;
    }
  /* if */

  // Project lines.
  *Xk = (*X0) * O->t();

  // Allocate space for wrapped lines and their projection.
  X1 = new cv::Mat(N1, D, CV_64FC1);
  assert( (NULL != X1) && (NULL != X1->data) );

  Xw = new cv::Mat(N1, D-1, CV_64FC1);
  assert( (NULL != Xw) && (NULL != Xw->data) );

  if ( (NULL == X1) || (NULL == X1->data) ||
       (NULL == Xw) || (NULL == Xw->data)
       )
    {
      result = false;
      goto mps_get_projection_matrix_and_centers_EXIT;
    }
  /* if */

  // Compute points on wrapped lines.
  {
    double const pi = 3.141592653589793238462643383279502884197169399375;
    double const tau = 2.0 * pi;

    for (int j = 0; j < N1; ++j)
      {
        int const * const srcrow = (int *)( (BYTE *)(kw->data) + kw->step[0] * j );
        double * const dstrow = (double *)( (BYTE *)(X1->data) + X1->step[0] * j );
        for (int i = 0; i < D; ++i)
          {
            dstrow[i] = -tau * (double)( srcrow[i] );
          }
        /* for */
      }
    /* for */
  }

  // Project wrapped lines.
  *Xw = (*X1) * O->t();


  SAFE_ASSIGN_PTR( O, O_out );
  SAFE_ASSIGN_PTR( Xk, Xk_out );
  SAFE_ASSIGN_PTR( kk, kk_out );
  SAFE_ASSIGN_PTR( Xw, Xw_out );
  SAFE_ASSIGN_PTR( kw, kw_out );
  SAFE_ASSIGN_PTR( idx, idx_out );
  if (NULL != width_out) *width_out = width;

 mps_get_projection_matrix_and_centers_EXIT:

  SAFE_DELETE( O );
  SAFE_DELETE( Xk );
  SAFE_DELETE( kk );
  SAFE_DELETE( Xw );
  SAFE_DELETE( kw );
  SAFE_DELETE( idx );
  SAFE_DELETE( V );
  SAFE_DELETE( X0 );
  SAFE_DELETE( X1 );

  return result;
}
/* mps_get_projection_matrix_and_centers */



//! Construct KD tree.
/*!
  Function consolidates regular and wrapped points and constructs KD tree over them.

  \param kk_in  Pointer to regular period-order vectors.
  \param Xk_in  Pointer to regular projected constellation lines.
  \param kw_in  Pointer to wrapped period-order vectors.
  \param Xw_in  Pointer to wrapped projected constellation lines.
  \param X_out  Address where the pointer to cv::Mat which holds all constellation points will be stored.
  \param k_out  Address where the pointer to cv::Mat which holds all period-order vectors will be stored.
  \param k_max_out  Address where the pointer to vector which holds maximal number of fringes for each wavelength will be stored.
  \param kd_tree_out    Address where the pointer to KD tree will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
mps_get_kd_tree(
                cv::Mat * const Xk_in,
                cv::Mat * const kk_in,
                cv::Mat * const Xw_in,
                cv::Mat * const kw_in,
                cv::Mat * * const X_out,
                cv::Mat * * const k_out,
                std::vector<int> * * const k_max_out,
                KDTreeRoot_ * * const kd_tree_out
                )
{
  bool result = true; // Assume success.

  cv::Mat * X = NULL;
  cv::Mat * k = NULL;
  std::vector<int> * k_max = NULL;
  KDTreeRoot * kd_tree = NULL;

  assert( (NULL != Xk_in) && (NULL != Xk_in->data) &&
          (NULL != kk_in) && (NULL != kk_in->data) &&
          (NULL != Xw_in) && (NULL != Xw_in->data) &&
          (NULL != kw_in) && (NULL != kw_in->data)
          );
  if ( (NULL == Xk_in) || (NULL == Xk_in->data) ||
       (NULL == kk_in) || (NULL == kk_in->data) ||
       (NULL == Xw_in) || (NULL == Xw_in->data) ||
       (NULL == kw_in) || (NULL == kw_in->data)
       )
    {
      result = false;
      goto mps_get_kd_tree_EXIT;
    }
  /* if */

  int const D = kk_in->cols;
  assert( D == kw_in->cols );
  assert( D - 1 == Xk_in->cols );
  assert( D - 1 == Xw_in->cols );
  int const Nk = Xk_in->rows;
  assert( Nk == kk_in->rows );
  int const Nw = Xw_in->rows;
  assert( Nw == kw_in->rows );
  int const N = Nk + Nw;

  // Allocate storage on heap.
  X = new cv::Mat(N, D - 1, CV_64FC1);
  assert( (NULL != X) && (NULL != X->data) );

  k = new cv::Mat(N, D, CV_32SC1);
  assert( (NULL != k) && (NULL != k->data) );

  k_max = new std::vector<int>();
  assert(NULL != k_max);

  kd_tree = new KDTreeRoot();
  assert(NULL != kd_tree);

  if ( (NULL == X) || (NULL == X->data) ||
       (NULL == k) || (NULL == k->data) ||
       (NULL == k_max) || (NULL == kd_tree)
       )
    {
      result = false;
      goto mps_get_kd_tree_EXIT;
    }
  /* if */

  // Consolidate regular and wrapped constellation points.
  int j = 0;
  for (; j < Nk; ++j)
    {
      double const * const Xk_row = (double *)( (BYTE *)Xk_in->data + Xk_in->step[0] * j );
      double * const X_row = (double *)( (BYTE *)X->data + X->step[0] * j );
      for (int i = 0; i < D - 1; ++i) X_row[i] = Xk_row[i];
    }
  /* for */

  for (; j < N; ++j)
    {
      int const jw = j - Nk;
      assert( (0 <= jw) && (jw < Xw_in->rows) );
      double const * const Xw_row = (double *)( (BYTE *)Xw_in->data + Xw_in->step[0] * jw );
      double * const X_row = (double *)( (BYTE *)X->data + X->step[0] * j );
      for (int i = 0; i < D - 1; ++i) X_row[i] = Xw_row[i];
    }
  /* for */

  if (0 < Nk)
    {
      k_max->reserve( (size_t)D );
      int const * const kk_row = (int *)( (BYTE *)kk_in->data );
      for (int i = 0; i < D; ++i)
        {
          k_max->push_back( kk_row[i] );
        }
      /* for */
      assert( D == (int)( k_max->size() ) );
    }
  /* if */

  for (j = 0; j < Nk; ++j)
    {
      int const * const kk_row = (int *)( (BYTE *)kk_in->data + kk_in->step[0] * j );
      int * const k_row = (int *)( (BYTE *)k->data +k->step[0] * j );
      for (int i = 0; i < D; ++i)
        {
          k_row[i] = kk_row[i];
          if (kk_row[i] > (*k_max)[i])
            {
              (*k_max)[i] = kk_row[i];
            }
          /* if */
        }
      /* for */
    }
  /* for */

  for (; j < N; ++j)
    {
      int const jw = j - Nk;
      assert( (0 <= jw) && (jw < kw_in->rows) );
      int const * const kw_row = (int *)( (BYTE *)kw_in->data + kw_in->step[0] * jw );
      int * const k_row = (int *)( (BYTE *)k->data + k->step[0] * j );
      for (int i = 0; i < D; ++i) k_row[i] = kw_row[i];
    }
  /* for */

  // Construct KD tree.
  {
    double const * const data = (double *)( X->data );
    bool const construct = kd_tree->ConstructTree(data, D-1, N, (int)(X->step[0]));
    assert(true == construct);
    result = result && construct;
  }


  SAFE_ASSIGN_PTR( X, X_out );
  SAFE_ASSIGN_PTR( k, k_out );
  SAFE_ASSIGN_PTR( k_max, k_max_out );
  SAFE_ASSIGN_PTR( kd_tree, kd_tree_out );

 mps_get_kd_tree_EXIT:

  SAFE_DELETE( X );
  SAFE_DELETE( k );
  SAFE_DELETE( k_max );
  SAFE_DELETE( kd_tree );

  return result;
}
/* mps_get_kd_tree */



//! Return standard weights.
/*!
  Returns standard weights for MPS unwrapping.

  \param lambda_in      Vector of used wavelengths.
  \param wgt_out     Address where pointer to computed weights will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
mps_get_weights(
                std::vector<double> const & lambda_in,
                std::vector<double> * * const wgt_out
                )
{
  bool result = false; // Assume failure.

  std::vector<double> * wgt = new std::vector<double>(lambda_in.size());
  assert(NULL != wgt);
  if (NULL != wgt)
    {
      int const i_max = (int)( lambda_in.size() );
      for (int i = 0; i < i_max; ++i)
        {
          (*wgt)[i] = 1.0 / ( lambda_in[i] * lambda_in[i] );
        }
      /* for */

      result = true;
    }
  /* if */

  SAFE_ASSIGN_PTR( wgt, wgt_out );
  SAFE_DELETE( wgt );

  return result;
}
/* mps_get_weights */



//! Unwraps phase using orthographic projection.
/*!
  Function unwraps wrapped phases using orthographic projection.

  \param WP_in  Vector of at least two wrapped phase images. The order of wrapped phase images must correspond to
  wavelength order used to construct the orthographic projection matrix O_in, constellation X_in, and KD tree kd_tree_in.
  \param O_in   Pointer to orthographic projection matrix.
  \param X_in   Pointer to points in constellation.
  \param k_in   Pointer to period-order numbers.
  \param kd_tree_in     Pointer to KD tree constructed over the constellation X_in.
  \param n      Vector of maximal frige counts for each wavelength.
  \param wgt    Vector of weights used to combine wrapped phases into absolute phase.
  \param idx    Address where cv::Mat holding indices into period-order vectors will be stored. May be NULL.
  \param dst    Address where cv::Mat holding distance to closest constellation point will be stored. May be NULL.
  \param abs_phase_out  Address where cv::Mat holding unwrapped phase will be stored. May be NULL.
  \return Returns true if successfull, false otherwise.
*/
bool
mps_unwrap_phase(
                 std::vector<cv::Mat *> const & WP_in,
                 cv::Mat * const O_in,
                 cv::Mat * const X_in,
                 cv::Mat * const k_in,
                 KDTreeRoot * const kd_tree_in,
                 std::vector<double> const & n_in,
                 std::vector<double> const & wgt_in,
                 cv::Mat * * const idx_out,
                 cv::Mat * * const dst_out,
                 cv::Mat * * const abs_phase_out
                 )
{
  bool result = true; // Assume success.

  cv::Mat * idx = NULL;
  cv::Mat * dst = NULL;
  cv::Mat * abs_phase = NULL;
  cv::Mat * WP = NULL;
  cv::Mat * WPO = NULL;

  int n_rows = 0;
  int n_cols = 0;

  assert( (NULL != O_in) && (NULL != O_in->data) &&
          (NULL != X_in) && (NULL != X_in->data) &&
          (NULL != k_in) && (NULL != k_in->data) &&
          (NULL != kd_tree_in) && (NULL != kd_tree_in->data)
          );
  if ( (NULL == O_in) || (NULL == O_in->data) ||
       (NULL == X_in) || (NULL == X_in->data) ||
       (NULL == k_in) || (NULL == k_in->data) ||
       (NULL == kd_tree_in) || (NULL == kd_tree_in->data)
       )
    {
      result = false;
      goto mps_unwrap_phase_EXIT;
    }
  /* if */

  int const D = (int)( WP_in.size() );
  assert( D == O_in->cols );
  assert( D - 1 == O_in->rows );
  assert( D - 1 == X_in->cols );
  assert( D == k_in->cols );
  assert( X_in->rows == k_in->rows );
  assert( D - 1 == kd_tree_in->n_dim );
  assert( X_in->rows = kd_tree_in->n_pts );
  assert( (void *)kd_tree_in->data == (void *)X_in->data );
  assert( D == (int)( n_in.size() ) );
  assert( D == (int)( wgt_in.size() ) );

  // Validate input wrapped phases; all inputs must have same size and type.
  {
    cv::Mat * WP_tmp = NULL;
    for (int i = 0; i < D; ++i)
      {
        if ( (NULL == WP_tmp) && (NULL != WP_in[0]) )
          {
            n_cols = WP_in[0]->cols;
            n_rows = WP_in[0]->rows;
          }
        /* if */

        WP_tmp = WP_in[i];
        if (NULL != WP_tmp)
          {
            assert( (NULL != WP_tmp) && (NULL != WP_tmp->data) );
            if ( (NULL == WP_tmp) || (NULL == WP_tmp->data) )
              {
                result = false;
                goto mps_unwrap_phase_EXIT;
              }
            /* if */

            assert( (CV_MAT_DEPTH(WP_tmp->type()) == CV_64F) &&
                    (CV_MAT_CN(WP_tmp->type()) == 1) ||
                    (n_cols == WP_tmp->cols) &&
                    (n_rows == WP_tmp->rows)
                    );
            if ( (CV_MAT_DEPTH(WP_tmp->type()) != CV_64F) ||
                 (CV_MAT_CN(WP_tmp->type()) != 1) ||
                 (n_cols != WP_tmp->cols) ||
                 (n_rows != WP_tmp->rows)
                 )
              {
                result = false;
                goto mps_unwrap_phase_EXIT;
              }
            /* if */
          }
        /* if */
      }
    /* for */
  }

  int const N = n_rows * n_cols;

  // Allocate required space.
  idx = new cv::Mat(n_rows, n_cols, CV_32SC1);
  assert( (NULL != idx) && (NULL != idx->data) );

  dst = new cv::Mat(n_rows, n_cols, CV_32FC1);
  assert( (NULL != dst) && (NULL != dst->data) );

  abs_phase = new cv::Mat(n_rows, n_cols, CV_64FC1, cv::Scalar(0.0));
  assert( (NULL != abs_phase) && (NULL != abs_phase->data) );

  WP = new cv::Mat(N, D, CV_64FC1);
  assert( (NULL != WP) && (NULL != WP->data) );

  WPO = new cv::Mat(N, D - 1, CV_64FC1);
  assert( (NULL != WPO) && (NULL != WPO->data) );

  if ( (NULL == idx) || (NULL == idx->data) ||
       (NULL == dst) || (NULL == dst->data) ||
       (NULL == abs_phase) || (NULL == abs_phase->data) ||
       (NULL == WP) || (NULL == WP->data) ||
       (NULL == WPO) || (NULL == WPO->data)
       )
    {
      result = false;
      goto mps_unwrap_phase_EXIT;
    }
  /* if */

  // Copy wrapped phase data from input images into one array.
  for (int j = 0; j < n_rows; ++j)
    {
      int const offset = j * n_cols;
      for (int i = 0; i < n_cols; ++i)
        {
          double * const dstrow = (double *)( (BYTE *)WP->data + WP->step[0] * (offset + i) );
          for (int d = 0; d < D; ++d)
            {
              cv::Mat * const WP_tmp = WP_in[d];
              double const * const srcrow = (double *)( (BYTE *)WP_tmp->data + WP_tmp->step[0] * j );
              dstrow[d] = srcrow[i];
            }
          /* for */
        }
      /* for */
    }
  /* for */

  // Apply orthographic projection.
  *WPO = (*WP) * O_in->t();

  // Find period-order number for each point.
  {
    KDTreeClosestPoint best;

    for (int j = 0; j < n_rows; ++j)
      {
        int const offset = j * n_cols;
        int * const idx_row = (int *)( (BYTE *)idx->data + idx->step[0] * j );
        float * const dst_row = (float *)( (BYTE *)dst->data + dst->step[0] * j );

        for (int i = 0; i < n_cols; ++i)
          {
            double * const query = (double *)( (BYTE *)WPO->data + WPO->step[0] * (offset + i) );
            best.query = query;

            bool const find = kd_tree_in->Find1NN(best);
            assert(true == find);

            idx_row[i] = best.idx;
            dst_row[i] = (float)( sqrt( best.dst2 ) );
          }
        /* for */
      }
    /* for */
  }

  // Unwrap phase. Note that we combine all unwrapped phases using given weights.
  {
    // Precompute required phase-offsets for each period-order vector.
    double const pi = 3.141592653589793238462643383279502884197169399375;
    cv::Mat kpi(k_in->rows, k_in->cols, CV_64FC1);
    for (int j = 0; j < k_in->rows; ++j)
      {
        int const * const srcrow = (int *)( (BYTE *)k_in->data + k_in->step[0] * j );
        double * const dstrow = (double *)( (BYTE *)kpi.data + kpi.step[0] * j );
        for (int i = 0; i < k_in->cols; ++i)
          {
            dstrow[i] = (2.0 * pi) * (double)( srcrow[i] );
          }
        /* for */
      }
    /* for */

    // Precompute scaling factors used to combine multiple unwrapped phases.
    std::vector<double> scl;
    scl.reserve( D );
    double const wgt_sum = std::accumulate(wgt_in.begin(), wgt_in.end(), 0.0);
    for (int d = 0; d < D; ++d)
      {
        double const scl_tmp = wgt_in[d] / (2.0 * pi * wgt_sum * n_in[d]);
        scl.push_back(scl_tmp);
      }
    /* for */

    // Fetch data pointer to phase offsets.
    double const * const kpi_ptr = (double *)( kpi.data );
    assert( sizeof(double) * D == kpi.step[0] );

    // Compute unwrapped phase.
    for (int j = 0; j < n_rows; ++j)
      {
        int const offset = j * n_cols;
        int const * const idx_row = (int *)( (BYTE *)idx->data + idx->step[0] * j );
        double * const abs_phase_row = (double *)( (BYTE *)abs_phase->data + abs_phase->step[0] * j );

        for (int i = 0; i < n_cols; ++i)
          {
            double * const wrapped_phase = (double *)( (BYTE *)WP->data + WP->step[0] * (offset + i) );
            int const phase_idx = idx_row[i];
            for (int d = 0; d < D; ++d)
              {
                double const phase_offset = kpi_ptr[D * phase_idx + d];
                abs_phase_row[i] += scl[d] * ( wrapped_phase[d] + phase_offset );
              }
            /* for */
          }
        /* for */
      }
    /* for */
  }


  SAFE_ASSIGN_PTR( idx, idx_out );
  SAFE_ASSIGN_PTR( dst, dst_out );
  SAFE_ASSIGN_PTR( abs_phase, abs_phase_out );

 mps_unwrap_phase_EXIT:

  SAFE_DELETE( idx );
  SAFE_DELETE( abs_phase );
  SAFE_DELETE( WP );
  SAFE_DELETE( WPO );

  return result;
}
/* mps_unwrap_phase */



/****** PHASE STATISTICS ON SLIDING WINDOW ******/


//! Compute statistics.
/*!
  Compute statistics for absolute phase using a sliding window.

  \param abs_phase      Absolute phase image.
  \param nx     Sliding window size along x dimension (number of columns).
  \param ny     Sliding window size along y dimension (number of rows).
  \param abs_phase_order_out    Pointer where the order information will be stored.
  \param abs_phase_deviation_out        Pointer where the deviation information will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
GetAbsolutePhaseOrderAndDeviation(
                                  cv::Mat * const abs_phase,
                                  int const nx,
                                  int const ny,
                                  cv::Mat * * const abs_phase_order_out,
                                  cv::Mat * * const abs_phase_deviation_out
                                  )
{
  assert( (NULL != abs_phase) && (NULL != abs_phase->data) );
  if ( (NULL == abs_phase) || (NULL == abs_phase->data) ) return false;

  assert( (CV_MAT_DEPTH(abs_phase->type()) == CV_64F) && (CV_MAT_CN(abs_phase->type()) == 1) );
  if ( (CV_MAT_DEPTH(abs_phase->type()) != CV_64F) || (CV_MAT_CN(abs_phase->type()) != 1) ) return false;

  // Fetch input image size.
  int const cols = abs_phase->cols;
  int const rows = abs_phase->rows;

  assert( (0 < nx) && (nx < cols) );
  if ( (nx <= 0) || (cols <= nx) ) return false;

  assert( (0 < ny) && (ny < rows) );
  if ( (ny < 0) && (rows <= ny) ) return false;

  bool result = true; // Assume success.

  cv::Mat * abs_phase_order = new cv::Mat(rows, cols, CV_32FC1);
  assert( (NULL != abs_phase_order) && (NULL != abs_phase_order->data) );

  cv::Mat * abs_phase_deviation = new cv::Mat(rows, cols, CV_32FC1);
  assert( (NULL != abs_phase_deviation) && (NULL != abs_phase_deviation->data) );

  if ( (NULL == abs_phase_order) || (NULL == abs_phase_order->data) ||
       (NULL == abs_phase_deviation) || (NULL == abs_phase_deviation->data)
       )
    {
      result = false;
      goto GetAbsolutePhaseOrderAndDeviation_EXIT;
    }
  /* if */

  // Compute mean and standard deviation.
  {
    int const cx = (nx - 1) / 2;
    int const cy = (ny - 1) / 2;

    int const maxx = cols - nx + cx + 1;
    int const maxy = rows - ny + cy + 1;

    // Precompute scaling factors to avoid division.
    int const n = nx * ny;
    std::vector<double> inv_length( (size_t)n );
    for (int i = 0; i < n; ++i) inv_length[i] = 1.0 / (double)(i + 1);

    double const scl_M2 = 1.0 / ((double)n - 1.0);

    // Set borders to zero.
    for (int y = 0; y < cy; ++y)
      {
        float * const phase_order = (float *)( (BYTE *)(abs_phase_order->data) + abs_phase_order->step[0] * y );
        float * const phase_deviation = (float *)( (BYTE *)(abs_phase_deviation->data) + abs_phase_deviation->step[0] * y );
        for (int x = 0; x < cols; ++x)
          {
            phase_order[x] = 0.0f;
            phase_deviation[x] = 0.0f;
          }
        /* for */
      }
    /* for */

    // Step through all pixels which have a completely valid ROI.    
    for (int y = cy; y < maxy; ++y)
      {
        double const * const ptr = (double *)( (BYTE *)(abs_phase->data) + abs_phase->step[0] * (y - cy) );
        double const * const ilen = &( inv_length[0] );
        float * const phase_order = (float *)( (BYTE *)(abs_phase_order->data) + abs_phase_order->step[0] * y );
        float * const phase_deviation = (float *)( (BYTE *)(abs_phase_deviation->data) + abs_phase_deviation->step[0] * y );
        
        // Set borders to zero.
        for (int x = 0; x < cx; ++x)
          {
            phase_order[x] = 0.0f;
            phase_deviation[x] = 0.0f;
          }
        /* for */

        for (int x = cx; x < maxx; ++x)
          {
            // Compute meand and standard deviation using online algorithm.
            double mean = 0.0;
            double M2 = 0.0;
            int idx = 0;
            double const * const ptr_ROI = ptr + (x - cx);
            for (int j = 0; j < ny; ++j)
              {
                double const * const src = (double *)( (BYTE *)(ptr_ROI) + abs_phase->step[0] * j ) ;
                for (int i = 0; i < nx; ++i)
                  {
                    double const value = src[i];
                    double const delta = value - mean;
                    mean += delta * ilen[idx++];
                    M2 += delta * (value - mean);
                  }
                /* for */
              }
            /* for */
            double const dev = sqrt( scl_M2 * M2 );
            assert(0 <= mean);
            assert(0 <= dev);
          
            phase_order[x] = (float)( fabs(mean - ptr[x]) );
            phase_deviation[x] = (float)( dev );
          }
        /* for */

        // Set borders to zero.
        for (int x = maxx; x < cols; ++x)
          {
            phase_order[x] = 0.0f;
            phase_deviation[x] = 0.0f;
          }
        /* for */
      }
    /* for */

    // Set borders to zero.
    for (int y = maxy; y < rows; ++y)
      {
        float * const phase_order = (float *)( (BYTE *)(abs_phase_order->data) + abs_phase_order->step[0] * y );
        float * const phase_deviation = (float *)( (BYTE *)(abs_phase_deviation->data) + abs_phase_deviation->step[0] * y );
        for (int x = 0; x < cols; ++x)
          {
            phase_order[x] = 0.0f;
            phase_deviation[x] = 0.0f;
          }
        /* for */
      }
    /* for */
  }


  SAFE_ASSIGN_PTR( abs_phase_order, abs_phase_order_out );
  SAFE_ASSIGN_PTR( abs_phase_deviation, abs_phase_deviation_out );

 GetAbsolutePhaseOrderAndDeviation_EXIT:

  SAFE_DELETE( abs_phase_order );
  SAFE_DELETE( abs_phase_deviation );

  return result;
}
/* GetAbsolutePhaseOrderAndDeviation */



//! Combine phase deviation or distance (single precision).
/*!
  Combines phase deviation or distance by keeping the highest deviation value (the worst case).
  Function may be used to get per-pixel maximum value of any two single precision images
  which have the same size.

  \param deviation_1        First phase deviation image; must be single precision.
  \param deviation_2        Second phase deviation image; must be single precision.
  \return Returns pointer to combined phase deviation image or NULL if unsuccessfull.
*/
cv::Mat *
CombinePhaseDeviationOrDistance(
                                cv::Mat const * const deviation_1,
                                cv::Mat const * const deviation_2
                                )
{
  assert( (NULL != deviation_1) && (NULL != deviation_1->data) );
  if ( (NULL == deviation_1) || (NULL == deviation_1->data) ) return NULL;

  assert( (NULL != deviation_2) && (NULL != deviation_2->data) );
  if ( (NULL == deviation_2) || (NULL == deviation_2->data) ) return NULL;

  assert( (CV_MAT_DEPTH(deviation_1->type()) == CV_32F) && (CV_MAT_CN(deviation_1->type()) == 1) );
  if ( (CV_MAT_DEPTH(deviation_1->type()) != CV_32F) || (CV_MAT_CN(deviation_1->type()) != 1) ) return NULL;

  assert( (CV_MAT_DEPTH(deviation_2->type()) == CV_32F) && (CV_MAT_CN(deviation_2->type()) == 1) );
  if ( (CV_MAT_DEPTH(deviation_2->type()) != CV_32F) || (CV_MAT_CN(deviation_2->type()) != 1) ) return NULL;

  int const cols = deviation_1->cols;
  int const rows = deviation_1->rows;

  assert( (cols == deviation_2->cols) && (rows == deviation_2->rows) );
  if ( (cols != deviation_2->cols) || (rows != deviation_2->rows) ) return NULL;

  cv::Mat * phase_deviation = new cv::Mat(rows, cols, CV_32FC1, cv::Scalar(0.0f));
  assert(NULL != phase_deviation);
  if (NULL == phase_deviation) return NULL;

  for (int y = 0; y < rows; ++y)
    {
      // Get row addresses.
      float const * const row_deviation_1 = (float *)( (BYTE *)(deviation_1->data) + deviation_1->step[0] * y );
      float const * const row_deviation_2 = (float *)( (BYTE *)(deviation_2->data) + deviation_2->step[0] * y );
      float       * const row_phase_deviation = (float *)( (BYTE *)(phase_deviation->data) + phase_deviation->step[0] * y );

      // Unrolled for loop with step 4.
      int x = 0;
      int const max_x = cols - 3;
      for (; x < max_x; x += 4)
        {
          row_phase_deviation[x    ] = (row_deviation_1[x    ] > row_deviation_2[x    ])? row_deviation_1[x    ] : row_deviation_2[x    ];
          row_phase_deviation[x + 1] = (row_deviation_1[x + 1] > row_deviation_2[x + 1])? row_deviation_1[x + 1] : row_deviation_2[x + 1];
          row_phase_deviation[x + 2] = (row_deviation_1[x + 2] > row_deviation_2[x + 2])? row_deviation_1[x + 2] : row_deviation_2[x + 2];
          row_phase_deviation[x + 3] = (row_deviation_1[x + 3] > row_deviation_2[x + 3])? row_deviation_1[x + 3] : row_deviation_2[x + 3];
        }
      /* for */

      // Complete to end.
      for (; x < cols; ++x)
        {
          row_phase_deviation[x] = (row_deviation_1[x] > row_deviation_2[x])? row_deviation_1[x] : row_deviation_2[x];
        }
      /* for */
    }
  /* for */

  return phase_deviation;
}
/* CombinePhaseDeviationOrDistance */



#endif /* !__BATCHACQUISITIONPROCESSINGPHASESHIFT_CPP */
