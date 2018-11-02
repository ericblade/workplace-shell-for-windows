//=====================================================================
// LAUNCHPD.CPP
//    Workplace Shell's LaunchPad program source module.
//
// Date Created :  08/30/95
// Last Update  :  09/03/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 08/30/95    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
#define STRICT
// Windows 3.1 SDK header file.
#include <windows.h>
// C-runtime library header files.
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef _DEBUG
#include <stdarg.h>
#endif            
// LaunchPad header files.
#include "launchpd.hpp"   
#include "resource.h"
// Workplace Shell API header file.
#include "..\..\wpshell\include\wps_api.h"
// Windows 3D control library header file (CTL3D.DLL).
#include "ctl3d.h"


//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
LAUNCHPAD_ENVIRONMENT   g_LaunchPadEnv;

   
//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// LOCAL FUNCTION PROROTYPES
//---------------------------------------------------------------------
HWND CreateLaunchPad(HINSTANCE hInstance);
BOOL LoadStringTable(HINSTANCE hInstance,int nTotalStrings);
void FreeStringTable(int nTotalStrings);

//---------------------------------------------------------------------
// FUNCTION DEFINITIONS 
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateLaunchPadControls():
//---------------------------------------------------------------------
void CreateLaunchPadControls(HWND hWnd)
{
   CreateWindow("Button","B1",(WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON),
      10,10,40,40,hWnd,(HMENU)(IDB_TOPOBJECT+0),g_LaunchPadEnv.hInst,NULL);
   CreateWindow("Button","B2",(WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON),
      60,10,40,40,hWnd,(HMENU)(IDB_TOPOBJECT+1),g_LaunchPadEnv.hInst,NULL);
   CreateWindow("Button","B3",(WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON),
      110,10,40,40,hWnd,(HMENU)(IDB_TOPOBJECT+2),g_LaunchPadEnv.hInst,NULL);
   CreateWindow("Button","B4",(WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON),
      160,10,40,40,hWnd,(HMENU)(IDB_TOPOBJECT+3),g_LaunchPadEnv.hInst,NULL);
}

//---------------------------------------------------------------------
// DisplayPopupMenu():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void DisplayPopupMenu(HWND hWnd,WORD x,WORD y)
{                                
/*
   hmenuLaunchPad = LoadMenu(g_LaunchPadEnv.hInst,MAKEINTRESOURCE(IDM_LAUNCHPAD));

   TrackPopupMenu(hmenuLaunchPad,TPM_
*/
}


//---------------------------------------------------------------------
// LaunchPadWndProc():
//---------------------------------------------------------------------
LRESULT CALLBACK LaunchPadWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{           
   HDC         hdc;
   PAINTSTRUCT ps;

   switch (uMsg)
   {  
      case WM_CREATE:  
         CreateLaunchPadControls(hWnd);
         return (TRUE);
              
      case WM_NCHITTEST:
         return (HTCAPTION);
         
      case WM_COMMAND:
         if (HIWORD(lParam)==BN_CLICKED)
            PostMessage(hWnd,WM_CLOSE,0,0L);
         break;
         
      case WPSM_COMMAND:
         if (wParam==DRAGTARGET_NOTIFY)
         {
            if (LOWORD(lParam)==DRAG_MOVE)
               return (TRUE);
         }
         return (FALSE);
           
      case WM_RBUTTONDOWN:
         DisplayPopupMenu(hWnd,LOWORD(lParam),HIWORD(lParam));
         break;
         
      case WM_PAINT:
         hdc = BeginPaint(hWnd,&ps);
         EndPaint(hWnd,&ps);
         break;
   
      case WM_CLOSE:
         DestroyWindow(hWnd);
         break;
      
      case WM_DESTROY:
         PostQuitMessage(0);
         break;
       
      default:  
         return (DefWindowProc(hWnd,uMsg,wParam,lParam));         
   }           
   
   return (0L);   
}

//---------------------------------------------------------------------
// GetPosition():
//---------------------------------------------------------------------
void GetPosition(POINT *pptLaunchPad)
{
   char  szBuffer[80];

   // Get top-left position of "Launch Pad" from system profile.
   GetPrivateProfileString("Workplace_Shell_for_Windows","LaunchPad","",szBuffer,sizeof(szBuffer),"SYSTEM.INI");

   // By default, center window list.
   pptLaunchPad->x = ((GetSystemMetrics(SM_CXSCREEN)-LAUNCHPAD_DEFAULT_CX)/2);
   pptLaunchPad->y = ((GetSystemMetrics(SM_CYSCREEN)-LAUNCHPAD_DEFAULT_CY)/2);

   // If there was a stored string in profile containing previous position...
   if (szBuffer[0]!='\0')
      sscanf(szBuffer,"%d %d",&(pptLaunchPad->x),&(pptLaunchPad->y));
}


