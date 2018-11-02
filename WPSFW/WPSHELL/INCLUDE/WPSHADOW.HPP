//=====================================================================
// WPSHADOW.HPP
//    WPSHADOW Class declaration.
//
// Date Created :  09/06/94
// Last Update  :  09/30/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 09/06/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPSHADOW_PROTOTYPED__

#define __WPSHADOW_PROTOTYPED__


//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
//
typedef struct {
   WORD     m_wOriginalUniqueID;
} FILE_SHADOW_STRUCT;


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPSHADOW : public WPOBJECT
{
   private:
      WORD        m_wOriginalUniqueID;    // Original object unique ID.
      WPOBJECT    *m_pwpobjOriginal;      // Original object link.

   public:
      // Shadow object constructors.
      WPSHADOW(void);
      WPSHADOW(WPSHADOW *pwpshadowDefaults);

      // Shadow initialization function.
      void        Initialize(BOOL fIsShell);

      // Original object management functions.
      WORD        GetOriginalUniqueID(void);
      void        SetOriginalUniqueID(WORD wUniqueID);
      WPOBJECT    *GetOriginalObject(void);
      void        SetOriginalObject(WPOBJECT *pwpobjOriginal);

      // Overloaded functions from the WPOBJECT class:

      // Title management functions.
      void        SetTitle(const char *pszTitle);
      const char  *GetTitle(void);
      const char  *GetTitleNoCRLF(void);
      const char  *GetCaptionTitle(void);

      // Appearance and position functions.
      void        GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex);
      void        SetIcon(char *pszIconPath,WORD wIconIndex);
      void        GetIcon(char *pszIconPath,WORD *pwIconIndex);

      // Object state functions.
      void        SetState(WORD wState);
      WORD        GetState(void);
      void        SetStateOR(WORD wState,WORD wStateMask);
      void        SetFlags(DWORD dwFlags);
      DWORD       GetFlags(void);
      void        SetFlagsOR(DWORD dwFlags,DWORD dwFlagsMask);

      BOOL        isOpen(void);
      BOOL        Open(void);
      BOOL        Close(void);

      // Destroy object instance function.
      virtual BOOL  Destroy(void);

      // Popup menu properties.
      void        InitializePopup(HMENU hmenuPopup);
      const char  *GetMenuName(void);
      void        MenuSelection(WORD wMenuItem);

      // Drag & drop property management functions.
      void        DropDPtoLP(LPPOINT lpptDevice);
      BOOL        isContainer(void);
      BOOL        TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction);
      BOOL        TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction);
      BOOL        DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice);
      BOOL        DropTarget(WPOBJECT *pwpobjDropSource,int nFunction);


      // Settings notebook functions.
      BOOL        isSettingsNotebookOpen(void);
      BOOL        OpenSettingsNotebook(void);
      BOOL        CloseSettingsNotebook(void);

      // Shell profile management functions.
      BOOL        ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
      BOOL        WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent);
};

#endif

// EOF WPSHADOW.HPP


