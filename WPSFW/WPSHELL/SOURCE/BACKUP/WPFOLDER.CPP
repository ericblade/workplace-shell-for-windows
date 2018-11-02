//=====================================================================
// WPFOLDER.CPP       
//    WPFOLDER Class implementation.
//
// Date Created :  01/26/94
// Last Update  :  04/13/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
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
#include "wps.hpp"                  // Workplace Shell header file.
#include "imagelib.hpp"             // DIB/BMP Image library header file.
// C-runtime library header files.
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <sys\stat.h>
// Windows library header files.
#include <windowsx.h>
#include <shellapi.h>
#include <commdlg.h>
// Workplace Shell resource ID's header file.
#include "resource.h"               // Workplace Shell resource IDs.
#include "wphelpid.h"               // Workplace Shell help file IDs.


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From MISFUNC.CPP file:
WPOBJECT *new_object(WPOBJECT *pwpobjNew);
void     delete_object(WPOBJECT *pwpobjDelete);
WPOBJECT *CreateTreeDuplicate(WPCONTAINER *pwpcontainerParent,WPOBJECT *pwpobjOrigRoot);
void     PlaySystemEventSound(const char *pszSystemEvent);
LRESULT  CALLBACK WPClientWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
// From FLDRFIND.CPP file:
BOOL CALLBACK FindDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
// From FLDRCOPY.CPP file:
BOOL CALLBACK CopyMoveAndCreateDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Folder object default values.
#define WPS_FOLDER_DEFAULT_ICONINDEX    ICONINDEX(IDI_DEFAULTFOLDER)
#define WPS_HELPFILE_ICONINDEX          ICONINDEX(IDI_HELP)
// Folder system menu resource name (popup menu).
#define WPFOLDER_SYSMENU_NAME          FolderPopUpMenu
// Cursor creation constants.
#define RGB_ICON_TRANSPARENT_COLOR     RGB(0x80,0x00,0x80)  // Dark Magenta
#define MAX_OBJECTS_IN_CURSOR          3                    // 3 objects
// Popup frame (rounded rect.) "roundness" ratio with rectangle size.
#define FRAME_ROUND_RATIO              (0.125F)             // 12.5%
// Objects relative positioning "error zone" calculation value.
#define ERROR_ZONE                     40                   // 40 pixels

//---------------------------------------------------------------------
// WORKPLACE SHELL FOLDER CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R I V A T E   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FUNCTIONS TO SETUP AND CLEANUP THE FOLDER'S VIEW DEVICE CONTEXT
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetUpDC():
//    This function initializes a folder device context for updating
// the children objects.  It sets up all the drawing attributes and 
// fonts used by this folder object.  It also returns the original 
// (or previous) value of the device context which are used when 
// restoring it to its original condition in "CleanUpDC()".
//---------------------------------------------------------------------
void WPFOLDER::SetUpDC(HDC hdc,HFONT *phfontOld,int *piPrevBk,int *piPrevMap)
{
   HFONT    hfont;
   LOGFONT  logfontIconText;    // Logical font information structure.
      
   // Set up the correct mapping mode and save the original mode.
   *piPrevMap = SetMapMode(hdc,MM_TEXT);
   // Set drawing mode to opaque.
   *piPrevBk  = SetBkMode(hdc,OPAQUE);
         
   // If current open view of folder is "icon view"... 
   if (GetOpenView()==VIEW_ICON)
      // Get logical font structure for icon text from parent folder.
      GetIconTextLogFont(&logfontIconText);
   else                                               
      // ..Else, get default system icon text font.
      SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&logfontIconText,0);
   
   // Create font.
   hfont = CreateFontIndirect(&logfontIconText);
   // Set the current font to the newly created small font.
   *phfontOld = (HFONT)SelectObject(hdc,(HGDIOBJ)hfont);
}                        
                        
//---------------------------------------------------------------------
// CleanUpDC():
//    This function resets a folder device context to its original 
// state before "SetUpDC()" was called.  It is needs as parameters
// the values returned by "SetUpDC()".
//---------------------------------------------------------------------
void WPFOLDER::CleanUpDC(HDC hdc,HFONT hfontOld,int iPrevBk,int iPrevMap)
{                                               
   HFONT hfont;

   // Clean-up device context font objects.
   hfont = (HFONT)SelectObject(hdc,(HGDIOBJ)hfontOld);   // Restore original font.
   ::DeleteObject(hfont);                                // Destroy font created.
   // Restore the original background drawing mode.
   SetBkMode(hdc,iPrevBk);
   // Restore the original mapping mode.
   SetMapMode(hdc,iPrevMap);
}

//---------------------------------------------------------------------
// FUNCTIONS TO PAINT AND RELOAD IMAGE BITMAPS
//---------------------------------------------------------------------
   
//---------------------------------------------------------------------
// PaintFolderImage():
//---------------------------------------------------------------------
void WPFOLDER::PaintFolderImage(HDC hdc)
{
   HDC      hdcMemory;
   BITMAP   bmImageInfo;
   HBITMAP  hbmpOld;
   RECT     rcClientArea;
   HPALETTE hpalOld,hpalOldMem;
   int      x,y;
                        
   // Check if memory pointer is not valid...
   if (GetImageBitmapHandle()==NULL)
      return;
      
   // Create memory device context.                        
   hdcMemory = CreateCompatibleDC(hdc);
   // Check if memory device-context was created successfully...
   if (hdcMemory==NULL)
      return;

   if (GetImagePalette()!=NULL)
   {
      hpalOldMem = SelectPalette(hdcMemory,GetImagePalette(),FALSE);
      hpalOld    = SelectPalette(hdc,GetImagePalette(),FALSE);
      // Assume the palette's already been realized (no need to call RealizePalette().  
      // It should have been realized in our WM_QUERYNEWPALETTE or WM_PALETTECHANGED messages...
   }
      
   // Select image bitmap into memory device-context.
   hbmpOld = (HBITMAP)SelectObject(hdcMemory,GetImageBitmapHandle());
   
   // Setup stretch blit mode tobe color-on-color.
   SetStretchBltMode(hdc,COLORONCOLOR);
   
   // Query the current client area rectangle.
   GetClientRect(GetHandle(),(LPRECT)&rcClientArea);

   // Get bitmap object image information.
   ::GetObject(GetImageBitmapHandle(),sizeof(BITMAP),&bmImageInfo);
        
   // Depending on the operation to do...             
   if (GetImageAttribute(IMAGE_NORMAL)==TRUE)
   {
      // Calculate center coordinates for bitmap on client area.
      x = max(((rcClientArea.right - bmImageInfo.bmWidth)/2),0);
      y = max(((rcClientArea.bottom - bmImageInfo.bmHeight)/2),0);
      // Paint bitmap in its normal size on the center of window.
      BitBlt(hdc,x,y,bmImageInfo.bmWidth,bmImageInfo.bmHeight,hdcMemory,0,0,SRCCOPY);
   } 
   else
   if (GetImageAttribute(IMAGE_TILED)==TRUE)
   {
      // Tile bitmap horizontally and vertically accross client area.    
      for (x=0; x < rcClientArea.right; x += bmImageInfo.bmWidth)
      {  
         for (y=0; y < rcClientArea.bottom; y += bmImageInfo.bmHeight)
            BitBlt(hdc,x,y,bmImageInfo.bmWidth,bmImageInfo.bmHeight,hdcMemory,0,0,SRCCOPY);
      }
   }        
   else
   if (GetImageAttribute(IMAGE_SCALED)==TRUE)
   {
      // Get dimensions of window and stretch to fit.
      GetClientRect(GetHandle(), (LPRECT)&rcClientArea);
      StretchBlt(hdc,0,0,rcClientArea.right,rcClientArea.bottom,hdcMemory,0,0,bmImageInfo.bmWidth,bmImageInfo.bmHeight,SRCCOPY);
   }
   
   // Clean-up before leaving function.
   // Select back the original bitmap into memory device-context.               
   SelectObject(hdcMemory,hbmpOld);
   // If we selected a custom palette, then...
   if (GetImagePalette()!=NULL)
   {                                   
      // Select original palettes into device-contexts   
      SelectPalette (hdcMemory,hpalOldMem,FALSE);
      SelectPalette (hdc,hpalOld,FALSE);
   }
   // Delete memory device-context.
   DeleteDC(hdcMemory);
}


//---------------------------------------------------------------------
// ReloadFolderImage():
//---------------------------------------------------------------------
void WPFOLDER::ReloadFolderImage(void)
{        
   HPBYTE   lpDIBMem;

   // Check if folder is NOT the desktop AND it is currently open, then...
   if ((GetType()!=OBJTYPE_DESKTOP)&&(isOpen()==TRUE))
   {                                         
      // First, destroy original image bitmap (if any).
      ::DeleteObject(GetImageBitmapHandle());           
      // Clear image bitmap handle.
      SetImageBitmapHandle(NULL);
      // Second, destroy original logical palette (if any).
      ::DeleteObject(GetImagePalette());           
      // Clear image palette handle.
      SetImagePalette(NULL);
      // Check if folder has an image as background...
      if (GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)
      {
         // Load new image bitmap into memory.
         lpDIBMem = LoadImage(GetImagePath());
         // Create logical palette from DIB information.
         SetImagePalette(CreatePaletteFromDIB(lpDIBMem));
         // Convert DIB image to regular bitmap object (store bitmap handle internally).
         SetImageBitmapHandle(ConvertDIBToDDB(lpDIBMem,GetImagePalette()));
         // Destroy memory image of bitmap.
         DestroyImage(lpDIBMem);
      }
   }
}


//---------------------------------------------------------------------
// FUNCTIONS TO CALCULATE FOLDER'S LOGICAL DOMAIN
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CalculateLogicalDomainRect():
//    This function is used to calculate and return a rectangle which 
// defines the "complete" logical domain of a folder's coordinates.
// This means that all children objects of this folder are encapsulated
// within this rectangle of logical coordinates.  This information is
// helpful when figuring out the visibility of a folder's objects and
// what the current range of scrollbars should be (if any).
//---------------------------------------------------------------------
void WPFOLDER::CalculateLogicalDomainRect(LPRECT lprcLogDomain)
{
   int  index;
   RECT rcObj;

   // Initialize logical domain to be the 1-pel wide rectangle (0,0)-(1,1).
   SetRect(lprcLogDomain,0,0,1,1);
   // Loop though all objects contained in this folder.
   for (index=0; index<GetObjectCount(); index++)
   {
      // Get current object's rectangle in logical space.
      GetObject(index)->GetObjectRect(&rcObj);
      // Add object's rectangle to the total logical domain rect.
      UnionRect(lprcLogDomain,lprcLogDomain,&rcObj);
   }
}

//---------------------------------------------------------------------
// FUNCTIONS USED BY FindObjectAtRelativePosition() FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// IconViewFindObject():
//    This function finds the next object in the folder's "Icon View" 
// which is positioned relative to the "wEventID" event, given
// a current object.
//    "wEventID" can be ID_EVENT_UP, ID_EVENT_DOWN, ID_EVENT_LEFT, 
// ID_EVENT_RIGHT.  The algorithm used is pretty much a "brute" 
// force algorithm, but the advantage is that it needs minimal starting
// information about the folder to find the matching object (if any).  
//    This function is ONLY valid for "Icon View" searches.
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::IconViewFindObject(WORD wEventID,WPOBJECT *pwpobjCurrent)
{         
   WPOBJECT *pwpobjRelative;
   POINT    ptFrom,ptWork,ptDiff;
   int      index,nSmallest;
      
   // Intialize found object variable.
   pwpobjRelative = NULL;  
   // Initialize smallest difference found variable to the largest positive integer.
   nSmallest = (int)0x7FFF;
   // Get relative point.
   ptFrom  = pwpobjCurrent->GetIconPos();
   // Search all objects in folder for best match...
   for (index=0; index<GetObjectCount(); index++)
   {
      // Get object's current icon position.
      ptWork = GetObject(index)->GetIconPos();
            
      // Calculate absolute distance in y-direction.
      ptDiff.y = ptWork.y-ptFrom.y;
      if (ptDiff.y<0) ptDiff.y = -ptDiff.y;
                                                       
      // Calculate absolute distance in x-direction.
      ptDiff.x = ptWork.x-ptFrom.x;
      if (ptDiff.x<0) ptDiff.x = -ptDiff.x;

      // Depending on which direction we want...
      switch (wEventID)
      {                
         case ID_EVENT_UP:
            // If object meets the basic criteria of being ABOVE the
            // original object, AND object is within error zone...
            if ((ptWork.y < ptFrom.y)&&(ptDiff.x<=ERROR_ZONE))
            {
               // If distance between original object and current 
               // object is less than current smallest, OR this is the
               // first object that meets the basic criteria...
               if ((ptDiff.y < nSmallest)||(pwpobjRelative==NULL))
               {                       
                  // ...Then, we have found a match candidate, store all
                  // the needed information and continue with search.
                  nSmallest = ptDiff.y;
                  pwpobjRelative = GetObject(index);
               }
            }
            break;
            
         case ID_EVENT_DOWN:
            // If object meets the basic criteria of being BELOW the
            // original object, AND object is within error zone...
            if ((ptWork.y > ptFrom.y)&&(ptDiff.x<=ERROR_ZONE))
            {
               // If distance between original object and current 
               // object is less than current smallest, OR this is the
               // first object that meets the basic criteria...
               if ((ptDiff.y < nSmallest)||(pwpobjRelative==NULL))
               {
                  // ...Then, we have found a match candidate, store all
                  // the needed information and continue with search.
                  nSmallest = ptDiff.y;
                  pwpobjRelative = GetObject(index);
               }
            }
            break;

         case ID_EVENT_LEFT:
            // If object meets the basic criteria of being LEFT of the
            // original object, AND object is within error zone...
            if ((ptWork.x < ptFrom.x)&&(ptDiff.y<=ERROR_ZONE))
            {
               // If distance between original object and current 
               // object is less than current smallest, OR this is the
               // first object that meets the basic criteria...
               if ((ptDiff.x < nSmallest)||(pwpobjRelative==NULL))
               {
                  // ...Then, we have found a match candidate, store all
                  // the needed information and continue with search.
                  nSmallest = ptDiff.x;
                  pwpobjRelative = GetObject(index);
               }
            }
            break;
            
         case ID_EVENT_RIGHT:
            // If object meets the basic criteria of being RIGHT of the
            // original object, AND object is within error zone...
            if ((ptWork.x > ptFrom.x)&&(ptDiff.y<=ERROR_ZONE))
            {
               // If distance between original object and current 
               // object is less than current smallest, OR this is the
               // first object that meets the basic criteria...
               if ((ptDiff.x < nSmallest)||(pwpobjRelative==NULL))
               {  
                  // ...Then, we have found a match candidate, store all
                  // the needed information and continue with search.
                  nSmallest = ptDiff.x;
                  pwpobjRelative = GetObject(index);
               }
            }
            break;
      }
   }   
   
   // Return object found to be at relative position.   
   return (pwpobjRelative);
}      
      
//---------------------------------------------------------------------
// DetailsViewFindObject():
//    This function finds the next object within the folder's "Details 
// View" which is positioned relative to the "wEventID" event, given
// a current object.
//    "wEventID" can be ID_EVENT_UP, ID_EVENT_DOWN, ID_EVENT_LEFT, 
// ID_EVENT_RIGHT.  
//    This function is ONLY valid for "Details View" searches.
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::DetailsViewFindObject(WORD wEventID,WPOBJECT *pwpobjCurrent)
{              
   WPOBJECT *pwpobjRelative;
   int      index;
   
   // Initialize object index to return "no object".
   index = -1;
   // Depending on which direction we want...
   switch (wEventID)
   {  
      case ID_EVENT_UP:
         // Get the previous object in folder (if any).
         index = max((GetObjectIndex(pwpobjCurrent)-1),0);
         break;

      case ID_EVENT_DOWN:
         // Get the next object in folder (if any).
         index = min((GetObjectIndex(pwpobjCurrent)+1),(GetObjectCount()-1));
         break;
            
      case ID_EVENT_LEFT:           
         // Move window visible area to the left.
         PostMessage(GetHandle(),WM_HSCROLL,SB_LINEUP,0L);
         break;          
            
      case ID_EVENT_RIGHT:                       
         // Move window visible area to the right.
         PostMessage(GetHandle(),WM_HSCROLL,SB_LINEDOWN,0L);
         break;
   }
   // Get new object at relative position from current object.
   pwpobjRelative = ((index<0)? NULL : GetObject(index));
   // Object found to be at relative position from object passed.
   return (pwpobjRelative);
}
                        
//---------------------------------------------------------------------
// FindFunction():
//---------------------------------------------------------------------
BOOL WPFOLDER::FindFunction(void)
{
   DLGPROC  lpfnFindDlgProc;
   BOOL     fReturnCode;
   
   // Make a procedure instance for the "Find..." dialog procedure.
   lpfnFindDlgProc = (DLGPROC)MakeProcInstance((FARPROC)FindDlgProc,g_wpEnvironment.hInst);
   // Create and display the "Find..." object dialog.
   fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_FIND),g_wpEnvironment.hwndDesktop,lpfnFindDlgProc,(LPARAM)(LPVOID)this);
   // Free the procedure instance created for dialog.
   FreeProcInstance((FARPROC)lpfnFindDlgProc);
   
   // Return dialog box's return code.
   return (fReturnCode);
}

                        
//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FOLDER WINDOWS 3.1 CLASS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetClassBackgroundBrush():
//    This function creates a brush of the specified background color
// and returns the brush's handle through the parameter variable.
//---------------------------------------------------------------------
void WPFOLDER::SetClassBackgroundBrush(HBRUSH *phbrBackground)
{
   // Create custom-color brush for window class background.
   *phbrBackground = CreateSolidBrush(m_clrrefBackground);
}


//---------------------------------------------------------------------
// RegisterFolderClass():
//    This function is a key function in creating folder objects 
// open windows.  It creates a window class with the unique name 
// returned by the "CreateUniqueClassName()" function.  It then
// extracts a copy of the folder's icon and initializes the window 
// class structure.  It finally registers the window class with the
// Windows 3.1 USER module.
//---------------------------------------------------------------------
char *WPFOLDER::RegisterFolderClass(WNDPROC lpfnFolderWndProc)
{
   WNDCLASS  wc;
   HICON     hIcon;
   char      *pszClassName;

   // Create unique class name string.
   pszClassName = (char *)CreateUniqueClassName();

   // Load a new instance of icon resource and return an icon handle.
   hIcon = ExtractIcon();

   // Fill in window class structure with parameters that describe the
   // main window.
   wc.style         = (CS_BYTEALIGNWINDOW | CS_DBLCLKS);
   wc.lpfnWndProc   = lpfnFolderWndProc;                      // Function to retrieve messages for this class.
   wc.cbClsExtra    = 0;                                      // No per-class extra data.
   wc.cbWndExtra    = sizeof(LONG);                           // pointer to per-window extra data.
   wc.hInstance     = g_wpEnvironment.hInst;                  // Application that owns the class.
   wc.hIcon         = hIcon;
   wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
   SetClassBackgroundBrush(&(wc.hbrBackground));
   wc.lpszMenuName  = NULL;
   wc.lpszClassName = pszClassName;                           // Name used in call to CreateWindow.

   // Register the window class and return success/failure code.
   if (RegisterClass(&wc)!=0)
      return (pszClassName);     // Return success.
   else
      return (NULL);             // Return failure.
}


//---------------------------------------------------------------------
// UnregisterFolderClass():
//    This function is the counterpart to "RegisterFolderClass()".
// It retrieves the class name, converts it to a class ID which is 
// posted with a message to the Desktop object.  The code that handles
// the WM_WPS_UNREGISTERCLASS message does the actual Windows API calls
// to unregister the custom folder class.
//---------------------------------------------------------------------
void WPFOLDER::UnregisterFolderClass(void)
{
   WORD  wClassID;
   char  szClassName[MAX_CLASS_NAME+1];

   // Unregister class used for this folder.
   GetClassName(GetHandle(),szClassName,MAX_CLASS_NAME);

   // Get class ID from string.
   sscanf(szClassName,"WPS%u",&wClassID);

   // Tell desktop window to unregister class for us.
   PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_UNREGISTERCLASS,wClassID,0L);
}



//---------------------------------------------------------------------
// DRAG & DROP TARGET MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDropTarget():
//    This function finds a drop target object (if any) given a deivce
// point and a boolean flag specifing if the drop if occurring inside
// a folder's area or not.  If no objects are currently under specified
// point, it returns "NULL".  
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::GetDropTarget(POINT ptDevice,BOOL *fTargetIsFolderArea)
{
   HWND     hwndDrop;
   WPFOLDER *pwpfolderDrop;
   WPOBJECT *pwpobjTarget;

   // Get window from window point.
   hwndDrop = WindowFromPoint(ptDevice);
   // Check if object drop was done on top of a window.
   if (hwndDrop!=NULL)
   {
      // Check if window is a Workplace Shell folder window.
      if (isWorkplaceShellWindow(hwndDrop)==TRUE)
      {
         // Get folder window's pointer to its folder object.
         pwpfolderDrop = (WPFOLDER *)SendMessage(hwndDrop,WM_WPS_QUERY_FOLDER_POINTER,0,0L);
         // Check that pointer is valid.
         if (pwpfolderDrop!=NULL)
         {
            // Get target object within drop folder.
            pwpobjTarget = pwpfolderDrop->ChildHitTest(ptDevice);
            // If no child objects are being hit by point,...
            if (pwpobjTarget==NULL)
            {
               // Set target object as the drop folder.
               pwpobjTarget          = pwpfolderDrop;
               *fTargetIsFolderArea = TRUE;
            }
            else
               *fTargetIsFolderArea = FALSE;

            // Return pointer to target object.
            return (pwpobjTarget);
         }
         else
            return (NULL); // Workplace Shell window did not return valid folder pointer.
      }
      else
         return (NULL);    // Windows 3.1 window is not a Workplace Shell window.
   }
   else
      return (NULL);       // Mouse point is not on a valid Windows 3.1 window.
}


