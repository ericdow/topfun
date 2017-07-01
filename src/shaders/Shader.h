#ifndef SHADER_H
#define SHADER_H

#include <fstream>

#include <GL/glew.h>

namespace TopFun {

class Shader {
 
 public:
  
  //!*************************************************************************80
  //! Constructor
  //!*************************************************************************80
  Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
  
  //!*************************************************************************80
  //! Destructor
  //!*************************************************************************80
  ~Shader() = default;

  //!*************************************************************************80
  //! Use - active the shader program
  //!*************************************************************************80
  inline void Use() const { 
    glUseProgram(program_); 
  }

  //!*************************************************************************80
  //! GetProgram - gets the progams ID
  //!*************************************************************************80
  inline GLuint GetProgram() const {
    return program_;
  }

 private:
  GLuint program_;
  
  //!*************************************************************************80
  //! SubstituteIncludes - replace any #include lines in a shader program
  //! \detail Since GLSL does not natively support #include statements, we 
  //! instead substitute the text of an #include'd file into the text of the
  //! shader program
  //! \param[in] file_stream - stream containing original shader program
  //! \param[in] path - path of includes (should live next to programs)
  //! returns - stream containing program with includes
  //!*************************************************************************80
  std::stringstream SubstituteIncludes(std::ifstream& file_stream, 
      const GLchar* path) const;
};

} // End namespace TopFun

#endif
