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
  \file   BatchAcquisitionDialogs.cpp
  \brief  Common dialogs.

  Common dialogs to select file or directory.

  \author Tomislav Petkovic
  \date   2017-05-23
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONDIALOGS_CPP
#define __BATCHACQUISITIONDIALOGS_CPP


#include "BatchAcquisition.h"
#include "BatchAcquisitionDialogs.h"
#include "BatchAcquisitionFileList.h"


#include <Shlobj.h>



//! Finds matching extension.
/*!
  Function finds matching extension (if any).

  \param extsz  Size of file type extensions list.
  \param ext    List of file type extensions.
  \param filename       Current filename.
  \return Index of extension of -1 if no extension matches.
*/
inline
int
GetMatchingExtension_inline(
                            int const extsz,
                            wchar_t const * const * const ext,
                            wchar_t const * const filename
                            )
{
  int idx = -1;

  assert(NULL != ext);
  if (NULL == ext) return idx;

  assert(NULL != filename);
  if (NULL == filename) return idx;

  for (int i = 0; i < extsz; ++i)
    {
      if (true == CheckExtension(filename, ext[i]))
        {
          idx = i;
          return i;
        }
      /* if */
    }
  /* for */

  return idx;
}
/* GetMatchingExtension_inline */



//! Append extension.
/*!
  Function appends extension to the current filename is there is no extension present.

  \param idx    File type index.
  \param extsz  Size of file type extensions list.
  \param ext    List of file type extensions.
  \param filename       Current filename.
*/
inline
void
AppendExtensionIfMissing_inline(
                                int const idx,
                                int const extsz,
                                wchar_t const * const * const ext,
                                std::wstring &filename
                                )
{
  assert(NULL != ext);
  if (NULL == ext) return;

  assert( (0 <= idx) && (idx < extsz) );
  if ( (0 > idx) || (idx >= extsz) ) return;

  if (false == CheckExtension(filename.c_str(), ext[idx]))
    {
      filename += std::wstring(ext[idx]);
    }
  /* if */
}
/* AppendExtensionIfMissing_inline */



