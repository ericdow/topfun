# build the audio library
set(SOURCES
  AudioManager.cpp
  AudioSource.cpp
)

set(libs_to_link 
  ${OPENAL_LIBRARY} 
  ${ALUT_LIBRARY} 
)

set(include_dirs 
  ${OPENAL_INCLUDE_DIR} 
  ${ALUT_INCLUDE_DIR} 
)

add_library(audio STATIC ${SOURCES})
target_include_directories(audio PUBLIC ${include_dirs})
target_link_libraries(audio ${libs_to_link})
