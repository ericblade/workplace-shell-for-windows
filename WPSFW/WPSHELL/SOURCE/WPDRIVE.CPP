//=====================================================================
// WPDRIVE.CPP
//    WPDIRECTORY and WPDRIVE Class implementation.
//
// Date Created :  01/23/95
// Last Update  :  10/10/95
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

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dos.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"

//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
WPOBJECT *CreateTreeDuplicate(WPCONTAINER *pwpcontainerParent,WPOBJECT *pwpobjOrigRoot);
void     DestroyRecursively(WPOBJECT *pwpobjDestroy);
UINT     ExecuteApplication(LPCSTR pszCmdLine,UINT nCmdShow);
BOOL     CopyDirectory(char *pszSourcePath,char *pszDestPath);
BOOL     DeleteDirectory(char *pszPath);


//---------------------------------------------------------------------
// WORKPLACE SHELL DIRECTORY CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateObjectFromFileStructure():
//---------------------------------------------------------------------
WPOBJECT *WPDIRECTORY::CreateObjectFromFileStructure(struct _find_t *pFile)
{
   WPOBJECT    *pwpObject;
   WPPROGRAM   *pwpProgram;
   WPDIRECTORY *pwpDirectory;
   WPDATAFILE  *pwpDataFile;
   BOOL        fExecutable;
   char        *pszCurrentFilePath;

   // Initialize local variable.
   pwpObject = NULL;

   // Allocate memory for string with path and filename (or subdirectory) in it.
   pszCurrentFilePath = new char[(strlen(GetPath())+1+13+1)];

   // Create string contents.
   sprintf(pszCurrentFilePath,"%s\\%s",GetPath(),pFile->name);

   // Depending on which file type...
   switch (pFile->attrib)
   {
      case _A_SUBDIR:
         // "File" is a subdirectory, therefore, create a WPS directory object.
         if ((strcmp(pFile->name,".")!=0)&&(strcmp(pFile->name,"..")!=0))
         {
            // Subdirectory is not "." (current), therefore, create directory object.
            pwpDirectory = new WPDIRECTORY;
            pwpDirectory->SetPath(pszCurrentFilePath);
            pwpObject = (WPOBJECT *)pwpDirectory;
         }
         break;

      default:
         // "File" is not a subdirectory,...
         // Check whether file found is an executable program or a data file...
         if ((stristr(pFile->name,".EXE")!=NULL)||(stristr(pFile->name,".COM")!=NULL)||
             (stristr(pFile->name,".BAT")!=NULL)||(stristr(pFile->name,".PIF")!=NULL))
            fExecutable = TRUE;
         else
            fExecutable = FALSE;
         // If file is an executable program...
         if (fExecutable==TRUE)
         {
            // File represents an executable program in directory.
            pwpProgram = new WPPROGRAM;
            pwpProgram->SetProgramPath(pszCurrentFilePath);
            pwpProgram->SetIcon(pszCurrentFilePath,0);
            pwpProgram->SetWorkingDir(GetPath());
            pwpObject = (WPOBJECT *)pwpProgram;
         }
         else
         {
            // File represents a data file in directory.
            pwpDataFile = new WPDATAFILE;
            pwpDataFile->SetDataName(pFile->name);
            pwpDataFile->SetDataPath(GetPath());
            pwpObject = (WPOBJECT *)pwpDataFile;
         }
         break;
   }

   // Deallocate memory used for file path.
   delete[] pszCurrentFilePath;

   // Add general object information.
   if (pwpObject!=NULL)
      pwpObject->SetTitle(pFile->name);

   // Return pointer to new Workplace Shell object representing file.
   return (pwpObject);
}


//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isFileInFolder():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::isFileInFolder(char *pszFilename)
{
   int index;

   for (index=0; index<GetObjectCount(); index++)
   {
      if (stricmp(pszFilename,GetObject(index)->GetTitle())==0)
         return (TRUE);
   }
   return (FALSE);
}

