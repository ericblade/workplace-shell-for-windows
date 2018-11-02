@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Progman to Workplace Shell Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming CONVINI.ZIP to CONVINI.BAK
RENAME CONVINI.ZIP CONVINI.BAK
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\CONVINI.ZIP *.*
CD .\BACKUP
CLS
ECHO CONVINI.ZIP file has been created in .\BACKUP directory.
