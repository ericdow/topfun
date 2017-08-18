# build the render library
set(SOURCES
  DepthMapRenderer.cpp
  ShadowCascadeRenderer.cpp
  Camera.cpp
)

set(include_dirs 
  ${ASSIMP_INCLUDE_DIR}
  ${LIBNOISE_INCLUDE_DIR}
)

add_library(render STATIC ${SOURCES})
target_include_directories(render PUBLIC ${include_dirs})
add_dependencies(render libnoise)
