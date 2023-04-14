// gets and prints all OpenGL errors
#ifndef ERRORS_H
#define ERRORS_H

#include "glad/gl.h"
#include <string>

unsigned int getGlErrors_(const char *file, int line);
#define getGlErrors() getGlErrors_(__FILE__, __LINE__); 
#endif