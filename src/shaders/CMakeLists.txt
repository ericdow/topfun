# copy the shader programs over 
set(FILES_TO_SEND
  terrain.vs
  terrain.frag
  text.vs
  text.frag
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
