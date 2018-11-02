//=====================================================================
// WPS_MAIN.C
//    Workplace Shell for Windows NLS-resources Library Main Source File.
//
// Date Created :  12/09/94
// Last Update  :  12/23/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 12/09/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows library header files.
#include <windows.h>

//---------------------------------------------------------------------
// LibMain():
//    DLL entry point for icon library.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
int CALLBACK LibMain(HINSTANCE hInstance,WORD wDataSeg,WORD wHeapSize,LPSTR lpszCmdLine)
{
   if (wHeapSize>0)
      UnlockData(0);

   return 1;
}


//---------------------------------------------------------------------
// WEP():
//    DLL exit point for icon library.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
int CALLBACK WEP(int nParam)
{
   return 1;
}

// EOF WPS_MAIN.C

