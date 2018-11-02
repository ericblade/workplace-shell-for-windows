//=====================================================================
// WPWINDOW.CPP
//    WPWINDOW Class implementation.
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

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <io.h>
#include <time.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Default window styles and extended styles.
#define WPWINDOW_DEFAULT_EX_STYLES  (WS_EX_ACCEPTFILES)
#define WPWINDOW_DEFAULT_STYLES     (WS_OVERLAPPEDWINDOW|WS_VISIBLE)
// No scrollbars enabled value.
#define IDT_SCROLLBAR               100   // Scrollbar initial delay timer ID.
#define IDT_SCROLLBAR_REPEAT        101   // Scrollbar repeat message timer ID.
#define SB_NONE                     0xFFFF
enum { SBC_THUMB, SBC_LEFT_ARROW,  SBC_RIGHT_ARROW, SBC_UP_ARROW, SBC_DOWN_ARROW,
                  SBC_LEFT_RANGE,  SBC_RIGHT_RANGE, SBC_UP_RANGE, SBC_DOWN_RANGE,
                  SBC_TOTAL_RANGE, SBC_NONE };
enum { SBS_NORMAL, SBS_PUSHEDIN };


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef struct {
   POINT    m_ptWindow;
   SIZE     m_sizeWindow;
   BOOL     m_fScrollable;
} FILE_WINDOW_STRUCT;


//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static WORD    g_wOriginalComponentHit;


//---------------------------------------------------------------------
// WORKPLACE SHELL WINDOW CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SCROLLBAR MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ScrollbarHitTest():
//---------------------------------------------------------------------
WORD WPWINDOW::ScrollbarHitTest(WORD wSBCode,POINT ptWindow,int *piNewThumbPos)
{
   RECT     rcComponent,rcRange;
   float    flRatio;
   int      iMin,iMax;

   // First test if point falls within scrollbar's frame area.
   rcComponent = GetScrollbarFrameRect(wSBCode);

   // Check if point falls within rectangle.
   if (PtInRect(&rcComponent,ptWindow)==FALSE)
      return (SBC_NONE);

   switch (wSBCode)
   {
      case SB_HORZ:
         // Test for scrollbar's thumb.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_THUMB);
         if ((g_wOriginalComponentHit==SBC_THUMB)||(PtInRect(&rcComponent,ptWindow)==TRUE))
         {
            // Get total possible range.
            rcRange = GetScrollComponentRect(wSBCode,SBC_TOTAL_RANGE);
            // Calculate scrollbar range ratio.
            flRatio = (ptWindow.x - rcRange.left)/(float)(rcRange.right - rcRange.left);
            // Get current scrollbar range.
            GetScrollRange(GetHandle(),SB_HORZ,&iMin,&iMax);
            // Calculate new thumb position.
            *piNewThumbPos = (int)(iMin + (flRatio * (iMax-iMin)));
            return (SBC_THUMB);
         }
         // Test for scrollbar's arrows.
         // Left arrow.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_LEFT_ARROW);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_LEFT_ARROW);
         // Right arrow.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_RIGHT_ARROW);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_RIGHT_ARROW);
         // Test for scrollbar's left range.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_LEFT_RANGE);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_LEFT_RANGE);
         // Test for scrollbar's right range.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_RIGHT_RANGE);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_RIGHT_RANGE);
         break;

      case SB_VERT:
         // Test for scrollbar's thumb.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_THUMB);
         if ((g_wOriginalComponentHit==SBC_THUMB)||(PtInRect(&rcComponent,ptWindow)==TRUE))
         {
            // Get total possible range.
            rcRange = GetScrollComponentRect(wSBCode,SBC_TOTAL_RANGE);
            // Calculate scrollbar range ratio.
            flRatio = (ptWindow.y - rcRange.top)/(float)(rcRange.bottom - rcRange.top);
            // Get current scrollbar range.
            GetScrollRange(GetHandle(),SB_VERT,&iMin,&iMax);
            // Calculate new thumb position.
            *piNewThumbPos = (int)(iMin + (flRatio * (iMax-iMin)));
            return (SBC_THUMB);
         }
         // Test for scrollbar's arrows.
         // Up arrow.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_UP_ARROW);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_UP_ARROW);
         // Down arrow.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_DOWN_ARROW);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_DOWN_ARROW);
         // Test for scrollbar's up range.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_UP_RANGE);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_UP_RANGE);
         // Test for scrollbar's down range.
         rcComponent = GetScrollComponentRect(wSBCode,SBC_DOWN_RANGE);
         if (PtInRect(&rcComponent,ptWindow)==TRUE)
            return (SBC_DOWN_RANGE);
         break;
   }

   // No scrollbar component's were hit.
   return (SBC_NONE);
}

//---------------------------------------------------------------------
// LoadScollComponentBitmap():
//---------------------------------------------------------------------
HBITMAP WPWINDOW::LoadScrollComponentBitmap(WORD wComponent,WORD wState)
{
   LPCSTR pszComponent;

   // Depending on what screen resolution we are currently in...
   if (GetSystemMetrics(SM_CXVSCROLL)<21)
   {
      // 640x480 VGA resolution.
      // 800x600 SVGA (large fonts) resolution.
      // 1280x1024 SVGA (or XGA/2) (small fonts) resolution.
      switch (wComponent)
      {
         case SBC_THUMB:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_THUMB);
            else
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_THUMB_PUSH);
            break;

         case SBC_LEFT_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_LEFT);
            else
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_LEFT_PUSH);
            break;

         case SBC_RIGHT_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_RIGHT);
            else
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_RIGHT_PUSH);
            break;

         case SBC_UP_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_UP);
            else
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_UP_PUSH);
            break;

         case SBC_DOWN_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_DOWN);
            else
               pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_DOWN_PUSH);
            break;

         default:
            pszComponent = MAKEINTRESOURCE(IDB_LORES_SB_THUMB);
            break;
      }
   }
   else
   {
      // 800x600 SVGA (small fonts) resolution.
      // 1024x768 SVGA (or XGA) resolution.
      // 1280x1024 SVGA (or XGA/2) resolution.
      switch (wComponent)
      {
         case SBC_THUMB:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_THUMB);
            else
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_THUMB_PUSH);
            break;

         case SBC_LEFT_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_LEFT);
            else
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_LEFT_PUSH);
            break;

         case SBC_RIGHT_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_RIGHT);
            else
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_RIGHT_PUSH);
            break;

         case SBC_UP_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_UP);
            else
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_UP_PUSH);
            break;

         case SBC_DOWN_ARROW:
            if (wState==SBS_NORMAL)
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_DOWN);
            else
               pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_DOWN_PUSH);
            break;

         default:
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_SB_THUMB);
            break;
      }
   }

   // Return the handle to scrollbar component bitmap.
   return (LoadBitmap(g_wpEnvironment.hInst,pszComponent));
}


//---------------------------------------------------------------------
// GetScrollComponentSize():
//---------------------------------------------------------------------
SIZE WPWINDOW::GetScrollComponentSize(WORD wComponent)
{
   SIZE     sizeComponent;
   HBITMAP  hbmp;
   BITMAP   bm;

   // Load scroll component's bitmap.
   hbmp = LoadScrollComponentBitmap(wComponent,SBS_NORMAL);

   // Get bitmap object's size.
   GetObject(hbmp,sizeof(BITMAP),&bm);

   // Delete loaded bitmap resource.
   DeleteObject(hbmp);

   // Arrange return value into structure.
   sizeComponent.cx = bm.bmWidth;
   sizeComponent.cy = bm.bmHeight;

   // Return component's size.
   return (sizeComponent);
}


//---------------------------------------------------------------------
// GetScrollbarFrameRect():
//---------------------------------------------------------------------
RECT WPWINDOW::GetScrollbarFrameRect(WORD wSBCode)
{
   RECT rcWindow,rcScrollbarFrame;
   SIZE sizeWindow;

   // Get current window size.
   GetWindowRect(GetHandle(),&rcWindow);
   sizeWindow.cx = rcWindow.right - rcWindow.left + 1;
   sizeWindow.cy = rcWindow.bottom - rcWindow.top + 1;

   // Depending on which scrollbar we need...
   switch (wSBCode)
   {
      case SB_HORZ:
         // Calculate horizontal scrollbar rectangle.
         SetRect(&rcScrollbarFrame,
            GetSystemMetrics(SM_CXFRAME),
            sizeWindow.cy-GetSystemMetrics(SM_CYFRAME)-GetSystemMetrics(SM_CYHSCROLL),
            sizeWindow.cx-GetSystemMetrics(SM_CXFRAME)-2,
            sizeWindow.cy-GetSystemMetrics(SM_CYFRAME)-2);

         // If the vertical scrollbar is active,
         if (isScrollbarEnabled(SB_BOTH)==TRUE)
            // Shorten the width of horz. scrollbar.
            rcScrollbarFrame.right -= (GetSystemMetrics(SM_CXVSCROLL)-1);
         break;


      case SB_VERT:
         // Calculate vertical scrollbar rectangle.
         SetRect(&rcScrollbarFrame,
            sizeWindow.cx-GetSystemMetrics(SM_CXFRAME)-GetSystemMetrics(SM_CXVSCROLL),
            GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION)-1,
            sizeWindow.cx-GetSystemMetrics(SM_CXFRAME)-2,
            sizeWindow.cy-GetSystemMetrics(SM_CYFRAME)-2);

         // If both scrollbars are active,...
         if (isScrollbarEnabled(SB_BOTH)==TRUE)
            // Shorten the height of the vertical scrollbar.
            rcScrollbarFrame.bottom -= (GetSystemMetrics(SM_CYHSCROLL)-1);
         break;
   }

   // Return scrollbar control's frame rectangle.
   return (rcScrollbarFrame);
}


//---------------------------------------------------------------------
// SetScrollComponentState():
//---------------------------------------------------------------------
WORD WPWINDOW::SetScrollComponentState(WORD wComponent,WORD wState)
{
   BYTE bMask[] = { 0x01, 0x02, 0x04, 0x08, 0x10 };
   WORD wPrevState;

   // Get current state of scrollbar component, and save it.
   wPrevState = GetScrollComponentState(wComponent);

   // As long as we have a mask available for such a component.
   if (wComponent<sizeof(bMask))
   {
      // Clear state bit.
      m_bScrollComponentsState &= (~bMask[wComponent]);
      // If scrollbar component is pushed in, then set state bit.
      if (wState==SBS_PUSHEDIN)
         m_bScrollComponentsState |= bMask[wComponent];
   }

   // Return previous state of scrollbar component.
   return (wPrevState);
}


//---------------------------------------------------------------------
// GetScrollComponentState():
//---------------------------------------------------------------------
WORD WPWINDOW::GetScrollComponentState(WORD wComponent)
{
   BYTE bMask[] = { 0x01, 0x02, 0x04, 0x08, 0x10 };

   // As long as we have a mask available for such a component.
   if (wComponent<sizeof(bMask))
   {
      // Test state bit.
      if (m_bScrollComponentsState & bMask[wComponent])
         return (SBS_PUSHEDIN);  // component is pushed in.
   }

   // Component is in normal state.
   return (SBS_NORMAL);
}

