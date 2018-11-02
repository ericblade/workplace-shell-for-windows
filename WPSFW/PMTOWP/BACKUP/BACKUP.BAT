@ECHO OFF
CLS
ECHO -------------------------------------------
ECHO Progman to Workplace Shell Backup Utility.
ECHO -------------------------------------------
ECHO -
ECHO Renaming PMTOWP.ZIP to PMTOWPBK.ZIP
RENAME PMTOWP.ZIP PMTOWPBK.ZIP
ECHO Compressing files in build directories...
CD ..
PKZIP -rP -x*.ZIP .\BACKUP\PMTOWP.ZIP *.*
CD .\BACKUP
CLS
ECHO PMTOWP.ZIP file has been created in .\BACKUP directory.
