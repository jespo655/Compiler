@echo off
:: g++ -std=c++11 *.cpp abstx/*.cpp utilities/*.cpp compile_time/*.cpp %*
:: g++ -std=gnu++11 parser/*.cpp *.cpp utilities/*.cpp compile_time/*.cpp %* -o cube.exe
g++ -std=gnu++11 *.cpp lexer/*.cpp utilities/*.cpp types/*.cpp runtime_dll/*.cpp runtime_dll/dyncall/lib32/libdyncall_s.lib -o cube.exe
:: parser/*.cpp


:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: /Q : Suppresses the display of xcopy messages.
:: /R : Copies read-only files.
xcopy /B /Q /Y "cube.exe" "C:/PATH/cube.exe"
:: xcopy /B /Q /Y "cube.exe" "D:/PATH/cube.exe"