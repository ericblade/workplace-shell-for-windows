//=====================================================================
// FLDRFIND.CPP
//    WPFOLDER "Find..." function dialog procedures and misc. 
// support functions.
// 
// Date Created :  08/03/94
// Last Update  :  08/06/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 08/03/94    | Creation.                               | MDA
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


//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FindResultsMarkSelectedObjects():
//---------------------------------------------------------------------
void FindResultsMarkSelectedObjects(HWND hDlg,WORD wListboxID)
{     
   MATCH_RECORD   *pMatchRecord;
   int            nCount,*panIndex,index;
   
   // Get total number of selected items.
   nCount = (int)SendDlgItemMessage(hDlg,wListboxID,LB_GETSELCOUNT,0,0L);
   // If there are no selected objects, then exit immediatedly.
   if ((nCount==0)||(nCount==LB_ERR))
      return;
      
   // Allocate memory for array of integer indexes.
   panIndex = new int[nCount];
   // Get indexes of objects selected in listbox.
   SendDlgItemMessage(hDlg,wListboxID,LB_GETSELITEMS,(WPARAM)nCount,(LPARAM)(LPVOID)panIndex);
   // For each object selected in listbox, set "m_fShow" flag to TRUE.
   for (index=0; index<nCount; index++)
   {                                                                   
      // Get pointer to match record stored in listbox entry's item data.
      pMatchRecord = (MATCH_RECORD *)SendDlgItemMessage(hDlg,wListboxID,LB_GETITEMDATA,(WPARAM)panIndex[index],0L);
      // Set "show" flag to TRUE (so that object will be made visible).
      pMatchRecord->m_fShow = TRUE;
   }                               
   // Deallocate memory used to store array of integers.
   delete[] panIndex;
}


//---------------------------------------------------------------------
// ResultsDlgProc():
//---------------------------------------------------------------------
BOOL CALLBACK ResultsDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   RECT           rcDialog;
   MATCH_RECORD   *pMatchList;
   WORD           wListboxID,wIndex;
   char           szTitle[32],szText[64];
   int            nMatchCount;
   
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
         // Type cast 'lParam' value to pointer to head of match list.                        
         pMatchList = (MATCH_RECORD *)(LPVOID)lParam;
         // Initialize count of match records in list.
         nMatchCount = 0;
         // Initialize object listboxes with match list contents...
         while (pMatchList!=NULL)
         {  
            // Depending on the object type in current match record...
            switch (pMatchList->m_pwpobjMatch->GetType())
            {
               case OBJTYPE_PROGRAM:
                  wListboxID = IDL_PROGRAMLIST;
                  break;
               case OBJTYPE_DATAFILE:
                  wListboxID = IDL_DATAFILELIST;
                  break;
               case OBJTYPE_FOLDER:
                  wListboxID = IDL_FOLDERLIST;
                  break;                 
               default:
                  // Get pointer to next match record and ignore current match record.
                  pMatchList = pMatchList->m_pNextRecord;
                  continue;
            }     
            // Increment count of number of match records in list (used later).
            nMatchCount++;
            // Now, insert object title into respective listbox.
            wIndex = (WORD)SendDlgItemMessage(hDlg,wListboxID,LB_ADDSTRING,0,(LPARAM)(LPSTR)pMatchList->m_pwpobjMatch->GetTitleNoCRLF());
            // Store pointer to match record in listbox item data field.
            SendDlgItemMessage(hDlg,wListboxID,LB_SETITEMDATA,wIndex,(LPARAM)(LPSTR)pMatchList);
            // Get pointer to next match record.
            pMatchList = pMatchList->m_pNextRecord;
         }     
         // Before returning, append the number of matches found to caption title.
         GetWindowText(hDlg,szTitle,sizeof(szTitle));
         sprintf(szText,GetString(IDSTR_FINDRESULTS_CAPTION),szTitle,nMatchCount);
         SetWindowText(hDlg,szText);
         return (TRUE);

      case WM_COMMAND:
         // Depending on the control...
         switch (wParam)
         {
            case IDOK:
               // Query which object should be showed to the user.
               FindResultsMarkSelectedObjects(hDlg,IDL_PROGRAMLIST);
               FindResultsMarkSelectedObjects(hDlg,IDL_DATAFILELIST);
               FindResultsMarkSelectedObjects(hDlg,IDL_FOLDERLIST);
               // Returning a 'TRUE' means 'OK' button pressed.
               EndDialog(hDlg,TRUE);
               break;

            case IDCANCEL:     
               // Returning 'FALSE' means 'Cancel' button pressed.
               EndDialog(hDlg,FALSE);
               break;
         }
         break;
   }         
   
   return (FALSE);
}


