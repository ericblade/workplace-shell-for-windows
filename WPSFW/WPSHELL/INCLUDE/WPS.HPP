//=====================================================================
// WPS.HPP
//    Class forward declarations
//
// Date Created :  01/24/94
// Last Update  :  10/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/24/94    | Creation.                               | MDA
//=====================================================================


#ifndef __WPCLASSES_PROTOTYPED__

#define __WPCLASSES_PROTOTYPED__

#define STRICT

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 API header file.
#include <windows.h>
// C-runtime library header file.
#include <stdio.h>
// IBM SAA CUA Controls Library/2 header files.
#include "fclnbw.h"      // include file for notebook control DLL.
#include "fclspbw.h"     // include file for spin button control DLL.

//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
char *GetString(UINT uIDString);    // used to get string resources.

// Function prototype used for debugging the program (in MAIN.CPP).
void dprintf( char *pszFormat, ... );


//---------------------------------------------------------------------
// SHELL PROPERTIES COMPILE-TIME FLAGS
//---------------------------------------------------------------------
#define ENABLE_EXITWINDOWS       TRUE     // Call ExitWindows() when exiting.
#define ENABLE_SHELLHOOK         TRUE     // Use windows hook WH_SHELL to set minimized positions.
#define ENABLE_SPECIAL           FALSE    // Any special processing by the shell.

//---------------------------------------------------------------------
// GLOBAL MACROS
//---------------------------------------------------------------------
// Custom isspace() C-macro replacement.
#define IsSpace(ch)  ((ch==' ')||(ch=='\t')||(ch=='\n')||(ch=='\r'))


//---------------------------------------------------------------------
// GLOBAL CONSTANTS
//---------------------------------------------------------------------
#define WPS_EXENAME              "WPSHELL.EXE"
#define MAX_TITLE_NAME           63    // 63 chars (+1 null) => 64 chars.
#define MAX_PATH_FILENAME        63    // 63 chars (+1 null) => 64 chars.
// "WPSxxxx" class name length (7 chars).
#define MAX_CLASS_NAME           10     // 9 chars (+1 null) => 10 chars.


//---------------------------------------------------------------------
// GLOBAL DATA TYPES
//---------------------------------------------------------------------

// Workplace Shell system defaults.
typedef struct {
   BOOL     fConfirmOnFolderDelete;
   BOOL     fConfirmOnDelete;
   BOOL     fConfirmOnFileSystemDelete;
   BOOL     fButtonAppearanceHide;
   BOOL     fWindowAnimationEnabled;
   WORD     wMinButtonBehavior;
   WORD     wOpenBehavior;
   char     szSystemSetupFolderName[MAX_TITLE_NAME+1];
   BOOL     fNoSaveEnabled;
   WORD     wFolderAutomaticClose;
} SYSTEM;

// Workplace Shell environment values.
typedef struct {
   HTASK         hTask;                      // Program task handle.
   HINSTANCE     hInst;                      // Program istance handle.
   HINSTANCE     hResInst;                   // NLS-Resources instance handle.
   HWND          hwndDesktop;                // Workplace Shell desktop handle (invisible window).
   BOOL          fAcceleratorTranslation;    // Flag to enable/disable accelerator key traslation.
   HWND          hwndCurrentModelessDialog;  // Current modeless dialog window handle.
   SIZE          sizeIconIndent;
   HCURSOR       hDragCursor;
   HCURSOR       hNoDragCursor;
   HCURSOR       hOriginalCursor;
   SYSTEM        stSystem;
   char          **apszStringTable;
   COLORREF      clrrefShadowTextColor;
   WORD          wCompFlags;
} WPENVIRONMENT;


//---------------------------------------------------------------------
// EXTERNAL GLOBAL VARIABLES
//---------------------------------------------------------------------
extern WPENVIRONMENT    g_wpEnvironment;

//---------------------------------------------------------------------
// ENUMERATION DEFINITIONS.
//---------------------------------------------------------------------
// Minimize button behavior flags.
enum { HIDE=0, MINTOVIEWER, MINTODESKTOP };
// Folder Automatic Close behavior flags.
enum { NEVER=0, SUBFOLDERS_ONLY, ALL_OBJECTS };

//---------------------------------------------------------------------
// USER-DEFINED WINDOW MESSAGES
//---------------------------------------------------------------------
// DESKTOP CUSTOM MESSAGES
#define WM_WPS_INITIALIZEDESKTOP          (WM_USER+1)
#define WM_WPS_WINDOW_CREATED             (WM_USER+2)    // used with ShellHook().
#define WM_WPS_WINDOW_DESTROYED           (WM_USER+3)    // used with ShellHook().
#define WM_WPS_WINDOW_UPDATE              (WM_USER+4)
#define WM_WPS_POPUP_MENU                 (WM_USER+5)
#define WM_WPS_DROP                       (WM_USER+6)
#define WM_WPS_NOTIFY_OBJECT_DESTROYED    (WM_USER+7)
#define WM_WPS_DELETE_OBJECT              (WM_USER+8)
#define WM_WPS_UNREGISTERCLASS            (WM_USER+9)
#define WM_WPS_DESTROY_NOTEBOOK           (WM_USER+10)
#define WM_WPS_DESTROY_SETTINGS_NOTEBOOK  (WM_USER+11)
#define WM_WPS_SYSSHUTDOWN                (WM_USER+12)
// FOLDER CUSTOM MESSAGES
#define WM_WPS_INITIALIZEFOLDER           (WM_WPS_INITIALIZEDESKTOP)
#define WM_WPS_QUERY_FOLDER_POINTER       (WM_USER+13)
#define WM_WPS_HWND_ASSOCIATE_OBJECT      (WM_USER+14)
// PROGRAM CUSTOM MESSAGES
#define WM_WPS_FIND_HINSTANCE             (WM_USER+15)
#define WM_WPS_HINSTANCE_ASSOCIATE_OBJECT (WM_USER+16)
#define WM_WPS_GET_HWND_FROM_OBJECT       (WM_USER+17)
// NOTEBOOK CUSTOM MESSAGES
#define WM_WPS_NOTEBOOK_SETFOCUS          (WM_USER+18)
#define WM_WPS_DIALOG_DONE                (WM_USER+19)


