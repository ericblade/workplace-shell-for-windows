//=====================================================================
// DESKTOP.CPP
//    WPDESKTOP class window procedures.
// 
// Date Created :  01/26/94
// Last Update  :  10/08/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
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
// C-runtime library header files.
#include <stdlib.h>
#include <string.h>
// Windows library header files.
#include <commdlg.h>
#include <ver.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL GLOBAL VARIABLES
//---------------------------------------------------------------------
// Defined in WPPROG.CPP file:
extern char *apszWinExecErrorString[32];

//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
BOOL CommonFileDialogBox(HWND hWnd,DWORD dwFlags,char *pszDialogTitle,char *pszFilterString,char *pszFile,char *pszFileTitle);
HINSTANCE ExecuteApplication(HWND hwndParent,LPCSTR pszOp,LPCSTR pszFile,LPCSTR pszParams,LPCSTR pszDir,int nCmdShow);


//---------------------------------------------------------------------
// FUNCTION DEFINITIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDESKTOPSubClassWindowProc():
//---------------------------------------------------------------------
LRESULT CALLBACK _loadds WPDESKTOPSubClassWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{                      
   WPDESKTOP *pobjDesktop;
   LRESULT   lReturn;

   // Query pointer to folder that owns this window procedure (this pointer is refernced later).
   pobjDesktop = (WPDESKTOP *)GetWindowLong(g_wpEnvironment.hwndDesktop,0);
             
   // Process messages.              
   switch (uMsg)
   {
      case WM_ERASEBKGND:
         // Erase Windows 3.1 desktop background by calling default Windows 3.1 desktop window proc.
         lReturn = CallWindowProc(pobjDesktop->GetWindows31DesktopProc(),hWnd,uMsg,wParam,lParam);
         // If desktop object is currently open, then...
         if (pobjDesktop->isOpen()==TRUE)
         {     
            // Check whether desktop size has changed since last desktop update.
            // NOTE: This is done to fix a syncronization problem that occurs with
            //       new video resolution managers that can switch resolutions without
            //       having to restart Windows (i.e. ATI Flex-Desk, etc...).
            pobjDesktop->CheckDesktopSizeChange();
            // If "Details View" is current open view, then draw details table.
            if (pobjDesktop->GetOpenView()==VIEW_DETAILS)
               pobjDesktop->DrawDetailsViewTable((HDC)wParam);
            // Draw top-level children objects on desktop area.
            pobjDesktop->DrawChildren((HDC)wParam);
         }
         return (lReturn);
         
      default:
         // Call default Windows 3.1 desktop window procedure for all other messages.
         return (CallWindowProc(pobjDesktop->GetWindows31DesktopProc(),hWnd,uMsg,wParam,lParam));
   }  
}


