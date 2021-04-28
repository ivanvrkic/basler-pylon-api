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
  \file   BatchAcquisitionDebug.cpp
  \brief  Helper functions for easier debugging.

  \author Tomislav Petkovic
  \date   2017-01-03
*/


#include "BatchAcquisitionStdAfx.h"


#ifndef __BATCHACQUISITIONDEBUG_CPP
#define __BATCHACQUISITIONDEBUG_CPP


#include "BatchAcquisitionMessages.h"
#include "BatchAcquisitionDebug.h"



//! Set thread name for MSVC.
/*!
  Function sets thread name for MSVC debugger.

  \param dwThreadID     Thread ID. Set to -1 if the name is intended for the caller thread.
  \param threadName     Thread name.
*/
void
SetThreadNameForMSVC(
                     DWORD const dwThreadID,
                     char const * const threadName
                     )
{

  assert(NULL != threadName);
  if (NULL == threadName) return;

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

#ifdef _DEBUG

  __try
    {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)(&info) );
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
  /* __try */

#endif /* _DEBUG */

}
/* SetThreadNameForMSVC */



//! Set thread name and ID for MSVC.
/*!
  Function sets thread name and ID for MSVC debugger.

  \param dwThreadID     Thread ID. Set to -1 if the name is intended for the caller thread.
  \param threadName     Thread name.
  \param threadID       Number to be added after thread name.
*/
void
SetThreadNameAndIDForMSVC(
                          DWORD const dwThreadID,
                          char const * const threadName,
                          int const threadID
                          )
{

  assert(NULL != threadName);
  if (NULL == threadName) return;

  char buffer[4096];
  int const count = sprintf_s(buffer, 4096, "%s%d", threadName, threadID);
  buffer[4095] = 0;

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = buffer;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

#ifdef _DEBUG

  __try
    {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)(&info) );
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
  /* __try */

#endif /* _DEBUG */

}
/* SetThreadNameAndIDForMSVC */



/****** THREAD STATE ******/

//! Blanks PastEvent structure.
/*!
  Blanks structure.

  \param ptr    Pointer to structure.
*/
void
PastEventsBlank_inline(
                       PastEvents * const ptr
                       )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->ticks_to_ms = BATCHACQUISITION_qNaN_dv;
  ptr->idx = 0;
  for (int i = 0; i < ptr->num_codes; ++i)
    {
      ptr->event_data[i].code = -1;
      ptr->event_data[i].duration = BATCHACQUISITION_qNaN_dv;
      ptr->event_data[i].elapsed = BATCHACQUISITION_qNaN_dv;
      ptr->event_data[i].QPC_added = -1;
      ptr->event_data[i].QPC_processed = -1;
    }
  /* for */
}
/* PastEventsBlank_inline */



//! Deletes past events storage.
/*!
  Deletes PastEvents structure.

  \param ptr    Pointer to PastEvents structure.
*/
void
PastEventsDelete(
                 PastEvents * const ptr
                 )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;
  PastEventsBlank_inline(ptr);
  free(ptr);
}
/* PastEventsDelete */



//! Creates past events storage.
/*!
  Creates structure to store past events.

  \return Returns pointer to PastEvents structure.
*/
PastEvents *
PastEventsCreate(
                 void
                 )
{
  PastEvents * const ptr = (PastEvents *)malloc( sizeof(PastEvents) );
  assert(NULL != ptr);
  PastEventsBlank_inline(ptr);

  if (NULL != ptr)
    {
      LARGE_INTEGER frequency;
      BOOL const res = QueryPerformanceFrequency( &(frequency) );
      assert(FALSE != res);

      ptr->ticks_to_ms = 1000.0 / (double)(frequency.QuadPart);
    }
  /* if */

  return ptr;
}
/* PastEventsCreate */



//! Add event to storage.
/*!
  Adds event to event storage.

  \param ptr    Pointer to event storage structure.
  \param event  Event ID.
*/
void
AddEvent(
         PastEvents * const ptr,
         int const event
         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  int const idx = ptr->idx;
  assert((0 <= idx) && (idx < ptr->num_codes));

  ptr->event_data[idx].code = event;
  ptr->event_data[idx].duration = BATCHACQUISITION_qNaN_dv;
  ptr->event_data[idx].QPC_processed = -1;

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE == res)
    {
      ptr->event_data[idx].QPC_added = ticks.QuadPart;
      int idx_prev = idx - 1;
      if (0 > idx_prev) idx_prev = ptr->num_codes - 1;
      double const elapsed = (double)(ticks.QuadPart - ptr->event_data[idx_prev].QPC_added) * ptr->ticks_to_ms;
      ptr->event_data[idx].elapsed = elapsed;
    }
  else
    {
      ptr->event_data[idx].QPC_added = (LONGLONG)0;
      ptr->event_data[idx].elapsed = BATCHACQUISITION_qNaN_dv;
    }
  /* if */

  ptr->idx = ptr->idx + 1;
  ptr->idx = ptr->idx % ptr->num_codes;
}
/* AddEvent */



