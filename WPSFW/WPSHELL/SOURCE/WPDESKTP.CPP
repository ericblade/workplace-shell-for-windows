//=====================================================================
// WPDESKTOP.CPP
//    WPDESKTOP Class implementation.
//
// Date Created :  01/26/94
// Last Update  :  10/08/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
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
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
// Windows library header files.
#include <windowsx.h>
#include <shellapi.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"

//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
LRESULT  CALLBACK _loadds  ShellHookProc(int iCode,WPARAM wParam,LPARAM lParam);
void                       DestroyRecursively(WPOBJECT *pwpobjDestroy);
HINSTANCE                  ExecuteApplication(HWND hwndParent,LPCSTR pszOp,LPCSTR pszFile,LPCSTR pszParams,LPCSTR pszDir,int nCmdShow);
LRESULT  CALLBACK          WPClientWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
// From DESKTOP.CPP file:
LRESULT  CALLBACK _loadds  WPDESKTOPSubClassWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL     CALLBACK          WPDESKTOPEnumWindowsProc(HWND hWnd,LPARAM lParam);
BOOL     CALLBACK          RunDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL     CALLBACK          ProductInfoDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// EXTERNAL GLOBAL VARIABLES
//---------------------------------------------------------------------
// Defined in WPPROG.CPP file:
extern char *apszWinExecErrorString[32];


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Desktop object default values.
#define WPS_DESKTOP_DEFAULT_ICONINDEX  ICONINDEX(IDI_DESKTOP)
// Desktop window styles.
#define WPDESKTOP_DEFAULT_EX_STYLES    (WS_EX_TRANSPARENT|WS_EX_ACCEPTFILES)
#define WPDESKTOP_DEFAULT_STYLES       (WS_POPUP)


//---------------------------------------------------------------------
// WORKPLACE SHELL DESKTOP CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SHELL HOOK MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// RegisterShellHook():
//---------------------------------------------------------------------
void WPDESKTOP::RegisterShellHook(void)
{
   #if ENABLE_SHELLHOOK
   // Install shell-application window hook.
   m_lpfnShellHookProc = MakeProcInstance((FARPROC)ShellHookProc,g_wpEnvironment.hInst);
   m_hhookShell = SetWindowsHookEx(WH_SHELL,(HOOKPROC)m_lpfnShellHookProc,g_wpEnvironment.hInst,NULL);
   #endif
}

//---------------------------------------------------------------------
// UnregisterShellHook():
//---------------------------------------------------------------------
void WPDESKTOP::UnregisterShellHook(void)
{
   #if ENABLE_SHELLHOOK
   // Unhook shell windows hook.
   UnhookWindowsHookEx(m_hhookShell);
   // Free procedure instance created for hook.
   FreeProcInstance(m_lpfnShellHookProc);
   #endif
}


//---------------------------------------------------------------------
// INTERNAL TOP-LEVEL WINDOW ARRAY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// AddTopLevelWindow():
//---------------------------------------------------------------------
BOOL WPDESKTOP::AddTopLevelWindow(HWND hwndTopLevel)
{
   // Make sure we can add another window handle to the internal list.
   if (m_nTopLevelWindowCount<MAX_TOP_LEVEL_WINDOWS)
   {
      // Add window information to end of list.
      m_TopLevel[m_nTopLevelWindowCount].m_hInstance    = (HINSTANCE)GetWindowWord(hwndTopLevel,GWW_HINSTANCE);
      m_TopLevel[m_nTopLevelWindowCount].m_hwnd         = hwndTopLevel;
      m_TopLevel[m_nTopLevelWindowCount].m_wMinBehavior = g_wpEnvironment.stSystem.wMinButtonBehavior;
      m_TopLevel[m_nTopLevelWindowCount].m_pwpobjOwner  = NULL;
      // Increment top-level window count.
      m_nTopLevelWindowCount++;
      // Successfully added window handle to list.
      return (TRUE);
   }
   else
      return (FALSE);   // No space available in list.
}

//---------------------------------------------------------------------
// DeleteTopLevelWindow():
//---------------------------------------------------------------------
BOOL WPDESKTOP::DeleteTopLevelWindow(HWND hwndTopLevel)
{
   int index,nDeleteIndex;

   // Set window handle index to undetermined.
   nDeleteIndex = -1;

   // First, search list for window handle location.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If window handle found in list, store index and exit "for loop".
      if (m_TopLevel[index].m_hwnd==hwndTopLevel)
      {
         nDeleteIndex = index;
         break;
      }
   }

   // If window handle not found in list, exit with failure return value.
   if (nDeleteIndex==-1)
      return (FALSE);

   // If we have a valid object pointer for window, notify owner before deleting.
   if (m_TopLevel[nDeleteIndex].m_pwpobjOwner!=NULL)
   {
      // Close application instance.
      m_TopLevel[nDeleteIndex].m_pwpobjOwner->Close();
   }

   // Shrink array elements to the right of deleted window handle.
   for (index=nDeleteIndex; index<m_nTopLevelWindowCount; index++)
      // Copy window handle to the right of current element.
      m_TopLevel[index] = m_TopLevel[index+1];

   // Decrement the number of top-level windows in list.
   m_nTopLevelWindowCount--;

   // Successfully deleted window handle from internal list.
   return (TRUE);
}


//---------------------------------------------------------------------
// SHELL INITIALIZATION FUNCTIONS (FOR PROGMAN FUNCTIONAL COMPATIBILITY)
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ShellLoadAndRun():
//    This function handles the loading and running of any programs
// specified in the "load=" and "run=" entries in the WIN.INI file.
// The section name within the profile is "Windows", and multiple
// program can be separated by blank spaces or tabs.  No parameters
// are allowed to be specified for this programs.
//---------------------------------------------------------------------
void WPDESKTOP::ShellLoadAndRun(void)
{
   UINT  uReturnCode;
   char  szBuffer[256],*pszExecute;

   // Get string in "load=" entry in WIN.INI file.
   GetProfileString("windows","load","",szBuffer,sizeof(szBuffer));
   // Initialize first application to execute.
   pszExecute = strtok(szBuffer," \t");
   // Walk the list and run the applications listed (Windows 3.0 compatibility).
   while (pszExecute)
   {
      #ifdef _DEBUG
      dprintf("tr ShellLoadAndRun(): load '%s'\n",pszExecute);
      #endif
      // Execute current application in list.
      uReturnCode = (UINT)ExecuteApplication(GetDesktopWindow(),"open",pszExecute,"","",SW_SHOWMINIMIZED);
      #ifdef _DEBUG
      // Check that load was successful.
      if (uReturnCode<32)
         dprintf("err ShellLoadAndRun(): error loading '%s' = %u\n",pszExecute,uReturnCode);
      #endif
      // Get next application in list.
      pszExecute = strtok(NULL," \t");
   }

   // Get string in "run=" entry in WIN.INI file.
   GetProfileString("windows","run","",szBuffer,sizeof(szBuffer));
   // Initialize first application to execute.
   pszExecute = strtok(szBuffer," \t");
   // Walk the list and run the applications listed (Windows 3.0 compatibility).
   while (pszExecute)
   {
      #ifdef _DEBUG
      dprintf("tr ShellLoadAndRun(): run '%s'\n",pszExecute);
      #endif
      // Execute current application in list.
      uReturnCode = (UINT)ExecuteApplication(GetDesktopWindow(),"open",pszExecute,"","",SW_SHOWNORMAL);
      #ifdef _DEBUG
      // Check that run was successful.
      if (uReturnCode<32)
         dprintf("err ShellLoadAndRun(): error running '%s' = %u\n",pszExecute,uReturnCode);
      #endif
      // Get next application in list.
      pszExecute = strtok(NULL," \t");
   }
}

//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECT PROFILE SUPPORT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// EnumerateWPSObjects():
//---------------------------------------------------------------------
void WPDESKTOP::EnumerateWPSObjects(WPFOLDER *pwpfolderParent,WORD *pwCurrentID)
{
   int index;

   // This support is necessary for reading and writting shadow
   // object links correctly.
   // Assign unqiue ID to current folder object.
   pwpfolderParent->SetUniqueID((*pwCurrentID)++);
   // Traverse children objects in a depth-first search method
   // and assign a unique ID to each.
   for (index=0; index<pwpfolderParent->GetObjectCount(); index++)
   {
      // If current child object is of the "folder" type...
      if ((pwpfolderParent->GetObject(index)->GetType()&OBJTYPE_FOLDER)==OBJTYPE_FOLDER)
         // Assign unique ID's to all of the children in the current folder object.
         EnumerateWPSObjects((WPFOLDER *)(pwpfolderParent->GetObject(index)),pwCurrentID);
      else
         // Assign unique ID to current child object and increment ID counter.
         pwpfolderParent->GetObject(index)->SetUniqueID((*pwCurrentID)++);
   }
}