//---------------------------------------------------------------------
// isTargetValid():
//    This function tests if a given target drop object or the 
// currently selected objects in a this folder are a valid target for 
// the current drag & drop function.   This function will return 
// TRUE if target(s) are valid for current operation and source objects.
// Otherwise, it returns FALSE.
//---------------------------------------------------------------------
BOOL WPFOLDER::isTargetValid(WPOBJECT *pwpobjTarget)
{
   int      index;
   WPOBJECT *pwpobjSource;

   // Check if there is no target to check.
   if (pwpobjTarget==NULL)
      return (FALSE);

   // Check if we are draging selected object(s) or a single non-selected object.
   if (m_fDragSelectedObjs==TRUE)
   {
      // Test all drop sources into the drop target and vice versa.
      for (index=0; index<GetSelectedObjectCount(); index++)
      {
         // Get pointer to selected object.
         pwpobjSource = GetSelectedObject(index);
         // Test if source object supports target object and vice versa.
         if (pwpobjSource->TestDropSource(pwpobjTarget,m_wDragFunction)==FALSE)
            return (FALSE);
      }
   }
   else
   {
      // Test single drag source into the drop target.
      // Test if source object supports target object and vice versa.
      if (m_pwpobjDrag->TestDropSource(pwpobjTarget,m_wDragFunction)==FALSE)
         return (FALSE);
   }

   // All source(s) support target object, and target object supports all source(s).
   // Therefore, we can return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// DrawShadowDragLine():
//    This function is in charge of drawing the connecting line between 
// drag & drop source object and the mouse cursor which represents a
// "shadow" object is being created.
//---------------------------------------------------------------------
void WPFOLDER::DrawShadowDragLine(POINT ptDevice,BOOL fRemoveLineOnly)
{
   HDC      hdcScreen;
   HPEN     hpenOld;
   POINT    ptLine[2];
   int      rop2Old;

   // Get the screen device context.
   hdcScreen = ::GetDC(NULL);
   // Set pen raster operation to be pen XOR w/screen.
   rop2Old = SetROP2(hdcScreen,R2_XORPEN);
   hpenOld = (HPEN)SelectObject(hdcScreen,GetStockObject(WHITE_PEN));

   // Check if shadow line is already drawn, and needs to be erased.
   if (m_fDragLineDrawn==TRUE)
   {
      // Set current line's coordinates.
      ptLine[0] = m_ptDragLineStartPoint;
      ptLine[1] = m_ptDragLineEndPoint;
      // XOR out current shadow drag line.
      Polyline(hdcScreen,&(ptLine[0]),2);
      // Set line drawn flag to false.
      m_fDragLineDrawn = FALSE;
   }

   // Check if we only wanted to remove current line without drawing new one.
   if (fRemoveLineOnly==FALSE)
   {
      // Store new shadow drag line end point.
      m_ptDragLineEndPoint = ptDevice;

      // Set new shadow drag line's coordinates.
      ptLine[0] = m_ptDragLineStartPoint;
      ptLine[1] = m_ptDragLineEndPoint;
      // XOR new shadow drag line into screen.
      Polyline(hdcScreen,&(ptLine[0]),2);
      // Set line drawn flag to true.
      m_fDragLineDrawn = TRUE;
   }

   // Clean-up before returning.
   SelectObject(hdcScreen,hpenOld);
   SetROP2(hdcScreen,rop2Old);
   ::ReleaseDC(NULL,hdcScreen);
}


//---------------------------------------------------------------------
// DrawRubberBandRect():
//    This function is in charge of drawing the "rubber-band" rectangle 
// used to select multiple objects from a folder object.  It is drawn
// using the XOR operation so that it can be drawn & erased quickly.
//---------------------------------------------------------------------
void WPFOLDER::DrawRubberBandRect(POINT ptDevice,BOOL fRemoveLineOnly)
{
   HDC      hdc;
   HPEN     hpenOld;
   HBRUSH   hbrushOld;
   int      rop2Old;

   // Change from device coordinates to logical (folder-relative) coordinates.
   // (Note: Must do it before GetDC() call since DPtoLP() uses GetDC() itself).
   DPtoLP(&ptDevice);
    // Get the folder's device context.
   hdc = GetDC();
   // Set pen raster operation to be pen XOR w/screen.
   rop2Old = SetROP2(hdc,R2_XORPEN);
   hpenOld = (HPEN)SelectObject(hdc,GetStockObject(WHITE_PEN));
   hbrushOld = (HBRUSH)SelectObject(hdc,GetStockObject(NULL_BRUSH));

   // Check if rubber-band rectangle is already drawn, and needs to be erased.
   if (m_fDragLineDrawn==TRUE)
   {
      // XOR out current rubber-band rectangle drag line.
      Rectangle(hdc,
                m_ptDragLineStartPoint.x,m_ptDragLineStartPoint.y,
                m_ptDragLineEndPoint.x,m_ptDragLineEndPoint.y);
      // Set line drawn flag to false.
      m_fDragLineDrawn = FALSE;
   }

   // Check if we only wanted to remove current line without drawing new one.
   if (fRemoveLineOnly==FALSE)
   {                         
      // Store new shadow drag line end point.
      m_ptDragLineEndPoint = ptDevice;
      // Draw current rubber-band rectangle drag line (using XOR operation).
      Rectangle(hdc,
                m_ptDragLineStartPoint.x,m_ptDragLineStartPoint.y,
                m_ptDragLineEndPoint.x,m_ptDragLineEndPoint.y);
      // Set line drawn flag to true.
      m_fDragLineDrawn = TRUE;
   }

   // Clean-up before returning.
   SelectObject(hdc,hbrushOld);
   SelectObject(hdc,hpenOld);
   SetROP2(hdc,rop2Old);
   ReleaseDC();
}


//---------------------------------------------------------------------
// MOUSE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnLeftMouseButton():
//---------------------------------------------------------------------
void WPFOLDER::OnLeftMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice)
{
   WPOBJECT *pwpobjHit;

   // Process left mouse button message.
   switch (uMsg)
   {
      case WM_LBUTTONDOWN:
         m_wLeftButtonState = MBS_DOWN;
         if ((wFlags&MK_CONTROL)&&(wFlags&MK_SHIFT))
         {
            // Left mouse button down while holding the CTRL-SHIFT keys down.
            // Function: select objects.
            m_wDragFunction = DRAG_SELECT;
         }
         else
         if ((wFlags&MK_CONTROL)&&(!(wFlags&MK_SHIFT)))
         {
            // Left mouse button down while holding the CTRL key down.
            // Function: select objects.
            m_wDragFunction = DRAG_SELECT;
         }
         else
         {
            // Check for left & right mouse down (task list).
            if (m_wRightButtonState==MBS_DOWN)
            {
               // So that right mouse button won't pull up task list too do this:
               m_wLeftButtonState  = MBS_UP; // Reset left mouse button.
               m_wRightButtonState = MBS_UP; // Reset right mouse button.
               // Pull-up task list.
               PostMessage(GetHandle(),WM_SYSCOMMAND,SC_TASKLIST,0L);
               break;
            }
            else
               // No control or alt keys pressed, just left mouse click.
               // Function: select objects.
               m_wDragFunction = DRAG_SELECT;
         }

         // Remove select focus from currently selected objects in folder (if any).
         ClearSelectedObjects(TRUE);

         // Check for child object "hit" by left mouse click.
         pwpobjHit = ChildHitTest(ptDevice);
         // If a child object has been hit.
         if (pwpobjHit!=NULL)
         {
            // Assign hit object as first selected object in folder.
            AddSelectedObject(pwpobjHit);
            // Update appearance of newly selected object.
            pwpobjHit->InvalidateIcon(FALSE,TRUE);
         }
         else
         {  
            // Start rubber-band mode for object selection.
            m_wDragFunction = DRAG_SELECT_RUBBERBAND;
            // Reset rubber-band line drawn flag equal to false.
            m_fDragLineDrawn = FALSE;
            // Store current mouse coordinates as start point for rubber-banding.
            m_ptDragLineStartPoint = ptDevice;
            // Convert from device coordinates to logical (folder-relative) coordinates.
            DPtoLP(&m_ptDragLineStartPoint);
         }

         // Update folder's appearance right now.
         UpdateWindow();
         break;

      case WM_LBUTTONUP:  
         // If drag function was rubber-banding selection...
         if (m_wDragFunction==DRAG_SELECT_RUBBERBAND)
            // Erase rubber-band rectangle used for object selection.
            DrawRubberBandRect(ptDevice,TRUE);             
            
         // Reset left mouse button.
         m_fDragMode = FALSE;          // Reset drag mode.
         m_wLeftButtonState = MBS_UP;  // Reset left mouse button state.
         // Release mouse capture.
         ReleaseCapture();
         break;
   }
}

//---------------------------------------------------------------------
// OnRightMouseButton():
//---------------------------------------------------------------------
void WPFOLDER::OnRightMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice)
{
   // Process right mouse button message.
   switch (uMsg)
   {
      case WM_RBUTTONDOWN:
         // Set internal right mouse button state to "down".
         m_wRightButtonState = MBS_DOWN;
         // Store first-click coordinates for later use by OnMouseMove() funct.
         m_ptMouseFirstClick = ptDevice;
         // Check if Ctrl+Shift keys, Shift key, or no keys were also down...
         if ((wFlags&MK_CONTROL)&&(wFlags&MK_SHIFT))
         {
            // Right mouse button down while holding the CTRL+SHIFT keys down.
            // Function: object(s) are being shadowed.
            m_wDragFunction = DRAG_SHADOW;
            // Reset shadow line drawn flag equal to false.
            m_fDragLineDrawn = FALSE;
            // Store current mouse coordinates as start point for shadow.
            m_ptDragLineStartPoint = ptDevice;
         }
         else
         if ((wFlags&MK_CONTROL)&&(!(wFlags&MK_SHIFT)))
         {
            // Right mouse button down while holding the CTRL key down.
            // Function: object(s) are being copied.
            m_wDragFunction = DRAG_COPY;
         }
         else
         {
            // Check for left & right mouse down (task list).
            if ((m_wLeftButtonState==MBS_DOWN)&&(wFlags&MK_LBUTTON))
            {
               // So that left mouse button won't pull up task list too.
               m_wLeftButtonState = MBS_UP;  // reset left mouse button.
               m_wRightButtonState = MBS_UP; // reset right mouse button.
               // Pull-up task list.
               PostMessage(GetHandle(),WM_SYSCOMMAND,SC_TASKLIST,0L);
            }
            else
            if (!(wFlags&MK_LBUTTON))
            {
               // No special dragging, we just want to move the object.
               // Function: object(s) are being moved.
               m_wDragFunction = DRAG_MOVE;
            }
         }
         break;

      case WM_RBUTTONUP:
         // Check if task list was not pulled up already as a consequence of 
         // the mouse clicks AND that we are not in drag mode.
         if ((m_fDragMode==FALSE)&&(m_wLeftButtonState==MBS_UP)&&(m_wRightButtonState==MBS_DOWN))
            PostMessage(GetHandle(),WM_WPS_POPUP_MENU,0,MAKELPARAM(ptDevice.x,ptDevice.y));
         else
         // Check if we are ending Drag & Drop mode.
         if ((m_fDragMode==TRUE)&&(m_wDragFunction!=DRAG_SELECT)&&(m_wRightButtonState==MBS_DOWN))
         {
            m_fDragMode = FALSE;    // end Drag & Drop mode.
            PostMessage(GetHandle(),WM_WPS_DROP,0,MAKELPARAM(ptDevice.x,ptDevice.y));
         }

         // Reset right mouse button.
         m_wRightButtonState = MBS_UP;
         break;
   }
}
         

//---------------------------------------------------------------------
// HandleRubberBandSelection():
//---------------------------------------------------------------------
void WPFOLDER::HandleRubberBandSelection(POINT ptDevice)
{
   RECT     rcRubberBand;
   LIST     listSelect;
   int      index;
   
   // Draw rubber-band rectangle for object selection.
   DrawRubberBandRect(ptDevice,TRUE);             
   // Create the rubber band rectangle. 
   if (m_ptDragLineStartPoint.x < m_ptDragLineEndPoint.x)
   {
      rcRubberBand.left  = m_ptDragLineStartPoint.x;
      rcRubberBand.right = m_ptDragLineEndPoint.x;
   }
   else
   {
      rcRubberBand.left  = m_ptDragLineEndPoint.x;
      rcRubberBand.right = m_ptDragLineStartPoint.x;
   }
   
   if (m_ptDragLineStartPoint.y < m_ptDragLineEndPoint.y)
   {
      rcRubberBand.top    = m_ptDragLineStartPoint.y;
      rcRubberBand.bottom = m_ptDragLineEndPoint.y;
   }
   else
   {
      rcRubberBand.top    = m_ptDragLineEndPoint.y;
      rcRubberBand.bottom = m_ptDragLineStartPoint.y;
   }
            
   // Check how many objects are included in this area.
   GetChildrenContainedInRect(rcRubberBand,&listSelect);
   // Clear out selected objects list.
   ClearSelectedObjects(TRUE);
   // Add new objects to selected objects list.
   for (index=0; index<listSelect.GetCount(); index++)
   {                                                       
      // Add object to selected objects list.
      AddSelectedObject((WPOBJECT *)listSelect.GetAt(index));
      // Invalidate object so that it can be redrawn again.
      ((WPOBJECT *)listSelect.GetAt(index))->InvalidateIcon(FALSE,TRUE);
   }
   // Clear out temporary list.
   listSelect.RemoveAll();
   // Update newly selected/unselected objects.
   UpdateWindow();
   // Draw rubber-band rectangle for object selection.
   DrawRubberBandRect(ptDevice,FALSE);             
}


//---------------------------------------------------------------------
// HandleDragModeInit():
//---------------------------------------------------------------------
BOOL WPFOLDER::HandleDragModeInit(WORD wFlags,POINT ptDevice)
{
   WPOBJECT *pwpobjHit,*pwpobjTarget;
   BOOL     fTargetIsFolderArea;
   RECT     rcUserError;

   // We are not on drag & drop mode yet...
   // If right button is down, select object(s) for Drag & Drop, and start it.
   if ((m_wRightButtonState==MBS_DOWN)&&(wFlags&MK_RBUTTON))
   {
      // Calculate user rect. area where we assume mouse movement error.
      // The error area is a 3x3 rectangle. PtInRect() does not include
      // the right and bottom coord. in the check. therefore, we add an
      // extra 1 pel to the right and bottom rect. coordinates.
      SetRect(&rcUserError,m_ptMouseFirstClick.x-2,m_ptMouseFirstClick.y-2,
                           m_ptMouseFirstClick.x+3,m_ptMouseFirstClick.y+3);
      
      // Check if we are still within this error area...
      if (PtInRect(&rcUserError,ptDevice)==TRUE)
         // Return failure which means 'stop processing'.
         return (FALSE); // ...ignore move message and exit function.
      
      // Get child object selected for Drag & Drop.
      pwpobjHit = ChildHitTest(ptDevice);
      
      // If no objects were selected, exit.
      if (pwpobjHit==NULL)
         // Return failure which means 'stop processing'.
         return (FALSE);
      
      // If object hit was not in list of currently selected objects...
      if (isObjSelected(pwpobjHit)==FALSE)
      {
         // Make object being dragged the object "hit".
         m_fDragSelectedObjs  = FALSE;
         m_pwpobjDrag         = pwpobjHit;
      }
      else
      {
         // Make object(s) being dragged the selected objects.
         m_fDragSelectedObjs  = TRUE;
         // Store object "hit" by right mouse button.
         m_pwpobjDrag         = pwpobjHit;
      }
      
      // Initialize drag flags.
      m_fDragMode        = TRUE;
      m_pwpobjDropTarget = NULL;
      
      // Depending on which view mode this folder is in...
      switch (GetOpenView())
      {
         case VIEW_ICON:
            // Convert icon position of object hit from logical to device coord.
            m_ptDragLineStartPoint = m_pwpobjDrag->GetIconPos();
            LPtoDP(&m_ptDragLineStartPoint);
            // Store the center coordinates of object hit as start point for shadow.
            m_ptDragLineStartPoint.x += (GetSystemMetrics(SM_CXICON)/2)-1;
            m_ptDragLineStartPoint.y += (GetSystemMetrics(SM_CYICON)/2)-1;
            break;
      
         case VIEW_DETAILS:
            // Store mouse coordinate as the starting point for shadow drag line.
            m_ptDragLineStartPoint = ptDevice;
            break;
      }
      
      // Start Drag & Drop mode by capturing the mouse movement.
      SetCapture(GetHandle());
      // Create cursors for mouse movement state.
      CreateDragCursors(m_wDragFunction,
            &(g_wpEnvironment.hDragCursor),
            &(g_wpEnvironment.hNoDragCursor));
      // Save original cursor handle.
      g_wpEnvironment.hOriginalCursor  = GetCursor();
      // Set current mouse cursor to the appropriate drag cursor.
      OnMouseMove(wFlags,ptDevice);
      // Play system event sound for "Begin Drag" event.
      PlaySystemEventSound("WPBeginDrag");
   }
   else
   // If left button is down, select objects.
   if ((m_wLeftButtonState==MBS_DOWN)&&(wFlags&MK_LBUTTON)&&
       ((m_wDragFunction==DRAG_SELECT)||(m_wDragFunction==DRAG_SELECT_RUBBERBAND)))
   {
      // Set drag mode flag (DRAG_SELECT or DRAG_SELECT_RUBBERBAND function).
      m_fDragMode = TRUE;
      // Start selection mode by capturing the mouse movement.
      SetCapture(GetHandle());
      // Set current mouse cursor to the appropriate drag cursor.
      OnMouseMove(wFlags,ptDevice);
   }

   // Return success which means 'continue processing'.
   return (TRUE);
}         
         

//---------------------------------------------------------------------
// OnMouseMove():
//---------------------------------------------------------------------
void WPFOLDER::OnMouseMove(WORD wFlags,POINT ptDevice)
{
   WPOBJECT *pwpobjTarget;
   BOOL     fTargetIsFolderArea;

   // Check for user moving mouse from non-client area into client area
   // with the left mouse button pressed down.
   if (m_fNCLButtonDown==TRUE)
   {
      // Reset left mouse button down flag.
      m_fNCLButtonDown = FALSE;
      // Restore visual state of min/max buttons to "up".
      SendMessage(GetHandle(),WM_NCMOUSEMOVE,HTMAXBUTTON,0L);
   }

   // If currently we are not in drag mode...
   if (m_fDragMode==FALSE)
   {  
      // Since we are not yet in drag mode, check if we need to be in drag mode.
      if (HandleDragModeInit(wFlags,ptDevice)==FALSE)
         return;
   }
   else
   {
      // We are in drag & drop mode already...
      // Check for type of drag & drop function being performed.
      if (m_wDragFunction==DRAG_SELECT)
      {     
         // Select objects within same folder only.
         pwpobjTarget = ChildHitTest(ptDevice);
         if (pwpobjTarget!=NULL)
         {
            // Set object area flag to be a child object.
            m_fTargetIsFolderArea = FALSE;

            // Check if object is already selected.
            if (isObjSelected(pwpobjTarget)==FALSE)
            {
               // Add object to list of selected objects.
               AddSelectedObject(pwpobjTarget);
               // Update visual state of object right now.
               pwpobjTarget->InvalidateIcon(TRUE,TRUE);
            }
         }
      }     
      else              
      // If drag function is rubber-band selection...
      if (m_wDragFunction==DRAG_SELECT_RUBBERBAND)
      {
         HandleRubberBandSelection(ptDevice);
      }
      else  // Else, if DRAG_MOVE, DRAG_COPY, or DRAG_SHADOW...
      {
         // We are in move, copy, or shadow Drag & Drop modes...
         // Get drop target object from mouse coordinates.
         pwpobjTarget = GetDropTarget(ptDevice,&fTargetIsFolderArea);

         // Check if new drop target is valid.
         if (pwpobjTarget!=NULL)
         {
            // If this target is not the same as the previous possible target...
            if ((pwpobjTarget!=m_pwpobjDropTarget)&&(m_pwpobjDropTarget!=NULL))
               // Unframe previous target.
               m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);

            // Store new possible target.
            m_pwpobjDropTarget    = pwpobjTarget;
            m_fTargetIsFolderArea = fTargetIsFolderArea;

            // Draw frame around possible target object if not selected already.
            m_pwpobjDropTarget->Frame(m_fTargetIsFolderArea);

            // Check if mouse is on top of a valid drop target.
            if (isTargetValid(m_pwpobjDropTarget)==TRUE)
               SetCursor(g_wpEnvironment.hDragCursor);
            else
               SetCursor(g_wpEnvironment.hNoDragCursor);
         }
         else
         {
            // If this target is not the same as the previous possible target...
            if (m_pwpobjDropTarget!=NULL)
               // Unframe previous target.
               m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);

            // No valid target selected.
            SetCursor(g_wpEnvironment.hNoDragCursor);
         }

         // If drag mode is DRAG_SHADOW, draw shadow connecting line.
         if (m_wDragFunction==DRAG_SHADOW)
            DrawShadowDragLine(ptDevice,FALSE);
      }
   }
}

//---------------------------------------------------------------------
// OnLeftMouseButtonDblClk():
//---------------------------------------------------------------------
void WPFOLDER::OnLeftMouseButtonDblClk(POINT ptDevice)
{
   WPOBJECT *pwpobjHit;

   // Get child object "hit" by left mouse double click.
   pwpobjHit = ChildHitTest(ptDevice);
   // Check if no child objects were "hit".
   if (pwpobjHit!=NULL)
      pwpobjHit->Open();   // "Open" child object hit.
   else
      MessageBeep(-1);     // Error beep, no objects hit.
}


//---------------------------------------------------------------------
// CURSOR CREATION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isRGBNormal():
//    This function tests a color reference value against the color 
// used as the transparent color in icons.
//---------------------------------------------------------------------
BOOL WPFOLDER::isRGBNormal(COLORREF clrrefPixel)
{
   if (clrrefPixel==RGB_ICON_TRANSPARENT_COLOR) return (FALSE);
   else                                         return (TRUE);
}


//---------------------------------------------------------------------
// isRGBWhite():
//    This function tests if the passed color reference will become a 
// white or black pixel in a monochrome cursor.
//---------------------------------------------------------------------
BOOL WPFOLDER::isRGBWhite(COLORREF clrrefPixel)
{
   if ((clrrefPixel==RGB(0xC0,0xC0,0xC0))||  // Light gray
       //(clrrefPixel==RGB(0x80,0x80,0x80))||  // Gray
       (clrrefPixel==RGB(0x00,0xFF,0x00))||  // Bright green
       (clrrefPixel==RGB(0xFF,0xFF,0x00))||  // Bright cyan
       (clrrefPixel==RGB(0xFF,0x00,0xFF))||  // Bright magenta
       (clrrefPixel==RGB(0xFF,0xFF,0xFF)))   // White
      return (TRUE);    // White.
   else
      return (FALSE);   // Black.
}

//---------------------------------------------------------------------
// CursorFromBitmap():
//---------------------------------------------------------------------
HCURSOR WPFOLDER::CursorFromBitmap(HBITMAP hbmCursor)
{
   HDC      hdc,hdcMem;
   HBITMAP  hbmOld;
   COLORREF clrrefPixel;
   BYTE     *pbXORMask,*pbANDMask,bBitMask;
   HCURSOR  hCursor;
   int      x,y,iCursorBytes,iByteOffset,nIconWidth,nIconHeight;

   // Create device context compatible with the display.
   hdc      = GetWindowDC(GetDesktopWindow());
   hdcMem   = CreateCompatibleDC(hdc);
   ::ReleaseDC(GetDesktopWindow(),hdc);
   // Select bitmap object on device context.
   hbmOld   = (HBITMAP)SelectObject(hdcMem,hbmCursor);
   
   // Get and store frequently used constants.
   nIconWidth  = GetSystemMetrics(SM_CXICON);
   nIconHeight = GetSystemMetrics(SM_CYICON);
   
   // Calculate number of bytes required to hold a cursor bitmap 1bpp
   iCursorBytes = (((nIconWidth/8)+1)*nIconHeight);
   // Make sure the bytes size is word (2 byte) aligned.
   iCursorBytes += (iCursorBytes%2);
   // Allocate space for the cursor masks buffers.
   pbXORMask = new BYTE [iCursorBytes];
   pbANDMask = new BYTE [iCursorBytes];
   // Initialize all bits in masks to zeros.
   memset(pbXORMask,0,iCursorBytes);
   memset(pbANDMask,0,iCursorBytes);

   // Loop through all pixels in memory device context and create cursor masks.
   for (y=0; y<nIconHeight; y++)
   {
      for (x=0; x<nIconWidth; x++)
      {
         // Look at current pixel.
         clrrefPixel = GetPixel(hdcMem,x,y);

         // Check if bit is normal or transparent.
         iByteOffset = (y*(nIconHeight/8))+(x/8);
         bBitMask    = (BYTE)(0x80 >> (x%8));
         if (isRGBNormal(clrrefPixel)==TRUE)
         {
            // Pixel is either black or white.
            if (isRGBWhite(clrrefPixel)==TRUE)
               pbXORMask[iByteOffset] |= bBitMask;
         }
         else
         {
            // Set bit to transparent.
            pbANDMask[iByteOffset] |= bBitMask;
         }
      }
   }

   // Create cursor from the XOR and AND bitmap bits.
   hCursor = CreateCursor(g_wpEnvironment.hInst,
                          (nIconWidth/2)-1,    // hotspot x.
                          (nIconHeight/2)-1,   // hotspot y.
                          nIconWidth,          // cursor width.
                          nIconHeight,         // cursor height.
                          pbANDMask,pbXORMask);                // cursor data.

   // Clean-up temporary objects and device contexts.
   delete pbXORMask;
   delete pbANDMask;

   // Restore original bitmap object into memory device context.
   SelectObject(hdcMem,hbmOld);
   // Destroy memory device context.
   DeleteDC(hdcMem);

   // Return handle to newly created mouse cursor.
   return (hCursor);
}

