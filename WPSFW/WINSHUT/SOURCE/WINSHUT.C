//=====================================================================
// WINSHUT.C
//    Windows shutdown utility initialization and main functions.
//
// Date Created :  05/09/95
// Last Update  :  05/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 05/09/95    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 header files.
#include <windows.h>
// C-runtime library header files.
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------
BOOL IsShell(char *pszShellFileName);

//---------------------------------------------------------------------
// WinMain():
//    Entry point for WINSHUT application.
//---------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND  hwndShell;
   int   nReturnCode;

   // Any other instances of app running?
   if (hPrevInstance!=NULL)
      // Allow only one copy of shell running at any time, therefore exit.
      return (FALSE);

   // Convert program command line to all uppercase.
   AnsiUpper(lpCmdLine);

   // If user did not specify the "skipwps" flag (i.e. Skip Workplace Shell Test)...
   if (strstr(lpCmdLine,"-SKIPWPS")==NULL)
   {
      // Check if "Workplace Shell for Windows" is currently running...
      hwndShell = FindWindow("WPS0000",NULL);
      if ((hwndShell!=NULL)&&(IsShell("WPSHELL.EXE")==TRUE))
      {
         // Post close message to the shell.
         PostMessage(hwndShell,WM_CLOSE,0,0L);
         // End program with a success return code.
         return (0);
      }
   }

   // Exit windows by calling Windows API.
   nReturnCode = ((ExitWindows(0,0)==TRUE)? 0 : -1);

   // Return exit code.
   return (nReturnCode);
}


//---------------------------------------------------------------------
// IsShell():
//---------------------------------------------------------------------
BOOL IsShell(char *pszShellFileName)
{
   char szSysINIShellName[128];

   // Get the contents of the "shell=" key...
   GetPrivateProfileString("BOOT","SHELL","PROGMAN.EXE",szSysINIShellName,
                           sizeof(szSysINIShellName),"SYSTEM.INI");

   // Convert all characters to uppercase.
   AnsiUpper(szSysINIShellName);
   AnsiUpper(pszShellFileName);

   // Check whether the specified shell name is in the "shell=" string...
   if (strstr(szSysINIShellName,pszShellFileName)!=NULL)
      return (TRUE);
   else
      return (FALSE);
}

// EOF WINSHUT.C