//! Add event processing time.
/*!
  Add event processing time.

  \param ptr    Pointer to event storage structure.
  \return Returns true if successfull.
*/
void
EventProcessed(
               PastEvents * const ptr
               )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  int idx = ptr->idx - 1;
  if (0 > idx) idx = ptr->num_codes - 1;
  assert((0 <= idx) && (idx < ptr->num_codes));

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE == res)
    {
      assert(-1 == ptr->event_data[idx].QPC_processed);
      assert(true == isnan_inline(ptr->event_data[idx].duration));
      ptr->event_data[idx].QPC_processed = ticks.QuadPart;
      double const duration = (double)(ticks.QuadPart - ptr->event_data[idx].QPC_added) * ptr->ticks_to_ms;
      ptr->event_data[idx].duration = duration;
    }
  else
    {
      ptr->event_data[idx].QPC_processed = -1;
      ptr->event_data[idx].duration = BATCHACQUISITION_qNaN_dv;
    }
  /* if */
}
/* EventProcessed */



//! Add event processing time to previous event.
/*!
  Add event processing time to previous event.

  \param ptr    Pointer to event storage structure.
  \return Returns true if successfull.
*/
void
PreviousEventProcessed(
                       PastEvents * const ptr
                       )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  int idx = ptr->idx - 2;
  if (0 > idx) idx = idx + ptr->num_codes;
  assert((0 <= idx) && (idx < ptr->num_codes));

  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE == res)
    {
      ptr->event_data[idx].QPC_processed = ticks.QuadPart;
      double const duration = (double)(ticks.QuadPart - ptr->event_data[idx].QPC_added) * ptr->ticks_to_ms;
      ptr->event_data[idx].duration = duration;
    }
  else
    {
      ptr->event_data[idx].QPC_processed = (LONGLONG)0;
      ptr->event_data[idx].duration = BATCHACQUISITION_qNaN_dv;
    }
  /* if */
}
/* PreviousEventProcessed */



//! Get current event.
/*!
  Returns current event ID.

  \param ptr    Pointer to event storage structure.
  \param code  Address where event ID will be stored.
  \param duration_ms Address where event duration in ms will be stored.
  \param QPC_added   Address where the QPC time when event processing started will be stored.
  \param QPC_processed   Address where the QPC time when event was processed will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
GetCurrentEvent(
                PastEvents * const ptr,
                int * const code,
                double * const duration_ms,
                LONGLONG * const QPC_added,
                LONGLONG * const QPC_processed
                )
{
  assert(NULL != ptr);
  if (NULL == ptr) return false;

  assert((0 <= ptr->idx) && (ptr->idx < ptr->num_codes));
  int idx = ptr->idx;
  if (0 < idx)
    {
      idx = idx - 1;
    }
  else if (0 == idx)
    {
      idx = ptr->num_codes - 1;
    }
  else
    {
      return false;
    }
  /* if */

  idx = idx % ptr->num_codes;
  if (NULL != code) *code = ptr->event_data[idx].code;
  if (NULL != duration_ms) *duration_ms = ptr->event_data[idx].duration;
  if (NULL != QPC_added) *QPC_added = ptr->event_data[idx].QPC_added;
  if (NULL != QPC_processed) *QPC_processed = ptr->event_data[idx].QPC_processed;

  return true;
}
/* GetCurrentEvent */



//! Get previous event.
/*!
  Returns previous event ID.

  \param ptr    Pointer to event storage structure.
  \param code  Address where event ID will be stored.
  \param duration_ms Address where event duration in ms will be stored.
  \param QPC_added   Address where the QPC time when event processing started will be stored.
  \param QPC_processed   Address where the QPC time when event was processed will be stored.
  \return Returns true if successfull, false otherwise.
*/
bool
GetPreviousEvent(
                 PastEvents * const ptr,
                 int * const code,
                 double * const duration_ms,
                 LONGLONG * const QPC_added,
                 LONGLONG * const QPC_processed
                 )
{
  assert(NULL != ptr);
  if (NULL == ptr) return false;

  assert((0 <= ptr->idx) && (ptr->idx < ptr->num_codes));
  int idx = ptr->idx;
  if (1 < idx)
    {
      idx = idx - 2;
    }
  else if ( (0 == idx) || (1 == idx) )
    {
      idx = ptr->num_codes - 2 + ptr->idx;
    }
  else
    {
      return false;
    }
  /* if */

  idx = idx % ptr->num_codes;
  if (NULL != code) *code = ptr->event_data[idx].code;
  if (NULL != duration_ms) *duration_ms = ptr->event_data[idx].duration;
  if (NULL != QPC_added) *QPC_added = ptr->event_data[idx].QPC_added;
  if (NULL != QPC_processed) *QPC_processed = ptr->event_data[idx].QPC_processed;

  return true;
}
/* GetPreviousEvent */



