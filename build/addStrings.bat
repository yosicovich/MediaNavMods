@echo off
@setlocal
@set SOURCE_DIR=%1
@set TARGET_DIR=%2
@set RESOURCE_DIR=%3

call :add_one %SOURCE_DIR% %TARGET_DIR% %RESOURCE_DIR% Rus
@goto :EOF

:add_one
@setlocal
@set SOURCE_DIR=%1
@set TARGET_DIR=%2
@set RESOURCE_DIR=%3
@set LANG=%4
ResourceHacker -open %RESOURCE_DIR%\strings_%LANG%.rc -save %TEMP%\strings_%LANG%.res -action compile
ResourceHacker -open "%SOURCE_DIR%\upgrade\Storage Card\System\data\LangDll%LANG%.dll" -save "%TARGET_DIR%\upgrade\Storage Card\System\data\LangDll%LANG%.dll" -action add -res %TEMP%\strings_%LANG%.res -mask ,,
del /F /Q %TEMP%\strings_%LANG%.res
@goto :EOF
