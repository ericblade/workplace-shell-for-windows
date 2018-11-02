//=====================================================================
// WPPROG.CPP
//    WPPROGRAM Class implementation.
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
BOOL        CopyFile(char *pszSource,char *pszDest);
BOOL        DeleteFile(char *pszFile);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Program object default values.
#define WPS_PROGRAM_DEFAULT_ICONINDEX  ICONINDEX(IDI_DEFAULTPROGRAM)
// Program object system menu resource name.
#define WPPROGRAM_SYSMENU_NAME         ProgramPopUpMenu

//---------------------------------------------------------------------
// LOCAL VARIABLES
//---------------------------------------------------------------------
char *apszWinExecErrorString[32] = {
   "Out of memory, file corrupt, or invalid relocations.",  // 0
   "Internal error",                                        // 1
   "File not found.",                                       // 2
   "Path not found.",                                       // 3
   "Internal error",                                        // 4
   "Sharing or network-protection error.",                  // 5
   "Library requires separate data segments for each task.",// 6
   "Internal error",                                        // 7
   "Insufficient memory to start application.",             // 8
   "Internal error",                                        // 9
   "Windows version is incorrect.",                         // 10
   "Executable file is invalid.",                           // 11
   "Application defined for different operating system.",   // 12
   "Application is defined for MS-DOS 4.0.",                // 13
   "Type of executable file is unknown.",                   // 14
   "Attempt made to load a real-mode application.",         // 15
   "Attempt made to load second instance of an executable.",// 16
   "Internal error",                                        // 17
   "Internal error",                                        // 18
   "Attempt made to load a compressed executable file.",    // 19
   "Dynamic-link library (DLL) file is invalid.",           // 20
   "Application requires MS-Windows 32-bit extensions.",    // 21
   "",                                                      // 22
   "",                                                      // 23
   "",                                                      // 24
   "",                                                      // 25
   "",                                                      // 26
   "Cannot print file",                                     // 27
   "",                                                      // 28
   "",                                                      // 29
   "",                                                      // 30
   "No Application is associated with this file"            // 31
   };

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WORKPLACE SHELL PROGRAM CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawDetailsIconText():
//    This function draws the program's real name, file size, date,
// and time on the details view.
//---------------------------------------------------------------------
void WPPROGRAM::DrawDetailsIconText(HDC hdc)
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
   // Get program's real name and draw it on "name" column.
   _splitpath(GetProgramPath(),NULL,NULL,szFName,szExt);
   sprintf(szInfo,"%s%-s",szFName,szExt);
   strupr(szInfo);
   DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX));

   // Try to get program file information...
   if (m_fValidFileInfo==TRUE)
   {
      // Calculate "size" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_SIZE_INDENT_CX+5),y,
                  (DV_SIZE_SIZE_CX-10),
                  DV_SIZE_SIZE_CY);
      // Make string containing program's size (in bytes).
      sprintf(szInfo,"%lu",m_statFileInfo.st_size);
      DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX));

      // Calculate "date" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_DATE_INDENT_CX+5),y,
                  (DV_DATE_SIZE_CX-10),
                  DV_DATE_SIZE_CY);
      // Make string containing program's size (in bytes).
      ptmDateTime = localtime(&(m_statFileInfo.st_mtime));
      sprintf(szInfo,"%u-%u-%u",(ptmDateTime->tm_mon+1),ptmDateTime->tm_mday,ptmDateTime->tm_year);
      DrawText(hdc,szInfo,-1,&rcText,(DT_SINGLELINE|DT_RIGHT|DT_VCENTER|DT_NOPREFIX));

      // Calculate "date" column text rectangle.
      SetRectCXCY(&rcText,
                  (DV_TIME_INDENT_CX+5),y,
                  (DV_TIME_SIZE_CX-10),
                  DV_TIME_SIZE_CY);
      // Make string containing program's size (in bytes).
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
// ():
//    This function parses the original parameter string of this
// program object for special parameter directives and constructs the
// final string by using the dropped data filename (if any).
//    The special parameter directives are:
//       no parm : If the program object is started by clicking on it no
//                 parameters are passed.  If you start the program object by
//                 dragging a file over it, the full filename is passed.
//       [ ]     : Prompt user for actual parameter.
//       [text]  : String inside brackets is displayed as prompt string.
//       %       : No parameters are passed.  Useful for program objects you may
//                 want to start from a folders pop-up menu
//       %*      : Enables you to open a data file object to the program object
//                 and drop it.  Or, you can open a data file object that you
//                 associated to a program.
//       %**P    : Insert drive and path information without the last backslach
//       %**D    : Insert drive with ':'  or UNC name.
//       %**N    : Insert file name without extension.
//       %**F    : Insert file name with extension
//       %**E    : Insert extension without leading dot.  In HPFS, the
//               : extension always comes after the last dot.
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPPROGRAM():
//---------------------------------------------------------------------
WPPROGRAM::WPPROGRAM(void) : WPOBJECT()
{
   // Initialize program object.
   SetType(OBJTYPE_PROGRAM);
   SetProgramPath("");
   SetParameters("");
   SetWorkingDir("");
   m_wExecCounter = 0;
   // Set default minimized behavior to "minimized to viewer".
   SetFlagsOR(MINBEHAVIOR_MIN_TO_VIEWER,MINBEHAVIOR_FLAGS_MASK);
   SetFlagsOR(OPENBEHAVIOR_CREATE,OPENBEHAVIOR_FLAGS_MASK);
}

