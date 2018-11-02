//=====================================================================
// IMAGELIB.CPP       
//    DIB/BMP Image Library for Windows and OS/2 PM Formats.
//
// Date Created :  12/07/94
// Last Update  :  12/07/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.5
//
// Copyright IBM Corporation, 1994. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 12/07/94    | Creation.                               | MDA
//=====================================================================

#define STRICT

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Windows library header files.
#include <windows.h>
#include <windowsx.h>
// C-runtime library header files.
#include <io.h>
// Image library header file.
#include "imagelib.hpp"


//---------------------------------------------------------------------
// EXTERNAL FUNCTIONS PROTOTYPES
//---------------------------------------------------------------------
          
//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
#define WIN30_PALETTE_VERSION    0x0300          

//---------------------------------------------------------------------
// FUNCTION DEFINITIONS
//---------------------------------------------------------------------

            
//---------------------------------------------------------------------
// GetDIBInfoHeaderSize():
//---------------------------------------------------------------------
DWORD GetDIBInfoHeaderSize(HPBYTE lpDIB)
{
   return (((BITMAPINFOHEADER *)lpDIB)->biSize);
}
   
//---------------------------------------------------------------------
// GetDIBWidth():
//---------------------------------------------------------------------
WORD GetDIBWidth(HPBYTE lpDIB)
{
   if (GetDIBInfoHeaderSize(lpDIB)==sizeof(BITMAPCOREHEADER))
      return ((WORD)(((BITMAPCOREHEADER *)lpDIB)->bcWidth));
   else
      return ((WORD)(((BITMAPINFOHEADER *)lpDIB)->biWidth));
}

//---------------------------------------------------------------------
// GetDIBHeight():
//---------------------------------------------------------------------
WORD GetDIBHeight(HPBYTE lpDIB)
{
   if (GetDIBInfoHeaderSize(lpDIB)==sizeof(BITMAPCOREHEADER))
      return ((WORD)(((BITMAPCOREHEADER *)lpDIB)->bcHeight));
   else
      return ((WORD)(((BITMAPINFOHEADER *)lpDIB)->biHeight));
}
   
//---------------------------------------------------------------------
// GetDIBNumberColors():
//---------------------------------------------------------------------
DWORD GetDIBNumberColors(HPBYTE lpDIB)
{
   DWORD dwNumColors;
   WORD  wBitCount;
   
   // Check if DIB is Windows or OS/2 PM bitmap...
   if (GetDIBInfoHeaderSize(lpDIB)==sizeof(BITMAPCOREHEADER))
   {                                              
      // DIB is OS/2 PM format bitmap...
      wBitCount = ((BITMAPCOREHEADER *)lpDIB)->bcBitCount;
      if (wBitCount!=24)
         dwNumColors = (1L << wBitCount);
      else
         dwNumColors = 0;
   }  
   else
   {                    
      // DIB is Windows format bitmap...
      wBitCount = ((BITMAPINFOHEADER *)lpDIB)->biBitCount;
      // Check if header is bigger than 36 bytes...
      if (GetDIBInfoHeaderSize(lpDIB) >= 36)
         dwNumColors = ((BITMAPINFOHEADER *)lpDIB)->biClrUsed;
      else
         dwNumColors = 0;
      // If number of colors is zero, then...         
      if (dwNumColors==0)                    
      {
         // Calculate number of colors by using the numbers of bits per color.
         if (wBitCount != 24)
            dwNumColors = (1L << wBitCount);
         else
            dwNumColors = 0;
      }
   }
   // Return number of colors in DIB image.   
   return (dwNumColors);
}


//---------------------------------------------------------------------
// GetDIBBitsAddr():
//---------------------------------------------------------------------
HPBYTE GetDIBBitsAddr(HPBYTE lpDIB)
{
   DWORD dwNumColors,dwColorTableSize;
                                           
   // Query the number of colors used in DIB bitmap.                                           
   dwNumColors = GetDIBNumberColors(lpDIB);
   // Check if DIB is Windows or OS/2 PM bitmap.
   if (GetDIBInfoHeaderSize(lpDIB)==sizeof(BITMAPCOREHEADER))
      dwColorTableSize = (dwNumColors * sizeof(RGBTRIPLE));    // OS/2 PM bitmap.
   else
      dwColorTableSize = (dwNumColors * sizeof(RGBQUAD));      // Windows bitmap.
   // Return address to beginning of bitmap bits.   
   return (lpDIB + GetDIBInfoHeaderSize(lpDIB) + dwColorTableSize);
}


