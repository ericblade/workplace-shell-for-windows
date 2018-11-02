//=====================================================================
// WPTEMPLT.HPP
//    WPTEMPLATE Class declaration.
// 
// Date Created :  02/25/94
// Last Update  :  11/15/94
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
     
#ifndef __WPTEMPLATE_PROTOTYPED__

#define __WPTEMPLATE_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPTEMPLATE : public WPOBJECT
{
   private:          
      // Template type information.
      DWORD    m_dwTemplateType;    
      
      // Template's object instance title string.
      char     m_szInstanceTitle[MAX_TITLE_NAME+1];                  
      
      // Template's object instance icon information.
      char     m_szInstanceIconPath[MAX_PATH_FILENAME+1];
      WORD     m_wInstanceIconIndex;
      
   protected:  
      // Template type management functions.
      void     SetTemplateType(DWORD dwTemplateType);
      DWORD    GetTemplateType(void);
            
      // Template's object instance information functions.
      void     SetInstanceTitle(char *pszTitle);
      char     *GetInstanceTitle(void);
      void     SetInstanceIcon(char *pszIconPath,WORD wIconIndex);
      void     GetInstanceIcon(char *pszIconPath,WORD *pwIconIndex);
   
   public:                                  
      WPTEMPLATE(void);
      WPTEMPLATE(WPTEMPLATE *pwpprogDefaults);

      // Template object default values.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      
      // New instance creation function.
      WPOBJECT    *CreateNewInstance(void);
      
      // Popup menu properties.
      
      // Drag & drop property management functions.
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL        DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDrop);
      BOOL        DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
            
      // Shell profile management functions.              
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPTEMPLT.HPP

