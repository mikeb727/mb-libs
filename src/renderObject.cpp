#include "renderObject.h"
#include "errors.h"
#include "glad/gl.h"
#include "shader.h"

// experimental; for vector slerp (lerping cube normals)
#include <glm/gtx/rotate_vector.hpp>
const float CUBE_NORMAL_SLERP_FACTOR = 0.0f;

#include <iostream>

// assume no texture in objects by default
RenderObject::RenderObject()
    : _pos(glm::zero<glm::vec3>()), _rot(glm::zero<glm::vec3>()),
      _modelMat(glm::identity<glm::mat4>()), _preBufferLen(0), _sp(NULL),
      _material(NULL), _vDataWidth(6) {}

void RenderObject::clearVertexData() {
  _verts.clear();
  _indices.clear();
  if (_preBufferLen > 0) {
    delete _preBuffer;
    _preBufferLen = 0;
  }
}

void RenderObject::genCube(float sideLength) {
  clearVertexData();

  for (int i = 0; i < 8; ++i) {
    _verts.push_back(
        glm::vec3(sideLength * (float)((i & 1)) - (0.5f * sideLength),
                  sideLength * (float)((i & 2) >> 1) - (0.5f * sideLength),
                  sideLength * (float)((i & 4) >> 2) - (0.5f * sideLength)));
  }

  _indices = {2, 3, 1, 1, 0, 2, 3, 7, 5, 5, 1, 3, 7, 6, 4, 4, 5, 7,
              6, 2, 0, 0, 4, 6, 3, 2, 6, 6, 7, 3, 5, 4, 0, 0, 1, 5};

  // prepare the vertex buffer
  _preBufferLen = _indices.size() * _vDataWidth;
  _preBuffer = new float[_preBufferLen];
  // compute normals triangle by triangle, since all the info is available to us
  for (int tri = 0; tri < 12; ++tri) {
    int vertOffsetA = (3 * tri) + 0;
    int vertOffsetB = (3 * tri) + 1;
    int vertOffsetC = (3 * tri) + 2;

    int vertIndexA = _indices[vertOffsetA];
    int vertIndexB = _indices[vertOffsetB];
    int vertIndexC = _indices[vertOffsetC];

    glm::vec3 vertA = _verts[vertIndexA];
    glm::vec3 vertB = _verts[vertIndexB];
    glm::vec3 vertC = _verts[vertIndexC];

    // spherical normals (for interpolation)
    glm::vec3 sphNormalA = glm::normalize(vertA);
    glm::vec3 sphNormalB = glm::normalize(vertB);
    glm::vec3 sphNormalC = glm::normalize(vertC);

    glm::vec3 triSegmentAB = vertB - vertA;
    glm::vec3 triSegmentAC = vertC - vertA;

    glm::vec3 vertNormalCommon =
        glm::normalize(glm::cross(triSegmentAB, triSegmentAC));

    glm::vec3 vertNormalA = glm::normalize(
        glm::slerp(vertNormalCommon, sphNormalA, CUBE_NORMAL_SLERP_FACTOR));
    glm::vec3 vertNormalB = glm::normalize(
        glm::slerp(vertNormalCommon, sphNormalB, CUBE_NORMAL_SLERP_FACTOR));
    glm::vec3 vertNormalC = glm::normalize(
        glm::slerp(vertNormalCommon, sphNormalC, CUBE_NORMAL_SLERP_FACTOR));

    _preBuffer[_vDataWidth * vertOffsetA + 0] = _verts[vertIndexA].x;
    _preBuffer[_vDataWidth * vertOffsetA + 1] = _verts[vertIndexA].y;
    _preBuffer[_vDataWidth * vertOffsetA + 2] = _verts[vertIndexA].z;
    _preBuffer[_vDataWidth * vertOffsetA + 3] = vertNormalA.x;
    _preBuffer[_vDataWidth * vertOffsetA + 4] = vertNormalA.y;
    _preBuffer[_vDataWidth * vertOffsetA + 5] = vertNormalA.z;

    _preBuffer[_vDataWidth * vertOffsetB + 0] = _verts[vertIndexB].x;
    _preBuffer[_vDataWidth * vertOffsetB + 1] = _verts[vertIndexB].y;
    _preBuffer[_vDataWidth * vertOffsetB + 2] = _verts[vertIndexB].z;
    _preBuffer[_vDataWidth * vertOffsetB + 3] = vertNormalB.x;
    _preBuffer[_vDataWidth * vertOffsetB + 4] = vertNormalB.y;
    _preBuffer[_vDataWidth * vertOffsetB + 5] = vertNormalB.z;

    _preBuffer[_vDataWidth * vertOffsetC + 0] = _verts[vertIndexC].x;
    _preBuffer[_vDataWidth * vertOffsetC + 1] = _verts[vertIndexC].y;
    _preBuffer[_vDataWidth * vertOffsetC + 2] = _verts[vertIndexC].z;
    _preBuffer[_vDataWidth * vertOffsetC + 3] = vertNormalC.x;
    _preBuffer[_vDataWidth * vertOffsetC + 4] = vertNormalC.y;
    _preBuffer[_vDataWidth * vertOffsetC + 5] = vertNormalC.z;
  }

  setupGl();
}

