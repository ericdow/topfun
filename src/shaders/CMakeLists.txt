# build the shader library
set(SOURCES
  Shader.cpp
)

add_library(shader STATIC ${SOURCES})

# copy the shader programs over 
set(FILES_TO_SEND
  fog.glsl
  shadow.glsl
  material.glsl
  light.glsl
  raymarch.glsl
  terrain.vs
  terrain.fs
  text.vs
  text.fs
  skybox.vs
  skybox.fs
  aircraft.vs
  aircraft.fs
  canopy.fs
  exhaust.vs
  exhaust.fs
  depthmap.vs
  depthmap.fs
  debug_quad.vs
  debug_quad.fs
  clouds.vs
  clouds_raymarch.fs
  clouds_blend.fs
) 

# this sends files over if they have changed
foreach(myfile ${FILES_TO_SEND}) 
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${myfile}" 
    COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${myfile}" 
    "${CMAKE_CURRENT_BINARY_DIR}/${myfile}" 
    DEPENDS   "${CMAKE_CURRENT_SOURCE_DIR}/${myfile}")
    list(APPEND myfile_dest "${CMAKE_CURRENT_BINARY_DIR}/${myfile}")
endforeach(myfile)
ADD_CUSTOM_TARGET(shaders ALL DEPENDS ${myfile_dest}) 
