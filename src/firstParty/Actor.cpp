#include "Actor.h"
#include "ComponentDB.h"
#include "EngineUtils.h"
#include "Rigidbody.h"
#include "ParticleSystem.h"
#include <iostream>

void Actor::Start() {
  // Iterate over components, calling start on them
  for (auto &comp : componentsWithOnStart) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnStart"](ref);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::Update() {
  for (auto &comp : componentsWithOnUpdate) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnUpdate"](ref);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::LateUpdate() {
  for (auto &comp : componentsWithOnLateUpdate) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnLateUpdate"](ref);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::OnDestroy() {
  for (auto &comp : components) {
    luabridge::LuaRef &ref = *comp.second;
    ref["enabled"] = false;
    if (ref["OnDestroy"].isFunction()) {
      try {
        ref["OnDestroy"](ref);
      }
      catch (luabridge::LuaException e) {
        EngineUtils::ReportError(name, e);
      }
    }
  }
}

void Actor::OnCollisionEnter(const Collision &collision) {
  for (auto &comp : componentsWithOnCollisionEnter) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnCollisionEnter"](ref, collision);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::OnCollisionExit(const Collision &collision) {
  for (auto &comp : componentsWithOnCollisionExit) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnCollisionExit"](ref, collision);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::OnTriggerEnter(const Collision &collision) {
  for (auto &comp : componentsWithOnTriggerEnter) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnTriggerEnter"](ref, collision);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::OnTriggerExit(const Collision &collision) {
  for (auto &comp : componentsWithOnTriggerExit) {
    luabridge::LuaRef &ref = *comp.second;
    if (!ref["enabled"]) {
      continue;
    }
    try {
      ref["OnTriggerExit"](ref, collision);
    }
    catch (luabridge::LuaException e) {
      EngineUtils::ReportError(name, e);
    }
  }
}

void Actor::ProcessAddedComponents() {
  for (auto &comp : justAddedComponents) {
    InjectConvenienceReferences(comp.second);
    components[comp.first] = comp.second;
    if (componentsWithOnStart.find(comp.first) != componentsWithOnStart.end() && (*comp.second)["enabled"]) {
      try {
        (*comp.second)["OnStart"]((*comp.second));
      }
      catch (luabridge::LuaException e) {
        EngineUtils::ReportError(name, e);
      }
    }
  }
  justAddedComponents.clear();
}

void Actor::ProcessRemovedComponents() {
  for (auto &key : componentsToRemove) {
    auto it = components.find(key);
    if (it != components.end()) {
      luabridge::LuaRef &ref = *it->second;
      if (ref["OnDestroy"].isFunction()) {
        try {
          ref["OnDestroy"](ref);
        }
        catch (luabridge::LuaException e) {
          EngineUtils::ReportError(name, e);
        }
      }
      // Only need to erase if the component actually exists
      components.erase(it);
      componentsWithOnStart.erase(key);
      componentsWithOnUpdate.erase(key);
      componentsWithOnLateUpdate.erase(key);
    }
  }
}

luabridge::LuaRef Actor::AddComponent(const std::string &type) {
  std::string key = "r" + std::to_string(ComponentDB::runtimeComponentsCreated++);
  std::shared_ptr<luabridge::LuaRef> newComp;
  if (type == "Rigidbody") {
    Rigidbody* rb = new Rigidbody();
    rb->key = key;
    rb->enabled = true;
    newComp = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), rb));
  }
  else if (type == "ParticleSystem") {
    ParticleSystem* ps = new ParticleSystem();
    ps->key = key;
    ps->enabled = true;
    newComp = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef(ComponentDB::GetLuaState(), ps));
  } 
  else {
    newComp = ComponentDB::CreateComponent(type, key);
    (*newComp)["type"] = type;
  }
  justAddedComponents[key] = newComp;
  return *newComp;
}

void Actor::RemoveComponent(luabridge::LuaRef ref) {
  ref["enabled"] = false;
  componentsToRemove.push_back(ref["key"]);
}

void Actor::InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> componentRef) {
  (*componentRef)["actor"] = this;
  std::string key = (*componentRef)["key"];
  if ((*componentRef)["OnStart"].isFunction()) {
    componentsWithOnStart[key] = componentRef;
  }
  if ((*componentRef)["OnUpdate"].isFunction()) {
    componentsWithOnUpdate[key] = componentRef;
  }
  if ((*componentRef)["OnLateUpdate"].isFunction()) {
    componentsWithOnLateUpdate[key] = componentRef;
  }
  if ((*componentRef)["OnCollisionEnter"].isFunction()) {
    componentsWithOnCollisionEnter[key] = componentRef;
  }
  if ((*componentRef)["OnCollisionExit"].isFunction()) {
    componentsWithOnCollisionExit[key] = componentRef;
  }
  if ((*componentRef)["OnTriggerEnter"].isFunction()) {
    componentsWithOnTriggerEnter[key] = componentRef;
  }
  if ((*componentRef)["OnTriggerExit"].isFunction()) {
    componentsWithOnTriggerExit[key] = componentRef;
  }
}

std::string Actor::GetName() {
  return name;
}

int Actor::GetId() {
  return id;
}


luabridge::LuaRef Actor::GetComponentByKey(const std::string &key) {
  auto it = components.find(key);
  if (it != components.end()) {
    luabridge::LuaRef ref = *it->second;
    if (ref["enabled"]) {
     return *it->second;
    }
  }
  return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Actor::GetComponent(const std::string &type) {
  for (auto &comp : components) {
    luabridge::LuaRef ref = *comp.second;
    if (ref["type"] == type && ref["enabled"]) {
      return ref;
    }
  }
  return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Actor::GetComponents(const std::string &type) {
  luabridge::LuaRef newTable = luabridge::newTable(ComponentDB::GetLuaState());
  int i = 1;
  for (auto &comp : components) {
    luabridge::LuaRef ref = *comp.second;
    if (ref["type"] == type && ref["enabled"]) {
      newTable[i] = ref;
      i++;
    }
  }
  if (i > 1) {
    return newTable;
  }
  return luabridge::LuaRef(ComponentDB::GetLuaState());
}
