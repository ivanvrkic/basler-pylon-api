/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2014 UniZG, Zagreb. All rights reserved.
 * (c) 2014 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionFileList.h
  \brief  File list for selected directory.

  \author Tomislav Petkovic
  \date   2014-12-13
*/


#ifndef __BATCHACQUISITIONFILELIST_H
#define __BATCHACQUISITIONFILELIST_H


#include "BatchAcquisition.h"


//! SL pattern.
/*!
  SL pattern may be of various types.
  For file lists it may be an image file or a pure black pattern.
*/
typedef
struct FilePattern_
{
  std::wstring * filename; //!< Filename.

  //! Default constructor for pure black pattern.
  FilePattern_();

  //! From string constructor.
  FilePattern_(const std::wstring &);

  //! From string pointer constructor.
  FilePattern_(wchar_t * const);
  
  //! Copy constructor.
  FilePattern_(const FilePattern_ &);

  //! Default destructor.
  ~FilePattern_();

  //! Default weak comparison operator.
  bool operator < (const FilePattern_ &);

  //! Test if we have filename.
  bool HaveFilename(void);

  //! Get filename.
  const std::wstring & GetFilename(void);
} FilePattern;


//! List of found images in selected directory.
typedef std::list<FilePattern_> sorted_file_list;

//! Iterator for going through images in normal direction.
typedef sorted_file_list::iterator sorted_file_list_iterator;

//! Iterator for going through images in reverse direction.
typedef sorted_file_list::reverse_iterator sorted_file_list_reverse_iterator;


//! Filename list.
/*!
  This structure stores current file iterator so we can move
  easily forward and backward through the file list.
*/
typedef
struct ImageFileList_
{
  sorted_file_list * filelist; /*!< File list. */
  sorted_file_list_iterator * it; /*!< File list iterator. */
  sorted_file_list_reverse_iterator * rit; /*!< Reverse file list iterator. */

  std::wstring * directory_name; /*!< Directory name containing input images. */
  int pattern_no; //!< Pattern number.

  CRITICAL_SECTION csFileList; //!< Critical section for syncronizing access to file list.

  volatile bool cycle; //!< Flag to indicate cycling through the directory.
  volatile bool initialized; //!< Flag to indicate structure is initialized.

  //! Constructor.
  ImageFileList_();

  //! Copy constructor.
  ImageFileList_(const ImageFileList_&);

  //! Blanks ImageFileList structure.
  void Blank(void);

  //! Initializes ImageFileList structure.
  void Initialize(void);

  //! Releases allocated resources.
  void Release(void);

  //! Initializes file list.
  bool TrySetDirectory(wchar_t const * const);

  //! Initializes file list.
  bool SetDirectory(wchar_t const * const, wchar_t const * const);

  //! Get current directory.
  wchar_t const * GetDirectory();

  //! Extend with black SL patterns.
  bool ExtendWithBlackSLPatterns(int const);

  //! Remove all black SL patterns.
  bool RemoveAllBlackSLPatterns(void);

  //! Steps to next image.
  bool Next(void);

  //! Steps to previous image.
  bool Prev(void);

  //! Checks if filename is valid.
  bool HaveFileName(void);

  //! Returns full filename of current image.
  bool GetFullFileName(wchar_t * const, int const);

  //! Returns filename without path.
  bool GetFileName(wchar_t * const, int const);

  //! Returns path.
  bool GetFilePath(wchar_t * const, int const);

  //! Returns pattern ID.
  bool GetID(int * const);

  //! Returns file index.
  int GetFileIndex(void);

  //! Checks if filename is valid.
  bool HaveFileNameAt(int const);

  //! Returns full filename of image at specified list index.
  bool GetFullFileNameAt(int const, wchar_t * const, int const);

  //! Returns filename without path of image at specified list index.
  bool GetFileNameAt(int const, wchar_t * const, int const);

  //! Returns path of image at specified list index.
  bool GetFilePathAt(int const, wchar_t * const, int const);

  //! Returns pattern ID at specified list index.
  bool GetIDAt(int const, int * const);

  //! Rewinds file list to start.
  bool Rewind(void);

  //! Size.
  int Size(void);

  //! At end.
  bool AtEnd(void);

  //! To end and stop cycling.
  bool ToEndAndStopCycling(void);

  //! Load image.
  cv::Mat * ReadImage(int);

  //! Destructor.
  ~ImageFileList_();

} ImageFileList;


//! Check file extension.
bool CheckExtension(wchar_t const * const, wchar_t const * const);

//! Delete file list.
void DeleteImageFileList(ImageFileList_ * const);


#endif /* !__BATCHACQUISITIONFILELIST_H */
