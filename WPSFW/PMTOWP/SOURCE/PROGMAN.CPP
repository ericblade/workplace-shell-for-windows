//=====================================================================
// PROGMAN.CPP
//    Program Manager DDE class implementation.
//
// Date Created :  05/06/94
// Last Update  :  05/06/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 05/06/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 header files.
#include <windows.h>
#include <ddeml.h>
// Workplace Shell header files.
#include "progman.hpp"
// C-runtime library header files.
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//---------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------
#define DDETIMEOUT   3000


//---------------------------------------------------------------------
// PROGMANDDE CLASS IMPLEMENTATION
//---------------------------------------------------------------------

/*-----------------------------------------------------------------------------
|  DDeCallback()                                                              |
|     This is required for using DDEML.  Our's doesn't do much because we     |
|     don't answer any server requests and all of our transactions are        |
|     synchronous.                                                            |
-----------------------------------------------------------------------------*/
HDDEDATA FAR PASCAL _export DdeCallback(UINT type, UINT fmt, HCONV hconv, HSZ hsz1, HSZ hsz2,
                                        HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
   return FALSE;
}


//---------------------------------------------------------------------
// PROGMAN():
//---------------------------------------------------------------------
PROGMANDDE::PROGMANDDE(void)
{                        
}

//---------------------------------------------------------------------
// PROGMAN():
//---------------------------------------------------------------------
PROGMANDDE::~PROGMANDDE(void)
{                        
}



