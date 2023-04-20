#include "glad/gl.h"
#include "mbgfx.h"
#include "shader.h"
#include <iostream>
#include <vector>

const int SHADOW_TEX_SIZE = 4096;
const float SHADOW_FRUSTUM_DEPTH = 100;
const float SHADOW_FRUSTUM_WIDTH = 60;

const int VBO_2D_MAX_SIZE = 4000;
const int VBO_3D_MAX_SIZE = 40000;
const int VAO_3D_DATA_WIDTH = 8;

const int CIRCLE_2D_RESOLUTION = 40;

GraphicsTools::Scene::Scene()
    : _windowWidth(0), _windowHeight(0), _nextCamId(0), _nextObjId(0),
      _activeCamId(-1), _dLight(NULL), _depth(99.0f) {

  glGenVertexArrays(1, &_2DVao);
  glGenBuffers(1, &_2DVbo);
  glBindVertexArray(_2DVao);
  glBindBuffer(GL_ARRAY_BUFFER, _2DVbo);
  glBufferData(GL_ARRAY_BUFFER, VBO_2D_MAX_SIZE * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &_3DVao);
  glGenBuffers(1, &_3DVbo);
  glBindVertexArray(_3DVao);
  glBindBuffer(GL_ARRAY_BUFFER, _3DVbo);
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

  _2DShader = new ShaderProgram("assets/2d_vs.glsl", "assets/2d_fs.glsl");
}

GraphicsTools::Scene::~Scene() {
  delete _depthShader;
  delete _2DShader;
}

Camera *GraphicsTools::Scene::activeCamera() const {
  return _activeCamId != -1 ? _cameras.at(_activeCamId) : NULL;
};

void GraphicsTools::Scene::setWindowDimensions(int w, int h) {
  _windowWidth = w;
  _windowHeight = h;
  _2DProj = glm::ortho(0.0f, (float)_windowWidth, 0.0f, (float)_windowHeight,
                       100.0f, -100.0f);
};

void GraphicsTools::Scene::setupShadows() {
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
                          -SHADOW_FRUSTUM_WIDTH, SHADOW_FRUSTUM_WIDTH, 0.1f,
                          SHADOW_FRUSTUM_DEPTH);
  _depthShader =
      new ShaderProgram("assets/depth_vs.glsl", "assets/depth_fs.glsl");
}

void GraphicsTools::Scene::renderShadows() {
  if (_dLight) {
    glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, _shadowFbo);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto &obj : _objs) {
      obj.second->draw(_lightView, _lightProj, _lightView, _depthShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void GraphicsTools::Scene::render() const {
  glViewport(0, 0, _windowWidth, _windowHeight);
  if (_activeCamId != -1 && _dLight) {
    _dLight->sp->use();
    _dLight->sp->setUniform("dirLight.dir", _dLight->_dir);
    _dLight->sp->setUniform("dirLight.ambient", _dLight->_ambientColor);
    _dLight->sp->setUniform("dirLight.diffuse", _dLight->_diffuseColor);
    _dLight->sp->setUniform("dirLight.specular", _dLight->_specularColor);
    _dLight->sp->setUniform("viewPos", activeCamera()->pos());
    glActiveTexture(GL_TEXTURE1); // unit 0 is reserved for object textures
    glBindTexture(GL_TEXTURE_2D, _depthMap);
    _dLight->sp->setUniform("shadowMap", 1);
    for (auto &obj : _objs) {
      obj.second->draw(activeCamera()->viewMatrix(),
                       activeCamera()->projMatrix(), _lightProj * _lightView);
    }
  }
}

void GraphicsTools::Scene::drawText2D(GraphicsTools::Font font, std::string str,
                                      GraphicsTools::ColorRgba color, float x0,
                                      float y0, float width, float drawScale) {
  _2DShader->use();
  _2DShader->setUniform("color", glm::vec4(color.r, color.g, color.b, 1.0));
  _2DShader->setUniform("transform", _2DProj);
  glActiveTexture(GL_TEXTURE0);
  _2DShader->setUniform("tex", 0);
  _2DShader->setUniform("useTex", 1);
  _2DShader->setUniform("drawDepth", _depth);
  glBindVertexArray(_2DVao);

  float x = x0, y = y0;
  std::string::const_iterator ch;
  for (ch = str.begin(); ch != str.end(); ++ch) {
    if (*ch == '\n') {
      x = x0;
      y -= font.size() * drawScale;
      continue;
    }
    GraphicsTools::TextGlyph tch(font.glyph(*ch));
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
    glBindBuffer(GL_ARRAY_BUFFER, _2DVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    x += (tch.charAdvance >> 6) * drawScale;
  }
  glBindVertexArray(0);
  _depth -= 1.0f;
}

void GraphicsTools::Scene::drawCircle2D(GraphicsTools::ColorRgba color, float x,
                                        float y, float r) {
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
  _2DShader->use();
  _2DShader->setUniform("transform", _2DProj);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  shaderColor /= 255.0f;
  _2DShader->setUniform("color", shaderColor);
  _2DShader->setUniform("useTex", 0);
  _2DShader->setUniform("drawDepth", _depth);
  glBindVertexArray(_2DVao);
  glBindBuffer(GL_ARRAY_BUFFER, _2DVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, verts_v.size() / 4);
  _depth -= 1.0f;
}

void GraphicsTools::Scene::drawRectangle2D(GraphicsTools::ColorRgba color,
                                           float x1, float y1, float x2,
                                           float y2) {
  std::vector<float> verts_v = {x1, y1, 0.0f, 0.0f, x2, y1, 0.0f, 0.0f,
                                x2, y2, 0.0f, 0.0f, x2, y2, 0.0f, 0.0f,
                                x1, y2, 0.0f, 0.0f, x1, y1, 0.0f, 0.0f};
  float *verts = verts_v.data();
  _2DShader->use();
  _2DShader->setUniform("transform", _2DProj);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  shaderColor /= 255.0f;
  _2DShader->setUniform("color", shaderColor);
  _2DShader->setUniform("useTex", 0);
  _2DShader->setUniform("drawDepth", _depth);
  glBindVertexArray(_2DVao);
  glBindBuffer(GL_ARRAY_BUFFER, _2DVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth -= 1.0f;
}

void GraphicsTools::Scene::drawLine2D(GraphicsTools::ColorRgba color,
                                      float thickness, float x1, float y1,
                                      float x2, float y2) {
  std::vector<float> verts_v = {x1, y1, 0.0f, 0.0f, x2, y2, 0.0f, 0.0f};
  float *verts = verts_v.data();
  _2DShader->use();
  _2DShader->setUniform("transform", _2DProj);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  shaderColor /= 255.0f;
  _2DShader->setUniform("color", shaderColor);
  _2DShader->setUniform("useTex", 0);
  _2DShader->setUniform("drawDepth", _depth);
  glBindVertexArray(_2DVao);
  glBindBuffer(GL_ARRAY_BUFFER, _2DVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glLineWidth(thickness);
  glDrawArrays(GL_LINES, 0, verts_v.size() / 4);
  _depth -= 1.0f;
}