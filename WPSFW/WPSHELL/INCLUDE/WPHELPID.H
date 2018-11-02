//=====================================================================
// WPHELPID.H
//    Workplace Shell for Windows Help IDs Header File.
//
// Date Created :  11/02/94
// Last Update  :  01/25/95
// Written By   :  Alex Gutkin / Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 11/02/94    | Creation.                               | AG
//=====================================================================

#ifndef __WPHELPID_PROTOTYPED__

#define __WPHELPID_PROTOTYPED__


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// DESKTOP OBJECT MENU IDs
//---------------------------------------------------------------------
// When button F1 pressed in "Open".
#define HELPID_LEVEL1_MOPEN            100
// When button F1 pressed in "Open -> Icon view"
#define HELPID_LEVEL1_MOPENICONVIEW    102
// When button F1 pressed in "Open -> Details view"
#define HELPID_LEVEL1_MOPENDETAILSVIEW 104
// When button F1 pressed in "Settings".
#define HELPID_LEVEL1_MSETTINGS        110
// When button F1 pressed in "Refresh now".
#define HELPID_LEVEL1_MREFRESHNOW      120
// When button F1 pressed in "Help".
#define HELPID_LEVEL1_MHELP            125
// When button F1 pressed in "Lockup now".
#define HELPID_LEVEL1_MLOCKUP          130
// When button F1 pressed in "Shutdown...".
#define HELPID_LEVEL1_MSHUTDOWN        132
// When button F1 pressed in "System setup".
#define HELPID_LEVEL1_MSYSSETUP        135
// When button F1 pressed in "Find...".
#define HELPID_LEVEL1_MFIND            140
// When button F1 pressed in "Select".
#define HELPID_LEVEL1_MSELECT          145
// When button F1 pressed in "Sort".
#define HELPID_LEVEL1_MSORT            150
// When button F1 pressed in "Sort -> Name".
#define HELPID_LEVEL2_MSORTNAME        152
// When button F1 pressed in "Sort -> Type".
#define HELPID_LEVEL2_MSORTTYPE        154
// When button F1 pressed in "Sort -> Real name".
#define HELPID_LEVEL2_MSORTREAL        156
// When button F1 pressed in  "Sort -> Size".
#define HELPID_LEVEL2_MSORTSIZE        158
// When button F1 pressed in  "Sort -> Last date".
#define HELPID_LEVEL2_MSORTDATE        160
// When button F1 pressed in  "Sort -> Last time".
#define HELPID_LEVEL2_MSORTTIME        162

// When button F1 pressed in  "Arrange".
#define HELPID_LEVEL1_MARRANGE         170
// When button F1 pressed in  "Save desktop".
#define HELPID_LEVEL1_MSAVEDESKTOP     175
// When button F1 pressed in  "Run...".
#define HELPID_LEVEL1_MRUN             180



//---------------------------------------------------------------------
//  FOLDER OBJECT MENU IDs
//---------------------------------------------------------------------
// When button F1 pressed in "Open parent".
#define HELPID_LEVEL1_MOPENPARENT      200
// When button F1 pressed in "Create another...".
#define HELPID_LEVEL1_MCREATEANOTHER   205
// When button F1 pressed in "Copy..".
#define HELPID_LEVEL1_MCOPY            210
// When button F1 pressed in "Move..".
#define HELPID_LEVEL1_MMOVE            215
// When button F1 pressed in "Create shadow...".
#define HELPID_LEVEL1_MCREATESHADOW    220
// When button F1 pressed in "Delete...".
#define HELPID_LEVEL1_MDELETE          225
// When button F1 pressed in "Window".
#define HELPID_LEVEL1_MWINDOW          230
// When button F1 pressed in "Window -> Restore".
#define HELPID_LEVEL2_MWNDRESTORE      232
// When button F1 pressed in "Window -> Move".
#define HELPID_LEVEL2_MWNDMOVE         234
// When button F1 pressed in "Window -> Size".
#define HELPID_LEVEL2_MWNDSIZE         236
// When button F1 pressed in "Window -> Minimize".
#define HELPID_LEVEL2_MWINDOWMIN       238
// When button F1 pressed in "Window -> Maximize".
#define HELPID_LEVEL2_MWINDOWMAX       240
// When button F1 pressed in "Window -> Hide".
#define HELPID_LEVEL2_MWINDOWHIDE      242
// When button F1 pressed in "Find...".
//#define HELPID_LEVEL1_MFIND            245
// When button F1 pressed in "Close".
#define HELPID_LEVEL1_MCLOSE           250