//---------------------------------------------------------------------
// CreateCursorBitmap():
//---------------------------------------------------------------------
HBITMAP WPFOLDER::CreateCursorBitmap(void)
{
   HDC      hdc,hdcMem,hdcMem2,hdcMask;
   RECT     rcBitmap;
   HBITMAP  hbmCursor,hbmCursor2,hbmMask,hbmOld,hbmOld2,hbmOldMask;
   HBRUSH   hbrBackground;
   HICON    hIcon;
   int      nMaxObjects,nScaleWidth,index,x,y,size;

   // Create device context compatible with the display.
   hdc = GetWindowDC(GetDesktopWindow());
   hdcMem = CreateCompatibleDC(hdc);
   // Create a bitmap compatible with the default icon size.
   hbmCursor = CreateCompatibleBitmap(hdc,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON));
   ::ReleaseDC(GetDesktopWindow(),hdc);
   // Select bitmap object on device context.
   hbmOld    = (HBITMAP)SelectObject(hdcMem,hbmCursor);

   // Clear bitmap.
   SetRect(&rcBitmap,0,0,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON));
   hbrBackground = CreateSolidBrush(RGB_ICON_TRANSPARENT_COLOR);
   FillRect(hdcMem,&rcBitmap,hbrBackground);

   if (m_fDragSelectedObjs==TRUE)
   {
      // Draw selected object(s) icon(s) on bitmap.
      if (GetSelectedObjectCount()>0)
      {
         // Get icon handle to first object selected.
         hIcon = GetFirstSelectedObject()->GetIconHandle();
         // If there is only 1 color
         if (GetSelectedObjectCount()==1)
            DrawIcon(hdcMem,0,0,hIcon);
         else
         {
            // There is more than 1 object selected.
            nMaxObjects = min(GetSelectedObjectCount(),MAX_OBJECTS_IN_CURSOR);
            // Calculate scaled cursor width.
            nScaleWidth = (GetSystemMetrics(SM_CXICON)/(nMaxObjects+1));

            // Create device context compatible with the display.
            hdc        = GetWindowDC(GetDesktopWindow());
            hdcMem2    = CreateCompatibleDC(hdc);
            hdcMask    = CreateCompatibleDC(hdc);
            // Create a bitmap compatible with the default icon size.
            hbmCursor2 = CreateCompatibleBitmap(hdc,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON));
            hbmMask    = CreateBitmap(GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),1,1,NULL);

            // Select bitmap object on device context.
            hbmOld2    = (HBITMAP)SelectObject(hdcMem2,hbmCursor2);
            hbmOldMask = (HBITMAP)SelectObject(hdcMask,hbmMask);
            // Calculate final icon size for each selected object.
            size = (2*nScaleWidth);
            // Go through all selected objects that will be displayed in cursor.
            // Walk the indexes backwards so that top-right icon will be
            // underneath bottom-left icon.
            for (index=(nMaxObjects-1); index>=0; index--)
            {
               // Clear bitmap.
               FillRect(hdcMem2,&rcBitmap,hbrBackground);

               // Draw icon in temporary bitmap.
               DrawIcon(hdcMem2,0,0,hIcon);

               // Create mask bitmap of selected object.
               SetBkColor(hdcMem2,RGB_ICON_TRANSPARENT_COLOR);
               BitBlt(hdcMask,0,0,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),hdcMem2,0,0,SRCCOPY);

               // Copy icon bitmap into final cursor bitmap using mask bitmap.
               x = (index * nScaleWidth)-1;
               y = (((nMaxObjects-1)-index) * nScaleWidth)-1;
               // XOR new icon bitmap with final cursor bitmap.
               StretchBlt(hdcMem,x,y,size,size,
                          hdcMem2,0,0,GetSystemMetrics(SM_CXICON),
                          GetSystemMetrics(SM_CYICON),SRCINVERT);

               // AND mask bitmap into final cursor bitmap.
               StretchBlt(hdcMem,x,y,size,size,
                          hdcMask,0,0,GetSystemMetrics(SM_CXICON),
                          GetSystemMetrics(SM_CYICON),SRCAND);

               // Again, XOR new icon bitmap with final cursor bitmap.
               StretchBlt(hdcMem,x,y,size,size,
                          hdcMem2,0,0,GetSystemMetrics(SM_CXICON),
                          GetSystemMetrics(SM_CYICON),SRCINVERT);

               // Get next selected object's icon (if available).
               if (index>0)
                  hIcon = GetNextSelectedObject()->GetIconHandle();
            }
            // Now, "hbmCursor" contains a "ready-to-convert" bitmap image of
            // the final cursor. do some clean-up before returning.

            // Destroy mask bitmap and memory device context.
            SelectObject(hdcMask,hbmOldMask);
            ::DeleteObject(hbmMask);
            DeleteDC(hdcMask);
            // Destroy temporary bitmap and memory device context.
            SelectObject(hdcMem2,hbmOld2);
            ::DeleteObject(hbmCursor2);
            DeleteDC(hdcMem2);
            // Release the desktop device context.
            ::ReleaseDC(GetDesktopWindow(),hdc);
         }
      }
   }
   else
   {
      // Get icon handle to object being dragged.
      hIcon = m_pwpobjDrag->GetIconHandle();
      // Draw icon on memory bitmap.
      DrawIcon(hdcMem,0,0,hIcon);
   }

   // Clean-up by destroying objects created.
   ::DeleteObject(hbrBackground);
   SelectObject(hdcMem,hbmOld);
   DeleteDC(hdcMem);

   // Return handle to created bitmap.
   return (hbmCursor);
}


//---------------------------------------------------------------------
// DrawIconOnBitmap():
//---------------------------------------------------------------------
void WPFOLDER::DrawIconOnBitmap(HBITMAP hbm,HICON hIcon)
{
   HDC     hdc,hdcMem;
   HBITMAP hbmOld;

   // Create device context compatible with the display.
   hdc    = GetWindowDC(GetDesktopWindow());
   hdcMem = CreateCompatibleDC(hdc);
   ::ReleaseDC(GetDesktopWindow(),hdc);
   // Select bitmap object on device context.
   hbmOld = (HBITMAP)SelectObject(hdcMem,hbm);

   // Draw icon in bitmap.
   DrawIcon(hdcMem,0,0,hIcon);

   // Restore original bitmap object into memory device context.
   SelectObject(hdcMem,hbmOld);
   // Delete memory device context.
   DeleteDC(hdcMem);
}



//---------------------------------------------------------------------
// CreateMoveCursors():
//---------------------------------------------------------------------
void WPFOLDER::CreateMoveCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag)
{
   HBITMAP hbm;
   HICON   hiconNoSign;

   // Create cursor bitmap as B&W replica of object(s) being dragged.
   hbm = CreateCursorBitmap();
   // Create real cursor from cursor bitmap created above.
   *phcurDrag   = CursorFromBitmap(hbm);
   // Load the "no" sign from our resource pool.
   hiconNoSign = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_NOSIGN));
   // Add the "no" sign on top of cursor bitmap.
   DrawIconOnBitmap(hbm,hiconNoSign);
   // Create real cursor from bitmap with "no" sign.
   *phcurNoDrag = CursorFromBitmap(hbm);
   // Clean-up by destroying resource icon and bitmap object.
   ::DestroyIcon(hiconNoSign);
   ::DeleteObject(hbm);
}

//---------------------------------------------------------------------
// CreateCopyCursors():
//---------------------------------------------------------------------
void WPFOLDER::CreateCopyCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag)
{
   HBITMAP hbm;
   HICON   hiconNoSign,hiconGrid;

   // Create cursor bitmap as B&W replica of object(s) being dragged.
   hbm = CreateCursorBitmap();
   // Load the transparency "grid" icon from our resource pool.
   hiconGrid = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_COPYGRID));
   // Draw 32x32 grid on cursor bitmap to create the illusion of transparency.
   DrawIconOnBitmap(hbm,hiconGrid);
   // Create real cursor from cursor bitmap created above.
   *phcurDrag   = CursorFromBitmap(hbm);
   // Load the "no" sign from our resource pool.
   hiconNoSign = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_NOSIGN));
   // Add the "no" sign on top of cursor bitmap.
   DrawIconOnBitmap(hbm,hiconNoSign);
   // Create real cursor from bitmap with "no" sign.
   *phcurNoDrag = CursorFromBitmap(hbm);
   // Clean-up by destroying resource icon and bitmap object.
   ::DestroyIcon(hiconGrid);
   ::DestroyIcon(hiconNoSign);
   ::DeleteObject(hbm);
}

//---------------------------------------------------------------------
// CreateShadowCursors():
//---------------------------------------------------------------------
void WPFOLDER::CreateShadowCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag)
{
   HBITMAP hbm;
   HICON   hiconNoSign,hiconShadowCircle;

   // Create cursor bitmap as B&W replica of object(s) being dragged.
   hbm = CreateCursorBitmap();
   // Load the transparency "grid" icon from our resource pool.
   hiconShadowCircle = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_SHADOWCIRCLE));
   // Draw shadow circle on center of cursor bitmap.
   DrawIconOnBitmap(hbm,hiconShadowCircle);
   // Create real cursor from cursor bitmap created above.
   *phcurDrag   = CursorFromBitmap(hbm);
   // Load the "no" sign from our resource pool.
   hiconNoSign = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_NOSIGN));
   // Add the "no" sign on top of cursor bitmap.
   DrawIconOnBitmap(hbm,hiconNoSign);
   // Create real cursor from bitmap with "no" sign.
   *phcurNoDrag = CursorFromBitmap(hbm);
   // Clean-up by destroying resource icon and bitmap object.
   ::DestroyIcon(hiconShadowCircle);
   ::DestroyIcon(hiconNoSign);
   ::DeleteObject(hbm);
}


//---------------------------------------------------------------------
// FOLDER VIEWS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------
                              
//---------------------------------------------------------------------
// SortCompareObjects():
//    This function is used by "SortIcons()" to compare pairs of 
// objects when deciding if they should be swapped.  A return value
// of TRUE means that "Obj1" is greater than "Obj2" and should be 
// swapped up the list (or array).  A return value of FALSE means 
// that "Obj1" is less than or equal to "Obj2" and should be left
// in order.
//---------------------------------------------------------------------
BOOL WPFOLDER::SortCompareObjects(WORD wSortKey,WPOBJECT *pwpobjObj1,WPOBJECT *pwpobjObj2)
{
   SORT_VALUE_STRUCT    svsSortValue1,svsSortValue2;
   
   // Get each object's sort key value.
   pwpobjObj1->GetSortValue(wSortKey,&svsSortValue1);
   pwpobjObj2->GetSortValue(wSortKey,&svsSortValue2);
                                     
   // Compare object key values and return appropriate boolean flag.
   // Depending on the sort criteria...
   switch (wSortKey)
   {
      case IDM_SORTBYNAME:
      case IDM_SORTBYREALNAME:
         // Sort objects by title name.
         return (stricmp(svsSortValue1.m_szValue,svsSortValue2.m_szValue) > 0);
         break;
      
      case IDM_SORTBYTYPE:
         // Sort objects by object type value (containers first then all others).
         if (((svsSortValue1.m_dwValue&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)&&
             ((svsSortValue1.m_dwValue&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER))
            return (FALSE);
         else
         if ((svsSortValue2.m_dwValue&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)
            return (TRUE);
         else
            return (FALSE);
         
      case IDM_SORTBYSIZE:    
      case IDM_SORTBYLASTACCESSDATE:
      case IDM_SORTBYLASTACCESSTIME:
         // Sort objects by double word value. 0 < k < max.
         return (svsSortValue1.m_dwValue > svsSortValue2.m_dwValue);
   }
   return (FALSE);
}
                              
                              
//---------------------------------------------------------------------
// SortIcons():
//    This function is in charge of sorting a folder's children
// objects in the specified order.  The sorting algorithm used is a 
// simple "Bubble Sort" since the time/complexity requirement is 
// minimal for this size array.
//---------------------------------------------------------------------
void WPFOLDER::SortIcons(WORD wSortKey)
{  
   BOOL     fSwapped;
   WPOBJECT *pwpobjTemp;
   int      index,nNumPairs;
   
   // Initialize "while" loop conditional variable to do at least one pass.
   nNumPairs = GetObjectCount();
   fSwapped  = TRUE;
   
   // We use a "Bubble Sort" algorithm to sort all children objects in folder.
   // While at least one swap occurred in loop (indication that we are not done yet)...
   while (fSwapped)
   {  
      // Decrement by one since we keep track of the number of object pairs unsorted.
      nNumPairs--;
      // Reset boolean flag to test if any swaps occurr.
      fSwapped = FALSE;
      // Loop for all object pairs that have not been sorted yet...
      for (index=0; index<nNumPairs; index++)
      {
         // Check if there is a need to swap the two adjancent objects...
         if (SortCompareObjects(wSortKey,GetObject(index),GetObject(index+1))==TRUE)
         {                                                              
            // Swap the two adjacent objects since object at (index) is greater
            // than object at (index+1).  Therefore, we must interchange them.
            pwpobjTemp = GetObject(index);
            SetObject(GetObject(index+1),index);
            SetObject(pwpobjTemp,index+1);
            // Set boolean flag since we just swapped the pair of objects.
            fSwapped = TRUE;
         }
      }
   }
}

//---------------------------------------------------------------------
// ArrangeIcons():
//---------------------------------------------------------------------
void WPFOLDER::ArrangeIcons(void)
{
   int   index,nIconSpacingX,nWindowWidth,nOffset;
   POINT ptNew;
   RECT  rcLogDomain,rcObject;
      
   // Store frequently used values in local variable.
   nIconSpacingX = GetSystemMetrics(SM_CXICONSPACING);      
      
   // Calculate the minimum width of folder window.
   nWindowWidth = max((GetWndSize().cx-nIconSpacingX),nIconSpacingX);

   // Initialize first object's position to top-left corner of folder window.
   ptNew.x = g_wpEnvironment.sizeIconIndent.cx;
   ptNew.y = g_wpEnvironment.sizeIconIndent.cy;                       
   
   // Walk through the list of objects, and assign an iconic position.
   // Starting from top to bottom, and going from left to right.
   for (index=0; index<GetObjectCount(); index++)
   {  
      // If it is the first object in a row...
      if (ptNew.x==g_wpEnvironment.sizeIconIndent.cx)
      {                                     
         // Get current object's width and height.
         GetObject(index)->GetObjectRect(&rcObject);
         // Calculate needed distance to prevent icon title overflowing to left.
         nOffset = ((rcObject.right-rcObject.left+1-GetSystemMetrics(SM_CXICON))/2)+1;
         // Check if object's width will overflow to the left...         
         if (nOffset > g_wpEnvironment.sizeIconIndent.cx)
            // Increase object's x-position.
            ptNew.x = nOffset;
      }
      // Set new icon's position.
      GetObject(index)->SetIconPos(ptNew.x,ptNew.y);
      // Get current object's width and height.
      GetObject(index)->GetObjectRect(&rcObject);
      // Next object's icon position will be determined by current object's width.
      nOffset = (((rcObject.right-rcObject.left+1)+GetSystemMetrics(SM_CXICON))+nIconSpacingX)/2;
      ptNew.x += nOffset;
      // If next object's position is too far to the right,... 
      if (ptNew.x > nWindowWidth)
      {
         // Then move next position to the next row.
         ptNew.x  = g_wpEnvironment.sizeIconIndent.cx;
         ptNew.y += GetSystemMetrics(SM_CYICONSPACING);
      }
   }

   // If folder has scrollable property for client space.
   if ((isScrollable()==TRUE)&&(isOpen()==TRUE))
   {
      // Get the current logical domain rect.
      CalculateLogicalDomainRect(&rcLogDomain);
      // Set the logical domain rect.
      SetLogicalDomain(&rcLogDomain);
      // Arrange scrollbars.
      ArrangeScrollbars();
   }

   // Invalidate and update whole window right now.
   Invalidate(TRUE,TRUE);
}


//---------------------------------------------------------------------
// DRAW MIN/MAX FOLDER BUTTONS FUNCTION.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawMinMaxButtons():
//---------------------------------------------------------------------
void WPFOLDER::DrawMinMaxButtons(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown)
{
   // Check which min/max button style is being used for folder...
   if ((GetFlags()&BUTTONAPPEARANCE_FLAGS_MASK)==BUTTONAPPEARANCE_HIDE)
      DrawMinMaxButtonsHideStyle(hdc,cx,fMinDown,fMaxDown);  // Hide style.
   else
      DrawMinMaxButtonsMinStyle(hdc,cx,fMinDown,fMaxDown);   // Min style.
}

//---------------------------------------------------------------------
// COPY, MOVE, AND DELETE SELECTED OBJECTS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateAnotherFunc():
//    This function handles the "Create another..." function from 
// system popup menu.
//---------------------------------------------------------------------
void WPFOLDER::CreateAnotherFunc(void)
{  
   COPYMOVECREATE_STRUCT   cmcsDlgInfo;
   WPOBJECT                *pwpobjCreate;
   DLGPROC                 lpfnCreateDlgProc;
   BOOL                    fReturnCode;
   int                     index;
                           
   // Initialize "Create another" information into data structure.
   cmcsDlgInfo.m_wFuncType       = IDM_CREATEANOTHER;
   // Store pointer to parent folder of create operation in field.
   cmcsDlgInfo.m_pwpfolderParent = this;
   // If object to create is not in selected group OR there is only 1 object selected in folder...
   if ((isObjSelected(GetObjectPopup())==FALSE)||(GetSelectedObjectCount()<=1))
   {
      // Set "single object" flag.
      cmcsDlgInfo.m_fSingleObject = TRUE;
      // Create object's title text into structure field.
      strcpy(cmcsDlgInfo.m_szNewName,GetObjectPopup()->GetTitleNoCRLF());
   }
   else
      // Operation is on a list of selected objects...
      cmcsDlgInfo.m_fSingleObject = FALSE;
   // Clear return value in structure.
   cmcsDlgInfo.m_pwpfolderTarget = NULL;
                           
   // Make a procedure instance for the "Create another..." dialog procedure.
   lpfnCreateDlgProc = (DLGPROC)MakeProcInstance((FARPROC)CopyMoveAndCreateDlgProc,g_wpEnvironment.hInst);
   // Display "Create another" dialog so user can choose the target folder.
   fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_COPYMOVECREATE),g_wpEnvironment.hwndDesktop,lpfnCreateDlgProc,(LPARAM)(LPVOID)&(cmcsDlgInfo));
   // Free the procedure instance created for dialog.
   FreeProcInstance((FARPROC)lpfnCreateDlgProc);
   
   // Depending on which button the user pressed...
   if ((fReturnCode==TRUE)&&(cmcsDlgInfo.m_pwpfolderTarget!=NULL))
   {
      // Do create operation.
      if (cmcsDlgInfo.m_fSingleObject==TRUE)
      {     
         // Create single object into target folder.          
         // First, create new object instance.
         pwpobjCreate = new_object(GetObjectPopup());
         // Get an available icon position on target folder.
         pwpobjCreate->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
         // Set no icon path and index so that object will pickup default icon.
         pwpobjCreate->SetIcon("",0);
         // If target folder is currently open...
         if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
            // Load icon of newly created object.
            pwpobjCreate->LoadIcon();
         // Create object's title name.
         pwpobjCreate->SetTitle(cmcsDlgInfo.m_szNewName);
         // Add newly created object instance into target folder.
         if (cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCreate)==FALSE)
            // Delete object instance since it could not be added to target folder.
            delete_object(pwpobjCreate);
         else  
         {
            // Repaint new object so it is visible.
            pwpobjCreate->InvalidateIcon(TRUE,TRUE); 
            // Open settings notebook of new object so that user can change settings.
            pwpobjCreate->OpenSettingsNotebook();
         }
      }
      else
      {   
         // Create all selected objects into target folder.
         for (index=0; index<GetSelectedObjectCount(); index++)
         {  
            // First, create new object instance of current object.
            pwpobjCreate = new_object(GetSelectedObject(index));
            // Check that a new object was created successfully...
            if (pwpobjCreate!=NULL)
            {
               // Get an available icon position on target folder.
               pwpobjCreate->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
               // Set no icon path and index so that object will pickup default icon.
               pwpobjCreate->SetIcon("",0);
               // If target folder is currently open...
               if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
                  // Load icon of newly created object.
                  pwpobjCreate->LoadIcon();
               // Create newly created object tree into target folder.
               if (cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCreate)==FALSE)
                  // Delete object instance since it could not be added to target folder.
                  delete_object(pwpobjCreate);
               else
               {
                  // Repaint new object so it is visible.
                  pwpobjCreate->InvalidateIcon(TRUE,TRUE);
                  // Open settings notebook of new object so that user can change settings.
                  pwpobjCreate->OpenSettingsNotebook();
               }
            }
         }
      }
   }
}
 
 
//---------------------------------------------------------------------
// CreateShadowFunc():
//    This function handles the "Create shadow..." function from 
// system popup menu.
//---------------------------------------------------------------------
void WPFOLDER::CreateShadowFunc(void)
{  
   COPYMOVECREATE_STRUCT   cmcsDlgInfo;
   WPSHADOW                *pwpobjCreate;
   DLGPROC                 lpfnCreateDlgProc;
   BOOL                    fReturnCode;
   int                     index;
                           
   // Initialize "Create shadow" information into data structure.
   cmcsDlgInfo.m_wFuncType       = IDM_CREATESHADOW;
   // Store pointer to parent folder of create operation in field.
   cmcsDlgInfo.m_pwpfolderParent = this;
   // If object to create is not in selected group OR there is only 1 object selected in folder...
   if ((isObjSelected(GetObjectPopup())==FALSE)||(GetSelectedObjectCount()<=1))
   {
      // Set "single object" flag.
      cmcsDlgInfo.m_fSingleObject = TRUE;
      // Create object's title text into structure field.
      strcpy(cmcsDlgInfo.m_szNewName,GetObjectPopup()->GetTitleNoCRLF());
   }
   else
      // Operation is on a list of selected objects...
      cmcsDlgInfo.m_fSingleObject = FALSE;
   // Clear return value in structure.
   cmcsDlgInfo.m_pwpfolderTarget = NULL;
                           
   // Make a procedure instance for the "Create another..." dialog procedure.
   lpfnCreateDlgProc = (DLGPROC)MakeProcInstance((FARPROC)CopyMoveAndCreateDlgProc,g_wpEnvironment.hInst);
   // Display "Create another" dialog so user can choose the target folder.
   fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_COPYMOVECREATE),g_wpEnvironment.hwndDesktop,lpfnCreateDlgProc,(LPARAM)(LPVOID)&(cmcsDlgInfo));
   // Free the procedure instance created for dialog.
   FreeProcInstance((FARPROC)lpfnCreateDlgProc);
   
   // Depending on which button the user pressed...
   if ((fReturnCode==TRUE)&&(cmcsDlgInfo.m_pwpfolderTarget!=NULL))
   {
      // Do create operation.
      if (cmcsDlgInfo.m_fSingleObject==TRUE)
      {     
         // Create single object into target folder.          
         // First, create new shadow object instance.
         pwpobjCreate = new WPSHADOW;
         // Add shadow link of new shadow instance to original object...
         if (GetObjectPopup()->AddShadow(pwpobjCreate)==FALSE)
         {
            // Shadow link could not be added, therefore delete shadow instance.
            delete pwpobjCreate;
            // Display message to user that no more shadows can be added to object.
            MessageBox(NULL,GetString(IDSTR_ERR_NO_MORE_SHADOWS),GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
         }  
         else
         {
            // Set shadow link to original object instance.
            pwpobjCreate->SetOriginalObject(GetObjectPopup());
            // Get an available icon position on target folder.
            pwpobjCreate->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
            // If target folder is currently open...
            if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
               // Load icon of newly created object.
               pwpobjCreate->LoadIcon();
            // Add newly created object instance into target folder.
            if (cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCreate)==FALSE)
            {                                               
               // Delete shadow link from original object to shadow instance.
               GetObjectPopup()->DeleteShadow(pwpobjCreate);
               // Delete object instance since it could not be added to target folder.
               delete pwpobjCreate;
               
            }
            else
               // Repaint new object so it is visible.
               pwpobjCreate->InvalidateIcon(TRUE,TRUE);
         }
      }
      else
      {   
         // Create all selected objects into target folder.
         for (index=0; index<GetSelectedObjectCount(); index++)
         {  
            // First, create new object instance of current object.
            pwpobjCreate = new WPSHADOW;
            // Set shadow link to original object instance.
            pwpobjCreate->SetOriginalObject(GetSelectedObject(index));
            // Get an available icon position on target folder.
            pwpobjCreate->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
            // If target folder is currently open...
            if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
               // Load icon of newly created object.
               pwpobjCreate->LoadIcon();
            // Create newly created object tree into target folder.
            if (cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCreate)==FALSE)
               // Delete object instance since it could not be added to target folder.
               delete_object(pwpobjCreate);
            else
               // Repaint new object so it is visible.
               pwpobjCreate->InvalidateIcon(TRUE,TRUE);
         }
      }
   }
}

   
//---------------------------------------------------------------------
// CopyFunc():
//    This function handles the "Copy..." function from system popup
// menu.
//---------------------------------------------------------------------
void WPFOLDER::CopyFunc(void)
{  
   COPYMOVECREATE_STRUCT   cmcsDlgInfo;
   WPOBJECT                *pwpobjCopy;
   DLGPROC                 lpfnCopyDlgProc;
   BOOL                    fReturnCode;
   int                     index;
                           
   // Initialize "Copy" information into data structure.
   cmcsDlgInfo.m_wFuncType       = IDM_COPY;
   // Store pointer to parent folder of copy operation in field.
   cmcsDlgInfo.m_pwpfolderParent = this;
   // If object to copy is not in selected group OR there is only 1 object selected in folder...
   if ((isObjSelected(GetObjectPopup())==FALSE)||(GetSelectedObjectCount()<=1))
   {
      // Set "single object" flag.
      cmcsDlgInfo.m_fSingleObject = TRUE;
      // Copy object's title text into structure field.
      strcpy(cmcsDlgInfo.m_szNewName,GetObjectPopup()->GetTitleNoCRLF());
   }
   else
      // Operation is on a list of selected objects...
      cmcsDlgInfo.m_fSingleObject = FALSE;
   // Clear return value in structure.
   cmcsDlgInfo.m_pwpfolderTarget = NULL;
                           
   // Make a procedure instance for the "Copy..." dialog procedure.
   lpfnCopyDlgProc = (DLGPROC)MakeProcInstance((FARPROC)CopyMoveAndCreateDlgProc,g_wpEnvironment.hInst);
   // Display "Copy" dialog so user can choose the target folder.
   fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_COPYMOVECREATE),g_wpEnvironment.hwndDesktop,lpfnCopyDlgProc,(LPARAM)(LPVOID)&(cmcsDlgInfo));
   // Free the procedure instance created for dialog.
   FreeProcInstance((FARPROC)lpfnCopyDlgProc);
   
   // Depending on which button the user pressed...
   if ((fReturnCode==TRUE)&&(cmcsDlgInfo.m_pwpfolderTarget!=NULL))
   {
      // Do copy operation.
      if (cmcsDlgInfo.m_fSingleObject==TRUE)
      {     
         // Copy single object into target folder.          
         // First, create tree duplicate of object.
         pwpobjCopy = CreateTreeDuplicate(NULL,GetObjectPopup());
         // Get an available icon position on target folder.
         pwpobjCopy->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
         // If target folder is currently open...
         if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
            // Load icon of newly created object.
            pwpobjCopy->LoadIcon();
         // Copy object's title name.
         pwpobjCopy->SetTitle(cmcsDlgInfo.m_szNewName);
         // Copy newly created object tree into target folder.
         cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCopy);
         // Repaint new object so it is visible.
         pwpobjCopy->InvalidateIcon(TRUE,TRUE);
      }
      else
      {   
         // Copy all selected objects into target folder.
         for (index=0; index<GetSelectedObjectCount(); index++)
         {
            // First, create tree duplicate of current object.
            pwpobjCopy = CreateTreeDuplicate(NULL,GetSelectedObject(index));
            // Check that object was replicated successfully...
            if (pwpobjCopy!=NULL)
            {
               // Get an available icon position on target folder.
               pwpobjCopy->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
               // If target folder is currently open...
               if (cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)
                  // Load icon of newly created object.
                  pwpobjCopy->LoadIcon();
               // Copy newly created object tree into target folder.
               cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjCopy);
               // Repaint new object so it is visible.
               pwpobjCopy->InvalidateIcon(TRUE,TRUE);
            }
         }
      }
   }
}
      

