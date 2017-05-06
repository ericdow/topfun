# build the utils library
set(SOURCES
  GLEnvironment.cpp
  KeyState.cpp
)

set(libs_to_link ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES})
set(include_dirs ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

add_library(utils STATIC ${SOURCES})
target_include_directories(utils PUBLIC ${include_dirs})
target_link_libraries(utils ${libs_to_link})
