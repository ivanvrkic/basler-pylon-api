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
  \file   BatchAcquisitionKeyboard.cpp
  \brief  Timed keyboard input.

  Functions for timed keyboard input.

  \author Tomislav Petkovic
  \date   2017-02-08
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONKEYBOARD_CPP
#define __BATCHACQUISITIONKEYBOARD_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionKeyboard.h"

#include "conio.h"



//! Wait for keypress.
/*!
  Function waits for keypress of a numbered key. Waiting is done by periodically
  checking if key was pressed until total waiting time exceeds the timeout.

  \param timeout        Maximum time to wait for keypress in milliseconds. Use negative value to wait indefinitely.
  \param slice_duration     Duration of one waiting slice in milliseconds.
  \param any_key_breaks     If true then function returns after any key is pressed; otherwise function returns only after user presses a numbered key.
  \param print_remaining_time  If true then function prints remaining time in seconds before returning.
  \param hWndCommand    If supplied then this window is brought to foreground after each time slice elapses.
  \return Returns number of pressed number key or -1 if no number key was pressed.
*/
int
TimedWaitForNumberKey(
                      unsigned int const timeout,
                      unsigned int const slice_duration,
                      bool const any_key_breaks,
                      bool const print_remaining_time,
                      HWND const hWndCommand
                      )
{
  int result = -1;

  // Elapsed time starts from 0.
  unsigned int t = 0;
  int t_remaining = -1;

  // Limit slice duration to 5ms.
  DWORD sleep_time = slice_duration;
  if (5 > sleep_time) sleep_time = 5;

  // Get console handle.
  HANDLE rhnd = GetStdHandle(STD_INPUT_HANDLE);
  if (rhnd == INVALID_HANDLE_VALUE) return result;

  INPUT_RECORD event_buffer;

  bool key_pressed = false;
  DWORD num_read = 0;
  BOOL get_event = FALSE;

  bool time_elapsed = false;
  do
    {
      // Check if a key has been pressed.
      {
        key_pressed = false;
        num_read = 0;
        get_event = FALSE;

        BOOL const peek_event = PeekConsoleInput(rhnd, &event_buffer, 1, &num_read);
        if ( (TRUE == peek_event) && (0 < num_read) )
          {
            get_event = ReadConsoleInput(rhnd, &event_buffer, 1, &num_read);
            //assert(TRUE == get_event);
          }
        /* if */
        
        if ( (TRUE == get_event) && (1 == num_read) )
          {
            key_pressed = (KEY_EVENT == event_buffer.EventType) && (TRUE == event_buffer.Event.KeyEvent.bKeyDown);
          }
        /* if */
      }

      // If no key was pressed sleep and check again.
      if (false == key_pressed)
        {
          SleepEx(sleep_time, TRUE);
          t += slice_duration;
          if (0 < timeout)
            {
              time_elapsed = (t >= timeout);

              if (true == print_remaining_time)
                {
                  int const delta = ( (int)(timeout) - (int)(t) + 500 ) / 1000;
                  if (delta != t_remaining)
                    {
                      t_remaining = delta;
                      wprintf(gMsgMenuSelectionTimeout, t_remaining);
                    }
                  /* if */
                }
              /* if */
            }
          /* if */

          // Bring query window to foreground.
          if ( (false == time_elapsed) && ((HWND)(NULL) != hWndCommand) )
            {
              BOOL const top = BringWindowToTop(hWndCommand);
              //assert(TRUE == top);

              BOOL const activate = SetForegroundWindow(hWndCommand);
              //assert(TRUE == activate);

              HWND hWndActive = SetActiveWindow(hWndCommand);
            }
          /* if */

          continue;
        }
      /* if */

      // Get pressed key.
      wint_t const key = event_buffer.Event.KeyEvent.wVirtualKeyCode;

      switch ( key )
        {
        case (wint_t)('0'): case (wint_t)(VK_NUMPAD0): result = 0; break;
        case (wint_t)('1'): case (wint_t)(VK_NUMPAD1): result = 1; break;
        case (wint_t)('2'): case (wint_t)(VK_NUMPAD2): result = 2; break;
        case (wint_t)('3'): case (wint_t)(VK_NUMPAD3): result = 3; break;
        case (wint_t)('4'): case (wint_t)(VK_NUMPAD4): result = 4; break;
        case (wint_t)('5'): case (wint_t)(VK_NUMPAD5): result = 5; break;
        case (wint_t)('6'): case (wint_t)(VK_NUMPAD6): result = 6; break;
        case (wint_t)('7'): case (wint_t)(VK_NUMPAD7): result = 7; break;
        case (wint_t)('8'): case (wint_t)(VK_NUMPAD8): result = 8; break;
        case (wint_t)('9'): case (wint_t)(VK_NUMPAD9): result = 9; break;
        }
      /* switch */

      if (-1 != result) break;
      if (true == any_key_breaks) break;
      if (0 < timeout) time_elapsed = (t >= timeout);
    }
  while (false == time_elapsed);

  if (true == print_remaining_time) wprintf(gMsgMenuSelectionTimeoutClear);

  return result;
}
/* TimedWaitForNumberKey */



