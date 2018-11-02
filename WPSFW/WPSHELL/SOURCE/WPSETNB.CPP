//=====================================================================
// WPSETNB.CPP
//    WPSETTINGSNB Class implementation.
// 
// Date Created :  03/14/94
// Last Update  :  12/09/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 03/14/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <stdlib.h>
// Windows library header files.
#include <shellapi.h>
#include <commdlg.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
BOOL CommonFileDialogBox(HWND hWnd,DWORD dwFlags,char *pszDialogTitle,char *pszFilterString,char *pszFile,char *pszFileTitle);


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// Struture used in the "Find Icon" dialog to store local information.
typedef struct {
   WPOBJECT *m_pwpobjObject;
   BOOL     m_fEditControlChanged;
   BOOL     m_fIgnoreEN_CHANGE;
} FINDDLGDATA;

//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WORKPLACE SHELL SETTINGS NOTEBOOK CLASS
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// GetCaptionTitle():
//---------------------------------------------------------------------
const char *WPSETTINGSNB::GetCaptionTitle(void)
{                            
   static char szCaptionTitle[MAX_TITLE_NAME+1];

   // Copy default title for settings notebook.   
   strcpy(szCaptionTitle,GetString(IDSTR_SETTNB_DEFAULT_TITLE));
   
   // Check if settings notebook has an owner object...
   if (GetOwner()!=NULL)
   {                                   
      // If owner object has a title, create new settings notebook title...
      if (strlen(GetOwner()->GetTitleNoCRLF())>0)
         sprintf(szCaptionTitle,GetString(IDSTR_SETTNB_TITLE),GetOwner()->GetTitleNoCRLF());
   }

   // Return caption title for settings notebook.   
   return (szCaptionTitle);
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPNOTEBOOK():
//---------------------------------------------------------------------
WPSETTINGSNB::WPSETTINGSNB(WPOBJECT *pwpobjOwner) : WPNOTEBOOK(pwpobjOwner)
{
   // Create settings notebook instance.   
}


//---------------------------------------------------------------------
// Create():
//---------------------------------------------------------------------
BOOL WPSETTINGSNB::Create(int x,int y,int cx,int cy)
{  
   // Call base class Create() function.
   return (WPNOTEBOOK::Create(x,y,cx,cy));
}

//---------------------------------------------------------------------
// UpdateNotebookIconResource():
//    This function assumes that the owner object's icon resource 
// information has already been updated, and we want to reflect the 
// changes into the settings notebook icon resource.
//---------------------------------------------------------------------
void WPSETTINGSNB::UpdateNotebookIconResource(void)
{  
   HWND  hwndPageOnTop;

   // If settings notebook does not have an onwer, then return.                       
   if (GetOwner()==NULL)
      return;                       

   // If old icon resource handle is valid, then destroy old icon resource.
   if ((m_hIcon!=NULL)||(m_hIcon!=(HICON)1))
      ::DestroyIcon(m_hIcon);
   
   // Load new instance of icon resource.
   m_hIcon = GetOwner()->ExtractIcon();
                                    
   // Query what page is on top right now (we assume that the
   // general page will be on top at the moment of this call).
   hwndPageOnTop = GetPageOnTop();
                                    
   // Invalidate the 'General' page.
   ::InvalidateRect(hwndPageOnTop,NULL,TRUE);
   ::UpdateWindow(hwndPageOnTop);
   
   // Update the non-client area of notebook.
   PostMessage(GetHandle(),WM_NCPAINT,0,0);
}

//---------------------------------------------------------------------
// InsertGeneralPage():
//---------------------------------------------------------------------
void WPSETTINGSNB::InsertGeneralPage(FARPAGEPROC fnPage,LPCSTR lpszDialogTemplateName) 
{
   // Insert "General" page.
   InsertPage(GetString(IDSTR_SETTNB_PAGE_GENERAL),(BKA_MAJOR|BKA_AUTOPAGESIZE),fnPage,NULL,lpszDialogTemplateName);
}


//---------------------------------------------------------------------
// WINDOW MESSAGE PROCESSING
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnClose():
//---------------------------------------------------------------------
void WPSETTINGSNB::OnClose(void)
{             
   // Have Workplace Shell desktop close and destroy notebook.
   SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_DESTROY_SETTINGS_NOTEBOOK,0,(DWORD)this);
}
         


