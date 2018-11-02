//=====================================================================
// MISCFUNC.CPP
//    Miscellaneous functions used in WPSHELL.
//
// Date Created :  01/26/94
// Last Update  :  02/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/26/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dos.h>
#include <direct.h>     // _mkdir() function is prototyped.
#include <errno.h>      // "errno" global variable is prototyped.
// Windows library header files.
#include <commdlg.h>
#include <mmsystem.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// new_object()
//    This function creates a new object of the same type passed as a
// parameter.  It does this by checking "pwpobjNew"'s object type, and
// calling the appropriate "new" C++ constructor for that object type.
//    It then returns a pointer to the newly created object (it has
// been initialized with the same values as the object passed).
//---------------------------------------------------------------------
WPOBJECT *new_object(WPOBJECT *pwpobjNew)
{
   // Depending on the object type...
   switch (pwpobjNew->GetType())
   {
      case OBJTYPE_TEMPLATE:
         return (new WPTEMPLATE((WPTEMPLATE *)pwpobjNew));

      case OBJTYPE_DATAFILE:
         return (new WPDATAFILE((WPDATAFILE *)pwpobjNew));

      case OBJTYPE_SHREDDER:
         return (new WPSHREDDER((WPSHREDDER *)pwpobjNew));

      case OBJTYPE_PROGRAM:
         return (new WPPROGRAM((WPPROGRAM *)pwpobjNew));

      case OBJTYPE_SYSTEMSETUP:
         return (new WPSYSSETUP((WPSYSSETUP *)pwpobjNew));

      case OBJTYPE_FOLDER:
         return (new WPFOLDER((WPFOLDER *)pwpobjNew));

      case OBJTYPE_DIRECTORY:
         return (new WPDIRECTORY((WPDIRECTORY *)pwpobjNew));

      case OBJTYPE_MINVIEWER:
         return (new WPMINVIEWER((WPMINVIEWER *)pwpobjNew));

      case OBJTYPE_SHADOW:
         return (new WPSHADOW((WPSHADOW *)pwpobjNew));

      case OBJTYPE_DRIVE:
         return (new WPDRIVE((WPDRIVE *)pwpobjNew));

      case OBJTYPE_DESKTOP :
         return (new WPDESKTOP());

      default:
         return (NULL);
   }
}


//---------------------------------------------------------------------
// delete_object()
//    This function destroys an existing object by calling the
// appropriate "delete" C++ destructor for the type.  If this is not
// done, the memory allocated for object could be freed incorrectly.
//---------------------------------------------------------------------
void delete_object(WPOBJECT *pwpobjDelete)
{
   // Depending on the object type...
   switch (pwpobjDelete->GetType())
   {
      case OBJTYPE_TEMPLATE:
         delete (WPTEMPLATE *)pwpobjDelete;
         break;

      case OBJTYPE_DATAFILE:
         delete (WPDATAFILE *)pwpobjDelete;
         break;

      case OBJTYPE_SHREDDER:
         delete (WPSHREDDER *)pwpobjDelete;
         break;

      case OBJTYPE_PROGRAM:
         delete (WPPROGRAM *)pwpobjDelete;
         break;

      case OBJTYPE_SYSTEMSETUP:
         delete (WPSYSSETUP *)pwpobjDelete;
         break;

      case OBJTYPE_FOLDER:
         delete (WPFOLDER *)pwpobjDelete;
         break;

      case OBJTYPE_MINWINDOW:
         delete (WPMINWNDOBJ *)pwpobjDelete;
         break;

      case OBJTYPE_MINVIEWER:
         delete (WPMINVIEWER *)pwpobjDelete;
         break;

      case OBJTYPE_SHADOW:
         delete (WPSHADOW *)pwpobjDelete;
         break;

      case OBJTYPE_DRIVE:
         delete (WPDRIVE *)pwpobjDelete;
         break;

      case OBJTYPE_DIRECTORY:
         delete (WPDIRECTORY *)pwpobjDelete;
         break;

      case OBJTYPE_DESKTOP:
         delete (WPDESKTOP *)pwpobjDelete;
         break;
   }
}