//---------------------------------------------------------------------
// WINDOW LIST HELP IDs
//---------------------------------------------------------------------
// When button F1 pressed in "Show"
#define HELPID_WNDLIST_SHOW            280
// When button F1 pressed in "Tile"
#define HELPID_WNDLIST_TILE            285
// When button F1 pressed in "Cascade"
#define HELPID_WNDLIST_CASCADE         290


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK PAGES HELP IDs
//---------------------------------------------------------------------

// Folder object:
// When button "Help" pressed in settings notebook "View" page.
#define HELPID_SETNB_VIEWPAGE          300
// When button "Help" pressed in settings notebook "Background" page.
#define HELPID_SETNB_BACKGROUNDPAGE    305
// When button "Help" pressed in settings notebook "Window" page.
#define HELPID_SETNB_WINDOWPAGE        310
// When button "Help" pressed in settings notebook "General" page.
#define HELPID_SETNB_GENERALPAGE       315
// When button "Help" pressed in settings notebook "Lockup" page.
#define HELPID_SETNB_LOCKUPPAGE        320
// When button "Help" pressed in settings notebook "Lockup" page.
#define HELPID_SETNB_DESKTOPPAGE       325

// Program object:
// When button "Help" pressed in settings notebook "Program" page.
#define HELPID_SETNB_PROGRAMPAGE       400
// When button "Help" pressed in settings notebook "Association" page.
#define HELPID_SETNB_ASSOCIATIONPAGE   405

// Data object:
// When button "Help" pressed in settings notebook "Type" page.
#define HELPID_SETNB_TYPEPAGE          500
// When button "Help" pressed in settings notebook "File" page.
#define HELPID_SETNB_FILEPAGE          505

// System Setup object:
// When button "Help" pressed in settings notebook "Confirmation" page.
#define HELPID_SYSTEM_SETNB_CONFIRMATIONSPAGE   600
// When button "Help" pressed in settings notebook "Window" page.
#define HELPID_SYSTEM_SETNB_WINDOWPAGE          605

// Drive object:
// When button "Help" pressed in settings notebook "Drive" page.
#define HELPID_SETNB_DRIVEPAGE         610



//---------------------------------------------------------------------
// OBJECT'S "GENERAL HELP" MENU HELP IDs
//---------------------------------------------------------------------
#define HELPID_GENERALHELP_OBJECT               700   // Generic object (Template, Shredder, etc..)
#define HELPID_GENERALHELP_PROGRAM              705   // Program
#define HELPID_GENERALHELP_DATAFILE             710   // Data-file
#define HELPID_GENERALHELP_FOLDER               715   // Folder
#define HELPID_GENERALHELP_DESKTOP              720   // Desktop
#define HELPID_GENERALHELP_MINWINDOWVIEWER      735   // Minimized Window Viewer
#define HELPID_GENERALHELP_MINWINDOWOBJ         740   // Minimized Window Object
#define HELPID_GENERALHELP_WINDOWLIST           745   // Window List
#define HELPID_GENERALHELP_DRIVE                750   // Drive object

//---------------------------------------------------------------------
// OBJECT'S "KEYS HELP" MENU HELP ID
//---------------------------------------------------------------------
#define HELPID_KEYSHELP                         800   

#endif

// EOF WPHELPID.H

