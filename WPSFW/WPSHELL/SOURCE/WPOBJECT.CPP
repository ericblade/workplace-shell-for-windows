//=====================================================================
// WPOBJECT.CPP
//    WPOBJECT Class implementation.
//
// Date Created :  01/24/94
// Last Update  :  10/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/24/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
// Windows library header files.
#include <shellapi.h>
// Workplace Shell resource ID's header file.
#include "resource.h"
#include "wphelpid.h"

//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
WPOBJECT *CreateTreeDuplicate(WPCONTAINER *pwpcontainerParent,WPOBJECT *pwpobjOrigRoot);
BOOL     RenameFileOrDirectory(char *pszOldName,char *pszNewName);
// From WPSETNB.CPP file:
BOOL CALLBACK FindIconDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Object class default values.
#define WPS_OBJECT_DEFAULT_ICONINDEX   ICONINDEX(IDI_DEFAULTOBJECT)

// Unique class name functions values.
#define MAP_SIZE                       1024                    // up to 1024 unique class names available.
#define MAP_ARRAYSIZE                  (MAP_SIZE/sizeof(BYTE)) // calculate number of bytes needed.


//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static BYTE   byteMap[MAP_ARRAYSIZE];     // array used to keep unique class names IDs.


//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECT CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SUPPORT FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// stristr():
//    This function is the same as "strstr()" but case-insensitive.
// It is used to search for text patterns regardless of case.
//---------------------------------------------------------------------
char *WPOBJECT::stristr(const char *s1,const char *s2)
{
   char *ps1,*ps2,*pszMatch;

   // Make local copies of the strings passed.
   ps1 = strdup(s1);
   ps2 = strdup(s2);

   // Make all strings lowercase (case insensitive search).
   strlwr(ps1);
   strlwr(ps2);

   // Do search of string within string.
   pszMatch = strstr(ps1,ps2);

   // Free local copies of parameter strings.
   free(ps1);
   free(ps2);

   // Return value returned by "strstr()" call.
   return (pszMatch);
}


//---------------------------------------------------------------------
// "ICON VIEW" DRAWING FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawIconResource():
//    This function draws the object's icon resource on the passed
// device context using the stored icon logical coordinates.
//---------------------------------------------------------------------
void  WPOBJECT::DrawIconResource(HDC hdc)
{
   HBRUSH  hbrSolid,hbrHatch,hbrOld;
   HPEN    hpenOld;
   int     iPrevBk;

   // Check if object is selected.
   if (isSelected()==TRUE)
   {
      // Create solid brush.
      hbrSolid = CreateSolidBrush(GetBkColor(hdc));
      // Select solid brush as current brush.
      hbrOld    = (HBRUSH)SelectObject(hdc,(HGDIOBJ)hbrSolid);

      // Draw 38x38 pel square.
      PatBlt(hdc,(m_ptIcon.x-3),(m_ptIcon.y-3),
             (GetSystemMetrics(SM_CXICON)+6),(GetSystemMetrics(SM_CYICON)+6),
             PATCOPY);

      // Clean-up and restore original back mode, pen, and brush objects to DC.
      SelectObject(hdc,(HGDIOBJ)hbrOld);
      DeleteObject(hbrSolid);
   }

   // Check if object is in "open" state, then highlight with hatch marks.
   if (isOpen()==TRUE)
   {
      // Create hatch brush with diagonal lines using default highlight color .
      hbrHatch  = CreateHatchBrush(HS_BDIAGONAL,GetTextColor(hdc));
      hbrOld    = (HBRUSH)SelectObject(hdc,(HGDIOBJ)hbrHatch);
      // We do not want a border drawn on the highlight rectangle, so select NULL pen.
      hpenOld   = (HPEN)SelectObject(hdc,(HGDIOBJ)GetStockObject(NULL_PEN));
      // Set current background mode to transparent.
      iPrevBk = SetBkMode(hdc,TRANSPARENT);

      // Draw highlight rectangle.
      Rectangle(hdc,(m_ptIcon.x-3),(m_ptIcon.y-3),
                m_ptIcon.x+GetSystemMetrics(SM_CXICON)+4,
                m_ptIcon.y+GetSystemMetrics(SM_CYICON)+4);

      // Clean-up and restore original back mode, pen, and brush objects to DC.
      SetBkMode(hdc,iPrevBk);
      SelectObject(hdc,(HGDIOBJ)hpenOld);
      SelectObject(hdc,(HGDIOBJ)hbrOld);
      DeleteObject(hbrHatch);
   }

   // Draw icon resource on device context.
   DrawIcon(hdc,m_ptIcon.x,m_ptIcon.y,m_hIcon);
}

//---------------------------------------------------------------------
// DrawIconText():
//    This function draws the object's title text underneath the
// object's icon resource on the passed device context.
//---------------------------------------------------------------------
void WPOBJECT::DrawIconText(HDC hdc)
{
   int iPrevBk;

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Set current background mode to opaque.
      iPrevBk  = SetBkMode(hdc,OPAQUE);

   // Draw title text below icon.
   DrawText(hdc,GetTitle(),strlen(GetTitle()),&m_rcText,(DT_CENTER|DT_WORDBREAK|DT_NOCLIP|DT_NOPREFIX));

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Restore original background mode.
      SetBkMode(hdc,iPrevBk);
}


//---------------------------------------------------------------------
// "DETAILS VIEW" DRAWING FUNCTIONS.
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DrawDetailsIconResource():
//    This function draws on the passed device context the object's
// icon resource on the left column format for the details view table.
//---------------------------------------------------------------------
void WPOBJECT::DrawDetailsIconResource(HDC hdc)
{
   HBRUSH  hbrSolid,hbrHatch,hbrOld;
   HPEN    hpenOld;
   RECT    rcDetailsEntry,rcIconEntry;
   int     iPrevBk;

   // Get the details view entry rectangle.
   GetFrameRect(&rcDetailsEntry);

   // Check if object is selected.
   if (isSelected()==TRUE)
   {
      // Create solid brush.
      hbrSolid = CreateSolidBrush(GetBkColor(hdc));
      // Select solid brush as current brush.
      hbrOld    = (HBRUSH)SelectObject(hdc,(HGDIOBJ)hbrSolid);

      // Draw details rectangle.
      PatBlt(hdc,rcDetailsEntry.left,rcDetailsEntry.top,
                 (rcDetailsEntry.right-rcDetailsEntry.left),
                 (rcDetailsEntry.bottom-rcDetailsEntry.top),
                 PATCOPY);

      // Clean-up and restore original back mode, pen, and brush objects to DC.
      SelectObject(hdc,(HGDIOBJ)hbrOld);
      DeleteObject(hbrSolid);
   }

   // Check if object is in "open" state, then highlight with hatch marks.
   if (isOpen()==TRUE)
   {
      // Create hatch brush with diagonal lines using default highlight color .
      hbrHatch  = CreateHatchBrush(HS_BDIAGONAL,GetTextColor(hdc));
      hbrOld    = (HBRUSH)SelectObject(hdc,(HGDIOBJ)hbrHatch);
      // We do not want a border drawn on the highlight rectangle, so select NULL pen.
      hpenOld   = (HPEN)SelectObject(hdc,(HGDIOBJ)GetStockObject(NULL_PEN));
      // Set current background mode to transparent.
      iPrevBk = SetBkMode(hdc,TRANSPARENT);

      // Store icon rectangle position and size.
      SetRectCXCY(&rcIconEntry,
                  (rcDetailsEntry.left+DV_ICON_INDENT_CX)-1,
                  (rcDetailsEntry.top+DV_ICON_INDENT_CY)-1,
                  DV_ICON_SIZE_CX+2,
                  DV_ICON_SIZE_CY+2);

      // Draw highlight rectangle.
      Rectangle(hdc,rcIconEntry.left,rcIconEntry.top,rcIconEntry.right+1,rcIconEntry.bottom+1);

      // Clean-up and restore original back mode, pen, and brush objects to DC.
      SetBkMode(hdc,iPrevBk);
      SelectObject(hdc,(HGDIOBJ)hpenOld);
      SelectObject(hdc,(HGDIOBJ)hbrOld);
      DeleteObject(hbrHatch);
   }

   // Draw icon resource on device context.
   DrawIcon(hdc,rcDetailsEntry.left+DV_ICON_INDENT_CX,
                rcDetailsEntry.top+DV_ICON_INDENT_CY,
                m_hIcon);
}

//---------------------------------------------------------------------
// DrawDetailsIconText():
//    This function draws the object's title text on the details view
// "title" column field.  This function can be overloaded by derived
// classes to display other pertinent (or applicable) information,
// like "real name", "size", "date", etc...
//---------------------------------------------------------------------
void WPOBJECT::DrawDetailsIconText(HDC hdc)
{
   int   iPrevBk;
   RECT  rcText;

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Set current background mode to opaque.
      iPrevBk  = SetBkMode(hdc,OPAQUE);

   // Calculate text rectangle position for "title" column.
   SetRectCXCY(&rcText,
               (DV_TITLE_INDENT_CX+5),
               (DV_INDENT_CY+(GetParent()->GetObjectIndex(this)*DV_LINEITEM_SIZE_CY)+DV_TITLE_INDENT_CY),
               (DV_TITLE_SIZE_CX-5),
               DV_TITLE_SIZE_CY);

   // Draw "title" text on details view column.
   DrawText(hdc,GetTitleNoCRLF(),-1,&rcText,(DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX));

   // If object is currently selected...
   if (isSelected()==TRUE)
      // Restore original background mode.
      SetBkMode(hdc,iPrevBk);
}


