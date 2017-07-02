# build the terrain library
set(SOURCES
  Terrain.cpp
  Sky.cpp
  TerrainTile.cpp
)

set(libs_to_link 
  ${OPENGL_LIBRARIES} 
  ${GLUT_LIBRARY} 
  ${GLEW_LIBRARY} 
  ${GLFW_LIBRARIES} 
  ${SOIL_LIBRARY} 
  ${LIBNOISE_LIBRARIES}
  shader
)

set(include_dirs 
  ${OPENGL_INCLUDE_DIRS} 
  ${GLUT_INCLUDE_DIR} 
  ${GLEW_INCLUDE_DIRS} 
  ${GLFW_INCLUDE_DIRS} 
  ${SOIL_INCLUDE_DIRS} 
  ${LIBNOISE_INCLUDE_DIR}
)

add_library(terrain STATIC ${SOURCES})
target_include_directories(terrain PUBLIC ${include_dirs})
target_link_libraries(terrain ${libs_to_link})
add_dependencies(terrain libnoise)

add_executable(TestTerrainTile TestTerrainTile.cpp) 
target_include_directories(TestTerrainTile PUBLIC ${include_dirs}) 
target_link_libraries(TestTerrainTile terrain utils)
