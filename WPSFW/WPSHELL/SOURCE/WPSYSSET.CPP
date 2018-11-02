//=====================================================================
// WPSYSSET.CPP
//    WPSYSSETUP Class implementation.
//
// Date Created :  03/29/94
// Last Update  :  10/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 03/29/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// SYSSETUP object default values.
#define WPS_SYSSETUP_DEFAULT_ICONINDEX  ICONINDEX(IDI_SYSTEM)


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WORKPLACE SHELL SYSSETUP CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSYSSETUP():
//---------------------------------------------------------------------
WPSYSSETUP::WPSYSSETUP(void) : WPSETUP()
{
   // Initialize object.
   SetType(OBJTYPE_SYSTEMSETUP);
   SetTitle(GetString(IDSTR_OBJTITLE_SYSTEM));
}


//---------------------------------------------------------------------
// WPSYSSETUP():
//---------------------------------------------------------------------
WPSYSSETUP::WPSYSSETUP(WPSYSSETUP *pwpsyssetupDefaults) : WPSETUP((WPSETUP *)pwpsyssetupDefaults)
{
   // Initialize object.
   SetType(OBJTYPE_SYSTEMSETUP);
}


//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPSYSSETUP::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_SYSSETUP_DEFAULT_ICONINDEX;
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InsertSettingsNotebookPages()
//---------------------------------------------------------------------
void WPSYSSETUP::InsertSettingsNotebookPages(void)
{          
   char szStatusText[80];

   // Insert the "Confirmation" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_CONFIRMATION),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPSYSSETUP::ConfirmationsPageProc,NULL,MAKEINTRESOURCE(IDD_SYSSETUP_CONFIRMATION_PAGE));
   // Insert the "Window" page into settings notebook.
   sprintf(szStatusText,GetString(IDSTR_SETTNB_STATUSTEXT),1,2);  // Page 1 of 2.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_WINDOW),(BKA_MAJOR|BKA_STATUSTEXTON|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPSYSSETUP::WindowPageProc,szStatusText,MAKEINTRESOURCE(IDD_SYSSETUP_WINDOW_PAGE));
   // Insert the "Window" page #2 into settings notebook.
   sprintf(szStatusText,GetString(IDSTR_SETTNB_STATUSTEXT),2,2);  // Page 2 of 2.
   GetSettingsNotebook()->InsertPage(NULL,(BKA_STATUSTEXTON|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPSYSSETUP::Window2PageProc,szStatusText,MAKEINTRESOURCE(IDD_SYSSETUP_WINDOW2_PAGE));
   // Insert default object pages.
   WPOBJECT::InsertSettingsNotebookPages();
}


//---------------------------------------------------------------------
// UpdateWindowPageSYSTEMProfile():
//---------------------------------------------------------------------
void WPSYSSETUP::UpdateWindowPageSYSTEMProfile(HWND hwndPage)
{
   // Button appearance.
   if (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_BUTTONAPPEARANCE),GetString(IDSTR_HIDE),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_BUTTONAPPEARANCE),GetString(IDSTR_MINIMIZE),GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // Window animation.
   if (IsDlgButtonChecked(hwndPage,IDB_ENABLED)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_ANIMATION),GetString(IDSTR_ENABLED),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_ANIMATION),GetString(IDSTR_DISABLED),GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // Minimize button behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_MINHIDE)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_MINBEHAVIOR),GetString(IDSTR_HIDE),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINTOVIEWER)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_MINBEHAVIOR),GetString(IDSTR_MINTOVIEWER),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_MINBEHAVIOR),GetString(IDSTR_MINTODESKTOP),GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // Object open behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_DISPLAY)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_OPENBEHAVIOR),GetString(IDSTR_DISPLAYEXISTING),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_OPENBEHAVIOR),GetString(IDSTR_CREATENEW),GetString(IDSTR_WIN_SYSTEM_PROFILE));
}


