//=====================================================================
// WPDATAFL.CPP
//    WPDATAFILE Class implementation.
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

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
// Windows library header files.
#include <shellapi.h>
#include <commdlg.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
BOOL        CommonFileDialogBox(HWND hWnd,DWORD dwFlags,char *pszDialogTitle,char *pszFilterString,char *pszFile,char *pszFileTitle);
HINSTANCE   ExecuteApplication(HWND hwndParent,LPCSTR pszOp,LPCSTR pszFile,LPCSTR pszParams,LPCSTR pszDir,int nCmdShow);
void        RemoveLeadingSpacesFromString(char *pszString);
void        RemoveTrailingSpacesFromString(char *pszString);
BOOL        CopyFile(char *pszSource,char *pszDest);
BOOL        DeleteFile(char *pszFile);


//---------------------------------------------------------------------
// EXTERNAL GLOBAL VARIABLES
//---------------------------------------------------------------------
// Defined in WPPROG.CPP
extern char *apszWinExecErrorString[32];

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Datafile object default values.
#define WPS_DATAFILE_DEFAULT_ICONINDEX  ICONINDEX(IDI_DEFAULTDATA)
// Datafile object system menu resource name.
#define WPDATAFILE_SYSMENU_NAME         DataFilePopUpMenu


//---------------------------------------------------------------------
// WORKPLACE SHELL DATAFILE CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawDetailsIconText():
//    This function draws the datafile's real name, file size, date,
// and time on the details view.
//---------------------------------------------------------------------
void WPDATAFILE::DrawDetailsIconText(HDC hdc)
{
   HFONT   hfont,hfontOld;     // Handles to fonts.
   LOGFONT logfontIconText;    // Logical font information structure.
   RECT    rcText;
   int     iPrevMap,iPrevBk;   // Previous mapping mode and background mode.
   int     y;                  // Y-coordinate for this object's row.
   char    szInfo[32],szFName[_MAX_FNAME],szExt[_MAX_EXT];
   struct tm *ptmDateTime;

   // First, draw the object's "title" text on details view column.
   WPOBJECT::DrawDetailsIconText(hdc);

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Set current background mode to opaque.
      iPrevBk  = SetBkMode(hdc,OPAQUE);

   // Setup device context modes correctly and save the original values.
   iPrevMap = SetMapMode(hdc,MM_TEXT);

   // Get default system icon text font.
   if (SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&logfontIconText,0)==FALSE)
      return;

   // Create font.
   hfont = CreateFontIndirect(&logfontIconText);

   // Set the current font to the newly created small font.
   hfontOld = (HFONT)SelectObject(hdc,(HGDIOBJ)hfont);

   // Caculate and store y-coordinate of object's row in details table.
   y = DV_INDENT_CY+(GetParent()->GetObjectIndex(this)*DV_LINEITEM_SIZE_CY);

   // Calculate "name" column text rectangle.
   SetRectCXCY(&rcText,
               (DV_NAME_INDENT_CX+5),y,
               (DV_NAME_SIZE_CX-10),
               DV_NAME_SIZE_CY);
   // Get datafile's real name and draw it on "name" column.
   _splitpath(GetDataName(),NULL,NULL,szFName,szExt);
   sprintf(szInfo,"%s%-s",szFName,szExt);
   strupr(szInfo);
   DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX));

   // Try to get datafile file information...
   if (m_fValidFileInfo==TRUE)
   {
      // Calculate "size" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_SIZE_INDENT_CX+5),y,
                  (DV_SIZE_SIZE_CX-10),
                  DV_SIZE_SIZE_CY);
      // Make string containing datafile's size (in bytes).
      sprintf(szInfo,"%lu",m_statFileInfo.st_size);
      DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX));

      // Calculate "date" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_DATE_INDENT_CX+5),y,
                  (DV_DATE_SIZE_CX-10),
                  DV_DATE_SIZE_CY);
      // Make string containing datafile's size (in bytes).
      ptmDateTime = localtime(&(m_statFileInfo.st_mtime));
      sprintf(szInfo,"%u-%u-%u",(ptmDateTime->tm_mon+1),ptmDateTime->tm_mday,ptmDateTime->tm_year);
      DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX));

      // Calculate "date" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_TIME_INDENT_CX+5),y,
                  (DV_TIME_SIZE_CX-10),
                  DV_TIME_SIZE_CY);
      // Make string containing datafile's size (in bytes).
      sprintf(szInfo,"%u:%02u:%02u %s",
      (ptmDateTime->tm_hour%12),
      ptmDateTime->tm_min,
      ptmDateTime->tm_sec,
      ((ptmDateTime->tm_hour<11)? "AM" : "PM") );
      // Draw "time" field information.
      DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX));
   }

   // Restore original mapping and background modes.
   SetMapMode(hdc,iPrevMap);

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Restore original background mode.
      SetBkMode(hdc,iPrevBk);

   // Clean-up device context objects.
   SelectObject(hdc,(HGDIOBJ)hfontOld);
   DeleteObject(hfont);
}

