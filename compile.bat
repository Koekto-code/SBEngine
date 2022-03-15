@echo off
cd source
mingw32-g++ gl.c -c -O2 -I"D:\Libs\glad\include"
mingw32-g++ main.cpp -c -O2 -I"D:\Libs\stb" -I"D:\Libs\glm-master" -I"D:\Libs\glad\include" -I"D:\Libs\glfw-3.3.4\include"
pause
mingw32-g++ gl.o main.o -s -o "../Release/ctest.exe" -lglfw3dll -L"D:\Libs\glfw-3.3.4\mingw32\src" -Wl,-subsystem,windows
pause