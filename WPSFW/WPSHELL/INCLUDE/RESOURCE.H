//=====================================================================
// RESOURCE.H
//    WPSHELL manager for Windows resource header file.
//
// Date Created :  01/26/94
// Last Update  :  12/21/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/26/94    | Creation.                               | MDA
//=====================================================================

//-----------------------------------------------------------------------------
// PRODUCT VERSION CONSTANTS
//-----------------------------------------------------------------------------
#define VER_MAJOR                2                 // Integer representing major version number.
#define VER_MINOR                0                 // Integer representing minor version number.
#define VER_STR_PRODUCTVERSION   "Release v2.0\0"  // String representing current version.
#define VER_STR_PRODUCTCOPYRIGHT "Copyright © IBM Corporation 1994-1996\0"


//-----------------------------------------------------------------------------
// ACCELERATOR TABLE IDs
//-----------------------------------------------------------------------------
// Accelerator Tables IDs
#define IDA_WPSHELL                   100
// Event IDs
#define ID_EVENT_MINIMUM_ID           80
#define ID_EVENT_SPECIAL              80
#define ID_EVENT_PREVIOUS             81
#define ID_EVENT_NEXT                 82
#define ID_EVENT_LEFT                 83
#define ID_EVENT_RIGHT                84
#define ID_EVENT_UP                   85
#define ID_EVENT_DOWN                 86
#define ID_EVENT_PAGEDOWN             87
#define ID_EVENT_PAGEUP               88
#define ID_EVENT_OPEN                 89
#define ID_EVENT_SETTINGS             90
#define ID_EVENT_DELETE               91
#define ID_EVENT_HELP                 92
#define ID_EVENT_SELECTALL            93
#define ID_EVENT_DESELECTALL          94
#define ID_EVENT_DESKTOP_POPUP        95
#define ID_EVENT_MAXIMUM_ID           95


//-----------------------------------------------------------------------------
// POPUP MENUS IDs
//-----------------------------------------------------------------------------
// System Menus IDs
#define DesktopPopUpMenu                  200
#define FolderPopUpMenu                   201
#define ObjectPopUpMenu                   202
#define ProgramPopUpMenu                  203
#define DataFilePopUpMenu                 204
#define TemplatePopUpMenu                 205
#define ShredderPopUpMenu                 206
#define MinimizedWindowViewerPopUpMenu    207
#define MinimizedWindowObjectPopUpMenu    208
// Menu Entries
#define IDM_MINIMUM_ID                10
#define IDM_OPENSETTINGS              10
#define IDM_OPENICON                  11
#define IDM_OPENTREE                  12
#define IDM_OPENDETAILS               13
#define IDM_OPENLIST                  14
#define IDM_OPENPROGRAM               15
#define IDM_OPENDATAFILE              16
#define IDM_OPENPARENT                17
#define IDM_REFRESHNOW                18
#define IDM_HELPINDEX                 19
#define IDM_HELPGENERAL               20
#define IDM_HELPUSING                 21
#define IDM_HELPKEYS                  22
#define IDM_HELPPRODINFO              23
#define IDM_CREATESHADOW              24
#define IDM_LOCKUPNOW                 25
#define IDM_SHUTDOWN                  26
#define IDM_SYSTEMSETUP               27
#define IDM_CREATEANOTHER             28
#define IDM_COPY                      29
#define IDM_MOVE                      30
#define IDM_WINDOWRESTORE             31
#define IDM_WINDOWMOVE                32
#define IDM_WINDOWSIZE                33
#define IDM_WINDOWMINIMIZE            34
#define IDM_WINDOWMAXIMIZE            35
#define IDM_WINDOWHIDE                36
#define IDM_FIND                      37
#define IDM_SELECTALL                 38
#define IDM_DESELECTALL               39
#define IDM_DELETE                    40
#define IDM_SORTBYNAME                41
#define IDM_SORTBYTYPE                42
#define IDM_SORTBYREALNAME            43
#define IDM_SORTBYSIZE                44
#define IDM_SORTBYLASTWRITEDATE       45
#define IDM_SORTBYLASTWRITETIME       46
#define IDM_SORTBYLASTACCESSDATE      47
#define IDM_SORTBYLASTACCESSTIME      48
#define IDM_SORTBYCREATIONDATE        49
#define IDM_SORTBYCREATIONTIME        50
#define IDM_ARRANGE                   51
#define IDM_CLOSE                     52
#define IDM_SAVEDESKTOP               53
#define IDM_RUN                       54
#define IDM_PRINT                     55
#define IDM_WINDOWLIST                56
#define IDM_ORIGINAL_LOCATE           57
#define IDM_ORIGINAL_COPY             58
#define IDM_ORIGINAL_DELETE           59
#define IDM_INSTALL                   60
#define IDM_MAXIMUM_ID                60