//---------------------------------------------------------------------
// GetWindowPageSYSTEMProfile():
//---------------------------------------------------------------------
void WPSYSSETUP::GetWindowPageSYSTEMProfile(void)
{
   char szValue[64];

   // Load default button appearance (hide or minimize).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_BUTTONAPPEARANCE),GetString(IDSTR_HIDE),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_HIDE))==0)
      g_wpEnvironment.stSystem.fButtonAppearanceHide = TRUE;
   else
      g_wpEnvironment.stSystem.fButtonAppearanceHide = FALSE;

   // Load default window animation status (enabled or disabled).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_ANIMATION),GetString(IDSTR_ENABLED),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_ENABLED))==0)
      g_wpEnvironment.stSystem.fWindowAnimationEnabled = TRUE;
   else
      g_wpEnvironment.stSystem.fWindowAnimationEnabled = FALSE;

   // Load default minimize window button behavior.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_MINBEHAVIOR),GetString(IDSTR_HIDE),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_HIDE))==0)
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_HIDE;
   else
   if (stricmp(szValue,GetString(IDSTR_MINTOVIEWER))==0)
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_VIEWER;
   else
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_DESKTOP;

   // Load default object open behavior.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_OPENBEHAVIOR),GetString(IDSTR_CREATENEW),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_CREATENEW))==0)
      g_wpEnvironment.stSystem.wOpenBehavior = OPENBEHAVIOR_CREATE;
   else
      g_wpEnvironment.stSystem.wMinButtonBehavior = OPENBEHAVIOR_DISPLAY;
}


//---------------------------------------------------------------------
// UpdateWindowPageControls():
//---------------------------------------------------------------------
void WPSYSSETUP::UpdateWindowPageControls(HWND hwndPage)
{
   // Button appearance selections.
   if (g_wpEnvironment.stSystem.fButtonAppearanceHide==TRUE)
   {
      // Check "hide button" radio button.
      CheckRadioButton(hwndPage,IDB_HIDEBUTTON,IDB_MINIMIZEBUTTON,IDB_HIDEBUTTON);
      // Disable all minimized button behavior.
      EnableWindow(GetDlgItem(hwndPage,IDB_MINHIDE),FALSE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTOVIEWER),FALSE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTODESKTOP),FALSE);
      // Default to GetString(IDSTR_HIDE) minimize behavior.
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_HIDE;
   }
   else
   {
      // Check "minimize button" radio button.
      CheckRadioButton(hwndPage,IDB_HIDEBUTTON,IDB_MINIMIZEBUTTON,IDB_MINIMIZEBUTTON);
      // Enable all minimized button behavior.
      EnableWindow(GetDlgItem(hwndPage,IDB_MINHIDE),TRUE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTOVIEWER),TRUE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTODESKTOP),TRUE);
   }

   // Window animation selections.
   if (g_wpEnvironment.stSystem.fWindowAnimationEnabled==TRUE)
      CheckRadioButton(hwndPage,IDB_ENABLED,IDB_DISABLED,IDB_ENABLED);
   else
      CheckRadioButton(hwndPage,IDB_ENABLED,IDB_DISABLED,IDB_DISABLED);

   // Check the correct radio button.
   if (g_wpEnvironment.stSystem.wMinButtonBehavior==MINBEHAVIOR_HIDE)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINHIDE);
   else
   if (g_wpEnvironment.stSystem.wMinButtonBehavior==MINBEHAVIOR_MIN_TO_VIEWER)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTOVIEWER);
   else
   if (g_wpEnvironment.stSystem.wMinButtonBehavior==MINBEHAVIOR_MIN_TO_DESKTOP)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTODESKTOP);

   // Object open behavior.
   if (g_wpEnvironment.stSystem.wOpenBehavior==OPENBEHAVIOR_DISPLAY)
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_DISPLAY);
   else
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_CREATE);
}