//---------------------------------------------------------------------
// CreateLaunchPad():
//---------------------------------------------------------------------
HWND CreateLaunchPad(HINSTANCE hInstance)
{
   WNDCLASS    wc;
   HWND        hwndLaunchPad;
   POINT       ptPosition;
      
   // Initialize the window class structure information.
   wc.style         = 0;
   wc.lpfnWndProc   = LaunchPadWndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = hInstance;
   wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_LAUNCHPAD));
   wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
   wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
   wc.lpszMenuName  = NULL;
   wc.lpszClassName = LAUNCHPAD_CLASSNAME;
   
   // Register Launch Pad's window class.   
   if (!RegisterClass(&wc))
       return FALSE;
   
   // Get default position for Launch Pad's main window.
   GetPosition(&ptPosition);
       
   // Create the main Launch Pad window.
   hwndLaunchPad = CreateWindow(LAUNCHPAD_CLASSNAME,"Launch Pad",
       (WS_POPUP | WS_BORDER),
       ptPosition.x,ptPosition.y,LAUNCHPAD_DEFAULT_CX,LAUNCHPAD_DEFAULT_CY,
       HWND_DESKTOP,NULL,hInstance, NULL );
                             
   if (hwndLaunchPad!=NULL)                             
      ShowWindow(hwndLaunchPad,SW_SHOW);       
       
   return (hwndLaunchPad);       
}


//---------------------------------------------------------------------
// WinMain():
//    Entry point for WPSHELL application, and main loop code.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
#pragma warning( disable : 4204 ) // Disable warning about "nonstandard extension used".
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
//   HACCEL      haccelShell;
//   HINSTANCE   hNotebookLibrary,hSpinButtonLibrary;
   HWND        hwndLaunchPad;
   MSG         Msg;

   // Any other instances of app running?
   if (hPrevInstance!=NULL)                       
   {     
      // Check whether there is already a copy of the LaunchPad running...
      hwndLaunchPad = FindWindow(LAUNCHPAD_CLASSNAME,NULL);
      // If found one...
      if (IsWindow(hwndLaunchPad)!=FALSE)
         // Bring LaunchPad top-level window to top.
         BringWindowToTop(hwndLaunchPad);
      // Allow only one copy of LaunchPad running at any time, therefore exit.
      return (FALSE);
   }

   #ifdef _DEBUG
   // Automatically start the Microsoft "Debug Window" utility which receives our debug output.
   WinExec("DBWIN.EXE",SW_SHOWNORMAL);
   #endif

/*
   // Load NLS resource library which contains language-dependent resources.
   if (LoadNLSResourceLibrary()==FALSE)
      // Error loading NLS resource library, therefore exit.
      return (FALSE);

   // Load string resource table from NLS resource library.
   if (LoadStringTable(g_hResInst,TOTAL_STRINGS)==FALSE)
   {
      // Free NLS resource library before exiting.
      FreeLibrary(g_hResInst);
      // Error loading string resources, therefore exit.
      return (FALSE);
   }

   // Initialize global structure containing LaunchPad environment values.
   InitializeEnvironmentStructure(hInstance);
  
*/ 

   g_LaunchPadEnv.hInst = hInstance;
 
   // Register this application's dialog boxes to use the 3D control library.
   Ctl3dRegister(hInstance);
   Ctl3dAutoSubclass(hInstance);