//-----------------------------------------------------------------------------
// ICONS RESOURCES IDs
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

// Icons used in Workplace Shell cursor creation.
#define IDI_NOSIGN                    344
#define IDI_COPYGRID                  345
#define IDI_SHADOWCIRCLE              346
// Non-client area window effects.
#define IDI_BUTTON_EFFECT             347


//-----------------------------------------------------------------------------
// CURSORS
//-----------------------------------------------------------------------------
#define IDC_WAITCURSOR                400
#define IDC_SPECIAL                   401

//-----------------------------------------------------------------------------
// BITMAPS
//-----------------------------------------------------------------------------
// Minimize/Maximize Window Buttons Bitmaps

// High-Resolution (1024x768).
// Style 1.
#define IDB_HIRES_1_MINMAX_UP         500
#define IDB_HIRES_1_MIN_DOWN          501
#define IDB_HIRES_1_MAX_DOWN          502
#define IDB_HIRES_1_X_MINMAX_UP       503
#define IDB_HIRES_1_X_MIN_DOWN        504
#define IDB_HIRES_1_X_MAX_DOWN        505
// Style 2.
#define IDB_HIRES_2_MINMAX_UP         506
#define IDB_HIRES_2_MIN_DOWN          507
#define IDB_HIRES_2_MAX_DOWN          508
#define IDB_HIRES_2_X_MINMAX_UP       509
#define IDB_HIRES_2_X_MIN_DOWN        510
#define IDB_HIRES_2_X_MAX_DOWN        511

// Low-Resolution (640x480 and 800x600).
// Style 1
#define IDB_LORES_1_MINMAX_UP         512
#define IDB_LORES_1_MIN_DOWN          513
#define IDB_LORES_1_MAX_DOWN          514
#define IDB_LORES_1_X_MINMAX_UP       515
#define IDB_LORES_1_X_MIN_DOWN        516
#define IDB_LORES_1_X_MAX_DOWN        517
// Style 2.
#define IDB_LORES_2_MINMAX_UP         518
#define IDB_LORES_2_MIN_DOWN          519
#define IDB_LORES_2_MAX_DOWN          520
#define IDB_LORES_2_X_MINMAX_UP       521
#define IDB_LORES_2_X_MIN_DOWN        522
#define IDB_LORES_2_X_MAX_DOWN        523



// Scrollbars Components Bitmaps

// High-Resolution (1024x768)
#define IDB_HIRES_SB_THUMB            550
#define IDB_HIRES_SB_THUMB_PUSH       551
#define IDB_HIRES_SB_LEFT             552
#define IDB_HIRES_SB_RIGHT            553
#define IDB_HIRES_SB_LEFT_PUSH        554
#define IDB_HIRES_SB_RIGHT_PUSH       555
#define IDB_HIRES_SB_UP               556
#define IDB_HIRES_SB_DOWN             557
#define IDB_HIRES_SB_UP_PUSH          558
#define IDB_HIRES_SB_DOWN_PUSH        559

// Low-Resolution (640x480 and 800x600).
#define IDB_LORES_SB_THUMB            560
#define IDB_LORES_SB_THUMB_PUSH       561
#define IDB_LORES_SB_LEFT             562
#define IDB_LORES_SB_RIGHT            563
#define IDB_LORES_SB_LEFT_PUSH        564
#define IDB_LORES_SB_RIGHT_PUSH       565
#define IDB_LORES_SB_UP               566
#define IDB_LORES_SB_DOWN             567
#define IDB_LORES_SB_UP_PUSH          568
#define IDB_LORES_SB_DOWN_PUSH        569