//---------------------------------------------------------------------
// MoveFunc():
//    This function handles the "Move..." function from system popup
// menu.
//---------------------------------------------------------------------
void WPFOLDER::MoveFunc(void)
{
   COPYMOVECREATE_STRUCT   cmcsDlgInfo;
   WPOBJECT                *pwpobjMove,**apwpobjSelected;
   DLGPROC                 lpfnMoveDlgProc;
   BOOL                    fReturnCode,fParentIsOpen;
   int                     index,nSelCount;
                           
   // Initialize "Move" information into data structure.
   cmcsDlgInfo.m_wFuncType       = IDM_MOVE;
   // Store pointer to parent folder of copy operation in field.
   cmcsDlgInfo.m_pwpfolderParent = this;
   // If object to copy is not in selected group OR there is only 1 object selected in folder...
   if ((isObjSelected(GetObjectPopup())==FALSE)||(GetSelectedObjectCount()<=1))
   {
      // Set "single object" flag.
      cmcsDlgInfo.m_fSingleObject = TRUE;
      // Copy object's title text into structure field.
      strcpy(cmcsDlgInfo.m_szNewName,GetObjectPopup()->GetTitleNoCRLF());
   }
   else
      // Operation is on a list of selected objects...
      cmcsDlgInfo.m_fSingleObject = FALSE;
   // Clear return value in structure.
   cmcsDlgInfo.m_pwpfolderTarget = NULL;
                           
   // Make a procedure instance for the "Move..." dialog procedure.
   lpfnMoveDlgProc = (DLGPROC)MakeProcInstance((FARPROC)CopyMoveAndCreateDlgProc,g_wpEnvironment.hInst);
   // Display "Move" dialog so user can choose the target folder.
   fReturnCode = (BOOL)DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_COPYMOVECREATE),g_wpEnvironment.hwndDesktop,lpfnMoveDlgProc,(LPARAM)(LPVOID)&(cmcsDlgInfo));
   // Free the procedure instance created for dialog.
   FreeProcInstance((FARPROC)lpfnMoveDlgProc);
   
   // Depending on which button the user pressed...
   if ((fReturnCode==TRUE)&&(cmcsDlgInfo.m_pwpfolderTarget!=NULL))
   {
      // Do copy operation.
      if (cmcsDlgInfo.m_fSingleObject==TRUE)
      {     
         // Move single object into target folder.
         pwpobjMove = GetObjectPopup();
         // Set flag if parent folder of object being moved is currently opened.
         fParentIsOpen = pwpobjMove->GetParent()->isOpen();
         // Invalidate object area so it will be removed on next paint.
         pwpobjMove->InvalidateIcon(FALSE,TRUE);
         // Remove source object from parent folder.
         pwpobjMove->GetParent()->DeleteObject(pwpobjMove);         
         // Get an available icon position on target folder.
         pwpobjMove->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
         // If target folder is currently open AND original parent folder was not open...
         if ((cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)&&(fParentIsOpen==FALSE))
            // Load icon of object being moved since target parent is open.
            pwpobjMove->LoadIcon();
         // Copy the object's title name.
         pwpobjMove->SetTitle(cmcsDlgInfo.m_szNewName);
         // Move object into target folder.
         cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjMove);
         // Repaint object so it is visible.
         pwpobjMove->InvalidateIcon(TRUE,TRUE);
      }
      else
      {  
         // Allocate enough memory to hold list of all selected objects.
         apwpobjSelected = new WPOBJECT *[GetSelectedObjectCount()];
         // Make a local copy of selected object list since the list is updated 
         // when deleting object from folder.
         for (nSelCount=0; nSelCount<GetSelectedObjectCount(); nSelCount++)
            apwpobjSelected[nSelCount] = GetSelectedObject(nSelCount);

         // Clear selected object list and invalidate their area.
         ClearSelectedObjects(TRUE);
         // Update folder's visual state right now.
         UpdateWindow();
      
         // Copy all selected objects into target folder.
         for (index=0; index<nSelCount; index++)
         {
            // Store pointer to selected object into local variable.
            pwpobjMove = apwpobjSelected[index];
            // Set flag if parent folder of object being moved is currently opened.
            fParentIsOpen = pwpobjMove->GetParent()->isOpen();
            // Invalidate object area so it will be removed on next paint.
            pwpobjMove->InvalidateIcon(FALSE,TRUE);
            // Remove source object from parent folder.
            pwpobjMove->GetParent()->DeleteObject(pwpobjMove);         
            // Get an available icon position on target folder.
            pwpobjMove->SetIconPos(cmcsDlgInfo.m_pwpfolderTarget->GetFirstAvailableIconPos(NULL));
            // If target folder is currently open AND original parent folder was not open...
            if ((cmcsDlgInfo.m_pwpfolderTarget->isOpen()==TRUE)&&(fParentIsOpen==FALSE))
               // Load icon of object being moved since target parent is open.
               pwpobjMove->LoadIcon();
            // Move object into target folder.
            cmcsDlgInfo.m_pwpfolderTarget->AddObject(pwpobjMove);
            // Repaint object so it is visible.
            pwpobjMove->InvalidateIcon(TRUE,TRUE);
         }
         // Deallocate memory used to hold copy of selected objects.
         delete[] apwpobjSelected;
      }
   }
}


//---------------------------------------------------------------------
// DeleteSelectedObjects():
//---------------------------------------------------------------------
void WPFOLDER::DeleteSelectedObjects(void)
{                    
   HCURSOR  hcurOriginal;
   WPOBJECT **apwpobjSelected,**apwpobjNotDeleted;
   int      nNotDeleted,nSelectedCount,index;

   // Get number of currently selected objects.
   nSelectedCount = GetSelectedObjectCount();
   // Allocate enough memory to hold list of all selected objects.
   apwpobjSelected   = new WPOBJECT *[nSelectedCount];
   apwpobjNotDeleted = new WPOBJECT *[nSelectedCount];
   // Get list of selected objects, and store them in local array...
   for (index=0; index<nSelectedCount; index++)
      apwpobjSelected[index] = GetSelectedObject(index);

   // Clear list of selected objects to "none" (do not invalidate them).
   ClearSelectedObjects(FALSE);

   // Initialize not deleted list index.
   nNotDeleted = 0;

   // Do delete operation for all selected objects.
   // While there exists a selected object.
   for (index=0; index<nSelectedCount; index++)
   {
      // Confirm with user the delete operation...
      switch (apwpobjSelected[index]->ConfirmOnDelete())
      {
         case IDYES:                             
            // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
            hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
            // If object is currently placed in an object which represents the file system...
            if (apwpobjSelected[index]->GetParent()->isDirectoryContainer()==TRUE)
               // Delete object from file system.
               apwpobjSelected[index]->DeleteObjectFromFileSystem();
            // Call object's virtual destroy handler.
            apwpobjSelected[index]->Destroy();
            // Check if original cursor was not the same as our "wait" cursor...      
            if (hcurOriginal!=GetCursor())
               // Restore original cursor and destroy custom wait cursor.
               DestroyCursor(SetCursor(hcurOriginal));
            break;

         case IDNO:
            // Do not delete current object and save in "not deleted" list.
            apwpobjNotDeleted[nNotDeleted++] = apwpobjSelected[index];
            break;

         case IDCANCEL:
            // While there are still selected objects remaining...
            while (index<nSelectedCount)
            {
               // Do not delete current object and save in "not deleted" list.
               apwpobjNotDeleted[nNotDeleted++] = apwpobjSelected[index];
               // Get next selected object.
               index++;
            }
            // Invalidate all "not deleted" objects and update them now.
            for (index=0; index<nNotDeleted; index++)
               apwpobjNotDeleted[index]->InvalidateIcon(TRUE,TRUE);
            // Deallocate memory used to hold copy of selected objects.
            delete[] apwpobjSelected;
            delete[] apwpobjNotDeleted;
            // Terminate delete selected objects operation.
            return;
      }
   }

   // Invalidate all "not deleted" objects and update them now.
   for (index=0; index<nNotDeleted; index++)
      apwpobjNotDeleted[index]->InvalidateIcon(TRUE,TRUE);

   // Deallocate memory used to hold copy of selected objects.
   delete[] apwpobjSelected;
   delete[] apwpobjNotDeleted;

   // Successfully deleted selected objects, return to calling function.
}

//---------------------------------------------------------------------
// WINDOWS 3.1 DRAG & DROP SERVICE FUNCTIONS
//    (these functions are called by OnWin31DragAndDropMsg())
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Win31DDQueryDropObject():
//---------------------------------------------------------------------
BOOL WPFOLDER::Win31DDQueryDropObject(BOOL fClientArea,LPDRAGINFO pDragInfo)
{
   POINT    ptDrop;
   WPOBJECT *pwpobjTarget;
   BOOL     fTargetIsFolderArea;

   // If cursor is on non-client area, return "not accepting".
   if (fClientArea==FALSE)
      return (FALSE);
   // Store drop point
   ptDrop.x = pDragInfo->x;
   ptDrop.y = pDragInfo->y;
   // Convert Window Point (client coord.) into device point.
   WPtoDP(&ptDrop);
   // Get target object within drop folder.
   pwpobjTarget = ChildHitTest(ptDrop);
   // If no child objects are being hit by point,...
   if (pwpobjTarget==NULL)
   {
      // set target object as the drop folder.
      pwpobjTarget          = this;
      fTargetIsFolderArea   = TRUE;
   }
   else
      fTargetIsFolderArea = FALSE;

   // If this target is not the same as the previous potential target...
   if ((pwpobjTarget!=m_pwpobjDropTarget)&&(m_pwpobjDropTarget!=NULL))
      // Unframe previous target.
      m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);

   // Store new potential target.
   m_pwpobjDropTarget    = pwpobjTarget;
   m_fTargetIsFolderArea = fTargetIsFolderArea;

   // Draw frame around potential target object if not selected already.
   m_pwpobjDropTarget->Frame(m_fTargetIsFolderArea);

   // Check if cursor is on top of a Workplace Shell folder object OR
   // the Workplace Shell desktop object...
   if ((pwpobjTarget->GetType()==OBJTYPE_FOLDER) ||
       (pwpobjTarget->GetType()==OBJTYPE_DESKTOP))
   {
      // Return "accepting".
      return (TRUE);
   }
   else
      // Return "not accepting".
      return (FALSE);
}

//---------------------------------------------------------------------
// Win31DDDragSelect():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPFOLDER::Win31DDDragSelect(BOOL fEnteringWindow,LPDRAGINFO pDragInfo)
{
   // If cursor is leaving the folder window...
   if (fEnteringWindow==FALSE)
   {
      // If this folder had a target framed, remove frame...
      if (m_pwpobjDropTarget!=NULL)
         // Unframe previous target.
         m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);
      // Set current potential drop target to "none".
      m_pwpobjDropTarget = NULL;
   }
   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// Win31DDDragSelect():
//---------------------------------------------------------------------
LRESULT WPFOLDER::Win31DDDropFiles(HDROP hDrop)
{
   POINT          ptDrop;
   WORD           wSize;
   HCURSOR        hcurOriginal;
   int            nFilesDropped;
   char           *pszPathName;
   struct _stat   FileStats;

   // If this folder does not have a target framed, return.
   if (m_pwpobjDropTarget==NULL)
   {
      // Free the memory block containing the pathnames
      DragFinish(hDrop);
      // Return from function.
      return (NULL);
   }

   // Unframe current drop target.
   m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);

   // Get drop (x,y) position where files were dropped (in client coord).
   DragQueryPoint(hDrop,&ptDrop);
   // If object receiving files dropped is this folder, then...
   if (m_pwpobjDropTarget==this)
      // Convert client point (or window point) to logical point.
      WPtoLP(&ptDrop);
   else
   {
      // Else, drop point becomes a "don't care" since object receiving
      // drop is not this folder but one of the child objects in the folder.
      ptDrop.x = -1;
      ptDrop.y = -1;
   }
   
   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
   
   // Get the number of pathnames that are being dropped on us.
   nFilesDropped = (int)DragQueryFile(hDrop,-1,NULL,0);
   // While we have more files to process...
   while ((nFilesDropped-- > 0) && (m_pwpobjDropTarget!=NULL))
   {
      // Get the length of the the pathname.
      wSize = (WORD)DragQueryFile(hDrop,(UINT)nFilesDropped,NULL,0) + 1;
      // Allocate a block of memory large enough for the pathname.
      pszPathName = (char *)malloc((size_t)wSize);
      // Check if out of memory.
      if (pszPathName == NULL)
         continue;
      // Copy the pathname into our local buffer
      DragQueryFile(hDrop,nFilesDropped,pszPathName,wSize);

      // Get file status to help decide which Workplace Shell object to create...
      if (_stat(pszPathName,&FileStats)==0)
         // Create Workplace Shell object from file/directory dropped.
         Win31DDCreateWPSObjectFromFileDropped(FileStats.st_mode,pszPathName,ptDrop);

      // Free memory allocated for file name.
      free(pszPathName);
   }
   // Free the memory block containing the pathnames
   DragFinish(hDrop);
   // Update the visual apperance of folder.
   m_pwpobjDropTarget->Invalidate(TRUE,TRUE);
   // Set current drop target to "none".
   m_pwpobjDropTarget = NULL;
                           
   // Check if original cursor was not the same as our "wait" cursor...      
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
   
   // Return from function.
   return (NULL);
}

//---------------------------------------------------------------------
// Win31DDCreateWPSObjectFromFileDropped():
//---------------------------------------------------------------------
void WPFOLDER::Win31DDCreateWPSObjectFromFileDropped(WORD wFileFlags,char *pszPathName,POINT ptDrop)
{
   WPOBJECT    *pwpobjNew;
   WPFOLDER    *pwpfolderNew;
   WPPROGRAM   *pwpprogNew;
   WPDATAFILE  *pwpdatafileNew;
   char        szDrive[_MAX_DRIVE+1],szPath[_MAX_DIR+1],
               szFName[_MAX_FNAME+1],szExt[_MAX_EXT+1],
               szString[_MAX_PATH+1];

   // Split pathname into its components.
    _splitpath(pszPathName,szDrive,szPath,szFName,szExt);

   // Check if pathname refers to a directory...
   if (wFileFlags&_S_IFDIR)
   {
      // Create Workplace Shell folder since path is a directory name.
      pwpfolderNew = new WPFOLDER;
      // Cast new pointer to folder object to generic object.
      pwpobjNew = (WPOBJECT *)pwpfolderNew;
   }
   // ...Else, pathname refers to a file...
   else
   {
      // If execute bits are set, file is a "program" file...
      if ((wFileFlags&_S_IEXEC)||(stricmp(szExt,".PIF")==0))
      {
         // Create Workplace Shell program object.
         pwpprogNew = new WPPROGRAM;
         // Set program path\filename string.
         pwpprogNew->SetProgramPath(pszPathName);
         // Set program object's icon resource to 0th index into file.
         pwpprogNew->SetIcon(pszPathName,0);
         // Cast new pointer to program object to generic object.
         pwpobjNew = (WPOBJECT *)pwpprogNew;
      }
      // ...Else, file is a "data" file...
      else
      {
         // Create Workplace Shell data object.
         pwpdatafileNew = new WPDATAFILE;
         // Create data file name (with extension).
         _makepath(szString,NULL,NULL,szFName,szExt);
         // Set data file name.
         pwpdatafileNew->SetDataName(szString);
         // Create data file's path string.
         _makepath(szString,szDrive,szPath,NULL,NULL);
         // Set data file path.
         pwpdatafileNew->SetDataPath(szString);
         
#if 0         
         // Check if data file meets any of the special cases below...
         // If data file is an icon file...
         if (stricmp(szExt,".ICO")==0)
            // Set data file's icon resource to 0th index into file.
            pwpdatafileNew->SetIcon(pszPathName,0);
         else
         // ...Else, if data file is a Windows help file...
         if (stricmp(szExt,".HLP")==0)
            // Set data file's icon resource to help icon.
            pwpdatafileNew->SetIcon(GetString(IDSTR_SHELL_FILENAME),WPS_HELPFILE_ICONINDEX);
#endif
            
         // Cast new pointer to data file object to generic object.
         pwpobjNew = (WPOBJECT *)pwpdatafileNew;
      }
   }
   // Get file name only (no extension or directory path).
   _makepath(szString,NULL,NULL,szFName,NULL);
   // Set title for new object (first letter upper case, rest lower case).
   strlwr(szString);
   szString[0] = (char)toupper(szString[0]);
   pwpobjNew->SetTitle(szString);
   // Get new location for object.
   pwpobjNew->SetIconPos(((WPFOLDER *)m_pwpobjDropTarget)->GetFirstAvailableIconPos(&ptDrop));
   // Load icon resource for new object only if drop folder is already open.
   if (m_pwpobjDropTarget->isOpen()==TRUE)
      pwpobjNew->LoadIcon();
   // Add object to drop folder.
   if (((WPFOLDER *)m_pwpobjDropTarget)->AddObject(pwpobjNew)==FALSE)
   {
      // Destroy icon resource for new object only if drop folder is already open.
      if (m_pwpobjDropTarget->isOpen()==TRUE)
         pwpobjNew->DestroyIcon();
      // Delete new object.
      delete pwpobjNew;
      // Display error message.
      MessageBox(GetHandle(),GetString(IDSTR_ERR_TOO_MANY_OBJECTS),GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
   }
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPFOLDER():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPFOLDER::WPFOLDER(void) : WPCONTAINER() , WPWINDOW()
{
   // Initialize folder object.
   SetType(OBJTYPE_FOLDER);
   m_clrrefBackground  = GetSysColor(COLOR_WINDOW);
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&m_lfIconText,0);
   m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(m_szImagePath,"");
   m_hpalImage         = NULL;
   m_hbmpImage         = NULL;
   m_pwpobjPopup       = NULL;
   m_wOpenView         = VIEW_ICON;
   m_fDragMode         = FALSE;
   m_pwpobjDropTarget  = NULL;
   m_wLeftButtonState  = MBS_UP;
   m_wRightButtonState = MBS_UP;
   m_fNCLButtonDown    = FALSE;
   // Set default minimized behavior to "hide".
   SetFlagsOR(BUTTONAPPEARANCE_HIDE,BUTTONAPPEARANCE_FLAGS_MASK);
   SetFlagsOR(MINBEHAVIOR_HIDE,MINBEHAVIOR_FLAGS_MASK);
   SetFlagsOR(OPENBEHAVIOR_DISPLAY,OPENBEHAVIOR_FLAGS_MASK);
}

//---------------------------------------------------------------------
// WPFOLDER():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPFOLDER::WPFOLDER(WPFOLDER *pwpfolderDefaults) :
   WPCONTAINER((WPCONTAINER *)pwpfolderDefaults) ,
   WPWINDOW((WPWINDOW *)pwpfolderDefaults)
{
   // Initialize folder object.
   SetType(OBJTYPE_FOLDER);
   m_clrrefBackground  = pwpfolderDefaults->GetBackgroundColor();
   pwpfolderDefaults->GetIconTextLogFont(&m_lfIconText);
   m_clrrefIconTextColor = pwpfolderDefaults->GetIconTextColor();
   strcpy(m_szImagePath,pwpfolderDefaults->GetImagePath());
   m_hpalImage         = NULL;
   m_hbmpImage         = NULL;
   m_pwpobjPopup       = NULL;
   m_wOpenView         = pwpfolderDefaults->GetOpenView();
   m_fDragMode         = FALSE;
   m_wLeftButtonState  = MBS_UP;
   m_wRightButtonState = MBS_UP;
   m_fNCLButtonDown    = FALSE;
}


//---------------------------------------------------------------------
// FOLDER CAPTION TEXT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetCaptionTitle():
//    This function returns the correct caption text title for this
// folder window.  It takes into consideration the open view mode
// and removes any special control ASCII chars. (such as CR and LF).
//---------------------------------------------------------------------
const char *WPFOLDER::GetCaptionTitle(void)
{
   static char szCaptionTitle[MAX_TITLE_NAME+1];

   // make caption text string from folder title and view mode.
   sprintf(szCaptionTitle,GetString(IDSTR_VIEW_CAPTION_TITLE),
      GetTitleNoCRLF(),GetString(IDSTR_VIEW_ICON+GetOpenView()));

   // return caption text string.
   return (szCaptionTitle);
}

                    
//---------------------------------------------------------------------
// FOLDER DEFAULT VALUES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPFOLDER::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_FOLDER_DEFAULT_ICONINDEX;
}
                    