//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// CLASS NAME MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CreateUniqueClassName():
//    This function returns a unique class name used to create window
// classes for the Workplace Shell.
//---------------------------------------------------------------------
const char *WPOBJECT::CreateUniqueClassName(void)
{
   int         i,iUnusedValue,b;
   static char szClassName[] = "WPSxxxx";
   BYTE        bBitValue[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

   // Scan, searching for free value
   for (i=0; i<MAP_ARRAYSIZE; i++ )
      if (byteMap[i]!=0xFF)
            break;

   // If a free value was not available, return NULL
   if (i==MAP_ARRAYSIZE)
      return (NULL);

   // Calculate which bit was free, since the value was not 0xFF,
   // one of the bits must be free.
   for (b=0; b<8; b++)
      if ((byteMap[i]&bBitValue[b])==0)
         break;

   // Set the value to in use
   byteMap[i] |= bBitValue[b];

   // Calculate the bit position's value
   iUnusedValue = (i*8)+b;

   // Build unique class name string
   sprintf(szClassName,"WPS%04u",(WORD)iUnusedValue);

   return (szClassName);
}

//---------------------------------------------------------------------
// DestroyUniqueClassName():
//    This function destroys the class name and resets the internal
// data structures so that we can reuse this class name later.
// The class name must be of the form "WPSxxxx" (where xxxx = 0-1023).
//---------------------------------------------------------------------
BOOL WPOBJECT::DestroyUniqueClassName(const char *pszClassName)
{
   int   index, Bit;
   WORD  wUsedValue;
   BYTE  bBitValue[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

   // Get the value from the string.
   if (sscanf(pszClassName,"WPS%u",&wUsedValue )!=1) return (FALSE);
   // If value passed was an invalid one.
   if (wUsedValue>=MAP_SIZE)
      return (FALSE);

   // Get the map array index and bit location.
   index = wUsedValue / 8;    // get byte index (divide by 8 bits/byte).
   Bit   = wUsedValue % 8;    // get bit index within byte (8 bits/byte).

   // Mark that bit as unused after verify it was already in use.
   if ((byteMap[index]&bBitValue[Bit])==0)
      return( FALSE );

   // Clear bit and make it available.
   byteMap[index] &= ~bBitValue[Bit];

   return (TRUE);
}

//---------------------------------------------------------------------
// isWorkplaceShellClassName():
//---------------------------------------------------------------------
BOOL WPOBJECT::isWorkplaceShellClassName(char *pszClassName)
{
   // if length of class name is not exactly 7 chars., then not a WPS class.
   if (strlen(pszClassName)!=7) return (FALSE);

   // begining 3 chars. MUST be "WPS".
   if ((pszClassName[0]=='W')&&
       (pszClassName[1]=='P')&&
       (pszClassName[2]=='S'))
      return (TRUE);       // yes, class name is a workplace shell name.
   else
      return (FALSE);      // no, class name is not a workplace shell name.
}


//---------------------------------------------------------------------
// isWorkplaceShellWindow():
//---------------------------------------------------------------------
BOOL WPOBJECT::isWorkplaceShellWindow(HWND hwnd)
{
   char szClassName[MAX_CLASS_NAME+1];

   // Get window's class name (up to 8 chars) and check if length of class
   // name is at least 7 characters (quick & easy "first check").
   if (GetClassName(hwnd,szClassName,MAX_CLASS_NAME)<7)
      return (FALSE);
   else
      return (isWorkplaceShellClassName(szClassName));

}


//---------------------------------------------------------------------
// SetIconHandle():
//    This function should only be used by derived classes that need to
// update the internal data of the object class.  It copies an icon
// handle into the private data member "m_hIcon".
//---------------------------------------------------------------------
void WPOBJECT::SetIconHandle(HICON hIcon)
{
   // Copy icon handle into private data member.
   m_hIcon = hIcon;
}

//---------------------------------------------------------------------
// CALCULATE OBJECT'S TITLE TEXT RECTANGLE FOR "ICON VIEW"
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CalcTextRect()
//    This function calculates the size and position of the rectangle
// needed to display the title text of the object in minimized state.
//---------------------------------------------------------------------
void WPOBJECT::CalcTextRect(void)
{
   HDC     hdc;
   HFONT   hfont,hfontOld;      // handles to fonts.
   LOGFONT logfontIconText;     // logical font information structure.
   int     iPrevMap,iXOffset;   // previous mapping mode.

   // Set text rectangle equal to an empty rectangle.
   SetRectEmpty(&m_rcText);

   // Get display device context.
   hdc = GetDC(GetDesktopWindow());

   // Set correct mapping mode and save original mapping mode..
   iPrevMap = SetMapMode(hdc,MM_TEXT);

   // If object has a parent container...
   if (GetParent()!=NULL)
      // Get logical font structure for icon text from parent folder.
      ((WPFOLDER *)GetParent())->GetIconTextLogFont(&logfontIconText);
   else
      // ..Else, get default system icon text font.
      SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&logfontIconText,0);

   // Create font.
   hfont = CreateFontIndirect(&logfontIconText);

   // Set the current font to the newly created font.
   hfontOld = (HFONT)SelectObject(hdc,(HGDIOBJ)hfont);

   // Set smallest rectangle size for encapsulating icon text.
   SetRect(&m_rcText,
           m_ptIcon.x,
           m_ptIcon.y+GetSystemMetrics(SM_CYICON)+6,
           m_ptIcon.x+GetSystemMetrics(SM_CXICONSPACING)-1,
           m_ptIcon.y+GetSystemMetrics(SM_CYICON)+6);

   // Calculate minimum bounding rectangle based on current font and width.
   DrawText(hdc,GetTitle(),strlen(GetTitle()),&m_rcText,(DT_CALCRECT|DT_CENTER|DT_WORDBREAK|DT_NOPREFIX));

   // Center new rectangle underneath icon (move left/right appropriatedly).
   iXOffset = (GetSystemMetrics(SM_CXICON)- (m_rcText.right-m_rcText.left+1)) / 2;
   OffsetRect(&m_rcText,iXOffset,0);

   // Restore original mapping and background modes.
   SetMapMode(hdc,iPrevMap);

   // Clean-up font objects and restore original font to device context.
   SelectObject(hdc,(HGDIOBJ)hfontOld);
   DeleteObject(hfont);

   // Return display device context.
   ReleaseDC(GetDesktopWindow(),hdc);
}


//---------------------------------------------------------------------
// RECTANGLE UTILITY FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetRectCXCY():
//    This function intializes a rectangle structure using the
// top-left coordinate and width and height.
//---------------------------------------------------------------------
void WPOBJECT::SetRectCXCY(LPRECT lprcRect,int x,int y,int cx,int cy)
{
   lprcRect->left   = x;
   lprcRect->top    = y;
   lprcRect->right  = x + cx - 1;
   lprcRect->bottom = y + cy - 1;
}



//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPOBJECT():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPOBJECT::WPOBJECT(void)
{
   // Clear everything to zeros.
   memset(this,0,sizeof(WPOBJECT));
   // Initialize default values.
   SetType(OBJTYPE_OBJECT);
   // Initialize icon resource to no icon resource (this will force the
   // correct default icon for instantiated object to be displayed).
   m_szIconPath[0] = '\0';
   m_wIconIndex = 0;
   // Clear all object flags.
   SetFlags(0L);
}

//---------------------------------------------------------------------
// WPOBJECT():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPOBJECT::WPOBJECT(WPOBJECT *pwpobjDefaults)
{
   // Clear everything to zeros.
   memset(this,0,sizeof(WPOBJECT));

   // Initialize object with default object passed as parameter.
   SetType(pwpobjDefaults->GetType());

   // Copy default icon position.
   // We cannot call SetIconPos() directly from the constructor because of a
   // strange bug in the Visual C++ 1.5.01 compiler.  The bug causes a GPF
   // Trap D (Page Fault) when a "virtual" function is called from within
   // a member function from within the constructor code.  The virtual function
   // tables have not been initialized completely at this point.  Therefore,
   // virtual function calls translate to "CALL FAR 0000:0000" producing the GPF.
   // Microsoft does not have fix for this, therefore we must "circumvent" problem
   // by copying the code inline.  Cannot call CalcTextRect() from here.  But,
   // that's OK since there is no need to calculate the new text rectangle yet.
   // It will be done later by one of the drawing functions.
   m_ptIcon = pwpobjDefaults->GetIconPos();

   // Icon resource.
   pwpobjDefaults->GetIcon(m_szIconPath,&m_wIconIndex);

   // Copy default object's title.
   // Again, to avoid the GPF described above, we must copy SetTitle() inline.
   // Also, we cannot call CalcTextRect() from here (produces the GPF).
   strcpy(m_szTitle,pwpobjDefaults->GetTitle());

   // Set object flags equal to default object's.
   SetFlags(pwpobjDefaults->GetFlags());
}

//---------------------------------------------------------------------
// ~WPOBJECT():
//    Destructor for base object class.
//---------------------------------------------------------------------
WPOBJECT::~WPOBJECT(void)
{
   // Check if Workplace Shell desktop window exists...
   if (IsWindow(g_wpEnvironment.hwndDesktop)==TRUE)
      // Notify the Workplace Shell desktop that this object is being destroyed.
      SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_NOTIFY_OBJECT_DESTROYED,0,(LPARAM)this);
}


//---------------------------------------------------------------------
// OBJECT INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Initialize():
//    This function gets called only once during the desktop
// initialization and its called by its parent container object so that
// this object can resume the state in which it was saved.
//    We then call all our children objects so they can do the same.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::Initialize(BOOL fIsShell)
{
   // If object was open when desktop was previuosly shutdown...
   if (isOpen()==TRUE)
   {
      // Reset "opened" state bit.
      SetState((GetState()&(~OBJSTATE_OPEN)));

      // Open object.
      Open();
   }
}


//---------------------------------------------------------------------
// TYPE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetType():
//    Returns the object type value.
//---------------------------------------------------------------------
DWORD WPOBJECT::GetType(void)
{
   return (m_dwType);
}


//---------------------------------------------------------------------
// SetType():
//    Stores the new object type value in the object.
//---------------------------------------------------------------------
void  WPOBJECT::SetType(DWORD dwType)
{
   m_dwType = dwType;
}

//---------------------------------------------------------------------
// UNIQUE ID MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetUniqueID():
//---------------------------------------------------------------------
WORD WPOBJECT::GetUniqueID(void)
{
   return (m_wUniqueID);
}

//---------------------------------------------------------------------
// SetUniqueID():
//---------------------------------------------------------------------
void WPOBJECT::SetUniqueID(WORD wUniqueID)
{
   m_wUniqueID = wUniqueID;
}


//---------------------------------------------------------------------
// TITLE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetTitle():
//    This function updates the object's title string.  It also makes
// sure that shadow objects' text rectangle gets updated correctly.
//---------------------------------------------------------------------
void WPOBJECT::SetTitle(const char *pszTitle)
{
   int index;

   // Store new title text in private data member.
   strcpy(m_szTitle,pszTitle);
   // Calculate text rectangle with for new title text.
   CalcTextRect();
   // Next, calculate new text rectangle for shadow objects (if any).
   for (index=0; index<GetShadowCount(); index++)
      GetShadowLink(index)->CalcTextRect();

}

//---------------------------------------------------------------------
// GetTitle():
//    Returns the object's title string as entered by user.
//---------------------------------------------------------------------
const char *WPOBJECT::GetTitle(void)
{
   return (m_szTitle);
}

//---------------------------------------------------------------------
// GetTitleNoCRLF():
//    This function returns the object's title string stripped of
// special control characters (like CR and LF).
//---------------------------------------------------------------------
const char *WPOBJECT::GetTitleNoCRLF(void)
{
   static char szTitleNoCRLF[MAX_TITLE_NAME+1];
   int         index,nLength,nPos;

   // Set string to empty.
   szTitleNoCRLF[0] = '\0';
   // Save string length (for optimization purposes).
   nLength = strlen(m_szTitle);
   // Loop through all characters in title.
   nPos = 0;
   for (index=0; index<nLength; index++)
   {
      // Check if character is not blank space, tab, or newline...
      if (!IsSpace(m_szTitle[index]))
         szTitleNoCRLF[nPos] = m_szTitle[index];   // Copy actual character.
      else
         szTitleNoCRLF[nPos] = ' ';                // Set character to blank space.

      // Increment position in string w/o CRLF.
      nPos++;
   }

   // Null-terminate string.
   szTitleNoCRLF[nPos] = '\0';

   // Return converted string.
   return (szTitleNoCRLF);
}

//---------------------------------------------------------------------
// GetCaptionTitle():
//---------------------------------------------------------------------
const char *WPOBJECT::GetCaptionTitle(void)
{
   return (GetTitleNoCRLF());
}
            
            
//---------------------------------------------------------------------
// UpdateFileSystemObjectName():
//---------------------------------------------------------------------
void WPOBJECT::UpdateFileSystemObjectName(char *szNewTitle)
{
   // Do nothing by default.
}


//---------------------------------------------------------------------
// APPEARANCE AND POSITION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetIconPos():
//    Stores the new icon top-left logical coordinates.  Parameter is
// a structure to a point.
//---------------------------------------------------------------------
void WPOBJECT::SetIconPos(POINT ptIcon)            // logical points.
{
   m_ptIcon = ptIcon;
   CalcTextRect();
}

//---------------------------------------------------------------------
// SetIconPos():
//    Stores the new icon top-left logical coordinates.  Parameters are
// an integer pair (x,y) reflecting a point.
//---------------------------------------------------------------------
void WPOBJECT::SetIconPos(int x,int y)             // logical points.
{
   m_ptIcon.x = x;
   m_ptIcon.y = y;
   CalcTextRect();
}


//---------------------------------------------------------------------
// GetIconPos():
//    Returns the current icon position for the object's icon.  The
// point coordinates are in logical points relative to the parent
// container object.
//---------------------------------------------------------------------
POINT WPOBJECT::GetIconPos(void)
{
   return (m_ptIcon);
}

//---------------------------------------------------------------------
// SetIcon():
//    Stores the new icon resource data.  Icon path is a null-terminated
// string that contains the path\filename where the icon resource is
// stored.  The icon index is a zero-based index which points to the
// ordinal position of the icon resource in the given path\filename.
// NOTE: The filename can be an .EXE, a .DLL or an .ICO file.
//---------------------------------------------------------------------
void WPOBJECT::SetIcon(char *pszIconPath,WORD wIconIndex)
{
   strcpy(m_szIconPath,pszIconPath);
   m_wIconIndex = wIconIndex;
}

//---------------------------------------------------------------------
// GetIcon():
//    Returns the current icon resource data.  Icon path is a null-
// terminated string that will contain the path\filename where the
// icon resource is stored.  The icon index is a zero-based index
// which points to the ordinal position of the icon resource in the
// given path\filename.
// NOTE: The filename can be an .EXE, a .DLL or an .ICO file.
//---------------------------------------------------------------------
void WPOBJECT::GetIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,m_szIconPath);
   *pwIconIndex = m_wIconIndex;
}

//---------------------------------------------------------------------
// GetIconHandle():
//---------------------------------------------------------------------
HICON WPOBJECT::GetIconHandle(void)
{
   return (m_hIcon);
}

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPOBJECT::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   strcpy(pszIconPath,GetString(IDSTR_SHELL_FILENAME));
   *pwIconIndex = WPS_OBJECT_DEFAULT_ICONINDEX;
}

//---------------------------------------------------------------------
// ExtractIcon():
//---------------------------------------------------------------------
HICON WPOBJECT::ExtractIcon(void)
{
   UINT    uPrevErrorMode;
   HICON   hIcon;
   WORD    wDefIconIndex,wIconID;
   char    szDefIconPath[MAX_PATH_FILENAME+1];

   // Turn-off the Windows 3.1 INT 24H error handling and "drive not ready" boxes.
   uPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

   // Get icon resource information.
   GetIcon(szDefIconPath,&wDefIconIndex);

   // For performance reasons, check if icon resource if one of the built-in ones.
   if (stricmp(szDefIconPath,GetString(IDSTR_SHELL_FILENAME))==0)
   {
      // Calculate icon ID from resource index.
      wIconID = ICONID(wDefIconIndex);
      // Load icon resource from Workplace Shell instance directly (for performance reasons).
      hIcon = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(wIconID));
   }
   else
      // Try to load icon resource with current values.
      hIcon = ::ExtractIcon(g_wpEnvironment.hInst,szDefIconPath,wDefIconIndex);

   // If load failed with current resource values.
   if ((hIcon==NULL)||(hIcon==(HICON)1))
   {
      // Get default icon resource information.
      GetDefaultIcon(szDefIconPath,&wDefIconIndex);
      // For performance reasons, check if icon resource if one of the built-in ones.
      if (stricmp(szDefIconPath,GetString(IDSTR_SHELL_FILENAME))==0)
      {
         // Calculate icon ID from resource index.
         wIconID = ICONID(wDefIconIndex);
         // Load icon resource from Workplace Shell instance directly (for performance reasons).
         hIcon = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(wIconID));
      }
      else
      {
         // Try to load default object icon resource.
         hIcon = ::ExtractIcon(g_wpEnvironment.hInst,szDefIconPath,wDefIconIndex);
         // If load failed with current default resource values.
         if ((hIcon==NULL)||(hIcon==(HICON)1))
            // Load icon resource from Workplace Shell instance (last resort)
            hIcon = ::LoadIcon(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDI_DEFAULTOBJECT));
      }
   }

   // Restore original Windows 3.1 error mode.
   SetErrorMode(uPrevErrorMode);

   // Return loaded icon resource.
   return (hIcon);
}


//---------------------------------------------------------------------
// LoadIcon():
//    This function loads the icon resource from the current object
// icon path\filename and icon index.  If the load fails, the object
// is assigned the default object icon.
//---------------------------------------------------------------------
BOOL WPOBJECT::LoadIcon(void)
{
   // Store new icon resource handle.
   m_hIcon = ExtractIcon();

   return (TRUE);
}