//---------------------------------------------------------------------
// FindUniqueID():
//---------------------------------------------------------------------
WPOBJECT *WPDESKTOP::FindUniqueID(WPOBJECT *pwpobjRoot,WORD wUniqueID)
{
   WPCONTAINER *pwpcontainerParent;
   WPOBJECT    *pwpobjMatch;
   int         index;

   // If current object matches the search ID...
   if (pwpobjRoot->GetUniqueID()==wUniqueID)
      // Return current object's pointer.
      return (pwpobjRoot);

   // If object is a container object,
   if ((pwpobjRoot->GetType()&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)
   {
      // Cast current object's pointer to a container object (for convenience of use).
      pwpcontainerParent = (WPCONTAINER *)pwpobjRoot;
      // Search all of its children.
      for (index=0; index<pwpcontainerParent->GetObjectCount(); index++)
      {
         // Search child object (and possibly its children if it is a container object)...
         pwpobjMatch = FindUniqueID(pwpcontainerParent->GetObject(index),wUniqueID);
         // If a perfect macth was found of ID being searched for,...
         if (pwpobjMatch!=NULL)
            // Then, return pointer to object found.
            return (pwpobjMatch);
      }
   }

   // No match found within current object (or its children).
   return (NULL);
}


//---------------------------------------------------------------------
// BindShadowObjectsToOriginalObjects():
//---------------------------------------------------------------------
BOOL WPDESKTOP::BindShadowObjectsToOriginalObjects(void)
{
   MATCH_RECORD   *pMatchList,*pMatchRecord;
   WPOBJECT       *pwpobjOriginal;
   WORD           wOriginalObjectUniqueID;

   // Initialize match list pointer.
   pMatchList = NULL;
   // Build list of all shadow objects in desktop.
   Find(OBJTYPE_SHADOW,"",FALSE,FALSE,&pMatchList);

   // Search the desktop for any shadow objects and bind their links to the
   // original object which they represent.  The way we shadows to their original
   // objects is by using their unique IDs as a reference link.

   // Initialize pointer to match record to point to beginning of list...
   pMatchRecord = pMatchList;
   // While there is a valid match record unprocessed...
   while (pMatchRecord!=NULL)
   {
      // Get original object's unique ID (it is stored and read from the WPS profile).
      wOriginalObjectUniqueID = ((WPSHADOW *)pMatchRecord->m_pwpobjMatch)->GetOriginalUniqueID();
      // Find original object by using the unique ID retrieved.
      pwpobjOriginal = FindUniqueID(this,wOriginalObjectUniqueID);
      // Check that returned pointer is valid...
      if (pwpobjOriginal!=NULL)
      {
         // Add shadow link to original object's list of shadow links.
         pwpobjOriginal->AddShadow((WPSHADOW *)(pMatchRecord->m_pwpobjMatch));
         // Store pointer to original object.
         ((WPSHADOW *)(pMatchRecord->m_pwpobjMatch))->SetOriginalObject(pwpobjOriginal);
      }
      else
         // Return "error binding shadow".
         return (FALSE);

      // Get next match record in list...
      pMatchRecord = pMatchRecord->m_pNextRecord;
   }

   // Free all memory used for match records...
   while (pMatchList!=NULL)
   {
      // Save pointer to current match record.
      pMatchRecord = pMatchList;
      // Get pointer to next record in match list.
      pMatchList   = pMatchList->m_pNextRecord;
      // Deallocate memory used for match record.
      delete pMatchRecord;
   }

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WINDOW MINIMZED PLACEMENT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// UpdateMinPlacement():
//    this function updates the minimized icon window position for the
// specified window.  It returns TRUE if wanted position was different
// from original minimized position.  It will return FALSE if wanted
// position and original position are the same.
//---------------------------------------------------------------------
void WPDESKTOP::UpdateMinPlacement(HWND hWnd,WORD wMinBehavior)
{
   WINDOWPLACEMENT   wndplApp;

   // Get current window placement.
   wndplApp.length  = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(hWnd,&wndplApp);
   wndplApp.flags   = (WPF_SETMINPOSITION);
   
   #ifdef _DEBUG
      dprintf("tr UpdateMinPlacement(): CP#1 hWnd = 0x%04X min pos (%d,%d)\n",
         (UINT)hWnd,wndplApp.ptMinPosition.x,wndplApp.ptMinPosition.y);
   #endif
                     
   // Depending on what the behavior for the minimized button is...
   switch (wMinBehavior)
   {
      case MINBEHAVIOR_HIDE:
      case MINBEHAVIOR_MIN_TO_VIEWER:
         // Modify window placement for minimized position.
         // We set the new min pos to be off-screen so that the user has
         // the impression that it is "hidden".
         wndplApp.ptMinPosition.x = GetSystemMetrics(SM_CXSCREEN);
         wndplApp.ptMinPosition.y = GetSystemMetrics(SM_CYSCREEN);
         break;

      case MINBEHAVIOR_MIN_TO_DESKTOP:
         // Modify window placement for minimized position.
         // We set the new min pos to be equal to (-1,-1) so that Windows 3.1
         // will display them on the desktop and arrange them automatically.
         wndplApp.ptMinPosition.x = -1;
         wndplApp.ptMinPosition.y = -1;
         break;

      default:
         // Invalid minimized behavior, return immediatedly.
         return;
   }

   #ifdef _DEBUG
      dprintf("tr UpdateMinPlacement(): CP#2 hWnd = 0x%04X min pos (%d,%d)\n",
         (UINT)hWnd,wndplApp.ptMinPosition.x,wndplApp.ptMinPosition.y);
   #endif

   // Check if window is currently hidden and set flags accordingly.
   // NOTE: This is a fix implemented to solve the problem where
   //       the Windows API function SetWindowPlacement() always makes
   //       the window visible no matter what the current state is.
   //       If the window was hidden, we make sure it stays hidden while
   //       updating its minimized window position with SetWindowPlacement().
   if (IsWindowVisible(hWnd)==FALSE)
      wndplApp.showCmd = SW_HIDE;

   // Set new minimized window placement.
   SetWindowPlacement(hWnd,&wndplApp);
}


//---------------------------------------------------------------------
// OnLeftMouseButtonDblClk():
//---------------------------------------------------------------------
void WPDESKTOP::OnLeftMouseButtonDblClk(POINT ptDevice)
{
   WPOBJECT *pwpobjHit;

   // Get child object "hit" by left mouse double click.
   pwpobjHit = ChildHitTest(ptDevice);
   // Check if no child objects were "hit".
   if (pwpobjHit!=NULL)
      pwpobjHit->Open();   // "Open" child object hit.
   else
      MessageBeep(-1);     // Error beep, no objects hit.
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPDESKTOP():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPDESKTOP::WPDESKTOP(void) : WPFOLDER()
{
   // Initialize default values.
   SetType(OBJTYPE_DESKTOP);
   m_wndprocWindows31Desktop = NULL;
   m_hhookShell              = NULL;
   m_lpfnShellHookProc       = NULL;
   // Store the background color of desktop by querying whatever
   // the current desktop background color selected is.
   m_clrrefBackground = GetSysColor(COLOR_BACKGROUND);
   // Set icon position to be the middle of the screen.
   SetIconPos((GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON))/2,
              (GetSystemMetrics(SM_CYSCREEN)-GetSystemMetrics(SM_CYICON))/2);
   // Set desktop window to cover the whole screen.
   SetWndPos(0,0);
   SetWndSize(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
   // No scrollbars supported for desktop.
   SetScrollProperty(FALSE);
   m_nTopLevelWindowCount = 0;
   // Set desktop icon.
   SetIcon("",0);
}


//---------------------------------------------------------------------
// APPEARANCE ATTRIBUTES SET & QUERY FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetBackgroundColor():
//    This function stores a color reference as the current background
// color for desktop.  See GetBackgroundColor() call for more info.
//---------------------------------------------------------------------
void WPDESKTOP::SetBackgroundColor(COLORREF clrrefBackground)
{
   int         aiColorID[1];
   COLORREF    aclrrefColorRGB[1];
   char        szRGB[15];

   // Store new color reference value.
   m_clrrefBackground = clrrefBackground;
   // Change COLOR_BACKGROUND system color.
   // (This change is not saved in WIN.INI when Windows exists)
   aiColorID[0]       = COLOR_BACKGROUND;
   aclrrefColorRGB[0] = clrrefBackground;
   SetSysColors(1,aiColorID,aclrrefColorRGB);
   // Create string with RGB values of background color.
   sprintf(szRGB,"%u %u %u",GetRValue(clrrefBackground),GetGValue(clrrefBackground),GetBValue(clrrefBackground));
   // Make permanent changes to background color entry in WIN.INI file.
   WriteProfileString("colors","Background",szRGB);
}

//---------------------------------------------------------------------
// GetBackgroundColor():
//    This function returns the color reference value of the folder's
// client area background.  This value is of the type COLORREF which
// is actually an RGB triplet with a 0,1, or 2 in the MSB.  This 0,1,
// or 2 determines what type of a color is produced by Windows GDI when
//  a brush is created (please refer to Windows 3.1 SDK docs for info).
//---------------------------------------------------------------------
COLORREF WPDESKTOP::GetBackgroundColor(void)
{
   return (m_clrrefBackground);
}

//---------------------------------------------------------------------
// GetImageAttribute():
//---------------------------------------------------------------------
BOOL WPDESKTOP::GetImageAttribute(DWORD dwAttributeFlag)
{
   BOOL fTiled;
   char szWallpaper[MAX_PATH_FILENAME+1];

   // Check if we want to know if desktop does not have a wallpaper bitmap...
   if (dwAttributeFlag==IMAGE_COLOR_ONLY)
   {
      // Query WIN.INI file key entry
      GetProfileString("Desktop","Wallpaper",GetString(IDSTR_NONE_PARENTHESIS),szWallpaper,(sizeof(szWallpaper)-1));
      return ((stristr(szWallpaper,(const char*)GetString(IDSTR_NONE_PARENTHESIS))!=NULL)? TRUE : FALSE);
   }

   // Query whether wallpaper is tiled or not.
   fTiled = ((GetProfileInt("Desktop","TileWallPaper",0)==1)? TRUE : FALSE);
   // Depending on which attribute is being queried...
   switch (dwAttributeFlag)
   {
      case IMAGE_NORMAL:
         return ((fTiled==TRUE)? FALSE : TRUE);

      case IMAGE_TILED:
         return (fTiled);

      default:
         return (FALSE);
   }
}


//---------------------------------------------------------------------
// SetImageAttribute():
//---------------------------------------------------------------------
void WPDESKTOP::SetImageAttribute(DWORD dwAttributeFlag)
{
   char szValue[2];

   // Depending on which attribute is passed...
   switch (dwAttributeFlag)
   {
      // Check if we want to set the "Color only" attribute (no wallpaper bitmap)...
      case IMAGE_COLOR_ONLY:
         // Set "Wallpaper" key value to "(None)".
         SystemParametersInfo(SPI_SETDESKWALLPAPER,0,(LPVOID)GetString(IDSTR_NONE_PARENTHESIS),(SPIF_UPDATEINIFILE|SPIF_SENDWININICHANGE));
         break;

      case IMAGE_NORMAL:
      case IMAGE_SCALED:
         // Write value of 0 as string.
         strcpy(szValue,"0");
         // Set "no-tile" attribute for desktop wallpaper.
         WriteProfileString("Desktop","TileWallPaper",szValue);
         // Broadcast WM_WININICHANGE message to all top-level windows.
         PostMessage(HWND_BROADCAST,WM_WININICHANGE,0,0L);
         break;

      case IMAGE_TILED:
         // Write value of 1 as string.
         strcpy(szValue,"1");
         // Set "tile" attribute for desktop wallpaper.
         WriteProfileString("Desktop","TileWallPaper",szValue);
         // Broadcast WM_WININICHANGE message to all top-level windows.
         PostMessage(HWND_BROADCAST,WM_WININICHANGE,0,0L);
         break;
   }
}


//---------------------------------------------------------------------
// GetImagePath():
//---------------------------------------------------------------------
char *WPDESKTOP::GetImagePath(void)
{
   static char szValue[MAX_PATH_FILENAME+1];    // Must be static since we return pointer.

   // Read current desktop wallpaper image from WIN.INI file.
   GetProfileString("Desktop","Wallpaper",GetString(IDSTR_NONE_PARENTHESIS),szValue,MAX_PATH_FILENAME);
   // Return the current desktop wallpaper bitmap image file path.
   return (szValue);
}

//---------------------------------------------------------------------
// SetImagePath():
//---------------------------------------------------------------------
void WPDESKTOP::SetImagePath(char *pszImagePath)
{
   // Change current desktop wallpaper to be the specified image.
   SystemParametersInfo(SPI_SETDESKWALLPAPER,0,(LPVOID)pszImagePath,(SPIF_UPDATEINIFILE|SPIF_SENDWININICHANGE));
}



//---------------------------------------------------------------------
// FOLDER CAPTION TEXT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetCaptionTitle():
//    This function overloads the folder GetCaptionTitle() function
// because we do not want a title set for the desktop window.  The
// reason is because Windows 3.1 positions windows in the z-order
// differently when they have titles than when they don't.  If desktop
// window has a title, it'll be brought to the top when selected.  We
// do not want that behavior, therefore, we make sure no title is given
// to the desktop window.
//---------------------------------------------------------------------
const char *WPDESKTOP::GetCaptionTitle(void)
{
   // Return empty caption text string.
   return (NULL);
}

//---------------------------------------------------------------------
// DESKTOP DEFAULT VALUES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPDESKTOP::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_DESKTOP_DEFAULT_ICONINDEX;
}


//---------------------------------------------------------------------
// DESKTOP STATE INITIALIZATION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetWindowExStyles():
//---------------------------------------------------------------------
DWORD WPDESKTOP::GetWindowExStyles(void)
{
   return (WPDESKTOP_DEFAULT_EX_STYLES);
}

//---------------------------------------------------------------------
// GetWindowStyles():
//---------------------------------------------------------------------
DWORD WPDESKTOP::GetWindowStyles(void)
{
   return (WPDESKTOP_DEFAULT_STYLES);
}

//---------------------------------------------------------------------
// Open():
//    This function creates a desktop window and reads displays its
// components.
//---------------------------------------------------------------------
BOOL WPDESKTOP::Open(void)
{
   int  index;
   char *pszClassName;

   // If desktop is not already open
   // (only one desktop instance is allowed open at any time)...
   if (isOpen()==FALSE)
   {
      // Create unique class for this folder instance.
      pszClassName = RegisterFolderClass(WPClientWindowProcWrapper);
      if (pszClassName!=NULL)
      {
         // Create desktop transparent window.
         if (CreateWindow(pszClassName,this)==FALSE)
         {
            // Unregister desktop class before exiting with an error.
            UnregisterFolderClass();
            // Return failure.
            return (FALSE);
         }

         // Set "opened" state bit.
         SetState((GetState()|OBJSTATE_OPEN));

         // Show and update Workplace Shell desktop.
         ShowWindow(GetHandle(),SW_SHOW);
         ::UpdateWindow(GetHandle());

         // Invalidate for redraw all the top-level desktop objects.
         for (index=0; index<GetObjectCount(); index++)
            GetObject(index)->InvalidateIcon(FALSE,TRUE);

         // Update Windows 3.1 desktop.
         UpdateWindow();

         // Register DDE services supported by the Workplace Shell for Windows shell.
         m_ddeserverPROGMAN.Initialize(this);   // Program Manager (PROGMAN) Service.
         m_ddeserverSHELL.Initialize(this);     // DOS Programs (WINOLDAPP) Service.
      }
      else
         // Return failure.
         return (FALSE);
   }

   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// InitializeDesktop():
//---------------------------------------------------------------------
BOOL WPDESKTOP::InitializeDesktop(void)
{
   HCURSOR  hcurOriginal;
   BOOL     fResult,fIsShell;
   int      index;

   // Initialize test if Workplace Shell is current default Windows shell.
   fIsShell = IsShell(TRUE);
   // Initialize resturn result to success.
   fResult = TRUE;

   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));

   // Read last desktop state from profile in windows directory.
   if (ReadDesktop()==TRUE)
   {
      // Perform initializations that apply to this specific instance.
      if (Open()==TRUE)
      {
         // Initialize all the children objects...
         for (index=0; index<GetObjectCount(); index++)
            GetObject(index)->Initialize(fIsShell);

         // Load/run any applications in the "load=" and "run=" entries in the WIN.INI
         // [windows] section -- ONLY if we are the current default Windows shell.
         if (fIsShell==TRUE)
            ShellLoadAndRun();
      }
      else
         // Failed opening desktop, set return result to failure.
         fResult = FALSE;
   }
   else
      // Failed reading desktop, set return result to failure.
      fResult = FALSE;

   // Check if original cursor was not the same as our "wait" cursor...
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));

   // Return result.
   return (fResult);
}


