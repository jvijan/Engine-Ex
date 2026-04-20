#include "ComponentDB.h"
#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "box2d/box2d.h"
#include "Actor.h"
#include "SceneDB.h"
#include "Rigidbody.h"
#include "Input.h"
#include "Helper.h"
#include "Renderer.h"
#include "AudioDB.h"
#include "ParticleSystem.h"
#include "EventBus.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#ifdef _WIN32
  #define URL_OPEN "start"
#elif __APPLE
  #define URL_OPEN "open"
#else
  #define URL_OPEN "xdg-open"
#endif


// Establish inheritance method from spec
void ComponentDB::EstablishInheritance(luabridge::LuaRef &instanceTable, luabridge::LuaRef &parentTable) {
  luabridge::LuaRef newMetatable = luabridge::newTable(luaState);
  newMetatable["__index"] = parentTable;

  instanceTable.push(luaState);
  newMetatable.push(luaState);
  lua_setmetatable(luaState, -2);
  lua_pop(luaState, 1);
}

lua_State* ComponentDB::GetLuaState() {
  return luaState;
}

// Initialize luaState and add Debug.Log as a global function
void ComponentDB::Init() {
  InitializeState();
  InitializeFunctions();
  InitializeComponents();
}

void ComponentDB::InitializeState() {
  luaState = luaL_newstate();
  luaL_openlibs(luaState);
  luaL_dostring(luaState, "jit.opt.start(3)");
  //luaL_dostring(luaState, "require('jit.v').on()");
}

