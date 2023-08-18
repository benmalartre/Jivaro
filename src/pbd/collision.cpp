#include <pxr/base/work/loops.h>

#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

void Collision::_ResetContacts(Particles* particles)
{
  const size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  _contacts.reserve(numParticles);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));
}

void Collision::_BuildContacts(Particles* particles)
{
  _contacts.clear();
  for (size_t index = 0; index < particles->GetNumParticles(); ++index) {
    if (CheckHit(index))
      _contacts.push_back(index);
  }
}

void Collision::_FindContacts(size_t begin, size_t end, Particles* particles)
{
  for (size_t index = begin; index < end; ++index) {
    _FindContact(index, particles);
  }
}
 
void Collision::_ResolveContacts(size_t begin, size_t end, Particles* particles, const float dt)
{
  const pxr::VtArray<int>& contacts = GetContacts();
  for (size_t contact = begin; contact < end; ++contact) {
    _ResolveContact(contacts[contact], particles, dt);
  }
}

void Collision::FindContacts(Particles* particles)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles));
  _BuildContacts(particles);
}

void Collision::ResolveContacts(Particles* particles, const float dt)
{
  pxr::WorkParallelForN(GetNumContacts(),
    std::bind(&Collision::_ResolveContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, dt), 1);
}

void Collision::FindContactsSerial(Particles* particles)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles);
  _BuildContacts(particles);
}

void Collision::ResolveContactsSerial(Particles* particles, const float dt)
{
  _ResolveContacts(0, GetNumContacts(), particles, dt);
}

//----------------------------------------------------------------------------------------
// Plane Collision
//----------------------------------------------------------------------------------------
PlaneCollision::PlaneCollision()
  : _position(0.f, 0.f, 0.f)
  , _normal(0.f, 1.f, 0.f)
  , _distance(0.1f)
{
}

PlaneCollision::PlaneCollision(const pxr::GfVec3f& normal, const pxr::GfVec3f& position) 
  : _position(position)
  , _normal(normal)
  , _distance(0.1f)
{
}

void PlaneCollision::_FindContact(size_t index, Particles* particles)
{
  if (!Affects(index))return;
  float radius = particles->radius[index];
  float d = pxr::GfDot(_normal, particles->predicted[index]) + _distance - radius;

  if (d < 0.0) {
    SetHit(index);
  }
}
 
void PlaneCollision::_ResolveContact(size_t index, Particles* particles, const float dt)
{
  float radius = particles->radius[index];
  float d = pxr::GfDot(_normal, particles->predicted[index]) + _distance - radius;

  if (d < 0.0) {
    pxr::GfVec3f delta = _normal * -d /** dt*/;
    particles->position[index] += delta;
    particles->predicted[index] += delta;
  }
}

//----------------------------------------------------------------------------------------
// Sphere Collision
//----------------------------------------------------------------------------------------
SphereCollision::SphereCollision()
  : _xform(1.f)
  , _invXform(1.f)
  , _radius(1.f)
{
}

SphereCollision::SphereCollision(const pxr::GfMatrix4f& xform, const float radius)
  : _xform(xform)
  , _invXform(xform.GetInverse())
  , _radius(radius)
{
}

void SphereCollision::_FindContact(size_t index, Particles* particles)
{
  if (!Affects(index))return;
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f local = _invXform.Transform(particles->position[index]);
  if (local.GetLengthSq() < radius2) {
    SetHit(index);
  }
}

void SphereCollision::_ResolveContact(size_t index, Particles* particles, const float dt)
{
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f& predicted = particles->predicted[index];
  const pxr::GfVec3f local = _invXform.Transform(predicted);
  if (local.GetLengthSq() < radius2) {
    const pxr::GfVec3f corrected = _xform.TransformDir(local.GetNormalized() * _radius - local);
    particles->position[index] += corrected;
    particles->predicted[index] += corrected;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE