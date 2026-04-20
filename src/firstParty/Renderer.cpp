#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include "Renderer.h"
#include "AudioDB.h"
#include "AudioHelper.h"
#include "Engine.h"
#include "ImageDB.h"
#include "Input.h"
#include "SDL2/SDL.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SceneDB.h"
#include "TextDB.h"
#include "glm/common.hpp"
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "EngineUtils.h"
#include "Helper.h"

//#define DebugGizmos

void Renderer::LoadRenderingProperties() {
  rapidjson::Document gameConfig;
  EngineUtils::ReadJsonFile("resources/game.config", gameConfig);
  if (gameConfig.HasMember("game_title")) {
    gameTitle = gameConfig["game_title"].GetString();
  }
  if (gameConfig.HasMember("intro_image")) {
    const rapidjson::Value& introNames = gameConfig["intro_image"];
    for (rapidjson::SizeType i = 0; i < introNames.Size(); i++) {
      const std::string currIntroimage = introNames[i].GetString();
      ImageDrawRequest currImage;
      currImage.imageName = currIntroimage;
      introImages.push_back(currImage);
    }
  }

  if (gameConfig.HasMember("font")) {
    const std::string font = gameConfig["font"].GetString();
    TextDB::LoadFont(font, 16);
  }

  if (gameConfig.HasMember("intro_text")) {
    if (!gameConfig.HasMember("font")) {
      std::cout << "error: text render failed. No font configured";
      exit(0);
    }

    const rapidjson::Value& introTextArray = gameConfig["intro_text"];
    for (rapidjson::SizeType i = 0; i < introTextArray.Size(); i++) {
      const std::string currIntroText = introTextArray[i].GetString();
      introTexts.push_back(currIntroText);
    }
  }

  if (gameConfig.HasMember("initial_scene")) {
    SceneDB::SwitchToScene(gameConfig["initial_scene"].GetString());
  }



  if (!std::filesystem::exists("resources/rendering.config")) {
    return;
  }
  rapidjson::Document renderConfig;
  EngineUtils::ReadJsonFile("resources/rendering.config", renderConfig);
  if (renderConfig.HasMember("x_resolution")) {
    windowSize.x = renderConfig["x_resolution"].GetInt();
  }
  if (renderConfig.HasMember("y_resolution")) {
    windowSize.y = renderConfig["y_resolution"].GetInt();
  }
  if (renderConfig.HasMember("clear_color_r")) {
    clearColorR = renderConfig["clear_color_r"].GetInt();
  }
  if (renderConfig.HasMember("clear_color_g")) {
    clearColorG = renderConfig["clear_color_g"].GetInt();
  }
  if (renderConfig.HasMember("clear_color_b")) {
    clearColorB = renderConfig["clear_color_b"].GetInt();
  }


}

void Renderer::InitializeRendering() {
  ActorTemplateDB::LoadTemplates();
  SceneDB::LoadAllScenes();
  LoadRenderingProperties();
  sdlWindow = Helper::SDL_CreateWindow(gameTitle.c_str(), windowPos.x, windowPos.y, windowSize.x, windowSize.y, SDL_WINDOW_SHOWN);
  sdlRenderer = Helper::SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  ImageDB::SetRenderer(sdlRenderer);
  TextDB::SetRenderer(sdlRenderer);
  AudioDB::Initialize();
  SDL_SetRenderDrawColor(sdlRenderer, clearColorR, clearColorG, clearColorB, 255);
  SDL_RenderClear(sdlRenderer);
}

void Renderer::RenderFrame() {
  ClearScreen();
  RenderAndClearAllImages();
  RenderAndClearAllUI();
  RenderAndClearAllText();
  RenderAndClearAllPixels();
  pushNumber = 0;

  // Update the screen
  Helper::SDL_RenderPresent(sdlRenderer);
  frameRendered = true;
}

