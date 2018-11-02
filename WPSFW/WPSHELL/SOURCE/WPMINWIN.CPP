//=====================================================================
// WPMINWIN.CPP
//    WPMINVIEWER Class implementation.
//
// Date Created :  04/02/94
// Last Update  :  12/16/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 04/02/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <io.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// Undocumented Windows API (from USER module):
extern "C" void FAR PASCAL    SwitchToThisWindow(HWND,BOOL);   // USER.172

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Minimized window viewer default values.
#define WPS_MINVIEWER_DEFAULT_ICONINDEX    ICONINDEX(IDI_MINVIEWER)
// Minimized window viewer system menu resource name (popup menu).
#define WPMINVIEWER_SYSMENU_NAME           MinimizedWindowViewerPopUpMenu
// Minimized window object system menu resource name (popup menu).
#define WPMINWINDOW_SYSMENU_NAME           MinimizedWindowObjectPopUpMenu
// Minviewer refresh timer constants.
#define TID_MINVIEWER_REFRESH_TIMER        200
#define MINVIEWER_REFRESH_TIME             250    // Every 250ms (or 1/4 sec).
// Maximum number of minimized windows that minimized window viewer will display.
#define MAX_MINWINDOWS                     MAX_CONTAINER_COMPONENTS
// Minimized window object default values.
#define WPS_MINWINOBJ_DEFAULT_ICONINDEX    ICONINDEX(IDI_DEFAULTPROGRAM)


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WORKPLACE SHELL MINIMIZED WINDOW OBJECT CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPMINWNDOBJ():
//---------------------------------------------------------------------
WPMINWNDOBJ::WPMINWNDOBJ(void) : WPOBJECT()
{
   // Initialize object.
   SetType(OBJTYPE_MINWINDOW);
}

//---------------------------------------------------------------------
// SetMinWndHandle():
//---------------------------------------------------------------------
void WPMINWNDOBJ::SetMinWndHandle(HWND hwndMinWnd)
{
   char szTitle[MAX_TITLE_NAME];

   // Store minimized window handle.
   m_hwndMinWnd = hwndMinWnd;

   // Query window's caption title text.
   GetWindowText(hwndMinWnd,szTitle,MAX_TITLE_NAME);
   SetTitle(szTitle);
}


//---------------------------------------------------------------------
// GetMinWndHandle():
//---------------------------------------------------------------------
HWND WPMINWNDOBJ::GetMinWndHandle(void)
{
   return (m_hwndMinWnd);
}


//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPMINWNDOBJ::Open(int nCmdShow)
{
   HWND  hWndLastActive;

   // If minimized window handle is not valid anymore,
   if (IsWindow(GetMinWndHandle())==FALSE)
      // Then, exit immediatedly.
      return (FALSE);

   // Get last active popup window from main window handle...
   hWndLastActive = GetLastActivePopup(GetMinWndHandle());
   // If there was no last active popup,
   if (hWndLastActive==NULL)
      // Use current window handle.
      hWndLastActive = GetMinWndHandle();

   // Give focus to last active popup from application.
   // (Undocumented Windows API, p.497 "Undocumented Windows").
   SwitchToThisWindow(hWndLastActive,TRUE);

   // If command show value is not "restore"...
   if (nCmdShow!=SW_RESTORE)
      // Force command show to activated window.
      ShowWindow(hWndLastActive,nCmdShow);

   return (TRUE);
}

//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPMINWNDOBJ::Open(void)
{
   return (Open(SW_RESTORE));
}


//---------------------------------------------------------------------
// APPEARANCE AND POSITION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPMINWNDOBJ::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_MINWINOBJ_DEFAULT_ICONINDEX;
}

//---------------------------------------------------------------------
// LoadIcon():
//---------------------------------------------------------------------
BOOL WPMINWNDOBJ::LoadIcon(void)
{
   HICON hIcon,hDupIcon;

   // Query class icon handle.
   hIcon = (HICON)GetClassWord(GetMinWndHandle(),GCW_HICON);
   if ((hIcon==NULL)||(hIcon==(HICON)-1))
   {
      // Try to get an icon by qeurying the window procedure.
      hIcon = (HICON)(WORD)SendMessage(GetMinWndHandle(),WM_QUERYDRAGICON,0,0L);
      if ((hIcon==NULL)||(hIcon==(HICON)-1))
      {
         // No icon is available for this minimized window, therefore load
         // default minimized object icon and return.
         return (WPOBJECT::LoadIcon());
      }
   }

   // Make duplicate copy of icon (since we do not want to reply on querying
   // this icon every time we need it.
   hDupIcon = CopyIcon(g_wpEnvironment.hInst,hIcon);

   // Store icon handle.
   SetIconHandle(hDupIcon);

   // Successfully loaded icon resource for minimized window object.
   return (TRUE);
}

