#include "ContactListener.h"

void ContactListener::BeginContact(b2Contact* contact) {
  b2Fixture* fixA = contact->GetFixtureA();
  b2Fixture* fixB = contact->GetFixtureB();

  Actor* actA = reinterpret_cast<Actor*>(fixA->GetUserData().pointer);
  Actor* actB = reinterpret_cast<Actor*>(fixB->GetUserData().pointer);

  // Return if either is a phantom fixture
  if (!actA || !actB) return;

  b2WorldManifold worldManifold;
  contact->GetWorldManifold(&worldManifold);

  Collision collision;
  collision.other = actB;
  collision.normal = worldManifold.normal;
  collision.point = worldManifold.points[0];
  collision.relativeVelocity = fixA->GetBody()->GetLinearVelocity() - fixB->GetBody()->GetLinearVelocity();

  if (!fixA->IsSensor() && !fixB->IsSensor()) {
    actA->OnCollisionEnter(collision);
    collision.other = actA;
    actB->OnCollisionEnter(collision);
  }
  else if (fixA->IsSensor() && fixB->IsSensor()) {
    collision.normal = b2Vec2(-999.0f, -999.0f);
    collision.point = b2Vec2(-999.0f, -999.0f);
    actA->OnTriggerEnter(collision);
    collision.other = actA;
    actB->OnTriggerEnter(collision);
  }
}

void ContactListener::EndContact(b2Contact* contact) {
  b2Fixture* fixA = contact->GetFixtureA();
  b2Fixture* fixB = contact->GetFixtureB();

  Actor* actA = reinterpret_cast<Actor*>(fixA->GetUserData().pointer);
  Actor* actB = reinterpret_cast<Actor*>(fixB->GetUserData().pointer);

  // Return if either is a phantom fixture
  if (!actA || !actB) return;

  b2WorldManifold worldManifold;
  contact->GetWorldManifold(&worldManifold);

  Collision collision;
  collision.other = actB;
  collision.normal = b2Vec2(-999.0f, -999.0f);
  collision.point = b2Vec2(-999.0f, -999.0f);
  collision.relativeVelocity = fixA->GetBody()->GetLinearVelocity() - fixB->GetBody()->GetLinearVelocity();

  if (!fixA->IsSensor() && !fixB->IsSensor()) {
    actA->OnCollisionExit(collision);
    collision.other = actA;
    actB->OnCollisionExit(collision);
  }
  else if (fixA->IsSensor() && fixB->IsSensor()) {
    actA->OnTriggerExit(collision);
    collision.other = actA;
    actB->OnTriggerExit(collision);
  }
}