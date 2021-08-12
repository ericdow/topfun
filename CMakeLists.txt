cmake_minimum_required (VERSION 3.5.1)
include(ExternalProject)

project(TopFun)

# load cmake modules (for things like where to find libraries)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# set various compiler flags
add_compile_options(-std=c++11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -Wextra")
set(CMAKE_VERBOSE_MAKEFILE ON)

# make sure that Boost math is found
find_package(Boost REQUIRED COMPONENTS math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l) 
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME ON)

# make sure that OpenGL stuff is found
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(SOIL REQUIRED)

# make sure FreeType is found
find_package(Freetype REQUIRED)

# make sure that OpenAL stuff is found
find_package(OpenAL REQUIRED)
find_package(ALUT REQUIRED)

# try to find google perftools 
find_package(Gperftools)

# build and install libnoise
ExternalProject_Add(
libnoise
URL ${CMAKE_SOURCE_DIR}/externals/libnoise-master.zip
PREFIX ${CMAKE_CURRENT_BINARY_DIR}/externals/libnoise
CMAKE_COMMAND cmake ..
BUILD_COMMAND make -j4
CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
)

SET(LIBNOISE_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/noise)
SET(LIBNOISE_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib/libnoise.so)

# build and install assimp
ExternalProject_Add(
assimp
URL ${CMAKE_SOURCE_DIR}/externals/assimp-3.1.1.zip
PREFIX ${CMAKE_CURRENT_BINARY_DIR}/externals/assimp-3.1.1
CMAKE_COMMAND cmake CMakeLists.txt -G "Unix Makefiles"
CMAKE_ARGS -DCMAKE_CXX_FLAGS=-fPIC
CMAKE_ARGS -DCMAKE_C_FLAGS=-fPIC
CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
BUILD_COMMAND make -j4
)

SET(ASSIMP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/assimp)
SET(ASSIMP_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib/libassimp.so)

include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)