//! Queries user for folder.
/*!
  Open selection dialog for the user to select a folder.
  This function uses a standard file open dialog introduced with Windows Vista.

  \param directory_in_out       Starting and selected directory.
  \return Returns S_OK if successfull and error code otherwise.
*/
HRESULT
SelectFolderDialog(
                   std::wstring & directory_in_out,
                   wchar_t const * const title
                   )
{
  IFileDialog * pfd = NULL;
  IShellItem * psiResult = NULL;
  IShellItem * pdir = NULL;
  DWORD dwFlags = 0;
  PWSTR pszFilePath = NULL;

  HRESULT hr = S_OK;

  if ( SUCCEEDED(hr) )
    {
      hr = CoCreateInstance(
                            CLSID_FileOpenDialog,
                            NULL,
                            CLSCTX_ALL,
                            IID_PPV_ARGS(&pfd)
                            );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) && (NULL != title) )
    {
      hr = pfd->SetTitle(title);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) && (0 < directory_in_out.size()) )
    {
      HRESULT hr_dir = S_OK;

      if ( SUCCEEDED(hr_dir) )
        {
          hr_dir = SHCreateItemFromParsingName(
                                               directory_in_out.c_str(),
                                               NULL,
                                               IID_PPV_ARGS(&pdir)
                                               );
          assert( SUCCEEDED(hr_dir) );
        }
      /* if */

      if ( SUCCEEDED(hr_dir) )
        {
          hr_dir = pfd->SetFolder(pdir);
          assert( SUCCEEDED(hr_dir) );
        }
      /* if */
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->GetOptions(&dwFlags);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->SetOptions(dwFlags | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->Show(NULL);
      assert( SUCCEEDED(hr) || (0x800704C7 == hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->GetResult(&psiResult);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      directory_in_out = std::wstring(pszFilePath);
      CoTaskMemFree(pszFilePath);
    }
  /* if */

  SAFE_RELEASE(psiResult);
  SAFE_RELEASE(pfd);
  SAFE_RELEASE(pdir);

  return hr;
}
/* SelectFolderDialog */



//! Queries user for folder.
/*!
  Open selection dialog for the user to select a folder.
  This function uses shell selection dialog introduces with Windows XP.

  \param szDirectory    Pointer to a string of length MAX_PATH.
  \param pszTitle       Pointer to a string containing window title. May be NULL.
  \return Returns true if user selects the folder.
*/
bool
SelectFolderDialog(
                   wchar_t * const szDirectory,
                   wchar_t const * const pszTitle
                   )
{
  bool selected = false;

  BROWSEINFO bi;
  ZeroMemory(&bi, sizeof(BROWSEINFO));

  bi.hwndOwner = NULL;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = szDirectory;
  bi.lpszTitle = pszTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS;
  bi.lParam = NULL;
  bi.iImage = 0;

  LPITEMIDLIST const pidl = SHBrowseForFolder(&bi);

  if (NULL != pidl)
    {
      BOOL const bRet = SHGetPathFromIDList(pidl, szDirectory);
      assert(TRUE == bRet);
      SHFree(pidl);

      selected = true;
    }
  else
    {
      szDirectory[0] = '\0';
    }
  /* if */

  return selected;
}
/* SelectFolderDialog */



//! File save dialog.
/*!
  Function opens dialog and queries user to select output filename and directory.

  \param filename_in_out        Reference to input-output filename.
  \param title    Title of the dialog.
  \param rgSize   Size of the extension filter specification.
  \param rgSpec   File dialog filter specification.
  \param extSize        Number of supported extensions.
  \param extNames       Array of supported extensions.
  \param type_to_idx    Table matching extension filter specification to extensions in extNames.
  \param piFileType_out Pointer to address where selected filter specification will be stored.
  \return Function returns S_OK if successfull and error code otherwise.
*/
HRESULT
FileSaveDialog(
               std::wstring & filename_in_out,
               wchar_t const * const title,
               int const rgSize,
               COMDLG_FILTERSPEC const rgSpec[],
               int const extSize,
               wchar_t const * const * const extNames,
               int const * const type_to_idx,
               UINT * const piFileType_out
               )
{
  IFileDialog * pfd = NULL;
  IShellItem * psiResult = NULL;
  DWORD dwFlags = 0;
  PWSTR pszFilePath = NULL;
  UINT iFileType = 1;

  HRESULT hr = S_OK;

  if ( SUCCEEDED(hr) )
    {
      hr = CoCreateInstance(
                            CLSID_FileSaveDialog,
                            NULL,
                            CLSCTX_ALL,
                            IID_PPV_ARGS(&pfd)
                            );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) && (NULL != title) )
    {
      hr = pfd->SetTitle(title);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  bool const have_filetype_filter = (0 < rgSize) && (NULL != rgSpec);
  if ( SUCCEEDED(hr) && (true == have_filetype_filter) )
    {
      hr = pfd->SetFileTypes(rgSize, rgSpec);
      assert( SUCCEEDED(hr) );

      if ( SUCCEEDED(hr) )
        {
          hr = pfd->SetFileTypeIndex(1); // Index starts from one!
          assert( SUCCEEDED(hr) );
        }
      /* if */
    }
  /* if */

  if ( SUCCEEDED(hr) && (0 < filename_in_out.size()) )
    {
      hr = pfd->SetFileName(filename_in_out.c_str());
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->GetOptions(&dwFlags);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) && (true == have_filetype_filter) )
    {
      hr = pfd->SetOptions(dwFlags | FOS_STRICTFILETYPES);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->Show(NULL);
      assert( SUCCEEDED(hr) || (0x800704C7 == hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = pfd->GetResult(&psiResult);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
      assert( SUCCEEDED(hr) );
    }
  /* if */

  if ( SUCCEEDED(hr) )
    {
      filename_in_out = std::wstring(pszFilePath);
      CoTaskMemFree(pszFilePath);
    }
  /* if */

  if ( SUCCEEDED(hr) && (true == have_filetype_filter) )
    {
      hr = pfd->GetFileTypeIndex( &iFileType );
      assert( SUCCEEDED(hr) );
    }
  /* if */

  bool const have_extensions = (0 < extSize) && (NULL != extNames) && (NULL != type_to_idx);
  if ( SUCCEEDED(hr) && (true == have_filetype_filter) && (true == have_extensions) )
    {
      int const have_ext = GetMatchingExtension_inline(extSize, extNames, filename_in_out.c_str());
      if (-1 == have_ext)
        {
          int const idx = type_to_idx[iFileType];
          AppendExtensionIfMissing_inline(idx, extSize, extNames, filename_in_out);
        }
      /* if */
    }
  /* if */

  if (NULL != piFileType_out) *piFileType_out = iFileType;

  SAFE_RELEASE(psiResult);
  SAFE_RELEASE(pfd);

  return hr;
}
/* FileSaveDialog */



#endif /* !__BATCHACQUISITIONDIALOGS_CPP */
