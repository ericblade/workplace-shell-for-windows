//=====================================================================
// WPPALETT.CPP
//    WPPALETTE Class implementation.
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

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
// Workplace Shell resource ID's header file.


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From PALETTE.CPP file:
LRESULT  CALLBACK WPPaletteWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
   
//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define WPPALETTE_CLASS_NAME     "WPPALETTE_Window"
                   
//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static int  sg_iClassUsageCount = 0;
                   

//---------------------------------------------------------------------
// P R I V A T E  M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// RegisterPaletteClass():
//    This function is a key function in creating palette object's
// open windows.  We register a window class for ALL palette objects
// so that they use the same wrapper window procedure.
//---------------------------------------------------------------------
BOOL WPPALETTE::RegisterPaletteClass(void)
{
   WNDCLASS  wc;

   // Check if class has already been initialized.
   if (++sg_iClassUsageCount==1)
   {                                              
      // Fill class structure fields.
      wc.style         = CS_HREDRAW | CS_VREDRAW;
      wc.lpszClassName = WPALETTE_CLASS_NAME;
      wc.hInstance     = g_wpEnvironment.hInst;
      wc.lpfnWndProc   = WPPaletteWindowProcWrapper;
      wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wc.hIcon         = NULL;
      wc.lpszMenuName  = NULL;
      wc.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = sizeof(LONG);
      // Register palette class, and check if successful...
      if (RegisterClass(&wc)==0)
         return (FALSE);
   }
   // Return success registering class.
   return (TRUE);
}

//---------------------------------------------------------------------
// UnregisterPaletteClass():
//---------------------------------------------------------------------
void WPPALETTE::UnregisterPaletteClass(void)
{
   // Decrement the usage count for the palette object...
   if (--sg_iClassUsageCount==0)
   {                                                           
      // Unregister the palette window class name.
      UnregisterClass(WPPALETTE_CLASS_NAME,g_wpEnvironment.hInst);
   }
}

//---------------------------------------------------------------------
// CreateWindow():
//    This function creates a Windows 3.1 window with the specified
// styles and attributes.  It does not force a window update or show
// command automatically.  It just creates a window of the class
// specified.  The "this" parameter is passed to the newly created
// window procedure in the create structure as a creation parameter
// (check WM_CREATE message).
//---------------------------------------------------------------------
BOOL WPPALETTE::CreateWindow(char *pszClassName,WPOBJECT *pwpobjOwner)
{  
   HWND hwnd;
   
   // Create custom window.
   hwnd = CreateWindowEx(
               GetWindowExStyles(),
               pszClassName,
               GetCaptionTitle(),
               GetWindowStyles(),
               GetWndPos().x,
               GetWndPos().y,
               GetWndSize().cx,
               GetWndSize().cy,
               NULL,
               (HMENU)NULL,
               g_wpEnvironment.hInst,
               this);

   // Store window handle.
   SetHandle(hwnd);

   // Check if window creation was successfull.
   return ((hwnd!=NULL)? TRUE : FALSE);
}
                         


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPPALETTE
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPPALETTE:WPPALETTE(void) : WPOBJECT() , WPWINDOW()
{                        
   // Initialize folder object.
   SetType(OBJTYPE_PALETTE);
   // Set window class properties.
   SetScrollProperty(FALSE);
}


//---------------------------------------------------------------------
// WPPALETTE():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPPALETTE::WPPALETTE(WPPALETTE *pwppalDefaults) : 
   WPOBJECT((WPOBJECT *)pwppalDefaults)
   WPWINDOW((WPWINDOW *)pwppalDefaults)

{
   // Initialize program object.   
   SetType(OBJTYPE_PALETTE);
   // Set window class properties.
   SetScrollProperty(FALSE);
}


