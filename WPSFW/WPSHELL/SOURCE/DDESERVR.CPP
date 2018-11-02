//=====================================================================
// DDESERVR.CPP
//    Windows DDE Server implementation.
// 
// Date Created :  06/01/94
// Last Update  :  02/17/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 06/01/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <ctype.h>
// Windows library header files.
#include <ddeml.h>

//---------------------------------------------------------------------
// CLASS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DDESERVER():
//---------------------------------------------------------------------
DDESERVER::DDESERVER(void)
{              
   // Initialize all data members of this class to zero.
   memset(this,0,sizeof(DDESERVER));
}

        
//---------------------------------------------------------------------
// Initialize():
//---------------------------------------------------------------------
BOOL DDESERVER::Initialize(char *pszService,FARPROC DdeCallback)
{  
   // Create a procedure instance for DDE callback function.   
   m_pfnCallback = MakeProcInstance(DdeCallback,g_wpEnvironment.hInst);
   // Initialize DDEML library and callback procedure.
   if (DdeInitialize(
          &m_dwID,
          (PFNCALLBACK)m_pfnCallback,
          (CBF_FAIL_POKES|CBF_SKIP_REGISTRATIONS|CBF_SKIP_UNREGISTRATIONS|
           CBF_SKIP_CONNECT_CONFIRMS),
          0L)!=DMLERR_NO_ERROR)
   {                                  
      // Free procedure instance.
      FreeProcInstance(m_pfnCallback);
      // Return failure.
      return (FALSE);
   }
   
   // Store copy of DDE service name to support.
   strcpy(m_szService,pszService);
   
   // Create service name string.
   m_hszService = DdeCreateStringHandle(m_dwID,m_szService,0);
   
   // Register the DDE service.
   if (DdeNameService(m_dwID,m_hszService,NULL,DNS_REGISTER)==0)
      return (FALSE);
   
   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// DDE TOPICS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CompareHszToStr():
//---------------------------------------------------------------------
BOOL DDESERVER::CompareHszToStr(HSZ hsz,char *psz)
{  
   char szHszString[128];

   // Get topic string from handle to string.
   DdeQueryString(m_dwID,hsz,szHszString,sizeof(szHszString),0);
   
   // Return TRUE if strings are equal, else return FALSE.
   return ((stricmp(szHszString,psz)==0)? TRUE : FALSE);
}


//---------------------------------------------------------------------
// GetTopicCount():
//---------------------------------------------------------------------
int DDESERVER::GetTopicCount(void)
{  
   // Return private data member value.
   return (m_nTopicCount);   
}
   
//---------------------------------------------------------------------
// AddTopic():
//---------------------------------------------------------------------
BOOL DDESERVER::AddTopic(char *pszTopic)
{                                                
   // Check if we have reached the maximum number of topics already...
   if ((m_nTopicCount+1)<=MAX_DDE_TOPICS)
   {                                          
      // Store internally pointer to topic string.
      m_pszTopics[m_nTopicCount++] = pszTopic;
      // Return success.
      return (TRUE);
   }
   else             
      // Return failure.
      return (FALSE);
}

//---------------------------------------------------------------------
// isTopic():
//---------------------------------------------------------------------
BOOL DDESERVER::isTopic(char *pszTestTopic)
{
   int index;
                           
   // Search through all available topics in DDE server class...                           
   for (index=0; index<m_nTopicCount; index++)
   {                 
      // If topic string being tested is supported by server...
      if (stricmp(m_pszTopics[index],pszTestTopic)==0)         
         // Return success.
         return (TRUE);
   }                   
   // Return failure.   
   return (FALSE);
}

   
//---------------------------------------------------------------------
// DDE SERVICE PROCESSING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// IsBlank():
//    This function returns TRUE if string is empty or contains all 
// space characters.  It returns FALSE otherwise.
//---------------------------------------------------------------------
BOOL DDESERVER::IsBlank(char *pszString)
{
   // Check if no string is available, return "is blank".
   if (pszString==NULL)
      return (TRUE);
                      
   // While there are characters to search in string...                      
   while (*pszString!=NULL)
   {
      // Test if current character is non-space char.
      if (!IsSpace(*pszString))
         // Return "is non-blank"
         return (FALSE);       
      // Increment to next character.
      pszString++;
   }
   // All chars. in string are space chars., return "is blank".
   return (TRUE);
}

   
//---------------------------------------------------------------------
// GetNextCommand():
//---------------------------------------------------------------------
BOOL DDESERVER::GetNextCommand(char *pszBuffer,char *pszCommandString,int *pnPosition)
{ 
   char  cStartDelimiter = '[', cEndDelimiter = ']',
         *pszStart,*pszEnd;
                               
   // Make command string "empty" (just in case).
   pszCommandString[0] = '\0';

   // Check if we have reached the end of the data buffer.
   if ((*pnPosition+1)>=(int)strlen(pszBuffer))
      // There are no more commands to process, return failure.
      return (FALSE);

   // There is still at least one more character to search for a command.
   // Increment the position index where the current search will start from.
   (*pnPosition)++;
      
   // Search for the next occurrence of the "start command" delimeter char.
   pszStart = strchr(&(pszBuffer[*pnPosition]),cStartDelimiter);
   // If no "start command" delimeter is found...
   if (pszStart==NULL)                
      // Return failure.
      return (FALSE);     
   
   // Search for the next occurrence of the "end command" delimeter char.
   pszEnd = strchr(pszStart,cEndDelimiter);
   // If no "end command" delimeter is found...
   if (pszEnd==NULL)                
      // Return failure.
      return (FALSE);     
          
   // Null-terminate data buffer so that we can use "strcpy()" to copy 
   // the newly found command string.  we then restore the original char.
   // to the data buffer position.          
   *pszEnd = '\0';
          
   // Copy newly found command string into command string buffer. we do not
   // want the delimiter charcaters included in the command string copy.
   
   strcpy(pszCommandString,(pszStart+1));
   
   // Restore the original "end delimiter" character to buffer.
   *pszEnd = cEndDelimiter;
   
   // Calculate new search position to be used next time this function
   // is called.  this allows us to be more "reentrant" than "strtok()" i.e.
   *pnPosition = (pszEnd - pszBuffer);
   
   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// GetCommandNameAndParameters():
//---------------------------------------------------------------------
BOOL DDESERVER::GetCommandNameAndParameters(char *pszCommandString,char **ppszName,char **ppszParameters)
{                                  
   int   nIndex;
   char  cStartDelimiter = '(',cEndDelimiter = ')',*pszStart,*pszEnd;
                   
   #ifdef _DEBUG              
   dprintf("tr GetCommandNameAndParameters(): %s\n",pszCommandString);
   #endif
      
   // Intialize command name and parameters string to "empty".                     
   *ppszName        = NULL;
   *ppszParameters  = NULL;
                
   // Initialize index into command string.
   nIndex = 0;
                   
   // Skip any white spaces at the beginning of the commans string.
   while (IsSpace(pszCommandString[nIndex]))
      nIndex++;
      
   // Now, search for the first occurrence of the "start parameters" delimiter char.
   pszStart = strchr(&(pszCommandString[nIndex]),cStartDelimiter);
   // Check if command string does not have a "start parameter" delimiter.
   if (pszStart==NULL)
      // Return failure.
      return (FALSE);   
   
   // Now, search for the "end parameters" delimiter char.
   pszEnd = strchr(pszStart,cEndDelimiter);
   // Check if command string does not have a "end parameter" delimiter.
   if (pszEnd==NULL)
      // Return failure.
      return (FALSE);   

   // Null-terminate the command name string by placing a '\0' where the "start
   // paremeters" list delimiter character is.          
   *pszStart = '\0';                                                   
   
   // Store pointer to start of command name string (it has been null-terminated).
   *ppszName = &(pszCommandString[nIndex]);

   // Null-terminate the parameter string by placing a '\0' where the "end 
   // paremeters" list delimiter character is.
   *pszEnd   = '\0';

   // Store pointer to start of parameter string (is has been null-terminated).          
   *ppszParameters = (pszStart+1);
                                                 
   // Return success.                 
   return (TRUE);   
}
                         
//---------------------------------------------------------------------
// DDE CALLBACK FOUNCTION
//---------------------------------------------------------------------
             
//---------------------------------------------------------------------
// DdeCallback():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
HDDEDATA DDESERVER::DdeCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2)
{                          
   char  szTopic[MAX_DDE_TOPIC_NAME],szCommand[256],szBuffer[64],
         *pszName,*pszParameters,*pszData;         
   int   nSize,nPosition;

   // Depending on which transaction type...
   switch (uType)
   {
      case XTYP_CONNECT:
         // Get service string from handle to string.
         DdeQueryString(m_dwID,hsz2,szBuffer,sizeof(szBuffer),0);
               
         #ifdef _DEBUG
         dprintf("tr XTYP_CONNECT (service): %s == %s\n",szBuffer,m_szService);
         #endif
         
         // Make sure we support service.
         if (stricmp(szBuffer,m_szService)!=0)
            return ((HDDEDATA)FALSE);
            
         // Get topic string from handle to string.
         DdeQueryString(m_dwID,hsz1,szBuffer,sizeof(szBuffer),0);
               
         #ifdef _DEBUG
         dprintf("tr XTYP_CONNECT (topic): %s == %s\n",szBuffer,m_szService);
         #endif
         
         // Make sure we support the requested topic.
         if (isTopic(szBuffer)==FALSE)
            return ((HDDEDATA)FALSE);
            
         // Return success.                        
         return ((HDDEDATA )(DWORD)TRUE);   

      case XTYP_REQUEST:
      case XTYP_ADVREQ:
         // hsz1 = topic string.
         // hsz2 = item string.
               
         #ifdef _DEBUG
         dprintf("tr XTYP_REQUEST: dw1 = 0x%08lX  dw2 = 0x%08lX\n",dwData1,dwData2);
         #endif
            
         // Get topic string from handle to string.
         DdeQueryString(m_dwID,hsz1,szTopic,sizeof(szTopic),0);
         // Make sure we support the requested topic.
         if (isTopic(szTopic)==FALSE)
            return ((HDDEDATA)NULL);
            
         // Service request and return pointer to response data.                  
         hData = ServiceRequest(hsz1,hsz2);            
         // Return the response data by creating a data handle.
         return (hData);
                  
      case XTYP_EXECUTE:       
         // Execute command string.
         // hsz1  = topic string.
         // hsz2  = not used
         // hData = command string.
         
         #ifdef _DEBUG
         dprintf("tr XTYP_EXECUTE:\n");
         #endif
         
         // Get size of data area.
         nSize = (int)DdeGetData(hData,NULL,0L,0L);
         // Allocate memory for data buffer.         
         pszData = new char[nSize];
         // Fill newly allocated data buffer with transaction data.
         DdeGetData(hData,pszData,(DWORD)nSize,0L);
         
         // Initialize command string position to the beginning of the command buffer.
         nPosition = -1;
         
         // Execute every command string in data buffer...
         while (GetNextCommand(pszData,szCommand,&nPosition)==TRUE)
         {             
            #ifdef _DEBUG              
            dprintf("tr Cmd: '%s'\n",szCommand);
            #endif
         
            // Parse out command name and parameter string from command string.
            if (GetCommandNameAndParameters(szCommand,&pszName,&pszParameters)==FALSE)
            {     
               #ifdef _DEBUG              
               dprintf("err GetCommandNameAndParameters(): failed.\n");
               #endif
            
               // Free memory allocated for data buffer.
               delete[] pszData;
               // Return command execution failed.
               return ((HDDEDATA)DDE_FNOTPROCESSED);
            }                       
            // Execute current command, and make sure it was successfull.
            if (ExecuteCommand(pszName,pszParameters)==FALSE)
            {           
               #ifdef _DEBUG              
               dprintf("err ExecuteCommand(): failed.\n");
               #endif
            
               // Free memory allocated for data buffer.
               delete[] pszData;
               // Return command execution failed.
               return ((HDDEDATA)DDE_FNOTPROCESSED);
            }
         }     
                  
         #ifdef _DEBUG                  
         dprintf("tr XTYP_EXECUTE(): done success.\n");
         #endif
         // Free memory allocated for data buffer.
         delete[] pszData;
         // Return command execution acknowledgement.
         return ((HDDEDATA)(DWORD)DDE_FACK);
   }  
   
   // Return "nothing" by default.
   return ((HDDEDATA)NULL);
}


//---------------------------------------------------------------------
// Uninitialize():
//---------------------------------------------------------------------
BOOL DDESERVER::Uninitialize(void)
{  
   BOOL  fSuccess;

   // Unregister the service name.                         
   DdeNameService(m_dwID,m_hszService,NULL,DNS_UNREGISTER);

   // Delete created string handles.
   DdeFreeStringHandle(m_dwID,m_hszService);

   // Uninitialize from the DDEML library.
   fSuccess = ((DdeUninitialize(m_dwID)==0)? TRUE : FALSE);
   
   // Free the procedure instance that we created for the DDE callback.
   FreeProcInstance((FARPROC)m_pfnCallback);
                     
   // Return status value.                     
   return (fSuccess);
}

// EOF DDESERVR.CPP