/****** WINDOW MESSAGES ******/

//! Event names.
/*!
  This static array contains event names of MS Window messages.
*/
static
TCHAR const * const WindowMessageNames[] = {
  L"WM_NULL", // 0
  L"WM_CREATE", // 1
  L"WM_DESTROY", // 2
  L"WM_MOVE", // 3
  L"WM_SIZE", // 4
  L"WM_ACTIVATE", // 5
  L"WM_SETFOCUS", // 6
  L"WM_KILLFOCUS", // 7
  L"WM_ENABLE", // 8
  L"WM_SETREDRAW", // 9
  L"WM_SETTEXT", // 10
  L"WM_GETTEXT", // 11
  L"WM_GETTEXTLENGTH", // 12
  L"WM_PAINT", // 13
  L"WM_CLOSE ", // 14
  L"WM_QUERYENDSESSION", // 15
  L"WM_QUIT", // 16
  L"WM_ERASEBKGND", // 17
  L"WM_WINDOWPOSCHANGING ", // 18
  L"WM_WINDOWPOSCHANGED", // 19
  L"WM_GETMINMAXINFO", // 20
  L"WM_NCCALCSIZE", // 21
  L"WM_NCHITTEST", // 22
  L"WM_NCPAINT", // 23
  L"WM_NCACTIVATE", // 24
  L"WM_ACTIVATEAPP", // 25
  L"WM_IME_SETCONTEXT", // 26
  L"WM_IME_NOTIFY", // 27
  L"WM_GETICON", // 28
  L"WM_SETICON", // 29
  L"WM_NCMOUSEMOVE", // 30
  L"WM_SETCURSOR", // 31
  L"WM_MOUSEACTIVATE", // 32
  L"WM_MOVING", // 33
  L"WM_MOUSEMOVE", // 34
  L"WM_LBUTTONDOWN", // 35
  L"WM_LBUTTONUP", // 36
  L"WM_LBUTTONDBLCLK", // 37
  L"WM_RBUTTONDOWN", // 38
  L"WM_RBUTTONUP", // 39
  L"WM_RBUTTONDBLCLK", // 40
  L"WM_MBUTTONDOWN", // 41
  L"WM_MBUTTONUP", // 42
  L"WM_MBUTTONDBLCLK", // 43
  L"WM_MOUSELAST", // 44
  L"WM_MOUSEWHEEL", // 45
  L"WM_NCMOUSELEAVE", // 46
  L"WM_XBUTTONDOWN", // 47
  L"WM_XBUTTONUP", // 48
  L"WM_XBUTTONDBLCLK", // 49
  L"WM_ENTERSIZEMOVE", // 50
  L"WM_EXITSIZEMOV", // 51
  L"WM_NCLBUTTONDOWN", // 52
  L"WM_NCLBUTTONUP", // 53
  L"WM_NCLBUTTONDBLCLK", // 54
  L"WM_NCRBUTTONDOWN", // 55
  L"WM_NCRBUTTONUP", // 56
  L"WM_NCRBUTTONDBLCLK", // 57
  L"WM_NCMBUTTONDOWN", // 58
  L"WM_NCMBUTTONUP", // 59
  L"WM_NCMBUTTONDBLCLK", // 60
  L"WM_NCXBUTTONDOWN", // 61
  L"WM_NCXBUTTONUP", // 62
  L"WM_NCXBUTTONDBLCLK", // 63
  L"WM_COMMAND", // 64
  L"WM_SYSCOMMAND", // 65
  L"WM_SIZING", // 66
  L"WM_CAPTURECHANGED", // 67
  L"WM_NCUAHDRAWCAPTION", // 68
  L"WM_NCUAHDRAWFRAME", // 69
  L"WM_INPUT", // 70
  L"WM_KEYDOWN", // 71
  L"WM_KEYUP", // 72
  L"WM_CHAR", // 73
  L"WM_DEADCHAR", // 74
  L"WM_SYSKEYDOWN", // 75
  L"WM_SYSKEYUP", // 76
  L"WM_SYSCHAR", // 77
  L"WM_SYSDEADCHAR", // 77
  L"WM_UNICHAR", // 79
  L"WM_NCDESTROY", // 80
  L"WM_LPKDRAWSWITCHWND", // 81
  L"WM_CONTEXTMENU", // 82
  L"WM_STYLECHANGING", // 83
  L"WM_STYLECHANGED", // 84
  L"WM_DISPLAYCHANGE", // 85
  L"WM_SYNCPAINT", // 86
  L"WM_DWMCOMPOSITIONCHANGED", // 87
  L"WM_DWMNCRENDERINGCHANGED", // 88
  L"WM_DWMWINDOWMAXIMIZEDCHANGE", // 89
  L"WM_DWMEXILEFRAME", // 90
  L"WM_UAHDESTROYWINDOW", // 91
  L"WM_UAHDRAWMENU", // 92
  L"WM_UAHDRAWMENUITEM", // 93
  L"WM_UAHINITMENU", // 94
  L"WM_UAHMEASUREMENUITEM", // 95
  L"WM_UAHNCPAINTMENUPOPUP", // 96
  L"WM_UAHUPDATE", // 97
  L"WM_CANCELMODE", // 98
  L"WM_GETTITLEBARINFOEX", // 99
  L"WM_WININICHANGE", // 100
  L"WM_PARENTNOTIFY", // 101
  L"WM_ENTERMENULOOP", // 102
  L"WM_EXITMENULOOP", // 103
  L"WM_NEXTMENU", // 104
  L"WM_INITMENU", // 105
  L"WM_INITMENUPOPUP", // 106
  L"WM_TIMER", // 107
  L"WM_HSCROLL", // 108
  L"WM_VSCROLL", // 109
  L"WM_MENUSELECT", // 110
  L"WM_MENUCHAR", // 111
  L"WM_QUERYOPEN", // 112
};



