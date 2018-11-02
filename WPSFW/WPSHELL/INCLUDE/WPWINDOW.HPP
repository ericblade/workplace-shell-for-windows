//=====================================================================
// WPWINDOW.HPP
//    WPWINDOW Class declaration.
// 
// Date Created :  01/26/94
// Last Update  :  03/09/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/26/94    | Creation.                               | MDA
//=====================================================================
     
#ifndef __WPWINDOW_PROTOTYPED__

#define __WPWINDOW_PROTOTYPED__
     
     
//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
      
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPWINDOW
{
   private:                              
      // Window handle and device context information.
      HWND        m_hwnd;
      HICON       m_hIcon;
      HDC         m_hdc;
      PAINTSTRUCT m_ps;
           
      // Window screen position and size information.
      POINT       m_ptWindow;        // top-left point of window (screen coordinates).
      SIZE        m_sizeWindow;      // width and height of window (screen coordinates).
      RECT        m_rcClient;        // client area (screen coordinates).
      
      // Logical system area information.
      RECT        m_rcLogical;       // logical domain of window (logical coordinates).
      RECT        m_rcVisible;       // visible area of logical domain (logical coordinates).
      
      // Vert and horiz scrollbar information.
      BOOL        m_fScrollbarHasInputCaptured; // has a scrollbar captured the user input?
      BOOL        m_fScrollable;                // is window area scrollable?
      WORD        m_wScrollbars;                // which scrollbars are present (if any).
      BYTE        m_bScrollComponentsState;     

   private:       
      // Scrollbars management functions.
      WORD          ScrollbarHitTest(WORD wSBCode,POINT ptWindow,int *piNewThumbPos);
      HBITMAP       LoadScrollComponentBitmap(WORD wComponent,WORD wState);
      SIZE          GetScrollComponentSize(WORD wComponent);
      RECT          GetScrollbarFrameRect(WORD wSBCode);
      WORD          SetScrollComponentState(WORD wComponent,WORD wState);
      WORD          GetScrollComponentState(WORD wComponent);
      RECT          GetScrollComponentRect(WORD wSBCode,WORD wComponent);
      void          ScrollWindowVert(int dy);
      void          ScrollWindowHoriz(int dx);

      // Window colors functions.
      COLORREF      GetFrameFillColor(BOOL fActive); 
      COLORREF      GetCaptionFillColor(BOOL fActive); 
      COLORREF      GetCaptionTextColor(BOOL fActive); 
      
      // Delay function used in window animation function.
      void          Wait(int nDelay);
      
      // Draw bitmap primitive function.
      void          DrawBitmap(HBITMAP hbm,HDC hdc,int x,int y);
                              
      // Non-client area painting primitives used by member function NCPaint().
      void          FillFrame(HDC hdc,int cx,int cy,BOOL fActive);
      void          FillCaptionBar(HDC hdc,int cx,int cy,BOOL fActive);
      void          Outline3DFrame(HDC hdc,int cx,int cy);
      void          DrawCaptionBarText(HDC hdc,int cx,int cy,BOOL fActive);
      void          DrawIconOnSystemMenu(HDC hdc,HICON hIcon);
            
   protected:
      // Window zoom in/out animation function.
      void          AnimateWindowFrame(LPRECT lprcStart,LPRECT lprcEnd,int nDelay,int nSteps);
    
      // Min/max buttons draw functions.
      void          DrawMinMaxButtonsHideStyle(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown);
      void          DrawMinMaxButtonsMinStyle(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown);
      virtual void  DrawMinMaxButtons(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown);
      
      // Scrollbars window controls drawing functions.
      void          DrawScrollComponent(HDC hdc,WORD wSBCode,WORD wComponent,WORD wState);
      void          DrawScrollArrows(HDC hdc,WORD wSBCode);
      void          DrawScrollArea(HDC hdc,WORD wSBCode);
      void          DrawScrollThumb(HDC hdc,WORD wSBCode);
      void          DrawScroll3DOutline(HDC hdc,WORD wSBCode);
      void          DrawScrollIntersectionRect(HDC hdc);
      void          DrawScrollbar(HDC hdc,WORD wSBCode);
                    
   public:
      WPWINDOW(void);
      WPWINDOW(WPWINDOW *pwpwindowDefault);
      ~WPWINDOW(void);
      
      // Window properties management functions.
      HWND          GetHandle(void);
      void          SetHandle(HWND hwnd);
      
      // Window positions and area management functions.
      void          SetWndPos(POINT ptWindow);
      void          SetWndPos(int x,int y);     
      POINT         GetWndPos(void);
      void          SetWndSize(SIZE sizeWindow);
      void          SetWndSize(int cx,int cy);
      SIZE          GetWndSize(void);
      void          SetLogicalDomain(LPRECT lprcLogical);
      RECT          GetLogicalDomain(void);
      void          SetVisibleArea(LPRECT lprcVisible);
      RECT          GetVisibleArea(void);
      BOOL          isPointVisible(POINT ptLogical);
                    
      // Scrollbar controls management functions.
      BOOL          isScrollbarCapturingInput(void); 
      void          SetScrollbarInputCaptureFlag(BOOL fCapture);
      BOOL          isScrollbarEnabled(WORD wSBCode);
      int           SetScrollPos(HWND hwnd,int nSBCode,int nPos,BOOL fRepaint);
      void          ShowScrollbar(WORD wSBCode,BOOL fEnable);                  
      void          ArrangeScrollbars(void);
      void          AdjustScrollbars(RECT rcVisible,RECT rcLogical);
      LRESULT       ScrollbarWindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam);
      
      // Scrollbar properties and client message handling functions.
      BOOL          isScrollable(void);
      void          SetScrollProperty(BOOL fScrollable);    // default is TRUE.
      void          OnScrollbarMessage(UINT uMsg,WORD wScrollCode,int nPos,HWND hwndSBCtl);

      // Window creation and display functions.
      virtual BOOL  CreateWindow(char *pszClassName,WPOBJECT *pwpobjOwner);
      virtual BOOL  DestroyWindow(void);
      virtual DWORD GetWindowExStyles(void);
      virtual DWORD GetWindowStyles(void);
      virtual void  UpdateWindowPos(void);
      
      // Window client-area painting functions.
      HDC           BeginPaint(void); // for WM_PAINT message.
      void          EndPaint(void);    
      HDC           GetDC(void);      // for any other time (except WM_PAINT).
      void          ReleaseDC(void);
      
      // Coordinate system convertion functions.
      // LP = Logical Point      WP = Window Point      DP = Device Point.
      void          LPtoWP(LPPOINT lpptLogical,int nPoints=1);
      void          WPtoDP(LPPOINT lpptWindow,int nPoints=1);
      void          DPtoWP(LPPOINT lpptDevice,int nPoints=1);
      void          WPtoLP(LPPOINT lpptWindow,int nPoints=1);
      void          LPtoDP(LPPOINT lpptLogical,int nPoints=1);
      void          DPtoLP(LPPOINT lpptDevice,int nPoints=1);
 
      // Window non-client area painting functions.
      void          NCPaint(BOOL fActive,HICON hIcon);
      
      // Profile management functions.
      BOOL          ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL          WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPWINDOW.HPP

