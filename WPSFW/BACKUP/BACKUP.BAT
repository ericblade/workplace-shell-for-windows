@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Workplace Shell for Windows Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming WPSFW.ZIP to WPSBAK.ZIP
RENAME WPSFW.ZIP WPSBAK.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\WPSFW.ZIP *.*
CD .\BACKUP
CLS
ECHO WPSFW.ZIP file has been created in .\BACKUP directory.