//! Get event name.
/*!
  Function returns pointer to a string which contains message name or NULL if message name is unknown.

  \param message MS Windows message identifier.
  \param print Reference where flag indicating whether to output the message or not will be stored.
  \return Returns pointer to string or NULL pointer if message is unknown.
*/
inline
TCHAR const *
GetWindowMessageName_inline(
                            UINT const message,
                            bool & print
                            )
{
  switch (message)
    {
    case WM_NULL:                     print = false; return WindowMessageNames[  0];
    case WM_CREATE:                   print = true;  return WindowMessageNames[  1];
    case WM_DESTROY:                  print = true;  return WindowMessageNames[  2];
    case WM_MOVE:                     print = false; return WindowMessageNames[  3];
    case WM_SIZE:                     print = false; return WindowMessageNames[  4];
    case WM_ACTIVATE:                 print = false; return WindowMessageNames[  5];
    case WM_SETFOCUS:                 print = true;  return WindowMessageNames[  6];
    case WM_KILLFOCUS:                print = true;  return WindowMessageNames[  7];
    case WM_ENABLE:                   print = true;  return WindowMessageNames[  8];
    case WM_SETREDRAW:                print = true;  return WindowMessageNames[  9];
    case WM_SETTEXT:                  print = true;  return WindowMessageNames[ 10];
    case WM_GETTEXT:                  print = true;  return WindowMessageNames[ 11];
    case WM_GETTEXTLENGTH:            print = true;  return WindowMessageNames[ 12];
    case WM_PAINT:                    print = false; return WindowMessageNames[ 13];
    case WM_CLOSE:                    print = true;  return WindowMessageNames[ 14];
    case WM_QUERYENDSESSION:          print = true;  return WindowMessageNames[ 15];
    case WM_QUIT:                     print = true;  return WindowMessageNames[ 16];
    case WM_ERASEBKGND:               print = true;  return WindowMessageNames[ 17];
    case WM_WINDOWPOSCHANGING:        print = false; return WindowMessageNames[ 18];
    case WM_WINDOWPOSCHANGED:         print = false; return WindowMessageNames[ 19];
    case WM_GETMINMAXINFO:            print = false; return WindowMessageNames[ 20];
    case WM_NCCALCSIZE:               print = true;  return WindowMessageNames[ 21];
    case WM_NCHITTEST:                print = false; return WindowMessageNames[ 22];
    case WM_NCPAINT:                  print = false; return WindowMessageNames[ 23];
    case WM_NCACTIVATE:               print = false; return WindowMessageNames[ 24];
    case WM_ACTIVATEAPP:              print = false; return WindowMessageNames[ 25];
    case WM_IME_SETCONTEXT:           print = false; return WindowMessageNames[ 26];
    case WM_IME_NOTIFY:               print = false; return WindowMessageNames[ 27];
    case WM_GETICON:                  print = false; return WindowMessageNames[ 28];
    case WM_SETICON:                  print = false; return WindowMessageNames[ 29];
    case WM_NCMOUSEMOVE:              print = false; return WindowMessageNames[ 30];
    case WM_SETCURSOR:                print = false; return WindowMessageNames[ 31];
    case WM_MOUSEACTIVATE:            print = false; return WindowMessageNames[ 32];
    case WM_MOVING:                   print = false; return WindowMessageNames[ 33];
    case WM_MOUSEMOVE:                print = false; return WindowMessageNames[ 34];
    case WM_LBUTTONDOWN:              print = false; return WindowMessageNames[ 35];
    case WM_LBUTTONUP:                print = false; return WindowMessageNames[ 36];
    case WM_LBUTTONDBLCLK:            print = false; return WindowMessageNames[ 37];
    case WM_RBUTTONDOWN:              print = false; return WindowMessageNames[ 38];
    case WM_RBUTTONUP:                print = false; return WindowMessageNames[ 39];
    case WM_RBUTTONDBLCLK:            print = false; return WindowMessageNames[ 40];
    case WM_MBUTTONDOWN:              print = false; return WindowMessageNames[ 41];
    case WM_MBUTTONUP:                print = false; return WindowMessageNames[ 42];
    case WM_MBUTTONDBLCLK:            print = false; return WindowMessageNames[ 43];
    case WM_MOUSELAST:                print = false; return WindowMessageNames[ 44];
    case WM_MOUSEWHEEL:               print = false; return WindowMessageNames[ 45];
    case WM_NCMOUSELEAVE:             print = false; return WindowMessageNames[ 46];
    case WM_XBUTTONDOWN:              print = false; return WindowMessageNames[ 47];
    case WM_XBUTTONUP:                print = false; return WindowMessageNames[ 48];
    case WM_XBUTTONDBLCLK:            print = false; return WindowMessageNames[ 49];
    case WM_ENTERSIZEMOVE:            print = true;  return WindowMessageNames[ 50];
    case WM_EXITSIZEMOVE:             print = true;  return WindowMessageNames[ 51];
    case WM_NCLBUTTONDOWN:            print = false; return WindowMessageNames[ 52];
    case WM_NCLBUTTONUP:              print = false; return WindowMessageNames[ 53];
    case WM_NCLBUTTONDBLCLK:          print = false; return WindowMessageNames[ 54];
    case WM_NCRBUTTONDOWN:            print = false; return WindowMessageNames[ 55];
    case WM_NCRBUTTONUP:              print = false; return WindowMessageNames[ 56];
    case WM_NCRBUTTONDBLCLK:          print = false; return WindowMessageNames[ 57];
    case WM_NCMBUTTONDOWN:            print = false; return WindowMessageNames[ 58];
    case WM_NCMBUTTONUP:              print = false; return WindowMessageNames[ 59];
    case WM_NCMBUTTONDBLCLK:          print = false; return WindowMessageNames[ 60];
    case WM_NCXBUTTONDOWN:            print = false; return WindowMessageNames[ 61];
    case WM_NCXBUTTONUP:              print = false; return WindowMessageNames[ 62];
    case WM_NCXBUTTONDBLCLK:          print = false; return WindowMessageNames[ 63];
    case WM_COMMAND:                  print = true;  return WindowMessageNames[ 64];
    case WM_SYSCOMMAND:               print = true;  return WindowMessageNames[ 65];
    case WM_SIZING:                   print = true;  return WindowMessageNames[ 66];
    case WM_CAPTURECHANGED:           print = true;  return WindowMessageNames[ 67];
    case 0x00AE:                      print = true;  return WindowMessageNames[ 68]; // WM_NCUAHDRAWCAPTION
    case 0x00AF:                      print = true;  return WindowMessageNames[ 69]; // WM_NCUAHDRAWFRAME
    case WM_INPUT:                    print = true;  return WindowMessageNames[ 70];
    case WM_KEYDOWN:                  print = true;  return WindowMessageNames[ 71]; // WM_KEYFIRST
    case WM_KEYUP:                    print = true;  return WindowMessageNames[ 72];
    case WM_CHAR:                     print = true;  return WindowMessageNames[ 73];
    case WM_DEADCHAR:                 print = true;  return WindowMessageNames[ 74];
    case WM_SYSKEYDOWN:               print = true;  return WindowMessageNames[ 75];
    case WM_SYSKEYUP:                 print = true;  return WindowMessageNames[ 76];
    case WM_SYSCHAR:                  print = true;  return WindowMessageNames[ 77];
    case WM_SYSDEADCHAR:              print = true;  return WindowMessageNames[ 78];
    case WM_UNICHAR:                  print = true;  return WindowMessageNames[ 79]; // WM_KEYLAST
    case WM_NCDESTROY:                print = true;  return WindowMessageNames[ 80];
    case 0x008C:                      print = true;  return WindowMessageNames[ 81]; // WM_LPKDRAWSWITCHWND
    case WM_CONTEXTMENU:              print = true;  return WindowMessageNames[ 82];
    case WM_STYLECHANGING:            print = true;  return WindowMessageNames[ 83];
    case WM_STYLECHANGED:             print = true;  return WindowMessageNames[ 84];
    case WM_DISPLAYCHANGE:            print = true;  return WindowMessageNames[ 85];
    case WM_SYNCPAINT:                print = true;  return WindowMessageNames[ 86];
    case WM_DWMCOMPOSITIONCHANGED:    print = true;  return WindowMessageNames[ 87];
    case WM_DWMNCRENDERINGCHANGED:    print = true;  return WindowMessageNames[ 88];
    case WM_DWMWINDOWMAXIMIZEDCHANGE: print = true;  return WindowMessageNames[ 89];
    case 0x0322:                      print = true;  return WindowMessageNames[ 90]; // WM_DWMEXILEFRAME
    case 0x0090:                      print = true;  return WindowMessageNames[ 91]; // WM_UAHDESTROYWINDOW
    case 0x0091:                      print = true;  return WindowMessageNames[ 92]; // WM_UAHDRAWMENU
    case 0x0092:                      print = true;  return WindowMessageNames[ 93]; // WM_UAHDRAWMENUITEM
    case 0x0093:                      print = true;  return WindowMessageNames[ 94]; // WM_UAHINITMENU
    case 0x0094:                      print = true;  return WindowMessageNames[ 95]; // WM_UAHMEASUREMENUITEM
    case 0x0095:                      print = true;  return WindowMessageNames[ 96]; // WM_UAHNCPAINTMENUPOPUP
    case 0x0096:                      print = true;  return WindowMessageNames[ 97]; // WM_UAHUPDATE
    case WM_CANCELMODE:               print = true;  return WindowMessageNames[ 98];
    case WM_GETTITLEBARINFOEX:        print = false; return WindowMessageNames[ 99];
    case WM_WININICHANGE:             print = true;  return WindowMessageNames[100];
    case WM_PARENTNOTIFY:             print = true;  return WindowMessageNames[101];
    case WM_ENTERMENULOOP:            print = true;  return WindowMessageNames[102];
    case WM_EXITMENULOOP:             print = true;  return WindowMessageNames[103];
    case WM_NEXTMENU:                 print = true;  return WindowMessageNames[104];
    case WM_INITMENU:                 print = true;  return WindowMessageNames[105];
    case WM_INITMENUPOPUP:            print = true;  return WindowMessageNames[106];
    case WM_TIMER:                    print = true;  return WindowMessageNames[107];
    case WM_HSCROLL:                  print = true;  return WindowMessageNames[108];
    case WM_VSCROLL:                  print = true;  return WindowMessageNames[109];
    case WM_MENUSELECT:               print = true;  return WindowMessageNames[110];
    case WM_MENUCHAR:                 print = true;  return WindowMessageNames[111];
    case WM_QUERYOPEN:                print = true;  return WindowMessageNames[112];
    }
  /* switch */
  print = true; return NULL;
}
/* GetWindowMessageName_inline */



