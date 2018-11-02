//=====================================================================
// VER041.H
//    WPSHELL Profile Data Structures for version 0.4.1.
//
// Date Created :  09/13/94
// Last Update  :  09/13/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/13/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPOBJECT
typedef struct {
   DWORD    m_dwType;
   char     m_szTitle[MAX_TITLE_NAME+1];
   POINT    m_ptIcon;
   char     m_szIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wIconIndex;
   WORD     m_wState;
   DWORD    m_dwFlags;
} FILE_OBJECT041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPCONTAINER
typedef struct {
   int      m_nObjects;
} FILE_CONTAINER041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPWINDOW
typedef struct {
   POINT    m_ptWindow;
   SIZE     m_sizeWindow;
   BOOL     m_fScrollable;
} FILE_WINDOW041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPFOLDER
typedef struct {
   // folder background color.
   COLORREF    m_rgbBackground;
   LOGFONT     m_lfIconText;
   COLORREF    m_clrrefIconTextColor;
   WORD        m_wOpenView;
} FILE_FOLDER041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPPROGRAM
typedef struct {
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];
      char     m_szParameters[MAX_PARAMETERS_LINE+1];
      char     m_szWorkingDir[MAX_WORKING_DIR+1];
} FILE_PROGRAM041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPDATAFILE
typedef struct {         
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1];
      char     m_szDataPath[MAX_DATAFILE_PATH+1];
      char     m_szDataName[MAX_DATAFILE_NAME+1];
} FILE_DATAFILE041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPTEMPLATE
typedef struct {
   // template type information.
   DWORD    m_dwTemplateType;
   // template's object instance title string.
   char     m_szInstanceTitle[MAX_TITLE_NAME+1];
   // template object instance icon information.
   char     m_szInstanceIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wInstanceIconIndex;
} FILE_TEMPLATE041_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSHRDDER
typedef struct {
   // The original 0.4.1 version had no fields.
} FILE_SHREDDER041_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECTS FOR VERSION 0.4.1
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FOLDER OBJECT
typedef struct {
   FILE_OBJECT041_STRUCT      m_obj;
   FILE_WINDOW041_STRUCT      m_wnd;
   FILE_FOLDER041_STRUCT      m_fol;
   FILE_CONTAINER041_STRUCT   m_con;
} FILE_WPS_FOLDER041_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// PROGRAM OBJECT
typedef struct {
   FILE_OBJECT041_STRUCT      m_obj;
   FILE_PROGRAM041_STRUCT     m_prg;
} FILE_WPS_PROGRAM041_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DATAFILE OBJECT
typedef struct {
   FILE_OBJECT041_STRUCT      m_obj;
   FILE_DATAFILE041_STRUCT    m_dat;
} FILE_WPS_DATAFILE041_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TEMPLATE OBJECT
typedef struct {
   FILE_OBJECT041_STRUCT      m_obj;
   FILE_TEMPLATE041_STRUCT    m_tmp;
} FILE_WPS_TEMPLATE041_STRUCT;
//---------------------------------------------------------------------


