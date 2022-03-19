@echo off
cd source
"mingw32-g++" gl.c -o"objects/gl.o" -c -O2 -I"D:\Libs\glad\include"
"mingw32-g++" physics.cpp -o"objects/physics.o" -c -O2 -I"D:\Libs\glm-master"
"mingw32-g++" object.cpp -o"objects/object.o" -c -O2 -I"D:\Libs\glm-master"
"mingw32-g++" camera.cpp -o"objects/camera.o" -c -O2 -I"D:\Libs\glm-master" -I"D:\Libs\glfw-3.3.4\include"
"mingw32-g++" shader.cpp -o"objects/shader.o" -c -O2 -I"D:\Libs\glm-master" -I"D:\Libs\glad\include"
"mingw32-g++" main.cpp -o"objects/main.o" -c -O2 -I"D:\Libs\stb" -I"D:\Libs\glm-master" -I"D:\Libs\glad\include" -I"D:\Libs\glfw-3.3.4\include"
"mingw32-g++" sph.c -o"objects/sph.o" -c -O1
"mingw32-g++" sphs.c -o"objects/sphs.o" -c -O1
"mingw32-g++" cube.c -o"objects/cube.o" -c -O2 -I"D:\Libs\glm-master"
pause
cd objects
"mingw32-g++" gl.o physics.o object.o camera.o shader.o main.o sph.o sphs.o cube.o app.res -s -o "../../Release/ctest.exe" -lglfw3dll -L"D:\Libs\glfw-3.3.4\mingw32\src" -Wl,-subsystem,windows
pause
