//=====================================================================
// WPSHADOW.CPP
//    WPSHADOW Class implementation.
//
// Date Created :  09/06/94
// Last Update  :  12/12/94
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

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <io.h>
// Workplace Shell resource ID's header file.
#include "resource.h"


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPSHADOW():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPSHADOW::WPSHADOW(void) : WPOBJECT()
{
   // Initialize program object.
   SetType(OBJTYPE_SHADOW);
   // Clear pointer to original object.
   SetOriginalUniqueID(0);
   // Clear pointer to original object.
   SetOriginalObject((WPOBJECT *)NULL);
}

//---------------------------------------------------------------------
// WPSHADOW():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPSHADOW::WPSHADOW(WPSHADOW *pwpshadowDefaults) : WPOBJECT((WPOBJECT *)pwpshadowDefaults)
{
   // Initialize program object.
   SetType(OBJTYPE_SHADOW);
   // Clear pointer to original object.
   SetOriginalUniqueID(pwpshadowDefaults->GetOriginalUniqueID());
   // Set pointer to original object.
   SetOriginalObject(pwpshadowDefaults->GetOriginalObject());
}


//---------------------------------------------------------------------
// SHADOW INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Initialize():
//    This function gets called only once during the desktop
// initialization and its called by its parent container object so that
// this object can resume the state in which it was saved.
//    In this case, the shadow objects do not really retain the
// state of their original objects.  The original objects do.
//    Therefore, the shadow objects only handle autostarting program
// objects if contained in a startup folder.
//---------------------------------------------------------------------
void WPSHADOW::Initialize(BOOL fIsShell)
{
   // Only handle initialization of program objects in a startup folder...
   // If shadowed object is a program object AND shadow has a parent container,
   // AND Workplace Shell is the current default Windows shell.
   if (((GetOriginalObject()->GetType()&OBJTYPE_PROGRAM)==OBJTYPE_PROGRAM) &&
       (GetParent()!=NULL) && (fIsShell==TRUE))
   {
      // Check if parent folder has startup property AND "Shift" key is NOT pressed.
      if (((GetParent()->GetFlags()&STARTUP_FLAGS_MASK)==STARTUP_PROPERTY) &&
          ((GetAsyncKeyState(VK_SHIFT)&0x8000)!=0x8000))
         // Open object.
         GetOriginalObject()->Open();
   }
}

//---------------------------------------------------------------------
// ORIGINAL OBJECT MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetOriginalUniqueID():
//---------------------------------------------------------------------
WORD WPSHADOW::GetOriginalUniqueID(void)
{
   // Get unique word ID from data member.
   return (m_wOriginalUniqueID);
}

//---------------------------------------------------------------------
// SetOriginalUniqueID():
//---------------------------------------------------------------------
void WPSHADOW::SetOriginalUniqueID(WORD wUniqueID)
{
   // Store unique word ID into data member.
   m_wOriginalUniqueID = wUniqueID;
}

//---------------------------------------------------------------------
// GetOriginalObject():
//---------------------------------------------------------------------
WPOBJECT *WPSHADOW::GetOriginalObject(void)
{
   return (m_pwpobjOriginal);
}

//---------------------------------------------------------------------
// SetOriginalObject():
//---------------------------------------------------------------------
void WPSHADOW::SetOriginalObject(WPOBJECT *pwpobjOriginal)
{
   m_pwpobjOriginal = pwpobjOriginal;
}


//---------------------------------------------------------------------
// OVERLOADED FUNCTIONS FROM THE WPOBJECT CLASS:
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TITLE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetTitle():
//---------------------------------------------------------------------
void WPSHADOW::SetTitle(const char *pszTitle)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
   {
      // Set the new object title to original object.
      GetOriginalObject()->SetTitle(pszTitle);
      CalcTextRect();
   }
}

//---------------------------------------------------------------------
// GetTitle()
//---------------------------------------------------------------------
const char *WPSHADOW::GetTitle(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Get the original object's title.
      return (GetOriginalObject()->GetTitle());
   else
      return ("");
}

//---------------------------------------------------------------------
// GetTitleNoCRLF():
//---------------------------------------------------------------------
const char *WPSHADOW::GetTitleNoCRLF(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Get the original object's title w/o CR/LF special chars.
      return (GetOriginalObject()->GetTitleNoCRLF());
   else
      return ("");
}

