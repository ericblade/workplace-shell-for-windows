//=====================================================================
// DDEPRGMN.CPP
//    Windows DDE PROGMAN Service Server implementation.
// 
// Date Created :  06/05/94
// Last Update  :  10/09/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 06/05/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <stdlib.h>

//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
HDDEDATA CALLBACK _loadds PROGMANServiceCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2);

    
//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
        
//---------------------------------------------------------------------
// STATIC VARIABLES
//---------------------------------------------------------------------
// NOTE: This variable contains the pointer to the same object WPDESKTOP
// class contains for DDE server for PROGMAN service.  This "local"
// global variable helps us call the right "current" PROGMAN server 
// when a transaction comes into our DDE callback function.
static DDEPROGMAN  *gs_pddeserverPROGMAN = NULL;


//---------------------------------------------------------------------
// CLASS FUNCTIONS
//---------------------------------------------------------------------
        
//---------------------------------------------------------------------
// Initialize():
//---------------------------------------------------------------------
BOOL DDEPROGMAN::Initialize(WPDESKTOP *pwpWPSDesktop)
{                               
   BOOL  fReturnValue;
                   
   // Store pointer to ourseleves (to be used by DDE callback fucntion
   // to reference this class instantiation).
   gs_pddeserverPROGMAN = this;
   
   // Store the pointer to the Workplace Shell desktop object in our private data.
   m_pwpWPSDesktop = pwpWPSDesktop;
   
   // Check if WPS for Windows is the current Windows shell...
   if (m_pwpWPSDesktop->IsShell()==TRUE)
      // Call base class initialization function.
      fReturnValue = DDESERVER::Initialize("PROGMAN",(FARPROC)PROGMANServiceCallback);
   else
      // Call base class initialization function.
      fReturnValue = DDESERVER::Initialize("WPS_PROGMAN",(FARPROC)PROGMANServiceCallback);
   
   // Register all the supported "topic" strings for this DDE server.
   AddTopic("PROGMAN");
   AddTopic("GROUPS");
   
   // Return value from initialize function.
   return (fReturnValue);
}

//---------------------------------------------------------------------
// RecursivelyQueryGroupNames():
//---------------------------------------------------------------------
void DDEPROGMAN::RecursivelyQueryGroupNames(WPFOLDER *pwpfolder,HDDEDATA *phData,DWORD *pdwDataOffset)
{                      
   int   index;                            
   char  szFolderTitle[MAX_TITLE_NAME+2+1]; // <string>+"\r\n"+"\0"
                              
   // Copy locally the name of the current folder.
   strcpy(szFolderTitle,pwpfolder->GetTitle());
   // Concatenate the group name terminator characters (line feed/new line).
   strcat(szFolderTitle,"\r\n");
              
   #ifdef _DEBUG              
   dprintf("%s",szFolderTitle);
   #endif
                              
   // Add current folder's name to the group name list 
   // (make sure to update the DDE data handle since a new one is created).
   *phData = DdeAddData(*phData,szFolderTitle,(DWORD)strlen(szFolderTitle)+1,*pdwDataOffset);
   // Increment current data handle offset counter.   
   *pdwDataOffset += strlen(szFolderTitle);               
   
   // Loop through all objects in current folder, and recursively add each
   // child folder object to the group names list.
   for (index=0; index<pwpfolder->GetObjectCount(); index++)
   {
      // If one of the children objects in this current folder IS itself a folder, 
      // AND is not the "Minimized Window Viewer"...
      if (((pwpfolder->GetObject(index)->GetType()&OBJTYPE_FOLDER)==OBJTYPE_FOLDER) &&
          ((pwpfolder->GetObject(index)->GetType()&OBJTYPE_MINVIEWER)!=OBJTYPE_MINVIEWER))
      
         // Recursively call it so it can add itself to the list of names.
         RecursivelyQueryGroupNames((WPFOLDER *)pwpfolder->GetObject(index),phData,pdwDataOffset);
   }
}
          
          
//---------------------------------------------------------------------
// QueryAllGroupNames():
//    This function queries all of the Workplace Shell desktop group 
// names by recursively accessing all "folder" objects and adding them
// to the list.
//---------------------------------------------------------------------
void DDEPROGMAN::QueryAllGroupNames(HDDEDATA *phData)
{        
   int   index;                                           
   DWORD dwDataOffset;
   
   // Initialize data handle offset value to zero.
   dwDataOffset = 0L;
   // Recursively query all folders in the Workplace Shell desktop,
   // and add each of their names to the list of group names.   
   for (index=0; index<m_pwpWPSDesktop->GetObjectCount(); index++)
   {
      // If one of the children objects in this current folder IS itself a folder,
      // AND is not the "Minimized Window Viewer"...
      if (((m_pwpWPSDesktop->GetObject(index)->GetType()&OBJTYPE_FOLDER)==OBJTYPE_FOLDER) &&
          ((m_pwpWPSDesktop->GetObject(index)->GetType()&OBJTYPE_MINVIEWER)!=OBJTYPE_MINVIEWER))
      
         // Recursively call it so it can add itself to the list of names.
         RecursivelyQueryGroupNames((WPFOLDER *)m_pwpWPSDesktop->GetObject(index),phData,&dwDataOffset);
   }
}

