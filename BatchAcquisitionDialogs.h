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
  \file   BatchAcquisitionDialogs.h
  \brief  Common dialogs.

  \author Tomislav Petkovic
  \date   2017-05-23
*/


#ifndef __BATCHACQUISITIONDIALOGS_H
#define __BATCHACQUISITIONDIALOGS_H


//! Queries user for folder.
HRESULT
SelectFolderDialog(
                   std::wstring &,
                   wchar_t const * const
                   );

//! Queries user for folder.
bool
SelectFolderDialog(
                   wchar_t * const,
                   wchar_t const * const
                   );

//! File save dialog.
HRESULT
FileSaveDialog(
               std::wstring &,
               wchar_t const * const,
               int const,
               COMDLG_FILTERSPEC const * const,
               int const,
               wchar_t const * const * const,
               int const * const,
               UINT * const
               );


#endif /* !__BATCHACQUISITIONDIALOGS_H */
