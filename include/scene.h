#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "font.h"
#include "light.h"
#include "renderObject.h"

#include <cmath>
#include <iostream>
#include <map>
#include <vector>

namespace GraphicsTools {

class Scene {
public:
  // ctor, dtor
  Scene();
  ~Scene();

  // manage elements
  void addRenderObject(RenderObject *obj);
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
  void setAlt2DShader(ShaderProgram *sp) { _shader2Alt = sp; };

  void setupShadows();
  void renderShadows() const;
  void render() const;

  // 2D drawing
  void drawText2D(GraphicsTools::Font font, std::string text,
                  GraphicsTools::ColorRgba textColor, float x0, float y0,
                  float width, GraphicsTools::TextAlignModeH alignment,
                  float scale);
  void drawCircle2D(GraphicsTools::ColorRgba color, float x, float y, float r);
  void drawCircleOutline2D(GraphicsTools::ColorRgba color, float x, float y,
                           float r, float thickness);
  void drawRectangle2D(GraphicsTools::ColorRgba color, float x1, float y1,
                       float x2, float y2);
  void drawLine2D(GraphicsTools::ColorRgba color, float thickness, float x1,
                  float y1, float x2, float y2);
  // specify points as [x1, y1, x2, y2, x3, y3, etc.]
  void drawMultiLine2D(GraphicsTools::ColorRgba color, float thickness,
                       int numPoints, float *points);
  void drawArrow2D(GraphicsTools::ColorRgba color, float x1, float y1, float x2,
                   float y2, float thickness);
  void drawAltShader2D();

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
  unsigned int _vao2, _vbo2;
  glm::mat4 _proj2;
  ShaderProgram *_shader2;
  ShaderProgram *_shader2Alt; // for 2D elements other than primitives
  float _depth;               // increment for every 2D element drawn

  // for rendering all 3D elements
  unsigned int _vao3, _vbo3;

  // for recomputing the text projection upon window resizing
  int _windowWidth, _windowHeight;
};

} // namespace GraphicsTools

#endif