//=====================================================================
// WPS_SDK.H
//    Workplace Shell for Windows initialization file (WPSHELL.INI)
// file structure header file.
//
// Date Created :  08/23/95
// Last Update  :  08/24/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 08/23/95    | Creation.                               | MDA
//=====================================================================

#ifndef __WPFWIN_SDK_PROTOTYPED__

#define __WPSFWIN_SDK_PROTOTYPED__

#define STRICT

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// C-runtime library header files.
#include <stdlib.h>  // Needed for _MAX_PATH constant.


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define  WPS_EXENAME                "WPSHELL.EXE"
#define  MAX_TITLE_NAME             63
#define  MAX_PATH_FILENAME          63
// Program object constants.
#define  MAX_PROGRAM_PATH           63
#define  MAX_PARAMETERS_LINE        63
#define  MAX_WORKING_DIR            63
// Datafile object constants.
#define  MAX_DATAFILE_SUBJECT       31
#define  MAX_DATAFILE_PATH          63
#define  MAX_DATAFILE_NAME          15

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
#define OBJTYPE_X             0x02000000L       // Not Implemented (Reserved).
#define OBJTYPE_PRINTER       0x04000000L       // Not Implemented (Reserved).
#define OBJTYPE_SHADOW        0x08000000L
#define OBJTYPE_DATAFILE      0x10000000L
#define OBJTYPE_WPSSETUP      0x20000000L
#define OBJTYPE_MINWINDOW     0x40000000L
#define OBJTYPE_PALETTE       0x80000000L       // Not Implemented (Reserved).
// Derived classes (2nd level).
#define OBJTYPE_FOLDER        (OBJTYPE_CONTAINER|OBJTYPE_WINDOW|0x00010000L)
#define OBJTYPE_COLORPALETTE  (OBJTYPE_PALETTE|OBJTYPE_WINDOW|0x00010000L)  // Not Implemented (Reserved).
#define OBJTYPE_FONTPALETTE   (OBJTYPE_PALETTE|OBJTYPE_WINDOW|0x00020000L)  // Not Implemented (Reserved).
// Derived classes (2nd level).
#define OBJTYPE_SYSTEMSETUP   (OBJTYPE_WPSSETUP|0x00010000L)
#define OBJTYPE_KEYBRDSETUP   (OBJTYPE_WPSSETUP|0x00020000L)    // Not Implemenrted (Reserved).
#define OBJTYPE_MOUSESETUP    (OBJTYPE_WPSSETUP|0x00030000L)    // Not Implemented (Reserved).
#define OBJTYPE_SOUNDSETUP    (OBJTYPE_WPSSETUP|0x00040000L)    // Not Implemented (Reserved).
// Derived classes (3rd level).
#define OBJTYPE_DESKTOP       (OBJTYPE_FOLDER|0x00000100L)
#define OBJTYPE_MINVIEWER     (OBJTYPE_FOLDER|0x00000200L)
#define OBJTYPE_DIRECTORY     (OBJTYPE_FOLDER|0x00000400L)
// Derived classes (4th level).
#define OBJTYPE_DRIVE         (OBJTYPE_DIRECTORY|0x00000001L)


//---------------------------------------------------------------------
// OBJECT FLAG VALUES
//---------------------------------------------------------------------
// object's behavior and functionality flags.
// object's minimized button (if any) behavior.
#define MINBEHAVIOR_FLAGS_MASK      0x0000000FL
#define MINBEHAVIOR_HIDE            0x00000001L
#define MINBEHAVIOR_MIN_TO_VIEWER   0x00000002L
#define MINBEHAVIOR_MIN_TO_DESKTOP  0x00000003L
// object open behavior flags.
#define OPENBEHAVIOR_FLAGS_MASK     0x000000F0L
#define OPENBEHAVIOR_DISPLAY        0x00000010L
#define OPENBEHAVIOR_CREATE         0x00000020L

// folder's behavior and functionality flags.
// folder's minimize button apperance.
#define BUTTONAPPERANCE_FLAGS_MASK  0x0000F000L
#define BUTTONAPPERANCE_HIDE        0x00001000L
#define BUTTONAPPERANCE_MINIMIZE    0x00002000L

// Folder's image flags (normal, tiled, scaled).
#define IMAGE_FLAGS_MASK            0x000F0000L
#define IMAGE_COLOR_ONLY            0x00000000L    // No image, color only.
#define IMAGE_NORMAL                0x00010000L    // Normal image (centered on folder).
#define IMAGE_TILED                 0x00020000L    // Tiled image.
#define IMAGE_SCALED                0x00040000L    // Scaled image (stretched to fit folder).

// Folder's text transparency flags.
#define TEXT_APPEARANCE_FLAGS_MASK  0x00100000L
#define TEXT_OPAQUE                 0x00000000L
#define TEXT_TRANSPARENT            0x00100000L

// Folder's icon view format flags.
#define ICONVIEW_FORMAT_FLAGS_MASK  0x00600000L
#define ICONVIEW_FORMAT_NONGRID     0x00000000L
#define ICONVIEW_FORMAT_FLOWED      0x00200000L
#define ICONVIEW_FORMAT_NONFLOWED   0x00400000L

// default object flags.
#define DEF_FOLDER_FLAGS    (MINBEHAVIOR_HIDE|OPENBEHAVIOR_DISPLAY|BUTTONAPPERANCE_HIDE)
#define DEF_PROGRAM_FLAGS   (MINBEHAVIOR_MIN_TO_VIEWER|OPENBEHAVIOR_CREATE)

//---------------------------------------------------------------------
// LOCAL ENUMERATIONS
//---------------------------------------------------------------------
enum { VIEW_ICON=0, VIEW_TREE, VIEW_DETAILS };

