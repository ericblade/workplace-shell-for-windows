//=====================================================================
// WPMINWIN.HPP
//    WPMINVIEWER Class declaration.
//
// Date Created :  04/02/94
// Last Update  :  11/15/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 04/02/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPMINVIEWER_PROTOTYPED__

#define __WPMINVIEWER_PROTOTYPED__


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// MINIMIZED WINDOW OBJECT CLASS
//---------------------------------------------------------------------
class WPMINWNDOBJ : public WPOBJECT
{
   private:
      HWND        m_hwndMinWnd;

   public:
      // Minimized window object constructor.
      WPMINWNDOBJ(void);

      // Minimized window handle management functions.
      void        SetMinWndHandle(HWND hwndMinWnd);
      HWND        GetMinWndHandle(void);

      // Minimized window object state functions.
      BOOL        Open(int nCmdShow);
      BOOL        Open(void);

      // Appearance and position functions.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      BOOL        LoadIcon(void);

      // Popup menu properties functions
      void        InitializePopup(HMENU hmenuPopup);
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);

      // Drag & drop property management functions.
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
};


//---------------------------------------------------------------------
// MINIMIZED WINDOW VIEWER CLASS
//---------------------------------------------------------------------
class WPMINVIEWER : public WPFOLDER
{
   private:
      WPDESKTOP      *m_pwpDesktop;

   public:
      WPMINVIEWER(void);
      WPMINVIEWER(WPMINVIEWER *pwpminviewerDefaults);

      // Minimized window viewer default values.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);

      // Minimized window viewer state functions.
      BOOL        Close(void);

      // Popup menu properties.
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);

      // Minimized window viewer contents management functions.
      int         QueryCurrentMinWindows(HWND *ahwndMinWindows);
      BOOL        IsWindowMinimizedToViewer(HWND hwndWindow);
      BOOL        IsObjectValid(WPMINWNDOBJ *pwpobjTest,WPMINWNDOBJ **apwpminwndValidList,int nListSize);
      WPMINWNDOBJ *GetMinWindowObjectInViewer(HWND hwnd);
      POINT       GetNewMinWndIconPos(void);
      void        UpdateMinViewerContents(int nCount,HWND *ahwndWindows);
      void        OnInitializeFolder(void);

      // Minimized viewer message processing functions.
      void        OnTimer(WORD wTimerID);

      // Drag & drop property management functions.
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);

      // Shell profile management functions.
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPMINWIN.HPP

