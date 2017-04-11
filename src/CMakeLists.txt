add_executable(quat_test quaternion_test.cpp) 
target_include_directories(quat_test PRIVATE ${Boost_INCLUDE_DIRS}) 
target_link_libraries(quat_test ${Boost_LIBRARIES})

add_executable(opengl_test opengl_test.cpp)
target_include_directories(opengl_test PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_libraries(opengl_test ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES})
