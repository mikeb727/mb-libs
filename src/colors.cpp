#include "colors.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

namespace GraphicsTools {

ColorRgba blend(ColorRgba c1, float w1, ColorRgba c2, float w2) {
  float sumWeights = w1 + w2;
  ColorRgba result = {((w1 * c1.r + w2 * c2.r) / sumWeights),
                      ((w1 * c1.g + w2 * c2.g) / sumWeights),
                      ((w1 * c1.b + w2 * c2.b) / sumWeights),
                      ((w1 * c1.a + w2 * c2.a) / sumWeights)};
  return result;
}

// see https://stackoverflow.com/questions/3018313/
ColorRgba hsv2rgb(ColorHsva in) {
  double hh, p, q, t, ff;
  long i;
  ColorRgba out;

  out.a = in.a;

  if (in.s <= 0.0) { // < is bogus, just shuts up warnings
    out.r = in.v;
    out.g = in.v;
    out.b = in.v;
    return out;
  }
  hh = in.h;
  if (hh >= 360.0)
    hh = 0.0;
  hh /= 60.0;
  i = (long)hh;
  ff = hh - i;
  p = in.v * (1.0 - in.s);
  q = in.v * (1.0 - (in.s * ff));
  t = in.v * (1.0 - (in.s * (1.0 - ff)));

  switch (i) {
  case 0:
    out.r = in.v;
    out.g = t;
    out.b = p;
    break;
  case 1:
    out.r = q;
    out.g = in.v;
    out.b = p;
    break;
  case 2:
    out.r = p;
    out.g = in.v;
    out.b = t;
    break;
  case 3:
    out.r = p;
    out.g = q;
    out.b = in.v;
    break;
  case 4:
    out.r = t;
    out.g = p;
    out.b = in.v;
    break;
  case 5:
  default:
    out.r = in.v;
    out.g = p;
    out.b = q;
    break;
  }
  return out;
}

ColorRgba randomColor() {
  std::default_random_engine generator;
  generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> hueDist(0, 360);
  std::normal_distribution<float> satDist(0.8, 0.2);
  std::normal_distribution<float> valDist(0.5, 0.2);
  float hue = hueDist(generator);
  float sat = satDist(generator);
  float val = valDist(generator);
  std::cerr << "[imageTools] hue " << hue << " sat " << sat << " val " << val
            << "\n";
  ColorRgba intermediate = GraphicsTools::hsv2rgb(
      GraphicsTools::ColorHsva{hue, std::clamp<float>(sat, 0.0, 1.0),
                               std::clamp<float>(val, 0.0, 1.0), 1.0});
  return ColorRgba{(intermediate.r), (intermediate.g), (intermediate.b), 1.0};
}

glm::vec4 colorToGlm(ColorRgba in) { return glm::vec4(in.r, in.g, in.b, in.a); }

glm::vec4 colorToGlm(ColorHsva in) {
  ColorRgba rgba = hsv2rgb(in);
  return glm::vec4(rgba.r, rgba.g, rgba.b, rgba.a);
}

ColorRgba operator*(ColorRgba c, float m) {
  return ColorRgba({c.r * m, c.g * m, c.b * m, c.a * m});
}
ColorRgba operator*(float m, ColorRgba c) {
  return ColorRgba({c.r * m, c.g * m, c.b * m, c.a * m});
}

} // namespace GraphicsTools