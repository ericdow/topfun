cmake_minimum_required (VERSION 3.5.1)

project(TopFun)

add_subdirectory(src)

# set various compiler flags
add_compile_options(-std=c++11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

# make sure that Boost math has been found
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED COMPONENTS math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l) 