//---------------------------------------------------------------------
// GetScrollComponentRect():
//---------------------------------------------------------------------
RECT WPWINDOW::GetScrollComponentRect(WORD wSBCode,WORD wComponent)
{
   RECT  rcScrollbarFrame,rcComponent,rcThumb;
   SIZE  sizeComponent;
   int   nPos,iMin,iMax,nOffset;

   // Get scrollbar's frame rectangle.
   rcScrollbarFrame = GetScrollbarFrameRect(wSBCode);

   // Depending on which scrollbar control...
   switch (wSBCode)
   {
      case SB_HORZ:
         switch (wComponent)
         {
            case SBC_THUMB:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_LEFT_ARROW);
               rcScrollbarFrame.left  += (sizeComponent.cx+1);
               rcScrollbarFrame.right -= (sizeComponent.cx+1);

               // Get thumb size.
               sizeComponent = GetScrollComponentSize(SBC_THUMB);

               // Reduce right side of scrollbar by the width of the thumb.
               rcScrollbarFrame.right -= (sizeComponent.cx-1);

               // Get current thumb position.
               nPos = GetScrollPos(GetHandle(),SB_HORZ);
               GetScrollRange(GetHandle(),SB_HORZ,&iMin,&iMax);

               // Calculate thumb offset (in pixels) from top of scrollbar.
               if ((iMax-iMin)!=0)
                  nOffset = (int)((rcScrollbarFrame.right - rcScrollbarFrame.left) * ((nPos-iMin) / (float)(iMax-iMin)));
               else
                  nOffset = 0;

               // Calculate thumb rectangle.
               sizeComponent      = GetScrollComponentSize(SBC_THUMB);
               rcComponent.left   = rcScrollbarFrame.left + nOffset;
               rcComponent.top    = rcScrollbarFrame.top + 1;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top + sizeComponent.cy;
               break;

            case SBC_LEFT_ARROW:
               sizeComponent = GetScrollComponentSize(SBC_LEFT_ARROW);
               rcComponent.left   = rcScrollbarFrame.left + 1;
               rcComponent.top    = rcScrollbarFrame.top + 1;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top + sizeComponent.cy;
               break;

            case SBC_RIGHT_ARROW:
               sizeComponent      = GetScrollComponentSize(SBC_RIGHT_ARROW);
               rcComponent.left   = rcScrollbarFrame.right - sizeComponent.cx;
               rcComponent.top    = rcScrollbarFrame.top + 1;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top + sizeComponent.cy;
               break;

            case SBC_LEFT_RANGE:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_LEFT_ARROW);
               rcScrollbarFrame.left  += sizeComponent.cx;
               rcScrollbarFrame.right -= sizeComponent.cx;
               // Get thumb's position and size.
               rcThumb = GetScrollComponentRect(SB_HORZ,SBC_THUMB);
               // Calculate "page-up range" rectangle
               SetRect(&rcComponent,
                  rcScrollbarFrame.left,rcScrollbarFrame.top,
                  rcThumb.left - 1,rcThumb.bottom);
               break;

            case SBC_RIGHT_RANGE:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_RIGHT_ARROW);
               rcScrollbarFrame.left  += sizeComponent.cx;
               rcScrollbarFrame.right -= sizeComponent.cx;
               // Get thumb's position and size.
               rcThumb = GetScrollComponentRect(SB_HORZ,SBC_THUMB);
               // Calculate "page-up range" rectangle
               SetRect(&rcComponent,
                  rcThumb.right + 1,rcThumb.top,
                  rcScrollbarFrame.right,rcScrollbarFrame.bottom);
               break;

            case SBC_TOTAL_RANGE:
               // Reduce the scrollbar frame by arrow and scrollbar frame sizes.
               sizeComponent = GetScrollComponentSize(SBC_RIGHT_ARROW);
               rcScrollbarFrame.left   += (sizeComponent.cx+1);
               rcScrollbarFrame.top    += 1;
               rcScrollbarFrame.right  -= (sizeComponent.cx+1);
               rcScrollbarFrame.bottom -= 1;
               // Copy frame rect. into component rect.
               CopyRect(&rcComponent,&rcScrollbarFrame);
               break;
         }
         break;

      case SB_VERT:
         switch (wComponent)
         {
            case SBC_THUMB:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_UP_ARROW);
               rcScrollbarFrame.top    += (sizeComponent.cy+1);
               rcScrollbarFrame.bottom -= (sizeComponent.cy+1);

               // Get thumb size.
               sizeComponent = GetScrollComponentSize(SBC_THUMB);

               // Reduce bottom of scrollbar by the height of the thumb.
               rcScrollbarFrame.bottom -= (sizeComponent.cy-1);

               // Get current thumb position.
               nPos = GetScrollPos(GetHandle(),SB_VERT);
               GetScrollRange(GetHandle(),SB_VERT,&iMin,&iMax);

               // Calculate thumb offset (in pixels) from top of scrollbar.
               if ((iMax-iMin)!=0)
                  nOffset = (int)((rcScrollbarFrame.bottom - rcScrollbarFrame.top) * ((nPos-iMin) / (float)(iMax-iMin)));
               else
                  nOffset = 0;

               // Calculate (x,y) coordinate to draw thumb in.
               sizeComponent      = GetScrollComponentSize(SBC_THUMB);
               rcComponent.left   = rcScrollbarFrame.left + 1;
               rcComponent.top    = rcScrollbarFrame.top  + nOffset;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top  + sizeComponent.cy;
               break;

            case SBC_UP_ARROW:
               sizeComponent      = GetScrollComponentSize(SBC_UP_ARROW);
               rcComponent.left   = rcScrollbarFrame.left + 1;
               rcComponent.top    = rcScrollbarFrame.top + 1;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top + sizeComponent.cy;
               break;

            case SBC_DOWN_ARROW:
               sizeComponent      = GetScrollComponentSize(SBC_DOWN_ARROW);
               rcComponent.left   = rcScrollbarFrame.left + 1;
               rcComponent.top    = rcScrollbarFrame.bottom - sizeComponent.cy;
               rcComponent.right  = rcComponent.left + sizeComponent.cx;
               rcComponent.bottom = rcComponent.top + sizeComponent.cy;
               break;

            case SBC_UP_RANGE:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_UP_ARROW);
               rcScrollbarFrame.top    += sizeComponent.cy;
               rcScrollbarFrame.bottom -= sizeComponent.cy;
               // Get thumb's position and size.
               rcThumb = GetScrollComponentRect(SB_VERT,SBC_THUMB);
               // Calculate "page-up range" rectangle
               SetRect(&rcComponent,
                  rcScrollbarFrame.left,rcScrollbarFrame.top,
                  rcThumb.right,rcThumb.bottom - 1);
               break;

            case SBC_DOWN_RANGE:
               // Reduce the scrollbar frame by arrow sizes.
               sizeComponent = GetScrollComponentSize(SBC_DOWN_ARROW);
               rcScrollbarFrame.top    += sizeComponent.cy;
               rcScrollbarFrame.bottom -= sizeComponent.cy;
               // Get thumb's position and size.
               rcThumb = GetScrollComponentRect(SB_VERT,SBC_THUMB);
               // Calculate "page-up range" rectangle
               SetRect(&rcComponent,
                  rcThumb.left,rcThumb.bottom + 1,
                  rcScrollbarFrame.right,rcScrollbarFrame.bottom);
               break;

            case SBC_TOTAL_RANGE:
               // Reduce the scrollbar frame by arrow and scrollbar frame sizes.
               sizeComponent = GetScrollComponentSize(SBC_DOWN_ARROW);
               rcScrollbarFrame.left   += 1;
               rcScrollbarFrame.top    += (sizeComponent.cy+1);
               rcScrollbarFrame.right  -= 1;
               rcScrollbarFrame.bottom -= (sizeComponent.cy+1);
               // Copy frame rect. into component rect.
               CopyRect(&rcComponent,&rcScrollbarFrame);
               break;
         }
         break;
   }

   // Return scrollbar control's component rectangle.
   return (rcComponent);
}


//---------------------------------------------------------------------
// ScrollWindowVert():
//    This function scrolls the client area of a window in the vertical
// direction.  The parameter "dy" is a positive or negative offset
// which is applied to the current client area origin.  If "dy" is
// negative, the client area will move upward.  If "dy" is
// positive, the client area will move downward.
//---------------------------------------------------------------------
void WPWINDOW::ScrollWindowVert(int dy)
{
   RECT  rcVisibleArea;
   int   nPos;

   // Check if window has "scrollable" property.
   if ((isScrollable()==FALSE)||(isScrollbarEnabled(SB_VERT)==FALSE))
      return;     // Window is not scrollable, return.

   // Get current visible area.
   rcVisibleArea = GetVisibleArea();

   // Move visible area in the desired vertical direction.
   OffsetRect(&rcVisibleArea,0,dy);

   // Store the new visible area.
   SetVisibleArea(&rcVisibleArea);

   // Position scrollbar accordingly.
   nPos = GetScrollPos(GetHandle(),SB_VERT);
   nPos += dy;
   SetScrollPos(GetHandle(),SB_VERT,nPos,TRUE);

   // Scroll window client area using Windows 3.1 API functions.
   // This way we do not have to repaint more than necessary.
   ScrollWindowEx(GetHandle(),0,-dy,NULL,NULL,NULL,NULL,(SW_INVALIDATE|SW_ERASE));
   UpdateWindow(GetHandle());
}

//---------------------------------------------------------------------
// ScrollWindowHoriz():
//    This function scrolls the client area of a window in the horizontal
// direction.  The parameter "dx" is a positive or negative offset
// which is applied to the current client area origin.  If "dx" is
// negative, the client area will move left.  If "dy" is
// positive, the client area will move right.
//---------------------------------------------------------------------
void WPWINDOW::ScrollWindowHoriz(int dx)
{
   RECT  rcVisibleArea;
   int   nPos;

   // Check if window has "scrollable" property.
   if ((isScrollable()==FALSE)||(isScrollbarEnabled(SB_HORZ)==FALSE))
      return;     // window is not scrollable, return.

   // Get current visible area.
   rcVisibleArea = GetVisibleArea();

   // Move visible area in the desired vertical direction.
   OffsetRect(&rcVisibleArea,dx,0);

   // Store the new visible area.
   SetVisibleArea(&rcVisibleArea);

   // Position scrollbar accordingly.
   nPos = GetScrollPos(GetHandle(),SB_HORZ);
   nPos += dx;
   SetScrollPos(GetHandle(),SB_HORZ,nPos,TRUE);

   // Scroll window client area using Windows 3.1 API functions.
   // This way we do not have to repaint more than necessary.
   ScrollWindowEx(GetHandle(),-dx,0,NULL,NULL,NULL,NULL,(SW_INVALIDATE|SW_ERASE));
   UpdateWindow(GetHandle());
}

//---------------------------------------------------------------------
// WINDOW COLORS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetFrameFillColor():
//    This function returns the color reference (in RGB format) of the
// color used to fill the window frame.
//---------------------------------------------------------------------
COLORREF WPWINDOW::GetFrameFillColor(BOOL fActive)
{
   // Check if window is active or inactive.
   if (fActive==TRUE)
      return (GetSysColor(COLOR_ACTIVEBORDER));
   else
      return (GetSysColor(COLOR_INACTIVEBORDER));
}


