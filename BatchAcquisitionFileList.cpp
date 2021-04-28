/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2014-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionFileList.cpp
  \brief  File list for selected directory.

  Functions to create a file list of all image files in a specified directory.

  \author Tomislav Petkovic
  \date   2017-02-22
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONFILELIST_CPP
#define __BATCHACQUISITIONFILELIST_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionFileList.h"
#include "BatchAcquisitionDialogs.h"


#pragma warning(push)
#pragma warning(disable: 4005)

#include <shlwapi.h>

#pragma warning(pop)

#pragma comment(lib, "Shlwapi.lib")



/****** HELPER FUNCTIONS ******/

//! Checks filename extension.
/*!
  Function checks filename extension. Letter case is ignored.

  \param fname  Filename.
  \param ext    Extension.
  \return Returns true if filename has the supplied extension.
*/
inline
bool
CheckExtension_inline(
                      wchar_t const * const fname,
                      wchar_t const * const ext
                      )
{
  assert(NULL != fname);
  assert(NULL != ext);
  if ( (NULL == fname) || (NULL == ext) ) return( false );

  bool match = false;

  int i = 0;
  int j = 0;
  while (0 != fname[i]) ++i;
  while (0 != ext[j]) ++j;

  int k = i - j;
  int l = 0;
  for (; (0 <= k) && (k < i) && (l < j); ++k, ++l)
    {
      if (towupper(fname[k]) != towupper(ext[l])) break;
    }
  /* for */

  if ( (k == i) && (l == j) ) match = true;

  return( match );
}
/* CheckExtension_inline */



//! Compares two strings based on last number in the string.
/*!
  Function compares two strings based on last number stored in the string. This
  is achieved by scanning the string backwards and accumulating digits from the
  first token of digits.

  \param first  First string to compare.
  \param second Second string to compare.
  \return Function returns true if the number stored in first string is less
  than number stored in the second string.
*/
inline
bool
CompareFileNameByLastNumber_inline(
                                   std::wstring first,
                                   std::wstring second
                                   )
{
  int i;

  /* We scan each string from the last character foreward. */

  size_t const firstlength = first.length();
  int firstnum = 0;

  /* Find last number in the first string. */
  for (i = (int)(firstlength) - 1; i >= 0; --i)
    {
      if ( ('0' <= first[i]) && (first[i] <= '9') ) break;
    }
  /* for */

  /* Convert to int until we read digits. */
  for (int mul = 1; i >= 0; --i)
    {
      if ( ('0' <= first[i]) && (first[i] <= '9') )
        {
          firstnum += (first[i] - '0') * mul;
          mul *= 10;
        }
      else
        {
          break;
        }
      /* if */
    }
  /* for */

  if ( (0 <= i) && ('-' == first[i]) ) firstnum = -firstnum;


  size_t const secondlength = second.length();
  int secondnum = 0;

  /* Find last number in the second string. */
  for (i = (int)(secondlength) - 1; i >= 0; --i)
    {
      if ( ('0' <= second[i]) && (second[i] <= '9') ) break;
    }
  /* for */

  /* Convert to int until we read digits. */
  for (int mul = 1; i >= 0; --i)
    {
      if ( ('0' <= second[i]) && (second[i] <= '9') )
        {
          secondnum += (second[i] - '0') * mul;
          mul *= 10;
        }
      else
        {
          break;
        }
      /* if */
    }
  /* for */

  if ( (0 <= i) && ('-' == second[i]) ) secondnum = -secondnum;

  return( firstnum < secondnum );
}
/* CompareFileNameByLastNumber_inline */



//! Compares two file patterns by filename.
/*!
  Function comapres two file patterns by stored filename.

  \param first  First filename.
  \param second Second filename.
  \return Function returns true if the first filename is before the second filename.
*/
inline
bool
CompareFilePattern_inline(
                          const FilePattern_ & first,
                          const FilePattern_ & second
                          )
{
  if ( (NULL != first.filename) && (NULL != second.filename) )
    {
      int const compare = first.filename->compare( *(second.filename) );
      return compare < 0;
    }
  else if ( (NULL == first.filename) && (NULL != second.filename) )
    {
      return false;
    }
  else if ( (NULL != first.filename) && (NULL == second.filename) )
    {
      return true;
    }
  else
    {
      return false;
    }
  /* if */
}
/* CompareFilePatternByLastNumber_inline */



//! Compares two filenames based on last number in the filename.
/*!
  Compares two filenames based on last number in the filename.
  
  \param first  First filename.
  \param second Second filename.
  \return Function returns true if the number in the first filename is less than
  the number in the second filename.
*/
inline
bool
CompareFilePatternByLastNumber_inline(
                                      FilePattern_ first,
                                      FilePattern_ second
                                      )
{
  if ( (NULL != first.filename) && (NULL != second.filename) )
    {
      return CompareFileNameByLastNumber_inline(*(first.filename), *(second.filename));
    }
  else if ( (NULL == first.filename) && (NULL != second.filename) )
    {
      return false;
    }
  else if ( (NULL != first.filename) && (NULL == second.filename) )
    {
      return true;
    }
  else
    {
      return true;
    }
  /* if */
}
/* CompareFilePatternByLastNumber_inline */



