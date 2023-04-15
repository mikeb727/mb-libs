#include "texture.h"
#include "errors.h"
#include "glad/gl.h"

#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int Texture::_nextGlTextureSlot = 0;

Texture::Texture(const char *path) {
  _glType = GL_TEXTURE_2D; // assume only 2D textures for now
  _glTextureSlot = _nextGlTextureSlot++;
  glGenTextures(1, &_glId);
  glBindTexture(_glType, _glId);
  int numChannels;
  unsigned char *texData = stbi_load(path, &_width, &_height, &numChannels, 0);
  switch (numChannels) { // stb_image always uses RGBA for four channels, RGB
                         // for three
  case 4:
    _pixelFormat = GL_RGBA;
    break;
  case 3:
    _pixelFormat = GL_RGB;
    break;
  case 2:
    _pixelFormat = GL_RG;
    break;
  case 1:
    _pixelFormat = GL_RED;
    break;
  default:
    std::fprintf(stderr,
                 "texture %i: warning: invalid number of channels; "
                 "using GL_RGB as fallback texture pixel format!\n",
                 _glId);
  }
  glTexImage2D(_glType, 0, _pixelFormat, _width, _height, 0, _pixelFormat,
               GL_UNSIGNED_BYTE, texData);
  glTexParameteri(_glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(_glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(_glType);
  stbi_image_free(texData);
  glBindTexture(_glType, 0);
}

void Texture::use() {
  glActiveTexture(GL_TEXTURE0 + _glTextureSlot);
  glBindTexture(_glType, _glId);
}