//---------------------------------------------------------------------
// WindowPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPSYSSETUP::WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize radio buttons.
         UpdateWindowPageControls(hwndPage);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Reset radio buttons to default folder object values.
               g_wpEnvironment.stSystem.fButtonAppearanceHide    = TRUE;
               g_wpEnvironment.stSystem.fWindowAnimationEnabled  = TRUE;
               g_wpEnvironment.stSystem.wMinButtonBehavior       = MINBEHAVIOR_HIDE;
               g_wpEnvironment.stSystem.wOpenBehavior            = OPENBEHAVIOR_CREATE;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_UNDO:
               GetWindowPageSYSTEMProfile();
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_HELP:
               // Open help file with "Window" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SYSTEM_SETNB_WINDOWPAGE);
               break;

            case IDB_HIDEBUTTON:
               g_wpEnvironment.stSystem.fButtonAppearanceHide = (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1);
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_MINIMIZEBUTTON:
               g_wpEnvironment.stSystem.fButtonAppearanceHide = (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1);
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_ENABLED:
               g_wpEnvironment.stSystem.fWindowAnimationEnabled = TRUE;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_DISABLED:
               g_wpEnvironment.stSystem.fWindowAnimationEnabled = FALSE;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_MINHIDE:
               g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_HIDE;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_MINTOVIEWER:
               g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_VIEWER;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_MINTODESKTOP:
               g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_DESKTOP;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_DISPLAY:
               g_wpEnvironment.stSystem.wOpenBehavior = OPENBEHAVIOR_DISPLAY;
               UpdateWindowPageControls(hwndPage);
               break;

            case IDB_CREATE:
               g_wpEnvironment.stSystem.wOpenBehavior = OPENBEHAVIOR_CREATE;
               UpdateWindowPageControls(hwndPage);
               break;
         }
         break;

      case PAGE_DONE:
         // Write out current "Window" page state.
         UpdateWindowPageSYSTEMProfile(hwndPage);
         break;
   }
}


//---------------------------------------------------------------------
// UpdateWindow2PageSYSTEMProfile():
//---------------------------------------------------------------------
void WPSYSSETUP::UpdateWindow2PageSYSTEMProfile(HWND hwndPage)
{
   // "Folder Automatic Close" system property.
   if (IsDlgButtonChecked(hwndPage,IDB_NEVER)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE),GetString(IDSTR_NEVER),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
   if (IsDlgButtonChecked(hwndPage,IDB_SUBFOLDERS_ONLY)==1)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE),GetString(IDSTR_SUBFOLDERS_ONLY),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else                                                               
      // Assume that "All Objects" is currently checked. 
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE),GetString(IDSTR_ALL_OBJECTS),GetString(IDSTR_WIN_SYSTEM_PROFILE));
}


//---------------------------------------------------------------------
// GetWindow2PageSYSTEMProfile():
//---------------------------------------------------------------------
void WPSYSSETUP::GetWindow2PageSYSTEMProfile(void)
{
   char szValue[64];

   // Load default "Folder Automatic Close" system property value (Never, Subfolders Only, or All Objects).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE),GetString(IDSTR_NEVER),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_SUBFOLDERS_ONLY))==0)
      g_wpEnvironment.stSystem.wFolderAutomaticClose = SUBFOLDERS_ONLY;
   else
   if (stricmp(szValue,GetString(IDSTR_ALL_OBJECTS))==0)
      g_wpEnvironment.stSystem.wFolderAutomaticClose = ALL_OBJECTS;
   else      
      g_wpEnvironment.stSystem.wFolderAutomaticClose = NEVER;
}


//---------------------------------------------------------------------
// UpdateWindow2PageControls():
//---------------------------------------------------------------------
void WPSYSSETUP::UpdateWindow2PageControls(HWND hwndPage)
{
   WORD  wControlID;
   
   // Depending on what state the "Folder Automatic Close" system property is in...         
   switch (g_wpEnvironment.stSystem.wFolderAutomaticClose)
   {  
      default:
      case NEVER:
         wControlID = IDB_NEVER;
         break;
               
      case SUBFOLDERS_ONLY:
         wControlID = IDB_SUBFOLDERS_ONLY;
         break;
               
      case ALL_OBJECTS:
         wControlID = IDB_ALLOBJECTS;
         break;
   }
   // Initialize "Folder Automatic Close" group radio buttons.
   CheckRadioButton(hwndPage,IDB_NEVER,IDB_ALLOBJECTS,wControlID);
}