//! Outputs received window message.
/*!
  Function prints received window message to console.

  \param hWnd   A handle to the window procedure that received the message.
  \param message        Received message.
  \param wParam  Additional message information.
  \param lParam Additional message information.
*/
void
PrintWindowMessageToConsole(
                            HWND const hWnd,
                            UINT const message,
                            WPARAM const wParam,
                            LPARAM const lParam
                            )
{
  bool print = true;
  int const sz = 16;
  TCHAR buffer[sz + 1];
  TCHAR const * messageID = GetWindowMessageName_inline(message, print);
  if (false == print) return;

  if (NULL == messageID)
    {
      int const cnt = swprintf_s(buffer, sz, _T("%u"), message);
      assert(0 < cnt);

      messageID = buffer;
    }
  /* if */
  assert(NULL != messageID);

  Debugfwprintf(stderr, gDbgReceivedWindowMessage, hWnd, messageID);
}
/* PrintWindowMessageToConsole */



//! Blanks PastMessages structure.
/*!
  Blanks structure.

  \param ptr    Pointer to structure.
*/
void
PastMessagesBlank_inline(
                         PastMessages * const ptr
                         )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  ptr->idx = 0;
  for (int i = 0; i < ptr->num_messages; ++i)
    {
      ptr->message[i] = 0;
      ptr->wParam[i] = 0;
      ptr->lParam[i] = 0;
    }
  /* for */
}
/* PastMessagesBlank_inline */



