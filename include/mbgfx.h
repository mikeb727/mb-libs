/* Functions for using graphics with OpenGL. Includes
    the OpenGL headers, so they don't have to be
    included in main. */

#ifndef IMAGES_H
#define IMAGES_H

#include "camera.h"
#include "errors.h"
#include "light.h"
#include "renderObject.h"
#include "shader.h"

#include <map>
#include <string>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <freetype/freetype.h>

class Scene;

namespace GraphicsTools {

// library initialization
int InitGraphics();
int CloseGraphics();

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

// data extracted from font using freetype
struct TextGlyph {
  unsigned int glTextureId;
  unsigned int sizeX, sizeY;
  int bearingX, bearingY;
  long charAdvance; // x distance to next glyph
};
typedef std::map<char, TextGlyph> charMap;

class Font {
private:
  int _size;
  std::string _filename;
  charMap _glyphs;
  bool _ready;

public:
  Font(std::string file, int displaySize); // size in points
  ~Font();
  TextGlyph glyph(char ch) const { return _glyphs.at(ch); };
  bool isReady() const { return _ready; };
  int size() const { return _size; };
};

class Scene {
public:
  // ctor, dtor
  Scene();
  ~Scene();

  // manage elements
  void addRenderObject(RenderObject *obj) { _objs.emplace(_nextObjId++, obj); };
  void removeRenderObject(int id) { _objs.erase(id); };
  void addCamera(Camera *cam) { _cameras.emplace(_nextCamId++, cam); };
  void removeCamera(int id) { _cameras.erase(id); };
  void setDirLight(DirectionalLight *light) { _dLight = light; };
  void setActiveCamera(int camId) { _activeCamId = camId; };

  // getters
  Camera *activeCamera() const;
  int windowWidth() const { return _windowWidth; };
  int windowHeight() const { return _windowHeight; };

  // setters
  void setWindowDimensions(int w, int h);
  void resetDepth() { _depth = -99.0f; };

  void setupShadows();
  void renderShadows();
  void render() const;

  // 2D drawing
  void drawText2D(GraphicsTools::Font font, std::string text,
                  GraphicsTools::ColorRgba textColor, float x0, float y0,
                  float width, float scale);
  void drawCircle2D(GraphicsTools::ColorRgba color, float x, float y, float r);
  void drawRectangle2D(GraphicsTools::ColorRgba color, float x1, float y1,
                       float x2, float y2);
  void drawLine2D(GraphicsTools::ColorRgba color, float thickness, float x1,
                  float y1, float x2, float y2);

private:
  // give each object a unique ID. possibly remove this and have external user
  // of the scene responsible for managing unique object IDs
  std::map<int, RenderObject *> _objs;
  int _nextObjId;

  std::map<int, Camera *> _cameras;
  int _nextCamId, _activeCamId;

  // only support a single directional light for simplicity. eventually use
  // deferred rendering to handle multiple lights/shadows
  DirectionalLight *_dLight;

  // for generating shadowmaps
  unsigned int _shadowFbo, _depthMap;
  glm::mat4 _lightView, _lightProj;
  ShaderProgram *_depthShader;

  // for rendering all 2D elements
  unsigned int _2DVao, _2DVbo;
  glm::mat4 _2DProj;
  ShaderProgram *_2DShader;
  float _depth; // increment for every 2D element drawn

  // for recomputing the text projection upon window resizing
  int _windowWidth, _windowHeight;
};

class Window {
public:
  Window(std::string title, int width, int height);
  ~Window();

  // getters
  int width() const { return _width; };
  int height() const { return _height; };

  // clear, then update to show graphics
  // keep clear, but make update responsibility of attached scene
  void clear();
  void update();
  bool shouldClose() const { return glfwWindowShouldClose(_win); };
  void setShouldClose(int close) { glfwSetWindowShouldClose(_win, close); };

  // drawing functions
  // make scene responsible for these
  void drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1, int x2,
                     int y2);
  void drawCircle(GraphicsTools::ColorRgba, float x, float y, float r);
  void drawCircleGradient(GraphicsTools::ColorRgba outer,
                          GraphicsTools::ColorRgba inner, int x, int y, int r);
  void drawText(std::string str, GraphicsTools::Font *font,
                GraphicsTools::ColorRgba, int x, int y,
                GraphicsTools::TextAlignModeH align =
                    GraphicsTools::TextAlignModeH::Left);
  void drawLine(GraphicsTools::ColorRgba color, int thickness, int x1, int y1,
                int x2, int y2);

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

  static void resizeFramebufferCallback(GLFWwindow *win, int w, int h);
};

} // namespace GraphicsTools

#endif
