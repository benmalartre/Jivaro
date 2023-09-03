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
  _contacts.reserve(numParticles);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));
}

void Collision::_BuildContacts(Particles* particles, const pxr::VtArray<Body*>& bodies)
{

  size_t numParticles = particles->position.size();
  size_t numBodies = bodies.size();
  for (auto& contact : _contacts)delete contact;

  _contacts.clear();
  _contacts.reserve(numBodies);

  pxr::VtArray<int> elements;
  int bodyIdx = 0;
  for (size_t index = 0; index < numParticles; ++index) {
    if (CheckHit(index)) {
      if (particles->body[index] != bodyIdx) {
        if (elements.size()) {
          _contacts.push_back(new CollisionConstraint(bodies[particles->body[bodyIdx]], elements));
          elements.clear();
          bodyIdx = particles->body[index];
          continue;
        } 
        bodyIdx = particles->body[index];
      }
      elements.push_back(index - bodies[bodyIdx]->offset);
    }
  } if (elements.size()) {
    _contacts.push_back(new CollisionConstraint(bodies[particles->body[bodyIdx]], elements));
  }
}

void Collision::_FindContacts(size_t begin, size_t end, Particles* particles)
{
  for (size_t index = begin; index < end; ++index) {
    _FindContact(index, particles);
  }
}


void Collision::FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles));
  _BuildContacts(particles, bodies);
}

void Collision::FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles);
  _BuildContacts(particles, bodies);
}

//----------------------------------------------------------------------------------------
// Plane Collision
//----------------------------------------------------------------------------------------
PlaneCollision::PlaneCollision(const float restitution, const float friction,
  const pxr::GfVec3f& normal, const pxr::GfVec3f& position, const float distance) 
  : Collision(restitution, friction)
  , _position(position)
  , _normal(normal)
  , _distance(0.1f)
{
}

void PlaneCollision::_FindContact(size_t index, Particles* particles)
{
  if (!Affects(index))return;
  float radius = particles->radius[index];
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) + _distance - radius;
  if (d < 0.0) {
    SetHit(index);
  }
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

void SphereCollision::_FindContact(size_t index, Particles* particles)
{
  if (!Affects(index))return;
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f local = _invXform.Transform(particles->predicted[index]);
  if (local.GetLengthSq() < radius2) {
    SetHit(index);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE