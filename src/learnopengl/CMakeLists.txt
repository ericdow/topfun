# copy the shader programs over 
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures.vs
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures.frag
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(hello_window hello_window.cpp)
target_include_directories(hello_window PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_libraries(hello_window ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES})

add_executable(hello_triangle hello_triangle.cpp)
target_include_directories(hello_triangle PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_libraries(hello_triangle ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES})

add_executable(shader_tutorial shader_tutorial.cpp)
target_include_directories(shader_tutorial PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_libraries(shader_tutorial ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES})

add_executable(texture_tutorial texture_tutorial.cpp)
target_include_directories(texture_tutorial PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${SOIL_INCLUDE_DIRS})
target_link_libraries(texture_tutorial ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES} ${SOIL_LIBRARY})
