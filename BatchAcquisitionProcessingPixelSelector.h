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
  \file   BatchAcquisitionProcessingPixelSelector.h
  \brief  Valid pixel selection.

  Functions for identifying valid pixels.

  \author Tomislav Petkovic, Tomislav Pribanic
  \date   2015-05-28
*/


#ifndef __BATCHACQUISITIONPROCESSINGPIXELSELECTOR_H
#define __BATCHACQUISITIONPROCESSINGPIXELSELECTOR_H


#include "BatchAcquisitionProcessing.h"


//! Get absolute threshold.
double GetAbsoluteThreshold(ImageSet * const, double const rel_thr);

//! Creates matrices holding pixel coordinates.
bool GetValidPixelCoordinates(cv::Mat * const, float const, cv::Mat * * const, cv::Mat * * const, cv::Mat * * const);

//! Get projector row or column.
bool GetProjectorCoordinate(cv::Mat * const, cv::Mat * const, cv::Mat * const, double const, cv::Mat * * const);




//! Validates input coordinates.
/*!
  Input coordinates must be matching matrices, i.e. they must have
  a same number of elements of the same type.

  \param x      Matrix holding x coordinates.
  \param y      Matrix holding y coordinates.
  \param depth  Matrix datatype.
  \return Function returns true if all coordinates are defined and are of the same size and type.
*/
inline
bool
CheckCoordinateArrays_inline(
                             cv::Mat * const x,
                             cv::Mat * const y,
                             int const depth
                             )
{
  assert( (NULL != x) && ( (NULL != x->data) != ((0 == x->cols) || (0 == x->rows)) ) );
  if ( (NULL == x) || ( (NULL == x->data) == ((0 < x->cols) && (0 < x->rows)) ) ) return false;

  assert( (NULL != y) && ( (NULL != y->data) != ((0 == y->cols) || (0 == y->rows)) ) );
  if ( (NULL == y) || ( (NULL == y->data) == ((0 < y->cols) && (0 < y->rows)) ) ) return false;

  assert( (x->rows == y->rows) && (x->cols == y->cols) );
  if ( (x->rows != y->rows) || (x->cols != y->cols) ) return false;

  assert( (depth == CV_MAT_DEPTH(x->type())) && (depth == CV_MAT_DEPTH(y->type())) );
  if ( (depth != CV_MAT_DEPTH(x->type())) || (depth != CV_MAT_DEPTH(y->type())) ) return false;

  return true;
}
/* CheckCoordinateArrays_inline */



#endif /* !__BATCHACQUISITIONPROCESSINGPIXELSELECTOR_H */
