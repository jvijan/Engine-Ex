#include "Rigidbody.h"
#include "ComponentDB.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <vector>

class ClosestRaycastCallback : public b2RayCastCallback {
public:
  HitResult result;
  bool hasHit = false;
  float closestFraction = 1.0f;

  float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
    if (!actor) return -1;
    if (fraction < closestFraction) {
      closestFraction = fraction;
      result.actor = actor;
      result.point = point;
      result.normal = normal;
      result.isTrigger = fixture->IsSensor();
      hasHit = true;
    }
    return fraction;
  }
};

class AllRaycastCallback : public b2RayCastCallback {
public:
  std::map<float, HitResult*> hits;

  float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
    if (!actor) return -1;
    HitResult* result = new HitResult();
    result->actor = actor;
    result->point = point;
    result->normal = normal;
    result->isTrigger = fixture->IsSensor();
    hits.insert({fraction, result});
    return 1.0f;
  }
};

void Rigidbody::OnStart() {
  // Set up the world correctly if we haven't already
  if (!worldInitialized) {
    world = new b2World(b2Vec2(0.0f, 9.8f));
    world->SetContactListener(&detector);
    worldInitialized = true;
  }
  // Create body definition for our new body
  b2BodyDef bodyDef;
  if (bodyType == "dynamic") {
    bodyDef.type = b2_dynamicBody;
  }
  else if (bodyType == "kinematic") {
    bodyDef.type = b2_kinematicBody;
  }
  else if (bodyType == "static") {
    bodyDef.type = b2_staticBody;
  }
  bodyDef.position = b2Vec2(x, y);
  bodyDef.bullet = precise;
  bodyDef.angularDamping = angularFriction;
  bodyDef.gravityScale = gravityScale;
  bodyDef.angle = rotationDegrees * b2_pi / 180.0f;

  body = world->CreateBody(&bodyDef);

  // Default values for now until we implement more
  if (!hasCollider && !hasTrigger) {
    b2PolygonShape phantomShape;
    phantomShape.SetAsBox(width * 0.5f, height* 0.5f);
    b2FixtureDef phantomFixture;
    phantomFixture.shape = &phantomShape;
    phantomFixture.density = density;
    phantomFixture.friction = friction;
    phantomFixture.restitution = bounciness;
    phantomFixture.isSensor = true;
    phantomFixture.userData.pointer = reinterpret_cast<uintptr_t>(nullptr);
    body->CreateFixture(&phantomFixture);
  }
  if (hasCollider) {
    b2Shape* colShape = nullptr;
    if (colliderType == "box") {
      b2PolygonShape* myShape = new b2PolygonShape();
      myShape->SetAsBox(width * 0.5f, height * 0.5f);
      colShape = myShape;
    }
    else if (colliderType == "circle") {
      b2CircleShape* myShape = new b2CircleShape();
      myShape->m_radius = radius;
      colShape = myShape;
    }
    b2FixtureDef fixture;
    fixture.shape = colShape;
    fixture.density = density;
    fixture.friction = friction;
    fixture.restitution = bounciness;
    fixture.isSensor = false;
    fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
    body->CreateFixture(&fixture);
  }
  if (hasTrigger) {
    b2Shape* trigShape = nullptr;
    if (triggerType == "box") {
      b2PolygonShape* shape = new b2PolygonShape();
      shape->SetAsBox(triggerWidth * 0.5f, triggerHeight * 0.5f);
      trigShape = shape;
    }
    else if (triggerType == "circle") {
      b2CircleShape* shape = new b2CircleShape();
      shape->m_radius = triggerRadius;
      trigShape = shape;
    }
    b2FixtureDef fixture;
    fixture.shape = trigShape;
    fixture.density = density;
    fixture.isSensor = true;
    fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
    body->CreateFixture(&fixture);
  }
}

void Rigidbody::OnDestroy() {
  world->DestroyBody(body);
}

b2Vec2 Rigidbody::GetPosition() {
  if (body == nullptr) {
    return b2Vec2(x, y);
  }
  return body->GetPosition();
}

float Rigidbody::GetRotation() {
  if (body == nullptr) {
    return rotationDegrees;
  }
  return body->GetAngle() * (180.0f / b2_pi);
}

