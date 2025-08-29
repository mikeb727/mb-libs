
#include "window.h"
#include "colors.h"
#include "errors.h"

namespace GraphicsTools {

WindowType operator|(WindowType lv, WindowType rv) {
  return WindowType(int(lv) | int(rv));
}

WindowBase::WindowBase(std::string title, int width, int height,
                       ColorRgba clearColor)
    : _title(title), _width(width), _height(height), _ready(false),
      _clearColor(clearColor) {}

WindowBase::~WindowBase() {}

void WindowBase::setUserPointer(std::string id, void *ptr) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  if (ptr) {
    _userPointers.emplace(id, ptr);
  } else {
    std::cerr << "warning: provided user pointer for key \"" << id
              << "\" is null!\n";
  }
};

void WindowBase::drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1,
                               int x2, int y2, float angle) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawRectangle2D(color, x1, y1, x2, y2, angle);
}

void WindowBase::drawCircle(GraphicsTools::ColorRgba color, float x, float y,
                            float r, float angle,
                            ShaderProgram *overrideShader) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawCircle2D(color, x, y, r, angle, overrideShader);
}

void WindowBase::drawCircleOutline(GraphicsTools::ColorRgba color, float x,
                                   float y, float r, float angle,
                                   float thickness) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawCircleOutline2D(color, x, y, r, angle, thickness);
}

void WindowBase::drawCircleGradient(GraphicsTools::ColorRgba outer,
                                    GraphicsTools::ColorRgba inner, int x,
                                    int y, int r) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
}

void WindowBase::drawText(std::string text, GraphicsTools::Font *font,
                          GraphicsTools::ColorRgba color, int x, int y,
                          float angle, int width,
                          GraphicsTools::TextAlignModeH al) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawText2D(*font, text, color, x, y, angle, width, al, 1);
}

void WindowBase::drawLine(GraphicsTools::ColorRgba color, int thickness, int x1,
                          int y1, int x2, int y2) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawLine2D(color, thickness, x1, y1, x2, y2);
}

void WindowBase::drawMultiLine(GraphicsTools::ColorRgba color, int thickness,
                               int numPoints, float *points) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawMultiLine2D(color, thickness, numPoints, points);
}

void WindowBase::drawArrow(GraphicsTools::ColorRgba color, float x1, float y1,
                           float x2, float y2, float thickness) {
  _sc->drawArrow2D(color, x1, y1, x2, y2, thickness);
}

void WindowBase::drawMultiArrow(GraphicsTools::ColorRgba color, float thickness,
                                int numPoints, float *points) {
  _sc->drawMultiArrow2D(color, thickness, numPoints, points);
}

Window::Window(std::string title, int width, int height, WindowType windowFlags)
    : WindowBase(title, width, height, Colors::Black) {

  // Initialize window
  glfwWindowHint(GLFW_DECORATED, !(windowFlags & WindowType::NoDecoration));
  glfwWindowHint(GLFW_FLOATING, (windowFlags & WindowType::AlwaysOnTop));

  if (windowFlags & WindowType::WindowedFullscreen) {
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    _width = mode->width;
    _height = mode->height;
  }

  GLFWmonitor *fullscreenMonitor = (windowFlags & WindowType::WindowedFullscreen) ? glfwGetPrimaryMonitor() : NULL;

  _win = glfwCreateWindow(_width, _height, _title.c_str(), fullscreenMonitor, NULL);
  _ready = setupGlfwWindow();
}

Window::~Window() { glfwDestroyWindow(_win); }

void Window::update() {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  glfwSwapBuffers(_win);
  glfwPollEvents();
}

void Window::getEvents() { glfwPollEvents(); }

void Window::clear() {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _sc->resetDepth();
}

void Window::resizeFramebufferCallback(GLFWwindow *win, int w, int h) {
  Window *gfxWin = (Window *)glfwGetWindowUserPointer(win);
  Scene *_sc = gfxWin->activeScene();
  glViewport(0, 0, w, h);
  gfxWin->_width = w;
  gfxWin->_height = h;
  if (_sc) {
    _sc->setWindowDimensions(w, h);
    Camera *cam = _sc->activeCamera();
    if (cam) {
      if (cam->projection() == CameraProjType::Perspective) {
        cam->setPerspective(cam->fov(), float(w) / float(h));
      }
    }
  }
}

bool Window::setupGlfwWindow() {
  if (!_win) {
    const char *errLog;
    int errCode = glfwGetError(&errLog);
    std::fprintf(stderr, "could not create window: %s (GLFW error %d)\n",
                 errLog, errCode);
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(_win);
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    getGlErrors();
    return false;
  }

  glfwSetWindowUserPointer(_win, this);
  glfwSetFramebufferSizeCallback(_win, resizeFramebufferCallback);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glfwGetFramebufferSize(_win, &_width, &_height);
  glViewport(0, 0, _width, _height);

  return true;
}

} // namespace GraphicsTools