//---------------------------------------------------------------------
// OBJECT TYPE DEFINITIONS.
//---------------------------------------------------------------------
// Pure-base class.
#define OBJTYPE_OBJECT        0x00000000L
// Base classes (1st level).
#define OBJTYPE_CONTAINER     0x00100000L
#define OBJTYPE_WINDOW        0x00200000L
#define OBJTYPE_PROGRAM       0x00400000L
#define OBJTYPE_TEMPLATE      0x00800000L
#define OBJTYPE_SHREDDER      0x01000000L
#define OBJTYPE_X             0x02000000L
#define OBJTYPE_PRINTER       0x04000000L
#define OBJTYPE_SHADOW        0x08000000L
#define OBJTYPE_DATAFILE      0x10000000L
#define OBJTYPE_WPSSETUP      0x20000000L
#define OBJTYPE_MINWINDOW     0x40000000L
#define OBJTYPE_PALETTE       0x80000000L
// Derived classes (2nd level).
#define OBJTYPE_FOLDER        (OBJTYPE_CONTAINER|OBJTYPE_WINDOW|0x00010000L)
#define OBJTYPE_COLORPALETTE  (OBJTYPE_PALETTE|OBJTYPE_WINDOW|0x00010000L)
#define OBJTYPE_FONTPALETTE   (OBJTYPE_PALETTE|OBJTYPE_WINDOW|0x00020000L)
// Derived classes (2nd level).
#define OBJTYPE_SYSTEMSETUP   (OBJTYPE_WPSSETUP|0x00010000L)
#define OBJTYPE_KEYBRDSETUP   (OBJTYPE_WPSSETUP|0x00020000L)
#define OBJTYPE_MOUSESETUP    (OBJTYPE_WPSSETUP|0x00030000L)
#define OBJTYPE_SOUNDSETUP    (OBJTYPE_WPSSETUP|0x00040000L)
// Derived classes (3rd level).
#define OBJTYPE_DESKTOP       (OBJTYPE_FOLDER|0x00000100L)
#define OBJTYPE_MINVIEWER     (OBJTYPE_FOLDER|0x00000200L)
#define OBJTYPE_DIRECTORY     (OBJTYPE_FOLDER|0x00000400L)
// Derived classes (4th level).
#define OBJTYPE_DRIVE         (OBJTYPE_DIRECTORY|0x00000001L)


//---------------------------------------------------------------------
// CLASS FORWARD DECLARATION
//---------------------------------------------------------------------
class WPOBJECT;         // Base class.
class WPCONTAINER;      // Derived classes.
class WPFOLDER;
class WPDESKTOP;
class WPPROGRAM;
class WPTEMPLATE;
class WPSHREDDER;
class WPDATAFILE;
class WPSYSSETUP;
class WPMINVIEWER;
class WPMINWNDOBJ;
class WPSHADOW;
class WPDIRECTORY;
class WPDRIVE;
//class WPPALETTE;
//class WPCOLORPALETTE;

class WPWINDOW;         // Base class.
class WPNOTEBOOK;       // Derived class.
class WPSETTINGSNB;     // Derived class.

class DDESERVER;        // Base class.
class DDEPROGMAN;       // Derived class.
class DDESHELL;         // Derived class.

// DDE server classes header files.
#include "ddeservr.hpp"    // DDE server class (DDESERVER).
#include "ddeprgmn.hpp"    // DDE 'PROGMAN' server class (DDEPROGMAN).
#include "ddeshell.hpp"    // DDE 'SHELL' server class (DDESHELL).

// Workplace Shell classes header files.
#include "wpobject.hpp"    // Object class (WPOBJECT).
#include "wpcontnr.hpp"    // Container clas (WPCONTAINER).
#include "wpwindow.hpp"    // Window class (WPWINDOW).
#include "wpfolder.hpp"    // Folder class (WPFOLDER).
#include "wpdesktp.hpp"    // Desktop class (WPDESKTOP).
#include "wpminwin.hpp"    // Minimized Window Viewer class (WPMINVIEWER).
#include "wpprog.hpp"      // Program class (WPPROGRAM).
#include "wptemplt.hpp"    // Template class (WPTEMPLATE).
#include "wpshredr.hpp"    // Shredder class (WPSHREDDER).
#include "wpdatafl.hpp"    // Data File class (WPDATAFILE).
#include "wpsetup.hpp"     // Setup objects class (WPSETUP).
#include "wpsysset.hpp"    // System Setup class (WPSYSSETUP).
#include "wpshadow.hpp"    // Shadow class (WPSHADOW).
#include "wpdrive.hpp"     // Directory and Drive class (WPDIRECTORY and WPDRIVE).
//#include "wppalett.hpp"    // Palette class (WPPALETTE).
//#include "wpclrpal.hpp"    // Color Palette class (WPCOLORPALETTE).

#include "wpnotebk.hpp"    // Notebook class (WPNOTEBOOK).
#include "wpsetnb.hpp"     // Settings Notebook class (WPSETTINGNB).

#endif

// EOF WPS.HPP



