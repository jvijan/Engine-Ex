#include "ParticleSystem.h"
#include "Renderer.h"
#include <algorithm>


void ParticleSystem::OnStart() {
  // Reserve vector space so we avoid reallocation
  particlePos.reserve(1000000);
  scales.reserve(1000000);
  rotations.reserve(1000000);
  startFrames.reserve(1000000);
  // freeList.reserve(1000000);
  isActive.reserve(1000000);
  velocities.reserve(1000000);

  // Create distributions and set seeds
  emitAngleDistribution = RandomEngine(emitAngleMin, emitAngleMax, 298);
  emitRadiusDistribution = RandomEngine(emitRadiusMin, emitRadiusMax, 404);
  rotationDistribution = RandomEngine(rotationMin, rotationMax, 440);
  scaleDistribution = RandomEngine(startScaleMin, startScaleMax, 494);
  speedDistribution = RandomEngine(startSpeedMin, startSpeedMax, 498);
  rotationSpeedDistribution = RandomEngine(rotationSpeedMin, rotationSpeedMax, 305);

  // Only create a default texture if empty, don't need otherwise
  if (textureName.empty()) {
    ImageDB::CreateDefaultParticleTextureWithName(textureName);
  }

  // Error checking for special cases, making sure to clamp specific values
  if (framesBetweenBursts < 1) framesBetweenBursts = 1;
  if (burstQuantity < 1) burstQuantity = 1;
  if (durationFrames < 1) durationFrames = 1;

  r = std::clamp(r, 0, 255);
  g = std::clamp(g, 0, 255);
  b = std::clamp(b, 0, 255);
  a = std::clamp(a, 0, 255);
  if (endR < 0) endR = r;
  if (endG < 0) endG = g;
  if (endB < 0) endB = b;
  if (endA < 0) endA = a;
}

void ParticleSystem::OnUpdate() {
  // Create new particles at beginning of OnUpdate
  if (localFrameNumber % framesBetweenBursts == 0 && !paused) {
    GenerateNewParticles();
  }
  // Draw particles at end of OnUpdate
  UpdateParticles();
  localFrameNumber++;
}

void ParticleSystem::GenerateNewParticles() {
  for (int i = 0; i < burstQuantity; i++) {
    // Position calculation
    float angleRadians = glm::radians(emitAngleDistribution.Sample());
    float radius = emitRadiusDistribution.Sample();
    float cosAngle = glm::cos(angleRadians);
    float sinAngle = glm::sin(angleRadians);
    float startingX = x + (cosAngle * radius);
    float startingY = y + (sinAngle * radius);

    float startScale = scaleDistribution.Sample();

    float startRotation = rotationDistribution.Sample();

    float speed = speedDistribution.Sample();
    float startingXVel = cosAngle * speed;
    float startingYVel = sinAngle * speed;

    float rSpeed = rotationSpeedDistribution.Sample();

    // Update vectors based on whether there's a free element or not
    if (!freeList.empty()) {
      size_t emptyIndex = freeList.front();
      freeList.pop_front();
      particlePos[emptyIndex] = glm::vec2(startingX, startingY);
      scales[emptyIndex] = startScale;
      initialScales[emptyIndex] = startScale;
      rotations[emptyIndex] = startRotation;
      startFrames[emptyIndex] = localFrameNumber;
      isActive[emptyIndex] = true;
      velocities[emptyIndex] = glm::vec2(startingXVel, startingYVel);
      rotationSpeeds[emptyIndex] = rSpeed;
    }
    else{
      particlePos.emplace_back(startingX, startingY);
      scales.push_back(startScale);
      initialScales.push_back(startScale);
      rotations.push_back(startRotation);
      startFrames.push_back(localFrameNumber);
      isActive.push_back(true);
      velocities.emplace_back(startingXVel, startingYVel);
      rotationSpeeds.push_back(rSpeed);
    }
  }
}

void ParticleSystem::UpdateParticles() {
  reqPos.clear();
  reqRotations.clear();
  reqScales.clear();
  reqColors.clear();
  for (size_t i = 0; i < particlePos.size(); i++) {
    // Check if particle is still active at this point, if not move to next
    if (!isActive[i]) {
      continue;
    }
    int framesAlive = localFrameNumber - startFrames[i];
    if (framesAlive >= durationFrames) {
      isActive[i] = false;
      freeList.push_back(i);
      continue;
    }

    // Use gravity to update velocity, TODO: If tiny performance improvements needed, make gravity a vec2 class variable and construct it in on start instead of every update
    velocities[i] += glm::vec2(gravityScaleX, gravityScaleY);

    // Use drag and angular drag to update velocity and angular velocity
    velocities[i].x *= dragFactor;
    velocities[i].y *= dragFactor;  
    rotationSpeeds[i] *= angularDragFactor;

    // Use particle velocities to update position and rotation
    particlePos[i] += velocities[i];
    rotations[i] += rotationSpeeds[i];

    // Handle color and scale changes
    float lifetimeProgress = static_cast<float>(framesAlive) / durationFrames;
    // Only modify scale if scale is non negative I think
    if (endScale >= 0) {
      scales[i] = glm::mix(initialScales[i], endScale, lifetimeProgress);
    }
    reqPos.push_back(particlePos[i]);
    reqRotations.push_back(rotations[i]);
    reqScales.push_back(scales[i]);
    reqColors.push_back({
      (Uint8)glm::mix(r, endR, lifetimeProgress),
      (Uint8)glm::mix(g, endG, lifetimeProgress),
      (Uint8)glm::mix(b, endB, lifetimeProgress),
      (Uint8)glm::mix(a, endA, lifetimeProgress)
    });
  }
  // Don't want to push empty requests to queue
  if (!reqPos.empty()) {
    Renderer::DrawParticleBatch(sortingOrder, textureName, std::move(reqPos), std::move(reqRotations), std::move(reqScales), std::move(reqColors));
  }
}

void ParticleSystem::Stop() {
  paused = true;
}

void ParticleSystem::Play() {
  paused = false;
}