void Renderer::RenderAndClearAllImages() {
  imageDrawRequestQueue.erase(
    std::remove_if(imageDrawRequestQueue.begin(), imageDrawRequestQueue.end(), 
    [](ImageDrawRequest& req) {
      return !IsOnScreen(req);
    }),
    imageDrawRequestQueue.end()
  );
  std::stable_sort(imageDrawRequestQueue.begin(), imageDrawRequestQueue.end(), [](const ImageDrawRequest &a, const ImageDrawRequest &b) {
    return a.sortingOrder < b.sortingOrder;
  });
  std::stable_sort(particleDrawRequestQueue.begin(), particleDrawRequestQueue.end(), [](const ParticleDrawRequest &a, const ParticleDrawRequest &b) {
    return a.sortingOrder < b.sortingOrder;
  });

  SDL_RenderSetScale(sdlRenderer, zoomFactor, zoomFactor);
  auto imgIt = imageDrawRequestQueue.begin();
  auto imgEnd = imageDrawRequestQueue.end();
  auto parIt = particleDrawRequestQueue.begin();
  auto parEnd = particleDrawRequestQueue.end();
  // Iterate over the two queues, choosing which to get next and drawing it
  while (imgIt != imgEnd || parIt != parEnd) {
    bool drawImage = false;
    if (parIt == parEnd || (imgIt != imgEnd && (imgIt->sortingOrder < parIt->sortingOrder || (imgIt->sortingOrder == parIt->sortingOrder && imgIt->pushNumber < parIt->pushNumber)))) {
      drawImage = true;
    }
    SDL_Texture* texture;
    float width;
    float height;
    if (drawImage) {
      const ImageDrawRequest& req = *imgIt;
      texture = ImageDB::GetTexture(req.imageName);
      ImageDB::GetTextureDimensions(req.imageName, width, height);
      DrawImageRequest(texture, width, height, req.x, req.y, req.rotationDegrees, req.scaleX, req.scaleY, req.pivotX, req.pivotY, req.color);
      SDL_SetTextureColorMod(texture, 255, 255, 255);
      SDL_SetTextureAlphaMod(texture, 255);
      imgIt++;
    }
    else {
      const ParticleDrawRequest& req = *parIt;
      texture = ImageDB::GetTexture(req.textureName);
      ImageDB::GetTextureDimensions(req.textureName, width, height);
      size_t particles = req.pos.size();
      for (size_t i = 0; i < particles; i++) {
        // Cull particles that aren't on screen
        if (!IsParticleOnScreen(req.pos[i], req.scales[i], width, height)) continue;
        DrawImageRequest(texture, width, height, req.pos[i].x, req.pos[i].y, req.rotationDegrees[i], req.scales[i],
          req.scales[i], 0.5f, 0.5f, req.colors[i]);
      }
      SDL_SetTextureColorMod(texture, 255, 255, 255);
      SDL_SetTextureAlphaMod(texture, 255);
      parIt++;
    }
  }
  imageDrawRequestQueue.clear();
  particleDrawRequestQueue.clear();
  SDL_RenderSetScale(sdlRenderer, 1, 1);
}


void Renderer::RenderAndClearAllText() {
  for (const TextDrawRequest &req : textDrawRequestQueue) {
    TextDB::DrawText(req);
  }
  textDrawRequestQueue.clear();
}

void Renderer::RenderAndClearAllUI() {
  std::stable_sort(uiDrawRequestQueue.begin(), uiDrawRequestQueue.end(), [](const UIDrawRequest &a, const UIDrawRequest &b) {
    return a.sortingOrder < b.sortingOrder;
  });
  for (const UIDrawRequest &req: uiDrawRequestQueue) {
    SDL_FRect rect;
    rect.x = req.x;
    rect.y = req.y;
    SDL_Texture* texture = ImageDB::GetTexture(req.imageName);
    Helper::SDL_QueryTexture(texture, &rect.w, &rect.h);
    SDL_SetTextureColorMod(texture, req.color.r, req.color.g, req.color.b);
    SDL_SetTextureAlphaMod(texture, req.color.a);
    Helper::SDL_RenderCopy(sdlRenderer, texture, nullptr, &rect);
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);
  }
  uiDrawRequestQueue.clear();
}

void Renderer::RenderAndClearAllPixels() {
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
  for (const PixelDrawRequest &req : pixelDrawRequestQueue) {
    SDL_SetRenderDrawColor(sdlRenderer, req.color.r, req.color.g, req.color.b, req.color.a);
    SDL_RenderDrawPoint(sdlRenderer, req.x, req.y);
  }
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
  pixelDrawRequestQueue.clear();
}

void Renderer::ClearScreen() {
  // Clear frame buffer at beginning of each frame
  SDL_SetRenderDrawColor(sdlRenderer, clearColorR, clearColorG, clearColorB, 255);
  SDL_RenderClear(sdlRenderer);
}

