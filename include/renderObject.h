// a 3D mesh that can be rendered in the scene. this object can generate
// geometry for simple primitives and sets up OpenGL buffers for the mesh to
// be drawn.
#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "material.h"
#include "shader.h"
#include "texture.h"

#include <iostream>
#include <vector>

// experimental; for vector slerp (lerping cube normals)
#include <glm/gtx/rotate_vector.hpp>
const float CUBE_NORMAL_SLERP_FACTOR = 0.1f;

namespace GraphicsTools {

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

  // generate primitive geometries
  void genSphere(float radius, int numLatSegments, int numLonSegments);
  void genCube(float sideLength);
  void genPlane(float width, float depth);
  void genTorus(float majorRadius, float minorRadius, int numMinorSegments,
                int numMajorSegments);

  // draw object with OpenGL functions
  void draw(glm::mat4 viewMat, glm::mat4 projMat, glm::mat4 lightMat,
            ShaderProgram *overrideShader = NULL);

  // debug
  void debugPrint(std::ostream &out = std::cerr);

private:
  glm::vec3 _pos;
  glm::vec3 _rot;
  glm::mat4 _modelMat;       // computed from position and euler angles
  glm::mat4 _normalMat;      // used for lighting calcs
  std::vector<float> _vData; // interleaved positions, normals, and optional
                             // texture coordinates for OpenGL
  unsigned int _vDataWidth;  // number of floats to define a vertex; 8 if
                             // 2D textures are used, 6 otherwise
  unsigned int _vao, _vbo;   // vertex array and buffer (OpenGL)
                             // set from Scene when drawing
  ShaderProgram *_sp;
  Material _material; // including texture (diffuse map)

  void recalc(glm::mat4 viewMat);
};

} // namespace GraphicsTools

#endif