#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>

class EventBus {
public:
    inline static std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> subscribers;
    inline static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> subscriptionsToAdd;
    inline static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> unsubscriptionsToProcess;

    static void Publish(const std::string& eventType, luabridge::LuaRef eventObject);
    static void Subscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function);
    static void ProcessPending();
};

#endif