//---------------------------------------------------------------------
// WPPROGRAM():
//---------------------------------------------------------------------
WPPROGRAM::WPPROGRAM(WPPROGRAM *pwpprogDefaults) : WPOBJECT((WPOBJECT *)pwpprogDefaults)
{
   // Initialize program object.
   SetType(OBJTYPE_PROGRAM);
   SetProgramPath(pwpprogDefaults->GetProgramPath());
   SetParameters(pwpprogDefaults->GetParameters());
   SetWorkingDir(pwpprogDefaults->GetWorkingDir());
   m_wExecCounter = 0;
}


//---------------------------------------------------------------------
// PROGRAM INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Initialize():
//    This function gets called only once during the desktop
// initialization and its called by its parent container object so that
// this object can resume the state in which it was saved.
//    We then call all our children objects so they can do the same.
//---------------------------------------------------------------------
void WPPROGRAM::Initialize(BOOL fIsShell)
{
   if (isOpen()==TRUE)
   {
      // Reset state from "opened" to "closed" (so that Open() works correctly).
      SetState((GetState()&(~OBJSTATE_OPEN)));
      // Open program object.
      //Open();
   }

   // If object has a parent container.
   if ((GetParent()!=NULL)&&(fIsShell==TRUE))
   {
      // Check if parent folder had startup property AND "Shift" key is NOT pressed.
      if (((GetParent()->GetFlags()&STARTUP_FLAGS_MASK)==STARTUP_PROPERTY) &&
          ((GetAsyncKeyState(VK_SHIFT)&0x8000)!=0x8000))
         // Open object.
         Open();
   }
}

//---------------------------------------------------------------------
// PROGRAM PATH, PARAMETERS, AND WORKING DIRECTORY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// SetProgramPath():
//---------------------------------------------------------------------
void WPPROGRAM::SetProgramPath(const char *pszProgramPath)
{
   // Copy new program name into private data member.
   strncpy(m_szProgramPath,pszProgramPath,MAX_PROGRAM_PATH);
   // Make sure program path is null-terminated.
   m_szProgramPath[MAX_PROGRAM_PATH] = '\0';
}

//---------------------------------------------------------------------
// GetProgramPath():
//---------------------------------------------------------------------
const char *WPPROGRAM::GetProgramPath(void)
{
   return (m_szProgramPath);
}