//---------------------------------------------------------------------
// PALETTE STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Open():
//    This function creates a window and display the current palette
// values (being colors, fonts, shcemes, etc...).
//---------------------------------------------------------------------
BOOL WPPALETTE::Open(void)
{
   char *pszClassName;

   // If folder is not already open...
   if (isOpen()==FALSE)
   {
      // Register class for palette objects...
      if (RegisterPaletteClass(WPClientWindowProcWrapper)==TRUE)
      {
         // Create palette object's overlapped window.
         if (CreateWindow(WPPALETTE_CLASS_NAME,this)==FALSE)
         {
            // Unregister palette object window class before exiting with an error.
            UnregisterPaletteClass();
            // Return failure.
            return (FALSE);
         }
         
         // Load icon resource for palette object.   
         m_hPaletteIcon = ExtractIcon();
            
         // Set "opened" state bit.
         SetState((GetState()|OBJSTATE_OPEN));

         // Update palette object icon's visual state on parent container.
         InvalidateIcon(TRUE,TRUE);
         
         // Update palette object's shadows visual state (if any).
         InvalidateShadows(TRUE);

         // Show window and update it.
         ShowWindow(GetHandle(),SW_SHOW);
         ::UpdateWindow(GetHandle());

         // Let Workplace Shell desktop know that this object "owns" the newly create palette object window.
         PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_HWND_ASSOCIATE_OBJECT,(WPARAM)GetHandle(),(LPARAM)this);
      }
      else
         // Return failure.
         return (FALSE);
   }
   else
   {
      // Give focus to already existing palette object window...
      // If window is minimized, restore to original size.
      if (IsIconic(GetHandle())!=FALSE)
         ShowWindow(GetHandle(),SW_RESTORE);

      // Set active focus to this palette object window.
      SetActiveWindow(GetHandle());
   }

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPPALETTE::Close(void)
{
   RECT              rcWindow,rcShrinkedWindow;
   WINDOWPLACEMENT   wplPalette;

   // Check if palette window is open...
   if (isOpen()==TRUE)
   {
      // If palette window is not minimized, "implode" window.
      if (IsIconic(GetHandle())==FALSE)
      {
         // Get current position and size of window.
         GetWindowRect(GetHandle(),&rcWindow);

         // Get final position and size of shrinked window (3x3 rectangle).
         SetRect(&rcShrinkedWindow,((rcWindow.left+rcWindow.right)/2)-1,
                                   ((rcWindow.top+rcWindow.bottom)/2)-1,
                                   ((rcWindow.left+rcWindow.right)/2)+1,
                                   ((rcWindow.top+rcWindow.bottom)/2)+1);

         // "Implode" window frame.
         AnimateWindowFrame(&rcWindow,&rcShrinkedWindow,0,8);
      }

      // Set palette window state to "closed".
      SetState((GetState()&(~OBJSTATE_OPEN)));

      // Store the last "normal" palette window size and position.
      // Get the "restored" or normal window position.
      wplPalette.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(GetHandle(),&wplPalette);
      // Calculate and store the "normal" window position and size.
      SetWndPos(wplPalette.rcNormalPosition.left,wplPalette.rcNormalPosition.top);
      SetWndSize((wplPalette.rcNormalPosition.right-wplPalette.rcNormalPosition.left+1),
                 (wplPalette.rcNormalPosition.bottom-wplPalette.rcNormalPosition.top+1));

      // Since the window is being closed, we must notify its
      // parent window to repaint the folder's icon completely because
      // the "hatch" rectangle on the icon must be erased.
      InvalidateIcon(TRUE,TRUE);

      // Invalidate folder's shadows (if any) visual state.
      InvalidateShadows(TRUE);

      // Call Windows 3.1 API to destroy palette window.
      ::DestroyWindow(GetHandle());
   }

   return (TRUE);       // Successfully closed window.
}


