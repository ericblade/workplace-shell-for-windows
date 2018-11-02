//=====================================================================
// WPCLRPAL.HPP
//    WPCOLORPALETTE Class declaration.
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

#ifndef __WPCOLORPALETTE_PROTOTYPED__

#define __WPCOLORPALETTE_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL ENUMERATIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define MAX_PALETTE_COLORS    32
        
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPCOLORPALETTE: public WPPALETTE
{       
   private:
      // Color Palette values.
      COLORREF    m_clrrefPalette[MAX_PALETTE_COLORS];
   
   public:            
      WPCOLORPALETTE(void);
      WPCOLORPALETTE(WPCOLORPALETTE *pwpfolderDefaults);
                     
      // Folder default values.                                                                            
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
                        
      // Palette color manipulation functions.
      void        SetPaletteColor(int nIndex,COLORREF clrrefColor);
      COLORREF    GetPaletteColor(int nIndex);
                                     
      // Color palette initialization.
      void        InitializeDefaultColors(void);
                                     
      // Profile management functions.
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};
        

#endif

// EOF WPCLRPAL.HPP
