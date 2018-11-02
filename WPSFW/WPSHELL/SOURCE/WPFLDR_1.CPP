//=====================================================================
// WPFLDR_1.CPP       
//    WPFOLDER Class implementation (Source Module #1).
//
// Date Created :  01/26/94
// Last Update  :  10/06/95
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
#include "wps.hpp"                  // Workplace Shell header file.
#include "imagelib.hpp"             // DIB/BMP Image library header file.
#include "wps_api.h"
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
// From WPS_API.CPP file:
BOOL IsRegisteredSourceWindow(HWND hwndSource);
BOOL IsRegisteredTargetWindow(HWND hwndTarget);


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
#define RGB_ICON_TRANSPARENT_COLOR_TC  RGB(0x84,0x00,0x84)  // "True Color" variation of above color.
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
   // Set drawing mode to depending on folder's current attributes.
   if ((GetFlags()&TEXT_APPEARANCE_FLAGS_MASK)==TEXT_OPAQUE)
      *piPrevBk  = SetBkMode(hdc,OPAQUE);
   else   
      *piPrevBk  = SetBkMode(hdc,TRANSPARENT);
         
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
// SUPPORT FUNCTIONS FOR MOUSE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// HandleDragModeInit():
//---------------------------------------------------------------------
BOOL WPFOLDER::HandleDragModeInit(WORD wFlags,POINT ptDevice)
{
   WPOBJECT *pwpobjHit;
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
   POINT    ptLogical;
   LIST     listNewSelected,listOriginalSelected,listInvalidate;
   DWORD    dwObject;
   int      index;

   // Draw rubber-band rectangle for object selection.
   DrawRubberBandRect(ptDevice,TRUE);             
   // Convert device coordinate point to logical coordinate point.           
   ptLogical = ptDevice;
   DPtoLP(&ptLogical);
   
   // Create the rubber band rectangle (check for top-left & bottom-right positions)...
   if (m_ptDragLineStartPoint.x < ptLogical.x)
   {
      rcRubberBand.left  = m_ptDragLineStartPoint.x;
      rcRubberBand.right = ptLogical.x;
   }
   else
   {
      rcRubberBand.left  = ptLogical.x;
      rcRubberBand.right = m_ptDragLineStartPoint.x;
   }
   
   if (m_ptDragLineStartPoint.y < ptLogical.y)
   {
      rcRubberBand.top    = m_ptDragLineStartPoint.y;
      rcRubberBand.bottom = ptLogical.y;
   }
   else
   {
      rcRubberBand.top    = ptLogical.y;
      rcRubberBand.bottom = m_ptDragLineStartPoint.y;
   }  
   
   // Check how many objects are included in this area.
   GetChildrenContainedInRect(rcRubberBand,&listNewSelected);
   
   // Make local copy of objects that were previously in selected list.
   for (index=0; index<GetSelectedObjectCount(); index++)
      listOriginalSelected.AddTail((DWORD)GetSelectedObject(index));
      
   // Clear out selected objects list.
   ClearSelectedObjects(FALSE);
   
   // Add new objects to selected objects list.
   for (index=0; index<listNewSelected.GetCount(); index++)
      // Add object to selected objects list.
      AddSelectedObject((WPOBJECT *)listNewSelected.GetAt(index));
         
   // Find and store objects that have been added that were NOT originally selected...         
   for (index=0; index<listNewSelected.GetCount(); index++)
   {                               
      // Get object pointer value as a double word.
      dwObject = listNewSelected.GetAt(index);
      // Check if object was not originally selected...
      if (listOriginalSelected.Find(dwObject)==-1)     
         // Add to invalidate list.
         listInvalidate.AddTail(dwObject);
   }

   // Find and store objects that have been removed that were originally selected...         
   for (index=0; index<listOriginalSelected.GetCount(); index++)
   {                               
      // Get object pointer value as a double word.
      dwObject = listOriginalSelected.GetAt(index);
      // Check if object was "unselected"...
      if (listNewSelected.Find(dwObject)==-1)     
         // Add to invalidate list.
         listInvalidate.AddTail(dwObject);
   }
         
   // Now, "listInvalidate" contains all the objects that need to be repainted.
   for (index=0; index<listInvalidate.GetCount(); index++)
      // Repaint object since it was newly selected or unselected.
      ((WPOBJECT *)(listInvalidate.GetAt(index)))->InvalidateIcon(TRUE,TRUE);
         
   // Clear out temporary lists.
   listOriginalSelected.RemoveAll();
   listNewSelected.RemoveAll();
   listInvalidate.RemoveAll();
   
   // Draw rubber-band rectangle used for object selection.
   DrawRubberBandRect(ptDevice,FALSE);             
}


