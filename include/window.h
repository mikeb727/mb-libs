#ifndef WINDOW_H
#define WINDOW_H

#include "errors.h"
#include "scene.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <random>

#include <GLFW/glfw3.h>

namespace GraphicsTools {

class Window {
public:
  Window(std::string title, int width, int height);
  ~Window();

  // getters
  int width() const { return _width; };
  int height() const { return _height; };
  GLFWwindow *glfwWindow() const { return _win; };
  void *userPointer(std::string id) const { return _userPointers.at(id); };
  bool ready() const { return _ready; };
  ColorRgba clearColor() const { return _clearColor; };

  // setters
  void setUserPointer(std::string id, void *ptr);
  void setClearColor(ColorRgba c) { _clearColor = c; };

  // clear, then update to show graphics
  // keep clear, but make update responsibility of attached scene
  void clear();
  void update();
  bool shouldClose() const { return glfwWindowShouldClose(_win); };
  void setShouldClose(int close) { glfwSetWindowShouldClose(_win, close); };

  // drawing functions
  // make scene responsible for these
  void drawArrow(GraphicsTools::ColorRgba color, float x1, float y1, float x2,
                 float y2, float thickness);
  void drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1, int x2,
                     int y2);
  void drawCircle(GraphicsTools::ColorRgba color, float x, float y, float r);
  void drawCircleOutline(GraphicsTools::ColorRgba color, float x, float y,
                         float r, float thickness);
  void drawCircleGradient(GraphicsTools::ColorRgba outer,
                          GraphicsTools::ColorRgba inner, int x, int y, int r);
  void drawText(std::string str, GraphicsTools::Font *font,
                GraphicsTools::ColorRgba, int x, int y, int width,
                GraphicsTools::TextAlignModeH align =
                    GraphicsTools::TextAlignModeH::Left);
  void drawLine(GraphicsTools::ColorRgba color, int thickness, int x1, int y1,
                int x2, int y2);
  void drawMultiLine(GraphicsTools::ColorRgba color, int thickness,
                     int numPoints, float *points);

  // load, then draw to show an image
  // use our texture object from the opengl tutorial
  void drawImage(void *, int, int, int);

  void attachScene(Scene *sc) {
    _sc = sc;
    _sc->setWindowDimensions(_width, _height);
  };
  Scene *activeScene() const { return _sc; };

private:
  std::string _title;
  int _width, _height;
  GLFWwindow *_win;
  Scene *_sc;
  std::map<std::string, void *> _userPointers; // ??
  bool _ready;
  ColorRgba _clearColor;

  static void resizeFramebufferCallback(GLFWwindow *win, int w, int h);
};
} // namespace GraphicsTools

#endif