//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDATAFILE():
//---------------------------------------------------------------------
WPDATAFILE::WPDATAFILE(void) : WPOBJECT()
{
   // Initialize datafile object.
   SetType(OBJTYPE_DATAFILE);
   SetDataSubject("");
   SetDataPath("");
   SetDataName("");
}

//---------------------------------------------------------------------
// WPDATAFILE():
//---------------------------------------------------------------------
WPDATAFILE::WPDATAFILE(WPDATAFILE *pwpdatafileDefaults) : WPOBJECT((WPOBJECT *)pwpdatafileDefaults)
{
   // Initialize datafile object.
   SetType(OBJTYPE_DATAFILE);
   SetDataSubject(pwpdatafileDefaults->GetDataSubject());
   SetDataPath(pwpdatafileDefaults->GetDataPath());
   SetDataName(pwpdatafileDefaults->GetDataName());
}


//---------------------------------------------------------------------
// DATAFILE PATH, PARAMETERS, AND WORKING DIRECTORY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDataSubject():
//---------------------------------------------------------------------
const char *WPDATAFILE::GetDataSubject(void)
{
   return (m_szDataSubject);
}

//---------------------------------------------------------------------
// GetDataPath():
//---------------------------------------------------------------------
const char *WPDATAFILE::GetDataPath(void)
{
   return (m_szDataPath);
}

//---------------------------------------------------------------------
// GetDataName():
//---------------------------------------------------------------------
const char *WPDATAFILE::GetDataName(void)
{
   return (m_szDataName);
}


//---------------------------------------------------------------------
// SetDataSubject():
//---------------------------------------------------------------------
void WPDATAFILE::SetDataSubject(const char *pszDataSubject)
{
   // Copy new datafile subject into private data member.
   strncpy(m_szDataSubject,pszDataSubject,MAX_DATAFILE_SUBJECT);
   // Make sure datafile subject is null-terminated.
   m_szDataSubject[MAX_DATAFILE_SUBJECT] = '\0';
}


//---------------------------------------------------------------------
// SetDataPath():
//---------------------------------------------------------------------
void WPDATAFILE::SetDataPath(const char *pszDataPath)
{
   int index;

   // Copy new datafile path into private data member.
   strncpy(m_szDataPath,pszDataPath,MAX_DATAFILE_PATH);
   // Make sure datafile path is null-terminated.
   m_szDataPath[MAX_DATAFILE_PATH] = '\0';
   // Make sure that working directory path is not terminated with backslash "\".
   // Initialize local variable with length of current working directory string.
   index = (strlen(m_szDataPath)-1);
   // While there are more characters to inspect...
   while (index>=0)
   {
      // Check if character is a white-space...
      if (IsSpace(m_szDataPath[index]))
         // Advance to previous character position.
         index--;
      else
      // Check if character is a backslash...
      if (m_szDataPath[index]=='\\')
      {
         // Remove backslash by null-terminating string at that position.
         m_szDataPath[index] = '\0';
         // Break out of while loop.
         break;
      }
      else
         // Since character is not any of the above, exit while loop.
         break;
   }

}


