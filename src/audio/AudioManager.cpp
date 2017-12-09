
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
  if (b == AL_NONE) {
    std::string e_message(alutGetErrorString(alutGetError()));
    std::string message = "Error loading audio buffer: " + e_message + "\n";
    throw std::runtime_error(message);
  }
  if (buffers_.find(handle) == buffers_.end()) {
    buffers_[handle] = b;
  }
  else {
    std::string message = "Buffer with handle '" + handle + "' exists\n";
    throw std::runtime_error(message);
  }
}

//****************************************************************************80
ALuint AudioManager::GetBuffer(const std::string& handle) {
  if (buffers_.find(handle) != buffers_.end()) {
    return buffers_[handle];
  }
  else {
    std::string message = "Buffer with handle '" + handle + "' not found\n";
    throw std::runtime_error(message);
  }
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
AudioManager::AudioManager() { }

//****************************************************************************80
AudioManager::~AudioManager() {
  // Delete the buffers
  for (auto& b : buffers_) {
    alDeleteBuffers(1, &(b.second));
  }
}

} // End namespace TopFun
