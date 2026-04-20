#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <cctype>
#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "ComponentDB.h"
#include "Helper.h"
#include "ImageDB.h"
#include "Rigidbody.h"
#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "Renderer.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "Actor.h"
#include "ParticleSystem.h"

class EngineUtils {
public:
  static void ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
  {
  	FILE* file_pointer = nullptr;
#ifdef _WIN32
  	fopen_s(&file_pointer, path.c_str(), "rb");
#else
  	file_pointer = fopen(path.c_str(), "rb");
#endif
  	char buffer[65536];
  	rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
  	out_document.ParseStream(stream);
  	std::fclose(file_pointer);

  	if (out_document.HasParseError()) {
  		rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
  		std::cout << "error parsing json at [" << path << "]" << std::endl;
  		exit(0);
  	}
  }

  static void InsertionSortActors(std::vector<const Actor*>& actors, const Actor* nextActor){
    auto insertPos = actors.begin();
    while(insertPos != actors.end() && (*insertPos)->id < nextActor->id) {
      insertPos++;
    }
    actors.insert(insertPos, nextActor);
  }

  // Helper function to read in actors from json
  static void ReadInActor(const rapidjson::Value &actor, Actor &newActor) {
    if (actor.HasMember("name")) {
      newActor.name = actor["name"].GetString();
    }
    if (actor.HasMember("components")) {
      const rapidjson::Value& components = actor["components"];
      for (auto it = components.MemberBegin(); it != components.MemberEnd(); it++) {
        std::string componentKey = it->name.GetString();
        const rapidjson::Value& component = it->value;
        std::string componentType;
        if (component.HasMember("type")) {
          componentType = component["type"].GetString();
        }
        else {
          componentType = componentKey;
        }
        if (componentType == "Rigidbody") {
          Rigidbody* rb = new Rigidbody();
          luabridge::LuaRef rbRef(ComponentDB::GetLuaState(), rb);
          rbRef["key"] = componentKey;
          rbRef["enabled"] = true;
          for (auto compIt = component.MemberBegin(); compIt != component.MemberEnd(); compIt++) {
            std::string varKey = compIt->name.GetString();
            if (compIt->value.IsString()) {
              rbRef[varKey] = compIt->value.GetString();
            }
            else if (compIt->value.IsInt()) {
              rbRef[varKey] = compIt->value.GetInt();
            }
            else if (compIt->value.IsDouble()) {
              rbRef[varKey] = compIt->value.GetDouble();
            }
            else if (compIt->value.IsBool()) {
              rbRef[varKey] = compIt->value.GetBool();
            }
          }
          newActor.components[componentKey] = std::make_shared<luabridge::LuaRef>(rbRef);
          continue;
        }

        else if (componentType == "ParticleSystem") {
          ParticleSystem* ps = new ParticleSystem();
          luabridge::LuaRef psRef(ComponentDB::GetLuaState(), ps);
          psRef["key"] = componentKey;
          psRef["enabled"] = true;
          for (auto compIt = component.MemberBegin(); compIt != component.MemberEnd(); compIt++) {
            std::string varKey = compIt->name.GetString();
            if (compIt->value.IsString()) {
              psRef[varKey] = compIt->value.GetString();
            }
            else if (compIt->value.IsInt()) {
              psRef[varKey] = compIt->value.GetInt();
            }
            else if (compIt->value.IsDouble()) {
              psRef[varKey] = compIt->value.GetDouble();
            }
            else if (compIt->value.IsBool()) {
              psRef[varKey] = compIt->value.GetBool();
            }
          }
          newActor.components[componentKey] = std::make_shared<luabridge::LuaRef>(psRef);
          continue;
        }

        // If the component exists already get it and override it, otherwise create a new one so we don't cause errors when
        // accessing JSON
        std::shared_ptr<luabridge::LuaRef> newComponent;
        auto compExists = newActor.components.find(componentKey);
        if (compExists != newActor.components.end()) {
          // Need to create a new table so we don't just create a pointer copy of the template

          newComponent = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::GetLuaState()));
          luabridge::LuaRef parentTable = *compExists->second;
          ComponentDB::EstablishInheritance(*newComponent, parentTable);
          // (*newComponent)["key"] = componentKey;
          // (*newComponent)["enabled"] = true;
        }
        else {
          newComponent = ComponentDB::CreateComponent(componentType, componentKey);
        }
        // Iterate over component data and override values within components
        for (auto compIt = component.MemberBegin(); compIt != component.MemberEnd(); compIt++) {
          std::string varKey = compIt->name.GetString();
          // Set the variable in our component correctly based on the type
          if (compIt->value.IsString()) {
            (*newComponent)[varKey] = compIt->value.GetString();
          }
          else if (compIt->value.IsInt()) {
            (*newComponent)[varKey] = compIt->value.GetInt();
          }
          else if (compIt->value.IsDouble()) {
            (*newComponent)[varKey] = compIt->value.GetDouble();
          }
          else if (compIt->value.IsBool()) {
            (*newComponent)[varKey] = compIt->value.GetBool();
          }
        }
        newActor.components[componentKey] = newComponent;
      }
    }
  }

  // Helper function to get word after a certain phrase appears
  static std::string obtain_word_after_phrase(const std::string &input, const std::string &phrase) {
    // Find position of the phrase in the string
    size_t pos = input.find(phrase);

    // If phrase not found, return empty string
    if (pos == std::string::npos) {
      return "";
    }

    // Find starting pos of next word skipping whitespace
    pos += phrase.length();
    while (pos < input.size() && std::isspace(input[pos])) {
      pos++;
    }
    //  If at end of string, return empty string
    if (pos == input.size()) return "";

    // Find end position of the word
    size_t endPos = pos;
    while (endPos < input.size() && !std::isspace(input[endPos])) {
      ++endPos;
    }

    // Extract and return the word
    return input.substr(pos, endPos-pos);
  }

  static uint64_t PackVec(glm::ivec2 vec) {
    uint32_t ux = static_cast<uint32_t>(vec.x);
    uint32_t uy = static_cast<uint32_t>(vec.y);

    uint64_t packed = static_cast<uint64_t>(ux) << 32;
    packed = packed | static_cast<uint64_t>(uy);
    return packed;
  }

  static void ReportError(const std::string &actorName, const luabridge::LuaException &e) {
    std::string errorMsg = e.what();
    std::replace(errorMsg.begin(), errorMsg.end(), '\\', '/');

    std::cout << "\033[31m" << actorName << " : " << errorMsg << "\033[0m\n";
  }
};

#endif