//---------------------------------------------------------------------
// ReadWPObject():
//---------------------------------------------------------------------
BOOL ReadWPObject(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   DWORD    dwType;
   WPOBJECT *pwpobjNew;

   // Read type of object from file record.
   read(hProfile,&dwType,sizeof(DWORD));
   // Reposition object pointer to begining of record.
   lseek(hProfile,(-1L*sizeof(DWORD)),SEEK_CUR);

   // Create object instance depending on the type...
   switch ((DWORD)dwType)
   {
      case OBJTYPE_PROGRAM:      // Program object.
         pwpobjNew = new WPPROGRAM;
         break;

      case OBJTYPE_FOLDER:       // Folder object.
         pwpobjNew = new WPFOLDER;
         break;

      case OBJTYPE_DATAFILE:     // Data File object.
         pwpobjNew = new WPDATAFILE;
         break;

      case OBJTYPE_TEMPLATE:     // Template object.
         pwpobjNew = new WPTEMPLATE;
         break;

      case OBJTYPE_SHREDDER:     // Shredder object.
         pwpobjNew = new WPSHREDDER;
         break;

      case OBJTYPE_SYSTEMSETUP:  // System Setup object.
         pwpobjNew = new WPSYSSETUP;
         break;

      case OBJTYPE_DRIVE:        // Drive object.
         pwpobjNew = new WPDRIVE;
         break;

      case OBJTYPE_DIRECTORY:    // Directory object.
         pwpobjNew = new WPDIRECTORY;
         break;

      //case OBJTYPE_PRINTER:      // Printer object.
         //pwpobjNew = new WPPRINTER;
      //   break;

      case OBJTYPE_MINVIEWER:    // Minimized Window Viewer object.
         pwpobjNew = new WPMINVIEWER;
         break;

      case OBJTYPE_SHADOW:       // Shadow object.
         pwpobjNew = new WPSHADOW;
         break;

      default:
         // No other objects supported, error.
         return (FALSE);
   }

   // Initialize object values and return.
   return (pwpobjNew->ReadObjectProfile(hProfile,pwpcontainerParent));
}


