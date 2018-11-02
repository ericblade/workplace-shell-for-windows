//=====================================================================
// WPSHREDR.HPP
//    WPSHREDDER Class declaration.
// 
// Date Created :  03/02/94
// Last Update  :  11/15/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 03/02/94    | Creation.                               | MDA
//=====================================================================
     
#ifndef __WPSHREDDER_PROTOTYPED__

#define __WPSHREDDER_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPSHREDDER : public WPOBJECT
{
   public:                                  
      WPSHREDDER(void);
      WPSHREDDER(WPSHREDDER *pwpprogDefaults);

      // Program object default values.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      
      // Popup menu properties.
      
      // Drag & drop property management functions.
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL        DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
};

#endif

// EOF WPSHREDR.HPP