//---------------------------------------------------------------------
// GetCaptionFillColor():
//    This function returns the color reference (in RGB format) of the
// color used to fill the window caption bar.
//---------------------------------------------------------------------
COLORREF WPWINDOW::GetCaptionFillColor(BOOL fActive)
{
   // Check if window is active or inactive.
   if (fActive==TRUE)
      return (GetSysColor(COLOR_ACTIVECAPTION));
   else
      return (GetSysColor(COLOR_INACTIVECAPTION));
}


//---------------------------------------------------------------------
// GetCaptionTextColor():
//    This function returns the color reference (in RGB format) of the
// color used to draw the window caption text.
//---------------------------------------------------------------------
COLORREF WPWINDOW::GetCaptionTextColor(BOOL fActive)
{
   // Check if window is active or inactive.
   if (fActive==TRUE)
      return (GetSysColor(COLOR_CAPTIONTEXT));
   else
      return (GetSysColor(COLOR_INACTIVECAPTIONTEXT));
}



//---------------------------------------------------------------------
// DELAY FUNCTION USED IN WINDOW ANIMATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Wait():
//    This function delays the calling process by "nDelay" clock ticks.
// A "clock tick" is the smallest unit of measure returned by clock().
// A "clock tick" is approximatedly 1/CLOCKS_PER_SEC seconds.
// NOTE: Use this function for small (< 1 sec) delays only, since it
// blocks the whole system.
//---------------------------------------------------------------------
void WPWINDOW::Wait(int nDelay)
{
   clock_t  end;

   end = clock() + nDelay;
   while (clock()<end);
}


//---------------------------------------------------------------------
// DRAW BITMAP PRIMITIVE FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawBitmap():
//---------------------------------------------------------------------
void WPWINDOW::DrawBitmap(HBITMAP hbm,HDC hdc,int x,int y)
{
   HDC      hdcMemory;
   HBITMAP  hbmOld;
   BITMAP   bm;

   // Get bitmap object information (we are interested in the width and height).
   GetObject(hbm,sizeof(BITMAP),&bm);

   // Create memory device context.
   hdcMemory = CreateCompatibleDC(hdc);

   // Select bitmap into memory device context.
   hbmOld = (HBITMAP)SelectObject(hdcMemory,hbm);

   // Draw bitmap from memory into device context passed.
   BitBlt(hdc,x,y,bm.bmWidth,bm.bmHeight,hdcMemory,0,0,SRCCOPY);

   // Clean-up before leaving.
   SelectObject(hdcMemory,hbmOld);
   DeleteDC(hdcMemory);
}

//---------------------------------------------------------------------
// NON-CLIENT AREA PAINTING PRIMITIVES USED BY MEMBER FUNCTION NCPAINT()
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FillFrame():
//    This function draws 4 rectangles around the window frame using
// the current frame fill color.
//---------------------------------------------------------------------
void WPWINDOW::FillFrame(HDC hdc,int cx,int cy,BOOL fActive)
{
   HPEN     hpenOld;
   HBRUSH   hbrFill,hbrOld;

   // Create solid brush used to fill window frame.
   hbrFill = CreateSolidBrush(GetFrameFillColor(fActive));

   // Select created solid brush into device context (save original brush).
   hbrOld = (HBRUSH)SelectObject(hdc,hbrFill);

   // Select "no-pen" so that we have no border drawn around rectangles (save original pen).
   hpenOld = (HPEN)SelectObject(hdc,GetStockObject(NULL_PEN));

   // Increment width and height because there is no pen selected.
   // Therefore, the actual rectangle being drawn is the rectangle "inside"
   // the rectangle coordinates being passed to the Rectangle() API call.
   cx++;
   cy++;

   // Paint the 4 window frame rectangles.
   Rectangle(hdc,0,0,cx,GetSystemMetrics(SM_CYFRAME));      // top.
   Rectangle(hdc,0,0,GetSystemMetrics(SM_CXFRAME),cy);      // left.
   Rectangle(hdc,cx-GetSystemMetrics(SM_CXFRAME),0,cx,cy);  // right.
   Rectangle(hdc,0,cy-GetSystemMetrics(SM_CYFRAME),cx,cy);  // bottom.

   // Clean-up before leaving.
   SelectObject(hdc,hpenOld);
   SelectObject(hdc,hbrOld);
   DeleteObject(hbrFill);
}


//---------------------------------------------------------------------
// FillCaptionBar():
//    This function draws a rectangle on the caption bar area using
// the current caption fill color.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPWINDOW::FillCaptionBar(HDC hdc,int cx,int cy,BOOL fActive)
{
   HPEN     hpenOld;
   HBRUSH   hbrOld,hbrFill;

   // Create solid brush used to fill caption bar area.
   hbrFill = CreateSolidBrush(GetCaptionFillColor(fActive));

   // Select the created brush into device context (save original brush).
   hbrOld = (HBRUSH)SelectObject(hdc,hbrFill);

   // Select "no-pen" so that we have no border drawn around rectangles (save original pen).
   hpenOld = (HPEN)SelectObject(hdc,GetStockObject(NULL_PEN));

   // Draw rectangle using the caption fill brush.
   Rectangle(hdc,
      (GetSystemMetrics(SM_CXSIZE)+GetSystemMetrics(SM_CXFRAME)+1),
      GetSystemMetrics(SM_CYFRAME),
      (cx-GetSystemMetrics(SM_CXFRAME)+1),
      (GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYSIZE)+1));

   // Clean-up before leaving.
   SelectObject(hdc,hpenOld);
   SelectObject(hdc,hbrOld);
   DeleteObject(hbrFill);
}


//---------------------------------------------------------------------
// Outline3DFrame():
//    This function outlines the window frame giving it the 3-D effect
// that OS/2 windows have.  It uses white and dark gray pen objects.
//    The white pen object we get from Windows 3.1 built-in stock
// objects.   The dark gray pen must be created since there is no
// Windows 3.1 built-in dark gray pen object available.
//---------------------------------------------------------------------
void WPWINDOW::Outline3DFrame(HDC hdc,int cx,int cy)
{
   int    fcx,fcy;
   HPEN   hpenDarkGray,hpenOld;
   POINT  ptLine[5];

   // Create dark gray pen.
   hpenDarkGray = CreatePen(PS_SOLID,1,RGB(128,128,128));   // dark gray.

   // Store the width and height of the window frame.
   fcx = GetSystemMetrics(SM_CXFRAME);
   fcy = GetSystemMetrics(SM_CYFRAME);

   // WHITE LINES:

   // Outside top-left coordinate points.
   ptLine[0].x = 0;
   ptLine[0].y = cy-1;
   ptLine[1].x = 0;
   ptLine[1].y = 0;
   ptLine[2].x = cx;
   ptLine[2].y = 0;
   hpenOld = (HPEN)SelectObject(hdc,GetStockObject(WHITE_PEN));
   Polyline(hdc,&(ptLine[0]),3);

   // Inside bottom-right coordinate points.
   ptLine[0].x = fcx-1;
   ptLine[0].y = cy-fcy;
   ptLine[1].x = cx-fcx;
   ptLine[1].y = ptLine[0].y;
   ptLine[2].x = ptLine[1].x;
   ptLine[2].y = fcy-1;
   Polyline(hdc,&(ptLine[0]),3);


   // DARK GRAY LINES:

   // Inside top-left coordinate points.
   ptLine[0].x = fcx-1;
   ptLine[0].y = cy-(fcy+1);
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = fcy-1;
   ptLine[2].x = cx-fcx+1;
   ptLine[2].y = ptLine[1].y;
   SelectObject(hdc,hpenDarkGray);
   Polyline(hdc,&(ptLine[0]),3);

   // Caption bar 3-d lines coordinate points.
   ptLine[0].x = fcx-1;
   ptLine[0].y = GetSystemMetrics(SM_CYCAPTION)+fcy-2;
   ptLine[1].x = cx-fcx-(GetSystemMetrics(SM_CXSIZE)*2)+2;
   ptLine[1].y = ptLine[0].y;
   Polyline(hdc,&(ptLine[0]),2);

   // Outside bottom-right coordinate points.
   ptLine[0].x = 0;
   ptLine[0].y = cy-1;
   ptLine[1].x = cx-1;
   ptLine[1].y = ptLine[0].y;
   ptLine[2].x = ptLine[1].x;
   ptLine[2].y = 0;
   Polyline(hdc,&(ptLine[0]),3);

   // Clean-up before leaving.
   SelectObject(hdc,hpenOld);
   ::DeleteObject(hpenDarkGray);
}


//---------------------------------------------------------------------
// DrawCaptionBarText():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPWINDOW::DrawCaptionBarText(HDC hdc,int cx,int cy,BOOL fActive)
{
   int   iOldBkMode;
   RECT  rcCaption;
   char  szTitleText[MAX_TITLE_NAME+1];

   // Calculate caption rectangle.
   SetRect(&rcCaption,
      (GetSystemMetrics(SM_CXSIZE)+GetSystemMetrics(SM_CXFRAME)+16),
      GetSystemMetrics(SM_CYFRAME),
      (cx-GetSystemMetrics(SM_CXFRAME)),
      (GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYSIZE)));

   // Reduce the caption rectangle by 1 so that the caption outline
   // is not included in it.
   InflateRect(&rcCaption,-1,-1);

   // Set caption text color.
   SetTextColor(hdc,GetCaptionTextColor(fActive));
   // Set background mode to transparent (no background painting).
   iOldBkMode = SetBkMode(hdc,TRANSPARENT);

   // Get window text.
   GetWindowText(GetHandle(),szTitleText,MAX_TITLE_NAME);

   // Draw caption text.
   DrawText(hdc,szTitleText,-1,&rcCaption,(DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX));

   // Restore background to original background mode.
   SetBkMode(hdc,iOldBkMode);
}


//---------------------------------------------------------------------
// DrawIconOnSystemMenu():
//---------------------------------------------------------------------
void WPWINDOW::DrawIconOnSystemMenu(HDC hdc,HICON hIcon)
{
   HDC     hdcMem;
   HBITMAP hbmMem,hbmOld;
   RECT    rect;
   HBRUSH  hbr;
   SIZE    sizeIcon;
   HICON   hicon3DEffect;
   int     iPrevStretchMode;

   // Query the system icon size.
   sizeIcon.cx = GetSystemMetrics(SM_CXICON);
   sizeIcon.cy = GetSystemMetrics(SM_CYICON);

   // Create memory device context and compatible bitmap
   // to stretch the icon into the system menu control.
   hdcMem = CreateCompatibleDC(hdc);
   hbmMem = CreateCompatibleBitmap(hdc,sizeIcon.cx,sizeIcon.cy);

   // Select bitmap object into memory device context (save original bitmap object).
   hbmOld = (HBITMAP)SelectObject(hdcMem,(HGDIOBJ)hbmMem);

   // Create a rectangle struct. with the size of a Windows 3.1 icon.
   SetRect(&rect,0,0,sizeIcon.cx,sizeIcon.cy);

   // Create color brush using the the system menu background color.
   hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

   // Clean memory bitmap using the created color brush.
   FillRect(hdcMem,&rect,hbr);

   // Draw icon into memory device context.
   DrawIcon(hdcMem,0,0,hIcon);

   // Draw 3-D "button" effect on icon.
   hicon3DEffect = LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_BUTTON_EFFECT));
   DrawIcon(hdcMem,0,0,hicon3DEffect);

   // Set the desired strecth blit mode of deleting the eliminated lines.
   iPrevStretchMode = SetStretchBltMode(hdc,STRETCH_DELETESCANS);

   // Draw system menu icon into actual window top-left corner.
   StretchBlt(hdc,GetSystemMetrics(SM_CXFRAME),
                  GetSystemMetrics(SM_CYFRAME),
                  GetSystemMetrics(SM_CXSIZE)+1,
                  GetSystemMetrics(SM_CYSIZE)+1,
                  hdcMem,0,0,sizeIcon.cx,sizeIcon.cy,SRCCOPY);

   // Restore the original stretch blit mode to device context.
   SetStretchBltMode(hdc,iPrevStretchMode);

   // Clean-up before leaving.
   DestroyIcon(hicon3DEffect);
   DeleteObject(hbr);
   SelectObject(hdcMem,hbmOld);
   DeleteObject(hbmMem);
   DeleteDC(hdcMem);
}



