//=====================================================================
// LAUNCHPD.HPP
//    Workplace Shell's LaunchPad program header file.
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

#ifndef __LAUNCHPAD_PROTOTYPED__

#define __LAUNCHPAD_PROTOTYPED__

//---------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------
#define LAUNCHPAD_CLASSNAME   "WPSLaunchPad"
// Default Launch Pad size.
#define LAUNCHPAD_DEFAULT_CX  260
#define LAUNCHPAD_DEFAULT_CY  70


//---------------------------------------------------------------------
// DATA STRUCTURE TYPES
//---------------------------------------------------------------------
// Workplace Shell environment values.
typedef struct {
   HINSTANCE     hInst;                      // Program istance handle.
   HINSTANCE     hResInst;                   // NLS-Resources instance handle.
   BOOL          fAcceleratorTranslation;    // Flag to enable/disable accelerator key traslation.
   HWND          hwndCurrentModelessDialog;  // Current modeless dialog window handle.
   HCURSOR       hDragCursor;
   HCURSOR       hNoDragCursor;
   HCURSOR       hOriginalCursor;
   char          **apszStringTable;
   COLORREF      clrrefShadowTextColor;
} LAUNCHPAD_ENVIRONMENT;


//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From LAUNCHPD.CPP File:
char *GetString(UINT uIDString);
void dprintf( char *pszFormat, ... );                                  


//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
extern LAUNCHPAD_ENVIRONMENT   g_LaunchPadEnv;


#endif

// EOF LAUNCHPD.HPP