void ComponentDB::InitializeFunctions() {
  // Add debug.log functionality
  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Debug")
      .addFunction("Log", ComponentDB::CppLog)
      .addFunction("LogError", ComponentDB::CppLogError)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginClass<Actor>("Actor")
      .addFunction("GetName", &Actor::GetName)
      .addFunction("GetID", &Actor::GetId)
      .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
      .addFunction("GetComponent", &Actor::GetComponent)
      .addFunction("GetComponents", &Actor::GetComponents)
      .addFunction("AddComponent", &Actor::AddComponent)
      .addFunction("RemoveComponent", &Actor::RemoveComponent)
    .endClass();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Actor")
      .addFunction("Find", SceneDB::Find)
      .addFunction("FindAll", SceneDB::FindAll)
      .addFunction("Instantiate", SceneDB::Instantiate)
      .addFunction("Destroy", SceneDB::Destroy)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Application")
      .addFunction("Quit", ExitHelper)
      .addFunction("Sleep", SleepHelper)
      .addFunction("GetFrame", Helper::GetFrameNumber)
      .addFunction("OpenURL", OpenUrl)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Input")
      .addFunction("GetKey", Input::GetKeyLua)
      .addFunction("GetKeyDown", Input::GetKeyDownLua)
      .addFunction("GetKeyUp", Input::GetKeyUpLua)
      .addFunction("GetMousePosition", Input::GetMousePos)
      .addFunction("GetMouseButton", Input::GetMouseButton)
      .addFunction("GetMouseButtonDown", Input::GetMouseButtonDown)
      .addFunction("GetMouseButtonUp", Input::GetMouseButtonUp)
      .addFunction("GetMouseScrollDelta", Input::GetMouseScrollDelta)
      .addFunction("HideCursor", Input::HideCursor)
      .addFunction("ShowCursor", Input::ShowCursor)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginClass<glm::vec2>("vec2")
      .addProperty("x", &glm::vec2::x)
      .addProperty("y", &glm::vec2::y)
    .endClass();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Text")
      .addFunction("Draw", Renderer::AddTextRequest)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Audio")
      .addFunction("Play", AudioDB::PlayAudio)
      .addFunction("Halt", AudioDB::HaltChannel)
      .addFunction("SetVolume", AudioDB::SetVolume)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Image")
      .addFunction("DrawUI", Renderer::DrawUI)
      .addFunction("DrawUIEx", Renderer::DrawUIEx)
      .addFunction("Draw", Renderer::Draw)
      .addFunction("DrawEx", Renderer::DrawEx)
      .addFunction("DrawPixel", Renderer::DrawPixel)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Camera")
      .addFunction("SetPosition", Renderer::SetCameraPos)
      .addFunction("GetPositionX", Renderer::GetCameraPosX)
      .addFunction("GetPositionY", Renderer::GetCameraPosY)
      .addFunction("SetZoom", Renderer::SetZoom)
      .addFunction("GetZoom", Renderer::GetZoom)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginNamespace("Scene")
      .addFunction("Load", SceneDB::LoadScene)
      .addFunction("GetCurrent", SceneDB::GetCurrentScene)
      .addFunction("DontDestroy", SceneDB::DontDestroy)
    .endNamespace();

  luabridge::getGlobalNamespace(luaState)
    .beginClass<b2Vec2>("Vector2")
      .addConstructor<void(*) (float, float)>()
      .addProperty("x", &b2Vec2::x)
      .addProperty("y", &b2Vec2::y)
      .addFunction("Normalize", &b2Vec2::Normalize)
      .addFunction("Length", &b2Vec2::Length)
      .addFunction("__add", &b2Vec2::operator_add)
      .addFunction("__sub", &b2Vec2::operator_sub)
      .addFunction("__mul", &b2Vec2::operator_mul)
      .addStaticFunction("Distance", &b2Distance)
      .addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
    .endClass();

    luabridge::getGlobalNamespace(luaState)
      .beginClass<Rigidbody>("Rigidbody")
        .addProperty("enabled", &Rigidbody::enabled)
        .addProperty("key", &Rigidbody::key)
        .addProperty("type", &Rigidbody::type)
        .addProperty("actor", &Rigidbody::actor)
        .addProperty("x", &Rigidbody::x)
        .addProperty("y", &Rigidbody::y)
        .addProperty("body_type", &Rigidbody::bodyType)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravityScale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angularFriction)
        .addProperty("rotation", &Rigidbody::rotationDegrees)
        .addProperty("has_collider", &Rigidbody::hasCollider)
        .addProperty("has_trigger", &Rigidbody::hasTrigger)
        .addProperty("collider_type", &Rigidbody::colliderType)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addProperty("trigger_type", &Rigidbody::triggerType)
        .addProperty("trigger_width", &Rigidbody::triggerWidth)
        .addProperty("trigger_height", &Rigidbody::triggerHeight)
        .addProperty("trigger_radius", &Rigidbody::triggerRadius)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
      .endClass();

      luabridge::getGlobalNamespace(luaState)
        .beginClass<Collision>("Collision")
          .addProperty("other", &Collision::other)
          .addProperty("point", &Collision::point)
          .addProperty("relative_velocity", &Collision::relativeVelocity)
          .addProperty("normal", &Collision::normal)
        .endClass();

      luabridge::getGlobalNamespace(luaState)
        .beginClass<HitResult>("HitResult")
          .addProperty("actor", &HitResult::actor)
          .addProperty("point", &HitResult::point)
          .addProperty("normal", &HitResult::normal)
          .addProperty("is_trigger", &HitResult::isTrigger)
        .endClass();

      luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Physics")
          .addFunction("Raycast", &Rigidbody::Raycast)
          .addFunction("RaycastAll", &Rigidbody::RaycastAll)
        .endNamespace();

      luabridge::getGlobalNamespace(luaState)
        .beginNamespace("Event")
          .addFunction("Publish", EventBus::Publish)
          .addFunction("Subscribe", EventBus::Subscribe)
          .addFunction("Unsubscribe", EventBus::Unsubscribe)
        .endNamespace();

      luabridge::getGlobalNamespace(luaState)
        .beginClass<ParticleSystem>("ParticleSystem")
          .addProperty("enabled", &ParticleSystem::enabled)
          .addProperty("key", &ParticleSystem::key)
          .addProperty("type", &ParticleSystem::type)
          .addProperty("actor", &ParticleSystem::actor)
          .addProperty("x", &ParticleSystem::x)
          .addProperty("y", &ParticleSystem::y)
          .addProperty("frames_between_bursts", &ParticleSystem::framesBetweenBursts)
          .addProperty("burst_quantity", &ParticleSystem::burstQuantity)
          .addProperty("start_scale_min", &ParticleSystem::startScaleMin)
          .addProperty("start_scale_max", &ParticleSystem::startScaleMax)
          .addProperty("rotation_min", &ParticleSystem::rotationMin)
          .addProperty("rotation_max", &ParticleSystem::rotationMax)
          .addProperty("start_color_r", &ParticleSystem::r)
          .addProperty("start_color_g", &ParticleSystem::g)
          .addProperty("start_color_b", &ParticleSystem::b)
          .addProperty("start_color_a", &ParticleSystem::a)
          .addProperty("emit_radius_min", &ParticleSystem::emitRadiusMin)
          .addProperty("emit_radius_max", &ParticleSystem::emitRadiusMax)
          .addProperty("emit_angle_min", &ParticleSystem::emitAngleMin)
          .addProperty("emit_angle_max", &ParticleSystem::emitAngleMax)
          .addProperty("image", &ParticleSystem::textureName)
          .addProperty("sorting_order", &ParticleSystem::sortingOrder)
          .addProperty("duration_frames", &ParticleSystem::durationFrames)
          .addProperty("start_speed_min", &ParticleSystem::startSpeedMin)
          .addProperty("start_speed_max", &ParticleSystem::startSpeedMax)
          .addProperty("rotation_speed_min", &ParticleSystem::rotationSpeedMin)
          .addProperty("rotation_speed_max", &ParticleSystem::rotationSpeedMax)
          .addProperty("gravity_scale_x", &ParticleSystem::gravityScaleX)
          .addProperty("gravity_scale_y", &ParticleSystem::gravityScaleY)
          .addProperty("drag_factor", &ParticleSystem::dragFactor)
          .addProperty("angular_drag_factor", &ParticleSystem::angularDragFactor)
          .addProperty("end_scale", &ParticleSystem::endScale)
          .addProperty("end_color_r", &ParticleSystem::endR)
          .addProperty("end_color_g", &ParticleSystem::endG)
          .addProperty("end_color_b", &ParticleSystem::endB)
          .addProperty("end_color_a", &ParticleSystem::endA)
          .addFunction("OnStart", &ParticleSystem::OnStart)
          .addFunction("OnUpdate", &ParticleSystem::OnUpdate)
          .addFunction("Stop", &ParticleSystem::Stop)
          .addFunction("Play", &ParticleSystem::Play)
          .addFunction("Burst", &ParticleSystem::GenerateNewParticles)
        .endClass();
}