//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WINDOW ZOOM IN/OUT ANIMATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// AnimateWindowFrame():
//    This function animates the window frame by drawing an removing
// a series of rectangles, "nSteps", from "rcStart" to "rcEnd".
//    Each rectangle will be shown for at least "nDelay" milliseconds
// before it is removed and the next one is drawn.
//    "rcStart" and "rcEnd" can be any arbitrary rectangles which
// positions are mapped onto the screen device context.
//    If "rcStart" < "rcEnd", then the animation will "explode".
// Else, if "rcStart" > "rcEnd" then the animation will "inplode".
//---------------------------------------------------------------------
void WPWINDOW::AnimateWindowFrame(LPRECT lprcStart,LPRECT lprcEnd,int nDelay,int nSteps)
{
   HDC      hdcScreen;
   HPEN     hpenFrame,hpenOld;
   HBRUSH   hbrOld;
   SIZE     sizeStart,sizeEnd,sizeGrowInc,sizeMoveInc;
   RECT     rcFrame,rcNextFrame;
   int      rop2Old,index;

   // If window animation is disabled, return immediatedly.
   if (g_wpEnvironment.stSystem.fWindowAnimationEnabled==FALSE)
      return;

   // Calculate width and height of start and end rectangles.
   sizeStart.cx = (lprcStart->right  - lprcStart->left + 1);
   sizeStart.cy = (lprcStart->bottom - lprcStart->top  + 1);

   sizeEnd.cx = (lprcEnd->right  - lprcEnd->left + 1);
   sizeEnd.cy = (lprcEnd->bottom - lprcEnd->top  + 1);

   // Calculate grow index and displacement offsets.
   sizeGrowInc.cx = (sizeEnd.cx - sizeStart.cx) / nSteps;
   sizeGrowInc.cy = (sizeEnd.cy - sizeStart.cy) / nSteps;

   sizeMoveInc.cx = (lprcEnd->left - lprcStart->left) / nSteps;
   sizeMoveInc.cy = (lprcEnd->top  - lprcStart->top)  / nSteps;

   // Get handle to screen device context.
   hdcScreen = ::GetDC(NULL);

   // Create white pen with frame width.
   hpenFrame = CreatePen(PS_SOLID,GetSystemMetrics(SM_CXFRAME),RGB(0xFF,0xFF,0xFF));

   // Select the newly created pen into screen device context.
   hpenOld = (HPEN)SelectObject(hdcScreen,hpenFrame);

   // Select "no-brush" for rectangle filling into the screen device contex.
   hbrOld  = (HBRUSH)SelectObject(hdcScreen,GetStockObject(NULL_BRUSH));

   // Set raster operation between selected pen and screen colors to be XOR.
   rop2Old = SetROP2(hdcScreen,R2_XORPEN);

   // Start animation by using "rcFrame" rect. as current frame rect.
   CopyRect(&rcFrame,lprcStart);

   // Animate window frame zoom...
   for (index=0; index<nSteps; index++)
   {
      // Display frame on screen.
      Rectangle(hdcScreen,rcFrame.left,rcFrame.top,rcFrame.right,rcFrame.bottom);

      // Make a copy of the frame's current position.
      CopyRect(&rcNextFrame,&rcFrame);

      // Calculate next frame's position using the current position.
      OffsetRect(&rcNextFrame,sizeMoveInc.cx,sizeMoveInc.cy);

      // Calculate next frame's size using the current size.
      rcNextFrame.right  += sizeGrowInc.cx;
      rcNextFrame.bottom += sizeGrowInc.cy;

      // Wait a few milliseconds for complete animation effect.
      Wait(nDelay);

      // Remove current frame from screen.
      Rectangle(hdcScreen,rcFrame.left,rcFrame.top,rcFrame.right,rcFrame.bottom);

      // Make the current frame rect. hold the next frame's position and size.
      CopyRect(&rcFrame,&rcNextFrame);
   }

   // Clean-up before leaving.
   SetROP2(hdcScreen,rop2Old);
   SelectObject(hdcScreen,hbrOld);
   SelectObject(hdcScreen,hpenOld);
   ::ReleaseDC(NULL,hdcScreen);
   DeleteObject(hpenFrame);
}

//---------------------------------------------------------------------
// MIN/MAX BUTTONS DRAW FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawMinMaxButtonsHideStyle():
//    This function draws the minimized and maximized buttons on the
// top-right side of the window.  The parameters flags "fMinDown" and
// "fMaxDown" specify which button should be up and/or down.
//    If boolean flag value is TRUE, the specified button will be drawn
// in the "pushed-in" (or down) position.   If it is FALSE, it will be
// drawn in "pushed-out" (or "up") position.
//    The "fHideStyle" flag specifies the "min" button style to draw.
// If it is TRUE, the small square with dash lines is used, else
// a small solid square is used.
//---------------------------------------------------------------------
void WPWINDOW::DrawMinMaxButtonsHideStyle(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown)
{
   LPCSTR  pszComponent;
   BITMAP  bmButtons;
   HBITMAP hbmpButtons;
   int     x,y;

   // Depending on what screen resolution we are currently in...
   if (GetSystemMetrics(SM_CYSIZE)<26)
   {
      // 640x480 VGA resolution.
      // 800x600 SVGA (large fonts) resolution.
      // If window is not maximized, then...
      if (IsZoomed(GetHandle())==FALSE)
      {
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_MINMAX_UP);
      }
      else
      {
         // Window is maximized, therefore, we need a different set of bitmaps.
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_X_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_X_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_X_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_1_X_MINMAX_UP);
      }

      // The respective button bitmap resource name has been set.
   }
   else
   {
      // 800x600 SVGA (small fonts) resolution.
      // 1024x768 SVGA (or XGA) resolution.
      // 1280x1024 SVGA (or XGA/2) resolution.

      // If window is not maximized, then...
      if (IsZoomed(GetHandle())==FALSE)
      {
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_MINMAX_UP);
      }
      else
      {
         // Window is maximized, therefore, we need a different set of bitmaps.
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_X_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_X_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_X_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_1_X_MINMAX_UP);
      }

      // The respective button bitmap resource name has been set.
   }

   // Load the respective bitmap resource from program instance.
   hbmpButtons = LoadBitmap(g_wpEnvironment.hInst,pszComponent);

   // Calculate location of buttons bitmap.
   GetObject(hbmpButtons,sizeof(BITMAP),&bmButtons);
   x  = (cx-bmButtons.bmWidth-GetSystemMetrics(SM_CXFRAME));
   y  = GetSystemMetrics(SM_CYFRAME);

   // Draw buttons bitmap at calculated location.
   DrawBitmap(hbmpButtons,hdc,x,y);

   // Clean-up before leaving.
   DeleteObject(hbmpButtons);
}


//---------------------------------------------------------------------
// DrawMinMaxButtonsMinStyle():
//    This function draws the minimized and maximized buttons on the
// top-right side of the window.  The parameters flags "fMinDown" and
// "fMaxDown" specify which button should be up and/or down.
//    If boolean flag value is TRUE, the specified button will be drawn
// in the "pushed-in" (or down) position.   If it is FALSE, it will be
// drawn in "pushed-out" (or "up") position.
//    The "fHideStyle" flag specifies the "min" button style to draw.
// If it is TRUE, the small square with dash lines is used, else
// a small solid square is used.
//---------------------------------------------------------------------
void WPWINDOW::DrawMinMaxButtonsMinStyle(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown)
{
   LPCSTR  pszComponent;
   BITMAP  bmButtons;
   HBITMAP hbmpButtons;
   int     x,y;

   // Depending on what screen resolution we are currently in...
   if (GetSystemMetrics(SM_CYSIZE)<26)
   {
      // 640x480 VGA resolution.
      // 800x600 SVGA (large fonts) resolution.
      // If window is not maximized, then...
      if (IsZoomed(GetHandle())==FALSE)
      {
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_MINMAX_UP);
      }
      else
      {
         // Window is maximized, therefore, we need a different set of bitmaps.
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_X_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_X_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_X_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_LORES_2_X_MINMAX_UP);
      }

      // The respective button bitmap resource name has been set.
   }
   else
   {
      // 800x600 SVGA (small fonts) resolution.
      // 1024x768 SVGA (or XGA) resolution.
      // 1280x1024 SVGA (or XGA/2) resolution.

      // If window is not maximized, then...
      if (IsZoomed(GetHandle())==FALSE)
      {
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_MINMAX_UP);
      }
      else
      {
         // Window is maximized, therefore, we need a different set of bitmaps.
         // Load respective buttons bitmap.
         if ((fMinDown==FALSE)&&(fMaxDown==FALSE))
            // Both buttons up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_X_MINMAX_UP);
         else
         if ((fMinDown==TRUE)&&(fMaxDown==FALSE))
            // Min button down, max button up.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_X_MIN_DOWN);
         else
         if ((fMinDown==FALSE)&&(fMaxDown==TRUE))
            // Min button up, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_X_MAX_DOWN);
         else
            // Min button down, max button down.
            pszComponent = MAKEINTRESOURCE(IDB_HIRES_2_X_MINMAX_UP);
      }

      // The respective button bitmap resource name has been set.
   }

   // Load the respective bitmap resource from program instance.
   hbmpButtons = LoadBitmap(g_wpEnvironment.hInst,pszComponent);

   // Calculate location of buttons bitmap.
   GetObject(hbmpButtons,sizeof(BITMAP),&bmButtons);
   x  = (cx-bmButtons.bmWidth-GetSystemMetrics(SM_CXFRAME));
   y  = GetSystemMetrics(SM_CYFRAME);

   // Draw buttons bitmap at calculated location.
   DrawBitmap(hbmpButtons,hdc,x,y);

   // Clean-up before leaving.
   DeleteObject(hbmpButtons);
}


//---------------------------------------------------------------------
// DrawMinMaxButtons():
//    This function draws the minimized and maximized buttons on the
// top-right side of the window.  The parameters flags "fMinDown" and
// "fMaxDown" specify which button should be up and/or down.
//    If boolean flag value is TRUE, the specified button will be drawn
// in the "pushed-in" (or down) position.   If it is FALSE, it will be
// drawn in "pushed-out" (or "up") position.
//    The "fHideStyle" flag specifies the "min" button style to draw.
// If it is TRUE, the small square with dash lines is used, else
// a small solid square is used.
//---------------------------------------------------------------------
void WPWINDOW::DrawMinMaxButtons(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown)
{
   // By default, we call the "hide" style of Min/Max buttons.
   DrawMinMaxButtonsHideStyle(hdc,cx,fMinDown,fMaxDown);
}


