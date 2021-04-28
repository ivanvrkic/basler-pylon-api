/*
 * FER
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 * 
 * (c) 2015 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionPatternSolid.cpp
  \brief  Solid color light patterns.

  Functions for generating solid color light patterns.

  \author Tomislav Petkovic
  \date   2016-04-06
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONPATTERNSOLID_CPP
#define __BATCHACQUISITIONPATTERNSOLID_CPP


#include "BatchAcquisitionPatternSolid.h"


/****** HELPER FUNCTIONS ******/


//! Create file name.
/*!
  Function creates file name for specific solid color pattern.

  \param pattern_type   Pattern type.
  \param R      Red channel value.
  \param G      Green channel value.
  \param B      Blue channel value.
  \param t_delay Delay time (in ms).
  \param t_exp   Exposure time (in ms).
  \param name_out       Pointer where the created filename will be stored.
  \param size_out       Size of the buffer.
  \return Function returns true if successfull, false otherwise.
*/
inline
bool
CreateFileName_inline(
                      StructuredLightPatternType const pattern_type,
                      int const R,
                      int const G,
                      int const B,
                      double const t_delay,
                      double const t_exp,
                      wchar_t * const name_out,
                      int const size_out
                      )
{
  bool succeeded = false;

  assert(NULL != name_out);
  if (NULL == name_out) return false;

  int const size = 2048;
  wchar_t name[size + 1];

  switch ( pattern_type )
    {
    case SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY:
    case SL_PATTERN_DLP_WHEEL_SOFTWARE_DELAY:
      {
        int const count = swprintf_s(name, size, L"DLP_r%03d_g%03d_b%03d_e%06.0f_d%06.0f.png", R, G, B, 1000.0 * t_exp, 1000.0 * t_delay);
        assert(count < size);
      }
      break;

    case SL_PATTERN_RED_CHANNEL_TRANSFER:
      {
        int const count = swprintf_s(name, size, L"red_channel_%03d.png", R);
        assert(count < size);
      }
      break;

    case SL_PATTERN_GREEN_CHANNEL_TRANSFER:
      {
        int const count = swprintf_s(name, size, L"green_channel_%03d.png", G);
        assert(count < size);
      }
      break;

    case SL_PATTERN_BLUE_CHANNEL_TRANSFER:
      {
        int const count = swprintf_s(name, size, L"blue_channel_%03d.png", B);
        assert(count < size);
      }
      break;

    case SL_PATTERN_CYAN_CHANNEL_TRANSFER:
      {
        assert(G == B);
        int const count = swprintf_s(name, size, L"cyan_channel_%03d.png", G);
        assert(count < size);
      }
      break;

    case SL_PATTERN_YELLOW_CHANNEL_TRANSFER:
      {
        assert(R == G);
        int const count = swprintf_s(name, size, L"yellow_channel_%03d.png", R);
        assert(count < size);
      }
      break;

    case SL_PATTERN_MAGENTA_CHANNEL_TRANSFER:
      {
        assert(R == B);
        int const count = swprintf_s(name, size, L"magenta_channel_%03d.png", B);
        assert(count < size);
      }
      break;

    case SL_PATTERN_GRAY_CHANNEL_TRANSFER:
      {
        double const r = 0.298936021293776 * (double)( R );
        double const g = 0.587043074451121 * (double)( G );
        double const b = 0.114020904255103 * (double)( B );
        int const G = (int)( r + g  + b );

        int const count = swprintf_s(name, size, L"gray_channel_%03d.png", G);
        assert(count < size);
      }
      break;

    case SL_PATTERN_DELAY_MEASUREMENT:
      {
        int const count = swprintf_s(name, size, L"delay_r%03d_g%03d_b%03d.png", R, G, B);
        assert(count < size);
      }
      break;

    case SL_PATTERN_DELAY_MEASUREMENT_WHITE:
      {
        int const count = swprintf_s(name, size, L"all_white.png");
        assert(count < size);
      }
      break;

    case SL_PATTERN_DELAY_MEASUREMENT_BLACK:
      {
        int const count = swprintf_s(name, size, L"all_black.png");
        assert(count < size);
      }
      break;

    case SL_PATTERN_DELAY_MEASUREMENT_WHITE_TO_BLACK:
      {
        int const count = swprintf_s(name, size, L"white_to_black_transition.png");
        assert(count < size);
      }
      break;

    case SL_PATTERN_DELAY_MEASUREMENT_BLACK_TO_WHITE:
      {
        int const count = swprintf_s(name, size, L"black_to_white_transition.png");
        assert(count < size);
      }
      break;

    default:
      {
        int const count = swprintf_s(name, size, L"r%03d_g%03d_b%03d.png", R, G, B);
        assert(count < size);
      }
      break;

    }
  /* switch */

  name[size] = 0;

  int i = 0;
  for (; (i < size_out) && name[i]; ++i) name_out[i] = name[i];
  if (i < size_out)
    {
      // String is properly copied and terminated.
      name_out[i] = 0;
      succeeded = true;
    }
  else
    {
      // String is truncated so filename is not valid.
      name_out[size_out - 1] = 0;
      succeeded = false;
    }
  /* for */

  return succeeded;
}
/* CreateFileName_inline */