//---------------------------------------------------------------------
// isFileInDirectory():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::isFileInDirectory(char *pszFilename)
{
   BOOL           fResult;
   struct _find_t FileFound;
   char           *pszCurrentPath;

   // Allocate space for path string (current path + '\' + filename + null).
   pszCurrentPath = new char[(strlen(GetPath())+1+13+1)];

   // Create string contents.
   sprintf(pszCurrentPath,"%s\\%s",GetPath(),pszFilename);

   // Find first file or subdirectory within current path.
   fResult = ((_dos_findfirst(pszCurrentPath,_A_SUBDIR,&FileFound)==0)? TRUE : FALSE);

   // Deallocate memory used for path string.
   delete[] pszCurrentPath;

   return (fResult);
}


//---------------------------------------------------------------------
// LoadDirectoryFiles():
//---------------------------------------------------------------------
void WPDIRECTORY::LoadDirectoryFiles(void)
{
   WPOBJECT       *pwpobjNewObject;
   HCURSOR        hcurOriginal;
   struct _find_t FileFound;
   char           *pszCurrentPath;

   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));

   // Allocate space for path string (current path + '\' + filename + null).
   pszCurrentPath = new char[(strlen(GetPath())+1+13+1)];

   // Create string contents.
   sprintf(pszCurrentPath,"%s\\*.*",GetPath());

   // Find first file or subdirectory within current path.
   if (_dos_findfirst(pszCurrentPath,_A_SUBDIR,&FileFound)==0)
   {
      // Check whether file is currently loaded in directory folder...
      if (isFileInFolder(FileFound.name)==FALSE)
      {
         // Create Workplace Shell object from DOS file structure.
         pwpobjNewObject = CreateObjectFromFileStructure(&FileFound);
         // Check that an object was successfully created...
         if (pwpobjNewObject!=NULL)
         {
            // Add object to directory folder.
            AddObject(pwpobjNewObject);
            // If folder is currently open,...
            if (isOpen()==TRUE)
               // Then load icon resource for new object.
               pwpobjNewObject->LoadIcon();
            // Set internal variable.
            m_fFilesLoaded = TRUE;
         }
      }

      // Loop until all files or directories have been scanned...
      while (_dos_findnext(&FileFound)==0)
      {
         // Check whether file is currently loaded in directory folder...
         if (isFileInFolder(FileFound.name)==FALSE)
         {
            // Create Workplace Shell object from DOS file structure.
            pwpobjNewObject = CreateObjectFromFileStructure(&FileFound);
            // Check that an object was successfully created...
            if (pwpobjNewObject!=NULL)
            {
               // Add object to directory folder.
               AddObject(pwpobjNewObject);
               // If folder is currently open,...
               if (isOpen()==TRUE)
                  // Then load icon resource for new object.
                  pwpobjNewObject->LoadIcon();
               // Set internal variable.
               m_fFilesLoaded = TRUE;
            }
         }
      }
   }

   // Deallocate memory used for path string.
   delete[] pszCurrentPath;

   // Check if original cursor was not the same as our "wait" cursor...
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
}


//---------------------------------------------------------------------
// FreeDirectoryFiles():
//---------------------------------------------------------------------
void WPDIRECTORY::FreeDirectoryFiles(void)
{
   WPOBJECT    **apwpobjDirectory;
   HCURSOR     hcurOriginal;
   int         index,nObjectCount;

   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));

   // Get number of objects in directory folder.
   nObjectCount = GetObjectCount();
   // Allocate enough memory to hold list of all objects.
   apwpobjDirectory = new WPOBJECT *[nObjectCount];

   // Get list of objects, and store them in local array...
   for (index=0; index<nObjectCount; index++)
      apwpobjDirectory[index] = GetObject(index);

   // Clear list of selected objects to "none" (do not invalidate them).
   ClearSelectedObjects(FALSE);

   // Do delete operation for all selected objects.
   for (index=0; index<nObjectCount; index++)
      // Call object's virtual key handler.
      apwpobjDirectory[index]->Destroy();

   // Deallocate memory used to hold copy of selected objects.
   delete[] apwpobjDirectory;

   // Set boolean flag to false since no files are current loaded.
   m_fFilesLoaded = FALSE;

   // Check if original cursor was not the same as our "wait" cursor...
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
}