/*
   // Load workplace shell accelerator table.
   haccelShell = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDA_WPSHELL));
   if (haccelShell==NULL)
      return (FALSE);
  
*/  

   hwndLaunchPad = CreateLaunchPad(hInstance);
  
   WpsRegisterWindow(hwndLaunchPad,WNDTYPE_TARGET);

   // Get and dispatch messages until a WM_QUIT message is received...
   while (GetMessage(&Msg,NULL,NULL,NULL))
   {
                            
/*                            
      // Check if message should go to a modeless dialog box (notebook pages).
      if ((g_LaunchPadEnv.hwndCurrentModelessDialog==NULL) ||
          (IsDialogMessage(g_LaunchPadEnv.hwndCurrentModelessDialog,&Msg)==FALSE))
      {
         // If we are in here, the message is intented for a window.
         // Check if accelerator table translation mode is enabled ?
         if (g_LaunchPadEnv.fAcceleratorTranslation==TRUE)
         {
            // It is, so we translate keystrokes using accelerator table.
            if (TranslateAccelerator(GetActiveWindow(),haccelShell,&Msg)==0)
            {
               TranslateMessage(&Msg);   // Translates virtual key codes.
               DispatchMessage(&Msg);    // Dispatches message to window.
            }
         }
         else
         {
               
*/               
            // No accelerator table translation wanted, pass message as is.
            TranslateMessage(&Msg);   // Translates virtual key codes.
            DispatchMessage(&Msg);    // Dispatches message to window.
/*            
         }
      }
*/      
   }

   // Unregister main Launch Pad window from Workplace Shell.
   WpsUnregisterWindow(hwndLaunchPad);

   // Unregister this application from 3D control library (CTL3D.DLL).
   Ctl3dUnregister(hInstance);

   // Clean-up libraries before exiting.
//   FreeLibrary(g_LaunchPadEnv.hResInst); // NLS resource library.

   // Free string table.
//   FreeStringTable(TOTAL_STRINGS);

   // Returns the value from PostQuitMessage() call.
   return (Msg.wParam);
}


//---------------------------------------------------------------------
// LoadStringTable():
//---------------------------------------------------------------------
BOOL LoadStringTable(HINSTANCE hInstance,int nTotalStrings)
{
   int   index,nSize;
   char  szTempString[255];

   // Allocate an array of string pointers that will hold all stings.
   g_LaunchPadEnv.apszStringTable = new char *[nTotalStrings];
   // If error allocating space for string pointers, then exit.
   if (g_LaunchPadEnv.apszStringTable==NULL)
      return (FALSE);
   // Load strings from resource string table.
   for (index=0; index<nTotalStrings; index++)
   {
      // Get string and store the its real size.
      nSize = LoadString(hInstance,(IDSTR_BASE_INDEX+index),szTempString,sizeof(szTempString));
      // If error loading string resource, exit.
      if (nSize==0)
         return (FALSE);
      // Allocate enough space for the string read.
      g_LaunchPadEnv.apszStringTable[index] = new char[nSize+1];
      // If error allocating memory, exit
      if (g_LaunchPadEnv.apszStringTable[index]==NULL)
         return (FALSE);
      // Copy read string from temporary space to newly allocated space.
      strcpy(g_LaunchPadEnv.apszStringTable[index],szTempString);
   }
   // Return success (all strings have been loaded).
   return (TRUE);
}


//---------------------------------------------------------------------
// GetString():
//    This function returns the string pointer to the string ID passed
// as parameter.  It indexes the global string pointer array created
// by LoadStringTable() function.
//---------------------------------------------------------------------
char *GetString(UINT uIDString)
{
   // Return pointer to string.
   return (g_LaunchPadEnv.apszStringTable[(uIDString-IDSTR_BASE_INDEX)]);
}

//---------------------------------------------------------------------
// FreeStringTable():
//---------------------------------------------------------------------
void FreeStringTable(int nTotalStrings)
{
   int   index;

   // Loop through each string in the loaded string table...
   for (index=0; index<nTotalStrings; index++)
      // ...And free the memory allocated for the string.
      delete (g_LaunchPadEnv.apszStringTable[index]);
   // Free memory allocated for the array of string pointers.
   delete (g_LaunchPadEnv.apszStringTable);
}

      
//---------------------------------------------------------------------
// dprintf():
//    This function is used to help debug this program.  It acts as a
// regular 'printf()' function but directs its output to the "Debug
// Window" application.  This "Debug Window" application is supplied
// with MS Visual C++ 1.x as DBWIN.EXE and DBWIN.DLL.
//    This function can be permanently removed or left here after
// all development is done.
//---------------------------------------------------------------------
void dprintf( char *pszFormat, ... )
{
#ifdef _DEBUG
    char    szStr[256];    // Maximum string allowed is 255 chars + NULL.
    va_list va_ptr;

    // Create string from variable argument list...
    va_start( va_ptr, pszFormat );
    vsprintf( szStr, pszFormat, va_ptr );
    va_end( va_ptr );
    // Display output string in "Debug Window" application.
    OutputDebugString( szStr );
    // optionally, you could display the output on a message box and block.
    //MessageBox(NULL,szStr,"Debug Output",(MB_OK | MB_ICONINFORMATION));
#endif
}
      

// EOF LAUNCHPD.CPP

