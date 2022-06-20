@echo off
cd source
REM "mingw32-g++" -c -O2 "D:/Libs/glad/src/gl.c" -o"objects/gl.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall app/main.cpp -o"objects/main.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall app/graphics.cpp -o"objects/graphics.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall app/callbacks.cpp -o"objects/callbacks.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall app/argparser.cpp -o"objects/argparser.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/print.cpp -o"objects/print.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/stb_image.c -o"objects/stb_image.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/glUtil.c -o"objects/glUtil.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/localMath.cpp -o"objects/localMath.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/camera.cpp -o"objects/camera.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/shader.cpp -o"objects/shader.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/mesh.cpp -o"objects/mesh.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/model.cpp -o"objects/model.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall runtime/sbObject.cpp -o"objects/sbObject.o" @inc.rsp
REM "mingw32-g++" -c -O2 -Wall models/sph.c -o"objects/sph.o" @inc.rsp
pause
cd objects
"mingw32-g++" main.o graphics.o gl.o callbacks.o argparser.o print.o stb_image.o glUtil.o localMath.o camera.o shader.o mesh.o model.o sbObject.o sph.o -g -o "../../App/view.exe" -lglfw3dll -lassimp-5 -L"D:/Libs/glfw-3.3.7/build/mingw32/src" -L"D:/Libs/assimp/mingw32/bin" -Wl,-subsystem,console
pause