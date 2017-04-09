cmake_minimum_required (VERSION 3.5.1)

project(TopFun)

# make sure that Boost math has been found
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED COMPONENTS math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l) 

add_executable(topfun quaternion_test.cpp) 

if(Boost_FOUND)
  ADD_DEFINITIONS("-DHAS_BOOST")
  include_directories(${Boost_INCLUDE_DIRS}) 
  target_link_libraries(topfun ${Boost_LIBRARIES})
endif()
