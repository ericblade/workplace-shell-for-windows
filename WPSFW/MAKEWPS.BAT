@echo off
cls
echo Creating Workplace Shell for Windows Setup Diskette...
deltree /y d:\wpssetup\temp
deltree /y d:\wpssetup\disk1
c:\mssetup\disklay\dsklayt2 d:\wpssetup\wpinst.lyt d:\wpssetup\wpinst.inf /c "d:\wpssetup\temp" /d "d:\wpssetup" /f /k 144
deltree /y d:\wpssetup\temp
EXPAND d:\wpssetup\disk1\wpinst.in_ d:\wpssetup\disk1\wpinst.inf
DEL    d:\wpssetup\disk1\wpinst.in_
PAUSE
EDIT d:\wpssetup\disk1\wpinst.inf
echo Done updating WPINST.INF file.
COMPRESS d:\wpssetup\disk1\wpinst.inf d:\wpssetup\disk1\wpinst.in_
DEL d:\wpssetup\disk1\wpinst.inf
echo Done (setup files are in D:\WPSSETUP\DISK1 directory).
