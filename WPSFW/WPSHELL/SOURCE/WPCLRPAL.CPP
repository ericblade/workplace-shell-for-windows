//=====================================================================
// WPCLRPAL.CPP
//    WPCOLORPALETTE Class implementation.
//
// Date Created :  01/04/95
// Last Update  :  01/04/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/04/95    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
// Workplace Shell resource ID's header file.

//---------------------------------------------------------------------
// LOCAL CONSTANTS 
//---------------------------------------------------------------------
// Color palette object default values.
#define WPS_COLORPALETTE_DEFAULT_ICONINDEX   ICONINDEX(IDI_COLORPALETTE)


//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static COLORREF gs_aclrrefDefaults[MAX_PALETTE_COLORS] =
   { RGB(),RGB(),RGB(),... };
   
   



//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPCOLORPALETTE
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPCOLORPALETTE:WPCOLORPALETTE(void) : WPPALETTE()
{                        
   // Initialize folder object.
   SetType(OBJTYPE_COLORPALETTE);
   // Initialize color palette to contain a default color palette.
   InitializeDefaultColors();
}


//---------------------------------------------------------------------
// WPCOLORPALETTE():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPCOLORPALETTE::WPCOLORPALETTE(WPCOLORPALETTE *pwpclrpalDefaults) : 
   WPPALETTE((WPPALETTE *)pwpclrpalDefaults)

{     
   int nIndex;
   
   // Initialize program object.   
   SetType(OBJTYPE_COLORPALETTE);
   // Copy the default object's color palette into new object's palette.
   for (nIndex=0; nIndex<MAX_PALETTE_COLORS; nIndex++)
      SetPaletteColor(nIndex,pwpclrpalDefaults->GetPaletteColor(nIndex));
}
                             
//---------------------------------------------------------------------
// COLOR PALETTE DEFAULT VALUES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPCOLORPALETTE::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_COLORPALETTE_DEFAULT_ICONINDEX;
}
                   
//---------------------------------------------------------------------
// SetPaletteColor():
//---------------------------------------------------------------------
void WPCOLORPALETTE::SetPaletteColor(int nIndex,COLORREF clrrefColor)
{
   m_clrrefPalette[nIndex] = clrrefColor;
}
                   
//---------------------------------------------------------------------
// GetPaletteColor():
//---------------------------------------------------------------------
COLORREF WPCOLORPALETTE::GetPaletteColor(int nIndex)
{
   return (m_clrrefPalette[nIndex]);
}
                   
//---------------------------------------------------------------------
// InitializeDefaultColors():
//---------------------------------------------------------------------
void WPCOLORPALETTE::InitializeDefaultColors(void)
{
   int   nIndex;
   
   memcpy((void *)m_clrrefPalette,(void *)g_aclrrefDefaults,sizeof(g_aclrrefDefaults));
   
}
   
   
                             
//---------------------------------------------------------------------
//  
//---------------------------------------------------------------------
   

// EOF WPCLRPAL.CPP