//! Test if pattern is black SL pattern.
/*!
  Function tests if a particular file pattern does not have filename defined.

  \param P      Reference to file pattern.
  \return Returns true if file pattern is black SL pattern.
*/
inline
bool
IsFilePatternBlackSLPattern_inline(
                                   const FilePattern_ & P
                                   )
{
  return (NULL == P.filename);
}
/* IsFilePatternBlackSLPattern_inline */



//! Rewind to start.
/*!
  Rewinds file list to first element in the list.

  Note that this helper function does not contain
  critical section guards and is therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P      Pointer to ImageFileList class.
*/
inline
bool
RewindToFirst_inline(
                     ImageFileList * const P
                     )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(true == P->initialized);
  if (false == P->initialized) return false;

  // Reset iterators.
  *(P->it) = P->filelist->begin(); // First element.
  *(P->rit) = P->filelist->rbegin(); // Last element.

  // Move reverse iterator to start.
  int const iend = (int)(P->filelist->size()) - 1;
  for (int i = 0; i < iend; ++i) (*(P->rit))++;

  return true;
}
/* RewindToFirst_inline */



//! Rewind to end.
/*!
  Rewinds file list to last element in the list.

  Note that this helper function does not contain
  critical section guards and is therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P      Pointer to ImageFileList class.
*/
inline
bool
RewindToLast_inline(
                    ImageFileList * const P
                    )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert(true == P->initialized);
  if (false == P->initialized) return false;

  if (P->filelist->empty()) return false;

  // Reset iterators.
  *(P->it) = P->filelist->begin(); // First element.
  *(P->rit) = P->filelist->rbegin(); // Last element.

  // Move iterator to end.
  int const iend = (int)(P->filelist->size()) - 1;
  for (int i = 0; i < iend; ++i) (*(P->it))++;

  return true;
}
/* RewindToLast_inline */