enum DRIVE_TYPE { WPDRIVE_UNKNOWN=0, WPDRIVE_FLOPPY=2, WPDRIVE_HARD=3,
                  WPDRIVE_NETWORK=4, WPDRIVE_CDROM=5 , WPDRIVE_RAMDISK=6 };


//---------------------------------------------------------------------
// LOCAL STRUCTURES (USED TO BUILD FILE STRUCTURES).
//---------------------------------------------------------------------

// Workplace Shell profile magic identification number.
#define WPSHELL_PROFILE_MAGIC_NUMBER   0x4B454F48L      // 'HOEK' (ASCII)

// WPS Profile Header data structure definition.
typedef struct {
   DWORD    m_dwMagicID;         // 'HOEK'
   WORD     m_wMajorVersion;     // i.e. v1.5  = 1
   WORD     m_wMinorVersion;     // i.e. v1.5  = 50
   DWORD    m_dwDataOffset;      // file offset to begining of data.
} WPSHELL_PROFILE_HEADER;


//---------------------------------------------------------------------
// WPOBJECT
typedef struct {
   DWORD    m_dwType;                           // Use OBJTYPE_* defines above.
   WORD     m_wUniqueID;                        // Used to link shadow objects.
   char     m_szTitle[MAX_TITLE_NAME+1];        // Object title.
   POINT    m_ptIcon;                           // (x,y) icon view coord.
   char     m_szIconPath[MAX_PATH_FILENAME+1];  // Icon resource path.
   WORD     m_wIconIndex;                       // Icon resource index (0-based).
   WORD     m_wState;                           // Object state flags (internal use only).
   DWORD    m_dwFlags;                          // Object general flags (see above).
} FILE_OBJECT_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPCONTAINER
typedef struct {
   int      m_nObjects;                         // Number of objects in container (folder).
} FILE_CONTAINER_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPWINDOW
typedef struct {
   POINT    m_ptWindow;                         // (x,y) position of window.
   SIZE     m_sizeWindow;                       // Size of window.
   BOOL     m_fScrollable;                      // Is window scrollable?
} FILE_WINDOW_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPFOLDER
typedef struct {
   // folder background color.
   COLORREF    m_rgbBackground;                 // Folder background color.
   LOGFONT     m_lfIconText;                    // Folder font information.
   COLORREF    m_clrrefIconTextColor;           // Folder icon text color.
   char        m_szImagePath[MAX_PATH_FILENAME+1];  // Background image path.
   WORD        m_wOpenView;                     // Current folder view (see above).
} FILE_FOLDER_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPPROGRAM
typedef struct {
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];     // Program path & file name.
      char     m_szParameters[MAX_PARAMETERS_LINE+1];   // Program command-line parameters.
      char     m_szWorkingDir[MAX_WORKING_DIR+1];       // Program working directory.
} FILE_PROGRAM_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDATAFILE
typedef struct {
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1]; // Data File subject text.
      char     m_szDataPath[MAX_DATAFILE_PATH+1];       // Data File path.
      char     m_szDataName[MAX_DATAFILE_NAME+1];       // Data File file name.
} FILE_DATAFILE_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPTEMPLATE
typedef struct {
   // template type information.
   DWORD    m_dwTemplateType;                           // Template type (see object types).
   // template's object instance title string.
   char     m_szInstanceTitle[MAX_TITLE_NAME+1];        // Title of instantiated template object.
   // template object instance icon information.
   char     m_szInstanceIconPath[MAX_PATH_FILENAME+1];  // Icon path of instantiated template object.
   WORD     m_wInstanceIconIndex;                       // Icon index of instantiated template object.
} FILE_TEMPLATE_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPSHADOW
typedef struct {
   WORD     m_wOriginalUniqueID;                // Original object's unique ID.
} FILE_SHADOW_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPDIRECTORY file data structure.
typedef struct {
   char  m_szPath[_MAX_PATH+1];                 // Directory path.
} FILE_DIRECTORY_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPDRIVE file data structure.
typedef struct {
   UINT  m_uDriveIndex;                         // Drive index (0-based).
   WORD  m_Type;                                // Type of drive (see above).
} FILE_DRIVE_STRUCT;
//---------------------------------------------------------------------



//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECTS DATA TYPES.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GENERIC OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
} FILE_WPS_OBJECT_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FOLDER, DESKTOP, and MINVIEWER OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_WINDOW_STRUCT      m_wnd;
   FILE_FOLDER_STRUCT      m_fol;
   FILE_CONTAINER_STRUCT   m_con;
} FILE_WPS_FOLDER_STRUCT;;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// PROGRAM OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_PROGRAM_STRUCT     m_prg;
} FILE_WPS_PROGRAM_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DATAFILE OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_DATAFILE_STRUCT    m_dat;
} FILE_WPS_DATAFILE_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TEMPLATE OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_TEMPLATE_STRUCT    m_tmp;
} FILE_WPS_TEMPLATE_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SHADOW OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_SHADOW_STRUCT      m_sha;
} FILE_WPS_SHADOW_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DIRECTORY OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_WINDOW_STRUCT      m_wnd;
   FILE_FOLDER_STRUCT      m_fol;
   FILE_DIRECTORY_STRUCT   m_dir;
   FILE_CONTAINER_STRUCT   m_con;
} FILE_WPS_DIRECTORY_STRUCT;;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DRIVE OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_WINDOW_STRUCT      m_wnd;
   FILE_FOLDER_STRUCT      m_fol;
   FILE_DRIVE_STRUCT       m_drv;
   FILE_CONTAINER_STRUCT   m_con;
} FILE_WPS_DRIVE_STRUCT;
//---------------------------------------------------------------------

#endif

// EOF WPS_SDK.H

