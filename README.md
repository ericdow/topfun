![alt text](https://github.com/ericdow/topfun/blob/master/topfun_logo.png)

![alt text](https://github.com/ericdow/topfun/blob/master/screenshot.png)

# TopFun

A flight simulator written in C++. The goal of this project was to learn as much about rendering, physics and game development as possible.

# Features

* Detailed aerodynamic model
* Collision detection
* Adaptive LoD terrain
* Cascaded shadow maps
* Volumetric clouds
* 3D sounds
* Joystick support

# Useful Resources

**Physics**

[An Introduction to Physically Based Modeling I](https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf)

[An Introduction to Physically Based Modeling II](https://www.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf)

[Gaffer on Games](https://gafferongames.com/tags/physics/)

[Game Physics Series](http://allenchou.net/game-physics-series/)

**Flight Simulation**

[Flight Simulation Dynamic Modeling Using Quaternions](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.50.7824&rep=rep1&type=pdf)

[JSBSimReferenceManual](http://jsbsim.sourceforge.net/JSBSimReferenceManual.pdf)

[Principles of Flight Simulation](https://www.amazon.com/Principles-Flight-Simulation-AIAA-Education/dp/1600867030/ref=sr_1_1?crid=IIIN3V3NKCT0&dchild=1&keywords=principles+of+flight+simulation&qid=1622587278&sprefix=principles+of+flight+sim%2Caps%2C162&sr=8-1)

**OpenGL**

[Learn OpenGL](https://learnopengl.com/)

[Creating Vast Game Worlds](http://www.humus.name/Articles/Persson_CreatingVastGameWorlds.pdf)

**Volumetric Cloud Rendering**

[Convincing Cloud Rendering](https://publications.lib.chalmers.se/records/fulltext/241770/241770.pdf)

[Real-Time Volumetric Rendering](http://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf)

[Real-Time Volume Graphics](http://www.real-time-volume-graphics.org/)

[Optimisations for Real-Time Volumetric Cloudscapes](https://arxiv.org/pdf/1609.05344.pdf)

[Temporal Reprojection Anti-Aliasing](https://github.com/playdeadgames/temporal/blob/master/GDC2016_Temporal_Reprojection_AA_INSIDE.pdf)

**Shaddow Maps**

[Cascaded Shadow Mapping](https://ogldev.org/www/tutorial49/tutorial49.html)

[Light Space Perspective Shadow Maps](https://www.cg.tuwien.ac.at/research/vr/lispsm/shadows_egsr2004_revised.pdf)

[Common Techniques to Improve Shadow Depth Maps](https://docs.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN)

**Terrain Rendering**

[Fast Terrain Rendering Using Geometrical MipMapping](http://www.humus.name/Articles/Persson_CreatingVastGameWorlds.pdf)

[Virtual Terrain Project](http://vterrain.org/)

**Procedural Texturing**

[Understanding Perlin Noise](http://adrianb.io/2014/08/09/perlinnoise.html)

[Procedural Terrain Splatmapping](https://alastaira.wordpress.com/2013/11/14/procedural-terrain-splatmapping/)

[Filtering Procedural Textures](https://www.iquilezles.org/www/articles/filtering/filtering.htm)

[Implementing Improved Perlin Noise](http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch05.html)

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
* libopenal1
* libopenal-dev
* libalut0
* libalut-dev

libnoise and assimp are built alongside TopFun

Make sure to add -fPIC to C/C++ flags in CMake when compiling assimp
