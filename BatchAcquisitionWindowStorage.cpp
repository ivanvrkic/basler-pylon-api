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
  \file   BatchAcquisitionWindowStorage.cpp
  \brief  Global storage of window parameters.

  Default window procedure function only accepts four parameters. Of those HWND to
  the window is unique and may be used to fetch additional parameters from the
  global storage. Here we define required functions and locks for this storage.

  \author Tomislav Petkovic
  \date   2015-05-19
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONWINDOWSTORAGE_CPP
#define __BATCHACQUISITIONWINDOWSTORAGE_CPP


#include "BatchAcquisitionWindowStorage.h"



//! Global map that enables window message procedure to access window data.
window_data_map * gMap = NULL;

//! Slim Reader/Writer lock for global window data map.
SRWLOCK gMapLock;




//! Creates global std::map to store window data.
/*!
  Creates global map that stores local window data.
  Data is identified via window handle.
*/
void
CreateWindowDataStorage(void)
{
  assert(NULL == gMap);
  gMap = new window_data_map;
  assert(NULL != gMap);

  if (NULL == gMap) exit(EXIT_FAILURE);

  InitializeSRWLock( &gMapLock );
}
/* CreateWindowDataStorage */



//! Destroys global std::map to store window data.
/*!
  Destroys global map that stores local window data.
*/
void
DestroyWindowDataStorage(void)
{
  AcquireSRWLockExclusive( &gMapLock );

  assert(NULL != gMap);
  if (NULL != gMap) delete gMap;
  gMap = NULL;

  ReleaseSRWLockExclusive( &gMapLock );
}
/* DestroyWindowDataStorage */



//! Gets window data.
/*!
  Gets pointer to window data from global storage.

  \param hWnd   Handle that uniquely identifies the window.
  \return Returns pointer to some data or NULL if no data exists.
*/
void *
GetWindowData(
              HWND const hWnd
              )
{
  void * ptr = NULL;

  BOOLEAN const get = TryAcquireSRWLockShared( &gMapLock );
  if (0 == get) return ptr;

  assert(NULL != gMap);
  if ( (NULL != gMap) && (false == gMap->empty()) )
    {
      window_data_map::const_iterator it = gMap->find((uint64_t)( hWnd ));
      if (gMap->end() != it) ptr = it->second;
    }
  /* if */

  ReleaseSRWLockShared( &gMapLock );

  return ptr;
}
/* GetWindowData */



//! Sets window data.
/*!
  Stores pointer to window data in global storage (std::map).

  \param ptr    Pointer to window data.
  \param hWnd   Handle that uniquely identifies the window.
*/
void
SetWindowData(
              void * const ptr,
              HWND const hWnd
              )
{
  assert(NULL != gMap);
  if (NULL == gMap) CreateWindowDataStorage();

  assert(NULL != ptr);
  if (NULL == ptr) return;

  AcquireSRWLockExclusive( &gMapLock );

  if (NULL != gMap) (*gMap)[(uint64_t)( hWnd )] = ptr;

  ReleaseSRWLockExclusive( &gMapLock );
}
/* SetWindowData */



#endif /* !__BATCHACQUISITIONWINDOWSTORAGE_CPP */