//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPMINWNDOBJ::InitializePopup(HMENU hmenuPopup)
{
   // Check if minimized window cannot be maximized.
   if ((GetWindowLong(GetMinWndHandle(),GWL_STYLE)&WS_MAXIMIZEBOX)!=WS_MAXIMIZEBOX)
      // Remove the "maximize" menu entry from popup menu.
      DeleteMenu(hmenuPopup,IDM_WINDOWMAXIMIZE,MF_BYCOMMAND);
}


//---------------------------------------------------------------------
// GetMenuName():
//---------------------------------------------------------------------
const char *WPMINWNDOBJ::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPMINWINDOW_SYSMENU_NAME));
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPMINWNDOBJ::MenuSelection(WORD wMenuItem)
{
   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_HELPGENERAL:
         // Open help file with "General help" for minimized window object.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_MINWINDOWOBJ);
         break;

      case IDM_WINDOWRESTORE:
         // Restore previous size of window and activate.
         Open(SW_RESTORE);
         break;

      case IDM_WINDOWMAXIMIZE:
         // Maximize window and activate.
         Open(SW_SHOWMAXIMIZED);
         break;

      case IDM_CLOSE:
         // Close minimized window.
         PostMessage(GetMinWndHandle(),WM_CLOSE,0,0L);
         break;

      case IDM_WINDOWLIST:
         // Pull-up window list.
         PostMessage(GetDesktopWindow(),WM_SYSCOMMAND,SC_TASKLIST,0L);
         break;

      default:
         // Call the default handler function.
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
BOOL WPMINWNDOBJ::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // If target is the minimized window viewer and drag function is "move"...
   if ((pwpobjDropTarget==GetParent())&&(nFunction==DRAG_MOVE))
      // Call default function handler for base class.
      return (WPOBJECT::TestDropSource(pwpobjDropTarget,nFunction));
   else
      // Drop operation is not supported.
      return (FALSE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPMINWNDOBJ::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // No target drop operations are supported by minimized window object.
   return (FALSE);
}


//---------------------------------------------------------------------
// WORKPLACE SHELL MINIMIZED WINDOW VIEWER CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPMINVIEWER():
//---------------------------------------------------------------------
WPMINVIEWER::WPMINVIEWER(void) : WPFOLDER()
{
   // Initialize object.
   SetType(OBJTYPE_MINVIEWER);
   SetTitle("Minimized Window Viewer");
}

//---------------------------------------------------------------------
// WPMINVIEWER():
//---------------------------------------------------------------------
WPMINVIEWER::WPMINVIEWER(WPMINVIEWER *pwpminviewerDefaults) : WPFOLDER((WPFOLDER *)pwpminviewerDefaults)
{
   // Initialize object.
   SetType(OBJTYPE_MINVIEWER);
}


//---------------------------------------------------------------------
// MINIMIZED WINDOW VIEWER DEFAULT VALUES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPMINVIEWER::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_MINVIEWER_DEFAULT_ICONINDEX;
}


//---------------------------------------------------------------------
// MINIMIZED WINDOW VIEWER STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPMINVIEWER::Close(void)
{
   // If minimized window viewer is open...
   if (isOpen()==TRUE)
   {
      // Destroy timer
      KillTimer(GetHandle(),TID_MINVIEWER_REFRESH_TIMER);
      // Call the default base class close function.
      return (WPFOLDER::Close());
   }
   // Its already closed, so we return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetMenuName():
//---------------------------------------------------------------------
const char *WPMINVIEWER::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPMINVIEWER_SYSMENU_NAME));
}


//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPMINVIEWER::MenuSelection(WORD wMenuItem)
{
   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_HELPGENERAL:
         // Open help file with "General help" for Minimized Window Viewer.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_MINWINDOWVIEWER);
         break;

      default:
         // Call the default handler function.
         WPFOLDER::MenuSelection(wMenuItem);
         break;
   }
}



