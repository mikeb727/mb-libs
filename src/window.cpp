
#include "window.h"
#include "vroot.h"
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>

#include <X11/CoreP.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/X.h>
#include <X11/keysym.h>

#include <X11/Xlib.h>

namespace GraphicsTools {

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
                               int x2, int y2) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawRectangle2D(color, x1, y1, x2, y2);
}

void WindowBase::drawCircle(GraphicsTools::ColorRgba color, float x, float y,
                            float r) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawCircle2D(color, x, y, r);
}

void WindowBase::drawCircleOutline(GraphicsTools::ColorRgba color, float x,
                                   float y, float r, float thickness) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawCircleOutline2D(color, x, y, r, thickness);
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
                          int width, GraphicsTools::TextAlignModeH al) {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  _sc->drawText2D(*font, text, color, x, y, width, al, 1);
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

Window::Window(std::string title, int width, int height)
    : WindowBase(title, width, height, Colors::Black) {

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

  _ready = true;
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

WindowGlx::WindowGlx(std::string title, int width, int height, int argc,
                     char **argv, bool isRoot)
    : WindowBase(title, width, height, Colors::Black) {

  if (isRoot) {
    std::cerr << "setting up a root window\n";
    XtAppContext xtac;
    Widget wgt = XtAppInitialize(&xtac, title.c_str(), nullptr, 0, &argc, argv,
                                 nullptr, nullptr, 0);
    std::cerr << "widget is " << wgt << "\n";
    if (!wgt){
      std::cerr << "widget is null!\n";
      return;
    }
    _disp = XtDisplay(wgt);
    std::cerr << "display is " << _disp << "\n";
    if (!_disp){
      std::cerr << "display is null!\n";
      return;
    }
    int glx_version = gladLoaderLoadGLX(_disp, XDefaultScreen(_disp));
    std::cerr << "glx version " << glx_version << " \n";
    if (glx_version == 0){
      std::cerr << "glx not loaded properly!\n";
      return;
    }
    _win = VirtualRootWindowOfScreen(XtScreen(wgt));
    std::cerr << "window is " << _win << "\n";
    if (!_win){
      std::cerr << "could not get root window!\n";
      return;
    }

    XtDestroyWidget(wgt);
    XWindowAttributes winAttrs;
    XGetWindowAttributes(_disp, _win, &winAttrs);
    XSelectInput(_disp, _win, winAttrs.your_event_mask | StructureNotifyMask);

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE, True, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, True,
        // GLX_SAMPLE_BUFFERS  , 1,
        // GLX_SAMPLES         , 4,
        None};
    // Get framebuffer config
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(_disp, DefaultScreen(_disp),
                                         visual_attribs, &fbcount);
    std::cerr << "fbcount is " << fbcount << " \n";                                  
    std::cerr << "fbc is " << fbc << " \n";                                  
    if (!fbc) {
      std::cerr << "couldn't get framebuffer config.\n";
      return;
    }

    // create context first, then init gl
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)glXGetProcAddress(
            (const GLubyte *)"glXCreateContextAttribsARB");
    int context_attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                             GLX_CONTEXT_MINOR_VERSION_ARB, 3, None};
    _context = glXCreateContextAttribsARB(_disp, fbc[0], nullptr, true,
                                          context_attribs);
    std::cerr << "context is " << _context << "\n";
    // Sync to ensure any errors generated are processed.
    glXMakeCurrent(_disp, _win, _context);
    int gl_version = gladLoaderLoadGL();
    std::cerr << "opengl version " << gl_version << " \n";

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree(fbc);
    XSync(_disp, false);
    // Set viewport to parent window's width/height
    glViewport(0, 0, winAttrs.width, winAttrs.height);
    _ready = true;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  else {
    // set up some X stuff first
    _disp = XOpenDisplay(nullptr);
    if (!_disp) {
      std::cerr << "could not open X window.\n";
      return;
    }
    int screen = XDefaultScreen(_disp);
    // we can load GLX before creating a context
    int glx_version = gladLoaderLoadGLX(_disp, screen);

    _win = XCreateSimpleWindow(_disp, DefaultRootWindow(_disp), 0, 0, _width,
                               _height, 0, 0, 0);
    XMapWindow(_disp, _win);

    XWindowAttributes winAttrs;
    XGetWindowAttributes(_disp, _win, &winAttrs);

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE, True, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, True,
        // GLX_SAMPLE_BUFFERS  , 1,
        // GLX_SAMPLES         , 4,
        None};
    // Get framebuffer config
    int fbcount;
    GLXFBConfig *fbc =
        glXChooseFBConfig(_disp, screen, visual_attribs, &fbcount);
    if (!fbc) {
      std::cerr << "couldn't get framebuffer config.\n";
      return;
    }

    // create context first, then init gl
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)glXGetProcAddress(
            (const GLubyte *)"glXCreateContextAttribsARB");
    int context_attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                             GLX_CONTEXT_MINOR_VERSION_ARB, 3, None};
    _context = glXCreateContextAttribsARB(_disp, fbc[0], nullptr, true,
                                          context_attribs);
    // Sync to ensure any errors generated are processed.
    XSync(_disp, false);
    glXMakeCurrent(_disp, _win, _context);
    int gl_version = gladLoaderLoadGL();
    std::cerr << "opengl version " << gl_version << " \n";

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree(fbc);
    _ready = true;
    // Set viewport to parent window's width/height
    glViewport(0, 0, winAttrs.width, winAttrs.height);
  }
}

WindowGlx::~WindowGlx() { XFree(&_win); }

void WindowGlx::update() {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  glXSwapBuffers(_disp, _win);
  XSync(_disp, false);
  XFlush(_disp);
  glfwPollEvents();
}

void WindowGlx::clear() {
  if (!_ready) {
    std::cerr << "window is not initialized!\n";
    return;
  }
  glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _sc->resetDepth();
}

void WindowGlx::reshape() {
  XResizeWindow(_disp, _win, 600, 600);
}

} // namespace GraphicsTools
