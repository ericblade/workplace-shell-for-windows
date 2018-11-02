//=====================================================================
// PMTOWP.CPP
//    Program Manager to WorkPlace Shell convertion utility.
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
// Workplace Shell header files.
#include "progman.hpp"
#include "resource.h"
#include "pmtowp.h"
// C-runtime library header files.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys\stat.h>
// Windows 3D control library header file (CTL3D.DLL).
#include "ctl3d.h"


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define SZ_APPNAME          "PMTOWP"
#define PROFILE_FILENAME    "\\WPSHELL.INI"
#define WM_USER_INITIATE    (WM_USER+10)

//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
HINSTANCE   g_hResInst = NULL;
BOOL        fSilentMode = FALSE, fUseWarpColors = FALSE;
char        **g_apszStringTable;


//---------------------------------------------------------------------
// PROGRAM MANAGER TO WORKPLACE SHELL FUNCTIONS
//---------------------------------------------------------------------
BOOL LoadNLSResourceLibrary(void);

BOOL LoadStringTable(HINSTANCE hInstance,int nTotalStrings);
char *GetString(UINT uIDString);
void FreeStringTable(int nTotalStrings);

void ReportError(char *pszErrorMessage);
HWND DisplayLogo(HINSTANCE hInstance);

int  ReadDesktopObjectCount(int hProfile);
void WriteDesktopObjectCount(int hProfile,int nCount);

BOOL WriteProfileHeader(int hProfile);
BOOL CreateDESKTOP(int hProfile);
BOOL CreatePROGMANFolder(int hfile,char *pszTitle,int nCount);
BOOL CreateGROUPFolder(int hfile,GROUPINFO *pGroup,int x,int y);

BOOL CreateDrivesFolder(int hfile);
BOOL CreateCommandPromptsFolder(int hfile);
BOOL CreateTemplatesFolder(int hfile);
BOOL CreateSystemSetupFolder(int hfile);
BOOL CreateSystemFolder(int hfile);
BOOL CreateMinViewer(int hfile);
BOOL ProcessOtherFolders(HWND hwnd,int hfile,int *nCount);
BOOL CALLBACK MainDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// WinMain():
//---------------------------------------------------------------------
int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpszCmdParam,int nCmdShow)
{
   PROGMANDDE  Progman;
   FARPROC     lpfnMainDlgProc;

   // an instance of PMTOWP is already running, exit.
   if (hPrevInstance)
      return (FALSE);

   // check if PMTOWP is in silent mode.
   if ((strstr(lpszCmdParam,"/S")!=NULL)||
       (strstr(lpszCmdParam,"/s")!=NULL))
      fSilentMode = TRUE;
   else
      fSilentMode = FALSE;

   // Register this application's dialog boxes to use the 3D control library.
   Ctl3dRegister(hInstance);
   Ctl3dAutoSubclass(hInstance);

   // Load NLS resource library which contains language-dependent resources.
   if (LoadNLSResourceLibrary()==FALSE)
   {
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      // Error loading NLS resource library, therefore exit.
      return (FALSE);
   }

   // Load string table corresponding to current language.
   if (LoadStringTable(g_hResInst,TOTAL_STRINGS)==FALSE)
   {
      // Check if program is not being run in silent mode...
      if (fSilentMode==FALSE)
         MessageBox(NULL,"Error -2: Could not load string table.","PM to WP",(MB_OK|MB_ICONSTOP));
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      return (FALSE);
   }

   // check if PMTOWP is using OS/2 Warp colors.
   if ((strstr(lpszCmdParam,"/W")!=NULL)||
       (strstr(lpszCmdParam,"/w")!=NULL))
      fUseWarpColors = TRUE;
   else
      fUseWarpColors = FALSE;

   // initialize DDEML and connect to PROGMAN.
   if (Progman.Initialize(hInstance)==FALSE)
   {
      // could not initialize DDEML library.
      ReportError(GetString(IDSTR_ERR_01));
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      // return failure.
      return (FALSE);
   }

   // connet with "PROGMAN" DDE service server (usually PROGMAN.EXE).
   if (Progman.Connect()==FALSE)
   {
      // could not connect to "PROGMAN" server (maybe not available).
      ReportError(GetString(IDSTR_ERR_02));
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      // return failure.
      return (FALSE);
   }

   // read all group information from 'PROGMAN' DDE server.
   if (Progman.ReadAllGroups()==FALSE)
   {
      // could not read group information from 'PROGMAN' DDE server.
      ReportError(GetString(IDSTR_ERR_03));
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      // return failure.
      return (FALSE);
   }

   // create procedure instance callback function.
   lpfnMainDlgProc = MakeProcInstance((FARPROC)MainDlgProc,hInstance);
   // create and call main dialog box.
   DialogBoxParam(g_hResInst,MAKEINTRESOURCE(IDD_MAIN),NULL,lpfnMainDlgProc,(LPARAM)&Progman);
   // free procedure instance callback function.
   FreeProcInstance(lpfnMainDlgProc);

   // disconnect and uninitialize DDE conversation with PROGMAN.
   Progman.Disconnect();
   Progman.Uninitialize();

   // Free string table memory.
   FreeStringTable(TOTAL_STRINGS);

   // Clean-up libraries before exiting.
   FreeLibrary(g_hResInst); // NLS resource library.

   // Unregister this application from 3D control library (CTL3D.DLL).
   Ctl3dUnregister(hInstance);

   // return from program (we are done).
   return (FALSE);
}

//---------------------------------------------------------------------
// LoadNLSResourceLibrary():
//---------------------------------------------------------------------
BOOL LoadNLSResourceLibrary(void)
{
   char *pszLanguage,szValue[16],szNLSLibrary[16],szErrorMsg[128];

   // Get current language selected by user for Windows.
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
   pszLanguage = "ENUS";

   // If language is Dutch, then pick dutch resources...
   if (stricmp(szValue,"nld")==0)
      pszLanguage = "DUTC";
   else
   // If language is any kind of Portuguese, then pick portuguese resources...
   if ((stricmp(szValue,"ptg")==0)||(stricmp(szValue,"ptb")==0))
      pszLanguage = "PORT";
   else
   // If language is German, then pick german resources...
   if (stricmp(szValue,"deu")==0)
      pszLanguage = "GRMN";
   else
   // If language is Swedish, then pick swedish resources...
   if (stricmp(szValue,"sve")==0)
      pszLanguage = "SWED";
   else
   // If language is any kind of French, then pick french resources...
   if ((stricmp(szValue,"fra")==0)||(stricmp(szValue,"frc")==0))
      pszLanguage = "FREN";
   else
   // If language is Finnish, then pick finnish resources...
   if (stricmp(szValue,"fin")==0)
      pszLanguage = "FINN";
   else
   // If language is any kind of Spanish, then pick spanish resources...
   if ((stricmp(szValue,"esn")==0)||(stricmp(szValue,"esp")==0))
      pszLanguage = "SPAN";
   else
   // If language is Italian, then pick italian resources...
   if (stricmp(szValue,"ita")==0)
      pszLanguage = "ITAL";

#if 0
   else
   // If language is Danish, then pick danish resources...
   if (stricmp(szValue,"dan")==0)
      pszLanguage = "DANI";
#endif

   // Build NLS resource library name from language.
   sprintf(szNLSLibrary,"PTW_%s.DLL",pszLanguage);
   // Load NLS resource library
   g_hResInst = LoadLibrary(szNLSLibrary);
   // Check that load operation was successfull...
   if (g_hResInst <= HINSTANCE_ERROR)
   {
      // Create error message string.
      sprintf(szErrorMsg,"Error -1: Could not locate and load '%s' NLS resource library.",szNLSLibrary);
      // Could not load NLS resource library, therefore display error message
      // to user since we need it to run and return failure.
      MessageBox(NULL,szErrorMsg,"PM to WP",(MB_OK|MB_ICONSTOP));
      // Return failure loading library.
      return (FALSE);
   }
   else
      // Return success loading library.
      return (TRUE);
}


