#ifndef COMPONENTDB_H
#define COMPONENTDB_H
#include <unordered_map>
#include <memory>
#include <string>
#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"


class ComponentDB {
private:

  inline static lua_State* luaState = nullptr;

  inline static std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> componentCache;

  static void InitializeState();

  static void InitializeFunctions();

  static void InitializeComponents();

  static void CppLog(const std::string &msg);

  static void CppLogError(const std::string &msg);

  static void OpenUrl(const std::string &url);

  static void ExitHelper();

  static void SleepHelper(int ms);

public:

  inline static int runtimeComponentsCreated = 0;

  static void Init();

  static lua_State* GetLuaState();

  static void EstablishInheritance(luabridge::LuaRef &instanceTable, luabridge::LuaRef &parentTable);

  static std::shared_ptr<luabridge::LuaRef> CreateComponent(const std::string &componentType, const std::string &key);

};


#endif
