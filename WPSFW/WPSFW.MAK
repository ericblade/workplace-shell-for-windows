#*******************************************************************************
#**  WPSFW.MAK                                                                **
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
#**  TRACKING THE BUILD                                                       **
#**                                                                           **
#**     Each step of the build process is written to a file which is specif-  **
#**     ied by the $(LOGFILE) macro.  This macro is defined in PRODUCT.INC.   **
#**                                                                           **
#*******************************************************************************
#**                                                                           **
#** Here are some examples of executing this makefile:                        **
#**                                                                           **
#**  1) To resume a build that has failed.  Correct the offending source      **
#**     module and issue the following:                                       **
#**                                                                           **
#**     NMAKE /f wpsfw.mak                                                    **
#**                                                                           **
#**  2) You can build any component seperatly like this:                      **
#**                                                                           **
#**     NMAKE /f wpsfw.mak wpshell                                            **
#**                                                                           **
#*******************************************************************************
!INCLUDE <wpsfw.inc>

#*******************************************************************************
#** Build the product. (This is the default if no target is specified)        **
#*******************************************************************************
product: convini launchpd os21icon pmtowp winshut warpicon wndlist wpshell \
         done

#*******************************************************************************
#** Delete OBJs and build the product.                                        **
#*******************************************************************************
clean: cleanup product

#*******************************************************************************
#** Delete all OBJ files from BASE directory.                                 **
#*******************************************************************************
cleanup:
   -for %i in (convini pmtowp os21icon warpicon wpshell wndlist) do del %i\build\*.obj
   -for %i in (convini pmtowp os21icon warpicon wpshell wndlist) do del %i\build\*.res
   -for %i in (convini pmtowp os21icon warpicon wpshell wndlist) do del %i\build\*.map
   -for %i in (convini pmtowp winshut wpshell wndlist) do del %i\build\*.exe
   -for %i in (os21icon warpicon) do del %i\build\*.dll
   -for %i in (os21icon warpicon) do del %i\build\*.lib
   -del $(BASE)\$(LOGFILE)


#*******************************************************************************
#** CONVINI Program and NLS Resources.                                        **
#*******************************************************************************
convini: convini.exe cnv_nls
cnv_nls: cnv_enus cnv_dutc cnv_grmn cnv_port cnv_swed cnv_fren cnv_finn cnv_span \
         cnv_ital

#*******************************************************************************
#** LAUNCHPD Program and NLS Resources.                                       **
#*******************************************************************************
launchpd: launchpd.exe lnp_nls
lnp_nls:

#*******************************************************************************
#** OS21ICON Library.                                                         **
#*******************************************************************************
os21icon: os21icon.dll

#*******************************************************************************
#** PMTOWP Program and NLS Resources.                                         **
#*******************************************************************************
pmtowp: pmtowp.exe ptw_nls
ptw_nls: ptw_enus ptw_dutc ptw_grmn ptw_port ptw_swed ptw_fren ptw_finn ptw_span \
         ptw_ital

#*******************************************************************************
#** WINSHUT Program.                                                          **
#*******************************************************************************
winshut: winshut.exe

#*******************************************************************************
#** WARPICON Library.                                                         **
#*******************************************************************************
warpicon: warpicon.dll

#*******************************************************************************
#** WNDLIST Program and NLS Resources.                                        **
#*******************************************************************************
wndlist: wndlist.exe

#*******************************************************************************
#** WPSHELL Program and NLS Resources.                                        **
#*******************************************************************************
wpshell: wpshell.exe wps_nls
wps_nls: wps_enus wps_dutc wps_grmn wps_port wps_swed wps_fren wps_finn wps_span \
         wps_ital

#*******************************************************************************
#** NLS Resources Only.                                                       **
#*******************************************************************************
nls: cnv_nls ptw_nls wps_nls wndlist


#*******************************************************************************
#** CONVINI.EXE Make Code.
#*******************************************************************************
convini.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\build
# Build the project
  @echo Building CONVINI.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f convini.mak

#*******************************************************************************
#** CONVINI NLS Resources Make Code.
#*******************************************************************************
cnv_enus:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\enus\build
# Build the project
  @echo Building CNV_ENUS.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_enus.mak

cnv_dutc:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\dutc\build
# Build the project
  @echo Building CNV_DUTC.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_dutc.mak

cnv_grmn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\grmn\build
# Build the project
  @echo Building CNV_GRMN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_grmn.mak

cnv_port:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\port\build
# Build the project
  @echo Building CNV_PORT.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_port.mak

cnv_swed:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\swed\build
# Build the project
  @echo Building CNV_SWED.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_swed.mak

cnv_fren:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\fren\build
# Build the project
  @echo Building CNV_FREN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_fren.mak

cnv_finn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\finn\build
# Build the project
  @echo Building CNV_FINN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_finn.mak

cnv_span:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\span\build
# Build the project
  @echo Building CNV_SPAN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_span.mak

