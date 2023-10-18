#ifndef SHADER_H
#define SHADER_H

#include "glad/gl.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

class ShaderProgram {

public:
  // ctor
  ShaderProgram(const char *vsPath, const char *fsPath);
  // rawString is dummy variable; its presence indicates use of actual source
  // vs. path to source
  ShaderProgram(const char *vsSource, const char *fsSource, bool rawString);

  // getters
  unsigned int id() const { return _glId; };

  void use();
  // overload this for all types
  void setUniform(const std::string &varName, int val);
  void setUniform(const std::string &varName, float val);
  void setUniform(const std::string &varName, float val1, float val2);
  void setUniform(const std::string &varName, glm::mat4 val);
  void setUniform(const std::string &varName, glm::vec3 val);
  void setUniform(const std::string &varName, glm::vec4 val);
  void setUniform(const std::string &varName, bool val);

private:
  unsigned int _glId;
  void compile(const char *vsSource, const char *fsSource, int &vsResult,
               int &fsResult, char vsLog[], char fsLog[]);
};

#endif