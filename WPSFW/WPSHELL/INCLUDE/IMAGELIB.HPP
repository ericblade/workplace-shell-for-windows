//=====================================================================
// IMAGELIB.HPP
//    DIB/BMP Image Library for Windows and OS/2 PM Formats Header File.
//
// Date Created :  12/07/94
// Last Update  :  12/07/94
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 12/07/94    | Creation.                               | MDA
//=====================================================================

#ifndef __IMAGELIB_PROTOTYPED__

#define __IMAGELIB_PROTOTYPED__


//---------------------------------------------------------------------
// LOCAL STRUCTURES
//---------------------------------------------------------------------
typedef BYTE huge    *HPBYTE;


//---------------------------------------------------------------------
// FUNCTION PROTOTYPES
//---------------------------------------------------------------------

DWORD    GetDIBInfoHeaderSize(HPBYTE lpDIB);
WORD     GetDIBWidth(HPBYTE lpDIB);
WORD     GetDIBHeight(HPBYTE lpDIB);
DWORD    GetDIBNumberColors(HPBYTE lpDIB);
HPBYTE   GetDIBBitsAddr(HPBYTE lpDIB);

HPBYTE   LoadImage(char *pszImagePath);
HPALETTE CreatePaletteFromDIB(HPBYTE lpDIB);
HBITMAP  ConvertDIBToDDB(HPBYTE lpDIB,HPALETTE hPalette);
void     DestroyImage(HPBYTE lpDIB);

#endif

// EOF IMAGELIB.HPP

