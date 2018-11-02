//=====================================================================
// PROGMAN.HPP
//    Program Manager DDE class declarations header file.
//
// Date Created :  05/06/94
// Last Update  :  05/16/94
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
                             
#include <ddeml.h>                             
                             
//---------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------
#define MAX_STRING   63

//---------------------------------------------------------------------
// CLASS DECLARATIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ITEMINFO
class ITEMINFO {
   public:
      char     m_szTitle[64];        // the program title
      char     m_szCommandLine[64];  // the command line: path (optional), executable name, and arguments
      char     m_szParameters[64];   // the parameters passed to the program.
      char     m_szWorkDir[64];      // the working directory (can be NULL)
      char     m_szIconPath[64];     // the file/executable containing the icon (can be NULL)
      int      m_iIconIndex;         // the icon number of the icon in the icon file
      int      m_x, m_y;             // the position of the program object in the group
      int      m_bMinimize;          // minimize at startup flag (TRUE or FALSE)
      int      m_iHotKey;            // the hotkey, add together:  Ctrl  = 1024
                                     //                            Alt   = 512
                                     //                            Shift = 256
                                     // and the ascii value of key.  (Not my method, Microsquish's :)
};                                


//---------------------------------------------------------------------
// GROUPINFO (Program Manager Group Detail Information Structure).
class GROUPINFO {
   public:
      char        m_szTitle[64];
      int         m_nItemCount;
      ITEMINFO    *m_pItemInfo;
};
      

//---------------------------------------------------------------------
// PROGMANDDE (Program Manager Shell DDE Interface Client Class).
class PROGMANDDE {
   private: 
      HINSTANCE   m_hInstance;
      DWORD       m_dwInstID;   
      HCONV       m_hConv;
      HSZ         m_hszPROGMAN;
      HSZ         m_hszGROUPS;
      FARPROC     m_pfnCallBack;
      
      int         m_nGroupCount;
      GROUPINFO   *m_pGroupInfo;

   protected:
      char  *ParseLine(char *psz,char *pszDelimiters);
      void  ParseDetailInfoString(GROUPINFO *pGroupInfo,char *pszBuffer);
      BOOL  ReadGroupDetails(GROUPINFO *pGroupInfo);
   
   public:
      PROGMANDDE(void);
      ~PROGMANDDE(void);
      
      // DDE client connect & disconnect functions.
      BOOL        Initialize(HINSTANCE hInstance);
      BOOL        Connect(void);
      BOOL        Disconnect(void); 
      BOOL        Uninitialize(void);
             
      // DDE query information functions.             
      BOOL        ReadAllGroups(void);
                     
      int         GetGroupCount(void);
      GROUPINFO   *GetGroupDetails(int index);
      GROUPINFO   *GetGroupDetails(char *pszGroupName);
};

// EOF PROGMAN.HPP