//---------------------------------------------------------------------
// Initialize():
//---------------------------------------------------------------------
BOOL PROGMANDDE::Initialize(HINSTANCE hInstance)
{
   // copy instance handle.
   m_hInstance = hInstance;
   // initialize DDE and register Callback function
   m_dwInstID  = 0L;
   m_pfnCallBack = MakeProcInstance((FARPROC) DdeCallback, m_hInstance);
   // initialize DDE client service with DDEML.
   if (!DdeInitialize(&m_dwInstID,(PFNCALLBACK)m_pfnCallBack,
                      (APPCLASS_STANDARD|APPCMD_CLIENTONLY|CBF_SKIP_ALLNOTIFICATIONS),0L)) 
   {
      m_hszPROGMAN = DdeCreateStringHandle(m_dwInstID,"PROGMAN",0);
      m_hszGROUPS  = DdeCreateStringHandle(m_dwInstID,"GROUPS",0);
      return (TRUE);
   } 
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// Uninitialize():
//---------------------------------------------------------------------
BOOL PROGMANDDE::Uninitialize(void)
{                              
   // free string handles create in Initialize().
   DdeFreeStringHandle(m_dwInstID,m_hszPROGMAN);
   DdeFreeStringHandle(m_dwInstID,m_hszGROUPS);
   // uninitialize DDEML layer.   
   return (DdeUninitialize(m_dwInstID));
}


//---------------------------------------------------------------------
// Connect():
//---------------------------------------------------------------------
BOOL PROGMANDDE::Connect(void)
{      
   // connect to PROGMAN DDE server using DDEML.
   if ((m_hConv = DdeConnect(m_dwInstID,m_hszPROGMAN,m_hszPROGMAN,NULL)) != (HCONV)NULL) 
      return (TRUE);
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// Disconnect():
//---------------------------------------------------------------------
BOOL PROGMANDDE::Disconnect(void)
{              
   // disconnect DDE session with PROGMAN.
   return (DdeDisconnect(m_hConv));
}


//---------------------------------------------------------------------
// ParseLine():
//    This function parses a DDE PROGMAN line of text corresponding
// to groups and items information.
//---------------------------------------------------------------------
char *PROGMANDDE::ParseLine(char *psz,char *pszDelm)
{
   static char *pszCurrent = NULL;
   static char OldDelm;
   int         TokenEnd;

   // If the given string is not NULL, reset the current string to the beginning of
   // this new string
   if( psz != NULL ) {
      pszCurrent = psz;
   } else {
      // Setup current string for next search
      TokenEnd = strlen( pszCurrent );
      pszCurrent[TokenEnd] = OldDelm;
      pszCurrent += TokenEnd + 1;
   }

   // Search from the current string pointer for the next token
   TokenEnd = strcspn( pszCurrent, pszDelm );
   OldDelm = pszCurrent[TokenEnd];
   pszCurrent[TokenEnd] = '\0';

   return( pszCurrent );
}


//---------------------------------------------------------------------
// ParseDetailInfoString():
//---------------------------------------------------------------------
void PROGMANDDE::ParseDetailInfoString(GROUPINFO *pGroupInfo,char *pszBuffer)
{                   
   ITEMINFO    *pItem;
   char        *pszToken,*psz;
   int         index;

   // make sure that as we find a new item, we "new" the space for it
   // and increment the item count in group.
                             
   // skip the group title
   ParseLine(pszBuffer,",");
   // skip group file path
   ParseLine(NULL,",");
   // get the number of items in group
   pszToken = ParseLine(NULL,",");
   if (pszToken!=NULL)
      pGroupInfo->m_nItemCount = atoi(pszToken);
   // move cursor to EOL
   ParseLine( NULL, "\n" );
         
   // allocate enough space to hold all items in group.
   pGroupInfo->m_pItemInfo = new ITEMINFO[pGroupInfo->m_nItemCount];  

   // if memory allocation error, return immediately.   
   if (pGroupInfo->m_pItemInfo==NULL)
      return;
   
   // loop through all item objects in group and parse out data...          
   for (index=0; index<pGroupInfo->m_nItemCount; index++)
   {             
      // initialize pointer to current item (for convenience).
      pItem = &(pGroupInfo->m_pItemInfo[index]);

      // get the program item title.
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL)
      {
         // Fill-in string with zeros (since strncpy() does not append '\0' if source 
         // string is longer than destination string).
         memset(pItem->m_szTitle,'\0',(MAX_STRING+1));
         // remove start and end quotes from description string.
         strncpy(pItem->m_szTitle,(pszToken+1),MAX_STRING);    // start at 2nd position in string.
         pItem->m_szTitle[strlen(pItem->m_szTitle)-1] = '\0';  // remove last character.
      }

      // get the program item command line.
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL) 
      {
         // Fill-in string with zeros (since strncpy() does not append '\0' if source 
         // string is longer than destination string).
         memset(pItem->m_szCommandLine,'\0',(MAX_STRING+1));
         // do not allow the surrounding quotes into the final string
         strncpy(pItem->m_szCommandLine,pszToken+1,MAX_STRING);
         pItem->m_szCommandLine[strlen(pItem->m_szCommandLine)-1] = '\0';
         // seperate the command line and the parameters since PROGMAN 
         // keeps both together in the "Command Line" field.
         psz = strchr(pItem->m_szCommandLine,' ');   // parse until blank space.
         // if parameters exists...
         if (psz!=NULL) 
         {                         
            // zero-terminate the command line string.
            *psz = '\0';
            // Fill-in string with zeros (since strncpy() does not append '\0' if source 
            // string is longer than destination string).
            memset(pItem->m_szParameters,'\0',(MAX_STRING+1));
            // copy parameters into parameters field.
            strncpy(pItem->m_szParameters,(psz+1),MAX_STRING);
         } 
         else 
            // no parameters found.
            strcpy(pItem->m_szParameters,"");
      }

      // get the default working directory
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL) 
      {
         // Fill-in string with zeros (since strncpy() does not append '\0' if source 
         // string is longer than destination string).
         memset(pItem->m_szWorkDir,'\0',(MAX_STRING+1));
         strncpy(pItem->m_szWorkDir,pszToken,MAX_STRING);
      }
      else
         strcpy(pItem->m_szWorkDir,"");
                              
      // get the program item's icon path
      pszToken = ParseLine(NULL,",");  
      
      if (pszToken!=NULL)
      {
         // Fill-in string with zeros (since strncpy() does not append '\0' if source 
         // string is longer than destination string).
         memset(pItem->m_szIconPath,'\0',(MAX_STRING+1));
         strncpy(pItem->m_szIconPath,pszToken,MAX_STRING);
      }
      else
         strcpy(pItem->m_szIconPath,"");

      // get (x,y) position in group.
      // first the x coordinate.
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL)
         pItem->m_x = atoi(pszToken);
      // then, the y coordinate.
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL)
         pItem->m_y = atoi(pszToken);
            
      // get Icon Index
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL)
         pItem->m_iIconIndex = atoi(pszToken);

      // get the hotkey value.
      pszToken = ParseLine(NULL,",");
      if (pszToken!=NULL)
         pItem->m_iHotKey = atoi(pszToken);

      // and finally, get minimized flag
      pszToken = ParseLine(NULL,"\n");
      if (pszToken!=NULL)
         pItem->m_bMinimize = (pszToken[0]=='1');
               
      // move until end-of-item delimiter (the newline character).
      //ParseLine(NULL,"\n");
   }
}


