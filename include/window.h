#ifndef WINDOW_H
#define WINDOW_H

#include "colors.h"
#include "scene.h"

#include <map>

#include <GLFW/glfw3.h>

namespace GraphicsTools {

enum WindowType {
  Null = 0x00,
  AlwaysOnTop = 0x01,
  NoDecoration = 0x02,
  WindowedFullscreen = 0x04,
  ScreensaverMode = AlwaysOnTop | NoDecoration | WindowedFullscreen,
};

WindowType operator|(WindowType lv, WindowType rv);

class WindowBase {
public:
  virtual ~WindowBase() = 0;

  // getters
  int width() const { return _width; };
  int height() const { return _height; };
  void *userPointer(std::string id) const { return _userPointers.at(id); };
  bool ready() const { return _ready; };
  ColorRgba clearColor() const { return _clearColor; };

  // setters
  void setUserPointer(std::string id, void *ptr);
  void setClearColor(ColorRgba c) { _clearColor = c; };

  // clear, then update to show graphics
  // keep clear, but make update responsibility of attached scene
  virtual void clear() = 0;
  virtual void update() = 0;
  virtual void getEvents() = 0;
  virtual bool shouldClose() const = 0;
  virtual void setShouldClose(int close) = 0;

  // drawing functions. these are the same for both GLFW and X11
  // make scene responsible for these
  void drawArrow(GraphicsTools::ColorRgba color, float x1, float y1, float x2,
                 float y2, float thickness);
  void drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1, int x2,
                     int y2, float angle);
  void drawCircle(GraphicsTools::ColorRgba color, float x, float y, float r,
                  float angle, ShaderProgram *overrideShader = NULL);
  void drawCircleOutline(GraphicsTools::ColorRgba color, float x, float y,
                         float angle, float r, float thickness);
  void drawCircleGradient(GraphicsTools::ColorRgba outer,
                          GraphicsTools::ColorRgba inner, int x, int y, int r);
  void drawText(std::string str, GraphicsTools::Font *font,
                GraphicsTools::ColorRgba, int x, int y, float angle, int width,
                GraphicsTools::TextAlignModeH align =
                    GraphicsTools::TextAlignModeH::Left);
  void drawLine(GraphicsTools::ColorRgba color, int thickness, int x1, int y1,
                int x2, int y2);
  void drawMultiLine(GraphicsTools::ColorRgba color, int thickness,
                     int numPoints, float *points);
  void drawMultiArrow(GraphicsTools::ColorRgba color, float thickness,
                      int numPoints, float *points);

  // load, then draw to show an image
  // use our texture object from the opengl tutorial
  // virtual void drawImage(void *, int, int, int) = 0;

  void attachScene(Scene *sc) {
    _sc = sc;
    _sc->setWindowDimensions(_width, _height);
  };
  Scene *activeScene() const { return _sc; };

protected:
  WindowBase(std::string title, int width, int height, ColorRgba clearColor);
  std::string _title;
  int _width, _height;
  Scene *_sc;
  std::map<std::string, void *> _userPointers; // ??
  bool _ready;
  ColorRgba _clearColor;
};

// GLFW window
class Window : public WindowBase {
public:
  Window(std::string title, int width, int height,
         WindowType windowFlags = WindowType::Null);
  ~Window();

  // getters
  GLFWwindow *glfwWindow() const { return _win; };

  // setters defined in WindowBase

  // clear, then update to show graphics
  // keep clear, but make update responsibility of attached scene
  void clear();
  void update();
  // separate function for polling events (previously buffer swap made this
  // depend on vblank/refresh rate)
  void getEvents();
  bool shouldClose() const { return glfwWindowShouldClose(_win); };
  void setShouldClose(int close) { glfwSetWindowShouldClose(_win, close); };

  // load, then draw to show an image
  // use our texture object from the opengl tutorial
  void drawImage(void *, int, int, int);

private:
  GLFWwindow *_win;
  // tasks common to normal and screensaver windows
  bool setupGlfwWindow();
  static void resizeFramebufferCallback(GLFWwindow *win, int w, int h);
};

} // namespace GraphicsTools

#endif