//---------------------------------------------------------------------
// ClientWindowProc():
//    Client window procedure for WPDESKTOP class.     
//---------------------------------------------------------------------
LRESULT WPDESKTOP::ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{                                          
   HDC         hdc;     // HDC for desktop window.
   PAINTSTRUCT ps;      // Paint structure for BeginPaint call.
   int         nResult;
                       
   // Process window message...
   switch (uMsg)
   {
      case WM_CREATE:  
         // Store desktop window handle in global variable.
         g_wpEnvironment.hwndDesktop = hWnd;
         // Return success so we can continue with the creation of window.
         return (WPFOLDER::ClientWindowProc(hWnd,uMsg,wParam,lParam));
      
      case WM_WPS_WINDOW_CREATED:
         OnTopLevelWindowCreated((HWND)wParam);
         break;
         
      case WM_WPS_WINDOW_DESTROYED:
         OnTopLevelWindowDestroyed((HWND)wParam);
         break;

      case WM_WPS_WINDOW_UPDATE:
         OnUpdateObjectMinPlacement((HINSTANCE)wParam);
         break;
                                            
      case WM_WPS_HWND_ASSOCIATE_OBJECT:
         OnAssociateObject((HWND)wParam,(WPOBJECT *)lParam);
         break;            

      case WM_WPS_FIND_HINSTANCE:
         return ((LRESULT)OnFindInstanceHandle((HINSTANCE)wParam));
      
      case WM_WPS_HINSTANCE_ASSOCIATE_OBJECT:
         OnAssociateObject((HINSTANCE)wParam,(WPOBJECT *)lParam);
         break;            
     
     case WM_WPS_GET_HWND_FROM_OBJECT:
         return (MAKELRESULT((WORD)OnGetWindowFromObject((WPOBJECT *)lParam),0));
         break;
      
      case WM_WPS_SYSSHUTDOWN:
         // User wants to shutdown Workplace Shell for Windows.
         PostMessage(hWnd,WM_CLOSE,0,0L);
         break;

      case WM_WPS_UNREGISTERCLASS:
         // Message sent by folder objects when closed.
         // wParam = unsigned integer between 0 and 1023.
         UnregisterClassName((WORD)wParam);
         break;
         
      case WM_WPS_NOTIFY_OBJECT_DESTROYED:        
         // Workplace shell object is being destroyed notification.
         OnNotifyObjectDestroyed((WPOBJECT *)lParam);
         break;
 
      case WM_WPS_DELETE_OBJECT:                           
         OnDeleteObject((WPOBJECT *)lParam);
         break;         

      case WM_WPS_DESTROY_NOTEBOOK:
         // Destroy notebook (pointer to notebook object in 'lParam').
         ((WPNOTEBOOK *)lParam)->GetOwner()->CloseSettingsNotebook();
         break;        

      case WM_WPS_DESTROY_SETTINGS_NOTEBOOK:
         // Destroy notebook (pointer to notebook object in 'lParam').
         ((WPSETTINGSNB *)lParam)->GetOwner()->CloseSettingsNotebook();
         break;        
         
      case WM_WINDOWPOSCHANGING:              
         // Make sure the desktop window is not resized or moved, and stays on the bottom of the Z-order.
         ((WINDOWPOS FAR *)lParam)->hwndInsertAfter = HWND_BOTTOM;
         ((WINDOWPOS FAR *)lParam)->flags |= (SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
         break;
      
      case WM_WININICHANGE:  
         // A change has occurred in the Windows initialization file.
         OnWinINIChange((LPCSTR)lParam);
         break;
      
      case WM_ERASEBKGND:
         // If we need to pass down the erase background message to the real desktop window,
         if ((g_wpEnvironment.wCompFlags&0x0001)==0x0001)
            // Call default Windows 3.1 desktop window procedure for all other messages.
            SendMessage(GetDesktopWindow(),uMsg,wParam,lParam);
         break;            
      
      case WM_PAINT:
         // Do nothing in the paint message since the desktop is a transparent window.
         hdc = ::BeginPaint(hWnd,&ps); 
         ::EndPaint(hWnd,&ps);         
         break;
         
      case WM_CLOSE:  
         // If "Save desktop" feature is enabled...
         if (g_wpEnvironment.stSystem.fNoSaveEnabled==TRUE)
            // User's answer would've been 'No'.
            nResult = IDNO;
         else      
         // Check if user wants the shell to save desktop automatically... 
         if ((GetFlags()&DESKTOP_FLAGS_MASK)==DESKTOP_SAVESETTINGS)
            // User's answer would've been 'Yes'.
            nResult = IDYES;                               
         else                                                      
            // User wants to be asked, therefore display message box query.
            nResult = MessageBox(NULL,GetString(IDSTR_SAVE_DESKTOP_QUESTION),GetString(IDSTR_SHUTDOWN_CAPTION),
                                 (MB_ICONQUESTION|MB_YESNOCANCEL));
                                 
         // Depending on user's answer to save the desktop...
         switch (nResult)
         {
            case IDYES:                      
               // Save desktop state.             
               SaveDesktop();   
               break;                
               
            case IDCANCEL:
               // Abort shutdown.
               return (NULL);
         }                   

         // First, query top-level windows before closing down Workplace Shell.
         // This is necessary because ExitWindows() will not let us close all
         // windows nicely if successful (it does not return at all!).
         if ((IsShell()==TRUE)&&(QueryQuit()==FALSE))
            // There is a top-level window that will not end its session, 
            // therefore ignore "Shutdown..." command
            return (NULL);
         
         // We are assured that all top-level windows will terminate their sessions,
         // therefore implode desktop window, clear background and close shell.
         OnClose();
                                    
         // Check if Workplace Shell for Windows is the default Windows shell...                                    
         if (IsShell()==TRUE)
         {                                    
            // ..It is, thertefore try to exit Windows 3.1.
            #if ENABLE_EXITWINDOWS
            // If for any strange reason an application cannot be terminated, abort shutdown.
            // Everything should terminate though since we already checked in QueryQuit().
            if (ExitWindows(0,0)==FALSE)
               return (NULL);
            #endif
         }
         else
         {
            // Workplace Shell for Windows was executed as a Windows program from 
            // another Windows shell (maybe PROGMAN.EXE).  Therefore, we do not terminate
            // Windows (do nothing).
         }
         break;     
         
      case WM_QUERYENDSESSION:
         // Check if the message did not come from "ourselves" (QueryQuit() function)...
         if (((BOOL)LOWORD(lParam))!=TRUE)
            // Save desktop state without asking user (just in case).
            SaveDesktop();   
         // Return "yes" to the query for ending this session.
         return (TRUE);

      case WM_DESTROY:   
         // Destroy WorkPlace Shell.
         OnDestroy();      
         break;

      case WM_NCACTIVATE:
      case WM_NCPAINT:  
      case WM_SETTEXT:
      case WM_NCMOUSEMOVE:
      case WM_NCLBUTTONUP:
      case WM_NCLBUTTONDOWN:
      case WM_SYSCOMMAND:               
      case WM_GETMINMAXINFO:
         // We want Windows to handle these messages, pass it to default window procedure.
         return (DefWindowProc(hWnd,uMsg,wParam,lParam));
           
      default: 
         // The rest of messages will be passed into the folder 
         // class' window procedure to be handled.
         return (WPFOLDER::ClientWindowProc(hWnd,uMsg,wParam,lParam));
   }      
   
   return (NULL);
}

//---------------------------------------------------------------------
// WPDESKTOPEnumWindowsProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL CALLBACK WPDESKTOPEnumWindowsProc(HWND hWnd,LPARAM lParam)
{                       
   char szClassName[32];
      
   // Get current window's class name...
   if (GetClassName(hWnd,szClassName,(sizeof(szClassName)-1)) > 0)
   {              
      // Make sure string is null-terminated before using it.         
      szClassName[31] = '\0';
      // Check for "WinFax 4.0 PRO" application window...
      if (stricmp(szClassName,"Sfaxmng")==0)
         // Return immediatedly since it will GPF's if queried to end session twice!
         return (TRUE);
   }

   // Query top-level application and return value converted to boolean.
   // NOTE: The LOWORD(lParam) value of TRUE is set so that the desktop window
   //       procedure can distinguish who sent the WM_QUERYENDSESSION
   //       message.  We assume that the ExitWindows() call will always
   //       send WM_QUERYENDSESSION with LOWORD(lParam) = 0 (FALSE).
   return ((BOOL)(SendMessage(hWnd,WM_QUERYENDSESSION,0,MAKELPARAM(TRUE,0))!=0));
}  


//---------------------------------------------------------------------
// RUN PROGRAM DIALOG PROCEDURE
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// RunDlgProc():
//    Dialog procedure for "Run..." menu entry in desktop system menu.
// It allows the user to run any application without having to create 
// a program (or data file) object for it.  It also lets users browse
// through their drives for the application wanted.  It uses the common
// file dialog for browsing the drives.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL CALLBACK RunDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{  
   RECT  rcDialog;
   UINT  uReturnValue;
   char  szFile[_MAX_PATH+1],
         szFileTitle[(_MAX_FNAME+_MAX_EXT)+1],
         szFilter[] = "Programs|*.exe;*.pif;*.com;*.bat|All Files (*.*)|*.*|",
         szErrorMsg[128],*pszParams;
   int   nCmdShow;         

   // Process dialog box message...
   switch (uMsg)
   {
      case WM_INITDIALOG:                                           
         // Get dialog box's size.                        
         GetWindowRect(hDlg,&rcDialog);
         // Center dialog box with respect to the screen.
         MoveWindow(hDlg,(GetSystemMetrics(SM_CXSCREEN)-rcDialog.right)/2,
                         (GetSystemMetrics(SM_CYSCREEN)-rcDialog.bottom)/2,
                         rcDialog.right,
                         rcDialog.bottom,FALSE);
         // By default, all applications are run in "normal" show state.
         CheckRadioButton(hDlg,IDB_NORMAL,IDB_MAXIMIZED,IDB_NORMAL);
         return (TRUE);
      
      case WM_COMMAND:
         switch (wParam)
         {
            case IDOK:
               // Get current contents of command line edit control.
               GetDlgItemText(hDlg,IDE_COMMANDLINE,szFile,sizeof(szFile));
               // Get show state to run application in.
               if (IsDlgButtonChecked(hDlg,IDB_NORMAL)==1)
                  nCmdShow = SW_SHOWNORMAL;
               else
               if (IsDlgButtonChecked(hDlg,IDB_MINIMIZED)==1)
                  nCmdShow = SW_SHOWMINIMIZED;
               else
                  nCmdShow = SW_SHOWMAXIMIZED;
                  
               
               // Find start of program parameters in command line by finding the 
               // first occurrence of a white space character in command line string.
               pszParams = strchr(szFile,' ');
               // If there are any parameters in command line string...
               if (pszParams!=NULL)
               {                      
                  // NULL-terminate program name.
                  *pszParams= '\0';          
                  // Point to parameters by incrementing pointer to next character.
                  pszParams++;
               }
                  
               // Execute program in command line.                               
               uReturnValue = (UINT)ExecuteApplication(GetDesktopWindow(),"open",szFile,pszParams,NULL,nCmdShow);
               
               // Check for possible errors executing program.
               if (uReturnValue<32)
               {    
                  switch (uReturnValue)
                  {
                     case 16:
                        break;
                        
                     default:
                        // Report error code as a meaningful text string.
                        sprintf(szErrorMsg,GetString(IDSTR_ERR_RUN),
                           uReturnValue,
                           szFile,
                           apszWinExecErrorString[uReturnValue]);
                        MessageBox(NULL,szErrorMsg,GetString(IDSTR_ERR_CAPTION),MB_OK|MB_ICONSTOP);
                        // Do not end dialog since could not execute application successfully.
                        return (FALSE);
                  }
               }        
               // End dialog since application was execute successfully.       
               EndDialog(hDlg,TRUE);
               break;
               
            case IDCANCEL:
               EndDialog(hDlg,FALSE);
               break;
            
            case IDB_RUNBROWSE:
               // Open "Run" dialog box, and select program to run.                         
               if (CommonFileDialogBox(hDlg,(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_SHAREAWARE),GetString(IDSTR_FILEDLG_TITLE_RUN),szFilter,szFile,szFileTitle)==TRUE)
                  SetDlgItemText(hDlg,IDE_COMMANDLINE,szFile);
               break;
             
            case IDB_HELP:
               // Open help file with "Run" help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_LEVEL1_MRUN);
               break;
         }                   
         break;
   }
   
   return (FALSE);
}         

//---------------------------------------------------------------------
// PRODUCT INFORMATION DIALOG PROCEDURE
//---------------------------------------------------------------------
         
//---------------------------------------------------------------------
// ProductInfoDlgProc():
//    Dialog procedure for Product Information dialog box.  This dialog
// box is created when the user selects the Desktop system menu ->
// Help -> Product Information menu entry.  It is a modal dialog box.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL CALLBACK ProductInfoDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{        
   RECT     rcDialog;
   
   // Process dialog box message...
   switch (uMsg)
   {
      case WM_INITDIALOG:      
         // Get dialog box's size.                        
         GetWindowRect(hDlg,&rcDialog);
         // Center dialog box with respect to the screen.
         MoveWindow(hDlg,(GetSystemMetrics(SM_CXSCREEN)-rcDialog.right)/2,
                         (GetSystemMetrics(SM_CYSCREEN)-rcDialog.bottom)/2,
                         rcDialog.right,
                         rcDialog.bottom,FALSE);
         return (TRUE);
      
      case WM_COMMAND:
         switch (wParam)
         {
            case IDOK:
               EndDialog(hDlg,0);
               return (TRUE);
         }                   
         break;
   }
     
   return (FALSE);
} 


// EOF DESKTOP.CPP