cnv_ital:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\convini\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\convini\nls\ital\build
# Build the project
  @echo Building CNV_ITAL.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f cnv_ital.mak


#*******************************************************************************
#** LAUNCHPD.EXE Make Code.
#*******************************************************************************
launchpd.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\launchpd\build
# Build the project
  @echo Building LAUNCHPD.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f launchpd.mak


#*******************************************************************************
#** OS21ICON.DLL Make Code.
#*******************************************************************************
os21icon.dll:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\os21icon\build
# Build the project
  @echo Building OS21ICON.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f os21icon.mak


#*******************************************************************************
#** PMTOWP.EXE Make Code.
#*******************************************************************************
pmtowp.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\build
# Build the project
  @echo Building PMTOWP.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f pmtowp.mak

#*******************************************************************************
#** PMTOWP NLS Resources Make Code.
#*******************************************************************************
ptw_enus:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\enus\build
# Build the project
  @echo Building PTW_ENUS.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_enus.mak

ptw_dutc:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\dutc\build
# Build the project
  @echo Building PTW_DUTC.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_dutc.mak

ptw_grmn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\grmn\build
# Build the project
  @echo Building PTW_GRMN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_grmn.mak

ptw_port:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\port\build
# Build the project
  @echo Building PTW_PORT.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_port.mak

ptw_swed:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\swed\build
# Build the project
  @echo Building PTW_SWED.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_swed.mak

ptw_fren:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\fren\build
# Build the project
  @echo Building PTW_FREN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_fren.mak

ptw_finn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\finn\build
# Build the project
  @echo Building PTW_FINN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_finn.mak

ptw_span:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\span\build
# Build the project
  @echo Building PTW_SPAN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_span.mak

ptw_ital:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\pmtowp\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\pmtowp\nls\ital\build
# Build the project
  @echo Building PTW_ITAL.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f ptw_ital.mak


#*******************************************************************************
#** WINSHUT.EXE Make Code.
#*******************************************************************************
winshut.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\winshut\build
# Build the project
  @echo Building WINSHUT.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f winshut.mak

#*******************************************************************************
#** WARPICON.DLL Make Code.
#*******************************************************************************
warpicon.dll:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\warpicon\build
# Build the project
  @echo Building WARPICON.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f warpicon.mak

#*******************************************************************************
#** WNDLIST.EXE Make Code.
#*******************************************************************************
wndlist.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wndlist\build
# Build the project
  @echo Building WNDLIST.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wndlist.mak


#*******************************************************************************
#** WPSHELL.EXE Make Code.
#*******************************************************************************
wpshell.exe:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);..\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\build
# Build the project
#  @echo Building WPSHELL.EXE... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wpshell.mak

#*******************************************************************************
#** WPSHELL NLS Resources Make Code.
#*******************************************************************************
wps_enus:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\enus\build
# Build the project
  @echo Building WPS_ENUS.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_enus.mak

wps_dutc:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\dutc\build
# Build the project
  @echo Building WPS_DUTC.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_dutc.mak

wps_grmn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\grmn\build
# Build the project
  @echo Building WPS_GRMN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_grmn.mak

wps_port:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\port\build
# Build the project
  @echo Building WPS_PORT.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_port.mak

wps_swed:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\swed\build
# Build the project
  @echo Building WPS_SWED.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_swed.mak

wps_fren:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\fren\build
# Build the project
  @echo Building WPS_FREN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_fren.mak

wps_finn:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\finn\build
# Build the project
  @echo Building WPS_FINN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_finn.mak

wps_span:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\span\build
# Build the project
  @echo Building WPS_SPAN.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_span.mak

wps_ital:
# Setup environment variables
  set path=$(CCBIN);$(PATH)
  set include=$(CCINC);$(BASE)\wpshell\include;
  set lib=$(CCLIB);
# Switch to build directory
  cd $(BASE)\wpshell\nls\ital\build
# Build the project
  @echo Building WPS_ITAL.DLL... >>$(LOGFILE)
  $(MAKE) -$(MAKEFLAGS) -f wps_ital.mak


#*******************************************************************************
#** Done building product
#*******************************************************************************
done:
  cd $(BASE)
  @echo Copying new release to \RELEASE directory. >>$(LOGFILE)
  COPYREL.BAT
  @echo Product build has completed successfully. >>$(LOGFILE)

#*******************************************************************************
#** Create product diskettes                                                **
#*******************************************************************************
setup:
  @echo Creating product diskettes... >>$(LOGFILE)
  @cls
  @echo Making diskette images.....
  xcopy $(BASE)\RELEASE\*.* $(SETUPDIR)\*.* /Y
  d:\makewps.bat
  del $(BASE)\SETUP\*.*
  xcopy $(SETUPDIR)\DISK1\*.* $(BASE)\SETUP\*.* /Y
  @echo Done!

# EOF WPSFW.MAK

