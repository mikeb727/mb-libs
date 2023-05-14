/* Functions for using graphics with OpenGL. Includes
    the OpenGL headers, so they don't have to be
    included in main. */

#ifndef IMAGES_H
#define IMAGES_H

#include "camera.h"
#include "colors.h"
#include "errors.h"
#include "light.h"
#include "shader.h"

#include <map>
#include <string>
#include <vector>

#include "glad/gl.h"
#include "texture.h"
#include <GLFW/glfw3.h>

#include <freetype/freetype.h>

namespace GraphicsTools {

// library setup/teardown
int InitGraphics();
int CloseGraphics();

enum TextAlignModeH { Left, Center, Right };

struct Material {
  ColorRgba diffuse;
  Texture *diffuseMap;
  ColorRgba specular;
  float shininess;
};

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

class RenderObject {
public:
  // ctor, dtor
  RenderObject();
  ~RenderObject();

  // getters
  glm::vec3 pos() const { return _pos; };
  unsigned int vao() const { return _vao; };
  unsigned int vbo() const { return _vbo; };
  const std::vector<float> vertexData() const { return _vData; };

  // setters
  void setPos(glm::vec3 newPos);
  void setRotation(glm::vec3 eulerAngles);
  void setShader(ShaderProgram *prog) { _sp = prog; };
  // do this before creating geometry if using texture!
  void setTexture(Texture *tex) { _material.diffuseMap = tex; };
  void setMaterial(Material mat) { _material = mat; };
  void setVao(unsigned int vao) { _vao = vao; };
  void setVbo(unsigned int vbo) { _vbo = vbo; };

  // generate geometry
  void genSphere(float radius, int numLatSegments, int numLonSegments);
  void genCube(float sideLength);
  void genPlane(float width, float depth);
  void genTorus(float majorRadius, float minorRadius, int numMinorSegments,
                int numMajorSegments);

  // draw object with OpenGL functions
  void draw(glm::mat4 viewMat, glm::mat4 projMat, glm::mat4 lightMat,
            ShaderProgram *overrideShader = NULL);

  // debug
  void debugPrint();

private:
  glm::vec3 _pos;
  glm::vec3 _rot;
  glm::mat4 _modelMat; // model matrix
  glm::mat4 _normalMat;
  std::vector<float> _vData; // interleaved positions, normals, and optional
                             // texture coordinates for OpenGL
  unsigned int _vDataWidth;  // number of floats to define a vertex; 8 if
                             // textures are used, 6 otherwise
  unsigned int _vao, _vbo;   // vertex array and buffer (OpenGL)
  ShaderProgram *_sp;
  Material _material; // texture is now part of material (as diffuse map)

  void recalc(glm::mat4 viewMat);
  void setupGl();
};

class Scene {
public:
  // ctor, dtor
  Scene();
  ~Scene();

  // manage elements
  void addRenderObject(RenderObject *obj) {
    _objs.emplace(_nextObjId++, obj);
    obj->setVao(_3DVao);
    obj->setVbo(_3DVbo);
  };
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
  void setShadows(bool s) { _useShadows = s; };

  void setupShadows();
  void renderShadows() const;
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
  void drawArrow2D(GraphicsTools::ColorRgba color, float x1, float y1, float x2, float y2, float thickness);


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
  bool _useShadows;
  unsigned int _shadowFbo, _depthMap;
  glm::mat4 _lightView, _lightProj;
  ShaderProgram *_depthShader;

  // for rendering all 2D elements
  unsigned int _2DVao, _2DVbo;
  glm::mat4 _2DProj;
  ShaderProgram *_2DShader;
  float _depth; // increment for every 2D element drawn

  // for rendering all 3D elements
  unsigned int _3DVao, _3DVbo;

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
  void drawArrow(GraphicsTools::ColorRgba color, float x1, float y1, float x2, float y2, float thickness);
  void drawRectangle(GraphicsTools::ColorRgba color, int x1, int y1, int x2,
                     int y2);
  void drawCircle(GraphicsTools::ColorRgba, float x, float y, float r);
  void drawCircleGradient(GraphicsTools::ColorRgba outer,
                          GraphicsTools::ColorRgba inner, int x, int y, int r);
  void drawText(std::string str, GraphicsTools::Font *font,
                GraphicsTools::ColorRgba, int x, int y, int width,
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
  std::map<std::string, void *> _userPointers; // ??
  bool _ready;
  ColorRgba _clearColor;

  static void resizeFramebufferCallback(GLFWwindow *win, int w, int h);
};

} // namespace GraphicsTools

#endif