//---------------------------------------------------------------------
// LoadStringTable():
//---------------------------------------------------------------------
BOOL LoadStringTable(HINSTANCE hInstance,int nTotalStrings)
{
   int   index,nSize;
   char  szTempString[255];

   // Allocate an array of string pointers that will hold all strings.
   g_apszStringTable = new char *[nTotalStrings];
   // If error allocating space for string pointers, then exit.
   if (g_apszStringTable==NULL)
      return (FALSE);
   // Load strings from resource string table.
   for (index=0; index<nTotalStrings; index++)
   {
      // Get string and store the its real size.
      nSize = LoadString(hInstance,(IDSTR_BASE_INDEX+index),szTempString,sizeof(szTempString));
      // If error loading string resource, exit.
      if (nSize==0)
         return (FALSE);
      // Allocate enough space for the string read.
      g_apszStringTable[index] = new char[nSize+1];
      // If error allocating memory, exit
      if (g_apszStringTable[index]==NULL)
         return (FALSE);
      // Copy read string from temporary space to newly allocated space.
      strcpy(g_apszStringTable[index],szTempString);
   }
   // Return success (all strings have been loaded).
   return (TRUE);
}


//---------------------------------------------------------------------
// GetString():
//    This function returns the string pointer to the string ID passed
// as parameter.  It indexes the global string pointer array created
// by LoadStringTable() function.
//---------------------------------------------------------------------
char *GetString(UINT uIDString)
{
   // Return pointer to string.
   return (g_apszStringTable[(uIDString-IDSTR_BASE_INDEX)]);
}

//---------------------------------------------------------------------
// FreeStringTable():
//---------------------------------------------------------------------
void FreeStringTable(int nTotalStrings)
{
   int   index;

   // Loop through each string in the loaded string table...
   for (index=0; index<nTotalStrings; index++)
      // ...And free the memory allocated for the string.
      delete (g_apszStringTable[index]);
   // Free memory allocated for the array of string pointers.
   delete (g_apszStringTable);
}



//---------------------------------------------------------------------
// ReportError():
//---------------------------------------------------------------------
void ReportError(char *pszErrorMessage)
{
   // only if not in "silent" mode, do we display the error message.
   if (fSilentMode==FALSE)
      MessageBox(NULL,pszErrorMessage,GetString(IDSTR_ERROR_CAPTION),(MB_OK|MB_ICONSTOP));
}


//---------------------------------------------------------------------
// InitializeDialog():
//---------------------------------------------------------------------
void InitializeDialog(HWND hDlg,PROGMANDDE *pProgman)
{
   int   index,iOffset;
   RECT  rcDialog;

   // center dialog box.
   GetWindowRect(hDlg,&rcDialog);
   SetWindowPos(hDlg,NULL,
      (GetSystemMetrics(SM_CXSCREEN)-(rcDialog.right-rcDialog.left))/2,
      (GetSystemMetrics(SM_CYSCREEN)-(rcDialog.bottom-rcDialog.top))/2,
      0,0,(SWP_NOZORDER | SWP_NOSIZE));

   // intialize radio buttons to "All Groups".
   CheckRadioButton(hDlg,IDC_ALL_GROUPS,IDC_SELECTED_GROUP,IDC_ALL_GROUPS);

   // initialize entry field.
   SendDlgItemMessage(hDlg,IDC_FOLDER_NAME,EM_LIMITTEXT,(WPARAM)MAX_STRING,0);
   SetDlgItemText(hDlg,IDC_FOLDER_NAME,GetString(IDSTR_PM_FOLDER));

   // initialize selected groups list box.
   for (index=0; index<pProgman->GetGroupCount(); index++)
   {
      iOffset = (int)(WORD)SendDlgItemMessage(hDlg,IDC_GROUP_LIST,LB_ADDSTRING,0,(LPARAM)(pProgman->GetGroupDetails(index)->m_szTitle));
      SendDlgItemMessage(hDlg,IDC_GROUP_LIST,LB_SETITEMDATA,(WPARAM)iOffset,(LPARAM)(pProgman->GetGroupDetails(index)));
   }

   // disable group listbox.
   EnableWindow(GetDlgItem(hDlg,IDC_GROUP_LIST),FALSE);

   // check all checkboxes by default, only if we are in silent mode...
   if (fSilentMode==TRUE)
   {
      CheckDlgButton(hDlg,IDC_SYSTEM_FOLDER,1);
      CheckDlgButton(hDlg,IDC_MINVIEWER,1);
      CheckDlgButton(hDlg,IDC_TEMPLATES,1);
   }
}

//---------------------------------------------------------------------
// ProfileExists():
//---------------------------------------------------------------------
BOOL ProfileExists(void)
{
   BOOL  fExists;
   char  szWorkingDir[128];
   int   hfileProfile;

   // get current Windows 3.1 directory (<drive>:\WINDOWS).
   GetWindowsDirectory(szWorkingDir,sizeof(szWorkingDir));
   strcat(szWorkingDir,PROFILE_FILENAME);

   // atttempt to open the ini file for update
   hfileProfile = open(szWorkingDir,(_O_BINARY|_O_RDWR));
   // chcek if file does not exists...
   if (hfileProfile==-1)
      fExists = FALSE;
   else
   {
      // close profile.
      close(hfileProfile);
      // file exists.
      fExists = TRUE;
   }

   // return flag.
   return (fExists);
}