// Special bitmaps.
#define IDB_SPECIAL_BITMAP            570
#define IDB_SPECIAL_MASK              571


//---------------------------------------------------------------------
// DIALOGS IDs
//---------------------------------------------------------------------
// 'Help->Product Info" dialog (from desktop's menu).
#define IDD_PRODUCTINFO                 600
// 'Find...' dialog (from folder object's menu).
#define IDD_FIND                        601
// 'Results' dialog (used by 'Find' dialog when objects found).
#define IDD_FINDRESULTS                 602
// 'Copy', 'Move', 'Create shadow' dialog (used in some objects).
#define IDD_COPYMOVECREATE              603
// 'Find Icon...' dialog (from 'General' page in all objects).
#define IDD_FINDICON                    609

//---------------------------------------------------------------------
// DEFAULT NOTEBOOK PAGE (DAILOG BOX)
//---------------------------------------------------------------------
#define IDD_DEFAULT_NOTEBOOK_PAGE       610

//---------------------------------------------------------------------
// SETTINGS NOTEBOOK PAGES (DIALOG BOXES)
//---------------------------------------------------------------------
// Folder object settings notebook pages (modeless dialog boxes).
#define IDD_VIEW_PAGE                   611
#define IDD_BACKGROUND_PAGE             612
#define IDD_WINDOW_PAGE                 613
#define IDD_GENERAL_PAGE                614
// Program object settings notebook pages (modeless dialog boxes).
#define IDD_PROGRAM_PAGE                615
#define IDD_ASSOCIATION_PAGE            616
// Data File object settings notebook pages (modeless dialog boxes).
#define IDD_TYPE_PAGE                   617
#define IDD_FILE_PAGE                   618
// Desktop object settings notebook pages (modeless dialog boxes).
#define IDD_LOCKUP_PAGE                 619
#define IDD_DESKTOP_PAGE                620
#define IDD_CREDITS_PAGE                621
#define IDD_CREDITS2_PAGE               622
// System setup settings notebook pages
#define IDD_SYSSETUP_CONFIRMATION_PAGE  623
#define IDD_SYSSETUP_WINDOW_PAGE        624
#define IDD_SYSSETUP_WINDOW2_PAGE       625
// System setup settings notebook pages
#define IDD_DRIVE_PAGE                  626

//---------------------------------------------------------------------
// RUN DIALOG ID
//---------------------------------------------------------------------
// "Run..." program dialog box.
#define IDD_RUN                         627


//---------------------------------------------------------------------
// "PRODUCT INFORMATION" DIALOG CONTROLS
//---------------------------------------------------------------------
#define IDS_PRODUCTNAME                 10
#define IDS_PRODUCTVERSION              11
#define IDS_PRODUCTCOPYRIGHT            12

//---------------------------------------------------------------------
// "FIND" DIALOG CONTROLS
//---------------------------------------------------------------------
#define IDE_TITLETEXT                   10
#define IDB_PROGRAM                     11
#define IDB_DATAFILE                    12
#define IDB_FOLDER                      13
#define IDB_EXACTMATCH                  14
#define IDB_STOPONMATCH                 15
#define IDB_CREATERESULT                16

//---------------------------------------------------------------------
// "RESULTS" DIALOG CONTROLS
//---------------------------------------------------------------------
#define IDL_PROGRAMLIST                 10
#define IDL_DATAFILELIST                11
#define IDL_FOLDERLIST                  12

//---------------------------------------------------------------------
// "COPY", "MOVE", AND "CREATE SHADOW" DIALOG CONTROLS
//---------------------------------------------------------------------
#define IDT_NEWNAME                     10
#define IDE_NEWNAME                     11
#define IDL_TARGETFOLDER                12


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK DIALOG BOXES
//---------------------------------------------------------------------
// "Find..." icon dialog box control's IDs.
#define IDE_ICONPATH                    10
#define IDB_BROWSE                      11
#define IDL_ICONPREVIEWLIST             12


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK CONTROLS
//---------------------------------------------------------------------
// Common IDs (used by 2 or more pages).
#define IDC_STATIC                      -1
#define IDB_UNDO                        5    // used in every page.
#define IDB_HELP                        6    // used in every page.
#define IDB_DEFAULT                     7    //
#define IDB_FIND                        8    //

