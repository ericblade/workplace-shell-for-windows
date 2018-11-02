//=====================================================================
// CONVINI.CPP
//    Convert initialization file utility program.
//
// Date Created :  09/08/94
// Last Update  :  12/06/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/08/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 header files.
#include <windows.h>
// Workplace Shell header files.
#include "convini.h"                // Version 1.x data structures and constants.
#include "ver05x.h"                 // Version 0.5.x data structures.
#include "resource.h"               // Program resources header file.
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
#define SZ_APPNAME               "CONVINI"
#define PROFILE_FILENAME         "\\WPSHELL.INI"
#define PROFILE_BACKUP_FILENAME  "\\WPSHELL.V5X"
#define WM_USER_INITIATE         (WM_USER+10)

//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
HINSTANCE   g_hResInst = NULL;
BOOL        fSilentMode = FALSE;
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

BOOL ProfileExists(void);

int  ReadDesktopObjectCount(int hProfile);
void WriteDesktopObjectCount(int hProfile,int nCount);

BOOL WriteProfileHeader(int hProfile);
BOOL CreateDESKTOP(int hProfile,int nCount);
BOOL ConvertOBJECT(int hfileRead,int hfileWrite);
BOOL ConvertSHREDDER(int hfileRead,int hfileWrite);
BOOL ConvertDATAFILE(int hfileRead,int hfileWrite);
BOOL ConvertPROGRAM(int hfileRead,int hfileWrite);
BOOL ConvertFOLDER(int hfileRead,int hfileWrite);
BOOL ConvertTEMPLATE(int hfileRead,int hfileWrite);
BOOL ConvertSHADOW(int hfileRead,int hfileWrite);
BOOL ConvertProfileObjects(int hfileRead,int hfileWrite);

BOOL CALLBACK MainDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);


//---------------------------------------------------------------------
// WinMain():
//---------------------------------------------------------------------
int PASCAL WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpszCmdParam,int nCmdShow)
{
   FARPROC     lpfnMainDlgProc;

   // an instance of CONVINI is already running, exit.
   if (hPrevInstance)
      return (FALSE);

   // check if CONVINI is in silent mode.
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
         MessageBox(NULL,"Error -2: Could not load string table.","Convert Utility",(MB_OK|MB_ICONSTOP));
      // Unregister this application from 3D control library (CTL3D.DLL).
      Ctl3dUnregister(hInstance);
      return (FALSE);
   }

   // create procedure instance callback function.
   lpfnMainDlgProc = MakeProcInstance((FARPROC)MainDlgProc,hInstance);
   // create and call main dialog box.
   DialogBox(g_hResInst,MAKEINTRESOURCE(IDD_MAIN),NULL,lpfnMainDlgProc);
   // free procedure instance callback function.
   FreeProcInstance(lpfnMainDlgProc);

   // Free string table memory.
   FreeStringTable(TOTAL_STRINGS);

   // Clean-up libraries before exiting.
   FreeLibrary(g_hResInst); // NLS resource library.

   // Unregister this application from 3D control library (CTL3D.DLL).
   Ctl3dUnregister(hInstance);

   // return from program (we are done).
   return (0);
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
   sprintf(szNLSLibrary,"CNV_%s.DLL",pszLanguage);
   // Load NLS resource library
   g_hResInst = LoadLibrary(szNLSLibrary);
   // Check that load operation was successfull...
   if (g_hResInst <= HINSTANCE_ERROR)
   {
      // Create error message string.
      sprintf(szErrorMsg,"Error -1: Could not locate and load '%s' NLS resource library.",szNLSLibrary);
      // Could not load NLS resource library, therefore display error message
      // to user since we need it to run and return failure.
      MessageBox(NULL,szErrorMsg,"Convert Utility",(MB_OK|MB_ICONSTOP));
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
   // if we are not in "silent" mode,...
   if (fSilentMode==FALSE)
      // then we display the error message.
      MessageBox(NULL,pszErrorMessage,GetString(IDSTR_ERROR_CAPTION),(MB_OK|MB_ICONSTOP));
}