//---------------------------------------------------------------------
// SetParameters():
//---------------------------------------------------------------------
void WPPROGRAM::SetParameters(const char *pszParameters)
{
   // Copy new program parameter string into private data member.
   strncpy(m_szParameters,pszParameters,MAX_PARAMETERS_LINE);
   // Make sure parameters line is null-terminated.
   m_szParameters[MAX_PARAMETERS_LINE] = '\0';

}

//---------------------------------------------------------------------
// GetParameters():
//---------------------------------------------------------------------
const char *WPPROGRAM::GetParameters(void)
{
   return (m_szParameters);
}

//---------------------------------------------------------------------
// SetWorkingDir():
//---------------------------------------------------------------------
void WPPROGRAM::SetWorkingDir(const char *pszWorkingDir)
{
   int index;

   // Copy new program working directory path string into private data member.
   strncpy(m_szWorkingDir,pszWorkingDir,MAX_WORKING_DIR);
   // Make sure path is null-terminated.
   m_szWorkingDir[MAX_WORKING_DIR] = '\0';
   // Make sure that working directory path is not terminated with backslash "\".
   // Initialize local variable with length of current working directory string.
   index = (strlen(m_szWorkingDir)-1);
   // While there are more characters to inspect...
   while (index>=0)
   {
      // Check if character is a white-space...
      if (IsSpace(m_szWorkingDir[index]))
         // Advance to previous character position.
         index--;
      else
      // Check if character is a backslash...
      if (m_szWorkingDir[index]=='\\')
      {
         // Check whether backslash is not from the root (the root is an exception)...
         if ((index==0)||(m_szWorkingDir[index-1]!=':'))
            // Remove backslash by null-terminating string at that position.
            m_szWorkingDir[index] = '\0';
         // Break out of while loop.
         break;
      }
      else
         // Since character is not any of the above, exit while loop.
         break;
   }
}

//---------------------------------------------------------------------
// GetWorkingDir():
//---------------------------------------------------------------------
const char *WPPROGRAM::GetWorkingDir(void)
{
   return (m_szWorkingDir);
}


//---------------------------------------------------------------------
// UpdateFileSystemObjectName():
//---------------------------------------------------------------------
void WPPROGRAM::UpdateFileSystemObjectName(char *szNewTitle)
{  
   char szFullPath[_MAX_PATH+1],szDrive[_MAX_DRIVE+1],
        szDir[_MAX_DIR+1],szName[_MAX_FNAME+1],szExt[_MAX_EXT+1];
        
   // Split the program's name from the current path.
   _splitpath(GetProgramPath(),szDrive,szDir,NULL,NULL);
   // Split extension from new file name.
   _splitpath(szNewTitle,NULL,NULL,szName,szExt);   
   // Put together the new program's name using original path.
   _makepath(szFullPath,szDrive,szDir,szName,szExt);
   // Change the program's name to the newly specified one.
   SetProgramPath(szFullPath);
}


//---------------------------------------------------------------------
// PROGRAM RESOURCES INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LoadFileInfo():
//---------------------------------------------------------------------
void WPPROGRAM::LoadFileInfo(void)
{
   UINT    uPrevErrorMode;

   // Turn-off the Windows 3.1 INT 24H error handling and "drive not ready" boxes.
   uPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

   // Try to get program file information...
   if (_stat(GetProgramPath(),&m_statFileInfo)==0)
      m_fValidFileInfo = TRUE;
   else
      m_fValidFileInfo = FALSE;

   // Restore original Windows 3.1 error mode before leaving.
   SetErrorMode(uPrevErrorMode);
}

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPPROGRAM::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_PROGRAM_DEFAULT_ICONINDEX;
}

