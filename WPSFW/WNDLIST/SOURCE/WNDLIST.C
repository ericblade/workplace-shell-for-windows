//=====================================================================
// WNDLIST.C
//    Window List program (Windows Task Manager replacement program).
//
// Date Created :  05/06/94
// Last Update  :  12/06/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 05/06/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 header files.
#include <windows.h>
// C-runtime library header files.
#include <stdio.h>
#include <string.h>
// Window List resource ID's header file.
#include "resource.h"
#include "..\..\wpshell\include\wphelpid.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
// Undocumented Windows API (from USER module):
void FAR PASCAL    SwitchToThisWindow(HWND,BOOL);   // USER.172


//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------
WORD GetLangBaseID(void);
void GetPositionAndSize(RECT *prcWindowList);
BOOL CALLBACK WindowListDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define MAX_TILE_WINDOWS_PER_COLUMN    4
#define HELP_FILENAME                  "WPSHELL.HLP"


//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
HINSTANCE g_hInstance;
WORD      g_wLangBaseID;
int       g_nTopWindows=0;


//---------------------------------------------------------------------
// FUNCTION DEFINITIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WinMain():
//---------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int nCmdShow)
{
   FARPROC  fpDlgProc;
   RECT     rcWindowList;

   // If we are currently running already, exit immediatedly.
   if (hPrevInstance!=0)
      return (FALSE);

   // Get base ID for NLS resources depending on current lanaguage.
   g_wLangBaseID = GetLangBaseID();

   // Get position and size of Window List dialog.
   GetPositionAndSize(&rcWindowList);

   // Store Window List instance globally.
   g_hInstance = hInstance;

   // Display Window List dialog now.
   fpDlgProc = MakeProcInstance((FARPROC)WindowListDlgProc,hInstance);
   DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_WINDOWLIST),0,fpDlgProc,MAKELONG((WORD)&rcWindowList,0));
   FreeProcInstance(fpDlgProc);

   // Exit program.
   return (FALSE);
}

//---------------------------------------------------------------------
// GetLangBaseID():
//---------------------------------------------------------------------
WORD GetLangBaseID(void)
{
   char  szValue[16];
   WORD  wLangBase;

   // Query current language being used by Windows.
   GetProfileString("Intl","sLanguage","enu",szValue,sizeof(szValue));

   // Depending on which language is currently selected in Windows choose
   // which resource library will be loaded.  Only a handful of languages
   // will be supported, therefore if the language is not known, it will
   // default to US-English.
   // Following are some of the language codes used by Windows:
   //    dan      Danish                        DANI
   //    deu      German                        GRMN
   //    eng      International English         ENUS
   //    enu      U.S. English                  ENUS
   //    esn      Modern Spanish                SPAN
   //    esp      Castilian Spanish             SPAN
   //    fin      Finnish                       FINN
   //    fra      French                        FREN
   //    frc      French Canadian               FREN
   //    isl      Icelandic
   //    ita      Italian                       ITAL
   //    nld      Dutch                         DUTC
   //    nor      Norwegian
   //    ptb      Brazilian Portuguese          PORT
   //    ptg      Portuguese                    PORT
   //    sve      Swedish                       SWED

   // By default, we use the English-US resource DLL.
   wLangBase = IDSTR_BASE_ENUS;

   // If language is Dutch, then pick dutch resources...
   if (stricmp(szValue,"nld")==0)
      wLangBase = IDSTR_BASE_DUTC;
   else
   // If language is any kind of Portuguese, then pick portuguese resources...
   if ((stricmp(szValue,"ptg")==0)||(stricmp(szValue,"ptb")==0))
      wLangBase = IDSTR_BASE_PORT;
   else
   // If language is German, then pick german resources...
   if (stricmp(szValue,"deu")==0)
      wLangBase = IDSTR_BASE_GRMN;
   else
   // If language is any kind of French, then pick french resources...
   if ((stricmp(szValue,"fra")==0)||(stricmp(szValue,"frc")==0))
      wLangBase = IDSTR_BASE_FREN;
   else
   // If language is Swedish, then pick swedish resources...
   if (stricmp(szValue,"sve")==0)
      wLangBase = IDSTR_BASE_SWED;
   else
   // If language is Finnish, then pick finnish resources...
   if (stricmp(szValue,"fin")==0)
      wLangBase = IDSTR_BASE_FINN;
   else
   // If language is any kind of Spanish, then pick spanish resources...
   if ((stricmp(szValue,"esn")==0)||(stricmp(szValue,"esp")==0))
      wLangBase = IDSTR_BASE_SPAN;
   else
   // If language is Italian, then pick italian resources...
   if (stricmp(szValue,"ita")==0)
      wLangBase = IDSTR_BASE_ITAL;

#if 0
   else
   // If language is Danish, then pick danish resources...
   if (stricmp(szValue,"dan")==0)
      wLangBase = IDSTR_BASE_DANI;
#endif

   // Return current language base ID for NLS resources.
   return (wLangBase);
}


