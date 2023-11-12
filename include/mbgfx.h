/* Functions for using graphics with OpenGL. Includes
    the OpenGL headers, so they don't have to be
    included in main. */

#ifndef IMAGES_H
#define IMAGES_H

// include glad for OpenGL and GLX before any of our headers!
#include "glad/gl.h"
#include "glad/glx.h"

#include "camera.h"
#include "colors.h"
#include "errors.h"
#include "font.h"
#include "light.h"
#include "material.h"
#include "renderObject.h"
#include "scene.h"
#include "shader.h"
#include "texture.h"
#include "window.h"

#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

// for creating X11 windows with GLX
// works, but to be completed at a later date
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
// function pointer; takes (Display*, GLXFBConfig, GLXContext, Bool, const int*)
// as args and returns GLX context
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig,
                                                     GLXContext, Bool,
                                                     const int *);

namespace GraphicsTools {

enum GraphicsMode { Glfw, Glx };

// library setup/teardown
int InitGraphics();
int InitForGlx();
int CloseGraphics();

// utility; transform a number x from [a1, a2] to [b1, b2]
float remap(float x, float a1, float a2, float b1, float b2);

} // namespace GraphicsTools

#endif