//---------------------------------------------------------------------
// OnRightMouseButton():
//---------------------------------------------------------------------
void WPDIRECTORY::OnRightMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice)
{
   // Process right mouse button message.
   if (uMsg==WM_RBUTTONDOWN)
   {
      // Check if user wants to do a "Shadow" or "Move" drag & drop function...
      if (((wFlags&MK_CONTROL)&&(wFlags&MK_SHIFT)) || (!(wFlags&MK_LBUTTON)))
         // Since these are not supported, do a "Copy" instead.
         wFlags = MK_CONTROL;
   }

   // Call base class function to handle message.
   WPFOLDER::OnRightMouseButton(uMsg,wFlags,ptDevice);
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDIRECTORY():
//---------------------------------------------------------------------
WPDIRECTORY::WPDIRECTORY(void) : WPFOLDER()
{
   // Initialize default values.
   SetType(OBJTYPE_DIRECTORY);
   // Initialize private variables.
   m_fFilesLoaded = FALSE;
   m_pszPath = strdup("C:");
}


//---------------------------------------------------------------------
// WPDIRECTORY():
//---------------------------------------------------------------------
WPDIRECTORY::WPDIRECTORY(WPDIRECTORY *pwpdirDefaults) : WPFOLDER((WPFOLDER *)pwpdirDefaults)
{
   // Initialize default values.
   SetType(OBJTYPE_DIRECTORY);
   // Initialize private variables.
   m_fFilesLoaded = FALSE;
   m_pszPath =  strdup(pwpdirDefaults->GetPath());
}
  


//---------------------------------------------------------------------
// DIRECTORY PATH MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetPath():
//---------------------------------------------------------------------
char *WPDIRECTORY::GetPath(void)
{
   return (m_pszPath);
}

//---------------------------------------------------------------------
// SetPath():
//---------------------------------------------------------------------
void WPDIRECTORY::SetPath(char *pszNewPath)
{
   // Check if current path is valid...
   if (m_pszPath!=NULL)
      // Free memory allocated for it.
      free(m_pszPath);

   // Check that new path is valid...
   if (pszNewPath!=NULL)
      // Make second copy of path string.
      m_pszPath = strdup(pszNewPath);
   else
      // Make current path be the root path.
      m_pszPath = strdup("C:");

   // Set "loaded" flag to false.
   m_fFilesLoaded = FALSE;
}
                               
//---------------------------------------------------------------------
// GetParentPath():
//---------------------------------------------------------------------
char *WPDIRECTORY::GetParentPath(void)
{                                                 
   if (GetParent()->isDirectoryContainer()==TRUE)
      return (((WPDIRECTORY *)GetParent())->GetPath());
   else
      return ("");
}
                               

//---------------------------------------------------------------------
// UpdateFileSystemObjectName():
//---------------------------------------------------------------------
void WPDIRECTORY::UpdateFileSystemObjectName(char *szNewTitle)
{  
   char szFullPath[_MAX_PATH+1];
        
   // Create new path using this directory's parent and the new name.
   sprintf(szFullPath,"%s\\%s",GetParentPath(),szNewTitle);
   // Change the program's name to the newly specified one.
   SetPath(szFullPath);
   // If directory folder is currently open,...
   if (isOpen())
      // "Reopen" again since this will refresh the file list again with new path.
      Open();
}
                            

                            
//---------------------------------------------------------------------
// DIRECTORY STATE FUNCTIONS
//---------------------------------------------------------------------
                                                
//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::Open(void)
{
   // If directory files have not been scanned and loaded yet...
   if (m_fFilesLoaded==FALSE)
   {
      // Scan files and subdirectories in current path and create WPS objects for them.
      LoadDirectoryFiles();
      // Arrange loaded icons.
      ArrangeIcons();
   }

   // Call base class's open function.
   return (WPFOLDER::Open());
}

//---------------------------------------------------------------------
// Close():
//    This function implements the "close" method of the object.  Since
// it is a virtual function, it can be overloaded by derived object
// classes and functionality can be added.
//---------------------------------------------------------------------
BOOL WPDIRECTORY::Close(void)
{
   BOOL fSuccess;

   // Call base class for default functionality when closing.
   fSuccess = WPFOLDER::Close();

   // Check if any children are currently opened...
   if (isAnyChildOpen()==FALSE)
      // No children objects are currently opened, therefore, destroy objects from memory.
      FreeDirectoryFiles();

   // Does nothing.
   return (fSuccess);
}


//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPDIRECTORY::InitializePopup(HMENU hmenuPopup)
{
   // Call base class function.
   WPFOLDER::InitializePopup(hmenuPopup);

   // Delete menu item "Create another..." in folder popup menu.
   DeleteMenu(hmenuPopup,IDM_CREATEANOTHER,MF_BYCOMMAND);
   // Delete menu item "Create shadow..." in folder popup menu.
   DeleteMenu(hmenuPopup,IDM_CREATESHADOW,MF_BYCOMMAND);
   // Delete menu item "Delete..." in folder popup menu.
   DeleteMenu(hmenuPopup,IDM_FIND,MF_BYCOMMAND);
}


//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPDIRECTORY::MenuSelection(WORD wMenuItem)
{
   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_REFRESHNOW:
         // Update directory folder contents with any new file(s).
         LoadDirectoryFiles();
         // Arrange loaded icons.
         ArrangeIcons();
         // Let base object handle the rest of the refresh.
         WPFOLDER::MenuSelection(wMenuItem);
         break;

      case IDM_CREATEANOTHER:
      case IDM_CREATESHADOW:
      case IDM_FIND:
         // Ignore these functions.
         break;

      default:
         // Let base object handle the rest of menu selections.
         WPFOLDER::MenuSelection(wMenuItem);
         break;
   }
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isDirectoryContainer():
//    This function is used to test if an object is a container object
// or just an object that allows objects to be dropped but not moved.
//    It is defined as a "virtual" function so that other objects can
// override the default behavior of "non-container" object.
//---------------------------------------------------------------------
BOOL WPDIRECTORY::isDirectoryContainer(void)
{
   // By default, an object is a non-container unless function is overloaded.
   return (TRUE);
}

//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPDIRECTORY::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if base class accepts object "pwpobjDropSource" being dropped on it.
   if (WPFOLDER::TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Check that the parent directory (container) of source object is NOT the target directory...
      if (pwpobjDropSource->GetParent()==this)
         return (FALSE);

      // Only accept "move" or "copy" operations to other directories...
      if ((nFunction==DROP_MOVE)||(nFunction==DROP_COPY))
         return (TRUE);
   }

   // Directory object will not accept source object for drag & drop opreation.
   return (FALSE);
}