// 'Program' page (WPPROGRAM objects).
#define IDE_PATHFILE                    10
#define IDE_PARAMETERS                  11
#define IDE_WORKDIR                     12

// 'Association' page (WPPROGRAM objects).
#define IDL_AVAILTYPES                  10
#define IDB_STARTNORMAL                 11
#define IDB_STARTMINIMIZED              12
#define IDB_STARTMAXIMIZED              13

// 'File' page (WPDATAFILE objects).
#define IDE_SUBJECT                     10
#define IDE_PATH                        11
#define IDE_NAME                        12

// 'Type' page (WPDATAFILE objects).
#define IDE_TYPE                        10

// 'View' page (WPFOLDER objects).
#define IDB_STARTUP                     10
#define IDB_FLOWED                      11
#define IDB_NONFLOWED                   12
#define IDB_NONGRID                     13
#define IDE_FONT                        14
#define IDB_CHANGEFONT                  15
#define IDB_TRANSPARENTBKGND            16

// 'Background' page (WPFOLDER objects).
#define IDB_COLORONLY                   10
#define IDB_CHANGECOLOR                 11
#define IDS_PREVIEWAREA                 12
#define IDB_IMAGEGROUP                  13
#define IDCB_BITMAPLIST                 14
#define IDS_FILETEXT                    15
#define IDB_NORMALIMAGE                 16
#define IDB_TILEDIMAGE                  17
#define IDB_SCALEDIMAGE                 18

// 'Window' page (WPFOLDER and WPPROGRAM objects).
#define IDS_BUTTON_APPEARANCE_GRPBX     10
#define IDB_HIDEBUTTON                  11
#define IDB_MINIMIZEBUTTON              12
#define IDB_MINHIDE                     13
#define IDB_MINTOVIEWER                 14
#define IDB_MINTODESKTOP                15
#define IDB_DISPLAY                     16
#define IDB_CREATE                      17
#define IDB_ENABLED                     18
#define IDB_DISABLED                    19
#define IDB_PROPAGATECHANGES            20  // Used only in WPSYSSETUP object.
#define IDB_NEVER                       21  // Used only in WPSYSSETUP object.
#define IDB_SUBFOLDERS_ONLY             22  // Used only in WPSYSSETUP object.
#define IDB_ALLOBJECTS                  23  // Used only in WPSYSSETUP object.

// 'General' page (all objects).
#define IDE_TITLE                       10
#define IDB_CREATEANOTHER               11
#define IDB_EDIT                        12
#define IDB_TEMPLATE                    13
#define IDS_ICON                        14

// 'Lockup' page (WPDESKTOP object).
#define IDB_NO_AUTOMATIC                10
#define IDB_AUTOMATIC                   11
#define IDSB_TIMEOUT                    12

// 'Desktop' page (WPDESKTOP object).
#define IDB_SAVEDESKTOPSETTINGS         10

// 'Confirmations' page (WPSYSSETUP object).
#define IDB_FOLDER_DELETE               10
#define IDB_DELETE                      11
#define IDB_FILE_SYSTEM_DELETE          12

// 'Drive' page (WPDRIVE object).
#define IDL_DRIVES                      10


// "Run..." program dialog control IDs.
#define IDE_COMMANDLINE                 10
#define IDB_NORMAL                      11
#define IDB_MINIMIZED                   12
#define IDB_MAXIMIZED                   13
#define IDB_RUNBROWSE                   14


//-----------------------------------------------------------------------------
// STRING TABLE IDs
//-----------------------------------------------------------------------------
#define IDSTR_BASE_INDEX                     2000

#define IDSTR_WPSFWIN                        (IDSTR_BASE_INDEX+0)
#define IDSTR_COPYRIGHT                      (IDSTR_BASE_INDEX+1)

