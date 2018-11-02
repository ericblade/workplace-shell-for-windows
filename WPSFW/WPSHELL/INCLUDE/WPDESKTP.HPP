//=====================================================================
// WPDESKTOP.HPP
//    WPDESKTOP Class declaration.
//
// Date Created :  01/26/94
// Last Update  :  10/08/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/26/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPDESKTOP_PROTOTYPED__

#define __WPDESKTOP_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Desktop system menu name (popup menu).
#define WPDESKTOP_SYSMENU_NAME   DesktopPopUpMenu   // defined in WPSHELL.RC
// Maximum top-level windows allowed in system.
#define MAX_TOP_LEVEL_WINDOWS    64

// Desktop object behavior and functionality flags.
// Desktop's save settings flag.
#define DESKTOP_FLAGS_MASK      0x0F000000L
#define DESKTOP_SAVESETTINGS    0x01000000L

// Workplace Shell profile magic identification number.
#define WPSHELL_PROFILE_MAGIC_NUMBER   0x4B454F48L

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// WPS Profile Header data structure definition.
typedef struct {
   DWORD    m_dwMagicID;         // 'HOEK'
   WORD     m_wMajorVersion;     // i.e. 1.4 = 0x0001
   WORD     m_wMinorVersion;     // i.e. 1.4 = 0x0040
   DWORD    m_dwDataOffset;      // file offset to begining of data.
} WPSHELL_PROFILE_HEADER;

// Top-level window information data structure used by Desktop
// object to track created top-level windows.
typedef struct {
   HINSTANCE   m_hInstance;
   HWND        m_hwnd;
   WORD        m_wMinBehavior;
   WPOBJECT    *m_pwpobjOwner;
} TOPLEVEL_WINDOW, *PTOPLEVEL_WINDOW;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPDESKTOP : public WPFOLDER
{
   private:
      // Current Windows shell information.
      BOOL              m_fIsShell;
   
      // Windows 3.1 desktop window sub-classing functions.
      WNDPROC           m_wndprocWindows31Desktop;

      // Shell hook information.
      HHOOK             m_hhookShell;
      FARPROC           m_lpfnShellHookProc;

      // Top-level window information.
      int               m_nTopLevelWindowCount;
      TOPLEVEL_WINDOW   m_TopLevel[MAX_TOP_LEVEL_WINDOWS];

      // DDE services supplied by Windows shells.
      DDEPROGMAN        m_ddeserverPROGMAN;     // PROGMAN's service.
      DDESHELL          m_ddeserverSHELL;       // DOS Program's service.

   private:
      // Windows 3.1 desktop window sub-classing functions.
      BOOL     IsATMDriverLoaded(void);
      void     Windows31SubclassDesktop(void);
      void     Windows31UnsubclassDesktop(void);

      // Shell hook management functions.
      void     RegisterShellHook(void);
      void     UnregisterShellHook(void);

      // Internal top-level window array management functions.
      BOOL     AddTopLevelWindow(HWND hwndTopLevel);
      BOOL     DeleteTopLevelWindow(HWND hwndTopLevel);

      // Shell initialization functions (for PROGMAN functional compatibility).
      void     ShellLoadAndRun(void);

      // Workplace Shell object profile support functions.
      void     EnumerateWPSObjects(WPFOLDER *pwpfolderParent,WORD *pwCurrentID);
      WPOBJECT *FindUniqueID(WPOBJECT *pwpobjRoot,WORD wUniqueID);
      BOOL     BindShadowObjectsToOriginalObjects(void);

   protected:
      // Window Minimized Placement function.
      void     UpdateMinPlacement(HWND hWnd,WORD wMinBehavior);
      
      // Mouse management functions (all points are in device coordinates).
      void      OnLeftMouseButtonDblClk(POINT ptDevice);

   public:
      WPDESKTOP(void);

      // Appearance attributes set & query functions
      void        SetBackgroundColor(COLORREF clrrefBackground);
      COLORREF    GetBackgroundColor(void);
      BOOL        GetImageAttribute(DWORD dwAttributeFlag);
      void        SetImageAttribute(DWORD dwAttributeFlag);
      char        *GetImagePath(void);
      void        SetImagePath(char *pszImagePath);

      // Folder caption text function.
      const char  *GetCaptionTitle(void);

      // Desktop default values.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);

      // Desktop state initialization functions.
      DWORD       GetWindowExStyles(void);
      DWORD       GetWindowStyles(void);
      BOOL        Open(void);
      BOOL        InitializeDesktop(void);
      BOOL        ReadDesktop(void);
      BOOL        SaveDesktop(void);

      // Windows 3.1 desktop window sub-classing functions.
      WNDPROC     GetWindows31DesktopProc(void);

      // Class management functions.
      void        SetClassBackgroundBrush(HBRUSH *phbrBackground);
      void        UnregisterClassName(char *pszClassName);
      void        UnregisterClassName(WORD wClassID);

      // Drawing and updating functions.
      void        InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd);
      void        Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd);
      void        UpdateWindow(void);

      // Desktop system menu commands functions.
      void        RunProgram(void);

      // Popup menu properties.
      void        InitializePopup(HMENU hmenuPopup);
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);
      void        KeyboardEvent(WORD wEventID);

      // Top-level window query support functions.
      WORD        GetTopLevelWindowMinimizeBehavior(HWND hwnd);
      WPOBJECT    *GetObjectFromInstanceHandle(HINSTANCE hInstance);

      // Desktop Workplace Shell message processing functions.
      void        OnTopLevelWindowCreated(HWND hwndCreated);
      void        OnTopLevelWindowDestroyed(HWND hwndDestroyed);
      void        OnUpdateObjectMinPlacement(HINSTANCE hInstUpdate);        
      BOOL        OnFindInstanceHandle(HINSTANCE hInstance);
      void        OnAssociateObject(HINSTANCE hInstance,WPOBJECT *pwpobjOwner);
      void        OnAssociateObject(HWND hwnd,WPOBJECT *pwpobjOwner);
      HWND        OnGetWindowFromObject(WPOBJECT *pwpobjOwner);
      void        OnNotifyObjectDestroyed(WPOBJECT *pwpobjDestroyed);
      void        OnDeleteObject(WPOBJECT *pwpobjDelete);

      // Desktop message processing functions.
      LRESULT     ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
      void        OnCreate(void);
      void        OnDestroy(void);
      void        OnWinINIChange(LPCSTR pszSection);
      void        OnNCPaint(void);
      LRESULT     OnEraseBackground(HDC hdc);

      // Settings notebook functions.
      void        InsertSettingsNotebookPages(void);
      void        ImageControls(HWND hwndBackground,BOOL fEnable);
      void        LockupPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      void        DesktopPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);

      // System shutdown query function.
      BOOL        IsShell(BOOL fCheck=FALSE);
      BOOL        QueryQuit(void);
      
      // Desktop size check & update done peridiocally.
      BOOL        CheckDesktopSizeChange(void);
      
      // Profile management functions.
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPDESKTP.HPP

