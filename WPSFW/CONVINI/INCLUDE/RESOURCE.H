//=====================================================================
// RESOURCE.H
//    CONVINI (Convert Initialization File) program resource ID header.
//
// Date Created :  09/26/94
// Last Update  :  09/14/95
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
#define IDD_MAIN                       101

// Dialog Control IDs (NO NLS MAPPING REQUIRED).
#define IDE_SOURCEPROFILE              1000
#define IDC_STATIC                     -1

//-----------------------------------------------------------------------------
// STRING TABLE IDs
//-----------------------------------------------------------------------------
#define IDSTR_BASE_INDEX               2000

#define IDSTR_CONVERT_CAPTION          (IDSTR_BASE_INDEX+0)
#define IDSTR_CONVERT_SUCCESS          (IDSTR_BASE_INDEX+1)
#define IDSTR_CONVERT_FAILURE          (IDSTR_BASE_INDEX+2)
#define IDSTR_ERROR_CAPTION            (IDSTR_BASE_INDEX+3)
#define IDSTR_ERR_10                   (IDSTR_BASE_INDEX+4)
#define IDSTR_ERR_11                   (IDSTR_BASE_INDEX+5)
#define IDSTR_ERR_12                   (IDSTR_BASE_INDEX+6)
#define IDSTR_ERR_13                   (IDSTR_BASE_INDEX+7)
#define IDSTR_ERR_20                   (IDSTR_BASE_INDEX+8)
#define IDSTR_ERR_21                   (IDSTR_BASE_INDEX+9)
#define IDSTR_ERR_22                   (IDSTR_BASE_INDEX+10)
#define IDSTR_ERR_23                   (IDSTR_BASE_INDEX+11)
#define IDSTR_ERR_40                   (IDSTR_BASE_INDEX+12)

// Do not move or modify this last constant!
#define IDSTR_LAST_INDEX               (IDSTR_BASE_INDEX+12)

#define TOTAL_STRINGS                  (IDSTR_LAST_INDEX-IDSTR_BASE_INDEX+1)

// EOF RESOURCE.H

