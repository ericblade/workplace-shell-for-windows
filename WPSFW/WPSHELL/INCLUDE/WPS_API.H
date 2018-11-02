//=====================================================================
// WPS_API.H
//    Workplace Shell for Windows external API header file.
// 
// Date Created :  08/26/95
// Last Update  :  09/06/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 08/26/95    | Creation.                               | MDA
//=====================================================================

#ifndef __WPSFW_API_PROTOTYPED__

#define __WPSFW_API_PROTOTYPED__

#define STRICT

//---------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------

#if !defined(__WPCLASSES_PROTOTYPED__)
// Objects Types:
// (if marked as "Abstract" then object cannot exists by itself)
#define OBJTYPE_CONTAINER     0x00100000L    // Abstract
#define OBJTYPE_WINDOW        0x00200000L    // Abstract
#define OBJTYPE_PROGRAM       0x00400000L
#define OBJTYPE_TEMPLATE      0x00800000L
#define OBJTYPE_SHREDDER      0x01000000L
#define OBJTYPE_SHADOW        0x08000000L
#define OBJTYPE_DATAFILE      0x10000000L
#define OBJTYPE_WPSSETUP      0x20000000L    // Abstract
#define OBJTYPE_MINWINDOW     0x40000000L
#define OBJTYPE_FOLDER        (OBJTYPE_CONTAINER|OBJTYPE_WINDOW|0x00010000L)
#define OBJTYPE_SYSTEMSETUP   (OBJTYPE_WPSSETUP|0x00010000L)
#define OBJTYPE_DESKTOP       (OBJTYPE_FOLDER|0x00000100L)
#define OBJTYPE_MINVIEWER     (OBJTYPE_FOLDER|0x00000200L)
#define OBJTYPE_DIRECTORY     (OBJTYPE_FOLDER|0x00000400L)
#define OBJTYPE_DRIVE         (OBJTYPE_DIRECTORY|0x00000001L)

#endif   // #ifndef OBJTYPE_OBJECT

                      
// Drag & Drop window types used for registration flag parameter.
#define WNDTYPE_SOURCE        0x0001   // D&D source only.
#define WNDTYPE_TARGET        0x0002   // D&D target only.
#define WNDTYPE_BOTH          (WNDTYPE_SOURCE|WNDTYPE_TARGET)

// Command values used with "WpsCommand()" API.
#define CMDTYPE_GETDESKTOPHANDLE 0x0001    
#define CMDTYPE_GETCHILDCOUNT    0x0002
#define CMDTYPE_GETCHILDHANDLE   0x0003
#define CMDTYPE_GETOBJECTINFO    0x0004

// Constants for WPSOBJINFO structure.
#define MAX_TITLE_NAME           63    // 63 chars (+1 null) => 64 chars.
#define MAX_PATH_FILENAME        63    // 63 chars (+1 null) => 64 chars.
   
   
//---------------------------------------------------------------------
// WORKPLACE SHELL MESSAGES
//---------------------------------------------------------------------
// WPS Message sent to source/target registered windows.
#define WPSM_COMMAND             (WM_USER+0x0300)  
// Notification codes used as "wParam" value when WPS message is sent.
#define DRAGTARGET_NOTIFY        10
#define DROPTARGET_NOTIFY        11
#define DRAGSOURCE_NOTIFY        12
#define DROPSOURCE_NOTIFY        13
#if !defined(__WPCLASSES_PROTOTYPED__)
// Drag & Drop function values passed as LOWORD(lParam) when WPS message is sent.
enum { DRAG_SELECT=0, DRAG_MOVE, DRAG_COPY, DRAG_SHADOW, DRAG_SELECT_RUBBERBAND };
enum { DROP_SELECT=0, DROP_MOVE, DROP_COPY, DROP_SHADOW, DROP_SELECT_RUBBERBAND };
#endif
   
   
//---------------------------------------------------------------------
// DATA TYPES & STRUCTURES
//---------------------------------------------------------------------
typedef DWORD  HWPSOBJ;    // Handle of Workplace Shell object.

typedef struct {   
   HWPSOBJ  m_hObject;                          // Object handle.
   DWORD    m_dwType;                           // See OBJTYPE_* constants defined above.
   char     m_szTitle[MAX_TITLE_NAME+1];        // Object's title string.
   char     m_szIconPath[MAX_PATH_FILENAME+1];  // Object's icon resource path.
   WORD     m_wIconIndex;                       // Object's icon index (0-based).
} WPSOBJINFO;

                        
//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

//---------------------------------------------------------------------
// WpsRegisterWindow():
//    This function will register a window with the Workplace Shell as
// to be able to drag & drop WPS objects to and/or from it.  The 
// interface with which WPS and the application owning the window 
// communicate is through message passing to this window.
//
// Parameters:
//    hwnd     = Window handle
//    wFlags   = Use one of the contants WNDTYPE_* defined above.  This
//               is used to selectively register windows as "drag & drop 
//               source only", "drag & drop target only", or "drag & drop 
//               source and target".
BOOL CALLBACK _export WpsRegisterWindow(HWND hwnd,WORD wFlags);

//---------------------------------------------------------------------
// WpsCommand():
//    This function allows an external application to query and access
// information about the current Workplace Shell desktop.  Its inter-
// face is generic as to allow many future enhancements and extensions
// to its command set.  The return value is dependent upon the command
// exectuted.
// 
// Parameters:
//    wCommand = Use one of the constants CMDTYPE_* defined above.  This 
//               is used to determine the information that is requested.
//               Depending on the command type, the DWORD return value
//               and the "dwData" parameter could be used or not used.
//    dwData   = This parameter's value depends on the command type 
//               being used.
DWORD CALLBACK _export WpsCommand(WORD wCommand,DWORD dwData,DWORD dwData2);


//---------------------------------------------------------------------
// WpsUnregisterWindow():
//
BOOL CALLBACK _export WpsUnregisterWindow(HWND hwnd);



#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */


#endif 

// EOF WPS_API.H

