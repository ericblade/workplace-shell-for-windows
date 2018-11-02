# Microsoft Visual C++ generated build script - Do not modify

PROJ = PTW_DUTC
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\WPSFW\PMTOWP\NLS\DUTC\BUILD\
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
DEFFILE = PTW_DUTC.DEF
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
SBRS = PTW_MAIN.SBR


PTW_MAIN_DEP = 

PTW_DUTC_RCDEP = d:\wpsfw\pmtowp\include\resource.h \
	d:\wpsfw\pmtowp\nls\dutc\resource\strings\\strtable.rc \
	d:\wpsfw\pmtowp\nls\dutc\resource\dialogbx\\main.dlg


all:	$(PROJ).DLL

PTW_MAIN.OBJ:	..\..\SOURCE\PTW_MAIN.C $(PTW_MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\..\SOURCE\PTW_MAIN.C

PTW_DUTC.RES:	..\RESOURCE\PTW_DUTC.RC $(PTW_DUTC_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r -FoD:\WPSFW\PMTOWP\NLS\DUTC\BUILD\PTW_DUTC.RES ..\RESOURCE\PTW_DUTC.RC


$(PROJ).DLL::	PTW_DUTC.RES

$(PROJ).DLL::	PTW_MAIN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
PTW_MAIN.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) PTW_DUTC.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	PTW_DUTC.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) PTW_DUTC.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
