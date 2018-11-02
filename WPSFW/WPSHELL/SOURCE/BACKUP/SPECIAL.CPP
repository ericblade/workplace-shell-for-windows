//---------------------------------------------------------------------
// SPECIAL.CPP
//

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
#include "wps.hpp"
#include "resource.h"

//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
BOOL     fSpecialMode = FALSE;


//---------------------------------------------------------------------
// DrawBitmapWithMask():
//---------------------------------------------------------------------
void DrawBitmapWithMask(HDC hdc,int x,int y,HBITMAP hbm,HBITMAP hbmMask)
{   
   HDC      hdcMemory;
   HBITMAP  hbmpOld;
   BITMAP   bm;

   GetObject(hbm,sizeof(BITMAP),&bm);
   hdcMemory = CreateCompatibleDC(hdc);
   
   hbmpOld = (HBITMAP)SelectObject(hdcMemory,hbmMask);
   BitBlt(hdc,x,y,bm.bmWidth,bm.bmHeight,hdcMemory,0,0,SRCAND);
   
   SelectObject(hdcMemory,hbm);
   BitBlt(hdc,x,y,bm.bmWidth,bm.bmHeight,hdcMemory,0,0,SRCPAINT);
   
   SelectObject(hdcMemory,hbmpOld);
   DeleteDC(hdcMemory);
}   
   
   
//---------------------------------------------------------------------
// BigShell():
//---------------------------------------------------------------------
void BigShell(POINT ptScreen)
{
   HDC      hdcScreen;
   HBITMAP  hbmShot,hbmShotMask;
   
   hbmShot     = LoadBitmap(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDB_SPECIAL_BITMAP));
   hbmShotMask = LoadBitmap(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDB_SPECIAL_MASK));
   
   hdcScreen = GetDC(NULL);
   
   DrawBitmapWithMask(hdcScreen,ptScreen.x,ptScreen.y,hbmShot,hbmShotMask);
          
   DeleteObject(hbmShot);
   DeleteObject(hbmShotMask);
          
   ReleaseDC(NULL,hdcScreen);
}


//---------------------------------------------------------------------
// SPECIALWindowProc():
//
//---------------------------------------------------------------------
void SPECIALWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{        
   POINT           ptPoint;
   static HCURSOR  hcursorShotgun;
   int             index;

   switch (uMsg)
   {                            
      case WM_CREATE:
         SetCapture(hWnd);
         fSpecialMode   = TRUE;
         hcursorShotgun = LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_SPECIAL));
         SetCursor(hcursorShotgun);
         break;
         
      case WM_MOUSEMOVE:
      case WM_SETCURSOR:
         SetCursor(hcursorShotgun);         
         break;
         
      case WM_LBUTTONUP:
         ptPoint = MAKEPOINT(lParam);
         ClientToScreen(hWnd,&ptPoint);
         // shoot...
         BigShell(ptPoint);
         // sounds...
         MessageBeep(-1);
         // kick-back...
         for (index=1; index<10; index++)
            SetCursorPos(ptPoint.x+(2*index),ptPoint.y+index);
         break;
         
      case WM_RBUTTONDBLCLK:
         DestroyCursor(hcursorShotgun);
         ReleaseCapture();
         fSpecialMode = FALSE;
         break;
   }         
}