//---------------------------------------------------------------------
// OpenProfile():
//---------------------------------------------------------------------
int OpenProfile(BOOL fOverwrite)
{
   char  szWorkingDir[128];
   int   hfileProfile;

   // get current Windows 3.1 directory (<drive>:\WINDOWS).
   GetWindowsDirectory(szWorkingDir,sizeof(szWorkingDir));
   strcat(szWorkingDir,PROFILE_FILENAME);

   // atttempt to open the ini file for update
   if (fOverwrite==TRUE)
   {
      // create or truncate file.
      hfileProfile = open(szWorkingDir,(_O_BINARY|_O_CREAT|_O_TRUNC|O_RDWR),(_S_IREAD|_S_IWRITE));
      // write out file header informatrion.
      WriteProfileHeader(hfileProfile);
      // write out desktop object information.
      CreateDESKTOP(hfileProfile);
      // initialize file to 0 top-level objects.
      WriteDesktopObjectCount(hfileProfile,0);
   }
   else
   {
      // open file for reading and writing.
      hfileProfile = open(szWorkingDir,(_O_BINARY|_O_RDWR));
   }

   // position file pointer at the end of file (append mode).
   lseek(hfileProfile,0L,SEEK_END);

   // return handle to profile.
   return (hfileProfile);
}

//---------------------------------------------------------------------
// ReadDesktopObjectCount():
//---------------------------------------------------------------------
int ReadDesktopObjectCount(int hProfile)
{
   FILE_WPS_FOLDER_STRUCT  ffs;

   lseek(hProfile,sizeof(WPSHELL_PROFILE_HEADER),SEEK_SET);
   read(hProfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT));
   return (ffs.m_con.m_nObjects);
}


//---------------------------------------------------------------------
// WriteDesktopObjectCount():
//---------------------------------------------------------------------
void WriteDesktopObjectCount(int hProfile,int nCount)
{
   FILE_WPS_FOLDER_STRUCT  ffs;

   lseek(hProfile,sizeof(WPSHELL_PROFILE_HEADER),SEEK_SET);
   read(hProfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT));
   ffs.m_con.m_nObjects = nCount;
   lseek(hProfile,sizeof(WPSHELL_PROFILE_HEADER),SEEK_SET);
   write(hProfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT));
}


