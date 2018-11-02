//=====================================================================
// NOTEBOOK.CPP
//    WPNOTEBOOK Class window procedures.
//
// Date Created :  03/11/94
// Last Update  :  06/10/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 03/11/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Notebook control ID.
#define IDC_NOTEBOOK            1023
// Notebook colors.
#define BOOK_COLOR_BLACK        RGB(0x00,0x00,0x00)
#define BOOK_COLOR_DRK_GRAY     RGB(0x80,0x80,0x80)
#define BOOK_COLOR_GRAY         RGB(0xC0,0xC0,0xC0)
#define BOOK_COLOR_WHITE        RGB(0xFF,0xFF,0xFF)


//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
extern HBRUSH  sg_hbrGray;


//---------------------------------------------------------------------
// WPNotebookWindowProcWrapper():
//    This function acts as a "thunking" layer between the Windows 3.1
// API interface and the Workplace Shell for Windows C++ interface.
// It is called by the USER module whenever there is a message for a
// Workplace Shell Folder notebook object (or a derived class object).
// It retrieves or stores the pointer to the C++ object class and calls
// the appropriate client window procedure for the object instance.
//
// Windows 3.1 Developer's Note:
//    Before WM_CREATE message is received, 3 other messages are sent
// by CreateWindowEx().  These messages cannot be passed to the class
// client window procedure and therefore MUST be passed to the default
// Windows 3.1 window procedure.  The messages are listed below for
// reference purposes (they are in order of appearance):
//       WM_GETMINMAXINFO  -> uMsg = 0x0024
//       WM_NCCREATE       -> uMsg = 0x0081
//       WM_NCCALCSIZE     -> uMsg = 0x0083
// Then  WM_CREATE         -> uMsg = 0x0001
//       ect...
//---------------------------------------------------------------------
LRESULT CALLBACK WPNotebookWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   WPNOTEBOOK     *pobjNotebook;  // Pointer to notebook object.
   CREATESTRUCT   *pCreateStruct; // Pointer to window creation structure.

   // On the WM_CREATE message 'lParam' contains the pointer to the
   // Workplace Shell Notebook object class that owns this window.
   if (uMsg==WM_CREATE)
   {
      // Get pointer to notebook object class from create structure.
      pCreateStruct = (CREATESTRUCT *)lParam;
      pobjNotebook  = (WPNOTEBOOK *)pCreateStruct->lpCreateParams;
      // Store pointer to notebook object into extra window longs.
      SetWindowLong(hWnd,0,(LONG)pobjNotebook);
   }
   else
      // Query pointer to notebook that owns this window procedure (this pointer is refernced later).
      pobjNotebook = (WPNOTEBOOK *)GetWindowLong(hWnd,0);

    // If we do not have a valid notebook object pointer...
    if (pobjNotebook==NULL)
    {
       // Log event (for debug version).
       #ifdef _DEBUG
          dprintf("tr WPNotebookWindowProcWrapper(): Msg: 0x%04X 'pobjNotebook' is NULL.\n",uMsg);
       #endif
       // Call Windows 3.1's default window procedure.
       return (DefWindowProc(hWnd,uMsg,wParam,lParam));
    }
    else
       // Else, call the "real" client window procedure for notebook object.
       return (pobjNotebook->ClientWindowProc(hWnd,uMsg,wParam,lParam));
}


//---------------------------------------------------------------------
// ClientWindowProc():
//---------------------------------------------------------------------
LRESULT WPNOTEBOOK::ClientWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   // Process window message.
   switch (uMsg)
   {
      case WM_CREATE:
         // Store the window handle for use later.
         SetHandle(hWnd);
         // Let notebook object know that create message is being processed.
         // NOTE: Here is where folder objects have a chance to animate
         //       the zoom effect for window creation.
         OnCreate();
         // Return success so we can continue with the creation of window.
         return ((LRESULT)0);

      case WM_ACTIVATE:
         OnActivate(((wParam==WA_INACTIVE)? TRUE : FALSE));
         return (TRUE);

      // We don't want the client area to have the focus, since it
      // doesn't accept input.  When it gets the focus, which it will
      // when the window is restored from a minimized state, for
      // instance, post a user message to ourselves to reset the
      // focus to the notebook.  Posting the message is necessary
      // because the focus shouldn't be tampered with when processing
      // a WM_SETFOCUS or WM_KILLFOCUS message.
      case WM_SETFOCUS:
         PostMessage(hWnd,WM_WPS_NOTEBOOK_SETFOCUS,0,0L);
         break;

      case WM_WPS_NOTEBOOK_SETFOCUS:
         // Give focus to notebook control in our client area.
         SetFocus(GetDlgItem(hWnd,IDC_NOTEBOOK));
         break;

      case WM_COMMAND:
         if (wParam==IDC_NOTEBOOK)
            OnCommand((HWND)LOWORD(lParam),HIWORD(lParam));
         break;

      case WM_SYSCOMMAND:
         // A system command has been requested.
         OnSysCommand((WORD)wParam,lParam);
         break;

      case WM_SETTEXT:
         // Set new window caption text.
         DefWindowProc(hWnd,uMsg,wParam,lParam);
         // Update window caption.
         OnNCPaint();
         break;

      case WM_QUERYDRAGICON:
         return (OnQueryDragIcon());

      case WM_ERASEBKGND:
         OnEraseBackground((HDC)wParam);
         break;

      case WM_NCACTIVATE:
         // Handle this message ONLY if the non-client area is becoming active OR
         // the window becoming active or inactive is not a notebook page (modeless dialog).
         // else, do not update non-client area to prevent flickering effect with
         // notebook window caption.
         if ((((BOOL)wParam)==TRUE)||(IsPageWindow((HWND)LOWORD(lParam))==FALSE))
            OnNCActivate((BOOL)wParam);
         return (TRUE);

      case WM_NCPAINT:
         OnNCPaint();
         return (NULL);

      case WM_NCMOUSEMOVE:
         OnNCMouseMove((WORD)wParam,MAKEPOINT(lParam));
         break;

      case WM_NCLBUTTONUP:
      case WM_NCLBUTTONDOWN:
         OnNCLButton(uMsg,(WORD)wParam,MAKEPOINT(lParam));
         break;

      case WM_MOUSEMOVE:
         OnMouseMove();
         break;

      case WM_MOVE:
         OnMove(MAKEPOINT(lParam));
         break;

      case WM_SIZE:
         OnSize((WORD)wParam,LOWORD(lParam),HIWORD(lParam));
         break;

      case WM_CLOSE:
         OnClose();
         break;

      default:
         // Call the default window procedure.
         return (DefWindowProc(hWnd,uMsg,wParam,lParam));
  }

  // We are done processing the window message...
  return (NULL);
}