//---------------------------------------------------------------------
// QueryDetailedGroupInfo():
//---------------------------------------------------------------------
BOOL DDEPROGMAN::QueryDetailedGroupInfo(HSZ hszGroupTitle,HDDEDATA *phData)
{                       
   WPFOLDER    *pwpfolderGroup;                 // this points to the found folder or "group" (if any).
   WPPROGRAM   *pwpProgramItem;
   WORD        wIconIndex;
   DWORD       dwDataOffset;  
   char        szGroupTitle[MAX_TITLE_NAME+1],  // this string holds the group title name.
               szWorkString[256];               // make sure this string is big enough.
   int         index,nCount;
   
   // Get name of group into a character string.
   DdeQueryString(m_dwID,hszGroupTitle,szGroupTitle,(DWORD)sizeof(szGroupTitle),0);
                  
   #ifdef _DEBUG                  
   dprintf("%s\n",szGroupTitle);
   #endif
   
   // Find group (folder) in Workplace Shell desktop.
   pwpfolderGroup = (WPFOLDER *)m_pwpWPSDesktop->Find(OBJTYPE_FOLDER,szGroupTitle);
   
   // Check if a group match was found.
   if (pwpfolderGroup==NULL)
      return (FALSE);
   
   // We have found the group (folder), therefore append group information
   // to data handle passed to this function.
   
   // First, initialize the data handle offset (internal count).
   dwDataOffset = 0L;
          
   // Format of group detail information is as follows:
   // "<title name>",<group file name>,<number of items in group>,<state flag>"\n"
   // "<item 0 title>","<command string and parameters>",<default working dir.>,
   // <icon path and file name>,<x>,<y>,<icon index>,<hotkey value>,<min flag>"\n"
   // .............
   // "<item n+1 title>","<command string and parameters>",<default working dir.>,
   // <icon path and file name>,<x>,<y>,<icon index>,<hotkey value>,<min flag>"\n\0"
   // 
   // Where:
   // <min flag>     :  0 = not minimized,  1 = minimized.
          
   // Now, start creating the group detailed data in the format above
   // that will be passed back to the client application who requested it.
   // Add "<title name>" field.
   sprintf(szWorkString,"\"%s\"",szGroupTitle);
   *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
   dwDataOffset += strlen(szWorkString);
   // Add ,<group file name> field (no file name exists).
   *phData = DdeAddData(*phData,",",(DWORD)strlen(",")+1,dwDataOffset);
   dwDataOffset += strlen(",");
   // Add ,<number of items in group>"\n" field.                
   // To accomplish this, we must loop through all available objects within
   // the found folder (group), and only count valid program objects since 
   // they are the only applicable PROGMAN's "program item" equivalent.
   for (nCount=0,index=0; index<pwpfolderGroup->GetObjectCount(); index++)
   {       
      // If current object is a program object (equivalent to a program item in PROGMAN)...
      if ((pwpfolderGroup->GetObject(index)->GetType()&OBJTYPE_PROGRAM)==OBJTYPE_PROGRAM)
         // Increment our program item count.
         nCount++;
   }
   // Finally, we know how many objects in the folder are valid program items.
   // Add to DDE data handle by prefixing it with "," and appending a "\n".
   sprintf(szWorkString,",%d,1\n",nCount);
   *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
   dwDataOffset += strlen(szWorkString);
             
   #ifdef _DEBUG                  
   dprintf(szWorkString);         
   #endif
         
   // Now, we will loop again though all objects in folder, but this time we will be
   // adding each of the valid program item's detailed information to the data handle.   
   for (index=0; index<pwpfolderGroup->GetObjectCount(); index++)
   {     
      // If current object is a program object (equivalent to a program item in PROGMAN)...
      if ((pwpfolderGroup->GetObject(index)->GetType()&OBJTYPE_PROGRAM)==OBJTYPE_PROGRAM)
      {
         // For convenience and simplicity of code following, we have casted the pointer
         // returned by GetObject() to a pointer to a WPPROGRAM object.
         pwpProgramItem = (WPPROGRAM *)pwpfolderGroup->GetObject(index);
         
         // Add "<item index title>" field.
         sprintf(szWorkString,"\"%s\"",pwpProgramItem->GetTitle());
         *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
         dwDataOffset += strlen(szWorkString);
         
         // Add ",<command string and parameters>" field.
         sprintf(szWorkString,",\"%s %s\"",pwpProgramItem->GetProgramPath(),pwpProgramItem->GetParameters());
         *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
         dwDataOffset += strlen(szWorkString);
         
         // Add ",<default working dir>" field.
         sprintf(szWorkString,",%s",pwpProgramItem->GetWorkingDir());
         *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
         dwDataOffset += strlen(szWorkString);
         
         // Add ",<icon path and file name>" field.
         pwpProgramItem->GetIcon(szGroupTitle,&wIconIndex); // we are reusing the "szGroupTitle" string here.
         sprintf(szWorkString,",%s",szGroupTitle);
         *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
         dwDataOffset += strlen(szWorkString);
         
         // Add ",<x>,<y>,<icon index>,<hotkey value>,<min flag>"\n" field.
         sprintf(szWorkString,",%d,%d,%u,0,%u\n",
            pwpProgramItem->GetIconPos().x,pwpProgramItem->GetIconPos().y,
            wIconIndex,
            (((pwpProgramItem->GetFlags()&START_PROG_MASK)==START_PROG_MINIMIZED)? 1 : 0) );
         *phData = DdeAddData(*phData,szWorkString,(DWORD)strlen(szWorkString)+1,dwDataOffset);
         dwDataOffset += strlen(szWorkString);
      }
   }
                
   #ifdef _DEBUG                  
   dprintf("tr QueryDetailedGroupInfo(): done.\n\n");                                   
   #endif
   
   // Return success.                 
   return (TRUE);
}

                          
//---------------------------------------------------------------------
// ServiceRequest():
//    This function services transaction requests that are posted to 
// the DDE callback function through the XTYP_REQUEST and XTYP_ADVREQ
// transaction types.
//---------------------------------------------------------------------
HDDEDATA DDEPROGMAN::ServiceRequest(HSZ hszTopic,HSZ hszItem)
{                              
   HDDEDATA    hData;
                      
   #ifdef _DEBUG                  
   dprintf("tr DDEPROGMAN::ServiceRequest():\n");
   #endif
      
   // If requested the list of all group names (all folder names in WPS)...
   if ((CompareHszToStr(hszTopic,"PROGMAN")==TRUE) &&
       (CompareHszToStr(hszItem,"GROUPS")==TRUE))
   {              
      // Create an empty data object to fill in later.
      hData = DdeCreateDataHandle(m_dwID,NULL,0L,0L,hszItem,CF_TEXT,0);
      // Query and fill in the list of all group names in the Workaplace Shell desktop.
      QueryAllGroupNames(&hData);
      // Return data handle containing list of group names.
      return (hData);
   }
   else
   // If requested detailed information about a specific group...
   if (CompareHszToStr(hszTopic,"PROGMAN")==TRUE)
   {
      // Create an empty data object to fill in later.
      hData = DdeCreateDataHandle(m_dwID,NULL,0L,0L,hszItem,CF_TEXT,0);
      // Query and fill in detailed information about requested group.
      if (QueryDetailedGroupInfo(hszItem,&hData)==FALSE)
      {  
         // Free data handle created previously.
         DdeFreeDataHandle(hData);
         // Return "error - no data available for group".         
         return ((HDDEDATA)NULL);
      }
      else
         // Return data handle containing detailed group information.
         return (hData);
   }
   else
      // ...Else, topic/item pair is not supported, therefore return nothing.
      return ((HDDEDATA)NULL);
}
   
