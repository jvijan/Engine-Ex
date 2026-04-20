#ifndef RENDERER_H
#define RENDERER_H

#include "Actor.h"
#include "Engine.h"
#include "SDL2/SDL.h"
#include "ImageDB.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "TextDB.h"
#include "glm/glm.hpp"
#include <deque>
#include <string>
#include <vector>

struct ParticleDrawRequest {
  int pushNumber = 0;
  int sortingOrder;
  std::string textureName;
  std::vector<glm::vec2> pos;
  std::vector<float> rotationDegrees;
  std::vector<float> scales;
  std::vector<SDL_Color> colors;
};


class Renderer {
private:
  inline static std::string gameTitle = "";
  inline static glm::ivec2 windowSize = glm::ivec2(640, 360);
  inline static glm::vec2 windowPos = glm::vec2(30, 30);
  inline static glm::vec2 cameraPos = glm::vec2(0, 0);
  inline static float zoomFactor = 1.0f;
  inline static int pushNumber = 0;

  inline static const int pixelsPerMeter = 100;

  inline static Uint8 clearColorR = 255;
  inline static Uint8 clearColorG = 255;
  inline static Uint8 clearColorB = 255;

  inline static SDL_Window* sdlWindow = nullptr;
  inline static SDL_Renderer* sdlRenderer = nullptr;

  inline static size_t introImageIndex = 0;
  inline static std::deque<ImageDrawRequest> introImages;
  inline static std::deque<std::string> introTexts;

  inline static std::deque<ImageDrawRequest> imageDrawRequestQueue;
  inline static std::deque<ParticleDrawRequest> particleDrawRequestQueue;
  inline static std::deque<TextDrawRequest> textDrawRequestQueue;
  inline static std::deque<UIDrawRequest> uiDrawRequestQueue;
  inline static std::deque<PixelDrawRequest> pixelDrawRequestQueue;

  static void LoadRenderingProperties();

  static bool IsOnScreen(ImageDrawRequest &req);

  static bool IsParticleOnScreen(glm::vec2 pos, float scale, float width, float height);

  static void DrawImageRequest(SDL_Texture* texture, float width, float height, float x, float y, float rotationDegrees,
                                float scaleX, float scaleY, float pivotX, float pivotY, SDL_Color color);

public:

  inline static bool frameRendered = false;

  static void InitializeRendering();

  static void RenderAndClearAllImages();
  static void RenderAndClearAllText();
  static void RenderAndClearAllUI();
  static void RenderAndClearAllPixels();

  static void ClearScreen();

  static void AddTextRequest(const std::string &text, const float x, const float y, const std::string fontName,
                                  const float fontSize, const float r, const float g, const float b, const float a);

  static void ClearAllQueues();

  static void RenderFrame();

  static void DrawUI(const std::string &imageName, const float x, const float y);
  static void DrawUIEx(const std::string &imageName, const float x, const float y, const float r, const float g,
                        const float b, const float a, const float sortingOrder);
  static void Draw(const std::string &imageName, const float x, const float y);
  static void DrawEx(const std::string &imageName, const float x, const float y, const float rotationDegrees,
                      const float scaleX, const float scaleY, const float pivotX, const float pivotY,
                      const float r, const float g, const float b, const float a, const float sortingOrder);
  static void DrawPixel(const float x, const float y, const float r, const float g, const float b, const float a);
  static void DrawParticleBatch(const int sortingOrder, const std::string &textureName, std::vector<glm::vec2> pos,
                                  std::vector<float> rotation, std::vector<float> scales, std::vector<SDL_Color> colors);

  static void SetCameraPos(const float x, const float y);
  static float GetCameraPosX();
  static float GetCameraPosY();
  static void SetZoom(const float newZoomFactor);
  static float GetZoom();

};

#endif
