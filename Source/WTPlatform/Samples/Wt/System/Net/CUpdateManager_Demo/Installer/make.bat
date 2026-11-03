set D=C:\Projects\WTPlatform\Samples\Wt\System\Net\CUpdateManager_Demo\Installer\
set DB=%D%build\
set NSIS=C:\Projects\Resources\software\toolsutilities\nsis\2.07\makensis.exe
set WWW=C:\Inetpub\wwwroot\updates\myprogram\updates\download
set AUMC=C:\Projects\WTPlatform\ToolsUtilities\AUM_Console\Debug\AUM_Console.exe
set ORIGDIR=C:\PF\Demo\
set PATCHDIR=C:\PF\Demo_Patch\
set WTPP=C:\Projects\Resources\software\toolsutilities\wtpp\wtpp.exe

cd /d "%D%\"

if not exist %AUMC% goto NO_AUMC
if not exist %WWW% goto NO_WWW
if not exist %NSIS% goto NO_NSIS
goto start
:NO_AUMC
	echo Error! there isn't an AUM_Console at %AUMC%
	goto done
:NO_WWW
	echo Error! there isn't a WWW root directory at %WWW%
	echo Try setting up IIS to make it?
	goto done
:NO_NSIS
	echo Error!  there isn't an NSIS tool at %NSIS%
	echo Try copying over the \\wtcfs01\dev\resources directory
	goto done

:start
call :makeorig
call :make10
goto done
call :make20
call :make30
call :make40
goto done


REM
REM Make the original and install it
REM
:makeorig
if exist build rmdir /s /q build
md build
if exist "%ORIGDIR%" rmdir /s /q "%ORIGDIR%"
if exist "%PATCHDIR%" rmdir /s /q "%PATCHDIR%"

%NSIS% "%D%initial.nsi"
build\install
goto :EOF

REM
REM Patch 10: simple change (is a smoke test)
REM
:make10
    set VERSION=10
    echo make%VERSION% started
    %NSIS% "%D%v%VERSION%.nsi"
    build\V%VERSION%_install
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=Makes nicer colors" "/DLONG=Those old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V%VERSION%_AUMC.dat"
    echo make%VERSION% ended
goto :EOF

REM
REM Patch 20: modified (broken) patch 10 manifest
REM
:make20
    set VERSION=20
    echo make%VERSION% started
    set DEST=build\v%VERSION%_patchmanifest.xml
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=Broken manifes of Makes nicer colors" "/DLONG=Broken manifest of Those old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V%VERSION%_AUMC.dat"
    REM And now destroy the patchmanifest file!
    textstat -enum loop 58 100 1 | textstat -function FILE::pokebyte "Q%DEST%" C1 Q65 -resultsonly
goto :EOF

REM
REM Patch 3x series: too big, too small, bad bytes
REM All use variants of the V10 patch update
REM
:make30

    REM
    REM Number 31--make the file too big
    REM
    set VERSION=31
    echo make%VERSION% started
    set DEST=build\V%VERSION%_patch.exe
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=patch too big of Makes nicer colors" "/DLONG=patch too big of Those old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V%VERSION%_AUMC.dat"
    textstat -enum loop 1 2 1 | textstat -function FILE::size "Q%DEST%" name size -calc Csize + 10 into Csize -function FILE::truncate "Q%DEST%" Csize

    REM
    REM Number 32--make the file too small
    REM
    set VERSION=32
    echo make%VERSION% started
    set DEST=build\V%VERSION%_patch.exe
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=patch too small of Makes nicer colors" "/DLONG=patch too small of Those old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V%VERSION%_AUMC.dat"
    textstat -enum loop 1 2 1 | textstat -function FILE::size "Q%DEST%" name size -calc Csize - 10 into Csize -function FILE::truncate "Q%DEST%" Csize

    REM
    REM Number 33--randomly change some bytes
    REM
    set VERSION=33
    echo make%VERSION% started
    set DEST=build\V%VERSION%_patch.exe
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=patch has bad bytes of Makes nicer colors" "/DLONG=patch has bad bytes of Those old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V%VERSION%_AUMC.dat"
    textstat -enum loop 1 20 1 | textstat -function FILE::size "Q%DEST%" name size -function MATH::randomrange 1 Csize name pos -function FILE::pokebyte "Q%DEST%" Cpos 30

goto :EOF

REM
REM Patch 40: like patch 10, but much larger
REM
:make40
    set VERSION=40
    echo make%VERSION% started
    echo about to ... copy make.bat "%PATCHDIR%largefile.txt"
    copy make.bat "%PATCHDIR%largefile.txt"
    if not exist largefile.txt textstat -enum loop 0 1000000 1 | textstat -use MATH -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 -function randomrange 0 1000000 > largefile.txt
    copy largefile.txt "%PATCHDIR%largefile.txt
    %WTPP% "/DDIR=%D%\" /DVERSION=%VERSION% "/DSHORT=huge patch nicer colors" "/DLONG=huge patch old colors are so october -- update yourself to november now" < "%D%Vversion_AUMC.dat" "/O%DB%V%VERSION%_AUMC.dat"
    %WTPP% /DVERSION=%VERSION% /Obuild/V%VERSION%_Upload.bat < Upload.bat
    %AUMC% read commands ifile "%DB%V40_AUMC.dat"
goto :EOF

:done
pause

