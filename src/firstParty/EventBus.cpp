#include "EventBus.h"
#include "ComponentDB.h"
#include "EngineUtils.h"
#include <algorithm>
#include <iostream>

void EventBus::Publish(const std::string& eventType, luabridge::LuaRef eventObject) {
    auto it = subscribers.find(eventType);
    if (it == subscribers.end()) {
        return;
    }
    for (auto& refs : it->second) {
        luabridge::LuaRef &ref = refs.first;
        luabridge::LuaRef &function = refs.second;
        try {
            function(ref, eventObject);
        }
        catch (luabridge::LuaException e) {
            EngineUtils::ReportError("EventBus Error", e);
        }
    }
}

void EventBus::Subscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function) {
    subscriptionsToAdd.emplace_back(eventType, component, function);
}

void EventBus::Unsubscribe(const std::string& eventType, luabridge::LuaRef component, luabridge::LuaRef function) {
    unsubscriptionsToProcess.emplace_back(eventType, component, function);
}

void EventBus::ProcessPending() {

    for (auto& unsub : unsubscriptionsToProcess) {
        auto it = subscribers.find(std::get<0>(unsub));
        if (it != subscribers.end()) {
            std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>& subs = it->second;
            subs.erase(
                std::remove_if(subs.begin(), subs.end(),
                    [&](const std::pair<luabridge::LuaRef, luabridge::LuaRef>& sub) {
                        return sub.first == std::get<1>(unsub) && sub.second == std::get<2>(unsub);
                    }),
                subs.end()
            );
        }
    }
    unsubscriptionsToProcess.clear();

    for (auto& sub : subscriptionsToAdd) {
        subscribers[std::get<0>(sub)].emplace_back(std::get<1>(sub), std::get<2>(sub));
    }
    subscriptionsToAdd.clear();
}
