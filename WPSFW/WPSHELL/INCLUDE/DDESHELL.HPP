//=====================================================================
// DDESHELL.HPP
//    Windows DDE SHELL Service Server Class.
// 
// Date Created :  09/01/94
// Last Update  :  03/13/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 09/01/94    | Creation.                               | MDA
//=====================================================================
 
#ifndef __DDESHELL_PROTOTYPED_

#define __DDESHELL_PROTOTYPED__

 
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
class DDESHELL : public DDESERVER
{     
   private:        
      // Pointer to Workplace Shell desktop object.
      WPDESKTOP   *m_pwpWPSDesktop;                
      
      // Information about current program being executed.
      HINSTANCE   m_hInstProgram;
      WPPROGRAM   *m_pwpprogOwner;
         
   protected:                 
      // SHELL data request processing.
      HDDEDATA ServiceRequest(HSZ hszTopic,HSZ hszItem);

      // SHELL execute commands function (empty since not applicable). 
      BOOL     ExecuteCommand(char *pszName,char *pszParameters);
   
   public:
      // Initialization function.
      BOOL     Initialize(WPDESKTOP *pwpWPSDesktop); 

      // WinOldApp's instance and owner object setting function.
      void     SetProgramInstanceAndObject(HINSTANCE hInstance,WPPROGRAM *pwpprogOwner);

      // DDE callback function.
      HDDEDATA DdeCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2);
      
      // Unitialization function.
      BOOL     Uninitialize(void);
};

#endif
      
// EOF DDESHELL.HPP

