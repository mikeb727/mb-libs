#include "shader.h"
#include "errors.h"

#include <fstream>
#include <iostream>
#include <sstream>

ShaderProgram::ShaderProgram(const char *vsPath, const char *fsPath) {
  std::string vsSourceInter, fsSourceInter;
  std::ifstream vsFile, fsFile;
  vsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    vsFile.open(vsPath);
    fsFile.open(fsPath);
    std::stringstream vsStream, fsStream;
    vsStream << vsFile.rdbuf();
    fsStream << fsFile.rdbuf();
    vsSourceInter = vsStream.str();
    fsSourceInter = fsStream.str();
    vsFile.close();
    fsFile.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "could not create shader program.\n";
  }
  const char *vsSource = vsSourceInter.c_str();
  const char *fsSource = fsSourceInter.c_str();

  unsigned int vsId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vsId, 1, &vsSource, NULL);
  glCompileShader(vsId);
  // check our work
  char compileLog[512];
  int vsCompileResult;
  glGetShaderiv(vsId, GL_COMPILE_STATUS, &vsCompileResult);
  if (!vsCompileResult) {
    glGetShaderInfoLog(vsId, 512, NULL, compileLog);
    std::fprintf(stderr, "could not compile vertex shader \"%s\":\n%s\n", vsPath, compileLog);
  }

  unsigned int fsId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fsId, 1, &fsSource, NULL);
  glCompileShader(fsId);
  int fsCompileResult;
  glGetShaderiv(fsId, GL_COMPILE_STATUS, &fsCompileResult);
  if (!fsCompileResult) {
    glGetShaderInfoLog(fsId, 512, NULL, compileLog);
    std::fprintf(stderr, "could not compile fragment shader \"%s\":\n%s\n", fsPath, compileLog);
  }

  _glId = glCreateProgram();
  glAttachShader(_glId, vsId);
  glAttachShader(_glId, fsId);
  glLinkProgram(_glId);

  glDeleteShader(vsId);
  glDeleteShader(fsId);
}

void ShaderProgram::use() { glUseProgram(_glId); }

void ShaderProgram::setUniform(const std::string &varName, bool val) {
  glUniform1i(glGetUniformLocation(_glId, varName.c_str()), val);
}

void ShaderProgram::setUniform(const std::string &varName, int val) {
  glUniform1i(glGetUniformLocation(_glId, varName.c_str()), val);
}

void ShaderProgram::setUniform(const std::string &varName, float val) {
  glUniform1f(glGetUniformLocation(_glId, varName.c_str()), val);
}

void ShaderProgram::setUniform(const std::string &varName, float val1,
                               float val2) {
  glUniform2f(glGetUniformLocation(_glId, varName.c_str()), val1, val2);
}

void ShaderProgram::setUniform(const std::string &varName, glm::mat4 val) {
  glUniformMatrix4fv(glGetUniformLocation(_glId, varName.c_str()), 1, GL_FALSE,
                     glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string &varName, glm::vec3 val) {
  glUniform3fv(glGetUniformLocation(_glId, varName.c_str()), 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string &varName, glm::vec4 val) {
  glUniform4fv(glGetUniformLocation(_glId, varName.c_str()), 1, glm::value_ptr(val));
}