//---------------------------------------------------------------------
// GetPositionAndSize():
//---------------------------------------------------------------------
void GetPositionAndSize(RECT *prcWindowList)
{
   char  szBuffer[80];

   // Get top-left position of "Window List" from system profile.
   GetPrivateProfileString("Workplace_Shell_for_Windows","WindowList","",szBuffer,sizeof(szBuffer),"SYSTEM.INI");

   // By default, center window list.
   prcWindowList->left   = (GetSystemMetrics(SM_CXSCREEN)/3);
   prcWindowList->top    = (GetSystemMetrics(SM_CYSCREEN)/3);
   prcWindowList->right  = prcWindowList->left + (GetSystemMetrics(SM_CXSCREEN)/3);
   prcWindowList->bottom = prcWindowList->top + (GetSystemMetrics(SM_CYSCREEN)/3);

   // If there was a stored string in profile containing previous position and size...
   if (szBuffer[0]!='\0')
   {
      sscanf(szBuffer,"%d %d %d %d",
             &(prcWindowList->left),&(prcWindowList->top),
             &(prcWindowList->right),&(prcWindowList->bottom));
   }
}


//---------------------------------------------------------------------
// InitializeTopWindowList():
//---------------------------------------------------------------------
void InitializeTopWindowList(HWND hDlg)
{
   HWND  hwndListBox,hwndTopWindow;
   char  szTitle[80];

   hwndListBox = GetDlgItem(hDlg,IDC_LISTBOX);
   hwndTopWindow = GetWindow(hDlg,GW_HWNDFIRST);
   while (hwndTopWindow)
   {
      if ( (hwndTopWindow!=hDlg) &&
           IsWindowVisible(hwndTopWindow) &&
           (GetWindow(hwndTopWindow,GW_OWNER)==NULL) )
      {
         // Get window title of minimized window.
         GetWindowText(hwndTopWindow,szTitle,79);
         // If title is not empty...
         if (strlen(szTitle)>0)
         {
            // Insert into window list.
            SendMessage(
               hwndListBox,
               LB_SETITEMDATA,
               (WPARAM)SendMessage(hwndListBox,LB_ADDSTRING,0,(LPARAM)(LPSTR)szTitle),
               hwndTopWindow);
            // Increment count.
            g_nTopWindows++;
         }
      }
      hwndTopWindow = GetWindow(hwndTopWindow,GW_HWNDNEXT);
   }
   // Select first item in listbox.
   SendMessage(hwndListBox,LB_SETSEL,TRUE,0);
}


//---------------------------------------------------------------------
// GetItemHitIndex():
//---------------------------------------------------------------------
int GetItemHitIndex(HWND hDlg,int x,int y)
{
   int      index;
   POINT    ptClick;
   RECT     rcItem;

   // Initialize point structure where user clicked.
   ptClick.x = x-2;
   ptClick.y = y-2;

   // Loop through all listbox items...
   for (index=0; index<g_nTopWindows; index++)
   {
      // Get listbox item's bounding rectangle.
      SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETITEMRECT,(WPARAM)index,(LPARAM)(LPRECT)&rcItem);
      // Test clicked point with item's rectangle.
      if (PtInRect((LPRECT)&rcItem,ptClick)==TRUE)
         return (index);
   }
   // Mouse click coordinates did not "hit" any listbox item.
   return (-1);
}


