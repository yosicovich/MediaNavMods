del /F /S /Q .\upgrade.lgu
del /F /S /Q .\unpacked\*
rem xcopy .\orig\* .\unpacked\* /E
xcopy .\patch\* .\unpacked\* /E /Y
import-patcher ".\orig\upgrade\Storage Card\System" ".\unpacked\upgrade\Storage Card\System"
rmdir /Q /S ".\unpacked\upgrade\Storage Card4"
dir2lgu -p m2 "Video-test" ./unpacked ./upgrade.lgu
rmdir /Q /S ".\unpacked\upgrade"