//! Returns full filename of current image.
/*!
  Returns full filename of current image (including directory name) using absolute path.

  Note that this helper function does not contain critical section guards and is
  therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P          Pointer to file list.
  \param have_filename_out  Address where flag indicating if current item has a filename is stored.
  \param name_out  Pointer to buffer where the name will be stored.
  \param size_out   Maximal size of the buffer.
  \param include_directory      If true then name_out includes directory name.
  \param include_filename       If true thne name_out includes filename.
  \return Returns true if successfull.
*/
inline
bool
GetName_inline(
               ImageFileList * const P,
               bool * const have_filename_out,
               wchar_t * const name_out,
               int const size_out,
               bool const include_directory,
               bool const include_filename
               )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert( (NULL != have_filename_out) || (NULL != name_out) );
  if ( (NULL == have_filename_out) && (NULL == name_out) ) return false;

  if ( NULL != name_out )
    {
      assert(0 < size_out);
      if (0 >= size_out) return false;
    }
  /* if */

  assert(true == P->initialized);

  // If there is no file then there is no filename to copy.
  if (P->filelist->empty()) return false;
  if (P->filelist->end() == *(P->it)) return false;
  if (P->filelist->rend() == *(P->rit)) return false;

  bool succeeded = true;

  // Test if current item has a filename.
  bool const have_filename = (*P->it)->HaveFilename();
  if (NULL != have_filename_out) *have_filename_out = have_filename;

  if (NULL == name_out) return succeeded;

  // Assemble requested name for the current item.
  std::wstring pName = std::wstring(_T(""));
  if (true == include_directory) pName += *(P->directory_name) + std::wstring(_T("\\"));
  if (true == include_filename)
    {
      if (true == have_filename)
        {
          pName += std::wstring( (*(P->it))->GetFilename() );
        }
      else
        {
          int const size = 64;
          wchar_t filename[size + 1];

          int const index = (int)std::distance(P->filelist->begin(), *(P->it));

          int const count = swprintf_s(filename, size, L"black_frame_%05d.png", index + 1);
          assert(count < size);
          filename[size] = 0;

          pName += std::wstring(filename);
        }
      /* if */
    }
  /* if */

  // Copy filename to destination string.
  wchar_t const * name = pName.c_str();
  int i = 0;
  for (; (i < size_out) && name[i]; ++i) name_out[i] = name[i];
  if (i < size_out)
    {
      // String is properly copied and terminated.
      name_out[i] = 0;
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
/* GetName_inline */



//! Returns full filename at specified index.
/*!
  Returns full filename of item at given index (including directory name) using absolute path.

  Note that this helper function does not contain critical section guards and is
  therefore not thread safe. If used from multiple
  threads call must be encapsulated within critical section.

  \param P       Pointer to file list.
  \param index  Item index.
  \param have_filename_out  Address where flag indicating if current item has a filename is stored.
  \param name_out  Pointer to buffer where the name will be stored.
  \param size_out   Maximal size of the buffer.
  \param include_directory      If true name includes directory name.
  \param include_filename       If true name includes filename.
  \return Returns true if successfull.
*/
inline
bool
GetNameAt_inline(
                 ImageFileList * const P,
                 int const index,
                 bool * const have_filename_out,
                 wchar_t * const name_out,
                 int const size_out,
                 bool const include_directory,
                 bool const include_filename
                 )
{
  assert(NULL != P);
  if (NULL == P) return false;

  assert( (NULL != have_filename_out) || (NULL != name_out) );
  if ( (NULL == have_filename_out) && (NULL == name_out) ) return false;

  if ( NULL != name_out )
    {
      assert(0 < size_out);
      if (0 >= size_out) return false;
    }
  /* if */

  assert(true == P->initialized);

  // If there is no file then there is no filename to copy.
  if (P->filelist->empty()) return false;
  if (0 > index) return false;
  if ((size_t)(index) >= P->filelist->size()) return false;

  bool succeeded = true;

  // Move to specified position.
  sorted_file_list_iterator it = P->filelist->begin();
  for (int i = 0; (i < index) && (it != P->filelist->end()); ++i) ++it;
  assert(P->filelist->end() != it);
  if (P->filelist->end() == it) return false;

  // Test if current item has a filename.
  bool const have_filename = it->HaveFilename();
  if (NULL != have_filename_out) *have_filename_out = have_filename;  

  if (NULL == name_out) return succeeded;

  // Assemble requested name for the current item.
  std::wstring pName = std::wstring(_T(""));
  if (true == include_directory) pName += *(P->directory_name) + std::wstring(_T("\\"));
  if (true == include_filename)
    {
      if (true == have_filename)
        {
          pName += std::wstring( it->GetFilename() );
        }
      else
        {
          int const size = 64;
          wchar_t filename[size + 1];

          int const index = (int)std::distance(P->filelist->begin(), *(P->it));
          
          int const count = swprintf_s(filename, size, L"black_frame_%05d.png", index + 1);
          assert(count < size);
          filename[size] = 0;

          pName += std::wstring(filename);
        }
      /* if */
    }
  /* if */

  // Copy filename to destination string.
  wchar_t const * name = pName.c_str();
  int i = 0;
  for (; (i < size_out) && name[i]; ++i) name_out[i] = name[i];
  if (i < size_out)
    {
      // String is properly copied and terminated.
      name_out[i] = 0;
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
/* GetNameAt_inline */



/****** MEMBER FUNCTIONS FOR SL PATTERN ******/

//! Default constructor.
/*!
  Sets image filename to NULL.
*/
FilePattern_::FilePattern_()
{
  this->filename = NULL;
}
/* FilePattern_::FilePattern_ */


//! From string constructor.
//!
/*!
  Sets filename to given string.

  \param name   Reference to filename.
*/
FilePattern_::FilePattern_(
                           const std::wstring & name
                           )
{
  this->filename = new std::wstring(name);
}
/* FilePattern_::FilePattern_ */



//! From string constructor.
//!
/*!
  Sets filename to given string.

  \param name   Reference to filename.
*/
FilePattern_::FilePattern_(
                           wchar_t * const name
                           )
{
  this->filename = NULL;
  if (NULL != name) this->filename = new std::wstring(name);
}
/* FilePattern_::FilePattern_ */



//! Copy constructor.
/*!
  Creates a copy of filename if it exists.

  \param P      Reference of object to copy.
*/
FilePattern_::FilePattern_(
                           const FilePattern_ & P
                           )
{
  this->filename = NULL;
  if (NULL != P.filename) this->filename = new std::wstring( *(P.filename) );
}
/* FilePattern_::FilePattern_ */



//! Default destructor.
/*!
  Deletes created filename.
*/
FilePattern_::~FilePattern_()
{
  SAFE_DELETE(this->filename);
}
/* FilePattern_::~FilePattern_ */



//! Default weak comparison operator.
/*!
  Compares two SL patterns.

  \param R      Right operand.
  \return Returns true if L is less than R.
*/
bool
FilePattern_::operator < (
                          const FilePattern_ & R
                          )
{
  return CompareFilePattern_inline(*this, R);
}
/* FilePattern_::operator < */



//! Test if we have filename.
/*!
  Tests if filename is defined.

  \return Returns true if filename exists.
*/
bool
FilePattern_::HaveFilename(
                           void
                           )
{
  return( NULL != this->filename );
}
/* FilePattern_::HaveFilename */



//! Get filename.
/*!
  Returns filename string or throws an exception if no filename exists.

  \return Reference to filename.
*/
const std::wstring &
FilePattern_::GetFilename(
            void
            )
{
  if (NULL == this->filename) throw std::exception();
  return *(this->filename);
}
/* FilePattern_::GetFilename */



/****** MEMBER FUNCTIONS FOR FILE LIST  ******/

//! Constructor.
/*!
  Creates image file list.
*/
ImageFileList_::ImageFileList_()
{
  this->Blank();
  this->Initialize();
  assert(true == this->initialized);
}
/* ImageFileList_::ImageFileList_ */



//! Copy constructor.
/*!
  Creates copy of the file list object.

  \param other  Pointer to other class.
*/
ImageFileList_::ImageFileList_(
                               const ImageFileList_& other
                               )
{
  this->Blank();
  this->Initialize();

  if (true == other.initialized)
    {
      CRITICAL_SECTION * const pCS = (CRITICAL_SECTION *)( &(other.csFileList) );
      EnterCriticalSection( pCS );
      {
        if (NULL != other.directory_name)
          {
            bool const set = this->SetDirectory(other.directory_name->c_str(), NULL);
            assert(true == set);
          }
        /* if */
        this->cycle = other.cycle;
      }
      LeaveCriticalSection( pCS );
    }
  /* if */
}
/* ImageFileList_::ImageFileList_ */



//! Blanks ImageFileList structure.
/*!
  Blanks ImageFileList structure.
*/
void
ImageFileList_::Blank(void)
{
  this->filelist = NULL;
  this->it = NULL;
  this->rit = NULL;
  this->directory_name = NULL;
  this->pattern_no = 0;
  ZeroMemory( &(this->csFileList), sizeof(this->csFileList) );
  this->cycle = true;
  this->initialized = false;
}
/* ImageFileList_::Blank */



//! Initializes ImageFileList structure.
/*!
  Initalizes ImageFileList structure.
*/
void
ImageFileList_::Initialize(void)
{
  assert(NULL == this->filelist);
  this->filelist = new sorted_file_list;
  assert(NULL != this->filelist);

  assert(NULL == this->it);
  this->it = new sorted_file_list_iterator;
  assert(NULL != this->it);

  assert(NULL == this->rit);
  this->rit = new sorted_file_list_reverse_iterator;
  assert(NULL != this->rit);

  assert(NULL == this->directory_name);
  this->directory_name = new std::wstring;
  assert(NULL != this->directory_name);

  InitializeCriticalSection( &(this->csFileList) );

  if ( (NULL == this->filelist) ||
       (NULL == this->it) ||
       (NULL == this->rit) ||
       (NULL == this->directory_name)
       )
    {
      this->Release();
      assert(false == this->initialized);
    }
  else
    {
      this->initialized = true;
    }
  /* if */

  this->directory_name->reserve(MAX_PATH + 1);
}
/* ImageFileList_::Initialize */



//! Releases allocated resources.
/*!
  Releases allocated resources.
*/
void
ImageFileList_::Release(void)
{
  this->initialized = false;

  EnterCriticalSection( &(this->csFileList) );
  {
    SAFE_DELETE(this->filelist);
    SAFE_DELETE(this->it);
    SAFE_DELETE(this->rit);
    SAFE_DELETE(this->directory_name);
  }
  LeaveCriticalSection( &(this->csFileList) );

  DeleteCriticalSection( &(this->csFileList) );

  this->Blank();
}
/* ImageFileList_::Release */



//! Initializes file list.
/*!
  Initializes image file list using the supplied directory name.
  If the supplied directory name is invalid then no initialization will be performed.

  \param directory      Image directory.
  \return Returns true if successfull.
*/
bool
ImageFileList_::TrySetDirectory(
                                wchar_t const * const directory
                                )
{
  assert(true == this->initialized);
  if (false == this->initialized) return false;

  assert(NULL != directory);
  if (NULL == directory) return false;

  if ( FALSE == PathIsDirectory(directory) ) return false;

  return this->SetDirectory(directory, NULL);
}
/* ImageFileList_::TrySetDirectory */



//! Initializes file list.
/*!
  Initializes image file list using the supplied directory name.
  If the supplied directory name is invalid then the function will query the user for one using GUI.
  Files in the directory are sorted by filename in ascending order.

  \param directory      Image directory.
  \param title          Dialog title.  May be NULL.
  \return Returns true if successfull.
*/
bool
ImageFileList_::SetDirectory(
                             wchar_t const * const directory,
                             wchar_t const * const title
                             )
{
  assert(true == this->initialized);
  if (false == this->initialized) return false;

  bool initialized = false;

  // Assume input directory is valid.
  std::wstring directory_in_out;
  directory_in_out.reserve(MAX_PATH + 1);
  if (NULL != directory)
    {
      directory_in_out = std::wstring(directory);
    }
  /* if */

  // Set dialog title.
  wchar_t const * pszTitle = title;
  if (NULL == pszTitle)
    {
      pszTitle = gMsgFileListSetDirectory;
    }
  /* if */
    
  // Query user for directory if input directory is invalid.
  if ( FALSE == PathIsDirectory(directory_in_out.c_str()) )
    {
      /* Query the user to select input directory if none is
         given or if given directory does not exist.
      */
      if ( (NULL == directory) && (NULL != this->directory_name) )
        {
          EnterCriticalSection( &(this->csFileList) );
          {
            directory_in_out = *(this->directory_name);
          }
          LeaveCriticalSection( &(this->csFileList) );
        }
      /* if */

      HRESULT const hr = SelectFolderDialog(directory_in_out, pszTitle);
      assert( SUCCEEDED(hr) != (0x800704C7 == hr) );
    }
  /* if */

  // List all files in selected directory and filter out images.
  if (FALSE != PathIsDirectory(directory_in_out.c_str()))
    {
      wchar_t szMask[MAX_PATH + 1];
      wchar_t szFile[MAX_PATH + 1];
      wchar_t *szFileAdr = &szFile[0];

      HRESULT const hr1 = StringCchCopyW(szMask, MAX_PATH, directory_in_out.c_str());
      assert(S_OK == hr1);

      szFile[MAX_PATH] = 0;
      int const count = _snwprintf_s(szFile, MAX_PATH, _T("\\*"));
      assert( (0 <= count) && (count <= MAX_PATH) );

      HRESULT const hr2 = StringCchCatW(szMask, MAX_PATH, szFile);
      assert(S_OK == hr2);

      WIN32_FIND_DATA ffd;
      HANDLE hFind = INVALID_HANDLE_VALUE;
      hFind = FindFirstFile(szMask, &ffd);

      size_t numfiles = 0;

      sorted_file_list * pFilelist = new sorted_file_list;
      assert(NULL != pFilelist);
      if (NULL != pFilelist)
        {

          if (INVALID_HANDLE_VALUE != hFind)
            {
              /* First empty the file list. */
              while ( ! pFilelist->empty() ) pFilelist->clear();

              do
                {
                  if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                      /* Skip directories. */
                    }
                  else if (true == CheckExtension_inline(ffd.cFileName, _T(".xml")))
                    {
                      /* Skip XML metadata. */
                    }
                  else if ( (true == CheckExtension_inline(ffd.cFileName, _T(".png"))) || // Default for saving images.
                            (true == CheckExtension_inline(ffd.cFileName, _T(".bmp"))) || // Windows bitmaps.
                            (true == CheckExtension_inline(ffd.cFileName, _T(".jpeg"))) || // JPEG.
                            (true == CheckExtension_inline(ffd.cFileName, _T(".jpg"))) ||
                            (true == CheckExtension_inline(ffd.cFileName, _T(".jpe"))) ||
                            (true == CheckExtension_inline(ffd.cFileName, _T(".jp2"))) ||
                            (true == CheckExtension_inline(ffd.cFileName, _T(".tif"))) || // TIFF.
                            (true == CheckExtension_inline(ffd.cFileName, _T(".tiff")))
                            )
                    {
                      /* Collect only files supporeted by WIC. */
                      StringCchCopyW(szFile, MAX_PATH, ffd.cFileName);
                      pFilelist->push_back( szFileAdr );
                      numfiles++;
                    }
                  /* if */
                }
              while (0 != FindNextFile(hFind, &ffd));

              FindClose(hFind);
            }
          else
            {
              /* If needed do some cleanup. */
            }
          /* if */

          /* Sort list. */
          pFilelist->sort();
          //pFilelist->sort( CompareFileNameByLastNumber_inline );

          /* Change filelist to new one. */
          EnterCriticalSection( &(this->csFileList) );
          {
            // Set directory name.
            assert(NULL != this->directory_name);
            if (NULL != this->directory_name) *(this->directory_name) = directory_in_out;

            // Set filelist.
            sorted_file_list * tmpFilelist = this->filelist;
            this->filelist = pFilelist;
            SAFE_DELETE(tmpFilelist);

            /* Create list iterators. */
            bool const rewind = RewindToFirst_inline(this);
            assert(true == rewind);

            /* Increase patern number. */
            this->pattern_no += 1;
          }
          LeaveCriticalSection( &(this->csFileList) );

          initialized = true;
        }
      /* if */
    }
  /* if */

  return initialized;
}
/* ImageFileList_::SetDirectory */



//! Get current directory.
/*!
  Gets pointer to current directory.
  The pointer is valid until the directory is changed.

  \return Pointer to current directory.
*/
wchar_t const *
ImageFileList_::GetDirectory(
                             void
                             )
{
  assert(true == this->initialized);
  if (NULL == this->directory_name) return NULL;
  wchar_t const * directory = NULL;
  EnterCriticalSection( &(this->csFileList) );
  {
    directory = this->directory_name->c_str();
  }
  LeaveCriticalSection( &(this->csFileList) );
  return directory;
}
/* ImageFileList_::GetDirectory */



//! Extend with black SL patterns.
/*!
  Adds pure black SL patterns so the total number of images in the list is equal to N.
  
  \param N      Number of images in the list. Must be equal to or larger than current number of images in the list.
  \return  Returns true if successfull, false otherwise.
*/

bool
ImageFileList_::ExtendWithBlackSLPatterns(
                                          int const N
                                          )
{
  assert(true == this->initialized);
  if (false == this->initialized) return false;

  bool succeeded = false;

  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist)
      {
        int const size = (int)(this->filelist->size());
        if (N > size)
          {
            succeeded = true;

            /* Remember which pattern is active. */
            int const index = (int)( std::distance(this->filelist->begin(), *(this->it)) );
            
            /* Add empty patterns to the end of the list. */
            FilePattern black_SL_pattern;
            for (int i = size; i < N; ++i)
              {
                this->filelist->push_back(black_SL_pattern);
              }
            /* for */
            assert( N == (int)( this->filelist->size() ) );
            
            /* Recreate list iterators. */
            bool const rewind = RewindToFirst_inline(this);
            assert(true == rewind);
            succeeded = succeeded && rewind;

            /* Rewind to active pattern. */
            for (int i = 0; i < index; ++i)
              {
                bool const step = this->Next();
                succeeded = succeeded && step;
              }
            /* for */
            assert( index == (int)( std::distance(this->filelist->begin(), *(this->it))) );
          }
        else if (N == size)
          {
            succeeded = true;
          }
        /* if */
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );  

  return succeeded;
}
/* ImageFileList_::ExtendWithBlackSLPatterns */



//! Remove all black SL patterns.
/*!
  Removes all black SL patterns from the list.

  \return Returns true if successfull, false otherwise.
*/
bool
ImageFileList_::RemoveAllBlackSLPatterns(
                                         void
                                         )
{
  assert(true == this->initialized);
  if (false == this->initialized) return false;

  bool succeeded = false;

  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist)
      {
        /* Remove all black SL patterns. */
        auto new_end = std::remove_if(this->filelist->begin(), this->filelist->end(), IsFilePatternBlackSLPattern_inline );
        this->filelist->erase(new_end, this->filelist->end());

        /* Re-sort list. */
        this->filelist->sort();

        /* Create list iterators. */
        bool const rewind = RewindToFirst_inline(this);
        assert(true == rewind);

        succeeded = true;
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );  

  return succeeded;  
}
/* ImageFileList_::RemoveAllBlackSLPatterns */



