@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Workplace Shell for Windows Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming WARPICON.ZIP to WARPBAK.ZIP
RENAME WAPRICON.ZIP WARPBAK.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\WARPICON.ZIP *.*
CD .\BACKUP
CLS
ECHO WARPICON.ZIP file has been created in .\BACKUP directory.
