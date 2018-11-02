# Microsoft Visual C++ generated build script - Do not modify

PROJ = WPSHELL
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\WPSFW\WPSHELL\BUILD\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = WPOBJECT.CPP
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /WX /Zi /AL /Od /D "_DEBUG" /GA /Fd"WPSHELL.PDB"
CFLAGS_R_WEXE = /nologo /f- /G3 /W3 /WX /Gf /AL /O1 /Ob2 /D "NDEBUG" /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:20480 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:20480 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib ddeml.lib mmsystem.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib ddeml.lib mmsystem.lib shell.lib 
RCFLAGS = /nologo /i"..\resource" 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = WPSHELL.DEF
OBJS_EXT = 
LIBS_EXT = CTL3D.LIB 
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
SBRS = DDEPRGMN.SBR \
		DDESERVR.SBR \
		DDESHELL.SBR \
		DESKTOP.SBR \
		FLDRCOPY.SBR \
		FLDRFIND.SBR \
		FOLDER.SBR \
		LIST.SBR \
		MAIN.SBR \
		MISCFUNC.SBR \
		NOTEBOOK.SBR \
		WPCONTNR.SBR \
		WPDATAFL.SBR \
		WPDESKTP.SBR \
		WPMINWIN.SBR \
		WPNOTEBK.SBR \
		WPOBJECT.SBR \
		WPPROG.SBR \
		WPSETNB.SBR \
		WPSETUP.SBR \
		WPSHADOW.SBR \
		WPSHREDR.SBR \
		WPSYSSET.SBR \
		WPTEMPLT.SBR \
		WPWINDOW.SBR \
		IMAGELIB.SBR \
		WPDRIVE.SBR \
		WPFLDR_1.SBR \
		WPFLDR_2.SBR \
		WPS_API.SBR


CTL3D_DEP = 

WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\system\wpsfwin.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\system\desktop.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\system\minview.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\defaults\object.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\defaults\data.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\defaults\program.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\defaults\folder.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\defaults\shredder.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\template.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\tmpobj.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\tmpprog.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\tmpflder.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\tmpdata.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\template\tmpshrdr.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\drives\drives.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\drives\floppy.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\drives\hdrive.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\syssetup.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\system.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\mouse.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\country.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\keyboard.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\sound.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\install.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\colorsch.ico


WPSHELL_RCDEP = d:\wpsfw\wpshell\resource\icons\setup\fontpal.ico \
	d:\wpsfw\wpshell\resource\icons\misc\migrate\pmfolder.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\os2sys.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\general.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\help.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\info.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\startup.ico \
	d:\wpsfw\wpshell\resource\icons\misc\os2obj\tutorial.ico \
	d:\wpsfw\wpshell\resource\icons\misc\mmedia\mmpm2.ico \
	d:\wpsfw\wpshell\resource\icons\misc\mmedia\cdplayer.ico \
	d:\wpsfw\wpshell\resource\icons\misc\mmedia\digvideo.ico \
	d:\wpsfw\wpshell\resource\icons\misc\dos\dosfull.ico \
	d:\wpsfw\wpshell\resource\icons\misc\dos\doswin.ico \
	d:\wpsfw\wpshell\resource\icons\template\tmpdrive.ico \
	d:\wpsfw\wpshell\resource\icons\drives\cdrom.ico \
	d:\wpsfw\wpshell\resource\icons\drives\netdrive.ico \
	d:\wpsfw\wpshell\resource\icons\drives\ramdrive.ico \
	d:\wpsfw\wpshell\resource\icons\misc\mmedia\mididata.ico \
	d:\wpsfw\wpshell\resource\icons\misc\mmedia\auddata.ico \
	d:\wpsfw\wpshell\resource\icons\cursor\nosign.ico \
	d:\wpsfw\wpshell\resource\icons\cursor\copygrid.ico \
	d:\wpsfw\wpshell\resource\icons\cursor\shadow.ico \
	d:\wpsfw\wpshell\resource\icons\ncwindow\button.ico \
	d:\wpsfw\wpshell\resource\cursors\wait.cur \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1bu_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1mn_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1mx_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1bu_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1mn_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_1mx_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2bu_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2mn_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2mx_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2bu_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2mn_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\hi_2mx_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1bu_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1mn_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1mx_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1bu_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1mn_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_1mx_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2bu_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2mn_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2mx_n.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2bu_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2mn_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\minmax\lo_2mx_x.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_stbup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_stbdn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_slaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_sraup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_sladn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_sradn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_suaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_sdaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_suadn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\hr_sdadn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_stbup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_stbdn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_slaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_sraup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_sladn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_sradn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_suaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_sdaup.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_suadn.bmp \
	d:\wpsfw\wpshell\resource\bitmaps\scrollbr\lr_sdadn.bmp


all:	$(PROJ).EXE

