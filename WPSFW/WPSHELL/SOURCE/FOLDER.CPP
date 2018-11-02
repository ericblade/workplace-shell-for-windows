//=====================================================================
// FOLDER.CPP
//    WPFOLDER class window procedure. 
// 
// Date Created :  01/26/94
// Last Update  :  12/07/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/26/94    | Creation.                               | MDA
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
// ClientWindowProc():
//    Client window procedure for WPFOLDER class.     
//---------------------------------------------------------------------
LRESULT WPFOLDER::ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{                                          
   // Check if a folder scrollbar has captured the user input.
   if (isScrollbarCapturingInput()==TRUE)
   {  
      // Pass message to scrollbar window procedure,...
      if (ScrollbarWindowProc(uMsg,wParam,lParam)==0L)
         return (NULL); // message processed, return.
   }
      
   // Process window message.
   switch (uMsg)
   {
      case WM_CREATE:  
         // Load the folder's children objects' icons.
         if (LoadIcons()==FALSE)
            return ((LRESULT)(-1));
         // Store the window handle for use later.
         SetHandle(hWnd);
         // Let folder object know that create message is being processed.
         // NOTE: Here is where folder objects have a chance to animate
         //       the zoom effect for window creation.
         OnCreate();
         // Post message to initialize folder.
         PostMessage(hWnd,WM_WPS_INITIALIZEFOLDER,0,0L);
         // Return success so we can continue with the creation of window.
         return ((LRESULT)0);

      case WM_WPS_INITIALIZEFOLDER:
         // Call folder class initialization function.
         OnInitializeFolder();
         break;

      case WM_WPS_QUERY_FOLDER_POINTER:
         // Return pointer to this folder class object.
         return ((LRESULT)this);              
         
      case WM_WPS_DROP:
         // Update object's icon position.
         OnDrop(MAKEPOINT(lParam));
         break;
         
      case WM_WPS_POPUP_MENU:
         // Display object popup menu.
         OnPopupMenu(MAKEPOINT(lParam));
         break;
         
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MOUSEMOVE:                                        
      case WM_LBUTTONDBLCLK:                          
         OnMouseMessage(uMsg,(WORD)wParam,MAKEPOINT(lParam));
         break;
         
      case WM_HSCROLL:
      case WM_VSCROLL:
         // Process scrollbar message by passing it to scrollbar message handler.
         OnScrollbarMessage(uMsg,(WORD)wParam,(int)LOWORD(lParam),(HWND)HIWORD(lParam));
         // If scrollabr message is not "thumb-track" AND folder has an image as background...
         if ((uMsg!=SB_THUMBTRACK)&&((GetFlags()&IMAGE_FLAGS_MASK)!=IMAGE_COLOR_ONLY))
            // Invalidate and update the whole client area right now.
            Invalidate(TRUE,TRUE);
         break;
         
      case WM_TIMER:
         // If folder object created a Windows 3.1 timer, this message must be processed.
         OnTimer((WORD)wParam);
         break;
         
      case WM_SYSCOMMAND:
         // A system command has been requested.
         OnSysCommand((WORD)wParam,lParam);
         break;
         
      case WM_COMMAND:      
         // A system popup menu or accelerator key selection was made.
         OnCommand((WORD)wParam);
         break;
           
      case WM_SETTEXT:     
         // Set new window caption text.
         DefWindowProc(hWnd,uMsg,wParam,lParam);
         // Update window caption.
         OnNCPaint();
         break;
           
      case WM_NCACTIVATE:                         
         OnNCActivate((BOOL)wParam);
         return (TRUE);
         
      case WM_DROPOBJECT:
      case WM_QUERYDROPOBJECT:
      case WM_DRAGSELECT:
      case WM_DRAGMOVE:          
      case WM_DROPFILES: 
         // Handle a Windows 3.1 drag & drop message.
         return (OnWin31DragAndDropMsg(uMsg,wParam,lParam));

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
         
      case WM_MOVE:
         OnMove(MAKEPOINT(lParam));
         break;                      
                                                              
      case WM_SIZE: 
         OnSize((WORD)wParam,LOWORD(lParam),HIWORD(lParam));
         break;

      case WM_PALETTECHANGED:
      case WM_QUERYNEWPALETTE:
         return (OnPaletteMessage(uMsg,(HWND)wParam));
               
      case WM_ERASEBKGND:
         return (OnEraseBackground((HDC)wParam));
      
      case WM_PAINT:          
         OnPaint();
         break;
      
      case WM_GETMINMAXINFO:
         ((MINMAXINFO FAR *)lParam)->ptMinTrackSize.y = 120;
         break;
         
      case WM_CLOSE:                    
         // Close folder, store folder state, and update parent folder.
         OnClose();         
         break;
    
      default: 
         // If unproccessed, pass it on to default window procedure.
         return (DefWindowProc(hWnd,uMsg,wParam,lParam));
   }      

   // We are done processing the window message...                                              
   return (NULL);
}

// EOF FOLDER.CPP