//---------------------------------------------------------------------
// HandleSelectOperation():
//---------------------------------------------------------------------
void WPFOLDER::HandleSelectOperation(POINT ptDevice)
{
   WPOBJECT *pwpobjTarget;

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


//---------------------------------------------------------------------
// HandleMoveCopyOrShadowOperation():
//---------------------------------------------------------------------
void WPFOLDER::HandleMoveCopyOrShadowOperation(POINT ptDevice)
{
   WPOBJECT *pwpobjTarget;
   BOOL     fTargetIsFolderArea;
   HWND     hwndTarget;

   // We are in move, copy, or shadow Drag & Drop modes...
   // Get window from cursor's currrent screen coordinates.
   hwndTarget = WindowFromPoint(ptDevice);  
   // Check that window is valid AND that it is registered as external target window...
   if ((hwndTarget!=NULL)&&(IsRegisteredTargetWindow(hwndTarget)==TRUE))
   {
      // The window currently having the drag focus is registered as an external target window.
      // Therefore, query its attributes and notify it of the event.
      if (SendMessage(hwndTarget,WPSM_COMMAND,DRAGTARGET_NOTIFY,MAKELPARAM(m_wDragFunction,0))==TRUE)
         SetCursor(g_wpEnvironment.hDragCursor);
      else
         SetCursor(g_wpEnvironment.hNoDragCursor);
   }
   else
   {
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
// OnMouseMove():
//---------------------------------------------------------------------
void WPFOLDER::OnMouseMove(WORD wFlags,POINT ptDevice)
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
         HandleSelectOperation(ptDevice);
      else              
      {
         // If drag function is rubber-band selection...
         if (m_wDragFunction==DRAG_SELECT_RUBBERBAND)
            HandleRubberBandSelection(ptDevice);
         else  // Else, if DRAG_MOVE, DRAG_COPY, or DRAG_SHADOW...
            HandleMoveCopyOrShadowOperation(ptDevice);
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
   {
      pwpobjHit->Open();   // "Open" child object hit.
      // If the "Shift" key is pressed at the time of the double-click, OR folder automatic close property is set,
      // OR object is a container and folder automatic close property is only set for subfolders, then...
      if ((((GetKeyState(VK_SHIFT)&0x8000)!=0) || (g_wpEnvironment.stSystem.wFolderAutomaticClose==ALL_OBJECTS)) ||
          ((pwpobjHit->isContainer()==TRUE) && (g_wpEnvironment.stSystem.wFolderAutomaticClose==SUBFOLDERS_ONLY)))
         // Close current folder automatically.
         Close();
   }
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
   if ((clrrefPixel==RGB_ICON_TRANSPARENT_COLOR) || (clrrefPixel==RGB_ICON_TRANSPARENT_COLOR_TC))
      return (FALSE);
   else                                         
      return (TRUE);
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
   HCURSOR  hcurOriginal;
   int      index,nNumPairs;
         
   // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
   hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
       
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
   
   // Check if original cursor was not the same as our "wait" cursor...      
   if (hcurOriginal!=GetCursor())
      // Restore original cursor and destroy custom wait cursor.
      DestroyCursor(SetCursor(hcurOriginal));
}


//---------------------------------------------------------------------
// ArrangeIconsInFlowedFormat():
//---------------------------------------------------------------------
void WPFOLDER::ArrangeIconsInFlowedFormat(void)
{
   POINT ptNew;
   RECT  rcObject;
   int   index,nIconSpacingX,nWindowWidth,nOffset;

   // Store frequently used values in local variable.
   nIconSpacingX = GetSystemMetrics(SM_CXICONSPACING);      

   // If window is currently open...
   if ((isOpen()==TRUE) || (IsWindow(GetHandle())!=FALSE))
   {
      // Then, get client-area size.
      GetClientRect(GetHandle(),&rcObject);
      nWindowWidth = (rcObject.right-1);
   }
   else                       
      // Else, approximate client-area size.
      nWindowWidth = GetWndSize().cx;
      
   // Calculate the minimum height of folder window.
   nWindowWidth = max((nWindowWidth-nIconSpacingX),nIconSpacingX);
   
   // Initialize first object's position to top-left corner of folder window.
   ptNew.x = g_wpEnvironment.sizeIconIndent.cx;
   ptNew.y = g_wpEnvironment.sizeIconIndent.cy;                       
            
   // Walk through the list of objects, and assign an iconic position.
   // Starting from left to right, and going from top to bottom.
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
}


//---------------------------------------------------------------------
// ArrangeIconsInNonFlowedFormat():
//---------------------------------------------------------------------
void WPFOLDER::ArrangeIconsInNonFlowedFormat(void)
{
   POINT ptNew;
   RECT  rcObject;
   int   index,nIconSpacingY,nWindowHeight,nOffset;

   // Store frequently used values in local variable.
   nIconSpacingY = GetSystemMetrics(SM_CYICONSPACING);      

   // If window is currently open...
   if ((isOpen()==TRUE) || (IsWindow(GetHandle())!=FALSE))
   {
      // Then, get client-area size.
      GetClientRect(GetHandle(),&rcObject);
      nWindowHeight = (rcObject.bottom-1);
   }
   else                       
      // Else, approximate client-area size.
      nWindowHeight = GetWndSize().cy;
      
   // Calculate the minimum height of folder window.
   nWindowHeight = max(nWindowHeight,nIconSpacingY);
      
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
      // Next object's icon position will be determined by current object's height.
      nOffset = ((rcObject.bottom-rcObject.top+1)+nIconSpacingY)/2;
      ptNew.y += nOffset;

      // If next object's position is too far down,... 
      if (ptNew.y > nWindowHeight)
      {
         // Then move next position to the next column.
         ptNew.x += GetSystemMetrics(SM_CXICONSPACING);
         ptNew.y  = g_wpEnvironment.sizeIconIndent.cy;
      }
      else      
      // If this is not the last object in list... 
      if ((index+1)<GetObjectCount())
      {
         // Get current object's width and height.
         GetObject(index+1)->GetObjectRect(&rcObject);
         // Check that next object is not going to overflow down...
         if ((ptNew.y+(rcObject.bottom-rcObject.top+1)) > nWindowHeight)
         {
            // Then move next position to the next column.
            ptNew.x += GetSystemMetrics(SM_CXICONSPACING);
            ptNew.y  = g_wpEnvironment.sizeIconIndent.cy;
         }
      }
   }
}


//---------------------------------------------------------------------
// ArrangeIcons():
//---------------------------------------------------------------------
void WPFOLDER::ArrangeIcons(DWORD dwFormat)
{
   RECT  rcLogDomain;
   
   // If want to use current format value (default)...   
   if (dwFormat==(DWORD)-1L)
      dwFormat = (GetFlags()&ICONVIEW_FORMAT_FLAGS_MASK);
         
   // Depending on which icon view format is wanted...
   switch (dwFormat)
   {  
      case ICONVIEW_FORMAT_FLOWED:
      case ICONVIEW_FORMAT_NONGRID:
         // This format places icons in rows accross the screen from left to right.
         ArrangeIconsInFlowedFormat();
         break;
   
      case ICONVIEW_FORMAT_NONFLOWED: 
         // This format places icons in columns from top to bottom of the folder.
         ArrangeIconsInNonFlowedFormat();
         break;
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
   Invalidate(FALSE,TRUE);
}

//---------------------------------------------------------------------
// FormatIconView():
//---------------------------------------------------------------------
void WPFOLDER::FormatIconView(void)
{
   // If format is NOT non-grid (default), then...
   if ((GetFlags()&ICONVIEW_FORMAT_FLAGS_MASK)!=ICONVIEW_FORMAT_NONGRID)
      // Arrange icons to the currently selected format.
      ArrangeIcons();
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
   RECT  rcLogDomain,rcVisible;

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
         // Get current client area size, top-left is always (0,0).
         GetClientRect(GetHandle(),&rcVisible);
         // Inflate rectangle size by 1 pixel to get the "real" client area size.
         rcVisible.right++;
         rcVisible.bottom++;
         // Set current visible area of folder to be top of logical domain.
         SetVisibleArea(&rcVisible);
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

// EOF WPFLDR_1.CPP


