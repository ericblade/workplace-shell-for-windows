# Microsoft Visual C++ generated build script - Do not modify

PROJ = LAUNCHPD
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\WPSFW\LAUNCHPD\BUILD\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = LAUNCHPD.CPP
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /WX /Zi /AL /Od /D "_DEBUG" /GA /Fd"LAUNCHPD.PDB"
CFLAGS_R_WEXE = /nologo /f- /G3 /W3 /WX /AL /O1 /Ob2 /D "NDEBUG" /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib shell.lib
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib shell.lib
RCFLAGS = /nologo /i"..\resource"
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = LAUNCHPD.DEF
OBJS_EXT = 
LIBS_EXT = ..\..\WPSHELL\BUILD\WPSHELL.LIB CTL3D.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = LAUNCHPD.SBR


WPSHELL_DEP = 

CTL3D_DEP = 

LAUNCHPD_DEP = d:\wpsfw\wpshell\include\wps_api.h


LAUNCHPD_RCDEP = d:\wpsfw\launchpd\resource\icons\main\fntpnl.ico \
	d:\wpsfw\launchpd\resource\icons\sysfunc\lpshutdn.ico \
	d:\wpsfw\launchpd\resource\icons\sysfunc\lockup.ico \
	d:\wpsfw\launchpd\resource\icons\sysfunc\statevw.ico \
	d:\wpsfw\launchpd\resource\icons\cursor\nosign.ico \
	d:\wpsfw\launchpd\resource\icons\cursor\copygrid.ico \
	d:\wpsfw\launchpd\resource\icons\cursor\shadow.ico \
	d:\wpsfw\launchpd\resource\cursors\wait.cur


all:	$(PROJ).EXE

LAUNCHPD.OBJ:	..\SOURCE\LAUNCHPD.CPP $(LAUNCHPD_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c ..\SOURCE\LAUNCHPD.CPP

LAUNCHPD.RES:	..\RESOURCE\LAUNCHPD.RC $(LAUNCHPD_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r -FoD:\WPSFW\LAUNCHPD\BUILD\LAUNCHPD.RES ..\RESOURCE\LAUNCHPD.RC


$(PROJ).EXE::	LAUNCHPD.RES

$(PROJ).EXE::	LAUNCHPD.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
LAUNCHPD.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
..\..\WPSHELL\BUILD\WPSHELL.LIB+
CTL3D.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) LAUNCHPD.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	LAUNCHPD.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) LAUNCHPD.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