//---------------------------------------------------------------------
// GetCaptionTitle():
//---------------------------------------------------------------------
const char *WPSHADOW::GetCaptionTitle(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Get the original object's caption title.
      return (GetOriginalObject()->GetCaptionTitle());
   else
      return ("");
}

//---------------------------------------------------------------------
// APPEARANCE AND POSITION FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetDefaultIcon():
//---------------------------------------------------------------------
void WPSHADOW::GetDefaultIcon(char *pszIconPath,WORD *pwIconIndex)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      GetOriginalObject()->GetDefaultIcon(pszIconPath,pwIconIndex);
   else
      WPOBJECT::GetDefaultIcon(pszIconPath,pwIconIndex);
}

//---------------------------------------------------------------------
// SetIcon():
//---------------------------------------------------------------------
void WPSHADOW::SetIcon(char *pszIconPath,WORD wIconIndex)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Set the new icon information to original object.
      GetOriginalObject()->SetIcon(pszIconPath,wIconIndex);
}

//---------------------------------------------------------------------
// GetIcon():
//---------------------------------------------------------------------
void WPSHADOW::GetIcon(char *pszIconPath,WORD *pwIconIndex)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Get original object's icon information.
      GetOriginalObject()->GetIcon(pszIconPath,pwIconIndex);
}


//---------------------------------------------------------------------
// OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetState():
//---------------------------------------------------------------------
void WPSHADOW::SetState(WORD wState)
{
   // Set state flags to original object.
   GetOriginalObject()->SetState(wState);
}

//---------------------------------------------------------------------
// GetState():
//---------------------------------------------------------------------
WORD WPSHADOW::GetState(void)
{
   // Get the original object's state flags information.
   return (GetOriginalObject()->GetState());
}

//---------------------------------------------------------------------
// GetStateOR():
//---------------------------------------------------------------------
void WPSHADOW::SetStateOR(WORD wState,WORD wStateMask)
{
   GetOriginalObject()->SetStateOR(wState,wStateMask);
}

//---------------------------------------------------------------------
// SetFlags():
//---------------------------------------------------------------------
void WPSHADOW::SetFlags(DWORD dwFlags)
{
   GetOriginalObject()->SetFlags(dwFlags);
}

//---------------------------------------------------------------------
// GetFlags():
//---------------------------------------------------------------------
DWORD WPSHADOW::GetFlags(void)
{
   return (GetOriginalObject()->GetFlags());
}

//---------------------------------------------------------------------
// SetFlagsOR():
//---------------------------------------------------------------------
void WPSHADOW::SetFlagsOR(DWORD dwFlags,DWORD dwFlagsMask)
{
   GetOriginalObject()->SetFlagsOR(dwFlags,dwFlagsMask);
}

