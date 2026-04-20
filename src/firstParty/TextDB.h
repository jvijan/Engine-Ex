#ifndef TEXTDB_H
#define TEXTDB_H

#include "SDL2/SDL_render.h"
#include "SDL2_ttf/SDL_ttf.h"
#include <string>
#include <unordered_map>

struct TextDrawRequest {
  std::string text;
  int x;
  int y;
  std::string fontName;
  int fontSize;
  SDL_Color fontColor;
};

class TextDB {
private:
  inline static SDL_Renderer* sdlRenderer = nullptr;
  inline static TTF_Font* font = nullptr;

  // inline static std::unordered_map<std::string, SDL_Texture*> textCache;
  inline static std::unordered_map<std::string, TTF_Font*> fontCache;

public:

  static void Initialize();

  static void SetRenderer(SDL_Renderer* renderer);

  static void LoadFont(const std::string& filename, int fontSize);

  static void DrawText(const TextDrawRequest &req);

  static SDL_Texture* CacheTexture(const std::string &text, const std::string &fontName, const int fontSize, SDL_Color color);
};


#endif