//---------------------------------------------------------------------
// SCROLLBARS WINDOW CONTROLS DRAWING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawScrollComponent():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollComponent(HDC hdc,WORD wSBCode,WORD wComponent,WORD wState)
{
   RECT     rcComponent;
   HBITMAP  hbmpComponent;

   // If component is "none", exit.
   if (wComponent==SBC_NONE)
      return;

   // If want to draw a range component...
   if ((wComponent>=SBC_LEFT_RANGE)&&(wComponent<=SBC_DOWN_RANGE))
      return;  // exit.

   // Get component's position.
   rcComponent = GetScrollComponentRect(wSBCode,wComponent);

   // Load component's bitmap.
   hbmpComponent = LoadScrollComponentBitmap(wComponent,wState);

   // Draw component's bitmap.
   DrawBitmap(hbmpComponent,hdc,rcComponent.left,rcComponent.top);

   // Delete loaded bitmap resource.
   DeleteObject(hbmpComponent);
}




//---------------------------------------------------------------------
// DrawScrollArrows():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollArrows(HDC hdc,WORD wSBCode)
{
   // Depending on which scrollbar to draw arrows on...
   switch (wSBCode)
   {
      case SB_HORZ:
         DrawScrollComponent(hdc,wSBCode,SBC_LEFT_ARROW,GetScrollComponentState(SBC_LEFT_ARROW));
         DrawScrollComponent(hdc,wSBCode,SBC_RIGHT_ARROW,GetScrollComponentState(SBC_RIGHT_ARROW));
         break;

      case SB_VERT:
         DrawScrollComponent(hdc,wSBCode,SBC_UP_ARROW,GetScrollComponentState(SBC_UP_ARROW));
         DrawScrollComponent(hdc,wSBCode,SBC_DOWN_ARROW,GetScrollComponentState(SBC_DOWN_ARROW));
         break;
   }
}

//---------------------------------------------------------------------
// DrawScrollArea():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollArea(HDC hdc,WORD wSBCode)
{
   RECT     rcArea;
   HBRUSH   hbrScrollArea;

   // Get the complete scrollbar scroll area.
   rcArea = GetScrollComponentRect(wSBCode,SBC_TOTAL_RANGE);

   // Create brush used to paint scrollbar's background.
   hbrScrollArea = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR));

   // Increment the right and bottom borders since "FillRect()" does not fill them.
   rcArea.right++;
   rcArea.bottom++;

   // Fill-in the scrollbar's background color using selected brush.
   FillRect(hdc,&rcArea,hbrScrollArea);

   // Clean-up before leaving.
   DeleteObject(hbrScrollArea);
}


//---------------------------------------------------------------------
// DrawScrollThumb():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollThumb(HDC hdc,WORD wSBCode)
{
   // Draw scrollbar's thumb in current state.
   DrawScrollComponent(hdc,wSBCode,SBC_THUMB,GetScrollComponentState(SBC_THUMB));
}



//---------------------------------------------------------------------
// DrawScroll3DOutline():
//---------------------------------------------------------------------
void WPWINDOW::DrawScroll3DOutline(HDC hdc,WORD wSBCode)
{
   HPEN     hpenDarkGray,hpenOld;
   RECT     rcScrollbarFrame;
   POINT    ptLine[3];

   // Get scrollbar's frame rectangle.
   rcScrollbarFrame = GetScrollbarFrameRect(wSBCode);

   // Create dark gray color pen used to draw scrollbar's frame.
   hpenDarkGray  = CreatePen(PS_SOLID,1,RGB(128,128,128));

   // Select the dark gray pen as current pen.
   hpenOld = (HPEN)SelectObject(hdc,hpenDarkGray);

   // Depending on which scrollbar control to outline, draw frame outline
   // around scrollbar area using the selected pen...
   switch (wSBCode)
   {
      case SB_HORZ:
         // First, draw the dark gray lines outlining the right and top side of frame.
         ptLine[0].x = rcScrollbarFrame.left;
         ptLine[0].y = rcScrollbarFrame.bottom-1;
         ptLine[1].x = ptLine[0].x;
         ptLine[1].y = rcScrollbarFrame.top;
         ptLine[2].x = rcScrollbarFrame.right+1;
         ptLine[2].y = ptLine[1].y;
         Polyline(hdc,&(ptLine[0]),3);

         // Select the white pen as current pen.
         SelectObject(hdc,GetStockObject(WHITE_PEN));

         // Last, draw the white lines outlining the bottom and right side of frame.
         ptLine[0].x = rcScrollbarFrame.left;
         ptLine[0].y = rcScrollbarFrame.bottom;
         ptLine[1].x = rcScrollbarFrame.right;
         ptLine[1].y = ptLine[0].y;
         ptLine[2].x = ptLine[1].x;
         ptLine[2].y = rcScrollbarFrame.top;
         Polyline(hdc,&(ptLine[0]),3);
         break;

      case SB_VERT:
         // First, draw the dark gray lines outlining the right and top side of frame...
         ptLine[0].x = rcScrollbarFrame.left;
         ptLine[0].y = rcScrollbarFrame.bottom-1;
         ptLine[1].x = ptLine[0].x;
         ptLine[1].y = rcScrollbarFrame.top;
         ptLine[2].x = rcScrollbarFrame.right+1;
         ptLine[2].y = ptLine[1].y;
         Polyline(hdc,&(ptLine[0]),3);

         // Select the white pen as current pen.
         SelectObject(hdc,GetStockObject(WHITE_PEN));

         // Last, draw the white lines outlining the bottom and right side of frame.
         ptLine[0].x = rcScrollbarFrame.left;
         ptLine[0].y = rcScrollbarFrame.bottom;
         ptLine[1].x = rcScrollbarFrame.right;
         ptLine[1].y = ptLine[0].y;
         ptLine[2].x = ptLine[1].x;
         ptLine[2].y = rcScrollbarFrame.top;
         Polyline(hdc,&(ptLine[0]),3);
         break;

   }

   // Clean-up before leaving.
   SelectObject(hdc,hpenOld);
   DeleteObject(hpenDarkGray);
}

//---------------------------------------------------------------------
// DrawScrollIntersectionRect():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollIntersectionRect(HDC hdc)
{
   RECT     rcIntersect,rcHScroll,rcVScroll;
   HBRUSH   hbrScrollArea;

   // Check if we have both scrollbars enabled...
   if (isScrollbarEnabled(SB_BOTH)==TRUE)
   {
      // Get horizontal and vertical scrollbar's frame.
      rcHScroll = GetScrollbarFrameRect(SB_HORZ);
      rcVScroll = GetScrollbarFrameRect(SB_VERT);

      // Calculate intersection rectangle on bottom-right corner of window.
      // Add 1 more to the right and bottom points since "FillRect()" does not fill them.
      SetRect(&rcIntersect,rcHScroll.right+1,rcVScroll.bottom+1,
                           rcVScroll.right+1,rcHScroll.bottom+1);

      // Create brush used to paint scrollbar's background.
      hbrScrollArea = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR));

      // Fill rectangle with scrollbar background color.
      FillRect(hdc,&rcIntersect,hbrScrollArea);

      // Clean-up before leaving.
      DeleteObject(hbrScrollArea);
   }
}


//---------------------------------------------------------------------
// DrawScrollbar():
//---------------------------------------------------------------------
void WPWINDOW::DrawScrollbar(HDC hdc,WORD wSBCode)
{
   // Draw arrows.
   DrawScrollArrows(hdc,wSBCode);

   // Draw scroll area background.
   DrawScrollArea(hdc,wSBCode);

   // Draw scrollbar's 3D outline.
   DrawScroll3DOutline(hdc,wSBCode);

   // Draw scrollbar's thumb component.
   DrawScrollThumb(hdc,wSBCode);

   // Draw scrollbar intersection component (vertical only).
   if (isScrollbarEnabled(SB_VERT)==TRUE)
      DrawScrollIntersectionRect(hdc);
}



//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPWINDOW():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPWINDOW::WPWINDOW(void)
{
   // Clear everything to zeros.
   memset(this,0,sizeof(WPWINDOW));

   // Initialize default values.
   m_hwnd        = NULL;
   m_hdc         = NULL;
   m_fScrollable = TRUE;
   m_wScrollbars = SB_NONE;
   m_ptWindow.x  = CW_USEDEFAULT;
   m_ptWindow.y  = CW_USEDEFAULT;
   m_sizeWindow.cx = ((GetSystemMetrics(SM_CXSCREEN)/4)*3); // 75% screen size.
   m_sizeWindow.cy = (GetSystemMetrics(SM_CYSCREEN)/2);     // 50% screen size.
}

//---------------------------------------------------------------------
// WPWINDOW():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPWINDOW::WPWINDOW(WPWINDOW *pwpwindowDefaults)
{
   // Clear everything to zeros.
   memset(this,0,sizeof(WPWINDOW));

   // Initialize default values.
   m_hwnd        = NULL;
   m_hdc         = NULL;
   m_fScrollable = TRUE;
   m_wScrollbars = SB_NONE;
   m_ptWindow    = pwpwindowDefaults->GetWndPos();
   m_sizeWindow  = pwpwindowDefaults->GetWndSize();
   m_rcLogical   = pwpwindowDefaults->GetLogicalDomain();
   m_rcVisible   = pwpwindowDefaults->GetVisibleArea();
}

//---------------------------------------------------------------------
// ~WPWINDOW():
//    Destructor for class.
//---------------------------------------------------------------------
WPWINDOW::~WPWINDOW(void)
{
   // do nothing.
}

//---------------------------------------------------------------------
// WINDOW PROPERTIES MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetHandle():
//    Returns the window handle for this object.
//---------------------------------------------------------------------
HWND WPWINDOW::GetHandle(void)
{
   return (m_hwnd);
}


//---------------------------------------------------------------------
// SetHandle():
//    Sets the window handle to handle passed as parameter.
//---------------------------------------------------------------------
void WPWINDOW::SetHandle(HWND hwnd)
{
   m_hwnd = hwnd;
}



//---------------------------------------------------------------------
// WINDOW POSITION AND AREA MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetWndPos():
//    Sets the new window top-left coordinate (x,y) equal to the point
// parameter.  The "ptWindow" parameter contains an (x,y) pair in
// screen (device) coordinates.
//---------------------------------------------------------------------
void WPWINDOW::SetWndPos(POINT ptWindow)
{
   m_ptWindow = ptWindow;
}

//---------------------------------------------------------------------
// SetWndPos():
//    Sets the new window top-left coordinate (x,y) equal to the
// values passed.  The two integer parameters represent an (x,y) pair
// in screen (device) coordinates.
//---------------------------------------------------------------------
void WPWINDOW::SetWndPos(int x,int y)
{
   m_ptWindow.x = x;
   m_ptWindow.y = y;
}

//---------------------------------------------------------------------
// GetWndPos():
//---------------------------------------------------------------------
POINT WPWINDOW::GetWndPos(void)
{
   return (m_ptWindow);
}

//---------------------------------------------------------------------
// SetWndSize():
//---------------------------------------------------------------------
void WPWINDOW::SetWndSize(SIZE sizeWindow)
{
   m_sizeWindow = sizeWindow;
}

