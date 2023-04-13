/* Functions for using graphics with SDL. Includes
    the OpenGL headers, so they don't have to be
    included in main. */

#ifndef IMAGES_H
#define IMAGES_H

#include <string>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

namespace GraphicsTools {

// library initialization
int InitGraphics();
int InitText();
int CloseGraphics();
int CloseText();

enum TextAlignModeH { Left, Center, Right };

// library-independent colors
// adapted from https://stackoverflow.com/questions/3018313/
struct ColorRgba {
  double r; // a fraction between 0 and 1
  double g; // a fraction between 0 and 1
  double b; // a fraction between 0 and 1
  double a;
};
struct ColorHsv {
  double h; // angle in degrees
  double s; // a fraction between 0 and 1
  double v; // a fraction between 0 and 1
};

namespace Colors {
const ColorRgba Black = {0, 0, 0, 255};
const ColorRgba Grey25 = {64, 64, 64, 255};
const ColorRgba Grey = {192, 192, 192, 255};
const ColorRgba White = {255, 255, 255, 255};
const ColorRgba Red = {255, 0, 0, 255};
const ColorRgba Blue = {0, 0, 255, 255};
const ColorRgba Green = {0, 255, 0, 255};
const ColorRgba Yellow = {255, 255, 0, 255};
} // namespace Colors

// color operations
ColorRgba blend(ColorRgba c1, double w1, ColorRgba c2, double w2);
ColorRgba randomColor();
ColorRgba hsv2rgb(ColorHsv in);

class Font {
private:
  int size;
  std::string filename;
  TTF_Font *f;

public:
  Font(const char *file, int displaySize); // size in points
  ~Font();
  TTF_Font *font() const { return f; };
};

class Window {
public:
  Window(std::string title, int width, int height);
  ~Window();

  // getters
  int height() const { return _height; };
  int width() const { return _width; };

  // clear, then update to show graphics
  void clear();
  void update();

  // drawing functions
  void drawRectangle(GraphicsTools::ColorRgba color, int x, int y, int w,
                     int h); // (x,y) is upper-left corner
  void drawCircle(GraphicsTools::ColorRgba, int x, int y, int r);
  void drawCircleGradient(GraphicsTools::ColorRgba outer,
                          GraphicsTools::ColorRgba inner, int x, int y, int r);
  void drawText(std::string str, GraphicsTools::Font *font,
                GraphicsTools::ColorRgba, int x, int y,
                GraphicsTools::TextAlignModeH align =
                    GraphicsTools::TextAlignModeH::Left);
  void drawLine(GraphicsTools::ColorRgba color, int thickness, int x1, int y1,
                int x2, int y2);

  // load, then draw to show an image
  SDL_Texture *loadImage(std::string filename);
  void drawImage(SDL_Texture *, int, int, int);

private:
  SDL_Renderer *ren;
  std::string name;
  int _height, _width;
  SDL_Window *win;
};

} // namespace GraphicsTools

#endif