//---------------------------------------------------------------------
// CloseProfile():
//---------------------------------------------------------------------
BOOL CloseProfile(int hfileProfile,int nIncrementCount)
{
   int   nComponents;

   // read current desktop object count.
   nComponents = ReadDesktopObjectCount(hfileProfile);
   // increment object count to the number of top-level objects added.
   nComponents += nIncrementCount;
   // write out the new number of desktop objects.
   WriteDesktopObjectCount(hfileProfile,nComponents);
   // close profile.
   close(hfileProfile);
   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// SetupProfile():
//---------------------------------------------------------------------
BOOL SetupProfile(int *phfile)
{
   int   iResponse;

   // initialize handle to zero.
   *phfile = 0;

   // check if profile exists (WPSHELL.INI) and we are not in silent mode.
   if ((ProfileExists()==TRUE)&&(fSilentMode==FALSE))
   {
      iResponse = MessageBox(NULL,GetString(IDSTR_FILE_EXISTS_QUERY),
                             GetString(IDSTR_PMTOWP_CAPTION),(MB_ICONQUESTION|MB_YESNOCANCEL));
      // if user wants to abort migration.
      if (iResponse==IDCANCEL)
         return (FALSE);
   }
   else
      // create new file.
      iResponse = IDYES;

   // depending on the user's response...
   switch (iResponse)
   {
      case IDYES:
         // create a new file and overwrite existing one.
         *phfile = OpenProfile(TRUE);
         break;

      case IDNO:
         // append to current file.
         *phfile = OpenProfile(FALSE);
         break;
   }

   // if could not open or create file...
   if (*phfile==-1)
      return (FALSE);

   // return success
   return (TRUE);
}


//---------------------------------------------------------------------
// WriteProfileHeader():
//---------------------------------------------------------------------
BOOL WriteProfileHeader(int hProfile)
{
   WPSHELL_PROFILE_HEADER  wpsProfileHeader;

   // Initialize Workplace Shell profile header information.
   // Store Workplace Shell profile magic ID number.
   wpsProfileHeader.m_dwMagicID     = WPSHELL_PROFILE_MAGIC_NUMBER;
   wpsProfileHeader.m_wMajorVersion = 0x0001;
   wpsProfileHeader.m_wMinorVersion = 0x0000;
   // Store file offset to begining of data.
   wpsProfileHeader.m_dwDataOffset = sizeof(WPSHELL_PROFILE_HEADER);

   // Write WPS profile header information.
   if (write(hProfile,&wpsProfileHeader,sizeof(WPSHELL_PROFILE_HEADER))!=sizeof(WPSHELL_PROFILE_HEADER))
      return (FALSE);

   // Return success writing profile header information.
   return (TRUE);
}


//---------------------------------------------------------------------
// CreateDESKTOP():
//---------------------------------------------------------------------
BOOL CreateDESKTOP(int hProfile)
{
   FILE_WPS_FOLDER_STRUCT  ffs;

   ffs.m_obj.m_dwType         = OBJTYPE_DESKTOP;
   strcpy(ffs.m_obj.m_szTitle,"");
   ffs.m_obj.m_ptIcon.x       = (GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXICON))/2;
   ffs.m_obj.m_ptIcon.y       = (GetSystemMetrics(SM_CYSCREEN)-GetSystemMetrics(SM_CYICON))/2;
   strcpy(ffs.m_obj.m_szIconPath,WPS_EXENAME);
   ffs.m_obj.m_wIconIndex     = 1;
   ffs.m_obj.m_wState         = 0L;
   ffs.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   ffs.m_wnd.m_ptWindow.x     = 0;
   ffs.m_wnd.m_ptWindow.y     = 0;
   ffs.m_wnd.m_sizeWindow.cx  = GetSystemMetrics(SM_CXSCREEN);
   ffs.m_wnd.m_sizeWindow.cy  = GetSystemMetrics(SM_CYSCREEN);
   ffs.m_wnd.m_fScrollable    = FALSE;
   // folder data.
   if (fUseWarpColors==TRUE)
      ffs.m_fol.m_rgbBackground  = RGB(0,128,128); // Teal background.
   else
      ffs.m_fol.m_rgbBackground  = GetSysColor(COLOR_BACKGROUND);
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(ffs.m_fol.m_lfIconText),0);
   ffs.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(ffs.m_fol.m_szImagePath,"");
   ffs.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   ffs.m_con.m_nObjects       = 0;

   // write out folder structure information.
   if (write(hProfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// CreatePROGMANFolder():
//---------------------------------------------------------------------
BOOL CreatePROGMANFolder(int hfile,char *pszTitle,int nCount)
{
   FILE_WPS_FOLDER_STRUCT  ffs;

   ffs.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(ffs.m_obj.m_szTitle,pszTitle);
   ffs.m_obj.m_ptIcon.x       = 30;
   ffs.m_obj.m_ptIcon.y       = GetSystemMetrics(SM_CYICONSPACING);
   strcpy(ffs.m_obj.m_szIconPath,WPS_EXENAME);
   ffs.m_obj.m_wIconIndex     = DEFAULT_PROGMANFOLDER_ICON_INDEX;
   ffs.m_obj.m_wState         = 0L;
   ffs.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   ffs.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   ffs.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   ffs.m_wnd.m_sizeWindow.cx  = 400;
   ffs.m_wnd.m_sizeWindow.cy  = 150;
   ffs.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   ffs.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(ffs.m_fol.m_lfIconText),0);
   ffs.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(ffs.m_fol.m_szImagePath,"");
   ffs.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   ffs.m_con.m_nObjects       = nCount;

   // write out folder structure information.
   if (write(hfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// CreateGROUPFolder():
//---------------------------------------------------------------------
BOOL CreateGROUPFolder(int hfile,GROUPINFO *pGroup,int x,int y)
{
   FILE_WPS_FOLDER_STRUCT  ffs;
   FILE_WPS_PROGRAM_STRUCT fps;
   ITEMINFO                *pItem;
   int                     index;

   // write out group information.
   ffs.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(ffs.m_obj.m_szTitle,pGroup->m_szTitle);
   ffs.m_obj.m_ptIcon.x       = 23+x;
   ffs.m_obj.m_ptIcon.y       = 5+y;
   strcpy(ffs.m_obj.m_szIconPath,WPS_EXENAME);
   ffs.m_obj.m_wIconIndex     = DEFAULT_GROUP_ICON_INDEX;
   ffs.m_obj.m_wState         = 0L;
   ffs.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   ffs.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   ffs.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   ffs.m_wnd.m_sizeWindow.cx  = 400;
   ffs.m_wnd.m_sizeWindow.cy  = 150;
   ffs.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   ffs.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(ffs.m_fol.m_lfIconText),0);
   ffs.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(ffs.m_fol.m_szImagePath,"");
   ffs.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   ffs.m_con.m_nObjects       = pGroup->m_nItemCount;

   // write group folder information.
   if (write(hfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // loop through each item in current group.
   for (index=0; index<pGroup->m_nItemCount; index++)
   {
      // for convenience, make a local pointer to group item information.
      pItem = &(pGroup->m_pItemInfo[index]);
      // reset values in program structure.
      memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
      // initialize program item information.
      fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
      strcpy(fps.m_obj.m_szTitle,pItem->m_szTitle);
      fps.m_obj.m_ptIcon.x       = ((index % 10)*GetSystemMetrics(SM_CXICONSPACING))+23;
      fps.m_obj.m_ptIcon.y       = ((index / 10)*GetSystemMetrics(SM_CYICONSPACING))+5;
      strcpy(fps.m_obj.m_szIconPath,pItem->m_szIconPath);
      fps.m_obj.m_wIconIndex     = (WORD)pItem->m_iIconIndex;
      fps.m_obj.m_wState         = 0L;
      fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
      // program information.
      strncpy(fps.m_prg.m_szProgramPath,pItem->m_szCommandLine,MAX_PROGRAM_PATH);
      strncpy(fps.m_prg.m_szParameters,pItem->m_szParameters,MAX_PARAMETERS_LINE);
      strncpy(fps.m_prg.m_szWorkingDir,pItem->m_szWorkDir,MAX_WORKING_DIR);

      // write program item information to profile.
      if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
         return (FALSE);
   }

   // successfully saved group and items in group.
   return (TRUE);
}


//---------------------------------------------------------------------
// MigrateAll():
//---------------------------------------------------------------------
BOOL MigrateAll(int hfile,PROGMANDDE *pProgman,char *pszFolderName)
{
   int   index,x,y;

   // create desktop folder with user-specified name which will contain all
   // of the PROGMAN groups.
   if (CreatePROGMANFolder(hfile,pszFolderName,pProgman->GetGroupCount())==FALSE)
      return (FALSE);

   // loop through all groups in PROGMAN and create folders.
   for (index=0; index<pProgman->GetGroupCount(); index++)
   {
      // calculate (x,y) position of group in PROGMAN folder.
      x = ((index % 10)*GetSystemMetrics(SM_CXICONSPACING))+23;
      y = ((index / 10)*GetSystemMetrics(SM_CYICONSPACING))+5;
      // create GROUP folder in PROGMAN folder.
      if (CreateGROUPFolder(hfile,pProgman->GetGroupDetails(index),x,y)==FALSE)
         return (FALSE);
   }

   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// MigrateGroup():
//---------------------------------------------------------------------
BOOL MigrateGroup(int hfile,GROUPINFO *pGroup)
{
   // migrate a specific group that the user selected.
   return (CreateGROUPFolder(hfile,pGroup,
              GetSystemMetrics(SM_CXICONSPACING)*2,
              GetSystemMetrics(SM_CYICONSPACING)));
}


//---------------------------------------------------------------------
// ProcessOtherFolders():
//---------------------------------------------------------------------
BOOL ProcessOtherFolders(HWND hwnd,int hfile,int *nCount)
{
   // check if we need to create other folders...
   // "Windows System" folder ?
   if (IsDlgButtonChecked(hwnd,IDC_SYSTEM_FOLDER)==1)
   {
      // create a "Windows System" folder.
      if (CreateSystemFolder(hfile)==TRUE)
         (*nCount)++;
      else
         return (FALSE);
   }
   // "Minimized Window Viewer" folder ?
   if (IsDlgButtonChecked(hwnd,IDC_MINVIEWER)==1)
   {
      // create a "Minimized Window Viewer" folder.
      if (CreateMinViewer(hfile)==TRUE)
         (*nCount)++;
      else
         return (FALSE);
   }
   // "Templates Folder" folder ?
   if (IsDlgButtonChecked(hwnd,IDC_TEMPLATES)==1)
   {
      // create a "Templates Folder" folder.
      if (CreateTemplatesFolder(hfile)==TRUE)
         (*nCount)++;
      else
         return (FALSE);
   }

   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// OnMigrateCommand():
//---------------------------------------------------------------------
BOOL OnMigrateCommand(HWND hwnd,PROGMANDDE *pProgman)
{
   GROUPINFO   *pGroup;
   char        szFolderName[MAX_STRING+1];
   int         hfile,nSelected,nMigratedFolders = 0;

   // migrate PROGMAN group(s).
   if (IsDlgButtonChecked(hwnd,IDC_ALL_GROUPS)==1)
   {
      // get name of folder to contain all groups.
      GetDlgItemText(hwnd,IDC_FOLDER_NAME,szFolderName,sizeof(szFolderName));
      // open and setup profile.
      if (SetupProfile(&hfile)==FALSE)
      {
         if (hfile==0)
            return ((BOOL)100);
         else
            return (FALSE);
      }
      // migrate all groups in PROGMAN.
      if (MigrateAll(hfile,pProgman,szFolderName)==TRUE)
      {
         // increment folder count by the number of migrated "groups".
         nMigratedFolders++;
         // process other folders that the user might want created.
         if (ProcessOtherFolders(hwnd,hfile,&nMigratedFolders)==FALSE)
            return (FALSE);
         // close profile, and increment top-level count of objects.
         CloseProfile(hfile,nMigratedFolders);
      }
   }
   else
   {
      // get selected group item in listbox.
      nSelected = (int)SendDlgItemMessage(hwnd,IDC_GROUP_LIST,LB_GETCURSEL,0,0);
      // if no error occurred...
      if (nSelected!=LB_ERR)
      {
         // get pointer to selected group information.
         pGroup = (GROUPINFO *)SendDlgItemMessage(hwnd,IDC_GROUP_LIST,LB_GETITEMDATA,(WPARAM)nSelected,0);
         // open and setup profile.
         if (SetupProfile(&hfile)==FALSE)
         {
            if (hfile==0)
               return ((BOOL)100);
            else
               return (FALSE);
         }
         // migrate selected group.
         if (MigrateGroup(hfile,pGroup)==TRUE)
         {
            // increment folder count by one "group".
            nMigratedFolders++;
            // process other folders that the user might want created.
            if (ProcessOtherFolders(hwnd,hfile,&nMigratedFolders)==FALSE)
               return (FALSE);
            // close profile, and increment top-level count of objects.
            CloseProfile(hfile,nMigratedFolders);
         }
      }
      else
      {
         // no item currently selected in listbox.
         MessageBox(hwnd,GetString(IDSTR_NO_SELECTION),GetString(IDSTR_ERROR_CAPTION),(MB_OK|MB_ICONSTOP));
         return (FALSE);
      }
   }
   // return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// QueryDriveType():
//---------------------------------------------------------------------
#pragma warning( disable : 4704 ) // Disable warning about "in-line assembler preculdes optimizations"
WORD QueryDriveType(int uDriveIndex)
{
   WORD  iType;
   BOOL  fCDROM=FALSE,fRAM=FALSE;

   //Validate possible drive indices.
   if ((0 > uDriveIndex) || (25 < uDriveIndex))
      return (WPDRIVE_UNKNOWN);

   iType = (WORD)::GetDriveType(uDriveIndex);

   //Check for CD-ROM on FIXED and REMOTE drives only.
   if ((DRIVE_FIXED==iType) || (DRIVE_REMOTE==iType))
   {
      _asm
      {
         mov     ax,1500h        //Check if MSCDEX exists.
         xor     bx,bx
         int     2fh

         or      bx,bx    //BX unchanged if MSCDEX is not around.
         jz      CheckRAMDrive   //No? Go check for RAM drive.

         mov     ax,150Bh  //Check if drive is using CD driver.

         mov     cx,uDriveIndex
         int     2fh

         mov     fCDROM,ax    //AX if the CD-ROM flag.
         or      ax,ax
         jnz     Exit         //Leave if we found a CD-ROM drive.

         CheckRAMDrive:
      }
   }

   //Check for RAM drives on FIXED disks only.
   if (DRIVE_FIXED==iType)
   {
      /*
      * Check for RAM drive is done by reading the boot sector
      * and looking at the number of FATs. RAM disks only have 1

      * while all others have 2.
      */
      _asm
      {
         push    ds

         mov     bx,ss
         mov     ds,bx

         sub     sp,0200h    //Reserve 512 bytes to read a sector
         mov     bx,sp       //and point BX there.

         mov     ax,uDriveIndex //Read the boot sector of the drive.
         mov     cx,1
         xor     dx,dx

         int     25h
         add     sp,2        //Int 25h requires stack cleanup.

         jc      DriveNotRAM

         mov     bx,sp
         cmp     ss:[bx+15h],0f8h    //Reverify fixed disk.
         jne     DriveNotRAM
         cmp     ss:[bx+10h],1  //Check if there's only one FAT.
         jne     DriveNotRAM
         mov     fRAM,1

         DriveNotRAM:
         add     sp,0200h
         pop     ds

         Exit:
         //Leave fRAM untouched--it's FALSE by default.
      }
   }


   /*
    * If either CD-ROM or RAM drive flags are set, return privately
    * defined flags for them (outside of Win32). Otherwise, return
    * the type given from GetDriveType.
    */

   if (fCDROM)
      return (WPDRIVE_CDROM);

   if (fRAM)
      return (WPDRIVE_RAMDISK);

   //Drive B on a one-drive system returns < 2 from GetDriveType.
   return (iType);
}


//---------------------------------------------------------------------
// CreateDrivesFolder():
//---------------------------------------------------------------------
BOOL CreateDrivesFolder(int hfile)
{
   FILE_WPS_FOLDER_STRUCT    fpgsData;
   FILE_WPS_DRIVE_STRUCT     fdrvData;
   int                       i,index,iIconSpacingX,iIconSpacingY,nDrives;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // Count the number of drives.
   nDrives = 0;
   for (index=0; index<26; index++)
   {
      if (GetDriveType(index)!=0)
         nDrives++;
   }

   i = 1;
   fpgsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_DRIVES));
   fpgsData.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fpgsData.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fpgsData.m_obj.m_szIconPath,WPS_EXENAME);
   fpgsData.m_obj.m_wIconIndex     = ICONINDEX(IDI_DRIVES_FOLDER);
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 400;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       =  nDrives;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   i = 0;
   for (index=0; index<26; index++)
   {
      if (GetDriveType(index)!=0)
      {
         // create drive objects.
         // object data.
         fdrvData.m_obj.m_dwType         = OBJTYPE_DRIVE;
         sprintf(fdrvData.m_obj.m_szTitle,"%s %c:",GetString(IDSTR_OBJTITLE_DRIVE),('A'+index));
         fdrvData.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
         fdrvData.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
         strcpy(fdrvData.m_obj.m_szIconPath,"");
         fdrvData.m_obj.m_wIconIndex     = 0;
         fdrvData.m_obj.m_wState         = 0L;
         fdrvData.m_obj.m_dwFlags        = (DEF_FOLDER_FLAGS);
         // window data.
         fdrvData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
         fdrvData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
         fdrvData.m_wnd.m_sizeWindow.cx  = 600;
         fdrvData.m_wnd.m_sizeWindow.cy  = 300;
         fdrvData.m_wnd.m_fScrollable    = TRUE;
         // folder data.
         fdrvData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
         SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fdrvData.m_fol.m_lfIconText),0);
         fdrvData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
         strcpy(fdrvData.m_fol.m_szImagePath,"");
         fdrvData.m_fol.m_wOpenView      = VIEW_ICON;
         // drive data.
         fdrvData.m_drv.m_uDriveIndex = index;
         fdrvData.m_drv.m_Type = QueryDriveType(index);
         // container data.
         fdrvData.m_con.m_nObjects       = 0;

         // Write out drive object information.
         if (write(hfile,&fdrvData,sizeof(FILE_WPS_DRIVE_STRUCT))!=sizeof(FILE_WPS_DRIVE_STRUCT))
            return (FALSE);

         // Increment position counter.
         i++;
      }
   }

   return (TRUE);
}


//---------------------------------------------------------------------
// CreateCommandPromptsFolder():
//---------------------------------------------------------------------
BOOL CreateCommandPromptsFolder(int hfile)
{
   FILE_WPS_FOLDER_STRUCT    fpgsData;
   FILE_WPS_PROGRAM_STRUCT   fps;
   int                       i,iIconSpacingX,iIconSpacingY;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // create "Command Prompts" folder now.
   // object data.
   i = 0;
   fpgsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_COMMANDPROMPTS));
   fpgsData.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fpgsData.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fpgsData.m_obj.m_szIconPath,DEFAULT_GROUP_ICON_PATH);
   fpgsData.m_obj.m_wIconIndex     = DEFAULT_FOLDER_ICON_INDEX;
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 400;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       = 1;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   i = 0;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_DOSPROMPT));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_DOSPROMPT_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"DOSPRMPT.PIF",MAX_PROGRAM_PATH);
   strncpy(fps.m_prg.m_szParameters,"",MAX_PARAMETERS_LINE);
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// CreateTemplatesFolder():
//---------------------------------------------------------------------
BOOL CreateTemplatesFolder(int hfile)
{
   FILE_WPS_FOLDER_STRUCT     ffsData;
   FILE_WPS_TEMPLATE_STRUCT   ftsData;
   int                        iIconNum,iIconSpacingX,iIconSpacingY;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // create "Templates" folder object on top of current desktop.
   ffsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(ffsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_TEMPLATES));
   ffsData.m_obj.m_ptIcon.x       = 30;
   ffsData.m_obj.m_ptIcon.y       = (int)(4.75*GetSystemMetrics(SM_CYICONSPACING));
   strcpy(ffsData.m_obj.m_szIconPath,DEFAULT_TEMPFOLDER_ICON_PATH);
   ffsData.m_obj.m_wIconIndex = DEFAULT_TEMPFOLDER_ICON_INDEX;
   ffsData.m_obj.m_wState         = 0L;
   ffsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   ffsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   ffsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   ffsData.m_wnd.m_sizeWindow.cx  = 400;
   ffsData.m_wnd.m_sizeWindow.cy  = 150;
   ffsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   ffsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(ffsData.m_fol.m_lfIconText),0);
   ffsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(ffsData.m_fol.m_szImagePath,"");
   ffsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   ffsData.m_con.m_nObjects       = 5;
   if (write(hfile,&ffsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // create "WPDATAFILE" template.
   iIconNum = 0;
   ftsData.m_obj.m_dwType         = OBJTYPE_TEMPLATE;
   strcpy(ftsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_DATAFILE));
   ftsData.m_obj.m_ptIcon.x       = ((iIconNum % 10) * iIconSpacingX + 23);
   ftsData.m_obj.m_ptIcon.y       = ((iIconNum / 10) * iIconSpacingY + 5);
   strcpy(ftsData.m_obj.m_szIconPath,DEFAULT_TEMPLATE_DATAFILE_ICON_PATH);
   ftsData.m_obj.m_wIconIndex = DEFAULT_TEMPLATE_DATAFILE_ICON_INDEX;
   ftsData.m_obj.m_wState         = 0L;
   ftsData.m_obj.m_dwFlags        = 0L;
   ftsData.m_tmp.m_dwTemplateType = OBJTYPE_DATAFILE;
   strncpy(&(ftsData.m_tmp.m_szInstanceTitle[0]),DEFAULT_DATAFILE_TITLE,MAX_TITLE_NAME);
   strncpy(&(ftsData.m_tmp.m_szInstanceIconPath[0]),DEFAULT_DATAFILE_ICON_PATH,MAX_PATH_FILENAME);
   ftsData.m_tmp.m_wInstanceIconIndex = DEFAULT_DATAFILE_ICON_INDEX;
   if (write(hfile,&ftsData,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);

   // create "WPFOLDER" template.
   iIconNum++;
   ftsData.m_obj.m_dwType           = OBJTYPE_TEMPLATE;
   strcpy(ftsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_FOLDER));
   ftsData.m_obj.m_ptIcon.x         = ((iIconNum % 10) * iIconSpacingX + 23);
   ftsData.m_obj.m_ptIcon.y         = ((iIconNum / 10) * iIconSpacingY + 5);
   strcpy(ftsData.m_obj.m_szIconPath,DEFAULT_TEMPLATE_FOLDER_ICON_PATH);
   ftsData.m_obj.m_wIconIndex       = DEFAULT_TEMPLATE_FOLDER_ICON_INDEX;
   ftsData.m_obj.m_wState           = 0L;
   ftsData.m_obj.m_dwFlags          = DEF_FOLDER_FLAGS;
   ftsData.m_tmp.m_dwTemplateType   = OBJTYPE_FOLDER;
   strncpy(&(ftsData.m_tmp.m_szInstanceTitle[0]),DEFAULT_FOLDER_TITLE,MAX_TITLE_NAME);
   strncpy(&(ftsData.m_tmp.m_szInstanceIconPath[0]),DEFAULT_FOLDER_ICON_PATH,MAX_PATH_FILENAME);
   ftsData.m_tmp.m_wInstanceIconIndex = DEFAULT_FOLDER_ICON_INDEX;
   if (write(hfile,&ftsData,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);


   // create "WPPROGRAM" template.
   iIconNum++;
   ftsData.m_obj.m_dwType         = OBJTYPE_TEMPLATE;
   strcpy(ftsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_PROGRAM));
   ftsData.m_obj.m_ptIcon.x       = ((iIconNum % 10) * iIconSpacingX + 23);
   ftsData.m_obj.m_ptIcon.y       = ((iIconNum / 10) * iIconSpacingY + 5);
   strcpy(ftsData.m_obj.m_szIconPath,DEFAULT_TEMPLATE_PROGRAM_ICON_PATH);
   ftsData.m_obj.m_wIconIndex = DEFAULT_TEMPLATE_PROGRAM_ICON_INDEX;
   ftsData.m_obj.m_wState         = 0L;
   ftsData.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   ftsData.m_tmp.m_dwTemplateType = OBJTYPE_PROGRAM;
   strncpy(&(ftsData.m_tmp.m_szInstanceTitle[0]),DEFAULT_PROGRAM_TITLE,MAX_TITLE_NAME);
   strncpy(&(ftsData.m_tmp.m_szInstanceIconPath[0]),DEFAULT_PROGRAM_ICON_PATH,MAX_PATH_FILENAME);
   ftsData.m_tmp.m_wInstanceIconIndex = DEFAULT_PROGRAM_ICON_INDEX;
   if (write(hfile,&ftsData,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);

   // create "WPSHREDDER" template.
   iIconNum++;
   ftsData.m_obj.m_dwType         = OBJTYPE_TEMPLATE;
   strcpy(ftsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_SHREDDER));
   ftsData.m_obj.m_ptIcon.x       = ((iIconNum % 10) * iIconSpacingX + 23);
   ftsData.m_obj.m_ptIcon.y       = ((iIconNum / 10) * iIconSpacingY + 5);
   strcpy(ftsData.m_obj.m_szIconPath,DEFAULT_TEMPLATE_SHREDDER_ICON_PATH);
   ftsData.m_obj.m_wIconIndex     = DEFAULT_TEMPLATE_SHREDDER_ICON_INDEX;
   ftsData.m_obj.m_wState         = 0L;
   ftsData.m_obj.m_dwFlags        = 0L;
   ftsData.m_tmp.m_dwTemplateType = OBJTYPE_SHREDDER;
   strncpy(&(ftsData.m_tmp.m_szInstanceTitle[0]),DEFAULT_SHREDDER_TITLE,MAX_TITLE_NAME);
   strncpy(&(ftsData.m_tmp.m_szInstanceIconPath[0]),DEFAULT_SHREDDER_ICON_PATH,MAX_PATH_FILENAME);
   ftsData.m_tmp.m_wInstanceIconIndex = DEFAULT_SHREDDER_ICON_INDEX;
   if (write(hfile,&ftsData,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);

   // create "WPDRIVE" template.
   iIconNum++;
   ftsData.m_obj.m_dwType         = OBJTYPE_TEMPLATE;
   strcpy(ftsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_DRIVE));
   ftsData.m_obj.m_ptIcon.x       = ((iIconNum % 10) * iIconSpacingX + 23);
   ftsData.m_obj.m_ptIcon.y       = ((iIconNum / 10) * iIconSpacingY + 5);
   strcpy(ftsData.m_obj.m_szIconPath,WPS_EXENAME);
   ftsData.m_obj.m_wIconIndex     = ICONINDEX(IDI_DRIVETEMPLATE);
   ftsData.m_obj.m_wState         = 0L;
   ftsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   ftsData.m_tmp.m_dwTemplateType = OBJTYPE_DRIVE;
   sprintf(&(ftsData.m_tmp.m_szInstanceTitle[0]),"%s %c:",GetString(IDSTR_OBJTITLE_DRIVE),'C');
   strncpy(&(ftsData.m_tmp.m_szInstanceIconPath[0]),"",MAX_PATH_FILENAME);
   ftsData.m_tmp.m_wInstanceIconIndex = 0;
   if (write(hfile,&ftsData,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// CreateSystemSetupFolder():
//---------------------------------------------------------------------
BOOL CreateSystemSetupFolder(int hfile)
{
   FILE_WPS_FOLDER_STRUCT    fpgsData;
   FILE_OBJECT_STRUCT        fobjData;
   FILE_WPS_PROGRAM_STRUCT   fps;
   int                       i,iIconSpacingX,iIconSpacingY;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // create "System Setup" folder now.
   // object data.
   i = 2;
   fpgsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_SYSTEMSETUP));
   fpgsData.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fpgsData.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fpgsData.m_obj.m_szIconPath,DEFAULT_GROUP_ICON_PATH);
   fpgsData.m_obj.m_wIconIndex     = DEFAULT_SYSTEMSETUPFOLDER_ICON_INDEX;
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 675;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       = 8;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // write program item information to profile.
   // object information.
   i = 0;
   fobjData.m_dwType         = OBJTYPE_SYSTEMSETUP;
   strcpy(fobjData.m_szTitle,GetString(IDSTR_OBJTITLE_SYSTEM));
   fobjData.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fobjData.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fobjData.m_szIconPath,WPS_EXENAME);
   fobjData.m_wIconIndex     = DEFAULT_SYSTEMSETUP_ICON_INDEX;
   fobjData.m_wState         = 0L;
   fobjData.m_dwFlags        = DEF_PROGRAM_FLAGS;

   if (write(hfile,&fobjData,sizeof(FILE_OBJECT_STRUCT))!=sizeof(FILE_OBJECT_STRUCT))
      return (FALSE);

   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_SCHEMEPALETTE));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_COLORSCHEME_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   sprintf(fps.m_prg.m_szParameters,"MAIN.CPL %s",GetString(IDSTR_CONTROLPANEL_COLORS));
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);


   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_FONTPALETTE));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_FONTPALETTE_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   sprintf(fps.m_prg.m_szParameters,"MAIN.CPL %s",GetString(IDSTR_CONTROLPANEL_FONTS));
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);



   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_SOUND));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_SOUND_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   strncpy(fps.m_prg.m_szParameters,"SND.CPL",MAX_PARAMETERS_LINE);
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);

   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_COUNTRY));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_COUNTRY_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   sprintf(fps.m_prg.m_szParameters,"MAIN.CPL %s",GetString(IDSTR_CONTROLPANEL_INTERNATIONAL));
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);


   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_DDINSTALL));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_DEVICEDRIVER_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   strncpy(fps.m_prg.m_szParameters,"DRIVERS.CPL",MAX_PARAMETERS_LINE);
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);


   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_MOUSE));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_MOUSE_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   sprintf(fps.m_prg.m_szParameters,"MAIN.CPL %s",GetString(IDSTR_CONTROLPANEL_MOUSE));
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);


   i++;
   // reset values in program structure.
   memset(&fps,0,sizeof(FILE_WPS_PROGRAM_STRUCT));
   // initialize program item information.
   fps.m_obj.m_dwType         = OBJTYPE_PROGRAM;
   strcpy(fps.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_KEYBOARD));
   fps.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fps.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fps.m_obj.m_szIconPath,WPS_EXENAME);
   fps.m_obj.m_wIconIndex     = (WORD)DEFAULT_KEYBOARD_ICON_INDEX;
   fps.m_obj.m_wState         = 0L;
   fps.m_obj.m_dwFlags        = DEF_PROGRAM_FLAGS;
   // program information.
   strncpy(fps.m_prg.m_szProgramPath,"CONTROL.EXE",MAX_PROGRAM_PATH);
   sprintf(fps.m_prg.m_szParameters,"MAIN.CPL %s",GetString(IDSTR_CONTROLPANEL_KEYBOARD));
   strncpy(fps.m_prg.m_szWorkingDir,"",MAX_WORKING_DIR);

   // write program item information to profile.
   if (write(hfile,&fps,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);


   return (TRUE);
}


