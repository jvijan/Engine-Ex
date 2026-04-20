#ifndef SCENEDB_H
#define SCENEDB_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Actor.h"
#include "ActorTemplateDB.h"

class SceneDB {
private:
  inline static int currActorId = 0;
  // Map of scene name to all actors in the scene (blueprint copies, value semantics is correct here)
  inline static std::unordered_map<std::string, std::vector<Actor>> originalScenes;
  inline static std::string currSceneName;

public:

  inline static std::vector<std::unique_ptr<Actor>> currentScene;
  inline static std::vector<std::unique_ptr<Actor>> actorsToAdd;
  inline static std::vector<std::unique_ptr<Actor>> instantiatedActors;
  inline static std::vector<Actor*> actorsToDestroy;
  inline static std::string nextScene;

  static void LoadAllScenes();

  static void SwitchToScene(const std::string &sceneName);

  static void LoadScene(const std::string &sceneName);

  static Actor* Find(const std::string &name);

  static luabridge::LuaRef FindAll(const std::string &name);

  static Actor* Instantiate(const std::string &templateName);

  static void Destroy(Actor* actor);

  static std::string GetCurrentScene();

  static void DontDestroy(Actor* actor);

};

#endif
