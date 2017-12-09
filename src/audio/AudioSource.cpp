#include "audio/AudioSource.h"
#include "audio/AudioManager.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
AudioSource::AudioSource() { 
  // Create the source and set its initial state
  alGenSources(1, &source_);
  alSourcei(source_, AL_LOOPING, AL_FALSE);
}

//****************************************************************************80
AudioSource::~AudioSource() {
  alDeleteSources(1, &source_);
}
  
//****************************************************************************80
void AudioSource::SetBuffer(const std::string& handle) {
  alSourcei(source_, AL_BUFFER, AudioManager::Instance().GetBuffer(handle)); 
}

} // End namespace TopFun
