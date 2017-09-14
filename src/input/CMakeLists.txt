# build the input library
set(SOURCES
  CallBackWorld.cpp
)

set(libs_to_link 
  ${GLFW_LIBRARIES}
)

set(include_dirs 
  ${GLFW_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIR}
)

add_library(input STATIC ${SOURCES})
target_include_directories(input PUBLIC ${include_dirs})
target_link_libraries(input ${libs_to_link})
add_dependencies(input assimp)