//---------------------------------------------------------------------
// WPPAGEDialogProc():
//    This dialog procedure is used by the notebook control's page
// windows.  The notebook pages are modeless dialog boxes which use
// this function as the "bridge" function between the notebook and the
// object that owns the notebook.
//    Everytime there is an event that has to be handled by the
// object owner of this notebook, this dialog procedure calls the
// function WPNOTEBOOK::CallPageProc() to handle the events.
//    CallPageProc() in turn calls the owner object's page procedure
// for the current page being processed.
//---------------------------------------------------------------------
BOOL CALLBACK _loadds WPPAGEDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   WPNOTEBOOK   *pwpnbNotebook;
   HDC          hdc;
   PAINTSTRUCT  ps;
   //char         szBuf[200];

   // Get pointer to notebook object that owns this modeless dialog (or "page").
   pwpnbNotebook = (WPNOTEBOOK *)GetWindowLong(hWnd,DWL_USER);

   //sprintf(szBuf,"WPPAGEDialogProc(): HWND = 0x%X   pwpnbNotebook = 0x%lX   uMsg = 0x%X.\n",
   //   (UINT)hWnd,(DWORD)pwpnbNotebook,(UINT)uMsg);
   //OutputDebugString(szBuf);

   // Process the modeless dialog message...
   switch (uMsg)
   {
      case WM_INITDIALOG:
         // Get pointer to notebook object from create structure.
         pwpnbNotebook = (WPNOTEBOOK *)lParam;
         // Check for invalid notebook object pointers being passed.
         if (pwpnbNotebook==NULL) return ((LRESULT)-1);
         // Store pointer to notebook object into extra dialog long.
         SetWindowLong(hWnd,DWL_USER,(DWORD)pwpnbNotebook);
         return (TRUE);

      case WM_CHILDACTIVATE:
         // The page window has been activated and we need to update
         // the global variable that holds the current page dialog.
         g_wpEnvironment.hwndCurrentModelessDialog = hWnd;
         break;

      case WM_CTLCOLOR:
         // Modify default color for certain Windows controls.
         switch ((int)HIWORD(lParam))
         {
            // Static controls.
            case CTLCOLOR_STATIC:
               SetBkColor((HDC)wParam,BOOK_COLOR_GRAY);
               return ((BOOL)sg_hbrGray);

            // Button controls.
            case CTLCOLOR_BTN:
               SetBkColor((HDC)wParam,BOOK_COLOR_GRAY);
               return ((BOOL)sg_hbrGray);

            // Button controls.
            case CTLCOLOR_DLG:
               SetBkColor((HDC)wParam,BOOK_COLOR_GRAY);
               return ((BOOL)sg_hbrGray);
         }
         break;

      case WM_WPS_DIALOG_DONE:
         // wParam         = Dialog box's ID.
         // LOWORD(lParam) = Dialog box's handle.
         pwpnbNotebook->CallPageProc(hWnd,PAGE_DIALOG_DONE,MAKELONG(LOWORD(lParam),wParam));
         break;

      case WM_COMMAND:
         // Control has been pressed, send PAGE_COMMAND message to page procedure
         // along with control's ID in the low word and notification code in high word.
         pwpnbNotebook->CallPageProc(hWnd,PAGE_COMMAND,MAKELONG(wParam,HIWORD(lParam)));
         break;

      case WM_PAINT:
         // Get page device context.
         hdc = BeginPaint(hWnd,&ps);
         // Do any custom page drawing.
         pwpnbNotebook->CallPageProc(hWnd,PAGE_PAINT,MAKELONG(hdc,0));
         // Release page device context.
         EndPaint(hWnd, &ps);
         break;

      case WM_CLOSE:
         // Destroy page dialog box.
         DestroyWindow(hWnd);
         break;

      case WM_DESTROY:
         // Check if this page being destroyed had the input focus.
         if (g_wpEnvironment.hwndCurrentModelessDialog==hWnd)
            // Reset global variable so that we do not use the page window handle.
            g_wpEnvironment.hwndCurrentModelessDialog = NULL;
         // Check that a valid notebook owner exists...
         if (pwpnbNotebook!=NULL)
            // Call the registered page procedure.         
            pwpnbNotebook->CallPageProc(hWnd,PAGE_DONE,0L);
         break;
   }

   // We are done processing window message.
   return (FALSE);
}


// EOF NOTEBOOK.CPP

