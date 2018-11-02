# Microsoft Visual C++ generated build script - Do not modify

PROJ = WPS_PORT
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\WPSFW\WPSHELL\NLS\PORT\BUILD\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = WPS_ENUS.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /Zp1 /W3 /Zi /ALw /Od /D "_DEBUG" /GD /Fd"WARPICON.PDB"
CFLAGS_R_WDLL = /nologo /f- /G3 /Zp1 /W3 /Gf /ALw /O1 /Ob2 /D "NDEBUG" /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE 
LIBS_D_WDLL = oldnames libw ldllcew 
LIBS_R_WDLL = oldnames libw ldllcew 
RCFLAGS = /nologo /i"..\resources" 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = WPS_PORT.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = WPS_MAIN.SBR


WPS_PORT_RCDEP = d:\wpsfw\wpshell\include\resource.h \
	d:\wpsfw\wpshell\resource\icons\system\wpsfwin.ico \
	d:\wpsfw\wpshell\nls\port\resource\menus\\menus.rc \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\prodinfo.dlg \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\find.dlg \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\copymove.dlg \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\findicon.dlg \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\defpage.dlg \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\settnb.dlg \
	d:\wpsfw\wpshell\include\fclspbw.h \
	d:\wpsfw\wpshell\nls\port\resource\dialogbx\\run.dlg \
	d:\wpsfw\wpshell\nls\port\resource\strings\\strtable.rc


WPS_MAIN_DEP = 

all:	$(PROJ).DLL

WPS_PORT.RES:	..\RESOURCE\WPS_PORT.RC $(WPS_PORT_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r -FoD:\WPSFW\WPSHELL\NLS\PORT\BUILD\WPS_PORT.RES ..\RESOURCE\WPS_PORT.RC

WPS_MAIN.OBJ:	..\..\SOURCE\WPS_MAIN.C $(WPS_MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SOURCE\WPS_MAIN.C


$(PROJ).DLL::	WPS_PORT.RES

$(PROJ).DLL::	WPS_MAIN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
WPS_MAIN.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) WPS_PORT.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	WPS_PORT.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) WPS_PORT.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