//---------------------------------------------------------------------
// MISCELLANEOUS FUNCTIONS USED IN "GENERAL" PAGE
//---------------------------------------------------------------------
   
//---------------------------------------------------------------------
// InitializeIconPreviewListbox():
//---------------------------------------------------------------------
BOOL InitializeIconPreviewListbox(HWND hDlg,char *pszIconPath)
{     
   BOOL     fBuiltIn;
   HICON    *phIcon; 
   int      nIconCount,index;
   
   // Turn listbox window redraw "off" (for optimization purposes).
   SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,WM_SETREDRAW,(WPARAM)FALSE,0L);
   // Reset contents of icon preview listbox.
   SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,LB_RESETCONTENT,0,0L);
   
   // Check if icon resources are from Workplace Shell module and set boolean variable.
   fBuiltIn = (strstr(pszIconPath,GetString(IDSTR_SHELL_FILENAME))!=NULL);
   
   // Query how many icon resources are stored in the path\filename passed.        
   nIconCount = (int)ExtractIcon(g_wpEnvironment.hInst,pszIconPath,(UINT)-1);
   // If error reading icon count from file, force "no icons" available.
   if (nIconCount<0)
      nIconCount = 0;
      
   // If file actually contains at least 1 icon resource...   
   if (nIconCount>0)
   {  
      // Extract each icon resource from file and store a icon handle.
      for (index=0; index<nIconCount; index++)    
      {  
         // Allocate space for all icon handles needed.
         phIcon = new HICON;
         // Check that the memory allocation did not fail...
         if (phIcon==NULL)
            return (FALSE);
            
         // Check if file icon resources being loaded are from Workplace Shell...
         if (fBuiltIn==TRUE)                         
            // ...Then, for optimization purposes, load the icon directly from module.
            *phIcon = LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE((IDI_BASE_ID+index)));
         else
            // ...Else, use "ExtractIcon()" API to extract icon resource from file.
            *phIcon = ExtractIcon(g_wpEnvironment.hInst,pszIconPath,(UINT)index);
            
         // Add listbox icon resource item.
         SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,LB_ADDSTRING,0,(LPARAM)(LPVOID)phIcon);
      }
   }
   
   // Select first icon element in listbox.
   SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,LB_SETCURSEL,(WPARAM)0,0L);
   // Turn listbox window redraw back "on" (to refresh the listbox contents).
   SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,WM_SETREDRAW,(WPARAM)TRUE,0L);
   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// HandleSelectionState():
//    This function handles the selecting/unselecting of listbox items
// in the icon preview listbox owner-drawn functions.
//---------------------------------------------------------------------
void HandleSelectionState(LPDRAWITEMSTRUCT lpdItem)
{
   InvertRect(lpdItem->hDC,(LPRECT)&(lpdItem->rcItem));
}

//---------------------------------------------------------------------
// HandleFocusState():
//    This function handles the focus/unfocus of listbox items
// in the icon preview listbox owner-drawn functions.
//---------------------------------------------------------------------
void HandleFocusState(LPDRAWITEMSTRUCT lpdItem)
{
   DrawFocusRect(lpdItem->hDC,(LPRECT)&(lpdItem->rcItem));
}
   
   
//---------------------------------------------------------------------
// DrawIconPreviewListboxItem():
//    This function handles the drawing of all items in the icon 
// preview owner-drawn listbox.
//---------------------------------------------------------------------
void DrawIconPreviewListboxItem(LPDRAWITEMSTRUCT lpdItem)
{     
   HICON *phIcon;

   // If nothing to draw...
   if (lpdItem->itemID == -1) 
      // Return immediatedly.
      return;
   
   // Type cast pointer to icon handle into local variable (for convenience).   
   phIcon = (HICON *) lpdItem->itemData;
   // Check what type of drawing needs to be done...
   if (lpdItem->itemAction == ODA_DRAWENTIRE) 
   {
      // Draw icon resource.
      DrawIcon(lpdItem->hDC,lpdItem->rcItem.left+3,lpdItem->rcItem.top+3,*phIcon);
                      
      // If item is selected, do additional drawing -- invert rect.                      
      if (lpdItem->itemState & ODS_SELECTED)
         HandleSelectionState(lpdItem);
      
      // If item has focus, do additional drawing -- dashed border.
      if (lpdItem->itemState & ODS_FOCUS)
         HandleFocusState(lpdItem);
   }
   else                  
   if (lpdItem->itemAction & ODA_SELECT)
      HandleSelectionState(lpdItem);
   else
   if (lpdItem->itemAction & ODA_FOCUS)
      HandleFocusState(lpdItem);   
}      

