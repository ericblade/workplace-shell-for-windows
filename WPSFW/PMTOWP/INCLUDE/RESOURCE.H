//=====================================================================
// RESOURCE.H
//    PMTOWP (Program Manager To Workplace Shell) program resource ID
// header.
//
// Date Created :  09/26/94
// Last Update  :  12/21/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/26/94    | Creation.                               | MDA
//=====================================================================

//-----------------------------------------------------------------------------
// PRODUCT VERSION CONSTANTS
//-----------------------------------------------------------------------------
#define VER_MAJOR                2                 // Integer representing major version number.
#define VER_MINOR                0                 // Integer representing minor version number.
#define VER_STR_PRODUCTVERSION   "Release v2.0\0"  // String representing current version.
#define VER_STR_PRODUCTCOPYRIGHT "Copyright © IBM Corporation 1994-1996\0"


//-----------------------------------------------------------------------------
// DIALOG RESOURCES IDs
//-----------------------------------------------------------------------------
// Dialog IDs.
#define IDD_MAIN                          101

// Dialog Control IDs.
#define IDC_ALL_GROUPS                    1000
#define IDC_SELECTED_GROUP                1001
#define IDC_GROUP_LIST                    1003
#define IDC_FOLDER_NAME                   1004
#define IDC_SYSTEM_FOLDER                 1005
#define IDC_MINVIEWER                     1006
#define IDC_TEMPLATES                     1007
#define IDC_TEXT                          1008
#define IDC_STATIC                        -1


//-----------------------------------------------------------------------------
// STRING TABLE IDs
//-----------------------------------------------------------------------------
#define IDSTR_BASE_INDEX                  2000

#define IDSTR_PMTOWP_CAPTION              (IDSTR_BASE_INDEX+0)
#define IDSTR_PMTOWP_SUCCESS              (IDSTR_BASE_INDEX+1)
#define IDSTR_PMTOWP_FAILURE              (IDSTR_BASE_INDEX+2)
#define IDSTR_ERROR_CAPTION               (IDSTR_BASE_INDEX+3)
#define IDSTR_ERR_01                      (IDSTR_BASE_INDEX+4)
#define IDSTR_ERR_02                      (IDSTR_BASE_INDEX+5)
#define IDSTR_ERR_03                      (IDSTR_BASE_INDEX+6)

#define IDSTR_PM_FOLDER                   (IDSTR_BASE_INDEX+7)
#define IDSTR_FILE_EXISTS_QUERY           (IDSTR_BASE_INDEX+8)
#define IDSTR_NO_SELECTION                (IDSTR_BASE_INDEX+9)

#define IDSTR_OBJTITLE_COMMANDPROMPTS     (IDSTR_BASE_INDEX+10)
#define IDSTR_OBJTITLE_DOSPROMPT          (IDSTR_BASE_INDEX+11)
#define IDSTR_OBJTITLE_TEMPLATES          (IDSTR_BASE_INDEX+12)
#define IDSTR_OBJTITLE_DATAFILE           (IDSTR_BASE_INDEX+13)
#define IDSTR_OBJTITLE_FOLDER             (IDSTR_BASE_INDEX+14)
#define IDSTR_OBJTITLE_PROGRAM            (IDSTR_BASE_INDEX+15)
#define IDSTR_OBJTITLE_SHREDDER           (IDSTR_BASE_INDEX+16)
#define IDSTR_OBJTITLE_SYSTEMSETUP        (IDSTR_BASE_INDEX+17)
#define IDSTR_OBJTITLE_SYSTEM             (IDSTR_BASE_INDEX+18)
#define IDSTR_OBJTITLE_SCHEMEPALETTE      (IDSTR_BASE_INDEX+19)
#define IDSTR_OBJTITLE_FONTPALETTE        (IDSTR_BASE_INDEX+20)
#define IDSTR_OBJTITLE_SOUND              (IDSTR_BASE_INDEX+21)
#define IDSTR_OBJTITLE_COUNTRY            (IDSTR_BASE_INDEX+22)
#define IDSTR_OBJTITLE_DDINSTALL          (IDSTR_BASE_INDEX+23)
#define IDSTR_OBJTITLE_MOUSE              (IDSTR_BASE_INDEX+24)
#define IDSTR_OBJTITLE_KEYBOARD           (IDSTR_BASE_INDEX+25)
#define IDSTR_OBJTITLE_WINDOWSSYSTEM      (IDSTR_BASE_INDEX+26)
#define IDSTR_OBJTITLE_DRIVES             (IDSTR_BASE_INDEX+27)
#define IDSTR_OBJTITLE_DRIVE              (IDSTR_BASE_INDEX+28)
#define IDSTR_OBJTITLE_STARTUP            (IDSTR_BASE_INDEX+29)
#define IDSTR_OBJTITLE_MINVIEWER          (IDSTR_BASE_INDEX+30)

#define IDSTR_CONTROLPANEL_COLORS         (IDSTR_BASE_INDEX+31)
#define IDSTR_CONTROLPANEL_FONTS          (IDSTR_BASE_INDEX+32)
#define IDSTR_CONTROLPANEL_INTERNATIONAL  (IDSTR_BASE_INDEX+33)
#define IDSTR_CONTROLPANEL_MOUSE          (IDSTR_BASE_INDEX+34)
#define IDSTR_CONTROLPANEL_KEYBOARD       (IDSTR_BASE_INDEX+35)

// Do not move or modify this last constant!
#define IDSTR_LAST_INDEX                  (IDSTR_BASE_INDEX+35)

#define TOTAL_STRINGS                     (IDSTR_LAST_INDEX-IDSTR_BASE_INDEX+1)


// EOF RESOURCE.H

