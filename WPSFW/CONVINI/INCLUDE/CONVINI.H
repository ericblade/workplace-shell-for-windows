//=====================================================================
// CONVINI.H
//    Convert Initialization File utility program.
//
// Date Created :  09/08/94
// Last Update  :  12/15/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/08/94    | Creation.                               | MDA
//=====================================================================

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
// Container object constants.
#define MAX_CONTAINER_COMPONENTS    64

// Misc. constants.
#define  MAX_CLASS_NAME             10     // 10 chars. max. for class name.

// default colors.
#define  DEFAULT_BACKGROUND_COLOR               GetSysColor(COLOR_WINDOW)
// default icons.
#define  DEFAULT_GROUP_ICON_PATH                WPS_EXENAME
#define  DEFAULT_GROUP_ICON_INDEX               6
#define  DEFAULT_ITEM_ICON_PATH                 WPS_EXENAME
#define  DEFAULT_ITEM_ICON_INDEX                5
#define  DEFAULT_PROGMANFOLDER_ICON_INDEX       26
// folder object defaults.
#define  DEFAULT_DATAFILE_TITLE                 "Data File"
#define  DEFAULT_DATAFILE_ICON_PATH             WPS_EXENAME
#define  DEFAULT_DATAFILE_ICON_INDEX            4
// folder object defaults.
#define  DEFAULT_FOLDER_TITLE                   "Folder"
#define  DEFAULT_FOLDER_ICON_PATH               WPS_EXENAME
#define  DEFAULT_FOLDER_ICON_INDEX              DEFAULT_GROUP_ICON_INDEX
// program object defaults.
#define  DEFAULT_PROGRAM_TITLE                  "Program"
#define  DEFAULT_PROGRAM_ICON_PATH              WPS_EXENAME
#define  DEFAULT_PROGRAM_ICON_INDEX             DEFAULT_ITEM_ICON_INDEX
// shredder object defaults.
#define  DEFAULT_SHREDDER_TITLE                 "Shredder"
#define  DEFAULT_SHREDDER_ICON_PATH             WPS_EXENAME
#define  DEFAULT_SHREDDER_ICON_INDEX            7
// default "templates" folder icon.
#define  DEFAULT_TEMPFOLDER_ICON_PATH           WPS_EXENAME
#define  DEFAULT_TEMPFOLDER_ICON_INDEX          8
// default template icons.
#define  DEFAULT_TEMPLATE_DATAFILE_ICON_PATH    WPS_EXENAME
#define  DEFAULT_TEMPLATE_DATAFILE_ICON_INDEX   12
#define  DEFAULT_TEMPLATE_FOLDER_ICON_PATH      WPS_EXENAME
#define  DEFAULT_TEMPLATE_FOLDER_ICON_INDEX     11
#define  DEFAULT_TEMPLATE_PROGRAM_ICON_PATH     WPS_EXENAME
#define  DEFAULT_TEMPLATE_PROGRAM_ICON_INDEX    10
#define  DEFAULT_TEMPLATE_SHREDDER_ICON_PATH    WPS_EXENAME
#define  DEFAULT_TEMPLATE_SHREDDER_ICON_INDEX   13

// System Setup Folder
#define  DEFAULT_SYSTEMSETUPFOLDER_ICON_INDEX   17
#define  DEFAULT_SYSTEMSETUP_ICON_INDEX         18

// Window System Folder
#define  DEFAULT_SYSTEMFOLDER_ICON_INDEX        0
#define  DEFAULT_STARTUPFOLDER_ICON_INDEX       31

// Minimzed Window Viewer
#define  DEFAULT_MINVIEWER_ICON_INDEX           2