//! Steps to next image.
/*!
  Steps to next image. If cycle flag is set and current image is last
  image in the directory function will rewind the list to begining.

  \return Returns true if successfull.
*/
bool
ImageFileList_::Next()
{
  assert(true == this->initialized);
  if (false == this->initialized) return false;

  bool succeeded = true;

  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist)
      {
        if (this->filelist->empty())
          {
            // List is empty.
            succeeded = false;
          }
        else if ( (false == this->cycle) && (this->filelist->end() == *(this->it)) )
          {
            // We are at the end of the list and cycling is prohibited.
            succeeded = false;
          }
        else
          {
            // Step to next item.
            if (this->filelist->end() != *(this->it)) ++(*(this->it));
            if (this->filelist->rbegin() != *(this->rit)) --(*(this->rit));

            // Rewind if needed.
            if ( (true == this->cycle) && (this->filelist->end() == *(this->it)) )
              {
                succeeded = RewindToFirst_inline( this );
                assert(true == succeeded);
              }
            /* if */
          }
        /* if */
      }
    else
      {
        succeeded = false;
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );

  return succeeded;
}
/* ImageFileList_::Next */



//! Steps to previous image.
/*!
  Steps to previous image. If cycle flag is set and current image is first
  image in the directory function will rewind the list to the end.

  \return Returns true if successfull.
*/
bool
ImageFileList_::Prev()
{
  assert(true == this->initialized);

  bool succeeded = true;

  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist)
      {
        if (this->filelist->empty())
          {
            // List is empty.
            succeeded = false;
          }
        else if ( (false == this->cycle) && (this->filelist->rend() == *(this->rit)) )
          {
            // We are at the start of the list and cycling is prohibited.
            succeeded = false;
          }
        else
          {
            // Step to previous item.
            if (this->filelist->begin() != *(this->it)) --(*(this->it));
            if (this->filelist->rend() != *(this->rit)) ++(*(this->rit));

            // Rewind if needed.
            if ( (true == this->cycle) && (this->filelist->rend() == *(this->rit)) )
              {
                succeeded = RewindToLast_inline( this );
                assert(true == succeeded);
              }
            /* if */
          }
        /* if */
      }
    else
      {
        succeeded = false;
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );

  return succeeded;
}
/* ImageFileList_::Prev */



