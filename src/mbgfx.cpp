#include "mbgfx.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>

using std::cerr;

namespace GraphicsTools {

int InitGraphics() {
  int result = glfwInit();
  if (!result){
    int errCode; const char *errLog; errCode  = glfwGetError(&errLog);
    cerr << "could not load GLFW: " << errLog << " (GLFW error " << errCode << ")\n";
    return result;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  
}

int InitText() {
  int result = TTF_Init();
  if (result != 0) {
    cerr << SDL_GetError() << "\n";
  }
  return result;
}

int CloseGraphics() {
  glfwTerminate();
  return 0;
}

int CloseText() {
  TTF_Quit();
  return 0;
}

ColorRgba blend(ColorRgba c1, double w1, ColorRgba c2, double w2) {
  double sumWeights = w1 + w2;
  ColorRgba result = {((w1 * c1.r + w2 * c2.r) / sumWeights),
                      ((w1 * c1.g + w2 * c2.g) / sumWeights),
                      ((w1 * c1.b + w2 * c2.b) / sumWeights),
                      ((w1 * c1.a + w2 * c2.a) / sumWeights)};
  return result;
}

// see https://stackoverflow.com/questions/3018313/
ColorRgba hsv2rgb(ColorHsv in) {
  double hh, p, q, t, ff;
  long i;
  ColorRgba out;

  // temp
  out.a = 1.0;

  if (in.s <= 0.0) { // < is bogus, just shuts up warnings
    out.r = in.v;
    out.g = in.v;
    out.b = in.v;
    return out;
  }
  hh = in.h;
  if (hh >= 360.0)
    hh = 0.0;
  hh /= 60.0;
  i = (long)hh;
  ff = hh - i;
  p = in.v * (1.0 - in.s);
  q = in.v * (1.0 - (in.s * ff));
  t = in.v * (1.0 - (in.s * (1.0 - ff)));

  switch (i) {
  case 0:
    out.r = in.v;
    out.g = t;
    out.b = p;
    break;
  case 1:
    out.r = q;
    out.g = in.v;
    out.b = p;
    break;
  case 2:
    out.r = p;
    out.g = in.v;
    out.b = t;
    break;
  case 3:
    out.r = p;
    out.g = q;
    out.b = in.v;
    break;
  case 4:
    out.r = t;
    out.g = p;
    out.b = in.v;
    break;
  case 5:
  default:
    out.r = in.v;
    out.g = p;
    out.b = q;
    break;
  }
  return out;
}

ColorRgba randomColor() {
  std::default_random_engine generator;
  generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<double> hueDist(0, 360);
  std::normal_distribution<double> satDist(0.8, 0.2);
  std::normal_distribution<double> valDist(0.5, 0.2);
  double hue = hueDist(generator);
  double sat = satDist(generator);
  double val = valDist(generator);
  std::cerr << "[imageTools] hue " << hue << " sat " << sat << " val " << val
            << "\n";
  ColorRgba intermediate = GraphicsTools::hsv2rgb(GraphicsTools::ColorHsv{
      hue, std::clamp(sat, 0.0, 1.0), std::clamp(val, 0.0, 1.0)});
  return ColorRgba{(intermediate.r * 255), (intermediate.g * 255),
                   (intermediate.b * 255), 255};
}

Font::Font(const char *fontFile, int fontSize) : size(fontSize) {
  filename = fontFile;
  f = TTF_OpenFont(fontFile, fontSize);
  if (f == NULL) {
    cerr << "font is null: " << TTF_GetError() << "\n";
  }
}

Font::~Font() { TTF_CloseFont(f); }

Window::Window(std::string n, int width, int height)
    : name(n), _width(width), _height(height) {

  // Initialize window
  win = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, _width, _height,
                         SDL_WINDOW_SHOWN);
  if (win == NULL) {
    cerr << SDL_GetError() << "\n";
  } else {
    // Initialize renderer
    ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == NULL) {
      cerr << SDL_GetError() << "\n";
    }
  }
}

Window::~Window() {
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
}