//---------------------------------------------------------------------
// UpdateIcon():
//---------------------------------------------------------------------
void WPOBJECT::UpdateIcon(char *pszIconPath,WORD wIconIndex)
{
   int index;

   // First, destroy current icon resources in shadow objects (if any).
   for (index=0; index<GetShadowCount(); index++)
      GetShadowLink(index)->DestroyIcon();

   // First, destroy current icon resource.
   DestroyIcon();

   // Check if icon path parameters are valid...
   if (pszIconPath!=NULL)
      // Store new icon resource information.
      SetIcon(pszIconPath,wIconIndex);

   // If this object has a parent container.
   if (GetParent()!=NULL)
   {
      // Check if shadow objects need their new icon resources re-loaded...
      for (index=0; index<GetShadowCount(); index++)
      {
         // If shadow object has a parent object...
         if (GetShadowLink(index)->GetParent()!=NULL)
         {
            // If shadow object's parent container object is currently open...
            if (GetShadowLink(index)->GetParent()->isOpen()==TRUE)
               // Then, Load new icon resource for shadow object.
               GetShadowLink(index)->LoadIcon();
         }
      }

      // If this object's parent container is open, load new icon resource.
      if (GetParent()->isOpen()==TRUE)
         LoadIcon();
   }
   else
      // Just load new icon resource.
      LoadIcon();

   // Repaint shadow objects icons (if necessary).
   for (index=0; index<GetShadowCount(); index++)
      GetShadowLink(index)->InvalidateIcon(TRUE,TRUE);

   // Repaint object icon.
   InvalidateIcon(TRUE,TRUE);
}


//---------------------------------------------------------------------
// DestroyIcon():
//    This function destroys the current icon resource for the object.
//---------------------------------------------------------------------
BOOL WPOBJECT::DestroyIcon(void)
{
   // If valid icon handle, destroy icon resource handle.
   if ((m_hIcon!=NULL)&&(m_hIcon!=(HICON)1))
   {
      ::DestroyIcon(m_hIcon);    // Destroy internal Windows 3.1 storage.
      m_hIcon = NULL;            // Reset icon resource handle to nothing.
      return (TRUE);             // Successfully destroyed icon resource.
   }
   else
      return (FALSE);            // No valid icon resource to destroy.
}


//---------------------------------------------------------------------
// GetPopupFrameRect():
//    This function calculates the position of the popup frame rect.
//---------------------------------------------------------------------
void WPOBJECT::GetPopupFrameRect(LPRECT lprcPopupFrame) // logical points.
{
   // Depending on parent container's open view...
   switch (GetParent()->GetOpenView())
   {
      case VIEW_ICON:
         // Calculate frame rectangle.
         SetRect(lprcPopupFrame,(m_ptIcon.x-3),(m_ptIcon.y-3),
                 (m_ptIcon.x+GetSystemMetrics(SM_CXICON)+2),
                 (m_ptIcon.y+GetSystemMetrics(SM_CYICON)+2));
         break;

      case VIEW_DETAILS:
         // Calculate frame rectangle.
         SetRectCXCY(lprcPopupFrame,
            DV_POPUPFRAME_INDENT_CX,
            DV_INDENT_CY+(GetParent()->GetObjectIndex(this)*DV_LINEITEM_SIZE_CY)+DV_POPUPFRAME_INDENT_CY,
            DV_POPUPFRAME_SIZE_CX,
            DV_POPUPFRAME_SIZE_CY);
         break;
   }
}


//---------------------------------------------------------------------
// GetFrameRect():
//    This function calculates the position of the frame rectangle.
//---------------------------------------------------------------------
void WPOBJECT::GetFrameRect(LPRECT lprcFrame)       // logical points.
{
   // Depending on parent container's open view...
   switch (GetParent()->GetOpenView())
   {
      case VIEW_ICON:
         // Calculate frame rectangle.
         SetRect(lprcFrame,(m_ptIcon.x-5),(m_ptIcon.y-5),
              (m_ptIcon.x+GetSystemMetrics(SM_CXICON)+4),
              (m_ptIcon.y+GetSystemMetrics(SM_CYICON)+4));
         break;

      case VIEW_DETAILS:
         // Calculate frame rectangle.
         SetRectCXCY(lprcFrame,
            DV_INDENT_CX,
            DV_INDENT_CY+(GetParent()->GetObjectIndex(this)*DV_LINEITEM_SIZE_CY),
            DV_LINEITEM_SIZE_CX-(2*DV_INDENT_CX),
            DV_LINEITEM_SIZE_CY);
         break;
   }
}


//---------------------------------------------------------------------
// GetObjectRect():
//    This function calculates the smallest rectangle that includes the
// icon and caption title text of this object.
//---------------------------------------------------------------------
void WPOBJECT::GetObjectRect(LPRECT lprcObj)       // logical points.
{
   RECT  rcIcon;

   // Depending on parent container's open view...
   switch (GetParent()->GetOpenView())
   {
      case VIEW_ICON:
         // Get maximum icon rectangle (same as frame rectangle).
         GetFrameRect(&rcIcon);
         // Calculate the text rectangle.
         CalcTextRect();
         // Calculate the union of the icon rectangle with the text rectangle.
         UnionRect(lprcObj,&rcIcon,&m_rcText);
         break;

      case VIEW_DETAILS:
         // The smallest rectangle is the same as the frame rectangle.
         GetFrameRect(lprcObj);
         break;
   }
}

//---------------------------------------------------------------------
// GetObjectRectDP():
//    This function calculates the smallest rectangle that includes the
// icon and caption title text of this object in device (screen) points.
//---------------------------------------------------------------------
void WPOBJECT::GetObjectRectDP(HDC hdc,LPRECT lprcObjDP)   // device points.
{
   int iPrevMode;

   // Get object rectangle in logical points.
   GetObjectRect(lprcObjDP);
   // Set correct mapping mode.
   iPrevMode = SetMapMode(hdc,MM_TEXT);
   // Convert logical points to device points.
   LPtoDP(hdc,(LPPOINT)lprcObjDP,2);
   // Restore original mapping mode.
   SetMapMode(hdc,iPrevMode);
}

//---------------------------------------------------------------------
// OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetState():
//---------------------------------------------------------------------
void WPOBJECT::SetState(WORD wState)
{
   m_wState = wState;
}

//---------------------------------------------------------------------
// GetState():
//---------------------------------------------------------------------
WORD WPOBJECT::GetState(void)
{
   return (m_wState);
}


//---------------------------------------------------------------------
// SetStateOR():
//---------------------------------------------------------------------
void WPOBJECT::SetStateOR(WORD wState,WORD wStateMask)
{
   // Clear specified state bits.
   m_wState &= (~wStateMask);
   // OR in wanted state value.
   m_wState |= wState;
}


//---------------------------------------------------------------------
// SetFlags():
//---------------------------------------------------------------------
void WPOBJECT::SetFlags(DWORD dwFlags)
{
   m_dwFlags = dwFlags;
}

//---------------------------------------------------------------------
// GetFlags():
//---------------------------------------------------------------------
DWORD WPOBJECT::GetFlags(void)
{
   return (m_dwFlags);
}

//---------------------------------------------------------------------
// SetFlagsOR():
//---------------------------------------------------------------------
void WPOBJECT::SetFlagsOR(DWORD dwFlags,DWORD dwFlagsMask)
{
   // Clear specified flags bits.
   m_dwFlags &= (~dwFlagsMask);
   // OR in wanted flag values.
   m_dwFlags |= dwFlags;
}


//---------------------------------------------------------------------
// SORT KEY VALUE QUERY FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetSortValue():
//    This function returns (in the passed structure), the key value
// requested from the current object.  Some key values that do not
// apply at this level, are set to 0 or NULL.  Derived objects that
// do support these key values should overload this virtual function
// and copy the correct value into the passed structure.
//---------------------------------------------------------------------
void WPOBJECT::GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue)
{
   // Depending on sort key value requested...
   switch (wSortKey)
   {
      case IDM_SORTBYNAME:
      case IDM_SORTBYREALNAME:
         // Get object title string without special characters.
         strcpy(psvsSortValue->m_szValue,GetTitleNoCRLF());
         strupr(psvsSortValue->m_szValue);
         break;

      case IDM_SORTBYTYPE:
         // Get object type value.
         psvsSortValue->m_dwValue = GetType();
         break;

      case IDM_SORTBYSIZE:
      case IDM_SORTBYLASTACCESSDATE:
      case IDM_SORTBYLASTACCESSTIME:
         // No applicable at this object level.
         psvsSortValue->m_dwValue = 0L;
         break;
   }
}


//---------------------------------------------------------------------
// SHADOW MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetShadowCount():
//---------------------------------------------------------------------
int WPOBJECT::GetShadowCount(void)
{
   // Return the current number of shadow links in object.
   return (m_ShadowList.GetCount());
}

//---------------------------------------------------------------------
// GetShadowLink():
//---------------------------------------------------------------------
WPSHADOW *WPOBJECT::GetShadowLink(int nIndex)
{
   return ((WPSHADOW *)(LPVOID)m_ShadowList.GetAt(nIndex));
}

//---------------------------------------------------------------------
// AddShadow():
//---------------------------------------------------------------------
BOOL WPOBJECT::AddShadow(WPSHADOW *pwpshadowAdd)
{
   return (m_ShadowList.AddTail((DWORD)(LPVOID)pwpshadowAdd));
}


//---------------------------------------------------------------------
// DeleteShadow():
//---------------------------------------------------------------------
BOOL WPOBJECT::DeleteShadow(WPSHADOW *pwpshadowDelete)
{
   m_ShadowList.Remove((DWORD)(LPVOID)pwpshadowDelete);
   return (TRUE);
}