//---------------------------------------------------------------------
// Window2PageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPSYSSETUP::Window2PageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{        
   WPOBJECT       *pwpObjectFound;
   WPDESKTOP      *pwpDesktop;
   MATCH_RECORD   *pMatchList=NULL,*pMatchRecord;
   DWORD          dwFlags;

   // Depending on what action this page is being notified on...
   switch (wAction)
   {           
      case PAGE_CREATE:
         // Initialize radio buttons.
         UpdateWindow2PageControls(hwndPage);
         break;
         
      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT: 
               // Uncheck "Propagate changes" checkbox.
               CheckDlgButton(hwndPage,IDB_PROPAGATECHANGES,0);
               // Make folder close property "Never".
               g_wpEnvironment.stSystem.wFolderAutomaticClose = NEVER;
               UpdateWindow2PageControls(hwndPage);
               break;
               
            case IDB_UNDO:
               CheckDlgButton(hwndPage,IDB_PROPAGATECHANGES,0);
               // Read current folder close property.
               GetWindow2PageSYSTEMProfile();          
               // Change page radio buttons to reflect current settings.
               UpdateWindow2PageControls(hwndPage);
               break;

            case IDB_HELP:
               // Open help file with "Window" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SYSTEM_SETNB_WINDOWPAGE);
               break;
               
            case IDB_NEVER:
               g_wpEnvironment.stSystem.wFolderAutomaticClose = NEVER;
               break;
               
            case IDB_SUBFOLDERS_ONLY:
               g_wpEnvironment.stSystem.wFolderAutomaticClose = SUBFOLDERS_ONLY;
               break;
               
            case IDB_ALLOBJECTS:
               g_wpEnvironment.stSystem.wFolderAutomaticClose = ALL_OBJECTS;
               break;
         }  
         break;

      case PAGE_DONE:
         // Check if "propagate changed" checkbox is selected...
         if (IsDlgButtonChecked(hwndPage,IDB_PROPAGATECHANGES)==1)
         {     
            // Query current 'Window' page settings from global variables.      
            dwFlags = 0L;
            // 'Minimized button behavior' properties.   
            dwFlags |= (DWORD)g_wpEnvironment.stSystem.wMinButtonBehavior;
            // 'Open behavior' properties.
            dwFlags |= (DWORD)g_wpEnvironment.stSystem.wOpenBehavior;
            
            // Get pointer to desktop object.            
            pwpDesktop = (WPDESKTOP *)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_QUERY_FOLDER_POINTER,0,0L);
            // Get list of all program objects in system.
            pwpObjectFound = pwpDesktop->Find(OBJTYPE_PROGRAM,"",FALSE,FALSE,&(pMatchList));
            // While there are more match records in list...
            while (pMatchList)
            {                  
               // Propagate 'Window' settings to program object.
               pMatchList->m_pwpobjMatch->SetFlagsOR(dwFlags,(MINBEHAVIOR_FLAGS_MASK|OPENBEHAVIOR_FLAGS_MASK));
               // Move to next match record and delete current one.
               pMatchRecord = pMatchList->m_pNextRecord;
               delete pMatchList;
               pMatchList = pMatchRecord;
            }              
         }
         // Write out current "Window" page #2 state.
         UpdateWindow2PageSYSTEMProfile(hwndPage);
         break;
   }
}