void RenderObject::genSphere(float radius, int numLatSegments,
                             int numLonSegments) {
  clearVertexData();

  // north pole
  _verts.push_back(glm::vec3(0.0f, radius, 0.0f));

  // latitude rings
  for (int i = 1; i < numLatSegments; ++i) {
    float minorAngle =
        (M_PI / 180.0f) * (90.0f - (180.0f * i / numLatSegments));
    float latRingRadius = cos(minorAngle) * radius;
    // longitude points along ring
    for (int j = 0; j < numLonSegments; ++j) {
      float majorAngle = (M_PI / 180.0f) * -(360.0f * j / numLonSegments);
      _verts.push_back(glm::vec3(cos(majorAngle) * latRingRadius,
                                 sin(minorAngle) * radius,
                                 sin(majorAngle) * latRingRadius));
    }
  }

  // south pole
  _verts.push_back(glm::vec3(0.0f, -radius, 0.0f));

  // north cap
  for (int j = 0; j < numLonSegments; ++j) {
    int triIndexA = 0;
    int triIndexB = 1 + (j % numLonSegments);
    int triIndexC = 1 + ((j + 1) % numLonSegments);
    _indices.push_back(triIndexA);
    _indices.push_back(triIndexB);
    _indices.push_back(triIndexC);
  }
  // middle bands
  for (int i = 0; i < numLatSegments - 2; ++i) {
    for (int j = 0; j < numLonSegments; ++j) {
      int quadIndexA = 1 + (i * numLonSegments) + (j % numLonSegments);
      int quadIndexB = 1 + ((i + 1) * numLonSegments) + (j % numLonSegments);
      int quadIndexC =
          1 + ((i + 1) * numLonSegments) + ((j + 1) % numLonSegments);
      int quadIndexD = 1 + (i * numLonSegments) + ((j + 1) % numLonSegments);
      _indices.push_back(quadIndexA);
      _indices.push_back(quadIndexB);
      _indices.push_back(quadIndexC);
      _indices.push_back(quadIndexC);
      _indices.push_back(quadIndexD);
      _indices.push_back(quadIndexA);
    }
  }

  // south cap
  for (int j = 0; j < numLonSegments; ++j) {
    int triIndexA =
        1 + ((numLatSegments - 2) * numLonSegments) + (j % numLonSegments);
    int triIndexB = ((numLatSegments - 1) * numLonSegments) + 1;
    int triIndexC = 1 + ((numLatSegments - 2) * numLonSegments) +
                    ((j + 1) % numLonSegments);
    _indices.push_back(triIndexA);
    _indices.push_back(triIndexB);
    _indices.push_back(triIndexC);
  }

  _preBufferLen = _indices.size() * _vDataWidth;
  _preBuffer = new float[_preBufferLen];
  // only need one vertex to compute the normal for a sphere
  for (int i = 0; i < _indices.size(); ++i) {
    _preBuffer[(_vDataWidth * i) + 0] = _verts[_indices[i]].x;
    _preBuffer[(_vDataWidth * i) + 1] = _verts[_indices[i]].y;
    _preBuffer[(_vDataWidth * i) + 2] = _verts[_indices[i]].z;
    _preBuffer[(_vDataWidth * i) + 3] = _preBuffer[(_vDataWidth * i)] / radius;
    _preBuffer[(_vDataWidth * i) + 4] =
        _preBuffer[(_vDataWidth * i) + 1] / radius;
    _preBuffer[(_vDataWidth * i) + 5] =
        _preBuffer[(_vDataWidth * i) + 2] / radius;
  }
  setupGl();
}

