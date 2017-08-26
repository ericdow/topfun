# TopFun

A (very) simple flight simulator written in C++

# Dependencies

* libboost-math-dev
* libglm-dev
* libglfw3-dev
* freeglut3-dev
* libglew-dev
* libxmu-dev
* libxi-dev
* libsoil-dev
* libfreetype6-dev

libnoise and assimp are built alongside TopFun

Make sure to add -fPIC to C/C++ flags in CMake when compiling assimp

# Features

* Detailed aerodynamic model
* Adaptive LoD terrain
* Cascaded shaddow maps
* Joystick support