//---------------------------------------------------------------------
// isOpen():
//---------------------------------------------------------------------
BOOL WPSHADOW::isOpen(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Check if original object is in open state.
      return (GetOriginalObject()->isOpen());
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// Open():
//---------------------------------------------------------------------
BOOL WPSHADOW::Open(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Open original object.
      return (GetOriginalObject()->Open());
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// Close():
//---------------------------------------------------------------------
BOOL WPSHADOW::Close()
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Close original object.
      return (GetOriginalObject()->Close());
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// DESTROY OBJECT INSTANCE FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Destroy():
//---------------------------------------------------------------------
BOOL WPSHADOW::Destroy(void)
{
   // Check if we have a valid link to our original object...
   if (GetOriginalObject()!=NULL)
      // Remove this shadow "link" from our original object's shadow list.
      GetOriginalObject()->DeleteShadow(this);

   // Have Workplace Shell desktop remove this object from our parent container.
   SendMessage(g_wpEnvironment.hwndDesktop,WM_WPS_DELETE_OBJECT,0,(LPARAM)this);

   // Successfully destroyed object.
   return (TRUE);
}


//---------------------------------------------------------------------
// POPUP MENU PROPERTIES FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// InitializePopup():
//---------------------------------------------------------------------
void WPSHADOW::InitializePopup(HMENU hmenuPopup)
{
   HMENU hmenuOriginalPopup;

   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Initialize original object's system poup menu first.
      GetOriginalObject()->InitializePopup(hmenuPopup);

   // Disable "Create shadow...", "Create another...", and "Copy..." menu entries (not applicable).
   EnableMenuItem(hmenuPopup,IDM_CREATESHADOW,MF_BYCOMMAND|MF_GRAYED);
   EnableMenuItem(hmenuPopup,IDM_CREATEANOTHER,MF_BYCOMMAND|MF_GRAYED);
   EnableMenuItem(hmenuPopup,IDM_COPY,MF_BYCOMMAND|MF_GRAYED);

   // Create a popup menu for the "Original ->" menu entries.
   hmenuOriginalPopup = CreatePopupMenu();
   // Append menu entries to "Original ->" popup menu.
   AppendMenu(hmenuOriginalPopup,MF_STRING|MF_ENABLED,IDM_ORIGINAL_LOCATE,GetString(IDSTR_SHADOWMENU_ORIGINAL_LOCATE));
   AppendMenu(hmenuOriginalPopup,MF_STRING|MF_ENABLED,IDM_ORIGINAL_COPY,GetString(IDSTR_SHADOWMENU_ORIGINAL_COPY));
   AppendMenu(hmenuOriginalPopup,MF_STRING|MF_ENABLED,IDM_ORIGINAL_DELETE,GetString(IDSTR_SHADOWMENU_ORIGINAL_DELETE));
   // Add menu separator and "Original ->" menu entry to end of object system popup menu.
   AppendMenu(hmenuPopup,MF_SEPARATOR,0,0L);
   AppendMenu(hmenuPopup,MF_POPUP|MF_ENABLED,(UINT)hmenuOriginalPopup,GetString(IDSTR_SHADOWMENU_ORIGINAL));
}

//---------------------------------------------------------------------
// GetMenuName():
//---------------------------------------------------------------------
const char *WPSHADOW::GetMenuName(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Return the name of the menu used by original object.
      return (GetOriginalObject()->GetMenuName());
   else
      return (WPOBJECT::GetMenuName());
}

//---------------------------------------------------------------------
// MenuSelection():
//---------------------------------------------------------------------
void WPSHADOW::MenuSelection(WORD wMenuItem)
{
   switch (wMenuItem)
   {
      case IDM_ORIGINAL_LOCATE:
         // Make original object visible and selected.
         // Open original object's parent (if not already open).
         GetOriginalObject()->GetParent()->Open();
         // Clear all currently selected objects and repaint them.
         GetOriginalObject()->GetParent()->ClearSelectedObjects(TRUE);
         // Make original object the currently selected object in container.
         GetOriginalObject()->GetParent()->AddSelectedObject(GetOriginalObject());
         // Repaint it so that it gets the selected object visual attributes.
         GetOriginalObject()->InvalidateIcon(TRUE,TRUE);
         break;

      case IDM_ORIGINAL_COPY:
         // Set the original object to be the currently selected object in parent folder.
         ((WPFOLDER *)(GetOriginalObject()->GetParent()))->SetObjectPopup(GetOriginalObject());
         // Pass user menu selection to original object's parent folder.
         ((WPFOLDER *)(GetOriginalObject()->GetParent()))->OnCommand(IDM_COPY);
         break;

      case IDM_ORIGINAL_DELETE:
         // Pass user menu selection to original object's function.
         GetOriginalObject()->MenuSelection(IDM_DELETE);
         break;

      case IDM_DELETE:
         // Pass user menu selection to base class function.
         WPOBJECT::MenuSelection(wMenuItem);
         break;

      case IDM_CREATESHADOW:
      case IDM_CREATEANOTHER:
      case IDM_COPY:
         // Ignore these menu selections (just in case).
         break;

      default:
         // Pass user menu selection to original object's function.
         GetOriginalObject()->MenuSelection(wMenuItem);
         break;
   }
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DropDPtoLP():
//    This function is in charge of converting and returning a logical
// point within the object given a device point where the drop
// occurred.  In this case, the shadow object calls the original
// object's member function since it represents a "link" to the
// original object.
//---------------------------------------------------------------------
void WPSHADOW::DropDPtoLP(LPPOINT lpptDevice)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Call original object's drop device point to logical point member function.
      GetOriginalObject()->DropDPtoLP(lpptDevice);
}

//---------------------------------------------------------------------
// isContainer():
//    A shadow object is only a container when the original object is
// a container.
//---------------------------------------------------------------------
BOOL WPSHADOW::isContainer(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      // Return value depends on original object being shadowed.
      return (GetOriginalObject()->isContainer());
   else
      return (WPOBJECT::isContainer());
}

//---------------------------------------------------------------------
// TestDropSource():
//---------------------------------------------------------------------
BOOL WPSHADOW::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // Check if original object class accepts target object "pwpobjDropTarget".
   if (GetOriginalObject()->TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Check if shadow class has any problems with being a drop source
      // on target object "pwpobjDropTarget" with function "nFunction"...
      switch (nFunction)
      {
         case DROP_MOVE:
            // Move function is only supported by shadow class, check with target object...
            return (pwpobjDropTarget->TestDropTarget(GetOriginalObject(),nFunction));

         case DROP_COPY:
         case DROP_SHADOW:
         default:
            // No other functions supported by shadow class.
            return (FALSE);
      }
   }
   else
      // Shadow class does not support the drop target and/or function specified.
      return (FALSE);
}

//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
BOOL WPSHADOW::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if original object accepts object "pwpobjDropSource" being dropped on it.
   return (GetOriginalObject()->TestDropTarget(pwpobjDropSource,nFunction));
}