//! Rounds color to nearest integer.
/*!
  Converts color value from float to integer.

  \param input  Color value in [0,1] interval.
  \return Returns color value in [0,255] interval.
*/
inline
int
ConvertColor_inline(
                    float const input
                    )
{
  float const scaled = 255.0f * input;

  float clipped = scaled;
  if (0 > clipped) clipped = 0;
  if (255 < clipped) clipped = 255;  

  int const output = (int)( clipped + 0.5f );

  return output;
}
/* ConvertColor_inline */



//! Rounds color to nearest integer.
/*!
  Converts color value from float to integer.

  \param input  Color value in [0,1] interval.
  \return Returns color value in [0,255] interval.
*/
int
ConvertColor(
             float const input
             )
{
  return ConvertColor_inline(input);
}
/* ConvertColor */



//! Rewind to start.
/*!
  Rewinds list iterators to first element in the list.

  Note that this helper function does not contain
  critical section guards and is therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P      Pointer to SolidPatternList class.
*/
inline
bool
RewindToFirst_inline(
                     SolidPatternList * const P
                     )
{
  assert(NULL != P);
  if (NULL == P) return false;

  if (P->patternlist->empty()) return false;
  
  // Reset iterators.
  *(P->it) = P->patternlist->begin(); // First element.
  *(P->rit) = P->patternlist->rbegin(); // Last element.

  // Move reverse iterator to start.
  int const iend = (int)(P->patternlist->size()) - 1;
  for (int i = 0; i < iend; ++i) (*(P->rit))++;

  return true;
}
/* RewindToFirst_inline */



//! Rewind to end.
/*!
  Rewinds list iterators to last element in the list.

  Note that this helper function does not contain
  critical section guards and is therefore not thread safe.
  If used from multiple threads call must be encapsulated within critical section.

  \param P      Pointer to SolidPatternList class.
*/
inline
bool
RewindToLast_inline(
                    SolidPatternList * const P
                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  if (P->patternlist->empty()) return false;

  // Reset iterators.
  *(P->it) = P->patternlist->begin(); // First element.
  *(P->rit) = P->patternlist->rbegin(); // Last element.

  // Move iterator to end.
  int const iend = (int)(P->patternlist->size()) - 1;
  for (int i = 0; i < iend; ++i) (*(P->it))++;

  return true;
}
/* RewindToLast_inline*/



//! Returns data of current pattern.
/*!
  Returns data of current pattern.

  Note that this helper function does not contain critical section guards and is
  therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P       Pointer to list.
  \param color_out Color of current solid pattern.
  \param pattern_type_out ID of current solid pattern.
  \param t_delay_out   Present to exposure delay time.
  \param t_exp_out Exposure time.
  \param skip_acquisition_out Flag to indicate image should not be acquired.
  \param name_out  Pointer to buffer where the name will be stored. 
  \param size_out   Maximal size of the buffer.
  \return Returns true if successfull.    
*/
inline
bool
GetItemData_inline(
                   SolidPatternList * const P,
                   D3DCOLORVALUE * const color_out,
                   StructuredLightPatternType * const pattern_type_out,
                   double * const t_delay_out,
                   double * const t_exp_out,
                   bool * const skip_acquisition_out,
                   wchar_t * const name_out,
                   int const size_out
                   )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert( (NULL != color_out) ||
          (NULL != pattern_type_out) ||
          (NULL != t_delay_out) ||
          (NULL != t_exp_out) ||
          (NULL != skip_acquisition_out) ||
          (NULL != name_out)
          );
  if ( (NULL == color_out) &&
       (NULL == pattern_type_out) &&
       (NULL == skip_acquisition_out) &&
       (NULL == t_delay_out) &&
       (NULL == t_exp_out) &&
       (NULL == name_out)
       )
    {
      return false;
    }
  /* if */

  assert( ((NULL != name_out) && (0 < size_out)) || (NULL == name_out) );
  if ( (NULL != name_out) && (0 >= size_out) ) return false;

  // If there is no file then there is no filename to copy.
  if (P->patternlist->empty()) return false;
  if (P->patternlist->end() == *(P->it)) return false;
  if (P->patternlist->rend() == *(P->rit)) return false;

  bool succeeded = false;
  
  // Fetch color and ID.
  SolidPattern const data = **(P->it);
  D3DCOLORVALUE const color = data.color;
  int const R = ConvertColor_inline( color.r );
  int const G = ConvertColor_inline( color.g );
  int const B = ConvertColor_inline( color.b );
  int const A = ConvertColor_inline( color.a );
  StructuredLightPatternType const pattern_type = data.pattern_type;
  double const t_delay = data.t_delay;
  double const t_exp = data.t_exp;
  bool const skip_acquisition = data.skip_acquisition;

  // Assign color.
  if (NULL != color_out)
    {
      *color_out = color;
      succeeded = true;
    }
  /* if */

  // Assign pattern type.
  if (NULL != pattern_type_out)
    {
      *pattern_type_out = pattern_type;
      succeeded = true;
    }
  /* if */

  // Assign delay time.
  if (NULL != t_delay_out)
    {
      *t_delay_out = t_delay;
      succeeded = true;
    }
  /* if */

  // Assign exposure time.
  if (NULL != t_exp_out)
    {
      *t_exp_out = t_exp;
      succeeded = true;      
    }
  /* if */

  // Assign skip acquisition flag.
  if (NULL != skip_acquisition_out)
    {
      *skip_acquisition_out = skip_acquisition;
      succeeded = true;
    }
  /* if */

  // Copy filename to destination string.
  if (NULL != name_out)
    {
      succeeded = CreateFileName_inline(pattern_type, R, G, B, t_delay, t_exp, name_out, size_out);
    }
  /* if */

  return succeeded;
}
/* GetItemData_inline */



