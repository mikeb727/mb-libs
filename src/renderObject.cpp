#include "renderObject.h"
#include "colors.h"
#include "errors.h"
#include "glad/gl.h"
#include "shader.h"

// experimental; for vector slerp (lerping cube normals)
#include <glm/gtx/rotate_vector.hpp>
const float CUBE_NORMAL_SLERP_FACTOR = 0.1f;

#include <iostream>

namespace GraphicsTools {

// assume no texture in objects by default
RenderObject::RenderObject()
    : _pos(glm::zero<glm::vec3>()), _rot(glm::zero<glm::vec3>()),
      _modelMat(glm::identity<glm::mat4>()), _sp(NULL),
      _material({Colors::White, NULL, Colors::White, 1}), _vDataWidth(8) {}

void RenderObject::genCube(float sideLength) {

  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  std::vector<glm::vec3> normals_v;

  for (int i = 0; i < 8; ++i) {
    verts_v.push_back(
        glm::vec3(sideLength * (float)((i & 1)) - (0.5f * sideLength),
                  sideLength * (float)((i & 2) >> 1) - (0.5f * sideLength),
                  sideLength * (float)((i & 4) >> 2) - (0.5f * sideLength)));
  }

  indices_v = {2, 3, 1, 1, 0, 2, 3, 7, 5, 5, 1, 3, 7, 6, 4, 4, 5, 7,
               6, 2, 0, 0, 4, 6, 3, 2, 6, 6, 7, 3, 5, 4, 0, 0, 1, 5};

  // only six possible normals in model space, so store these in memory
  // divide index subscript by 6 to get corresponding normal subscript
  normals_v = {
      {0.0f, 0.0f, -1.0f},
      {1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f},
      {-1.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f},
      {0.0f, -1.0f, 0.0f}
  };

  for (int i = 0; i < indices_v.size(); ++i){
    _vData.push_back(verts_v.at(indices_v.at(i)).x);
    _vData.push_back(verts_v.at(indices_v.at(i)).y);
    _vData.push_back(verts_v.at(indices_v.at(i)).z);
    _vData.push_back(glm::slerp(normals_v.at(i/6), glm::normalize(verts_v.at(indices_v.at(i))), CUBE_NORMAL_SLERP_FACTOR).x);
    _vData.push_back(glm::slerp(normals_v.at(i/6), glm::normalize(verts_v.at(indices_v.at(i))), CUBE_NORMAL_SLERP_FACTOR).y);
    _vData.push_back(glm::slerp(normals_v.at(i/6), glm::normalize(verts_v.at(indices_v.at(i))), CUBE_NORMAL_SLERP_FACTOR).z);
    _vData.push_back(verts_v.at(indices_v.at(i)).x);
    _vData.push_back(verts_v.at(indices_v.at(i)).y);

  }
}

void RenderObject::genSphere(float radius, int numLatSegments,
                             int numLonSegments) {
  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  std::vector<glm::vec3> normals_v;

  // north pole
  verts_v.push_back(glm::vec3(0.0f, radius, 0.0f));

  // latitude rings
  for (int i = 1; i < numLatSegments; ++i) {
    float minorAngle =
        (M_PI / 180.0f) * (90.0f - (180.0f * i / numLatSegments));
    float latRingRadius = cos(minorAngle) * radius;
    // longitude points along ring
    for (int j = 0; j < numLonSegments; ++j) {
      float majorAngle = (M_PI / 180.0f) * -(360.0f * j / numLonSegments);
      verts_v.push_back(glm::vec3(cos(majorAngle) * latRingRadius,
                                  sin(minorAngle) * radius,
                                  sin(majorAngle) * latRingRadius));
    }
  }

  // south pole
  verts_v.push_back(glm::vec3(0.0f, -radius, 0.0f));

  // north cap
  for (int j = 0; j < numLonSegments; ++j) {
    int triIndexA = 0;
    int triIndexB = 1 + (j % numLonSegments);
    int triIndexC = 1 + ((j + 1) % numLonSegments);
    indices_v.push_back(triIndexA);
    indices_v.push_back(triIndexB);
    indices_v.push_back(triIndexC);
  }
  // middle bands
  for (int i = 0; i < numLatSegments - 2; ++i) {
    for (int j = 0; j < numLonSegments; ++j) {
      int quadIndexA = 1 + (i * numLonSegments) + (j % numLonSegments);
      int quadIndexB = 1 + ((i + 1) * numLonSegments) + (j % numLonSegments);
      int quadIndexC =
          1 + ((i + 1) * numLonSegments) + ((j + 1) % numLonSegments);
      int quadIndexD = 1 + (i * numLonSegments) + ((j + 1) % numLonSegments);
      indices_v.push_back(quadIndexA);
      indices_v.push_back(quadIndexB);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexD);
      indices_v.push_back(quadIndexA);
    }
  }

  // south cap
  for (int j = 0; j < numLonSegments; ++j) {
    int triIndexA =
        1 + ((numLatSegments - 2) * numLonSegments) + (j % numLonSegments);
    int triIndexB = ((numLatSegments - 1) * numLonSegments) + 1;
    int triIndexC = 1 + ((numLatSegments - 2) * numLonSegments) +
                    ((j + 1) % numLonSegments);
    indices_v.push_back(triIndexA);
    indices_v.push_back(triIndexB);
    indices_v.push_back(triIndexC);
  }