//---------------------------------------------------------------------
// LoadIcon():
//---------------------------------------------------------------------
BOOL WPPROGRAM::LoadIcon(void)
{
   // Read file information (size, date, time, etc..).
   LoadFileInfo();
   // Read icon resource for program object.
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
void WPPROGRAM::GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue)
{
   char  szFName[_MAX_FNAME],szExt[_MAX_EXT];

   // Depending on sort key value requested...
   switch (wSortKey)
   {
      case IDM_SORTBYREALNAME:
         // Get object title string without special characters.
         // get program's real name and draw it on "name" column.
         _splitpath(GetProgramPath(),NULL,NULL,szFName,szExt);
         sprintf(psvsSortValue->m_szValue,"%s%-s",szFName,szExt);
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
// PROGRAM OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPPROGRAM::Open(void)
{
   // Call the open function with default parameter line.
   return (Open((char *)GetParameters()));
}


//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPPROGRAM::Open(char *pszParameterLine)
{
   UINT        uReturnValue;
   HWND        hwndProgram;
   HINSTANCE   hInstProgram;
   BOOL        fExists;
   int         nCmdShow;
   char        szErrorMsg[256];

   // If only one program object is allowed at a time...
   if ((GetFlags()&OPENBEHAVIOR_FLAGS_MASK)==OPENBEHAVIOR_DISPLAY)
   {
      // Get window handle from owner object.
      hwndProgram = (HWND)(WORD)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_GET_HWND_FROM_OBJECT,0,(LPARAM)this);
      // If a program copy already exists...
      if (IsWindow(hwndProgram)==TRUE)
      {
         // Give focus to already existing program instance...
         // If program window is minimized, restore to original size.
         if (IsIconic(hwndProgram)==TRUE)
            ShowWindow(hwndProgram,SW_RESTORE);
         // Set active focus to this folder.
         SetActiveWindow(hwndProgram);
         // Open function has been completed.
         return (TRUE);
      }
   }

   // If no parameter line passed, use program's default
   if (pszParameterLine==NULL)
      pszParameterLine = (char *)GetParameters();

   // Check if program window has to be started minimized...
   switch ((GetFlags()&START_PROG_MASK))
   {
      case START_PROG_NORMAL:
         nCmdShow = SW_SHOWNORMAL;     // Start normal.
         break;
         
      case START_PROG_MINIMIZED:
         nCmdShow = SW_SHOWMINIMIZED;  // Start minimized.
         break;
         
      case START_PROG_MAXIMIZED:
         nCmdShow = SW_SHOWMAXIMIZED;  // Start maximized
         break;
   }
      
   // Execute application.
   hInstProgram = ExecuteApplication(GetDesktopWindow(),"open",GetProgramPath(),pszParameterLine,GetWorkingDir(),nCmdShow);

   #ifdef _DEBUG
   dprintf("tr Open(): executed program instance 0x%04X\n",hInstProgram);
   #endif

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
               GetProgramPath(),
               apszWinExecErrorString[uReturnValue]);
            MessageBox(NULL,szErrorMsg,GetString(IDSTR_ERR_CAPTION),MB_OK|MB_ICONSTOP);
            return (FALSE);
      }
   }

   // Check that executable program instance is not the one already running.
   fExists = (BOOL)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_FIND_HINSTANCE,(WPARAM)hInstProgram,0L);

   // If it does not exists in Workplace Shell desktop list...
   if (fExists==FALSE)
   {
      // Set "opened" state bit.
      //SetState((GetState()|OBJSTATE_OPEN));

      // Increment execution counter.
      m_wExecCounter++;

      #ifdef _DEBUG
      dprintf("tr Open(): associate instance.\n");
      #endif

      // Let Workplace Shell desktop know that this object "owns" the newly executed instance of this program.
      PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_HINSTANCE_ASSOCIATE_OBJECT,(WPARAM)hInstProgram,(LPARAM)this);

      // Update visual state of program icon.
      InvalidateIcon(TRUE,TRUE);
   }
   #ifdef _DEBUG
      dprintf("tr Open(): instance already exists.\n");
      dprintf("tr Open(): exec count = %d.\n",m_wExecCounter);
   #endif

   // Executed program successfully.
   return (TRUE);
}