//---------------------------------------------------------------------
// SetDataName():
//---------------------------------------------------------------------
void WPDATAFILE::SetDataName(const char *pszDataName)
{
   // Copy new datafile name into private data member.
   strncpy(m_szDataName,pszDataName,MAX_DATAFILE_NAME);
   // Make sure datafile name is null-terminated.
   m_szDataName[MAX_DATAFILE_NAME] = '\0';
}


//---------------------------------------------------------------------
// GetFullDataPath():
//---------------------------------------------------------------------
const char *WPDATAFILE::GetFullDataPath(void)
{
   static char szFullDataPath[MAX_DATAFILE_PATH+MAX_DATAFILE_NAME+1];
   char        szDataPath[MAX_DATAFILE_PATH+1];
   int         nLength;

   // Make a local copy of data file path string (if any).
   strcpy(szDataPath,GetDataPath());
   // If data file object has a data file path...
   if (strlen(szDataPath)>0)
   {
      // Get length of data path string.
      nLength = strlen(szDataPath);
      // If the data path is terminated by a backslash character,
      if (szDataPath[nLength-1]=='\\')
         // Remove last backslash char. since we automatically add it.
         szDataPath[nLength-1] = '\0';
      // Create full datafile path string.
      sprintf(szFullDataPath,"%s\\%s",szDataPath,GetDataName());
   }
   else
      // Create full datafile path string.
      sprintf(szFullDataPath,"%s",GetDataName());

   // Return pointer to static string with full datafile path.
   return (szFullDataPath);
}


//---------------------------------------------------------------------
// UpdateFileSystemObjectName():
//---------------------------------------------------------------------
void WPDATAFILE::UpdateFileSystemObjectName(char *szNewTitle)
{  
   SetDataName(szNewTitle);
}



//---------------------------------------------------------------------
// DATAFILE RESOURCES INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LoadFileInfo():
//---------------------------------------------------------------------
void WPDATAFILE::LoadFileInfo(void)
{
   UINT    uPrevErrorMode;

   // Turn-off the Windows 3.1 INT 24H error handling and "drive not ready" boxes.
   uPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

   // Try to get program file information...
   if (_stat(GetFullDataPath(),&m_statFileInfo)==0)
      m_fValidFileInfo = TRUE;
   else
      m_fValidFileInfo = FALSE;

   // Restore original Windows 3.1 error mode before leaving.
   SetErrorMode(uPrevErrorMode);
}

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPDATAFILE::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   char szProgramAssociated[_MAX_PATH+1];

   // Check if executable is associated with data file...
   
   if ((strlen(GetDataName())>0) && (FindExecutable(GetDataName(),GetDataPath(),szProgramAssociated)>(HINSTANCE)32))
   {
      // Set program associated with data file and icon index.
      strcpy(pszIconPath,szProgramAssociated);
      *pwIconIndex = 0;
   }
   else
   {
      // All of the following built-in types come from WPS shell executable.
      strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
      // Check if datafile is an audio file (*.WAV)...
      if (stristr(GetDataName(),".ICO")!=NULL)
      {
         // Use icon data file name for icon path.
         strcpy(pszIconPath,GetFullDataPath());
         *pwIconIndex = 0;
      }
      else
      // Check if datafile is an audio file (*.WAV)...
      if (stristr(GetDataName(),".WAV")!=NULL)
         *pwIconIndex = ICONINDEX(IDI_AUDIO_FILE);
      else
      // Check if datafile is a music file (*.MID)...
      if (stristr(GetDataName(),".MID")!=NULL)
         *pwIconIndex = ICONINDEX(IDI_MIDI_FILE);
      // File is none of the previous, therefore assume defaults...
      else
         *pwIconIndex = WPS_DATAFILE_DEFAULT_ICONINDEX;
   }
}

//---------------------------------------------------------------------
// LoadIcon():
//---------------------------------------------------------------------
BOOL WPDATAFILE::LoadIcon(void)
{
   // Read file information (size, date, time, etc..).
   LoadFileInfo();
   // Read icon resource for datafile object.
   return (WPOBJECT::LoadIcon());
}