//---------------------------------------------------------------------
// ParseCommandParametersIntoList():
//---------------------------------------------------------------------
void DDEPROGMAN::ParseCommandParametersIntoList(char *pszParamString,char **ppszParamList,int *nCount)
{    
   char *pszStart,*pszEnd;
       
   // Initialize parameter count to "none".
   *nCount = 0;       
           
   // Parameter list in non-existent...
   if ((pszParamString==NULL) || (pszParamString[0]=='\0'))
      return;
      
   // Start parsing parameter list from the beginning.
   pszStart = pszParamString;
   
   // While we have more parameters in the list to parse...
   while (pszStart!=NULL)
   {                
      // If this parameter has begining quotes, skip them.
      if (pszStart[0]=='"')
         pszStart++;       
         
      // Get the next occurence of a parameter separator char. (i.e. a comma).
      pszEnd = strchr(pszStart,',');
  
      // If there are no more parameters (this one is the last one)...
      if (pszEnd==NULL)                                               
      {                                
         // Check that the parameter does not have ending quotes,     
         pszEnd = strchr(pszStart,'"');                          
         // If parameter DOES have ending quotes, remove them.
         if (pszEnd!=NULL)
            *pszEnd = '\0';
         // Store pointer to null-terminated parameter string.
         ppszParamList[(*nCount)++] = pszStart;
         // End parameter parsing.
         pszStart = NULL;
      }
      else
      {                 
         // Null-terminate end of parameter string by placing a zero
         // on the parameter separator character (i.e. the comma).
         *pszEnd = '\0';
         // If parameter DOES have ending quotes, remove them.
         if (pszEnd[-1]=='"')
            pszEnd[-1] = '\0';
         // Store pointer to null-terminated parameter string.
         ppszParamList[(*nCount)++] = pszStart;
         // Point to begining of next parameter in list.
         pszStart = (pszEnd+1);
      }
   }
}        