//---------------------------------------------------------------------
// ReadGroupDetails():
//---------------------------------------------------------------------
BOOL PROGMANDDE::ReadGroupDetails(GROUPINFO *pGroupInfo)
{   
   HSZ         hszReqItem;
   HDDEDATA    hddedGroups;
   char        *pTempBuffer;
   DWORD       dwDataSize;

   // create string handle for requested group.
   hszReqItem = DdeCreateStringHandle(m_dwInstID,pGroupInfo->m_szTitle,0);
   // request information about objects in the group
   if ((hddedGroups = DdeClientTransaction(NULL,0,m_hConv,hszReqItem,CF_TEXT,XTYP_REQUEST,DDETIMEOUT,NULL)) 
          != (HCONV)NULL) 
   {
      // extract the data into a local buffer
      dwDataSize = DdeGetData(hddedGroups, NULL, 0, 0);
      // allocate temporary memory space to store group items data.
      if ((pTempBuffer = (char *)malloc((size_t)dwDataSize)) != NULL) 
      {     
         // get group detailed information about its items.
         dwDataSize = DdeGetData(hddedGroups, pTempBuffer, dwDataSize, 0);
         // initialize item count to zero.
         pGroupInfo->m_nItemCount = 0;
         // the data is returned as a big nasty string that's gotta be parsed
         // the first line is some useless info about the group.
         // each object has one line (seperated by "\r\n") that starts with it's
         // title in quotes followed by a comma and the command line in quotes,
         // so we find the object we're looking for by comparing the beginning
         // of each line up to a ","
         ParseDetailInfoString(pGroupInfo,pTempBuffer);
         // free temporary memory.
         free(pTempBuffer);       
         // free string handle.
         DdeFreeStringHandle(m_dwInstID,hszReqItem);
         // free data handle.
         DdeFreeDataHandle(hddedGroups);
         // return success.
         return (TRUE);
      }
      else
         return (FALSE);
   }
   else
      return (FALSE);   
}

 

//---------------------------------------------------------------------
// ReadAllGroups(): 
//---------------------------------------------------------------------
BOOL PROGMANDDE::ReadAllGroups(void)
{  
   char        *pTempBuffer,*pszSource;
   DWORD       dwDataSize;
   HDDEDATA    hddedGroups;
   int         iNumGroups,index;

   // Request group titles from PROGMAN
   if ((hddedGroups = DdeClientTransaction(NULL,0,m_hConv,m_hszGROUPS,CF_TEXT,XTYP_REQUEST,DDETIMEOUT,NULL)) 
           != (HCONV)NULL) 
   {
      // if it worked, extract the data into a local buffer
      dwDataSize = DdeGetData(hddedGroups, NULL, 0, 0);
      if ((pTempBuffer = (char *)malloc((size_t)dwDataSize)) != NULL) 
      {
         dwDataSize = DdeGetData(hddedGroups,pTempBuffer,dwDataSize,0);
         // the data is a string with group names seperated by "\r\n"
         // first, count the number of lines to get the number of groups.
         for(iNumGroups = -1, pszSource = pTempBuffer; pszSource;
                pszSource = strchr(pszSource + 1, '\n'), iNumGroups ++);
         // now, allocate enough memory to hold all group information records.
         m_nGroupCount = iNumGroups;
         m_pGroupInfo  = new GROUPINFO[iNumGroups];
         // loop through, extracting each group title, and set each pointer in the array.
         pszSource = strtok(pTempBuffer, "\r\n");
         index = 0;                                   
         // while valid group name...
         while ((pszSource!=NULL)&&(index<iNumGroups))
         {                          
            // make local copy of group title.
            strcpy(m_pGroupInfo[index].m_szTitle,pszSource);
            // read detailed group information.
            if (ReadGroupDetails(&(m_pGroupInfo[index]))==FALSE)
               return (FALSE);
            // increment group count;
            index++;
            // get next group title string.
            pszSource = strtok(NULL, "\r\n");
         }             
         // free all memory allocated.
         free(pTempBuffer);
         // free DDE data handle.
         DdeFreeDataHandle(hddedGroups);
         // successfully read all group information.
         return (TRUE);
      }
      else
         return (FALSE);
   }
   else
      return (FALSE);   
}   

//---------------------------------------------------------------------
// GetGroupCount():
//---------------------------------------------------------------------
int PROGMANDDE::GetGroupCount(void)
{
   return (m_nGroupCount);
}

//---------------------------------------------------------------------
// GetGroupDetails():
//---------------------------------------------------------------------
GROUPINFO *PROGMANDDE::GetGroupDetails(int nIndex)
{
   return (&(m_pGroupInfo[nIndex]));
}


// EOF PROGMAN.CPP

