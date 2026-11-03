; Installs the PATCH demo program
;
;
Name "CUpdateManager_Demo"
SetOverwrite on
OutFile "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\Installer\Build\V10_Install.exe"
SilentInstall silent


Section ""; (default section)

StrCpy $INSTDIR "C:\PF\Demo_Patch"

SetOutPath "$INSTDIR"
File "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\Debug\CUpdateManager_Demo.exe"
File /oname=color.txt "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\color_patch.txt"
File /oname=version.txt "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\version_patch.txt"

SetOutPath "$INSTDIR\updates\"
File "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\updates\masterupdate.xml"
File "C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\updates\WTRerunProgramUtility.exe"

SectionEnd
