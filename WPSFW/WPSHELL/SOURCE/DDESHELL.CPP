//=====================================================================
// DDESHELL.CPP
//    Windows DDE PROGMAN Service Server implementation.
//
// Date Created :  09/01/94
// Last Update  :  03/21/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/01/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <dos.h>
// Windows library header files.
#include <ddeml.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL DATA TYPES
//---------------------------------------------------------------------
// ICONPROPS structure is defined in the Windows Shell DDE interface documentation
// but not defined in the DDE.H or DDEML.H header files.  Therefore, we must
// define it locally in order to use it.  Used for "GetIcon" item string in
// "AppProperties" topic in "Shell" DDE service (WinOldApp "DOS" applications).
#pragma pack(1)

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif   /* __cplusplus */

typedef struct tagICONPROPS {    /* ip */
/*
   WORD     reserved:12,      // The commented out fields are not needed
            fResponse:1,      // since DDEDATA structure already supplies
            fRelease:1,       // them when creating a HDDEDATA.
            reserved2:1,
            fAckReq:1;

   int      cfFormat;
*/
   int      nWidth;
   int      nHeight;
   BYTE     nPlanes;
   BYTE     nBitsPixel;
   WORD     wANDOffset;
   WORD     wANDSelector;     // Selector = 0
   WORD     wXOROffset;
   WORD     wXORSelector;     // Selector = 0
   BYTE     nValue[1];
} ICONPROPS;

typedef struct {
   POINT    ptHotSpot;              // Unused in icon header (usually=16,16).
   int      nWidth,nHeight;         // Width and height of icon in pixels.
   WORD     wBytesPerRow;           // Bytes used per row in monochrome bitmap (usually 4 bytes).
   BYTE     bBitsPixel,bPlanes;     // Bits per pixel and number of planes in color bitmap.
} ICONHEADER;

#ifdef __cplusplus
}
#endif   /* __cplusplus */

#pragma pack()


//---------------------------------------------------------------------
// LOCAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
HDDEDATA CALLBACK _loadds SHELLServiceCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2);


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define RGB_ICON_TRANSPARENT_COLOR     RGB(0x80,0x00,0x80)  // Dark Magenta

//---------------------------------------------------------------------
// STATIC VARIABLES
//---------------------------------------------------------------------
// NOTE: This variable contains the pointer to the same object WPDESKTOP
// class contains for DDE server for SHELL service.  This "local"
// global variable helps us call the right "current" SHELL server
// when a transaction comes into our DDE callback function.
static DDESHELL   *gs_pddeserverSHELL = NULL;


//---------------------------------------------------------------------
// CLASS FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Initialize():
//---------------------------------------------------------------------
BOOL DDESHELL::Initialize(WPDESKTOP *pwpWPSDesktop)
{
   BOOL  fReturnValue;

   // Store pointer to ourseleves (to be used by DDE callback fucntion
   // to reference this class instantiation).
   gs_pddeserverSHELL = this;

   // Store the pointer to the Workplace Shell desktop object in our private data.
   m_pwpWPSDesktop = pwpWPSDesktop;

   // Check if WPS for Windows is the current Windows shell...
   if ((m_pwpWPSDesktop->IsShell()==TRUE)&&
       (!(GetPrivateProfileInt(GetString(IDSTR_WPSFWIN_SECTION_NAME),"DEBUG_DOSDDEFlags",0x0000,GetString(IDSTR_WIN_SYSTEM_PROFILE))&0x0001)))
      // Call base class initialization function.
      fReturnValue = DDESERVER::Initialize("SHELL",(FARPROC)SHELLServiceCallback);
   else
      // Call base class initialization function.
      fReturnValue = DDESERVER::Initialize("WPS_SHELL",(FARPROC)SHELLServiceCallback);

   // Register all the supported "topic" strings for this DDE server.
   AddTopic("APPPROPERTIES");

   // Return value from initialize function.
   return (fReturnValue);
}

//---------------------------------------------------------------------
// SetProgramInstanceAndObject():
//---------------------------------------------------------------------
void DDESHELL::SetProgramInstanceAndObject(HINSTANCE hInstance,WPPROGRAM *pwpprogOwner)
{
   // Store the instance handle and owner object pointer of program just
   // executed by a Workplace Shell program object.
   m_hInstProgram = hInstance;
   m_pwpprogOwner = pwpprogOwner;
}