//! Deletes past messages storage.
/*!
  Deletes PastMessages structure.

  \param ptr    Pointer to PastMessages structure.
*/
void
PastMessagesDelete(
                   PastMessages * const ptr
                   )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;
  PastMessagesBlank_inline(ptr);
  free(ptr);
}
/* PastMessagesDelete */



//! Creates past messages storage.
/*!
  Creates structure to store messages events.

  \return Returns pointer to PastMessages structure.
*/
PastMessages *
PastMessagesCreate(
                   void
                   )
{
  PastMessages * const ptr = (PastMessages *)malloc( sizeof(PastMessages) );
  assert(NULL != ptr);
  PastMessagesBlank_inline(ptr);
  return ptr;
}
/* PastMessagesCreate */



//! Add message to storage.
/*!
  Adds message to message storage.

  \param ptr    Pointer to message storage structure.
  \param message Message ID.
  \param wParam  Message parameter.
  \param lParam  Message parameter.
*/
void
AddMessage(
           PastMessages * const ptr,
           UINT const message,
           WPARAM const wParam,
           LPARAM const lParam
           )
{
  assert(NULL != ptr);
  if (NULL == ptr) return;

  int const idx = ptr->idx;
  assert((0 <= idx) && (idx < ptr->num_messages));

  ptr->message[idx] = message;
  ptr->wParam[idx] = wParam;
  ptr->lParam[idx] = lParam;

  ptr->idx = ptr->idx + 1;
  ptr->idx = ptr->idx % ptr->num_messages;
}
/* AddMessage */