//---------------------------------------------------------------------
// WINDOW MESSAGES MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnCreate():
//---------------------------------------------------------------------
void WPPALETTE::OnCreate(void)
{                
   WORD     wViewMode;
   RECT     rcIcon,rcWindow,rcLogDomain;
   HPBYTE   lpDIBMem;
                     
   // If folder has parent, get parent's open view mode.
   if (GetParent()!=NULL)
      wViewMode = GetParent()->GetOpenView();
   else
      wViewMode = VIEW_ICON;  // Else, default to icon view mode.

   // Depending on the folder's view mode...
   switch (wViewMode)
   {
      case VIEW_ICON:
         // Get position and size of object's icon rectangle.
         SetRect(&rcIcon,GetIconPos().x,GetIconPos().y,
                 GetIconPos().x+GetSystemMetrics(SM_CXICON)-1,
                 GetIconPos().y+GetSystemMetrics(SM_CYICON)-1);
         break;

      case VIEW_DETAILS:
         // First, get object's details view table entry position.
         GetFrameRect(&rcIcon);
         // Calculate position of object's icon rectangle within details view.
         SetRectCXCY(&rcIcon,
                     (rcIcon.left+DV_ICON_INDENT_CX),
                     (rcIcon.top+DV_ICON_INDENT_CY),
                     DV_ICON_SIZE_CX,DV_ICON_SIZE_CY);
         break;
   }
   
   // If parent folder exists AND is open AND is not in minimized state...
   if ((GetParent()!=NULL)&&(GetParent()->isOpen()==TRUE)&&(IsIconic(((WPFOLDER *)GetParent())->GetHandle())==FALSE))
      // Convert rectangle from logical coordinates to device coordinates.
      ((WPFOLDER *)GetParent())->LPtoDP((LPPOINT)&rcIcon,2);
   else       
   {  
      // Folder does not have parent OR parent folder is not open OR
      // parent folder is minimized.
      // Therefore, folder will get animated from center of screen.
      rcIcon.left   = ((GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON))/2);
      rcIcon.top    = ((GetSystemMetrics(SM_CYSCREEN)-GetSystemMetrics(SM_CYICON))/2);
      rcIcon.right  = rcIcon.left+GetSystemMetrics(SM_CXICON);
      rcIcon.bottom = rcIcon.top+GetSystemMetrics(SM_CYICON);
   }

   // Get window position and size rectangle.
   GetWindowRect(GetHandle(),&rcWindow);

   // "Explode" window frame.
   AnimateWindowFrame(&rcIcon,&rcWindow,0,8);

   // If folder has scrollable property for client space.
   if (isScrollable()==TRUE)
   {
      // Get the current logical domain rect.
      CalculateLogicalDomainRect(&rcLogDomain);
      // Set the logical domain rect.
      SetLogicalDomain(&rcLogDomain);
   }
}
                                       
//---------------------------------------------------------------------
// OnMove():
//    This function updates the internal variables that keep track of
// the folder window position.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPNOTEBOOK::OnMove(POINT ptTopLeft)
{                          
   RECT rcWindow;

   // If window is not iconic, save new window position...
   if (IsIconic(GetHandle())==FALSE)   
   {
      // Get new window size.
      GetWindowRect(GetHandle(),&rcWindow);
      // Update internal window size values.
      SetWndPos(rcWindow.left,rcWindow.top);
   }
}

//---------------------------------------------------------------------
// OnSize():         
//    This function updates the internal variables that keep track of
// the folder window size and logical visible area.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPNOTEBOOK::OnSize(WORD wSizeType,int nClientWidth,int nClientHeight)
{     
   RECT rcWindow,rcVisible;
   
   // If window is not iconic, save new window size...
   if (IsIconic(GetHandle())==FALSE)
   {                     
      // Get new window size.
      GetWindowRect(GetHandle(),&rcWindow);
      // Update internal window size values.
      SetWndSize((rcWindow.right-rcWindow.left+1),
                 (rcWindow.bottom-rcWindow.top+1));
   
      // Update logical visible area in notebook.
      rcVisible = GetVisibleArea();
      SetRect(&rcVisible,rcVisible.left,rcVisible.top,
              (rcVisible.left+nClientWidth),(rcVisible.top+nClientHeight));
      SetVisibleArea(&rcVisible);
   
      // Adjust the notebook window to fit the client window.
      if (GetDlgItem(GetHandle(),IDC_NOTEBOOK)!=NULL)
         MoveWindow(GetDlgItem(GetHandle(),IDC_NOTEBOOK),
                    0,0,nClientWidth,nClientHeight,TRUE);
   }
}  


//---------------------------------------------------------------------
// OnCommand():
//---------------------------------------------------------------------
void WPPALETTE::OnCommand(HWND hwndPalette,WORD wNotifyCode)
{
   
}