// Function to add component to our compoment type cache
void ComponentDB::InitializeComponents() {
  std::string path = "resources/component_types";

  if (!std::filesystem::exists(path)) {
    return;
  }

  for (const auto& file : std::filesystem::directory_iterator(path)) {
    if (luaL_dofile(luaState, file.path().string().c_str()) != LUA_OK) {
      std::cout << "problem with lua file " + file.path().stem().string();
      exit(0);
    }
    std::string componentType = file.path().stem().string();
    componentCache.insert({ componentType, std::make_shared<luabridge::LuaRef>(luabridge::getGlobal(luaState, componentType.c_str())) });
  }
}

// Create a new instance of a component
std::shared_ptr<luabridge::LuaRef> ComponentDB::CreateComponent(const std::string &componentType, const std::string &key) {
  auto it = componentCache.find(componentType);
  if (it == componentCache.end()) {
    std::cout << "error: failed to locate component " + componentType;
    exit(0);
  }
  // Create a new instance using the old parent table
  luabridge::LuaRef newInstance = luabridge::newTable(luaState);
  luabridge::LuaRef parentTable = luabridge::getGlobal(luaState, componentType.c_str());

  EstablishInheritance(newInstance, parentTable);

  newInstance["key"] = key;
  newInstance["enabled"] = true;

  return std::make_shared<luabridge::LuaRef>(newInstance);
}

void ComponentDB::CppLog(const std::string &msg) {
  std::cout << msg << "\n";
}

void ComponentDB::CppLogError(const std::string &msg) {
  std::cout << msg << "\n";
}

void ComponentDB::OpenUrl(const std::string &url) {
  std::string cmd = std::string(URL_OPEN) + " " + url;
  std::system(cmd.c_str());
}

void ComponentDB::ExitHelper() {
  exit(0);
}

void ComponentDB::SleepHelper(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