//---------------------------------------------------------------------
// UpdateConfirmationsPageSYSTEMProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPSYSSETUP::UpdateConfirmationsPageSYSTEMProfile(HWND hwndPage)
{
   // Confirm on folder delete.
   if (g_wpEnvironment.stSystem.fConfirmOnFolderDelete==TRUE)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFOLDERDELETE),GetString(IDSTR_YES),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFOLDERDELETE),GetString(IDSTR_NO),GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // Confirm on delete.
   if (g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMDELETE),GetString(IDSTR_YES),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMDELETE),GetString(IDSTR_NO),GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // Confirm on file system delete.
   if (g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete==TRUE)
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFILEDELETE),GetString(IDSTR_YES),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   else
      WritePrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFILEDELETE),GetString(IDSTR_NO),GetString(IDSTR_WIN_SYSTEM_PROFILE));

}


//---------------------------------------------------------------------
// GetConfirmationsPageSYSTEMProfile():
//---------------------------------------------------------------------
void WPSYSSETUP::GetConfirmationsPageSYSTEMProfile(void)
{
   char szValue[64];

   // Load default confirm on folder delete operation flag (yes/no).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFOLDERDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnFolderDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnFolderDelete = FALSE;

   // Load default confirm on delete operations flag (yes/no).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnDelete = FALSE;

   // Load default confirm on folder delete operation flag (yes/no).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFILEDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete = FALSE;
}


//---------------------------------------------------------------------
// UpdateConfirmationsPageControls():
//---------------------------------------------------------------------
void WPSYSSETUP::UpdateConfirmationsPageControls(HWND hwndPage)
{
   // Confirm on delete checkbox.
   if (g_wpEnvironment.stSystem.fConfirmOnFolderDelete==TRUE)
      CheckDlgButton(hwndPage,IDB_FOLDER_DELETE,1);
   else
      CheckDlgButton(hwndPage,IDB_FOLDER_DELETE,0);

   // Confirm on delete checkbox.
   if (g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)
      CheckDlgButton(hwndPage,IDB_DELETE,1);
   else
      CheckDlgButton(hwndPage,IDB_DELETE,0);

   // Confirm on delete checkbox.
   if (g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete==TRUE)
      CheckDlgButton(hwndPage,IDB_FILE_SYSTEM_DELETE,1);
   else
      CheckDlgButton(hwndPage,IDB_FILE_SYSTEM_DELETE,0);
}


//---------------------------------------------------------------------
// ConfirmationsPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPSYSSETUP::ConfirmationsPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize checkboxes.
         UpdateConfirmationsPageControls(hwndPage);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Reset checkboxes to default values.
               g_wpEnvironment.stSystem.fConfirmOnFolderDelete       = TRUE;
               g_wpEnvironment.stSystem.fConfirmOnDelete             = TRUE;
               g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete   = TRUE;
               // Update confirmations page.
               UpdateConfirmationsPageControls(hwndPage);
               break;

            case IDB_UNDO:
               // Get current values from system profile.
               GetConfirmationsPageSYSTEMProfile();
               // Update confirmations page.
               UpdateConfirmationsPageControls(hwndPage);
               break;

            case IDB_HELP:
               // Open help file with "Confirmations" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SYSTEM_SETNB_CONFIRMATIONSPAGE);
               break;

            case IDB_FOLDER_DELETE:
               // User clicked on checkbox, update confirm on folder delete flag.
               g_wpEnvironment.stSystem.fConfirmOnFolderDelete = (IsDlgButtonChecked(hwndPage,IDB_FOLDER_DELETE)==1);
               break;

            case IDB_DELETE:
               // User clicked on checkbox, update confirm on delete flag.
               g_wpEnvironment.stSystem.fConfirmOnDelete = (IsDlgButtonChecked(hwndPage,IDB_DELETE)==1);
               break;

            case IDB_FILE_SYSTEM_DELETE:
               // User clicked on checkbox, update confirm on folder delete flag.
               g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete = (IsDlgButtonChecked(hwndPage,IDB_FILE_SYSTEM_DELETE)==1);
               break;
         }
         break;

      case PAGE_DONE:
         // Update system profile with current confirmation settings.
         UpdateConfirmationsPageSYSTEMProfile(hwndPage);
         break;
   }
}

// EOF WPSYSSET.CPP


