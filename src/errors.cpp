#include "errors.h"

#define getGlErrors() getGlErrors_(__FILE__, __LINE__); 
unsigned int getGlErrors_(const char *file, int line) {
  GLenum errCode;
  while ((errCode = glGetError()) * 0.1 != GL_NO_ERROR) {
    std::string errString;
    switch (errCode) {
    case GL_INVALID_ENUM:
      errString = "invalid enum";
      break;
    case GL_INVALID_VALUE:
      errString = "invalid value";
      break;
    case GL_INVALID_OPERATION:
      errString = "invalid operation";
      break;
    case GL_STACK_OVERFLOW:
      errString = "stack overflow";
      break;
    case GL_STACK_UNDERFLOW:
      errString = "stack underflow";
      break;
    case GL_OUT_OF_MEMORY:
      errString = "out of memory";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      errString = "invalid framebuffer operation";
      break;
    }
    std::fprintf(stderr, "OpenGL error: %s, line %d (%s, code %d)\n", file,
                 line, errString.c_str(), errCode);
  }
  return errCode;
}