//---------------------------------------------------------------------
// SetWndSize():
//---------------------------------------------------------------------
void WPWINDOW::SetWndSize(int cx,int cy)
{
   m_sizeWindow.cx = cx;
   m_sizeWindow.cy = cy;
}


//---------------------------------------------------------------------
// GetWndSize():
//---------------------------------------------------------------------
SIZE WPWINDOW::GetWndSize(void)
{
   return (m_sizeWindow);
}

//---------------------------------------------------------------------
// SetLogicalDomain():
//---------------------------------------------------------------------
void WPWINDOW::SetLogicalDomain(LPRECT lprcLogical)
{
   // Store new logical domain value.
   m_rcLogical = *lprcLogical;
}

//---------------------------------------------------------------------
// GetLogicalDomain():
//---------------------------------------------------------------------
RECT WPWINDOW::GetLogicalDomain(void)
{
   return (m_rcLogical);
}

//---------------------------------------------------------------------
// SetVisibleArea():
//---------------------------------------------------------------------
void WPWINDOW::SetVisibleArea(LPRECT lprcVisible)
{     
   // Store new visible area.
   m_rcVisible = *lprcVisible;
}

//---------------------------------------------------------------------
// GetVisibleArea():
//---------------------------------------------------------------------
RECT WPWINDOW::GetVisibleArea(void)
{
   return (m_rcVisible);
}


//---------------------------------------------------------------------
// isPointVisible():
//---------------------------------------------------------------------
BOOL WPWINDOW::isPointVisible(POINT ptLogical)
{
   // Check if window is iconized.
   if (IsIconic(GetHandle())==TRUE)
      return (FALSE);               // no visible points.

   // If point is in visible area.
   return (PtInRect(&(m_rcVisible),ptLogical));
}



//---------------------------------------------------------------------
// SCROLLBAR PROPERTIES MANAGEMENT FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isScrollbarCapturingInput():
//---------------------------------------------------------------------
BOOL WPWINDOW::isScrollbarCapturingInput(void)
{
   // If window is scrollable...
   if (isScrollable()==TRUE)
      // Return flag state.
      return (m_fScrollbarHasInputCaptured);
   else
      // Return failure.
      return (FALSE);
}


//---------------------------------------------------------------------
// SetScrollbarInputCaptureFlag():
//---------------------------------------------------------------------
void WPWINDOW::SetScrollbarInputCaptureFlag(BOOL fCapture)
{
   m_fScrollbarHasInputCaptured = fCapture;
}


//---------------------------------------------------------------------
// isScrollbarEnabled():
//---------------------------------------------------------------------
BOOL WPWINDOW::isScrollbarEnabled(WORD wSBCode)
{
   // Depending on scrollbar...
   switch (wSBCode)
   {
      case SB_VERT:
      case SB_HORZ:
         // If both scrollbars are currently enabled...
         if (m_wScrollbars==SB_BOTH)
            return (TRUE);          // Return TRUE.
         else
            // Return TRUE only if "wSBCode" scrollbar is enabled.
            return ((wSBCode==m_wScrollbars)? TRUE : FALSE);

      case SB_BOTH:
         // Return TRUE only if both scrollbars are enabled.
         return ((wSBCode==m_wScrollbars)? TRUE : FALSE);

      default:
         return (FALSE);
   }
}

//---------------------------------------------------------------------
// SetScrollPos():
//---------------------------------------------------------------------
int WPWINDOW::SetScrollPos(HWND hwnd,int nSBCode,int nPos,BOOL fRepaint)
{
   HDC   hdc;
   int   nPrevPos;

   // Do a call to the real Windows 3.1 scrollbar API function "SetScrollPos()"
   // but make sure we do not repaint the scrollbar.
   nPrevPos = ::SetScrollPos(hwnd,nSBCode,nPos,FALSE);

   // Now, check if scrollbar needs repainting.
   if ((fRepaint==TRUE)&&(nSBCode!=SB_BOTH))
   {
      // Get window device context.
      hdc = GetWindowDC(hwnd);

      // Draw scrollbar.
      DrawScrollbar(hdc,(WORD)nSBCode);

      // Return window device context.
      ::ReleaseDC(hwnd,hdc);
   }

   // Return previous scrollbar position.
   return (nPrevPos);
}


//---------------------------------------------------------------------
// ShowScrollbar():
//---------------------------------------------------------------------
void WPWINDOW::ShowScrollbar(WORD wSBCode,BOOL fEnable)
{
   switch (wSBCode)
   {
      case SB_VERT:
         switch (m_wScrollbars)
         {
            case SB_NONE:
               if (fEnable==TRUE)
               {
                  m_wScrollbars = SB_VERT;
                  ::ShowScrollBar(GetHandle(),SB_VERT,TRUE);
               }
               break;

            case SB_VERT:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_NONE;
                  ::ShowScrollBar(GetHandle(),SB_VERT,FALSE);
               }
               break;

            case SB_HORZ:
               if (fEnable==TRUE)
               {
                  m_wScrollbars = SB_BOTH;
                  ::ShowScrollBar(GetHandle(),SB_VERT,TRUE);
               }
               break;

            case SB_BOTH:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_HORZ;
                  ::ShowScrollBar(GetHandle(),SB_VERT,FALSE);
               }
               break;
         }
         break;

      case SB_HORZ:
         switch (m_wScrollbars)
         {
            case SB_NONE:
               if (fEnable==TRUE)
               {
                  m_wScrollbars = SB_HORZ;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,TRUE);
               }
               break;

            case SB_VERT:
               if (fEnable==TRUE)
               {
                  m_wScrollbars = SB_BOTH;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,TRUE);
               }
               break;

            case SB_HORZ:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_NONE;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,FALSE);
               }
               break;

            case SB_BOTH:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_VERT;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,FALSE);
               }
               break;
         }
         break;

      case SB_BOTH:
         switch (m_wScrollbars)
         {
            case SB_NONE:
               if (fEnable==TRUE)
               {
                  m_wScrollbars = SB_BOTH;
                  ::ShowScrollBar(GetHandle(),SB_BOTH,TRUE);
               }
               break;

            case SB_VERT:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_NONE;
                  ::ShowScrollBar(GetHandle(),SB_VERT,FALSE);
               }
               else
               {
                  m_wScrollbars = SB_BOTH;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,TRUE);
               }
               break;

            case SB_HORZ:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_NONE;
                  ::ShowScrollBar(GetHandle(),SB_HORZ,FALSE);
               }
               else
               {
                  m_wScrollbars = SB_BOTH;
                  ::ShowScrollBar(GetHandle(),SB_VERT,TRUE);
               }
               break;

            case SB_BOTH:
               if (fEnable==FALSE)
               {
                  m_wScrollbars = SB_NONE;
                  ::ShowScrollBar(GetHandle(),SB_BOTH,FALSE);
               }
               break;
         }
         break;
   }
}

//---------------------------------------------------------------------
// ArrangeScrollbars():
//---------------------------------------------------------------------
void WPWINDOW::ArrangeScrollbars(void)
{
   RECT rcVisible,rcLogicalDomain;

   // If window has scrollable property, setup scrollbar ranges accordingly.
   if (isScrollable()==TRUE)
   {
      // Get current scrollbar
      rcVisible         = GetVisibleArea();
      rcLogicalDomain   = GetLogicalDomain();

      // Adjust scrollbars as need be (remove if not needed).
      AdjustScrollbars(rcVisible,rcLogicalDomain);
      // Set horizontal range and position for horizontal scrollbar.
      if (isScrollbarEnabled(SB_HORZ)==TRUE)
      {
         SetScrollRange(GetHandle(),SB_HORZ,rcLogicalDomain.left,rcLogicalDomain.right,FALSE);
         SetScrollPos(GetHandle(),SB_HORZ,rcVisible.left,TRUE);
      }
      // Set vertical range and position for vertical scrollbar.
      if (isScrollbarEnabled(SB_VERT)==TRUE)
      {
         SetScrollRange(GetHandle(),SB_VERT,rcLogicalDomain.top,rcLogicalDomain.bottom,FALSE);
         SetScrollPos(GetHandle(),SB_VERT,rcVisible.top,TRUE);
      }
   }
}

//---------------------------------------------------------------------
// AdjustScrollbars():
//---------------------------------------------------------------------
void WPWINDOW::AdjustScrollbars(RECT rcVisible,RECT rcLogical)
{
   // Check first if all of the logical domain is contained within the visible area.
   if (((rcVisible.left<=rcLogical.left)&&(rcVisible.right>=rcLogical.right))&&
       ((rcVisible.top<=rcLogical.top)&&(rcVisible.bottom>=rcLogical.bottom)))
      ShowScrollbar(SB_BOTH,FALSE); // Remove both scrollbars.
   else
   // Check first if the logical domain is not contained in any direction of the visible area.
   if (((rcVisible.left>rcLogical.left)||(rcVisible.right<rcLogical.right))&&
       ((rcVisible.top>rcLogical.top)||(rcVisible.bottom<rcLogical.bottom)))
      ShowScrollbar(SB_BOTH,TRUE);  // Show both scrollbars.
   else
   {
      // Now, we check if we need the horizontal scrollbar...
      if ((rcVisible.left<=rcLogical.left)&&(rcVisible.right>=rcLogical.right))
         ShowScrollbar(SB_HORZ,FALSE); // Show horizontal scrollbar.
      else
         ShowScrollbar(SB_HORZ,TRUE);  // Remove horizontal scrollbar.

      // Check if we need the vertical scrollbar.
      if ((rcVisible.top<=rcLogical.top)&&(rcVisible.bottom>=rcLogical.bottom))
         ShowScrollbar(SB_VERT,FALSE); // Show vertical scrollbar.
      else
         ShowScrollbar(SB_VERT,TRUE);  // Remove vertical scrollbar.
   }
}


