# copy the shader programs over 
set(FILES_TO_SEND
  awesomeface.png
  container.jpg
  textures.vs
  textures.frag
  transformations.vs
  transformations.frag
  coordinates.vs
  coordinates.frag
) 

# this sends files over if they have changed
foreach(myfile ${FILES_TO_SEND}) 
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${myfile}" 
    COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${myfile}" 
    "${CMAKE_CURRENT_BINARY_DIR}/${myfile}" 
    DEPENDS   "${CMAKE_CURRENT_SOURCE_DIR}/${myfile}")
    list(APPEND myfile_dest "${CMAKE_CURRENT_BINARY_DIR}/${myfile}")
endforeach(myfile)
ADD_CUSTOM_TARGET(foo ALL DEPENDS ${myfile_dest}) 

# build the executables
set(EXECUTABLES
  HelloWindow.cpp
  HelloTriangle.cpp
  ShaderTutorial.cpp
  TextureTutorial.cpp
  glmTutorial.cpp
  CoordinatesTutorial.cpp
)

set(libs_to_link ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARIES} ${SOIL_LIBRARY})
set(include_dirs ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${SOIL_INCLUDE_DIRS})

foreach(exec ${EXECUTABLES})
  get_filename_component(EXEC_NAME ${exec} NAME_WE)
  add_executable(${EXEC_NAME} ${exec})
  target_include_directories(${EXEC_NAME} PRIVATE ${include_dirs})
  target_link_libraries(${EXEC_NAME} ${libs_to_link})
endforeach(exec)
