add_executable(TopFun TopFun.cpp) 
target_include_directories(TopFun PUBLIC terrain utils ${FREETYPE_INCLUDE_DIRS}) 
target_link_libraries(TopFun terrain utils ${FREETYPE_LIBRARIES})

add_subdirectory(tests)
add_subdirectory(learnopengl)
add_subdirectory(utils)
add_subdirectory(terrain)
add_subdirectory(shaders)