void RenderObject::genTorus(float majorRadius, float minorRadius,
                            int numMinorSegments, int numMajorSegments) {

  clearVertexData();

  std::vector<glm::vec3> normals;

  // rings centered on axis of rotation first. that way same sphere indexing
  // can be used
  for (int j = 0; j < numMinorSegments; ++j) {
    float minorAngle = (M_PI / 180.0f) * (360.0f * j / numMinorSegments);
    for (int i = 0; i < numMajorSegments; ++i) {
      float majorAngle = (M_PI / 180.0f) * (360.0f * i / numMajorSegments);
      // points along ring
      _verts.push_back(glm::vec3(
          (majorRadius + (minorRadius * cos(minorAngle))) * cos(majorAngle),
          minorRadius * sin(minorAngle),
          (majorRadius + (minorRadius * cos(minorAngle))) * sin(majorAngle)));
      // calc normals here, since they're a function of the angles
      normals.push_back(glm::vec3(cos(majorAngle) * cos(minorAngle),
                                  sin(minorAngle),
                                  sin(majorAngle) * cos(minorAngle)));
    }
  }

  // bands
  for (int j = 0; j < numMajorSegments; ++j) {
    for (int i = 0; i < numMinorSegments; ++i) {
      int quadIndexA =
          ((i % numMinorSegments) * numMajorSegments) + (j % numMajorSegments);
      int quadIndexB = (((i + 1) % numMinorSegments) * numMajorSegments) +
                       (j % numMajorSegments);
      int quadIndexC = (((i + 1) % numMinorSegments) * numMajorSegments) +
                       ((j + 1) % numMajorSegments);
      int quadIndexD = ((i % numMinorSegments) * numMajorSegments) +
                       ((j + 1) % numMajorSegments);
      _indices.push_back(quadIndexA);
      _indices.push_back(quadIndexB);
      _indices.push_back(quadIndexC);
      _indices.push_back(quadIndexC);
      _indices.push_back(quadIndexD);
      _indices.push_back(quadIndexA);
    }
  }

  _preBufferLen = _indices.size() * _vDataWidth;
  _preBuffer = new float[_preBufferLen];
  for (int i = 0; i < _indices.size(); ++i) {
    _preBuffer[(_vDataWidth * i) + 0] = _verts[_indices[i]].x;
    _preBuffer[(_vDataWidth * i) + 1] = _verts[_indices[i]].y;
    _preBuffer[(_vDataWidth * i) + 2] = _verts[_indices[i]].z;
    _preBuffer[(_vDataWidth * i) + 3] = normals[_indices[i]].x;
    _preBuffer[(_vDataWidth * i) + 4] = normals[_indices[i]].y;
    _preBuffer[(_vDataWidth * i) + 5] = normals[_indices[i]].z;
  }
  setupGl();
}