//---------------------------------------------------------------------
// AddItem():
//---------------------------------------------------------------------
void DDEPROGMAN::AddItem(char **ppszParameters,int nParamCount)
{
   WPPROGRAM   *pwpprogItem;
   POINT       ptIconPos = {-1,-1};       
   WORD        wIconIndex = 0;
   int         iValue;
   char        *pszProgramParam,szWorkDir[_MAX_PATH+1],szDrive[_MAX_DRIVE+1],
               szDir[_MAX_DIR+1],szAssociationFile[_MAX_PATH+1];

   // Instructs to add an item (icon) to the currently active group.  
   // Format of parameters is: CmdLine[,Name[,IconPath[,IconIndex[,xPos,yPos[,DefDir[,HotKey[,fMinimize]]]]]]]
   // create a new item (program) object.
   pwpprogItem = new WPPROGRAM;          
   // Depending on how many paramaters have been passed, initialize object...
   switch (nParamCount)
   {        
      // [fMinimize]
      case 9:
         // Convert minimize flag into integer.
         iValue = atoi(ppszParameters[8]);       
         // If 'Start minimized' flag is set...
         if (iValue==1)
            pwpprogItem->SetFlagsOR(START_PROG_MASK,START_PROG_MINIMIZED);
      // [HotKey]
      case 8:
         // Ignore hot key value.
      // [DefDir]
      case 7:    
         // Store default directory path.
         pwpprogItem->SetWorkingDir(ppszParameters[6]);
      // [yPos]
      case 6:       
         if (IsBlank(ppszParameters[5])==FALSE)
            // Convert the yPos first and store it temporatedly in local variable.
            ptIconPos.y = atoi(ppszParameters[5]);
      // [xPos]
      case 5:
         if (IsBlank(ppszParameters[4])==FALSE)
            // Convert the xPos first and store it temporatedly in local variable.
            ptIconPos.x = atoi(ppszParameters[4]);
      // [IconIndex]
      case 4:       
         if (IsBlank(ppszParameters[3])==FALSE)
            // Convert icon index from 
            wIconIndex = (WORD)atoi(ppszParameters[3]);
      // [IconPath]
      case 3: 
      // [Name]
      case 2:  
         // Set title name of program item object.
         pwpprogItem->SetTitle(ppszParameters[1]);
      // [CmdLine]
      case 1:
         // [xPos,yPos] again:
         // If user wants us to figure out the (x,y) position of icon...
         if (((ptIconPos.x==-1)&&(ptIconPos.y==-1)) || (nParamCount<5))
         {     
            // If we are replacing an item...
            if (m_fReplaceItem==TRUE)          
            {
               // Copy original icon position of program object.
               ptIconPos = m_ptIconPos;
               // Reset 'ReplaceItem' flag.
               m_fReplaceItem = FALSE;
            }
            else
               // Get next new object's icon position in folder.                                                   
               ptIconPos = m_pwpfolderActiveGroup->GetFirstAvailableIconPos();
         }  
         // Parse out command line for program name and parameters...
         // Look for space separator between program name and parameters.
         pszProgramParam = strchr(ppszParameters[0],' ');
         if (pszProgramParam!=NULL) 
         {                                             
            // Null-terminate program name string (Parameters[0] is program name only).
            *pszProgramParam = '\0';
            // Increment pointer since parameter list starts on next character.
            pszProgramParam++;
         }     
         // Set program item object's icon position.
         pwpprogItem->SetIconPos(ptIconPos);
             
         // [IconPath[,IconIndex]] again (if no information passed):
         if (IsBlank(ppszParameters[2])==FALSE)
         {     
            #ifdef _DEBUG
               dprintf("tr AddItem(): SetIcon('%s') CP#0\n",ppszParameters[2]); 
            #endif
            // Set the icon resource for program item.
            pwpprogItem->SetIcon(ppszParameters[2],wIconIndex);
         }
         else
         {   
            #ifdef _DEBUG
               dprintf("tr AddItem(): FindExecutable('%s')\n",ppszParameters[0]); 
            #endif
                                
            // Try to find a association (if any) for program file being created...
            if (FindExecutable(ppszParameters[0],"",szAssociationFile) > (HINSTANCE)32)
            {
               #ifdef _DEBUG
                  dprintf("tr AddItem(): SetIcon('%s') CP#1\n",szAssociationFile); 
               #endif
               // Set the program icon equal to the associated program name at index zero.
               pwpprogItem->SetIcon(szAssociationFile,0);
            }
            else
            {
               #ifdef _DEBUG
                  dprintf("tr AddItem(): SetIcon('%s') CP#2\n",ppszParameters[0]); 
               #endif
               // Set the program icon equal to the program name at index zero.
               pwpprogItem->SetIcon(ppszParameters[0],0);
            }
         }
         
         // [Name] again (if no information passed):
         if (nParamCount<2)
            // Set title of program equal to 
            pwpprogItem->SetTitle("Program");         
         // Set program path and parameter line arguments.
         pwpprogItem->SetProgramPath(ppszParameters[0]);
         // If no default working directory was specified, use program's directory path.
         if ((nParamCount<7) || 
             (nParamCount>=7)&&(IsBlank(ppszParameters[6])==TRUE))
         {  
            // Extract program's path from command line string (if any).
            _splitpath(ppszParameters[0],szDrive,szDir,NULL,NULL);
            _makepath(szWorkDir,szDrive,szDir,NULL,NULL);
            // Store default directory path of program.
            pwpprogItem->SetWorkingDir(szWorkDir);
         }       
         // If program item has a parameter line...
         if (pszProgramParam!=NULL)
            pwpprogItem->SetParameters(pszProgramParam);
         break;
   }
   // Add program item object to currently active folder (if any).
   if (m_pwpfolderActiveGroup!=NULL)
   {  
      // Load icon resource for new program item object.
      pwpprogItem->LoadIcon();
      // Add new program item to currently active folder.      
      m_pwpfolderActiveGroup->AddObject(pwpprogItem);
      // Update new program item within folder.
      pwpprogItem->InvalidateIcon(TRUE,TRUE);
   }
   else                  
      // No folders active, destroy program object.
      delete pwpprogItem;
}