//---------------------------------------------------------------------
// InitializeDialog():
//---------------------------------------------------------------------
void InitializeDialog(HWND hDlg)
{
   RECT  rcDialog;
   char  szSourceProfile[256];

   // Center dialog box.
   GetWindowRect(hDlg,&rcDialog);
   SetWindowPos(hDlg,NULL,
      (GetSystemMetrics(SM_CXSCREEN)-(rcDialog.right-rcDialog.left))/2,
      (GetSystemMetrics(SM_CYSCREEN)-(rcDialog.bottom-rcDialog.top))/2,
      0,0,(SWP_NOZORDER | SWP_NOSIZE));

   // Initialize entry field.
   GetWindowsDirectory(szSourceProfile,(sizeof(szSourceProfile)-1));
   strcat(szSourceProfile,PROFILE_FILENAME);
   SetDlgItemText(hDlg,IDE_SOURCEPROFILE,szSourceProfile);

   // Check if profile exists and is valid.
   if (ProfileExists()==FALSE)
      // Disable "Convert" pushbutton.
      EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
}

//---------------------------------------------------------------------
// ProfileExists():
//---------------------------------------------------------------------
BOOL ProfileExists(void)
{
   WPSHELL_PROFILE_HEADER  wpprofHeader;
   char                    szWorkingDir[256];
   int                     hfileProfile;

   // Get current Windows 3.1 directory (<drive>:\WINDOWS).
   GetWindowsDirectory(szWorkingDir,(sizeof(szWorkingDir)-1));
   strcat(szWorkingDir,PROFILE_FILENAME);

   // Atttempt to open the ini file for update
   hfileProfile = open(szWorkingDir,(_O_BINARY|_O_RDWR));
   // Check if file does not exists...
   if (hfileProfile==-1)
   {
      ReportError(GetString(IDSTR_ERR_10));
      // Return error reading or opening profile.
      return (FALSE);
   }
   else
   {
      // Read Workplace Shell profile header.
      if (read(hfileProfile,&wpprofHeader,sizeof(WPSHELL_PROFILE_HEADER))!=sizeof(WPSHELL_PROFILE_HEADER))
      {
         ReportError(GetString(IDSTR_ERR_11));
         // Close profile.
         close(hfileProfile);
         // Return error reading or opening profile.
         return (FALSE);
      }

      // Check that profile is not version 0.5.x or above.
      if ((wpprofHeader.m_dwMagicID!=WPSHELL_PROFILE_MAGIC_NUMBER) ||
          ((wpprofHeader.m_wMajorVersion==0x0000) && (wpprofHeader.m_wMinorVersion<=0x0500)) )
      {
         ReportError(GetString(IDSTR_ERR_12));
         // Close profile.
         close(hfileProfile);
         // Return error reading or opening profile.
         return (FALSE);
      }

      // Check that profile is not already version 1.x or above.
      if (wpprofHeader.m_wMajorVersion >= 0x0001)
      {
         ReportError(GetString(IDSTR_ERR_13));
         // Close profile.
         close(hfileProfile);
         // Return error reading or opening profile.
         return (FALSE);
      }
      // Close profile.
      close(hfileProfile);
   }

   // Return success verifying profile.
   return (TRUE);
}