DDEPRGMN.OBJ:	..\SOURCE\DDEPRGMN.CPP $(DDEPRGMN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\DDEPRGMN.CPP

DDESERVR.OBJ:	..\SOURCE\DDESERVR.CPP $(DDESERVR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\DDESERVR.CPP

DDESHELL.OBJ:	..\SOURCE\DDESHELL.CPP $(DDESHELL_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\DDESHELL.CPP

DESKTOP.OBJ:	..\SOURCE\DESKTOP.CPP $(DESKTOP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\DESKTOP.CPP

FLDRCOPY.OBJ:	..\SOURCE\FLDRCOPY.CPP $(FLDRCOPY_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\FLDRCOPY.CPP

FLDRFIND.OBJ:	..\SOURCE\FLDRFIND.CPP $(FLDRFIND_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\FLDRFIND.CPP

FOLDER.OBJ:	..\SOURCE\FOLDER.CPP $(FOLDER_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\FOLDER.CPP

LIST.OBJ:	..\SOURCE\LIST.CPP $(LIST_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\LIST.CPP

MAIN.OBJ:	..\SOURCE\MAIN.CPP $(MAIN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\MAIN.CPP

MISCFUNC.OBJ:	..\SOURCE\MISCFUNC.CPP $(MISCFUNC_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\MISCFUNC.CPP

NOTEBOOK.OBJ:	..\SOURCE\NOTEBOOK.CPP $(NOTEBOOK_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\NOTEBOOK.CPP

WPCONTNR.OBJ:	..\SOURCE\WPCONTNR.CPP $(WPCONTNR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPCONTNR.CPP

WPDATAFL.OBJ:	..\SOURCE\WPDATAFL.CPP $(WPDATAFL_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPDATAFL.CPP

WPDESKTP.OBJ:	..\SOURCE\WPDESKTP.CPP $(WPDESKTP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPDESKTP.CPP

WPMINWIN.OBJ:	..\SOURCE\WPMINWIN.CPP $(WPMINWIN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPMINWIN.CPP

WPNOTEBK.OBJ:	..\SOURCE\WPNOTEBK.CPP $(WPNOTEBK_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPNOTEBK.CPP

WPOBJECT.OBJ:	..\SOURCE\WPOBJECT.CPP $(WPOBJECT_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c ..\SOURCE\WPOBJECT.CPP

WPPROG.OBJ:	..\SOURCE\WPPROG.CPP $(WPPROG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPPROG.CPP

WPSETNB.OBJ:	..\SOURCE\WPSETNB.CPP $(WPSETNB_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPSETNB.CPP

WPSETUP.OBJ:	..\SOURCE\WPSETUP.CPP $(WPSETUP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPSETUP.CPP

WPSHADOW.OBJ:	..\SOURCE\WPSHADOW.CPP $(WPSHADOW_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPSHADOW.CPP

WPSHREDR.OBJ:	..\SOURCE\WPSHREDR.CPP $(WPSHREDR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPSHREDR.CPP

WPSYSSET.OBJ:	..\SOURCE\WPSYSSET.CPP $(WPSYSSET_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPSYSSET.CPP

WPTEMPLT.OBJ:	..\SOURCE\WPTEMPLT.CPP $(WPTEMPLT_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPTEMPLT.CPP

WPWINDOW.OBJ:	..\SOURCE\WPWINDOW.CPP $(WPWINDOW_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPWINDOW.CPP

WPSHELL.RES:	..\RESOURCE\WPSHELL.RC $(WPSHELL_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r -FoD:\WPSFW\WPSHELL\BUILD\WPSHELL.RES ..\RESOURCE\WPSHELL.RC

IMAGELIB.OBJ:	..\SOURCE\IMAGELIB.CPP $(IMAGELIB_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\IMAGELIB.CPP

WPDRIVE.OBJ:	..\SOURCE\WPDRIVE.CPP $(WPDRIVE_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPDRIVE.CPP

WPFLDR_1.OBJ:	..\SOURCE\WPFLDR_1.CPP $(WPFLDR_1_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPFLDR_1.CPP

WPFLDR_2.OBJ:	..\SOURCE\WPFLDR_2.CPP $(WPFLDR_2_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPFLDR_2.CPP

WPS_API.OBJ:	..\SOURCE\WPS_API.CPP $(WPS_API_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\SOURCE\WPS_API.CPP


$(PROJ).EXE::	WPSHELL.RES

$(PROJ).EXE::	DDEPRGMN.OBJ DDESERVR.OBJ DDESHELL.OBJ DESKTOP.OBJ FLDRCOPY.OBJ FLDRFIND.OBJ \
	FOLDER.OBJ LIST.OBJ MAIN.OBJ MISCFUNC.OBJ NOTEBOOK.OBJ WPCONTNR.OBJ WPDATAFL.OBJ \
	WPDESKTP.OBJ WPMINWIN.OBJ WPNOTEBK.OBJ WPOBJECT.OBJ WPPROG.OBJ WPSETNB.OBJ WPSETUP.OBJ \
	WPSHADOW.OBJ WPSHREDR.OBJ WPSYSSET.OBJ WPTEMPLT.OBJ WPWINDOW.OBJ IMAGELIB.OBJ WPDRIVE.OBJ \
	WPFLDR_1.OBJ WPFLDR_2.OBJ WPS_API.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
DDEPRGMN.OBJ +
DDESERVR.OBJ +
DDESHELL.OBJ +
DESKTOP.OBJ +
FLDRCOPY.OBJ +
FLDRFIND.OBJ +
FOLDER.OBJ +
LIST.OBJ +
MAIN.OBJ +
MISCFUNC.OBJ +
NOTEBOOK.OBJ +
WPCONTNR.OBJ +
WPDATAFL.OBJ +
WPDESKTP.OBJ +
WPMINWIN.OBJ +
WPNOTEBK.OBJ +
WPOBJECT.OBJ +
WPPROG.OBJ +
WPSETNB.OBJ +
WPSETUP.OBJ +
WPSHADOW.OBJ +
WPSHREDR.OBJ +
WPSYSSET.OBJ +
WPTEMPLT.OBJ +
WPWINDOW.OBJ +
IMAGELIB.OBJ +
WPDRIVE.OBJ +
WPFLDR_1.OBJ +
WPFLDR_2.OBJ +
WPS_API.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\ddk\lib\+
CTL3D.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) WPSHELL.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	WPSHELL.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) WPSHELL.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
