@echo off
:: g++ -std=c++11 *.cpp abstx/*.cpp utilities/*.cpp compile_time/*.cpp %*
g++ -std=c++11 parser/*.cpp *.cpp utilities/*.cpp compile_time/*.cpp %*

:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: /Q : Suppresses the display of xcopy messages.
:: /R : Copies read-only files.
xcopy /B /Q /Y "a.exe" "C:/PATH/jm.exe"
:: xcopy /B /Q /Y "a.exe" "D:/PATH/jm.exe"