void RenderObject::genPlane(float width, float depth) {
  clearVertexData();

  for (int i = 0; i < 4; ++i) {
    _verts.push_back(glm::vec3(width * (float)((i & 1)) - (0.5f * width), 0.0f,
                               depth * (float)((i & 2) >> 1) - (0.5f * depth)));
  }

  _indices = {0, 2, 3, 3, 1, 0};

  _preBufferLen = _indices.size() * _vDataWidth;
  _preBuffer = new float[_preBufferLen];

  for (int i = 0; i < _indices.size(); ++i) {
    _preBuffer[(_vDataWidth * i) + 0] = _verts[_indices[i]].x;
    _preBuffer[(_vDataWidth * i) + 1] = _verts[_indices[i]].y;
    _preBuffer[(_vDataWidth * i) + 2] = _verts[_indices[i]].z;
    _preBuffer[(_vDataWidth * i) + 3] = 0.0f;
    _preBuffer[(_vDataWidth * i) + 4] = 1.0f;
    _preBuffer[(_vDataWidth * i) + 5] = 0.0f;
    // texture coords (fixed scaling for checkerboard; TODO make this a
    // variable)
    if (_material->diffuseMap) {
      _preBuffer[(_vDataWidth * i) + 6] = _verts[_indices[i]].x * 0.1f;
      _preBuffer[(_vDataWidth * i) + 7] = _verts[_indices[i]].z * 0.1f;
    }
  }

  setupGl();
}

void RenderObject::setupGl() {
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  // careful: sizeof() for dynamic arrays returns the pointer size, *not* the
  // array size! use the size of individual data items times the array length
  // instead!
  glBufferData(GL_ARRAY_BUFFER, _preBufferLen * sizeof(float), _preBuffer,
               GL_STATIC_DRAW);

  glBindVertexArray(_vao);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, _vDataWidth * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, _vDataWidth * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  if (_material && _material->diffuseMap) {
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, _vDataWidth * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
  }
}

void RenderObject::debugPrint() {
  for (int i = 0; i < _indices.size(); ++i) {
    std::fprintf(stderr, "%i (index %i)\n", i, _indices[i]);
    std::fprintf(stderr, "vert %f, %f, %f\n", _preBuffer[(_vDataWidth * i) + 0],
                 _preBuffer[(_vDataWidth * i) + 1],
                 _preBuffer[(_vDataWidth * i) + 2]);
    std::fprintf(stderr, "norm %f, %f, %f\n", _preBuffer[(_vDataWidth * i) + 3],
                 _preBuffer[(_vDataWidth * i) + 4],
                 _preBuffer[(_vDataWidth * i) + 5]);
    if (_material->diffuseMap) {
      std::fprintf(stderr, "tex %f, %f\n", _preBuffer[(_vDataWidth * i) + 6],
                   _preBuffer[(_vDataWidth * i) + 7]);
    }
    std::fprintf(stderr, "\n");
  }
}

RenderObject::~RenderObject() { delete _preBuffer; }

void RenderObject::recalc(glm::mat4 viewMat = glm::identity<glm::mat4>()) {
  _modelMat = glm::translate(_pos) *
              // glm::rotate(_rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
              // glm::rotate(_rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
              glm::rotate(_rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
  _normalMat = glm::transpose(glm::inverse(_modelMat));
}

void RenderObject::setPos(glm::vec3 newPos) {
  _pos = newPos;
  recalc();
};

void RenderObject::setRotation(glm::vec3 eulerAngles) {
  _rot = eulerAngles;
  recalc();
}

void RenderObject::draw(glm::mat4 viewMat, glm::mat4 projMat, glm::mat4 lightMat,
                        ShaderProgram *overrideShader) {
  if (overrideShader) {
    overrideShader->use();
    overrideShader->setUniform("modelMat", _modelMat);
    overrideShader->setUniform("viewMat", viewMat);
    overrideShader->setUniform("projMat", projMat);
  } else {
    recalc(viewMat);
    _sp->use();
    _sp->setUniform("modelMat", _modelMat);
    _sp->setUniform("viewMat", viewMat);
    _sp->setUniform("projMat", projMat);
    _sp->setUniform("normalMat", _normalMat);
    _sp->setUniform("lightMat", lightMat);
    if (_material) {
      _sp->setUniform("material.diffuse", _material->diffuse);
      _sp->setUniform("material.useDiffuseMap", _material->diffuseMap != NULL);
      _sp->setUniform("material.specular", _material->specular);
      _sp->setUniform("material.shininess", _material->shininess);
      if (_material->diffuseMap) {
        glActiveTexture(GL_TEXTURE1);
        _material->diffuseMap->use();
      }
    }
  }
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, _indices.size());
}