//---------------------------------------------------------------------
// MINIMIZED WINDOW VIEWER CONTENTS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// QueryCurrentMinWindows():
//---------------------------------------------------------------------
int WPMINVIEWER::QueryCurrentMinWindows(HWND *ahwndMinWindows)
{
   HWND  hwnd;
   int   nMinWindows;

   // Intiialize minimized top-level window count.
   nMinWindows = 0;
   // Get first top-level window.
   hwnd = GetWindow(GetHandle(),GW_HWNDFIRST);
   // While there are still more top-level windows...
   while (hwnd!=NULL)
   {
      // Test for top-level minimized windows...
      if ( (hwnd != GetHandle())             &&
           (IsWindowVisible(hwnd)==TRUE)     &&
           (GetWindow(hwnd,GW_OWNER)==NULL)  &&
           (IsIconic(hwnd)==TRUE) )
      {
         // Store handle to minimized top-level window and increment count.
         ahwndMinWindows[nMinWindows++] = hwnd;
      }
      // Get next top-level window.
      hwnd = GetWindow(hwnd,GW_HWNDNEXT);
   }
   // Return the count of minimized top-level windows found.
   return (nMinWindows);
}

//---------------------------------------------------------------------
// IsWindowMinimizedToViewer():
//---------------------------------------------------------------------
BOOL WPMINVIEWER::IsWindowMinimizedToViewer(HWND hwndWindow)
{
   // Check if window's minimize behavior is "minimize to viewer".
   if (m_pwpDesktop->GetTopLevelWindowMinimizeBehavior(hwndWindow)==(WORD)MINBEHAVIOR_MIN_TO_VIEWER)
      return (TRUE);
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// IsObjectValid():
//---------------------------------------------------------------------
BOOL WPMINVIEWER::IsObjectValid(WPMINWNDOBJ *pwpobjTest,WPMINWNDOBJ **apwpminwndValidList,int nListSize)
{
   int index;

   // Loop through all objects in valid list...
   for (index=0; index<nListSize; index++)
      // If object being tested is equal to the valid object in this array index,
      if (pwpobjTest==apwpminwndValidList[index])
         // Tested being object is valid.
         return (TRUE);

   // Minimized window object not in valid list, therefore it is an invalid object.
   return (FALSE);

}

//---------------------------------------------------------------------
// GetMinWindowObjInViewer():
//---------------------------------------------------------------------
WPMINWNDOBJ *WPMINVIEWER::GetMinWindowObjectInViewer(HWND hwnd)
{
   int   index;

   // Loop through all available minimized window objects in the viewer.
   for (index=0; index<GetObjectCount(); index++)
   {
      // Check if this object's minimized window handle is equal to the window handle wanted.
      if (((WPMINWNDOBJ *)GetObject(index))->GetMinWndHandle()==hwnd)
         return ((WPMINWNDOBJ *)GetObject(index));    // Return pointer to object.
   }
   // No match found, return "nothing".
   return (NULL);
}

//---------------------------------------------------------------------
// UpdateMinViewerContents():
//---------------------------------------------------------------------
void WPMINVIEWER::UpdateMinViewerContents(int nCount,HWND *ahwndWindows)
{
   int            index,nValidMinWindows;
   WPMINWNDOBJ    *pwpMinWindow,*apwpValidMinWindows[MAX_MINWINDOWS];
   RECT           rcIcon;

   // Initialize valid minimized windows list counter.
   nValidMinWindows = 0;
   // Loop through all minimized window, and filter out the ones that need to be
   // in the minimized window viewer.
   for (index=0; index<nCount; index++)
   {
      // If top-level minimized window has "minimized to viewer" behavior...
      if (m_pwpDesktop->GetTopLevelWindowMinimizeBehavior(ahwndWindows[index])==MINBEHAVIOR_MIN_TO_VIEWER)
      {
         // Test if window is already in minimized window viewer...
         pwpMinWindow = GetMinWindowObjectInViewer(ahwndWindows[index]);
         // If is not already in minimized window viewer...
         if (pwpMinWindow==NULL)
         {
            // Create new instance of minimized window object.
            pwpMinWindow = new WPMINWNDOBJ;
            // Initialize minimized window object.
            pwpMinWindow->SetMinWndHandle(ahwndWindows[index]);
            // Position new object inside minimized window viewer.
            pwpMinWindow->SetIconPos(GetFirstAvailableIconPos());
            // Add new minimized window object to minimized window viewer.
            AddObject((WPOBJECT *)pwpMinWindow);
            // Load icon for new object.
            pwpMinWindow->LoadIcon();
            // Update new object on viewer.
            pwpMinWindow->InvalidateIcon(TRUE,TRUE);
         }
         // Insert minimized window object into list of valid objects.
         apwpValidMinWindows[nValidMinWindows++] = pwpMinWindow;
      }
   }

   // Prune invalid objects from list in minimized window viewer.
   for (index=0; index<GetObjectCount(); index++)
   {
      // Make local copy of object pointer.
      pwpMinWindow = (WPMINWNDOBJ *)GetObject(index);
      // If minimized window viewer object is not in valid list...
      if (IsObjectValid(pwpMinWindow,(WPMINWNDOBJ **)apwpValidMinWindows,nValidMinWindows)==FALSE)
      {
         // Invalidate object icon area.
         pwpMinWindow->GetObjectRect(&rcIcon);
         // Delete invalid minimized window object.
         DeleteObject(index);
         // Destroy any icon resource.
         pwpMinWindow->DestroyIcon();
         // Deallocate memory.
         delete pwpMinWindow;
         // Update minimized window viewer client area by erasing object's icon.
         InvalidateRect(&rcIcon,TRUE,TRUE);
         // Decrement index counter because we have shrunk minimized window viewer object list
         // by one object, therefore there is an "untested" object now at position "index".
         // Therefore we must check position "index" again.
         index--;
      }
   }
}

//---------------------------------------------------------------------
// OnInitializeFolder():
//---------------------------------------------------------------------
void WPMINVIEWER::OnInitializeFolder(void)
{
   // Get pointer to Workplace Shell desktop object which is used later
   // to quickly access the minimized window's minimize behavior.
   m_pwpDesktop = (WPDESKTOP *)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_QUERY_FOLDER_POINTER,0,0L);

   // We need to create a Windows 3.1 timer to refresh the minimized
   // window viewer.
   SetTimer(GetHandle(),TID_MINVIEWER_REFRESH_TIMER,MINVIEWER_REFRESH_TIME,NULL);
}