void Renderer::AddTextRequest(const std::string &text, const float x, const float y, const std::string fontName,
                                  const float fontSize, const float r, const float g, const float b, const float a) {
  textDrawRequestQueue.push_back({text, (int)x, (int)y, fontName, (int)fontSize, SDL_Color{(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a}});
}

void Renderer::DrawImageRequest(SDL_Texture* texture, float width, float height, float x, float y, float rotationDegrees,
                                float scaleX, float scaleY, float pivotX, float pivotY, SDL_Color color) {
    SDL_FRect rect;
    rect.w = width;
    rect.h = height;

    glm::vec2 finalRenderingPos = glm::vec2(x, y) - cameraPos;

    // Choose flip mode based on negative scale
    int flipMode = SDL_FLIP_NONE;
    if (scaleX < 0) {
        flipMode |= SDL_FLIP_HORIZONTAL;
    }
    if (scaleY < 0) {
        flipMode |= SDL_FLIP_VERTICAL;
    }

    // Apply scale to rect
    float xScale = glm::abs(scaleX);
    float yScale = glm::abs(scaleY);

    rect.w *= xScale;
    rect.h *= yScale;

    // Set pivot point based on scale and request pivot points
    SDL_FPoint pivotPoint { pivotX * rect.w, pivotY * rect.h };

    // Set the x and y to be at the pivot point, along with scale it using the window size center
    rect.x = static_cast<int>((finalRenderingPos.x * pixelsPerMeter + windowSize.x * 0.5f / zoomFactor) - pivotPoint.x);
    rect.y = static_cast<int>((finalRenderingPos.y * pixelsPerMeter + windowSize.y * 0.5f / zoomFactor) - pivotPoint.y);

    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);
    Helper::SDL_RenderCopyEx(0, "dummy", sdlRenderer, texture, nullptr, &rect, rotationDegrees, &pivotPoint, static_cast<SDL_RendererFlip>(flipMode));
}

void Renderer::DrawUI(const std::string &imageName, const float x, const float y) {
  DrawUIEx(imageName, x, y, 255, 255, 255, 255, 0);
}

