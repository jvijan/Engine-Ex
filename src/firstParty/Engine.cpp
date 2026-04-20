#include "Engine.h"
#include "Actor.h"
#include "ActorTemplateDB.h"
#include "AudioDB.h"
#include "ComponentDB.h"
#include "EngineUtils.h"
#include "EventBus.h"
#include "Helper.h"
#include "ImageDB.h"
#include "Input.h"
#include "Renderer.h"
#include "SDL_scancode.h"
#include "SceneDB.h"
#include "TextDB.h"
#include "box2d/box2d.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

void Engine::GetInput() {
  SDL_Event e;
  while (Helper::SDL_PollEvent(&e)) {
    // Quit if needed
    if (e.type == SDL_QUIT) {
      gameRunning = false;
    }
    Input::ProcessEvent(e);
  }
  if (gameState != MAIN) {
    return;
  }
}

void Engine::Update() {
  // Inject references into the actors and call start for them
  for (auto& actor : SceneDB::actorsToAdd) {
    Actor* act = actor.get();
    for (auto &comp : act->components) {
      act->InjectConvenienceReferences(comp.second);
    }
    SceneDB::currentScene.push_back(std::move(actor));
    act->Start();
  }
  SceneDB::actorsToAdd.clear();

  for (auto& actor : SceneDB::currentScene) {
    actor->ProcessAddedComponents();
  }
  for (auto& actor : SceneDB::currentScene) {
    actor->Update();
  }
  for (auto& actor : SceneDB::currentScene) {
    actor->LateUpdate();
  }
  for (auto& actor : SceneDB::currentScene) {
    actor->ProcessRemovedComponents();
  }
  for (Actor* actor : SceneDB::actorsToDestroy) {
    actor->OnDestroy();
  }
  // Convert to a set so that we don't do O(n^2) loop
  std::unordered_set<Actor*> toDestroy(SceneDB::actorsToDestroy.begin(), SceneDB::actorsToDestroy.end());
  SceneDB::currentScene.erase(
    std::remove_if(SceneDB::currentScene.begin(), SceneDB::currentScene.end(),
      [&toDestroy](const std::unique_ptr<Actor>& a) {
        if (toDestroy.count(a.get()) > 0) {
          return true;
        }
        return false;
      }),
    SceneDB::currentScene.end()
  );
  SceneDB::actorsToDestroy.clear();

  // Get our new actorsToAdd for next frame from the newly instantiated actors
  SceneDB::actorsToAdd = std::move(SceneDB::instantiatedActors);
  SceneDB::instantiatedActors.clear();
  if (!SceneDB::nextScene.empty()) {
    SceneDB::SwitchToScene(SceneDB::nextScene);
    SceneDB::nextScene = "";
  }
}

// Public function to call, runs the game engine loop
void Engine::Run() {
  int frameNumber = 0;
  Input::Init();
  ComponentDB::Init();
  Renderer::InitializeRendering();

// Game loop
  while (gameRunning) {
    frameNumber = Helper::GetFrameNumber();
    newSceneSwitching = false;
    GetInput();
    Update();
    EventBus::ProcessPending();
    Rigidbody::PhysicsStep();
    Renderer::RenderFrame();
    // Don't forget to update input at end of frame
    Input::LateUpdate();
  }
}
