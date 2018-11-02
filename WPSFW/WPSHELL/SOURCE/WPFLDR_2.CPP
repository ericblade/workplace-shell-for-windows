//=====================================================================
// WPFLDR_2.CPP
//    WPFOLDER Class implementation (Source Module #2).
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
// Popup frame (rounded rect.) "roundness" ratio with rectangle size.
#define FRAME_ROUND_RATIO              (0.125F)             // 12.5%
// Objects relative positioning "error zone" calculation value.
#define ERROR_ZONE                     40                   // 40 pixels

//---------------------------------------------------------------------
// WORKPLACE SHELL FOLDER CLASS
//---------------------------------------------------------------------

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
   DrawText(hdc,GetString(IDSTR_DETAILS_ICON_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

   // "Title" column.
   SetRectCXCY(&rcText,DV_TITLE_INDENT_CX,0,DV_TITLE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_TITLE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

   // "Name" column.
   SetRectCXCY(&rcText,DV_NAME_INDENT_CX,0,DV_NAME_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_NAME_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

   // "Size" column.
   SetRectCXCY(&rcText,DV_SIZE_INDENT_CX,0,DV_SIZE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_SIZE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

   // "Date" column.
   SetRectCXCY(&rcText,DV_DATE_INDENT_CX,0,DV_DATE_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_DATE_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

   // "Time" column.
   SetRectCXCY(&rcText,DV_TIME_INDENT_CX,0,DV_TIME_SIZE_CX,(DV_INDENT_CY-1));
   DrawText(hdc,GetString(IDSTR_DETAILS_TIME_COLUMN),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX));

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
              (rcVisible.left+nClientWidth+1),(rcVisible.top+nClientHeight+1));
      SetVisibleArea(&rcVisible);

      // Re-orginize icon view to fit into new folder window size (if necessary).
      FormatIconView();

      // Arrange scrollbars (if necessary).
      ArrangeScrollbars();

      // Check if folder has an image as background...
      if (GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)
         // Invalidate the whole folder client area since we need to redraw image.
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
// ReloadPreviewAreaImage():
//    This function frees the original bitmap and palette information
// passed as parameters and loads a new instance of the currently 
// selected bitmap and palette.  If the "FreeOnly" parameter is TRUE,
// this function only frees the bitmap and palette but does not reload
// the new bitmap and palette information.
//---------------------------------------------------------------------
BOOL WPFOLDER::ReloadPreviewAreaImage(HBITMAP *phbmpImage,HPALETTE *phpalImage,BOOL fFreeOnly)
{
   HPBYTE   lpDIBMem;
   
   // Check if we previously had created a bitmap image in memory...
   if (*phbmpImage!=NULL)
   {
      // First, destroy original image bitmap (if any).
      ::DeleteObject(*phbmpImage);
      *phbmpImage = NULL;
   }

   // Check if we previously had a logical palette...   
   if (*phpalImage!=NULL)
   {
      // Second, destroy original logical palette (if any).
      ::DeleteObject(*phpalImage);
      *phpalImage = NULL;                 
   }
      
   // Check if folder has an image as background AND we do want to load a new image...
   if ((GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)&(fFreeOnly==FALSE))
   {
      // Load new image bitmap into memory.
      lpDIBMem = LoadImage(GetImagePath());
      // Create logical palette from DIB information.
      *phpalImage = CreatePaletteFromDIB(lpDIBMem);
      // Convert DIB image to regular bitmap object (store bitmap handle internally).
      *phbmpImage = ConvertDIBToDDB(lpDIBMem,*phpalImage);
      // Destroy memory image of bitmap.
      DestroyImage(lpDIBMem);
      // Return success.
      return (TRUE);
   }
   else
      // Return failure since we do not have a bitmap image currently selected as background.
      return (FALSE);
}

         
//---------------------------------------------------------------------
// UpdatePreviewArea():
//    This function updates (repaints) the background page preview are
// using the current background color and current background image 
// (bitmap).  It tries to convey the best representation of what the
// folder's background will look like if the current settings are 
// chosen.
//---------------------------------------------------------------------
void WPFOLDER::UpdatePreviewArea(HDC hdc,HWND hwndPage)
{
   HDC      hdcMemory;
   RECT     rcPreview;
   HPEN     hpenOld;
   HBRUSH   hbrushOld;
   HPALETTE hpalOldMem,hpalOld;
   HBITMAP  hbmpOld;
   BITMAP   bmImageInfo;
   SIZE     sizeImage;
   POINT    ptImage;
   int      x,y,nImageMaxSide;
   float    fRatio;
   

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

   // If folder has background image selected...
   if (GetImageAttribute(IMAGE_COLOR_ONLY)==FALSE)
   {                                              
      // If image information (bitmap and palette data) is valid...
      if (m_hbmpPreviewImage!=NULL)
      {                                            
         // Paint image on preview area...
         
         // Create memory device context.                        
         hdcMemory = CreateCompatibleDC(hdc);
         // Check if memory device-context was created successfully...
         if (hdcMemory==NULL)
            return;
            
         // If we do have a logical palette created...
         if (m_hpalPreviewPalette!=NULL)
         {
            hpalOldMem = SelectPalette(hdcMemory,m_hpalPreviewPalette,FALSE);
            hpalOld    = SelectPalette(hdc,m_hpalPreviewPalette,FALSE);
            // Assume the palette's already been realized (no need to call RealizePalette().  
            // It should have been realized in our WM_QUERYNEWPALETTE or WM_PALETTECHANGED messages...
         }
            
         // Select image bitmap into memory device-context.
         hbmpOld = (HBITMAP)SelectObject(hdcMemory,m_hbmpPreviewImage);
         
         // Setup stretch blit mode tobe color-on-color.
         SetStretchBltMode(hdc,COLORONCOLOR);
         
         // Get bitmap object image information.
         ::GetObject(m_hbmpPreviewImage,sizeof(BITMAP),&bmImageInfo);

         // Get preview area's static frame position again.
         // (NOTE: This time we will not convert to dialog points yet)
         GetWindowRect(GetDlgItem(hwndPage,IDS_PREVIEWAREA),&rcPreview);
         // Calculate preview area within static frame (3 pixel margin since frame is at 2 pixels).
         InflateRect(&rcPreview,-3,-3);

         // Depending on the operation to do...
         if (GetImageAttribute(IMAGE_NORMAL)==TRUE)
         {  
            // Get greater side of the original image bitmap.
            nImageMaxSide = max(bmImageInfo.bmHeight,bmImageInfo.bmWidth);
            // Calculate stretch ratio for complete bitmap.
            fRatio = min(1.0F,((rcPreview.bottom-rcPreview.top+1)/(float)nImageMaxSide));
            // Normalize final size of bitmap into preview area.
            sizeImage.cx = (int)(bmImageInfo.bmWidth * fRatio);
            sizeImage.cy = (int)(bmImageInfo.bmHeight * fRatio);
            // Calculate origin into preview area of normalized bitmap image.
            ptImage.x = rcPreview.left + (((rcPreview.right-rcPreview.left+1)-sizeImage.cx)/2);
            ptImage.y = rcPreview.top  + (((rcPreview.bottom-rcPreview.top+1)-sizeImage.cy)/2);
            // Convert from screen coord. to client (dialog) coord.
            MapWindowPoints(HWND_DESKTOP,hwndPage,&ptImage,1);
            // Draw normalized image bitmap into preview area.
            StretchBlt(hdc,ptImage.x,ptImage.y,sizeImage.cx,sizeImage.cy,
                       hdcMemory,0,0,bmImageInfo.bmWidth,bmImageInfo.bmHeight,SRCCOPY);
         } 
         else
         if (GetImageAttribute(IMAGE_TILED)==TRUE)
         {
            // Convert from screen coord. to client (dialog) coord.
            MapWindowPoints(HWND_DESKTOP,hwndPage,(LPPOINT)&rcPreview,2);
            // Tile bitmap horizontally and vertically accross client area.    
            for (x=rcPreview.left; x<rcPreview.right; x+=bmImageInfo.bmWidth)
            {    
               sizeImage.cx = min(bmImageInfo.bmWidth,(rcPreview.right-x));
               for (y=rcPreview.top; y<rcPreview.bottom; y+=bmImageInfo.bmHeight)
               {
                  sizeImage.cy = min(bmImageInfo.bmHeight,(rcPreview.bottom-y));
                  StretchBlt(hdc,x,y,sizeImage.cx,sizeImage.cy,
                             hdcMemory,0,0,sizeImage.cx,sizeImage.cy,SRCCOPY);
               }
            }
         }        
         else
         if (GetImageAttribute(IMAGE_SCALED)==TRUE)
         {
            // Convert from screen coord. to client (dialog) coord.
            MapWindowPoints(HWND_DESKTOP,hwndPage,(LPPOINT)&rcPreview,2);
            // Get stretch image to fit area.
            StretchBlt(hdc,rcPreview.left,rcPreview.top,(rcPreview.right-rcPreview.left),(rcPreview.bottom-rcPreview.top),
                       hdcMemory,0,0,bmImageInfo.bmWidth,bmImageInfo.bmHeight,SRCCOPY);
         }
         
         // Clean-up before leaving function.
         // Select back the original bitmap into memory device-context.               
         SelectObject(hdcMemory,hbmpOld);
         // If we selected a custom palette, then...
         if (m_hpalPreviewPalette!=NULL)
         {                                   
            // Select original palettes into device-contexts   
            SelectPalette (hdcMemory,hpalOldMem,FALSE);
            SelectPalette (hdc,hpalOld,FALSE);
         }
         // Delete memory device-context.
         DeleteDC(hdcMemory);
      }
   }
}


//---------------------------------------------------------------------
// BackgroundPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPFOLDER::BackgroundPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   BOOL     fColorOnly;
   RECT     rcPreview;
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
         // Initialize temporary preview image variables.
         m_hbmpPreviewImage = NULL;
         m_hpalPreviewPalette = NULL;
         // Load the current background image (if any) to display in the preview area.
         ReloadPreviewAreaImage(&m_hbmpPreviewImage,&m_hpalPreviewPalette,FALSE);
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
               // Load the current background image (if any) to display in the preview area.
               ReloadPreviewAreaImage(&m_hbmpPreviewImage,&m_hpalPreviewPalette,FALSE);
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
               // Get preview area's static frame position.
               GetWindowRect(GetDlgItem(hwndPage,IDS_PREVIEWAREA),&rcPreview);
               // Convert from screen coord. to client (dialog) coord.
               MapWindowPoints(HWND_DESKTOP,hwndPage,(LPPOINT)&rcPreview,2);
               // Repaint settings page to reflect new image (if any).          
               ::InvalidateRect(hwndPage,&rcPreview,TRUE);
               break;

            case IDCB_BITMAPLIST:
               // Check if notification message is because combobox selection has changed.
               if (HIWORD(dwParam)==CBN_SELCHANGE)
               {
                  // Read and update image filename since selection has changed.
                  UpdateBackgroundPageImageSettings(hwndPage,IDCB_BITMAPLIST);
                  // Load the current background image (if any) to display in the preview area.
                  ReloadPreviewAreaImage(&m_hbmpPreviewImage,&m_hpalPreviewPalette,FALSE);
                  // Get preview area's static frame position.
                  GetWindowRect(GetDlgItem(hwndPage,IDS_PREVIEWAREA),&rcPreview);
                  // Convert from screen coord. to client (dialog) coord.
                  MapWindowPoints(HWND_DESKTOP,hwndPage,(LPPOINT)&rcPreview,2);
                  // Repaint settings page to reflect new image (if any).          
                  ::InvalidateRect(hwndPage,&rcPreview,TRUE);
               }
                  
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
         // Paint the background preview area (and image preview, if applicable).
         UpdatePreviewArea((HDC)LOWORD(dwParam),hwndPage);
         break;

      case PAGE_DONE:
         // Update all image settings (since "wControlID" parameter is 0).
         UpdateBackgroundPageImageSettings(hwndPage,0);
         // Free the current background image information (if any) used in the preview area.
         ReloadPreviewAreaImage(&m_hbmpPreviewImage,&m_hpalPreviewPalette,TRUE);
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
   DWORD    dwNewIconFormat;
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
         // Initialize icon view format radio buttons...
         switch (GetFlags()&ICONVIEW_FORMAT_FLAGS_MASK)
         {
            case ICONVIEW_FORMAT_FLOWED:
               CheckRadioButton(hwndPage,IDB_FLOWED,IDB_NONGRID,IDB_FLOWED);
               break;
            case ICONVIEW_FORMAT_NONFLOWED:
               CheckRadioButton(hwndPage,IDB_FLOWED,IDB_NONGRID,IDB_NONFLOWED);
               break;
            case ICONVIEW_FORMAT_NONGRID:
               CheckRadioButton(hwndPage,IDB_FLOWED,IDB_NONGRID,IDB_NONGRID);
               break;
         }
         // Get logical font structure for icon text from parent folder.
         GetIconTextLogFont(&logfontIconText);
         // Create font from logical font information.
         hfontIconText = CreateFontIndirect(&logfontIconText);
         // Set font name using font to edit control.
         SendDlgItemMessage(hwndPage,IDE_FONT,WM_SETFONT,(WPARAM)hfontIconText,0);
         // Set edit control text to be font height and facename.
         sprintf(szTextBuffer,"%d.%s",((72*logfontIconText.lfHeight)/(-96)),logfontIconText.lfFaceName);
         SetDlgItemText(hwndPage,IDE_FONT,szTextBuffer);
         // Initialize text transparency property checkbox.
         if ((GetFlags()&TEXT_APPEARANCE_FLAGS_MASK)==TEXT_TRANSPARENT)
            CheckDlgButton(hwndPage,IDB_TRANSPARENTBKGND,1);   // check it.
         else
            CheckDlgButton(hwndPage,IDB_TRANSPARENTBKGND,0);   // uncheck it.
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

         // Check which icon format radio button is selected...
         if (IsDlgButtonChecked(hwndPage,IDB_FLOWED)==1)
            // Set flowed icon view format property.
            dwNewIconFormat = ICONVIEW_FORMAT_FLOWED;
         else
         if (IsDlgButtonChecked(hwndPage,IDB_NONFLOWED)==1)
            // Set flowed icon view format property.
            dwNewIconFormat = ICONVIEW_FORMAT_NONFLOWED;
         else
         if (IsDlgButtonChecked(hwndPage,IDB_NONGRID)==1)
            // Set flowed icon view format property.
            dwNewIconFormat = ICONVIEW_FORMAT_NONGRID;

         // If "transparent text" property check box is checked...
         if (IsDlgButtonChecked(hwndPage,IDB_TRANSPARENTBKGND)==1)
            // Set transparent background property.
            SetFlagsOR(TEXT_TRANSPARENT,TEXT_APPEARANCE_FLAGS_MASK);
         else
            // Else, clear transparent background property.
            SetFlagsOR(TEXT_OPAQUE,TEXT_APPEARANCE_FLAGS_MASK);

         // Check if icon view format has changed from original...
         if ((GetFlags()&ICONVIEW_FORMAT_FLAGS_MASK)!=dwNewIconFormat)
         {
            // Update format flag to new one.
            SetFlagsOR(dwNewIconFormat,ICONVIEW_FORMAT_FLAGS_MASK);
            // Format folder icons with new style.
            FormatIconView();
         }
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

// EOF WPFLDR_2.CPP