b2Vec2 Rigidbody::GetVelocity() {
  if (body == nullptr) {
    return b2Vec2(0.0f, 0.0f);
  }
  return body->GetLinearVelocity();
}

float Rigidbody::GetAngularVelocity() {
  if (body == nullptr) {
    return 0.0f;
  }
  return body->GetAngularVelocity() * (180.0f / b2_pi);
}

float Rigidbody::GetGravityScale() {
  if (body == nullptr) {
    return gravityScale;
  }
  return body->GetGravityScale();
}

b2Vec2 Rigidbody::GetUpDirection() {
  if (body == nullptr) {
    float angle = rotationDegrees * (b2_pi / 180.0f);
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    return result;
  }
  float angle = body->GetAngle();
  b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
  result.Normalize();
  return result;
}

b2Vec2 Rigidbody::GetRightDirection() {
  if (body == nullptr) {
    float angle = rotationDegrees * (b2_pi / 180.0f);
    b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
    return result;
  }
  float angle = body->GetAngle();
  b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
  result.Normalize();
  return result;
}

void Rigidbody::AddForce(b2Vec2 force) {
  if (body == nullptr) {
    return;
  }
  body->ApplyForceToCenter(force, true);
}

void Rigidbody::SetVelocity(b2Vec2 vel) {
  if (body == nullptr) {
    return;
  }
  body->SetLinearVelocity(vel);
}

void Rigidbody::SetPosition(b2Vec2 pos) {
  if (body == nullptr) {
    x = pos.x;
    y = pos.y;
    return;
  }
  body->SetTransform(pos, GetRotation());
}

void Rigidbody::SetRotation(float degClockwise) {
  if (body == nullptr) {
    rotationDegrees = degClockwise;
    return;
  }
  body->SetTransform(GetPosition(), degClockwise * (b2_pi / 180.0f));
}

void Rigidbody::SetAngularVelocity(float degClockwise) {
  if (body == nullptr) {
    return;
  }
  body->SetAngularVelocity(degClockwise * (b2_pi / 180.0f));
}

void Rigidbody::SetGravityScale(float scale) {
  if (body == nullptr) {
    gravityScale = scale;
    return;
  }
  body->SetGravityScale(scale);
}

void Rigidbody::SetUpDirection(b2Vec2 dir) {
  dir.Normalize();
  float angle = glm::atan(dir.x, -dir.y);
  if (body == nullptr) {
    rotationDegrees = angle * (180.0f / b2_pi);
    return;
  }
  body->SetTransform(GetPosition(), angle);
}

void Rigidbody::SetRightDirection(b2Vec2 dir) {
  dir.Normalize();
  float angle = glm::atan(dir.x, -dir.y) - (b2_pi / 2.0f);
  if (body == nullptr) {
    rotationDegrees = angle * (180.0f / b2_pi);
    return;
  }
  body->SetTransform(GetPosition(), angle);
}

void Rigidbody::PhysicsStep() {
  if (worldInitialized) {
    world->Step(1.0f / 60.0f, 8, 3);
  }
}

HitResult* Rigidbody::Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
  if (!worldInitialized || dist <= 0.0f) return nullptr;
  dir.Normalize();
  b2Vec2 end(pos.x + dir.x * dist, pos.y + dir.y * dist);
  ClosestRaycastCallback callback;
  world->RayCast(&callback, pos, end);
  if (!callback.hasHit) return nullptr;
  static HitResult result;
  result = callback.result;
  return &result;
}

luabridge::LuaRef Rigidbody::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
  if (!worldInitialized || dist <= 0.0f)
    return luabridge::LuaRef(ComponentDB::GetLuaState());
  dir.Normalize();
  b2Vec2 end(pos.x + dir.x * dist, pos.y + dir.y * dist);
  AllRaycastCallback callback;
  world->RayCast(&callback, pos, end);
  luabridge::LuaRef table = luabridge::newTable(ComponentDB::GetLuaState());
  // Iterate over map
  int i = 1;
  for (auto it = callback.hits.begin(); it != callback.hits.end(); it++) {
    table[i] = it->second;
    i++; 
  }
  return table;
}
