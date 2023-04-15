// a 3D mesh that can be rendered in the scene. this object can generate
// geometry for simple primitives and sets up OpenGL buffers for the mesh to
// be drawn.
#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "shader.h"
#include "texture.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <vector>

struct Material {
  glm::vec3 diffuse;
  Texture *diffuseMap;
  glm::vec3 specular;
  float shininess;
};

class RenderObject {
public:
  // ctor, dtor
  RenderObject();
  ~RenderObject();

  // generate geometry and set up OpenGL buffers
  void genSphere(float radius, int numLatSegments, int numLonSegments);
  void genCube(float sideLength);
  void genPlane(float width, float depth);
  void genTorus(float majorRadius, float minorRadius, int numMinorSegments,
                int numMajorSegments);

  // getters
  glm::vec3 pos() const { return _pos; };

  // draw object with OpenGL functions
  void draw(glm::mat4 viewMat, glm::mat4 projMat, glm::mat4 lightMat, ShaderProgram *overrideShader = NULL);

  // setters
  void setPos(glm::vec3 newPos);
  void setRotation(glm::vec3 eulerAngles);
  void setShader(ShaderProgram *prog) { _sp = prog; };
  // do this before creating geometry if using texture!
  void setTexture(Texture *tex) {
    _material->diffuseMap = tex;
    _vDataWidth = _material->diffuseMap ? 8 : 6;
  };
  void setMaterial(Material *mat) { _material = mat; };

  // debug
  void debugPrint();

private:
  glm::vec3 _pos;
  glm::vec3 _rot;
  glm::mat4 _modelMat; // model matrix
  glm::mat4 _normalMat;
  std::vector<glm::vec3> _verts;
  std::vector<unsigned int> _indices;
  unsigned int _preBufferLen;
  float *_preBuffer;        // interleaved positions, normals, and optional
                            // texture coordinates for OpenGL
  unsigned int _vDataWidth; // number of floats to define a vertex; 8 if
                            // textures are used, 6 otherwise
  unsigned int _vao, _vbo;  // vertex array and buffer (OpenGL)
  ShaderProgram *_sp;
  Material *_material; // texture is now part of material (as diffuse map)

  void recalc(glm::mat4 viewMat);
  void clearVertexData();
  void setupGl();
};

#endif