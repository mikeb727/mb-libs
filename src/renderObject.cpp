#include "renderObject.h"

#include <cmath>

#include <glm/gtx/quaternion.hpp>

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
  normals_v = {{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
               {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};

  for (int i = 0; i < indices_v.size(); ++i) {
    _vData.push_back(verts_v.at(indices_v.at(i)).x);
    _vData.push_back(verts_v.at(indices_v.at(i)).y);
    _vData.push_back(verts_v.at(indices_v.at(i)).z);
    _vData.push_back(glm::slerp(normals_v.at(i / 6),
                                glm::normalize(verts_v.at(indices_v.at(i))),
                                CUBE_NORMAL_SLERP_FACTOR)
                         .x);
    _vData.push_back(glm::slerp(normals_v.at(i / 6),
                                glm::normalize(verts_v.at(indices_v.at(i))),
                                CUBE_NORMAL_SLERP_FACTOR)
                         .y);
    _vData.push_back(glm::slerp(normals_v.at(i / 6),
                                glm::normalize(verts_v.at(indices_v.at(i))),
                                CUBE_NORMAL_SLERP_FACTOR)
                         .z);
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

void RenderObject::genLine(float thickness, int resolution, float x1, float y1,
                           float z1, float x2, float y2, float z2) {
  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  std::vector<glm::vec3> normals_v;

  // generate base ring
  std::vector<glm::vec3> baseRing;
  for (int i = 0; i < resolution; ++i) {
    float angle = 2 * M_PI * i / resolution;
    baseRing.push_back(glm::vec3(0, 0.5 * thickness * std::sin(angle),
                                 -0.5 * thickness * std::cos(angle)));
  }

  // rotate to face from (x1,y1,z1) to (x2,y2,z2)
  glm::vec3 seg(x2 - x1, y2 - y1, z2 - z1);
  float lat = std::acos(
      glm::dot(glm::normalize(glm::vec3(seg.x, seg.y, 0)), glm::vec3(1, 0, 0)));
  float lon =
      glm::length(glm::vec3(seg.x, 0, seg.z)) == 0
          ? 0
          : std::acos(glm::dot(glm::normalize(glm::vec3(seg.x, 0, seg.z)),
                               glm::vec3(1, 0, 0)));
  glm::mat4 rotation =
      glm::mat4(cos(lon) * cos(lat), cos(lon) * sin(lat), sin(lon), 0,
                -sin(lat), cos(lat), 0, 0, -sin(lon) * cos(lat),
                -sin(lon) * sin(lat), cos(lon), 0, 0, 0, 0, 1);
  // rotate and translate to (x1,y1,z1) and add to vertex list
  for (glm::vec3 b : baseRing) {
    glm::vec4 newVert = rotation * glm::vec4(b.x, b.y, b.z, 1);
    verts_v.push_back(
        glm::vec3(glm::translate(glm::vec3(x1, y1, z1)) * newVert));
    normals_v.push_back(glm::normalize(glm::vec3(newVert)));
  }
  for (glm::vec3 b : baseRing) {
    glm::vec4 newVert = rotation * glm::vec4(b.x, b.y, b.z, 1);
    verts_v.push_back(
        glm::vec3(glm::translate(glm::vec3(x2, y2, z2)) * newVert));
    normals_v.push_back(glm::normalize(glm::vec3(newVert)));
  }

  verts_v.push_back({x1, y1, z1});
  normals_v.push_back(glm::normalize(glm::vec3(x1 - x2, y1 - y2, z1 - z2)));

  verts_v.push_back({x2, y2, z2});
  normals_v.push_back(glm::normalize(glm::vec3(x2 - x1, y2 - y1, z2 - z1)));

  // indices
  for (int i = 0; i < resolution; ++i) {
    int quadIndexA = i;
    int quadIndexB = (i + 1) % resolution;
    int quadIndexC = ((i + 1) % resolution) + resolution;
    int quadIndexD = i + resolution;

    indices_v.push_back(quadIndexA);
    indices_v.push_back(2 * resolution);
    indices_v.push_back(quadIndexB);

    indices_v.push_back(quadIndexA);
    indices_v.push_back(quadIndexB);
    indices_v.push_back(quadIndexC);
    indices_v.push_back(quadIndexC);
    indices_v.push_back(quadIndexD);
    indices_v.push_back(quadIndexA);

    indices_v.push_back(quadIndexC);
    indices_v.push_back(2 * resolution + 1);
    indices_v.push_back(quadIndexD);
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
};

void RenderObject::clearGeometry() { _vData.clear(); }

void RenderObject::genMultiLine(float thickness, int resolution, int numPoints,
                                float *points) {
  std::vector<glm::vec3> verts_v;
  std::vector<unsigned int> indices_v;
  std::vector<glm::vec3> normals_v;
  float nearTrunc, farTrunc = 0;
  glm::mat4 rotation = glm::identity<glm::mat4>();

  // generate base ring
  std::vector<glm::vec3> baseRing;
  for (int i = 0; i < resolution; ++i) {
    float angle = 2 * M_PI * i / resolution;
    baseRing.push_back(glm::vec3(0, 0.5 * thickness * std::sin(angle),
                                 -0.5 * thickness * std::cos(angle)));
  }
  // for each segment (pair of points)
  for (int p = 0; p < numPoints - 1; ++p) {

    float x0, y0, z0, x1, y1, z1, x2, y2, z2, nextAngle;
    glm::vec3 seg01, seg12, seg012axis;
    x0 = points[(3 * p)];
    y0 = points[(3 * p) + 1];
    z0 = points[(3 * p) + 2];
    x1 = points[(3 * p) + 3];
    y1 = points[(3 * p) + 4];
    z1 = points[(3 * p) + 5];
    seg01 = {x1 - x0, y1 - y0, z1 - z0};
    // rotate to face from current point to next point
    // compute lat/lon of first segment
    if (p == 0) {
      float lat = std::acos(glm::dot(
          glm::normalize(glm::vec3(seg01.x, seg01.y, 0)), glm::vec3(1, 0, 0)));
      float lon = glm::length(glm::vec3(seg01.x, 0, seg01.z)) == 0
                      ? 0
                      : std::acos(glm::dot(
                            glm::normalize(glm::vec3(seg01.x, 0, seg01.z)),
                            glm::vec3(1, 0, 0)));
      rotation = glm::mat4(cos(lon) * cos(lat), cos(lon) * sin(lat), sin(lon),
                           0, -sin(lat), cos(lat), 0, 0, -sin(lon) * cos(lat),
                           -sin(lon) * sin(lat), cos(lon), 0, 0, 0, 0, 1);
    } else {
      rotation = glm::rotate(nextAngle, seg012axis);
    }
    if (p + 2 < numPoints) {
      x2 = points[(3 * p) + 6];
      y2 = points[(3 * p) + 7];
      z2 = points[(3 * p) + 8];
      seg12 = {x2 - x1, y2 - y1, z2 - z1};
      nextAngle =
          std::acos(glm::dot(glm::normalize(seg01), glm::normalize(seg12)));
      seg012axis = glm::cross(glm::normalize(seg01), glm::normalize(seg12));
      farTrunc = fabs((0.5 * thickness) * std::tan(0.5 * nextAngle));
    }
    // rotate and translate to (x1,y1,z1) and add to vertex list
    for (glm::vec3 &b : baseRing) {
      b = rotation * glm::vec4(b, 1);
      verts_v.push_back(
          glm::vec3(glm::translate(glm::vec3(x0, y0, z0) +
                                   (nearTrunc * glm::normalize(seg01))) *
                    glm::vec4(b, 1)));
      normals_v.push_back(glm::normalize(glm::vec3(b)));
    }
    for (glm::vec3 &b : baseRing) {
      verts_v.push_back(
          glm::vec3(glm::translate(glm::vec3(x1, y1, z1) -
                                   (farTrunc * glm::normalize(seg01))) *
                    glm::vec4(b, 1)));
      normals_v.push_back(glm::normalize(glm::vec3(b)));
    }
    nearTrunc = farTrunc;
    farTrunc = 0;
  }

  verts_v.push_back({points[0], points[1], points[2]});
  normals_v.push_back(glm::normalize(glm::vec3(
      points[3] - points[0], points[4] - points[1], points[5] - points[2])));

  verts_v.push_back({points[(3 * (numPoints - 1)) + 0],
                     points[(3 * (numPoints - 1)) + 1],
                     points[(3 * (numPoints - 1)) + 2]});
  normals_v.push_back(glm::normalize(glm::vec3(
      points[(3 * (numPoints - 1))] - points[(3 * (numPoints - 1)) - 3],
      points[(3 * (numPoints - 1)) + 1] - points[(3 * (numPoints - 1)) - 2],
      points[(3 * (numPoints - 1)) + 2] - points[(3 * (numPoints - 1)) - 1])));

  // indices (pairs of segment endpoints)
  for (int j = 0; j < 2 * numPoints - 3; ++j) {
    for (int i = 0; i < resolution; ++i) {
      int quadIndexA = (j * resolution) + i;
      int quadIndexB = (j * resolution) + (i + 1) % resolution;
      int quadIndexC = (j * resolution) + ((i + 1) % resolution) + resolution;
      int quadIndexD = (j * resolution) + i + resolution;

      indices_v.push_back(quadIndexA);
      indices_v.push_back(quadIndexB);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexC);
      indices_v.push_back(quadIndexD);
      indices_v.push_back(quadIndexA);
    }
  }

  for (int i = 0; i < resolution; ++i) {
    indices_v.push_back(i);
    indices_v.push_back(verts_v.size() - 2);
    indices_v.push_back((i + 1) % resolution);
  }
  for (int i = 0; i < resolution; ++i) {
    indices_v.push_back((resolution * (2 * (numPoints - 1) - 1)) +
                        (i + 1) % resolution);
    indices_v.push_back(verts_v.size() - 1);
    indices_v.push_back((resolution * (2 * (numPoints - 1) - 1)) + i);
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

RenderObject::~RenderObject() {}

void RenderObject::recalc(glm::mat4 viewMat = glm::identity<glm::mat4>()) {
  _modelMat = glm::translate(_pos) * glm::toMat4(_rot);
  _normalMat = glm::inverseTranspose(glm::mat3(_modelMat));
}

void RenderObject::setPos(glm::vec3 newPos) {
  _pos = newPos;
  recalc();
};

void RenderObject::setRotation(glm::vec3 axis, float angle) {
  glm::vec3 normAxis(glm::normalize(axis));
  _rot = glm::quat(glm::cos(0.5 * angle), glm::sin(0.5 * angle) * normAxis.x,
                   glm::sin(0.5 * angle) * normAxis.y,
                   glm::sin(0.5 * angle) * normAxis.z);
  recalc();
}

void RenderObject::setRotation(glm::quat q) {
  _rot = q;
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