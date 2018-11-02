//=====================================================================
// WPTEMPLT.CPP
//    WPTEMPLATE Class implementation.
// 
// Date Created :  02/25/94
// Last Update  :  01/25/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 02/25/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <io.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Template object default values.
#define WPS_TEMPLATE_DEFAULT_ICONINDEX    ICONINDEX(IDI_GENERICTEMPLATE)


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef struct {                                
   // Template type information.
   DWORD    m_dwTemplateType;
   // Template's object instance title string.
   char     m_szInstanceTitle[MAX_TITLE_NAME+1];
   // Template object instance icon information.
   char     m_szInstanceIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wInstanceIconIndex;
} FILE_TEMPLATE_STRUCT;
 

//---------------------------------------------------------------------
// WORKPLACE SHELL TEMPLATE CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TEMPLATE TYPE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetTemplateType():
//---------------------------------------------------------------------
void WPTEMPLATE::SetTemplateType(DWORD dwTemplateType)
{  
   m_dwTemplateType = dwTemplateType;
}

//---------------------------------------------------------------------
// GetTemplateType():
//---------------------------------------------------------------------
DWORD WPTEMPLATE::GetTemplateType(void)
{
   return (m_dwTemplateType);
}



//---------------------------------------------------------------------
// TEMPLATE'S OBJECT INSTANCE ICON INFORMATION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetInstanceTitle():
//---------------------------------------------------------------------
void WPTEMPLATE::SetInstanceTitle(char *pszTitle)
{
   strncpy(m_szInstanceTitle,pszTitle,MAX_TITLE_NAME);
}
   
//---------------------------------------------------------------------
// GetInstanceTitle():
//---------------------------------------------------------------------
char *WPTEMPLATE::GetInstanceTitle(void)
{
   return (m_szInstanceTitle);   
}

   

//---------------------------------------------------------------------
// SetInstanceIcon():
//---------------------------------------------------------------------
void WPTEMPLATE::SetInstanceIcon(char *pszIconPath,WORD wIconIndex)
{
   strcpy(m_szInstanceIconPath,pszIconPath);
   m_wInstanceIconIndex = wIconIndex;
}


//---------------------------------------------------------------------
// GetInstanceIcon():
//---------------------------------------------------------------------
void WPTEMPLATE::GetInstanceIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,m_szInstanceIconPath);
   *pwIconIndex = m_wInstanceIconIndex;
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPTEMPLATE():
//---------------------------------------------------------------------
WPTEMPLATE::WPTEMPLATE(void) : WPOBJECT()
{
   // Initialize template object.   
   SetType(OBJTYPE_TEMPLATE);
   // Set template object instance type.
   SetTemplateType(OBJTYPE_DATAFILE);
   // Copy template object instance title and icon.
   SetInstanceTitle("");
   SetInstanceIcon("",0);
}

//---------------------------------------------------------------------
// WPTEMPLATE():
//---------------------------------------------------------------------
WPTEMPLATE::WPTEMPLATE(WPTEMPLATE *pwptemplateDefaults) : WPOBJECT((WPOBJECT *)pwptemplateDefaults)
{  
   char  szIconPath[MAX_PATH_FILENAME+1];
   WORD  wIconIndex;

   // Initialize template object.   
   SetType(OBJTYPE_TEMPLATE);
   // Set template object instance type.
   SetTemplateType(pwptemplateDefaults->GetTemplateType());
   // Copy template object instance title and icon.
   SetInstanceTitle(pwptemplateDefaults->GetInstanceTitle());
   pwptemplateDefaults->GetInstanceIcon(szIconPath,&wIconIndex);
   SetInstanceIcon(szIconPath,wIconIndex);
}

//---------------------------------------------------------------------
// DEFAULT VALUES FOR TEMPLATE OBJECT
//---------------------------------------------------------------------
  
