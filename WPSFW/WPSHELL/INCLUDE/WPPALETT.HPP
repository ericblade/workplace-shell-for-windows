//=====================================================================
// WPPALETT.HPP
//    WPPALETTE Class declaration.
// 
// Date Created :  01/04/95
// Last Update  :  01/04/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/04/95    | Creation.                               | MDA
//=====================================================================

#ifndef __WPPALETTE_PROTOTYPED__

#define __WPPALETTE_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL ENUMERATIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
        
//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPPALETTE: public WPOBJECT, public WPWINDOW
{       
   protected:
      HICON       m_hPaletteIcon;
      BOOL        m_fNCLButtonDown;

   private:   
      BOOL  RegisterPaletteClass(void);

   public:            
      WPPALETTE(void);
      WPPALETTE(WPPALETTE *pwppalDefaults);
                     
      // Folder state functions.
      BOOL              Open(void);
      BOOL              Close(void);

      // Client window procedure for palette objects.      
      virtual LRESULT   ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
      
      // Window messages management functions.
      void              OnCreate(void);
      void              OnMove(POINT ptTopLeft);
      void              OnSize(WORD wSizeType,int nClientWidth,int nClientHeight);
      void              OnCommand(HWND hwndPalette,WORD wNotifyCode);
      void              OnSysCommand(WORD wValue,LPARAM lParam);
      LRESULT           OnQueryDragIcon(void);
      void              OnEraseBackground(HDC hdc);
      virtual void      OnPaint(void);            
      void              OnNCPaint(void);
      void              OnNCActivate(BOOL fActive);
      void              OnNCMouseMove(WORD wHitTestCode,POINT ptDevice);
      void              OnNCLButton(UINT uMsg,WORD wHitTestCode,POINT ptDevice);
      void              OnMouseMove(void);
      void              OnClose(void);
};
        

#endif

// EOF WPPALETT.HPP

