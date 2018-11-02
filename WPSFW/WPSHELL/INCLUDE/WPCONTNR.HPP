//=====================================================================
// WPCONTNR.HPP
//    WPCONTAINER Class declaration.
// 
// Date Created :  01/25/94
// Last Update  :  12/08/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/25/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPCONTAINER_PROTOTYPED__

#define __WPCONTAINER_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Generic linked list class header file.
#include "list.hpp"

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define MAX_CONTAINER_COMPONENTS    64      
// container's startup property.                  
#define STARTUP_FLAGS_MASK   0x00000F00L
#define STARTUP_PROPERTY     0x00000100L
    
//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// Used by container object to store container-specific information to profile.
typedef struct {       
   int      m_nObjects;
} FILE_CONTAINER_STRUCT;
    
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPCONTAINER : public WPOBJECT
{
   private:
      LIST              m_ObjectList;
      LIST              m_SelectedObjectList;
      int               m_nCurrentSelectedObject;

   protected:
      // Object components management function (used by derived classes only).
      void              SetObject(WPOBJECT *pwpobjSet,int nPos);
   
   public:
      WPCONTAINER(void);
      WPCONTAINER(WPCONTAINER *pwpcontainerDefaults);
      ~WPCONTAINER(void);
         
      // Object initialization function.
      void              Initialize(BOOL fIsShell);
         
      // Object components management functions.
      virtual BOOL      AddObject(WPOBJECT *pwpobjAdd,int nPos=-1);
      virtual BOOL      DeleteObject(WPOBJECT *pwpobjDelete);        
      virtual BOOL      DeleteObject(int nPos);                      

      // Object components query functions.
      virtual int       GetObjectCount(void);
      virtual WPOBJECT  *GetObject(int nPos);
      virtual int       GetObjectIndex(WPOBJECT *pwpobjQuery);
      
      // Object selection management functions.
      void              ClearSelectedObjects(BOOL fInvalidateObjects);
      int               GetSelectedObjectCount(void);
      WPOBJECT          *GetSelectedObject(int nPos);
      BOOL              AddSelectedObject(WPOBJECT *pwpobjAdd);
      WPOBJECT          *GetFirstSelectedObject(void);
      WPOBJECT          *GetNextSelectedObject(void);
      BOOL              isObjSelected(WPOBJECT *pwpobjTest);
                     
      // Container state functions.
      BOOL              isAnyChildOpen(void);
                     
      // Find object function.
      WPOBJECT          *Find(DWORD dwType,const char *pszTitle,BOOL fExact=TRUE,BOOL fStopOnMatch=TRUE,MATCH_RECORD **ppMatchList=NULL);
          
      // Color selection functions.
      virtual void      SetDefaultColors(HDC hdc,WPOBJECT *pwpobjDrawn) = 0;
      virtual void      SetFrameColors(HDC hdc,WPOBJECT *pwpobjDrawn) = 0;
      
      // Drag & drop property management functions.
      BOOL              isContainer(void);
      BOOL              TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL              TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL              DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      
      // Container open view query function.
      virtual WORD      GetOpenView(void) = 0; 
                                       
      // Object to file system functions.
      BOOL              CopyObjectToFileSystem(char *pszDestPath);
      char              *MakeFATName(char *pszString,BOOL fMakeUnique=FALSE,char *pszPath="");
                                       
      // Shell profile management functions.              
      BOOL              ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL              WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPCONTNR.HPP