//---------------------------------------------------------------------
// DestroyRecursively():
//    This function destroys a tree of objects recursively.  It parses
// the object tree in a depth-first search manner, then it starts
// destroying the leaf nodes first, and then its parent.
//    Before destroying an object, this function makes sure that the
// object is properly closed (if open).
//---------------------------------------------------------------------
void DestroyRecursively(WPOBJECT *pwpobjDestroy)
{
   WPCONTAINER *pwpContainer;
   int         index;

   // Check if object is not a shadow object...
   if ((pwpobjDestroy->GetType()&OBJTYPE_SHADOW)!=OBJTYPE_SHADOW)
   {
      // If object has a settings notebook open, close it & destory it.
      pwpobjDestroy->CloseSettingsNotebook();

      // Close object before destroying it.
      pwpobjDestroy->Close();
   }

   // Check if object is container object...
   if ((pwpobjDestroy->GetType()&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)
   {
      // Convert object pointer to container object.
      pwpContainer = (WPCONTAINER *)pwpobjDestroy;
      // Destroy the container children before destroying itself.
      for (index=0; index<pwpContainer->GetObjectCount(); index++)
         // Destroy its children first.
         DestroyRecursively(pwpContainer->GetObject(index));
   }

   // Destroy object.
   delete_object(pwpobjDestroy);
}


//---------------------------------------------------------------------
// CreateTreeDuplicate():
//    This function creates a duplicate of the passed tree of objects.
// It gets as parameters a pointer to the parent container object of the
// newly created tree, and a pointer to the root of the tree to be
// duplicated.
//    It returns a pointer to the newly created tree duplicate root.
//---------------------------------------------------------------------
WPOBJECT *CreateTreeDuplicate(WPCONTAINER *pwpcontainerParent,WPOBJECT *pwpobjOrigRoot)
{
   WPOBJECT    *pwpobjDup,*pwpobjChildDup;
   WPCONTAINER *pwpcontainerDup;
   int         index;

   // Create a new instance of root object.
   pwpobjDup = new_object(pwpobjOrigRoot);
   // Check if object is duplicable (if not, then exit).
   if (pwpobjDup==NULL)
      return (NULL);
   // Make new object point to its parent container.
   pwpobjDup->SetParent(pwpcontainerParent);

   // Check if original object is a container object.
   if ((pwpobjOrigRoot->GetType()&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)
   {
      // Cast pointer from generic object to container object (for convenience).
      pwpcontainerDup = (WPCONTAINER *)pwpobjDup;
      // Create duplicates for all of the container children components.
      for (index=0; index<((WPCONTAINER *)pwpobjOrigRoot)->GetObjectCount(); index++)
      {
         // Duplicate child object.
         pwpobjChildDup = CreateTreeDuplicate(pwpcontainerDup,((WPCONTAINER *)pwpobjOrigRoot)->GetObject(index));
         // If child object was duplicable, then add to duplicated parent container object.
         if (pwpobjChildDup!=NULL)
            pwpcontainerDup->AddObject(pwpobjChildDup);
      }
   }

   // Return pointer to new duplicate.
   return (pwpobjDup);
}


//---------------------------------------------------------------------
// ShellHookProc():
//    This callback hook procedure is used to manage the application
// windows created by Windows 3.1.  This hook receives a notification
// message everytime a top-level window (OVERLAPPED or POPUP), is
// created or destroyed.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "'lParam' not used."
LRESULT CALLBACK _loadds ShellHookProc(int iCode,WPARAM wParam,LPARAM lParam)
{
   DWORD    dwStyle;

   // Depending on what function is being reported by Windows 3.1 hook...
   switch (iCode)
   {
      case HSHELL_ACTIVATESHELLWINDOW:
         break;

      case HSHELL_WINDOWCREATED:
         // Get window styles of newly created top-level window.
         dwStyle = GetWindowLong((HWND)wParam,GWL_STYLE);
         // Check if top-level window has minimized button AND caption bar.
         if ((dwStyle&WS_MINIMIZEBOX)&&(dwStyle&WS_CAPTION))
            // Notify Workplace Shell desktop of top-level window being created.
            PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_WINDOW_CREATED,wParam,0L);
         break;

      case HSHELL_WINDOWDESTROYED:
         // Get window styles of top-level window about to be destroyed.
         dwStyle = GetWindowLong((HWND)wParam,GWL_STYLE);
         // Check if top-level window has minimized button AND caption bar.
         if ((dwStyle&WS_MINIMIZEBOX)&&(dwStyle&WS_CAPTION))
            // Notify Workplace Shell desktop of top-level window being created.
            PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_WINDOW_DESTROYED,wParam,0L);
         break;
   }

   return (0L);
}


