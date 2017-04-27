cmake_minimum_required (VERSION 3.5.1)

project(TopFun)

# load cmake modules (for things like where to find libraries)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# set various compiler flags
add_compile_options(-std=c++11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

# make sure that Boost math is found
find_package(Boost REQUIRED COMPONENTS math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l) 
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME ON)

# make sure that Eigen 3 is found
find_package(Eigen3 REQUIRED)

# make sure that OpenGL stuff is found
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(SOIL REQUIRED)

add_subdirectory(external)
add_subdirectory(src)