//---------------------------------------------------------------------
// ReadDesktop():
//---------------------------------------------------------------------
BOOL WPDESKTOP::ReadDesktop(void)
{
   WPSHELL_PROFILE_HEADER  wpsProfileHeader;
   int                     hProfile;
   char                    szWindowsPath[MAX_PATH_FILENAME+1];

   // Get current windows directory.
   GetWindowsDirectory(szWindowsPath,MAX_PATH_FILENAME);
   strcat(szWindowsPath,"\\");
   strcat(szWindowsPath,GetString(IDSTR_DESKTOP_FILENAME));

   // Open WPSHELL.INI file.
   hProfile = open(szWindowsPath,(_O_BINARY|_O_RDONLY));
   // If open failed, exit immediately.
   if (hProfile==-1)
      return (FALSE);

   // Read WPS profile header information.
   if (read(hProfile,&wpsProfileHeader,sizeof(WPSHELL_PROFILE_HEADER))!=sizeof(WPSHELL_PROFILE_HEADER))
   {
      // Close profile.
      close(hProfile);
      // Return failure.
      return (FALSE);
   }

   // Check that file is a valid Workplace Shell v1.x profile.
   if ((wpsProfileHeader.m_dwMagicID!=WPSHELL_PROFILE_MAGIC_NUMBER) ||
       (!((wpsProfileHeader.m_wMajorVersion>=0x0001) &&
         (wpsProfileHeader.m_wMinorVersion>=0x0000))))
   {
      // Close profile.
      close(hProfile);
      // Return failure.
      return (FALSE);
   }

   // Move file pointer to begining of data.
   if (lseek(hProfile,wpsProfileHeader.m_dwDataOffset,SEEK_SET)==-1)
   {
      // Close profile.
      close(hProfile);
      // Return failure.
      return (FALSE);
   }

   // Read desktop information, children objects, and custom settings.
   if (ReadObjectProfile(hProfile,NULL)==FALSE)
   {
      // Close profile.
      close(hProfile);
      // Return failure.
      return (FALSE);
   }

   // Close profile.
   close(hProfile);

   // Bind shadow object's links now that we have read all objects (using unique IDs).
   return (BindShadowObjectsToOriginalObjects());
}


//---------------------------------------------------------------------
// SaveDesktop():
//---------------------------------------------------------------------
BOOL WPDESKTOP::SaveDesktop(void)
{
   HCURSOR                 hcurOriginal;
   WORD                    wCurrentID;
   WPSHELL_PROFILE_HEADER  wpsProfileHeader;
   int                     hProfile;
   char                    szWindowsPath[MAX_PATH_FILENAME+1];
      
   #ifdef _DEBUG
   dprintf("tr SaveDesktop(): saving desktop\n");
   #endif      
      
   // Get current windows directory.
   GetWindowsDirectory(szWindowsPath,MAX_PATH_FILENAME);
   strcat(szWindowsPath,"\\");
   strcat(szWindowsPath,GetString(IDSTR_DESKTOP_FILENAME));

   // Open WPSHELL.INI file.
   hProfile = open(szWindowsPath,_O_BINARY|_O_CREAT|_O_TRUNC|_O_RDWR);
   if (hProfile==-1)
      return (FALSE);

   // Initialize Workplace Shell profile header information.
   // Store Workplace Shell profile magic ID number.
   wpsProfileHeader.m_dwMagicID     = WPSHELL_PROFILE_MAGIC_NUMBER;
   wpsProfileHeader.m_wMajorVersion = VER_MAJOR;
   wpsProfileHeader.m_wMinorVersion = VER_MINOR;
   // Store file offset to begining of data.
   wpsProfileHeader.m_dwDataOffset = sizeof(WPSHELL_PROFILE_HEADER);

   // Write WPS profile header information.
   if (write(hProfile,&wpsProfileHeader,sizeof(WPSHELL_PROFILE_HEADER))!=sizeof(WPSHELL_PROFILE_HEADER))
      return (FALSE);

   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));

   // Initialize unique ID variable.
   wCurrentID = 0;
   // Assign unique IDs to all Workplace Shell objects (used by shadow objects).
   EnumerateWPSObjects(this,&wCurrentID);

   // Write desktop information, children objects, and custom settings.
   WriteObjectProfile(hProfile,NULL);

   // Check if original cursor was not the same as our "wait" cursor...
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));

   // Close WPSHELL.INI file.
   if (close(hProfile)==0) return (TRUE);
   else                    return (FALSE);
}


//---------------------------------------------------------------------
// WINDOWS 3.1 DESKTOP WINDOW SUB-CLASSING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// IsATMDriverLoaded():
//    This function checks for the Adobe Type Manager (ATM) driver in
// the SYSTEM.INI file [boot] section.  This is needed since there are
// some inter-action problem when the Workplace Shell and the ATM
// driver are loaded.
//---------------------------------------------------------------------
BOOL WPDESKTOP::IsATMDriverLoaded(void)
{
   char szValue[128];

   #ifdef _DEBUG
   dprintf("tr IsATMDriverLoaded(): entering function...\n");
   #endif

   // Read in the "atm.system.drv=" entry (if any) from SYSTEM.INI file.
   GetPrivateProfileString("boot","atm.system.drv","",szValue,sizeof(szValue),"SYSTEM.INI");
   // Check if value in key is "system.drv".
   if (stricmp(szValue,"system.drv")==0)
   {
      #ifdef _DEBUG
      dprintf("tr IsATMDriverLoaded(): ATM driver loaded (CP#1).\n");
      #endif
      // ATM driver is loaded, return TRUE.
      return (TRUE);
   }
   else
   {
      // Read in the "system.drv=" entry (if any) from SYSTEM.INI file.
      GetPrivateProfileString("boot","system.drv","",szValue,sizeof(szValue),"SYSTEM.INI");
      // Check if value in key is "atmsys.drv" (ATM driver).
      if (stricmp(szValue,"atmsys.drv")==0)
      {
         #ifdef _DEBUG
         dprintf("tr IsATMDriverLoaded(): ATM driver loaded (CP#2).\n");
         #endif
         // ATM driver is loaded, return TRUE.
         return (TRUE);
      }
   }

   #ifdef _DEBUG
   dprintf("tr IsATMDriverLoaded(): ATM driver not loaded.\n");
   #endif
   // No ATM driver has been loaded, return FALSE.
   return (FALSE);
}


