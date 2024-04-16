// a camera for viewing 2D or 3D space.
#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

// matrix math
#include <glm/ext.hpp>
#include <glm/glm.hpp>

// glm now requires this flag to use vector rotation
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp> // debug; print matrices

namespace GraphicsTools {

// available camera projection types
// undefined: default case; no functionality
enum CameraProjType { Undefined, Perspective, Orthographic };

class Camera {
public:
  // ctor
  Camera();
  // using default copy ctor

  // getters
  glm::vec3 pos() const { return _pos; };
  // local axes
  glm::vec3 localRight() const { return _right; };
  glm::vec3 localForward() const { return _forward; };
  glm::vec3 localUp() const { return _up; };
  float yaw() const { return _yaw; };
  float pitch() const { return _pitch; };
  float fov() const { return _fov; };
  float aspectRatio() const { return _aspectRatio; };
  CameraProjType projection() const { return _projType; };
  // separate for doing translation along camera axes vs. world axes
  glm::mat4 viewMatrix() const { return _view; };
  glm::mat4 projMatrix() const { return _proj; };

  // setters
  void setPos(glm::vec3 pos);
  void setYaw(float yaw);
  void setPitch(float pitch);
  // also change projection type
  void setPerspective(float fov, float aspectRatio);
  void setOrtho(float width, float height);
  void setOrtho2(float width, float height);

  // debug
  void debugPrint(std::ostream &out = std::cerr,
                  bool printMatrices = false) const;

  friend std::ostream &operator<<(std::ostream &os, const Camera &c);

private:
  glm::vec3 _pos, _right, _forward, _up;
  glm::mat4 _view, _proj;
  float _orthoWidth, _orthoHeight;
  CameraProjType _projType;
  float _fov, _aspectRatio;
  float _yaw, _pitch;
  // recompute transformation matrices after setting position 
  void recalc();
  // create debug output stream
  void writeDebugStream(std::ostream &os) const;
};

} // namespace GraphicsTools

#endif