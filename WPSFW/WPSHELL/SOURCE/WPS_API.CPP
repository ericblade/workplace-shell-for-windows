//=====================================================================
// WPS_API.CPP
//    Workplace Shell for Windows API module to external client apps.
//
// Date Created :  08/26/95
// Last Update  :  10/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 08/26/95    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <ctype.h>
// Workplace Shell API header file.
#include "wps_api.h"
   
//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static LIST gs_RegSourceWndList,gs_RegTargetWndList;
                     
                     
//---------------------------------------------------------------------
// INTERNAL FUNCTIONS (USED BY WORKPLACE SHELL)
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// IsRegisteredSourceWindow():
//---------------------------------------------------------------------
BOOL IsRegisteredSourceWindow(HWND hwndSource)
{
   // Search list of registered source windows for match...
   if (gs_RegSourceWndList.Find(MAKELONG((WORD)hwndSource,0))!=-1)
      return (TRUE);    // Found match.
   else
      return (FALSE);   // No window found.
}

//---------------------------------------------------------------------
// IsRegisteredTargetWindow():
//---------------------------------------------------------------------
BOOL IsRegisteredTargetWindow(HWND hwndTarget)
{                        
   #ifdef _DEBUG
   //dprintf("tr IsRegisteredTargetWindow(): Target Wnd 0x%04X\n",(WORD)hwndTarget);
   #endif
   // Search list of registered target windows for match...
   if (gs_RegTargetWndList.Find(MAKELONG((WORD)hwndTarget,0))!=-1)
      return (TRUE);    // Found match.
   else
      return (FALSE);   // No window found.
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
                     
     
//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS
//---------------------------------------------------------------------
     
//---------------------------------------------------------------------
// WpsRegisterWindow():
//---------------------------------------------------------------------
extern "C" BOOL CALLBACK _export WpsRegisterWindow(HWND hwnd,WORD wFlags)
{     
   #ifdef _DEBUG
   //dprintf("tr WpsRegisterWindow(): Target Wnd 0x%04X\n",(WORD)hwnd);
   #endif

   // Make sure that window being passed is a valid window.
   if (IsWindow(hwnd)==FALSE)
      return (FALSE);
      
   // If window is being registered as drag & drop source window...
   if (wFlags & WNDTYPE_SOURCE)
   {
      // Add to registered source window list.
      if (gs_RegSourceWndList.AddTail(MAKELONG((WORD)hwnd,0))==FALSE)
         return (FALSE);
   }
        
   // If window is being registered as drag & drop target window...
   if (wFlags & WNDTYPE_TARGET)
   {
      // Add to registered target window list.
      if (gs_RegTargetWndList.AddTail(MAKELONG((WORD)hwnd,0))==FALSE)
         return (FALSE);
   }                           
   
   // Success adding window to registered list.
   return (TRUE);
}


//---------------------------------------------------------------------
// WpsCommand():
//---------------------------------------------------------------------
extern "C" DWORD CALLBACK _export WpsCommand(WORD wCommand,DWORD dwData1,DWORD dwData2)
{        
   DWORD       dwLocalData;
   WPOBJECT    *pwpObject;
   WPSOBJINFO  *pObjectInfo;              

   // Depending on the command requested...
   switch (wCommand)
   {
      case CMDTYPE_GETDESKTOPHANDLE:
         // Get Workplace Shell desktop object's pointer.
         dwLocalData = (DWORD)SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_QUERY_FOLDER_POINTER,0,0L);
         // Return it as an object handle.
         return (dwLocalData);
         break;
         
      case CMDTYPE_GETCHILDCOUNT:
         // Make sure that a valid pointer was passed...
         if (IsBadReadPtr((const void FAR *)dwData1,1)==0)
         {  
            // Cast pointer to generic object record.
            pwpObject = (WPOBJECT *)dwData1;
            // Make sure that object is a container object.
            if (pwpObject->isContainer()==TRUE)
               // Return number of children objects that belong to this container.
               return ((DWORD)((WPCONTAINER *)pwpObject)->GetObjectCount());
         }
         return ((DWORD)-1L);
      
      case CMDTYPE_GETCHILDHANDLE:
         // Make sure that a valid pointer was passed...
         if (IsBadReadPtr((const void FAR *)dwData1,1)==0)
         {  
            // Cast pointer to generic object record.
            pwpObject = (WPOBJECT *)dwData1;
            // Make sure that object is a container object.
            if (pwpObject->isContainer()==TRUE)
               // Return the object handle for the child object requested. 
               return ((DWORD)((WPCONTAINER *)pwpObject)->GetObject((int)LOWORD(dwData2)));
         }
         break;
      
      case CMDTYPE_GETOBJECTINFO:
         // Make sure that a valid pointer was passed...
         if (IsBadReadPtr((const void FAR *)dwData1,1)==0)
         {  
            // Cast pointer to generic object record.
            pwpObject = (WPOBJECT *)dwData1;
            // We assume that caller has allocated global shared memory with
            // enough space to hold all the object's information.
            // First, lock global memory handle to created a pointer.
            pObjectInfo = (WPSOBJINFO *)GlobalLock((HGLOBAL)LOWORD(dwData2));
            if (pObjectInfo!=NULL)
            {
               // Now, we are ready to copy object information into structure.
               pObjectInfo->m_hObject = dwData1;
               pObjectInfo->m_dwType  = pwpObject->GetType();
               strcpy(pObjectInfo->m_szTitle,pwpObject->GetTitle());
               pwpObject->GetIcon(pObjectInfo->m_szIconPath,&(pObjectInfo->m_wIconIndex));
               // We are done, unlock global memory pointer.
               GlobalUnlock((HGLOBAL)LOWORD(dwData2));
               // Successfully completed command, return success.
               return ((DWORD)TRUE);
            }
         }
         break;
     
   }
   
   // Default return value.
   return (0L);
}


//---------------------------------------------------------------------
// WpsUnregisterWindow():
//---------------------------------------------------------------------
extern "C" BOOL CALLBACK _export WpsUnregisterWindow(HWND hwnd)
{
   // Remove from registered source window list (if applicable).
   gs_RegSourceWndList.Remove(MAKELONG((WORD)hwnd,0));
   
   // Remove from registered target window list (if applicable).
   gs_RegTargetWndList.Remove(MAKELONG((WORD)hwnd,0));
   
   // Success removing window from registered lists.
   return (TRUE);
}


// EOF WPS_API.CPP

