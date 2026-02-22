#include "scene.h"
#include "colors.h"
#include "shader.h"

const int SHADOW_TEX_SIZE = 8192;
const float SHADOW_FRUSTUM_DEPTH = 200;
const float SHADOW_FRUSTUM_WIDTH = 500;

const int VBO_2D_MAX_SIZE = 4000;
const int VBO_3D_MAX_SIZE = 80000;
const int VAO_3D_DATA_WIDTH = 8;

const int CIRCLE_2D_RESOLUTION = 96;

#ifdef COMPILE_TIME_SHADERS
const char *vs2 =

#include "../assets/2d_vs_ct.glsl"

    ;
const char *fs2 =

#include "../assets/2d_fs_ct.glsl"
    ;
#endif

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

#ifdef COMPILE_TIME_SHADERS
  _shader2 = new ShaderProgram(vs2, fs2, true);
#else
  _shader2 = new ShaderProgram("assets/2d_vs.glsl", "assets/2d_fs.glsl");
#endif
  _shader2Alt = NULL;
}

Scene::~Scene() {
  delete _depthShader;
  delete _shader2;
}

void Scene::addRenderObject(RenderObject *obj) {
  _objs.emplace(_nextObjId++, obj);
  obj->setVao(_vao3);
  obj->setVbo(_vbo3);
};

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
  _lightView = glm::lookAt(-100.0f * _dLight->_dir, glm::zero<glm::vec3>(),
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
    // set light uniforms for ALL shaders using the light!
    for (ShaderProgram *sp : _dLight->_shaders) {
      sp->use();
      sp->setUniform("dirLight.dir", _dLight->_dir);
      sp->setUniform("dirLight.ambient", colorToGlm(_dLight->_ambientColor));
      sp->setUniform("dirLight.diffuse", colorToGlm(_dLight->_diffuseColor));
      sp->setUniform("dirLight.specular", colorToGlm(_dLight->_specularColor));
      sp->setUniform("viewPos", activeCamera()->pos());
      if (_useShadows) {
        glActiveTexture(GL_TEXTURE1); // unit 0 is reserved for object textures
        glBindTexture(GL_TEXTURE_2D, _depthMap);
        sp->setUniform("shadowMap", 1);
      }
    }
    for (auto &obj : _objs) {
      obj.second->draw(activeCamera()->viewMatrix(),
                       activeCamera()->projMatrix(), _lightProj * _lightView);
    }
  }
}