//---------------------------------------------------------------------
// OpenProfiles():
//---------------------------------------------------------------------
BOOL OpenProfiles(int *phfileRead,int *phfileWrite)
{
   WPSHELL_PROFILE_HEADER  wpsProfileHeader;
   char                    szWorkingDir[256],szBackupFile[256];

   // get current Windows 3.1 directory (<drive>:\WINDOWS).
   GetWindowsDirectory(szWorkingDir,sizeof(szWorkingDir)-1);
   // concatenate profile name.
   strcat(szWorkingDir,PROFILE_FILENAME);

   // get current Windows 3.1 directory (<drive>:\WINDOWS).
   GetWindowsDirectory(szBackupFile,sizeof(szBackupFile)-1);
   // concatenate backup name of profile.
   strcat(szBackupFile,PROFILE_BACKUP_FILENAME);

   // rename original file.
   if (rename(szWorkingDir,szBackupFile)!=0)
   {
      ReportError(GetString(IDSTR_ERR_20));
      return (FALSE);
   }

   // atttempt to open the ini file for update
   // create a new WPSHELL.INI file and truncate the original WPSHELL.INI file.
   *phfileWrite = open(szWorkingDir,(_O_BINARY|_O_CREAT|_O_TRUNC|O_RDWR),(_S_IREAD|_S_IWRITE));
   // check that open was successfull.
   if (*phfileWrite==-1)
   {
      ReportError(GetString(IDSTR_ERR_21));
      return (FALSE);
   }
   // write out file header informatrion.
   WriteProfileHeader(*phfileWrite);

   // position file pointer at the end of file (append mode).
   lseek(*phfileWrite,0L,SEEK_END);

   // open original profile for reading.
   *phfileRead = open(szBackupFile,(_O_BINARY|_O_RDONLY));
   // check that open was successfull.
   if (*phfileRead==-1)
   {
      // close new profile.
      close(*phfileWrite);
      // display error message to user.
      ReportError(GetString(IDSTR_ERR_22));
      return (FALSE);
   }

   // read header information as to move file pointer to beginning of data.
   if (read(*phfileRead,&wpsProfileHeader,sizeof(WPSHELL_PROFILE_HEADER))!=sizeof(WPSHELL_PROFILE_HEADER))
   {
      // close new profile.
      close(*phfileWrite);
      // close backup profile.
      close(*phfileRead);
      // display error message to user.
      ReportError(GetString(IDSTR_ERR_23));
      return (FALSE);
   }

   // return handle to profile.
   return (TRUE);
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
// CloseProfiles():
//---------------------------------------------------------------------
BOOL CloseProfiles(int hfileRead,int hfileWrite)
{
   // close profile.
   close(hfileRead);
   // close profile.
   close(hfileWrite);
   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// SetupProfiles():
//---------------------------------------------------------------------
BOOL SetupProfiles(int *phfileRead,int *phfileWrite)
{
   // initialize handles to "no file".
   *phfileRead  = -1;
   *phfileWrite = -1;

   // create a new WPSHELL.INI file and backup the existing one.
   return (OpenProfiles(phfileRead,phfileWrite));
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
BOOL CreateDESKTOP(int hProfile,int nCount)
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
   ffs.m_fol.m_rgbBackground  = GetSysColor(COLOR_BACKGROUND);
   SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&(ffs.m_fol.m_lfIconText),0);
   ffs.m_fol.m_clrrefIconTextColor = RGB(0,0,0);
   ffs.m_fol.m_wOpenView      = VIEW_ICON;
   // container data.
   ffs.m_con.m_nObjects       = nCount;

   // write out folder structure information.
   if (write(hProfile,&ffs,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// ReadObjectType05X():
//---------------------------------------------------------------------
BOOL ReadObjectType05X(int hfileProfile,DWORD *pdwObjType)
{
   // Read in a double word (32-bits) object ID.
   if (read(hfileProfile,pdwObjType,sizeof(DWORD))!=sizeof(DWORD))
      return (FALSE);
   // Move file pointer back to beginning of double word ID (32-bits).
   if (lseek(hfileProfile,-(long)sizeof(DWORD),SEEK_CUR)==-1L)
      return (FALSE);
   // Return success reading object type from 0.4.1 version profile.
   return (TRUE);
}


//---------------------------------------------------------------------
// Convert05X0bjInfoTo10ObjInfo():
//---------------------------------------------------------------------
void Convert05XObjInfoTo10ObjInfo(FILE_OBJECT05X_STRUCT *pObj05X,FILE_OBJECT_STRUCT *pObj)
{
   // Convert 0.5.x object information into 1.0 object information.
   pObj->m_dwType      = pObj05X->m_dwType;
   pObj->m_wUniqueID   = pObj05X->m_wUniqueID;
   strcpy(pObj->m_szTitle,pObj05X->m_szTitle);
   pObj->m_ptIcon      = pObj05X->m_ptIcon;
   strcpy(pObj->m_szIconPath,pObj05X->m_szIconPath);
   pObj->m_wIconIndex  = pObj05X->m_wIconIndex;
   pObj->m_wState      = pObj05X->m_wState;
   pObj->m_dwFlags     = pObj05X->m_dwFlags;
}

//---------------------------------------------------------------------
// ConvertOBJECT():
//---------------------------------------------------------------------
BOOL ConvertOBJECT(int hfileRead,int hfileWrite)
{
   FILE_OBJECT05X_STRUCT   wps05XObject;
   FILE_OBJECT_STRUCT      wpsObject;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XObject,sizeof(FILE_OBJECT05X_STRUCT))!=sizeof(FILE_OBJECT05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&wps05XObject,&wpsObject);

   // Write out new object information with new data structure.
   if (write(hfileWrite,&wpsObject,sizeof(FILE_OBJECT_STRUCT))!=sizeof(FILE_OBJECT_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// ConvertSHRDDER():
//---------------------------------------------------------------------
BOOL ConvertSHREDDER(int hfileRead,int hfileWrite)
{
   FILE_OBJECT05X_STRUCT   wps05XObject;
   FILE_OBJECT_STRUCT      wpsObject;        // Ver 1.0 only.

   // Read all information from old profile.
   if (read(hfileRead,&wps05XObject,sizeof(FILE_OBJECT05X_STRUCT))!=sizeof(FILE_OBJECT05X_STRUCT))
      return (FALSE);

   // NOTE: In versions 0.5.x and above, the shredder object does not contain extra information.
   // Read empty shredder information from old profile (need to keep alignment).
   // if (read(hfileRead,&wps05XShredder,sizeof(FILE_SHREDDER05X_STRUCT))!=sizeof(FILE_SHREDDER05X_STRUCT))
   //   return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&wps05XObject,&wpsObject);

   // Write out new object information with new data structure.
   if (write(hfileWrite,&wpsObject,sizeof(FILE_OBJECT_STRUCT))!=sizeof(FILE_OBJECT_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// ConvertDATAFILE():
//---------------------------------------------------------------------
BOOL ConvertDATAFILE(int hfileRead,int hfileWrite)
{
   FILE_WPS_DATAFILE05X_STRUCT  wps05XDatafile;
   FILE_WPS_DATAFILE_STRUCT     wpsDatafile;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XDatafile,sizeof(FILE_WPS_DATAFILE05X_STRUCT))!=sizeof(FILE_WPS_DATAFILE05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&(wps05XDatafile.m_obj),&(wpsDatafile.m_obj));
   wpsDatafile.m_dat = *(FILE_DATAFILE_STRUCT *)&(wps05XDatafile.m_dat);

   // Write out new datafile information with new data structure.
   if (write(hfileWrite,&wpsDatafile,sizeof(FILE_WPS_DATAFILE_STRUCT))!=sizeof(FILE_WPS_DATAFILE_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// ConvertPROGRAM():
//---------------------------------------------------------------------
BOOL ConvertPROGRAM(int hfileRead,int hfileWrite)
{
   FILE_WPS_PROGRAM05X_STRUCT  wps05XProgram;
   FILE_WPS_PROGRAM_STRUCT     wpsProgram;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XProgram,sizeof(FILE_WPS_PROGRAM05X_STRUCT))!=sizeof(FILE_WPS_PROGRAM05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&(wps05XProgram.m_obj),&(wpsProgram.m_obj));
   wpsProgram.m_prg = *(FILE_PROGRAM_STRUCT *)&(wps05XProgram.m_prg);

   // Write out new program information with new data structure.
   if (write(hfileWrite,&wpsProgram,sizeof(FILE_WPS_PROGRAM_STRUCT))!=sizeof(FILE_WPS_PROGRAM_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// ConvertFOLDER():
//---------------------------------------------------------------------
BOOL ConvertFOLDER(int hfileRead,int hfileWrite)
{
   FILE_WPS_FOLDER05X_STRUCT  wps05XFolder;
   FILE_WPS_FOLDER_STRUCT     wpsFolder;
   int                        index;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XFolder,sizeof(FILE_WPS_FOLDER05X_STRUCT))!=sizeof(FILE_WPS_FOLDER05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&(wps05XFolder.m_obj),&(wpsFolder.m_obj));
   wpsFolder.m_wnd = *(FILE_WINDOW_STRUCT *)&(wps05XFolder.m_wnd);
   // Convert field-by-field folder information.
   wpsFolder.m_fol.m_rgbBackground = wps05XFolder.m_fol.m_rgbBackground;
   wpsFolder.m_fol.m_lfIconText = wps05XFolder.m_fol.m_lfIconText;
   wpsFolder.m_fol.m_clrrefIconTextColor = wps05XFolder.m_fol.m_clrrefIconTextColor;
   strcpy(wpsFolder.m_fol.m_szImagePath,"");       // Extra field in 1.0 format.
   wpsFolder.m_fol.m_wOpenView = wps05XFolder.m_fol.m_wOpenView;
   // convert container information.
   wpsFolder.m_con = *(FILE_CONTAINER_STRUCT *)&(wps05XFolder.m_con);

   // Write out new folder information with new data structure.
   if (write(hfileWrite,&wpsFolder,sizeof(FILE_WPS_FOLDER_STRUCT))!=sizeof(FILE_WPS_FOLDER_STRUCT))
      return (FALSE);

   // Convert any children object that this folder might have...
   for (index=0; index<wpsFolder.m_con.m_nObjects; index++)
   {
      // Make sure that conversion was successfull...
      if (ConvertProfileObjects(hfileRead,hfileWrite)==FALSE)
         return (FALSE);
   }

   // Return success.
   return (TRUE);
}

//---------------------------------------------------------------------
// ConvertTEMPLATE():
//---------------------------------------------------------------------
BOOL ConvertTEMPLATE(int hfileRead,int hfileWrite)
{
   FILE_WPS_TEMPLATE05X_STRUCT  wps05XTemplate;
   FILE_WPS_TEMPLATE_STRUCT     wpsTemplate;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XTemplate,sizeof(FILE_WPS_TEMPLATE05X_STRUCT))!=sizeof(FILE_WPS_TEMPLATE05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&(wps05XTemplate.m_obj),&(wpsTemplate.m_obj));
   wpsTemplate.m_tmp = *(FILE_TEMPLATE_STRUCT *)&(wps05XTemplate.m_tmp);

   // Write out new template information with new data structure.
   if (write(hfileWrite,&wpsTemplate,sizeof(FILE_WPS_TEMPLATE_STRUCT))!=sizeof(FILE_WPS_TEMPLATE_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// ConvertSHADOW():
//---------------------------------------------------------------------
BOOL ConvertSHADOW(int hfileRead,int hfileWrite)
{
   FILE_WPS_SHADOW05X_STRUCT  wps05XShadow;
   FILE_WPS_SHADOW_STRUCT     wpsShadow;

   // Read all information from old profile.
   if (read(hfileRead,&wps05XShadow,sizeof(FILE_WPS_SHADOW05X_STRUCT))!=sizeof(FILE_WPS_SHADOW05X_STRUCT))
      return (FALSE);

   // Copy old information into new profile data structure.
   Convert05XObjInfoTo10ObjInfo(&(wps05XShadow.m_obj),&(wpsShadow.m_obj));
   wpsShadow.m_sha = *(FILE_SHADOW_STRUCT *)&(wps05XShadow.m_sha);

   // Write out new shadow information with new data structure.
   if (write(hfileWrite,&wpsShadow,sizeof(FILE_WPS_SHADOW_STRUCT))!=sizeof(FILE_WPS_SHADOW_STRUCT))
      return (FALSE);

   // Return success.
   return (TRUE);
}




//---------------------------------------------------------------------
// ConvertProfileObjects():
//---------------------------------------------------------------------
BOOL ConvertProfileObjects(int hfileRead,int hfileWrite)
{
   DWORD dwObjType;
   BOOL  fResult;

   // Read next object type from original profile.
   if (ReadObjectType05X(hfileRead,&dwObjType)==FALSE)
      return (FALSE);

   // Depending on the object type read...
   switch (dwObjType)
   {
      case OBJTYPE_SHREDDER:
         fResult = ConvertSHREDDER(hfileRead,hfileWrite);
         break;

      case OBJTYPE_SYSTEMSETUP:
         fResult = ConvertOBJECT(hfileRead,hfileWrite);
         break;

      case OBJTYPE_PROGRAM:
         fResult = ConvertPROGRAM(hfileRead,hfileWrite);
         break;

      case OBJTYPE_DATAFILE:
         fResult = ConvertDATAFILE(hfileRead,hfileWrite);
         break;

      case OBJTYPE_DESKTOP:
      case OBJTYPE_FOLDER:
      case OBJTYPE_MINVIEWER:
         fResult = ConvertFOLDER(hfileRead,hfileWrite);
         break;

      case OBJTYPE_TEMPLATE:
         fResult = ConvertTEMPLATE(hfileRead,hfileWrite);
         break;

      case OBJTYPE_SHADOW:
         fResult = ConvertSHADOW(hfileRead,hfileWrite);
         break;

      default:
         fResult = FALSE;
         ReportError(GetString(IDSTR_ERR_40));
   }

   // Return result of conversion.
   return (fResult);
}


//---------------------------------------------------------------------
// ConvertProfile05XTo10():
//---------------------------------------------------------------------
BOOL ConvertProfile05XTo10(int hfileRead,int hfileWrite)
{
   // Converting the desktop (i.e. folder) object and its children.
   return (ConvertProfileObjects(hfileRead,hfileWrite));
}


//---------------------------------------------------------------------
// OnConvertCommand():
//---------------------------------------------------------------------
BOOL OnConvertCommand(HWND hwnd)
{
   BOOL  fResult;
   int   hfileRead,hfileWrite;

   // Open and setup profiles...
   if (SetupProfiles(&hfileRead,&hfileWrite)==FALSE)
      return (FALSE);

   // Convert old profile into new format.
   fResult = ConvertProfile05XTo10(hfileRead,hfileWrite);

   // Close profile, and increment top-level count of objects.
   CloseProfiles(hfileRead,hfileWrite);

   // Return success.
   return (fResult);
}


//---------------------------------------------------------------------
// MAIN DIALOG PROCEDURE
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// MainDlgProc():
//---------------------------------------------------------------------
BOOL CALLBACK MainDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
   // Process window messages.
   switch(msg)
   {
      case WM_INITDIALOG:
         // Iniialize dialog controls.
         InitializeDialog(hwnd);
         // If we are in silent mode, automatically convert profile.
         if (fSilentMode==TRUE)
            PostMessage(hwnd,WM_COMMAND,(WPARAM)IDOK,0);
         return (TRUE);

      case WM_COMMAND:
         switch (wParam)
         {
            case IDOK:
               // Disable pushbuttons while converting profile.
               EnableWindow(GetDlgItem(hwnd,IDCANCEL),FALSE);
               EnableWindow(GetDlgItem(hwnd,IDOK),FALSE);

               // Convert profile...
               switch (OnConvertCommand(hwnd))
               {
                  case TRUE:
                     // Display message so user knows we have migrated successfully.
                     if (fSilentMode==FALSE)
                        MessageBox(hwnd,GetString(IDSTR_CONVERT_SUCCESS),GetString(IDSTR_CONVERT_CAPTION),(MB_OK|MB_ICONINFORMATION));
                     break;

                  case FALSE:
                     // Display message so user knows that migration failed.
                     if (fSilentMode==FALSE)
                        MessageBox(hwnd,GetString(IDSTR_CONVERT_FAILURE),GetString(IDSTR_ERROR_CAPTION),(MB_OK|MB_ICONSTOP));
                     break;
               }

               // Re-enable pushbuttons again.
               EnableWindow(GetDlgItem(hwnd,IDCANCEL),TRUE);
               EnableWindow(GetDlgItem(hwnd,IDOK),TRUE);
               // If we are in silent mode, exit program.
               if (fSilentMode==TRUE)
                  PostMessage(hwnd,WM_CLOSE,0,0);
               break;

            case IDCANCEL:
               PostMessage(hwnd,WM_CLOSE,0,0);
               break;
         }
         break;

      case WM_CLOSE:
         // Destroy window.
         EndDialog(hwnd,0);
         break;
   }

   // Default return value.
   return (FALSE);
}


// EOF CONVINI.CPP


