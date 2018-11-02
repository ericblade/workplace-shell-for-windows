//=====================================================================
// VER05X.H
//    WPSHELL Profile Data Structures for version 0.5.x.
//
// Date Created :  09/13/94
// Last Update  :  12/15/94
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
   WORD     m_wUniqueID;
   char     m_szTitle[MAX_TITLE_NAME+1];
   POINT    m_ptIcon;
   char     m_szIconPath[MAX_PATH_FILENAME+1];
   WORD     m_wIconIndex;
   WORD     m_wState;
   DWORD    m_dwFlags;
} FILE_OBJECT05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPCONTAINER
typedef struct {
   int      m_nObjects;
} FILE_CONTAINER05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPWINDOW
typedef struct {
   POINT    m_ptWindow;
   SIZE     m_sizeWindow;
   BOOL     m_fScrollable;
} FILE_WINDOW05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPFOLDER
typedef struct {
   // folder background color.
   COLORREF    m_rgbBackground;
   LOGFONT     m_lfIconText;
   COLORREF    m_clrrefIconTextColor;
   WORD        m_wOpenView;
} FILE_FOLDER05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPPROGRAM
typedef struct {
      char     m_szProgramPath[MAX_PROGRAM_PATH+1];
      char     m_szParameters[MAX_PARAMETERS_LINE+1];
      char     m_szWorkingDir[MAX_WORKING_DIR+1];
} FILE_PROGRAM05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WPDATAFILE
typedef struct {         
      char     m_szDataSubject[MAX_DATAFILE_SUBJECT+1];
      char     m_szDataPath[MAX_DATAFILE_PATH+1];
      char     m_szDataName[MAX_DATAFILE_NAME+1];
} FILE_DATAFILE05X_STRUCT;
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
} FILE_TEMPLATE05X_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSHADOW                                 
typedef struct {
   WORD     m_wOriginalUniqueID;
} FILE_SHADOW05X_STRUCT;
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// WORKPLACE SHELL OBJECTS FOR VERSION 0.5.x
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// FOLDER OBJECT
typedef struct {
   FILE_OBJECT05X_STRUCT      m_obj;
   FILE_WINDOW05X_STRUCT      m_wnd;
   FILE_FOLDER05X_STRUCT      m_fol;
   FILE_CONTAINER05X_STRUCT   m_con;
} FILE_WPS_FOLDER05X_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// PROGRAM OBJECT
typedef struct {
   FILE_OBJECT05X_STRUCT      m_obj;
   FILE_PROGRAM05X_STRUCT     m_prg;
} FILE_WPS_PROGRAM05X_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DATAFILE OBJECT
typedef struct {
   FILE_OBJECT05X_STRUCT      m_obj;
   FILE_DATAFILE05X_STRUCT    m_dat;
} FILE_WPS_DATAFILE05X_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TEMPLATE OBJECT
typedef struct {
   FILE_OBJECT05X_STRUCT      m_obj;
   FILE_TEMPLATE05X_STRUCT    m_tmp;
} FILE_WPS_TEMPLATE05X_STRUCT;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SHADOW OBJECT
typedef struct {
   FILE_OBJECT05X_STRUCT      m_obj;
   FILE_SHADOW05X_STRUCT      m_sha;
} FILE_WPS_SHADOW05X_STRUCT;
//---------------------------------------------------------------------

