# SBEngine v0.1.0-alpha
24 june, 2022

# About
This is mainly the implementation of basic physics engine.
Now program allows to load model from Wavefront OBJ file and see it falling/orbiting/rotating.
Processing collisions between two custom bodies isn't implemented yet.

Tested builds with MinGW 9.2.0 and MinGW-w64 8.1.0.

# Dependencies
! Some parts of original libraries may be deleted to reduce their size (such as .git folders, tests, cmake files, documentation...)
```
	GLFW 3.x https://github.com/glfw/glfw
	Assimp 5.2.2 https://github.com/assimp/assimp
	GLM 0.9.9.8 https://github.com/g-truc/glm
	GLAD Loader 2.0.0 (for GL 3.3): https://gen.glad.sh/
	STB https://github.com/nothings/stb
All of them except Assimp are placed in ./source/deps/
```
# Launching
```
Program waits 2 seconds from launch to reduce the chance of object to crash and fall apart.
If this happened, try launching the program without heavy CPU loads or specifying -t 0.01 option.

view [options]...
Options:
  -m <file>  		Load the model from <file>.
  -h <float> 		Set hardness to <float>. Value is non-negative.
  -v <float> 		Set viscosity to <float>. Values are from 0 to 1.
  -t <float> 		Set time multiplier to <float>.
  -p <int>   		Multiply physics frequency by <unsigned>. Value is positive.
  --m1 <int>   		Set first marker to <int>-th vertex of model. Value is non-negative.
  --m2 <int>   		Set second marker to <int>-th vertex of model. Value is non-negative.
  --mass <float>	Specify initial planet mass. Recommended 2.5e+14.
```
# Controls:
```
WASD/Space/Shift - accelerate camera

Change values: (without Alt- higher, lower otherwise)
	H: hardness (global)
	G: viscosity (global)
	M: planet mass
	U: rotation force
	F: first marker destination
	V: second marker destination
	K: time modifier (0 - time stopped, 1 - normal)

Ctrl + K: reset time modifier to 1.0
R: reset object position
B: display variables & other information
Y: toggle sky texture (or retry to load)
T: rotate object (apply force to an object perpendicularly to both markers)

ESC: free cursor
LMB: allocate cursor | set object velocity to 0
RMB: hold object
RMB + Alt: push object
RMB + LMB: set the same object velocity as the camera (don't block rotation)
RMB + LMB + Alt: set the same object velocity as the camera and block rotation
MMB: reset render distance
Scroll: change render distance
Alt + Scroll: change camera FoV
```
