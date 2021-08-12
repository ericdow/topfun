# build the utils library
set(SOURCES
  GLEnvironment.cpp
)

set(libs_to_link 
  ${OPENGL_LIBRARIES} 
  ${GLUT_LIBRARY} 
  ${GLEW_LIBRARIES} 
  ${GLFW_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${ASSIMP_LIBRARIES}
  input
)

set(include_dirs 
  ${OPENGL_INCLUDE_DIRS} 
  ${GLUT_INCLUDE_DIR} 
  ${GLEW_INCLUDE_DIRS} 
  ${GLFW_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIR}
)

add_library(utils STATIC ${SOURCES})
target_include_directories(utils PUBLIC ${include_dirs})
target_link_libraries(utils ${libs_to_link})
add_dependencies(utils assimp)