//---------------------------------------------------------------------
// DropTarget():
//    This function is called when this container object is acting as
// a drop target for a Drag & Drop operation.  The first paremeter
// "pwpobjDropSource" is the object in that is being "dropped" into
// this container.
// The second parameter, "nFunction", is the function code for the
// Drag & Drop operation the user chose to do (move, copy, etc..).
//---------------------------------------------------------------------
BOOL WPDIRECTORY::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   HCURSOR  hcurOriginal;

   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Depending on which drop operation is being performed...
      switch (nFunction)
      {
         case DROP_MOVE:
         case DROP_COPY:
            // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
            hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
            // Copy actual file(s) represented in object to this directory.
            if (pwpobjDropSource->CopyObjectToFileSystem(GetPath())==TRUE)
            {
               // Update directory folder contents.
               LoadDirectoryFiles();
               // Arrange loaded icons.
               ArrangeIcons();
            }
            // Check if original cursor was not the same as our "wait" cursor...
            if (hcurOriginal!=GetCursor())
               // Restore original cursor and destroy custom wait cursor.
               DestroyCursor(SetCursor(hcurOriginal));
            // Return result.
            return (TRUE);
      }
   }

   // Failed dropping source object into this directory object.
   return (FALSE);
}


//---------------------------------------------------------------------
// OBJECT TO FILE SYSTEM FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CopyObjectToFileSystem():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::CopyObjectToFileSystem(char *pszDestPath)
{
   char  szDestPath[_MAX_PATH+1];

   // Create destination directory name fully qualified.
   sprintf(szDestPath,"%s\\%s",pszDestPath,MakeFATName((char *)GetTitleNoCRLF()));
   // Copy directory recursively.
   return (CopyDirectory(GetPath(),szDestPath));
}

