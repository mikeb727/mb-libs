#include "glad/gl.h"
#include "mbgfx.h"
#include "shader.h"

#include <cmath>
#include <iostream>
#include <vector>

const int SHADOW_TEX_SIZE = 4096;
const float SHADOW_FRUSTUM_DEPTH = 100;
const float SHADOW_FRUSTUM_WIDTH = 60;

const int VBO_2D_MAX_SIZE = 4000;
const int VBO_3D_MAX_SIZE = 40000;
const int VAO_3D_DATA_WIDTH = 8;

const int CIRCLE_2D_RESOLUTION = 100;

namespace GraphicsTools {

Scene::Scene()
    : _windowWidth(0), _windowHeight(0), _nextCamId(0), _nextObjId(0),
      _activeCamId(-1), _dLight(NULL), _depth(-99.0f), _useShadows(false) {

  glGenVertexArrays(1, &_vao2);
  glGenBuffers(1, &_vbo2);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferData(GL_ARRAY_BUFFER, VBO_2D_MAX_SIZE * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &_vao3);
  glGenBuffers(1, &_vbo3);
  glBindVertexArray(_vao3);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo3);
  glBufferData(GL_ARRAY_BUFFER, VBO_3D_MAX_SIZE * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                        VAO_3D_DATA_WIDTH * sizeof(float), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                        VAO_3D_DATA_WIDTH * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                        VAO_3D_DATA_WIDTH * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  _depthShader = NULL;
  _shader2 = new ShaderProgram("assets/2d_vs.glsl", "assets/2d_fs.glsl");
  _shader2Alt = NULL;
}

Scene::~Scene() {
  delete _depthShader;
  delete _shader2;
}

Camera *Scene::activeCamera() const {
  return _activeCamId != -1 ? _cameras.at(_activeCamId) : NULL;
};

void Scene::setWindowDimensions(int w, int h) {
  _windowWidth = w;
  _windowHeight = h;
  // careful! z-axis is reversed in orthographic projection
  _proj2 = glm::ortho(0.0f, (float)_windowWidth, 0.0f, (float)_windowHeight,
                      -1000.0f, 1000.0f);
};

void Scene::setupShadows() {
  // set up framebuffer for shadows
  glGenFramebuffers(1, &_shadowFbo);
  // set up shadow map texture
  glGenTextures(1, &_depthMap);
  glBindTexture(GL_TEXTURE_2D, _depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_TEX_SIZE,
               SHADOW_TEX_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glBindFramebuffer(GL_FRAMEBUFFER, _shadowFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         _depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // create light space matrices
  _lightView = glm::lookAt(-30.0f * _dLight->_dir, glm::zero<glm::vec3>(),
                           glm::vec3(0.0f, 1.0f, 0.001f));
  _lightProj = glm::ortho(-SHADOW_FRUSTUM_WIDTH, SHADOW_FRUSTUM_WIDTH,
                          -SHADOW_FRUSTUM_WIDTH, SHADOW_FRUSTUM_WIDTH, 0.01f,
                          SHADOW_FRUSTUM_DEPTH);
  _depthShader =
      new ShaderProgram("assets/depth_vs.glsl", "assets/depth_fs.glsl");
  _useShadows = true;
}

void Scene::renderShadows() const {
  if (_dLight) {
    glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, _shadowFbo);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_NONE);
    for (auto &obj : _objs) {
      obj.second->draw(_lightView, _lightProj, _lightView, _depthShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void Scene::render() const {
  if (_useShadows) {
    renderShadows();
  }
  glCullFace(GL_BACK);
  glViewport(0, 0, _windowWidth, _windowHeight);
  if (_activeCamId != -1 && _dLight) {
    _dLight->sp->use();
    _dLight->sp->setUniform("dirLight.dir", _dLight->_dir);
    _dLight->sp->setUniform("dirLight.ambient",
                            colorToGlm(_dLight->_ambientColor));
    _dLight->sp->setUniform("dirLight.diffuse",
                            colorToGlm(_dLight->_diffuseColor));
    _dLight->sp->setUniform("dirLight.specular",
                            colorToGlm(_dLight->_specularColor));
    _dLight->sp->setUniform("viewPos", activeCamera()->pos());
    if (_useShadows) {
      glActiveTexture(GL_TEXTURE1); // unit 0 is reserved for object textures
      glBindTexture(GL_TEXTURE_2D, _depthMap);
      _dLight->sp->setUniform("shadowMap", 1);
    }
    for (auto &obj : _objs) {
      obj.second->draw(activeCamera()->viewMatrix(),
                       activeCamera()->projMatrix(), _lightProj * _lightView);
    }
  }
}

void Scene::drawText2D(Font font, std::string str, ColorRgba color, float x0,
                       float y0, float width,
                       GraphicsTools::TextAlignModeH alignment,
                       float drawScale) {
  glClear(GL_DEPTH_BUFFER_BIT);
  _shader2->use();
  _shader2->setUniform("color", glm::vec4(color.r, color.g, color.b, 1.0));
  _shader2->setUniform("transform", _proj2);
  glActiveTexture(GL_TEXTURE0);
  _shader2->setUniform("tex", 0);
  _shader2->setUniform("useTex", 1);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);

  float x = x0, y = y0;
  // two iterators; one for drawing glyphs left to right, one for computing x
  // offset for center- and right-aligned text
  std::string::const_iterator ch;
  for (ch = str.begin(); ch != str.end(); ++ch) {
    if (x == x0 && alignment == Right) {
      std::string::const_iterator chAlignCalcH = ch;
      while (x > (x0 - width) && ch != str.end()) {
        x -= (font.glyph(*ch).charAdvance >> 6) * drawScale;
        std::cerr << x << std::endl;
        ++chAlignCalcH;
      }
    }
    if (*ch == '\n') {
      x = x0;
      y -= font.size() * drawScale;
      continue;
    }
    TextGlyph tch(font.glyph(*ch));
    if ((width != -1) && ((x + tch.bearingX + tch.sizeX - x0) > width)) {
      x = x0;
      y -= font.size() * drawScale;
    }
    float quadX = (x + tch.bearingX) * drawScale;
    float quadY = (y + tch.bearingY - tch.sizeY) * drawScale;
    float quadW = tch.sizeX * drawScale;
    float quadH = tch.sizeY * drawScale;
    float verts[6][4] = {{quadX, quadY, 0.0f, 1.0f},
                         {quadX + quadW, quadY, 1.0f, 1.0f},
                         {quadX + quadW, quadY + quadH, 1.0f, 0.0f},
                         {quadX + quadW, quadY + quadH, 1.0f, 0.0f},
                         {quadX, quadY + quadH, 0.0f, 0.0f},
                         {quadX, quadY, 0.0f, 1.0f}};
    glBindTexture(GL_TEXTURE_2D, tch.glTextureId);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    x += (tch.charAdvance >> 6) * drawScale;
  }
  glBindVertexArray(0);
  _depth += 1.0f;
}

void Scene::drawCircle2D(ColorRgba color, float x, float y, float r) {
  std::vector<float> verts_v;
  verts_v.push_back(x);
  verts_v.push_back(y);
  verts_v.push_back(0.0f);
  verts_v.push_back(0.0f);
  for (int i = 0; i < CIRCLE_2D_RESOLUTION + 1; ++i) {
    float angle = (360.0f * i / (float)CIRCLE_2D_RESOLUTION) * (M_PI / 180.0f);
    verts_v.push_back(x + (r * cos(angle)));
    verts_v.push_back(y + (r * sin(angle)));
    verts_v.push_back(0.0f);
    verts_v.push_back(0.0f);
  }
  float *verts = verts_v.data();
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawCircleOutline2D(GraphicsTools::ColorRgba color, float x,
                                float y, float r, float thickness) {
  std::vector<float> verts_v;
  for (int i = 0; i < CIRCLE_2D_RESOLUTION + 1; ++i) {
    float angle = (360.0f * i / (float)CIRCLE_2D_RESOLUTION) * (M_PI / 180.0f);
    verts_v.push_back(x + ((r - 0.25f * thickness) * cos(angle)));
    verts_v.push_back(y + ((r - 0.25f * thickness) * sin(angle)));
    verts_v.push_back(0.0f);
    verts_v.push_back(0.0f);
  }
  float *verts = verts_v.data();
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glLineWidth(thickness);
  glDrawArrays(GL_LINE_STRIP, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawRectangle2D(ColorRgba color, float x1, float y1, float x2,
                            float y2) {
  std::vector<float> verts_v = {x1, y1, 0.0f, 0.0f, x2, y1, 0.0f, 0.0f,
                                x2, y2, 0.0f, 0.0f, x2, y2, 0.0f, 0.0f,
                                x1, y2, 0.0f, 0.0f, x1, y1, 0.0f, 0.0f};
  float *verts = verts_v.data();
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawLine2D(ColorRgba color, float thickness, float x1, float y1,
                       float x2, float y2) {
  std::vector<float> verts_v = {x1, y1, 0.0f, 0.0f, x2, y2, 0.0f, 0.0f};
  float *verts = verts_v.data();
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glLineWidth(thickness);
  glDrawArrays(GL_LINES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawMultiLine2D(GraphicsTools::ColorRgba color, float thickness,
                            int numPoints, float *points) {
  std::vector<float> verts_v;
  for (int v = 0; v < numPoints; ++v) {
    verts_v.push_back(points[2 * v]);
    verts_v.push_back(points[2 * v + 1]);
    verts_v.push_back(0.0f);
    verts_v.push_back(0.0f);
  }
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float),
                  verts_v.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glLineWidth(thickness);
  glDrawArrays(GL_LINE_STRIP, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawArrow2D(GraphicsTools::ColorRgba color, float x1, float y1,
                        float x2, float y2, float thickness) {
  using std::sin, std::cos, std::atan2;
  float direction = atan2(y2 - y1, x2 - x1);
  float perp = direction + (M_PI / 2.0);
  float headHeight = sqrt(3) * thickness;
  std::vector<float> verts_v = {
      x1 + (0.5f * thickness * cos(perp)),
      y1 + (0.5f * thickness * sin(perp)),
      0,
      0,
      x1 - (0.5f * thickness * cos(perp)),
      y1 - (0.5f * thickness * sin(perp)),
      0,
      0,
      x2 - (headHeight * cos(direction)) - (0.5f * thickness * cos(perp)),
      y2 - (headHeight * sin(direction)) - (0.5f * thickness * sin(perp)),
      0,
      0,
      x2 - (headHeight * cos(direction)) - (0.5f * thickness * cos(perp)),
      y2 - (headHeight * sin(direction)) - (0.5f * thickness * sin(perp)),
      0,
      0,
      x2 - (headHeight * cos(direction)) + (0.5f * thickness * cos(perp)),
      y2 - (headHeight * sin(direction)) + (0.5f * thickness * sin(perp)),
      0,
      0,
      x1 + (0.5f * thickness * cos(perp)),
      y1 + (0.5f * thickness * sin(perp)),
      0,
      0,
      x2 - (headHeight * cos(direction)) - (thickness * cos(perp)),
      y2 - (headHeight * sin(direction)) - (thickness * sin(perp)),
      0,
      0,
      x2,
      y2,
      0,
      0,
      x2 - (headHeight * cos(direction)) + (thickness * cos(perp)),
      y2 - (headHeight * sin(direction)) + (thickness * sin(perp)),
      0,
      0,
  };
  float *verts = verts_v.data();
  _shader2->use();
  _shader2->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  _shader2->setUniform("color", shaderColor);
  _shader2->setUniform("useTex", 0);
  _shader2->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawAltShader2D() {
  _shader2Alt->use();
  _shader2->setUniform("transform", _proj2);
  std::vector<float> verts_v = {0.0f,
                                0.0f,
                                0.0f,
                                0.0f,
                                (float)_windowWidth,
                                0.0f,
                                0.0f,
                                0.0f,
                                (float)_windowWidth,
                                (float)_windowHeight,
                                0.0f,
                                0.0f,
                                (float)_windowWidth,
                                (float)_windowHeight,
                                0.0f,
                                0.0f,
                                0.0f,
                                (float)_windowHeight,
                                0.0f,
                                0.0f,
                                0.0f,
                                0.0f,
                                0.0f,
                                0.0f};
  float *verts = verts_v.data();

  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

} // namespace GraphicsTools