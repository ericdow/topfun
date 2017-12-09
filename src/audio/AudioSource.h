#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <iostream>
#include <string>

#include <AL/al.h>
#include <AL/alut.h>

#include <glm/glm.hpp>

namespace TopFun {

class AudioSource {
 
 public:
  //**************************************************************************80
  //! \brief AudioSource - Constructor
  //**************************************************************************80
  AudioSource();
  
  //**************************************************************************80
  //! \brief ~AudioSource - Destructor
  //**************************************************************************80
  ~AudioSource();
  
  //**************************************************************************80
  //! \brief SetBuffer - set the audio data buffer
  //! \param[in] handle - string that buffer is referred to 
  //**************************************************************************80
  void SetBuffer(const std::string& handle);
  
  //**************************************************************************80
  //! \brief Play - play the source
  //**************************************************************************80
  inline void Play() { alSourcePlay(source_); }
  
  //**************************************************************************80
  //! \brief Pause - pause the source
  //**************************************************************************80
  inline void Pause() { alSourcePause(source_); }
  
  //**************************************************************************80
  //! \brief SetLooping - set the looping state of the source
  //**************************************************************************80
  inline void SetLooping(bool loop) {
    alSourcei(source_, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  }
  
  //**************************************************************************80
  //! \brief SetPosition - set the position of the source
  //! \param[in] pos - position of the source
  //**************************************************************************80
  inline void SetPosition(const glm::vec3& pos) {
    alSource3f(source_, AL_POSITION, pos.x, pos.y, pos.z);
  }
  
  //**************************************************************************80
  //! \brief SetGain - set the gain of the source
  //! \param[in] gain - gain of the source
  //**************************************************************************80
  inline void SetGain(ALfloat gain) {
    alSourcef(source_, AL_GAIN, gain);
  }
  
  //**************************************************************************80
  //! \brief SetPitch - set the pitch multiplier of the source
  //! \param[in] p - pitch multiplier of the source
  //**************************************************************************80
  inline void SetPitch(ALfloat p) {
    alSourcef(source_, AL_PITCH, p);
  }
  
  //**************************************************************************80
  //! \brief SetRollOff - set the roll-off factor of the source
  //! \param[in] rof - roll-off factor of the source
  //**************************************************************************80
  inline void SetRollOff(ALfloat rof) {
    alSourcef(source_, AL_ROLLOFF_FACTOR, rof);
  }

  //**************************************************************************80
  //! \brief SetReferenceDistance - set the reference distance of the source
  //! \param[in] d_ref - reference distance of the source
  //**************************************************************************80
  inline void SetReferenceDistance(ALfloat d_ref) {
    alSourcef(source_, AL_REFERENCE_DISTANCE, d_ref);
  }
  
  //**************************************************************************80
  //! \brief SetVelocity - set the velocity of the source
  //! \param[in] vel - velocity of the source
  //**************************************************************************80
  inline void SetVelocity(const glm::vec3& vel) {
    alSource3f(source_, AL_VELOCITY, vel.x, vel.y, vel.z);
  }

 private:
  ALuint source_;

};
} // End namespace TopFun

#endif