//---------------------------------------------------------------------
// BringObjectToTop():
//---------------------------------------------------------------------
void BringObjectToTop(WPOBJECT *pwpobjObject)
{
   // Open parent folder so that the user can see the object.
   pwpobjObject->GetParent()->Open();        
   
   // Unselect any objects currently selected.             
   // NOTE: Nasty side-effect that clears any previously hilighted objects.
   //       Therefore, it clears any previouly objects that are part of the 
   //       "Find..." result.  So, for now, it has been commented out.
   //pwpobjObject->GetParent()->ClearSelectedObjects(TRUE);
   
   // Select the object so that object is hilighted by default.
   pwpobjObject->GetParent()->AddSelectedObject(pwpobjObject);
   // Update visual state of object so it will show selected.         
   pwpobjObject->InvalidateIcon(TRUE,TRUE);
}
   
   
//---------------------------------------------------------------------
// FindDlgOnCommandOK():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void FindDlgOnCommandOK(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
   WPFOLDER       *pwpfolFolder;
   WPOBJECT       *pwpObjectFound;
   DLGPROC        lpfnResultsDlgProc;
   BOOL           fExact,fStopOnMatch,fReturnCode; 
   MATCH_RECORD   *pMatchList,*pMatchRecord;
   char           szSearchText[MAX_TITLE_NAME+1],szText[128];

   // Get pointer to caller folder object.
   pwpfolFolder = (WPFOLDER *)(LPVOID)GetWindowLong(hDlg,DWL_USER);
   
   // Get current text in edit field control.
   GetDlgItemText(hDlg,IDE_TITLETEXT,szSearchText,MAX_TITLE_NAME);
   // Check if user wants an exact match of the text entered.
   if (IsDlgButtonChecked(hDlg,IDB_EXACTMATCH)==1) 
      fExact = TRUE; 
   else                                            
      fExact = FALSE;

   // Check if user wants to stop on the first object found.
   if (IsDlgButtonChecked(hDlg,IDB_STOPONMATCH)==1) 
      fStopOnMatch = TRUE; 
   else                                            
      fStopOnMatch = FALSE;

   // Initialize local variables. 
   pwpObjectFound = NULL;
   pMatchList     = NULL;
                         
   if (IsDlgButtonChecked(hDlg,IDB_PROGRAM)==1)
      // Search folder's domain for object match(s)...
      pwpObjectFound = pwpfolFolder->Find(OBJTYPE_PROGRAM,szSearchText,fExact,fStopOnMatch,&(pMatchList));
               
   if ((IsDlgButtonChecked(hDlg,IDB_DATAFILE)==1)&&((pwpObjectFound==NULL)||(fStopOnMatch==FALSE)))
      // Search folder's domain for object match(s)...
      pwpObjectFound = pwpfolFolder->Find(OBJTYPE_DATAFILE,szSearchText,fExact,fStopOnMatch,&(pMatchList));
               
   if ((IsDlgButtonChecked(hDlg,IDB_FOLDER)==1)&&((pwpObjectFound==NULL)||(fStopOnMatch==FALSE)))
      // Search folder's domain for object match(s)...
      pwpObjectFound = pwpfolFolder->Find(OBJTYPE_FOLDER,szSearchText,fExact,fStopOnMatch,&(pMatchList));
                  
   // If we found an object AND we want to see the first match...
   if ((fStopOnMatch==TRUE)&&(pwpObjectFound!=NULL))
   {                           
      // Hide dialog box.
      ShowWindow(hDlg,SW_HIDE);
      // Bring object to top and select it.
      BringObjectToTop(pwpObjectFound);
      // Return 'success'...
      EndDialog(hDlg,TRUE);
   }
   else
   // If we found an object AND we asked for list of objects...
   if ((fStopOnMatch==FALSE)&&(pMatchList!=NULL))
   {     
      // Make a procedure instance for the "Results" dialog procedure.
      lpfnResultsDlgProc = (DLGPROC)MakeProcInstance((FARPROC)ResultsDlgProc,g_wpEnvironment.hInst);
      // Create and display the "Results" object dialog.
      fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_FINDRESULTS),g_wpEnvironment.hwndDesktop,lpfnResultsDlgProc,(LPARAM)(LPVOID)pMatchList);
      // Free the procedure instance created for dialog.
      FreeProcInstance((FARPROC)lpfnResultsDlgProc);
                                             
      // If user pressed 'OK' pushbutton on "Results" dilaog ...
      if (fReturnCode==TRUE)
      {        
         // Hide dialog box.
         ShowWindow(hDlg,SW_HIDE);
         // Start from the head of the match list.
         pMatchRecord = pMatchList;
         // While there are more match records to test...
         while (pMatchRecord!=NULL)
         {                         
            // If user wants to "see" object on the screen...
            if (pMatchRecord->m_fShow==TRUE)
               // Bring object to top and select it.
               BringObjectToTop(pMatchRecord->m_pwpobjMatch);
               
            // Get pointer to next object in match list.   
            pMatchRecord = pMatchRecord->m_pNextRecord;
         }
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
                  
      // Return value from "Results" dialog...
      EndDialog(hDlg,fReturnCode);
   }
   else
   {  
      // Display message notifying the user that no object was found.
      sprintf(szText,"The object '%s' was not found",szSearchText);
      MessageBox(hDlg,szText,"Workplace Shell",(MB_OK|MB_ICONINFORMATION));
   }
}



