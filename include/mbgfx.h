/* Functions for using graphics with OpenGL. Includes
    the OpenGL headers, so they don't have to be
    included in main. */

#ifndef IMAGES_H
#define IMAGES_H

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

#include "glad/gl.h"

#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>

namespace GraphicsTools {

// library setup/teardown
int InitGraphics();
int CloseGraphics();

// utility; transform a number x from [a1, a2] to [b1, b2]
float remap(float x, float a1, float a2, float b1, float b2);

} // namespace GraphicsTools

#endif
