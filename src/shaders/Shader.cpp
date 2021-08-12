#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

#include "shaders/Shader.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
  // 1. Retrieve the vertex/fragment source code from filePath
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;
  // ensures ifstream objects can throw exceptions:
  vShaderFile.exceptions (std::ifstream::badbit);
  fShaderFile.exceptions (std::ifstream::badbit);
  try {
    // Open files
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    // Substitute #include statements
    std::stringstream vShaderStream = SubstituteIncludes(vShaderFile,
        vertexPath);
    std::stringstream fShaderStream = SubstituteIncludes(fShaderFile,
        fragmentPath);
    // Close file handles
    vShaderFile.close();
    fShaderFile.close();
    // Convert stream into string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  }
  catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
  }
  const GLchar* vShaderCode = vertexCode.c_str();
  const GLchar * fShaderCode = fragmentCode.c_str();
  // 2. Compile shaders
  GLuint vertex, fragment;
  GLint success;
  GLchar infoLog[512];
  // Vertex Shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);
  // Print compile errors if any
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cout << vertexPath << std::endl      
      << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" 
      << infoLog << std::endl;
  }
  // Fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);
  // Print compile errors if any
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cout << fragmentPath << std::endl
      << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" 
      << infoLog << std::endl;
  }
  // Shader program_
  program_ = glCreateProgram();
  glAttachShader(program_, vertex);
  glAttachShader(program_, fragment);
  glLinkProgram(program_);
  // Print linking errors if any
  glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program_, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" 
      << infoLog << std::endl;
  }
  // Delete the shaders
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
std::stringstream Shader::SubstituteIncludes(std::ifstream& file_stream,
    const GLchar* path) const {
  std::string line;
  std::stringstream string_stream_out;
  // extract the path prefix
  std::string include_path(path);
  while (include_path.back() != '/') {
    include_path.pop_back();
  } 
  while (file_stream.good()) {
    std::getline(file_stream, line);
    // if line contains #include, substitute text
    if (line.find("#include") != std::string::npos) {
      std::istringstream iss(line);
      std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
          std::istream_iterator<std::string>()};
      // Remove quotes
      tokens[1].erase(0,1);
      tokens[1].pop_back();
      try {
        // Open include file
        std::ifstream include_file;
        include_file.open(include_path + tokens[1]);
        std::string include_line;
        // Loop over include file and insert into output
        while (include_file.good()) {
          std::getline(include_file, include_line);
          string_stream_out << include_line << std::endl;
        }
        // Close file handles
        include_file.close();
      }
      catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::INCLUDE_NOT_SUCCESFULLY_READ" << std::endl;
        std::cout << include_path + tokens[1] << std::endl;
      }
    }
    // otherwise, just write the line out
    else {
      string_stream_out << line << std::endl;
    }
  }
  return string_stream_out;
}

} // End namespace TopFun
