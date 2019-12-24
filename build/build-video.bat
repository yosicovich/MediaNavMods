@echo off
set TARGETPATH=.\rel\video

del /F /Q %TARGETPATH%\upgrade.lgu
del /F /S /Q .\unpacked\*
rem xcopy .\orig\* .\unpacked\* /E
xcopy .\patch\* .\unpacked\* /E /Y
import-patcher ".\orig\upgrade\Storage Card\System" ".\unpacked\upgrade\Storage Card\System"
rmdir /Q /S ".\unpacked\upgrade\Storage Card4"

del /F /Q ".\unpacked\upgrade\Storage Card\System\mods\codecs\audiocorefilter.dll"
del /F /Q ".\unpacked\upgrade\Storage Card\System\mods\codecs\MatroskaFilter.dll"

call addStrings.bat .\orig .\unpacked .\sources
dir2lgu -p m2 "9.1.3.1.3" ./unpacked %TARGETPATH%\upgrade.lgu
rem rmdir /Q /S ".\unpacked\upgrade"
