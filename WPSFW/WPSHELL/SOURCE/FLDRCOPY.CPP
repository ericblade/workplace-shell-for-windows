//=====================================================================
// FLDRCOPY.CPP
//    WPFOLDER's "Copy...", "Move...", "Create another...", "Create
// shadow" function dialog procedures and misc. support functions.
// 
// Date Created :  08/07/94
// Last Update  :  12/12/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 08/07/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"                 
// Windows header files.
#include <windowsx.h>
// C-runtime library header files.
#include <string.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------

         
//---------------------------------------------------------------------
// InitializeTargetFolderListbox():
//    This function initializes the listbox with a sorted list of all
// potential and valid target folders for operation.  The listbox 
// entries will have the folder's title and the folder's parent folder
// title in parenthesis.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void InitializeTargetFolderListbox(HWND hDlg,COPYMOVECREATE_STRUCT *pcmcsDlgInfo)
{  
   WPDESKTOP      *pwpDesktop;
   MATCH_RECORD   *pMatchList=NULL,*pMatchRecord;
   char           szText[128],szTempTitle[MAX_TITLE_NAME+1];
   int            nIndex;
   
   // Get pointer to desktop object.            
   pwpDesktop = (WPDESKTOP *)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_QUERY_FOLDER_POINTER,0,0L);
   // Find all folders available.
   pwpDesktop->Find(OBJTYPE_FOLDER,"",FALSE,FALSE,&(pMatchList));
   // While there are more match records in list...
   while (pMatchList)
   {                  
      // If object in match record is a folder object...
      if (pMatchList->m_pwpobjMatch->GetType()==OBJTYPE_FOLDER)
      {                       
         // Create listbox item string by using folder's name and parent folder's name.
         strcpy(szTempTitle,pMatchList->m_pwpobjMatch->GetTitleNoCRLF());
         sprintf(szText,"%s (%s)",szTempTitle,pMatchList->m_pwpobjMatch->GetParent()->GetTitleNoCRLF());
         // Insert folder name into listbox.
         nIndex = ListBox_AddString(GetDlgItem(hDlg,IDL_TARGETFOLDER),(LPSTR)szText);
         // Store pointer to target folder object in listbox item data field.
         ListBox_SetItemData(GetDlgItem(hDlg,IDL_TARGETFOLDER),nIndex,(LPVOID)pMatchList->m_pwpobjMatch);
      }
      // Move to next match record and delete current one.
      pMatchRecord = pMatchList->m_pNextRecord;
      delete pMatchList;
      pMatchList = pMatchRecord;
   }              
   
   // Insert "Desktop" string into listbox.
   nIndex = ListBox_AddString(GetDlgItem(hDlg,IDL_TARGETFOLDER),(LPSTR)GetString(IDSTR_OBJTITLE_DESKTOP));
   // Store pointer to Desktop in listbox item data field.
   ListBox_SetItemData(GetDlgItem(hDlg,IDL_TARGETFOLDER),nIndex,(LPVOID)pwpDesktop);
   
   // Select first folder in list by default.
   ListBox_SetCurSel(GetDlgItem(hDlg,IDL_TARGETFOLDER),0);
}
               