//! Checks if filename is valid.
/*!
  Function checks if current item has a valid filename.

  \return Returns true if current item has a valid filename.
*/
bool
ImageFileList_::HaveFileName(
                             void
                             )
{
  assert(true == this->initialized);
  bool succeeded = false;
  bool have_filename = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetName_inline(this, &have_filename, NULL, 0, false, false);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded && have_filename;  
}
/* ImageFileList_::HaveFileName */



//! Returns full filename of current image.
/*!
  Returns full filename (including absolute path) of the current image.

  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFullFileName(
                                wchar_t * const name_out,
                                int const size_out
                                )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetName_inline(this, NULL, name_out, size_out, true, true);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFullFileName */



//! Returns filename without path.
/*!
  Returns the filename of the current image (no path).

  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFileName(
                            wchar_t * const name_out,
                            int const size_out
                            )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetName_inline(this, NULL, name_out, size_out, false, true);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFileName */



//! Returns path.
/*!
  Returns absolute pathname for the current image.

  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFilePath(
                            wchar_t * const name_out,
                            int const size_out
                            )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetName_inline(this, NULL, name_out, size_out, true, false);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFilePath */



//! Returns pattern ID.
/*!
  Returns pattern ID.

  \param pattern_no_out    Pointer where pattern number will be stored.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetID(
                      int * const pattern_no_out
                      )
{
  assert(true == this->initialized);
  if (NULL == pattern_no_out) return false;
  EnterCriticalSection( &(this->csFileList) );
  {
    *pattern_no_out = this->pattern_no;
  }
  LeaveCriticalSection( &(this->csFileList) );
  return true;
}
/* ImageFileList_::GeID */



