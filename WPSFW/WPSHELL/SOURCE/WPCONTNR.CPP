//=====================================================================
// WPCONTNR.CPP
//    WPCONTAINER Class implementation.
// 
// Date Created :  01/25/94
// Last Update  :  12/09/95
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.52
// 
// Copyright IBM Corporation, 1994-1995. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 01/25/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <io.h>

//---------------------------------------------------------------------
// LOCAL CONTANTS
//---------------------------------------------------------------------
#define MAX_FNAME_DIGITS   4

//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS 
//---------------------------------------------------------------------
// From WPDESKTOP.CPP file:
BOOL ReadWPObject(int hProfile,WPCONTAINER *pwpcontainerParent);
// From MISCFUNC.CPP file:
BOOL CreateDirectory(char *pszDirectory);


//---------------------------------------------------------------------
// WORKPLACE SHELL CONTAINER CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S 
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// OBJECT COMPONENTS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// SetObject():
//    This function is available only to derived classes as a method 
// to quickly manage the array of object components stored in the 
// container class.  It is mainly used by sort functions when swapping
// objects in the array.
//---------------------------------------------------------------------
void WPCONTAINER::SetObject(WPOBJECT *pwpobjSet,int nPos)
{    
   // Copy passed object pointer into array of object components.
   m_ObjectList.SetAt(nPos,(DWORD)(LPVOID)pwpobjSet);
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// WPCONTAINER():
//    Constructor for class with no parameters.
//---------------------------------------------------------------------
WPCONTAINER::WPCONTAINER(void) : WPOBJECT()
{ 
   // Initialize class data with default values.
   SetType(OBJTYPE_CONTAINER);
   m_nCurrentSelectedObject = 0;         
}                 

                 
//---------------------------------------------------------------------
// WPCONTAINER():
//    Constructor for class with a parameter containing another object
// of the same class which holds the default values for new object.
//---------------------------------------------------------------------
WPCONTAINER::WPCONTAINER(WPCONTAINER *pwpcontainerDefaults)   
   : WPOBJECT((WPOBJECT *)pwpcontainerDefaults)
{
   // Initialize class data with default values.
   SetType(OBJTYPE_CONTAINER);
   m_nCurrentSelectedObject = 0;         
}                 

//---------------------------------------------------------------------
// ~WPCONTAINER():
//    Destructor for class.
//---------------------------------------------------------------------
WPCONTAINER::~WPCONTAINER(void) 
{     
   // do nothing.
}


//---------------------------------------------------------------------
// OBJECT INITIALIZATION FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Initialize():
//    This function gets called only once during the desktop 
// initialization and its called by its parent conteiner object so that
// this object can resume the state in which it was saved.
//    We then call all our children objects so they can do the same.
//---------------------------------------------------------------------
void WPCONTAINER::Initialize(BOOL fIsShell)
{  
   int index;
      
   // If container object was originally open...         
   if (isOpen()==TRUE)
   {
      // Reset "opened" state bit (since we are not really open yet).
      SetState((GetState()&(~OBJSTATE_OPEN)));
      // Restore container object to open state.
      Open();               
   }

   // Intialize all of the container's children objects...
   for (index=0; index<GetObjectCount(); index++)
      GetObject(index)->Initialize(fIsShell);
}   


//---------------------------------------------------------------------
// OBJECT COMPONENTS MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// AddObject():
//    This function appends or inserts a new object into the container
// object list.  If no parameter is given for "nPos" it defaults to -1
// which means append to end of list.
//---------------------------------------------------------------------
BOOL WPCONTAINER::AddObject(WPOBJECT *pwpobjAdd,int nPos)
{  
   // If no valid position index is passed,...
   if (nPos<0)
      // Assume default and add new object to the end of the list.
      m_ObjectList.AddTail((DWORD)(LPVOID)pwpobjAdd);
   else
      return (FALSE);
/* TBD      
   else
      // Else, insert new object before 
      m_ObjectList->AddAt((DWORD)(LPVOID)pwpobjAdd,nPos);
*/   
   // Set new objects's parent to this container object.
   pwpobjAdd->SetParent(this);      
      
   // Successfully added object to list.
   return (TRUE);
}


//---------------------------------------------------------------------
// DeleteObject():
//    This function deletes a specific object from the array of objects
// stored in this class.  
//---------------------------------------------------------------------
BOOL WPCONTAINER::DeleteObject(WPOBJECT *pwpobjDelete)
{  
   // Remove object from selected object list (if currently in it).                    
   m_SelectedObjectList.Remove((DWORD)(LPVOID)pwpobjDelete);      
   // Remove object from list of objects.
   m_ObjectList.Remove((DWORD)(LPVOID)pwpobjDelete);                            
   // Return success.
   return (TRUE);
}
                 

//---------------------------------------------------------------------
// DeleteObject():        
//    This function deletes an object stored in ordinal position "nPos"
// from container array.
//---------------------------------------------------------------------
BOOL WPCONTAINER::DeleteObject(int nPos)
{     
   // Remove object from selected object list (if currently in it).                    
   m_SelectedObjectList.Remove((DWORD)(LPVOID)GetObject(nPos));      
   // Remove object at specified oridinal position in list of objects.
   m_ObjectList.RemoveAt(nPos);                            
   // Return success.
   return (TRUE);
}


//---------------------------------------------------------------------
// OBJECT COMPONENTS QUERY FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// GetObjectCount():
//    This function returns the number of objects currently stored
// in this container object.  if 0, there are no objects contained.
//---------------------------------------------------------------------
int WPCONTAINER::GetObjectCount(void)
{
   return (m_ObjectList.GetCount());
}


//---------------------------------------------------------------------
// GetObject():
//    This function returns a pointer to the object stored at a 
// the specific index "nPos".  If "nPos" is not within the currently
// valid range of indexes, this function will return NULL.
//---------------------------------------------------------------------
WPOBJECT *WPCONTAINER::GetObject(int nPos)
{     
   return ((WPOBJECT *)(LPVOID)m_ObjectList.GetAt(nPos));
}


//---------------------------------------------------------------------
// GetObjectIndex():
//    This function returns the index (or position) in this container 
// object of the specified object "pwpobjQuery".  If the passed object
// does not exists in this container object, this function retuns -1.
//---------------------------------------------------------------------
int WPCONTAINER::GetObjectIndex(WPOBJECT *pwpobjQuery)
{
   return (m_ObjectList.Find((DWORD)(LPVOID)pwpobjQuery));
}


//---------------------------------------------------------------------
// OBJECT SELECTION MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ClearSelectedObjects():
//---------------------------------------------------------------------
void WPCONTAINER::ClearSelectedObjects(BOOL fInvalidateObjects)
{     
   int      index;
    
   // If we need to invalidate selected object's icons...                   
   if (fInvalidateObjects==TRUE)
   {              
      // Loop through the whole list of selected objects. 
      for (index=0; index<GetSelectedObjectCount(); index++)
         // Invalidate selected object for update later.
         GetSelectedObject(index)->InvalidateIcon(FALSE,TRUE);
   }
   // Remove all objects in list of selected objects.
   m_SelectedObjectList.RemoveAll();
}


//---------------------------------------------------------------------
// GetSelectedObjectCount():
//---------------------------------------------------------------------
int WPCONTAINER::GetSelectedObjectCount(void)
{
   return (m_SelectedObjectList.GetCount());
}

//---------------------------------------------------------------------
// GetSelectedObject():
//---------------------------------------------------------------------
WPOBJECT *WPCONTAINER::GetSelectedObject(int nPos)
{
   return ((WPOBJECT *)(LPVOID)m_SelectedObjectList.GetAt(nPos));
}


               
//---------------------------------------------------------------------
// AddSelectedObject():
//---------------------------------------------------------------------
BOOL WPCONTAINER::AddSelectedObject(WPOBJECT *pwpobjAdd)
{                   
   // Check if object is already selected.
   if (isObjSelected(pwpobjAdd)==TRUE)
      return (TRUE);    // Return that it is already selected.
   
   // Add object to selected object list.
   return (m_SelectedObjectList.AddTail((DWORD)(LPVOID)pwpobjAdd));
}

//---------------------------------------------------------------------
// GetFirstSelectedObject():
//---------------------------------------------------------------------
WPOBJECT *WPCONTAINER::GetFirstSelectedObject(void)
{                                                 
   // Reset current selected object index to the begining.
   m_nCurrentSelectedObject = 0;
   // Check if we have at least 1 selected object.
   if (GetSelectedObjectCount()>0)
      return (GetSelectedObject(0));  // Return first object in list.
   else
      return (NULL);                  // Return error.
}

//---------------------------------------------------------------------
// GetNextSelectedObject():
//---------------------------------------------------------------------
WPOBJECT *WPCONTAINER::GetNextSelectedObject(void)
{                   
   // Check if we have enough selected objects for next index.                                          
   if ((m_nCurrentSelectedObject+1)<GetSelectedObjectCount())
      return (GetSelectedObject(++m_nCurrentSelectedObject));
   else
      return (NULL);       // Return error.
}

//---------------------------------------------------------------------
// isObjSelected():
//---------------------------------------------------------------------
BOOL WPCONTAINER::isObjSelected(WPOBJECT *pwpobjTest)
{  
   // If object being tested is found in current list, then return TRUE.
   return (m_SelectedObjectList.Find((DWORD)(LPVOID)pwpobjTest)>=0);
}
              

//---------------------------------------------------------------------
// OBJECT STATE FUNCTIONS
//---------------------------------------------------------------------
              
//---------------------------------------------------------------------
// isAnyChildOpen():
//    Returns TRUE if object is "open" or any child objects (if 
// applicable) are in "open" state.  It returns FALSE otherwise.  Since 
// this is a virtual function, it can be overloaded by derived objects 
// which can actually have child objects.
//---------------------------------------------------------------------
BOOL WPCONTAINER::isAnyChildOpen(void)
{                              
   int index;
                    
   // First check if this directory folder is currently open...                    
   if (isOpen()==TRUE)                                         
      return (TRUE);
         
   // Loop through all children objects and test if any is currently open...         
   for (index=0; index<GetObjectCount(); index++)
      if (GetObject(index)->isAnyChildOpen()==TRUE)
         return (TRUE);
         
   // No children objects are currently opened, return FALSE.   
   return (FALSE);
}

              

//---------------------------------------------------------------------
// FIND OBJECT FUNCTION
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Find():
//    This function finds an object or list of object(s) that match a
// specific search criteria specified by the function parameters.
// It can search for any object type and title text combination.  The
// object's title text can be searched for exactly or any occurrence
// of a substring.
//
// Parameters:
//    dwType      : Specifies the type of object to search.
//    pszTitle    : Object's title text string.
//    fExact      : Match title text exactly (TRUE) or 
//                  Match title text approximatedly (substring) (FALSE)
//    fStopOnMatch: Stop search on first match (default TRUE).
//    ppMatchList : Pointer to pointer to head of match list.
//                  This value is ignored if "fStopOnMatch" == TRUE.
//                                           
// Return value:               
// "fStopOnMatch" == TRUE:
//    Pointer to object found to match search criteria or NULL if no
// objects matching criteria are found.
// "fStopOnMatch" == FALSE:
//    Return value is undefined and must be ignored.
//---------------------------------------------------------------------
WPOBJECT *WPCONTAINER::Find(DWORD dwType,const char *pszTitle,BOOL fExact,BOOL fStopOnMatch,MATCH_RECORD **ppMatchList)
{          
   int      index;
   WPOBJECT *pwpobjFound;
                                                 
   // First, test if this object is meets criteria.
   pwpobjFound = WPOBJECT::Find(dwType,pszTitle,fExact,fStopOnMatch,ppMatchList);
   // If this container object matched criteria, then return object pointer.
   if ((fStopOnMatch==TRUE)&&(pwpobjFound!=NULL))
      return (this);
   
   // Search all child components of container object.                                              
   for (index=0; index<GetObjectCount(); index++)
   {     
      // Have child object test criteria.
      pwpobjFound = GetObject(index)->Find(dwType,pszTitle,fExact,fStopOnMatch,ppMatchList);
      // If object found while searching child component, then return object pointer.
      if ((fStopOnMatch==TRUE)&&(pwpobjFound!=NULL))
         return (pwpobjFound);
   }         
   // No matches found, return "none".   
   return (NULL);
}


//---------------------------------------------------------------------
// DRAG & DROP PROPERTY MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// isContainer(): 
//    Any object derived from the container class is, by default, a 
// container.
//---------------------------------------------------------------------
BOOL WPCONTAINER::isContainer(void)
{
   // By default, an object is a non-container unless function is overloaded.
   return (TRUE);
}

//---------------------------------------------------------------------
// TestDropSource():
//---------------------------------------------------------------------
BOOL WPCONTAINER::TestDropSource(WPOBJECT *pwpobjDropTarget,int nFunction)
{
   // Check if object class accepts being dropped on an object "pwpobjDropTarget".
   if (WPOBJECT::TestDropSource(pwpobjDropTarget,nFunction)==TRUE)
   {
      // Check if any container class has any problems with being a drop source
      // on target object "pwpobjDropTarget" with function "nFunction".
      switch (nFunction)
      {    
         case DROP_MOVE:
         case DROP_COPY:           
         case DROP_SHADOW:
            // Check if target object supports "drag & drop" and function.
            return (pwpobjDropTarget->TestDropTarget(this,nFunction));

         default:
            // No other functions supported by container class.
            return (FALSE);
      }
   }
   else     
      // Object class does not support drop target and/or function.
      return (FALSE);
}


//---------------------------------------------------------------------
// TestDropTarget():
//---------------------------------------------------------------------
BOOL WPCONTAINER::TestDropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{                  
   WPCONTAINER *pwpContainer;

   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (WPOBJECT::TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {
      // Test if "this" container object can receive "pwpobjDropSource" as a dropped object.
      switch (nFunction)
      {  
         case DROP_MOVE:
         case DROP_COPY:              
         case DROP_SHADOW:
            // We must check if target object is a descendant of source object.
            pwpContainer = this;
            // Check that we have not reached the root.
            while (pwpContainer!=NULL)
            {                             
               // Check recursively if we are a descendant of the source object.
               if (pwpContainer==pwpobjDropSource)
                  return (FALSE);
               // Keep moving up the parent ladder.
               pwpContainer = pwpContainer->GetParent();
            }  
            return (TRUE);
            
         default:
            // No other functions supported by container class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}


//---------------------------------------------------------------------
// DropTarget():
//    This function is called when this container object is acting as
// a drop target for a Drag & Drop operation.  The first paremeter
// "pwpobjDropSource" is the object in that is being "dropped" into
// this container.
// The second parameter, "nFunction", is the function code for the 
// Drag & Drop operation the user chose to do (move, copy, etc..).
//---------------------------------------------------------------------
BOOL WPCONTAINER::DropTarget(WPOBJECT *pwpobjDropSource,int nFunction)
{
   // Check if object class accepts object "pwpobjDropSource" being dropped on it.
   if (TestDropTarget(pwpobjDropSource,nFunction)==TRUE)
   {    
      // Depending on which drop operation is being performed...
      switch (nFunction)
      {
         case DROP_MOVE:   
         case DROP_COPY:
         case DROP_SHADOW:
            // Add, copy, or shadow new object in container object.  
            AddObject(pwpobjDropSource);
            // Update container object to show newly added object.
            pwpobjDropSource->InvalidateIcon(TRUE,TRUE);
            return (TRUE);

         default:          
            // No other functions supported by container class.
            return (FALSE);
      }
   }
   else
      return (FALSE);
}
   

//---------------------------------------------------------------------
// OBJECT TO FILE SYSTEM FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CopyObjectToFileSystem():
//---------------------------------------------------------------------
BOOL WPCONTAINER::CopyObjectToFileSystem(char *pszDestPath)
{  
   int   index;
   char  szDirName[_MAX_PATH+1];
               
   // Create destination directory name fully qualified.       
   sprintf(szDirName,"%s\\%s",pszDestPath,MakeFATName((char *)GetTitleNoCRLF()));
   // Create directory in file system.
   if (CreateDirectory(szDirName)==TRUE)
   {                        
      // Copy all children objects to file system... 
      for (index=0; index<GetObjectCount(); index++)
         GetObject(index)->CopyObjectToFileSystem(szDirName);
      // Successfully copied all objects to file system.
      return (TRUE);
   }  
   
   // Failed to create directory or copy children objects.
   return (FALSE);
}

//---------------------------------------------------------------------
// MakeFATName():
//---------------------------------------------------------------------
char *WPCONTAINER::MakeFATName(char *pszString,BOOL fMakeUnique,char *pszPath)
{                    
   int         nCount,nDigits; 
   char        szTempName[_MAX_PATH+1],szExt[_MAX_EXT+1],*pszExtension;
   static char szFATName[_MAX_FNAME+_MAX_EXT+1];
   DWORD       dwIndex;
   
   // Initialize local variables.
   nCount = 0;
   memset(szFATName,0,sizeof(szFATName));
   memset(szExt,0,sizeof(szExt));
   // Copy the first 8 valid characters...
   for (dwIndex=0; dwIndex<(DWORD)strlen(pszString); dwIndex++)
   {              
      // If current character is alpha numeric... 
      if (IsCharAlphaNumeric(pszString[dwIndex])!=FALSE)
      {
         // Copy character to FAT name string.
         szFATName[nCount++] = pszString[dwIndex];         
         // Check if we already have enough characters for FAT name (w/o extension)...
         if (nCount>=_MAX_FNAME) 
            // Stop copying characters.
            break;
      }
   }         
   
   // Null-terminate FAT-compatible name.
   szFATName[nCount] = '\0';         

   // If no valid characters were found in parameter string...
   if (nCount==0)
   {     
      // Make up file name.
      strcpy(szFATName,"NEW");
      fMakeUnique = TRUE;
   }
      
   // Find extension (if any) in the original parameter string.
   pszExtension = strchr(pszString,'.');
   if (pszExtension!=NULL)
   {     
      // Copy the extension (if any) into local variable.           
      dwIndex=0;
      while ((IsCharAlphaNumeric(*pszExtension)!=FALSE) && (dwIndex<=3))
         szExt[dwIndex++] = *pszExtension;
   }
         
   // Check whether we need to use a unique number in the name of the file...
   if (fMakeUnique==TRUE)
   {     
      // Loop through file names with different number of digits...
      for (nDigits=1; nDigits<MAX_FNAME_DIGITS; nDigits++)
      {  
         // Truncate the name to the maximum allowed from current digit count.
         szFATName[7-nDigits] = '\0';
         // Loop through all the possible values for the current number of digits...
         for (dwIndex=0; dwIndex<(DWORD)(0x0FL*nDigits); dwIndex++)
         {           
            // Make unique file name from current index.
            // NOTE: A file name like "THISISMY.TXT" will become "THIS~000.TXT" with nDigit=3...
            sprintf(szTempName,"%s\\%s~%0*X%s",pszPath,szFATName,nDigits,(int)dwIndex,szExt);
            // Check if file already exists...
            if (access(szTempName,0)!=0)
            {        
               // Assume that file does not exist since "access()" function failed.
               // Therefore, make a "path-less" copy of the file name and return it to caller.
               sprintf(szTempName,"%s~%0*X%s",szFATName,nDigits,(int)dwIndex,szExt);
               strcpy(szFATName,szTempName);
               return (szFATName);
            }
         }
      }
   }

   // Return FAT-compatible name.   
   return (szFATName);
}



//---------------------------------------------------------------------
// SHELL PROFILE MANAGEMENT FUNCTIONS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ReadObjectProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPCONTAINER::ReadObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{  
   int                     index;              
   FILE_CONTAINER_STRUCT   fcsData;

   // Read container extra information.
   if (read(hProfile,&fcsData,sizeof(FILE_CONTAINER_STRUCT))!=sizeof(FILE_CONTAINER_STRUCT)) 
      return (FALSE);
   
   // Depending on how many children components this container has...
   for (index=0; index<fcsData.m_nObjects; index++)
   {
      // Read new child object.
      if (ReadWPObject(hProfile,(WPCONTAINER *)this)==FALSE) 
         return (FALSE);
   }

   return (TRUE);
}


//---------------------------------------------------------------------
// WriteObjectProfile():
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
BOOL WPCONTAINER::WriteObjectProfile(int hProfile,WPCONTAINER *pwpcontainerParent)
{
   int                     index;
   FILE_CONTAINER_STRUCT   fcsData;

   // Save the number of child objects in this container object.                                          
   fcsData.m_nObjects = GetObjectCount();            
            
   // Read container extra information.
   if (write(hProfile,&fcsData,sizeof(FILE_CONTAINER_STRUCT))!=sizeof(FILE_CONTAINER_STRUCT)) 
      return (FALSE);
   
   // Depending on how many children components this container has...
   for (index=0; index<GetObjectCount(); index++)
   {
      // Write each child object.
      if (GetObject(index)->WriteObjectProfile(hProfile,(WPCONTAINER *)this)==FALSE) 
         return (FALSE);
   }

   return (TRUE);
}

// EOF WPCONTNR.CPP

