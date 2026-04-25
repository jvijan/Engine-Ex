#include "TextDB.h"
#include "Helper.h"
#include "SDL2/SDL_render.h"
#include "SDL2_ttf/SDL_ttf.h"
#include "SDL_rect.h"
#include "SDL_surface.h"
#include <filesystem>
#include <iostream>

void TextDB::SetRenderer(SDL_Renderer* renderer) {
  TTF_Init();
  sdlRenderer = renderer;
}

void TextDB::LoadFont(const std::string &filename, int fontSize) {
  std::string base = "resources/fonts/" + filename;
  std::string path = base + ".ttf";
  if (!std::filesystem::exists(path))
    path = base + ".otf";

  if (!std::filesystem::exists(path)) {
    std::cout << "error: font " << filename << " missing";
    exit(0);
  }
  font = TTF_OpenFont(path.c_str(), fontSize);
  if (font == nullptr) {
    std::cout << "Font failed to load";
    exit(0);
  }
  // Concatenate font filename with font size so we don't have nested maps, put underscore in between to guarantee uniqueness
  // If no underscore, could have something like font32 with fontname font and size 32, and font32 with fontname font3 and size 2
  fontCache[filename + "_" + std::to_string(fontSize)] = font;
}

SDL_Texture* TextDB::CacheTexture(const std::string &text, const std::string &fontName, const int fontSize, SDL_Color color) {
  /*
  auto it = textCache.find(text);
  if (it != textCache.end()) {
    return it->second;
  }
  */
  std::string fontKey = fontName + "_" + std::to_string(fontSize);
  auto it = fontCache.find(fontKey);
  if (it == fontCache.end()) {
    LoadFont(fontName, fontSize);
  }
  SDL_Surface* surface = TTF_RenderText_Solid(fontCache[fontKey], text.c_str(), color);

  SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
  return texture;
}

void TextDB::DrawText(const TextDrawRequest &req) {
  SDL_Texture* texture = CacheTexture(req.text, req.fontName, req.fontSize, req.fontColor);
  float width;
  float height;
  Helper::SDL_QueryTexture(texture, &width, &height);

  SDL_FRect rect;
  rect.x = req.x;
  rect.y = req.y;
  rect.w = width;
  rect.h = height;
  Helper::SDL_RenderCopy(sdlRenderer, texture, nullptr, &rect);
}