//---------------------------------------------------------------------
// MINIMIZED VIEWER MESSAGE PROCESSING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnTimer():
//    This function is called when the minimized window viewer needs
// to refresh its contents.  This function only does anything if the
// minimized window viewer is open and visible (not iconic).
//---------------------------------------------------------------------
void WPMINVIEWER::OnTimer(WORD wTimerID)
{
   int   nMinWindows;
   HWND  hwndMinWindows[MAX_MINWINDOWS];

   // Check that minimized window viewer is open...
   if (isOpen()==TRUE)
   {
      // Check that minimized window viewer is not in iconized state.
      if (IsIconic(GetHandle())==FALSE)
      {
         // If timer generated is the refresh timer...
         if (wTimerID==TID_MINVIEWER_REFRESH_TIMER)
         {
            // Get new list of minimized windows.
            nMinWindows = QueryCurrentMinWindows(&(hwndMinWindows[0]));
            // Update minimized window viewer contents.
            UpdateMinViewerContents(nMinWindows,&(hwndMinWindows[0]));
         }
      }
   }
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
BOOL WPMINVIEWER::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // If this minimized window object has a parent...
   if (pwpobjDropSource->GetParent()!=NULL)
   {
      // Test if drop source belongs to this min viewer and object is being "moved".
      if ((pwpobjDropSource->GetParent()==this)&&(nFunction==DRAG_MOVE))
         // Call default function handler for base class.
         return (WPFOLDER::TestDropTarget(pwpobjDropSource,nFunction));
   }
   else
   {
      // If object is only being "moved" and it is a type "minimized window" object.
      if ((nFunction==DRAG_MOVE)&&((pwpobjDropSource->GetType()&OBJTYPE_MINWINDOW)==OBJTYPE_MINWINDOW))
         // Call default function handler for base class.
         return (WPFOLDER::TestDropTarget(pwpobjDropSource,nFunction));
   }
   // Drop operation is not supported by minimized window viewer.
   return (FALSE);
}


//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPMINVIEWER::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_CONTAINER_STRUCT   fcsData;
   FILE_FOLDER_STRUCT      ffsData;

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

   // Save the number of child objects in this container object.
   fcsData.m_nObjects = 0;

   // Write container extra information.
   if (write(hProfile,&fcsData,sizeof(FILE_CONTAINER_STRUCT))!=sizeof(FILE_CONTAINER_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPMINWIN.CPP


