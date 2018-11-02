#*******************************************************************************
#**  WPSFW.INC                                                                **
#**                                                                           **
#**  IBM Workplace Shell for Windows                                          **
#**  XXXX-XXX (c) Copyright International Business Machines Corp. 1994-1995   **
#**  All Rights Reserved                                                      **
#**  Licensed Materials - Property of IBM                                     **
#**                                                                           **
#**  U.S. Government Users. RESTRICTED RIGHTS -                               **
#**  Use, Duplication, or Disclosure restricted by                            **
#**  GSA APD Schedule Contract with IBM Corporation.                          **
#**                                                                           **
#*******************************************************************************

SRCDRV   = D:
TOOLSDRV = C:
SETUPDIR = $(SRCDRV)\WPSSETUP

# Base directory for the product
BASE        = $(SRCDRV)\wpsfw

# Save original path in this macro
ORGPATH     = $(PATH)

# Logfile for determining build status
LOGFILE     = $(BASE)\build.log

# Set directories for build
CCBIN       = $(TOOLSDRV)\msvc\bin
CCINC       = $(TOOLSDRV)\msvc\include;$(TOOLSDRV)\msvc\include\sys
CCLIB       = $(TOOLSDRV)\msvc\lib

# Set product version information
#VER_MAJOR              = 1
#VER_MINOR              = 52
#VER_STR_PRODUCTVERSION = "\"Release v1.52\0\""

#VERDEF = /d VER_MAJOR=$(VER_MAJOR) /d VER_MINOR=$(VER_MINOR) /d VER_STR_PRODUCTVERSION=$(VER_STR_PRODUCTVERSION)

# EOF WPSFW.INC