//---------------------------------------------------------------------
// InvalidateShadow():
//---------------------------------------------------------------------
void WPOBJECT::InvalidateShadows(BOOL fUpdateNow)
{
   int index;

   // Invalidate each shadow link to object.
   for (index=0; index<GetShadowCount(); index++)
      GetShadowLink(index)->InvalidateIcon(fUpdateNow,TRUE);
}



//---------------------------------------------------------------------
// PARENT CONTAINER OBJECT MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetParent():
//    This function sets the object's parent to a new container object.
//---------------------------------------------------------------------
void WPOBJECT::SetParent(WPCONTAINER *pwpcontainerParent)
{
   m_pwpcontainerParent = pwpcontainerParent;
}


//---------------------------------------------------------------------
// GetParent():
//    Returns the object's current parent container object.
//---------------------------------------------------------------------
WPCONTAINER *WPOBJECT::GetParent(void)
{
   return (m_pwpcontainerParent);
}


//---------------------------------------------------------------------
// OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isOpen():
//    Returns TRUE if object is "open" and FALSE otherwise.  Since this
// is a virtual function, it can be overloaded by derived objects which
// can actually be in open state.
//---------------------------------------------------------------------
BOOL WPOBJECT::isOpen()
{
   // Test the "open" state bit in object flag.
   return ((GetState()&OBJSTATE_OPEN)? TRUE : FALSE);
}

//---------------------------------------------------------------------
// isAnyChildOpen():
//    Returns TRUE if object is "open" or any child objects (if
// applicable) are in "open" state.  It returns FALSE otherwise.  Since
// this is a virtual function, it can be overloaded by derived objects
// which can actually have child objects.
//---------------------------------------------------------------------
BOOL WPOBJECT::isAnyChildOpen(void)
{
   // Return current state of object.
   return (isOpen());
}


//---------------------------------------------------------------------
// Open():
//    This function implements the "open" method of the object.  Since
// it is a virtual function, it can be overloaded by derived object
// classes and functionality can be added.
//    The default action of open for all objects is to open the
// "settings" view of that object.  If the object's open function
// is overloaded in a derived class, this functionality can be
// modified and enhanced.
//---------------------------------------------------------------------
BOOL WPOBJECT::Open(void)
{
   // Open "Settings" notebook view for object.
   return (OpenSettingsNotebook());
}


//---------------------------------------------------------------------
// Close():
//    This function implements the "close" method of the object.  Since
// it is a virtual function, it can be overloaded by derived object
// classes and functionality can be added.
//---------------------------------------------------------------------
BOOL WPOBJECT::Close(void)
{
   // Does nothing.
   return (TRUE);
}

//---------------------------------------------------------------------
// isSelected():
//    Returns TRUE if object is currently selected by parent container
// object.
//---------------------------------------------------------------------
BOOL WPOBJECT::isSelected(void)
{
   if (GetParent()!=NULL)
      return (GetParent()->isObjSelected(this));
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// CONFIRMATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ConfirmOnDelete():
//---------------------------------------------------------------------
int WPOBJECT::ConfirmOnDelete(void)
{
   char  szConfirmText[256];

   // If "confirm on delete" or "confirm on file system delete" system flag is set...
   if ((g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)||
       (g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete==TRUE))
   {
      // Check if parent container of this object is a directory object...
      if ((GetParent()!=NULL)&&(GetParent()->isDirectoryContainer()==TRUE))
         // Create special confirmation string with object's title text.
         sprintf(szConfirmText,GetString(IDSTR_OBJ_CONFIRM_ON_FILESYS_DELETE),GetTitle());
      else
      {
         // Since it is not a file system delete, make sure user wants to confirm delete...
         if (g_wpEnvironment.stSystem.fConfirmOnDelete==TRUE)
            // Create normal confirmation string with object's title text.
            sprintf(szConfirmText,GetString(IDSTR_OBJ_CONFIRM_ON_DELETE),GetTitle());
         else
            // Automatically confirm delete operation...
            return (IDYES);
      }
      // Display message box with confirmation text.
      return (MessageBox(NULL,szConfirmText,GetString(IDSTR_OBJ_CONFIRM_ON_DELETE_CAPTION),(MB_YESNOCANCEL|MB_ICONEXCLAMATION)));
   }
   else
      // Automatically confirm delete operation...
      return (IDYES);
}


//---------------------------------------------------------------------
// FIND OBJECT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Find():
//---------------------------------------------------------------------
WPOBJECT *WPOBJECT::Find(DWORD dwType,const char *pszTitle,BOOL fExact,BOOL fStopOnMatch,MATCH_RECORD **ppMatchList)
{
   MATCH_RECORD *pMatchRecord;

   if (fStopOnMatch==TRUE)
   {
      // If we need an exact match...
      if (fExact==TRUE)
      {
         // If object is of the type specified AND has the same exact title text...
         if (((dwType & GetType())==dwType) &&
             (stricmp(GetTitle(),pszTitle)==0))
            return (this);    // Return pointer to this object.
      }
      else
      {
         // If object is of the type specified AND has the some matching text...
         if (((dwType & GetType())==dwType) &&
             (stristr(GetTitle(),pszTitle)!=NULL))
            return (this);    // Return pointer to this object.
      }
   }
   else
   {
      // If we need an exact match...
      if (fExact==TRUE)
      {
         // If object is of the type specified AND has the same exact title text...
         if (((dwType & GetType())==dwType) &&
             (stricmp(GetTitle(),pszTitle)==0))
         {
            // Add a new record to the head of the list.
            pMatchRecord = new MATCH_RECORD;
            pMatchRecord->m_pNextRecord = *ppMatchList;
            *ppMatchList = pMatchRecord;
            // Add object information to new record.
            pMatchRecord->m_pwpobjMatch = this;
            pMatchRecord->m_fShow       = FALSE;
            return (this);    // Return pointer to this object.
         }
      }
      else
      {
         // If object is of the type specified AND has the some matching text...
         if (((dwType & GetType())==dwType) &&
             (stristr(GetTitle(),pszTitle)!=NULL))
         {
            // Add a new record to the head of the list.
            pMatchRecord = new MATCH_RECORD;
            pMatchRecord->m_pNextRecord = *ppMatchList;
            *ppMatchList = pMatchRecord;
            // Add object information to new record.
            pMatchRecord->m_pwpobjMatch = this;
            pMatchRecord->m_fShow       = FALSE;
            return (this);    // Return pointer to this object.
         }
      }
   }

   return (NULL);    // This object does not meet the criteria.
}


//---------------------------------------------------------------------
// DESTROY OBJECT INSTANCE FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Destroy():
//---------------------------------------------------------------------
BOOL WPOBJECT::Destroy(void)
{
   int      index,nLinksCount;
   WPSHADOW **apshadowLinks;

   // If object has a settings notebook open, close it & destory it.
   CloseSettingsNotebook();

   // If object is in open state, close it before destroying it.
   Close();

   // Query the number of shadow links associated with this object.
   nLinksCount   = GetShadowCount();
   // Allocate memory to temporarily store array of shadow links.
   apshadowLinks = new WPSHADOW *[nLinksCount];
   // Backup object's shadow links into local array of pointers.
   for (index=0; index<GetShadowCount(); index++)
      apshadowLinks[index] = GetShadowLink(index);

   // Destroy all shadow links created for this object (use local array).
   for (index=0; index<nLinksCount; index++)
      apshadowLinks[index]->Destroy();

   // Deallocate memory used to temporarily store array of shadow links.
   delete[] apshadowLinks;

   // Have Workplace Shell desktop remove this object from our parent container.
   SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_DELETE_OBJECT,0,(LPARAM)this);

   // Successfully destroyed object.
   return (TRUE);
}


//---------------------------------------------------------------------
// DRAWING FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Draw():
//    This function draws the object in its iconic form.   Derived
// classes can add or change to its behavior.
//---------------------------------------------------------------------
void WPOBJECT::Draw(HDC hdc)   // Draws object's icon & text.
{
   if (GetParent()!=NULL)
   {
      if (GetParent()->isOpen()==TRUE)
      {
         // Have the parent container object set the default colors for the object.
         GetParent()->SetDefaultColors(hdc,this);
         // Depending on which view the parent container is in...
         switch (GetParent()->GetOpenView())
         {
            case VIEW_ICON:
               // Draw object in icon view.
               DrawIconResource(hdc);
               // Draw icon text on device context.
               DrawIconText(hdc);
               break;

            case VIEW_DETAILS:
               // Draw object in details view.
               DrawDetailsIconResource(hdc);
               // Draw object information (title, name, size, and date).
               DrawDetailsIconText(hdc);
               break;
         }
      }
   }
}