//---------------------------------------------------------------------
// OpenPopupMenu():
//---------------------------------------------------------------------
void OpenPopupMenu(HWND hDlg,int x,int y)
{
   HMENU    hMenu,hPopupMenu;
   POINT    ptScreen;

   // Convert client coordinates to screen coordinates.
   ptScreen.x = x;
   ptScreen.y = y;
   ClientToScreen(hDlg,&ptScreen);

   // Load system menu for object hit.
   hMenu = LoadMenu(g_hInstance,MAKEINTRESOURCE((IDM_WINDOWLIST+GetLangBaseID())));

   // If could not menu for current language, then...
   if (hMenu==NULL)
      // Load default (english) system menu for object hit.
      hMenu = LoadMenu(g_hInstance,MAKEINTRESOURCE(IDM_WINDOWLIST_ENUS));

   // Extract popup menu from system menu (needed step to get popup only!).
   hPopupMenu = GetSubMenu(hMenu,0);

   // Display popup menu and track user input.
   TrackPopupMenu(hPopupMenu,(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON),
                  ptScreen.x,ptScreen.y,0,hDlg,NULL);

   // Destroy menu resource.
   DestroyMenu(hMenu);
}

//---------------------------------------------------------------------
// OnShow():
//---------------------------------------------------------------------
void OnShow(HWND hwnd)
{
   HWND hwndPopup;

   if (IsWindow(hwndPopup = GetLastActivePopup(hwnd)))
   {
      if (!(GetWindowLong(hwndPopup,GWL_STYLE)&WS_DISABLED))
      {
         // Switch to the specified window.
         SwitchToThisWindow(hwndPopup,TRUE);
         return;
      }
   }

   // Error, cannot switch to this window.
   MessageBeep(0);
}


//---------------------------------------------------------------------
// OnMinimize():
//---------------------------------------------------------------------
void OnMinimize(HWND hwnd)
{
   // Is it already minimized?...
   if (IsIconic(hwnd)==FALSE)
   {
      // Does it have a minimize button?...
      if ( (GetWindowLong(hwnd,GWL_STYLE)&WS_MINIMIZEBOX) &&
           (!(GetWindowLong(hwnd,GWL_STYLE)&WS_DISABLED)) )
         PostMessage(hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0);
   }
}


//---------------------------------------------------------------------
// OnClose():
//---------------------------------------------------------------------
void OnClose(HWND hwnd)
{
   if ( IsWindow(hwnd) &&
        (! (GetWindowLong(hwnd,GWL_STYLE)&WS_DISABLED)) )
   {
      PostMessage(hwnd,WM_CLOSE,0,0);
   }
}


//---------------------------------------------------------------------
// IsIndexInList():
//---------------------------------------------------------------------
BOOL IsIndexInList(int index,int *aiList,int total)
{
   int i;

   for (i=0; i<total; i++)
   {
      if (index==aiList[i])
         return (TRUE);
   }

   return (FALSE);
}


//---------------------------------------------------------------------
// SetCascadeWindowPos():
//---------------------------------------------------------------------
void SetCascadeWindowPos(HWND hwndCascade,int nIndex,int nTotal)
{
   SIZE  sizeCascade;
   POINT ptCascade;

   // Calculate width and height of cascade window.
   sizeCascade.cx = (GetSystemMetrics(SM_CXSCREEN)/4)*3;
   sizeCascade.cy = (GetSystemMetrics(SM_CYSCREEN)/10)*7;

   // Calculate top-left point of cascade window.
   ptCascade.x = ((nIndex * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXSIZE))) % (GetSystemMetrics(SM_CXSCREEN)-sizeCascade.cx));
   ptCascade.y = ((nIndex * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYSIZE))) % (GetSystemMetrics(SM_CYSCREEN)-sizeCascade.cy));

   // Restore window to normal position.
   OnShow(hwndCascade);

   // Check if window is still iconic...
   if (IsIconic(hwndCascade)==TRUE)
      // return immediatedly.
      return;

   // Check if cascade window has a resize frame...
   if ((GetWindowLong(hwndCascade,GWL_STYLE) & WS_THICKFRAME)==WS_THICKFRAME)
      // Now, we are ready to change the position and size of cascade window.
      SetWindowPos(hwndCascade,NULL,ptCascade.x,ptCascade.y,sizeCascade.cx,sizeCascade.cy,SWP_NOZORDER);
   else
      // Now, we are ready to change the position of cascade window.
      SetWindowPos(hwndCascade,NULL,ptCascade.x,ptCascade.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}