//---------------------------------------------------------------------
// FOLDER STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Open():
//    This function creates a folder window and display its components.
//---------------------------------------------------------------------
BOOL WPFOLDER::Open(void)
{
   char *pszClassName;

   // If folder is not already open...
   if (isOpen()==FALSE)
   {
      // Create unique class for this folder instance.
      pszClassName = RegisterFolderClass(WPClientWindowProcWrapper);
      if (pszClassName!=NULL)
      {
         // Create folder overlapped window.
         if (CreateWindow(pszClassName,this)==FALSE)
         {
            // Unregister folder class before exiting with an error.
            UnregisterFolderClass();
            // Return failure.
            return (FALSE);
         }

         // Set "opened" state bit.
         SetState((GetState()|OBJSTATE_OPEN));

         // Update folder icon's visual state on parent container.
         InvalidateIcon(TRUE,TRUE);
         
         // Update folder's shadows visual state (if any).
         InvalidateShadows(TRUE);

         // Show window and update it.
         ShowWindow(GetHandle(),SW_SHOW);
         ::UpdateWindow(GetHandle());

         // Let Workplace Shell desktop know that this object "owns" the newly create folder window.
         PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_HWND_ASSOCIATE_OBJECT,(WPARAM)GetHandle(),(LPARAM)this);
      }
      else
         // Return failure.
         return (FALSE);
   }
   else
   {
      // Give focus to already existing folder...
      // If folder is minimized, restore to original size.
      if (IsIconic(GetHandle())!=FALSE)
         ShowWindow(GetHandle(),SW_RESTORE);

      // Set active focus to this folder.
      SetActiveWindow(GetHandle());
   }

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPFOLDER::Close(void)
{
   RECT              rcWindow,rcShrinkedWindow;
   WINDOWPLACEMENT   wplFolder;

   // Check if folder window is open...
   if (isOpen()==TRUE)
   {
      // Play system event sound associated with "close folder" event.
      PlaySystemEventSound("WPFolderClose");

      // If folder window is not minimized, "implode" window.
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

      // Set folder state to "closed".
      SetState((GetState()&(~OBJSTATE_OPEN)));

      // Store the last "normal" folder window size and position.
      // Get the "restored" or normal window position.
      wplFolder.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(GetHandle(),&wplFolder);
      // Calculate and store the "normal" window position and size.
      SetWndPos(wplFolder.rcNormalPosition.left,wplFolder.rcNormalPosition.top);
      SetWndSize((wplFolder.rcNormalPosition.right-wplFolder.rcNormalPosition.left+1),
                 (wplFolder.rcNormalPosition.bottom-wplFolder.rcNormalPosition.top+1));

      // Since the window is being closed, we must notify its
      // parent window to repaint the folder's icon completely because
      // the "hatch" rectangle on the icon must be erased.
      InvalidateIcon(TRUE,TRUE);

      // Invalidate folder's shadows (if any) visual state.
      InvalidateShadows(TRUE);

      // Destory icons of objects contained in folder.
      if (DestroyIcons()==FALSE)
         return (FALSE);

      // Post message to workplace shell to unregister folder class.
      UnregisterFolderClass();

      // Call Windows 3.1 API to destroy folder window.
      ::DestroyWindow(GetHandle());
   }

   return (TRUE);       // Successfully closed window.
}

//---------------------------------------------------------------------
// GetObjectPopup():
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::GetObjectPopup(void)
{
   // If folder is open, 
   if (isOpen()==TRUE)  
      // Return pointer to object that owns current menu popup.
      return (m_pwpobjPopup);
   else                                                        
      // No objects currently own any menu popups.
      return (NULL);
}

//---------------------------------------------------------------------
// SetObjectPopup():
//---------------------------------------------------------------------
void WPFOLDER::SetObjectPopup(WPOBJECT *pwpobjPopup)
{  
   // Store pointer to object that owns current menu popup.
   m_pwpobjPopup = pwpobjPopup;
}


//---------------------------------------------------------------------
// GetOpenView():
//    This function returns the current folder view.
//---------------------------------------------------------------------
WORD WPFOLDER::GetOpenView(void)
{     
   // Return current folder view (icon, details, or tree).
   return (m_wOpenView);
}


//---------------------------------------------------------------------
// SetOpenView():
//    This function sets the current folder view.  If the passed value
// is invalid, we default to "icon view".
//---------------------------------------------------------------------
void WPFOLDER::SetOpenView(WORD wOpenView)
{
   RECT  rcLogDomain;

   // Check if "view" parameter contains a valid value...
   if ((wOpenView<VIEW_ICON)||(wOpenView>VIEW_DETAILS))
      m_wOpenView = VIEW_ICON;   // Default to icon view.
   else
      m_wOpenView = wOpenView;   // Store new open view value.
    
   // If folder is open, then update its caption and scrollbars...                                         
   if (isOpen()==TRUE)
   {                                         
      // Update folder's caption text title.
      SetWindowText(GetHandle(),GetCaptionTitle());
   
      // If folder has scrollable property for client space...
      if (isScrollable()==TRUE)
      {
         // Recalculate folder's logical domain.
         // Get the current logical domain rect.
         CalculateLogicalDomainRect(&rcLogDomain);
         // Set the logical domain rect.
         SetLogicalDomain(&rcLogDomain);
         // Arrange scrollbars.
         ArrangeScrollbars();
      }
   }
}


//---------------------------------------------------------------------
// INITIALIZATION FUNCTION (CALLED AFTER OnCreate())
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnInitializeFolder():
//    This member function is implemented so that derived classes
// can overload this function and add special initialization
// functionality when a folder object is opened.  This function is
// called when the WM_INITIALIZEFOLDER message is received right after
// a WM_CREATE message.
//---------------------------------------------------------------------
void WPFOLDER::OnInitializeFolder(void)
{
   // Does nothing.
}


//---------------------------------------------------------------------
// APPEARANCE ATTRIBUTES SET & QUERY FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetBackgroundColor():
//    This function stores a color reference as the current background
// color for folder.  See GetBackgroundColor() call for more info.
//---------------------------------------------------------------------
void WPFOLDER::SetBackgroundColor(COLORREF clrrefBackground)
{
   HBRUSH   hbrBackground;

   // Store new color reference value.
   m_clrrefBackground = clrrefBackground;
   // Check if folder is not open, then exit (there is no window created).
   if (isOpen()==FALSE)
      return;
   // Create new solid brush using new color reference value.
   hbrBackground = CreateSolidBrush(m_clrrefBackground);
   // Check that brush is valid.
   if (hbrBackground==NULL)
      return;
   // Change the current class brush and delete previous class brush.
   ::DeleteObject((HBRUSH)SetClassWord(GetHandle(),GCW_HBRBACKGROUND,(WORD)hbrBackground));
}

//---------------------------------------------------------------------
// GetBackgroundColor():
//    This function returns the color reference value of the folder's
// client area background.  This value is of the type COLORREF which
// is actually an RGB triplet with a 0,1, or 2 in the MSB.  This 0,1,
// or 2 determines what type of a color is produced by Windows GDI when
//  a brush is created (please refer to Windows 3.1 SDK docs for info).
//---------------------------------------------------------------------
COLORREF WPFOLDER::GetBackgroundColor(void)
{
   return (m_clrrefBackground);
}

//---------------------------------------------------------------------
// SetIconTextLogFont():
//---------------------------------------------------------------------
void WPFOLDER::SetIconTextLogFont(LOGFONT *plogfontIconText)
{
   // Store attributes of logical font structure internally.
   m_lfIconText = *plogfontIconText;
}

//---------------------------------------------------------------------
// GetIconTextLogFont():
//---------------------------------------------------------------------
void WPFOLDER::GetIconTextLogFont(LOGFONT *plogfontIconText)
{
   // Return copy of current logical font information for icon text.
   *plogfontIconText = m_lfIconText;
}

//---------------------------------------------------------------------
// SetIconTextColor():
//---------------------------------------------------------------------
void WPFOLDER::SetIconTextColor(COLORREF clrrefColor)
{
   // Store color reference value for icon title text color.
   m_clrrefIconTextColor = clrrefColor;
}

//---------------------------------------------------------------------
// GetIconTextColor():
//---------------------------------------------------------------------
COLORREF WPFOLDER::GetIconTextColor(void)
{
   // Return icon title text color.
   return (m_clrrefIconTextColor);
}
   
//---------------------------------------------------------------------
// GetImageAttribute():
//---------------------------------------------------------------------
BOOL WPFOLDER::GetImageAttribute(DWORD dwAttributeFlag)
{
   return ((GetFlags()&IMAGE_FLAGS_MASK)==dwAttributeFlag);
}
   
   
//---------------------------------------------------------------------
// SetImageAttribute():
//---------------------------------------------------------------------
void WPFOLDER::SetImageAttribute(DWORD dwAttributeFlag)
{
   SetFlagsOR(dwAttributeFlag,IMAGE_FLAGS_MASK);
}

   
//---------------------------------------------------------------------
// GetImagePath(): 
//---------------------------------------------------------------------
char *WPFOLDER::GetImagePath(void)
{          
   // Return the current bitmap image file path.
   return (m_szImagePath);      
}

//---------------------------------------------------------------------
// SetImagePath():
//---------------------------------------------------------------------
void WPFOLDER::SetImagePath(char *pszImagePath)
{
   strcpy(m_szImagePath,pszImagePath);
}

//---------------------------------------------------------------------
// GetImagePalette():
//---------------------------------------------------------------------
HPALETTE WPFOLDER::GetImagePalette(void)
{
   return (m_hpalImage);
}          

//---------------------------------------------------------------------
// SetImagePalette(): 
//---------------------------------------------------------------------
void WPFOLDER::SetImagePalette(HPALETTE hpalImage)
{
   m_hpalImage = hpalImage;
}

//---------------------------------------------------------------------
// GetImageBitmapHandle():
//---------------------------------------------------------------------
HBITMAP WPFOLDER::GetImageBitmapHandle(void)
{
   return (m_hbmpImage);
}          

//---------------------------------------------------------------------
// SetImageBitmapHandle(): 
//---------------------------------------------------------------------
void WPFOLDER::SetImageBitmapHandle(HBITMAP hbmpImage)
{
   m_hbmpImage = hbmpImage;
}



//---------------------------------------------------------------------
// COLOR SELECTION FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetDefaultColors():
//---------------------------------------------------------------------
void WPFOLDER::SetDefaultColors(HDC hdc,WPOBJECT *pwpobjDrawn)
{
   // If object is currently selected in folder...
   if (isObjSelected(pwpobjDrawn)==TRUE)
   {
      // Use "selected" background attributes.
      SetBkColor(hdc,RGB(128,128,128));
      // Set the icon text color for an object which is selected.
      SetTextColor(hdc,RGB(255,255,255));
   }
   else
   // ...Else, if object is a "shadow" object...
   if ((pwpobjDrawn->GetType()&OBJTYPE_SHADOW)==OBJTYPE_SHADOW)
   {                     
      // Use "normal" window background attributes.
      SetBkColor(hdc,GetBackgroundColor());
      // Set the icon text color for a shadow object.
      SetTextColor(hdc,g_wpEnvironment.clrrefShadowTextColor);
   }
   // ...Else, set normal folder colors.
   else
   {
      // Use "normal" window background attributes.
      SetBkColor(hdc,GetBackgroundColor());
      // Set the icon text color for an object which is not selected.
      SetTextColor(hdc,GetIconTextColor());
   }
}

//---------------------------------------------------------------------
// SetFrameColors():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::SetFrameColors(HDC hdc,WPOBJECT *pwpobjDrawn)
{
   // Set the text icon caption text color for an object which is not in focus.
   SetTextColor(hdc,RGB(255,255,255));
   // Use "normal" window background attributes.
   SetBkColor(hdc,GetBackgroundColor());
}



//---------------------------------------------------------------------
// CHILDREN OBJECT'S VISUAL REPRESENTATION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetFirstAvailableIconPos():
//    This function is used to get the next icon position in a folder
// which we assume to be ordered in left->right and top->bottom
// ordering.
//---------------------------------------------------------------------
POINT WPFOLDER::GetFirstAvailableIconPos(POINT *pptStart)
{
   POINT    ptIcon;
   SIZE     sizeIconSpacing;
   RECT     rcObject,rcNewObject,rcIntersect;
   WORD     wOriginalOpenView;
   int      index,nMaxWidth;

   // Initialize local variable (for convenience and speed).
   sizeIconSpacing.cx = GetSystemMetrics(SM_CXICONSPACING);
   sizeIconSpacing.cy = GetSystemMetrics(SM_CYICONSPACING);

   // If folder is in the open state, get current width...
   if (isOpen()==TRUE)
   {
      nMaxWidth = (GetWndSize().cx - sizeIconSpacing.cx);
      // Make sure that our minimum width is met.
      if (nMaxWidth < sizeIconSpacing.cx)
         nMaxWidth = sizeIconSpacing.cx;
   }
   else
      // Else, approximate by using half the current screen size.
      nMaxWidth = (GetSystemMetrics(SM_CXSCREEN)/2);

   // If starting point is "don't care", then...
   if ((pptStart==NULL)||(pptStart->x==-1)||(pptStart->y==-1))
   {
      // Initialize starting point to be the top-left corner of folder.
      ptIcon.x = g_wpEnvironment.sizeIconIndent.cx;
      ptIcon.y = g_wpEnvironment.sizeIconIndent.cy;
   }
   else
      // Else, initialize starting point to be the point passed to the function.
      ptIcon = *pptStart;

   // Set default view to icon view so that subsequent calls to get
   // the objects' rectangular area are in "icon view" coordinates.
   wOriginalOpenView = m_wOpenView;
   m_wOpenView = VIEW_ICON;

   // While we have not found an available icon position...
   while (TRUE)
   {
      // Create current icon position rectangle (using just the icon size).
      SetRect(&rcNewObject,ptIcon.x,ptIcon.y,
              (ptIcon.x+GetSystemMetrics(SM_CXICON)),
              (ptIcon.y+GetSystemMetrics(SM_CYICON)) );
      // Test current icon position against all objects in folder...
      for (index=0; index < GetObjectCount(); index++)
      {
         // Get current object's total area (icon + title)
         GetObject(index)->GetObjectRect(&rcObject);
         // Test current position's area against current object's area.
         if (IntersectRect(&rcIntersect,&rcObject,&rcNewObject)!=FALSE)
            // Icon position is already "taken" by current object,
            // exit loop and calculate next icon position.
            break;
      }

      // Check if current icon position is not "taken" by any object in folder.
      if (index>=GetObjectCount())
      {
         // Before leaving, restore original folder view value.
         m_wOpenView = wOriginalOpenView;
         // Return current icon position since it is available.
         return (ptIcon);
      }

      // ...Else, calculate next icon position.
      ptIcon.x += sizeIconSpacing.cx;
      // If horizontal position of icon is greater than the maximum width allowed...
      if (ptIcon.x > nMaxWidth)
      {
         // Move current icon position to the begining of next row.
         ptIcon.x  = g_wpEnvironment.sizeIconIndent.cx;
         ptIcon.y += sizeIconSpacing.cy;
      }
   }
}

//---------------------------------------------------------------------
// UpdateIcon():
//    This function is an overloaded member function from the WPOBJECT
// class.  Since WPFOLDER objects need some extra work to update their
// icon resource (namely update the "opened" folder class icon value),
// we have overloaded it and do the extra work here.
//---------------------------------------------------------------------
void WPFOLDER::UpdateIcon(char *pszIconPath,WORD wIconIndex)
{
   HICON hOldIcon;

   // Have default function handle changing the icon that appears on
   // the parent container's client area.
   WPOBJECT::UpdateIcon(pszIconPath,wIconIndex);

   // Now, we need to update the opened folder's class icon value.
   if (isOpen()==TRUE)
   {
      // In one operation (in order of execution):
      // (1) Load new instance of icon resource.
      // (2) copy new icon resource handle into class word value.
      // (3) retrieve and save old icon resource handle.
      hOldIcon = (HICON)SetClassWord(GetHandle(),GCW_HICON,(WORD)ExtractIcon());

      // If old icon resource handle is valid, then destroy old icon resource.
      if ((hOldIcon!=NULL)||(hOldIcon!=(HICON)1))
         ::DestroyIcon(hOldIcon);

      // Repaint folder window's non-client area.
      OnNCPaint();
   }
}


//---------------------------------------------------------------------
// LoadIcons():
//---------------------------------------------------------------------
BOOL WPFOLDER::LoadIcons(void)
{                  
   HCURSOR  hcurOriginal;
   BOOL     fResult;
   int      index;         

   // Initialize return result to success.
   fResult = TRUE;                                                       
   
   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
   
   // Have each object in folder load its own icon.
   for (index=0; index<GetObjectCount(); index++)
      if (GetObject(index)->LoadIcon()==FALSE)
      {           
         // Failed loading icon, set result to failure.
         fResult = FALSE;                              
         // Stop loading rest of icons.
         break;
      } 
   
   // Check if original cursor was not the same as our "wait" cursor...      
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
   
   // Return TRUE or FALSE.
   return (fResult);
}

//---------------------------------------------------------------------
// DestroyIcons():
//---------------------------------------------------------------------
BOOL WPFOLDER::DestroyIcons(void)
{
   int   index;
   BOOL  fSuccess=TRUE;

   // Have each object in folder destroy its own icon resource.
   for (index=0; index<GetObjectCount(); index++)
      if (GetObject(index)->DestroyIcon()==FALSE)
         fSuccess = FALSE;

   // Return TRUE or FALSE.
   return (fSuccess);
}

//---------------------------------------------------------------------
// DRAG & DROP SUPPORT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ChildHitTest():
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::ChildHitTest(POINT ptDevice)  // device point.
{
   int   index;
   POINT ptLogical;

   // Convert device point into logical point.
   ptLogical = ptDevice;
   DPtoLP(&ptLogical);

   // First check if logical point is not visible within folder.
   if (isPointVisible(ptLogical)==FALSE)
      return (NULL);    // Point not visible.

   // Test if device point "hits" any child objects in folder.

   // We check child array from right to left because that is the
   // z-order convention (right most is top most).
   for (index=(GetObjectCount()-1); index>=0; index--)
   {
      // Child object has found a "hit".
      if (GetObject(index)->HitTest(ptLogical)==TRUE)
         return (GetObject(index));    // Return child object's pointer.
   }

   // No child objects were "hit", so return none.
   return (NULL);
}


//---------------------------------------------------------------------
// GetChildrenContainedInRect():
//---------------------------------------------------------------------
int WPFOLDER::GetChildrenContainedInRect(RECT rcLogical,LIST *plistHit)  // logical point.
{
   int index;

   // Test if any child objects are contained inside logical rectangle.

   // We check child array from right to left because that is the
   // z-order convention (right most is top most).
   for (index=(GetObjectCount()-1); index>=0; index--)
   {
      // Child object has found a "hit".
      if (GetObject(index)->ContainedInRect(&rcLogical)==TRUE)
         // Store object pointer since it is contained inside rectangle.
         plistHit->AddTail((DWORD)GetObject(index));         
   }

   // Return the number of child objects that were contained inside rectangle.
   return (plistHit->GetCount());
}


//---------------------------------------------------------------------
// DropDptoLP():
//---------------------------------------------------------------------
void WPFOLDER::DropDPtoLP(LPPOINT lpptDevice)
{
   // Check if device point is located on top of our own window.
   if (GetHandle()==WindowFromPoint(*lpptDevice))
   {
      // Do a logical to device coordinate convertion.
      DPtoLP(lpptDevice);
      // Offset logical point to compensate for the cursor "hotspot".
      lpptDevice->x -= ((GetSystemMetrics(SM_CXICON)/2)-1);
      lpptDevice->y -= ((GetSystemMetrics(SM_CYICON)/2)-1);
   }
   else
   {
      // Because the drop point was not on top of our window client area,
      // we must calculate a reasonable drop point within the client area.
      // The method is:
      //    If there is at least one object in folder,
      //       Then get icon point for last object in folder.
      //       Add horiz. spacing to it and check for invalid point.
      //    Else
      //       Start object at top-left of client area.

      // Check if there is at least 1 object in folder.
      if (GetObjectCount()>0)
      {
         // Get last object's icon coordinates.
         *lpptDevice = GetObject(GetObjectCount()-1)->GetIconPos();
         lpptDevice->x += GetSystemMetrics(SM_CXICONSPACING);
         // If x-coordinate is invalid (less than zero)
         if ((lpptDevice->x < 0)||
             ((lpptDevice->x + GetSystemMetrics(SM_CXICONSPACING)) < 0))
         {
            // Move object coordinates to a new row.
            lpptDevice->x  = g_wpEnvironment.sizeIconIndent.cx;
            lpptDevice->y += g_wpEnvironment.sizeIconIndent.cy;
         }
      }
      else
      {
         // No objects in folder, so start object at top-left corner.
         lpptDevice->x = g_wpEnvironment.sizeIconIndent.cx;
         lpptDevice->y = g_wpEnvironment.sizeIconIndent.cy;
      }
   }
}


//---------------------------------------------------------------------
// MOUSE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateDragCursors():
//---------------------------------------------------------------------
void WPFOLDER::CreateDragCursors(WORD wDragFunction,HCURSOR *phcurDrag,HCURSOR *phcurNoDrag)
{
   // Depending on the drag function being performed...
   switch (wDragFunction)
   {
      case DRAG_SELECT:
         // Load default arrow cursor for both cases.
         *phcurDrag   = LoadCursor(NULL,IDC_ARROW);
         *phcurNoDrag = LoadCursor(NULL,IDC_ARROW);
         break;

      case DRAG_MOVE:
         // Create a cursor from selected object(s) icon(s).
         CreateMoveCursors(phcurDrag,phcurNoDrag);
         break;

      case DRAG_COPY:
         // Create a cursor from selected object(s) icon(s).
         CreateCopyCursors(phcurDrag,phcurNoDrag);
         break;

      case DRAG_SHADOW:
         // Create a cursor from selected object(s) icon(s).
         CreateShadowCursors(phcurDrag,phcurNoDrag);
         break;
   }
}



//---------------------------------------------------------------------
// OnMouseMessage():
//---------------------------------------------------------------------
void WPFOLDER::OnMouseMessage(UINT uMsg,WORD wFlags,POINT ptWindow)
{
   POINT ptDevice;

   // Point passed in mouse messages are in window coordinates (WP).
   // Therefore, we must convert this point to device coordinates.
   ptDevice = ptWindow;
   WPtoDP(&ptDevice);   // convert from Window point to Device point.

   // Process mouse message.
   switch (uMsg)
   {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
         OnLeftMouseButton(uMsg,wFlags,ptDevice);
         break;

      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
         OnRightMouseButton(uMsg,wFlags,ptDevice);
         break;

      case WM_LBUTTONDBLCLK:
         OnLeftMouseButtonDblClk(ptDevice);
         break;

      case WM_MOUSEMOVE:
         OnMouseMove(wFlags,ptDevice);
         break;
   }
}