//! Wait for keypress.
/*!
  Function waits for keypress of a valid key. Waiting is done by periodically
  checking if a key was pressed until total waiting time exceeds the timeout.

  \param timeout        Maximum time to wait for the keypress in milliseconds.
  \param slice_duration     Duration of one waiting slice in milliseconds.
  \param message        Message to print if the user presses a valid key. May be NULL.
  If NULL then no message will be displayed.
  \param keys_in   Pointer to array of allowed keys. If NULL then the default "0123456789" is used.
  \param keys_in_size Number of elements in keys_in array.
  \return Returns number of pressed number key or -1 if no number key was pressed.
*/
int
TimedWaitForSelectedKeys(
                         unsigned int const timeout,
                         unsigned int const slice_duration,
                         TCHAR const * const message,
                         wint_t const * const keys_in,
                         int const keys_in_size
                         )
{
  int result = -1;

  assert( sizeof(wint_t) == sizeof(L'0') );

  wint_t const * const keys_num = (wint_t *)L"0123456789";
  int const keys_num_size = 10;

  wint_t const * const keys = (NULL == keys_in)? keys_num : keys_in;
  int const keys_size = (NULL == keys_in)? keys_num_size : keys_in_size;

  unsigned int t = 0;
  if (0 == slice_duration) t = timeout;

  // Get console handle.
  HANDLE rhnd = GetStdHandle(STD_INPUT_HANDLE);
  if (rhnd == INVALID_HANDLE_VALUE) return result;

  INPUT_RECORD event_buffer;
  
  bool key_pressed = false;
  DWORD num_read = 0;
  BOOL get_event = FALSE;

  do
    {
      // Check if a key has been pressed.
      {
        key_pressed = false;
        num_read = 0;
        get_event = FALSE;

        BOOL const peek_event = PeekConsoleInput(rhnd, &event_buffer, 1, &num_read);
        if ( (TRUE == peek_event) && (0 < num_read) )
          {
            get_event = ReadConsoleInput(rhnd, &event_buffer, 1, &num_read);
            //assert(TRUE == get_event);
          }
        /* if */
        
        if ( (TRUE == get_event) && (1 == num_read) )
          {
            key_pressed = (KEY_EVENT == event_buffer.EventType) && (TRUE == event_buffer.Event.KeyEvent.bKeyDown);
          }
        /* if */
      }

      // If no key was pressed sleep and check again.
      if (false == key_pressed)
        {
          SleepEx(slice_duration, TRUE);
          t += slice_duration;
          continue;
        }
      /* if */

      // Get pressed key.
      wint_t const key = event_buffer.Event.KeyEvent.wVirtualKeyCode;

      for (int i = 0; i < keys_size; ++i)
        {
          if (key == keys[i])
            {
              assert(-1 == result);
              result = i;
            }
          /* if */
        }
      /* for */

      if (-1 != result) break;
      if (NULL != message) wprintf(message);
    }
  while (t < timeout);

  return result;
}
/* TimedWaitForSelectedKeys */



#endif /* !__BATCHACQUISITIONKEYBOARD_CPP */
