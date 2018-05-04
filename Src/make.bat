@echo off

:: static include paths makes the code nicer, but is not worth it because it makes the program compile 50-100% slower
:: set INCLUDE_PATHS=-Iutilities -Itypes
set SRC_FILES=*.cpp lexer/*.cpp utilities/*.cpp types/*.cpp runtime_dll/*.cpp abstx/*.cpp parser/statement_parser.cpp
:: parser/*.cpp code_gen/*.cpp
set LIBS32=runtime_dll/dyncall/lib32/libdyncall_s.lib
set LIBS64=runtime_dll/dyncall/lib64/libdyncall_s.lib
set OUTPUT_NAME=cube.exe

:: comment this line out if only 32bit g++ is installed on 64 bit system
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto 64BIT

:32BIT
echo Compiling cube_32...
g++ -std=gnu++14 %INCLUDE_PATHS% %SRC_FILES% %LIBS32% -o %OUTPUT_NAME%
goto END

:64BIT
echo Compiling cube_64...
g++ -std=gnu++14 %INCLUDE_PATHS% %SRC_FILES% %LIBS64% -o %OUTPUT_NAME%

:END

:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: /Q : Suppresses the display of xcopy messages.
:: /R : Copies read-only files.
if exist C:\PATH\ xcopy /B /Q /Y "cube.exe" "C:/PATH/cube.exe"
if exist D:\PATH\ xcopy /B /Q /Y "cube.exe" "D:/PATH/cube.exe"