//---------------------------------------------------------------------
// CreateGroup():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void DDEPROGMAN::CreateGroup(char **ppszParameters,int nParamCount)
{
   WPFOLDER    *pwpfolderGroup;                               

   // Instructs to create a new group or activate the window of an existing group.
   // Format of parameters is: "GroupName"[,GroupPath]         
   // Search Workplace Shell desktop for a match of the specified group name.
   pwpfolderGroup = (WPFOLDER *)m_pwpWPSDesktop->Find(OBJTYPE_FOLDER,ppszParameters[0]);
   // If no match was found already in the shell, create a new folder object...
   if (pwpfolderGroup==NULL)
   {
      // Create new "group" (actually its a folder object).       
      pwpfolderGroup = new WPFOLDER;
      // Set new "group"'s title name.                
      pwpfolderGroup->SetTitle(ppszParameters[0]);
      // Load icon resource for new folder object.
      pwpfolderGroup->LoadIcon();
      // Get position for group folder on the desktop.
      pwpfolderGroup->SetIconPos(m_pwpWPSDesktop->GetFirstAvailableIconPos());
      // Add new folder to desktop.
      m_pwpWPSDesktop->AddObject((WPOBJECT *)pwpfolderGroup);        
      // Invalidate new folder and update it on desktop immediatedly.
      pwpfolderGroup->InvalidateIcon(TRUE,TRUE);
   }
   // Open new folder and show it on desktop.
   pwpfolderGroup->Open();
   // Make folder the currently "active" group.
   m_pwpfolderActiveGroup = pwpfolderGroup;
}


