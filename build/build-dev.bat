del /F /S /Q .\upgrade.lgu
del /F /S /Q .\unpacked\*
xcopy .\orig\* .\unpacked\* /E
xcopy .\patch\* .\unpacked\* /E /Y
import-patcher ".\orig\upgrade\Storage Card\System" ".\unpacked\upgrade\Storage Card\System"
del /F /S /Q .\unpacked\upgrade\booter_standalone.bin
dir2lgu -p m2 "9.1.3.0.3" ./unpacked ./upgrade.lgu
rmdir /Q /S ".\unpacked\upgrade"