//---------------------------------------------------------------------
// CONFIRMATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ConfirmOnDelete():
//---------------------------------------------------------------------
int WPFOLDER::ConfirmOnDelete(void)
{
   char  szConfirmText[256];

   // If "confirm on delete" system flag is set...
   if ((g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)||
       (g_wpEnvironment.stSystem.fConfirmOnFolderDelete==TRUE)||
       (g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete==TRUE))
   {                             
      // Check if parent container of this object is a directory folder...
      if ((GetParent()!=NULL)&&(GetParent()->isDirectoryContainer()==TRUE))
         // Build special confirmation string with object's title text.
         sprintf(szConfirmText,GetString(IDSTR_FOL_CONFIRM_ON_FILESYS_DELETE),GetTitle());
      else           
      {
         // Since it is not a file system delete, make sure user wants to confirm delete...
         if ((g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)||
             (g_wpEnvironment.stSystem.fConfirmOnFolderDelete==TRUE))
            // Build normal confirmation string with object's title text.
            sprintf(szConfirmText,GetString(IDSTR_FOL_CONFIRM_ON_DELETE),GetTitle());
         else              
            // Automatically confirm delete operation...
            return (IDYES);
      }
      // Display message box with confirmation text.
      return (MessageBox(NULL,szConfirmText,GetString(IDSTR_FOL_CONFIRM_ON_DELETE_CAPTION),(MB_YESNOCANCEL|MB_ICONEXCLAMATION)));
   }
   else
      // Automatically confirm delete operation...
      return (IDYES);
}


//---------------------------------------------------------------------
// DRAWING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawChildren():
//---------------------------------------------------------------------
void WPFOLDER::DrawChildren(HDC hdc)
{                              
   HFONT hfontOld;
   int   index,iPrevMap,iPrevBk;   
   
   // Setup device context.
   SetUpDC(hdc,&hfontOld,&iPrevBk,&iPrevMap);
      
   // Draw children objects on folder's client area.
   for (index=0; index<GetObjectCount(); index++)
      GetObject(index)->Draw(hdc);
      
   // Clean-up device context before leaving.
   CleanUpDC(hdc,hfontOld,iPrevBk,iPrevMap);
}


//---------------------------------------------------------------------
// DrawDetailsViewTable():
//---------------------------------------------------------------------
void WPFOLDER::DrawDetailsViewTable(HDC hdc)
{
   HPEN        hpenDarkGray,hpenOld;
   POINT       ptLine[2];
   RECT        rcClient,rcText;
   COLORREF    clrrefTextOld;
   int         iOldBkMode;

   // Create dark gray pen used in drawing table lines.
   hpenDarkGray = CreatePen(PS_SOLID,1,RGB(128,128,128));

   // Select created dark gray pen into device context (and save original pen).
   hpenOld = (HPEN)SelectObject(hdc,hpenDarkGray);

   // Get folder's visible rect.
   rcClient = GetVisibleArea();

   // Start by drawing the table columns (vertical lines):
   // Line between "Icon" and "Title".
   ptLine[0].x = DV_TITLE_INDENT_CX;
   ptLine[0].y = 0;
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = rcClient.bottom;
   Polyline(hdc,&(ptLine[0]),2);

   // Line between "Title" and "Name".
   ptLine[0].x = DV_NAME_INDENT_CX;
   ptLine[0].y = 0;
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = rcClient.bottom;
   Polyline(hdc,&(ptLine[0]),2);

   // Line between "Name" and "Size".
   ptLine[0].x = DV_SIZE_INDENT_CX;
   ptLine[0].y = 0;
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = rcClient.bottom;
   Polyline(hdc,&(ptLine[0]),2);

   // Line between "Size" and "Date".
   ptLine[0].x = DV_DATE_INDENT_CX;
   ptLine[0].y = 0;
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = rcClient.bottom;
   Polyline(hdc,&(ptLine[0]),2);

   // Line between "Date" and "Time".
   ptLine[0].x = DV_TIME_INDENT_CX;
   ptLine[0].y = 0;
   ptLine[1].x = ptLine[0].x;
   ptLine[1].y = rcClient.bottom;
   Polyline(hdc,&(ptLine[0]),2);

   // Now, draw the table horizontal lines:
   ptLine[0].x = 0;
   ptLine[0].y = DV_INDENT_CY-1;
   ptLine[1].x = rcClient.right;
   ptLine[1].y = ptLine[0].y;
   Polyline(hdc,&(ptLine[0]),2);

   // Clean-up before leaving.
   SelectObject(hdc,hpenOld);
   ::DeleteObject(hpenDarkGray);

   // Draw table cloumns' text.

   // Set current text color to black and current background mode to transparent.
   clrrefTextOld = SetTextColor(hdc,RGB(0,0,0));
   iOldBkMode    = SetBkMode(hdc,TRANSPARENT);

   // "Icon" column.
   SetRectCXCY(&rcText,DV_INDENT_CX,0,(DV_TITLE_INDENT_CX-1),(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_ICON_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // "Title" column.
   SetRectCXCY(&rcText,DV_TITLE_INDENT_CX,0,DV_TITLE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_TITLE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // "Name" column.
   SetRectCXCY(&rcText,DV_NAME_INDENT_CX,0,DV_NAME_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_NAME_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // "Size" column.
   SetRectCXCY(&rcText,DV_SIZE_INDENT_CX,0,DV_SIZE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_SIZE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // "Date" column.
   SetRectCXCY(&rcText,DV_DATE_INDENT_CX,0,DV_DATE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_DATE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // "Time" column.
   SetRectCXCY(&rcText,DV_TIME_INDENT_CX,0,DV_TIME_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_TIME_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER));

   // Clean-up before leaving.
   SetBkMode(hdc,iOldBkMode);
   SetTextColor(hdc,clrrefTextOld);
}


//---------------------------------------------------------------------
// PopupFrame():
//    This function draws a rounded rectangle around the object.
// It uses the XOR boolean operation to combine pen colors with
// background colors in the device context.  This facilitates the
// "unframing" process since we just apply the XOR operation again
// to remove the rectangle and restore the original colors.
//---------------------------------------------------------------------
void WPFOLDER::PopupFrame(void)     // draws rounded rect. w/dots.
{
   HDC     hdc;
   HPEN    hpenSelect,hpenOld;
   HBRUSH  hbrOld;
   RECT    rcArea;
   int     iPrevROP2;

   // Check if we must "frame" the folder window client area...
   if (((GetState()&OBJSTATE_TRACKPOPUPMENU)==OBJSTATE_TRACKPOPUPMENU)&&
       (GetObjectPopup()==this)&&(isOpen()==TRUE))
   {
      // We need to draw a rounded rectangle on the folder window client area.
      // Check that the client area doesn't already have popup frame drawn on it.
      if (!(GetState()&OBJSTATE_POPUPFRAME))
      {
         // Set "popup frame drawn" state bit.
         SetState(GetState() | OBJSTATE_POPUPFRAME);

         // Get folder's client area device context.
         hdc = GetDC();

         // Have the parent container object set the default colors for a selected object.
         SetFrameColors(hdc,this);

         // Create a pen with alternating dots in a gray shade.
         hpenSelect = CreatePen(PS_DOT,1,GetTextColor(hdc));
         hpenOld    = (HPEN)SelectObject(hdc,(HGDIOBJ)hpenSelect);
         // Select the NULL brush since we do not want to fill in rectangle.
         hbrOld     = (HBRUSH)SelectObject(hdc,(HGDIOBJ)GetStockObject(NULL_BRUSH));

         // Get total object area rectangle.
         GetClientRect(GetHandle(),&rcArea);

         // Shrink rectangle by 1 pixel.
         InflateRect(&rcArea,-1,-1);

         // Convert window coord. to logical coord.
         ::DPtoLP(hdc,(LPPOINT)&rcArea,2);

         // Set XOR operation between pen object and device context background.
         iPrevROP2 = SetROP2(hdc,R2_XORPEN);

         // Draw rounded rectangle using current pen and brush.
         RoundRect(hdc,
                   rcArea.left,rcArea.top,rcArea.right,rcArea.bottom,
                   (int)((rcArea.right-rcArea.left)*FRAME_ROUND_RATIO),
                   (int)((rcArea.bottom-rcArea.top)*FRAME_ROUND_RATIO));

         // Restore original raster operation to device context.
         SetROP2(hdc,iPrevROP2);

         // Clean-up and restore original pen, and brush objects to DC.
         SelectObject(hdc,(HGDIOBJ)hbrOld);
         SelectObject(hdc,(HGDIOBJ)hpenOld);
         ::DeleteObject(hpenSelect);

         // Return client area's device context.
         ReleaseDC();
      }
   }
   else
   // ...Or, the folder object's icon (on its parent's client area).
      WPOBJECT::PopupFrame();
}


//---------------------------------------------------------------------
// PopupUnFrame():
//    This function removes the existing popup frame rectangle by drawing
// it again with the XOR boolean operation.
//---------------------------------------------------------------------
void WPFOLDER::PopupUnFrame(void)   // removes popup frame rect.
{
   WPOBJECT::PopupUnFrame();
}


//---------------------------------------------------------------------
// Frame():
//    This function draws a frame rectangle around the object.
// It uses the XOR boolean operation to combine pen colors with
// background colors in the device context.  This facilitates the
// "unframing" process since we just apply the XOR operation again
// to remove the rectangle and restore the original colors.
//---------------------------------------------------------------------
void WPFOLDER::Frame(BOOL fInside)  // draws highlight rectangle.
{
   HDC     hdc;
   HPEN    hpenSelect,hpenOld;
   RECT    rcArea;

   // Check if frame area is folder's client area or object icon.
   if (fInside==FALSE)
      WPOBJECT::Frame(fInside);
   else
   {
      // Check if object is already framed.
      if ((GetState()&OBJSTATE_FRAME)==OBJSTATE_FRAME)
         return;

      // Get folder's client area device context.
      hdc = GetDC();

      // We need to draw a framed rectangle around folder's client area.
      // First, we need to set the selected color.

      // Have the parent container object set the default colors for a framed object.
      SetFrameColors(hdc,this);

      // Create a pen with alternating dots in a gray shade.
      hpenSelect = CreatePen(PS_SOLID,1,GetTextColor(hdc));
      hpenOld    = (HPEN)SelectObject(hdc,(HGDIOBJ)hpenSelect);

      // Get total object area rectangle.
      GetClientRect(GetHandle(),&rcArea);

      // Shrink rectangle by 1 pixel.
      InflateRect(&rcArea,-2,-2);

      // Convert window coord. to logical coord.
      ::DPtoLP(hdc,(LPPOINT)&rcArea,2);

      // Draw frame rectangle using current pen and XOR raster operation.
      DrawFocusRect(hdc,&rcArea);

      // Clean-up and restore original pen, and brush objects to DC.
      SelectObject(hdc,(HGDIOBJ)hpenOld);
      ::DeleteObject(hpenSelect);

      // Set frame state bit.
      SetState((GetState()|OBJSTATE_FRAME));

      // Release folder's client area device context.
      ReleaseDC();
   }
}


//---------------------------------------------------------------------
// UnFrame():
//    This function removes the existing frame rectangle by drawing
// it again with the XOR boolean operation.
//---------------------------------------------------------------------
void WPFOLDER::UnFrame(BOOL fInside)  // removes frame rect.
{
   WPOBJECT::UnFrame(fInside);
}


//---------------------------------------------------------------------
// InvalidateRect():
//    This function invalidates a rectangular area on the folder's
// device context.  The parameter "lprcArea" points to a rectangle
// which contains the invalid folder area in logical units (relative
// to the device context).  We must convert these points to window
// (or client) units before passing them to the Windows 3.1 API
// function "InvalidateRect()".
//---------------------------------------------------------------------
void WPFOLDER::InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Check that folder is open...
   if (isOpen()==TRUE)
   {
      // Convert invalid area from logical coord. to window (client) coord.
      if (lprcArea!=NULL)
         LPtoWP((LPPOINT)lprcArea,2);
      // Invalidate rectangular area in window.
      ::InvalidateRect(GetHandle(),lprcArea,fEraseBkgrnd);
      // Check if window needs to be updated right away.
      if (fUpdateNow==TRUE)
         ::UpdateWindow(GetHandle());
   }
}


//---------------------------------------------------------------------
// Invalidate():
//    This function invalidates the "opened" object area (if any). The
// behavior of this function is to invalidate the whole client area
// of folder if opened.
//---------------------------------------------------------------------
void WPFOLDER::Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Call the default invalidation function for a rectangular area.
   InvalidateRect(NULL,fUpdateNow,fEraseBkgrnd);
}


//---------------------------------------------------------------------
// UpdateWindow():
//    This function calls the Windows 3.1 API window update function
// to redraw the invalid region's contents.
//---------------------------------------------------------------------
void WPFOLDER::UpdateWindow(void)
{
   // Visually update folder window right now.
   ::UpdateWindow(GetHandle());
}


//---------------------------------------------------------------------
// WORKPLACE SHELL (WPS) WINDOW MESSAGES MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnDrop():
//    This function handles the WM_WPS_DROP message.  Regardless of
// what drag function was selected, it will release the mouse capture,
// restore the original mouse cursor, and destroy the custom cursors.
//---------------------------------------------------------------------
void WPFOLDER::OnDrop(POINT ptDevice)
{
   int      nSelectedCount,index;
   WPOBJECT *pwpobjDropSource,*pwpobjDropTarget,
            **apwpobjSelected;

   // Restore original mouse cursor.
   SetCursor(g_wpEnvironment.hOriginalCursor);
   // Release mouse capture.
   ReleaseCapture();
   // Destroy icon of object(s) dropped.
   DestroyCursor(g_wpEnvironment.hDragCursor);
   DestroyCursor(g_wpEnvironment.hNoDragCursor);

   // Play system event sound for "End Drag" event.
   PlaySystemEventSound("WPEndDrag");

   // If drop mode is DROP_SHADOW, remove current shadow drag line.
   if (m_wDragFunction==DROP_SHADOW)
      DrawShadowDragLine(ptDevice,TRUE);

   // Unframe and reset drop target variable used for framing selection.
   if (m_pwpobjDropTarget!=NULL)
   {
      m_pwpobjDropTarget->UnFrame(m_fTargetIsFolderArea);
      m_pwpobjDropTarget = NULL;
   }

   // Handle drop action.

   // First, get drop target object.
   pwpobjDropTarget = GetDropTarget(ptDevice,&m_fTargetIsFolderArea);
   // Check if drop target is valid and supported by all sources.
   if (isTargetValid(pwpobjDropTarget)==TRUE)
   {
      // Convert the mouse device point to target object's logical point.
      pwpobjDropTarget->DropDPtoLP(&ptDevice);

      // Check if dragged objects where selected objects...
      if (m_fDragSelectedObjs==TRUE)
      {
         // Get number of currently selected objects.
         nSelectedCount = GetSelectedObjectCount();
         // Allocate enough memory to hold list of all selected objects.
         apwpobjSelected = new WPOBJECT *[nSelectedCount];
         // Get list of selected objects, and store them in local array...
         for (index=0; index<nSelectedCount; index++)
            apwpobjSelected[index] = GetSelectedObject(index);

         // Clear list of selected objects to "none" (do not invalidate them).
         ClearSelectedObjects(TRUE);

         // Drop target is valid, so start "dropping" sources into target.
         for (index=0; index<nSelectedCount; index++)
         {
            // Store current object's pointer into local variable.
            pwpobjDropSource = apwpobjSelected[index];
            // "Drop" source object into target object.
            if (pwpobjDropSource->DropSource(pwpobjDropTarget,m_wDragFunction,ptDevice)==FALSE)
            {
               // Drop operation was not performed.
               //pwpobjDropSource = NULL;
            }
            else
               // Update next object's position (this way objects do not stack).
               ptDevice.x += GetSystemMetrics(SM_CXICONSPACING);

            // Check if we need to wrap to next line.  the complete object icon
            // must be visible within the current screen resolution.
            if (ptDevice.x>=(GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON)))
            {
               // Start position on next icon row.
               ptDevice.x  = g_wpEnvironment.sizeIconIndent.cx;
               ptDevice.y += GetSystemMetrics(SM_CYICONSPACING);
            }
         }
         // Deallocate memory used to hold copy of selected objects.
         delete[] apwpobjSelected;
         // Update source folder right now.
         UpdateWindow();
      }
      else
      {
         // Dragged object was not in the selected list.
         // "Drop" source object into target object.
         m_pwpobjDrag->DropSource(pwpobjDropTarget,m_wDragFunction,ptDevice);
      }
   }
}

//---------------------------------------------------------------------
// OBJECTS POSITION CALCULATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FindObjectAtRelativePosition():
//---------------------------------------------------------------------
WPOBJECT *WPFOLDER::FindObjectAtRelativePosition(WORD wEventID,WPOBJECT *pwpobjCurrent)
{
   WPOBJECT *pwpobjRelative;
   int      index;

   // If no object is currently selected,
   if (pwpobjCurrent==NULL)
      // Then, return first object in folder, or "none" if folder is empty.
      return (((GetObjectCount()>0)? GetObject(0) : NULL));
   // Initialize current index to "no object".
   index = -1;         
   // Depending on which position event is passed...
   switch (wEventID)
   {
      case ID_EVENT_PREVIOUS:
         index = max((GetObjectIndex(pwpobjCurrent)-1),0);
         break;
         
      case ID_EVENT_NEXT:
         index = min((GetObjectIndex(pwpobjCurrent)+1),(GetObjectCount()-1));
         break;
         
      case ID_EVENT_PAGEDOWN:
         // Scroll window's visible area down.
         PostMessage(GetHandle(),WM_VSCROLL,SB_PAGEDOWN,0L);
         break;          
            
      case ID_EVENT_PAGEUP:                  
         // Scroll window's visible area up.
         PostMessage(GetHandle(),WM_VSCROLL,SB_PAGEUP,0L);
         break;
      
      default:
         // Initialize pointer to object.
         pwpobjRelative = NULL;
         // Depending on current open view...
         switch (GetOpenView())
         {
            case VIEW_ICON:
               pwpobjRelative = IconViewFindObject(wEventID,pwpobjCurrent);
               break;
            
            case VIEW_DETAILS:        
               pwpobjRelative = DetailsViewFindObject(wEventID,pwpobjCurrent);
               break;
         }
         // Return found object relative to passed position.
         return (pwpobjRelative);      
   }      
         
   // Return object found or "no object" at all.
   return ((index<0)? NULL : GetObject(index));
}

//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPFOLDER::InitializePopup(HMENU hmenuPopup)
{
   WORD  wMenuID;
   HMENU hmenuSubPopup;

   // Set the current folder open view with a check mark.
   switch (GetOpenView())
   {  
      default:
      case VIEW_ICON:
         wMenuID = IDM_OPENICON;
         break;
         
      case VIEW_DETAILS:
         wMenuID = IDM_OPENDETAILS;
         break;
   }
   // Get "open" popup menu.
   hmenuSubPopup = GetSubMenu(hmenuPopup,0);
   // Check menu item in "open" popup menu.
   CheckMenuItem(hmenuSubPopup,wMenuID,(MF_BYCOMMAND|MF_CHECKED));
                                         
   // Call base class function.                                         
   WPCONTAINER::InitializePopup(hmenuPopup);
}


//---------------------------------------------------------------------
// GetMenuName():
//    This function returns a null-terminated string with the name of
// the menu resource name used as the system menu for this folder.
//---------------------------------------------------------------------
const char *WPFOLDER::GetMenuName(void)
{
   // Return the default generic object menu.
   return (MAKEINTRESOURCE(WPFOLDER_SYSMENU_NAME));  
}
   
         
//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPFOLDER::MenuSelection(WORD wMenuItem)
{
   int index;

   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_OPENICON:
         // Check if current view is not equal to chosen view.
         if (GetOpenView()!=VIEW_ICON)
         {
            // Reset folder's open view.
            SetOpenView(VIEW_ICON);
            // Invalidate folder's client area and update it now.
            Invalidate(TRUE,TRUE);
         }
         // Open view (if already open, make active window with input focus).
         Open();
         break;

      case IDM_OPENDETAILS:
         // Check if current view is not equal to chosen view.
         if (GetOpenView()!=VIEW_DETAILS)
         {
            // Reset folder's open view.
            SetOpenView(VIEW_DETAILS);
            // Invalidate folder's client area and update it now.
            Invalidate(TRUE,TRUE);
         }
         // Open view (if already open, make active window with input focus).
         Open();
         break;

      case IDM_OPENPARENT:      
         // Check if object has a valid parent...
         if (GetParent()!=NULL)                  
            // Open parent container object.
            GetParent()->Open();
         break;

      case IDM_REFRESHNOW:
         Invalidate(TRUE,TRUE);     // Repaint whole folder client area.
         break;

      case IDM_HELPGENERAL:
         // Open help file with "General help" for folders objects.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_FOLDER);
         break;
      
      case IDM_WINDOWRESTORE:
         // If folder is in open state, restore it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_RESTORE,0L);    // Restore window.
         break;

      case IDM_WINDOWMOVE:
         // If folder is in open state, move it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MOVE,0L);       // Move window.
         break;

      case IDM_WINDOWSIZE:
         // If folder is in open state, resize it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_SIZE,0L);       // Size window.
         break;

      case IDM_WINDOWMINIMIZE:
         // If folder is in open state, minimize it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MINIMIZE,0L);   // Minimize window.
         break;

      case IDM_WINDOWMAXIMIZE:
         // If folder is in open state, maximize it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MAXIMIZE,0L);   // Maximize window.
         break;

      case IDM_WINDOWHIDE:
         // If folder is in open state, minimize it.
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_SYSCOMMAND,SC_MINIMIZE,0L);   // Hide window.
         break;

      case IDM_SELECTALL:
         // Add all objects in folder to "selected" list.
         for (index=0; index<GetObjectCount(); index++)
            AddSelectedObject(GetObject(index));
         // Repaint the whole folder.
         Invalidate(TRUE,TRUE);
         break;
      
      case IDM_FIND:    
         // Find object(s) that match a user-specified criteria.
         FindFunction();
         break;                      
                      
      case IDM_DESELECTALL:
         // Clear all selected objects from list.
         ClearSelectedObjects(TRUE);
         break;
  
      case IDM_SORTBYNAME:
      case IDM_SORTBYTYPE:
      case IDM_SORTBYREALNAME:
      case IDM_SORTBYSIZE:
      case IDM_SORTBYLASTACCESSDATE:
      case IDM_SORTBYLASTACCESSTIME:
         // Sort icons by specified criteria and arrange icons to fit folder client area.
         SortIcons(wMenuItem);
         ArrangeIcons();
         break;
  
      case IDM_ARRANGE:    
         // Arrange icons and repaint whole folder client area.
         ArrangeIcons();   
         break;

      case IDM_CLOSE:
         // If folder is open...
         if (isOpen()==TRUE)
            PostMessage(GetHandle(),WM_CLOSE,0,0L);   // post message to close it.
         break;

      default:
         // Let base object handle the rest of menu selections.
         WPCONTAINER::MenuSelection(wMenuItem);
         break;
   }
}

