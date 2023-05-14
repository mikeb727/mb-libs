#include "light.h"

#include <iostream>

namespace GraphicsTools {

void PointLight::setDistance(float dist) {
  // assuming proportions:
  //   linear: l = k1/dist; k1 = 4.9
  //   quadratic: q = (k2/dist)^2; k2 = 8.6
  _attenuationD0 = 1.0f;
  _attenuationD1 = 4.9f / dist;
  _attenuationD2 = pow(8.6f / dist, 2);
}

void PointLight::debugPrint(std::ostream &out) {
  out << "point light pos " << _pos.x << " " << _pos.y << " " << _pos.z << "\n"
      << "ambient " << _ambientColor << "\n"
      << "diffuse " << _diffuseColor << "\n"
      << "specular " << _specularColor << "\n";
}
} // namespace GraphicsTools