CTest 0.2.2 alpha

Controls:
	WASD Space Shift - accelerate the camera
	IJKL - accelerate the cube
	E/Q - precision of physics (main.cpp: tdiv variable)
	U/H - hardness of spheres (main.cpp: kf variable)
	T/G - viscosity (main.cpp: visc variable)
	!! Recommended to up viscosity to 0.5-0.7 to see the difference between simple spheres and tetrahedron. !!
	M/N - multiply/divide main sphere mass by 1.1 
	O/P - add/remove a sphere (to/from end of "sph" vector)
	R - reset the scene (8 spheres and cube)
	B - debug information

	ESC - free cursor
	LMB - hold cursor; accelerate the sphere in the cameraForward vector direction (radius 8 m)
	RMB - accelerate the sphere in the direction to camera (radius 8 m.)
  	MMB (press) - reset render distance
	MMB (rotating) - change FOV


Quantity of small spheres is limited by 65535.
Djanibekov effect is noticeable on the rotated tetrahedron.

Compiles well with MSVS and GCC both.
Libraries used:
GLFW 3.3.4 (https://glfw.org https://github.com/glfw/glfw)
GLM 0.9.9.8 (https://github.com/g-truc/glm)
stb (https://github.com/nothings/stb)

15 mar 2022
