//=====================================================================
// WPSYSSET.HPP
//    WPSYSSETUP Class declaration.
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
     
#ifndef __WPSYSSETUP_PROTOTYPED__

#define __WPSYSSETUP_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPSYSSETUP : public WPSETUP
{
   public:                                  
      WPSYSSETUP(void);
      WPSYSSETUP(WPSYSSETUP *pwpsyssetDefaults);

      // Program object default values.
      void     GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      
      // Settings notebook functions.
      void     InsertSettingsNotebookPages(void);
      // 'Window' page functions.
      void     UpdateWindowPageSYSTEMProfile(HWND hwndPage);
      void     GetWindowPageSYSTEMProfile(void);
      void     UpdateWindowPageControls(HWND hwndPage);
      void     WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      void     UpdateWindow2PageSYSTEMProfile(HWND hwndPage);
      void     GetWindow2PageSYSTEMProfile(void);
      void     UpdateWindow2PageControls(HWND hwndPage);
      void     Window2PageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      // 'Confirmations' page functions.
      void     UpdateConfirmationsPageSYSTEMProfile(HWND hwndPage);
      void     GetConfirmationsPageSYSTEMProfile(void);
      void     UpdateConfirmationsPageControls(HWND hwndPage);
      void     ConfirmationsPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
};

#endif

// EOF WPSYSSET.HPP

