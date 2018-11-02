//=====================================================================
// RESOURCE.H
//    Workplace Shell's LaunchPad program resource header file.
//
// Date Created :  09/02/95
// Last Update  :  09/13/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/02/95    | Creation.                               | MDA
//=====================================================================


//-----------------------------------------------------------------------------
// PRODUCT VERSION CONSTANTS
//-----------------------------------------------------------------------------
#define VER_MAJOR                1                 // Integer representing major version number.
#define VER_MINOR                60                // Integer representing minor version number.
#define VER_STR_PRODUCTVERSION   "Release v1.6\0"  // String representing current version.
#define VER_STR_PRODUCTCOPYRIGHT "Copyright © IBM Corporation 1994-1995\0"


//---------------------------------------------------------------------
// ICON RESOURCES IDs
//---------------------------------------------------------------------
// Main application icon(s).
#define IDI_LAUNCHPAD      10
// Built-in system functionality icons.
#define IDI_SHUTDOWN       21
#define IDI_LOCKUP         22
#define IDI_MINVIEWER      23
// Icons used in cursor creation.
#define IDI_NOSIGN         34
#define IDI_COPYGRID       35
#define IDI_SHADOWCIRCLE   36


//---------------------------------------------------------------------
// CURSOR RESOURCES IDs
//---------------------------------------------------------------------
#define IDC_WAITCURSOR     50


//---------------------------------------------------------------------
// LAUNCH PAD'S CONTROL IDs
//---------------------------------------------------------------------
#define IDB_SHUTDOWN       100
#define IDB_LOCKUP         101
#define IDB_WINDOWLIST     102
#define IDB_FIND           103
#define IDB_TOPDRAWER      200
#define IDB_TOPOBJECT      300

//---------------------------------------------------------------------
// LAUNCH PAD'S CHILD POPUP WINDOW IDs
//---------------------------------------------------------------------
#define IDP_DRAWER_POPUP   400

//---------------------------------------------------------------------
// STRING TABLE IDs
//---------------------------------------------------------------------
#define IDSTR_BASE_INDEX   1000


// EOF RESOURCE.H