//---------------------------------------------------------------------
// SetTileWindowPos():
//---------------------------------------------------------------------
void SetTileWindowPos(HWND hwndTile,int nIndex,int nTotal)
{
   SIZE  sizeTile;
   POINT ptTile;
   int   nRow,nCol;

   // Calculate the number of rows and columns necessary.
   nRow = ((nTotal < MAX_TILE_WINDOWS_PER_COLUMN)? nTotal : MAX_TILE_WINDOWS_PER_COLUMN);
   nCol = ((nTotal-1) / nRow)+1;

   // Calculate width and height of tile window.
   sizeTile.cx = (GetSystemMetrics(SM_CXSCREEN)/nCol);
   sizeTile.cy = (GetSystemMetrics(SM_CYSCREEN)/nRow);

   // Calculate top-left point of tile window.
   ptTile.x = ((nIndex / nRow) * sizeTile.cx);
   ptTile.y = ((nIndex % nRow) * sizeTile.cy);

   // Restore window to normal position.
   OnShow(hwndTile);

   // Check if window is still iconic...
   if (IsIconic(hwndTile)==TRUE)
      // return immediatedly.
      return;

   // Check if cascade window has a resize frame...
   if ((GetWindowLong(hwndTile,GWL_STYLE) & WS_THICKFRAME)==WS_THICKFRAME)
      // Now, we are ready to change the position and size of cascade window.
      SetWindowPos(hwndTile,NULL,ptTile.x,ptTile.y,sizeTile.cx,sizeTile.cy,SWP_NOZORDER);
   else
      // Now, we are ready to change the position of cascade window.
      SetWindowPos(hwndTile,NULL,ptTile.x,ptTile.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}



//---------------------------------------------------------------------
// OnCommand():
//---------------------------------------------------------------------
void OnCommand(HWND hDlg,WORD wMenuItem)
{
   int   index,CaretIndex,nMulSelCount,MulSelIndex[64];
   HWND  hwndTopWindow;

   // Hide taskman.
   ShowWindow(hDlg,SW_HIDE);

   // Get caret index.
   CaretIndex = (int)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETCARETINDEX,0,0);

   // Get multiple selected count and index array.
   nMulSelCount = (int)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETSELITEMS,(WPARAM)64,(LPARAM)(int FAR *)&(MulSelIndex[0]));

   if (IsIndexInList(CaretIndex,&(MulSelIndex[0]),nMulSelCount)==TRUE)
   {
      // Loop through all selected windows and restore them.
      for (index=0; index<nMulSelCount; index++)
      {
         // Get window handle of task selected.
         hwndTopWindow = (HWND)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETITEMDATA,(WPARAM)MulSelIndex[index],0);

         switch (wMenuItem)
         {
            case IDM_SHOW:
               OnShow(hwndTopWindow);
               break;
            case IDM_MINIMIZE:
               OnMinimize(hwndTopWindow);
               break;
            case IDM_CLOSE:
               OnClose(hwndTopWindow);
               break;
            case IDM_CASCADE:
               SetCascadeWindowPos(hwndTopWindow,index,nMulSelCount);
               break;
            case IDM_TILE:
               SetTileWindowPos(hwndTopWindow,index,nMulSelCount);
               break;
         }
      }
   }
   else
   {
      // Get window handle of task selected.
      hwndTopWindow = (HWND)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETITEMDATA,(WPARAM)CaretIndex,0);

      switch (wMenuItem)
      {
         case IDM_SHOW:
            OnShow(hwndTopWindow);
            break;
         case IDM_MINIMIZE:
            OnMinimize(hwndTopWindow);
            break;
         case IDM_CLOSE:
            OnClose(hwndTopWindow);
            break;
         case IDM_CASCADE:
            SetCascadeWindowPos(hwndTopWindow,0,1);
            break;
         case IDM_TILE:
            SetTileWindowPos(hwndTopWindow,0,1);
            break;
      }
   }
}


