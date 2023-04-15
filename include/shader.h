#ifndef SHADER_H
#define SHADER_H

#include "glad/gl.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <string>

class ShaderProgram {

public:
  ShaderProgram(const char *vsPath, const char *fsPath);
  void use();
  // overload this for all types
  void setUniform(const std::string &varName, int val);
  void setUniform(const std::string &varName, float val);
  void setUniform(const std::string &varName, float val1, float val2);
  void setUniform(const std::string &varName, glm::mat4 val);
  void setUniform(const std::string &varName, glm::vec3 val);
  void setUniform(const std::string &varName, glm::vec4 val);
  unsigned int id() const { return spId; };

private:
  unsigned int spId;
};

#endif