//=====================================================================
// WPSHREDR.CPP
//    WPSHREDDER Class implementation.
// 
// Date Created :  03/02/94
// Last Update  :  02/10/95
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
// From MISFUNC.CPP file:
void PlaySystemEventSound(const char *pszSystemEvent);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Shredder object default values.
#define WPS_SHREDDER_DEFAULT_ICONINDEX    ICONINDEX(IDI_DEFAULTSHREDDER)


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WORKPLACE SHELL SHREDDER CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSHREDDER():
//---------------------------------------------------------------------
WPSHREDDER::WPSHREDDER(void) : WPOBJECT()
{
   // Initialize template object.   
   SetType(OBJTYPE_SHREDDER);
}

//---------------------------------------------------------------------
// WPSHREDDER():
//---------------------------------------------------------------------
WPSHREDDER::WPSHREDDER(WPSHREDDER *pwpshredderDefaults) : WPOBJECT((WPOBJECT *)pwpshredderDefaults)
{
   // Initialize template object.   
   SetType(OBJTYPE_SHREDDER);
}


//---------------------------------------------------------------------
// DEFAULT VALUES FOR SHREDDER OBJECT
//---------------------------------------------------------------------
  
//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPSHREDDER::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_SHREDDER_DEFAULT_ICONINDEX;
}


//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------


                   
//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TestDropSource():
//---------------------------------------------------------------------
BOOL WPSHREDDER::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{                    
   // Check if object class accepts target object "pwpobjDropTarget".
   if (WPOBJECT::TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Check if template class has any problems with being a drop source
      // on target object "pwpobjDropTarget" with function "nFunction".
      switch (nFunction)
      {    
         case DROP_MOVE:
         case DROP_COPY:           
         case DROP_SHADOW:
            if (pwpobjDropTarget->TestDropTarget(this,nFunction)==FALSE)
               return (FALSE);
            return (TRUE);   // Supported, "drag & drop".

         default:
            // No other functions supported by container class.
            return (FALSE);
      }
   }
   else            
      // Program class does not support drop target and/or function.
      return (FALSE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
BOOL WPSHREDDER::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{  
   // Check if object class accepts source object "pwpobjDropSource".
   if (WPOBJECT::TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Check if template object can be the target of source object "pwpobjDropSource".
      switch (nFunction)
      {  
         case DROP_MOVE:      
            // Only accept objects which are being "moved" to shredder.
            return (TRUE);    
         
         case DROP_COPY:
            // Check if parent of source object is a file system container...
            if (pwpobjDropSource->GetParent()->isDirectoryContainer()==TRUE)
               // Only accept objects whose parent is a directory folder.
               return (TRUE);
         
         case DROP_SHADOW:    // No "shadow".
         default: 
            return (FALSE);
      }
   }                
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//---------------------------------------------------------------------
BOOL WPSHREDDER::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{                                      
   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {  
      switch (nFunction)
      {
         case DROP_MOVE:  
         case DROP_COPY:   // "Copy" only when source parent is a directory folder.
            // Before we delete them, confirm operation.
            if (pwpobjDropSource->ConfirmOnDelete()==IDYES)
            {
               // Play system event sound for shredding and object.
               PlaySystemEventSound("WPShred");
               // If object is currently placed in an object which represents the file system...
               if (pwpobjDropSource->GetParent()->isDirectoryContainer()==TRUE)
                  // Delete object from file system.
                  pwpobjDropSource->DeleteObjectFromFileSystem();
               // Destroy object being dropped into shredder.
               return (pwpobjDropSource->Destroy());
            }
            else              
               // User does not want to delete object.
               return (FALSE);
               
         case DROP_SHADOW:    // No "shadow".
         default:          
            // No other functions supported by program class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}

// EOF WPSHREDR.CPP


   