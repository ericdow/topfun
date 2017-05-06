add_executable(quat_test quaternion_test.cpp) 
target_include_directories(quat_test PRIVATE ${Boost_INCLUDE_DIRS}) 
target_link_libraries(quat_test ${Boost_LIBRARIES})

add_executable(eigen_test eigen_test.cpp) 
target_include_directories(eigen_test PRIVATE ${EIGEN3_INCLUDE_DIR})
target_link_libraries(eigen_test ${EIGEN_LIBRARIES})
