#ifndef CONTACTLISTENER_H
#define CONTACTLISTENER_H
#include "box2d/box2d.h"
#include "Actor.h"

class Actor;

class ContactListener : public b2ContactListener {
public:
  void BeginContact(b2Contact *contact) override;
  void EndContact(b2Contact *contact) override;
};

class Collision {
public:
  Actor* other;
  b2Vec2 point;
  b2Vec2 relativeVelocity;
  b2Vec2 normal;
};

class HitResult {
public:
  Actor* actor = nullptr;
  b2Vec2 point;
  b2Vec2 normal;
  bool isTrigger = false;
};

#endif