//---------------------------------------------------------------------
// ExecuteApplication():
//    This function is a wrapper function for the WinExec() API.  It
// sets the mouse cursor to the "wait" cursor before executing the
// application, calls WinExec(), and restores it after execution has
// completed.
//---------------------------------------------------------------------
UINT ExecuteApplication(LPCSTR pszCmdLine,UINT nCmdShow)
{
   HCURSOR  hcurWait,hcurOriginal;
   UINT     uResult;

   // Load custom "wait" cursor (i.e. clock).
   hcurWait     = LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR));
   // Set current cursor to be the wait cursor (notifying user to wait).
   hcurOriginal = SetCursor(hcurWait);
   // Execute command line (application) with the specified show command.
   uResult = WinExec(pszCmdLine,nCmdShow);
   // Restore original cursor.
   SetCursor(hcurOriginal);
   // Destroy custom wait cursor.
   DestroyCursor(hcurWait);
   // Return result from WinExec() API.
   return (uResult);
}

//---------------------------------------------------------------------
// ExecuteApplication():
//---------------------------------------------------------------------
HINSTANCE ExecuteApplication(HWND hwndParent,LPCSTR pszOp,LPCSTR pszFile,LPCSTR pszParams,LPCSTR pszDir,int nCmdShow)
{
   HCURSOR     hcurWait,hcurOriginal;
   HINSTANCE   hInstance;

   // Load custom "wait" cursor (i.e. clock).
   hcurWait     = LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR));
   // Set current cursor to be the wait cursor (notifying user to wait).
   hcurOriginal = SetCursor(hcurWait);
   // Execute command line (application) with the specified show command.
   hInstance  = ShellExecute(hwndParent,pszOp,pszFile,pszParams,pszDir,nCmdShow);
   // Restore original cursor.
   SetCursor(hcurOriginal);
   // Destroy custom wait cursor.
   DestroyCursor(hcurWait);
   // Return result from ShellExecute() API.
   return (hInstance);
}


//---------------------------------------------------------------------
// GetUndocProc():
//    This function is used to get the addresses of an undocumented
// Windows 3.1 function.  It needs the function name, module name,
// and ordinal position (as a safety check).
//   This function will return NULL if anything goes wrong or
// looks "fishy".
//    Otherwise, it will return a far pointer to the Windows 3.1
// undocumented procedure.
//---------------------------------------------------------------------
FARPROC GetUndocProc(char *szFunction,char *szModule,int nOrdinal)
{
   HMODULE  hModule;
   FARPROC  lpfnByName,lpfnByOrd;

   // Get module's handle.
   hModule = GetModuleHandle(szModule);

   // Check if valid module handle was returned.
   if (hModule==NULL)
   {
      // If not, then try loading module library.
      hModule = LoadLibrary(szModule);
      // Check if could not load module.
      if (hModule==NULL)
         return (NULL);       // Return failure.
   }

   // Get procedure address by using its name.
   lpfnByName = GetProcAddress(hModule,szFunction);

   // Check that a valid address was returned.
   if (lpfnByName==NULL)
      return (NULL);          // Return failure.

   // Get procedure address by using its ordinal position within module.
   lpfnByOrd = GetProcAddress(hModule,(LPSTR)(DWORD)nOrdinal);

   // Check that a valid address was returned.
   if (lpfnByOrd==NULL)       // Return failure.
      return (NULL);

   // Check if both addresses agree. if they are different, error.
   if (lpfnByName!=lpfnByOrd)
      return (NULL);          // Return failure.

   // Successful getting procedure's address, pass it back.
   return (lpfnByName);
}