//! Returns data at specified index.
/*!
  Returns data of item at given index.

  Note that this helper function does not contain critical section guards and is
  therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P   Pointer to list.
  \param index  Item index.
  \param color_out Color of current solid pattern.   
  \param pattern_type_out ID of current solid pattern.
  \param t_delay_out   Present to exposure delay time (in ms).
  \param t_exp_out Exposure time (in ms).
  \param skip_acquisition_out Flag to indicate image should not be acquired.
  \param name_out  Pointer to buffer where the name will be stored. 
  \param size_out   Maximal size of the buffer.
  \return Returns true if successfull.    
*/
inline
bool
GetItemDataAt_inline(
                     SolidPatternList * const P,
                     int const index,
                     D3DCOLORVALUE * const color_out,
                     StructuredLightPatternType * const pattern_type_out,
                     double * const t_delay_out,
                     double * const t_exp_out,
                     bool * const skip_acquisition_out,
                     wchar_t * const name_out,
                     int const size_out
                     )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert( (NULL != color_out) ||
          (NULL != pattern_type_out) ||
          (NULL != t_delay_out) ||
          (NULL != t_exp_out) ||
          (NULL != skip_acquisition_out) ||
          (NULL != name_out)
          );
  if ( (NULL == color_out) &&
       (NULL == pattern_type_out) &&
       (NULL == skip_acquisition_out) &&
       (NULL == t_delay_out) &&
       (NULL == t_exp_out) &&
       (NULL == name_out)
       )
    {
      return false;
    }
  /* if */

  assert( ((NULL != name_out) && (0 < size_out)) || (NULL == name_out) );
  if ( (NULL != name_out) && (0 >= size_out) ) return false;

  // If there is no file then there is no filename to copy.
  if (P->patternlist->empty()) return false;
  if (0 > index) return false;
  if ((size_t)(index) >= P->patternlist->size()) return false;

  bool succeeded = false;

  // Move to specified position.
  solid_pattern_list_iterator it = P->patternlist->begin();
  for (int i = 0; (i < index) && (it != P->patternlist->end()); ++i) ++it;
  assert(P->patternlist->end() != it);
  if (P->patternlist->end() == it) return false;

  // Fetch color and ID.
  SolidPattern const data = *it;
  D3DCOLORVALUE const color = data.color;
  int const R = ConvertColor_inline( color.r );
  int const G = ConvertColor_inline( color.g );
  int const B = ConvertColor_inline( color.b );
  int const A = ConvertColor_inline( color.a );
  StructuredLightPatternType const pattern_type = data.pattern_type;
  double const t_delay = data.t_delay;
  double const t_exp = data.t_exp;
  bool const skip_acquisition = data.skip_acquisition;

  // Assign color.
  if (NULL != color_out)
    {
      *color_out = color;
      succeeded = true;
    }
  /* if */

  // Assign pattern type.
  if (NULL != pattern_type_out)
    {
      *pattern_type_out = pattern_type;
      succeeded = true;
    }
  /* if */

  // Assign delay time.
  if (NULL != t_delay_out)
    {
      *t_delay_out = t_delay;
      succeeded = true;
    }
  /* if */

  // Assign exposure time.
  if (NULL != t_exp_out)
    {
      *t_exp_out = t_exp;
      succeeded = true;      
    }
  /* if */

  // Assign skip acquisition flag.
  if (NULL != skip_acquisition_out)
    {
      *skip_acquisition_out = skip_acquisition;
      succeeded = true;
    }
  /* if */

  // Copy filename to destination string.
  if (NULL != name_out)
    {
      succeeded = CreateFileName_inline(pattern_type, R, G, B, t_delay, t_exp, name_out, size_out);
    }
  /* if */

  return succeeded;
}
/* GetItemDataAt_inline */



