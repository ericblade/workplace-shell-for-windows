//=====================================================================
// WPLAUNCH.HPP
//    WPLAUNCHPAD Class declaration.
// 
// Date Created :  11/05/95
// Last Update  :  11/05/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/25/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPLAUNCHPAD_PROTOTYPED__

#define __WPLAUNCHPAD_PROTOTYPED__

//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPLAUNCHPAD : public WPFOLDER
{
   private:               
      int               m_nColumnCount;
      LIST              m_ColumnList;
   
   public:       
      // Window creation and display functions.
      virtual DWORD     GetWindowExStyles(void);
      virtual DWORD     GetWindowStyles(void);
   
      // Object components management functions.
      virtual BOOL      AddObject(WPOBJECT *pwpobjAdd,int nPos=-1);
      virtual BOOL      DeleteObject(WPOBJECT *pwpobjDelete);        
      virtual BOOL      DeleteObject(int nPos);

      // Object components query functions.
      virtual int       GetObjectCount(void);
      virtual WPOBJECT  *GetObject(int nPos);
      virtual int       GetObjectIndex(WPOBJECT *pwpobjQuery);
      
      // Window messages management functions.
      virtual LRESULT   ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
      virtual void      OnCreate(void);
      virtual void      OnCommand(WORD wValue);
      virtual void      OnPopupMenu(POINT ptDevice);
      virtual void      OnNCPaint(void);
      virtual void      OnNCActivate(BOOL fActive);
      virtual void      OnNCMouseMove(WORD wHitTestCode,POINT ptDevice);
      virtual LRESULT   OnWin31DragAndDropMsg(UINT uMsg,WPARAM wParam,LPARAM lParam);
      virtual LRESULT   OnEraseBackground(HDC hdc);
      virtual void      OnPaint(void);
      virtual void      OnClose(void);
      
      // Settings notebook functions.
      virtual void  InsertSettingsNotebookPages(void);
      
};

#endif

// EOF WPLAUNCH.HPP