//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPPROGRAM::Close(void)
{
   // Decrement number of program copies running.
   if (m_wExecCounter>0)
      m_wExecCounter--;

   // If all copies of program are closed, then reset "opened" flag.
   if (m_wExecCounter==0)
   {
      // Reset "opened" state bit to "not-opened" state.
      SetState((GetState()&(~OBJSTATE_OPEN)));
      // Update visual state of program icon.
      InvalidateIcon(TRUE,TRUE);
   }

   #ifdef _DEBUG
      dprintf("tr Close(): exec count = %d.\n",m_wExecCounter);
   #endif

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
const char *WPPROGRAM::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPPROGRAM_SYSMENU_NAME));  // return the default generic object menu.
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPPROGRAM::MenuSelection(WORD wMenuItem)
{
   // Process menu selections from the program's system menu.
   switch (wMenuItem)
   {
      case IDM_OPENPROGRAM:
         // The "open program" function executes the program.
         Open();
         break;

      case IDM_HELPGENERAL:
         // Open help file with "General help" for program objects.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_PROGRAM);
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
BOOL WPPROGRAM::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // Check if object class accepts target object "pwpobjDropTarget".
   if (WPOBJECT::TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Check if program class has any problems with being a drop source
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
BOOL WPPROGRAM::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if object class accepts source object "pwpobjDropSource".
   if (WPOBJECT::TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Depending on what drag & drop function is being performed...
      switch (nFunction)
      {
         case DROP_MOVE:
            // If object being tested is a data file...
            if ((pwpobjDropSource->GetType()&OBJTYPE_DATAFILE)==OBJTYPE_DATAFILE)
               // We support any type of file drag & drop.
               return (TRUE);
            else
               return (FALSE);

         case DROP_COPY:
         case DROP_SHADOW:
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
BOOL WPPROGRAM::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Depending on what drag & drop function is being performed...
      switch (nFunction)
      {
         case DROP_MOVE:
            // If object being dropped into this program object is a data file...
            if ((pwpobjDropSource->GetType()&OBJTYPE_DATAFILE)==OBJTYPE_DATAFILE)
            {
               // Open program object using data file as parameter.
               Open((char *)((WPDATAFILE *)pwpobjDropSource)->GetFullDataPath());
            }
            // Source object not supported by program object.
            return (FALSE);

         case DROP_COPY:
         case DROP_SHADOW:
         default:
            // no other functions supported by program class.
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
//---------------------------------------------------------------------
void WPPROGRAM::InsertSettingsNotebookPages(void)
{
   // Insert the "Program" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_PROGRAM),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPPROGRAM::ProgramPageProc,NULL,MAKEINTRESOURCE(IDD_PROGRAM_PAGE));
   // Insert the "Association" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_ASSOCIATION),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPPROGRAM::AssociationPageProc,NULL,MAKEINTRESOURCE(IDD_ASSOCIATION_PAGE));
   // Insert the "Window" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_WINDOW),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPPROGRAM::WindowPageProc,NULL,MAKEINTRESOURCE(IDD_WINDOW_PAGE));
   // Insert the default object pages into the settings notebook.
   WPOBJECT::InsertSettingsNotebookPages();
}


//---------------------------------------------------------------------
// FindProgram():
//---------------------------------------------------------------------
BOOL WPPROGRAM::FindProgram(HWND hwndPage)
{
   char  szFile[_MAX_PATH+1],szFileTitle[(_MAX_FNAME+_MAX_EXT)+1],
         szFilter[] = "Programs|*.exe;*.pif;*.com;*.bat|"
                      "All Files (*.*)|*.*|";

   // Open "Run" dialog box, and select program to run.
   if (CommonFileDialogBox(hwndPage,(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST),GetString(IDSTR_FILEDLG_TITLE_FIND_PROGRAM),szFilter,szFile,szFileTitle)==TRUE)
   {
      // Store returned file path and name into program path string.
      SetProgramPath(szFile);
      // Copy file name and path into page entry field control.
      SetDlgItemText(hwndPage,IDE_PATHFILE,GetProgramPath());
      // Return sucessfully found file.
      return (TRUE);
   }
   else
      // Return failure since user didn't select a valid file.
      return (FALSE);
}

//---------------------------------------------------------------------
// ProgramPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPPROGRAM::ProgramPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   char  szTextLine[MAX_LINE+1];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize edit controls in page.
         SetDlgItemText(hwndPage,IDE_PATHFILE,GetProgramPath());
         SetDlgItemText(hwndPage,IDE_PARAMETERS,GetParameters());
         SetDlgItemText(hwndPage,IDE_WORKDIR,GetWorkingDir());
         // If program object is currently owned by a directory folder...
         if (GetParent()->isDirectoryContainer()==TRUE)
         {
            // Do not allow user to change file name or path from this page.
            SendDlgItemMessage(hwndPage,IDE_PATHFILE,EM_SETREADONLY,TRUE,0L);
            SendDlgItemMessage(hwndPage,IDE_PARAMETERS,EM_SETREADONLY,TRUE,0L);
            SendDlgItemMessage(hwndPage,IDE_WORKDIR,EM_SETREADONLY,TRUE,0L);
            // Disable the "Find..." button since we do not want the user to use it.
            EnableWindow(GetDlgItem(hwndPage,IDB_FIND),FALSE);
         }
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_FIND:
               // Let user "browse" for wanted program file by using the
               // common file dialog in Windows.
               if (FindProgram(hwndPage)==TRUE)
               {
                  // Force the file name selected with icon index 0 into
                  // icon resource associated with program object.
                  // Update owner object and its settings notebook.
                  UpdateIcon((char *)GetProgramPath(),0);
                  // If this object's settings notebook is open, update its icon
                  // resource to match the new icon associated with program object.
                  if (isSettingsNotebookOpen()==TRUE)
                     GetSettingsNotebook()->UpdateNotebookIconResource();
               }
               break;

            case IDB_UNDO:
               SetDlgItemText(hwndPage,IDE_PATHFILE,GetProgramPath());
               SetDlgItemText(hwndPage,IDE_PARAMETERS,GetParameters());
               SetDlgItemText(hwndPage,IDE_WORKDIR,GetWorkingDir());
               break;

            case IDB_HELP:
               // Open help file with "Program" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_PROGRAMPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Retrieve program information from 'Program' page entry fields.
         GetDlgItemText(hwndPage,IDE_PATHFILE,szTextLine,MAX_PROGRAM_PATH);
         SetProgramPath(szTextLine);
         GetDlgItemText(hwndPage,IDE_PARAMETERS,szTextLine,MAX_PARAMETERS_LINE);
         SetParameters(szTextLine);
         GetDlgItemText(hwndPage,IDE_WORKDIR,szTextLine,MAX_WORKING_DIR);
         SetWorkingDir(szTextLine);
         // Update file information (size, date, time, etc...).
         LoadFileInfo();
         break;
   }
}


//---------------------------------------------------------------------
// AssociationPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPPROGRAM::AssociationPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{              
   WORD wControlID; 

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize "Current Types" list box.
         SendDlgItemMessage(hwndPage,IDL_AVAILTYPES,LB_ADDSTRING,0,(LPARAM)(LPSTR)GetString(IDSTR_NONE_PARENTHESIS));
         SendDlgItemMessage(hwndPage,IDL_AVAILTYPES,LB_SETCURSEL,0,0L);
         // Initialize "Start program" radio button group depending on
         // the current value of the program object flag,...
         switch ((GetFlags()&START_PROG_MASK))
         {
            case START_PROG_NORMAL:
               wControlID = IDB_STARTNORMAL;     // Start normal.
               break;
               
            case START_PROG_MINIMIZED:
               wControlID = IDB_STARTMINIMIZED;  // Start minimized.
               break;
               
            case START_PROG_MAXIMIZED:
               wControlID = IDB_STARTMAXIMIZED;  // Start maximized
               break;
         }
         // Set corresponding radio button control depending on which program property is set.
         CheckRadioButton(hwndPage,IDB_STARTNORMAL,IDB_STARTMAXIMIZED,wControlID);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_UNDO:
               // Undo current "Start minimized" flag checkbox...
               switch ((GetFlags()&START_PROG_MASK))
               {
                  case START_PROG_NORMAL:
                     wControlID = IDB_STARTNORMAL;     // Start normal.
                     break;
                     
                  case START_PROG_MINIMIZED:
                     wControlID = IDB_STARTMINIMIZED;  // Start minimized.
                     break;
                     
                  case START_PROG_MAXIMIZED:
                     wControlID = IDB_STARTMAXIMIZED;  // Start maximized
                     break;
               }
               // Set corresponding radio button control depending on which program property is set.
               CheckRadioButton(hwndPage,IDB_STARTNORMAL,IDB_STARTMAXIMIZED,wControlID);
               break;

            case IDB_HELP:
               // Open help file with "Association" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_ASSOCIATIONPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Check if "Start normal" radio button is set...
         if (IsDlgButtonChecked(hwndPage,IDB_STARTNORMAL)==1)
            // Set "Start normal" flag in program object flags.
            SetFlagsOR(START_PROG_NORMAL,START_PROG_MASK);
         else          
         // Check if "Start minimized" radio button is set...
         if (IsDlgButtonChecked(hwndPage,IDB_STARTMINIMIZED)==1)
            // Set "Start minimzed" flag in program object flags.
            SetFlagsOR(START_PROG_MINIMIZED,START_PROG_MASK);
         else
            // Set "Start maximized" flag in program object flags.
            SetFlagsOR(START_PROG_MAXIMIZED,START_PROG_MASK);
         break;
   }
}


//---------------------------------------------------------------------
// GetWindowPageFlags():
//---------------------------------------------------------------------
DWORD WPPROGRAM::GetWindowPageFlags(HWND hwndPage)
{
   DWORD dwFlags=0L;

   // Button appearance.
   if (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1)
      dwFlags |= BUTTONAPPEARANCE_HIDE;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINIMIZEBUTTON)==1)
      dwFlags |= BUTTONAPPEARANCE_MINIMIZE;

   // Minimize button behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_MINHIDE)==1)
      dwFlags |= MINBEHAVIOR_HIDE;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINTOVIEWER)==1)
      dwFlags |= MINBEHAVIOR_MIN_TO_VIEWER;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINTODESKTOP)==1)
      dwFlags |= MINBEHAVIOR_MIN_TO_DESKTOP;

   // Object open behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_DISPLAY)==1)
      dwFlags |= OPENBEHAVIOR_DISPLAY;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_CREATE)==1)
      dwFlags |= OPENBEHAVIOR_CREATE;

   // Return queried object flags.
   return (dwFlags);
}


