//=====================================================================
// WPNOTEBK.HPP
//    WPNOTEBOOK Class declaration.
// 
// Date Created :  03/10/94
// Last Update  :  06/10/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 03/10/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPNOTEBOOK_PROTOTYPED__

#define __WPNOTEBOOK_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// ENUMERATIONS
//---------------------------------------------------------------------
enum { PAGE_CREATE=0, PAGE_HELP       , PAGE_NEWSIZE, PAGE_SELECTED, 
       PAGE_UPDATE  , PAGE_COMMAND    , PAGE_PAINT  , PAGE_UPDATE_OBJECT,
       PAGE_DONE    , PAGE_DIALOG_DONE, PAGE_DELETED };

//---------------------------------------------------------------------
// CONSTANTS
//---------------------------------------------------------------------
#define MAX_PAGES_PER_NOTEBOOK   16

//---------------------------------------------------------------------
// DATA TYPES
//---------------------------------------------------------------------
// Pointer to generic page procedure.
typedef void (FAR WPOBJECT::*FARPAGEPROC)(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);

    
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPNOTEBOOK : public WPWINDOW 
{       
   private:
      HWND        m_hwndNotebookControl;
      int         m_nPageCount;
      WORD        m_wPageID[MAX_PAGES_PER_NOTEBOOK];
      HWND        m_hwndPage[MAX_PAGES_PER_NOTEBOOK];
      FARPAGEPROC m_fnPage[MAX_PAGES_PER_NOTEBOOK];
      WPOBJECT    *m_pwpobjOwner;          
      
   protected:
      HICON       m_hIcon;

   public:
      BOOL        m_fNCLButtonDown;
      
   protected:      
      DWORD                GetWindowStyles(void);                             
      BOOL                 CreateWindow(char *pszClassName,WPNOTEBOOK *pwpnbOwner);
      
      BOOL                 LoadOwnerIcon(void);

      virtual const char   *GetCaptionTitle(void);

      int                  GetIndexFromPageID(WORD wPageID);
      HWND                 GetWindowFromPageID(WORD wPageID);
      WORD                 GetPageIDFromWindow(HWND hWnd);
      HWND                 GetPageOnTop(void);
      
      FARPAGEPROC          GetPageProc(HWND hWnd);
      
   public:
      WPNOTEBOOK(WPOBJECT *pwpobjOwner);
      ~WPNOTEBOOK(void);
                           
      BOOL                 IsPageWindow(HWND hWnd);
      HWND                 GetWindowFromPageIndex(int nPageIndex);
                           
      WPOBJECT             *GetOwner(void);
      HICON                GetIconHandle(void);
      
      virtual BOOL         Create(int x,int y,int cx,int cy);
      void                 ShowNotebook(void);
      virtual BOOL         Close(void);

      BOOL                 InsertPage(char *pszTabText,WORD wPageStyle,FARPAGEPROC fnPage,char *pszStatusText,LPCSTR pszDialogTemplateName = NULL);
              
      void                 CallPageProc(HWND hWnd,WORD wAction,DWORD dwParam);

      // Client window procedure for notebook objects.      
      LRESULT              ClientWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
      
      // Window message processing.
      void                 OnCreate(void);
      void                 OnActivate(BOOL fActive);
      void                 OnMove(POINT ptTopLeft);
      void                 OnSize(WORD wSizeType,int nClientWidth,int nClientHeight);
      void                 OnCommand(HWND hwndNotebookControl,WORD wNotifyCode);
      void                 OnSysCommand(WORD wValue,LPARAM lParam);
      LRESULT              OnQueryDragIcon(void);
      void                 OnEraseBackground(HDC hdc);
      void                 OnNCPaint(void);
      void                 OnNCActivate(BOOL fActive);
      void                 OnNCMouseMove(WORD wHitTestCode,POINT ptDevice);
      void                 OnNCLButton(UINT uMsg,WORD wHitTestCode,POINT ptDevice);
      void                 OnMouseMove(void);
      void                 OnClose(void);
};               

#endif

// EOF WPNOTEBK.HPP