void Scene::drawText2D(Font font, std::string str, ColorRgba color,
                       ColorRgba backgroundColor, float x0, float y0,
                       float angle, float width,
                       GraphicsTools::TextAlignModeH alignment, float drawScale,
                       GraphicsTools::ShaderProgram *overrideShader) {
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  if (backgroundColor == Colors::None) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glBlendColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_COLOR);
  }
  GraphicsTools::ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  glm::mat4 modelMat = glm::translate(glm::vec3(x0, y0, 0.0f)) *
                       glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
  sh->use();
  sh->setUniform("color", glm::vec4(color.r, color.g, color.b, 1.0));
  sh->setUniform("transform", _proj2 * modelMat);
  glActiveTexture(GL_TEXTURE0);
  sh->setUniform("tex", 0);
  sh->setUniform("useTex", 1);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);

  // bounds for a background rectangle
  float rX = -10, rY = 0, rW = 0, rH = 0;

  float x = 0, y = 0;
  // two iterators; one for drawing glyphs left to right, one for computing x
  // offset for center- and right-aligned text
  std::string::const_iterator ch;
  float shiftTotal;
  for (ch = str.begin(); ch != str.end(); ++ch) {
    if (x == 0 && alignment != Left) {
      float shiftMult = alignment == Center ? 0.5 : 1;
      std::string::const_iterator chAlignCalcH = ch;
      // shift starting position of the line to the left until:
      //  - max width reached (unless width is unlimited)
      //  - newline character reached
      //  - end of string reached
      while ((x > (-shiftMult * width) || width == -1) &&
             chAlignCalcH != str.end()) {
        if ((*chAlignCalcH) == '\n')
          break;
        x -= shiftMult * (font.glyph(*chAlignCalcH).charAdvance >> 6) *
             drawScale;
        // std::cerr << x << std::endl;
        ++chAlignCalcH;
      }
      shiftTotal = x;
    }
    rX = std::min(x - 10, rX);
    if (*ch == '\n') {
      x = 0;
      y -= font.size() * drawScale;
      rH += font.size() * drawScale;
      continue;
    }
    TextGlyph tch(font.glyph(*ch));
    if ((width != -1) && ((x + tch.bearingX + tch.sizeX - 0) > width)) {
      rW = width;
      x = 0;
      y -= font.size() * drawScale;
      rH += font.size() * drawScale;
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
    rW = std::max(x - shiftTotal, rW);
  }
  rY -= (rH + 10);
  rW += 20;
  rH += 30;
  _depth -= 0.5f;
  if (backgroundColor != Colors::None) {
    sh->setUniform("color", colorToGlm(backgroundColor));
    sh->setUniform("useTex", 0);
    sh->setUniform("drawDepth", _depth);
    float quadX = rX * drawScale;
    float quadY = rY * drawScale;
    float quadW = rW * drawScale;
    float quadH = rH * drawScale;
    float verts[6][4] = {{quadX, quadY, 0.0f, 1.0f},
                         {quadX + quadW, quadY, 1.0f, 1.0f},
                         {quadX + quadW, quadY + quadH, 1.0f, 0.0f},
                         {quadX + quadW, quadY + quadH, 1.0f, 0.0f},
                         {quadX, quadY + quadH, 0.0f, 0.0f},
                         {quadX, quadY, 0.0f, 1.0f}};
    glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBlendColor(0, 0, 0, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  }
  _depth += 1.5f;
}

void Scene::drawCircle2D(ColorRgba color, float x, float y, float r,
                         float angle, ShaderProgram *overrideShader) {
  std::vector<float> verts_v;
  verts_v.push_back(0.0f);
  verts_v.push_back(0.0f);
  verts_v.push_back(0.0f);
  verts_v.push_back(0.0f);
  for (int i = 0; i < CIRCLE_2D_RESOLUTION + 1; ++i) {
    float theta = (360.0f * i / (float)CIRCLE_2D_RESOLUTION) * (M_PI / 180.0f);
    verts_v.push_back((r * cos(theta)));
    verts_v.push_back((r * sin(theta)));
    verts_v.push_back(0.0f);
    verts_v.push_back(0.0f);
  }
  float *verts = verts_v.data();
  ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  glm::mat4 modelMat = glm::translate(glm::vec3(x, y, 0.0f)) *
                       glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
  sh->use();
  sh->setUniform("transform", _proj2 * modelMat);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawCircleOutline2D(GraphicsTools::ColorRgba color, float x,
                                float y, float r, float angle, float thickness,
                                GraphicsTools::ShaderProgram *overrideShader) {
  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  // inner ring verts
  for (int i = 0; i < CIRCLE_2D_RESOLUTION; ++i) {
    float angle = (360.0f * i / (float)CIRCLE_2D_RESOLUTION) * (M_PI / 180.0f);
    verts_v.push_back(glm::vec3(((r - thickness) * cos(angle)),
                                ((r - thickness) * sin(angle)), 0));
  }
  // outer ring verts
  for (int i = 0; i < CIRCLE_2D_RESOLUTION; ++i) {
    float angle = (360.0f * i / (float)CIRCLE_2D_RESOLUTION) * (M_PI / 180.0f);
    verts_v.push_back(glm::vec3((r * cos(angle)), (r * sin(angle)), 0));
  }

  // quads - start with inner ring
  for (int i = 0; i < CIRCLE_2D_RESOLUTION; ++i) {
    int quadIndexA = (i % CIRCLE_2D_RESOLUTION);
    int quadIndexB = (i % CIRCLE_2D_RESOLUTION) + CIRCLE_2D_RESOLUTION;
    int quadIndexC = ((i + 1) % CIRCLE_2D_RESOLUTION) + CIRCLE_2D_RESOLUTION;
    int quadIndexD = ((i + 1) % CIRCLE_2D_RESOLUTION);
    indices_v.push_back(quadIndexA);
    indices_v.push_back(quadIndexB);
    indices_v.push_back(quadIndexC);
    indices_v.push_back(quadIndexC);
    indices_v.push_back(quadIndexD);
    indices_v.push_back(quadIndexA);
  }

  std::vector<float> v_data;

  for (unsigned int i : indices_v) {
    v_data.push_back(verts_v.at(i).x);
    v_data.push_back(verts_v.at(i).y);
    v_data.push_back(verts_v.at(i).x);
    v_data.push_back(verts_v.at(i).y);
  }

  float *verts = v_data.data();

  ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  glm::mat4 modelMat = glm::translate(glm::vec3(x, y, 0.0f)) *
                       glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
  sh->use();
  sh->setUniform("transform", _proj2 * modelMat);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, v_data.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, v_data.size() / 4);
  _depth += 1.0f;
}

void Scene::drawRectangle2D(ColorRgba color, float x1, float y1, float x2,
                            float y2, float angle,
                            GraphicsTools::ShaderProgram *overrideShader) {
  std::vector<float> verts_v = {x1, y1, 0.0f, 0.0f, x2, y1, 0.0f, 0.0f,
                                x2, y2, 0.0f, 0.0f, x2, y2, 0.0f, 0.0f,
                                x1, y2, 0.0f, 0.0f, x1, y1, 0.0f, 0.0f};
  float *verts = verts_v.data();

  ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  glm::mat4 modelMat = glm::translate(glm::vec3(x1, y1, 0.0f)) *
                       glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
  sh->use();
  sh->setUniform("transform", _proj2 * modelMat);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawLine2D(ColorRgba color, float thickness, float x1, float y1,
                       float x2, float y2,
                       GraphicsTools::ShaderProgram *overrideShader) {
  using std::sin, std::cos, std::atan2;
  float direction = atan2(y2 - y1, x2 - x1);
  float perp = direction + (M_PI / 2.0);

  std::vector<float> verts_v = {x1 + (0.5f * thickness * cos(perp)),
                                y1 + (0.5f * thickness * sin(perp)),
                                0,
                                0,
                                x1 - (0.5f * thickness * cos(perp)),
                                y1 - (0.5f * thickness * sin(perp)),
                                0,
                                0,
                                x2 - (0.5f * thickness * cos(perp)),
                                y2 - (0.5f * thickness * sin(perp)),
                                0,
                                0,
                                x2 - (0.5f * thickness * cos(perp)),
                                y2 - (0.5f * thickness * sin(perp)),
                                0,
                                0,
                                x2 + (0.5f * thickness * cos(perp)),
                                y2 + (0.5f * thickness * sin(perp)),
                                0,
                                0,
                                x1 + (0.5f * thickness * cos(perp)),
                                y1 + (0.5f * thickness * sin(perp)),
                                0,
                                0};
  float *verts = verts_v.data();
  GraphicsTools::ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  sh->use();
  sh->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glLineWidth(thickness);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawMultiLine2D(GraphicsTools::ColorRgba color, float thickness,
                            int numPoints, float *points,
                            GraphicsTools::ShaderProgram *overrideShader) {
  if (numPoints == 2) {
    drawLine2D(color, thickness, points[0], points[1], points[2], points[3]);
    return;
  }
  std::vector<float> verts_v;
  genMultiLine2D(verts_v, thickness, numPoints, points);
  float *verts = verts_v.data();

  GraphicsTools::ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  sh->use();
  sh->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawArrow2D(GraphicsTools::ColorRgba color, float x1, float y1,
                        float x2, float y2, float thickness,
                        GraphicsTools::ShaderProgram *overrideShader) {
  using std::sin, std::cos, std::atan2;
  float direction = atan2(y2 - y1, x2 - x1);
  float perp = direction + (M_PI / 2.0);
  float headHeight = sqrt(3) * thickness;
  std::vector<float> verts_v;
  genArrow2D(verts_v, x1, y1, x2, y2, thickness);
  float *verts = verts_v.data();

  GraphicsTools::ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  sh->use();
  sh->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

void Scene::drawMultiArrow2D(GraphicsTools::ColorRgba color, float thickness,
                             int numPoints, float *points,
                             GraphicsTools::ShaderProgram *overrideShader) {

  if (numPoints == 2) {
    drawArrow2D(color, thickness, points[0], points[1], points[2], points[3]);
    return;
  }
  std::vector<float> verts_v;
  genMultiLine2D(verts_v, thickness, numPoints - 1, points);
  genArrow2D(verts_v, points[(2 * numPoints) - 4], points[(2 * numPoints) - 3],
             points[(2 * numPoints) - 2], points[(2 * numPoints) - 1],
             thickness);

  float *verts = verts_v.data();

  GraphicsTools::ShaderProgram *sh = overrideShader ? overrideShader : _shader2;
  sh->use();
  sh->setUniform("transform", _proj2);
  glm::vec4 shaderColor(color.r, color.g, color.b, color.a);
  sh->setUniform("color", shaderColor);
  sh->setUniform("useTex", 0);
  sh->setUniform("drawDepth", _depth);
  glBindVertexArray(_vao2);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
  glBufferSubData(GL_ARRAY_BUFFER, 0, verts_v.size() * sizeof(float), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, verts_v.size() / 4);
  _depth += 1.0f;
}

// draw to entire window using alternative shader
void Scene::drawAltShader2D() {
  _shader2Alt->use();
  _shader2Alt->setUniform("transform", _proj2);
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

void Scene::genArrow2D(std::vector<float> &verts_v, float x1, float y1,
                       float x2, float y2, float thickness) {
  using std::sin, std::cos, std::atan2;
  float direction = atan2(y2 - y1, x2 - x1);
  float perp = direction + (M_PI / 2.0);
  float headHeight = sqrt(3) * thickness;
  std::vector<float> arrowVerts = {
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
  verts_v.insert(verts_v.end(), arrowVerts.begin(), arrowVerts.end());
}

void Scene::genMultiLine2D(std::vector<float> &verts_v, float thickness,
                           int numPoints, float *points) {
  float nearTrunc, farTrunc = 0;
  // generate verts per each pair of adjacent points
  for (int p = 0; p < numPoints - 1; ++p) {
    using std::sin, std::cos, std::atan2;
    float x0, y0, x1, y1, x2, y2, nextAngle;
    nextAngle = 0;
    x0 = points[(2 * p)];
    y0 = points[(2 * p) + 1];
    x1 = points[(2 * p) + 2];
    y1 = points[(2 * p) + 3];
    float direction = atan2(y1 - y0, x1 - x0);
    float perp = direction + (M_PI / 2.0);
    if (p + 2 < numPoints) {
      x2 = points[(2 * p) + 4], y2 = points[(2 * p) + 5];
      nextAngle = atan2(y2 - y1, x2 - x1) - direction;
      farTrunc = fabs((0.5 * thickness) * std::tan(nextAngle / 2));
    }
    verts_v.push_back(x0 + (nearTrunc * cos(direction)) +
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y0 + (nearTrunc * sin(direction)) +
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    verts_v.push_back(x0 + (nearTrunc * cos(direction)) -
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y0 + (nearTrunc * sin(direction)) -
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    verts_v.push_back(x1 - (farTrunc * cos(direction)) -
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y1 - (farTrunc * sin(direction)) -
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    verts_v.push_back(x1 - (farTrunc * cos(direction)) -
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y1 - (farTrunc * sin(direction)) -
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    verts_v.push_back(x1 - (farTrunc * cos(direction)) +
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y1 - (farTrunc * sin(direction)) +
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    verts_v.push_back(x0 + (nearTrunc * cos(direction)) +
                      (0.5f * thickness * cos(perp)));
    verts_v.push_back(y0 + (nearTrunc * sin(direction)) +
                      (0.5f * thickness * sin(perp)));
    verts_v.push_back(0);
    verts_v.push_back(0);
    if (p + 2 < numPoints) {
      // counterclockwise bend triangle fill
      float nextDirection = atan2(y2 - y1, x2 - x1);
      float nextPerp = nextDirection + (M_PI / 2.0);

      if (nextAngle > 0) {
        verts_v.push_back(x1 - (farTrunc * cos(direction)) -
                          (0.5f * thickness * cos(perp)));
        verts_v.push_back(y1 - (farTrunc * sin(direction)) -
                          (0.5f * thickness * sin(perp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
        verts_v.push_back(x1 + (farTrunc * cos(nextDirection)) -
                          (0.5f * thickness * cos(nextPerp)));
        verts_v.push_back(y1 + (farTrunc * sin(nextDirection)) -
                          (0.5f * thickness * sin(nextPerp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
        verts_v.push_back(x1 - (farTrunc * cos(direction)) +
                          (0.5f * thickness * cos(perp)));
        verts_v.push_back(y1 - (farTrunc * sin(direction)) +
                          (0.5f * thickness * sin(perp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
      } else if (nextAngle < 0) {
        verts_v.push_back(x1 - (farTrunc * cos(direction)) -
                          (0.5f * thickness * cos(perp)));
        verts_v.push_back(y1 - (farTrunc * sin(direction)) -
                          (0.5f * thickness * sin(perp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
        verts_v.push_back(x1 + (farTrunc * cos(nextDirection)) +
                          (0.5f * thickness * cos(nextPerp)));
        verts_v.push_back(y1 + (farTrunc * sin(nextDirection)) +
                          (0.5f * thickness * sin(nextPerp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
        verts_v.push_back(x1 - (farTrunc * cos(direction)) +
                          (0.5f * thickness * cos(perp)));
        verts_v.push_back(y1 - (farTrunc * sin(direction)) +
                          (0.5f * thickness * sin(perp)));
        verts_v.push_back(0);
        verts_v.push_back(0);
      }
    }
    nearTrunc = farTrunc;
  }
}

} // namespace GraphicsTools