//---------------------------------------------------------------------
// UpdateWindowPageControls():
//---------------------------------------------------------------------
void WPPROGRAM::UpdateWindowPageControls(HWND hwndPage,DWORD dwFlags)
{
   // Check the minimize behavior radio button.
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_HIDE)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINHIDE);
   else
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_MIN_TO_VIEWER)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTOVIEWER);
   else
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_MIN_TO_DESKTOP)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTODESKTOP);

   // Check the object open behavior radio button.
   if ((dwFlags&OPENBEHAVIOR_FLAGS_MASK)==OPENBEHAVIOR_DISPLAY)
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_DISPLAY);
   else
   if ((dwFlags&OPENBEHAVIOR_FLAGS_MASK)==OPENBEHAVIOR_CREATE)
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_CREATE);
}


//---------------------------------------------------------------------
// WindowPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPPROGRAM::WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Disable radio buttons and groupbox that are not applicable to this object.
         EnableWindow(GetDlgItem(hwndPage,IDS_BUTTON_APPEARANCE_GRPBX),FALSE);
         EnableWindow(GetDlgItem(hwndPage,IDB_HIDEBUTTON),FALSE);
         EnableWindow(GetDlgItem(hwndPage,IDB_MINIMIZEBUTTON),FALSE);
         // Hide radio buttons and groupbox that are not applicable to this object.
         ShowWindow(GetDlgItem(hwndPage,IDS_BUTTON_APPEARANCE_GRPBX),SW_HIDE);
         ShowWindow(GetDlgItem(hwndPage,IDB_HIDEBUTTON),SW_HIDE);
         ShowWindow(GetDlgItem(hwndPage,IDB_MINIMIZEBUTTON),SW_HIDE);
         // Initialize radio buttons.
         UpdateWindowPageControls(hwndPage,GetFlags());
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Reset radio buttons to default folder object values.
               UpdateWindowPageControls(hwndPage,(MINBEHAVIOR_MIN_TO_VIEWER|OPENBEHAVIOR_CREATE));
               break;

            case IDB_UNDO:
               // Update radio buttons to reflect original object values.
               UpdateWindowPageControls(hwndPage,GetFlags());
               break;

            case IDB_HELP:
               // Open help file with "Window" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_WINDOWPAGE);
               break;

            case IDB_MINHIDE:
               if (IsDlgButtonChecked(hwndPage,IDB_MINHIDE)==1)
                  UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_HIDE));
               break;

            case IDB_MINTOVIEWER:
               if (IsDlgButtonChecked(hwndPage,IDB_MINTOVIEWER)==1)
                  UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_MIN_TO_VIEWER));
               break;

            case IDB_MINTODESKTOP:
               if (IsDlgButtonChecked(hwndPage,IDB_MINTODESKTOP)==1)
                  UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_MIN_TO_DESKTOP));
               break;

            case IDB_DISPLAY:
               UpdateWindowPageControls(hwndPage,(OPENBEHAVIOR_DISPLAY));
               break;

            case IDB_CREATE:
               UpdateWindowPageControls(hwndPage,(OPENBEHAVIOR_CREATE));
               break;
         }
         break;

      case PAGE_DONE:
         SetFlagsOR(GetWindowPageFlags(hwndPage),
                    (MINBEHAVIOR_FLAGS_MASK|OPENBEHAVIOR_FLAGS_MASK));
         break;
   }
}

