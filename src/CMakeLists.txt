add_executable(topfun quaternion_test.cpp) 

include_directories(${Boost_INCLUDE_DIRS}) 
target_link_libraries(topfun ${Boost_LIBRARIES})