//---------------------------------------------------------------------
// KeyboardEvent():
//---------------------------------------------------------------------
void WPFOLDER::KeyboardEvent(WORD wEventID)
{
   int      index;
   WPOBJECT *pwpobjSelected;

   // Depending on the ID_EVENT_* code...
   switch (wEventID)
   {
      case ID_EVENT_OPEN:
         // Open all currently selected objects (if any)...
         // While there exists a selected object.
         for (index=0; index<GetSelectedObjectCount(); index++)
            // Call object's virtual key handler.
            GetSelectedObject(index)->Open();
         break;

      case ID_EVENT_SETTINGS:
         // Open the settings notebooks of all currently selected objects (if any)...
         // While there exists a selected object.
         for (index=0; index<GetSelectedObjectCount(); index++)
            // Call object's virtual key handler.
            GetSelectedObject(index)->OpenSettingsNotebook();
         break;

      case ID_EVENT_DELETE:
         // Delete all selected objects in folder (if any).
         DeleteSelectedObjects();
         break;
      
      case ID_EVENT_PREVIOUS:
      case ID_EVENT_NEXT:
      case ID_EVENT_LEFT:
      case ID_EVENT_RIGHT:
      case ID_EVENT_UP:
      case ID_EVENT_DOWN:
      case ID_EVENT_PAGEUP:
      case ID_EVENT_PAGEDOWN:
         // Get object at relative position from first object selected.
         pwpobjSelected = FindObjectAtRelativePosition(wEventID,GetFirstSelectedObject());
         // If there is an object in folder that met the criteria...
         if (pwpobjSelected!=NULL)
         {
            // Clear all other selected objects.
            ClearSelectedObjects(TRUE);
            // Make found object the new selected object.
            AddSelectedObject(pwpobjSelected);
            // Update found object's visual state right now.
            pwpobjSelected->InvalidateIcon(TRUE,TRUE);
         }
         break;

      case ID_EVENT_SELECTALL:
         MenuSelection(IDM_SELECTALL);
         break;

      case ID_EVENT_DESELECTALL:
         MenuSelection(IDM_DESELECTALL);
         break;        
         
      case ID_EVENT_HELP:       
         // Open help file with "General help" for folders objects.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_FOLDER);
         break;
         
      case ID_EVENT_DESKTOP_POPUP:
         // User wants desktop popup menu.
         PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_POPUP_MENU,
                     0,MAKELPARAM(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)));
         break;
 
      default:
         MessageBeep(-1);
         break;
   }
}


//---------------------------------------------------------------------
// WINDOW MESSAGES SUPPORT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// HandleDragModeInit():
//---------------------------------------------------------------------
BOOL WPFOLDER::HandleDragModeInit(WORD wFlags,POINT ptDevice)
{
   WPOBJECT *pwpobjHit,*pwpobjTarget;
   BOOL     fTargetIsFolderArea;
   RECT     rcUserError;

   // We are not on drag & drop mode yet...
   // If right button is down, select object(s) for Drag & Drop, and start it.
   if ((m_wRightButtonState==MBS_DOWN)&&(wFlags&MK_RBUTTON))
   {
      // Calculate user rect. area where we assume mouse movement error.
      // The error area is a 3x3 rectangle. PtInRect() does not include
      // the right and bottom coord. in the check. therefore, we add an
      // extra 1 pel to the right and bottom rect. coordinates.
      SetRect(&rcUserError,m_ptMouseFirstClick.x-2,m_ptMouseFirstClick.y-2,
                           m_ptMouseFirstClick.x+3,m_ptMouseFirstClick.y+3);
      
      // Check if we are still within this error area...
      if (PtInRect(&rcUserError,ptDevice)==TRUE)
         // Return failure which means 'stop processing'.
         return (FALSE); // ...ignore move message and exit function.
      
      // Get child object selected for Drag & Drop.
      pwpobjHit = ChildHitTest(ptDevice);
      
      // If no objects were selected, exit.
      if (pwpobjHit==NULL)
         // Return failure which means 'stop processing'.
         return (FALSE);
      
      // If object hit was not in list of currently selected objects...
      if (isObjSelected(pwpobjHit)==FALSE)
      {
         // Make object being dragged the object "hit".
         m_fDragSelectedObjs  = FALSE;
         m_pwpobjDrag         = pwpobjHit;
      }
      else
      {
         // Make object(s) being dragged the selected objects.
         m_fDragSelectedObjs  = TRUE;
         // Store object "hit" by right mouse button.
         m_pwpobjDrag         = pwpobjHit;
      }
      
      // Initialize drag flags.
      m_fDragMode        = TRUE;
      m_pwpobjDropTarget = NULL;
      
      // Depending on which view mode this folder is in...
      switch (GetOpenView())
      {
         case VIEW_ICON:
            // Convert icon position of object hit from logical to device coord.
            m_ptDragLineStartPoint = m_pwpobjDrag->GetIconPos();
            LPtoDP(&m_ptDragLineStartPoint);
            // Store the center coordinates of object hit as start point for shadow.
            m_ptDragLineStartPoint.x += (GetSystemMetrics(SM_CXICON)/2)-1;
            m_ptDragLineStartPoint.y += (GetSystemMetrics(SM_CYICON)/2)-1;
            break;
      
         case VIEW_DETAILS:
            // Store mouse coordinate as the starting point for shadow drag line.
            m_ptDragLineStartPoint = ptDevice;
            break;
      }
      
      // Start Drag & Drop mode by capturing the mouse movement.
      SetCapture(GetHandle());
      // Create cursors for mouse movement state.
      CreateDragCursors(m_wDragFunction,
            &(g_wpEnvironment.hDragCursor),
            &(g_wpEnvironment.hNoDragCursor));
      // Save original cursor handle.
      g_wpEnvironment.hOriginalCursor  = GetCursor();
      // Set current mouse cursor to the appropriate drag cursor.
      OnMouseMove(wFlags,ptDevice);
      // Play system event sound for "Begin Drag" event.
      PlaySystemEventSound("WPBeginDrag");
   }
   else
   // If left button is down, select objects.
   if ((m_wLeftButtonState==MBS_DOWN)&&(wFlags&MK_LBUTTON)&&
       ((m_wDragFunction==DRAG_SELECT)||(m_wDragFunction==DRAG_SELECT_RUBBERBAND)))
   {
      // Set drag mode flag (DRAG_SELECT or DRAG_SELECT_RUBBERBAND function).
      m_fDragMode = TRUE;
      // Start selection mode by capturing the mouse movement.
      SetCapture(GetHandle());
      // Set current mouse cursor to the appropriate drag cursor.
      OnMouseMove(wFlags,ptDevice);
   }

   // Return success which means 'continue processing'.
   return (TRUE);
}         

//---------------------------------------------------------------------
// HandleRubberBandSelection():
//---------------------------------------------------------------------
void WPFOLDER::HandleRubberBandSelection(POINT ptDevice)
{
   RECT     rcRubberBand;
   LIST     listSelect;
   int      index;
   
   // Draw rubber-band rectangle for object selection.
   DrawRubberBandRect(ptDevice,TRUE);             
   // Create the rubber band rectangle. 
   if (m_ptDragLineStartPoint.x < m_ptDragLineEndPoint.x)
   {
      rcRubberBand.left  = m_ptDragLineStartPoint.x;
      rcRubberBand.right = m_ptDragLineEndPoint.x;
   }
   else
   {
      rcRubberBand.left  = m_ptDragLineEndPoint.x;
      rcRubberBand.right = m_ptDragLineStartPoint.x;
   }
   
   if (m_ptDragLineStartPoint.y < m_ptDragLineEndPoint.y)
   {
      rcRubberBand.top    = m_ptDragLineStartPoint.y;
      rcRubberBand.bottom = m_ptDragLineEndPoint.y;
   }
   else
   {
      rcRubberBand.top    = m_ptDragLineEndPoint.y;
      rcRubberBand.bottom = m_ptDragLineStartPoint.y;
   }
            
   // Check how many objects are included in this area.
   GetChildrenContainedInRect(rcRubberBand,&listSelect);
   // Clear out selected objects list.
   ClearSelectedObjects(TRUE);
   // Add new objects to selected objects list.
   for (index=0; index<listSelect.GetCount(); index++)
   {                                                       
      // Add object to selected objects list.
      AddSelectedObject((WPOBJECT *)listSelect.GetAt(index));
      // Invalidate object so that it can be redrawn again.
      ((WPOBJECT *)listSelect.GetAt(index))->InvalidateIcon(FALSE,TRUE);
   }
   // Clear out temporary list.
   listSelect.RemoveAll();
   // Update newly selected/unselected objects.
   UpdateWindow();
   // Draw rubber-band rectangle for object selection.
   DrawRubberBandRect(ptDevice,FALSE);             
}

//---------------------------------------------------------------------
// WINDOW MESSAGES MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnCreate():
//---------------------------------------------------------------------
void WPFOLDER::OnCreate(void)
{
   WORD     wViewMode;
   RECT     rcIcon,rcWindow,rcLogDomain;
   HCURSOR  hcurOriginal;
   HPBYTE   lpDIBMem;
                     
   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
                     
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

   // Play system event sound associated with "open folder" event.
   PlaySystemEventSound("WPFolderOpen");

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
   
   // Check if we need to draw an image as the folder's background.
   if ((GetType()!=OBJTYPE_DESKTOP)&&(GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE))
   {
      // Load image bitmap and store pointer to image in memory.
      lpDIBMem = LoadImage(GetImagePath());
      // Create logical palette from DIB informtation.
      SetImagePalette(CreatePaletteFromDIB(lpDIBMem));
      // Convert DIB image to regular bitmap object (store bitmap handle internally).
      SetImageBitmapHandle(ConvertDIBToDDB(lpDIBMem,GetImagePalette()));
      // Destroy memory image of bitmap.
      DestroyImage(lpDIBMem);
   }
                           
   // Check if original cursor was not the same as our "wait" cursor...      
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
}


//---------------------------------------------------------------------
// OnMove():
//    This function updates the internal variables that keep track of
// the folder window position.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::OnMove(POINT ptTopLeft)
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
void WPFOLDER::OnSize(WORD wSizeType,int nClientWidth,int nClientHeight)
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

      // Update logical visible area in folder.
      rcVisible = GetVisibleArea();
      SetRect(&rcVisible,rcVisible.left,rcVisible.top,
              (rcVisible.left+nClientWidth),(rcVisible.top+nClientHeight));
      SetVisibleArea(&rcVisible);

      // Arrange scrollbars (if necessary).
      ArrangeScrollbars();

      // Check if folder has an image as background...                    
      if (GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)
         // Invalidate the whole folder client area sice we need to redraw image.
         Invalidate(FALSE,TRUE);
   }
}

//---------------------------------------------------------------------
// OnTimer():
//    This member function is supposed to be overloaded by derived
// classes which might use a Windows 3.1 timer to implement some
// functionaility of the derived object.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::OnTimer(WORD wTimerID)
{
   // This function does nothing.
}



//---------------------------------------------------------------------
// OnSysCommand():
//---------------------------------------------------------------------
void WPFOLDER::OnSysCommand(WORD wValue,LPARAM lParam)
{
   RECT  rcWindow,rcShrinkedWindow;

   // Depending on what system command is requested...
   switch (wValue)
   {                 
      // Horizontal scrollbar activated.
      case SC_HSCROLL:
      case 0xF086:
         // Set the capture input flag for scrollbar.
         SetScrollbarInputCaptureFlag(TRUE);
         // Notify scrollbar of activation.
         PostMessage(GetHandle(),WM_SYSCOMMAND,SC_HSCROLL,lParam);
         break;

      // Vertical scrollbar activated.
      case SC_VSCROLL:
      case 0xF077:
         // Set the capture input flag for scrollbar.
         SetScrollbarInputCaptureFlag(TRUE);
         // Notify scrollbar of activation.
         PostMessage(GetHandle(),WM_SYSCOMMAND,SC_VSCROLL,lParam);
         break;

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

            // "Implode" window frame.
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
// OnCommand():
//---------------------------------------------------------------------
void WPFOLDER::OnCommand(WORD wValue)
{
   int   index;

   // If it was a keyboard event...
   if ((wValue>=ID_EVENT_MINIMUM_ID)&&(wValue<=ID_EVENT_MAXIMUM_ID))
      // Call the folder's keyboard event handler.
      KeyboardEvent(wValue);
   else
   // If it was a popup menu selection...
   if ((wValue>=IDM_MINIMUM_ID)&&(wValue<=IDM_MAXIMUM_ID)&&(GetObjectPopup()!=NULL))
   {
      // Depending on which menu selection was made...
      switch (wValue)
      {     
         case IDM_CREATEANOTHER:
            // Call function that handles the create another of objects...
            CreateAnotherFunc();
            break;
         
         case IDM_CREATESHADOW: 
            // Call function that handles the create shadows of objects...
            CreateShadowFunc();
            break;
      
         case IDM_COPY:
            // Call function that handles the copying of objects...
            CopyFunc();
            break;
      
         case IDM_MOVE:
            // Call function that handles the moving of objects...
            MoveFunc();
            break;
      
         case IDM_DELETE:
            // If object with popup menu is also in "selected" objects list,...
            if (isObjSelected(GetObjectPopup())==TRUE)
               // Delete selected objects from folder (if any).
               DeleteSelectedObjects();
            else
               // Call the menu handler for object that just had popup menu displayed.
               GetObjectPopup()->MenuSelection(wValue);
            break;
            
         default:
            // If object with popup menu is also in "selected" objects list,...
            if (isObjSelected(GetObjectPopup())==TRUE)
            {
               // Do menu command on all selected objects...
               for (index=0; index<GetSelectedObjectCount(); index++)
                  // Call the menu handler for selected object.
                  GetSelectedObject(index)->MenuSelection(wValue);
            }  
            else
               // Call the menu handler for object that just had popup menu displayed.
               GetObjectPopup()->MenuSelection(wValue);
            break;
      }

      // Before leaving, reset object with popup menu to "no-object".
      SetObjectPopup(NULL);
   }
}                                 

//---------------------------------------------------------------------
// OnPopupMenu():
//    This function is called whenever the user selected the system
// menu (by single-clicking MB2) on the folder area.
//---------------------------------------------------------------------
void WPFOLDER::OnPopupMenu(POINT ptDevice)
{
   HMENU       hMenu,hPopupMenu;
   WPOBJECT    *pwpobjHit;
   int         index,nHeight;

   // Set folder's "popup-menu tracking" state flag.
   SetState( (GetState() | OBJSTATE_TRACKPOPUPMENU) );

   // Check if an child object was "hit" by mouse click.
   pwpobjHit = ChildHitTest(ptDevice);
   // If no child objects were hit, then we have selected the folder itself.
   if (pwpobjHit==NULL)
      pwpobjHit = this;

   // Store pointer to object "hit".
   SetObjectPopup(pwpobjHit);

   // Draw rounded rectangle frame around object hit.
   pwpobjHit->PopupFrame();

   // Load system menu for object hit.
   hMenu = LoadMenu(g_wpEnvironment.hResInst,pwpobjHit->GetMenuName());
   // Extract popup menu from system menu (needed step to get popup only!).
   hPopupMenu = GetSubMenu(hMenu,0);

   // Intialize popup menu for current instance of object hit.
   pwpobjHit->InitializePopup(hPopupMenu);
         
   // Calculate the (aproximate) height of popup menu by querying all of the
   // menu item entries and adding its respective (approximate) height value.
   nHeight = 0;
   for (index=0; index<GetMenuItemCount(hPopupMenu); index++)
   {                                                                
      // Check if menu item is a "menu separator" (which has a smaller height)...
      if (GetMenuState(hPopupMenu,index,MF_BYPOSITION)&MF_SEPARATOR)
         nHeight += (GetSystemMetrics(SM_CYMENU)/2);
      else
         // Otherwise, add the full height of a menu item.
         nHeight += (GetSystemMetrics(SM_CYMENU)-4);
   } 

   // Display popup with mouse on bottom-left of menu and track user input.
   TrackPopupMenu(hPopupMenu,(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON),
                  ptDevice.x + ((pwpobjHit!=this)? GetSystemMetrics(SM_CXICON) : 0),
                  (ptDevice.y-nHeight),0,GetHandle(),NULL);

   // Destroy menu resource.
   DestroyMenu(hMenu);

   // Remove rounded rectangle around framed object.
   pwpobjHit->PopupUnFrame();

   // Reset folder's "popup-menu tracking" state flag.
   SetState( (GetState() & (~OBJSTATE_TRACKPOPUPMENU)) );
}

//---------------------------------------------------------------------
// OnNCPaint():
//---------------------------------------------------------------------
void WPFOLDER::OnNCPaint(void)
{
   BOOL  fActive;
   HICON hIcon;

   // If window is minimized, do not paint, just exit.
   if (IsIconic(GetHandle())==TRUE)
      return;

   // Find out if we are currently the active window or not.
   fActive = ((GetActiveWindow()==GetHandle())? TRUE : FALSE);

   // Get folder's class icon handle (for drawing system menu icon).
   hIcon = (HICON)GetClassWord(GetHandle(),GCW_HICON);

   // Paint non-client area of window.
   NCPaint(fActive,hIcon);
}


//---------------------------------------------------------------------
// OnNCActivate():
//---------------------------------------------------------------------
void WPFOLDER::OnNCActivate(BOOL fActive)
{
   HICON hIcon;

   // If window is minimized, do not update non-client area, just exit.
   if (IsIconic(GetHandle())==TRUE)
      return;

   // Get folder's class icon handle (for drawing system menu icon).
   hIcon = (HICON)GetClassWord(GetHandle(),GCW_HICON);

   // Paint non-client area of window.
   NCPaint(fActive,hIcon);
}


//---------------------------------------------------------------------
// OnNCMouseMove():
//---------------------------------------------------------------------
void WPFOLDER::OnNCMouseMove(WORD wHitTestCode,POINT ptDevice)
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
void WPFOLDER::OnNCLButton(UINT uMsg,WORD wHitTestCode,POINT ptDevice)
{

   // If window is currently minimzed, then call default window procedure.
   if (IsIconic(GetHandle())==TRUE)
   {
      DefWindowProc(GetHandle(),uMsg,(WPARAM)wHitTestCode,MAKELPARAM(ptDevice.x,ptDevice.y));
      return;
   }
      
   // Depending on which state the left mouse button is in...      
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
// OnWindows31DragAndDropMsg():
//---------------------------------------------------------------------
LRESULT WPFOLDER::OnWin31DragAndDropMsg(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   // Depending on which Drag & Drop message...
   switch (uMsg)
   {
      case WM_QUERYDROPOBJECT:
         // Service drag & drop message.
         return (Win31DDQueryDropObject((wParam==0),(LPDRAGINFO)lParam));

      case WM_DRAGSELECT:
         // Service drag & drop message.
         return (Win31DDDragSelect((wParam==1),(LPDRAGINFO)lParam));

      case WM_DROPFILES:
         // Service drag & drop message.
         return (Win31DDDropFiles((HDROP)wParam));

      default:
         return (DefWindowProc(GetHandle(),uMsg,wParam,lParam));
   }
}

                         
//---------------------------------------------------------------------
// OnPaletteMessage():
//---------------------------------------------------------------------
LRESULT WPFOLDER::OnPaletteMessage(UINT uMsg,HWND hWndPalette)
{  
   HDC      hdc;
   UINT     uColorsChanged;
   HPALETTE hpalOld;
   
   // If folder has image as background AND has a valid palette to manage...                                                                         
   if ((GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)&&(GetImagePalette()!=NULL))
   {
      // If this window changed the palette, ignore message.      
      if ((uMsg==WM_PALETTECHANGED) && (hWndPalette==GetHandle()))
         return (0);
      
      // Realize folder window image palette into window device-context.   
      hdc = ::GetDC(GetHandle());
      hpalOld = SelectPalette(hdc,GetImagePalette(),FALSE);
      uColorsChanged = RealizePalette(hdc);                
      SelectPalette(hdc,hpalOld,FALSE);
      ::ReleaseDC(GetHandle(),hdc);
      
      // If realizing the palette causes the palette to change, redraw completely.
      if (uColorsChanged > 0)
         Invalidate(FALSE,TRUE);
         
      // Return the number of palette entries that changed to accomodate our palette.
      return (uColorsChanged);      
      
   }
   else
      // Folder does not have a palette to manage, therefore call default window proc.
      return (DefWindowProc(GetHandle(),uMsg,(WPARAM)hWndPalette,0L));
}

                         
                         
//---------------------------------------------------------------------
// OnEraseBackground():
//    This function draws a folder's background by painting the 
// background color first and then drawing a user-selected bitmap (if
// any).  This bitmap could be tiled, centered or strecht blit to the
// folder's client size.
//---------------------------------------------------------------------
LRESULT WPFOLDER::OnEraseBackground(HDC hdc)
{            
   LRESULT lResult;
                     
   // Initialize return value as non-zero.                     
   lResult = TRUE;             
   // Depending on the type of background the folder has...
   if (GetImageAttribute(IMAGE_COLOR_ONLY)==TRUE)
   {       
      // Let default window procedure paint background color.
      lResult = DefWindowProc(GetHandle(),WM_ERASEBKGND,(WPARAM)hdc,0L);
   }
   else             
   // If image has "Normal" attribute.
   if (GetImageAttribute(IMAGE_NORMAL)==TRUE)
   {
      // First, let default window procedure paint background color.
      lResult = DefWindowProc(GetHandle(),WM_ERASEBKGND,(WPARAM)hdc,0L);
      // Paint image bitmap.
      PaintFolderImage(hdc);
   }
   else
   {
      // Image must have "Tiled" or "Scaled" attribute.
      if (GetImageBitmapHandle()==NULL)
         // First, let default window procedure paint background color.
         lResult = DefWindowProc(GetHandle(),WM_ERASEBKGND,(WPARAM)hdc,0L);
      else
         // Paint image bitmap.
         PaintFolderImage(hdc);
   }
   
   // Return result value.
   return (lResult);
}

                   
//---------------------------------------------------------------------
// OnPaint():
//---------------------------------------------------------------------
void WPFOLDER::OnPaint(void)
{
   HDC hdc;

   // Start paint function.
   hdc = BeginPaint();
   // If "details view" is current open view, then draw details table.
   if (GetOpenView()==VIEW_DETAILS)
      DrawDetailsViewTable(hdc);
   #ifdef _DEBUG
      dprintf("tr OnPaint(): WM_PAINT of 0x%04X.\n",GetCurrentTask());
   #endif
   // Draw all children objects in folder.
   DrawChildren(hdc);
   // end paint function.
   EndPaint();
}


//---------------------------------------------------------------------
// OnClose():
//---------------------------------------------------------------------
void WPFOLDER::OnClose(void)
{
   // close folder window.
   Close();
   
   // Check if have to draw an image as the folder's background.
   if ((GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)&&(GetImageBitmapHandle()!=NULL))
   {
      // Destroy crrent bitmap object.
      ::DeleteObject(GetImageBitmapHandle());
      // Reset image bitmap handle to NULL (i.e. no image currently loaded).
      SetImageBitmapHandle(NULL);
   }
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InsertSettingsNotebookPages()
//---------------------------------------------------------------------
void WPFOLDER::InsertSettingsNotebookPages(void)
{
   // Insert the "View" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_VIEW),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPFOLDER::ViewPageProc,NULL,MAKEINTRESOURCE(IDD_VIEW_PAGE));
   // Insert the "Background" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_BACKGROUND),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPFOLDER::BackgroundPageProc,NULL,MAKEINTRESOURCE(IDD_BACKGROUND_PAGE));
   // Insert the "Window" page into settings notebook.
   GetSettingsNotebook()->InsertPage(GetString(IDSTR_SETTNB_PAGE_WINDOW),(BKA_MAJOR|BKA_AUTOPAGESIZE),(FARPAGEPROC)&WPFOLDER::WindowPageProc,NULL,MAKEINTRESOURCE(IDD_WINDOW_PAGE));
   // Insert the default object pages into the settings notebook.
   WPOBJECT::InsertSettingsNotebookPages();
}


//---------------------------------------------------------------------
// GeneralPageProc():
//---------------------------------------------------------------------
void WPFOLDER::GeneralPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // First, call the default "General" page procedure...
   WPOBJECT::GeneralPageProc(nPageIndex,hwndPage,wAction,dwParam);

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_DONE:
         // Update folder caption title if open.
         if (isOpen()==TRUE)
            // Update folder's caption text title.
            SetWindowText(GetHandle(),GetCaptionTitle());
         break;
   }
}


//---------------------------------------------------------------------
// GetWindowPageFlags():
//---------------------------------------------------------------------
DWORD WPFOLDER::GetWindowPageFlags(HWND hwndPage)
{
   DWORD dwFlags=0L;

   // Button appearance.
   if (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1)
      dwFlags |= BUTTONAPPEARANCE_HIDE;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINIMIZEBUTTON)==1)
      dwFlags |= BUTTONAPPEARANCE_MINIMIZE;

   // Minimize button behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_MINHIDE)==1)
      dwFlags |= MINBEHAVIOR_HIDE;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINTOVIEWER)==1)
      dwFlags |= MINBEHAVIOR_MIN_TO_VIEWER;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_MINTODESKTOP)==1)
      dwFlags |= MINBEHAVIOR_MIN_TO_DESKTOP;

   // Object open behavior.
   if (IsDlgButtonChecked(hwndPage,IDB_DISPLAY)==1)
      dwFlags |= OPENBEHAVIOR_DISPLAY;
   else
   if (IsDlgButtonChecked(hwndPage,IDB_CREATE)==1)
      dwFlags |= OPENBEHAVIOR_CREATE;

   // Return queried object flags.
   return (dwFlags);
}


