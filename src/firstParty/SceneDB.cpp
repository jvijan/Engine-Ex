#include <filesystem>
#include <cstdlib>
#include <memory>
#include <vector>
#include "Actor.h"
#include "ActorTemplateDB.h"
#include "Rigidbody.h"
#include "EngineUtils.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "SceneDB.h"



void SceneDB::LoadAllScenes() {
  std::string sceneDir = "resources/scenes";

  // Iterate over all files in the scenes directory and load them into memory
  for (const auto& file : std::filesystem::directory_iterator(sceneDir)) {
    std::string sceneName = file.path().stem().string();
    rapidjson::Document scene;
    EngineUtils::ReadJsonFile(file.path().string(), scene);
    if (scene.HasMember("actors") && scene["actors"].IsArray()) {
      const rapidjson::Value& actors = scene["actors"];
      std::vector<Actor> currActors;
      // Iterate over all actors in the scene and instantiate them, and push them back into an array to put into map
      for(rapidjson::SizeType i = 0; i < actors.Size(); i++) {
        const rapidjson::Value& copyActor = actors[i];
        Actor newActor;
        // Check if the actor has a template and set default variables to that if it exists
        if (copyActor.HasMember("template")) {
          std::string templateName = copyActor["template"].GetString();

          newActor = ActorTemplateDB::GetActor(templateName);
        }
        // Look for any variables in actor and update them if they're there, otherwise just keep them default
        EngineUtils::ReadInActor(copyActor, newActor);
        currActors.push_back(newActor);
      } // End for loop over actors
      originalScenes[sceneName] = currActors;
    } // End if statement to check for actor array
  } // End for loop over all scenes
}

void SceneDB::SwitchToScene(const std::string &sceneName) {
  // If the scene doesn't exist, print out the error
  auto it = originalScenes.find(sceneName);
  if (it == originalScenes.end()) {
    std::cout << "error: scene " << sceneName << " is missing";
    exit(0);
  }
  std::vector<Actor>& sceneActors = it->second;

  // Keep actors preserved from previous scene if they're not supposed to be destroyed
  std::vector<std::unique_ptr<Actor>> dontDestroyActors;
  for (auto& actor : currentScene) {
    if (actor->dontDestroy && !actor->destroyed) {
      dontDestroyActors.push_back(std::move(actor));
    }
  }
  for (auto& actor : actorsToAdd) {
    if (actor->dontDestroy && !actor->destroyed) {
      dontDestroyActors.push_back(std::move(actor));
    }
  }
  for (auto& actor : instantiatedActors) {
    if (actor->dontDestroy && !actor->destroyed) {
      dontDestroyActors.push_back(std::move(actor));
    }
  }

  // Clear current scene
  currentScene.clear();
  actorsToAdd.clear();
  actorsToDestroy.clear();
  instantiatedActors.clear();

  for (Actor &actor : sceneActors) {
    auto newActor = std::make_unique<Actor>(actor);
    for (auto &comp : newActor->components) {
      if ((*comp.second)["type"] == std::string("Rigidbody")) {
        Rigidbody* orig = comp.second->cast<Rigidbody*>();
        Rigidbody* rb = new Rigidbody(*orig);
        comp.second = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), rb));
      }
      else if ((*comp.second)["type"] == std::string("ParticleSystem")) {
        ParticleSystem* orig = comp.second->cast<ParticleSystem*>();
        ParticleSystem* ps = new ParticleSystem(*orig);
        comp.second = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), ps));
      }
      else {
        std::shared_ptr<luabridge::LuaRef> newComponent = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::GetLuaState()));
        luabridge::LuaRef parentTable = *comp.second;
        ComponentDB::EstablishInheritance(*newComponent, parentTable);
        comp.second = newComponent;
      }
    }
    newActor->id = currActorId++;
    actorsToAdd.push_back(std::move(newActor));
  }

  // Put dont destroy actors back into scene (no re-injection needed — Actor address is stable on the heap)
  for (auto& actor : dontDestroyActors) {
    currentScene.push_back(std::move(actor));
  }

  currSceneName = sceneName;
}

Actor* SceneDB::Find(const std::string &name) {
  for (auto& actor : actorsToAdd) {
    if (actor && actor->name == name && !actor->destroyed) {
      return actor.get();
    }
  }
  for (auto& actor : currentScene) {
    if (actor->name == name && !actor->destroyed) {
      return actor.get();
    }
  }
  for (auto& actor : instantiatedActors) {
    if (actor->name == name && !actor->destroyed) {
      return actor.get();
    }
  }
  return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef SceneDB::FindAll(const std::string &name) {
  luabridge::LuaRef newTable = luabridge::newTable(ComponentDB::GetLuaState());
  int i = 1;
  for (auto& actor : actorsToAdd) {
    if (actor && actor->name == name && !actor->destroyed) {
      newTable[i] = actor.get();
      i++;
    }
  }
  for (auto& actor : currentScene) {
    if (actor->name == name && !actor->destroyed) {
      newTable[i] = actor.get();
      i++;
    }
  }
  for (auto& actor : instantiatedActors) {
    if (actor->name == name && !actor->destroyed) {
      newTable[i] = actor.get();
      i++;
    }
  }
  if (i > 1) {
    return newTable;
  }
  return luabridge::LuaRef(ComponentDB::GetLuaState());
}

Actor* SceneDB::Instantiate(const std::string &templateName) {
  auto newActor = std::make_unique<Actor>(ActorTemplateDB::GetActor(templateName));
  newActor->id = currActorId++;
  for (auto& comp : newActor->components) {
    std::shared_ptr<luabridge::LuaRef> newComponent;
    if ((*comp.second)["type"] == std::string("Rigidbody")) {
      Rigidbody* orig = comp.second->cast<Rigidbody*>();
      Rigidbody* rb = new Rigidbody(*orig);
      newComponent = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), rb));
    }
    else if ((*comp.second)["type"] == std::string("ParticleSystem")) {
      ParticleSystem* orig = comp.second->cast<ParticleSystem*>();
      ParticleSystem* ps = new ParticleSystem(*orig);
      newComponent = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), ps));
    } 
    else {
      newComponent = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::GetLuaState()));
      luabridge::LuaRef parentTable = *comp.second;
      ComponentDB::EstablishInheritance(*newComponent, parentTable);
    }
    (*newComponent)["key"] = comp.first;
    (*newComponent)["enabled"] = true;
    comp.second = newComponent;
  }
  Actor* act = newActor.get();
  instantiatedActors.push_back(std::move(newActor));
  return act;
}

void SceneDB::Destroy(Actor *actor) {
  actor->destroyed = true;
  for (auto &comp : actor->components) {
    (*comp.second)["enabled"] = false;
  }
  actorsToDestroy.push_back(actor);
}

std::string SceneDB::GetCurrentScene() {
  return currSceneName;
}

void SceneDB::DontDestroy(Actor *actor) {
  actor->dontDestroy = true;
}

void SceneDB::LoadScene(const std::string &sceneName) {
  nextScene = sceneName;
}