void Renderer::DrawUIEx(const std::string &imageName, const float x, const float y, const float r, const float g,
                      const float b, const float a, const float sortingOrder) {
  uiDrawRequestQueue.push_back({imageName, (int)x, (int)y, (int)sortingOrder, SDL_Color{(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a}});
}

void Renderer::Draw(const std::string &imageName, const float x, const float y) {
  DrawEx(imageName, x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
}

void Renderer::DrawEx(const std::string &imageName, const float x, const float y, const float rotationDegrees,
                    const float scaleX, const float scaleY, const float pivotX, const float pivotY,
                    const float r, const float g, const float b, const float a, const float sortingOrder) {
  imageDrawRequestQueue.push_back({imageName, x, y, scaleX, scaleY, pivotX, pivotY, (int)rotationDegrees, (int)sortingOrder, SDL_Color{(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a}, pushNumber++});
}

void Renderer::DrawPixel(const float x, const float y, const float r, const float g, const float b, const float a) {
  pixelDrawRequestQueue.push_back({(int)x, (int)y, SDL_Color{(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a}});
}

void Renderer::DrawParticleBatch(const int sortingOrder, const std::string& textureName, std::vector<glm::vec2> positions,
                                  std::vector<float> rotations, std::vector<float> scales, std::vector<SDL_Color> colors) {
    particleDrawRequestQueue.push_back({pushNumber++, sortingOrder,textureName,std::move(positions), std::move(rotations), std::move(scales), std::move(colors)});
}

void Renderer::ClearAllQueues() {
  uiDrawRequestQueue.clear();
  textDrawRequestQueue.clear();
  imageDrawRequestQueue.clear();
}

void Renderer::SetCameraPos(const float x, const float y) {
  cameraPos = glm::vec2(x, y);
}

float Renderer::GetCameraPosX() {
  return cameraPos.x;
}

float Renderer::GetCameraPosY() {
  return cameraPos.y;
}

void Renderer::SetZoom(const float newZoomFactor) {
  zoomFactor = newZoomFactor;
}

float Renderer::GetZoom() {
  return zoomFactor;
}

bool Renderer::IsOnScreen(ImageDrawRequest &req) {
  SDL_FRect rect;
  SDL_Texture* texture = ImageDB::GetTexture(req.imageName);
  ImageDB::GetTextureDimensions(req.imageName, rect.w, rect.h);
  float screenLeft = 0.0f;
  float screenRight = windowSize.x / zoomFactor;
  float screenTop = 0.0f;
  float screenBottom = windowSize.y / zoomFactor;


  glm::vec2 finalRenderingPos = glm::vec2(req.x, req.y) - cameraPos;

  // Choose flip mode based on negative scale
  int flipMode = SDL_FLIP_NONE;
  if (req.scaleX < 0) {
      flipMode |= SDL_FLIP_HORIZONTAL;
  }
  if (req.scaleY < 0) {
      flipMode |= SDL_FLIP_VERTICAL;
  }

  // Apply scale to rect
  float xScale = glm::abs(req.scaleX);
  float yScale = glm::abs(req.scaleY);

  rect.w *= xScale;
  rect.h *= yScale;

  // Set pivot point based on scale and request pivot points
  SDL_FPoint pivotPoint { req.pivotX * rect.w, req.pivotY * rect.h };

  // Set the x and y to be at the pivot point, along with scale it using the window size center
  rect.x = static_cast<int>((finalRenderingPos.x * pixelsPerMeter + windowSize.x * 0.5f / zoomFactor) - pivotPoint.x);
  rect.y = static_cast<int>((finalRenderingPos.y * pixelsPerMeter + windowSize.y * 0.5f / zoomFactor) - pivotPoint.y);
  glm::vec2 minPoints = glm::vec2(rect.x, rect.y);
  glm::vec2 maxPoints = glm::vec2(rect.x + rect.w, rect.y + rect.h);

  if (req.rotationDegrees != 0) {
    // Rotate each point using rotation matrix, currently have our pivot set to the top left corner, use rotation matrices :)
    // Need to rotate clockwise because of how SDL works, so use clockwise [cos sin; -sin cos] rotation matrix
    float angleRads = glm::radians(static_cast<float>(req.rotationDegrees));
    glm::mat2 rotationMatrix = glm::mat2(glm::cos(angleRads), -glm::sin(angleRads), glm::sin(angleRads), glm::cos(angleRads));
    // Move our pivot point to actually be in screen space instead of just relative to our top left
    glm::vec2 pivotPos = glm::vec2(rect.x + pivotPoint.x, rect.y + pivotPoint.y);
    // All of these variables names used for where the points are BEFORE rotation, need to check min and max values after
    // Also need to shift points back to where they should be using the pivot point
    glm::vec2 topLeft = rotationMatrix * (glm::vec2(rect.x, rect.y) - pivotPos) + pivotPos;
    glm::vec2 topRight = rotationMatrix * (glm::vec2(rect.x + rect.w, rect.y) - pivotPos) + pivotPos;
    glm::vec2 bottomLeft = rotationMatrix * (glm::vec2(rect.x, rect.y + rect.h) - pivotPos) + pivotPos;
    glm::vec2 bottomRight = rotationMatrix * (glm::vec2(rect.x + rect.w, rect.y + rect.h) - pivotPos) + pivotPos;
    minPoints = glm::min(glm::min(topLeft, topRight), glm::min(bottomLeft, bottomRight));
    maxPoints = glm::max(glm::max(topLeft, topRight), glm::max(bottomLeft, bottomRight));
  }

  const float padding = 2.0f;
  if (minPoints.x < screenRight + padding && maxPoints.x > screenLeft - padding && minPoints.y < screenBottom + padding && maxPoints.y > screenTop - padding) {
    return true;
  }
  return false;
}

bool Renderer::IsParticleOnScreen(glm::vec2 pos, float scale, float width, float height) {
  float screenLeft = 0.0f;
  float screenRight = windowSize.x / zoomFactor;
  float screenTop = 0.0f;
  float screenBottom = windowSize.y / zoomFactor;

  glm::vec2 finalRenderingPos = pos - cameraPos;

  float scaledWidth = width * scale;
  float scaledHeight = height * scale;

  SDL_FPoint pivotPoint {0.5f * scaledWidth, 0.5f * scaledHeight};

  float x = static_cast<int>((finalRenderingPos.x * pixelsPerMeter + windowSize.x * 0.5f / zoomFactor) - pivotPoint.x);
  float y = static_cast<int>((finalRenderingPos.y * pixelsPerMeter + windowSize.y * 0.5f / zoomFactor) - pivotPoint.y);

  const float padding = 2.0f;
  if (x < screenRight + padding && x + scaledWidth > screenLeft - padding && y < screenBottom + padding && y + scaledHeight > screenTop - padding) {
    return true;
  }
  return false;
}