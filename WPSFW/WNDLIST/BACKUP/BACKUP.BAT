@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Workplace Shell for Windows Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming WNDLIST.ZIP to WNDLIST.ZIP
RENAME WNDLIST.ZIP WNDLIST.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\WNDLIST.ZIP *.*
CD .\BACKUP
CLS
ECHO WNDLIST.ZIP file has been created in .\BACKUP directory.
