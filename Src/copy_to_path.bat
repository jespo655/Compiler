@echo off

:: /B indicates a binary file
:: /Y supresses prompting to confirm you want to overwrite an existing destination file (seems to not do anything)
:: /Q : Suppresses the display of xcopy messages.
:: /R : Copies read-only files.
if exist C:\PATH\ xcopy /B /Q /Y "BUILD/cube.exe" "C:/PATH/cube.exe"
if exist D:\PATH\ xcopy /B /Q /Y "BUILD/cube.exe" "D:/PATH/cube.exe"