//---------------------------------------------------------------------
// IsTargetFolderValid():
//    This function validates a specific folder as a valid target of
// the wanted operation (copy, move, create, etc...).
//---------------------------------------------------------------------
BOOL IsTargetFolderValid(WPFOLDER *pwpfolderTarget,COPYMOVECREATE_STRUCT *pcmcsDlgInfo)
{      
   int index,nFunc;
   
   // Depending on which function type requested...   
   switch (pcmcsDlgInfo->m_wFuncType)
   {             
      case IDM_CREATEANOTHER:
         nFunc = DROP_COPY;
         break;
         
      case IDM_CREATESHADOW:
         nFunc = DROP_COPY;
         break;
         
      case IDM_COPY:
         nFunc = DROP_COPY;
         break;
      
      case IDM_MOVE:          
         nFunc = DROP_MOVE;
         break;
         
      default:          
         // No other function types allowed.
         return (FALSE);
   }
   
   // If operation is being done on one or more objects...
   if (pcmcsDlgInfo->m_fSingleObject==TRUE)
      // Test if target folder supports the source object.
      return (pwpfolderTarget->TestDropTarget(pcmcsDlgInfo->m_pwpfolderParent->GetObjectPopup(),nFunc));
   else
   {           
      // Test the target folder against all of the source objects...
      for (index=0; index<pcmcsDlgInfo->m_pwpfolderParent->GetSelectedObjectCount(); index++)
      {  
         // Test target folder with current selected object...
         if (pwpfolderTarget->TestDropTarget(pcmcsDlgInfo->m_pwpfolderParent->GetSelectedObject(index),nFunc)==FALSE)
            // Cannot do operation on target folder.
            return (FALSE);
      }   
   }
   
   // "wFuncType" operation is valid on target folder.
   return (TRUE);
}

               
//---------------------------------------------------------------------
// CopyMoveAndCreateDlgProc():                            
//    This dialog handles the "Copy", "Move", "Create another", and
// "Create shadow" functions for any object within a folder.
//---------------------------------------------------------------------
BOOL CALLBACK CopyMoveAndCreateDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{        
   RECT                    rcDialog;
   COPYMOVECREATE_STRUCT   *pcmcsDlgInfo;
   char                    szText[128];
   int                     nIndex;
     
   // Process dialog box message...
   switch (uMsg)
   {
      case WM_INITDIALOG:      
         // Get dialog box's size.                        
         GetWindowRect(hDlg,&rcDialog);
         // Center dialog box with respect to the screen.
         MoveWindow(hDlg,(GetSystemMetrics(SM_CXSCREEN)-rcDialog.right)/2,
                         (GetSystemMetrics(SM_CYSCREEN)-rcDialog.bottom)/2,
                         rcDialog.right,
                         rcDialog.bottom,FALSE);
         // Store pointer to dialog structure which is in 'lParam'.
         SetWindowLong(hDlg,DWL_USER,(DWORD)lParam);
         // Type cast 'lParam' into local pointer variable (for convenience).
         pcmcsDlgInfo = (COPYMOVECREATE_STRUCT *)(LPVOID)lParam;
         // If folder object has a title caption...
         if (strlen(pcmcsDlgInfo->m_pwpfolderParent->GetTitleNoCRLF())>0)
            strcpy(szText,pcmcsDlgInfo->m_pwpfolderParent->GetTitleNoCRLF());
         else
            // Default to "Desktop" if folder ha no name.
            strcpy(szText,GetString(IDSTR_OBJTITLE_DESKTOP));
         // Create title string and set it to dialog caption.
         strcat(szText," - ");
         
         // Depending on the function type...
         switch (pcmcsDlgInfo->m_wFuncType)
         {  
            case IDM_CREATEANOTHER:
               // Append function type to dialog caption text string.
               strcat(szText,GetString(IDSTR_CMCDLG_TITLE_CREATEANOTHER));
               // Set caption text for 'OK' pushbutton.
               SetDlgItemText(hDlg,IDOK,GetString(IDSTR_CMCDLG_PUSHBUTTON_CREATE));
               break;
            
            case IDM_CREATESHADOW:
               // Append function type to dialog caption text string.
               strcat(szText,GetString(IDSTR_CMCDLG_TITLE_CREATESHADOW));
               // Set caption text for 'OK' pushbutton.
               SetDlgItemText(hDlg,IDOK,GetString(IDSTR_CMCDLG_PUSHBUTTON_SHADOW));
               break;
         
            case IDM_COPY:
               // Append function type to dialog caption text string.
               strcat(szText,GetString(IDSTR_CMCDLG_TITLE_COPY));
               // Set caption text for 'OK' pushbutton.
               SetDlgItemText(hDlg,IDOK,GetString(IDSTR_CMCDLG_PUSHBUTTON_COPY));
               break;
            
            case IDM_MOVE:
               // Append function type to dialog caption text string.
               strcat(szText,GetString(IDSTR_CMCDLG_TITLE_MOVE));
               // Set caption text for 'OK' pushbutton.
               SetDlgItemText(hDlg,IDOK,GetString(IDSTR_CMCDLG_PUSHBUTTON_MOVE));
               break;
               
            default:
               // Else, function not supported, exit immediatedly.
               EndDialog(hDlg,FALSE);
               return (TRUE);
         }         
         
         // Set dialog caption text to create string.
         SetWindowText(hDlg,szText);                
         // Intialize target folder listbox control.
         InitializeTargetFolderListbox(hDlg,pcmcsDlgInfo);
         // If operation is going to be on a single object AND we are not creating a shadow object(s)...
         if ((pcmcsDlgInfo->m_fSingleObject==TRUE)&&(pcmcsDlgInfo->m_wFuncType!=IDM_CREATESHADOW))
         {
            // Initialize "New Name" edit control.
            SetDlgItemText(hDlg,IDE_NEWNAME,pcmcsDlgInfo->m_szNewName);
            // Give focus to edit control.
            SetFocus(GetDlgItem(hDlg,IDE_NEWNAME));
         }  
         else
         {
            // Hide static text and edit control since they are not used
            // because there are multiple objects being operated on.
            ShowWindow(GetDlgItem(hDlg,IDT_NEWNAME),SW_HIDE);
            ShowWindow(GetDlgItem(hDlg,IDE_NEWNAME),SW_HIDE);
            // Set focus to target folder listbox control.
            SetFocus(GetDlgItem(hDlg,IDL_TARGETFOLDER));
         }
         return (FALSE);
      
      case WM_COMMAND:
         // Depending on the control...
         switch (wParam)
         {
            case IDL_TARGETFOLDER:
               // If user double-clicked an item in the listbox,...
               if (HIWORD(lParam)==LBN_DBLCLK)
               {  
                  // Act as if user had pressed 'OK' pushbutton.
                  PostMessage(hDlg,WM_COMMAND,(WPARAM)IDOK,0L);
               }
               else  
               // If listbox selection is about to change,...
               if (HIWORD(lParam)==LBN_SELCHANGE)
               {  
                  // Get the new listbox selection index.
                  nIndex = ListBox_GetCurSel(GetDlgItem(hDlg,IDL_TARGETFOLDER));
                  // Check if new listbox selection of a target folder is valid for operation...               
                  if (IsTargetFolderValid(
                         (WPFOLDER *)ListBox_GetItemData(GetDlgItem(hDlg,IDL_TARGETFOLDER),nIndex),
                         (COPYMOVECREATE_STRUCT *)GetWindowLong(hDlg,DWL_USER))==FALSE)
                  {
                     // Disable 'OK' pushbutton since operation is not valid.
                     EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
                  }  
                  else  
                     // Enable 'OK' pushbutton since operation is valid.
                     EnableWindow(GetDlgItem(hDlg,IDOK),TRUE);
               }
               break;
         
            case IDOK:  
               // If 'OK' pushbutton is disabled, then skip processing and ignore.
               if (IsWindowEnabled(GetDlgItem(hDlg,IDOK))==FALSE)
                  break;
               // Get current item selected in target folders listbox.
               nIndex = ListBox_GetCurSel(GetDlgItem(hDlg,IDL_TARGETFOLDER));
               // If there is a valid item currently selected in listbox...
               if (nIndex!=LB_ERR)
               {                  
                  // Retrieve pointer to dialog structure and store it in local variable.
                  pcmcsDlgInfo = (COPYMOVECREATE_STRUCT *)GetWindowLong(hDlg,DWL_USER);
                  // If edit field is visible, then get text from edit field...
                  if (IsWindowVisible(GetDlgItem(hDlg,IDE_NEWNAME))==TRUE)
                     // Copy text from edit control into string.
                     GetDlgItemText(hDlg,IDE_NEWNAME,pcmcsDlgInfo->m_szNewName,MAX_TITLE_NAME);
                  // Get pointer to folder object from listbox item's private data.
                  pcmcsDlgInfo->m_pwpfolderTarget = (WPFOLDER *)ListBox_GetItemData(GetDlgItem(hDlg,IDL_TARGETFOLDER),nIndex);
               }
               // Handle the 'OK' button...
               EndDialog(hDlg,TRUE);
               break;
               
            case IDCANCEL:     
               // Returning 'FALSE' means 'Cancel' button pressed.
               EndDialog(hDlg,FALSE);
               break;
               
            case IDB_HELP:
               // Retrieve pointer to dialog structure and store it in local variable.
               pcmcsDlgInfo = (COPYMOVECREATE_STRUCT *)GetWindowLong(hDlg,DWL_USER);
               // Depending on what function is being performed by dialog...
               switch (pcmcsDlgInfo->m_wFuncType)
               {
                  case IDM_CREATEANOTHER:         
                     // Open help file with "Create another..." page help information.
                     WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_LEVEL1_MCREATEANOTHER);
                     break;
                     
                  case IDM_CREATESHADOW:
                     // Open help file with "Create shadow..." page help information.
                     WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_LEVEL1_MCREATESHADOW);
                     break;

                  case IDM_COPY:
                     // Open help file with "Copy..." page help information.
                     WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_LEVEL1_MCOPY);
                     break;
                     
                  case IDM_MOVE:
                     // Open help file with "Move..." page help information.
                     WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_LEVEL1_MMOVE);
                     break;
               }
               break;
         }                   
         break;
   }
     
   return (FALSE);
} 

// EOF FLDRCOPY.CPP

