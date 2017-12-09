
#include "audio/AudioManager.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
AudioManager& AudioManager::Instance() {
  static AudioManager am;
  return am; 
}

//****************************************************************************80
void AudioManager::AddBuffer(const std::string& path, 
    const std::string& handle) {
  // Load the buffer and store in the map
  ALuint b = alutCreateBufferFromFile(path.c_str());
  if (buffers_.find(handle) == buffers_.end()) {
    buffers_[handle] = b;
  }
  else {
    std::string message = "Buffer with handle already exists\n";
    throw std::runtime_error(message);
  }
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
AudioManager::AudioManager() {
  // Initialize OpenAL
  alutInit(NULL, NULL);
}

//****************************************************************************80
AudioManager::~AudioManager() {
  // Delete the buffers
  for (auto& b : buffers_) {
    alDeleteBuffers(1, &(b.second));
  }
}

} // End namespace TopFun