//---------------------------------------------------------------------
// OnListboxNotify():
//---------------------------------------------------------------------
void OnListboxNotify(HWND hDlg,WORD wNotifyCode)
{
   HWND  hwndTopWindow;
   int   CaretIndex;

   if (wNotifyCode==LBN_DBLCLK)
   {
      // Hide taskman.
      ShowWindow(hDlg,SW_HIDE);
      // Get caret index.
      CaretIndex = (int)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETCARETINDEX,0,0);
      // Get window handle of task selected.
      hwndTopWindow = (HWND)SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_GETITEMDATA,(WPARAM)CaretIndex,0);
      // Restore window.
      OnShow(hwndTopWindow);
      // Close Window List.
      PostMessage(hDlg,WM_CLOSE,0,0);
   }
}

//---------------------------------------------------------------------
// SaveWindowListPositionandSize():
//---------------------------------------------------------------------
void SaveWindowListPositionAndSize(HWND hDlg)
{
   RECT  rcWindowList;
   char  szBuffer[80];

   // Get current Window List position and size.
   GetWindowRect(hDlg,&rcWindowList);
   // Create profile string from Window List rectangle.
   sprintf(szBuffer,"%d %d %d %d",
           rcWindowList.left,rcWindowList.top,rcWindowList.right,rcWindowList.bottom);
   // Write out profile string to system profile.
   WritePrivateProfileString("Workplace_Shell_for_Windows","WindowList",szBuffer,"SYSTEM.INI");
}


//---------------------------------------------------------------------
// GetWindowListCaptionString():
//---------------------------------------------------------------------
char *GetWindowListCaptionString(void)
{
   // Depending on current language...
   switch (g_wLangBaseID)
   {
      case IDSTR_BASE_DUTC:
         return ("Venster lijst");

      case IDSTR_BASE_PORT:
         return ("Lista de janelas");

      case IDSTR_BASE_GRMN:
         return ("Fensterliste");

      case IDSTR_BASE_SWED:
          return ("Aktiva Sessioner");

      case IDSTR_BASE_FREN:
         return ("Liste de fenêtres");

      case IDSTR_BASE_FINN:
          return ("Ikkunaluettelo");

      case IDSTR_BASE_SPAN:
          return ("Configuración del Sistema");

      case IDSTR_BASE_ITAL:
          return ("Impostazioni di Sistema");

      case IDSTR_BASE_ENUS:
      default:
         // Return English-US string.
         return ("Window List");
   }
}