//---------------------------------------------------------------------
// CommonFileDialogBox():
//    This function is a wrapper for the common dialog box.  It can
// be customized to any title text and filter string.  It returns
// TRUE if user selected valid file name, file is stored in "pszFile".
// It returns FALSE if user cancelled dialog box or there was an error
// creating the dialog box.  The "pszFileTitle" string contains the
// file name without path.
//---------------------------------------------------------------------
BOOL CommonFileDialogBox(HWND hWnd,DWORD dwFlags,char *pszDialogTitle,char *pszFilterString,char *pszFile,char *pszFileTitle)
{
   OPENFILENAME   ofn;
   int            nLength,index;
   char           chReplace;

   // Reset file name string.
   pszFile[0]      = '\0';
   // Reset file title string (only if it is valid string pointer).
   if (pszFileTitle!=NULL)
      pszFileTitle[0] = '\0';

   // Convert filter string into a COMMDLG z-separated string.
   nLength   = strlen(pszFilterString);
   chReplace = pszFilterString[nLength-1];
   for (index=0; pszFilterString[index]!='\0'; index++)
      if (pszFilterString[index]==chReplace)
         pszFilterString[index] = '\0';

   // Fill with zeros the open file name structure.
   memset(&ofn,0,sizeof(OPENFILENAME));

   // Fill-in the necessary information into structure.
   ofn.lStructSize     = sizeof(OPENFILENAME);
   ofn.hwndOwner       = hWnd;
   ofn.lpstrFilter     = pszFilterString;
   ofn.nFilterIndex    = 1;
   ofn.lpstrFile       = pszFile;
   ofn.nMaxFile        = _MAX_PATH;
   ofn.lpstrFileTitle  = pszFileTitle;
   ofn.nMaxFileTitle   = (_MAX_FNAME+_MAX_EXT);
   ofn.lpstrTitle      = pszDialogTitle;
   ofn.Flags           = dwFlags;

   // Call common file dialog box...
   return (GetOpenFileName(&ofn));
}

//---------------------------------------------------------------------
// PlaySystemEventSound():
//---------------------------------------------------------------------
void PlaySystemEventSound(const char *pszSystemEvent)
{
   BOOL  fSoundEnabled;

   // Get system flag if "system sounds" are enabled or disabled.
   if (SystemParametersInfo(SPI_GETBEEP,0,&fSoundEnabled,0)==TRUE)
   {
      // If system sounds are enabled,
      if (fSoundEnabled==TRUE)
         // Play system sound for folder close event.
         sndPlaySound(pszSystemEvent,(SND_ASYNC|SND_NODEFAULT));
   }
}