//---------------------------------------------------------------------
// DeleteObjectFromFileSystem():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::DeleteObjectFromFileSystem(void)
{
   return (DeleteDirectory(GetPath()));
}



//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_FOLDER_STRUCT      ffsData;
   FILE_DIRECTORY_STRUCT   fdsData;

   // Object information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read folder's extra information.
   if (read(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);

   // Store folder's profile information into folder's internal variables.
   WPFOLDER::SetProfileInfo(&ffsData);

   // Read directory's extra information.
   if (read(hProfile,&fdsData,sizeof(FILE_DIRECTORY_STRUCT))!=sizeof(FILE_DIRECTORY_STRUCT))
      return (FALSE);

   // Store directory object information.
   SetPath(fdsData.m_szPath);

   // Container information.
   if (WPCONTAINER::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   return (TRUE);
}

//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPDIRECTORY::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_CONTAINER_STRUCT   fcsData;
   FILE_FOLDER_STRUCT      ffsData;
   FILE_DIRECTORY_STRUCT   fdsData;

   // Object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Get folder's profile information into folder's profile structure.
   WPFOLDER::GetProfileInfo(&ffsData);

   // Write folder's extra information.
   if (write(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);

   // Initialize directory object information.
   strncpy(fdsData.m_szPath,GetPath(),_MAX_PATH);

   // Write directory's extra information.
   if (write(hProfile,&fdsData,sizeof(FILE_DIRECTORY_STRUCT))!=sizeof(FILE_DIRECTORY_STRUCT))
      return (FALSE);

   // Save no child objects from this container object.
   fcsData.m_nObjects = 0;

   // Write container extra information.
   if (write(hProfile,&fcsData,sizeof(FILE_CONTAINER_STRUCT))!=sizeof(FILE_CONTAINER_STRUCT))
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// WORKPLACE SHELL DRIVE CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDriveCount():
//---------------------------------------------------------------------
int WPDRIVE::GetDriveCount(void)
{
   int   nDriveIndex,nDriveCount;

   // Initialize the count of drives available to zero.
   nDriveCount = 0;
   // Scan all possible drives available and test their availability...
   for (nDriveIndex=0; nDriveIndex<26; nDriveIndex++)
   {
      // If drive is available...
      if (::GetDriveType(nDriveIndex)!=0)
         nDriveCount++;
   }
   // Return drive count.
   return (nDriveCount);
}

//---------------------------------------------------------------------
// QueryDriveType():
//---------------------------------------------------------------------
#pragma warning( disable : 4704 ) // Disable warning about "in-line assembler preculdes optimizations"
DRIVE_TYPE WPDRIVE::QueryDriveType(UINT uDriveIndex)
{
   int   iType;
   BOOL  fCDROM=FALSE,fRAM=FALSE;

   //Validate possible drive indices.
   if ((0 > uDriveIndex) || (25 < uDriveIndex))
      return (WPDRIVE_UNKNOWN);

   iType = ::GetDriveType(uDriveIndex);

   //Check for CD-ROM on FIXED and REMOTE drives only.
   if ((DRIVE_FIXED==iType) || (DRIVE_REMOTE==iType))
   {
      _asm
      {
         mov     ax,1500h        //Check if MSCDEX exists.
         xor     bx,bx
         int     2fh

         or      bx,bx    //BX unchanged if MSCDEX is not around.
         jz      CheckRAMDrive   //No? Go check for RAM drive.

         mov     ax,150Bh  //Check if drive is using CD driver.

         mov     cx,uDriveIndex
         int     2fh

         mov     fCDROM,ax    //AX if the CD-ROM flag.
         or      ax,ax
         jnz     Exit         //Leave if we found a CD-ROM drive.

         CheckRAMDrive:
      }
   }

   //Check for RAM drives on FIXED disks only.
   if (DRIVE_FIXED==iType)
   {
      /*
      * Check for RAM drive is done by reading the boot sector
      * and looking at the number of FATs. RAM disks only have 1

      * while all others have 2.
      */
      _asm
      {
         push    ds

         mov     bx,ss
         mov     ds,bx

         sub     sp,0200h    //Reserve 512 bytes to read a sector
         mov     bx,sp       //and point BX there.

         mov     ax,uDriveIndex //Read the boot sector of the drive.
         mov     cx,1
         xor     dx,dx

         int     25h
         add     sp,2        //Int 25h requires stack cleanup.

         jc      DriveNotRAM

         mov     bx,sp
         cmp     ss:[bx+15h],0f8h    //Reverify fixed disk.
         jne     DriveNotRAM
         cmp     ss:[bx+10h],1  //Check if there's only one FAT.
         jne     DriveNotRAM
         mov     fRAM,1

         DriveNotRAM:
         add     sp,0200h
         pop     ds

         Exit:
         //Leave fRAM untouched--it's FALSE by default.
      }
   }


   /*
    * If either CD-ROM or RAM drive flags are set, return privately
    * defined flags for them (outside of Win32). Otherwise, return
    * the type given from GetDriveType.
    */

   if (fCDROM)
      return (WPDRIVE_CDROM);

   if (fRAM)
      return (WPDRIVE_RAMDISK);

   //Drive B on a one-drive system returns < 2 from GetDriveType.
   return ((DRIVE_TYPE)(iType));
}

//---------------------------------------------------------------------
// QueryDriveTypeString():
//---------------------------------------------------------------------
char *WPDRIVE::QueryDriveTypeString(UINT uDriveIndex)
{
   // Depending on which type of drive this is, return corresponding string.
   switch (QueryDriveType(uDriveIndex))
   {
      case WPDRIVE_FLOPPY:
         return (GetString(IDSTR_DRIVETYPE_FLOPPY));

      case WPDRIVE_HARD:
         return (GetString(IDSTR_DRIVETYPE_HARD));

      case WPDRIVE_NETWORK:
         return (GetString(IDSTR_DRIVETYPE_NETWORK));

      case WPDRIVE_CDROM:
         return (GetString(IDSTR_DRIVETYPE_CDROM));

      case WPDRIVE_RAMDISK:
         return (GetString(IDSTR_DRIVETYPE_RAMDISK));

      default:
         return (GetString(IDSTR_DRIVETYPE_UNKNOWN));
   }
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDRIVE():
//---------------------------------------------------------------------
WPDRIVE::WPDRIVE(void) : WPDIRECTORY()
{
   // Initialize default values.
   SetType(OBJTYPE_DRIVE);
   m_uDriveIndex = 2;            // Default to "Drive C:".
   m_Type = WPDRIVE_HARD;        // Default to hard drive.
}

//---------------------------------------------------------------------
// WPDRIVE():
//---------------------------------------------------------------------
WPDRIVE::WPDRIVE(WPDRIVE *pwpdriveDefaults) : WPDIRECTORY((WPDIRECTORY *)pwpdriveDefaults)
{
   // Initialize default values.
   SetType(OBJTYPE_DRIVE);
   m_uDriveIndex = pwpdriveDefaults->GetDriveIndex();
   m_Type = pwpdriveDefaults->GetDriveType();
}

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPDRIVE::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   // Copy name of shell executable.
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   // Depending on which drive type...
   switch (GetDriveType())
   {
      case WPDRIVE_FLOPPY:
         *pwIconIndex = ICONINDEX(IDI_FLOPPYDRIVE);
         break;

      case WPDRIVE_NETWORK:
         *pwIconIndex = ICONINDEX(IDI_NETWORK_DRIVE);
         break;

      case WPDRIVE_CDROM:
         *pwIconIndex = ICONINDEX(IDI_CDROM_DRIVE);
         break;

      case WPDRIVE_RAMDISK:
         *pwIconIndex = ICONINDEX(IDI_RAM_DRIVE);
         break;

      default:
         *pwIconIndex = ICONINDEX(IDI_HARDDRIVE);
         break;
   }
}

//---------------------------------------------------------------------
// GetDriveIndex():
//---------------------------------------------------------------------
UINT WPDRIVE::GetDriveIndex(void)
{
   return (m_uDriveIndex);
}

//---------------------------------------------------------------------
// SetDriveIndex():
//---------------------------------------------------------------------
void WPDRIVE::SetDriveIndex(UINT uDriveIndex)
{
   char  szPath[] = "X:";

   // Store drive index.
   m_uDriveIndex = uDriveIndex;
   // Query the drive type.
   m_Type = QueryDriveType(uDriveIndex);
   // Construct default path using drive letter.
   szPath[0] = (char)('A' + uDriveIndex);
   SetPath(szPath);
   // Destroy directory file objects from memory.
   FreeDirectoryFiles();
   // Clear current icon so we pick up default icon representing new drive index.
   UpdateIcon("",0);
}

//---------------------------------------------------------------------
// GetDriveType():
//---------------------------------------------------------------------
DRIVE_TYPE WPDRIVE::GetDriveType(void)
{
   return (m_Type);
}

//---------------------------------------------------------------------
// SetDriveType():
//---------------------------------------------------------------------
void WPDRIVE::SetDriveType(DRIVE_TYPE Type)
{
   m_Type = Type;
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InsertSettingsNotebookPages(void)
//------- --------------------------------------------------------------
void WPDRIVE::InsertSettingsNotebookPages(void)
{
   // Insert the "Drive" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_DRIVE),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPDRIVE::DrivePageProc,NULL,MAKEINTRESOURCE(IDD_DRIVE_PAGE));
   // Insert the default object pages into the settings notebook.
   WPDIRECTORY::InsertSettingsNotebookPages();
}


//---------------------------------------------------------------------
// InitializeDrivesList():
//---------------------------------------------------------------------
void WPDRIVE::InitializeDrivesList(HWND hwndPage)
{
   int   nIndex,nDriveIndex;
   char  szTempString[64];

   // Loop through all possible drives A-Z...
   for (nDriveIndex=0; nDriveIndex<26; nDriveIndex++)
   {
      // Check if drive is available...
      if (::GetDriveType(nDriveIndex)!=0)
      {
         // Create listbox string for current drive index.
         sprintf(szTempString,"%c: (%s)",('A'+nDriveIndex),QueryDriveTypeString((UINT)nDriveIndex));
         // Add string to end of list of drives.
         nIndex = (int)SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_ADDSTRING,0,(LPARAM)(LPCSTR)szTempString);
         // Store drive index in
         SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_SETITEMDATA,(WPARAM)nIndex,(LPARAM)(DWORD)nDriveIndex);
         // Check if this is the current selection.
         if (nDriveIndex==(int)GetDriveIndex())
            // Select first item in list.
            SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_SETCURSEL,(WPARAM)nIndex,0L);
      }
   }
}


