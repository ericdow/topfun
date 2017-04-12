# create a joystick library
add_library (joystick joystick.cc)

add_executable(joystick_test test.cc)
target_link_libraries(joystick_test joystick)