/****** MESSAGE OUTPUT AND SIGNAL TESTING ******/

//! Prints debug message.
/*!
  Outputs debug message to debugger console and to supplied output stream.

  \param stream Pointer to output stream (e.g. stdout or stderr).
  If NULL message will be output to debugger console only.
  \param format Format string for output.
  \return Function returns number of characters written.
*/
int
Debugfprintf(
             FILE * stream,
             char const * const format,
             ...
             )
{
  va_list argptr;
  char buffer[4096];
  TCHAR Tbuffer[4096];

  assert(NULL != format);
  int count = 0;

  /* Wrap the call to vsprintf_s and print the message to temporary buffer. */
  va_start(argptr, format);
  if (NULL != format) count = vsprintf_s(buffer, 4096, format, argptr);
  va_end(argptr);

  /* Output to debugger. */
  for (int i = 0; i < 4096; ++i) Tbuffer[i] = (TCHAR)( buffer[i] );
  Tbuffer[4095] = 0;
  OutputDebugString(Tbuffer);

  /* Output to console. */
  FILE * output = stream;
  if (NULL != output) fprintf(output, "%s", buffer);

  return( count );
}
/* Debugfprintf */



//! Prints debug message.
/*!
  Outputs debug message to debugger console and to supplied output stream.

  \param stream Pointer to output stream (e.g. stdout or stderr).
  If NULL message will be output to debugger console only.
  \param format Format string for output.
  \return Function returns number of characters written.
*/
int
Debugfwprintf(
              FILE * stream,
              wchar_t const * const format,
              ...
              )
{
  va_list argptr;
  wchar_t buffer[4096];

  assert(NULL != format);
  int count = 0;

  /* Wrap the call to vsprintf_s and print the message to temporary buffer. */
  va_start(argptr, format);
  if (NULL != format) count = vswprintf_s(buffer, 4096, format, argptr);
  va_end(argptr);

  /* Output to debugger. */
  OutputDebugString(buffer);

  /* Output to console. */
  FILE * output = stream;
  if (NULL != output) fwprintf(output, L"%s", buffer);

  return( count );
}
/* Debugfwprintf */



//! Check singla state.
/*!
  Checks if named event is in signalled state. Note that event
  must not be auto reset event for the function to work correctly.

  \param ptr Pointer to synchronization class.
  \param name   Name of the event.
  \param H     Event index. This index should correspond to the structure index in std::vector array of SynchronizationEvents_ structure.
  \return Returns true if event is signalled.
*/
bool
DebugIsSignalled(
                 SynchronizationEvents * const ptr,
                 SynchronizationCodes const name,
                 int const H
                 )
{
  assert(NULL != ptr);
  if (NULL == ptr) return false;

  DWORD const signalled = ptr->EventWaitFor( name, H, (DWORD)0 );
  return (WAIT_OBJECT_0 == signalled);
}
/* DebugIsSignalled */




/****** TIME MEASUREMENT ******/

//! Records current time.
/*!
  Records current time in the timer structure.

  \return Returns pointer to the timer structure.
*/
DEBUG_TIMER *
DebugTimerInit(
               void
               )
{
  DEBUG_TIMER * const D = (DEBUG_TIMER *)malloc( sizeof(DEBUG_TIMER) );
  assert(NULL != D);
  if (NULL == D) return( NULL );

  /* Get CPU frequency. */
  {
    BOOL const res = QueryPerformanceFrequency( &(D->frequency) );
    assert(FALSE != res);
  }

  D->invfrq = 1000.0 / (double)(D->frequency.QuadPart);

  /* Get start time. */
  {
    LARGE_INTEGER ticks;
    BOOL const res = QueryPerformanceCounter( &ticks );
    assert(FALSE != res);

    if (FALSE != res)
      {
        D->clockStart = ticks;
        D->clockStop = ticks;
      }
    else
      {
        D->clockStart.QuadPart = 0;
        D->clockStop.QuadPart = 0;
      }
    /* if */
  }

  /* Set tic and toc values. */
  D->tic.QuadPart = 0;
  D->toc.QuadPart = 0;

  /* No time elapsed. */
  D->elapsed = BATCHACQUISITION_qNaN_dv;

  return( D );
}
/* DebugTimerInit */



