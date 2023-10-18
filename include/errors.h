// gets and prints all OpenGL errors
#ifndef ERRORS_H
#define ERRORS_H

#include "glad/gl.h"

#include <string>

namespace GraphicsTools {

#define getGlErrors() getGlErrors_(__FILE__, __LINE__);
unsigned int getGlErrors_(const char *file, int line);

} // namespace GraphicsTools

#endif