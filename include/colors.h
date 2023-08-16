#ifndef COLORS_H
#define COLORS_H

#include <glm/glm.hpp>
#include <iostream>

namespace GraphicsTools {

// library-independent colors
// adapted from https://stackoverflow.com/questions/3018313/
// all components in [0, 1] unless otherwise specified
struct ColorRgba {
  float r;
  float g;
  float b;
  float a;
};
struct ColorHsva {
  float h; // angle in degrees
  float s;
  float v;
  float a;
};

namespace Colors {
const ColorRgba Black = {0, 0, 0, 1};
const ColorRgba Grey25 = {0.25, 0.25, 0.25, 1};
const ColorRgba Grey = {0.75, 0.75, 0.75, 1};
const ColorRgba White = {1, 1, 1, 1};
const ColorRgba Red = {1, 0, 0, 1};
const ColorRgba Blue = {0, 0, 1, 1};
const ColorRgba Green = {0, 1, 0, 1};
const ColorRgba Yellow = {1, 1, 0, 1};
} // namespace Colors

// color operations
ColorRgba blend(ColorRgba c1, float w1, ColorRgba c2, float w2);
ColorRgba hsv2rgb(ColorHsva in);
ColorRgba randomColor();
ColorRgba operator*(ColorRgba c, float m);
ColorRgba operator*(float m, ColorRgba c);

// for use in shaders
glm::vec4 colorToGlm(ColorRgba in);
glm::vec4 colorToGlm(ColorHsva in);

std::ostream &operator<<(std::ostream &os, ColorRgba c);
std::ostream &operator<<(std::ostream &os, ColorHsva c);

} // namespace GraphicsTools

#endif