#define IDSTR_ERR_CAPTION                    (IDSTR_BASE_INDEX+2)
#define IDSTR_ERR_WARN_CAPTION               (IDSTR_BASE_INDEX+3)
#define IDSTR_ERR_CREATE_MSG_QUEUE           (IDSTR_BASE_INDEX+4)
#define IDSTR_ERR_WPSHELL_INI_FILE           (IDSTR_BASE_INDEX+5)
#define IDSTR_ERR_RUN                        (IDSTR_BASE_INDEX+6)
#define IDSTR_ERR_OBJEXEC                    (IDSTR_BASE_INDEX+7)
#define IDSTR_ERR_SAVE_DSKTOP_CAPTION        (IDSTR_BASE_INDEX+8)
#define IDSTR_ERR_SAVE_DSKTOP                (IDSTR_BASE_INDEX+9)
#define IDSTR_ERR_FIND_FOLDER_CAPTION        (IDSTR_BASE_INDEX+10)
#define IDSTR_ERR_SYSSETUP_NOT_FOUND         (IDSTR_BASE_INDEX+11)
#define IDSTR_ERR_TOO_MANY_OBJECTS           (IDSTR_BASE_INDEX+12)
#define IDSTR_ERR_NO_MORE_SHADOWS            (IDSTR_BASE_INDEX+13)
#define IDSTR_ERR_CANNOT_RUN_INSTALL         (IDSTR_BASE_INDEX+14)

#define IDSTR_SHUTDOWN_CAPTION               (IDSTR_BASE_INDEX+15)
#define IDSTR_SAVE_DESKTOP_QUESTION          (IDSTR_BASE_INDEX+16)

#define IDSTR_OBJ_CONFIRM_ON_DELETE_CAPTION  (IDSTR_BASE_INDEX+17)
#define IDSTR_OBJ_CONFIRM_ON_DELETE          (IDSTR_BASE_INDEX+18)
#define IDSTR_OBJ_CONFIRM_ON_FILESYS_DELETE  (IDSTR_BASE_INDEX+19)
#define IDSTR_FOL_CONFIRM_ON_DELETE_CAPTION  (IDSTR_BASE_INDEX+20)
#define IDSTR_FOL_CONFIRM_ON_DELETE          (IDSTR_BASE_INDEX+21)
#define IDSTR_FOL_CONFIRM_ON_FILESYS_DELETE  (IDSTR_BASE_INDEX+22)

#define IDSTR_VIEW_ICON                      (IDSTR_BASE_INDEX+23)
#define IDSTR_VIEW_TREE                      (IDSTR_BASE_INDEX+24)
#define IDSTR_VIEW_DETAILS                   (IDSTR_BASE_INDEX+25)
#define IDSTR_VIEW_CAPTION_TITLE             (IDSTR_BASE_INDEX+26)
#define IDSTR_DETAILS_ICON_COLUMN            (IDSTR_BASE_INDEX+27)
#define IDSTR_DETAILS_TITLE_COLUMN           (IDSTR_BASE_INDEX+28)
#define IDSTR_DETAILS_NAME_COLUMN            (IDSTR_BASE_INDEX+29)
#define IDSTR_DETAILS_SIZE_COLUMN            (IDSTR_BASE_INDEX+30)
#define IDSTR_DETAILS_DATE_COLUMN            (IDSTR_BASE_INDEX+31)
#define IDSTR_DETAILS_TIME_COLUMN            (IDSTR_BASE_INDEX+32)

#define IDSTR_SHADOWMENU_ORIGINAL            (IDSTR_BASE_INDEX+33)
#define IDSTR_SHADOWMENU_ORIGINAL_LOCATE     (IDSTR_BASE_INDEX+34)
#define IDSTR_SHADOWMENU_ORIGINAL_COPY       (IDSTR_BASE_INDEX+35)
#define IDSTR_SHADOWMENU_ORIGINAL_DELETE     (IDSTR_BASE_INDEX+36)
#define IDSTR_DRIVEMENU_INSTALL              (IDSTR_BASE_INDEX+37)