//---------------------------------------------------------------------
// DropSource():
//    This function is called when this object is acting as a drop
// source for a Drag & Drop operation.  The first parameter,
// "pwpobjDropTarget" is the object this container is being "dropped on".
// The second parameter, "nFunction", is the function code for the
// Drag & Drop operation the user chose to do (move, copy, etc..).
//    The third parameter is the device (or physical) point where this
// container object was dropped on the screen.  We use this point to
// position the final object correctly within the target object
// (if applicable).
//---------------------------------------------------------------------
BOOL WPSHADOW::DropSource(WPOBJECT *pwpobjDropTarget,int nFunction,POINT ptDevice)
{
   // Check if object class accepts being dropped on an object "pwpobjDropTarget".
   if (TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Depending on what drag & drop function...
      switch (nFunction)
      {
         case DROP_COPY:
         case DROP_SHADOW:
            // Return failure since copy and shadow actions are not supported by
            // shadow object.
            return (FALSE);

         case DROP_MOVE:
         default:
            // Call the default action handle for all other actions.
            return (WPOBJECT::DropSource(pwpobjDropTarget,nFunction,ptDevice));
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//---------------------------------------------------------------------
BOOL WPSHADOW::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   return (GetOriginalObject()->DropTarget(pwpobjDropSource,nFunction));
}


//---------------------------------------------------------------------
// SETTINGS NOTEBOOK FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isSettingsNotebookOpen():
//---------------------------------------------------------------------
BOOL WPSHADOW::isSettingsNotebookOpen(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      return (GetOriginalObject()->isSettingsNotebookOpen());
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// OpenSettingsNotebook():
//---------------------------------------------------------------------
BOOL WPSHADOW::OpenSettingsNotebook(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      return (GetOriginalObject()->OpenSettingsNotebook());
   else
      return (FALSE);
}

//---------------------------------------------------------------------
// CloseSettingsNotebook():
//---------------------------------------------------------------------
BOOL WPSHADOW::CloseSettingsNotebook(void)
{
   // If we have a valid pointer to original object...
   if (GetOriginalObject()!=NULL)
      return (GetOriginalObject()->CloseSettingsNotebook());
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// WORKPLACE SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
BOOL WPSHADOW::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_SHADOW_STRUCT   fssData;

   // Read object class information.
   if (WPOBJECT::ReadObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Read shadow class information.
   if (read(hProfile,&fssData,sizeof(FILE_SHADOW_STRUCT))!=sizeof(FILE_SHADOW_STRUCT))
      return (FALSE);

   // Initialize shadow values.
   SetOriginalUniqueID(fssData.m_wOriginalUniqueID);

   // Return success reading shadow object.
   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
BOOL WPSHADOW::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   FILE_SHADOW_STRUCT   fssData;

   // write out object information.
   if (WPOBJECT::WriteObjectProfile(hProfile,pwpcontainerParent)==FALSE)
      return (FALSE);

   // Initialize shadow values.
   fssData.m_wOriginalUniqueID  = GetOriginalObject()->GetUniqueID();

   // Write shadow class information.
   if (write(hProfile,&fssData,sizeof(FILE_SHADOW_STRUCT))!=sizeof(FILE_SHADOW_STRUCT))
      return (FALSE);

   // Return success writing shadow object.
   return (TRUE);
}


// EOF WPSHADOW.CPP


