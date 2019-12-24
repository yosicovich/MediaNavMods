del /F /Q .\upgrade.lgu
del /F /S /Q .\unpacked\*
rem xcopy .\orig\* .\unpacked\* /E
xcopy .\patch\* .\unpacked\* /E /Y
rem xcopy .\test\* .\unpacked\* /E /Y
import-patcher ".\orig\upgrade\Storage Card\System" ".\unpacked\upgrade\Storage Card\System"
rmdir /Q /S ".\unpacked\upgrade\Storage Card4"
del /F /Q .\unpacked\upgrade\booter_standalone.bin
del /F /Q ".\unpacked\upgrade\Storage Card\System\Version_Info.txt"
del /F /Q ".\unpacked\upgrade\Storage Card\nk.bin"
call addStrings.bat .\orig .\unpacked .\sources
dir2lgu -p m2 "Video-dev" ./unpacked ./upgrade.lgu
rmdir /Q /S ".\unpacked\upgrade"