//---------------------------------------------------------------------
// Windows31SubclassDesktop():
//    This function subclasses the Windows desktop. But, before we
// do that, we check for a special case which causes an inter-action
// problem.  If the Adobe Type Manager (ATM) driver is loaded, we need
// to wait until the driver has un-subclassed itself from the Windows
// desktop.  Then, we can safely subclass the desktop ourselves.
//    If we do not do this, the Workplace Shell desktop icons will not
// be drawn and thereofore be invisible.  Since the ATM driver
// overwrites our WndProc() address when un-subclassing, our WndProc()
// NEVER gets called.
//    The Windows desktop class name is "#32769".
//---------------------------------------------------------------------
void WPDESKTOP::Windows31SubclassDesktop(void)
{
   WNDCLASS wcDesktopClass;
   //MSG      Msg;

   #ifdef _DEBUG
   dprintf("tr Windows31SubclassDesktop(): entering function...\n");
   #endif

   // If ATM driver is loaded AND we could get the Windows desktop class info...
   if ((IsATMDriverLoaded()==TRUE) && (GetClassInfo(NULL,"#32769",&wcDesktopClass)!=FALSE))
   {
      #ifdef _DEBUG
      dprintf("tr Windows31SubclassDesktop(): ATM driver is loaded.\n");
      #endif

      // Repaint desktop window in order for
      ::InvalidateRect(GetDesktopWindow(),NULL,TRUE);
      ::UpdateWindow(GetDesktopWindow());

      //SetTimer(GetDesktopWindow(),0x123,1000,NULL);

      // While the ATM driver has the Windows desktop sub-classed...
      while (wcDesktopClass.lpfnWndProc!=(WNDPROC)GetWindowLong(GetDesktopWindow(),GWL_WNDPROC))
      {
         #ifdef _DEBUG
         dprintf("tr Windows31SubclassDesktop(): waiting for desktop to un-subclassed by ATM.\n");
         #endif

         /*
         // Try to yield to other apps and drivers (i.e. the ATM driver)...
         if (GetMessage(&Msg,NULL,0,0)!=FALSE)
         {
            MessageBeep(-1);
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
         }
         */
         // Force a desktop window repaint.
         ::UpdateWindow(GetDesktopWindow());
      }
   }

   //KillTimer(GetDesktopWindow(),0x123);

   #ifdef _DEBUG
   dprintf("tr Windows31SubclassDesktop(): subclass Windows desktop.\n");
   #endif
   // Sub-class Windows 3.1 desktop window (using Windows macro in "windowsx.h").
   m_wndprocWindows31Desktop = SubclassWindow(GetDesktopWindow(),WPDESKTOPSubClassWindowProc);
   #ifdef _DEBUG
   dprintf("tr Windows31SubclassDesktop(): leaving function.\n");
   #endif
}


//---------------------------------------------------------------------
// Windows31UnsubclassDesktop():
//---------------------------------------------------------------------
void WPDESKTOP::Windows31UnsubclassDesktop(void)
{
   // Unsub-class Windows 3.1 desktop window by replacing our window
   // procedure address with the original Windows 3.1 desktop window
   // procedure into the window class.
   SubclassWindow(GetDesktopWindow(),m_wndprocWindows31Desktop);
}

//---------------------------------------------------------------------
// GetWindows31DesktopProc():
//---------------------------------------------------------------------
WNDPROC WPDESKTOP::GetWindows31DesktopProc(void)
{
   return (m_wndprocWindows31Desktop);
}



//---------------------------------------------------------------------
// CLASS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetClassBackgroundBrush():
//    This function has been overloaded from its derived class
//---------------------------------------------------------------------
void WPDESKTOP::SetClassBackgroundBrush(HBRUSH *phbrBackground)
{
   // Set background brush to nothing since Desktop is transparent.
   *phbrBackground = (HBRUSH)(NULL);
}


//---------------------------------------------------------------------
// UnregisterClassName():
//---------------------------------------------------------------------
void WPDESKTOP::UnregisterClassName(char *pszClassName)
{
   // Windows 3.1 class unregistration.
   UnregisterClass(pszClassName,g_wpEnvironment.hInst);
   // Destroy unique class name.
   DestroyUniqueClassName(pszClassName);
}

//---------------------------------------------------------------------
// UnregisterClassName():
//---------------------------------------------------------------------
void WPDESKTOP::UnregisterClassName(WORD wClassID)
{
   char   szClassName[MAX_CLASS_NAME+1];

   // wParam = unsigned integer between 0 and 1024.
   sprintf(szClassName,"WPS%04d",wClassID);
   // Unregistre class name.
   UnregisterClassName(szClassName);
}

//---------------------------------------------------------------------
// DRAWING AND UPDATING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InvalidateRect():
//    This function invalidates a rectangular area on the desktop's
// device context.
//---------------------------------------------------------------------
void WPDESKTOP::InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Check that folder is open...
   if (isOpen()==TRUE)
   {
      // Invalidate rectangular area in window.
      ::InvalidateRect(GetDesktopWindow(),lprcArea,fEraseBkgrnd);
      // Check if window needs to be updated right away.
      if (fUpdateNow==TRUE)
         ::UpdateWindow(GetDesktopWindow());
   }
}


//---------------------------------------------------------------------
// Invalidate():
//    This function invalidates the complete desktop area.
//---------------------------------------------------------------------
void WPDESKTOP::Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Invalidate complete rectangular area of desktop window.
   InvalidateRect(NULL,fUpdateNow,fEraseBkgrnd);
}

//---------------------------------------------------------------------
// UpdateWindow():
//    This function calls the Windows 3.1 API window update function
// to redraw the invalid region's contents.
//---------------------------------------------------------------------
void WPDESKTOP::UpdateWindow(void)
{
   // Visually update folder window right now.
   ::UpdateWindow(GetHandle());
}


//---------------------------------------------------------------------
// DESKTOP SYSTEM MENU COMMANDS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// RunProgram():
//    This function executes a program file (*.exe;*.pif;*.com;*.bat)
// from Windows.  It prompts the user for a valid program name and path
// and then tries to execute it using "ExecuteApplication()".
//---------------------------------------------------------------------
void WPDESKTOP::RunProgram(void)
{
   DLGPROC  lpfnRunDlgProc;

   // Make procedure instance for "Run Program" dialog.
   lpfnRunDlgProc = (DLGPROC)MakeProcInstance((FARPROC)RunDlgProc,g_wpEnvironment.hInst);
   // Open dialog box inorder to query command line to execute.
   DialogBox(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_RUN),GetHandle(),lpfnRunDlgProc);
   // Free procedure instance for dialog.
   FreeProcInstance((FARPROC)lpfnRunDlgProc);
}



//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPDESKTOP::InitializePopup(HMENU hmenuPopup)
{
   // Check if no screen saver is currently active...
   if (GetProfileInt("Windows","ScreenSaveActive",0)==0)
      // Disable "Lockup now" menu item.
      EnableMenuItem(hmenuPopup,IDM_LOCKUPNOW,(MF_BYCOMMAND|MF_GRAYED));

   // Call base class function.
   WPFOLDER::InitializePopup(hmenuPopup);
}


//---------------------------------------------------------------------
// GetMenuName():
//    This function returns a null-terminated string with the name of
// the menu resource name used as the system menu for the desktop.
//---------------------------------------------------------------------
const char *WPDESKTOP::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPDESKTOP_SYSMENU_NAME));  // Return the default generic object menu.
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPDESKTOP::MenuSelection(WORD wMenuItem)
{
   DLGPROC  lpfnProductInfoDlgProc;
   WPOBJECT *pwpobjSystemSetup;

   // Process menu selection from system popup menu.
   switch (wMenuItem)
   {
      case IDM_OPENICON:
         // Check if current view is not equal to chosen view.
         if (GetOpenView()!=VIEW_ICON)
         {
            // Reset folder's open view.
            SetOpenView(VIEW_ICON);
            // Invalidate folder's client area and update it now.
            Invalidate(TRUE,TRUE);
         }
         break;

      case IDM_OPENDETAILS:
         // Check if current view is not equal to chosen view.
         if (GetOpenView()!=VIEW_DETAILS)
         {
            // Reset folder's open view.
            SetOpenView(VIEW_DETAILS);
            // Invalidate folder's client area and update it now.
            Invalidate(TRUE,TRUE);
         }
         break;

      case IDM_REFRESHNOW:
         Invalidate(TRUE,TRUE);
         break;

      case IDM_HELPINDEX:
         // Open help file with "Contents" help information.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTENTS,0L);
         break;

      case IDM_HELPGENERAL:
         // Open help file with "General help" for desktop objects.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_DESKTOP);
         break;

      case IDM_HELPPRODINFO:
         // Open product information modal dialog box.
         lpfnProductInfoDlgProc = (DLGPROC)MakeProcInstance((FARPROC)ProductInfoDlgProc,g_wpEnvironment.hInst);
         DialogBox(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_PRODUCTINFO),GetHandle(),lpfnProductInfoDlgProc);
         FreeProcInstance((FARPROC)lpfnProductInfoDlgProc);
         break;

      case IDM_LOCKUPNOW:
         PostMessage(GetHandle(),WM_SYSCOMMAND,SC_SCREENSAVE,0L);
         break;

      case IDM_SHUTDOWN:
         PostMessage(GetHandle(),WM_WPS_SYSSHUTDOWN,0,0L);
         break;

      case IDM_SYSTEMSETUP:
         // Find "System Setup" folder.
         pwpobjSystemSetup = Find(OBJTYPE_FOLDER,g_wpEnvironment.stSystem.szSystemSetupFolderName);
         // If folder was found...
         if (pwpobjSystemSetup!=NULL)
         {
            if (pwpobjSystemSetup->Open()==FALSE)
               MessageBeep(-1);
         }
         else
            // Notify user that system setup folder was not found.
            MessageBox(NULL,GetString(IDSTR_ERR_SYSSETUP_NOT_FOUND),
                       GetString(IDSTR_ERR_FIND_FOLDER_CAPTION),(MB_OK|MB_ICONEXCLAMATION));
         break;

      case IDM_ARRANGE:
         ArrangeIcons();
         break;

      case IDM_SAVEDESKTOP:
         // Save current desktop configuration.
         if (SaveDesktop()==FALSE)
            MessageBox(NULL,GetString(IDSTR_ERR_SAVE_DSKTOP),
                       GetString(IDSTR_ERR_SAVE_DSKTOP_CAPTION),(MB_OK|MB_ICONSTOP));
         break;

      case IDM_RUN:
         RunProgram();
         break;

      default:
         // Let folder handle the rest of menu selections.
         WPFOLDER::MenuSelection(wMenuItem);
         break;
   }
}