//---------------------------------------------------------------------
// ReplaceItem():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void DDEPROGMAN::ReplaceItem(char **ppszParameters,int nParamCount)
{        
   WPPROGRAM   *pwpprogItem;
   int         index;

   // Instructs to delete an item and record its (x,y) position in the group.
   // Format of parameters is: "ItemName" 
   // Reset 'ReplaceItem' flag (just in case we fail for loop tests).
   m_fReplaceItem = FALSE;                            
   // Loop through all objects in currently active folder and check if program item
   // being replaced actually exists...
   for (index=0; index<m_pwpfolderActiveGroup->GetObjectCount(); index++)
   {     
      // If object is a program object AND matches the title name being replaced...
      if (((m_pwpfolderActiveGroup->GetObject(index)->GetType()&OBJTYPE_PROGRAM)==OBJTYPE_PROGRAM) &&
           (stricmp(m_pwpfolderActiveGroup->GetObject(index)->GetTitle(),ppszParameters[0])==0))
      {  
         // Store pointer to program item object.  
         pwpprogItem = (WPPROGRAM *)m_pwpfolderActiveGroup->GetObject(index);
         // Set the 'ReplaceItem' flag.
         m_fReplaceItem = TRUE;        
         // Store the current icon position of object to be replaced.
         m_ptIconPos = pwpprogItem->GetIconPos();
         // Remove program item from active folder.
         m_pwpfolderActiveGroup->DeleteObject(index);
         // Destroy program object from shell.
         pwpprogItem->Destroy();
         // Update active folder so that we get the "replacement" effect.
         m_pwpfolderActiveGroup->Invalidate(TRUE,TRUE);
         // Stop searching the folder.           
         return;
      }
   }
}
   
//---------------------------------------------------------------------
// ShowGroup():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void DDEPROGMAN::ShowGroup(char **ppszParameters,int nParamCount)
{  
   WPFOLDER *pwpfolderGroup;
   int      iValue;

   // Instructs to minimize, maximize, or restore the window of an existing group.                        
   // Format of parameters is: "GroupName",ShowCommand
   // Search Workplace Shell desktop for a match of the specified group name.
   pwpfolderGroup = (WPFOLDER *)m_pwpWPSDesktop->Find(OBJTYPE_FOLDER,ppszParameters[0]);
   // If match was found in the shell, show folder object...
   if (pwpfolderGroup!=NULL)
   {     
      // Convert string with show value to an integer.
      iValue = atoi(ppszParameters[1]);
      // Depending on which show value the command has passed...         
      switch (iValue)
      {  
         case 1:
         case 3:
         case 5:
            // Make current folder found the currently active folder.
            m_pwpfolderActiveGroup = pwpfolderGroup;
         case 4:
         case 8:
            // Activates and displays the group window. if min/max, restore window size.
            pwpfolderGroup->Open();
            break;
               
         case 2:
            // Make current folder found the currently active folder.
            m_pwpfolderActiveGroup = pwpfolderGroup;
         case 7:
            // Activates the group window and minimize it.
            //pwpfolderGroup->Close();
            break;                   
      }
   }
}
   
