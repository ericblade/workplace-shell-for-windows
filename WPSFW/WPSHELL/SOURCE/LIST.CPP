//=====================================================================
// LIST.CPP
//    LIST Class implementation.
// 
// Date Created :  10/28/94
// Last Update  :  10/28/94
// Written By   :  Mario E. De Armas 
// Compiler Used:  MS Visual C++ 1.5
// 
// Copyright IBM Corporation, 1994. All rights reserved.
// 
// 
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER 
// 10/28/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows 3.1 API header file.
#include <windows.h>           
// Workplace Shell header files.
#include "list.hpp"

//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS 
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LIST CLASS
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// P R O T E C T E D   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// NewNode(): 
//---------------------------------------------------------------------
LIST_NODE *LIST::NewNode(DWORD dwValue)
{
   LIST_NODE   *pNewNode;
    
   // Allocate memory for new list node.     
   pNewNode = new LIST_NODE;
   // Check that memory allocation was successfull...
   if (pNewNode!=NULL)
   {
      // Initialize new list node values.
      pNewNode->m_dwValue   = dwValue;
      pNewNode->m_pNextNode = NULL;
      pNewNode->m_pPrevNode = NULL;      
   }
   // return pointer to new list node (or NULL of memory allocation failed).
   return (pNewNode);
}

   
//---------------------------------------------------------------------
// FreeNode():
//---------------------------------------------------------------------
void LIST::FreeNode(LIST_NODE *pNode)
{     
   // Check that pointer to node is valid.          
   if (pNode!=NULL)
      // Deallocate memory from list node.
      delete pNode;
}


//---------------------------------------------------------------------
// RemoveNode():
//---------------------------------------------------------------------
void LIST::RemoveNode(LIST_NODE *pNode)
{     
   // Check that node is valid...
   if (pNode==NULL)
      return;
   
   // If there is a node before the node to remove...
   if (pNode->m_pPrevNode!=NULL)                     
      // Make previous node's next field point to current node's next field.
      pNode->m_pPrevNode->m_pNextNode = pNode->m_pNextNode;
      
   // If there is a node after the node to remove...   
   if (pNode->m_pNextNode!=NULL)   
      // Make next node's previous field point to current node's previous field.
      pNode->m_pNextNode->m_pPrevNode = pNode->m_pPrevNode;
   
   // If node to remove is the current head of list...
   if (m_pNodeHead==pNode)
      m_pNodeHead = pNode->m_pNextNode;
      
   // If node to remove is the current tail of list...
   if (m_pNodeTail==pNode)
      m_pNodeTail = pNode->m_pPrevNode;
      
   // Decrement node count.
   m_nCount--; 
   
   // free node.
   FreeNode(pNode);
}

//---------------------------------------------------------------------
// FindByValue():
//---------------------------------------------------------------------
LIST_NODE *LIST::FindByValue(DWORD dwValue)
{
   LIST_NODE   *pNode;
   
   // Initialize search by starting from head node.
   pNode = m_pNodeHead; 
   // While current node is valid...
   while (pNode!=NULL)
   {                           
      // Check if current node matches value being searched...
      if (pNode->m_dwValue==dwValue)                          
         // Match found, return pointer to current node.
         return (pNode);         
      // Get next node in list.
      pNode = pNode->m_pNextNode;
   }
   // No matches found, return failure.   
   return (NULL);
}


//---------------------------------------------------------------------
// FindByPosition():
//---------------------------------------------------------------------
LIST_NODE *LIST::FindByPosition(int nPosition)
{
   LIST_NODE   *pNode;
   int         nIndex;
   
   // Initialize search by starting from head node.
   pNode  = m_pNodeHead; 
   nIndex = 0;
   // While current node is valid...
   while ((pNode!=NULL)&&(nIndex<nPosition))
   {     
      // Increment index position counter.
      nIndex++;                      
      // Get next node in list.
      pNode = pNode->m_pNextNode;
   }
   // No matches found, return failure.   
   return (pNode);
}


//---------------------------------------------------------------------
// P U B L I C   M E M B E R   F U N C T I O N S
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LIST():
//---------------------------------------------------------------------
LIST::LIST(void)
{
   m_nCount    = 0;
   m_pNodeHead = NULL;   
   m_pNodeTail = NULL;
}
                
//---------------------------------------------------------------------
// ~LIST():
//---------------------------------------------------------------------
LIST::~LIST(void)
{              
   // Remove all remaining nodes before destroying list object.
   RemoveAll();   
}


//---------------------------------------------------------------------
// GetCount():
//---------------------------------------------------------------------
int LIST::GetCount(void)
{
   return (m_nCount);
}
                     
//---------------------------------------------------------------------
// IsEmpty():
//---------------------------------------------------------------------
int LIST::IsEmpty(void)
{
   return (m_nCount==0);
}
   
//---------------------------------------------------------------------
// GetHead():
//---------------------------------------------------------------------
LIST_NODE *LIST::GetHead(void)
{                
   return (m_pNodeHead);
}
                     
//---------------------------------------------------------------------
// GetTail():
//---------------------------------------------------------------------
LIST_NODE *LIST::GetTail(void)
{                
   return (m_pNodeTail);
}