//---------------------------------------------------------------------
// KeyboardEvent():
//---------------------------------------------------------------------
void WPDESKTOP::KeyboardEvent(WORD wEventID)
{
   // Depending on the ID_EVENT_* code...
   switch (wEventID)
   {
      case ID_EVENT_SPECIAL:
         //SPECIALWindowProc(GetHandle(),WM_CREATE,0,0L);
         break;

      default:
         WPFOLDER::KeyboardEvent(wEventID);
         break;
   }
}

//---------------------------------------------------------------------
// GetTopLevelWindowMinimizeBehavior():
//---------------------------------------------------------------------
WORD WPDESKTOP::GetTopLevelWindowMinimizeBehavior(HWND hwnd)
{
   int index;

   // Loop through all top-level windows and return minimize behavior flag.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If current top-level window being tested is equal to window wanted...
      if (m_TopLevel[index].m_hwnd==hwnd)
         // Return the minimize behavior flag for matched window.
         return (m_TopLevel[index].m_wMinBehavior);
   }
   // No match found for window wanted, therefore we return the system default behavior flag.
   return (g_wpEnvironment.stSystem.wMinButtonBehavior);
}


//---------------------------------------------------------------------
// GetObjectFromInstanceHandle():
//---------------------------------------------------------------------
WPOBJECT *WPDESKTOP::GetObjectFromInstanceHandle(HINSTANCE hInstance)
{
   int index;

   // Loop through all top-level windows and return owner object of matching instance.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If current top-level instance being tested is equal to instance wanted...
      if (m_TopLevel[index].m_hInstance==hInstance)
         // Return the pointer object who owns instance.
         return (m_TopLevel[index].m_pwpobjOwner);
   }
   // No match found for instance wanted, therefore we return "no match".
   return (NULL);
}


//---------------------------------------------------------------------
// DESKTOP WORKPLACE SHELL MESSAGE PROCESSING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnTopLevelWindowCreated():
//---------------------------------------------------------------------
void WPDESKTOP::OnTopLevelWindowCreated(HWND hwndCreated)
{
   char szWindowTitle[MAX_TITLE_NAME+1];

   // Get window text of newly created window.
   GetWindowText(hwndCreated,szWindowTitle,MAX_TITLE_NAME);

   #ifdef _DEBUG
   dprintf("tr OnTopLevelWindowCreated(): hwnd = 0x%04X\n",hwndCreated);
   #endif

   // First, check that top-level window is not "Task List" or "Window List",
   // AND that it does have a title.
   // NOTE: Some applications (including DOS box) do not make their windows
   //       visible at create time, therefore we cannot test for visibility.
   if ((stricmp(szWindowTitle,"Task List")!=0)&&
       (stricmp(szWindowTitle,"Window List")!=0)&&
       (strlen(szWindowTitle)>0))
   {
      #ifdef _DEBUG
      dprintf("tr OnTopLevelWindowCreated(): store window 0x%04X as top-level\n",hwndCreated);
      #endif
      // Depending on what the default system behavior for the minimized button is...
      UpdateMinPlacement(hwndCreated,g_wpEnvironment.stSystem.wMinButtonBehavior);
      // Reset DOS DDE instance and object pointer values.
      m_ddeserverSHELL.SetProgramInstanceAndObject(NULL,NULL);
      // Successfully changed the minimized position of window.
      // Now, we need to store its window handle internally.
      AddTopLevelWindow(hwndCreated);
   }
}

//---------------------------------------------------------------------
// OnTopLevelWindowDestroyed():
//---------------------------------------------------------------------
void WPDESKTOP::OnTopLevelWindowDestroyed(HWND hwndDestroyed)
{
   // Remove top-level window from internal window handle list.
   DeleteTopLevelWindow(hwndDestroyed);
}


//---------------------------------------------------------------------
// OnFindInstanceHandle():
//---------------------------------------------------------------------
BOOL WPDESKTOP::OnFindInstanceHandle(HINSTANCE hInstance)
{
   int index;

   // Search top-level window list for instance handle match...
   for (index=0; index<m_nTopLevelWindowCount; index++)
      if (m_TopLevel[index].m_hInstance==hInstance)
         return (TRUE);    // match found...

   // Instance handle match not found in top-level window list.
   return (FALSE);
}


//---------------------------------------------------------------------
// OnAssociateObject():
//---------------------------------------------------------------------
void WPDESKTOP::OnAssociateObject(HINSTANCE hInstance,WPOBJECT *pwpobjOwner)
{
   int   index;
   char  szClassName[16];

   #ifdef _DEBUG
   dprintf("tr OnAssociateObject(hInst): hInst = 0x%04X\n",hInstance);
   #endif

   // Search list of top-level windows for specific instance handle.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If instance handle match is found in top-level window list...
      if (hInstance==m_TopLevel[index].m_hInstance)
      {                                                 
         #ifdef _DEBUG
         dprintf("tr OnAssociateObject(hInst): match found for instance -> hwnd = 0x%04X\n",m_TopLevel[index].m_hwnd);
         #endif
      
         // Get class name of window being associated...
         if (GetClassName(m_TopLevel[index].m_hwnd,szClassName,(sizeof(szClassName)-1))>0)
         {     
            // Check if window created is a DOS application window...
            if (stricmp(szClassName,"tty")==0)
               // Give the "Shell" DDE server (DOS apps) the instance of the program
               // that was just executed.  If the program was a "WinOldApp" (or DOS box),
               // then the DDE server will use this instance to supply information about
               // the application's title, working dir, and icon.
               m_ddeserverSHELL.SetProgramInstanceAndObject(hInstance,(WPPROGRAM *)pwpobjOwner);
         }

         // Store pointer to object that "owns" or "executed" top-level window.
         m_TopLevel[index].m_pwpobjOwner  = pwpobjOwner;
         m_TopLevel[index].m_wMinBehavior = (WORD)(pwpobjOwner->GetFlags()&MINBEHAVIOR_FLAGS_MASK);
         // Depending on what the behavior for the minimized button is...
         if (m_TopLevel[index].m_wMinBehavior!=g_wpEnvironment.stSystem.wMinButtonBehavior)
            UpdateMinPlacement(m_TopLevel[index].m_hwnd,m_TopLevel[index].m_wMinBehavior);
      }
   }
}


//---------------------------------------------------------------------
// OnAssociateObject():
//---------------------------------------------------------------------
void WPDESKTOP::OnAssociateObject(HWND hwnd,WPOBJECT *pwpobjOwner)
{
   int   index;

   #ifdef _DEBUG
   dprintf("tr OnAssociateObject(hwnd): hwnd = 0x%04X\n",hwnd);
   #endif

   // Search list of top-level windows for specific window handle.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If window handle match is found in top-level window list...
      if (hwnd==m_TopLevel[index].m_hwnd)
      {
         #ifdef _DEBUG
         dprintf("tr OnAssociateObject(hwnd): match found for window -> hInst = 0x%04X\n",m_TopLevel[index].m_hInstance);
         #endif
         // Store pointer to object that "owns" or "executed" top-level window.
         m_TopLevel[index].m_pwpobjOwner  = pwpobjOwner;
         m_TopLevel[index].m_wMinBehavior = (WORD)(pwpobjOwner->GetFlags()&MINBEHAVIOR_FLAGS_MASK);
         // Depending on what the behavior for the minimized button is...
         if (m_TopLevel[index].m_wMinBehavior!=g_wpEnvironment.stSystem.wMinButtonBehavior)
            UpdateMinPlacement(m_TopLevel[index].m_hwnd,m_TopLevel[index].m_wMinBehavior);
      }
   }
}