//---------------------------------------------------------------------
// CreateSystemFolder():
//---------------------------------------------------------------------
BOOL CreateSystemFolder(int hfile)
{
   FILE_WPS_FOLDER_STRUCT   fpgsData;
   int                      i,iIconSpacingX,iIconSpacingY;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // create "Windows System" folder now.
   // object data.
   fpgsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_WINDOWSSYSTEM));
   fpgsData.m_obj.m_ptIcon.x       = 30;
   fpgsData.m_obj.m_ptIcon.y       = (int)(2.25*GetSystemMetrics(SM_CYICONSPACING));
   strcpy(fpgsData.m_obj.m_szIconPath,DEFAULT_GROUP_ICON_PATH);
   fpgsData.m_obj.m_wIconIndex     = DEFAULT_SYSTEMFOLDER_ICON_INDEX;
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 400;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       = 4;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   if (CreateCommandPromptsFolder(hfile)==FALSE)
      return (FALSE);

   if (CreateDrivesFolder(hfile)==FALSE)
      return (FALSE);

   if (CreateSystemSetupFolder(hfile)==FALSE)
      return (FALSE);

   // create "Startup" folder now.
   // object data.
   i = 3;
   fpgsData.m_obj.m_dwType         = OBJTYPE_FOLDER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_STARTUP));
   fpgsData.m_obj.m_ptIcon.x       = ((i % 10) * iIconSpacingX + 23);
   fpgsData.m_obj.m_ptIcon.y       = ((i / 10) * iIconSpacingY + 5);
   strcpy(fpgsData.m_obj.m_szIconPath,DEFAULT_GROUP_ICON_PATH);
   fpgsData.m_obj.m_wIconIndex     = DEFAULT_STARTUPFOLDER_ICON_INDEX;
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = (DEF_FOLDER_FLAGS|0x00000100L);
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 400;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       = 0;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// CreateMinViewer():
//---------------------------------------------------------------------
BOOL CreateMinViewer(int hfile)
{
   FILE_WPS_FOLDER_STRUCT   fpgsData;
   int                      iIconSpacingX,iIconSpacingY;

   // query default icon spacing.
   iIconSpacingX = GetSystemMetrics( SM_CXICONSPACING );
   iIconSpacingY = GetSystemMetrics( SM_CYICONSPACING );

   // create "Windows System" folder now.
   // object data.
   fpgsData.m_obj.m_dwType         = OBJTYPE_MINVIEWER;
   strcpy(fpgsData.m_obj.m_szTitle,GetString(IDSTR_OBJTITLE_MINVIEWER));
   fpgsData.m_obj.m_ptIcon.x       = 30;
   fpgsData.m_obj.m_ptIcon.y       = (int)(3.5*GetSystemMetrics(SM_CYICONSPACING));
   strcpy(fpgsData.m_obj.m_szIconPath,DEFAULT_GROUP_ICON_PATH);
   fpgsData.m_obj.m_wIconIndex     = DEFAULT_MINVIEWER_ICON_INDEX;
   fpgsData.m_obj.m_wState         = 0L;
   fpgsData.m_obj.m_dwFlags        = DEF_FOLDER_FLAGS;
   // window data.
   fpgsData.m_wnd.m_ptWindow.x     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_ptWindow.y     = CW_USEDEFAULT;
   fpgsData.m_wnd.m_sizeWindow.cx  = 400;
   fpgsData.m_wnd.m_sizeWindow.cy  = 150;
   fpgsData.m_wnd.m_fScrollable    = TRUE;
   // folder data.
   fpgsData.m_fol.m_rgbBackground  = DEFAULT_BACKGROUND_COLOR;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(fpgsData.m_fol.m_lfIconText),0);
   fpgsData.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   strcpy(fpgsData.m_fol.m_szImagePath,"");
   fpgsData.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   fpgsData.m_con.m_nObjects       = 0;

   if (write(hfile,&fpgsData,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   return (TRUE);
}


//---------------------------------------------------------------------
// MAIN DIALOG PROCEDURE
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// MainDlgProc():
//---------------------------------------------------------------------
BOOL CALLBACK MainDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
   static PROGMANDDE    *pProgman;

   // process window messages.
   switch(msg)
   {
      case WM_INITDIALOG:
         // store pointer to PROGMANDDE object.
         pProgman = (PROGMANDDE *)lParam;
         // iniialize dialog controls.
         InitializeDialog(hwnd,pProgman);
         // if we are in silent mode, automatically select "all groups".
         if (fSilentMode==TRUE)
            PostMessage(hwnd,WM_COMMAND,(WPARAM)IDOK,0);
         return (TRUE);

      case WM_COMMAND:
         switch (wParam)
         {
            case IDC_ALL_GROUPS:
               EnableWindow(GetDlgItem(hwnd,IDC_TEXT),TRUE);
               EnableWindow(GetDlgItem(hwnd,IDC_FOLDER_NAME),TRUE);
               EnableWindow(GetDlgItem(hwnd,IDC_GROUP_LIST),FALSE);
               break;

            case IDC_SELECTED_GROUP:
               EnableWindow(GetDlgItem(hwnd,IDC_TEXT),FALSE);
               EnableWindow(GetDlgItem(hwnd,IDC_FOLDER_NAME),FALSE);
               EnableWindow(GetDlgItem(hwnd,IDC_GROUP_LIST),TRUE);
               break;

            case IDOK:
               // disable pushbuttons while migrating PROGMAN group(s).
               EnableWindow(GetDlgItem(hwnd,IDCANCEL),FALSE);
               EnableWindow(GetDlgItem(hwnd,IDOK),FALSE);

               // migrate group(s)...
               switch (OnMigrateCommand(hwnd,pProgman))
               {
                  case TRUE:
                     // display message so user knows we have migrated successfully.
                     if (fSilentMode==FALSE)
                        MessageBox(hwnd,GetString(IDSTR_PMTOWP_SUCCESS),GetString(IDSTR_PMTOWP_CAPTION),(MB_OK|MB_ICONINFORMATION));
                     break;

                  case FALSE:
                     // display message so user knows that migration failed.
                     if (fSilentMode==FALSE)
                        MessageBox(hwnd,GetString(IDSTR_PMTOWP_FAILURE),GetString(IDSTR_PMTOWP_CAPTION),(MB_OK|MB_ICONSTOP));
                     break;
               }

               // disable pushbuttons while migrating PROGMAN group(s).
               EnableWindow(GetDlgItem(hwnd,IDCANCEL),TRUE);
               EnableWindow(GetDlgItem(hwnd,IDOK),TRUE);
               // if we are in silent mode, exit program.
               if (fSilentMode==TRUE)
                  PostMessage(hwnd,WM_CLOSE,0,0);
               break;

            case IDCANCEL:
               PostMessage(hwnd,WM_CLOSE,0,0);
               break;
         }
         break;

      case WM_CLOSE:
         // destroy window.
         EndDialog(hwnd,0);
         break;
   }

   // default return value.
   return (FALSE);
}



// EOF PMTOWP.CPP


