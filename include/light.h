#ifndef LIGHT_H
#define LIGHT_H

#include "colors.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <iostream>

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

  void setDistance(float dist) {
    // assuming proportions:
    //   linear: l = k1/dist; k1 = 4.9
    //   quadratic: q = (k2/dist)^2; k2 = 8.6
    _attenuationD0 = 1.0f;
    _attenuationD1 = 4.9f / dist;
    _attenuationD2 = pow(8.6f / dist, 2);
  }
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