  for (unsigned int i : indices_v) {
    _vData.push_back(verts_v.at(i).x);
    _vData.push_back(verts_v.at(i).y);
    _vData.push_back(verts_v.at(i).z);
    glm::vec3 normal = glm::normalize(verts_v.at(i));
    _vData.push_back(normal.x);
    _vData.push_back(normal.y);
    _vData.push_back(normal.z);
    _vData.push_back(verts_v.at(i).x);
    _vData.push_back(verts_v.at(i).y);
  }
}

void RenderObject::genTorus(float majorRadius, float minorRadius,
                            int numMinorSegments, int numMajorSegments) {

  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  std::vector<glm::vec3> normals_v;

  // rings centered on axis of rotation first. that way same sphere indexing
  // can be used
  for (int j = 0; j < numMinorSegments; ++j) {
    float minorAngle = (M_PI / 180.0f) * (360.0f * j / numMinorSegments);
    for (int i = 0; i < numMajorSegments; ++i) {
      float majorAngle = (M_PI / 180.0f) * (360.0f * i / numMajorSegments);
      // points along ring
      verts_v.push_back(glm::vec3(
          (majorRadius + (minorRadius * cos(minorAngle))) * cos(majorAngle),
          minorRadius * sin(minorAngle),
          (majorRadius + (minorRadius * cos(minorAngle))) * sin(majorAngle)));
      // calc normals here, since they're a function of the angles
      normals_v.push_back(glm::vec3(cos(majorAngle) * cos(minorAngle),
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
      indices_v.push_back(quadIndexA);
      indices_v.push_back(quadIndexB);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexD);
      indices_v.push_back(quadIndexA);
    }
  }

  for (unsigned int i : indices_v) {
    _vData.push_back(verts_v.at(i).x);
    _vData.push_back(verts_v.at(i).y);
    _vData.push_back(verts_v.at(i).z);
    _vData.push_back(normals_v.at(i).x);
    _vData.push_back(normals_v.at(i).y);
    _vData.push_back(normals_v.at(i).z);
    _vData.push_back(verts_v.at(i).x);
    _vData.push_back(verts_v.at(i).y);
  }
}

void RenderObject::genPlane(float width, float depth) {

  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;

  for (int i = 0; i < 4; ++i) {
    verts_v.push_back(
        glm::vec3(width * (float)((i & 1)) - (0.5f * width), 0.0f,
                  depth * (float)((i & 2) >> 1) - (0.5f * depth)));
  }

  indices_v = {0, 2, 3, 3, 1, 0};

  for (unsigned int i : indices_v) {
    _vData.push_back(verts_v.at(i).x);
    _vData.push_back(verts_v.at(i).y);
    _vData.push_back(verts_v.at(i).z);
    _vData.push_back(0.0f);
    _vData.push_back(1.0f);
    _vData.push_back(0.0f);
    _vData.push_back(verts_v.at(i).x * 0.1f);
    _vData.push_back(verts_v.at(i).z * 0.1f);
  }
}

void RenderObject::debugPrint(std::ostream &out) {
  // for (int i = 0; i < _indices.size(); ++i) {
  //   std::fprintf(stderr, "%i (index %i)\n", i, _indices[i]);
  //   std::fprintf(stderr, "vert %f, %f, %f\n", _preBuffer[(_vDataWidth * i) +
  //   0],
  //                _preBuffer[(_vDataWidth * i) + 1],
  //                _preBuffer[(_vDataWidth * i) + 2]);
  //   std::fprintf(stderr, "norm %f, %f, %f\n", _preBuffer[(_vDataWidth * i) +
  //   3],
  //                _preBuffer[(_vDataWidth * i) + 4],
  //                _preBuffer[(_vDataWidth * i) + 5]);
  //   if (_material->diffuseMap) {
  //     std::fprintf(stderr, "tex %f, %f\n", _preBuffer[(_vDataWidth * i) + 6],
  //                  _preBuffer[(_vDataWidth * i) + 7]);
  //   }
  //   std::fprintf(stderr, "\n");
  // }
}

RenderObject::~RenderObject() {}

void RenderObject::recalc(glm::mat4 viewMat = glm::identity<glm::mat4>()) {
  _modelMat = glm::translate(_pos) *
              // glm::rotate(_rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
              // glm::rotate(_rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
              glm::rotate(_rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
  _normalMat = glm::inverseTranspose(glm::mat3(_modelMat));
}

void RenderObject::setPos(glm::vec3 newPos) {
  _pos = newPos;
  recalc();
};

void RenderObject::setRotation(glm::vec3 eulerAngles) {
  _rot = eulerAngles;
  recalc();
}

void RenderObject::draw(glm::mat4 viewMat, glm::mat4 projMat,
                        glm::mat4 lightMat, ShaderProgram *overrideShader) {

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
    _sp->setUniform("material.diffuse", colorToGlm(_material.diffuse));
    _sp->setUniform("material.useDiffuseMap", _material.diffuseMap != NULL);
    _sp->setUniform("material.specular", colorToGlm(_material.specular));
    _sp->setUniform("material.shininess", _material.shininess);
    if (_material.diffuseMap) {
      glActiveTexture(GL_TEXTURE1);
      _material.diffuseMap->use();
    }
  }
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * _vData.size(),
                  _vData.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, _vData.size() / _vDataWidth);
}

} // namespace GraphicsTools