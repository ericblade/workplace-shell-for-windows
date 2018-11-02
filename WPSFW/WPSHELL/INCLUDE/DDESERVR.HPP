//=====================================================================
// DDESERVR.HPP
//    Windows DDE Server Class.
//
// Date Created :  06/01/94
// Last Update  :  06/30/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 06/01/94    | Creation.                               | MDA
//=====================================================================

#ifndef __DDESERVER_PROTOTYPED__

#define __DDESERVER_PROTOTYPED__


//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
#include <ddeml.h>

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define MAX_DDE_SERVICE_NAME  32
#define MAX_DDE_TOPICS        10
#define MAX_DDE_TOPIC_NAME    32

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class DDESERVER
{
   protected:
      DWORD    m_dwID;
      // ASCII null-terminated strings.
      char     m_szService[MAX_DDE_SERVICE_NAME];
      int      m_nTopicCount;
      char     *m_pszTopics[MAX_DDE_TOPICS];
      // HSZ DDE string handles.
      HSZ      m_hszService;
      // DDE callback procedure instance.
      FARPROC  m_pfnCallback;

   protected:
      // DDE topics functions.
      BOOL           CompareHszToStr(HSZ hsz,char *psz);
      int            GetTopicCount(void);
      BOOL           AddTopic(char *pszTopic);
      BOOL           isTopic(char *pszTestTopic);
                        
      // DDE service processing functions:
      // XTYP_REQUEST or XTYP_ADVREQ
      virtual HDDEDATA  ServiceRequest(HSZ hszTopic,HSZ hszItem) = 0;

      // XTYP_EXECUTE functions.
      BOOL           IsBlank(char *pszString);
      virtual BOOL   GetNextCommand(char *pszBuffer,char *pszCommandString,int *pnPosition);
      virtual BOOL   GetCommandNameAndParameters(char *pszCommandString,char **ppszName,char **ppszParameters);
      virtual BOOL   ExecuteCommand(char *pszName,char *pszParameters) = 0;

   public:
      // Default constructor.
      DDESERVER(void);

      // Initialization function.
      virtual BOOL   Initialize(char *pszService,FARPROC DdeCallback);

      // DDE callback function.
      virtual HDDEDATA DdeCallback(UINT uType,UINT uFmt,HCONV hConv,HSZ hsz1,HSZ hsz2,HDDEDATA hData,DWORD dwData1,DWORD dwData2);

      // Uninitialization function.
      virtual BOOL   Uninitialize(void);
};

#endif

// EOF DDESERVR.HPP

