//=====================================================================
// WPNOTEBK.CPP
//    WPNOTEBOOK Class implementation.
// 
// Date Created :  03/10/94
// Last Update  :  12/09/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 03/10/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// From NOTEBOOK.CPP file:
LRESULT  CALLBACK          WPNotebookWindowProcWrapper(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL     CALLBACK _loadds  WPPAGEDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define WPNOTEBOOK_CLASS_NAME   "WPNOTEBOOK"
#define WPPAGE_CLASS_NAME       "WPPAGE"
#define IDC_NOTEBOOK            1023

#define BOOK_MAJOR_HEIGHT       35          // major tab height
#define BOOK_MAJOR_WIDTH        120         // major tab width

#define BOOK_COLOR_BLACK        RGB(0x00,0x00,0x00)
#define BOOK_COLOR_DRK_GRAY     RGB(0x80,0x80,0x80)
#define BOOK_COLOR_GRAY         RGB(0xC0,0xC0,0xC0)
#define BOOK_COLOR_WHITE        RGB(0xFF,0xFF,0xFF)


//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
HBRUSH         sg_hbrGray;

//---------------------------------------------------------------------
// LOCAL GLOBAL VARIABLES
//---------------------------------------------------------------------
static int  sg_iClassUsageCount = 0;


//---------------------------------------------------------------------
// WORKPLACE SHELL NOTEBOOK CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetOwner():
//---------------------------------------------------------------------
WPOBJECT *WPNOTEBOOK::GetOwner(void)
{                   
   return (m_pwpobjOwner);
}


//---------------------------------------------------------------------
// GetIconHandle():
//---------------------------------------------------------------------
HICON WPNOTEBOOK::GetIconHandle(void)
{
   return (m_hIcon);
}


//---------------------------------------------------------------------
// GetIndexFromPageID():
//---------------------------------------------------------------------
int WPNOTEBOOK::GetIndexFromPageID(WORD wPageID)
{
   int   index;       
                    
   // Search page ID array for match...   
   for (index=0; index<m_nPageCount; index++)
      if (m_wPageID[index]==wPageID)
         return (index);   // Page ID match found.

   // No page ID match found, return "error".   
   return (-1);
}

//---------------------------------------------------------------------
// GetWindowFromPageID():
//---------------------------------------------------------------------
HWND WPNOTEBOOK::GetWindowFromPageID(WORD wPageID)
{                  
   return ((HWND)LOWORD(SendMessage(m_hwndNotebookControl,BKM_QUERYPAGEWINDOWHWND,wPageID,0L)));
}  

//---------------------------------------------------------------------
// GetPageIDFromWindow():
//---------------------------------------------------------------------
WORD WPNOTEBOOK::GetPageIDFromWindow(HWND hWnd)
{  
   int index;
   
   // Search page window handle array for match...   
   for (index=0; index<m_nPageCount; index++)
      if (m_hwndPage[index]==hWnd)
         return (m_wPageID[index]);   // Page window handle match found.

   // No page window handle match found, return "error".   
   return (NULL);
}  

//---------------------------------------------------------------------
// GetPageOnTop():
//---------------------------------------------------------------------
HWND WPNOTEBOOK::GetPageOnTop(void)
{
   WORD  wTopPageID;
   HWND  hwndTopPage;
      
   // Query current top page.
   wTopPageID = LOWORD(SendMessage(m_hwndNotebookControl,BKM_QUERYPAGEID,0,MAKELPARAM(BKA_TOP,0)));
   if (wTopPageID==NULL)
      return (NULL);
         
   // Get window handle from notebook page ID. 
   hwndTopPage = GetWindowFromPageID(wTopPageID);
   if (hwndTopPage==NULL)
      return (NULL);

   // Return top page window handle.
   return (hwndTopPage);
}

//---------------------------------------------------------------------
// GetPageProc():
//---------------------------------------------------------------------
FARPAGEPROC WPNOTEBOOK::GetPageProc(HWND hWnd)
{
   WORD  wPageID;
   int   index;
               
   // Get page ID from window handle.               
   wPageID = GetPageIDFromWindow(hWnd);
   // Check that page ID is valid.   
   if (wPageID!=NULL)
   {                              
      // Get page index from page ID.
      index = GetIndexFromPageID(wPageID);
      // Check that page index is valid.
      if (index!=-1)
         return (m_fnPage[index]);  // return pointer to page procedure.
   }            
   
   // No page procedure available... 
   return (NULL);
}


//---------------------------------------------------------------------
// CallPageProc():
//---------------------------------------------------------------------
void WPNOTEBOOK::CallPageProc(HWND hWnd,WORD wAction,DWORD dwParam)
{                                
   int index;
   
   // Check if we have a valid page procedure...
   if ((GetOwner()!=NULL)&&(GetPageProc(hWnd)!=NULL))
   {                                      
      // Get page index from window handle.
      index = GetIndexFromPageID(GetPageIDFromWindow(hWnd));
      // Check that page index is valid...
      if (index!=-1)                       
         // Call page procedure with parameters.
         (GetOwner()->*(GetPageProc(hWnd)))(index,hWnd,wAction,dwParam);
   }
}       


//---------------------------------------------------------------------
// P U B L I C  M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPNOTEBOOK():
//---------------------------------------------------------------------
WPNOTEBOOK::WPNOTEBOOK(WPOBJECT *pwpobjOwner) : WPWINDOW()
{
   WNDCLASS wc;
   
   // Object owner of notebook control.
   m_pwpobjOwner = pwpobjOwner;
   m_nPageCount  = 0;
   // Set window class properties.
   SetScrollProperty(FALSE);
   
   // Check if class has already been initialized.
   if (++sg_iClassUsageCount==1)
   {                                              
      // Create background brush.      
      sg_hbrGray = CreateSolidBrush(BOOK_COLOR_GRAY);
   
      // Fill class structure fields.
      wc.style         = CS_HREDRAW | CS_VREDRAW;
      wc.lpszClassName = WPNOTEBOOK_CLASS_NAME;
      wc.hInstance     = g_wpEnvironment.hInst;
      wc.lpfnWndProc   = WPNotebookWindowProcWrapper;
      wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wc.hIcon         = NULL;
      wc.lpszMenuName  = NULL;
      wc.hbrBackground = CreateSolidBrush(BOOK_COLOR_GRAY);;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = sizeof(LONG);
      // Register notebook class, and check if successful...
      if (RegisterClass(&wc)==0) 
         return;
   }
}

//---------------------------------------------------------------------
// ~WPNOTEBOOK():
//---------------------------------------------------------------------
WPNOTEBOOK::~WPNOTEBOOK(void)
{     
   // Decrement the usage count for the notebook object...
   if (--sg_iClassUsageCount==0)
   {                                                           
      // Destroy the gray solid brush.
      DeleteObject(sg_hbrGray);
      // Unregister the notebook and page class names.
      UnregisterClass(WPNOTEBOOK_CLASS_NAME,g_wpEnvironment.hInst);
   }
}

//---------------------------------------------------------------------
// IsPageWindow():
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::IsPageWindow(HWND hWnd)
{  
   int index;
   
   // Search page window handle array for match...   
   for (index=0; index<m_nPageCount; index++)
      if (m_hwndPage[index]==hWnd)
         return (TRUE);   // page window handle match found, return "true".

   // No page window handle match found, return "false".   
   return (FALSE);
}  

//---------------------------------------------------------------------
// GetWindowFromPageIndex():
//---------------------------------------------------------------------
HWND WPNOTEBOOK::GetWindowFromPageIndex(int nPageIndex)
{       
   // Check if page index is within valid range...   
   if ((nPageIndex>0)&&(nPageIndex<m_nPageCount))
      // Return window handle of requested page.
      return (m_hwndPage[nPageIndex]);
   else                
      // Page index is not valid, therefore return failure.
      return (NULL);
}


//---------------------------------------------------------------------
// GetWindowStyles():
//---------------------------------------------------------------------
DWORD WPNOTEBOOK::GetWindowStyles(void)
{                   
   // Return styles of notebook window.
   return ((WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN));
}


//---------------------------------------------------------------------
// LoadOnwerIcon():
//    This function loads the icon resource from the owner object
// icon path\filename and icon index.  If the load fails, the object
// is assigned the default object icon.
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::LoadOwnerIcon(void)
{
   // Check if we have a valid owner object.
   if (GetOwner()==NULL)
      return (FALSE);

   // Load a new instance of owner's icon resource and return an icon handle.
   m_hIcon = GetOwner()->ExtractIcon();
   
   // Successfully loaded icon resource.
   return (TRUE);
}


//---------------------------------------------------------------------
// GetCaptionTitle():
//---------------------------------------------------------------------
const char *WPNOTEBOOK::GetCaptionTitle(void)
{                      
   // If there is a valid owner object, get object's title w/o special chars.
   if (GetOwner()!=NULL)
      return (GetOwner()->GetTitleNoCRLF());
   else
      return ("Notebook");
}

                         
//---------------------------------------------------------------------
// CreateWindow():
//    This function creates a Windows 3.1 window with the specified
// styles and attributes.  It does not force a window update or show
// command automatically.  It just creates a window of the class
// specified.  The "pwpnbOwner" parameter is passed to the newly
// created window procedure in the create structure as a creation
// parameter (check WM_CREATE message).
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::CreateWindow(char *pszClassName,WPNOTEBOOK *pwpnbOwner)
{  
   HWND hwnd;
   
   // Create custom window.
   hwnd = CreateWindowEx(
               GetWindowExStyles(),
               pszClassName,
               pwpnbOwner->GetCaptionTitle(),
               GetWindowStyles(),
               GetWndPos().x,
               GetWndPos().y,
               GetWndSize().cx,
               GetWndSize().cy,
               NULL,
               (HMENU)NULL,
               g_wpEnvironment.hInst,
               pwpnbOwner);

   // Store window handle.
   SetHandle(hwnd);

   // Check if window creation was successfull.
   if (hwnd!=NULL)
      return (TRUE);
   else
      return (FALSE);
}
                         

//---------------------------------------------------------------------
// Create():
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::Create(int x,int y,int cx,int cy)
{                             
   RECT rcClient;

   // Initialize top-level window's values.
   SetWndPos(x,y);
   SetWndSize(cx,cy);
   
   // Create top-level window for notebook control.                               
   if (CreateWindow(WPNOTEBOOK_CLASS_NAME,this)==FALSE)
      return (FALSE);
      
   // Load owner's icon resource and store internally.
   if (LoadOwnerIcon()==FALSE)
      return (FALSE);
      
   // Create notebook control as a child window of client area.
   GetClientRect(GetHandle(),&rcClient);  // get client area size.
   m_hwndNotebookControl = 
      ::CreateWindow(CCL_NOTEBOOK,       
                     NULL,                 
                     (BKS_SPIRALBIND      |
                      BKS_BACKPAGESBR     | // back pages on bottom right  
                      BKS_MAJORTABRIGHT   | // major tabs on right...     
                      BKS_STATUSTEXTRIGHT | // status text right-justified  
                      WS_CHILD),            // child window                                   
                     0,0,rcClient.right,rcClient.bottom,
                     GetHandle(),
                     (HMENU)IDC_NOTEBOOK,
                     g_wpEnvironment.hInst,
                     NULL);
   
   // Check if notebook chlid control was created successfully...
   if (m_hwndNotebookControl==NULL) 
      return (FALSE);

   // Set major tab size                                               
   SendMessage(m_hwndNotebookControl,BKM_SETDIMENSIONS,BKA_MAJORTAB,
               MAKELONG(BOOK_MAJOR_WIDTH,BOOK_MAJOR_HEIGHT));  
   // Major tab color                                                  
   SendMessage(m_hwndNotebookControl,BKM_SETNOTEBOOKCOLORS,BKA_BACKGROUNDMAJORCOLOR,
               (DWORD)(BOOK_COLOR_GRAY));    
   // Notebook window color                                           
   SendMessage(m_hwndNotebookControl,BKM_SETNOTEBOOKCOLORS,BKA_BACKGROUNDCOLOR,
              (DWORD)(BOOK_COLOR_GRAY));  
   // Notebook page window color                                           
   SendMessage(m_hwndNotebookControl,BKM_SETNOTEBOOKCOLORS,BKA_BACKGROUNDPAGECOLOR,
              (DWORD)(BOOK_COLOR_GRAY));  
   // Status text color                                                
   SendMessage(m_hwndNotebookControl,BKM_SETNOTEBOOKCOLORS,BKA_FOREGROUNDCOLOR,
              (DWORD)(BOOK_COLOR_BLACK));      

   // Successfully created the notebook object.              
   return (TRUE);             
}
 
 
//---------------------------------------------------------------------
// InsertPage():
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::InsertPage(char *pszTabText,WORD wPageStyle,FARPAGEPROC fnPage,char *pszStatusText,LPCSTR pszDialogTemplateName)
{  
   HWND     hwndPage;                      
      
   // If no dialog template resource is passed to function,...
   if (pszDialogTemplateName==NULL)                           
      // Choose default dialog template for a notebook page.
      pszDialogTemplateName = (LPCSTR)MAKEINTRESOURCE(IDD_DEFAULT_NOTEBOOK_PAGE);

   // Create modeless dialog window to be used as the notebook page.
   // We can use WPPAGEDialogProc() directly as the instantiated dialog
   // procedure because we declared it with the "_loadds" keyword which 
   // automatically does the same thing as MakeProcInstance().
   hwndPage = CreateDialogParam(g_wpEnvironment.hResInst,
                                pszDialogTemplateName,
                                m_hwndNotebookControl,
                                (DLGPROC)WPPAGEDialogProc,
                                (LPARAM)this);
                                
   // Check that page dialog window was created successfully...
   if (hwndPage==NULL)
      return (FALSE);
                  
   // Store page window handle in private notebook data array.
   m_hwndPage[m_nPageCount] = hwndPage;

   // Insert a new page into notebook control.
   m_wPageID[m_nPageCount] = LOWORD(SendMessage(m_hwndNotebookControl,BKM_INSERTPAGE,0,MAKELPARAM(wPageStyle,BKA_LAST)));
   // Check that new page was successfully created...                                     
   if (m_wPageID[m_nPageCount]==NULL)
      return (FALSE);                                     
   
   // Store pointer to generic page callback procedure.
   m_fnPage[m_nPageCount] = fnPage;

   // Associate new page with a the newly created window handle.
   if (SendMessage(m_hwndNotebookControl,BKM_SETPAGEWINDOWHWND,m_wPageID[m_nPageCount],MAKELPARAM((WORD)hwndPage,0))==FALSE)
      return (FALSE);
       
   // Set the page's tab text.
   if ((wPageStyle&BKA_MAJOR)||(wPageStyle&BKA_MINOR))
   {
      if (SendMessage(m_hwndNotebookControl,BKM_SETTABTEXT,m_wPageID[m_nPageCount],(LPARAM)(DWORD)((LPSTR)pszTabText))==FALSE)
         return (FALSE);
   }
   
   // Set the page's status text.
   if (wPageStyle&BKA_STATUSTEXTON)
   {
      if (SendMessage(m_hwndNotebookControl,BKM_SETSTATUSLINETEXT,m_wPageID[m_nPageCount],(LPARAM)(DWORD)((LPSTR)pszStatusText))==FALSE)
         return (FALSE);
   }
   
   // Increment the total page count.
   if ((m_nPageCount+1)<MAX_PAGES_PER_NOTEBOOK)
      m_nPageCount++;
   else
      return (FALSE);

   // Create controls for this page.
   CallPageProc(hwndPage,PAGE_CREATE,0L);
   
   // Successfully inserted new page into notebook.   
   return (TRUE);
} 


//---------------------------------------------------------------------
// ShowNotebook():
//---------------------------------------------------------------------
void WPNOTEBOOK::ShowNotebook(void)
{
   // Set starting page to be the top page of notebook.
   SendMessage(m_hwndNotebookControl,BKM_TURNTOPAGE,m_wPageID[0],0L);
   // Finally, show the notebook control and its parent window.
   ShowWindow(m_hwndNotebookControl,SW_SHOWNORMAL);
   ShowWindow(GetHandle(),SW_SHOWNORMAL);
}


//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPNOTEBOOK::Close(void)
{       
   int   index;
   RECT rcWindow,rcShrinkedWindow;
       
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
      
   // Destroy all created page windows.
   for (index=0; index<m_nPageCount; index++)                                        
      ::DestroyWindow(GetWindowFromPageID(m_wPageID[index]));
      
   // Delete all pages in notebook control.
   SendMessage(m_hwndNotebookControl,BKM_DELETEPAGE,0,BKA_ALL);
                               
   // Destroy top-level window created to hold notebook control.                               
   ::DestroyWindow(GetHandle());
   
   return (TRUE);
}
  
//---------------------------------------------------------------------
// WINDOW MESSAGES MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OnCreate():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnCreate(void)
{     
   RECT     rcIcon,rcWindow;
   WPFOLDER *pwpfolderParent,*pwpfolderOwner;
         
   // Check if we have a valid owner object.
   if (GetOwner()==NULL)
      return;         
   
   // For convenience, cast pointer to container object and assign it to local variable.         
   pwpfolderParent = (WPFOLDER *)(GetOwner()->GetParent());
            
   // Check if parent container object of notebook owner object is open AND is not minimized...
   if ((pwpfolderParent!=NULL) &&
       (pwpfolderParent->isOpen()==TRUE) && 
       (IsIconic(pwpfolderParent->GetHandle())==FALSE))
   {
      // Depending on the folder's view mode...
      switch (pwpfolderParent->GetOpenView())
      {  
         case VIEW_ICON:
            // Get position and size of object's icon rectangle.
            SetRect(&rcIcon,GetOwner()->GetIconPos().x,GetOwner()->GetIconPos().y,
                    GetOwner()->GetIconPos().x+GetSystemMetrics(SM_CXICON)-1,
                    GetOwner()->GetIconPos().y+GetSystemMetrics(SM_CYICON)-1);
            break;
      
         case VIEW_DETAILS:
            // First, get object's details view table entry position.
            GetOwner()->GetFrameRect(&rcIcon);
            // Calculate position of object's icon rectangle within details view.
            GetOwner()->SetRectCXCY(&rcIcon,
                            (rcIcon.left+DV_ICON_INDENT_CX),
                            (rcIcon.top+DV_ICON_INDENT_CY),
                            DV_ICON_SIZE_CX,DV_ICON_SIZE_CY);
            break;
      }
      
      // Convert from logical (folder) coordinates to device (screen) coordinates.
      pwpfolderParent->LPtoDP((LPPOINT)&rcIcon,2);
   }
   else
   {  
      // Check if owner object is a container object...
      if ((GetOwner()->GetType()&OBJTYPE_CONTAINER)==OBJTYPE_CONTAINER)
      {   
         // For convenience, cast pointer to object to pointer to folder.
          pwpfolderOwner = (WPFOLDER *)GetOwner();
          // Check if owner folder object is open and not iconic.
          if ((pwpfolderOwner->isOpen()==TRUE) && 
              (IsIconic(pwpfolderOwner->GetHandle())==FALSE))
          {
             // Start animation from center of owner container object's folder window.
             GetWindowRect(pwpfolderOwner->GetHandle(),&rcIcon);
             rcIcon.left  += ((rcIcon.right-rcIcon.left)-GetSystemMetrics(SM_CXICON))/2;
             rcIcon.top   += ((rcIcon.bottom-rcIcon.top)-GetSystemMetrics(SM_CYICON))/2;
             rcIcon.right  = rcIcon.left + GetSystemMetrics(SM_CXICON);
             rcIcon.bottom = rcIcon.top + GetSystemMetrics(SM_CYICON);
          }    
          else
          {
             // Start window animation from center of screen.
             rcIcon.left   = (GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON))/2;
             rcIcon.top    = (GetSystemMetrics(SM_CYSCREEN)-GetSystemMetrics(SM_CYICON))/2;
             rcIcon.right  = rcIcon.left + GetSystemMetrics(SM_CXICON);
             rcIcon.bottom = rcIcon.top + GetSystemMetrics(SM_CYICON);
          }
      }
      else
      {
         // Start window animation from center of screen.
         rcIcon.left   = (GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON))/2;
         rcIcon.top    = (GetSystemMetrics(SM_CYSCREEN)-GetSystemMetrics(SM_CYICON))/2;
         rcIcon.right  = rcIcon.left + GetSystemMetrics(SM_CXICON);
         rcIcon.bottom = rcIcon.top + GetSystemMetrics(SM_CYICON);
      }
      
   }

   // Get window position and size rectangle.
   GetWindowRect(GetHandle(),&rcWindow);
   
   // "Explode" window frame.
   AnimateWindowFrame(&rcIcon,&rcWindow,0,8);
}
                                       
