@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Workplace Shell for Windows Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming OS21ICON.ZIP to OS21BAK.ZIP
RENAME OS21ICON.ZIP OS21BAK.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\OS21ICON.ZIP *.*
CD .\BACKUP
CLS
ECHO OS21ICON.ZIP file has been created in .\BACKUP directory.