//---------------------------------------------------------------------
// DrivePageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPDRIVE::DrivePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   UINT  uDriveIndex;
   int   nIndex;
   char  szTempString[32];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize listbox with list of available drives.
         InitializeDrivesList(hwndPage);
         // If drive object has currently an open view...
         if (isAnyChildOpen()==TRUE)
            // Disable drives listbox.
            EnableWindow(GetDlgItem(hwndPage,IDL_DRIVES),FALSE);
         break;

      case PAGE_SELECTED:
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDL_DRIVES:
               // If drives listbox selection has changed...
               if (HIWORD(dwParam)==LBN_SELCHANGE)
               {
                  // Get current selection's drive index.
                  nIndex = (int)SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_GETCURSEL,0,0L);
                  uDriveIndex = (UINT)SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_GETITEMDATA,(WPARAM)nIndex,0L);
                  // Create new drive's title string.
                  sprintf(szTempString,"%s %c:",GetString(IDSTR_OBJTITLE_DRIVE),('A'+uDriveIndex));
                  // Set "General"'s page edit control to new title.
                  SetDlgItemText(GetSettingsNotebook()->GetWindowFromPageIndex(4),IDE_TITLE,szTempString);
               }
               break;

            case IDB_UNDO:
               break;

            case IDB_HELP:
               // Open help file with "Drive" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_DRIVEPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Get current selection's drive index.
         nIndex = (int)SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_GETCURSEL,0,0L);
         uDriveIndex = (UINT)SendDlgItemMessage(hwndPage,IDL_DRIVES,LB_GETITEMDATA,(WPARAM)nIndex,0L);
         // If user has selected a different drive...
         if (uDriveIndex!=GetDriveIndex())
            // Change drive index.
            SetDriveIndex(uDriveIndex);
         break;
   }
}

