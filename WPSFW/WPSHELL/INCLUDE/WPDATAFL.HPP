//=====================================================================
// WPDATAFL.HPP
//    WPDATAFILE Class declaration.
// 
// Date Created :  03/02/94
// Last Update  :  11/10/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | DATAFILEMER 
// 03/02/94    | Creation.                               | MDA
//=====================================================================
     
#ifndef __WPDATAFILE_PROTOTYPED__

#define __WPDATAFILE_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// C-runtime library header files.
#include <sys\types.h>
#include <sys\stat.h>

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define  MAX_DATAFILE_SUBJECT    31
#define  MAX_DATAFILE_PATH       63
#define  MAX_DATAFILE_NAME       15
                                    
//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef struct {         
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1];
      char     m_szDataPath[MAX_DATAFILE_PATH+1];
      char     m_szDataName[MAX_DATAFILE_NAME+1];
} FILE_DATAFILE_STRUCT;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPDATAFILE : public WPOBJECT
{
   private:          
      // Data file name and path.
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1];
      char     m_szDataPath[MAX_DATAFILE_PATH+1];
      char     m_szDataName[MAX_DATAFILE_NAME+1];
      
      // DATAFILE file information.
      BOOL           m_fValidFileInfo;
      struct _stat   m_statFileInfo;    // DATAFILE file information.
      
      
   protected:  
      // Drawing "details view" DATAFILE object information.
      void     DrawDetailsIconText(HDC hdc);
      
   public:                                  
      WPDATAFILE(void);
      WPDATAFILE(WPDATAFILE *pwpdatafileDefaults);
      
      // Data file name and path management functions.
      const char  *GetDataSubject(void);
      const char  *GetDataPath(void);
      const char  *GetDataName(void);
      void        SetDataSubject(const char *pszDataSubject);
      void        SetDataPath(const char *pszDataPath);
      void        SetDataName(const char *pszDataName);
      const char  *GetFullDataPath(void);
      void        UpdateFileSystemObjectName(char *szNewTitle);
      
      
      // Data file resources initialization function.
      void        LoadFileInfo(void);
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      BOOL        LoadIcon(void);
               
      // Sort key values query function.
      void        GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue);
                    
      // Data file object state functions.
      BOOL        Open(void);
      BOOL        Print(void);
      BOOL        Close(void);
      
      // Popup menu properties.
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);
      
      // Drag & drop property management functions.
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL        DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDrop);
      BOOL        DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
                    
      // Settings notebook functions.
      void        InsertSettingsNotebookPages(void);
      void        TypePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      BOOL        FindDataFile(HWND hwndPage);
      void        FilePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);

      // Object to file system functions.
      BOOL        CopyObjectToFileSystem(char *pszDestPath);
      BOOL        DeleteObjectFromFileSystem(void);
                    
      // Shell profile management functions.              
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPDATAFL.HPP
