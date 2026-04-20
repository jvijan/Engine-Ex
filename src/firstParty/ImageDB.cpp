#include "ImageDB.h"
#include <filesystem>
#include <iostream>
#include <string>
#include "SDL2_image/SDL_image.h"
#include "SceneDB.h"
#include "Helper.h"


ImageDB::ImageDB() {
}

void ImageDB::SetRenderer(SDL_Renderer *renderer) {
  sdlRenderer = renderer;
  LoadAllTextures();
}

void ImageDB::LoadAllTextures() {
  const std::string imagePath = "resources/images";
  if (!std::filesystem::exists(imagePath)) {
    return;
  }

  for (const auto &imageFile: std::filesystem::directory_iterator(imagePath)) {
    std::string name = imageFile.path().stem().string();
    SDL_Texture* texture = IMG_LoadTexture(sdlRenderer, imageFile.path().string().c_str());
    loadedTextures[name] = texture;
    CacheTextureDimensions(name, texture);
  }

}

SDL_Texture* ImageDB::GetTexture(const std::string &name) {
  auto it = loadedTextures.find(name);
  if (it == loadedTextures.end()) {
    std::cout << "error: missing image " + name;
    exit(0);
  }
  return it->second;
}


void ImageDB::CreateDefaultParticleTextureWithName(const std::string &name) {
  if (loadedTextures.find(name) != loadedTextures.end()) {
    return;
  }

  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

  Uint32 whiteColor = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
  SDL_FillRect(surface, nullptr, whiteColor);

  SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
  SDL_FreeSurface(surface);
  loadedTextures[name] = texture;
  CacheTextureDimensions(name, texture);
  return;
}

void ImageDB::GetTextureDimensions(const std::string &name, float &width, float &height) {
  auto it = textureDimensions.find(name);
  if (it == textureDimensions.end()) {
    return;
  }
  width = it->second.first;
  height = it->second.second;
}

void ImageDB::CacheTextureDimensions(const std::string &name, SDL_Texture *texture) {
  float width;
  float height;
  Helper::SDL_QueryTexture(texture, &width, &height);
  textureDimensions[name] = {width, height};
}