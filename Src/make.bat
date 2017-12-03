@echo off

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto 64BIT

:32BIT
g++ -std=gnu++11 *.cpp lexer/*.cpp utilities/*.cpp types/*.cpp runtime_dll/*.cpp runtime_dll/dyncall/lib32/libdyncall_s.lib -o cube.exe
goto END

:64BIT
g++ -std=gnu++11 *.cpp lexer/*.cpp utilities/*.cpp types/*.cpp runtime_dll/*.cpp runtime_dll/dyncall/lib64/libdyncall_s.lib -o cube.exe
:: parser/*.cpp

:END


:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: /Q : Suppresses the display of xcopy messages.
:: /R : Copies read-only files.
xcopy /B /Q /Y "cube.exe" "C:/PATH/cube.exe"
:: xcopy /B /Q /Y "cube.exe" "D:/PATH/cube.exe"