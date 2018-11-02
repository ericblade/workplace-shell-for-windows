# Microsoft Visual C++ generated build script - Do not modify

PROJ = CNV_ENUS
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\WPSFW\CONVINI\NLS\ENUS\BUILD\
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
DEFFILE = CNV_ENUS.DEF
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
SBRS = CNV_MAIN.SBR


CNV_ENUS_RCDEP = d:\wpsfw\convini\include\resource.h \
	d:\wpsfw\convini\nls\enus\resource\strings\\strtable.rc \
	d:\wpsfw\convini\nls\enus\resource\dialogbx\\main.dlg


CNV_MAIN_DEP = 

all:	$(PROJ).DLL

CNV_ENUS.RES:	..\RESOURCE\CNV_ENUS.RC $(CNV_ENUS_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r -FoD:\WPSFW\CONVINI\NLS\ENUS\BUILD\CNV_ENUS.RES ..\RESOURCE\CNV_ENUS.RC

CNV_MAIN.OBJ:	..\..\SOURCE\CNV_MAIN.C $(CNV_MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SOURCE\CNV_MAIN.C


$(PROJ).DLL::	CNV_ENUS.RES

$(PROJ).DLL::	CNV_MAIN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
CNV_MAIN.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) CNV_ENUS.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	CNV_ENUS.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) CNV_ENUS.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
