#ifndef FONT_H
#define FONT_H

#include "glad/gl.h"

#include <map>
#include <string>
#include <iostream>

#include <freetype/freetype.h>

namespace GraphicsTools {

enum TextAlignModeH { Left, Center, Right };

// data extracted from font using freetype
struct TextGlyph {
  unsigned int glTextureId;
  unsigned int sizeX, sizeY;
  int bearingX, bearingY;
  long charAdvance; // x distance to next glyph
};
// map ASCII characters to their glyphs
typedef std::map<char, TextGlyph> charMap;

class Font {
private:
  int _size;
  std::string _filename;
  charMap _glyphs;
  bool _ready;

public:
  Font(std::string file, int displaySize); // size in points
  ~Font();
  TextGlyph glyph(char ch) const { return _glyphs.at(ch); };
  bool isReady() const { return _ready; };
  int size() const { return _size; };
};

} // namespace GraphicsTools

#endif