//---------------------------------------------------------------------
// SORT KEY VALUE QUERY FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetSortValue():
//    This function returns (in the passed structure), the key value
// requested from the current object.  Derived objects that
// do support other key values should overload this virtual function
// and copy the correct value into the passed structure.
//---------------------------------------------------------------------
void WPDATAFILE::GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue)
{
   // Depending on sort key value requested...
   switch (wSortKey)
   {
      case IDM_SORTBYREALNAME:
         // Get object title string without special characters.
         strcpy(psvsSortValue->m_szValue,GetDataName());
         strupr(psvsSortValue->m_szValue);
         break;

      case IDM_SORTBYSIZE:
         // Read the latest file statistics.
         LoadFileInfo();
         // Store the requested value in return structure.
         psvsSortValue->m_dwValue = (DWORD)m_statFileInfo.st_size;
         break;

      case IDM_SORTBYLASTACCESSDATE:
      case IDM_SORTBYLASTACCESSTIME:
         // Read the latest file statistics.
         LoadFileInfo();
         // Store the requested value in return structure.
         psvsSortValue->m_dwValue = (DWORD)m_statFileInfo.st_atime;
         break;

      default:
         // Call base class function.
         WPOBJECT::GetSortValue(wSortKey,psvsSortValue);
         break;
   }
}


//---------------------------------------------------------------------
// DATAFILE OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPDATAFILE::Open(void)
{
   UINT        uReturnValue;
   char        szErrorMsg[256];

   // Execute application associated with this data file object extension.
   uReturnValue = (UINT)ExecuteApplication(GetDesktopWindow(),"open",
                           GetFullDataPath(),"",GetDataPath(),SW_SHOWNORMAL);

   // Check for possible erros executing data file's associated program.
   if (uReturnValue<32)
   {
      switch (uReturnValue)
      {
         case 16:
            return (FALSE);
            break;

         default:
            // Report error code as a meaningful text string.
            sprintf(szErrorMsg,GetString(IDSTR_ERR_OBJEXEC),
               uReturnValue,
               GetTitle(),
               GetFullDataPath(),
               apszWinExecErrorString[uReturnValue]);
            MessageBox(NULL,szErrorMsg,GetString(IDSTR_ERR_CAPTION),MB_OK|MB_ICONSTOP);
            return (FALSE);
      }
   }

   // Executed datafile successfully.
   return (TRUE);
}


//---------------------------------------------------------------------
// Print():
//---------------------------------------------------------------------
BOOL WPDATAFILE::Print(void)
{
   UINT        uReturnValue;
   HINSTANCE   hInstProgram;
   char        szErrorMsg[128];

   // Execute application.
   hInstProgram = ExecuteApplication(GetDesktopWindow(),"print",GetFullDataPath(),"",GetDataPath(),SW_SHOWNORMAL);

   // Convert instance handle to unsigned integer.
   uReturnValue = (UINT)hInstProgram;

   // Check for possible erros executing program.
   if (uReturnValue<32)
   {
      switch (uReturnValue)
      {
         case 16:
            return (FALSE);
            break;

         default:
            // Report error code as a meaningful text string.
            sprintf(szErrorMsg,GetString(IDSTR_ERR_OBJEXEC),
               uReturnValue,
               GetTitle(),
               GetFullDataPath(),
               apszWinExecErrorString[uReturnValue]);
            MessageBox(NULL,szErrorMsg,GetString(IDSTR_ERR_CAPTION),MB_OK|MB_ICONSTOP);
            return (FALSE);
      }
   }

   // Printed program successfully.
   return (TRUE);
}


//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPDATAFILE::Close(void)
{
   // Let base object class handle the rest of close.
   return (WPOBJECT::Close());
}


//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetMenuName():
//    This function returns a null-terminated string with the name of
// the menu resource name used as the system menu for that object.
//---------------------------------------------------------------------
const char *WPDATAFILE::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPDATAFILE_SYSMENU_NAME));  // return the default generic object menu.
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPDATAFILE::MenuSelection(WORD wMenuItem)
{
   // Process menu selections from the datafile's system menu.
   switch (wMenuItem)
   {
      case IDM_OPENDATAFILE:
         // The "open" function executes the program associated with datafile and
         // opens the datafile for editing (Windows 3.1 Registration Database).
         Open();
         break;

      case IDM_HELPGENERAL:
         // Open help file with "General help" for data file objects.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_DATAFILE);
         break;

      case IDM_PRINT:
         // The "print" function prints datafile object by using program
         // associated with datafile (Windows 3.1 Registration Database).
         Print();
         break;

      default:
         // Let object handle the rest of menu selections.
         WPOBJECT::MenuSelection(wMenuItem);
         break;
   }
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TestDropSource():
//---------------------------------------------------------------------
BOOL WPDATAFILE::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // Check if object class accepts target object "pwpobjDropTarget".
   if (WPOBJECT::TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Check if datafile class has any problems with being a drop source
      // on target object "pwpobjDropTarget" with function "nFunction".
      switch (nFunction)
      {
         case DROP_MOVE:
         case DROP_COPY:
         case DROP_SHADOW:
            return (pwpobjDropTarget->TestDropTarget(this,nFunction));

         default:
            // No other functions supported by datafile class.
            return (FALSE);
      }
   }
   else
      // Datafile class does not support drop target and/or function.
      return (FALSE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
BOOL WPDATAFILE::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if object class accepts source object "pwpobjDropSource".
   if (WPOBJECT::TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Check if datafile object can be the target of source object "pwpobjDropSource".
      return (FALSE);
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropSource():
//---------------------------------------------------------------------
BOOL WPDATAFILE::DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice)
{
   // Check if object class accepts being dropped on an object "pwpobjDropTarget".
   if (TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Depending on what drag & drop function...
      switch (nFunction)
      {
         case DROP_MOVE:
            // If we are being dropped into a program object...
            if ((pwpobjDropTarget->GetType()&OBJTYPE_PROGRAM)==OBJTYPE_PROGRAM)
            {
               // Now, move (or give) ourseleves into the drop target
               return (pwpobjDropTarget->DropTarget(this,nFunction));
            }
            else
               // ...Else, call the default function handler.
               return (WPOBJECT::DropSource(pwpobjDropTarget,nFunction,ptDevice));

         case DROP_COPY:
            // Copy datafile object into target object.
         case DROP_SHADOW:
            // Shadow datafile object in target object.
            return (WPOBJECT::DropSource(pwpobjDropTarget,nFunction,ptDevice));

         default:
            // No other functions supported by datafile class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//---------------------------------------------------------------------
BOOL WPDATAFILE::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
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
            // No other functions supported by datafile class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InsertSettingsNotebookPages(void)
//------- --------------------------------------------------------------
void WPDATAFILE::InsertSettingsNotebookPages(void)
{
   // Insert the "Type" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_TYPE),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPDATAFILE::TypePageProc,NULL,MAKEINTRESOURCE(IDD_TYPE_PAGE));
   // Insert the "File" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_FILE),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPDATAFILE::FilePageProc,NULL,MAKEINTRESOURCE(IDD_FILE_PAGE));
   // Insert the default object pages into the settings notebook.
   WPOBJECT::InsertSettingsNotebookPages();
}

//---------------------------------------------------------------------
// TypePageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPDATAFILE::TypePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   HWND  hwndFilePage;
   LONG  lSize;
   char  szFileExt[_MAX_EXT+1],
         szClassName[32],szDataName[MAX_DATAFILE_NAME+1],
         szTypeString[MAX_DATAFILE_SUBJECT+1];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         break;

      case PAGE_SELECTED:
         // Initialize "Type" string to be "(None)".
         strcpy(szTypeString,GetString(IDSTR_NONE_PARENTHESIS));
         // Get "File" page's window handle.
         hwndFilePage = GetSettingsNotebook()->GetWindowFromPageIndex(nPageIndex+1);
         // Get text in "File" page's "Name" field.
         if (hwndFilePage!=NULL)
            GetDlgItemText(hwndFilePage,IDE_NAME,szDataName,MAX_DATAFILE_NAME);
         else
            strcpy(szDataName,"");
         // Get data-file (3-character) extention.
         _splitpath(szDataName,NULL,NULL,NULL,szFileExt);
         // Check if file has an extension..
         if (strlen(szFileExt)>0)
         {
            // Initialize variable containing maximum size of string.
            lSize = (sizeof(szClassName)-1);
            // Query the class name registered for data-file extentsion.
            if (RegQueryValue(HKEY_CLASSES_ROOT,szFileExt,szClassName,&lSize)==ERROR_SUCCESS)
            {
               // Initialize variable containing maximum size of string.
               lSize = (sizeof(szTypeString)-1);
               // Query the class description using the class name previuosly retrieved.
               RegQueryValue(HKEY_CLASSES_ROOT,szClassName,szTypeString,&lSize);
            }
         }
         // Display "Type" string in read-only edit field.
         SetDlgItemText(hwndPage,IDE_TYPE,szTypeString);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_UNDO:
               break;

            case IDB_HELP:
               // Open help file with "Type" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_TYPEPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         break;
   }
}

//---------------------------------------------------------------------
// FindDataFile():
//---------------------------------------------------------------------
BOOL WPDATAFILE::FindDataFile(HWND hwndPage)
{
   char  szFile[_MAX_PATH+1],szFileTitle[(_MAX_FNAME+_MAX_EXT)+1],
         szDrive[_MAX_DRIVE+1],szPath[_MAX_PATH+1],
         szName[_MAX_FNAME+1],szExt[_MAX_EXT+1],
         szWork[_MAX_DRIVE+_MAX_PATH+1],
         szFilter[] = "All Files (*.*)|*.*|"
                      "Data Files|*.dat|"
                      "Text Files|*.txt;*.doc;*.asc|"
                      "Graphic Files|*.bmp;*.ico;*.cur;*.pcx|"
                      "C/C++ Files|*.c;*.h;*.cpp;*.hpp|"
                      "Pascal Files|*.pas|"
                      "Make/Project Files|*.mak;*.prj|"
                      "Programs|*.exe;*.pif;*.com;*.bat|";

   // Open "Find Data File" dialog box, and wait until user selects a data file.
   if (CommonFileDialogBox(hwndPage,(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST),GetString(IDSTR_FILEDLG_TITLE_FIND_DATAFILE),szFilter,szFile,szFileTitle)==TRUE)
   {
      // Split user-selected data file into path and filename.
      _splitpath(szFile,szDrive,szPath,szName,szExt);
      // Create <drive>\<path> string string.
      sprintf(szWork,"%s%s",szDrive,szPath);
      // Store data file's path string.
      SetDataPath(szWork);
      // Create data filename string (<name>.<ext>).
      sprintf(szWork,"%s%s",szName,szExt);
      // Store data file's filename string.
      SetDataName(szWork);
      // Copy filename and path into page's entry field controls.
      SetDlgItemText(hwndPage,IDE_PATH,GetDataPath());
      SetDlgItemText(hwndPage,IDE_NAME,GetDataName());
      // Return sucessfully found file.
      return (TRUE);
   }
   else
      // Return failure since user didn't select a valid file.
      return (FALSE);
}

//---------------------------------------------------------------------
// FilePageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPDATAFILE::FilePageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{  
   char szTempString[MAX_DATAFILE_PATH+1];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Set text size limit on edit fields.
         SendDlgItemMessage(hwndPage,IDE_SUBJECT,EM_LIMITTEXT,(WPARAM)MAX_DATAFILE_SUBJECT,0L);
         SendDlgItemMessage(hwndPage,IDE_PATH,EM_LIMITTEXT,(WPARAM)MAX_DATAFILE_PATH,0L);
         SendDlgItemMessage(hwndPage,IDE_NAME,EM_LIMITTEXT,(WPARAM)MAX_DATAFILE_NAME,0L);
         // Initialize edit fields with data object information.
         SetDlgItemText(hwndPage,IDE_SUBJECT,GetDataSubject());
         SetDlgItemText(hwndPage,IDE_PATH,GetDataPath());
         SetDlgItemText(hwndPage,IDE_NAME,GetDataName());
         // If data object is currently owned by a directory folder...
         if (GetParent()->isDirectoryContainer()==TRUE)
         {
            // Do not allow user to change file name or path from this page.
            SendDlgItemMessage(hwndPage,IDE_SUBJECT,EM_SETREADONLY,TRUE,0L);
            SendDlgItemMessage(hwndPage,IDE_PATH,EM_SETREADONLY,TRUE,0L);
            SendDlgItemMessage(hwndPage,IDE_NAME,EM_SETREADONLY,TRUE,0L); 
            // Disable the "Find..." button since we do not want the user to use it.
            EnableWindow(GetDlgItem(hwndPage,IDB_FIND),FALSE);
         }
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_FIND:
               // Let user browse thourgh disks for data file wanted.
               if (FindDataFile(hwndPage)==TRUE)
               {
                  // Update icon since it might be different from the previous one.
                  UpdateIcon();
                  // Update settings notebook icon resource.
                  GetSettingsNotebook()->UpdateNotebookIconResource();
               }
               break;

            case IDB_UNDO:
               SetDlgItemText(hwndPage,IDE_SUBJECT,GetDataSubject());
               SetDlgItemText(hwndPage,IDE_PATH,GetDataPath());
               SetDlgItemText(hwndPage,IDE_NAME,GetDataName());
               break;

            case IDB_HELP:
               // Open help file with "File" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_FILEPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Retrieve datafile information from 'File' page entry fields.
         GetDlgItemText(hwndPage,IDE_SUBJECT,szTempString,MAX_DATAFILE_SUBJECT);
         SetDataSubject(szTempString);
         // Retrieve datafile information from 'File' page entry fields.
         GetDlgItemText(hwndPage,IDE_PATH,szTempString,MAX_DATAFILE_PATH);
         RemoveLeadingSpacesFromString(szTempString);
         RemoveTrailingSpacesFromString(szTempString);
         SetDataPath(szTempString);
         // Retrieve datafile information from 'File' page entry fields.
         GetDlgItemText(hwndPage,IDE_NAME,szTempString,MAX_DATAFILE_NAME);
         RemoveLeadingSpacesFromString(szTempString);
         RemoveTrailingSpacesFromString(szTempString);
         SetDataName(szTempString);         
         // Update data file information (size, date, time, etc...).
         LoadFileInfo();
         // Update icon since it might be different from the previous one.
         UpdateIcon();
         break;
   }
}

//---------------------------------------------------------------------
// OBJECT TO FILE SYSTEM FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CopyObjectToFileSystem():
//---------------------------------------------------------------------
BOOL WPDATAFILE::CopyObjectToFileSystem(char *pszDestPath)
{
   char szDest[_MAX_PATH+1];

   sprintf(szDest,"%s\\%s",pszDestPath,GetDataName());
   return (CopyFile((char *)GetFullDataPath(),szDest));
}

//---------------------------------------------------------------------
// DeleteObjectFromFileSystem():
//---------------------------------------------------------------------
BOOL WPDATAFILE::DeleteObjectFromFileSystem(void)
{
   return (DeleteFile((char *)GetFullDataPath()));
}


//---------------------------------------------------------------------
// WORKPLACE SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPDATAFILE::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_DATAFILE_STRUCT   fdsData;

   // Object information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read datafile class information.
   if (read(hProfile,&fdsData,sizeof(FILE_DATAFILE_STRUCT))!=sizeof(FILE_DATAFILE_STRUCT))
      return (FALSE);

   // Initialize datafile object values using data values read from profile.
   SetDataSubject(fdsData.m_szDataSubject);
   SetDataPath(fdsData.m_szDataPath);
   SetDataName(fdsData.m_szDataName);

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPDATAFILE::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_DATAFILE_STRUCT   fdsData;

   // Object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Initialize datafile data structure with datafile values.
   strcpy(fdsData.m_szDataSubject,GetDataSubject());
   strcpy(fdsData.m_szDataPath,GetDataPath());
   strcpy(fdsData.m_szDataName,GetDataName());

   // Write datafile class information.
   if (write(hProfile,&fdsData,sizeof(FILE_DATAFILE_STRUCT))!=sizeof(FILE_DATAFILE_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPDATAFL.CPP


