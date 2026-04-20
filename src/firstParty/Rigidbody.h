#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#include "box2d/box2d.h"
#include "ContactListener.h"
#include "Actor.h"
#include "LuaBridge/LuaBridge.h"
#include <string>
#include <vector>

class Rigidbody {
private:

public:
  inline static b2World* world = nullptr;
  inline static bool worldInitialized = false;

  float x = 0.0f;
  float y = 0.0f;
  std::string bodyType = "dynamic";
  bool precise = true;
  float gravityScale = 1.0f;
  float density = 1.0f;
  float angularFriction = 0.3f;
  float rotationDegrees = 0.0f;
  bool hasCollider = true;
  bool hasTrigger = true;
  b2Body* body = nullptr;
  float width = 1.0f;
  float height = 1.0f;
  float radius = 0.5f;
  float friction = 0.3f;
  float bounciness = 0.3f;
  std::string colliderType = "box";
  std::string triggerType = "box";
  float triggerWidth = 1.0f;
  float triggerHeight = 1.0f;
  float triggerRadius = 0.5f;
  ContactListener detector;


  std::string type = "Rigidbody";
  std::string key = "???";
  Actor* actor = nullptr;
  bool enabled = true;

  void OnStart();
  void OnDestroy();

  /* Getters */
  b2Vec2 GetPosition();
  float GetRotation();
  b2Vec2 GetVelocity();
  float GetAngularVelocity();
  float GetGravityScale();
  b2Vec2 GetUpDirection();
  b2Vec2 GetRightDirection();

  /* Modifiers */
  void AddForce(b2Vec2 force);
  void SetVelocity(b2Vec2 vel);
  void SetPosition(b2Vec2 pos);
  void SetRotation(float degClockwise);
  void SetAngularVelocity(float degClockwise);
  void SetGravityScale(float scale);
  void SetUpDirection(b2Vec2 dir);
  void SetRightDirection(b2Vec2 dir);

  static void PhysicsStep();
  static HitResult* Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
  static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);
};

#endif
