// a camera for viewing 3D space.
#ifndef CAMERA_H
#define CAMERA_H

// matrix math
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <iostream>

enum CameraProjType { Undefined, Perspective, Orthographic };

class Camera {
public:
  // ctor
  Camera();

  // getters
  glm::vec3 pos() const { return _pos; };
  // local axes
  glm::vec3 right() const { return _right; };
  glm::vec3 forward() const { return _forward; };
  glm::vec3 up() const { return _up; };
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
  void setPerspective(float fov, float aspectRatio);
  void setOrtho(float width, float height);

  // debug
  void debugPrint(std::ostream &out = std::cerr) const;

private:
  glm::vec3 _pos, _right, _forward, _up;
  glm::mat4 _view, _proj;
  float _orthoWidth, _orthoHeight;
  CameraProjType _projType;
  float _fov, _aspectRatio;
  float _yaw, _pitch;
  // recompute transformation matrices
  void recalc();
};

#endif