//---------------------------------------------------------------------
// OnSysCommand():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnSysCommand(WORD wValue,LPARAM lParam)
{
   RECT  rcWindow,rcShrinkedWindow;

   // Depending on what system command is requested...
   switch (wValue)
   {
      // Minimize folder command.
      case SC_MINIMIZE:
         // Check if folder is already minimized.
         if (IsIconic(GetHandle())==FALSE)
         {
            // Get window rectangle size.
            GetWindowRect(GetHandle(),&rcWindow);
            // Get final position and size of shrinked window (3x3 rectangle).
            SetRect(&rcShrinkedWindow,((rcWindow.left+rcWindow.right)/2)-1,
                                      ((rcWindow.top+rcWindow.bottom)/2)-1,
                                      ((rcWindow.left+rcWindow.right)/2)+1,
                                      ((rcWindow.top+rcWindow.bottom)/2)+1);
            // "Inplode" window frame.
            AnimateWindowFrame(&rcWindow,&rcShrinkedWindow,0,8);
         }       
         // Call the default system command handler...                                           
         DefWindowProc(GetHandle(),WM_SYSCOMMAND,wValue,lParam);
         break;
         
      default:       
         // Pass system command to default window procedure.
         DefWindowProc(GetHandle(),WM_SYSCOMMAND,wValue,lParam);
   }   
}

//---------------------------------------------------------------------
// OnQueryDragIcon():
//---------------------------------------------------------------------
LRESULT WPNOTEBOOK::OnQueryDragIcon(void)
{
   return (MAKELRESULT(m_hPaletteIcon,0));
}


//---------------------------------------------------------------------
// OnEraseBackground():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnEraseBackground(HDC hdc)
{
   // Have default window procedure erase window background.
   DefWindowProc(GetHandle(),WM_ERASEBKGND,(WPARAM)hdc,0L);

   // If notebook window is not minimized...
   if (IsIconic(GetHandle())==TRUE)
      // We are in minimized state, so draw notebook's owner icon.
      DrawIcon(hdc,0,0,m_hPaletteIcon);
}

//---------------------------------------------------------------------
// OnPaint():
//---------------------------------------------------------------------
void WPPALETTE::OnPaint(void)
{                     
   HDC hdc;

   // Start paint function.
   hdc = BeginPaint();
   // Draw palette object values in client area.
   //DrawPaletteValues(hdc);
   // end paint function.
   EndPaint();
}

      
//---------------------------------------------------------------------
// OnNCPaint():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnNCPaint(void)
{         
   BOOL  fActive;
             
   // If window is minimized, do not paint, just exit.                                           
   if (IsIconic(GetHandle())==TRUE)
      return;                                           
                                           
   // Check if currently active window is one of the notebook pages...
   if (IsPageWindow(GetActiveWindow())==TRUE)
      // Always true to prevent problem with notebook caption bar highlighting.   
      fActive = TRUE;   
   else
      // Find out if notebook window is the currently active window.
      fActive = ((GetActiveWindow()==GetHandle())? TRUE : FALSE);
   
   // Paint non-client area of window.
   NCPaint(fActive,m_hPaletteIcon);
}