//! Returns file index.
/*!
  Returns file index.

  \return File index or -1 if there is no current file.
*/
int
ImageFileList_::GetFileIndex(
                             void
                             )
{
  assert(true == this->initialized);

  int index = -1;

  EnterCriticalSection( &(this->csFileList) );
  {
    if ( (NULL != this->filelist) &&
         (this->filelist->empty() == false) &&
         (this->filelist->end() != *(this->it)) &&
         (this->filelist->rend() != *(this->rit))
         )
      {
        index = (int)std::distance(this->filelist->begin(), *(this->it));
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );

  return index;
}
/* ImageFileList_::GetFileIndex */



//! Checks if filename is valid at specified list index.
/*!
  Function checks if item at specified list index has a valid filename.

  \return Returns true if item has a valid filename.
*/
bool
ImageFileList_::HaveFileNameAt(
                               int const index
                               )
{
  assert(true == this->initialized);
  bool succeeded = false;
  bool have_filename = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetNameAt_inline(this, index, &have_filename, NULL, 0, false, false);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded && have_filename;  
}
/* ImageFileList_::HaveFileNameAt */



//! Returns full filename of image at specified list index.
/*!
  Returns full filename (including absolute path) of image at specified list index.

  \param index File index.
  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFullFileNameAt(
                                  int const index,
                                  wchar_t * const name_out,
                                  int const size_out
                                  )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetNameAt_inline(this, index, NULL, name_out, size_out, true, true);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFullFileNameAt */



//! Returns filename without path of image at specified list index.
/*!
  Returns the filename of image at specified index (no path).

  \param index File index.
  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFileNameAt(
                              int const index,
                              wchar_t * const name_out,
                              int const size_out
                              )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetNameAt_inline(this, index, NULL, name_out, size_out, false, true);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFileNameAt */



//! Returns path of image at specified list index.
/*!
  Returns absolute pathname for image at specified index.

  \param index File index.
  \param name_out Pointer to buffer where requested name will be stored.
  \param size_out Maximal size of the buffer.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetFilePathAt(
                              int const index,
                              wchar_t * const name_out,
                              int const size_out
                              )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = GetNameAt_inline(this, index, NULL, name_out, size_out, true, false);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::GetFilePathAt */



//! Returns path of image at specified list index.
/*!
  Returns pattern ID.

  \param index File index.
  \param pattern_no_out    Pointer where ID will be stored.
  \return Returns true if successfull.
*/
bool
ImageFileList_::GetIDAt(
                        int const index,
                        int * const pattern_no_out
                        )
{
  assert(true == this->initialized);
  if (NULL == pattern_no_out) return false;
  EnterCriticalSection( &(this->csFileList) );
  {
    *pattern_no_out = this->pattern_no;
  }
  LeaveCriticalSection( &(this->csFileList) );
  return true;
}
/* ImageFileList_::GeIDAt */



//! Rewinds file list to start.
/*!
  Rewinds file list to start.

  \return Returns true if successfull.
*/
bool
ImageFileList_::Rewind(
                       void
                       )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    succeeded = RewindToFirst_inline( this );
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::Rewind */



//! Size.
/*!
  Returns filelist size.

  \return Number of items in the file list.
*/
int
ImageFileList_::Size(
                     void
                     )
{
  assert(true == this->initialized);
  int size = 0;
  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist) size = (int)(this->filelist->size());
  }
  LeaveCriticalSection( &(this->csFileList) );
  return size;
}
/* ImageFileList_::Rewind */