//---------------------------------------------------------------------
// WPClientWindowProcWrapper():
//    This function acts as a "thunking" layer between the Windows 3.1
// API interface and the Workplace Shell for Windows C++ interface.
// It is called by the USER module whenever there is a message for a
// Workplace Shell Folder object (or a derived class object).  It
// retrieves or stores the pointer to the C++ object class and calls
// the appropriate client window procedure for the object instance.
//
// Windows 3.1 Developer's Note:
//    Before WM_CREATE message is received, 3 other messages are sent
// by CreateWindowEx().  These messages cannot be passed to the class
// client window procedure and therefore MUST be passed to the default
// Windows 3.1 window procedure.  The messages are listed below for
// reference purposes (they are in order of appearance):
//       WM_GETMINMAXINFO  -> uMsg = 0x0024
//       WM_NCCREATE       -> uMsg = 0x0081
//       WM_NCCALCSIZE     -> uMsg = 0x0083
// Then  WM_CREATE         -> uMsg = 0x0001
//       ect...
//---------------------------------------------------------------------
LRESULT CALLBACK WPClientWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   WPFOLDER       *pobjFolder;    // Pointer to folder object.
   CREATESTRUCT   *pCreateStruct; // Pointer to window creation structure.

   // On the WM_CREATE message 'lParam' contains the pointer to the
   // Workplace Shell object class that owns this window.
   if (uMsg==WM_CREATE)
   {
      // Get pointer to folder object class from create structure.
      pCreateStruct = (CREATESTRUCT *)lParam;
      pobjFolder    = (WPFOLDER *)pCreateStruct->lpCreateParams;
      // Store pointer to folder object into extra window longs.
      SetWindowLong(hWnd,0,(LONG)pobjFolder);
   }
   else
      // Query pointer to folder that owns this window procedure (this pointer is refernced later).
      pobjFolder = (WPFOLDER *)GetWindowLong(hWnd,0);

    // If we do not have a valid folder object pointer...
    if (pobjFolder==NULL)
    {
       // Log event (for debug version).
       #ifdef _DEBUG
          dprintf("tr WPClientWindowProcWrapper(): Msg: 0x%04X 'pobjFolder' is NULL.\n",uMsg);
       #endif
       // Call Windows 3.1's default window procedure.
       return (DefWindowProc(hWnd,uMsg,wParam,lParam));
    }
    else
       // Else, call the "real" client window procedure for folder object.
       return (pobjFolder->ClientWindowProc(hWnd,uMsg,wParam,lParam));
}


//---------------------------------------------------------------------
// RemoveTrailingSpacesFromString():
//    This function removes trailing spaces from a null-terminated
// string.  It uses the <ctype.h> macro "issapce()" which returns
// TRUE if character is a blank space, tab, or newline chars.
//---------------------------------------------------------------------
void RemoveTrailingSpacesFromString(char *pszString)
{
   int  index;

   // If no string is passed OR string is empty, then exit.
   if ((pszString==NULL)||(strlen(pszString)==0))
      return;

   // Initialize index to point to last character in string.
   index = (strlen(pszString)-1);
   // While index is valid AND current ASCII char. is a space...
   while ((index>=0)&&(IsSpace(pszString[index])))
      // Shorten the string by current character and point to previous char.
      pszString[index--] = '\0';
}


//---------------------------------------------------------------------
// RemoveLeadingSpacesFromString():
//    This function removes leading spaces from a null-terminated
// string.  It uses the <ctype.h> macro "issapce()" which returns
// TRUE if character is a blank space, tab, or newline chars.
//---------------------------------------------------------------------
void RemoveLeadingSpacesFromString(char *pszString)
{
   int  index;

   // If no string is passed OR string is empty, then exit.
   if ((pszString==NULL)||(strlen(pszString)==0))
      return;

   // Start from the beginning of string and search for a non-space char.
   for (index=0; index<(int)strlen(pszString); index++)
   {
      // If current character is a non-space character...
      if (!IsSpace(pszString[index]))
      {
         // Copy string without any leading spaces (must use memmove() to ensure
         // that overlapping regions of string get copied correctly).
         memmove(pszString,&(pszString[index]),(strlen(pszString)-index));
         // Exit "for" loop.
         break;
      }
   }
}


//---------------------------------------------------------------------
// LoadStringTable():
//---------------------------------------------------------------------
BOOL LoadStringTable(HINSTANCE hInstance,int nTotalStrings)
{
   int   index,nSize;
   char  szTempString[255];

   // Allocate an array of string pointers that will hold all stings.
   g_wpEnvironment.apszStringTable = new char *[nTotalStrings];
   // If error allocating space for string pointers, then exit.
   if (g_wpEnvironment.apszStringTable==NULL)
      return (FALSE);
   // Load strings from resource string table.
   for (index=0; index<nTotalStrings; index++)
   {
      // Get string and store the its real size.
      nSize = LoadString(hInstance,(IDSTR_BASE_INDEX+index),szTempString,sizeof(szTempString));
      // If error loading string resource, exit.
      if (nSize==0)
         return (FALSE);
      // Allocate enough space for the string read.
      g_wpEnvironment.apszStringTable[index] = new char[nSize+1];
      // If error allocating memory, exit
      if (g_wpEnvironment.apszStringTable[index]==NULL)
         return (FALSE);
      // Copy read string from temporary space to newly allocated space.
      strcpy(g_wpEnvironment.apszStringTable[index],szTempString);
   }
   // Return success (all strings have been loaded).
   return (TRUE);
}


