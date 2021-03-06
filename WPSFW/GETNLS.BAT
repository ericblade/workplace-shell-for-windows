@ECHO OFF
ECHO Get NLS Resources
DELTREE /Y %1
MKDIR %1
CD %1
MKDIR WPSHELL
MKDIR PMTOWP
MKDIR CONVINI
MKDIR WNDLIST

REM WPSHELL
CD WPSHELL
XCOPY \WPSFW\WPSHELL\NLS\%1\RESOURCE\*.* *.* /S /E /Y

REM PMTOWP
CD ..\PMTOWP
XCOPY \WPSFW\PMTOWP\NLS\%1\RESOURCE\*.* *.* /S /E /Y
DEL *.RC

REM CONVINI
CD ..\CONVINI
XCOPY \WPSFW\CONVINI\NLS\%1\RESOURCE\*.* *.* /S /E /Y
DEL *.RC

REM WNDLIST
CD ..\WNDLIST
XCOPY \WPSFW\WNDLIST\RESOURCE\MENUS\*.* *.* /S /E /Y
E MENUS.RC

REM COMPRESS
CD ..
PKZIP -rP NLS_%1.ZIP *.*

REM DONE
ECHO DONE