/****** SOLID PATTERN LIST ******/

//! Blanks SolidPatternList structure.
/*!
  Blanks SolidPatternList structure.
*/
void SolidPatternList::Blank(
                             void
                             )
{
  this->patternlist = NULL;
  this->it = NULL;
  this->rit = NULL;
  ZeroMemory( &(this->csPatternList), sizeof(this->csPatternList) );
  this->cycle = true;
}
/* SolidPatternList::Blank */



//! Initializes SolidPatternList structure.
/*!
  Initalizes SolidPatternList structure.
*/
void
SolidPatternList::Initialize(
                             void
                             )
{
  this->Blank();

  this->patternlist = new solid_pattern_list;
  this->it = new solid_pattern_list_iterator;
  this->rit = new solid_pattern_list_reverse_iterator;

  InitializeCriticalSection( &(this->csPatternList) );

  assert(NULL != this->patternlist);
  assert(NULL != this->it);
  assert(NULL != this->rit);
}
/* SolidPatternList::Initialize */



//! Releases allocated resources.
/*!
  Releases allocated resources.
*/
void
SolidPatternList::Release(
                          void
                          )
{
  EnterCriticalSection( &(this->csPatternList) );
  {
    SAFE_DELETE(this->patternlist);
    SAFE_DELETE(this->it);
    SAFE_DELETE(this->rit);
  }
  LeaveCriticalSection( &(this->csPatternList) );

  DeleteCriticalSection( &(this->csPatternList) );

  this->Blank();
}
/* SolidPatternList::Release */



//! Steps to next image.
/*!
  Steps to next image. If cycle flag is set and current image
  is last image then the list rewinds to begining.

  \return Returns true if successfull.
*/
bool
SolidPatternList::Next(
                       void
                       )
{
  bool succeeded = true;

  EnterCriticalSection( &(this->csPatternList) );

  if (this->patternlist->empty())
    {
      // List is empty.
      succeeded = false;
    }
  else if ( (false == this->cycle) && (this->patternlist->end() == *(this->it)) )
    {
      // We are at the end of the list and cycling is prohibited.
      succeeded = false;
    }
  else
    {
      // Step to next item.
      if (this->patternlist->end() != *(this->it)) ++(*(this->it));
      if (this->patternlist->rbegin() != *(this->rit)) --(*(this->rit));

      // Rewind if needed.
      if ( (true == this->cycle) && (this->patternlist->end() == *(this->it)) )
        {
          succeeded = RewindToFirst_inline( this );
          assert(true == succeeded);
        }
      /* if */
    }
  /* if */

  LeaveCriticalSection( &(this->csPatternList) );

  return succeeded;
}
/* SolidPatternList::Next */



//! Steps to previous image.
/*!
  Steps to previous image. If cycle flag is set and current image is first
  image in the list then list rewinds to the end.

  \return Returns true if successfull.
*/
bool
SolidPatternList::Prev(
                       void
                       )
{
  bool succeeded = true;

  EnterCriticalSection( &(this->csPatternList) );

  if (this->patternlist->empty())
    {
      // List is empty.
      succeeded = false;
    }
  else if ( (false == this->cycle) && (this->patternlist->rend() == *(this->rit)) )
    {
      // We are at the start of the list and cycling is prohibited.
      succeeded = false;
    }
  else
    {
      // Step to previous item.
      if (this->patternlist->begin() != *(this->it)) --(*(this->it));
      if (this->patternlist->rend() != *(this->rit)) ++(*(this->rit));

      // Rewind if needed.
      if ( (true == this->cycle) && (this->patternlist->rend() == *(this->rit)) )
        {
          succeeded = RewindToLast_inline( this );
          assert(true == succeeded);
        }
      /* if */
    }
  /* if */

  LeaveCriticalSection( &(this->csPatternList) );

  return succeeded;
}
/* SolidPatternList::Prev */



//! Returns color of current solid pattern.
/*!
  Returns color of current solid pattern.

  \param color_out    Pointer to color structure.
  \return Returns true if successfull.  
*/
bool
SolidPatternList::GetColor(
                           D3DCOLORVALUE * const color_out
                           )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, color_out, NULL, NULL, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetColor */



//! Returns pattern ID of current solid pattern.
/*!
  Returns ID of current solid pattern.

  \param pattern_type_out    Pointer where ID will be stored.
  \return Returns true if successfull.  
*/
bool
SolidPatternList::GetID(
                        StructuredLightPatternType * const pattern_type_out
                        )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, NULL, pattern_type_out, NULL, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetID */



//! Returns delay time of current solid pattern.
/*!
  Returns delay time in milliseconds of current solid pattern.

  \param t_delay_out    Pointer where delay time in milliseconds will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetDelay(
                           double * const t_delay_out
                           )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, NULL, NULL, t_delay_out, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetDelay */



