# build the aircraft library
set(SOURCES
  Aircraft.cpp
)

set(libs_to_link
  terrain 
  ${OPENGL_LIBRARIES} 
  ${GLUT_LIBRARY} 
  ${GLEW_LIBRARY} 
  ${GLFW_LIBRARIES} 
  ${SOIL_LIBRARY}
  ${ASSIMP_LIBRARIES} 
)

set(include_dirs 
  ${OPENGL_INCLUDE_DIRS} 
  ${GLUT_INCLUDE_DIR} 
  ${GLEW_INCLUDE_DIRS} 
  ${GLFW_INCLUDE_DIRS} 
  ${SOIL_INCLUDE_DIRS} 
  ${ASSIMP_INCLUDE_DIR}
)

add_library(aircraft STATIC ${SOURCES})
target_include_directories(aircraft PUBLIC ${include_dirs})
target_link_libraries(aircraft ${libs_to_link})
add_dependencies(aircraft assimp)