//---------------------------------------------------------------------
// GetString():
//    This function returns the string pointer to the string ID passed
// as parameter.  It indexes the global string pointer array created
// by LoadStringTable() function.
//---------------------------------------------------------------------
char *GetString(UINT uIDString)
{
   // Return pointer to string.
   return (g_wpEnvironment.apszStringTable[(uIDString-IDSTR_BASE_INDEX)]);
}

//---------------------------------------------------------------------
// FreeStringTable():
//---------------------------------------------------------------------
void FreeStringTable(int nTotalStrings)
{
   int   index;

   // Loop through each string in the loaded string table...
   for (index=0; index<nTotalStrings; index++)
      // ...And free the memory allocated for the string.
      delete (g_wpEnvironment.apszStringTable[index]);
   // Free memory allocated for the array of string pointers.
   delete (g_wpEnvironment.apszStringTable);
}

//---------------------------------------------------------------------
// FILE SYSTEM MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateDirectory():
//---------------------------------------------------------------------
BOOL CreateDirectory(char *pszDirectory)
{
   // Create the specified directory path into destination drive.
   if (_mkdir(pszDirectory)!=0)
   {
      // Check that directory was not created because it already existed.
      if (errno!=EACCES)
         return (FALSE);
   }
   // Successfully created directory (or already existed).
   return (TRUE);
}


//---------------------------------------------------------------------
// CopyFile():
//---------------------------------------------------------------------
BOOL CopyFile(char *pszSource,char *pszDest)
{
   HFILE    hfileSource,hfileDest;
   OFSTRUCT lpofSource,lpofDest;
   UINT     uBytesRead;
   BYTE     *pBuffer;

   // Open source file (for read only)...
   hfileSource = OpenFile(pszSource,&lpofSource,(OF_READ|OF_SEARCH));
   if (hfileSource==HFILE_ERROR)
      return (FALSE);
   // Open destination file (for read/write)...
   hfileDest = OpenFile(pszDest,&lpofDest,(OF_CREATE|OF_READWRITE));
   if (hfileDest==HFILE_ERROR)
   {
      _lclose(hfileSource);
      return (FALSE);
   }

   // Initialize local variable.
   uBytesRead = 0;

   // Allocate a 8KB memory buffer (the biggest possible).
   pBuffer = new BYTE[8192];
   // Check if memory allocation was successfull...
   if (pBuffer!=NULL)
   {
      // Read source file a block at a time and copy over to the destination file...
      do
      {
         uBytesRead = _lread(hfileSource,pBuffer,8192);
         // Check that we were successfull reading data from source file...
         if (uBytesRead==HFILE_ERROR)
            break;
         // Copy source file data into destination path...
         _lwrite(hfileDest,pBuffer,uBytesRead);
      }
      while (uBytesRead != 0);

      // Free memory allocated for buffer.
      delete[] pBuffer;
   }

   // Close source and destination files.
   _lclose(hfileSource);
   _lclose(hfileDest);

   // Return failure if error reading data, otherwise return success.
   return ((uBytesRead==HFILE_ERROR)? FALSE : TRUE);
}


