#include <pxr/base/work/loops.h>

#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

void Collision::_ResetContacts(Particles* particles)
{
  const size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));
}

void Collision::_BuildContacts(Particles* particles, const pxr::VtArray<Body*>& bodies,
  pxr::VtArray<Constraint*>& contacts)
{
  size_t numParticles = particles->position.size();
  size_t numBodies = bodies.size();

  pxr::VtArray<int> elements;
  _numContacts = 0;
  size_t numConstraints = 0;
  int bodyIdx = -1;
  for (size_t index = 0; index < numParticles; ++index) {
    if (CheckHit(index)) {
      if (particles->body[index] != bodyIdx) {
        if (elements.size()) {
          contacts.push_back(new CollisionConstraint(bodies[bodyIdx], this, elements));
          _numContacts += elements.size();
          numConstraints++;
          elements.clear();
        }
        bodyIdx = particles->body[index];
      } 
      elements.push_back(index - bodies[particles->body[index]]->offset);
    }
  } if (elements.size()) {
    contacts.push_back(new CollisionConstraint(bodies[bodyIdx], this, elements));
    _numContacts += elements.size();
    numConstraints++;
  }
}

void Collision::_FindContacts(size_t begin, size_t end, Particles* particles, float dt)
{
  for (size_t index = begin; index < end; ++index) {
    _FindContact(index, particles, dt);
  }
}

void Collision::FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies, 
  pxr::VtArray<Constraint*>& contacts, float dt)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, dt));
  _BuildContacts(particles, bodies, contacts);
}

void Collision::FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
  pxr::VtArray<Constraint*>& contacts, float dt)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles, dt);
  _BuildContacts(particles, bodies, contacts);
}

//----------------------------------------------------------------------------------------
// Plane Collision
//----------------------------------------------------------------------------------------
PlaneCollision::PlaneCollision(const float restitution, const float friction,
  const pxr::GfVec3f& normal, const pxr::GfVec3f& position, const float distance) 
  : Collision(restitution, friction)
  , _position(position)
  , _normal(normal)
  , _distance(distance)
{
}

void PlaneCollision::_FindContact(size_t index, Particles* particles, float dt)
{
  if (!Affects(index))return;
  float radius = particles->radius[index];
  float d = pxr::GfDot(_normal, particles->predicted[index] + particles->velocity[index] * dt - _position) - _distance - radius;
  if (d < 0.f) {
    SetHit(index);
  }
}

pxr::GfVec3f PlaneCollision::ResolveContact(Particles* particles, size_t index)
{
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) - particles->radius[index];
  if (d < 0.f) return _normal * -d;
  else return pxr::GfVec3f(0.f);
}

pxr::GfVec3f PlaneCollision::ResolveVelocity(Particles* particles, float depth, size_t index)
{
  pxr::GfVec3f tangent(-particles->velocity[index][0], 0.f, -particles->velocity[index][2]);
  return _normal * _restitution * depth + tangent * _friction;
}

//----------------------------------------------------------------------------------------
// Sphere Collision
//----------------------------------------------------------------------------------------
SphereCollision::SphereCollision(const float restitution, const float friction,
  const pxr::GfMatrix4f& xform, const float radius)
  : Collision(restitution, friction)
  , _xform(xform)
  , _invXform(xform.GetInverse())
  , _radius(radius)
{
}

void SphereCollision::_FindContact(size_t index, Particles* particles, float dt)
{
  if (!Affects(index))return;
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f local = _invXform.Transform(particles->predicted[index] + particles->velocity[index] * dt * 2.f);
  if (local.GetLengthSq() < radius2) {
    SetHit(index);
  }
}

pxr::GfVec3f SphereCollision::ResolveContact(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);
}

pxr::GfVec3f SphereCollision::ResolveVelocity(Particles* particles, float depth, size_t index)
{
  return pxr::GfVec3f(0.f);
}

JVR_NAMESPACE_CLOSE_SCOPE