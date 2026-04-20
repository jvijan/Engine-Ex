#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <string>
#include <vector>
#include <deque>
#include "Actor.h"
#include "Helper.h"
#include "SDL2/SDL.h"
#include "glm/glm.hpp"

class Actor;

struct ParticleSystem {
  // Particle System data
  std::vector<glm::vec2> particlePos;
  std::vector<glm::vec2> velocities;
  std::vector<float> scales;
  std::vector<float> initialScales;
  std::vector<float> rotations;
  std::vector<float> rotationSpeeds;
  std::vector<int> startFrames;
  std::vector<bool> isActive;
  std::deque<int> freeList;
  std::string textureName = "";
  float emitAngleMin = 0.0f;
  float emitAngleMax = 360.0f;
  float emitRadiusMin = 0.0f;
  float emitRadiusMax = 0.5f;
  RandomEngine emitAngleDistribution;
  RandomEngine emitRadiusDistribution;
  RandomEngine rotationDistribution;
  RandomEngine scaleDistribution;
  RandomEngine speedDistribution;
  RandomEngine rotationSpeedDistribution;
  float startScaleMin = 1.0f;
  float startScaleMax = 1.0f;
  float rotationMin = 0.0f;
  float rotationMax = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float startSpeedMin = 0.0f;
  float startSpeedMax = 0.0f;
  float rotationSpeedMin = 0.0f;
  float rotationSpeedMax = 0.0f;
  float gravityScaleX = 0.0f;
  float gravityScaleY = 0.0f;
  float dragFactor = 1.0f;
  float angularDragFactor = 1.0f;
  float endScale = -1.0f; // Change later if scale can be negative
  int sortingOrder = 9999;
  int r = 255;
  int g = 255;
  int b = 255;
  int a = 255;
  int endR = -1; // Change later if color can be negative
  int endG = -1; // Change later ^
  int endB = -1; // Change later ^
  int endA = -1; // Change later ^
  int localFrameNumber = 0;
  int framesBetweenBursts = 1;
  int burstQuantity = 1;
  int durationFrames = 300;
  bool paused = false;

  // Lua Ref stuff
  std::string type = "ParticleSystem";
  std::string key = "???";
  Actor* actor = nullptr;
  bool enabled = true;

  std::vector<glm::vec2> reqPos;
  std::vector<float> reqRotations;
  std::vector<float> reqScales;
  std::vector<SDL_Color> reqColors;

  void OnStart();
  void OnUpdate();
  void GenerateNewParticles();
  void UpdateParticles();
  void Stop();
  void Play();
};

#endif