//! At end.
/*!
  Tests if we have reached the end of the file list.

  \return Returns true if iterator is at the end of the file list (e.g. there are no more files to process).
*/
bool
ImageFileList_::AtEnd(
                      void
                      )
{
  assert(true == this->initialized);
  bool atend = true;
  EnterCriticalSection( &(this->csFileList) );
  {
    if (NULL != this->filelist) atend = this->filelist->end() == *(this->it);
  }
  LeaveCriticalSection( &(this->csFileList) );
  return atend;
}
/* ImageFileList_::AtEnd */



//! To end and stop cycling.
/*!
  Function rewinds list to end and sets cycling flag to false.

  \return Returns true if successfull.
*/
bool
ImageFileList_::ToEndAndStopCycling(
                                    void
                                    )
{
  assert(true == this->initialized);
  bool succeeded = false;
  EnterCriticalSection( &(this->csFileList) );
  {
    this->cycle = false;
    succeeded = RewindToLast_inline( this );
    if (NULL != this->filelist)
      {
        if (this->filelist->end() != *(this->it)) ++(*(this->it));
        if (this->filelist->rbegin() != *(this->rit)) --(*(this->rit));
      }
    /* if */
  }
  LeaveCriticalSection( &(this->csFileList) );
  return succeeded;
}
/* ImageFileList_::ToEndAndStopCycling */



