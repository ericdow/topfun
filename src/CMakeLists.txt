add_executable(TopFun TopFun.cpp) 
target_include_directories(TopFun PUBLIC terrain utils) 
target_link_libraries(TopFun terrain utils)

add_subdirectory(tests)
add_subdirectory(utils)
add_subdirectory(terrain)
add_subdirectory(shaders)
add_subdirectory(static_test)