//---------------------------------------------------------------------
// RemoveHead():
//---------------------------------------------------------------------
LIST_NODE *LIST::RemoveHead(void)
{         
   LIST_NODE *pOldHead;

   // Save pointer to original head node.   
   pOldHead = m_pNodeHead;
   // If a head node exists...
   if (m_pNodeHead!=NULL)
   {  
      // Make the new head be the node after the original head node.
      m_pNodeHead = m_pNodeHead->m_pNextNode;
      // If node being removed was the ONLY node...
      if (GetCount()==1)
         m_pNodeTail = m_pNodeHead;
      // Decrement node count.   
      m_nCount--;
   }     
   // Return pointer to original head node (if any, it could be NULL).
   return (pOldHead);
}
   
   
//---------------------------------------------------------------------
// RemoveTail():
//---------------------------------------------------------------------
LIST_NODE *RemoveTail(void)
{
   return (NULL);
}
   
   
//---------------------------------------------------------------------
// AddHead():
//---------------------------------------------------------------------
BOOL LIST::AddHead(DWORD dwValue)
{
   LIST_NODE   *pNewNode,*pOldHead;
          
   // Create new node and initialize it.          
   pNewNode = NewNode(dwValue);
   // Make sure node was created successfully...
   if (pNewNode==NULL)
      return (FALSE);

   // Save pointer to original head node.   
   pOldHead = m_pNodeHead;
   // Add new node as new head of list.    
   m_pNodeHead = pNewNode;
   // Make new head node point to original head node (next).
   pNewNode->m_pNextNode = pOldHead;
   // If there was an original head node,...
   if (pOldHead!=NULL)
      // Make original head node point back to new head node (previous).
      pOldHead->m_pPrevNode = pNewNode;
   // If there was no original tail node,..      
   if (m_pNodeTail==NULL)
      // Make new node the current tail node also.
      m_pNodeTail = pNewNode;
   // Increment node count by one.
   m_nCount++;   
   
   // Return success.
   return (TRUE);
}
   
//---------------------------------------------------------------------
// AddTail():
//---------------------------------------------------------------------
BOOL LIST::AddTail(DWORD dwValue)
{
   LIST_NODE   *pNewNode,*pOldTail;
          
   // Create new node and initialize it.          
   pNewNode = NewNode(dwValue);
   // Make sure node was created successfully...
   if (pNewNode==NULL)
      return (FALSE);

   // Save pointer to original tail node.   
   pOldTail = m_pNodeTail;
   // Add new node as new tail of list.    
   m_pNodeTail = pNewNode;
   // Make new tail node point to original tail node (previous).
   pNewNode->m_pPrevNode = pOldTail;
   // If there was an original tail node,...
   if (pOldTail!=NULL)
      // Make original tail node point forward to new tail node (next).
      pOldTail->m_pNextNode = pNewNode;
   // If there was no original head node,..      
   if (m_pNodeHead==NULL)
      // Make new node the current head node also.
      m_pNodeHead = pNewNode;
   // Increment node count by one.
   m_nCount++;   
   
   // Return success.
   return (TRUE);
}
   
//---------------------------------------------------------------------
// Remove():
//---------------------------------------------------------------------
void LIST::Remove(DWORD dwValue)
{         
   // Find pointer to list node and then call helper function to remove it.
   RemoveNode(FindByValue(dwValue));
}
   
   
//---------------------------------------------------------------------
// RemoveAll():
//---------------------------------------------------------------------
void LIST::RemoveAll(void)
{         
   LIST_NODE *pDeleteNode;
   
   // Get current head node and store its posinter in local variable.
   pDeleteNode = RemoveHead();
   // While there was a valid node removed from list...
   while (pDeleteNode!=NULL)
   {                                                             
      // Deallocate memory used for node.
      delete pDeleteNode;
      // Remove current head node and store its pointer in local variable.
      pDeleteNode = RemoveHead();
   }
}
       

//---------------------------------------------------------------------
// GetAt():
//---------------------------------------------------------------------
DWORD LIST::GetAt(int nPosition)
{  
   // Call helper function which does the same function.
   return (FindIndex(nPosition));
}


//---------------------------------------------------------------------
// SetAt():
//---------------------------------------------------------------------
void LIST::SetAt(int nPosition,DWORD dwValue)
{
   LIST_NODE *pNode;

   // Find list node which is at specified ordinal position.
   pNode = FindByPosition(nPosition);
   // Check that returned list node is valid...
   if (pNode!=NULL)
      // Modify current list node value with new value passed in.
      pNode->m_dwValue = dwValue;
}
                  
                  
//---------------------------------------------------------------------
// RemoveAt():
//---------------------------------------------------------------------
void LIST::RemoveAt(int nPosition)
{  
   // Remove list node at specified location by first finding the node's
   // structure pointer by using its ordinal position.
   RemoveNode(FindByPosition(nPosition));
}


//---------------------------------------------------------------------
// InsertBefore():
//---------------------------------------------------------------------
// TBD
   
//---------------------------------------------------------------------
// InsertAfter():
//---------------------------------------------------------------------
// TBD
   
//---------------------------------------------------------------------
// Find():
//---------------------------------------------------------------------
int LIST::Find(DWORD dwValue)
{
   LIST_NODE   *pNode;
   int         nIndex;
   
   // Initialize search by starting from head node.
   pNode  = m_pNodeHead; 
   nIndex = 0;
   // While current node is valid...
   while (pNode!=NULL)
   {                           
      // Check if current node matches value being searched...
      if (pNode->m_dwValue==dwValue)                          
         // Match found, return pointer to current node.
         return (nIndex);         
      // Get next node in list.
      pNode = pNode->m_pNextNode;
      // Increment index position counter.
      nIndex++;
   }
   // No matches found, return failure.   
   return (-1);
}

//---------------------------------------------------------------------
// FindIndex():
//---------------------------------------------------------------------
DWORD LIST::FindIndex(int nPosition)
{
   LIST_NODE *pNode;
   
   // Find list node at oridinal position 'nPosition'.
   pNode = FindByPosition(nPosition);
   // Check that list node returned is valid...
   if (pNode!=NULL)                            
      // Return value stored at list node found.
      return (pNode->m_dwValue);
   // No list node found at specified position, return failure.
   return (0L);
}

// EOF LIST.CPP

