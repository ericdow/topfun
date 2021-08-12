# build the sky library
set(SOURCES
  Sky.cpp
  CloudRenderer.cpp
  NoiseCube.cpp
)

set(libs_to_link 
  ${OPENGL_LIBRARIES} 
  ${GLUT_LIBRARY} 
  ${GLEW_LIBRARIES} 
  ${GLFW_LIBRARIES} 
  ${SOIL_LIBRARY} 
  shader
  render
)

set(include_dirs 
  ${OPENGL_INCLUDE_DIRS} 
  ${GLUT_INCLUDE_DIR} 
  ${GLEW_INCLUDE_DIRS} 
  ${GLFW_INCLUDE_DIRS} 
  ${SOIL_INCLUDE_DIRS} 
)

add_library(sky STATIC ${SOURCES})
target_include_directories(sky PUBLIC ${include_dirs})
target_link_libraries(sky ${libs_to_link})