//! Returns elapsed time from start.
/*!
  Returns elapsed time from the timer start, i.e. from the call to DebugTimerInit.

  \param D      Pointer to timer structure.
  \return Elapsed time in ms or NaN if unsuccessfull.
*/
double
DebugTimerQueryStart(
                     DEBUG_TIMER * const D
                     )
{
  assert(NULL != D);
  if (NULL == D) return BATCHACQUISITION_qNaN_dv;

  /* Fetch current time. */
  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return BATCHACQUISITION_qNaN_dv;

  /* Store current time. */
  D->clockStop = ticks;

  /* Compute elapsed time. */
  D->elapsed = (double)(D->clockStop.QuadPart - D->clockStart.QuadPart) * D->invfrq;

  return( D->elapsed );
}
/* DebugTimerQueryStart */



//! Return elapsed time from last query.
/*!
  Returns elapsed time from last timer query, i.e. call to either
  DebugTimerInit or DebugTimerQueryStart.

  \param D      Pointer to timer structure.
  \return Elapsed time in ms.
*/
double
DebugTimerQueryLast(
                    DEBUG_TIMER * const D
                    )
{
  assert(NULL != D);
  if (NULL == D) return BATCHACQUISITION_qNaN_dv;

  /* Fetch current time. */
  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return BATCHACQUISITION_qNaN_dv;

  /* Compute elapsed time. */
  D->elapsed = (double)(ticks.QuadPart - D->clockStop.QuadPart) * D->invfrq;

  /* Store current time. */
  D->clockStop = ticks;

  return( D->elapsed );
}
/* DebugTimerQueryLast */



//! Store current QPC time.
/*!
  Stores curret QPC time into tic variable.
  This effectively enables time measurement using the DebugQueryTimerToc function.

  \param D Pointer to timer structure.
*/
void
DebugTimerQueryTic(
                   DEBUG_TIMER * const D
                   )
{
  assert(NULL != D);
  if (NULL == D) return;

  /* Fetch current time. */
  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return;

  D->tic = ticks;

  return;
}
/* DebugTimerQueryTic */



//! Return elapsed time from last tic.
/*!
  Fetches current QPC time and computes elapsed time in ms from the last call to
  DebugTimerQueryTic.

  \param D      Pointer to timer structure.
  \return Returns elapsed time in ms or NaN if DebugTimerQueryTic was not called.
*/
double
DebugTimerQueryToc(
                   DEBUG_TIMER * const D
                   )
{
  double elapsed = BATCHACQUISITION_qNaN_dv;

  assert(NULL != D);
  if (NULL == D) return elapsed;

  /* Fetch current time. */
  LARGE_INTEGER ticks;
  BOOL const res = QueryPerformanceCounter( &ticks );
  assert(TRUE == res);
  if (TRUE != res) return elapsed;

  /* Store current time. */
  D->toc = ticks;

  /* Compute elapsed time. */
  if ( (0 < D->tic.QuadPart) && (0 < D->toc.QuadPart) )
    {
      elapsed = (double)(ticks.QuadPart - D->tic.QuadPart) * D->invfrq;
    }
  /* if */

  return elapsed;
}
/* DebugTimerQueryToc */



//! Destroy timer structure.
/*!
  Deallocates memory associated with the timer structure.

  \param D      Pointer to timer structure.
*/
void
DebugTimerDestroy(
                  DEBUG_TIMER * const D
                  )
{
  assert(NULL != D);
  if (NULL == D) return;

  free( D );
}
/* DebugTimerDestroy */



/****** BREAK ON TIME-CRITICAL ACTIONS ******/

#if defined(_DEBUG) || defined(DEBUG)

//! Time the wait operation.
/*!
  This function times the EnterCriticalSection operation and breaks
  program execution if the allowed time is exceeded.

  \param lpCriticalSection      A pointer to the critical section object.
  \param timeout        Allowed time in ticks to wait for ownership of the critical section object.
  \param stop_execution Flag to indicate debugger should be invoked if the time limit is exceeded.
*/
void
DebugEnterCriticalSection(
                          LPCRITICAL_SECTION lpCriticalSection,
                          LARGE_INTEGER const timeout,
                          bool const stop_execution
                          )
{
  if (true == stop_execution)
    {
      LARGE_INTEGER ticks1, ticks2;

      BOOL const res1 = QueryPerformanceCounter( &ticks1 );
      assert(TRUE == res1);

      EnterCriticalSection(lpCriticalSection);

      BOOL const res2 = QueryPerformanceCounter( &ticks2 );
      assert(TRUE == res2);

      assert(ticks2.QuadPart >= ticks1.QuadPart);
      assert(ticks2.QuadPart - ticks1.QuadPart < timeout.QuadPart);
    }
  else
    {
      EnterCriticalSection(lpCriticalSection);
    }
  /* if */
}
/* DebugEnterCriticalSection */

#endif



#endif /* !__BATCHACQUISITIONDEBUG_CPP */
