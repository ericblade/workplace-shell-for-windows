//=====================================================================
// WPSETUP.CPP
//    WPSETUP Class implementation.
// 
// Date Created :  04/02/94
// Last Update  :  09/30/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 04/02/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// SYSSETUP object default values.
#define WPS_WPSSETUP_DEFAULT_ICONINDEX  ICONINDEX(IDI_SYSTEMSETUP)


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WORKPLACE SHELL SYSSETUP CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSETUP():
//---------------------------------------------------------------------
WPSETUP::WPSETUP(void) : WPOBJECT()
{    
   // Initialize setup object.   
   SetType(OBJTYPE_WPSSETUP);
}

//---------------------------------------------------------------------
// WPSETUP():
//---------------------------------------------------------------------
WPSETUP::WPSETUP(WPSETUP *pwpsetupDefaults) : WPOBJECT((WPOBJECT *)pwpsetupDefaults)
{  
   // Initialize setup object.   
   SetType(OBJTYPE_WPSSETUP);
}

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPSETUP::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_WPSSETUP_DEFAULT_ICONINDEX;
}

// EOF WPSETUP.CPP

 