//---------------------------------------------------------------------
// UpdateWindowPageControls():
//---------------------------------------------------------------------
void WPFOLDER::UpdateWindowPageControls(HWND hwndPage,DWORD dwFlags)
{
   // Button appearance selections.
   if ((dwFlags&BUTTONAPPEARANCE_FLAGS_MASK)==BUTTONAPPEARANCE_HIDE)
   {
      // Check "hide button" radio button.
      CheckRadioButton(hwndPage,IDB_HIDEBUTTON,IDB_MINIMIZEBUTTON,IDB_HIDEBUTTON);
      // Disable all minimized button behavior.
      EnableWindow(GetDlgItem(hwndPage,IDB_MINHIDE),FALSE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTOVIEWER),FALSE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTODESKTOP),FALSE);
   }
   else
   if ((dwFlags&BUTTONAPPEARANCE_FLAGS_MASK)==BUTTONAPPEARANCE_MINIMIZE)
   {
      // Check "minimize button" radio button.
      CheckRadioButton(hwndPage,IDB_HIDEBUTTON,IDB_MINIMIZEBUTTON,IDB_MINIMIZEBUTTON);
      // Enable all minimized button behavior.
      EnableWindow(GetDlgItem(hwndPage,IDB_MINHIDE),TRUE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTOVIEWER),TRUE);
      EnableWindow(GetDlgItem(hwndPage,IDB_MINTODESKTOP),TRUE);
   }

   // Check the minimize behavior radio button.
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_HIDE)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINHIDE);
   else
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_MIN_TO_VIEWER)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTOVIEWER);
   else
   if ((dwFlags&MINBEHAVIOR_FLAGS_MASK)==MINBEHAVIOR_MIN_TO_DESKTOP)
      CheckRadioButton(hwndPage,IDB_MINHIDE,IDB_MINTODESKTOP,IDB_MINTODESKTOP);

   // Check the object open behavior radio button.
   if ((dwFlags&OPENBEHAVIOR_FLAGS_MASK)==OPENBEHAVIOR_DISPLAY)
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_DISPLAY);
   else
   if ((dwFlags&OPENBEHAVIOR_FLAGS_MASK)==OPENBEHAVIOR_CREATE)
      CheckRadioButton(hwndPage,IDB_DISPLAY,IDB_CREATE,IDB_CREATE);
}

//---------------------------------------------------------------------
// WindowPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Disable functions not available.
         EnableWindow(GetDlgItem(hwndPage,IDB_CREATE),FALSE);
         // Initialize radio buttons.
         UpdateWindowPageControls(hwndPage,GetFlags());
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_DEFAULT:
               // Reset radio buttons to default folder object values.
               UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_HIDE|MINBEHAVIOR_HIDE|OPENBEHAVIOR_DISPLAY));
               break;

            case IDB_UNDO:
               UpdateWindowPageControls(hwndPage,GetFlags());
               break;

            case IDB_HELP:
               // Open help file with "Window" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_WINDOWPAGE);
               break;

            case IDB_HIDEBUTTON:              
               if (IsDlgButtonChecked(hwndPage,IDB_HIDEBUTTON)==1)
                  UpdateWindowPageControls(hwndPage,BUTTONAPPEARANCE_HIDE|MINBEHAVIOR_HIDE);
               break;

            case IDB_MINIMIZEBUTTON:
               if (IsDlgButtonChecked(hwndPage,IDB_MINIMIZEBUTTON)==1)
                  UpdateWindowPageControls(hwndPage,BUTTONAPPEARANCE_MINIMIZE);
               break;

            case IDB_MINHIDE:
               UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_HIDE));
               break;

            case IDB_MINTOVIEWER:
               UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_MIN_TO_VIEWER));
               break;

            case IDB_MINTODESKTOP:
               UpdateWindowPageControls(hwndPage,(BUTTONAPPEARANCE_MINIMIZE|MINBEHAVIOR_MIN_TO_DESKTOP));
               break;
         }
         break;

      case PAGE_DONE:
         SetFlagsOR(GetWindowPageFlags(hwndPage),
                  (MINBEHAVIOR_FLAGS_MASK|OPENBEHAVIOR_FLAGS_MASK|BUTTONAPPEARANCE_FLAGS_MASK));
         break;
   }
}


//---------------------------------------------------------------------
// ChangeFont():
//---------------------------------------------------------------------
BOOL WPFOLDER::ChangeFont(HWND hwndViewPage)
{
   CHOOSEFONT  cf;
   LOGFONT     lfNewFont;
   HFONT       hfontIconText;
   char        szTextBuffer[LF_FACESIZE+6];
   int         index;

   // Set new logical font structure equal to current logical font.
   GetIconTextLogFont(&lfNewFont);

   // Clear all values in structure to zeros and NULL's.
   memset(&cf,0,sizeof(CHOOSEFONT));

   // Initialize values in choose font structure (used by ChooseFont() Windows API call).
   cf.lStructSize    = sizeof(CHOOSEFONT);
   cf.hwndOwner      = hwndViewPage;
   cf.lpLogFont      = &lfNewFont;
   cf.rgbColors      = m_clrrefIconTextColor;
   cf.Flags          = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS;

   // If user successfully selected a valid font...
   if (ChooseFont(&cf)!=FALSE)
   {
      // Set current font equal to new font.
      SetIconTextLogFont(&lfNewFont);
      // Store the user-selected color of icon text font.
      m_clrrefIconTextColor = cf.rgbColors;
      // Query current font handle from edit control.
      hfontIconText = (HFONT)(WORD)SendDlgItemMessage(hwndViewPage,IDE_FONT,WM_GETFONT,0,0);
      // Destroy current font resource created for edit conrrol.
      ::DeleteObject(hfontIconText);
      // Create new font handle from logical font information.
      hfontIconText = CreateFontIndirect(&lfNewFont);
      // Set font name using font to edit control.
      SendDlgItemMessage(hwndViewPage,IDE_FONT,WM_SETFONT,(WPARAM)hfontIconText,0);
      // Set edit control text to be font height and facename.
      sprintf(szTextBuffer,"%d.%s",(cf.iPointSize/10),lfNewFont.lfFaceName);
      SetDlgItemText(hwndViewPage,IDE_FONT,szTextBuffer);
      // Recalculate the icon text rectangle for all objects in folder.
      for (index=0; index<GetObjectCount(); index++)
         GetObject(index)->CalcTextRect();
      // Repaint the folder and update the icon text with new font.
      Invalidate(TRUE,TRUE);
      // Return success.
      return (TRUE);
   }
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// ViewPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::ViewPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   LOGFONT  logfontIconText;
   HFONT    hfontIconText;
   char     szTextBuffer[LF_FACESIZE+6];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize startup property checkbox.
         if ((GetFlags()&STARTUP_FLAGS_MASK)==STARTUP_PROPERTY)
            CheckDlgButton(hwndPage,IDB_STARTUP,1);   // check it.
         else
            CheckDlgButton(hwndPage,IDB_STARTUP,0);   // uncheck it.
         // Check non-grid selection for icon view page.
         CheckRadioButton(hwndPage,IDB_FLOWED,IDB_NONGRID,IDB_NONGRID);
         // Get logical font structure for icon text from parent folder.
         GetIconTextLogFont(&logfontIconText);
         // Create font from logical font information.
         hfontIconText = CreateFontIndirect(&logfontIconText);
         // Set font name using font to edit control.
         SendDlgItemMessage(hwndPage,IDE_FONT,WM_SETFONT,(WPARAM)hfontIconText,0);
         // Set edit control text to be font height and facename.
         sprintf(szTextBuffer,"%d.%s",((72*logfontIconText.lfHeight)/(-96)),logfontIconText.lfFaceName);
         SetDlgItemText(hwndPage,IDE_FONT,szTextBuffer);
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_CHANGEFONT:
               ChangeFont(hwndPage);
               break;

            case IDB_DEFAULT:
               // Set default "startup" property checkbox value.
               CheckDlgButton(hwndPage,IDB_STARTUP,0);   // uncheck it.
               // set folder font equal to default icon title font.
               // ???
               break;

            case IDB_UNDO:
               // Undo "startup" property checkbox value.
               if ((GetFlags()&STARTUP_FLAGS_MASK)==STARTUP_PROPERTY)
                  CheckDlgButton(hwndPage,IDB_STARTUP,1);   // Check it.
               else
                  CheckDlgButton(hwndPage,IDB_STARTUP,0);   // Uncheck it.
               break;

            case IDB_HELP:
               // Open help file with "View" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_VIEWPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Query current font handle from edit control.
         hfontIconText = (HFONT)(WORD)SendDlgItemMessage(hwndPage,IDE_FONT,WM_GETFONT,0,0);
         // Destroy font resource created for edit conrrol.
         ::DeleteObject(hfontIconText);
         // If "startup" property check box is checked...
         if (IsDlgButtonChecked(hwndPage,IDB_STARTUP)==1)
            // Set startup property.
            SetFlagsOR(STARTUP_PROPERTY,STARTUP_FLAGS_MASK);
         else
            // Else, clear startup property.
            SetFlagsOR(0L,STARTUP_FLAGS_MASK);
         break;
   }
}

//---------------------------------------------------------------------
// ChangeColor():
//---------------------------------------------------------------------
BOOL WPFOLDER::ChangeColor(HWND hwndBackgroundPage)
{
   CHOOSECOLOR    cc;
   DWORD          dwCustomColors[16];

   // Clear all values in structure to zeros and NULL's.
   memset(&cc,0,sizeof(CHOOSECOLOR));
   // Set all custom colors to black
   memset(dwCustomColors,0xFF,(16*sizeof(DWORD)));

   // Initialize values in choose font structure (used by ChooseFont() Windows API call).
   cc.lStructSize    = sizeof(CHOOSECOLOR);
   cc.hwndOwner      = hwndBackgroundPage;
   cc.rgbResult      = GetBackgroundColor();
   cc.lpCustColors   = dwCustomColors;
   cc.Flags          = CC_RGBINIT;

   // If user successfully selected a valid font...
   if (ChooseColor(&cc)!=FALSE)
   {
      // Set current backgrund color equal to new color.
      SetBackgroundColor(cc.rgbResult);
      // Repaint the folder so new background color is used.
      Invalidate(TRUE,TRUE);
      // Return success.
      return (TRUE);
   }
   else
      return (FALSE);
}
         
         
//---------------------------------------------------------------------
// ImageControls():
//---------------------------------------------------------------------
void WPFOLDER::ImageControls(HWND hwndBackgroundPage,BOOL fEnable)
{
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDS_FILETEXT),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDCB_BITMAPLIST),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_NORMALIMAGE),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_TILEDIMAGE),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_SCALEDIMAGE),fEnable);
   EnableWindow(GetDlgItem(hwndBackgroundPage,IDB_IMAGEGROUP),fEnable);
}
               

//---------------------------------------------------------------------
// UpdateBackgroundPageImageSettings():    
//    This function queries the current settings in the "Background"
// page and updates the specific folder flags to reflect those 
// settings.  The first parameter is the "Background" page window 
// handle.  The second parameter is used to specify which group of
// image settings we want to update (if == 0, we update them all).
// By passing which control ID is the cause of this update we can 
// deduce which settings need to be updated.
//---------------------------------------------------------------------
void WPFOLDER::UpdateBackgroundPageImageSettings(HWND hwndPage,WORD wControlID)
{
   BOOL  fColorOnly;
   char  szPath[MAX_PATH_FILENAME+1],
         szImageFile[MAX_PATH_FILENAME+1];

   // Get current state of "Color only" checkbox button.
   fColorOnly = ((IsDlgButtonChecked(hwndPage,IDB_COLORONLY)==1)? TRUE : FALSE);
   // Check if user wants only a color as background or a color and an image.
   if (fColorOnly==TRUE)
      // Set background flags to "color only"
      SetImageAttribute(IMAGE_COLOR_ONLY);
   else
   {  
      // Check if user wants image "normal", "tiled", or "scaled"...
      if (IsDlgButtonChecked(hwndPage,IDB_NORMALIMAGE)==1)
         SetImageAttribute(IMAGE_NORMAL);
      else
      if (IsDlgButtonChecked(hwndPage,IDB_TILEDIMAGE)==1)
         SetImageAttribute(IMAGE_TILED);
      else
         SetImageAttribute(IMAGE_SCALED);
       
      // Check if user has selected a new image...
      if ((wControlID==0)||(wControlID==IDCB_BITMAPLIST))
      {
         // Get current bitmap image selected in list.
         DlgDirSelectComboBoxEx(hwndPage,szPath,MAX_PATH_FILENAME,IDCB_BITMAPLIST);
         // If the user has a valid bitmap image currently selected...
         if (stristr(szPath,GetString(IDSTR_IMAGE_NONE))==NULL)
         {                               
            // Get current Windows directory.
            GetWindowsDirectory(szPath,MAX_PATH_FILENAME);
            // Again, Get current bitmap image selected in list.
            DlgDirSelectComboBoxEx(hwndPage,szImageFile,MAX_PATH_FILENAME,IDCB_BITMAPLIST);
            // Concatenate current Windows path and current image filename.
            strcat(szPath,"\\");
            strcat(szPath,szImageFile);
         }  
         // Store the new image's path (or "(none)" if none).            
         SetImagePath(szPath);
      }
   }
   
   // Check if user has selected a new image...
   if ((wControlID==0)||(wControlID==IDCB_BITMAPLIST))
      // Update folder image (if any) to match the newly selected image (if any).
      ReloadFolderImage();

   // Invalidate folder's client area.
   Invalidate(TRUE,TRUE);
}


//---------------------------------------------------------------------
// BackgroundPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::BackgroundPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   HDC      hdc;
   RECT     rcPreview;   
   HPEN     hpenOld;
   HBRUSH   hbrushOld;
   BOOL     fColorOnly;
   char     szPath[MAX_PATH_FILENAME+1];
   int      nListIndex;

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:            
         // Store current folder background state flag in local boolean variable.
         fColorOnly = GetImageAttribute(IMAGE_COLOR_ONLY);
         // Set "Color only" checkbox button with current folder background state flag.
         CheckDlgButton(hwndPage,IDB_COLORONLY,((fColorOnly)? 1 : 0));
         // Set "Tiled" image as default selection.
         if (fColorOnly==TRUE)
            CheckRadioButton(hwndPage,IDB_NORMALIMAGE,IDB_SCALEDIMAGE,IDB_TILEDIMAGE);
         else
         {        
            // Depending on which image type is currently in use...
            if (GetImageAttribute(IMAGE_NORMAL)==TRUE)
               CheckRadioButton(hwndPage,IDB_NORMALIMAGE,IDB_SCALEDIMAGE,IDB_NORMALIMAGE);
            else
            if (GetImageAttribute(IMAGE_SCALED)==TRUE)
               CheckRadioButton(hwndPage,IDB_NORMALIMAGE,IDB_SCALEDIMAGE,IDB_SCALEDIMAGE);
            else
               CheckRadioButton(hwndPage,IDB_NORMALIMAGE,IDB_SCALEDIMAGE,IDB_TILEDIMAGE);
         }                                        
         // Get current Windows directory and concatenate "*.BMP" to it.
         GetWindowsDirectory(szPath,(MAX_PATH_FILENAME-3));
         strcat(szPath,"\\*.BMP");
         // Set ComboBox control to contain a list of image (bitmap files).
         DlgDirListComboBox(hwndPage,(LPSTR)szPath,IDCB_BITMAPLIST,0,0);
         // Get current Windows directory and concatenate "*.DIB" to it.
         GetWindowsDirectory(szPath,(MAX_PATH_FILENAME-3));
         strcat(szPath,"\\*.DIB");
         // Set ComboBox control to contain a list of image (device-independent bitmap files).
         SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_DIR,0,(LPARAM)(LPCSTR)szPath);
         // Add the "(none)" string to the top of the list and make it the current selection.
         SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_INSERTSTRING,0,(LPARAM)(LPCSTR)GetString(IDSTR_IMAGE_NONE));
         SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_SETCURSEL,0,0);
         // If user has currently selected an image for background...
         if (fColorOnly==FALSE)
         {  
            // Extract filename from current image path string.
            _splitpath(GetImagePath(),NULL,NULL,szPath,NULL);
            // Check if image is currently in ComboBox's listbox...
            if (SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_SELECTSTRING,0,(LPARAM)(LPSTR)szPath)==CB_ERR)
            {
               // Fill-in ComboBox with last image selected.
               nListIndex = (int)SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_ADDSTRING,0,(LPARAM)(LPSTR)szPath);
               // Make current folder image the current selection in ComboBox.
               SendDlgItemMessage(hwndPage,IDCB_BITMAPLIST,CB_SETCURSEL,(WPARAM)(WORD)nListIndex,0);
            }
         }
         // Enable/Disable image controls depending on current folder background state flag.
         ImageControls(hwndPage,((fColorOnly==TRUE)? FALSE : TRUE));
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {      
            case IDB_COLORONLY:
               // Get current state of "Color only" checkbox button.
               fColorOnly = ((IsDlgButtonChecked(hwndPage,IDB_COLORONLY)==1)? TRUE : FALSE);
               // Check "Tiled" image as default.
               CheckRadioButton(hwndPage,IDB_NORMALIMAGE,IDB_SCALEDIMAGE,IDB_TILEDIMAGE);
               // Enable/Disable image controls depending on current folder background state flag.
               ImageControls(hwndPage,((fColorOnly==TRUE)? FALSE : TRUE));
               // Update current image settings.
               UpdateBackgroundPageImageSettings(hwndPage,0);
               break;
         
            case IDB_CHANGECOLOR:
               // Query user for new color choice...
               if (ChangeColor(hwndPage)==TRUE)
                  // Repaint preview area if user selected a color from the color dialog.
                  ::InvalidateRect(hwndPage,NULL,TRUE);
               break;   

            case IDB_NORMALIMAGE:
            case IDB_TILEDIMAGE:
            case IDB_SCALEDIMAGE:
               // Read and update image attributes.
               UpdateBackgroundPageImageSettings(hwndPage,LOWORD(dwParam));
               break;
                                     
            case IDCB_BITMAPLIST:
               // Check if notification message is because combobox selection has changed.
               if (HIWORD(dwParam)==CBN_SELCHANGE)
                  // Read and update image filename since selection has changed.
                  UpdateBackgroundPageImageSettings(hwndPage,IDCB_BITMAPLIST);
               break;
               
            case IDB_DEFAULT:
               // Set current background color equal to default window color.
               SetBackgroundColor(GetSysColor(COLOR_WINDOW));
               // Set backgroung to "Color only".
               CheckDlgButton(hwndPage,IDB_COLORONLY,1);
               // Call this page procedure to simulate user pressing "Color only" checkbox.
               BackgroundPageProc(nPageIndex,hwndPage,PAGE_COMMAND,MAKELPARAM(IDB_COLORONLY,0));
               // Repaint the folder so new background color is used.
               Invalidate(TRUE,TRUE);
               break;

            case IDB_UNDO:
               break;

            case IDB_HELP:
               // Open help file with "Background" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_BACKGROUNDPAGE);
               break;
         }
         break;
         
      case PAGE_PAINT:
         // Retrieve notebook page device context.
         hdc = (HDC)LOWORD(dwParam);          
         // Get preview area's static frame position.
         GetWindowRect(GetDlgItem(hwndPage,IDS_PREVIEWAREA),&rcPreview);
         // Calculate preview area within static frame (2 pixel margin).
         InflateRect(&rcPreview,-2,-2);
         // Convert from screen coord. to client (dialog) coord.
         MapWindowPoints(HWND_DESKTOP,hwndPage,(LPPOINT)&rcPreview,2);
         // Setup preview area's outline pen to be a black outline.
         hpenOld = (HPEN)SelectObject(hdc,GetStockObject(BLACK_PEN));
         // Setup preview area's inside fill color to be the current background color.
         hbrushOld = (HBRUSH)SelectObject(hdc,CreateSolidBrush(GetBackgroundColor()));
         // Paint current background color on preview area's rectangle.
         Rectangle(hdc,rcPreview.left,rcPreview.top,rcPreview.right,rcPreview.bottom);
         // Restore original color brush and destroy created brush.
         ::DeleteObject(SelectObject(hdc,hbrushOld));
         // Restore original pen (do not destroy previous since it was a "stock" object).
         SelectObject(hdc,hpenOld);
         break;
      
      case PAGE_DONE:                        
         // Update all image settings (since "wControlID" parameter is 0).
         UpdateBackgroundPageImageSettings(hwndPage,0);
         break;
   }
}


//---------------------------------------------------------------------
// PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetProfileInfo():
//---------------------------------------------------------------------
void WPFOLDER::GetProfileInfo(FILE_FOLDER_STRUCT *pffsData)
{
   // Save color of folder's background brush.
   pffsData->m_rgbBackground        = m_clrrefBackground;
   pffsData->m_lfIconText           = m_lfIconText;
   pffsData->m_clrrefIconTextColor  = m_clrrefIconTextColor;
   strcpy(pffsData->m_szImagePath,m_szImagePath);
   pffsData->m_wOpenView            = m_wOpenView;
}

//---------------------------------------------------------------------
// SetProfileInfo():
//---------------------------------------------------------------------
void WPFOLDER::SetProfileInfo(FILE_FOLDER_STRUCT *pffsData)
{
   // Store color of folder's background brush.
   m_clrrefBackground    = pffsData->m_rgbBackground;
   m_lfIconText          = pffsData->m_lfIconText;
   m_clrrefIconTextColor = pffsData->m_clrrefIconTextColor;
   strcpy(m_szImagePath,pffsData->m_szImagePath);
   m_wOpenView           = pffsData->m_wOpenView;
}


//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPFOLDER::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_FOLDER_STRUCT   ffsData;

   // Object information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read container extra information.
   if (read(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);
   // Assign profile values.
   m_clrrefBackground      = ffsData.m_rgbBackground;
   m_lfIconText            = ffsData.m_lfIconText;
   m_clrrefIconTextColor   = ffsData.m_clrrefIconTextColor;
   strcpy(m_szImagePath,ffsData.m_szImagePath);
   m_wOpenView             = ffsData.m_wOpenView;

   // Container information.
   if (WPCONTAINER::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPFOLDER::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_FOLDER_STRUCT   ffsData;

   // Object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Folder window information.
   if (WPWINDOW::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Save color of folder's background brush.
   ffsData.m_rgbBackground       = m_clrrefBackground;
   ffsData.m_lfIconText          = m_lfIconText;
   ffsData.m_clrrefIconTextColor = m_clrrefIconTextColor;
   strcpy(ffsData.m_szImagePath,m_szImagePath);
   ffsData.m_wOpenView           = m_wOpenView;

   // Write folder's extra information.
   if (write(hProfile,&ffsData,sizeof(FILE_FOLDER_STRUCT))!=sizeof(FILE_FOLDER_STRUCT))
      return (FALSE);

   // Container information.
   if (WPCONTAINER::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   return (TRUE);
}

// EOF WPFOLDER.CPP

