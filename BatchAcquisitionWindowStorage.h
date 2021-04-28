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
  \file   BatchAcquisitionWindowStorage.h
  \brief  Global storage of window parameters.

  Header for global storage of window parameters.

  \author Tomislav Petkovic
  \date   2015-05-19
*/


#ifndef __BATCHACQUISITIONWINDOWSTORAGE_H
#define __BATCHACQUISITIONWINDOWSTORAGE_H


#include "BatchAcquisition.h"


#include <map>


#pragma warning(push)
#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(pop)



//! Global window data map.
/*!
  Code for DirectX display window is written to support more than one display window. However,
  as the WndProc is shared between all opened display windows we require a global data map
  that may be used to fetch data pertaining to particular instance of WndProc. We use
  std::map for this with window handle as the key.
 */
typedef std::map<uint64_t, void *> window_data_map;



/****** GLOBAL DATA STORAGE ******/

//! Creates global std::map to store window data.
void CreateWindowDataStorage(void);

//! Destroys global std::map to store window data.
void DestroyWindowDataStorage(void);

//! Gets window data.
void * GetWindowData(HWND const);

//! Sets window data.
void SetWindowData(void * const, HWND const);



#endif /* !__BATCHACQUISITIONWINDOWSTORAGE_H */
