//=====================================================================
// WPSETNB.HPP
//    WPSETTINGSNB Class declaration.
//
// Date Created :  03/14/94
// Last Update  :  05/01/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 03/14/94    | Creation.                               | MDA
//=====================================================================

#ifndef __WPSETTINGSNB_PROTOTYPED__

#define __WPSETTINGSNB_PROTOTYPED__


//---------------------------------------------------------------------
// CLASS DECLARATION
//---------------------------------------------------------------------
class WPSETTINGSNB : public WPNOTEBOOK
{
   protected:
      // Settings notebook caption title function.
      const char     *GetCaptionTitle(void);

   public:
      WPSETTINGSNB(WPOBJECT *pwpobjOwner);

      // Create settings notebook.
      BOOL           Create(int x,int y,int cx,int cy);

      // Update notebook icon resource and the 'General' page.
      void           UpdateNotebookIconResource(void);

      // Settings notebook page creation.
      void           InsertGeneralPage(FARPAGEPROC fnPage,LPCSTR lpszDialogTemplateName);

      // Window message processing.
      void           OnClose(void);
};

#endif

// EOF WPSETNB.HPP

