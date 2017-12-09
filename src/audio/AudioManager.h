#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <unordered_map>
#include <string>
#include <iostream>

#include <AL/al.h>
#include <AL/alut.h>

#include <glm/glm.hpp>

namespace TopFun {

class AudioManager {
 
 public:
  //**************************************************************************80
  //! \brief Instance - get the instance of the singleton
  //**************************************************************************80
  static AudioManager& Instance();
  
  //**************************************************************************80
  //! \brief SetUp - initialize the audio environment  
  //**************************************************************************80
  static void SetUp() { alutInit(NULL,NULL); }
  
  //**************************************************************************80
  //! \brief TearDown - initialize the audio environment  
  //**************************************************************************80
  static void TearDown() { alutExit(); }

  //**************************************************************************80
  //! \brief AddBuffer - add a new buffer to the library
  //! \param[in] path - where the audio file is located
  //! \param[in] handle - string to refer to the buffer by
  //**************************************************************************80
  void AddBuffer(const std::string& path, const std::string& handle);
  
  //**************************************************************************80
  //! \brief GetBuffer - get the ID of a loaded buffer 
  //! \param[in] handle - string to refer to the buffer by
  //**************************************************************************80
  ALuint GetBuffer(const std::string& handle);
  
  //**************************************************************************80
  //! \brief SetListenerPosition - set the position of the listener
  //! \param[in] pos - position of the listener
  //**************************************************************************80
  inline void SetListenerPosition(const glm::vec3& pos) const {
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
  }
  
  //**************************************************************************80
  //! \brief SetListenerOrientation - set the orientation of the listener
  //! \param[in] fu - orientation of the listener (stacked front, up vectors)
  //**************************************************************************80
  inline void SetListenerOrientation(const std::array<ALfloat,6>& fu) const {
    alListenerfv(AL_ORIENTATION, fu.data());
  }
  
  //**************************************************************************80
  //! \brief SetListenerVelocity - set the velocity of the listener
  //! \param[in] vel - velocity of the listener
  //**************************************************************************80
  inline void SetListenerVelocity(const glm::vec3& vel) const {
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
  }

 private:
  std::unordered_map<std::string, ALuint> buffers_;
  
  //**************************************************************************80
  //! \brief AudioManager - Constructor
  //**************************************************************************80
  AudioManager();
  
  //**************************************************************************80
  //! \brief ~AudioManager - Destructor
  //**************************************************************************80
  ~AudioManager();

};
} // End namespace TopFun

#endif