//! Returns exposure time of current solid pattern.
/*!
  Returns exposure time in milliseconds of current solid pattern.

  \param t_exp_out    Pointer where exposure time in milliseconds will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetExposure(
                              double * const t_exp_out
                              )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, NULL, NULL, NULL, t_exp_out, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetExposure */


//! Returns skip acquisition flag.
/*!
  Returns skip acquisition flag.

  \param skip_acquisition_out Pointer where skip acquisition flag will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetSkipAcquisition(
                                     bool * const skip_acquisition_out
                                     )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, NULL, NULL, NULL, NULL, skip_acquisition_out, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetSkipAcquisition */



//! Returns filename that uniquely describes the pattern.
/*!
  Returns filename that uniquely describes the pattern.

  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetFileName(
                              wchar_t * const name_out,
                              int const size_out
                              )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemData_inline(this, NULL, NULL, NULL, NULL, NULL, name_out, size_out);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetFileName */



//! Returns image index.
/*!
  Returns image index.

  \return Image index or -1 if there is no current image.
*/
int
SolidPatternList::GetImageIndex(
                                void
                                )
{
  int index = -1;

  EnterCriticalSection( &(this->csPatternList) );

  if ( (this->patternlist->empty() == false) &&
       (this->patternlist->end() != *(this->it)) &&
       (this->patternlist->rend() != *(this->rit))
       )
    {
      index = (int)std::distance(this->patternlist->begin(), *(this->it));
    }
  /* if */

  LeaveCriticalSection( &(this->csPatternList) );
  
  return index;
}
/* SolidPatternList::GetImageIndex */



//! Returns color at specified list index.
/*!
  Returns color of solid pattern at specified list index.

  \param index Image index.
  \param color_out    Pointer to color structure.
  \return Returns true if successfull.  
*/
bool
SolidPatternList::GetColorAt(
                             int const index,
                             D3DCOLORVALUE * const color_out
                             )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, color_out, NULL, NULL, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetColorAt */



//! Returns pattern ID at specified list index.
/*!
  Returns ID of pattern at specified list index.

  \param index Image index.
  \param pattern_type_out    Pointer where ID will be stored.
  \return Returns true if successfull.  
*/
bool
SolidPatternList::GetIDAt(
                          int const index,
                          StructuredLightPatternType * const pattern_type_out
                          )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, NULL, pattern_type_out, NULL, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetIDAt */



//! Returns delay time at specified list index.
/*!
  Returns delay time in milliseconds at specified list index.

  \param index Image index.
  \param t_delay_out    Pointer where delay time in milliseconds will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetDelayAt(
                             int const index,
                             double * const t_delay_out
                             )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, NULL, NULL, t_delay_out, NULL, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetDelayAt */



//! Returns exposure time at specified list index.
/*!
  Returns exposure time in milliseconds at specified list index.

  \param index Image index.
  \param t_exp_out    Pointer where exposure time in milliseconds will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetExposureAt(
                                int const index,
                                double * const t_exp_out
                                )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, NULL, NULL, NULL, t_exp_out, NULL, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetExposureAt */



//! Returns skip acquisition flag.
/*!
  Returns skip acquisition flag.

  \param index Image index.
  \param skip_acquisition_out Pointer where skip acquisition flag will be stored.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetSkipAcquisitionAt(
                                       int const index,
                                       bool * const skip_acquisition_out
                                       )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, NULL, NULL, NULL, NULL, skip_acquisition_out, NULL, 0);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetSkipAcquisitionAt */



//! Returns filename at specified list index.
/*!
  Returns the filename of image at specified index (no path).

  \param index Image index.
  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GetFileNameAt(
                                int const index,
                                wchar_t * const name_out,
                                int const size_out
                                )
{
  EnterCriticalSection( &(this->csPatternList) );
  bool const succeeded = GetItemDataAt_inline(this, index, NULL, NULL, NULL, NULL, NULL, name_out, size_out);
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::GetFileNameAt */



//! Rewinds list to start.
/*!
  Rewinds list to start.

  \return Returns true if successfull.
*/
bool
SolidPatternList::Rewind(
                         void
                         )
{
  EnterCriticalSection( &(this->csPatternList) );  
  bool const succeeded = RewindToFirst_inline( this );
  LeaveCriticalSection( &(this->csPatternList) );
  return succeeded;
}
/* SolidPatternList::Rewind */



//! Size.
/*!
  Returns patternlist size.

  \return Number of items in the list.
*/
int
SolidPatternList::Size(
                       void
                       )
{
  int size = 0;
  EnterCriticalSection( &(this->csPatternList) );
  size = (int)(this->patternlist->size());
  LeaveCriticalSection( &(this->csPatternList) );
  return size;
}
/* SolidPatternList::Size */



