//=====================================================================
// WPOBJECT.HPP
//    WPOBJECT Class declaration.
//
// Date Created :  01/24/94
// Last Update  :  11/10/95
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

#ifndef __WPOBJECT_PROTOTYPED__

#define __WPOBJECT_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
#include "list.hpp"

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Object state flags (mask of bits reserved for object class 0x000F).
#define OBJSTATE_OPEN         0x0001   // open/closed.
#define OBJSTATE_POPUPFRAME   0x0002   // popup frame/no popup frame.
#define OBJSTATE_FRAME        0x0004   // square frame/no square frame.

// Object's behavior and functionality flags.
// Object's minimized button (if any) behavior.
#define MINBEHAVIOR_FLAGS_MASK      0x0000000FL
#define MINBEHAVIOR_HIDE            0x00000001L
#define MINBEHAVIOR_MIN_TO_VIEWER   0x00000002L
#define MINBEHAVIOR_MIN_TO_DESKTOP  0x00000003L
// Object open behavior flags.
#define OPENBEHAVIOR_FLAGS_MASK     0x000000F0L
#define OPENBEHAVIOR_DISPLAY        0x00000010L
#define OPENBEHAVIOR_CREATE         0x00000020L

// Object system menu name (popup menu).
#define WPOBJECT_SYSMENU_NAME       ObjectPopUpMenu   // defined in WPSHELL.RC

// Details view fields' size and position constants.
#define DV_INDENT_CX             0
#define DV_INDENT_CY             28
// Overall size and position.
#define DV_LINEITEM_SIZE_CX      (DV_TIME_INDENT_CX+DV_TIME_SIZE_CX)
#define DV_LINEITEM_SIZE_CY      (GetSystemMetrics(SM_CXICON)+4)
// Icon column.
#define DV_ICON_INDENT_CX        (DV_INDENT_CX+18)
#define DV_ICON_INDENT_CY        1
#define DV_ICON_SIZE_CX          GetSystemMetrics(SM_CXICON)
#define DV_ICON_SIZE_CY          GetSystemMetrics(SM_CYICON)
// Title column.
#define DV_TITLE_INDENT_CX       ((2*DV_ICON_INDENT_CX)+DV_ICON_SIZE_CX)
#define DV_TITLE_INDENT_CY       0
#define DV_TITLE_SIZE_CX         200
#define DV_TITLE_SIZE_CY         DV_LINEITEM_SIZE_CY
// Name column.
#define DV_NAME_INDENT_CX       (DV_TITLE_INDENT_CX+DV_TITLE_SIZE_CX)
#define DV_NAME_INDENT_CY       0
#define DV_NAME_SIZE_CX         120
#define DV_NAME_SIZE_CY         DV_LINEITEM_SIZE_CY
// Size column.
#define DV_SIZE_INDENT_CX       (DV_NAME_INDENT_CX+DV_NAME_SIZE_CX)
#define DV_SIZE_INDENT_CY       0
#define DV_SIZE_SIZE_CX         60
#define DV_SIZE_SIZE_CY         DV_LINEITEM_SIZE_CY
// Date column.
#define DV_DATE_INDENT_CX       (DV_SIZE_INDENT_CX+DV_SIZE_SIZE_CX)
#define DV_DATE_INDENT_CY       0
#define DV_DATE_SIZE_CX         110
#define DV_DATE_SIZE_CY         DV_LINEITEM_SIZE_CY
// Time column.
#define DV_TIME_INDENT_CX       (DV_DATE_INDENT_CX+DV_DATE_SIZE_CX)
#define DV_TIME_INDENT_CY       0
#define DV_TIME_SIZE_CX         110
#define DV_TIME_SIZE_CY         DV_LINEITEM_SIZE_CY

// Popup frame (rounded rect.) size and position.
#define DV_POPUPFRAME_INDENT_CX  (DV_INDENT_CX+1)
#define DV_POPUPFRAME_INDENT_CY  2
#define DV_POPUPFRAME_SIZE_CX    (DV_LINEITEM_SIZE_CX-DV_POPUPFRAME_INDENT_CX)
#define DV_POPUPFRAME_SIZE_CY    (GetSystemMetrics(SM_CYICON)-1)

