@echo off
g++ -std=c++11 *.cpp abstx/*.cpp utilities/*.cpp compile_time/*.cpp

:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: copy /B /Y "a.exe" "D:/PATH/jm.exe"
copy /B "a.exe" "D:/PATH/jm.exe"