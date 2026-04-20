#ifndef ACTOR_H
#define ACTOR_H
#include "lua.hpp"
#include "ContactListener.h"
#include "LuaBridge/LuaBridge.h"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

class Collision;

class Actor
{

public:
  int id;
	std::string name;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> components;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnStart;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnUpdate;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnLateUpdate;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnCollisionEnter;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnCollisionExit;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnTriggerEnter;
	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> componentsWithOnTriggerExit;

	std::map<std::string, std::shared_ptr<luabridge::LuaRef>> justAddedComponents;
	std::vector<std::string> componentsToRemove;

	bool destroyed = false;
	bool dontDestroy = false;

	void Start();
	void Update();
	void LateUpdate();
	void OnDestroy();
	void OnCollisionEnter(const Collision &collision);
	void OnCollisionExit(const Collision &collsion);
	void OnTriggerEnter(const Collision &collision);
	void OnTriggerExit(const Collision &collsion);

	void InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> componentRef);

	std::string GetName();
	int GetId();
	luabridge::LuaRef GetComponentByKey(const std::string &key);
	luabridge::LuaRef GetComponent(const std::string &type);
	luabridge::LuaRef GetComponents(const std::string &type);

	void ProcessAddedComponents();
	luabridge::LuaRef AddComponent(const std::string &type);
	void ProcessRemovedComponents();
	void RemoveComponent(luabridge::LuaRef ref);

	Actor(): id(0), name(""){}
};

#endif