//---------------------------------------------------------------------
// ScrollbarWindowProc():
//---------------------------------------------------------------------
LRESULT WPWINDOW::ScrollbarWindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   HDC         hdc;
   POINT       ptDevice;
   int         iNewThumbPos;
   static WORD s_wWMScrollbar;
   static BOOL s_fRepeatMessage;
   static WORD s_wSBCode;

   // Process message...
   switch (uMsg)
   {
      case WM_SYSCOMMAND:
         // Check that either scrollbar has been activated.
         if ((wParam==SC_VSCROLL)||(wParam==SC_HSCROLL))
         {
            // Reset the global variable used to hold the original component hit.
            g_wOriginalComponentHit = SBC_NONE;
            // Store which scrollbar has been made active.
            s_wSBCode      = (WORD)((wParam==SC_VSCROLL)? SB_VERT : SB_HORZ);
            s_wWMScrollbar = (WORD)((wParam==SC_VSCROLL)? WM_VSCROLL : WM_HSCROLL);
            // Capture all mouse input from now on.
            SetCapture(GetHandle());
            // Get the current mouse coordinates.
            GetCursorPos(&ptDevice);
            // Convert device coordinates to non-client window coordinates.
            ptDevice.x -= GetWndPos().x;
            ptDevice.y -= GetWndPos().y;
            // Depending on what component the mouse is positioned on scrollbar.
            g_wOriginalComponentHit = ScrollbarHitTest(s_wSBCode,ptDevice,&iNewThumbPos);
            // Depending on which scrollbar component was hit...
            switch (g_wOriginalComponentHit)
            {
               case SBC_THUMB:
                  PostMessage(GetHandle(),s_wWMScrollbar,SB_THUMBTRACK,MAKELPARAM((WORD)iNewThumbPos,0));
                  break;

               case SBC_LEFT_ARROW:
               case SBC_UP_ARROW:
                  PostMessage(GetHandle(),s_wWMScrollbar,SB_LINEUP,0L);
                  break;

               case SBC_RIGHT_ARROW:
               case SBC_DOWN_ARROW:
                  PostMessage(GetHandle(),s_wWMScrollbar,SB_LINEDOWN,0L);
                  break;

               case SBC_LEFT_RANGE:
               case SBC_UP_RANGE:
                  PostMessage(GetHandle(),s_wWMScrollbar,SB_PAGEUP,0L);
                  break;

               case SBC_RIGHT_RANGE:
               case SBC_DOWN_RANGE:
                  PostMessage(GetHandle(),s_wWMScrollbar,SB_PAGEDOWN,0L);
                  break;

               case SBC_NONE:
                  return (0L);
            }
            // Set the component hit's new state.
            SetScrollComponentState(g_wOriginalComponentHit,SBS_PUSHEDIN);
            // Get window device context.
            hdc = GetWindowDC(GetHandle());
            // Draw component hit's new state.
            DrawScrollComponent(hdc,s_wSBCode,g_wOriginalComponentHit,SBS_PUSHEDIN);
            // Release window device context.
            ::ReleaseDC(GetHandle(),hdc);
         }
         // Reset message repeating flag.
         s_fRepeatMessage = FALSE;
         // Setup a timer for an interval of 400 msecs.
         SetTimer(GetHandle(),IDT_SCROLLBAR,400,NULL);
         break;

      case WM_TIMER:
         // If initial delay timer is set.
         if ((wParam==IDT_SCROLLBAR)&&(s_fRepeatMessage==FALSE))
         {
            // Kill initial delay timer.
            KillTimer(GetHandle(),IDT_SCROLLBAR);
            // Set repeat message flag.
            s_fRepeatMessage = TRUE;
            // Setup a timer for an interval of 50 msecs.
            SetTimer(GetHandle(),IDT_SCROLLBAR_REPEAT,50,NULL);
         }
         else
         if ((wParam==IDT_SCROLLBAR_REPEAT)&&(s_fRepeatMessage==TRUE))
         {
            SetScrollComponentState(g_wOriginalComponentHit,SBS_NORMAL);
            // Get the current mouse coordinates.
            GetCursorPos(&ptDevice);
            // Convert device coordinates to non-client window coordinates.
            ptDevice.x -= GetWndPos().x;
            ptDevice.y -= GetWndPos().y;
            // Depending on what component the mouse is positioned on scrollbar.
            switch (ScrollbarHitTest(s_wSBCode,ptDevice,&iNewThumbPos))
            {
               case SBC_THUMB:
                  if (g_wOriginalComponentHit==SBC_THUMB)
                  {
                     PostMessage(GetHandle(),s_wWMScrollbar,SB_THUMBTRACK,MAKELPARAM((WORD)iNewThumbPos,0));
                     SetScrollComponentState(g_wOriginalComponentHit,SBS_PUSHEDIN);
                  }
                  break;

               case SBC_LEFT_ARROW:
               case SBC_UP_ARROW:
                  if ((g_wOriginalComponentHit==SBC_LEFT_ARROW)||(g_wOriginalComponentHit==SBC_UP_ARROW))
                  {
                     PostMessage(GetHandle(),s_wWMScrollbar,SB_LINEUP,0L);
                     SetScrollComponentState(g_wOriginalComponentHit,SBS_PUSHEDIN);
                  }
                  break;

               case SBC_RIGHT_ARROW:
               case SBC_DOWN_ARROW:
                  if ((g_wOriginalComponentHit==SBC_RIGHT_ARROW)||(g_wOriginalComponentHit==SBC_DOWN_ARROW))
                  {
                     PostMessage(GetHandle(),s_wWMScrollbar,SB_LINEDOWN,0L);
                     SetScrollComponentState(g_wOriginalComponentHit,SBS_PUSHEDIN);
                  }
                  break;

               case SBC_LEFT_RANGE:
               case SBC_UP_RANGE:
                  if ((g_wOriginalComponentHit==SBC_LEFT_RANGE)||(g_wOriginalComponentHit==SBC_UP_RANGE))
                     PostMessage(GetHandle(),s_wWMScrollbar,SB_PAGEUP,0L);
                  break;

               case SBC_RIGHT_RANGE:
               case SBC_DOWN_RANGE:
                  if ((g_wOriginalComponentHit==SBC_RIGHT_RANGE)||(g_wOriginalComponentHit==SBC_DOWN_RANGE))
                     PostMessage(GetHandle(),s_wWMScrollbar,SB_PAGEDOWN,0L);
                  break;
            }
         }
         // Get window device context.
         hdc = GetWindowDC(GetHandle());
         // Draw component hit's new state.
         DrawScrollComponent(hdc,s_wSBCode,g_wOriginalComponentHit,GetScrollComponentState(g_wOriginalComponentHit));
         // Release window device context.
         ::ReleaseDC(GetHandle(),hdc);
         break;

      case WM_MOUSEMOVE:
         // Only if scrollbar thumb is original component hit we need to process this message.
         if (g_wOriginalComponentHit==SBC_THUMB)
         {
            // Store mouse point (in client coordinates).
            ptDevice = MAKEPOINT(lParam);
            // Convert client coordinates to screen (device) coordinates.
            ClientToScreen(GetHandle(),&ptDevice);
            // Convert device coordinates to non-client window coordinates.
            ptDevice.x -= GetWndPos().x;
            ptDevice.y -= GetWndPos().y;
            // If mouse cursor is on scrollbar thumb...
            if (ScrollbarHitTest(s_wSBCode,ptDevice,&iNewThumbPos)==SBC_THUMB)
            {
               PostMessage(GetHandle(),s_wWMScrollbar,SB_THUMBTRACK,MAKELPARAM((WORD)iNewThumbPos,0));
               SetScrollComponentState(g_wOriginalComponentHit,SBS_PUSHEDIN);
            }
            else
               SetScrollComponentState(g_wOriginalComponentHit,SBS_NORMAL);
         }
         break;

      case WM_LBUTTONUP:
      case WM_NCLBUTTONUP:
         // Reset scrollbar input capture internal flag.
         SetScrollbarInputCaptureFlag(FALSE);
         // Release mouse input capture, make sure we had it originally.
         if (GetCapture()==GetHandle())
            ReleaseCapture();
         // Destroy scrollbar timer.
         KillTimer(GetHandle(),IDT_SCROLLBAR);
         KillTimer(GetHandle(),IDT_SCROLLBAR_REPEAT);
         // Reset state of scrollbar component.
         SetScrollComponentState(g_wOriginalComponentHit,SBS_NORMAL);
         // Get window device context.
         hdc = GetWindowDC(GetHandle());
         // Draw component hit's new state.
         DrawScrollComponent(hdc,s_wSBCode,g_wOriginalComponentHit,GetScrollComponentState(g_wOriginalComponentHit));
         // Release window device context.
         ::ReleaseDC(GetHandle(),hdc);
         break;

      case WM_HSCROLL:
      case WM_VSCROLL:
      case WM_ERASEBKGND:
      case WM_PAINT:
         // Return negative one, meaning that we want to pass this message to
         // the object's window procedure.
         return (-1L);

      default:
         break;
   }

   // Return zero, meaning that we have successfully handled the message.
   return (0L);
}



//---------------------------------------------------------------------
// isScrollable():
//---------------------------------------------------------------------
BOOL WPWINDOW::isScrollable(void)
{
   return (m_fScrollable);
}

//---------------------------------------------------------------------
// SetScrollProperty():
//---------------------------------------------------------------------
void WPWINDOW::SetScrollProperty(BOOL fScrollable)
{
   m_fScrollable = fScrollable;
}


//---------------------------------------------------------------------
// OnScrollbarMessage():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPWINDOW::OnScrollbarMessage(UINT uMsg,WORD wScrollCode,int nPos,HWND hwndSBCtl)
{
   int nFinalPos,nCurPos;

   // Process horizontal or vertical scrollbar messages.
   switch (uMsg)
   {
      // Check if message came from the horizontal scrollbar.
      case WM_HSCROLL:

         // Query the current scrollbar position.
         nCurPos = GetScrollPos(GetHandle(),SB_HORZ);

         // Process the scrollbar code...
         switch (wScrollCode)
         {
            case SB_BOTTOM:
               nFinalPos = GetLogicalDomain().right;
               break;

            case SB_ENDSCROLL:
               return;

            case SB_LINEDOWN:
               nFinalPos = nCurPos + GetSystemMetrics(SM_CXICON);
               break;

            case SB_LINEUP:
               nFinalPos = nCurPos - GetSystemMetrics(SM_CXICON);
               break;

            case SB_PAGEDOWN:
               nFinalPos = nCurPos + GetSystemMetrics(SM_CXICONSPACING);
               break;

            case SB_PAGEUP:
               nFinalPos = nCurPos - GetSystemMetrics(SM_CXICONSPACING);
               break;

            case SB_TOP:
               nFinalPos = GetLogicalDomain().left;
               break;

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
               nFinalPos = nPos;
               break;
         }

         // Make sure that final position is in the logical domain range.
         if (nFinalPos < GetLogicalDomain().left)
            nFinalPos = GetLogicalDomain().left;   // force minimum position.
         else
         if (nFinalPos > GetLogicalDomain().right)
            nFinalPos = GetLogicalDomain().right;  // force maximum position.

         // If final position is different than current position...
         if (nFinalPos != nCurPos)
            // Scroll window horizontally by passing an offset from current position.
            ScrollWindowHoriz((nFinalPos-nCurPos));

         break;


      // Check if message came from the vertical scrollbar.
      case WM_VSCROLL:

         // Query the current scrollbar position.
         nCurPos = GetScrollPos(GetHandle(),SB_VERT);

         // Process the scrollbar code...
         switch (wScrollCode)
         {
            case SB_BOTTOM:
               nFinalPos = GetLogicalDomain().bottom;
               break;

            case SB_ENDSCROLL:
               return;

            case SB_LINEDOWN:
               nFinalPos = nCurPos + GetSystemMetrics(SM_CYICON);
               break;

            case SB_LINEUP:
               nFinalPos = nCurPos - GetSystemMetrics(SM_CYICON);
               break;

            case SB_PAGEDOWN:
               nFinalPos = nCurPos + GetSystemMetrics(SM_CYICONSPACING);
               break;

            case SB_PAGEUP:
               nFinalPos = nCurPos - GetSystemMetrics(SM_CYICONSPACING);
               break;

            case SB_TOP:
               nFinalPos = GetLogicalDomain().top;
               break;

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
               nFinalPos = nPos;
               break;
         }

         // Make sure that final position is in the logical domain range.
         if (nFinalPos < GetLogicalDomain().top)
            nFinalPos = GetLogicalDomain().top;    // force minimum position.
         else
         if (nFinalPos > GetLogicalDomain().bottom)
            nFinalPos = GetLogicalDomain().bottom; // force maximum position.

         // If final position is different than current position...
         if (nFinalPos != nCurPos)
            // Scroll window vertically by passing an offset from current position.
            ScrollWindowVert((nFinalPos-nCurPos));

         break;
   }
}



