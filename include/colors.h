#ifndef COLORS_H
#define COLORS_H

#include <glm/glm.hpp>

namespace GraphicsTools {

// library-independent colors
// adapted from https://stackoverflow.com/questions/3018313/
struct ColorRgba {
  float r; // a fraction between 0 and 1
  float g; // a fraction between 0 and 1
  float b; // a fraction between 0 and 1
  float a; // a fraction between 0 and 1
};
struct ColorHsva {
  float h; // angle in degrees
  float s; // a fraction between 0 and 1
  float v; // a fraction between 0 and 1
  float a; // a fraction between 0 and 1
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
ColorRgba randomColor();
ColorRgba hsv2rgb(ColorHsva in);
// for use in shaders
glm::vec4 colorToGlm(ColorRgba in);
glm::vec4 colorToGlm(ColorHsva in);

} // namespace GraphicsTools

#endif