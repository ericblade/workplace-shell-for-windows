@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Workplace Shell for Windows Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming WPSHELL.ZIP to WPSBAK.ZIP
RENAME WPSHELL.ZIP WPSBAK.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\WPSHELL.ZIP *.*
CD .\BACKUP
CLS
ECHO WPSHELL.ZIP file has been created in .\BACKUP directory.
