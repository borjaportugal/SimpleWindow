:: This scipt depends on 'copy_dlls.bat' that takes as argument the target platform

@echo off

:: Handle the case where this bat is in the solution folder or in the project folder
if exist ..\copy_dlls.bat ( call ..\copy_dlls.bat x64 ) else call .\copy_dlls.bat x64