//---------------------------------------------------------------------
// OnGetWindowFromObject():
//    This function retrieves a window handle from a specified object
// pointer.  The internal data structure managed by the Desktop object
// keeps track of any top-level window owned by an WP object.
//---------------------------------------------------------------------
HWND WPDESKTOP::OnGetWindowFromObject(WPOBJECT *pwpobjOwner)
{
   int   index;

   // Search list of top-level windows for specific object pointer.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If instance handle match is found in top-level window list...
      if (pwpobjOwner==m_TopLevel[index].m_pwpobjOwner)
         return (m_TopLevel[index].m_hwnd);  // top-level window owned found.
   }

   // Object does not own any top-level windows...
   return (NULL);
}

//---------------------------------------------------------------------
// OnUpdateObjectMinPlacement():
//---------------------------------------------------------------------
void WPDESKTOP::OnUpdateObjectMinPlacement(HINSTANCE hInstUpdate)
{
   int index;

   // Search list of top-level windows for specific object pointer.
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      // If instance handle match is found in top-level window list...
      if (hInstUpdate==m_TopLevel[index].m_hInstance)
      {  
         #ifdef _DEBUG
         dprintf("tr OnUpdateObjectMinPlacement(): Minimized position updated for hWnd = 0x%04X.\n",m_TopLevel[index].m_hwnd);
         #endif
         // Update minimized window position and check if original position was different...
         UpdateMinPlacement(m_TopLevel[index].m_hwnd,m_TopLevel[index].m_wMinBehavior);
         return;
      }
   }
}


//---------------------------------------------------------------------
// OnNotifyObjectDestroyed():
//    This function gets called whenever a top-level window is
// destroyed and we get notified through the WH_SHELL hook callback
// which we have installed.
//---------------------------------------------------------------------
void WPDESKTOP::OnNotifyObjectDestroyed(WPOBJECT *pwpobjDestroyed)
{
   int index;

   // Search top-level window list for object pointer match...
   for (index=0; index<m_nTopLevelWindowCount; index++)
   {
      if (m_TopLevel[index].m_pwpobjOwner==pwpobjDestroyed)
      {
         // Remove top-level window from internal window handle list.
         DeleteTopLevelWindow(m_TopLevel[index].m_hwnd);
         // We are done, so break out of for loop.
         break;
      }
   }
}


//---------------------------------------------------------------------
// OnDeleteObject():
//    this function is called whenever a WP object is to be deleted by
// the Desktop object.  It deletes any shadow links the object might
// have and makes sure the object (and any children) gets deleted
// visually from parent (if open).
//---------------------------------------------------------------------
void WPDESKTOP::OnDeleteObject(WPOBJECT *pwpobjDelete)
{
   RECT        rcObj;
   WPCONTAINER *pwpcontainerParent;

   // Delete object from its parent container object.
   if (pwpobjDelete->GetParent()!=NULL)
   {
      // Get current object's logical area on parent container.
      pwpobjDelete->GetObjectRect(&rcObj);
      // Save pointer to parent container.
      pwpcontainerParent = pwpobjDelete->GetParent();
      // Delete object from parent container.
      pwpobjDelete->GetParent()->DeleteObject(pwpobjDelete);
      // Check if parent container open view is not "icon view"...
      if (pwpcontainerParent->GetOpenView()!=VIEW_ICON)
         // Invalidate the parent container's whole client area.
         pwpcontainerParent->Invalidate(FALSE,TRUE);
      else
         // Clean object's icon on parent container's work area.
         pwpcontainerParent->InvalidateRect(&rcObj,TRUE,TRUE);
   }

   // Destroy "deleted" object and all of its children (if any) recursively.
   DestroyRecursively(pwpobjDelete);
}


//---------------------------------------------------------------------
// DESKTOP MESSAGE PROCESSING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnCreate():
//---------------------------------------------------------------------
void WPDESKTOP::OnCreate(void)
{
   // Sub-class Windows 3.1 desktop window.
   Windows31SubclassDesktop();

   // Position Workplace Shell desktop on the bottom.
   SetWindowPos(GetHandle(),HWND_BOTTOM,0,0,0,0,(SWP_NOSIZE|SWP_NOMOVE));

   // Register shell hook.
   RegisterShellHook();

   // Call base class' function.
   WPFOLDER::OnCreate();
}


//---------------------------------------------------------------------
// OnDestroy():
//---------------------------------------------------------------------
void WPDESKTOP::OnDestroy(void)
{
   int index;

   // Destroy recursively all Workplace Shell objects.
   for (index=0; index<GetObjectCount(); index++)
      DestroyRecursively(GetObject(index));

   // Close desktop's settings notebook.
   CloseSettingsNotebook();

   // Unsub-class Windows 3.1 desktop window and repaint background.
   Windows31UnsubclassDesktop();
   ::InvalidateRect(GetDesktopWindow(),NULL,TRUE);
   ::UpdateWindow(GetDesktopWindow());

   // Unhook Shell hook.
   UnregisterShellHook();

   // Uninitialize and unregister all DDE services from desktop shell.
   m_ddeserverPROGMAN.Uninitialize();
   m_ddeserverSHELL.Uninitialize();

   // Quit Workplace Shell application.
   PostQuitMessage(0);
}


//---------------------------------------------------------------------
// OnWinINIChange():
//---------------------------------------------------------------------
void WPDESKTOP::OnWinINIChange(LPCSTR pszSection)
{
   // Check if the "colors" section in WIN.INI has changed...
   if ((pszSection==NULL)||(stricmp(pszSection,"Colors")==0))
      // Store new color reference value.
      m_clrrefBackground = GetSysColor(COLOR_BACKGROUND);
}


//---------------------------------------------------------------------
// OnEraseBackground():
//    This function just calls the default window procedure to erase
// the invisible Workplace Shell desktop window.  Since it is invisible,
// we do not need to do any special work.  The original Windows desktop
// window procedure.
//---------------------------------------------------------------------
LRESULT WPDESKTOP::OnEraseBackground(HDC hdc)
{
   // First, let default window procedure paint background color.
   return (DefWindowProc(GetHandle(),WM_ERASEBKGND,(WPARAM)hdc,0L));
}

//---------------------------------------------------------------------
// OnNCPaint():
//---------------------------------------------------------------------
void WPDESKTOP::OnNCPaint(void)
{
   // Do nothing.
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InsertSettingsNotebookPages(void)
//---------------------------------------------------------------------
void WPDESKTOP::InsertSettingsNotebookPages(void)
{   
   char szStatusText[80];

   // Insert the default object pages into the settings notebook.
   WPFOLDER::InsertSettingsNotebookPages();
   // Insert the "Lockup" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_LOCKUP),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPDESKTOP::LockupPageProc,NULL,MAKEINTRESOURCE(IDD_LOCKUP_PAGE));
   // Insert the "Desktop" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_DESKTOP),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPDESKTOP::DesktopPageProc,NULL,MAKEINTRESOURCE(IDD_DESKTOP_PAGE));
   // Insert the "Credits" page #1 into settings notebook.
   sprintf(szStatusText,GetString(IDSTR_SETTNB_STATUSTEXT),1,2);  // Page 1 of 2.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_CREDITS),(BKA_MAJOR|BKA_STATUSTEXTON|BKA_AUTOPAGESIZE),(FARPAGEPROC)NULL,szStatusText,MAKEINTRESOURCE(IDD_CREDITS_PAGE));
   // Insert the "Credits" page #2 into settings notebook.
   sprintf(szStatusText,GetString(IDSTR_SETTNB_STATUSTEXT),2,2);  // Page 2 of 2.
   GetSettingsNotebook()->InsertPage(NULL,(BKA_STATUSTEXTON|BKA_AUTOPAGESIZE),(FARPAGEPROC)NULL,szStatusText,MAKEINTRESOURCE(IDD_CREDITS2_PAGE));
}

//---------------------------------------------------------------------
// ImageControls():
//---------------------------------------------------------------------
void WPDESKTOP::ImageControls(HWND hwndBackgroundPage,BOOL fEnable)
{
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDS_FILETEXT),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDCB_BITMAPLIST),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_NORMALIMAGE),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_TILEDIMAGE),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_SCALEDIMAGE),FALSE);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_IMAGEGROUP),fEnable);
}

