#include "camera.h"

#include <cmath>
#include <iomanip>
#include <iostream>

#include <glm/gtx/string_cast.hpp> // debug; print matrices

namespace GraphicsTools {

Camera::Camera()
    : _pos(glm::zero<glm::vec3>()), _fov(45.0f), _aspectRatio(1.0f), _yaw(0.0f),
      _pitch(0.0f), _projType(CameraProjType::Undefined), _orthoWidth(0),
      _orthoHeight(0) {
  _view = glm::translate(glm::identity<glm::mat4>(), _pos);
}

void Camera::setPos(glm::vec3 pos) {
  _pos = pos;
  recalc();
};

void Camera::setYaw(float yaw) {
  _yaw = fmod(yaw, 360.0f);
  recalc();
};
void Camera::setPitch(float pitch) {
  _pitch = fmod(pitch, 360.0f);
  recalc();
};

void Camera::setPerspective(float fov, float aspectRatio) {
  _projType = CameraProjType::Perspective;
  _fov = fov;
  _aspectRatio = aspectRatio;
  _proj = glm::perspective(glm::radians(_fov), _aspectRatio, 0.1f, 2000.0f);
};

void Camera::setOrtho(float width, float height) {
  _projType = CameraProjType::Orthographic;
  _proj =
      glm::ortho(_pos.x - width / 2.0f, _pos.x + width / 2.0f,
                 _pos.y - height / 2.0f, _pos.y + height / 2.0f, 0.1f, 2000.0f);
};

void Camera::recalc() {
  glm::mat4 rotations =
      glm::rotate(glm::identity<glm::mat4>(), glm::radians(-_pitch),
                  glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::identity<glm::mat4>(), glm::radians(-_yaw),
                  glm::vec3(0.0f, 1.0f, 0.0f));
  _view = rotations * glm::translate(glm::identity<glm::mat4>(), -_pos);

  // for converting world axes to (camera) local
  glm::mat4 reverseRotations = glm::transpose(rotations);

  _right = glm::vec3(reverseRotations * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
  _forward = glm::vec3(reverseRotations * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
  _up = glm::cross(_right, _forward);
}

void Camera::debugPrint(std::ostream &out) const {
  out << std::setiosflags(std::ios::fixed) << std::setprecision(2);
  out << "camera\n";
  switch (_projType) {
  case Undefined:
    out << "undefined projection\n";
    break;
  case Perspective:
    out << "perspective projection; fov " << _fov << ", aspect ratio "
        << _aspectRatio << "\n";
    break;
  case Orthographic:
    out << "orthographic projection; width " << _orthoWidth << ", height "
        << _orthoHeight << "\n";
    break;
  }
  out << "world pos " << _pos.x << " " << _pos.y << " " << _pos.z << "\n";
  out << "local x " << _right.x << " " << _right.y << " " << _right.z << "\n";
  out << "local y " << _forward.x << " " << _forward.y << " " << _forward.z
      << "\n";
  out << "local z " << _up.x << " " << _up.y << " " << _up.z << "\n";
  out << "yaw " << _yaw << " pitch " << _pitch << "\n";
  out << "view matrix :\n" << glm::to_string(_view) << "\n";
  out << "projection matrix :\n" << glm::to_string(_proj) << "\n";

}

} // namespace GraphicsTools