//---------------------------------------------------------------------
// OnNCActivate():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnNCActivate(BOOL fActive)
{  
   // If window is minimized, do not update non-client area, just exit.
   if (IsIconic(GetHandle())==TRUE) 
      return;      

   // Paint non-client area of window.
   NCPaint(fActive,m_hPaletteIcon);
}
   
                       
//---------------------------------------------------------------------
// OnNCMouseMove():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnNCMouseMove(WORD wHitTestCode,POINT ptDevice)
{  
   HDC   hdc;
   RECT  rcWindow;
   int   nWidth;

   // If window is currently minimzed, then call default window procedure.
   if (IsIconic(GetHandle())==TRUE)
   {
      DefWindowProc(GetHandle(),WM_NCMOUSEMOVE,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
      return;
   }     
   
   // Get the folder window device context.
   hdc = GetWindowDC(GetHandle());
   
   // Get window frame size and position.
   GetWindowRect(GetHandle(),&rcWindow); 
   // Calculate window frame width (used by draw min/max button function). 
   nWidth = (rcWindow.right - rcWindow.left);
   
   // Check if mouse movement is on top of min/max window buttons.
   if ((wHitTestCode==HTMAXBUTTON)||(wHitTestCode==HTMINBUTTON))
   {
      // If left button was pressed...
      if (m_fNCLButtonDown==TRUE)
      {
         // Draw min/max buttons in their respective state.
         if (wHitTestCode==HTMAXBUTTON)
            DrawMinMaxButtons(hdc,nWidth,FALSE,TRUE);
         else
            DrawMinMaxButtons(hdc,nWidth,TRUE,FALSE);
      }
      else
         DrawMinMaxButtons(hdc,nWidth,FALSE,FALSE);
         
      // Release window device context before leaving.
      ::ReleaseDC(GetHandle(),hdc);
   }
   else
   {
      DrawMinMaxButtons(hdc,nWidth,FALSE,FALSE);
      // Release window device context before leaving.
      ::ReleaseDC(GetHandle(),hdc);
      // Call the default window procedure, we do not want to handle this message.
      DefWindowProc(GetHandle(),WM_NCMOUSEMOVE,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
   }
}                     

//---------------------------------------------------------------------
// OnNCLButton():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnNCLButton(UINT uMsg,WORD wHitTestCode,POINT ptDevice)
{                  

   // If window is currently minimzed, then call default window procedure.
   if (IsIconic(GetHandle())==TRUE)
   {
      DefWindowProc(GetHandle(),uMsg,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
      return;
   }     

   switch (uMsg)
   {
      case WM_NCLBUTTONDOWN:
         // Check if user clicked on top of one of the min/max window buttons.
         if ((wHitTestCode==HTMAXBUTTON)||(wHitTestCode==HTMINBUTTON))
         {                                                                    
            // User clicked min or max window button, set left button down flag.
            m_fNCLButtonDown = TRUE;
            // Update visual button state now.
            SendMessage(GetHandle(),WM_NCMOUSEMOVE,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
         }        
         else                         
            // We do not process this message, let default window proc. handle it.
            DefWindowProc(GetHandle(),uMsg,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
         break;
            
            
      case WM_NCLBUTTONUP:
         // If left mouse button was currently pressed (down).
         if (m_fNCLButtonDown==TRUE)
         {
            // Reset internal "left-mouse-button-down" flag.
            m_fNCLButtonDown = FALSE;
            // Update the visual button state.
            SendMessage(GetHandle(),WM_MOUSEMOVE,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
            // Take corresponding action depending on button depressed.
            // If user pressed max/restore button.
            if (wHitTestCode==HTMAXBUTTON)
            {  
               // If window is not maximized already...
               if (IsZoomed(GetHandle())==FALSE)
                  PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MAXIMIZE,MAKELPARAM(ptDevice.x,ptDevice.y)); // maximize.
               else
                  PostMessage(GetHandle(),WM_SYSCOMMAND,SC_RESTORE,MAKELPARAM(ptDevice.x,ptDevice.y));  // restore.
            }
            else
            // If user pressed the min button.
            if (wHitTestCode==HTMINBUTTON)
               PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MINIMIZE,MAKELPARAM(ptDevice.x,ptDevice.y));    // minimize.
         }
         else         
            // We do not handle this message, let default window proc. handle it.
            DefWindowProc(GetHandle(),uMsg,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
         break;
   }
}
         

//---------------------------------------------------------------------
// OnMouseMove():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnMouseMove(void)
{   
   // Check for user moving mouse from non-client area into client area
   // with the left mouse button pressed down.
   if (m_fNCLButtonDown==TRUE)
   {  
      // Reset left mouse button down flag.
      m_fNCLButtonDown = FALSE;
      // Restore visual state of min/max buttons to "up". 
      SendMessage(GetHandle(),WM_NCMOUSEMOVE,HTMAXBUTTON,0L);
   }
}
         
//---------------------------------------------------------------------
// OnClose():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnClose(void)
{     
   // Have Workplace Shell desktop close and destroy notebook.
   SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_DESTROY_NOTEBOOK,0,(DWORD)this);
}
         

//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------



// EOF WPPALETT.CPP