//! At end.
/*!
  Tests if we have reached the end of the list.

  \return Returns true if iterator is at the end of the list.
*/
bool
SolidPatternList::AtEnd(
                        void
                        )
{
  EnterCriticalSection( &(this->csPatternList) );  
  bool const atend = this->patternlist->end() == *(this->it);
  LeaveCriticalSection( &(this->csPatternList) );
  return atend;
}
/* SolidPatternList::AtEnd */



/****** SOLID PATTERN GENERATORS ******/


//! Generate pattern for DLP color wheel analysis.
/*!
  Creates solid color pattern for DLP color wheel analysis.
  DLP color wheel usually contains several segments that will be illuminated
  depending on the projected color. This function generates 2N test images
  of the same color that may be used to record the illumination pattern.

  \param N Number of slices per one wheel rotation.
  \param t_exp Duration of one full wheel rotation (in ms).
  \param red Intensity value of the red channel. Must be in [0,1] interval.
  \param green Intensity value of the green channel. Must be in [0,1] interval.
  \param blue Intensity value of the blue channel. Must be in [0,1] interval.
  \param hardware_delay If true then hardware delay is used if supported by the camera, otherwise software delay is used.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GenerateDLPWheelPattern(
                                          int const N,
                                          double const t_exp,
                                          float const red,
                                          float const green,
                                          float const blue,
                                          bool const hardware_delay
                                          )
{
  bool initialized = false;

  assert( (0 < N) && (0 < t_exp) );
  if ( (0 >= N) || (0 >= t_exp) ) return initialized;

  solid_pattern_list * pList = new solid_pattern_list;
  assert(NULL != pList);
  if (NULL != pList)
    {
      while ( ! pList->empty() ) pList->clear();

      SolidPattern pattern;
      pattern.color.r = red;
      pattern.color.g = green;
      pattern.color.b = blue;
      pattern.color.a = 1.0f;
      pattern.pattern_type = (true == hardware_delay)? SL_PATTERN_DLP_WHEEL_HARDWARE_DELAY : SL_PATTERN_DLP_WHEEL_SOFTWARE_DELAY;
      pattern.t_delay = 0.0;
      pattern.t_exp = 0.0;
      pattern.skip_acquisition = false;

      // Compute delta as uniform slice of one wheel rotation. However, as the
      // delay timers are not entierly precise an extension may be added to exposure
      // time so the recorded time slices overlap (so there are no holes or missing
      // data when one wheel rotation is visualized).
      double const t_delta = t_exp / (double)(N);
      pattern.t_exp = t_delta;

      // Add images for two wheel rotations. Note that normally delay time increases
      // starting from 0, however, for testing pruposes delay time may decrease 
      // starting from the maximum toward zero.
      int const m = 2;
      pattern.t_delay = 0; // Increasing delays.
      //pattern.t_delay = m * t_exp; // Decreasing delay.
      for (int i = 0; i < m * N; ++i)
        {
          if (0 > pattern.t_delay) pattern.t_delay = 0;
          pList->push_back( pattern );

          pattern.t_delay += t_delta; // Increasing delays.
          //pattern.t_delay -= t_delta; // Decreasing delays.
        }
      /* for */

      /* Change list to new one. */
      EnterCriticalSection( &(this->csPatternList) );
      {
        // Set pattern list.
        solid_pattern_list * tmpList = this->patternlist;
        this->patternlist = pList;
        SAFE_DELETE(tmpList);

        /* Update list iterators. */
        bool const rewind = RewindToFirst_inline(this);
        assert(true == rewind);
      }
      LeaveCriticalSection( &(this->csPatternList) );

      initialized = true;
    }
  /* if */

  return initialized;
}
/* SolidPatternList::GenerateDLPWheelPattern */