// Maximum number of shadow links per object.
#define MAX_SHADOW_OBJECTS       3

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
// File structure used to store object's attributes to profile.
typedef struct {
   DWORD    m_dwType;
   WORD     m_wUniqueID;
   char     m_szTitle[MAX_TITLE_NAME+1];
   POINT    m_ptIcon;
   char     m_szIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wIconIndex;
   WORD     m_wState;
   DWORD    m_dwFlags;
} FILE_OBJECT_STRUCT;

// "Find..." function's match record structure (used by Find() function).
typedef struct _MATCH_RECORD {
   WPOBJECT             *m_pwpobjMatch;
   BOOL                 m_fShow;
   struct _MATCH_RECORD *m_pNextRecord;
} MATCH_RECORD;

// "Sort" functions use the following structure to query key values from
// the objects being sorted (used by GetSortValue() and
// SortCompareObjects() functions).
typedef union {
   char     m_szValue[MAX_TITLE_NAME+1];
   DWORD    m_dwValue;
} SORT_VALUE_STRUCT;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPOBJECT
{
   private:
      DWORD       m_dwType;           // Object type value.
      WORD        m_wUniqueID;        // Object ID used only by read/write profile functions.
      char        m_szTitle[MAX_TITLE_NAME+1];
      POINT       m_ptIcon;           // Object's icon top-left coordinate.
      RECT        m_rcText;           // Object's title text rectangle.
      RECT        m_rcObject;         // Object's total size rectangle.
      HICON       m_hIcon;            // Object's icon handle.
      char        m_szIconPath[MAX_PATH_FILENAME+1];
      WORD        m_wIconIndex;

      WORD        m_wState;           // Object's state and visual state.
      DWORD       m_dwFlags;          // Object's behavior and functionality flags.

      LIST        m_ShadowList;       // Shadow pointer list.

      WPCONTAINER  *m_pwpcontainerParent;    // Object's parent container object.

      WPSETTINGSNB *m_pwpnbSettingsNotebook; // Object's setting notebook.


   protected:
      // Support functions.
      char          *stristr(const char *s1,const char *s2);

      // "Icon view" drawing functions.
      void          DrawIconResource(HDC hdc);
      void          DrawIconText(HDC hdc);

      // "Details view" drawing functions.
      void          DrawDetailsIconResource(HDC hdc);
      virtual void  DrawDetailsIconText(HDC hdc);


      // Class name management functions.
      const char    *CreateUniqueClassName(void);
      BOOL          DestroyUniqueClassName(const char *pszClassName);
      BOOL          isWorkplaceShellClassName(char *pszClassName);
      BOOL          isWorkplaceShellWindow(HWND hwnd);

      // Private data access functions (to be used by derived classes).
      void          SetIconHandle(HICON hIcon);

   public:
      WPOBJECT(void);
      WPOBJECT(WPOBJECT *pwpobjDefaults);
      ~WPOBJECT(void);

      // Calculate object's title text rectangle for "icon view".
      void          CalcTextRect(void);

      // Rectangle utility function.
      void          SetRectCXCY(LPRECT lprcRect,int x,int y,int cx,int cy);

      // Object initialization function.
      virtual void  Initialize(BOOL fIsShell);

      // Type management functions.
      virtual DWORD GetType(void);
      virtual void  SetType(DWORD dwType);

      // Unique ID management functions.
      WORD          GetUniqueID(void);
      void          SetUniqueID(WORD wUniqueID);

      // Title management functions.
      virtual void       SetTitle(const char *pszTitle);
      virtual const char *GetTitle(void);
      virtual const char *GetTitleNoCRLF(void);
      virtual const char *GetCaptionTitle(void);
      virtual void       UpdateFileSystemObjectName(char *szNewTitle);
      

      // Appearance and position functions.
      void          SetIconPos(POINT ptIcon);           // logical points.
      void          SetIconPos(int x,int y);            // logical points.
      POINT         GetIconPos(void);
      virtual void  SetIcon(char *pszIconPath,WORD wIconIndex);
      virtual void  GetIcon(char *pszIconPath,WORD *pwIconIndex);
      HICON         GetIconHandle(void);

      virtual void  GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      HICON         ExtractIcon(void);
      virtual BOOL  LoadIcon(void);
      virtual void  UpdateIcon(char *pszIconPath=NULL,WORD wIconIndex=0);
      BOOL          DestroyIcon(void);
      void          GetPopupFrameRect(LPRECT lprcPopupFrame);   // logical points.
      void          GetFrameRect(LPRECT lprcFrame);             // logical points.
      void          GetObjectRect(LPRECT lprcObj);              // logical points.
      void          GetObjectRectDP(HDC hdc,LPRECT lprcObjDP);  // device points.

      // Object state functions.
      virtual void  SetState(WORD wState);
      virtual WORD  GetState(void);
      virtual void  SetStateOR(WORD wState,WORD wStateMask);
      virtual void  SetFlags(DWORD dwFlags);
      virtual DWORD GetFlags(void);
      virtual void  SetFlagsOR(DWORD dwFlags,DWORD dwFlagsMask);

      // Sort key values query function.
      virtual void   GetSortValue(WORD wSortKey,SORT_VALUE_STRUCT *psvsSortValue);

      // Shadow management functions.
      int           GetShadowCount(void);
      WPSHADOW      *GetShadowLink(int nIndex);
      BOOL          AddShadow(WPSHADOW *pwpshadowAdd);
      BOOL          DeleteShadow(WPSHADOW *pwpshadowDelete);
      void          InvalidateShadows(BOOL fUpdateNow);

      // Parent container object management functions.
      void          SetParent(WPCONTAINER *pwpcontainerParent);
      WPCONTAINER   *GetParent(void);

      // Object state functions.
      virtual BOOL  isOpen(void);
      virtual BOOL  isAnyChildOpen(void);
      virtual BOOL  Open(void);
      virtual BOOL  Close(void);
      BOOL          isSelected(void);

      // Confirmation function.
      virtual int   ConfirmOnDelete(void);

      // Find object function.
      virtual WPOBJECT *Find(DWORD dwType,const char *pszTitle,BOOL fExact=TRUE,BOOL fStopOnMatch=TRUE,MATCH_RECORD **ppMatchList=NULL);

      // Destroy object instance function.
      virtual BOOL  Destroy(void);

      // Drawing functions.
      virtual void  Draw(HDC hdc);           // draws object's icon & text.
      virtual void  PopupFrame(void);        // draws rounded rect.
      virtual void  PopupUnFrame(void);      // removes the rounded rect.
      virtual void  Frame(BOOL fInside);     // draws square frame.
      virtual void  UnFrame(BOOL fInside);   // removes square frame.

      virtual void  InvalidateIcon(BOOL fUpdateNow,BOOL fEraseBkgrnd);
      virtual void  InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd);
      virtual void  Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd);

      // Popup menu properties.
      virtual void  InitializePopup(HMENU hmenuPopup);
      virtual const char *GetMenuName(void);
      virtual void  MenuSelection(WORD wMenuItem);

      // Drag & drop property management functions.
      virtual BOOL  HitTest(POINT ptTest);
      virtual BOOL  ContainedInRect(LPRECT lprcLogical);
      virtual void  DropDPtoLP(LPPOINT lpptDevice);
      virtual BOOL  isContainer(void);
      virtual BOOL  isDirectoryContainer(void);
      virtual BOOL  TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      virtual BOOL  TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      virtual BOOL  DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice);
      virtual BOOL  DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);

      // Settings notebook functions.
      virtual BOOL  isSettingsNotebookOpen(void);
      virtual BOOL  OpenSettingsNotebook(void);
      void          SetSettingsNotebook(WPSETTINGSNB *pwpnbSettingsNotebook);
      WPSETTINGSNB  *GetSettingsNotebook(void);
      virtual void  InsertSettingsNotebookPages(void);
      virtual void  GeneralPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      virtual BOOL  CloseSettingsNotebook(void);

      // Object to file system functions.
      virtual BOOL  CopyObjectToFileSystem(char *pszDestPath);
      virtual BOOL  DeleteObjectFromFileSystem(void);

      // Shell profile management functions.
      virtual BOOL  ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      virtual BOOL  WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPOBJECT.HPP