//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPDRIVE::InitializePopup(HMENU hmenuPopup)
{
   // Call base class function.
   WPDIRECTORY::InitializePopup(hmenuPopup);

   // If drives type is floppy or CD-ROM...
   if ((GetDriveType()==WPDRIVE_FLOPPY)||(GetDriveType()==WPDRIVE_CDROM))
   {
      // Add "Install" menu item to popup menu.
      AppendMenu(hmenuPopup,MF_SEPARATOR,0,0L);
      AppendMenu(hmenuPopup,MF_STRING|MF_ENABLED,IDM_INSTALL,GetString(IDSTR_DRIVEMENU_INSTALL));
   }
}


//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPDRIVE::MenuSelection(WORD wMenuItem)
{
   char  szInstallProgram[_MAX_PATH+1];

   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_INSTALL:
         // Create command line for a "SETUP.EXE" installation program.
         sprintf(szInstallProgram,"%s\\setup.exe",GetPath());
         // Try to install new software from drive.
         if (ExecuteApplication(szInstallProgram,SW_NORMAL)<32)
         {
            // Create command line for a "INSTALL.EXE" installation program.
            sprintf(szInstallProgram,"%s\\install.exe",GetPath());
            // Try to install new software from drive.
            if (ExecuteApplication(szInstallProgram,SW_NORMAL)<32)
            {
               // Notify user that we could not run any installation program(s).
               MessageBox(NULL,GetString(IDSTR_ERR_CANNOT_RUN_INSTALL),
                          GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
            }
         }
         break;

      default:
         // Let base object handle the rest of menu selections.
         WPDIRECTORY::MenuSelection(wMenuItem);
         break;
   }
}