//---------------------------------------------------------------------
// DDE CALLBACK FOUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DdeCallback():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
HDDEDATA DDESHELL::DdeCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2)
{
   // Depending on what type of DDE transaction...
   if (uType==XTYP_DISCONNECT)
   {
      #ifdef _DEBUG
      dprintf("tr XTYP_DISCONNECT: fSameInst = %lu\n",(DWORD)dwData2);
      #endif
      // Post message to desktop window to reset this object's minimized position.
      PostMessage(g_wpEnvironment.hwndDesktop,WM_WPS_WINDOW_UPDATE,(WPARAM)m_hInstProgram,0L);
      // Reset DOS DDE instance and object pointer values.
      SetProgramInstanceAndObject(NULL,NULL);
   }

   return (DDESERVER::DdeCallback(uType,uFmt,hConv,hsz1,hsz2,hData,dwData1,dwData2));
}


//---------------------------------------------------------------------
// ServiceRequest():
//    This function services transaction requests that are posted to
// the DDE callback function through the XTYP_REQUEST and XTYP_ADVREQ
// transaction types.
//---------------------------------------------------------------------
HDDEDATA DDESHELL::ServiceRequest(HSZ hszTopic,HSZ hszItem)
{
   HDDEDATA    hData;
   WPPROGRAM   *pwpprogWinOldApp;
   ICONPROPS   *pipData;
   HICON       hWinOldAppIcon;
   BYTE        *pIconHeader;
   char        *pszData;
   int         nSize,nANDMaskSize,nBitsPixel,nIconSize,i;

   #ifdef _DEBUG
   dprintf("tr DDESHELL::ServiceRequest():\n");
   #endif

   // If requested the application properties of a WinOldApp...
   if (CompareHszToStr(hszTopic,"APPPROPERTIES")==TRUE)
   {
      // Find owner object of WinOldApp instance requested (if any).
      // ORIGINAL: pwpprogWinOldApp = (WPPROGRAM *)m_pwpWPSDesktop->GetObjectFromInstanceHandle((HINSTANCE)0);
      pwpprogWinOldApp = m_pwpprogOwner;
      // Check that pointer to owner object is valid.
      if (pwpprogWinOldApp==NULL)
         // Return "no data available for WinOldApp instance".
         return ((HDDEDATA)NULL);

      // Create an empty data object that will be filled-in later.
      hData = DdeCreateDataHandle(m_dwID,NULL,0L,0L,hszItem,CF_TEXT,0);
      // If item requested is the description of "WinOldApp"
      if (CompareHszToStr(hszItem,"GETDESCRIPTION")==TRUE)
      {
         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetDescription\n");
         #endif
         // Calculate the amount of space needed to hold request data.
         nSize = strlen(pwpprogWinOldApp->GetTitleNoCRLF())+1;
         // Allocate enough space for DOS application's title string.
         pszData = new char[nSize];
         // Copy DOS application's title into temporary space.
         strcpy(pszData,pwpprogWinOldApp->GetTitleNoCRLF());

         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetDescription = '%s'\n",pszData);
         #endif

         // Add DDE data and title string to DDE data handle.
         hData = DdeAddData(hData,(LPVOID)pszData,nSize,0L);

         // Deallocate temporary buffers used to build DDE data structures.
         delete pszData;
      }
      else
      if (CompareHszToStr(hszItem,"GETWORKINGDIR")==TRUE)
      {
         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetWorkingDir\n");
         #endif
         // Calculate the amount of space needed to hold request data.
         nSize = strlen(pwpprogWinOldApp->GetWorkingDir())+1;
         // Allocate enough space for DDE data and application title string.
         pszData = new char[nSize];
         // Copy DOS application's working directory string.
         strcpy(pszData,pwpprogWinOldApp->GetWorkingDir());

         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetWorkingDir = '%s'\n",pszData);
         #endif

         // Add working directory string to DDE data handle.
         hData = DdeAddData(hData,(LPVOID)pszData,nSize,0L);

         // Deallocate temporary buffers used to build DDE data structures.
         delete pszData;
      }
      else
      if (CompareHszToStr(hszItem,"GETICON")==TRUE)
      {
         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetIcon\n");
         #endif

         // Load icon from DOS application.
         hWinOldAppIcon = pwpprogWinOldApp->ExtractIcon();

         // Lock memory address of icon header and data for access.
         pIconHeader = (BYTE *)GlobalLock(hWinOldAppIcon);

         // Calculate the amount of space needed to hold icon data.
         nANDMaskSize = (GetSystemMetrics(SM_CXICON)*GetSystemMetrics(SM_CYICON))/8;
         nBitsPixel   = ((ICONHEADER *)pIconHeader)->bBitsPixel;
         nIconSize    = nANDMaskSize*(nBitsPixel+1)*((ICONHEADER *)pIconHeader)->bPlanes;

         // Calculate the amount of space needed to hold request data.
         nSize = (sizeof(ICONPROPS) + nIconSize - 1);

         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetIcon size = %d\n",nSize);
         #endif

         // Allocate enough space for DDE data and application title string.
         pipData = (ICONPROPS *)new BYTE[nSize];
         // Initialize allocated buffer to zeros.
         memset((LPVOID)pipData,0,nSize);
         // Initialize DOS application's icon information.
         pipData->nWidth       = GetSystemMetrics(SM_CXICON);
         pipData->nHeight      = GetSystemMetrics(SM_CYICON);
         pipData->nPlanes      = ((ICONHEADER *)pIconHeader)->bPlanes;
         pipData->nBitsPixel   = ((ICONHEADER *)pIconHeader)->bBitsPixel;
         pipData->wANDOffset   = FIELDOFFSET(ICONPROPS,nValue);
         pipData->wXOROffset   = pipData->wANDOffset+nANDMaskSize;

         // Check that calculated icon size is not greater than the real icon size.
         if (nIconSize > (i = (UINT)GlobalSize(hWinOldAppIcon) - sizeof(ICONHEADER)))
            nIconSize = i;

         // Copy icon AND & XOR bitmap information.
         hmemcpy((void huge *)pipData->nValue,
                 (const void huge *)(pIconHeader+sizeof(ICONHEADER)),nIconSize);

         // Unlock memory where icon resource is stored.
         GlobalUnlock(hWinOldAppIcon);

         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): %ux%u  Planes = %u  BitsPixel = %u\n",pipData->nWidth,pipData->nHeight,pipData->nPlanes,pipData->nBitsPixel);
         dprintf("tr DDESHELL::ServiceRequest(): AND Offset = 0x%04X   XOR Offset = 0x%04X\n",pipData->wANDOffset,pipData->wXOROffset);
         #endif

         // Destroy icon from DOS application.
         DestroyIcon(hWinOldAppIcon);

         // Adjust structure offsets to 4 bytes extra.
         pipData->wANDOffset += sizeof(DWORD);
         pipData->wXOROffset += sizeof(DWORD);

         // Add DDE data and icon properties to DDE data handle.
         hData = DdeAddData(hData,(LPVOID)pipData,nSize,0L);

         #ifdef _DEBUG
         dprintf("tr DDESHELL::ServiceRequest(): GetIcon  AND = 0x%04X   XOR = 0x%04X\n",
            pipData->wANDOffset,pipData->wXOROffset);
         #endif

         // Deallocate temporary buffers used to build DDE data structures.
         delete pipData;
      }
      else
      {
         // Free data handle created previously.
         DdeFreeDataHandle(hData);
         // Return "error - no data available for WinOldApp instance".
         return ((HDDEDATA)NULL);
      }

      // Return data handle containing requested data.
      return (hData);
   }
   else
      // ...Else, topic is not supported, therefore return nothing.
      return ((HDDEDATA)NULL);
}

//---------------------------------------------------------------------
// ExecuteCommand():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL DDESHELL::ExecuteCommand(char *pszName,char *pszParameters)
{
   // No commands supported by server, return failure executing command.
   return (FALSE);
}

//---------------------------------------------------------------------
// Uninitialize():
//---------------------------------------------------------------------
BOOL DDESHELL::Uninitialize(void)
{
   BOOL  fReturnValue;

   // Call base class uninitialization function.
   fReturnValue = DDESERVER::Uninitialize();

   // Clean up the pointer to ourseleves (to be used by DDE callback
   // fucntion to reference this class instantiation).
   gs_pddeserverSHELL = NULL;

   // Return value from uninitialize function.
   return (fReturnValue);
}

//---------------------------------------------------------------------
// SHELLServiceCallback():
//---------------------------------------------------------------------
HDDEDATA CALLBACK _loadds SHELLServiceCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2)
{
   // If dde server object for "SHELL" service is available...
   if (gs_pddeserverSHELL!=NULL)
      return (gs_pddeserverSHELL->DdeCallback(uType,uFmt,hConv,hsz1,hsz2,hData,dwData1,dwData2));
   else
      // Return false by default.
      return (FALSE);
}


// EOF DDESHELL.CPP

