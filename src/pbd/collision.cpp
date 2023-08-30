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
  size_t numContacts = _contacts.size();
  _deltas.resize(numContacts);
  _normals.resize(numContacts);
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
  Collision::_Hit hit;
  for (size_t contact = begin; contact < end; ++contact) {
    _ResolveContact(contacts[contact], particles, dt, &hit);
    _deltas[contact] = hit.delta;
    _normals[contact] = hit.normal;
  }
}

void Collision::_UpdateVelocities(size_t begin, size_t end, Particles* particles, const float invDt)
{
  for (size_t index = begin; index < end; ++index) {
    _UpdateVelocity(index, particles, invDt);
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

void Collision::UpdateVelocities(Particles* particles,  const float invDt)
{
  pxr::WorkParallelForN(GetNumContacts(),
    std::bind(&Collision::_UpdateVelocities, this,
      std::placeholders::_1, std::placeholders::_2, particles, invDt), 1);
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

void Collision::UpdateVelocitiesSerial(Particles* particles,  const float invDt)
{
  _UpdateVelocities(0, GetNumContacts(), particles, invDt);
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
 
void PlaneCollision::_ResolveContact(size_t index, Particles* particles, const float dt, Collision::_Hit* hit)
{
  float radius = particles->radius[index];
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) + _distance - radius;

  pxr::GfVec3f delta = _normal * -d * dt;
  particles->position[index] += delta;
  particles->predicted[index] += delta;

  hit->delta = delta;
  hit->normal = _normal;
}

void PlaneCollision::_UpdateVelocity(size_t index, Particles* particles, const float invDt)
{
  
  //particles->velocity[_contacts[index]] += 
  //  _normals[index] * _restitution * _deltas[index].GetLength() /** invDt*/+
  //  pxr::GfVec3f(_deltas[index][0] * _friction, 0.f, _deltas[index][2] * _friction) /** invDt*/;
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

void SphereCollision::_ResolveContact(size_t index, Particles* particles, const float dt, Collision::_Hit* hit)
{
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f& predicted = particles->predicted[index];
  const pxr::GfVec3f local = _invXform.Transform(predicted);
  const pxr::GfVec3f corrected = _xform.TransformDir(local.GetNormalized() * _radius - local) * dt;
  particles->position[index] += corrected;
  particles->predicted[index] += corrected;

  hit->delta = corrected;
  hit->normal = _invXform.Transform(particles->position[index]).GetNormalized();
}

void SphereCollision::_UpdateVelocity(size_t index, Particles* particles, const float invDt)
{
  //particles->velocity[_contacts[index]] += _normals[index] * _restitution * _deltas[index].GetLength();
}

JVR_NAMESPACE_CLOSE_SCOPE