//---------------------------------------------------------------------
// WINDOW CREATION AND DISPLAY FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateWindow():
//    This function creates a Windows 3.1 window with the specified
// styles and attributes.  It does not force a window update or show
// command automatically.  It just creates a window of the class
// specified.  The "pwpobjOwner" parameter is passed to the newly
// created window procedure in the create structure as a creation
// parameter (check WM_CREATE message).
//---------------------------------------------------------------------
BOOL WPWINDOW::CreateWindow(char *pszClassName,WPOBJECT *pwpobjOwner)
{
   // Create custom window.
   m_hwnd = CreateWindowEx(
               GetWindowExStyles(),
               pszClassName,
               pwpobjOwner->GetCaptionTitle(),
               GetWindowStyles(),
               m_ptWindow.x,
               m_ptWindow.y,
               (m_sizeWindow.cx-1),
               (m_sizeWindow.cy-1),
               NULL,
               (HMENU)NULL,
               g_wpEnvironment.hInst,
               pwpobjOwner);

   // Get handle to class icon.
   m_hIcon = (HICON)GetClassWord(m_hwnd,GCW_HICON);

   // Check if window creation was successfull.
   if (m_hwnd!=NULL)
      return (TRUE);
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// DestroyWindow():
//---------------------------------------------------------------------
BOOL WPWINDOW::DestroyWindow(void)
{
   return (TRUE);
}

//---------------------------------------------------------------------
// GetWindowExStyles():
//---------------------------------------------------------------------
DWORD WPWINDOW::GetWindowExStyles(void)
{
   return (WPWINDOW_DEFAULT_EX_STYLES);
}

//---------------------------------------------------------------------
// GetWindowStyles():
//---------------------------------------------------------------------
DWORD WPWINDOW::GetWindowStyles(void)
{
   return (WPWINDOW_DEFAULT_STYLES);
}


//---------------------------------------------------------------------
// UpdateWindow():
//---------------------------------------------------------------------
void WPWINDOW::UpdateWindowPos(void)
{
   // Make changes in window position and size now.
   // (Only if window has been created).
   if (GetHandle()!=NULL)
      SetWindowPos(GetHandle(),(HWND)NULL,
                   m_ptWindow.x,m_ptWindow.y,
                   m_sizeWindow.cx,m_sizeWindow.cy,
                   (SWP_NOACTIVATE|SWP_NOZORDER));
}


//---------------------------------------------------------------------
// WINDOW CLIENT-AREA PAINTING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// BeginPaint():
//---------------------------------------------------------------------
HDC WPWINDOW::BeginPaint(void)
{
   // Check that device context is not already gotten.
   if (m_hdc!=NULL) return (m_hdc);

   // Get window device context using Windows 3.1 API call.
   m_hdc = ::BeginPaint(m_hwnd,&m_ps);

   // Setup map modes, origins, etc...
   SetMapMode(m_hdc,MM_TEXT);
   SetWindowOrg(m_hdc,m_rcVisible.left,m_rcVisible.top);

   // Return ready device context.
   return (m_hdc);
}

//---------------------------------------------------------------------
// EndPaint():
//---------------------------------------------------------------------
void WPWINDOW::EndPaint(void)
{
   // Check if we have a valid device context to release.
   if (m_hdc!=NULL)
      ::EndPaint(m_hwnd,&m_ps);

   // Reset device context to "none open".
   m_hdc = NULL;
}



//---------------------------------------------------------------------
// GetDC():
//---------------------------------------------------------------------
HDC WPWINDOW::GetDC(void)
{
   // Check that device context is not already gotten.
   if (m_hdc!=NULL) return (m_hdc);

   // Get window device context using Windows 3.1 API call.
   m_hdc = ::GetDC(m_hwnd);

   // Setup map modes, origins, etc...
   SetMapMode(m_hdc,MM_TEXT);
   SetWindowOrg(m_hdc,m_rcVisible.left,m_rcVisible.top);

   // Return ready device context.
   return (m_hdc);
}

//---------------------------------------------------------------------
// ReleaseDC():
//---------------------------------------------------------------------
void WPWINDOW::ReleaseDC(void)
{
   // Check if we have a valid device context to release.
   if (m_hdc!=NULL)
      ::ReleaseDC(m_hwnd,m_hdc);

   m_hdc = NULL;
}


//---------------------------------------------------------------------
// COORDINATE SYSTEM CONVERTION FUNCTIONS
//---------------------------------------------------------------------
// NOTE:
//    DP or Device Point(s)  = Coordinate relative to the screen device's
//                             top-left corner as (0,0).
//    WP or Window Point(s)  = Coordinate relative to the window's client
//                             area as (0,0).
//    LP or Logical Point(s) = Coordinate relative to the folder's device
//                             context origin or logical view.


//---------------------------------------------------------------------
// LPtoWP():
//    This function converts Logical point(s) into Window points(s).
//---------------------------------------------------------------------
void WPWINDOW::LPtoWP(LPPOINT lpptLogical,int nPoints)
{
   HDC hdc;

   // Get window device context.
   hdc = GetDC();
   // Call Windows 3.1 logical to window point convertion.
   ::LPtoDP(hdc,lpptLogical,nPoints);
   // Release window device context.
   ReleaseDC();
}


//---------------------------------------------------------------------
// WPtoDP():
//    This function converts Window point(s) into Device points(s).
//---------------------------------------------------------------------
void WPWINDOW::WPtoDP(LPPOINT lpptWindow,int nPoints)
{
   int index;

   // Convert window point(s) into device point(s).
   for (index=0; index<nPoints; index++)
      ClientToScreen(GetHandle(),&(lpptWindow[index]));
}


//---------------------------------------------------------------------
// DPtoWP():
//    This function converts Device point(s) into Window points(s).
//---------------------------------------------------------------------
void WPWINDOW::DPtoWP(LPPOINT lpptDevice,int nPoints)
{
   int index;

   // Convert device point(s) into window point(s).
   for (index=0; index<nPoints; index++)
      ScreenToClient(GetHandle(),&(lpptDevice[index]));
}


//---------------------------------------------------------------------
// WPtoLP():
//    This function converts Window point(s) into Logical points(s).
//---------------------------------------------------------------------
void WPWINDOW::WPtoLP(LPPOINT lpptWindow,int nPoints)
{
   HDC hdc;

   // Get window device context.
   hdc = GetDC();
   // Call Windows 3.1 logical to window point convertion.
   ::DPtoLP(hdc,lpptWindow,nPoints);
   // Release window device context.
   ReleaseDC();
}


//---------------------------------------------------------------------
// LPtoDP():
//    This function converts Logical point(s) into Device points(s).
//---------------------------------------------------------------------
void WPWINDOW::LPtoDP(LPPOINT lpptLogical,int nPoints)
{
   // First, convert logical point(s) into window point(s).
   LPtoWP(lpptLogical,nPoints);
   // Then convert window point(s) into device point(s).
   WPtoDP(lpptLogical,nPoints);
}


//---------------------------------------------------------------------
// DPtoLP():
//    This function converts Device point(s) into Logical points(s).
//---------------------------------------------------------------------
void WPWINDOW::DPtoLP(LPPOINT lpptDevice,int nPoints)
{
   // First convert device point(s) into window point(s).
   DPtoWP(lpptDevice,nPoints);
   // Then convert window point(s) into logical point(s).
   WPtoLP(lpptDevice,nPoints);
}


//---------------------------------------------------------------------
// WINDOW NON-CLIENT AREA PAINTING FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// NCPaint():
//    This function paints the non-client area of the window as
// appropriate for the active state and screen resolution.
//---------------------------------------------------------------------
void WPWINDOW::NCPaint(BOOL fActive,HICON hIcon)
{
   HDC   hdc;
   RECT  rcWindow;
   int   nWidth,nHeight;

   // Get window device context (which includes the non-client areas).
   hdc = GetWindowDC(m_hwnd);

   // Get window size and position.
   GetWindowRect(m_hwnd,&rcWindow);

   // Calculate its width and height.
   nWidth  = rcWindow.right  - rcWindow.left;
   nHeight = rcWindow.bottom - rcWindow.top;

   // Draw object icon on system menu (top-left window button).
   DrawIconOnSystemMenu(hdc,hIcon);

   // Fill window frame with frame fill color.
   FillFrame(hdc,nWidth,nHeight,fActive);

   // Fill caption bar area.
   FillCaptionBar(hdc,nWidth,nHeight,fActive);

   // Draw window 3-D frame outline.
   Outline3DFrame(hdc,nWidth,nHeight);

   // Draw window title text on caption bar.
   DrawCaptionBarText(hdc,nWidth,nHeight,fActive);

   // Draw min/max buttons in "up" position.
   DrawMinMaxButtons(hdc,nWidth,FALSE,FALSE);

   // Check if window has scrollable property.
   if (isScrollable()==TRUE)
   {
      // Check if horizontal scrollbar is enabled and needs to be painted.
      if (isScrollbarEnabled(SB_HORZ)==TRUE)
         DrawScrollbar(hdc,SB_HORZ);

      // Check if vertical scrollbar is enabled and needs to be painted.
      if (isScrollbarEnabled(SB_VERT)==TRUE)
         DrawScrollbar(hdc,SB_VERT);
   }

   // Release device context before returning.
   ::ReleaseDC(m_hwnd,hdc);
}




//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPWINDOW::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_WINDOW_STRUCT   fwsData;

   // Read container extra information.
   if (read(hProfile,&fwsData,sizeof(FILE_WINDOW_STRUCT))!=sizeof(FILE_WINDOW_STRUCT))
      return (FALSE);

   // Assign profile values.
   SetWndPos(fwsData.m_ptWindow);
   SetWndSize(fwsData.m_sizeWindow);
   SetScrollProperty(fwsData.m_fScrollable);

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPWINDOW::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_WINDOW_STRUCT   fwsData;
   WINDOWPLACEMENT   wplFolder;

   // If window is exists, and is maximized...
   if (IsWindow(GetHandle())==TRUE)
   {
      // Store the last "normal" folder window size and position.
      // Get the "restored" or normal window position.
      wplFolder.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(GetHandle(),&wplFolder);
      // Calculate and store the "normal" window position and size.
      fwsData.m_ptWindow.x    = wplFolder.rcNormalPosition.left;
      fwsData.m_ptWindow.y    = wplFolder.rcNormalPosition.top;
      fwsData.m_sizeWindow.cx = (wplFolder.rcNormalPosition.right-wplFolder.rcNormalPosition.left+1);
      fwsData.m_sizeWindow.cy = (wplFolder.rcNormalPosition.bottom-wplFolder.rcNormalPosition.top+1);
   }
   else
   {
      // Get and store the last window position and size.
      fwsData.m_ptWindow   = GetWndPos();
      fwsData.m_sizeWindow = GetWndSize();
   }
   // Get scrollable property flag.
   fwsData.m_fScrollable = isScrollable();

   // Read container extra information.
   if (write(hProfile,&fwsData,sizeof(FILE_WINDOW_STRUCT))!=sizeof(FILE_WINDOW_STRUCT))
      return (FALSE);
   else
      return (TRUE);
}

// EOF WPWINDOW.CPP

