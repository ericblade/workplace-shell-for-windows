//=====================================================================
// WPFOLDER.HPP
//    WPFOLDER Class declaration.
//
// Date Created :  01/26/94
// Last Update  :  09/06/95
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

#ifndef __WPFOLDER_PROTOTYPED__

#define __WPFOLDER_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
#include <shellapi.h>

//---------------------------------------------------------------------
// LOCAL ENUMERATIONS
//---------------------------------------------------------------------
// Values used in mouse management functions.
enum { MBS_UP=0, MBS_DOWN };
enum { VIEW_ICON=0, VIEW_TREE, VIEW_DETAILS };
enum { DRAG_SELECT=0, DRAG_MOVE, DRAG_COPY, DRAG_SHADOW, DRAG_SELECT_RUBBERBAND };
enum { DROP_SELECT=0, DROP_MOVE, DROP_COPY, DROP_SHADOW, DROP_SELECT_RUBBERBAND };


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Folder object state flags (mask of bits reserved for folder class 0x00F0).
#define OBJSTATE_TRACKPOPUPMENU     0x0010   // in popup menu tracking mode.

// Folder's behavior and functionality flags.
// Folder's minimize button appearance.
#define BUTTONAPPEARANCE_FLAGS_MASK 0x0000F000L
#define BUTTONAPPEARANCE_HIDE       0x00001000L
#define BUTTONAPPEARANCE_MINIMIZE   0x00002000L

// Folder's image flags (normal, tiled, scaled).
#define IMAGE_FLAGS_MASK            0x000F0000L
#define IMAGE_COLOR_ONLY            0x00000000L    // No image, color only.
#define IMAGE_NORMAL                0x00010000L    // Normal image (centered on folder).
#define IMAGE_TILED                 0x00020000L    // Tiled image.
#define IMAGE_SCALED                0x00040000L    // Scaled image (strechted to fit folder).

// Folder's text transparency flags.    
#define TEXT_APPEARANCE_FLAGS_MASK  0x00100000L
#define TEXT_OPAQUE                 0x00000000L
#define TEXT_TRANSPARENT            0x00100000L

// Folder's icon view format flags.    
#define ICONVIEW_FORMAT_FLAGS_MASK  0x00600000L
#define ICONVIEW_FORMAT_NONGRID     0x00000000L
#define ICONVIEW_FORMAT_FLOWED      0x00200000L
#define ICONVIEW_FORMAT_NONFLOWED   0x00400000L


// Undocumented Windows 3.1 Drag & Drop constants.
#define DRAGOBJ_PROGRAM             0x0001   // Single: program file.
#define DRAGOBJ_DATA                0x0002   // Single: data file.
#define DRAGOBJ_DIRECTORY           0x0003   // Single: directory name.
#define DRAGOBJ_MULTIPLE            0x0004   // Multiple: any of the above.

// Undocumented Windows 3.1 Drag & Drop interface messsages.
#define WM_DROPOBJECT               0x022A
#define WM_QUERYDROPOBJECT          0x022B
#define WM_BEGINDRAG                0x022C
#define WM_DRAGLOOP                 0x022D
#define WM_DRAGSELECT               0x022E
#define WM_DRAGMOVE                 0x022F


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef struct {
   // Folder background color.
   COLORREF    m_rgbBackground;
   LOGFONT     m_lfIconText;
   COLORREF    m_clrrefIconTextColor;
   char        m_szImagePath[MAX_PATH_FILENAME+1];
   WORD        m_wOpenView;
} FILE_FOLDER_STRUCT;

// Undocumented Windows 3.1 Drag & Drop structure.
typedef struct {
   HWND        hwndSource; // File Manager window that owns the source listbox.
   HANDLE      hUnknown;   // Unknown (maybe handle of drag icon).
   WORD        wFlags;     // One of the DRAGOBJ_ constants above.
   char near   *szList;    // Near pointer to list of files.
   HANDLE      hOfStruct;  // Handle of global OFSTRUCT If single file and has assoication.
   int         x,y;        // Current cursor position in client coords.
   LONG        lUnknown;   // Unknown.
} DRAGINFO, *LPDRAGINFO;

