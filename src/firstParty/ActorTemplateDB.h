#ifndef ACTORTEMPLATEDB_H
#define ACTORTEMPLATEDB_H

#include "Actor.h"
#include <string>
#include <unordered_map>
class ActorTemplateDB {
private:
  inline static std::unordered_map<std::string, Actor> actorTemplates;

public:

  // Load all actor templates into template map
  static void LoadTemplates();

  static Actor GetActor(const std::string &name);
};

#endif
