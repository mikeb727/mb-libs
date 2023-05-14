#include "mbgfx.h"
#include "camera.h"

#include <GLFW/glfw3.h>
#include <cmath>

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

float remap(float x, float a1, float a2, float b1, float b2){
  return (x - a1) * ((b2 - b1) / (a2 - a1)) + b1;
}

Font::Font(std::string file, int fontSize) : _size(fontSize), _ready(false) {
  _filename = file;
  FT_Library ft;
  int result = FT_Init_FreeType(&ft);
  // load freetype for drawing text
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not load FreeType: " << errLog << " (FreeType error "
              << result << ")\n";
    return;
  }

  // load our font
  FT_Face font;
  result = FT_New_Face(ft, _filename.c_str(), 0, &font);
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not load font \"" << _filename << "\": " << errLog
              << " (FreeType error " << result << ")\n";
    return;
  }

  result = FT_Set_Pixel_Sizes(font, 0, _size);
  if (result) {
    const char *errLog = FT_Error_String(result);
    std::cerr << "could not set font pixel sizes: " << errLog
              << " (FreeType error " << result << ")\n";
    return;
  }

  // load glyph textures into memory
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (unsigned char c = 0; c < 128; ++c) {
    result = FT_Load_Char(font, c, FT_LOAD_RENDER);
    if (result) {
      const char *errLog = FT_Error_String(result);
      std::cerr << "could not load glyph for character \"" << c
                << "\": " << errLog << " (FreeType error " << result << ")\n";
      continue;
    }
    unsigned int glyphTex;
    glGenTextures(1, &glyphTex);
    glBindTexture(GL_TEXTURE_2D, glyphTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->glyph->bitmap.width,
                 font->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 font->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TextGlyph ch = {glyphTex,
                    font->glyph->bitmap.width,
                    font->glyph->bitmap.rows,
                    font->glyph->bitmap_left,
                    font->glyph->bitmap_top,
                    font->glyph->advance.x};
    _glyphs.emplace(c, ch);
  }
  FT_Done_Face(font);
  FT_Done_FreeType(ft);
  _ready = true;
}

Font::~Font() {}

} // namespace GraphicsTools