#define IDSTR_YES                            (IDSTR_BASE_INDEX+38)
#define IDSTR_NO                             (IDSTR_BASE_INDEX+39)
#define IDSTR_HIDE                           (IDSTR_BASE_INDEX+40)
#define IDSTR_MINIMIZE                       (IDSTR_BASE_INDEX+41)
#define IDSTR_ENABLED                        (IDSTR_BASE_INDEX+42)
#define IDSTR_DISABLED                       (IDSTR_BASE_INDEX+43)
#define IDSTR_MINTOVIEWER                    (IDSTR_BASE_INDEX+44)
#define IDSTR_MINTODESKTOP                   (IDSTR_BASE_INDEX+45)
#define IDSTR_CREATENEW                      (IDSTR_BASE_INDEX+46)
#define IDSTR_DISPLAYEXISTING                (IDSTR_BASE_INDEX+47)
#define IDSTR_SYSTEMSETUP                    (IDSTR_BASE_INDEX+48)
#define IDSTR_NEVER                          (IDSTR_BASE_INDEX+49)
#define IDSTR_SUBFOLDERS_ONLY                (IDSTR_BASE_INDEX+50)
#define IDSTR_ALL_OBJECTS                    (IDSTR_BASE_INDEX+51)

#define IDSTR_WIN_SYSTEM_PROFILE             (IDSTR_BASE_INDEX+52)
#define IDSTR_WPSFWIN_SECTION_NAME           (IDSTR_BASE_INDEX+53)
#define IDSTR_DESKTOP_HORZICONINDENT         (IDSTR_BASE_INDEX+54)
#define IDSTR_DESKTOP_VERTICONINDENT         (IDSTR_BASE_INDEX+55)
#define IDSTR_SYSTEM_CONFIRMFOLDERDELETE     (IDSTR_BASE_INDEX+56)
#define IDSTR_SYSTEM_CONFIRMDELETE           (IDSTR_BASE_INDEX+57)
#define IDSTR_SYSTEM_CONFIRMFILEDELETE       (IDSTR_BASE_INDEX+58)
#define IDSTR_SYSTEM_BUTTONAPPEARANCE        (IDSTR_BASE_INDEX+59)
#define IDSTR_SYSTEM_ANIMATION               (IDSTR_BASE_INDEX+60)
#define IDSTR_SYSTEM_MINBEHAVIOR             (IDSTR_BASE_INDEX+61)
#define IDSTR_SYSTEM_OPENBEHAVIOR            (IDSTR_BASE_INDEX+62)
#define IDSTR_SYSTEM_SYSTEMSETUPFOLDERNAME   (IDSTR_BASE_INDEX+63)
#define IDSTR_SYSTEM_NOSAVE                  (IDSTR_BASE_INDEX+64)
#define IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE    (IDSTR_BASE_INDEX+65)
#define IDSTR_COLOR_SHADOWTEXT               (IDSTR_BASE_INDEX+66)

#define IDSTR_SHELL_FILENAME                 (IDSTR_BASE_INDEX+67)
#define IDSTR_DESKTOP_FILENAME               (IDSTR_BASE_INDEX+68)
#define IDSTR_HELP_FILENAME                  (IDSTR_BASE_INDEX+69)
#define IDSTR_WIN_VERSION_MESSAGE            (IDSTR_BASE_INDEX+70)
#define IDSTR_NONE_PARENTHESIS               (IDSTR_BASE_INDEX+71)
#define IDSTR_IMAGE_NONE                     (IDSTR_BASE_INDEX+72)
#define IDSTR_FILEDLG_TITLE_RUN              (IDSTR_BASE_INDEX+73)
#define IDSTR_FILEDLG_TITLE_BROWSE           (IDSTR_BASE_INDEX+74)
#define IDSTR_FILEDLG_TITLE_FIND_PROGRAM     (IDSTR_BASE_INDEX+75)
#define IDSTR_FILEDLG_TITLE_FIND_DATAFILE    (IDSTR_BASE_INDEX+76)
#define IDSTR_CMCDLG_TITLE_COPY              (IDSTR_BASE_INDEX+77)
#define IDSTR_CMCDLG_TITLE_MOVE              (IDSTR_BASE_INDEX+78)
#define IDSTR_CMCDLG_TITLE_CREATEANOTHER     (IDSTR_BASE_INDEX+79)
#define IDSTR_CMCDLG_TITLE_CREATESHADOW      (IDSTR_BASE_INDEX+80)
#define IDSTR_CMCDLG_PUSHBUTTON_COPY         (IDSTR_BASE_INDEX+81)
#define IDSTR_CMCDLG_PUSHBUTTON_MOVE         (IDSTR_BASE_INDEX+82)
#define IDSTR_CMCDLG_PUSHBUTTON_CREATE       (IDSTR_BASE_INDEX+83)
#define IDSTR_CMCDLG_PUSHBUTTON_SHADOW       (IDSTR_BASE_INDEX+84)
#define IDSTR_FINDRESULTS_CAPTION            (IDSTR_BASE_INDEX+85)

#define IDSTR_SETTNB_DEFAULT_TITLE           (IDSTR_BASE_INDEX+86)
#define IDSTR_SETTNB_TITLE                   (IDSTR_BASE_INDEX+87)
#define IDSTR_SETTNB_STATUSTEXT              (IDSTR_BASE_INDEX+88)
#define IDSTR_SETTNB_PAGE_GENERAL            (IDSTR_BASE_INDEX+89)
#define IDSTR_SETTNB_PAGE_VIEW               (IDSTR_BASE_INDEX+90)
#define IDSTR_SETTNB_PAGE_BACKGROUND         (IDSTR_BASE_INDEX+91)
#define IDSTR_SETTNB_PAGE_WINDOW             (IDSTR_BASE_INDEX+92)
#define IDSTR_SETTNB_PAGE_PROGRAM            (IDSTR_BASE_INDEX+93)
#define IDSTR_SETTNB_PAGE_ASSOCIATION        (IDSTR_BASE_INDEX+94)
#define IDSTR_SETTNB_PAGE_TYPE               (IDSTR_BASE_INDEX+95)
#define IDSTR_SETTNB_PAGE_FILE               (IDSTR_BASE_INDEX+96)
#define IDSTR_SETTNB_PAGE_LOCKUP             (IDSTR_BASE_INDEX+97)
#define IDSTR_SETTNB_PAGE_DESKTOP            (IDSTR_BASE_INDEX+98)
#define IDSTR_SETTNB_PAGE_CONFIRMATION       (IDSTR_BASE_INDEX+99)
#define IDSTR_SETTNB_PAGE_DRIVE              (IDSTR_BASE_INDEX+100)
#define IDSTR_SETTNB_PAGE_CREDITS            (IDSTR_BASE_INDEX+101)

#define IDSTR_OBJTITLE_GENERIC               (IDSTR_BASE_INDEX+102)
#define IDSTR_OBJTITLE_FOLDER                (IDSTR_BASE_INDEX+103)
#define IDSTR_OBJTITLE_PROGRAM               (IDSTR_BASE_INDEX+104)
#define IDSTR_OBJTITLE_DATAFILE              (IDSTR_BASE_INDEX+105)
#define IDSTR_OBJTITLE_SHREDDER              (IDSTR_BASE_INDEX+106)
#define IDSTR_OBJTITLE_SYSTEM                (IDSTR_BASE_INDEX+107)
#define IDSTR_OBJTITLE_DESKTOP               (IDSTR_BASE_INDEX+108)
#define IDSTR_OBJTITLE_DRIVE                 (IDSTR_BASE_INDEX+109)

#define IDSTR_DRIVETYPE_UNKNOWN              (IDSTR_BASE_INDEX+110)
#define IDSTR_DRIVETYPE_FLOPPY               (IDSTR_BASE_INDEX+111)
#define IDSTR_DRIVETYPE_HARD                 (IDSTR_BASE_INDEX+112)
#define IDSTR_DRIVETYPE_NETWORK              (IDSTR_BASE_INDEX+113)
#define IDSTR_DRIVETYPE_CDROM                (IDSTR_BASE_INDEX+114)
#define IDSTR_DRIVETYPE_RAMDISK              (IDSTR_BASE_INDEX+115)


// Do not move or modify this last constant!
#define IDSTR_LAST_INDEX                     (IDSTR_BASE_INDEX+115)


#define TOTAL_STRINGS                        (IDSTR_LAST_INDEX-IDSTR_BASE_INDEX+1)


// EOF RESOURCE.H