//---------------------------------------------------------------------
// PopupFrame():
//    This function draws a rounded rectangle around the object.
// It uses the XOR boolean operation to combine pen colors with
// background colors in the device context.  This facilitates the
// "unframing" process since we just apply the XOR operation again
// to remove the rectangle and restore the original colors.
//---------------------------------------------------------------------
void WPOBJECT::PopupFrame(void)     // Draws rounded rect. w/dots.
{
   HDC     hdc;
   HPEN    hpenSelect,hpenOld;
   HBRUSH  hbrOld;
   RECT    rcArea;
   int     iPrevROP2;

   // We need to draw a rounded rectangle around the object.
   // Check that we have a parent container to draw on, and that the object
   // does not already have popup frame drawn around it.
   if ((GetParent()!=NULL)&&(!(m_wState&OBJSTATE_POPUPFRAME)))
   {
      // Set "popup frame drawn" state bit.
      m_wState |= OBJSTATE_POPUPFRAME;

      // Get parent's client area device context.
      hdc = ((WPFOLDER *)GetParent())->GetDC();

      // Have the parent container object set the default colors for a selected object.
      GetParent()->SetFrameColors(hdc,this);

      // Create a pen with alternating dots in a gray shade.
      hpenSelect = CreatePen(PS_DOT,1,GetTextColor(hdc));
      hpenOld    = (HPEN)SelectObject(hdc,(HGDIOBJ)hpenSelect);
      // Select the NULL brush since we do not want to fill in rectangle.
      hbrOld     = (HBRUSH)SelectObject(hdc,(HGDIOBJ)GetStockObject(NULL_BRUSH));

      // Get total object area rectangle.
      GetPopupFrameRect(&rcArea);

      // Set XOR operation between pen object and device context background.
      iPrevROP2 = SetROP2(hdc,R2_XORPEN);

      // Draw rounded rectangle using current pen and brush.
      RoundRect(hdc,
                rcArea.left,rcArea.top,rcArea.right,rcArea.bottom,
                GetSystemMetrics(SM_CXICON)/2,
                GetSystemMetrics(SM_CYICON)/2);

      // Restore original raster operation to device context.
      SetROP2(hdc,iPrevROP2);

      // Clean-up and restore original pen, and brush objects to DC.
      SelectObject(hdc,(HGDIOBJ)hbrOld);
      SelectObject(hdc,(HGDIOBJ)hpenOld);
      DeleteObject(hpenSelect);

      // Return parent's client area device context.
      ((WPFOLDER *)GetParent())->ReleaseDC();
   }
}


//---------------------------------------------------------------------
// PopupUnFrame():
//    This function removes the existing popup frame rectangle by drawing
// it again with the XOR boolean operation.
//---------------------------------------------------------------------
void WPOBJECT::PopupUnFrame(void)   // Removes popup frame rect.
{
   // Clear popup frame state bit so that PopupFrame() can re-draw popup frame.
   m_wState &= (~OBJSTATE_POPUPFRAME);

   // Just call PopupFrame() again to XOR rectangle out and restore original background.
   PopupFrame();

   // Clear popup frame state bit since PopupFrame() set it again.
   m_wState &= (~OBJSTATE_POPUPFRAME);
}


//---------------------------------------------------------------------
// Frame():
//    This function draws a frame rectangle around the object.
// It uses the XOR boolean operation to combine pen colors with
// background colors in the device context.  This facilitates the
// "unframing" process since we just apply the XOR operation again
// to remove the rectangle and restore the original colors.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::Frame(BOOL fInside)  // Draws highlight rectangle.
{
   HDC     hdc;
   HPEN    hpenSelect,hpenOld;
   RECT    rcArea;

   // Check if object is already framed.
   if ((m_wState&OBJSTATE_FRAME)==OBJSTATE_FRAME)
      return;

   // We need to draw a framed rectangle around object.
   // first, we need to set the selected color.
   if (GetParent()!=NULL)
   {
      // Get parent folder's client area device context.
      hdc = ((WPFOLDER *)GetParent())->GetDC();

      // Have the parent container object set the default colors for a framed object.
      GetParent()->SetFrameColors(hdc,this);

      // Create a pen with alternating dots in a gray shade.
      hpenSelect = CreatePen(PS_SOLID,1,GetTextColor(hdc));
      hpenOld    = (HPEN)SelectObject(hdc,(HGDIOBJ)hpenSelect);

      // Get total object area rectangle.
      GetFrameRect(&rcArea);

      // Draw frame rectangle using current pen and XOR raster operation.
      DrawFocusRect(hdc,&rcArea);

      // Clean-up and restore original pen, and brush objects to DC.
      SelectObject(hdc,(HGDIOBJ)hpenOld);
      DeleteObject(hpenSelect);

      // Return parent's device context.
      ((WPFOLDER *)GetParent())->ReleaseDC();
   }

   // Set frame state bit.
   m_wState |= OBJSTATE_FRAME;
}


//---------------------------------------------------------------------
// UnFrame():
//    This function removes the existing frame rectangle by drawing
// it again with the XOR boolean operation.
//---------------------------------------------------------------------
void WPOBJECT::UnFrame(BOOL fInside)  // removes frame rect.
{
   // If frame is not drawn already.
   if ((m_wState&OBJSTATE_FRAME)==0)
      return;

   // Clear frame state bit so that Frame() will re-draw frame.
   m_wState &= (~OBJSTATE_FRAME);

   // Just call Frame() again to XOR rectangle out and restore original background.
   Frame(fInside);

   // Clear frame state bit again since Frame() set it.
   m_wState &= (~OBJSTATE_FRAME);
}


//---------------------------------------------------------------------
// InvalidateIcon():
//    This function invalidates the object's rectangle area so that
// it can be updated.
//---------------------------------------------------------------------
void WPOBJECT::InvalidateIcon(BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   RECT rcFrame,rcInvalid;

   // If object has parent container.
   if (GetParent()!=NULL)
   {
      // Check if parent folder is actually open.
      if (GetParent()->isOpen()==TRUE)
      {
         // Get rectangle containing object's icon outter frame.
         GetFrameRect(&rcFrame);
         // Get rectangle containing object icon and caption text.
         GetObjectRect(&rcInvalid);
         // Merge both rectangles into the smallest rectangle that will contain both rectangles.
         UnionRect(&rcInvalid,&rcInvalid,&rcFrame);
         // Invalidate rectangle on container's
         GetParent()->InvalidateRect(&rcInvalid,fUpdateNow,fEraseBkgrnd);
      }
   }
}


//---------------------------------------------------------------------
// InvalidateRect():
//    This function invalidates a rectangular area on the object's
// device context.  Since this is a virtual function, derived objects
// will be able to implement this function.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Do nothing for this call.
}


//---------------------------------------------------------------------
// Invalidate():
//    This function invalidates the "opened" object area (if any). The
// default behavior for this virtual method is to invalidate the
// iconic area of the object.
//---------------------------------------------------------------------
void WPOBJECT::Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd)
{
   // Call the default invalidation handler for an object.
   InvalidateIcon(fUpdateNow,fEraseBkgrnd);
}



//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::InitializePopup(HMENU hmenuPopup)
{
   // Does nothing.
}