//---------------------------------------------------------------------
// ExecuteCommand():
//---------------------------------------------------------------------
BOOL DDEPROGMAN::ExecuteCommand(char *pszName,char *pszParameters)
{  
   char  *apszParameter[MAX_PARAMETERS];
   int   nParamCount;
   #ifdef _DEBUG   
      int   index;
   #endif
           
   #ifdef _DEBUG           
   // Debug information...
   dprintf("tr ExecuteCommand():\n   name: '%s'\n   param: '%s'\n",pszName,pszParameters);
   #endif

   // Clear parameter pointer list.
   memset(apszParameter,0,(sizeof(char *)*MAX_PARAMETERS));
                   
   // Parse out the parameters into a list of pointers to strings.
   ParseCommandParametersIntoList(pszParameters,apszParameter,&nParamCount);
              
   #ifdef _DEBUG           
   // Debug information.
   dprintf("tr ExecuteCommand():  param count = %d\n",nParamCount);
   for (index=0; index<nParamCount; index++)
      dprintf("   %d: '%s'\n",index,apszParameter[index]);
   #endif
      
   // Depending on which command to execute...
   // If command is 'AddItem' AND there is at least 1 parameter...
   if ((stricmp(pszName,"AddItem")==0)&&(nParamCount>=1))
   {                                     
      // Execute command.
      AddItem(apszParameter,nParamCount);
   }
   else                                            
   // If command is 'CreateGroup' AND there is at least 1 parameter...
   if ((stricmp(pszName,"CreateGroup")==0)&&(nParamCount>=1))
   {   
      // Execute command.
      CreateGroup(apszParameter,nParamCount);  
   }
   else
   // If command is 'DeleteGroup' AND there is at least 1 parameter...
   if ((stricmp(pszName,"DeleteGroup")==0)&&(nParamCount>=1))
   {                     
      // Instructs to delete an existing group.
      // Format of parameters is: "GroupName"
   }
   else
   if (stricmp(pszName,"DeleteItem")==0)
   {  
      // Instructs to delete item from currently active group.  
      // Format of parameters is: "ItemName"  
   }
   else
   if (stricmp(pszName,"ExitProgman")==0)
   {  
      // Instructs to exit and optionally save its information.                  
      // Format of parameters is: bSaveGroups
      // Ignore command.
   }
   else
   if (stricmp(pszName,"Reload")==0)
   {                        
      // Instructs to remove and reload and existing group.
      // Format of parameters is: "GroupName" (if empty means all groups).
      // Ignore command.
   }
   else
   // If command is 'ReplaceItem' AND there is exactly 1 parameters...
   if ((stricmp(pszName,"ReplaceItem")==0)&&(nParamCount==1))
   {                                          
      // Execute command.
      ReplaceItem(apszParameter,nParamCount);                
   }
   else 
   // If command is 'ShowGroup' AND there are exactly 2 parameters...
   if ((stricmp(pszName,"ShowGroup")==0)&&(nParamCount==2))
   {  
      // Execute command.
      ShowGroup(apszParameter,nParamCount);                
   }
   else                             
      // Failure executing command.
      return (FALSE);

   // Successfully executed command.
   return (TRUE);
}

//---------------------------------------------------------------------
// Uninitialize():
//---------------------------------------------------------------------
BOOL DDEPROGMAN::Uninitialize(void)
{  
   BOOL  fReturnValue;                                           
                                           
   // Call base class uninitialization function.
   fReturnValue = DDESERVER::Uninitialize();
   
   // Clean up the pointer to ourseleves (to be used by DDE callback 
   // fucntion to reference this class instantiation).
   gs_pddeserverPROGMAN = NULL;   
   
   // Return value from uninitialize function.
   return (fReturnValue);
}

//---------------------------------------------------------------------
// PROGMANServiceCallback():
//---------------------------------------------------------------------
HDDEDATA CALLBACK _loadds PROGMANServiceCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2)
{                                        
   // If dde server object for PROGMAN service is available...            
   if (gs_pddeserverPROGMAN!=NULL)
      return (gs_pddeserverPROGMAN->DdeCallback(uType,uFmt,hConv,hsz1,hsz2,hData,dwData1,dwData2));
   else
      // Return false by default.
      return (FALSE);
}


// EOF DDEPRGMN.CPP