//---------------------------------------------------------------------
// FindDlgProc():                                                      
//    This dialog box handles the entering the data needed to do an 
// object search and also calls the function to search the folder's 
// domain for the specified object(s).
//   If one or more objects are found to match the search criteria,
// it creates the "Results" dialog and initializes it with the search
// results (only if the "Stop on first match" checkbox is not checked).
// Otherwise, it displays the first match.
//---------------------------------------------------------------------
BOOL CALLBACK FindDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{        
   RECT           rcDialog;
   WPFOLDER       *pwpfolFolder;
   char           szText[128];
     
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
         // Store pointer to caller folder object which is in 'lParam'.
         SetWindowLong(hDlg,DWL_USER,(DWORD)lParam);
         // Type cast 'lParam' into local pointer variable (for convenience).
         pwpfolFolder = (WPFOLDER *)(LPVOID)lParam;
         // If folder object has a title caption...
         if (strlen(pwpfolFolder->GetTitleNoCRLF())>0)
         {
            // Append folder caption text to dialog caption.
            GetWindowText(hDlg,szText,MAX_TITLE_NAME);
            strcat(szText," - ");
            strcat(szText,pwpfolFolder->GetTitleNoCRLF());
            SetWindowText(hDlg,szText);                
         }
         // Initialize dialog box's checkboxes to default values.
         CheckDlgButton(hDlg,IDB_PROGRAM,1);    // check program object 
         CheckDlgButton(hDlg,IDB_DATAFILE,1);   // check date file object 
         CheckDlgButton(hDlg,IDB_FOLDER,1);     // check folder object 
         return (TRUE);
      
      case WM_COMMAND:
         // Depending on the control...
         switch (wParam)
         {
            case IDOK:           
               // Handle the 'OK' button...
               FindDlgOnCommandOK(hDlg,wParam,lParam);
               break;
               
            case IDCANCEL:     
               // Returning 'FALSE' means 'Cancel' button pressed.
               EndDialog(hDlg,FALSE);
               break;
         }                   
         break;
   }
     
   return (FALSE);
} 

// EOF FLDRFIND.CPP