//! Generate channel transfer pattern.
/*!
  Creates solid patterns for measuring channel transfer functions.

  \param NRed   Number of test images for red channel.
  \param NGreen Number of test images for green channel.
  \param NBlue  Number of test images for blue channel.
  \param NCyan  Number of test images for cyan channel.
  \param NYellow  Number of test images for yellow channel.
  \param NMagenta  Number of test images for magenta channel.
  \param NGray  Number of test images for gray channel.
  \param t_exp Duration of one full wheel rotation (in ms).
  \return Returns true if successfull, false otherwise.
*/
bool
SolidPatternList::GenerateChannelTransferPattern(
                                                 int const NRed,
                                                 int const NGreen,
                                                 int const NBlue,
                                                 int const NCyan,
                                                 int const NYellow,
                                                 int const NMagenta,
                                                 int const NGray,
                                                 double const t_exp
                                                 )
{
  bool initialized = false;

  solid_pattern_list * pList = new solid_pattern_list;
  assert(NULL != pList);
  if (NULL != pList)
    {
      while ( ! pList->empty() ) pList->clear();

      SolidPattern pattern;
      pattern.color.r = 0.0f;
      pattern.color.g = 0.0f;
      pattern.color.b = 0.0f;
      pattern.color.a = 1.0f;
      pattern.t_delay = 0.0;
      pattern.t_exp = t_exp;
      pattern.skip_acquisition = false;

      // Add blue images.
      float const blue_delta = 1.0f / (float)(NBlue);
      pattern.pattern_type = SL_PATTERN_BLUE_CHANNEL_TRANSFER;
      pattern.color.b = 1.0f;
      for (int i = 0; i < NBlue; ++i)
        {
          assert( 0.0f < pattern.color.b );
          pList->push_back( pattern );
          pattern.color.b -= blue_delta;
        }
      /* for */
      pattern.color.b = 0.0f;

      // Add green images.
      float const green_delta = 1.0f / (float)(NGreen);
      pattern.pattern_type = SL_PATTERN_GREEN_CHANNEL_TRANSFER;
      pattern.color.g = 1.0f;
      for (int i = 0; i < NGreen; ++i)
        {
          assert( 0.0f < pattern.color.g );
          pList->push_back( pattern );
          pattern.color.g -= green_delta;
        }
      /* for */
      pattern.color.g = 0.0f;

      // Add red images.
      float const red_delta = 1.0f / (float)(NRed);
      pattern.pattern_type = SL_PATTERN_RED_CHANNEL_TRANSFER;
      pattern.color.r = 1.0f;
      for (int i = 0; i < NRed; ++i)
        {
          assert( 0.0f < pattern.color.r );
          pList->push_back( pattern );
          pattern.color.r -= red_delta;
        }
      /* for */
      pattern.color.r = 0.0f;

      // Add cyan images.
      float const cyan_delta = 1.0f / (float)(NCyan);
      pattern.pattern_type = SL_PATTERN_CYAN_CHANNEL_TRANSFER;
      pattern.color.g = 1.0f;
      pattern.color.b = 1.0f;
      for (int i = 0; i < NCyan; ++i)
        {
          assert( 0.0f < pattern.color.g );
          assert( 0.0f < pattern.color.b );
          pList->push_back( pattern );
          pattern.color.g -= cyan_delta;
          pattern.color.b -= cyan_delta;
        }
      /* for */
      pattern.color.g = 0.0f;
      pattern.color.b = 0.0f;

      // Add yellow images.
      float const yellow_delta = 1.0f / (float)(NYellow);
      pattern.pattern_type = SL_PATTERN_YELLOW_CHANNEL_TRANSFER;
      pattern.color.r = 1.0f;
      pattern.color.g = 1.0f;
      for (int i = 0; i < NYellow; ++i)
        {
          assert( 0.0f < pattern.color.r );
          assert( 0.0f < pattern.color.g );
          pList->push_back( pattern );
          pattern.color.r -= yellow_delta;
          pattern.color.g -= yellow_delta;
        }
      /* for */
      pattern.color.r = 0.0f;
      pattern.color.g = 0.0f;

      // Add magenta images.
      float const magenta_delta = 1.0f / (float)(NMagenta);
      pattern.pattern_type = SL_PATTERN_MAGENTA_CHANNEL_TRANSFER;
      pattern.color.r = 1.0f;
      pattern.color.b = 1.0f;
      for (int i = 0; i < NMagenta; ++i)
        {
          assert( 0.0f < pattern.color.r );
          assert( 0.0f < pattern.color.b );
          pList->push_back( pattern );
          pattern.color.r -= magenta_delta;
          pattern.color.b -= magenta_delta;
        }
      /* for */
      pattern.color.r = 0.0f;
      pattern.color.b = 0.0f;

      // Add gray images.
      float const gray_delta = 1.0f / (float)(NGray);
      pattern.pattern_type = SL_PATTERN_GRAY_CHANNEL_TRANSFER;
      pattern.color.b = 1.0f;
      pattern.color.g = 1.0f;
      pattern.color.r = 1.0f;
      for (int i = 0; i < NGray; ++i)
        {
          assert( 0.0f < pattern.color.b );
          assert( 0.0f < pattern.color.g );
          assert( 0.0f < pattern.color.r );
          pList->push_back( pattern );
          pattern.color.r -= gray_delta;
          pattern.color.g -= gray_delta;
          pattern.color.b -= gray_delta;
        }
      /* for */

      /* Change list to new one. */
      EnterCriticalSection( &(this->csPatternList) );
      {
        // Set pattern list.
        solid_pattern_list * tmpList = this->patternlist;
        this->patternlist = pList;
        SAFE_DELETE(tmpList);

        /* Update list iterators. */
        bool const rewind = RewindToFirst_inline(this);
        assert(true == rewind);
      }
      LeaveCriticalSection( &(this->csPatternList) );

      initialized = true;
    }
  /* if */

  return initialized;
}
/* SolidPatternList::GenerateChannelTransferPattern */



