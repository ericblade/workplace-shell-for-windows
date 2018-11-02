//=====================================================================
// PMTOWP.H
//    Program Manager to WorkPlace Shell convertion utility.
//
// Date Created :  02/01/94
// Last Update  :  12/16/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 02/01/94    | Creation.                               | MDA
//=====================================================================

//-----------------------------------------------------------------------------
// WPSHELL.EXE ICON RESOURCES IDs
//-----------------------------------------------------------------------------
// Icon Resources Base ID.
#define IDI_BASE_ID                   300
// Macro to get icon index from icon ID.
#define ICONINDEX(id)                 (id-IDI_BASE_ID)
#define ICONID(index)                 (index+IDI_BASE_ID)
// Windows 3.1 system icon.
#define IDI_WINSYS                    300
#define IDI_DESKTOP                   301
#define IDI_MINVIEWER                 302
// Default object icons.
#define IDI_DEFAULTOBJECT             303
#define IDI_DEFAULTDATA               304
#define IDI_DEFAULTPROGRAM            305
#define IDI_DEFAULTFOLDER             306
#define IDI_DEFAULTSHREDDER           307
// Template folder and object icons.
#define IDI_TEMPLATE_FOLDER           308
#define IDI_GENERICTEMPLATE           309
#define IDI_PROGTEMPLATE              310
#define IDI_FOLDERTEMPLATE            311
#define IDI_DATATEMPLATE              312
#define IDI_SHREDDERTEMPLATE          313
// Drives icons.
#define IDI_DRIVES_FOLDER             314
#define IDI_FLOPPYDRIVE               315
#define IDI_HARDDRIVE                 316

// Setup folder objects.
#define IDI_SYSTEMSETUP               317
#define IDI_SYSTEM                    318
#define IDI_MOUSE                     319
#define IDI_COUNTRY                   320
#define IDI_KEYBOARD                  321
#define IDI_SOUND                     322
#define IDI_SELECTIVEINSTALL          323
#define IDI_COLORPALETTE              324
#define IDI_FONTPALETTE               325

// Miscellaneous icons:
// Windows PROGMAN migration default icons.
#define IDI_PROGMANFOLDER             326
// Misc. desktop objects
#define IDI_OS2SYS                    327
#define IDI_GENERAL                   328
#define IDI_HELP                      329
#define IDI_INFO                      330
#define IDI_STARTUP                   331
#define IDI_TUTORIAL                  332
// Misc. multimedia.
#define IDI_MULTIMEDIAFOLDER          333
#define IDI_CDPLAYER                  334
#define IDI_DIGITALVIDEO              335
// Misc. DOS sessions.
#define IDI_DOSFULLSCREEN             336
#define IDI_DOSWINDOW                 337
   
// Templates #2
#define IDI_DRIVETEMPLATE             338

// Drives #2
#define IDI_CDROM_DRIVE               339
#define IDI_NETWORK_DRIVE             340
#define IDI_RAM_DRIVE                 341

// Misc. multimedia icons #2
#define IDI_MIDI_FILE                 342
#define IDI_AUDIO_FILE                343 


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define  WPS_EXENAME                "WPSHELL.EXE"
#define  MAX_TITLE_NAME             63
#define  MAX_PATH_FILENAME          63
#define  MAX_PROGRAM_PATH           63
#define  MAX_PARAMETERS_LINE        63
#define  MAX_WORKING_DIR            63

#define  MAX_CLASS_NAME             10     // 10 chars. max. for class name.

// default colors.
#define  DEFAULT_BACKGROUND_COLOR               ((fUseWarpColors==TRUE)? RGB(255,255,255) : GetSysColor(COLOR_WINDOW))
// default icons.
#define  DEFAULT_GROUP_ICON_PATH                WPS_EXENAME
#define  DEFAULT_GROUP_ICON_INDEX               ICONINDEX(IDI_DEFAULTFOLDER)
#define  DEFAULT_ITEM_ICON_PATH                 WPS_EXENAME
#define  DEFAULT_ITEM_ICON_INDEX                ICONINDEX(IDI_DEFAULTPROGRAM)
#define  DEFAULT_PROGMANFOLDER_ICON_INDEX       ICONINDEX(IDI_PROGMANFOLDER)
// folder object defaults.
#define  DEFAULT_DATAFILE_TITLE                 GetString(IDSTR_OBJTITLE_DATAFILE)
#define  DEFAULT_DATAFILE_ICON_PATH             WPS_EXENAME
#define  DEFAULT_DATAFILE_ICON_INDEX            ICONINDEX(IDI_DEFAULTDATA)
// folder object defaults.
#define  DEFAULT_FOLDER_TITLE                   GetString(IDSTR_OBJTITLE_FOLDER)
#define  DEFAULT_FOLDER_ICON_PATH               WPS_EXENAME
#define  DEFAULT_FOLDER_ICON_INDEX              DEFAULT_GROUP_ICON_INDEX
// program object defaults.
#define  DEFAULT_PROGRAM_TITLE                  GetString(IDSTR_OBJTITLE_PROGRAM)
#define  DEFAULT_PROGRAM_ICON_PATH              WPS_EXENAME
#define  DEFAULT_PROGRAM_ICON_INDEX             DEFAULT_ITEM_ICON_INDEX
// shredder object defaults.
#define  DEFAULT_SHREDDER_TITLE                 GetString(IDSTR_OBJTITLE_SHREDDER)
#define  DEFAULT_SHREDDER_ICON_PATH             WPS_EXENAME
#define  DEFAULT_SHREDDER_ICON_INDEX            ICONINDEX(IDI_DEFAULTSHREDDER)
// default "templates" folder icon.
#define  DEFAULT_TEMPFOLDER_ICON_PATH           WPS_EXENAME
#define  DEFAULT_TEMPFOLDER_ICON_INDEX          ICONINDEX(IDI_TEMPLATE_FOLDER)
// default template icons.
#define  DEFAULT_TEMPLATE_DATAFILE_ICON_PATH    WPS_EXENAME
#define  DEFAULT_TEMPLATE_DATAFILE_ICON_INDEX   ICONINDEX(IDI_DATATEMPLATE)
#define  DEFAULT_TEMPLATE_FOLDER_ICON_PATH      WPS_EXENAME
#define  DEFAULT_TEMPLATE_FOLDER_ICON_INDEX     ICONINDEX(IDI_FOLDERTEMPLATE)
#define  DEFAULT_TEMPLATE_PROGRAM_ICON_PATH     WPS_EXENAME
#define  DEFAULT_TEMPLATE_PROGRAM_ICON_INDEX    ICONINDEX(IDI_PROGTEMPLATE)
#define  DEFAULT_TEMPLATE_SHREDDER_ICON_PATH    WPS_EXENAME
#define  DEFAULT_TEMPLATE_SHREDDER_ICON_INDEX   ICONINDEX(IDI_SHREDDERTEMPLATE)