// "Copy...", "Move...", and "Create another..." input/output dialog structure.
typedef struct {
   WORD        m_wFuncType;                     // input.
   WPFOLDER    *m_pwpfolderParent;              // input.
   BOOL        m_fSingleObject;                 // input.
   char        m_szNewName[MAX_TITLE_NAME+1];   // input/output.
   WPFOLDER    *m_pwpfolderTarget;              // output.
} COPYMOVECREATE_STRUCT;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPFOLDER : public WPCONTAINER , public WPWINDOW
{
   private:
      // Folder visual attributes.
      LOGFONT     m_lfIconText;              // Icon text logical font.
      COLORREF    m_clrrefIconTextColor;     // Icon text color.
      char        m_szImagePath[MAX_PATH_FILENAME+1];   // Folder background image file.
      HPALETTE    m_hpalImage;               // Used to keep image's logical palette.
      HBITMAP     m_hbmpImage;               // Used to keep image's device-dependent bitmap handle.

      // Popup menu.
      WPOBJECT    *m_pwpobjPopup;            // Object selected for popup menu.

      // Mouse button states.
      WORD        m_wLeftButtonState;        // State of left mouse button.
      WORD        m_wRightButtonState;       // State of right mouse button.
      POINT       m_ptMouseFirstClick;       // First-click mouse coordinate.
      BOOL        m_fNCLButtonDown;          // Non-client area left mouse click flag.

      // Folder object view (VIEW_* flags).
      WORD        m_wOpenView;               // Current open folder view.

      // Drag & Drop information.

      // Drag information.
      BOOL        m_fDragMode;               // TRUE = In drag/select mode.
      WORD        m_wDragFunction;           // Move, Copy, Shadow, etc...

      // Drag source information.
      BOOL        m_fDragSelectedObjs;       // TRUE = Draging selected objects.
      WPOBJECT    *m_pwpobjDrag;             // FALSE = Object being dragged.
      // Drag functions temporary information
      // (only used when DRAG_SHADOW & DRAG_SELECT_RUBBERBAND).
      BOOL        m_fDragLineDrawn;
      POINT       m_ptDragLineStartPoint;     
      POINT       m_ptDragLineEndPoint;      

      // Drop information.
      POINT       m_ptDrop;                  // Window point where drop occurred.

      // Drop target information.
      BOOL        m_fTargetIsFolderArea;     // TRUE = mouse on client area of folder.
      WPOBJECT    *m_pwpobjDropTarget;       // Drop target object.

   protected:
      // Folder visual attributes.
      COLORREF    m_clrrefBackground;        // Background color.
      // Settings notebook - "Background" page temporary preview image data.
      HBITMAP     m_hbmpPreviewImage;
      HPALETTE    m_hpalPreviewPalette;
      
      
   // ------------------ SOURCE MODULE #1 BEGINS -------------------------
      
   private:
      // Functions to setup and cleanup folder's view device context.
      void              SetUpDC(HDC hdc,HFONT *phfontOld,int *piPrevBk,int *piPrevMap);
      void              CleanUpDC(HDC hdc,HFONT hfontOld,int iPrevBk,int iPrevMap);

      // Functions to paint and reload image bitmaps.
      void              PaintFolderImage(HDC hdc);
      void              ReloadFolderImage(void);

      // Functions to calculate folder's logical domain.
      void              CalculateLogicalDomainRect(LPRECT lprcLogDomain);

      // Functions used by FindObjectAtRelativePosition() function.
      WPOBJECT          *IconViewFindObject(WORD wEventID,WPOBJECT *pwpobjCurrent);
      WPOBJECT          *DetailsViewFindObject(WORD wEventID,WPOBJECT *pwpobjCurrent);

      // Functions used by "Find..." menu function.
      BOOL              FindFunction(void);


   protected:
      // Folder Windows 3.1 class management functions.
      virtual void      SetClassBackgroundBrush(HBRUSH *phbrBackground);
      virtual char      *RegisterFolderClass(WNDPROC lpfnFolderWndProc);

      // Drag & drop target management functions.
      virtual WPOBJECT  *GetDropTarget(POINT ptDevice,BOOL *fTargetIsFolderArea);
      virtual BOOL      isTargetValid(WPOBJECT *pwpobjTarget);
      virtual void      DrawShadowDragLine(POINT ptDevice,BOOL fRemoveLineOnly);
      virtual void      DrawRubberBandRect(POINT ptDevice,BOOL fRemoveLineOnly);

      // Support functions for mouse management functions.
      BOOL              HandleDragModeInit(WORD wFlags,POINT ptDevice); // Used by OnMouseMove().
      void              HandleRubberBandSelection(POINT ptDevice);      // Used by OnMouseMove().
      void              HandleSelectOperation(POINT ptDevice);      // Used by OnMouseMove().
      void              HandleMoveCopyOrShadowOperation(POINT ptDevice);// Used by OnMouseMove().
      
      // Mouse management functions (all points are in device coordinates).
      virtual void      OnLeftMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice);
      virtual void      OnRightMouseButton(UINT uMsg,WORD wFlags,POINT ptDevice);
      virtual void      OnMouseMove(WORD wFlags,POINT ptDevice);
      virtual void      OnLeftMouseButtonDblClk(POINT ptDevice);

      // Cursor creation functions.
      BOOL              isRGBNormal(COLORREF clrrefPixel);
      BOOL              isRGBWhite(COLORREF clrrefPixel);
      HCURSOR           CursorFromBitmap(HBITMAP hbmCursor);
      HBITMAP           CreateCursorBitmap(void);
      void              DrawIconOnBitmap(HBITMAP hbm,HICON hIcon);
      virtual void      CreateMoveCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag);
      virtual void      CreateCopyCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag);
      virtual void      CreateShadowCursors(HCURSOR *phcurDrag,HCURSOR *phcurNoDrag);

      // Folder views management functions
      BOOL              SortCompareObjects(WORD wSortKey,WPOBJECT *pwpobjObj1,WPOBJECT *pwpobjObj2);
      void              SortIcons(WORD wSortKey);
      void              ArrangeIconsInFlowedFormat(void);
      void              ArrangeIconsInNonFlowedFormat(void);
      void              ArrangeIcons(DWORD dwFormat=(DWORD)-1L);
      void              FormatIconView(void);

      // Draw min/max buttons function.
      void              DrawMinMaxButtons(HDC hdc,int cx,BOOL fMinDown,BOOL fMaxDown);

      // Create another, create shadow, copy, and move objects functions.
      void              CreateAnotherFunc(void);
      void              CreateShadowFunc(void);
      void              CopyFunc(void);
      void              MoveFunc(void);

      // Delete selected objects function.
      void              DeleteSelectedObjects(void);

      // Windows 3.1 Drag & Drop service functions.
      BOOL              Win31DDQueryDropObject(BOOL fClientArea,LPDRAGINFO pDragInfo);
      BOOL              Win31DDDragSelect(BOOL fEnteringWindow,LPDRAGINFO pDragInfo);
      LRESULT           Win31DDDropFiles(HDROP hDrop);
      void              Win31DDCreateWPSObjectFromFileDropped(WORD wFileFlags,char *pszPathName,POINT ptDrop);

   public:
      WPFOLDER(void);
      WPFOLDER(WPFOLDER *pwpfolderDefaults);

      // Folder Windows 3.1 class management functions.
      virtual void      UnregisterFolderClass(void);

      // Folder caption text function.
      const char        *GetCaptionTitle(void);

      // Folder default values.
      void              GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);

      // Folder state functions.
      BOOL              Open(void);
      BOOL              Close(void);
      WPOBJECT          *GetObjectPopup(void);
      void              SetObjectPopup(WPOBJECT *pwpobjPopup);
      WORD              GetOpenView(void);
      void              SetOpenView(WORD wOpenView);

      // Intialization function (called after OnCreate()).
      virtual void      OnInitializeFolder(void);
                      
   // ------------------ SOURCE MODULE #2 BEGINS -------------------------
                      
      // Appearance attributes set & query functions
      virtual void      SetBackgroundColor(COLORREF clrrefBackground);
      virtual COLORREF  GetBackgroundColor(void);
      void              SetIconTextLogFont(LOGFONT *plogfontIconText);
      void              GetIconTextLogFont(LOGFONT *plogfontIconText);
      void              SetIconTextColor(COLORREF clrrefColor);
      COLORREF          GetIconTextColor(void);
      virtual BOOL      GetImageAttribute(DWORD wAttributeFlag);
      virtual void      SetImageAttribute(DWORD wAttributeFlag);
      virtual char      *GetImagePath(void);
      virtual void      SetImagePath(char *pszImagePath);
      HPALETTE          GetImagePalette(void);
      void              SetImagePalette(HPALETTE hpalImage);
      HBITMAP           GetImageBitmapHandle(void);
      void              SetImageBitmapHandle(HBITMAP hbmpImage);

      // Color selection functions.
      void              SetDefaultColors(HDC hdc,WPOBJECT *pwpobjDrawn);
      void              SetFrameColors(HDC hdc,WPOBJECT *pwpobjDrawn);

      // Children object's visual representation functions.
      POINT             GetFirstAvailableIconPos(POINT *pptStart = NULL);
      void              UpdateIcon(char *pszIconPath,WORD wIconIndex);
      virtual BOOL      LoadIcons(void);
      virtual BOOL      DestroyIcons(void);

      // Drag & drop support functions.
      virtual void      OnDrop(POINT ptDevice);         // drop point.
      virtual WPOBJECT  *ChildHitTest(POINT ptDevice);  // device point.
      virtual int       GetChildrenContainedInRect(RECT rcLogical,LIST *plistHit); // logical point.
      void              DropDPtoLP(LPPOINT lpptDevice); // from device -> logical.

      // Mouse management functions (all points are in device coordinates).
      virtual void      CreateDragCursors(WORD wDragFunction,HCURSOR *phcurDrag,HCURSOR *phcurNoDrag);
      virtual void      OnMouseMessage(UINT uMsg,WORD wFlags,POINT ptWindow);

      // Confirmation function.
      int               ConfirmOnDelete(void);

      // Drawing functions.
      virtual void      DrawChildren(HDC hdc);
      void              DrawDetailsViewTable(HDC hdc);
      void              PopupFrame(void);       // draws rounded rect.
      void              PopupUnFrame(void);     // removes the rounded rect.
      void              Frame(BOOL fInside);    // draws square frame.
      void              UnFrame(BOOL fInside);  // removes square frame.
      void              InvalidateRect(LPRECT lprcArea,BOOL fUpdateNow,BOOL fEraseBkgrnd);
      void              Invalidate(BOOL fUpdateNow,BOOL fEraseBkgrnd);
      virtual void      UpdateWindow(void);

      // Objects position calculation function.
      WPOBJECT          *FindObjectAtRelativePosition(WORD wEventID,WPOBJECT *pwpobjCurrent);

      // Popup menu properties.
      void              InitializePopup(HMENU hmenuPopup);
      const char        *GetMenuName(void);
      void              MenuSelection(WORD wMenuItem);
      virtual void      KeyboardEvent(WORD wEventID);
      
      // Window messages management functions.
      virtual LRESULT   ClientWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
      virtual void      OnCreate(void);
      virtual void      OnMove(POINT ptTopLeft);
      virtual void      OnSize(WORD wSizeType,int nClientWidth,int nClientHeight);
      virtual void      OnTimer(WORD wTimerID);
      virtual void      OnSysCommand(WORD wValue,LPARAM lParam);
      virtual void      OnCommand(WORD wValue);
      virtual void      OnPopupMenu(POINT ptDevice);
      virtual void      OnNCPaint(void);
      virtual void      OnNCActivate(BOOL fActive);
      virtual void      OnNCMouseMove(WORD wHitTestCode,POINT ptDevice);
      virtual void      OnNCLButton(UINT uMsg,WORD wHitTestCode,POINT ptDevice);
      virtual LRESULT   OnWin31DragAndDropMsg(UINT uMsg,WPARAM wParam,LPARAM lParam);
      virtual LRESULT   OnPaletteMessage(UINT uMsg,HWND hWndPalette);
      virtual LRESULT   OnEraseBackground(HDC hdc);
      virtual void      OnPaint(void);
      virtual void      OnClose(void);

      // Settings notebook functions.
      void              InsertSettingsNotebookPages(void);
      // "General" page functions.
      void              GeneralPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      // "Window" page functions.
      DWORD             GetWindowPageFlags(HWND hwndPage);
      void              UpdateWindowPageControls(HWND hwndPage,DWORD dwFlags);
      void              WindowPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      // "Background" page functions.
      BOOL              ChangeColor(HWND hwndOwner);
      virtual void      ImageControls(HWND hwndBackground,BOOL fEnable);
      void              UpdateBackgroundPageImageSettings(HWND hwndPage,WORD wControlID);
      BOOL              ReloadPreviewAreaImage(HBITMAP *phbmpImage,HPALETTE *phpalImage,BOOL fFreeOnly);
      void              UpdatePreviewArea(HDC hdc,HWND hwndPage);
      void              BackgroundPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      // "View" page functions.
      BOOL              ChangeFont(HWND hwndOwner);
      void              ViewPageProc(int nPageIndex,HWND hwndPage,WORD wAction,DWORD dwParam);
      
      // Profile management functions.
      void              GetProfileInfo(FILE_FOLDER_STRUCT *pffsData);
      void              SetProfileInfo(FILE_FOLDER_STRUCT *pffsData);
      BOOL              ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL              WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPFOLDER.HPP