//---------------------------------------------------------------------
// LockupPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPDESKTOP::LockupPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   SPBLIMITS      spbLimits;
   SPBQUERYVALUE  spbQueryValue;
   BOOL           fScreenSaveActive;
   int            nTimeOut;
   long           lValue;

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Get default system parameters for lockup.
         SystemParametersInfo(SPI_GETSCREENSAVEACTIVE,0,&fScreenSaveActive,0);
         SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT,0,&nTimeOut,0);
         // Check respective radio button.
         if (fScreenSaveActive==TRUE)
            CheckRadioButton(hwndPage,IDB_NO_AUTOMATIC,IDB_AUTOMATIC,IDB_AUTOMATIC);
         else
            CheckRadioButton(hwndPage,IDB_NO_AUTOMATIC,IDB_AUTOMATIC,IDB_NO_AUTOMATIC);
         // Set timeout limits and current value in minutes,
         // value returned from SystemParamInfo() call is in seconds.
         spbLimits.lUpperLimit = 60;
         spbLimits.lLowerLimit = 0;
         SendDlgItemMessage(hwndPage,IDSB_TIMEOUT,SPBM_SETLIMITS,0,(LPARAM)&spbLimits);
         SendDlgItemMessage(hwndPage,IDSB_TIMEOUT,SPBM_SETCURRENTVALUE,0,(LPARAM)(nTimeOut/60));
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Set default radio button ("Automatic Lockup").
               CheckRadioButton(hwndPage,IDB_NO_AUTOMATIC,IDB_AUTOMATIC,IDB_AUTOMATIC);
               // Set the default time out period in seconds.
               SendDlgItemMessage(hwndPage,IDSB_TIMEOUT,SPBM_SETCURRENTVALUE,0,(LPARAM)5);
               break;

            case IDB_UNDO:
               // Get default system parameters for lockup.
               SystemParametersInfo(SPI_GETSCREENSAVEACTIVE,0,&fScreenSaveActive,0);
               SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT,0,&nTimeOut,0);
               // Check respective radio button.
               if (fScreenSaveActive==TRUE)
                  CheckRadioButton(hwndPage,IDB_NO_AUTOMATIC,IDB_AUTOMATIC,IDB_AUTOMATIC);
               else
                  CheckRadioButton(hwndPage,IDB_NO_AUTOMATIC,IDB_AUTOMATIC,IDB_NO_AUTOMATIC);
               // Set time out period in seconds (value returned in SystemParamInfo() call
               // is in milliseconds).
               SendDlgItemMessage(hwndPage,IDSB_TIMEOUT,SPBM_SETCURRENTVALUE,0,(LPARAM)(nTimeOut/60));
               break;

            case IDB_HELP:
               // Open help file with "Lockup" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_LOCKUPPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Query current "Lockup page" values.
         fScreenSaveActive = (IsDlgButtonChecked(hwndPage,IDB_AUTOMATIC)==1);
         // Get time out value from spin button.
         spbQueryValue.lpValueBuf = (LPVOID)&lValue;
         spbQueryValue.wBufSize   = 0;
         SendDlgItemMessage(hwndPage,IDSB_TIMEOUT,SPBM_QUERYVALUE,0,(LPARAM)&spbQueryValue);
         // Convert spin button's value from minutes to milliseconds.
         nTimeOut = (int)(lValue*60);
         // Set new system parameters for screen saver ("lock up").
         SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,(UINT)fScreenSaveActive,0,(SPIF_UPDATEINIFILE|SPIF_SENDWININICHANGE));
         SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT,(UINT)nTimeOut,0,(SPIF_UPDATEINIFILE|SPIF_SENDWININICHANGE));
         break;
   }
}


//---------------------------------------------------------------------
// DesktopPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPDESKTOP::DesktopPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Depending on the current state of the flag...
         if ((GetFlags()&DESKTOP_FLAGS_MASK)==DESKTOP_SAVESETTINGS)
            // Check checkbox since flag is set.
            CheckDlgButton(hwndPage,IDB_SAVEDESKTOPSETTINGS,1);
         // Check whether we have the "No Save" feature is enabled.
         if (g_wpEnvironment.stSystem.fNoSaveEnabled==TRUE)
            EnableWindow(GetDlgItem(hwndPage,IDB_SAVEDESKTOPSETTINGS),FALSE);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Uncheck checkbox since by default we do not save desktop automatically.
               CheckDlgButton(hwndPage,IDB_SAVEDESKTOPSETTINGS,1);
               break;

            case IDB_UNDO:
               // Depending on the current state of the flag...
               if ((GetFlags()&DESKTOP_FLAGS_MASK)==DESKTOP_SAVESETTINGS)
                  // Check checkbox since flag is set.
                  CheckDlgButton(hwndPage,IDB_SAVEDESKTOPSETTINGS,1);
               break;

            case IDB_HELP:
               // Open help file with "Desktop" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_DESKTOPPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Query current "Save Desktop settings" checbox state...
         if (IsDlgButtonChecked(hwndPage,IDB_SAVEDESKTOPSETTINGS)==1)
            // Set "Save Desktop settings" flag.
            SetFlagsOR(DESKTOP_SAVESETTINGS,DESKTOP_FLAGS_MASK);
         else
            // Reset "Save Desktop settings" flag.
            SetFlagsOR(0L,DESKTOP_FLAGS_MASK);
         break;
   }
}


//---------------------------------------------------------------------
// SYSTEM SHUTDOWN QUERY FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// IsShell():
//    This function is used when processing the WM_CLOSE message.  It
// tells us if Workplace Shell for Windows is the default Windows shell
// or not by examining the SYSTEM.INI file and checking the "shell="
// profile entry.
//---------------------------------------------------------------------
BOOL WPDESKTOP::IsShell(BOOL fCheck)
{
   char  szValue[128];

   // If we want the function to check at this moment...
   if (fCheck==TRUE)
   {
      // Read in the "shell=" entry from SYSTEM.INI file.
      GetPrivateProfileString("boot","shell","none",szValue,sizeof(szValue),"SYSTEM.INI");
      // Check line contents for a match to Workplace Shell for Windows executable name.
      if (stristr(szValue,GetString(IDSTR_SHELL_FILENAME))!=NULL)
         // Workplace Shell for Windows is the current default shell.
         m_fIsShell = TRUE;
      else
         // Workplace Shell for Windows is not the current default shell.
         m_fIsShell = FALSE;
   }

   // Return TRUE if Workplace Shell for Windows is the current shell.
   return (m_fIsShell);
}


//---------------------------------------------------------------------
// QueryQuit():
//    This function queries all the top-level windows in the system if
// it is OK to end their session.  If, for any reason, a top-level
// window returns "no" (or FALSE), we abort enumeration and return
// result.
//---------------------------------------------------------------------
BOOL WPDESKTOP::QueryQuit(void)
{
   BOOL           fQuit;
   WNDENUMPROC    lpfnEnumWindowsProc;

   // Create a procedure callback instance for windows enumeration function.
   lpfnEnumWindowsProc = (WNDENUMPROC)MakeProcInstance((FARPROC)WPDESKTOPEnumWindowsProc,g_wpEnvironment.hInst);
   // Enumerate all top-level windows and return result.
   fQuit = EnumWindows(lpfnEnumWindowsProc,0);
   // Destroy procedure callback instance.
   FreeProcInstance((FARPROC)lpfnEnumWindowsProc);
   // Return result value.
   return (fQuit);
}

//---------------------------------------------------------------------
// DESKTOP SIZE MANAGEMENT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CheckDesktopSizeChange():
//---------------------------------------------------------------------
BOOL WPDESKTOP::CheckDesktopSizeChange(void)
{
   SIZE  sizeDesktop;
   
   // Get current Workplace Shell desktop size.   
   sizeDesktop = GetWndSize();
   // Check Workplace Shell's desktop size against the Windows' desktop size...
   if ((GetSystemMetrics(SM_CXSCREEN)!=sizeDesktop.cx) ||
       (GetSystemMetrics(SM_CYSCREEN)!=sizeDesktop.cy))
   {    
   
      #ifdef _DEBUG
      dprintf("tr CheckDesktopSizeChange(): (%d,%d) -> (%d,%d)\n",sizeDesktop.cx,sizeDesktop.cy,
               GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
      #endif

      // Prepare "size" structure with the current Windows' desktop size.
      sizeDesktop.cx = GetSystemMetrics(SM_CXSCREEN);
      sizeDesktop.cy = GetSystemMetrics(SM_CYSCREEN);                    
      
      // Set the Workplace shell's desktop size to be the same.
      SetWndSize(sizeDesktop);
      SetWindowPos(GetHandle(),NULL,0,0,sizeDesktop.cx,sizeDesktop.cy,
                   (SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
      // Return "true" that the desktop size had to be changed.                   
      return (TRUE);
   }
   else
      // Return "false" that the desktop size had to be changed.
      return (FALSE);
}


//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPDESKTOP::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   // Read desktop and children objects...
   if (WPFOLDER::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Force desktop window to cover the whole screen size (i.e. change of resolution).
   SetWndPos(0,0);
   SetWndSize(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

   // Force "closed" state for Desktop object.
   SetState((GetState()&(~OBJSTATE_OPEN)));

   // If desktop color read from WPSHELL.INI profile is different than the
   // desktop color in WIN.INI, then change WIN.INI color to match color
   // in WPSHELL.INI profile.
   if (GetBackgroundColor()!=GetSysColor(COLOR_BACKGROUND))
      SetBackgroundColor(GetBackgroundColor());

   // Return success.
   return (TRUE);
}

// EOF WPDESKTP.CPP


