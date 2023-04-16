#include "mbgfx.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>

namespace GraphicsTools {

int InitGraphics() {
  int result = glfwInit();
  if (!result) {
    const char *errLog;
    int errCode = glfwGetError(&errLog);
    std::cerr << "could not load GLFW: " << errLog << " (GLFW error " << errCode
              << ")\n";
    return result;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  return 1;
}

int CloseGraphics() {
  glfwTerminate();
  return 0;
}

ColorRgba blend(ColorRgba c1, double w1, ColorRgba c2, double w2) {
  double sumWeights = w1 + w2;
  ColorRgba result = {((w1 * c1.r + w2 * c2.r) / sumWeights),
                      ((w1 * c1.g + w2 * c2.g) / sumWeights),
                      ((w1 * c1.b + w2 * c2.b) / sumWeights),
                      ((w1 * c1.a + w2 * c2.a) / sumWeights)};
  return result;
}

// see https://stackoverflow.com/questions/3018313/
ColorRgba hsv2rgb(ColorHsv in) {
  double hh, p, q, t, ff;
  long i;
  ColorRgba out;

  // temp
  out.a = 1.0;

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
  std::uniform_real_distribution<double> hueDist(0, 360);
  std::normal_distribution<double> satDist(0.8, 0.2);
  std::normal_distribution<double> valDist(0.5, 0.2);
  double hue = hueDist(generator);
  double sat = satDist(generator);
  double val = valDist(generator);
  std::cerr << "[imageTools] hue " << hue << " sat " << sat << " val " << val
            << "\n";
  ColorRgba intermediate = GraphicsTools::hsv2rgb(GraphicsTools::ColorHsv{
      hue, std::clamp(sat, 0.0, 1.0), std::clamp(val, 0.0, 1.0)});
  return ColorRgba{(intermediate.r * 255), (intermediate.g * 255),
                   (intermediate.b * 255), 255};
}

Font::Font(std::string file, int fontSize) : _size(fontSize), _ready(false) {
  _filename = file;
  FT_Library ft;
  int result = FT_Init_FreeType(&ft);
  // load freetype for drawing text
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not load FreeType: " << errLog << " (FreeType error "
              << result << ")\n";
    return;
  }

  // load our font
  FT_Face font;
  result = FT_New_Face(ft, _filename.c_str(), 0, &font);
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not load font \"" << _filename << "\": " << errLog
              << " (FreeType error " << result << ")\n";
    return;
  }

  result = FT_Set_Pixel_Sizes(font, 0, _size);
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not set font pixel sizes: " << errLog
              << " (FreeType error " << result << ")\n";
    return;
  }

  // load glyph textures into memory
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (unsigned char c = 0; c < 128; ++c) {
    result = FT_Load_Char(font, c, FT_LOAD_RENDER);
    if (result) {
      const char *errLog = FT_Error_String(result);
      std::cerr << "could not load glyph for character \"" << c
                << "\": " << errLog << " (FreeType error " << result << ")\n";
      continue;
    }
    unsigned int glyphTex;
    glGenTextures(1, &glyphTex);
    glBindTexture(GL_TEXTURE_2D, glyphTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->glyph->bitmap.width,
                 font->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 font->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TextGlyph ch = {glyphTex,
                    font->glyph->bitmap.width,
                    font->glyph->bitmap.rows,
                    font->glyph->bitmap_left,
                    font->glyph->bitmap_top,
                    font->glyph->advance.x};
    _glyphs.emplace(c, ch);
  }
  FT_Done_Face(font);
  FT_Done_FreeType(ft);
  _ready = true;
}

Font::~Font() {}

Window::Window(std::string title, int width, int height)
    : _title(title), _width(width), _height(height) {

  // Initialize window
  _win = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
  if (!_win) {
    const char *errLog;
    int errCode = glfwGetError(&errLog);
    std::fprintf(stderr, "could not create window: %s (GLFW error %d)\n",
                 errLog, errCode);
    glfwTerminate();
  }
  glfwMakeContextCurrent(_win);
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    getGlErrors();
  }

  glfwSetWindowUserPointer(_win, this);
  glfwSetFramebufferSizeCallback(_win, resizeFramebufferCallback);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, _width, _height);
}

Window::~Window() { glfwDestroyWindow(_win); }

void Window::update() {
  glfwSwapBuffers(_win);
  glfwPollEvents();
}

void Window::clear() {
  glClearColor(0.0, 0.0, 0.0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _sc->resetDepth();
}

void Window::drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1,
                           int x2, int y2) {
  _sc->drawRectangle2D(color, x1, y1, x2, y2);
}

void Window::drawCircle(GraphicsTools::ColorRgba color, float x, float y,
                        float r) {
  _sc->drawCircle2D(color, x, y, r);
}

void Window::drawCircleGradient(GraphicsTools::ColorRgba outer,
                                GraphicsTools::ColorRgba inner, int x, int y,
                                int r) {}

void Window::drawText(std::string text, GraphicsTools::Font *font,
                      GraphicsTools::ColorRgba color, int x, int y,
                      GraphicsTools::TextAlignModeH al) {
  _sc->drawText2D(*font, text, color, x, y, -1, 1);
}

void Window::drawLine(GraphicsTools::ColorRgba color, int thickness, int x1,
                      int y1, int x2, int y2) {
  _sc->drawLine2D(color, thickness, x1, y1, x2, y2);
}

void Window::resizeFramebufferCallback(GLFWwindow *win, int w, int h) {
  Window *gfxWin = (Window *)glfwGetWindowUserPointer(win);
  Scene *_sc = gfxWin->activeScene();
  glViewport(0, 0, w, h);
  if (_sc) {
    _sc->setWindowDimensions(w, h);
  }
}

} // namespace GraphicsTools
