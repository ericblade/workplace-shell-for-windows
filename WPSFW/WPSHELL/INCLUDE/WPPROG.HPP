//=====================================================================
// WPPROG.HPP
//    WPPROGRAM Class declaration.
// 
// Date Created :  02/23/94
// Last Update  :  11/10/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 02/23/94    | Creation.                               | MDA
//=====================================================================
     
#ifndef __WPPROGRAM_PROTOTYPED__

#define __WPPROGRAM_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// C-runtime library header files.
#include <sys\types.h>
#include <sys\stat.h>

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define  MAX_PROGRAM_PATH     63
#define  MAX_PARAMETERS_LINE  63
#define  MAX_WORKING_DIR      63
#define  MAX_LINE             63

// Program object state flags.           
#define  START_PROG_MASK      0x00000300L
#define  START_PROG_NORMAL    0x00000000L
#define  START_PROG_MINIMIZED 0x00000100L
#define  START_PROG_MAXIMIZED 0x00000200L
   
//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// Data structure used to store Program object information in profile.
typedef struct {         
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];
      char     m_szParameters[MAX_PARAMETERS_LINE+1];
      char     m_szWorkingDir[MAX_WORKING_DIR+1];
} FILE_PROGRAM_STRUCT;

                                           
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPPROGRAM : public WPOBJECT
{
   private:          
      // Program executable pathname, parameters, and working directory.
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];
      char     m_szParameters[MAX_PARAMETERS_LINE+1];
      char     m_szWorkingDir[MAX_WORKING_DIR+1];
 
      // Execution counter variable (keeps track of how many copies are running).
      WORD     m_wExecCounter;
      
      // Program file information.
      BOOL           m_fValidFileInfo;
      struct _stat   m_statFileInfo;    // program file information.
      
      
   protected:  
      // Drawing "details view" program object information.
      void     DrawDetailsIconText(HDC hdc);

   public:                                  
      WPPROGRAM(void);
      WPPROGRAM(WPPROGRAM *pwpprogDefaults);
                  
      // Program initialization function.                  
      void        Initialize(BOOL fIsShell);
      
      // Program path, parameters, and working directory management functions.
      void        SetProgramPath(const char *pszProgramPath);
      const char  *GetProgramPath(void);
      void        SetParameters(const char *pszParameters);
      const char  *GetParameters(void);
      void        SetWorkingDir(const char *pszWorkingDir);
      const char  *GetWorkingDir(void);
      void        UpdateFileSystemObjectName(char *szNewTitle);

      // Program resources initialization function.
      void        LoadFileInfo(void);
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      BOOL        LoadIcon(void);

      // Sort key values query function.
      void        GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue);
                    
      // Program object state functions.
      BOOL        Open(void);
      BOOL        Open(char *pszParameterLine);
      BOOL        Close(void);
      
      // Popup menu properties.
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);
      
      // Drag & drop property management functions.
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL        DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
         
      // Settings notebook functions.
      void        InsertSettingsNotebookPages(void);
      BOOL        FindProgram(HWND hwndPage);
      void        ProgramPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      void        AssociationPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      DWORD       GetWindowPageFlags(HWND hwndPage);      
      void        UpdateWindowPageControls(HWND hwndPage,DWORD dwFlags);
      void        WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
                                        
      // Object to file system functions.
      BOOL        CopyObjectToFileSystem(char *pszDestPath);
      BOOL        DeleteObjectFromFileSystem(void);
                                        
      // Shell profile management functions.              
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);

};

#endif

// EOF WPPROG.HPP