//! Load image.
/*!
  Loads current image and steps to next one.

  \return Returns cv::Mat pointer or NULL if unsuccessfull.
*/
cv::Mat *
ImageFileList_::ReadImage(
                          int const index
                          )
{
  wchar_t wfilename[MAX_PATH];

  if (0 > index)
    {
      bool const get = this->GetFullFileName(wfilename, MAX_PATH);
      assert(true == get);
      if (true != get) return NULL;
    }
  else
    {
      bool const get = this->GetFullFileNameAt(index, wfilename, MAX_PATH);
      //assert(true == get);
      if (true != get) return NULL;
    }
  /* if */

  // Note that default filename is stored as unicode string so it must be transcoded to work with ANSI C functions.
  char cfilename[MAX_PATH + 1];
  cfilename[MAX_PATH] = 0;
  bool ansi_valid = true;

  int const numch = WideCharToMultiByte(
                                        CP_ACP, // Transcode for ANSI fopen/fclose function.
                                        0,
                                        wfilename, // Input string.
                                        -1,
                                        cfilename, // Output string.
                                        MAX_PATH,
                                        NULL,
                                        NULL
                                        );
  assert( (0 < numch) && (numch < MAX_PATH) );

  if ( (0 >= numch) || (MAX_PATH <= numch) )
    {
      DWORD const error_code = GetLastError();
      assert(ERROR_SUCCESS == error_code);
      ansi_valid = false;
    }
  /* if */

  if (false == ansi_valid) return NULL;

  cv::Mat * image = new cv::Mat();
  assert(NULL != image);
  if (NULL == image) return image;

  *image = cv::imread(cfilename);

  bool const next = this->Next();
  //assert(true == next);

  return image;
}
/* ImageFileList_::ReadImage */



//! Destructor.
/*!
  Blanks class variables and releases allocated resources.
*/
ImageFileList_::~ImageFileList_()
{
  this->Release();
  this->Blank();
  assert(false == this->initialized);
}
/* ImageFileList_::~ImageFileList_ */



//! Checks filename extension.
/*!
  Function checks filename extension. Letter case is ignored.

  \see CheckExtension_inline

  \param fname  Filename.
  \param ext    Extension.
  \return Returns true if filename has the supplied extension.
*/
bool
CheckExtension(
               wchar_t const * const fname,
               wchar_t const * const ext
               )
{
  return CheckExtension_inline(fname, ext);
}
/* CheckExtension */



//! Delete file list.
/*!
  Deletes image file list which was created using operator new.

  \param ptr    Pointer to image file list.
*/
void
DeleteImageFileList(
                    ImageFileList_ * const ptr
                    )
{
  //assert(NULL != ptr);
  if (NULL == ptr) return;
  delete ptr;
}
/* DeleteImageFileList */


#endif /* !__BATCHACQUISITIONFILELIST_CPP */
