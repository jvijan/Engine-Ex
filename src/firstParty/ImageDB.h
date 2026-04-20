#ifndef IMAGEDB_H
#define IMAGEDB_H

#include "SDL2/SDL.h"
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

struct ImageDrawRequest {
  std::string imageName;
  float x;
  float y;
  float scaleX;
  float scaleY;
  float pivotX;
  float pivotY;
  int rotationDegrees;
  int sortingOrder;
  SDL_Color color;
  int pushNumber = 0;
};

struct UIDrawRequest {
  std::string imageName;
  int x;
  int y;
  int sortingOrder;
  SDL_Color color;
};

struct PixelDrawRequest {
  int x;
  int y;
  SDL_Color color;
};

class ImageDB {
private:

  inline static SDL_Renderer *sdlRenderer = nullptr;
  inline static std::unordered_map<std::string, SDL_Texture*> loadedTextures;
  inline static std::unordered_map<std::string, std::pair<float, float>> textureDimensions;

public:
  ImageDB();

  static void SetRenderer(SDL_Renderer *renderer);

  static void LoadAllTextures();

  static SDL_Texture* GetTexture(const std::string& name);

  static void CreateDefaultParticleTextureWithName(const std::string& name);

  static void GetTextureDimensions(const std::string& name, float& w, float& h);

  static void CacheTextureDimensions(const std::string& name, SDL_Texture* texture);

};

#endif