// System Setup Folder
#define  DEFAULT_SYSTEMSETUPFOLDER_ICON_INDEX   ICONINDEX(IDI_SYSTEMSETUP)
#define  DEFAULT_SYSTEMSETUP_ICON_INDEX         ICONINDEX(IDI_SYSTEM)
#define  DEFAULT_COLORSCHEME_ICON_INDEX         ICONINDEX(IDI_COLORPALETTE)
#define  DEFAULT_FONTPALETTE_ICON_INDEX         ICONINDEX(IDI_FONTPALETTE)
#define  DEFAULT_SOUND_ICON_INDEX               ICONINDEX(IDI_SOUND)
#define  DEFAULT_COUNTRY_ICON_INDEX             ICONINDEX(IDI_COUNTRY)
#define  DEFAULT_DEVICEDRIVER_ICON_INDEX        ICONINDEX(IDI_SELECTIVEINSTALL)
#define  DEFAULT_MOUSE_ICON_INDEX               ICONINDEX(IDI_MOUSE)
#define  DEFAULT_KEYBOARD_ICON_INDEX            ICONINDEX(IDI_KEYBOARD)
                           
// Window System Folder                          
#define  DEFAULT_DRIVES_ICON_INDEX              ICONINDEX(IDI_DRIVES_FOLDER)
#define  DEFAULT_SYSTEMFOLDER_ICON_INDEX        ICONINDEX(IDI_WINSYS)
#define  DEFAULT_STARTUPFOLDER_ICON_INDEX       ICONINDEX(IDI_STARTUP)

// Command Prompts Folder
#define  DEFAULT_DOSPROMPT_ICON_INDEX           ICONINDEX(IDI_DOSFULLSCREEN)

// Minimzed Window Viewer
#define  DEFAULT_MINVIEWER_ICON_INDEX           ICONINDEX(IDI_MINVIEWER)

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

enum { WPDRIVE_UNKNOWN=0, WPDRIVE_FLOPPY=2, WPDRIVE_HARD=3, 
       WPDRIVE_NETWORK=4, WPDRIVE_CDROM=5 , WPDRIVE_RAMDISK=6 };


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

// Workplace Shell profile magic identification number.
#define WPSHELL_PROFILE_MAGIC_NUMBER   0x4B454F48L

// WPS Profile Header data structure definition.
typedef struct {
   DWORD    m_dwMagicID;         // 'HOEK'
   WORD     m_wMajorVersion;     // i.e. 0   = 0x0000
   WORD     m_wMinorVersion;     // i.e. 4.1 = 0x0401
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
// WPDRIVE
typedef struct {
   UINT  m_uDriveIndex;
   WORD  m_Type;
} FILE_DRIVE_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// PROGRAM MANAGER OBJECTS MAPPED INTO WORKPLACE SHELL OBJECTS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// PROGMAN GROUP
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_WINDOW_STRUCT      m_wnd;
   FILE_FOLDER_STRUCT      m_fol;
   FILE_CONTAINER_STRUCT   m_con;
} FILE_PROGMAN_GROUP_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// PROGMAN ITEM
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_PROGRAM_STRUCT     m_prg;
} FILE_PROGMAN_ITEM_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WORKPLACE SHELL DEFAULT OBJECT COMPONENTS.
//---------------------------------------------------------------------

typedef FILE_PROGMAN_GROUP_STRUCT   FILE_WPS_FOLDER_STRUCT;
typedef FILE_PROGMAN_ITEM_STRUCT    FILE_WPS_PROGRAM_STRUCT;

//---------------------------------------------------------------------
// WPS TEMPLATE OBJECT 
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_TEMPLATE_STRUCT    m_tmp;
} FILE_WPS_TEMPLATE_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPS DRIVE OBJECT
typedef struct {
   FILE_OBJECT_STRUCT      m_obj;
   FILE_WINDOW_STRUCT      m_wnd;
   FILE_FOLDER_STRUCT      m_fol;
   FILE_DRIVE_STRUCT       m_drv;
   FILE_CONTAINER_STRUCT   m_con;
} FILE_WPS_DRIVE_STRUCT;
//---------------------------------------------------------------------
   
// EOF PMTOWP.H
   

