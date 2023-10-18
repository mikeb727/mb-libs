#ifndef LIGHT_H
#define LIGHT_H

#include "colors.h"
#include "shader.h"

#include <iostream>

#include <glm/glm.hpp>

namespace GraphicsTools {

struct PointLight {
  glm::vec3 _pos;
  ColorRgba _ambientColor;
  ColorRgba _diffuseColor;
  ColorRgba _specularColor;

  float _attenuationD0;
  float _attenuationD1;
  float _attenuationD2;

  ShaderProgram *sp;

  void setDistance(float dist);
  void debugPrint(std::ostream &out = std::cerr);
};

struct DirectionalLight {
  glm::vec3 _dir;
  ColorRgba _ambientColor;
  ColorRgba _diffuseColor;
  ColorRgba _specularColor;
  ShaderProgram *sp;
};

} // namespace GraphicsTools

#endif