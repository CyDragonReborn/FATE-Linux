REM Uploads a patch by number
set WWW=C:\Inetpub\wwwroot\updates\myprogram\updates\download

copy V%VERSION%_patchmanifest.xml "%WWW%\patchmanifest.xml"
copy V%VERSION%_patch.exe "%WWW%\patch.exe"