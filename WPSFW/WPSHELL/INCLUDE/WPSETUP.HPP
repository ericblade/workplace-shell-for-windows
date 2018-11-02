//=====================================================================
// WPSETUP.HPP
//    WPSETUP Class declaration.
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
     
#ifndef __WPSETUP_PROTOTYPED__

#define __WPSETUP_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPSETUP : public WPOBJECT
{
   public:                                  
      WPSETUP(void);
      WPSETUP(WPSETUP *pwpsetupDefaults);
      
      // Setup object default values.
      void     GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
};

#endif

// EOF WPSETUP.HPP
