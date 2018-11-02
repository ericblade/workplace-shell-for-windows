//=====================================================================
// DDEPRGMN.HPP
//    Windows DDE PROGMAN Service Server Class.
// 
// Date Created :  06/04/94
// Last Update  :  06/20/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 06/04/94    | Creation.                               | MDA
//=====================================================================
 
#ifndef __DDEPROGMAN_PROTOTYPED_

#define __DDEPROGMAN_PROTOTYPED__

 
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Maximum number of parameters allowed per DDE command executed.
#define MAX_PARAMETERS  12   

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class DDEPROGMAN : public DDESERVER
{     
   private:        
      // Pointer to Workplace Shell desktop object.
      WPDESKTOP   *m_pwpWPSDesktop;                
      // Data used as temporary storage when executing command strings.
      WPFOLDER    *m_pwpfolderActiveGroup;
      BOOL        m_fReplaceItem;
      POINT       m_ptIconPos;

   private:
      // Functions used to query all group names in Workplace Shell desktop.
      void     RecursivelyQueryGroupNames(WPFOLDER *pwpfolder,HDDEDATA *phData,DWORD *pdwDataOffset);
      void     QueryAllGroupNames(HDDEDATA *phData);
      // Functions used to query detailed information about a group in Workplace Shell desktop.
      BOOL     QueryDetailedGroupInfo(HSZ hszGroupTitle,HDDEDATA *phData);
      // Functions used to parse out parameter list int command strings.
      void     ParseCommandParametersIntoList(char *pszParamString,char **ppszParamList,int *nCount);
      // Functions used to execute the supported commands in DDE server.
      void     AddItem(char **ppszParameters,int nParamCount);
      void     CreateGroup(char **ppszParameters,int nParamCount);
      void     ReplaceItem(char **ppszParameters,int nParamCount);
      void     ShowGroup(char **ppszParameters,int nParamCount);
         
   protected:                 
      // PROGMAN data request processing.
      HDDEDATA ServiceRequest(HSZ hszTopic,HSZ hszItem);
   
      // PROGMAN command string processing.
      BOOL     ExecuteCommand(char *pszName,char *pszParameters);
      
   public:
      // Initialization function.
      BOOL     Initialize(WPDESKTOP *pwpWPSDesktop); 

      // Unitialization function.
      BOOL     Uninitialize(void);
};

#endif
      
// EOF DDEPRGMN.HPP

