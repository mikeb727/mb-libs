#ifndef MATERIAL_H
#define MATERIAL_H

#include "colors.h"
#include "texture.h"

namespace GraphicsTools {

struct Material {
  ColorRgba diffuse;
  Texture *diffuseMap;
  ColorRgba specular;
  float shininess;
};

} // namespace GraphicsTools

#endif