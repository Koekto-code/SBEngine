# CTest 0.2.2 alpha
15 mar 2022 (written 2 months ago)

# Controls:
```
WASD Space Shift - accelerate the camera
IJKL - accelerate the cube
E/Q - precision of physics (main.cpp: tdiv variable)
U/H - hardness of material (main.cpp: kf variable)
T/G - material viscosity (main.cpp: visc variable)
M/N - multiply/divide main sphere mass by 1.1
O/P - add/remove a sphere
R - reset the scene (8 spheres and cube)
B - debug information

ESC - free cursor
LMB - hold cursor; accelerate the sphere in the cameraForward vector direction (radius 8 m)
RMB - accelerate the sphere in the direction to camera (radius 8 m.)
MMB (press) - reset render distance
MMB (rotating) - change FOV
```
Djanibekov effect is acting and noticeable even with simple "struts" system.
```md
Compiles well with MSVS and GCC both.
# Libraries used:
GLFW 3.3.4 (https://glfw.org https://github.com/glfw/glfw)
	with GLAD 2.0 (https://gen.glad.sh)
GLM 0.9.9.8 (https://github.com/g-truc/glm)
stb (https://github.com/nothings/stb)
```
