
@echo off

:: The dll file names that need to be copied
set dll_names=SDL2.dll

set copy_opt=/S /Y /I

:: Source folder
set dll_folder=..\external\lib\%1
:: Destination folder
set bin_folder=..\bin\%1

:: Handle the case where this bat is in the solution folder or in the project folder
if not exist %bin_folder% set bin_folder=.\bin\%1
if not exist %dll_folder% set dll_folder=.\external\lib\%1

echo File is %dll_folder%

:: Copy all the dlls
for %%i in (%dll_names%) do (
if not exist %bin_folder%\Debug\%%i xcopy %dll_folder%\%%i %bin_folder%\Debug\ %copy_opt%
if not exist %bin_folder%\Release\%%i xcopy %dll_folder%\%%i %bin_folder%\Release\ %copy_opt%
)
