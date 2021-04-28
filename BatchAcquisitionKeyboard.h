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
  \file   BatchAcquisitionKeyboard.h
  \brief  Timed keyboard input.

  \author Tomislav Petkovic
  \date   2017-02-08
*/



#ifndef __BATCHACQUISITIONKEYBOARD_H
#define __BATCHACQUISITIONKEYBOARD_H


//! Wait for keypress.
int
TimedWaitForNumberKey(
                      unsigned int const,
                      unsigned int const,
                      bool const,
                      bool const,
                      HWND const
                      );

//! Wait for keypress.
int
TimedWaitForSelectedKeys(
                         unsigned int const,
                         unsigned int const,
                         TCHAR const * const,
                         wint_t const * const,
                         int const
                         );


#endif /* !__BATCHACQUISITIONKEYBOARD_H */
