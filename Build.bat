@echo off
g++ -o notepad_clone.exe notepad_clone.cpp -lgdi32 -lcomdlg32 -lshell32 -lole32 -mwindows -v
pause