//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPTEMPLATE::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_TEMPLATE_DEFAULT_ICONINDEX;
}

                
//---------------------------------------------------------------------
// NEW INSTANCE CREATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateNewInstance():
//---------------------------------------------------------------------
WPOBJECT *WPTEMPLATE::CreateNewInstance(void)
{
   WPOBJECT *pwpobjNew;

   // Create new instance of built-in objects.
   switch (GetTemplateType())
   {
      case OBJTYPE_DATAFILE:
         pwpobjNew = (WPOBJECT *)new WPDATAFILE;
         pwpobjNew->SetIcon("",0);
         break;        
   
      case OBJTYPE_FOLDER:
         pwpobjNew = (WPOBJECT *)new WPFOLDER;
         pwpobjNew->SetIcon(m_szInstanceIconPath,m_wInstanceIconIndex);
         break;
         
      case OBJTYPE_PROGRAM:
         pwpobjNew = (WPOBJECT *)new WPPROGRAM;
         pwpobjNew->SetIcon(m_szInstanceIconPath,m_wInstanceIconIndex);
         break;
         
      case OBJTYPE_SHREDDER:
         pwpobjNew = (WPOBJECT *)new WPSHREDDER;
         pwpobjNew->SetIcon(m_szInstanceIconPath,m_wInstanceIconIndex);
         break;        
         
      case OBJTYPE_DRIVE:
         pwpobjNew = (WPOBJECT *)new WPDRIVE;
         pwpobjNew->SetIcon(m_szInstanceIconPath,m_wInstanceIconIndex);
         break;        
         
      default:
         return (NULL);
   }     

   // Initialize new object instance.
   pwpobjNew->LoadIcon();
   pwpobjNew->SetTitle(GetInstanceTitle());
   // Return pointer to instantiated object.
   return (pwpobjNew);
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
BOOL WPTEMPLATE::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
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
            return (pwpobjDropTarget->TestDropTarget(this,nFunction));

         default:
            // No other functions supported by container class.
            return (FALSE);
      }
   }
   else            
      // Template class does not support drop target and/or function.
      return (FALSE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPTEMPLATE::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{  
   // Template object cannot be the target of any drag & drop function.            
   return (FALSE);
}


//---------------------------------------------------------------------
// DropSource():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPTEMPLATE::DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice)
{                                
   WPOBJECT *pwpobjNewInstance;

   // Check if object class accepts being dropped on an object "pwpobjDropTarget".
   if (TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {                  
      // Depending on what drag & drop function...
      switch (nFunction)
      {
         case DROP_MOVE:                                 
            // If we are being dropped into a container object...
            if (pwpobjDropTarget->isContainer()==TRUE)
            {  
               // Create new object instance of template object.
               pwpobjNewInstance = CreateNewInstance();
               // Set our new icon position to the new container's logical point.
               pwpobjNewInstance->SetIconPos(ptDevice);
               // Now, move (or give) ourseleves into the drop target     
               if (pwpobjDropTarget->DropTarget(pwpobjNewInstance,nFunction)==TRUE)
               {                 
                  // Automatically open settings notebook...
                  pwpobjNewInstance->OpenSettingsNotebook();
                  return (TRUE);    // Success droping new object instance into target.
               }                
               else
                  return (FALSE);   // Failed droping new object instance into target.
            }  
            else
               // Let the drop target do anyhting that has to be done.
               return (pwpobjDropTarget->DropTarget(this,nFunction));
                     
         case DROP_COPY:
            // Copy template object into target object.
         case DROP_SHADOW:
            // Shadow template object in target object.
            return (WPOBJECT::DropSource(pwpobjDropTarget,nFunction,ptDevice));
            
         default:          
            // No other functions supported by program.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//---------------------------------------------------------------------
BOOL WPTEMPLATE::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{                                      
   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {  
      switch (nFunction)
      {
         case DROP_MOVE:   
         case DROP_COPY:
         case DROP_SHADOW:
         default:          
            // No other functions supported by program class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// WORKPLACE SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPTEMPLATE::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_TEMPLATE_STRUCT   ftsData;
            
   // Object information.                                
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);
            
   // Read template class information.
   if (read(hProfile,&ftsData,sizeof(FILE_TEMPLATE_STRUCT))!=sizeof(FILE_TEMPLATE_STRUCT))
      return (FALSE);

   // Initialize template object values using data values read from profile.
   SetTemplateType(ftsData.m_dwTemplateType);
   SetInstanceTitle(&(ftsData.m_szInstanceTitle[0]));
   SetInstanceIcon(&(ftsData.m_szInstanceIconPath[0]),ftsData.m_wInstanceIconIndex);
   
   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPTEMPLATE::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_TEMPLATE_STRUCT   ftsData;
      
   // Object information.                                
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);
      
   // Initialize template data structure with template values.
   ftsData.m_dwTemplateType = GetTemplateType();
   strncpy(ftsData.m_szInstanceTitle,GetInstanceTitle(),MAX_TITLE_NAME);
   GetInstanceIcon(&(ftsData.m_szInstanceIconPath[0]),&(ftsData.m_wInstanceIconIndex));

   // Write template class information.
   if (write(hProfile,&ftsData,sizeof(FILE_TEMPLATE_STRUCT))!=sizeof(FILE_TEMPLATE_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPTEMPLT.CPP

 