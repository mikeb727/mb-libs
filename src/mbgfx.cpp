#include "mbgfx.h"

namespace GraphicsTools {

int InitGraphics() {
  int result = glfwInit();
  if (!result) {
    const char *errLog;
    int errCode = glfwGetError(&errLog);
    std::cerr << "could not load GLFW: " << errLog << " (GLFW error " << errCode
              << ")\n";
    return result;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  return 1;
}

int CloseGraphics() {
  glfwTerminate();
  return 0;
}

float remap(float x, float a1, float a2, float b1, float b2) {
  return (x - a1) * ((b2 - b1) / (a2 - a1)) + b1;
}

} // namespace GraphicsTools
