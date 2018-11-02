//=====================================================================
// PALETTE.CPP
//    WPPALETTE class window procedure. 
// 
// Date Created :  01/04/95
// Last Update  :  01/04/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/04/95    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"                 


//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
    
//---------------------------------------------------------------------
// WPPalleteWindowProcWrapper():
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
// reference purposes (they are in order of apperance):
//       WM_GETMINMAXINFO  -> uMsg = 0x0024
//       WM_NCCREATE       -> uMsg = 0x0081
//       WM_NCCALCSIZE     -> uMsg = 0x0083
// Then  WM_CREATE         -> uMsg = 0x0001
//       ect...
//---------------------------------------------------------------------
LRESULT CALLBACK WPPaletteWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{                                          
   WPPALETTE      *pobjPalette;   // Pointer to palette object.
   CREATESTRUCT   *pCreateStruct; // Pointer to window creation structure.

   // On the WM_CREATE message 'lParam' contains the pointer to the 
   // Workplace Shell Palette object class that owns this window.
   if (uMsg==WM_CREATE)
   {
      // Get pointer to palette object class from create structure.
      pCreateStruct = (CREATESTRUCT *)lParam;                  
      pobjPalette   = (WPPALETTE *)pCreateStruct->lpCreateParams;
      // Store pointer to palette object into extra window longs.
      SetWindowLong(hWnd,0,(LONG)pobjPalette);
   }
   else
      // Query pointer to palette that owns this window procedure (this pointer is refernced later).
      pobjPalette = (WPPALETTE *)GetWindowLong(hWnd,0);         

    // If we do not have a valid notebook object pointer...
    if (pobjPalette==NULL)
    {                                                    
       // Log event (for debug version).
       #ifdef _DEBUG
          dprintf("tr WPPaletteWindowProcWrapper(): Msg: 0x%04X 'pobjPalette' is NULL.\n",uMsg);
       #endif
       // Call Windows 3.1's default window procedure.
       return (DefWindowProc(hWnd,uMsg,wParam,lParam));
    }   
    else
       // Else, call the "real" client window procedure for notebook object.
       return (pobjPallete->ClientWindowProc(hWnd,uMsg,wParam,lParam));
}


//---------------------------------------------------------------------
// ClientWindowProc(): 
//---------------------------------------------------------------------
LRESULT WPPALLETE::ClientWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            
      case WM_COMMAND:            
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
         
      case WM_PAINT:
         OnPaint();
         break;

      case WM_NCACTIVATE: 
         // Handle this message ONLY if the non-client area is becoming active OR
         // the window becoming active or inactive is not a notebook page (modeless dialog).
         // else, do not update non-client area to prevent flickering effect with 
         // notebook window caption.
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


// EOF PALETTE.CPP