//---------------------------------------------------------------------
// OBJECT TYPE DEFINITIONS.
//---------------------------------------------------------------------
// pure-base class.
#define OBJTYPE_OBJECT      0x00000000L
// base classes (1st level).
#define OBJTYPE_CONTAINER   0x00100000L
#define OBJTYPE_WINDOW      0x00200000L
#define OBJTYPE_PROGRAM     0x00400000L
#define OBJTYPE_TEMPLATE    0x00800000L
#define OBJTYPE_SHREDDER    0x01000000L
#define OBJTYPE_DRIVE       0x02000000L
#define OBJTYPE_PRINTER     0x04000000L
#define OBJTYPE_SHADOW      0x08000000L
#define OBJTYPE_DATAFILE    0x10000000L
#define OBJTYPE_WPSSETUP    0x20000000L
// derived classes (2nd level).
#define OBJTYPE_FOLDER      (OBJTYPE_CONTAINER|OBJTYPE_WINDOW|0x00010000L)
// derived classes (3rd level).
#define OBJTYPE_DESKTOP     (OBJTYPE_FOLDER|0x00000100L)
#define OBJTYPE_MINVIEWER   (OBJTYPE_FOLDER|0x00000200L)
// derived classes (2nd level).
#define OBJTYPE_SYSTEMSETUP (OBJTYPE_WPSSETUP|0x00010000L)
#define OBJTYPE_KEYBRDSETUP (OBJTYPE_WPSSETUP|0x00020000L)
#define OBJTYPE_MOUSESETUP  (OBJTYPE_WPSSETUP|0x00030000L)
#define OBJTYPE_SOUNDSETUP  (OBJTYPE_WPSSETUP|0x00040000L)


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

// default object flags.
#define DEF_FOLDER_FLAGS    (MINBEHAVIOR_HIDE|OPENBEHAVIOR_DISPLAY|BUTTONAPPERANCE_HIDE)
#define DEF_PROGRAM_FLAGS   (MINBEHAVIOR_MIN_TO_VIEWER|OPENBEHAVIOR_CREATE)

//---------------------------------------------------------------------
// LOCAL ENUMERATIONS
//---------------------------------------------------------------------
enum { VIEW_ICON=0, VIEW_TREE, VIEW_DETAILS };


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

// Workplace Shell profile magic identification number.
#define WPSHELL_PROFILE_MAGIC_NUMBER   0x4B454F48L

// WPS Profile Header data structure definition.
typedef struct {
   DWORD    m_dwMagicID;         // 'HOEK' 
   WORD     m_wMajorVersion;     // i.e. 0   = 0x0000  
   WORD     m_wMinorVersion;     // i.e. 5.1 = 0x0501
   DWORD    m_dwDataOffset;      // file offset to begining of data.
} WPSHELL_PROFILE_HEADER;


//---------------------------------------------------------------------
// WPOBJECT
typedef struct {
   DWORD    m_dwType;
   WORD     m_wUniqueID;
   char     m_szTitle[MAX_TITLE_NAME+1];
   POINT    m_ptIcon;
   char     m_szIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wIconIndex;
   WORD     m_wState;
   DWORD    m_dwFlags;
} FILE_OBJECT_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPCONTAINER
typedef struct {
   int      m_nObjects;
} FILE_CONTAINER_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPWINDOW
typedef struct {
   POINT    m_ptWindow;
   SIZE     m_sizeWindow;
   BOOL     m_fScrollable;
} FILE_WINDOW_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPFOLDER
typedef struct {
   // folder background color.
   COLORREF    m_rgbBackground;
   LOGFONT     m_lfIconText;
   COLORREF    m_clrrefIconTextColor;
   char        m_szImagePath[MAX_PATH_FILENAME+1];
   WORD        m_wOpenView;
} FILE_FOLDER_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPPROGRAM
typedef struct {
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];
      char     m_szParameters[MAX_PARAMETERS_LINE+1];
      char     m_szWorkingDir[MAX_WORKING_DIR+1];
} FILE_PROGRAM_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDATAFILE
typedef struct {         
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1];
      char     m_szDataPath[MAX_DATAFILE_PATH+1];
      char     m_szDataName[MAX_DATAFILE_NAME+1];
} FILE_DATAFILE_STRUCT;
//---------------------------------------------------------------------
           
//---------------------------------------------------------------------
// WPTEMPLATE
typedef struct {
   // template type information.
   DWORD    m_dwTemplateType;
   // template's object instance title string.
   char     m_szInstanceTitle[MAX_TITLE_NAME+1];
   // template object instance icon information.
   char     m_szInstanceIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wInstanceIconIndex;
} FILE_TEMPLATE_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPSHADOW   
typedef struct {
   WORD     m_wOriginalUniqueID;
} FILE_SHADOW_STRUCT;
//---------------------------------------------------------------------
   
   

//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECTS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FOLDER OBJECT
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
