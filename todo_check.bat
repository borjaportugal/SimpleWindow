@echo off

:: by default use current directory
set input_folder=.\;

:: if s directory was specified, use it
if not (%1 == "") ( set input_folder=%1 )

set wildcards=*.h *.cpp *.inl *.c

:: scan for keywords
pushd %input_folder%
call:find "TODO"
popd

goto :eof

:: ------- Functions -------

:: searches for the input parameter
:find
  echo -------
  echo %* FOUND:
  findstr -s -n -i -l %* %wildcards%
  echo -------
exit /b 0