//! Generate pattern for delay measurement.
/*!
  Creates sequence of black and white images for delay time measurement.
  The pattern is comprised of N+k white images, then N+k black images,
  then N black images followed by k white images.

  \param t_vblank Duration of one VBLANK interval (in ms).
  \param k Exposure time mulitplier; camera exposure time is t_vblank times k.
  \param N Number of consecutive images required to stabilize projector output.
  \return Returns true if successfull.
*/
bool
SolidPatternList::GenerateDelayMeasurementPattern(
                                                  double const t_vblank,
                                                  double const k,
                                                  double const N
                                                  )
{
  bool initialized = false;

  assert(0 < t_vblank);
  if (0 >= t_vblank) return initialized;

  assert(0 < k);
  if (0 >= k) return initialized;

  assert(0 < N);
  if (0 >= N) return initialized;


  solid_pattern_list * pList = new solid_pattern_list;
  assert(NULL != pList);
  if (NULL != pList)
    {
      while ( ! pList->empty() ) pList->clear();

      SolidPattern pattern;
      pattern.color.a = 1.0f;
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT;
      pattern.t_delay = 0.0;
      pattern.t_exp = k * t_vblank;
      pattern.skip_acquisition = true;

      int const iN = (int)(N + 0.5);
      int const iNk = (int)(N + k + 0.5);

      // Add white.
      pattern.color.r = 1.0f;
      pattern.color.g = 1.0f;
      pattern.color.b = 1.0f;
      for (int i = 0; i <= iN; ++i) pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT_WHITE;
      pattern.skip_acquisition = false;
      pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT;
      pattern.skip_acquisition = true;
      for (int i = 0; i <= iNk; ++i) pList->push_back( pattern );

      // Add white-to-black transition.
      for (int i = 0; i <= iN; ++i) pList->push_back( pattern );
      pattern.color.r = 0.0f;
      pattern.color.g = 0.0f;
      pattern.color.b = 0.0f;
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT_WHITE_TO_BLACK;
      pattern.skip_acquisition = false;
      pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT;
      pattern.skip_acquisition = true;
      for (int i = 0; i <= iNk; ++i) pList->push_back( pattern );

      // Add black.
      for (int i = 0; i <= iN; ++i) pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT_BLACK;
      pattern.skip_acquisition = false;
      pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT;
      pattern.skip_acquisition = true;
      for (int i = 0; i <= iNk; ++i) pList->push_back( pattern );

      // Add black-to-white transition.
      for (int i = 0; i <= iN; ++i) pList->push_back( pattern );
      pattern.color.r = 1.0f;
      pattern.color.g = 1.0f;
      pattern.color.b = 1.0f;
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT_BLACK_TO_WHITE;
      pattern.skip_acquisition = false;
      pList->push_back( pattern );
      pattern.pattern_type = SL_PATTERN_DELAY_MEASUREMENT;
      pattern.skip_acquisition = true;
      for (int i = 0; i <= iNk; ++i) pList->push_back( pattern );

      /* Change list to new one. */
      EnterCriticalSection( &(this->csPatternList) );
      {
        // Set pattern list.
        solid_pattern_list * tmpList = this->patternlist;
        this->patternlist = pList;
        SAFE_DELETE(tmpList);

        /* Update list iterators. */
        bool const rewind = RewindToFirst_inline(this);
        assert(true == rewind);
      }
      LeaveCriticalSection( &(this->csPatternList) );

      initialized = true;
    }
  /* if */

  return initialized;
}
/* SolidPatternList::GenerateDelayMeasurementPattern */



/****** RENDERING FROM IMAGE PARAMETERS ******/


//! Renders solid structured light pattern.
/*!
  Paints entire render target in solid color.

  \param pRenderTarget  Pointer to ID2D1RenderTarget.
  \param red    Red channel value.
  \param green  Green channel value.
  \param blue   Blue channel value.
  \param alpha  Opacity.
  \return Returns S_OK if successfull.
*/
HRESULT
RenderSolidPattern(
                   float const red,
                   float const green,
                   float const blue,
                   float const alpha,
                   ID2D1RenderTarget * const pRenderTarget
                   )
{
  assert(NULL != pRenderTarget);
  if (NULL == pRenderTarget) return E_INVALIDARG;

  ID2D1SolidColorBrush * pBrush = NULL;

  HRESULT hr = S_OK;

  // Create solid color brush.
  if (SUCCEEDED(hr))
    {
      hr = pRenderTarget->CreateSolidColorBrush(
                                                D2D1::ColorF(red, green, blue, alpha),
                                                &pBrush
                                                );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      D2D1_SIZE_F const renderTargetSize = pRenderTarget->GetSize();
      D2D1_RECT_F destinationRectangle = D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height);

      pRenderTarget->BeginDraw();
      
      pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
      pRenderTarget->FillRectangle(destinationRectangle, pBrush);

      hr = pRenderTarget->EndDraw();
      assert( SUCCEEDED(hr) );
    }
  /* if */

  SAFE_RELEASE(pBrush);

  return hr;
}
/* RenderSolidPattern */



#endif /* !__BATCHACQUISITIONPATTERNSOLID_CPP */