//---------------------------------------------------------------------
// LoadImage():
//---------------------------------------------------------------------
HPBYTE LoadImage(char *pszImagePath)
{          
   BITMAPFILEHEADER  bmfhImage;
   HPBYTE            lpDIB;
   DWORD             dwDIBSize,dwOffset,dwHeaderSize;
   WORD              wDIBRead;
   int               hfileImage;
              
   // Open image (bitmap) file for reading.              
   hfileImage = _lopen(pszImagePath,(OF_READ|OF_SHARE_DENY_WRITE));
   // Check that open operation was successfull.
   if (hfileImage == HFILE_ERROR)
      return (NULL);
   
   // Read bitmap file header and check that read is successfull.
   if (_lread(hfileImage,(LPSTR)&bmfhImage,sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
   {
      // Read failed, therefore close file and exit function.
      _lclose(hfileImage);
      return (NULL);
   }  
      
   // Check that file type is of type "BM" (BitMap).      
   if (bmfhImage.bfType != *(WORD *)"BM")
   {  
      // Check failed, therefore close file and exit function.
      _lclose(hfileImage);
      return (NULL);
   }
      
   // Calculate size of Device-Independent Bitmap (DIB) file.
   dwDIBSize = (filelength(hfileImage) - sizeof(BITMAPFILEHEADER));
                     
   // Allocate enought memory to hold the DIB file in memory. 
   lpDIB = (HPBYTE)GlobalAllocPtr(GMEM_MOVEABLE,dwDIBSize);
   // Check that memory allocation was successfull...   
   if (lpDIB == NULL)
   {                                                 
      // Chec failed, therefore close file and exit function.
      _lclose(hfileImage);
      return (NULL);
   }                
      
   // Initialize file offset variable.
   dwOffset = 0L;
   
   // While we haven't read the whole file in yet...  
   while (dwDIBSize > 0)
   {                   
      // Calculate maximum size of read operation.
      wDIBRead = (WORD)min((DWORD)0xFFFEL,dwDIBSize);
      // Read in a block of data and check that it was successfull...      
      if (_lread(hfileImage,(LPSTR)(lpDIB+dwOffset),wDIBRead) != wDIBRead)
      {
         // Read failed, therefore close file, free memory, and exit function.
         _lclose(hfileImage);
         GlobalFreePtr(lpDIB);
         return (NULL);
      }
                                   
      // Read was successfull, therefore increment loop variables for next read.                                   
      dwDIBSize -= wDIBRead;
      dwOffset  += wDIBRead;
   }
      
   // Close file since we have read the whole file into memory.
   _lclose(hfileImage);
                 
   // Get size of DIB file header.
   dwHeaderSize = GetDIBInfoHeaderSize(lpDIB);
                  
   // Check that header size is supported...                  
   if ((dwHeaderSize < 12) || ((dwHeaderSize > 12) && (dwHeaderSize < 16)))
   {             
      // Check failed, therefore free memory and exit function.   
      GlobalFreePtr(lpDIB);
      return (NULL);
   }                
      
   // We have successfully read Image file in, return pointer to memory image.      
   return (lpDIB);
}
   

//---------------------------------------------------------------------
// CreatePaletteFromDIB():
//---------------------------------------------------------------------
HPALETTE CreatePaletteFromDIB(HPBYTE lpDIB)
{
   LPLOGPALETTE     pLogicalPalette;
   HPALETTE         hLogicalPalette;
   WORD             wPalIndex,wNumColors;
   LPBITMAPINFO     lpbmiDIB;
   LPBITMAPCOREINFO lpbmcDIB;

   // Check that we have a valid pointer to a DIB.
   if (lpDIB==NULL)  
      return (NULL);
   // Cast pointer to DIB 
   lpbmiDIB = (LPBITMAPINFO)lpDIB;
   lpbmcDIB = (LPBITMAPCOREINFO)lpDIB;
   // Query the number of colors used by DIB image.
   wNumColors  = (WORD)GetDIBNumberColors(lpDIB);
   // Check that we have a valid number of colors...
   if (wNumColors > 0)
   {
      // Allocate enough memory to hold logical palette for DIB.
      pLogicalPalette = (LPLOGPALETTE)GlobalAllocPtr((GMEM_MOVEABLE|GMEM_ZEROINIT),
         (sizeof(LOGPALETTE)+(sizeof(PALETTEENTRY)*wNumColors)));
      // Check that memory allocation was successfull...
      if (pLogicalPalette==NULL)                        
         // Memory allocation failed, return failure.
         return (NULL);
      // Initialize logical palette structure information.
      pLogicalPalette->palVersion    = WIN30_PALETTE_VERSION;
      pLogicalPalette->palNumEntries = wNumColors;
      // Loop for each color entry in DIB's palette...
      for (wPalIndex=0; wPalIndex<wNumColors; wPalIndex++)
      {
         // Check if DIB is Windows or OS/2 PM bitmap...
         if (GetDIBInfoHeaderSize(lpDIB)==sizeof(BITMAPCOREHEADER))
         {  
            // DIB is OS/2 PM format bitmap, copy palette entry information.
            pLogicalPalette->palPalEntry[wPalIndex].peRed   = lpbmcDIB->bmciColors[wPalIndex].rgbtRed;
            pLogicalPalette->palPalEntry[wPalIndex].peGreen = lpbmcDIB->bmciColors[wPalIndex].rgbtGreen;
            pLogicalPalette->palPalEntry[wPalIndex].peBlue  = lpbmcDIB->bmciColors[wPalIndex].rgbtBlue;
            pLogicalPalette->palPalEntry[wPalIndex].peFlags = 0;
         }
         else
         {                        
            // DIB is Windows format bitmap, copy palette entry information.
            pLogicalPalette->palPalEntry[wPalIndex].peRed   = lpbmiDIB->bmiColors[wPalIndex].rgbRed;
            pLogicalPalette->palPalEntry[wPalIndex].peGreen = lpbmiDIB->bmiColors[wPalIndex].rgbGreen;
            pLogicalPalette->palPalEntry[wPalIndex].peBlue  = lpbmiDIB->bmiColors[wPalIndex].rgbBlue;
            pLogicalPalette->palPalEntry[wPalIndex].peFlags = 0;
         }
      }
      
      // Create logical palette from information in logical palette structure.
      hLogicalPalette = CreatePalette(pLogicalPalette);
      // Free memory allocated for logical palette structure.
      GlobalFreePtr(pLogicalPalette);
   }  
   
   // Return handle to created logical palette.
   return (hLogicalPalette);
}
   
   
//---------------------------------------------------------------------
// ConvertDIBToDDB():
//---------------------------------------------------------------------
HBITMAP ConvertDIBToDDB(HPBYTE lpDIB,HPALETTE hPalette)
{                                                                  
   HPBYTE   lpDIBBits;
   HBITMAP  hBitmap;
   HDC      hdcScreen;
   HPALETTE hpalOld  = NULL;

   if (lpDIB==NULL)
      return (NULL);

   lpDIBBits = GetDIBBitsAddr(lpDIB);
   
   hdcScreen = GetDC (NULL);
   if (hdcScreen==NULL)
      return NULL;

   if (hPalette!=NULL)
      hpalOld = SelectPalette(hdcScreen,hPalette,FALSE);

   RealizePalette(hdcScreen);

   hBitmap = CreateDIBitmap(hdcScreen,(LPBITMAPINFOHEADER) lpDIB,CBM_INIT,lpDIBBits,
                            (LPBITMAPINFO)lpDIB,DIB_RGB_COLORS);

   if (hpalOld!=NULL)
      SelectPalette(hdcScreen,hpalOld,FALSE);

   ReleaseDC(NULL,hdcScreen);

   return (hBitmap);
}
   
   
//---------------------------------------------------------------------
// DestroyImage():
//---------------------------------------------------------------------
void DestroyImage(HPBYTE lpDIB)
{     
   // If image pointer passed is valid...         
   if (lpDIB!=NULL)                      
      // Free image memory from global heap.
      GlobalFreePtr(lpDIB);
}


// EOF IMAGELIB.C