//---------------------------------------------------------------------
// OnActivate():
//---------------------------------------------------------------------
void WPNOTEBOOK::OnActivate(BOOL fActive)
{            
   WORD  wPageID;   
   HWND  hwndPage;

   // If notebook is gaining focus, disable global accelerator key translation.
   g_wpEnvironment.fAcceleratorTranslation = fActive;
   // If page is becoming inactive...
   if (fActive==FALSE)         
      // Disable current modeless dialog handle.
      g_wpEnvironment.hwndCurrentModelessDialog = NULL;
   else
   {                                            
      // Query current top page.
      wPageID = LOWORD(SendMessage(m_hwndNotebookControl,BKM_QUERYPAGEID,0,MAKELPARAM(BKA_TOP,0)));
      if (wPageID==NULL)
         return; 
      // Get window handle from notebook page ID. 
      hwndPage = GetWindowFromPageID(wPageID);
      if (hwndPage==NULL)
         return;    
      // Else, set current modeless dialog to be this page.
      g_wpEnvironment.hwndCurrentModelessDialog = hwndPage;
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
void WPNOTEBOOK::OnCommand(HWND hwndNotebookControl,WORD wNotifyCode)
{                                    
   WORD  wPageID;
   HWND  hwndPage,hwndControl;
      
   // Query current top page.
   wPageID = LOWORD(SendMessage(hwndNotebookControl,BKM_QUERYPAGEID,0,MAKELPARAM(BKA_TOP,0)));
   if (wPageID==NULL)
      return;         
         
   // Get window handle from notebook page ID. 
   hwndPage = GetWindowFromPageID(wPageID);
   if (hwndPage==NULL)
      return;
                                          
   // Process notebook notification code.
   switch (wNotifyCode)
   {
      case BKN_HELP:
         CallPageProc(hwndPage,PAGE_HELP,0L);
         break;     
         
      case BKN_NEWPAGESIZE:
         CallPageProc(hwndPage,PAGE_NEWSIZE,0L);
         break;
         
      case BKN_PAGESELECTED:  
         // Store handle of notebook page selected (used to translate dialog messages).
         g_wpEnvironment.hwndCurrentModelessDialog = hwndPage;
         // Select the first enabled and visible child dialog control 
         // as the current control with focus.
         hwndControl = GetWindow(hwndPage,GW_CHILD);
         while ((IsWindowEnabled(hwndControl)==FALSE)&&
                (IsWindowVisible(hwndControl)==FALSE)&&
                (GetWindow(hwndControl,GW_HWNDNEXT)!=NULL))
            hwndControl = GetWindow(hwndControl,GW_HWNDNEXT);
         SetFocus(hwndControl);
         // Call resgistered page procedure.
         CallPageProc(hwndPage,PAGE_SELECTED,0L);
         break;
         
      case BKN_PAGEDELETED:
         // Query current top page.
         wPageID = LOWORD(SendMessage(hwndNotebookControl,BKM_QUERYPAGEID,0,MAKELPARAM(BKA_DELETED,0)));
         if (wPageID!=NULL)
         {     
            // Get window handle from notebook page ID.
            hwndPage = GetWindowFromPageID(wPageID);
            if (hwndPage!=NULL)
               CallPageProc(GetWindowFromPageID(wPageID),PAGE_DELETED,0L);
         }
         break;      
   }
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
   return (MAKELRESULT(m_hIcon,0));
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
      DrawIcon(hdc,0,0,m_hIcon);
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
   NCPaint(fActive,m_hIcon);
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
   NCPaint(fActive,m_hIcon);
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
         
// EOF WPNOTEBOOK.CPP
                   