// wrapper for an OpenGL texture
#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad/gl.h"

#include <iostream>
#include <string>

class Texture {
public:
  // ctor
  Texture(const char *path);

  // getters
  int width() const { return _width; };
  int height() const { return _height; };
  int glSlot() const { return _glTextureSlot; };

  // bind texture for use in drawing geometry
  void use();

private:
  unsigned int _glId;
  unsigned int _glType;
  unsigned int _pixelFormat; // RGB, RGBA, etc.
  unsigned int _glTextureSlot;
  static int _nextGlTextureSlot;
  int _width;
  int _height;
};

#endif
