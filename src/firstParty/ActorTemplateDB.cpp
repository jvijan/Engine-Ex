#include "ActorTemplateDB.h"
#include "Actor.h"
#include "EngineUtils.h"
#include "rapidjson/document.h"
#include <filesystem>

void ActorTemplateDB::LoadTemplates() {
  // Don't do anything if the actor_templates doesn't exist so we don't crash
  if (!std::filesystem::exists("resources/actor_templates")) {
    return;
  }
  // Add all templates to a map after overriding default actor constructor items
  for (const auto &file : std::filesystem::directory_iterator("resources/actor_templates")) {
    std::string templateName = file.path().stem().string();
    rapidjson::Document templateFile;
    EngineUtils::ReadJsonFile(file.path().string(), templateFile);
    Actor currActor;
    EngineUtils::ReadInActor(templateFile, currActor);
    actorTemplates[templateName] = currActor;
  }

}

Actor ActorTemplateDB::GetActor(const std::string &name) {
  auto it = actorTemplates.find(name);
  if (it == actorTemplates.end()) {
    std::cout << "error: template " << name << " is missing";
    std::exit(0);
  }
  return it->second;
}