//---------------------------------------------------------------------
// OBJECT TO FILE SYSTEM COPYING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CopyObjectToFileSystem():
//---------------------------------------------------------------------
BOOL WPPROGRAM::CopyObjectToFileSystem(char *pszDestPath)
{
   char szDest[_MAX_PATH+1],szName[_MAX_FNAME+1],szExt[_MAX_EXT+1];

   _splitpath(GetProgramPath(),NULL,NULL,szName,szExt);
   sprintf(szDest,"%s\\%s%s",pszDestPath,szName,szExt);
   return (CopyFile((char *)GetProgramPath(),szDest));
}

//---------------------------------------------------------------------
// DeleteObjectFromFileSystem():
//---------------------------------------------------------------------
BOOL WPPROGRAM::DeleteObjectFromFileSystem(void)
{
   return (DeleteFile((char *)GetProgramPath()));
}



//---------------------------------------------------------------------
// WORKPLACE SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPPROGRAM::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_PROGRAM_STRUCT   fpsData;

   // Object information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read program class information.
   if (read(hProfile,&fpsData,sizeof(FILE_PROGRAM_STRUCT))!=sizeof(FILE_PROGRAM_STRUCT))
      return (FALSE);

   // Initialize program object values using data values read from profile.
   SetProgramPath(fpsData.m_szProgramPath);
   SetParameters(fpsData.m_szParameters);
   SetWorkingDir(fpsData.m_szWorkingDir);

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPPROGRAM::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_PROGRAM_STRUCT   fpsData;

   // Write out object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Initialize program data structure with program values.
   strcpy(fpsData.m_szProgramPath,GetProgramPath());
   strcpy(fpsData.m_szParameters,GetParameters());
   strcpy(fpsData.m_szWorkingDir,GetWorkingDir());

   // Write program class information.
   if (write(hProfile,&fpsData,sizeof(FILE_PROGRAM_STRUCT))!=sizeof(FILE_PROGRAM_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPPROG.CPP