//---------------------------------------------------------------------
// WindowListDlgProc():
//---------------------------------------------------------------------
BOOL CALLBACK WindowListDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   int   index;
   int   nWndWidth,nWndHeight;
   RECT  *prcWindowList;

   switch (uMsg)
   {
      case WM_INITDIALOG:
         // Set "Window List" caption title string depending on language.
         SetWindowText(hDlg,GetWindowListCaptionString());
         // Cast pointer to Window List rectangle.
         prcWindowList = (RECT *)lParam;
         // Position window on screen.
         SetWindowPos(hDlg,NULL,prcWindowList->left,prcWindowList->top,
                                (prcWindowList->right-prcWindowList->left+1),
                                (prcWindowList->bottom-prcWindowList->top+1),
                                (SWP_NOZORDER));
         // Position all controls to fit window.
         // Initialize dialog listbox.
         InitializeTopWindowList(hDlg);
         return (TRUE);

      case WM_ACTIVATEAPP:
         // This message is sent when a window is about to be activated or deactivated.
         if (wParam==FALSE)
         {
            // Hide dialog box.
            ShowWindow(hDlg,SW_HIDE);
            // Post message to close.
            PostMessage(hDlg,WM_CLOSE,0,0);
         }
         break;

      case WM_SIZE:
         // Get new size of Window List dialog box.
         nWndWidth  = LOWORD( lParam );
         nWndHeight = HIWORD( lParam );
         // Set position for the window titles list box
         SetWindowPos(GetDlgItem(hDlg,IDC_LISTBOX),
                       NULL,
                       2, 2,
                       nWndWidth - 4,
                       nWndHeight - 4,
                       SWP_NOZORDER
                     );
         // Repaint dialog box.
         InvalidateRect(hDlg,NULL,TRUE);
         UpdateWindow(hDlg);
         return( TRUE );


      case WM_PARENTNOTIFY:
         // This message is received when a control in the "Window List" is created/destroyed
         // or has a mouse click.
         // The "Window List" is only interested in handling right mouse clicks.
         if (wParam==WM_RBUTTONDOWN)   // we assume that it is the listbox control.
         {
            // Get item index where user has clicked right mouse button.
            index = GetItemHitIndex(hDlg,(int)LOWORD(lParam),(int)HIWORD(lParam));
            // If valid item was "hit"...
            if (index>=0)
            {
               // Frame item hit.
               if (LB_ERR!=SendDlgItemMessage(hDlg,IDC_LISTBOX,LB_SETCARETINDEX,(WPARAM)index,0))
                  // Bring popup-up.
                  OpenPopupMenu(hDlg,(int)LOWORD(lParam),(int)HIWORD(lParam));
            }
         }
         break;

      case WM_COMMAND:
         // If user pressed ESC...
         if (wParam==2)
         {
            // Hide dialog box.
            ShowWindow(hDlg,SW_HIDE);
            // Post message to close.
            PostMessage(hDlg,WM_CLOSE,0,0);
            break;
         }

         // If user press ENTER...
         if (wParam==1)
         {
            // Hide dialog box.
            ShowWindow(hDlg,SW_HIDE);
            // Restore selected application(s).
            OnCommand(hDlg,IDM_SHOW);
            // Post message to close.
            PostMessage(hDlg,WM_CLOSE,0,0);
         }

         // Call function to act on command...
         switch (wParam)
         {
            case IDM_HELPINDEX:
               // Open help file with "Contents" of help file.
               WinHelp(GetDesktopWindow(),HELP_FILENAME,HELP_CONTENTS,0L);
               break;

            case IDM_HELPGENERAL:
               // Open help file with "General help" for window list.
               WinHelp(GetDesktopWindow(),HELP_FILENAME,HELP_CONTEXT,HELPID_GENERALHELP_WINDOWLIST);
               break;

            case IDM_HELPUSING:
               // Open help file with "Using help" for help manager.
               WinHelp(GetDesktopWindow(),"WINHELP.HLP",HELP_CONTENTS,0L);
               break;

            case IDM_HELPKEYS:
               // Open help file with "Keys help" information.
               WinHelp(GetDesktopWindow(),HELP_FILENAME,HELP_CONTEXT,HELPID_KEYSHELP);
               break;

            case IDM_SHOW:
            case IDM_MINIMIZE:
            case IDM_TILE:
            case IDM_CASCADE:
            case IDM_CLOSE:
               // Hide dialog box.
               ShowWindow(hDlg,SW_HIDE);
               // Act on user command...
               OnCommand(hDlg,(WORD)wParam);
               // Post message to close.
               PostMessage(hDlg,WM_CLOSE,0,0);
               break;

            case IDC_LISTBOX:
               // Listbox notification message.
               OnListboxNotify(hDlg,HIWORD(lParam));
               break;
         }
         break;

      case WM_CLOSE:
         // Save current Window List position and size.
         SaveWindowListPositionAndSize(hDlg);
         // Close dialog.
         EndDialog(hDlg,0);
         break;
   }

   // Return false by default.
   return (FALSE);
}


// EOF WNDLIST.C