//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPDRIVE::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_FOLDER_STRUCT      ffsData;
   FILE_DRIVE_STRUCT       fdsData;

   // Object information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read folder's extra information.
   if (read(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);

   // Store folder's profile information into folder's internal variables.
   WPFOLDER::SetProfileInfo(&ffsData);

   // Read drive's extra information.
   if (read(hProfile,&fdsData,sizeof(FILE_DRIVE_STRUCT))!=sizeof(FILE_DRIVE_STRUCT))
      return (FALSE);

   // Store drive object information.
   SetDriveIndex(fdsData.m_uDriveIndex);
   SetDriveType((DRIVE_TYPE)fdsData.m_Type);

   // Container information.
   if (WPCONTAINER::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPDRIVE::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_CONTAINER_STRUCT   fcsData;
   FILE_FOLDER_STRUCT      ffsData;
   FILE_DRIVE_STRUCT       fdsData;

   // Object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Get folder's profile information into folder's profile structure.
   WPFOLDER::GetProfileInfo(&ffsData);

   // Write folder's extra information.
   if (write(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);

   // Initialize drive object information.
   fdsData.m_uDriveIndex = GetDriveIndex();
   fdsData.m_Type        = (WORD)GetDriveType();

   // Write drive's extra information.
   if (write(hProfile,&fdsData,sizeof(FILE_DRIVE_STRUCT))!=sizeof(FILE_DRIVE_STRUCT))
      return (FALSE);

   // Save the number of child objects in this container object.
   fcsData.m_nObjects = 0;

   // Write container extra information.
   if (write(hProfile,&fcsData,sizeof(FILE_CONTAINER_STRUCT))!=sizeof(FILE_CONTAINER_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPDRIVE.CPP