//---------------------------------------------------------------------
// GetMenuName():
//    This function returns a null-terminated string with the name of
// the menu resource name used as the system menu for that object.
//---------------------------------------------------------------------
const char *WPOBJECT::GetMenuName(void)
{
   return (MAKEINTRESOURCE(WPOBJECT_SYSMENU_NAME));  // Return the default generic object menu.
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPOBJECT::MenuSelection(WORD wMenuItem)
{
   HCURSOR  hcurOriginal;

   // Process system menu selection...
   switch (wMenuItem)
   {
      case IDM_OPENSETTINGS:
         OpenSettingsNotebook();
         break;

      case IDM_HELPINDEX:
         // Open help file with "Contents" of help file.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTENTS,0L);
         break;

      case IDM_HELPGENERAL:
         // Open help file with "General help" for generic object information.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_GENERALHELP_OBJECT);
         break;

      case IDM_HELPUSING:
         // Open help file with "Using help" for help manager.
         WinHelp(g_wpEnvironment.hwndDesktop,NULL,HELP_HELPONHELP,0L);
         break;

      case IDM_HELPKEYS:
         // Open help file with "Keys help" for generic object information.
         WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_KEYSHELP);
         break;

      case IDM_DELETE:
         // Confirm delete operation before destroying object.
         if (ConfirmOnDelete()==IDYES)
         {
            // Load custom "wait" cursor (i.e. clock) and set current cursor to it.
            hcurOriginal = SetCursor(LoadCursor(g_wpEnvironment.hInst,MAKEINTRESOURCE(IDC_WAITCURSOR)));
            // If object is currently placed in an object which represents the file system...
            if (GetParent()->isDirectoryContainer()==TRUE)
               // Delete object from file system.
               DeleteObjectFromFileSystem();
            // Delete object from parent container and destroy instance.
            Destroy();
            // Check if original cursor was not the same as our "wait" cursor...
            if (hcurOriginal!=GetCursor())
               // Restore original cursor and destroy custom wait cursor.
               DestroyCursor(SetCursor(hcurOriginal));
         }
         break;

      default:
         break;
   }
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// HitTest():
//    This function tests a logical point to see if it has "hit" any
// area of this object.  Notice that the object is made up of two
// different rectangles: the icon rectangle and the caption text rect.
//    We return TRUE if any area of the object is being "hit" by the
// logical point. we return FALSE otherwise.
// NOTE: The potential "hit" area is NOT the union of both rectangles.
//       They are two separate non-intersecting rectangles.
//---------------------------------------------------------------------
BOOL WPOBJECT::HitTest(POINT ptTest)   // ptTest contains logical points.
{
   RECT rcIcon;

   // Depending on parent container's open view mode...
   switch (GetParent()->GetOpenView())
   {
      case VIEW_ICON:
         // Create icon rectangle area.
         SetRect(&rcIcon,m_ptIcon.x,m_ptIcon.y,
                 (m_ptIcon.x+GetSystemMetrics(SM_CXICON)-1),
                 (m_ptIcon.y+GetSystemMetrics(SM_CYICON)-1));

         // Check if logical point is in object's icon rectangle.
         if (PtInRect(&rcIcon,ptTest)==TRUE)
            return (TRUE);

         // Check if logical point is in object's caption text rectangle.
         if (PtInRect(&m_rcText,ptTest)==TRUE)
            return (TRUE);
         break;

      case VIEW_DETAILS:
         // Get total area that makes up object.
         GetObjectRect(&rcIcon);
         // Check if logical point is in object's icon rectangle.
         if (PtInRect(&rcIcon,ptTest)==TRUE)
            return (TRUE);
         break;
   }

   // Point is not wihtin object "hit-area".
   return (FALSE);
}

//---------------------------------------------------------------------
// ContainedInRect():
//---------------------------------------------------------------------
BOOL WPOBJECT::ContainedInRect(LPRECT lprcLogical) // lprcLogical contains logical points.
{
   RECT rcIcon,rcUnion;

   // Depending on parent container's open view mode...
   switch (GetParent()->GetOpenView())
   {
      case VIEW_ICON:
         // Create icon rectangle area.
         SetRect(&rcIcon,m_ptIcon.x,m_ptIcon.y,
                 (m_ptIcon.x+GetSystemMetrics(SM_CXICON)-1),
                 (m_ptIcon.y+GetSystemMetrics(SM_CYICON)-1));
         break;

      case VIEW_DETAILS:
         // Get total area that makes up object.
         GetObjectRect(&rcIcon);
         break;
   }

   // Check if logical point is in object's icon rectangle.
   UnionRect(&rcUnion,&rcIcon,lprcLogical);

   // If union rectangle is equal to original logical rectangle,
   // then object is contained inside rectangle.
   return ((EqualRect(&rcUnion,lprcLogical)==0)? FALSE : TRUE);
}



//---------------------------------------------------------------------
// DropDPtoLP():
//    This function is in charge of converting and returning a logical
// point within the object given a device point where the drop
// occurred.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::DropDPtoLP(LPPOINT lpptDevice)
{
   // Empty function (by default it does nothing).
}


//---------------------------------------------------------------------
// isContainer():
//    This function is used to test if an object is a container object
// or just an object that allows objects to be dropped but not moved.
//    It is defined as a "virtual" function so that other objects can
// override the default behavior of "non-container" object.
//---------------------------------------------------------------------
BOOL WPOBJECT::isContainer(void)
{
   // By default, an object is a non-container unless function is overloaded.
   return (FALSE);
}

//---------------------------------------------------------------------
// isDirectoryContainer():
//    This function is used to test if an object is a container object
// or just an object that allows objects to be dropped but not moved.
//    It is defined as a "virtual" function so that other objects can
// override the default behavior of "non-container" object.
//---------------------------------------------------------------------
BOOL WPOBJECT::isDirectoryContainer(void)
{
   // By default, an object is a non-container unless function is overloaded.
   return (FALSE);
}


//---------------------------------------------------------------------
// TestDropSource():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPOBJECT::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // Check if target is this very same object.
   if (this==pwpobjDropTarget)
      return (FALSE);
   else
      return (TRUE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPOBJECT::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if source is this very same object.
   if (this==pwpobjDropSource)
      return (FALSE);
   else
      return (TRUE);
}


//---------------------------------------------------------------------
// DropSource():
//    This function is called when this object is acting as a drop
// source for a Drag & Drop operation.  The first parameter,
// "pwpobjDropTarget" is the object this container is being "dropped on".
// The second parameter, "nFunction", is the function code for the
// Drag & Drop operation the user chose to do (move, copy, etc..).
//    The third parameter is the device (or physical) point where this
// container object was dropped on the screen.  We use this point to
// position the final object correctly within the target object
// (if applicable).
//---------------------------------------------------------------------
BOOL WPOBJECT::DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice)
{
   WPCONTAINER *pwpcontainerParent;
   WPOBJECT    *pwpobjCopy;
   WPSHADOW    *pwpshadowNewShadow;

   // Check if object class accepts being dropped on an object "pwpobjDropTarget".
   if (TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Depending on what drag & drop function...
      switch (nFunction)
      {
         case DROP_MOVE:
            // If we are being dropped into a container object...
            if (pwpobjDropTarget->isContainer()==TRUE)
            {
               // If target container is a directory, then...
               if (pwpobjDropTarget->isDirectoryContainer()==TRUE)
                  // Pass original object since we do not need to duplicate it.
                  return (pwpobjDropTarget->DropTarget(this,nFunction));
               else
               {
                  // Remove ourselves from our current parent container (if any).
                  if (GetParent()!=NULL)
                  {
                     // Invalidate object icon area (do not redraw yet).
                     InvalidateIcon(FALSE,TRUE);
                     // Save pointer to our current parent container.
                     pwpcontainerParent = GetParent();
                     // Delete ourseleves from our current parent container.
                     GetParent()->DeleteObject(this);
                     // Check if current open view is not "icon view"...
                     if (pwpcontainerParent->GetOpenView()!=VIEW_ICON)
                        // Invalidate the parent container's whole client area.
                        pwpcontainerParent->Invalidate(FALSE,TRUE);
                  }
                  // Set our new icon position to the new container's logical point.
                  SetIconPos(ptDevice);
               }
            }
            // Now, move (or give) ourseleves into the drop target
            return (pwpobjDropTarget->DropTarget(this,nFunction));

         case DROP_COPY:
            // If we are being dropped into a container object...
            if (pwpobjDropTarget->isContainer()==TRUE)
            {
               // If target container is a directory, then...
               if (pwpobjDropTarget->isDirectoryContainer()==TRUE)
                  // Pass original object since we do not need to duplicate it.
                  return (pwpobjDropTarget->DropTarget(this,nFunction));
               else
               {
                  // Since we are being dropped on a container object, we have to
                  // duplicate ourseleves and all of our children objects (if applicable).
                  pwpobjCopy = CreateTreeDuplicate(NULL,this);
                  // Set our new icon position to the new container's logical point.
                  pwpobjCopy->SetIconPos(ptDevice);
                  // If container object being dropped on is open,...
                  if (pwpobjDropTarget->isOpen()==TRUE)
                     // Load icon resource of new object created.
                     pwpobjCopy->LoadIcon();
                  // After creating a duplicate of our object,
                  // send the new object copy into the target object.
                  return (pwpobjDropTarget->DropTarget(pwpobjCopy,nFunction));
               }
            }
            // Call the drop target function.
            return (pwpobjDropTarget->DropTarget(this,nFunction));

         case DROP_SHADOW:
            // If we are being dropped into a container object...
            if (pwpobjDropTarget->isContainer()==TRUE)
            {
               // Since we are a being dropped on a container object, we have to create
               // a shadow link to the original object.
               pwpshadowNewShadow = new WPSHADOW;
               // Check that we have room to add another shadow link to original object.
               if (AddShadow(pwpshadowNewShadow)==FALSE)
               {
                  // Destroy new shadow object instance.
                  delete pwpshadowNewShadow;
                  // Display message to user that no more shadows can be added to object.
                  MessageBox(NULL,GetString(IDSTR_ERR_NO_MORE_SHADOWS),GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
                  // Do not continue drop operation.
                  return (FALSE);

               }
               // Set shadow's link to original object which it represents.
               pwpshadowNewShadow->SetOriginalObject(this);
               // Set our new icon position to the new container's logical point.
               pwpshadowNewShadow->SetIconPos(ptDevice);
               // If container object being dropped on is open,...
               if (pwpobjDropTarget->isOpen()==TRUE)
                  // Load icon resource of new object created.
                  pwpshadowNewShadow->LoadIcon();
               // After creating a duplicate of our object,
               // send the new shadow object into the target object.
               return (pwpobjDropTarget->DropTarget(pwpshadowNewShadow,nFunction));
            }
            // Return failure creating shadow object (drop target not supported).
            return (FALSE);

         default:
            // No other functions supported by program.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL  WPOBJECT::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Do nothing and return.
   return (TRUE);
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isSettingsNotebookOpen():
//---------------------------------------------------------------------
BOOL WPOBJECT::isSettingsNotebookOpen(void)
{
   return ((m_pwpnbSettingsNotebook!=NULL)? TRUE : FALSE);
}

//---------------------------------------------------------------------
// OpenSettingsNotebook():
//---------------------------------------------------------------------
BOOL WPOBJECT::OpenSettingsNotebook(void)
{
   int   cx,cy;

   // Check if settings notebook is currently open...
   if (isSettingsNotebookOpen()==FALSE)
   {
      // Create settings notebook for this object.
      SetSettingsNotebook(new WPSETTINGSNB(this));
      // Check that settings notebook was created successfully...
      if (GetSettingsNotebook()==NULL)
         return (FALSE);
      // Initialize settings notebook size (depending on screen resolution)...
      if (GetSystemMetrics(SM_CXSCREEN)>1024)
      {
         // All resolutions greater than 1024x768, which include:
         // XGA & SVGA 1040x768, 1280x1024, and 1600x1280.
         cx = 640;
         cy = 540;
      }
      else
      if (GetSystemMetrics(SM_CXSCREEN)==1024)
      {
         // Resolution equal to 1024x768, which include:
         // XGA & SVGA (1024x768).
         cx = 600;
         cy = 520;
      }
      else
      {
         // All other resolutions less than 1024x768, which include:
         // EGA, VGA (640x480), and SVGA (800x600).
         cx = 525;
         cy = 425;
      }
      // Create settings notebook to the specified size (which depends on the screen resolution).
      GetSettingsNotebook()->Create(CW_USEDEFAULT,CW_USEDEFAULT,cx,cy);
      // Insert pages that are required.
      InsertSettingsNotebookPages();
      // Make notebook visible.
      GetSettingsNotebook()->ShowNotebook();
   }
   else
   {
      // Give focus to already existing folder...
      // If folder is minimized, restore to original size.
      if (IsIconic(m_pwpnbSettingsNotebook->GetHandle())!=FALSE)
         ShowWindow(m_pwpnbSettingsNotebook->GetHandle(),SW_RESTORE);

      // Set active focus to this folder.
      SetActiveWindow(m_pwpnbSettingsNotebook->GetHandle());
   }

   // Successfully opened settings notebook.
   return (TRUE);
}

//---------------------------------------------------------------------
// SetSettingsNotebook():
//---------------------------------------------------------------------
void WPOBJECT::SetSettingsNotebook(WPSETTINGSNB *pwpnbSettingsNotebook)
{
   // Store pointer to object's settings notebook.
   m_pwpnbSettingsNotebook = pwpnbSettingsNotebook;
}


//---------------------------------------------------------------------
// GetSettingsNotebook():
//---------------------------------------------------------------------
WPSETTINGSNB *WPOBJECT::GetSettingsNotebook(void)
{
   return (m_pwpnbSettingsNotebook);
}


//---------------------------------------------------------------------
// InsertSettingsNotebookPages(void)
//---------------------------------------------------------------------
void WPOBJECT::InsertSettingsNotebookPages(void)
{
   // Insert the "General" page.
   GetSettingsNotebook()->InsertGeneralPage((FARPAGEPROC)&WPOBJECT::GeneralPageProc,MAKEINTRESOURCE(IDD_GENERAL_PAGE));
}


//---------------------------------------------------------------------
// GeneralPageProc():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
void WPOBJECT::GeneralPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam)
{
   HDC         hdc;
   RECT        rcIcon;
   DLGPROC     lpfnFindIconDlgProc;
   char        szTitle[MAX_TITLE_NAME+1],szOldName[MAX_TITLE_NAME+1],szNewName[MAX_TITLE_NAME+1];

   // Depending on what action this page is being notified on...
   switch (wAction)
   {
      case PAGE_CREATE:
         // Initialize edit control.
         SetDlgItemText(hwndPage,IDE_TITLE,GetTitle());
         break;

      case PAGE_PAINT:
         // Retrieve notebook page device context.
         hdc = (HDC)LOWORD(dwParam);
         // Get icon static control's position.
         GetWindowRect(GetDlgItem(hwndPage,IDS_ICON),&rcIcon);
         // Calculate center in static control.
         rcIcon.left += ((rcIcon.right-rcIcon.left)-GetSystemMetrics(SM_CXICON))/2;
         rcIcon.top  += ((rcIcon.bottom-rcIcon.top)-GetSystemMetrics(SM_CYICON))/2;
         // Convert from screen coord. to client (dialog) coord.
         ScreenToClient(hwndPage,(LPPOINT)&rcIcon);
         // Paint current icon resource on folder.
         if ((GetSettingsNotebook()->GetIconHandle()!=NULL)&&
             (GetSettingsNotebook()->GetIconHandle()!=(HICON)1) )
         {
            // Draw icon in center of static control.
            DrawIcon(hdc,rcIcon.left,rcIcon.top,GetSettingsNotebook()->GetIconHandle());
         }
         break;

      case PAGE_COMMAND:
         // Depending on which button was pushed...
         switch (LOWORD(dwParam))
         {
            case IDB_FIND:
               // Open "Find..." icon dialog box...
               lpfnFindIconDlgProc = (DLGPROC)MakeProcInstance((FARPROC)FindIconDlgProc,g_wpEnvironment.hInst);
               DialogBoxParam(g_wpEnvironment.hResInst,MAKEINTRESOURCE(IDD_FINDICON),hwndPage,lpfnFindIconDlgProc,(LPARAM)this);
               FreeProcInstance((FARPROC)lpfnFindIconDlgProc);
               break;

            case IDB_DEFAULT:
               // Copy default object title.
               SetDlgItemText(hwndPage,IDE_TITLE,"Name");
               break;

            case IDB_UNDO:
               // Restore original object title.
               SetDlgItemText(hwndPage,IDE_TITLE,GetTitle());
               break;

            case IDB_HELP:
               // Open help file with "General" page help information.
               WinHelp(g_wpEnvironment.hwndDesktop,GetString(IDSTR_HELP_FILENAME),HELP_CONTEXT,HELPID_SETNB_GENERALPAGE);
               break;
         }
         break;

      case PAGE_DONE:
         // Get "General" page edit control's text.
         GetWindowText(GetDlgItem(hwndPage,IDE_TITLE),szTitle,MAX_TITLE_NAME);
         // If object is currently owned by a directory folder...
         if ((GetParent()!=NULL) && (GetParent()->isDirectoryContainer()==TRUE))
         {
            // Check whether the new title text is different from the original title text...
            if (lstrcmpi(szTitle,GetTitle())!=0)
            {                                   
               // Try to rename file or directory to match as closely as possible the title string.
               // EXAMPLE: "This is my application" -> "THIS_IS_."
               // (name clashes w/ "TEST.TXT") -> "TEST1.TXT"
               // Create "FAT" sanctioned file/directory name from what user typed in. 
               strcpy(szTitle,GetParent()->MakeFATName(szTitle,TRUE,((WPDIRECTORY *)GetParent())->GetPath()));
               // Create old and new file name paths.
               sprintf(szOldName,"%s\\%s",((WPDIRECTORY *)GetParent())->GetPath(),GetTitle());
               sprintf(szNewName,"%s\\%s",((WPDIRECTORY *)GetParent())->GetPath(),szTitle);
               // Try to rename file/directory...
               if (RenameFileOrDirectory(szOldName,szNewName)==TRUE)
                  // Change other internal information of object associated with file.
                  UpdateFileSystemObjectName(szTitle);
               else         
                  // Do not change anything since renaming of file failed.
                  strcpy(szTitle,GetTitle());
            }
         }   
         // Copy new title into object.
         SetTitle(szTitle);
         // If object has a parent container, invalidate the whole client area.
         if (GetParent()!=NULL)
            GetParent()->Invalidate(TRUE,TRUE);
         break;
   }
}


//---------------------------------------------------------------------
// CloseSettingsNotebook():
//---------------------------------------------------------------------
BOOL WPOBJECT::CloseSettingsNotebook(void)
{
   // Check if settings notebook is open...
   if (isSettingsNotebookOpen()==TRUE)
   {
      // Close settings notebook before destroying.
      m_pwpnbSettingsNotebook->Close();
      // Destroy notebook object.
      delete m_pwpnbSettingsNotebook;
      // Clear pointer to settings notebook (since we use it to test existence).
      m_pwpnbSettingsNotebook = NULL;
   }
   // Successfully closed and destroyed settings notebook.
   return (TRUE);
}

//---------------------------------------------------------------------
// OBJECT TO FILE SYSTEM FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CopyObjectToFileSystem():
//---------------------------------------------------------------------
BOOL WPOBJECT::CopyObjectToFileSystem(char *pszDestPath)
{
   return (TRUE);
}

//---------------------------------------------------------------------
// DeleteObjectFromFileSystem():
//---------------------------------------------------------------------
BOOL WPOBJECT::DeleteObjectFromFileSystem(void)
{
   return (TRUE);
}


//---------------------------------------------------------------------
// WORKPLACE SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPOBJECT::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_OBJECT_STRUCT   fosData;

   // Read object class information.
   if (read(hProfile,&fosData,sizeof(FILE_OBJECT_STRUCT))!=sizeof(FILE_OBJECT_STRUCT))
      return (FALSE);

   // Initialize object values (do not use member functions to set these values).
   m_dwType = fosData.m_dwType;
   m_wUniqueID = fosData.m_wUniqueID;
   strcpy(m_szTitle,fosData.m_szTitle);
   m_ptIcon = fosData.m_ptIcon;
   strcpy(m_szIconPath,fosData.m_szIconPath);
   m_wIconIndex = fosData.m_wIconIndex;
   m_wState = fosData.m_wState;
   m_dwFlags = fosData.m_dwFlags;

   // Check if object has a parent container object...
   if (pwpcontainerParent!=NULL)
      // Add newly read object to parent container object and return.
      return (pwpcontainerParent->AddObject(this));
   else
      // Return success.
      return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPOBJECT::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_OBJECT_STRUCT   fosData;

   // Initialize object values.
   fosData.m_dwType     = GetType();
   fosData.m_wUniqueID  = GetUniqueID();
   strcpy(fosData.m_szTitle,GetTitle());
   fosData.m_ptIcon     = GetIconPos();
   GetIcon(fosData.m_szIconPath,&(fosData.m_wIconIndex));
   fosData.m_wState     = m_wState;
   fosData.m_dwFlags    = m_dwFlags;

   // Write object class information.
   if (write(hProfile,&fosData,sizeof(FILE_OBJECT_STRUCT))!=sizeof(FILE_OBJECT_STRUCT))
      return (FALSE);

   return (TRUE);
}

// EOF WPOBJECT.CPP

