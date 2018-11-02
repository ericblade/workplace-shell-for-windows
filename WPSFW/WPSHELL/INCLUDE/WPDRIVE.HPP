//=====================================================================
// WPDRIVE.HPP
//    WPDIRECTORY and WPDRIVE Class declaration.
//
// Date Created :  01/23/95
// Last Update  :  11/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/23/95    | Creation.                               | MDA
//=====================================================================

#ifndef __WPDRIVE_PROTOTYPED__

#define __WPDRIVE_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// C-runtime library header files.
#include <stdlib.h>  // Needed for _MAX_PATH constant.
#include <dos.h>     // Needed for _find_t data structure definition.

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
enum DRIVE_TYPE { WPDRIVE_UNKNOWN=0, WPDRIVE_FLOPPY=2, WPDRIVE_HARD=3, 
                  WPDRIVE_NETWORK=4, WPDRIVE_CDROM=5 , WPDRIVE_RAMDISK=6 };


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// WPDIRECTORY file data structure.
typedef struct {
   char  m_szPath[_MAX_PATH+1];
} FILE_DIRECTORY_STRUCT;

              
// WPDRIVE file data structure.
typedef struct {
   UINT  m_uDriveIndex;
   WORD  m_Type;
} FILE_DRIVE_STRUCT;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPDIRECTORY : public WPFOLDER
{
   private:               
      // File objects loaded flag.
      BOOL  m_fFilesLoaded;         
      // Directory path (including drive letter but without ending backslash).
      char  *m_pszPath;
   
   private:             
      // Function that creates a WPS object from a DOS file structure.
      WPOBJECT *CreateObjectFromFileStructure(struct _find_t *pFile);
      
   protected:
      // Function to scan and load files in current directory path.
      BOOL     isFileInFolder(char *pszFilename);      
      BOOL     isFileInDirectory(char *pszFilename);      
      void     LoadDirectoryFiles(void);
      void     FreeDirectoryFiles(void);
   
      // Mouse management functions (all points are in device coordinates).
      void     OnRightMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice);
      
   public:
      WPDIRECTORY(void);
      WPDIRECTORY(WPDIRECTORY *pwpdirDefaults);
          
      // Directory path management functions.
      char     *GetPath(void);
      void     SetPath(char *pszNewPath);
      char     *GetParentPath(void);
      void     UpdateFileSystemObjectName(char *szNewTitle);
               
      // Directory state functions.
      BOOL     Open(void);
      BOOL     Close(void);
      
      // Popup menu properties.
      void     InitializePopup(HMENU hmenuPopup);
      void     MenuSelection(WORD wMenuItem);
      
      // Drag & drop property management functions.
      BOOL     isDirectoryContainer(void);
      BOOL     TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL     DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);

      // Object to file system functions.
      BOOL     CopyObjectToFileSystem(char *pszDestPath);
      BOOL     DeleteObjectFromFileSystem(void);
           
           
      // Profile management functions.
      BOOL     ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL     WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPDRIVE : public WPDIRECTORY
{
   private:
      // Drive information.
      UINT        m_uDriveIndex;
      DRIVE_TYPE  m_Type;
        
   private:        
      // Function to query number of drives available in system.
      int         GetDriveCount(void);
      
      // Function to query drive type given a drive index.
      DRIVE_TYPE  QueryDriveType(UINT uDriveIndex);
      char        *QueryDriveTypeString(UINT uDriveIndex);
    
   public:
      WPDRIVE(void);
      WPDRIVE(WPDRIVE *pwpdriveDefaults);

      // Drive's default values.                                                                            
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      
      // Drive information management functions.
      UINT        GetDriveIndex(void);
      void        SetDriveIndex(UINT uDriveIndex);
      DRIVE_TYPE  GetDriveType(void);
      void        SetDriveType(DRIVE_TYPE Type);

      // Settings notebook functions.
      void        InsertSettingsNotebookPages(void);
      void        InitializeDrivesList(HWND hwndPage);
      void        DrivePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
                           
      // Popup menu properties.
      void        InitializePopup(HMENU hmenuPopup);
      void        MenuSelection(WORD wMenuItem);
                           
      // Profile management functions.
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};


#endif

// EOF WPDRIVE.HPP