//---------------------------------------------------------------------
// CopyDirectory():
//---------------------------------------------------------------------
BOOL CopyDirectory(char *pszSourcePath,char *pszDestPath)
{
   struct _find_t FileFound;
   char           szPath[_MAX_PATH+1],szFullSrcPath[_MAX_PATH+1];

   // Create Directory.
   if (CreateDirectory(pszDestPath)==FALSE)
      return (FALSE);

   // Create string contents.
   sprintf(szPath,"%s\\*.*",pszSourcePath);

   // Find first file or subdirectory within current path.
   if (_dos_findfirst(szPath,(_A_HIDDEN|_A_RDONLY|_A_SUBDIR),&FileFound)==0)
   {
      // Create string contents.
      sprintf(szPath,"%s\\%s",pszDestPath,FileFound.name);
      sprintf(szFullSrcPath,"%s\\%s",pszSourcePath,FileFound.name);
      // If file found is a subdirectory...
      if (FileFound.attrib==_A_SUBDIR)
      {
         // Check that it is not current and parent subdirectory...
         if ((strcmp(FileFound.name,".")!=0)&&(strcmp(FileFound.name,"..")!=0))
            // Copy recursively the subdirectory...
            CopyDirectory(szFullSrcPath,szPath);
      }
      else
         // Copy file from directory.
         CopyFile(szFullSrcPath,szPath);

      // Loop until all files or directories have been scanned...
      while (_dos_findnext(&FileFound)==0)
      {
         // Create string contents.
         sprintf(szPath,"%s\\%s",pszDestPath,FileFound.name);
         sprintf(szFullSrcPath,"%s\\%s",pszSourcePath,FileFound.name);
         // If file found is a subdirectory...
         if (FileFound.attrib==_A_SUBDIR)
         {
            // Check that it is not current and parent subdirectory...
            if ((strcmp(FileFound.name,".")!=0)&&(strcmp(FileFound.name,"..")!=0))
               // Copy recursively the subdirectory...
               CopyDirectory(szFullSrcPath,szPath);
         }
         else
            // Copy file from directory.
            CopyFile(szFullSrcPath,szPath);
      }
   }

   return (TRUE);
}


//---------------------------------------------------------------------
// DeleteFile():
//---------------------------------------------------------------------
BOOL DeleteFile(char *pszFile)
{
   // Make sure that file attributes are normal (not read-only for example).
   _dos_setfileattr(pszFile,_A_NORMAL);
   // Delete specified file.
   return ((_unlink(pszFile)==0)? TRUE : FALSE);
}


//---------------------------------------------------------------------
// DeleteDirectory():
//---------------------------------------------------------------------
BOOL DeleteDirectory(char *pszPath)
{
   BOOL           fContinue,fSuccess;
   struct _find_t fileInfo ;
   char           szDirectory[_MAX_PATH+1] ;

   // Initialize local variable.
   fSuccess = TRUE;
   // Create string containing current directory to be deleted with "*.*" wildcard.
   sprintf(szDirectory,"%s\\*.*",pszPath);
   // Find first occurrence of any file/subdirectory matching "*.*" wildcard.
   fContinue = ((_dos_findfirst(szDirectory,(_A_HIDDEN|_A_RDONLY|_A_SUBDIR),&fileInfo)==0)? TRUE : FALSE);
   // While there is at least one more file/subdirectory to process...
   while ((fContinue==TRUE) && (fSuccess==TRUE))
   {
      // Create string containing current file/directory to be deleted.
      sprintf(szDirectory,"%s\\%s",pszPath,fileInfo.name);
      // Check if it is a subdirectory...
      if (fileInfo.attrib == _A_SUBDIR)
      {
         // Check that it is not the current or parent subdirectory...
         if ((strcmp(fileInfo.name,".")!=0)&&(strcmp(fileInfo.name,"..")!=0))
            // Delete child subdirectory recursively...
            fSuccess = DeleteDirectory(szDirectory);
      }
      else
         // Delete current file.
         fSuccess = DeleteFile(szDirectory);

      // Get next file/subdirectory to process (if any)...
      fContinue = ((_dos_findnext(&fileInfo)==0)? TRUE : FALSE);
   }

   // Remove directory.
   _rmdir(pszPath);

   // Return result.
   return (fSuccess);
}


//---------------------------------------------------------------------
// RenameFileorDirectory():
//---------------------------------------------------------------------
BOOL RenameFileOrDirectory(char *pszOldName,char *pszNewName)
{  
   // Check for valid parameters.
   if ((pszOldName==NULL) || (pszNewName==NULL))
      return (FALSE);            
   // Try to rename old file/directory to the new name.
   return ((rename(pszOldName,pszNewName)==0)? TRUE : FALSE);
}