void Window::update() { SDL_RenderPresent(ren); }

SDL_Texture *Window::loadImage(std::string fileName) {
  SDL_Texture *tex = IMG_LoadTexture(ren, fileName.c_str());
  if (tex == NULL) {
    std::cout << SDL_GetError() << "\n";
  }
  return tex;
}

void Window::drawImage(SDL_Texture *img, int x, int y, int alpha) {
  SDL_Rect pos;
  pos.x = x;
  pos.y = y;

  SDL_QueryTexture(img, NULL, NULL, &pos.w, &pos.h);
  SDL_SetTextureBlendMode(img, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(img, alpha);
  SDL_RenderCopy(ren, img, NULL, &pos);
}

void Window::clear() { SDL_RenderClear(ren); }

void Window::drawRectangle(GraphicsTools::ColorRgba color, int x, int y, int w,
                           int h) {
  SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
  SDL_Rect target;
  target.x = x;
  target.y = y;
  target.w = w;
  target.h = h;
  SDL_RenderFillRect(ren, &target);
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
}

void Window::drawCircle(GraphicsTools::ColorRgba color, int x, int y, int r) {
  SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
  for (int _y = y - r; _y <= y + r; _y++) {
    int w = sqrt((r * r) - ((_y - y) * (_y - y)) - r);
    SDL_RenderDrawLine(ren, x - w, _y, x + w, _y);
  }
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
}

void Window::drawCircleGradient(GraphicsTools::ColorRgba outer,
                                GraphicsTools::ColorRgba inner, int x, int y,
                                int r) {
  for (int _y = y - r; _y <= y + r; _y++) {
    int w = sqrt(pow(r, 2) - pow(_y - y, 2) - r);
    for (int _x = x - w; _x <= x + w; _x++) {
      int d = sqrt(pow(abs(x - _x), 2) + pow(abs(y - _y), 2));
      ColorRgba pointColor = blend(outer, d, inner, r - d);
      SDL_SetRenderDrawColor(ren, pointColor.r, pointColor.g, pointColor.b,
                             pointColor.a);
      SDL_RenderDrawPoint(ren, _x, _y);
    }
  }
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
}
void Window::drawText(std::string text, GraphicsTools::Font *font,
                      GraphicsTools::ColorRgba color, int x, int y,
                      GraphicsTools::TextAlignModeH al) {

  int alignmentShift = 0;

  // We need to first render to a surface as that's what TTF_RenderText returns,
  // then load that surface into a texture
  SDL_Surface *surf = TTF_RenderText_Blended(
      font->font(), text.c_str(),
      SDL_Color({(unsigned char)color.r, (unsigned char)color.g,
                 (unsigned char)color.b, (unsigned char)color.a}));
  if (surf == NULL) {
    std::cerr << "surface is null: " << SDL_GetError() << "\n";
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surf);
  if (texture == NULL) {
    std::cerr << "texture is null: " << SDL_GetError() << "\n";
  }

  // shift the x-coordinate based on the text alignment
  if (al == Center) {
    TTF_SizeText(font->font(), text.c_str(), &alignmentShift, NULL);
    alignmentShift = alignmentShift / 2;
  } else if (al == Right) {
    TTF_SizeText(font->font(), text.c_str(), &alignmentShift, NULL);
  }

  SDL_Rect pos;
  pos.x = x - alignmentShift;
  pos.y = y;
  SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);

  SDL_RenderCopy(ren, texture, NULL, &pos);

  // Clean up the surface and font
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surf);
}

void Window::drawLine(GraphicsTools::ColorRgba color, int thickness, int x1,
                      int y1, int x2, int y2) {
  SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
  for (int dy = -thickness / 2; dy <= thickness / 2; dy++) {
    int w = sqrt(pow(thickness / 2, 2) - pow(dy, 2));
    for (int dx = -w; dx <= w; dx++) {
      SDL_RenderDrawLine(ren, x1 + dx, y1 + dy, x2 + dx, y2 + dy);
    }
  }
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
}

} // namespace GraphicsTools
