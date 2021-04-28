/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 * 
 * (c) 2015 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionPatternSolid.h
  \brief  Solid color light patterns.

  \author Tomislav Petkovic
  \date   2016-04-06
*/


#ifndef __BATCHACQUISITIONPATTERNSOLID_H
#define __BATCHACQUISITIONPATTERNSOLID_H


#include "BatchAcquisition.h"


//! Solid pattern.
/*!
  Solid pattern consists of single color in various intensities.
*/
typedef
struct SolidPattern_
{
  D3DCOLORVALUE color; //!< Color.
  StructuredLightPatternType pattern_type; //!< Pattern ID.
  double t_delay; //!< Trigger to acquisition delay (in ms).
  double t_exp; //!< Exposure time (in ms).
  bool skip_acquisition; //!< Flag to indicate image should not be acquired.
} SolidPattern;


//! List of found images in selected directory.
typedef std::list<SolidPattern> solid_pattern_list;

//! Iterator for going through images in normal direction.
typedef solid_pattern_list::iterator solid_pattern_list_iterator;

//! Iterator for going through images in reverse direction.
typedef solid_pattern_list::reverse_iterator solid_pattern_list_reverse_iterator;


//! Filename list.
/*!
  This structure stores current file iterator so we can move
  easily forward and backward through the file list.
*/
typedef
struct SolidPatternList_
{
  solid_pattern_list * patternlist; /*!< Structured light image list. */
  solid_pattern_list_iterator * it; /*!< Structured light image list iterator. */
  solid_pattern_list_reverse_iterator * rit; /*!< Reverse structured light image list iterator. */  

  CRITICAL_SECTION csPatternList; //!< Critical section for syncronizing access to image list.

  bool cycle; //!< Flag to indicate cycling through the list.

  //! Blanks SolidPatternList structure.
  void Blank(void);

  //! Initializes SolidPatternList structure.
  void Initialize(void);

  //! Releases allocated resources.
  void Release(void);

  //! Steps to next image.
  bool Next(void);

  //! Steps to previous image.
  bool Prev(void);

  //! Returns color of current solid pattern.
  bool GetColor(D3DCOLORVALUE * const);

  //! Returns pattern ID of current solid pattern.
  bool GetID(StructuredLightPatternType * const);

  //! Returns delay time of current solid pattern.
  bool GetDelay(double * const);

  //! Returns exposure time of current solid pattern.
  bool GetExposure(double * const);

  //! Returns skip acquisition flag.
  bool GetSkipAcquisition(bool * const);

  //! Returns filename that uniquely describes the pattern.
  bool GetFileName(wchar_t * const, int const);

  //! Returns image index.
  int GetImageIndex(void);

  //! Returns color at specified list index.
  bool GetColorAt(int const, D3DCOLORVALUE * const);

  //! Returns pattern ID at specified list index.
  bool GetIDAt(int const, StructuredLightPatternType * const);

  //! Returns delay time at specified list index.
  bool GetDelayAt(int const, double * const);

  //! Returns exposure time at specified list index.
  bool GetExposureAt(int const, double * const);

  //! Returns skip acquisition flag at specified list index.
  bool GetSkipAcquisitionAt(int const, bool * const);

  //! Returns filename at specified list index.
  bool GetFileNameAt(int const, wchar_t * const, int const);

  //! Rewinds list to start.
  bool Rewind(void);

  //! Size.
  int Size(void);

  //! At end.
  bool AtEnd(void);

  //! Generate pattern for DLP color wheel analysis.
  bool GenerateDLPWheelPattern(
                               int const,
                               double const,
                               float const,
                               float const,
                               float const,
                               bool const
                               );

  //! Generate channel transfer pattern.
  bool GenerateChannelTransferPattern(
                                      int const,
                                      int const,
                                      int const,
                                      int const,
                                      int const,
                                      int const,
                                      int const,
                                      double const
                                      );

  //! Generate pattern for delay measurement.
  bool GenerateDelayMeasurementPattern(
                                       double const,
                                       double const,
                                       double const
                                       );

} SolidPatternList;


//! Rounds color to nearest integer.
int
ConvertColor(
             float const
             );

//! Renders solid structured light pattern.
HRESULT
RenderSolidPattern(
                   float const,
                   float const,
                   float const,
                   float const,
                   ID2D1RenderTarget * const
                   );



#endif /* !__BATCHACQUISITIONPATTERNSOLID_H */