#if 0


//---------------------------------------------------------------------
// DeleteDirectory():
//---------------------------------------------------------------------
BOOL DeleteDirectory(char *pszPath)
{
   BOOL           fContinue,fSuccess;
   struct _find_t fileInfo ;
   char           szCurrentDirectory[_MAX_PATH+1] ;

   // Initialize local variable.
   fSuccess = TRUE;
   // Store current working directory.
   _getcwd(szCurrentDirectory,_MAX_PATH);
   // Change directory to path to be deleted.
   _chdir(pszPath);
   // Find first occurrence of any file/subdirectory matching "*.*" wildcard.
   fContinue = ((_dos_findfirst("*.*",(_A_HIDDEN|_A_RDONLY|_A_SUBDIR),&fileInfo)==0)? TRUE : FALSE);
   // While there is at least one more file/subdirectory to process...
   while ((fContinue==TRUE) && (fSuccess==TRUE))
   {
      if (fileInfo.attrib == _A_SUBDIR)
      {
         // Check that it is not current and parent subdirectory...
         if ((strcmp(fileInfo.name,".")!=0)&&(strcmp(fileInfo.name,"..")!=0))
            // Delete child subdirectory recursively...
            fSuccess = DeleteDirectory(fileInfo.name);
      }
      else
         // Delete file.
         fSuccess = DeleteFile(fileInfo.name);

      // Get next file/subdirectory to process (if any)...
      fContinue = ((_dos_findnext(&fileInfo)==0)? TRUE : FALSE);
   }

   // Change current directory back to original current directory.
   _chdir(szCurrentDirectory);
   // Remove directory.
   _rmdir(pszPath);

   // Return result.
   return (fSuccess);
}


//---------------------------------------------------------------------
// DeleteDirectory():
//---------------------------------------------------------------------
BOOL DeleteDirectory(char *pszPath)
{
   struct _find_t FileFound;
   char           *pszCurrentPath;

   // Allocate space for path string (current path + '\' + filename + null).
   pszCurrentPath = new char[(strlen(pszPath)+1+_MAX_FNAME+_MAX_EXT+1)];

   // Create string contents.
   sprintf(pszCurrentPath,"%s\\*.*",pszPath);

   // Find first file or subdirectory within current path.
   if (_dos_findfirst(pszCurrentPath,_A_SUBDIR,&FileFound)==0)
   {
      // Create string contents.
      sprintf(pszCurrentPath,"%s\\%s",pszPath,FileFound.name);
      // If file found is a subdirectory...
      if (FileFound.attrib==_A_SUBDIR)
      {
         // Check that it is not current and parent subdirectory...
         if ((strcmp(FileFound->name,".")!=0)&&(strcmp(FileFound->name,"..")!=0))
         {
            // Delete recursively the subdirectory...
            DeleteDirectory(pszCurrentPath);
         }
      }
      else
         // Delete file from directory.
         DeleteFile(pszCurrentPath);

      // Loop until all files or directories have been scanned...
      while (_dos_findnext(&FileFound)==0)
      {
         // Create string contents.
         sprintf(pszCurrentPath,"%s\\%s",pszPath,FileFound.name);
         // If file found is a subdirectory...
         if (FileFound.attrib==_A_SUBDIR)
         {
            // Check that it is not current and parent subdirectory...
            if ((strcmp(FileFound->name,".")!=0)&&(strcmp(FileFound->name,"..")!=0))
            {
               // Delete recursively the subdirectory...
               DeleteDirectory(pszCurrentPath);
            }
         }
         else
            // Delete file from directory.
            DeleteFile(pszCurrentPath);
      }
   }

   // Remove Directory.
   _rmdir(pszPath);

   // Deallocate memory used for path string.
   delete[] pszCurrentPath;

   return (TRUE);
}
#endif

// EOF MISCFUNC.CPP