//---------------------------------------------------------------------
// FindIconDlgProc():
//    This dialog procedure is used for the "Find Icon..." dialog.
//---------------------------------------------------------------------
BOOL CALLBACK FindIconDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{        
   WPOBJECT             *pwpobjObject;
   WORD                 wIconIndex;
   HICON                *phIcon;
   LPMEASUREITEMSTRUCT  lpmisIconListboxItem;
   FINDDLGDATA          *pfddFindDlgData;
   char                 szIconPath[_MAX_PATH+1],
                        szFilter[] = 
                        "Icons |*.ico;*.dll;*.exe|"
                        "All Files (*.*)|*.*|";
   
   // Process dialog box message...
   switch (uMsg)
   {
      case WM_INITDIALOG:
         // Allocate space for local "find dialog" structure.
         pfddFindDlgData = new FINDDLGDATA;         
         // Initialize local struture data.             
         pfddFindDlgData->m_pwpobjObject        = (WPOBJECT *)lParam;
         pfddFindDlgData->m_fEditControlChanged = FALSE;
         pfddFindDlgData->m_fIgnoreEN_CHANGE    = TRUE;
         // Save pointer to owner object.
         SetWindowLong(hDlg,DWL_USER,(DWORD)(LPVOID)pfddFindDlgData);
         // Initialize dialog box entry fields.
         pwpobjObject = (WPOBJECT *)lParam;                
         pwpobjObject->GetIcon(szIconPath,&wIconIndex);
         SetDlgItemText(hDlg,IDE_ICONPATH,szIconPath);
         // Initialize icon preview listbox control (owner-drawn)...
         if (InitializeIconPreviewListbox(hDlg,szIconPath)==FALSE)
            // Terminate dialog box if error occurrs.
            EndDialog(hDlg,0);      
         // Set the current selection in listbox to the current icon resource.
         SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,LB_SETCURSEL,(WPARAM)wIconIndex,0L);
         // Return "success" (actually it means that we did not set the focus).
         return (TRUE);
         
      case WM_COMMAND:
         // Depending on which button was pressed...
         switch (wParam)
         {  
            case IDE_ICONPATH:
               // If edit control's text has been modified.
               if (HIWORD(lParam)==EN_CHANGE)
               {  
                  // Extract pointer to local "find dialog" data structure.
                  pfddFindDlgData = (FINDDLGDATA *)GetWindowLong(hDlg,DWL_USER);
                  // Set "edit control has changed" flag.
                  if (pfddFindDlgData->m_fIgnoreEN_CHANGE==FALSE)
                     pfddFindDlgData->m_fEditControlChanged = TRUE;
                  else                                   
                     pfddFindDlgData->m_fIgnoreEN_CHANGE    = FALSE;
               }
               break;
            
            case IDB_BROWSE:
               // Call file dialog.
               if (CommonFileDialogBox(hDlg,(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_SHAREAWARE),GetString(IDSTR_FILEDLG_TITLE_BROWSE),szFilter,szIconPath,NULL)==TRUE)
               {                   
                  // Extract pointer to local "find dialog" data structure.
                  pfddFindDlgData = (FINDDLGDATA *)GetWindowLong(hDlg,DWL_USER);
                  // Ignore the next EN_CHANGE message generated by SetDlgItemText() call.
                  pfddFindDlgData->m_fIgnoreEN_CHANGE = TRUE;
                  // Set current path\filename to the filename the user chose in file dialog.                      
                  SetDlgItemText(hDlg,IDE_ICONPATH,szIconPath);
                  // Reinitialize icon preview list to show the new file's icon resources (if any).
                  if (InitializeIconPreviewListbox(hDlg,szIconPath)==FALSE)
                     // Terminate dialog box if error occurrs.
                     EndDialog(hDlg,0);      
               }
               break;
               
            case IDL_ICONPREVIEWLIST:
               // Icon preview listbox has sent us a notification message...
               // If message is that user has double clicked on an item in listbox...
               if (HIWORD(lParam)==LBN_DBLCLK)
               {
                  // User has double clicked on item in listbox, therefore 
                  // automatically select icon and close dialog box.
                  PostMessage(hDlg,WM_COMMAND,IDOK,MAKELPARAM(0,1));
               }
               break;
           
            case IDOK:
               // Extract pointer to local "find dialog" data structure.
               pfddFindDlgData = (FINDDLGDATA *)GetWindowLong(hDlg,DWL_USER);
               // Check if we need to update icon preview listbox...
               if (pfddFindDlgData->m_fEditControlChanged==TRUE)
               {
                  // Query the dialog box's entry field and store the current path\filename.
                  GetDlgItemText(hDlg,IDE_ICONPATH,szIconPath,MAX_PATH_FILENAME);
                  // Reinitialize icon preview list to show the new file's icon resources (if any).
                  if (InitializeIconPreviewListbox(hDlg,szIconPath)==FALSE)
                     // Terminate dialog box if error occurrs.
                     EndDialog(hDlg,0);
                  // Clear "edit control has changed" flag.
                  pfddFindDlgData->m_fEditControlChanged = FALSE;
               }
               else
               {
                  // Type cast pointer to owner object into local pointer variable.
                  pwpobjObject = (WPOBJECT *)pfddFindDlgData->m_pwpobjObject;
                  // Query the dialog box's entry field and store its information.
                  GetDlgItemText(hDlg,IDE_ICONPATH,szIconPath,MAX_PATH_FILENAME);
                  // Query the icon preview listbox and store current icon index.
                  wIconIndex = (WORD)SendDlgItemMessage(hDlg,IDL_ICONPREVIEWLIST,LB_GETCURSEL,0,0L);
                  // Save values stored in dialog box entry fields by updating the owner 
                  // object and its settings notebook.
                  pwpobjObject->UpdateIcon(szIconPath,wIconIndex);
                  pwpobjObject->GetSettingsNotebook()->UpdateNotebookIconResource();
                  // End dialog.
                  EndDialog(hDlg,0);
               }
               break;
               
            case IDCANCEL:
               // User canceled dialog box, return.
               EndDialog(hDlg,0);
               break;
         }                   
         break;        
      
      case WM_MEASUREITEM:
         // Type cast pointer to measure item structure from "lParam" (for convenience).
         lpmisIconListboxItem = ((LPMEASUREITEMSTRUCT)lParam);
         // Assign the width and height of an item in the owner-drawn listbox.
         lpmisIconListboxItem->itemWidth  = GetSystemMetrics(SM_CXICON)+6;
         lpmisIconListboxItem->itemHeight = GetSystemMetrics(SM_CYICON)+6;
         // Return success.
         return (TRUE);
                                         
      case WM_DRAWITEM:                                       
         // Draw an item in the icon preview listbox control.
         DrawIconPreviewListboxItem((LPDRAWITEMSTRUCT)lParam);
         // Return success.
         return (TRUE);
         
      case WM_DELETEITEM:
         // Type cast pointer to delete item structure "itemData" 
         // field into pointer to icon handle.
         phIcon = (HICON *)((LPDELETEITEMSTRUCT)lParam)->itemData;
         // Destroy icon resource first.         
         DestroyIcon(*phIcon);
         // Free memory allocated to hold icon handle.
         delete (WORD *)phIcon;
         // Return success.
         return (TRUE);  
         
      case WM_DESTROY:
         // Extract pointer to local "find dialog" data structure.
         pfddFindDlgData = (FINDDLGDATA *)GetWindowLong(hDlg,DWL_USER);
         // Deallocate memory used for local structure.
         delete pfddFindDlgData;
         break;
   }
     
   return (FALSE);
}


// EOF WPSETNB.CPP

