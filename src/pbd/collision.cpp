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
  pxr::VtArray<Constraint*>& contacts, float dt)
{
  CollisionConstraint* constraint = NULL;
  size_t numParticles = particles->position.size();
  size_t numBodies = bodies.size();

  _p2c.resize(numParticles, -1);
  _c2p.clear();
  _c2p.reserve(numParticles);
  size_t c2p_idx = 0;
  pxr::VtArray<int> elements;
  _numContacts = 0;
  size_t numConstraints = 0;
  int bodyIdx = -1;
  for (size_t index = 0; index < numParticles; ++index) {
    if (CheckHit(index)) {
      _p2c[index] = _numContacts++;
      _c2p.push_back(index);
      if (particles->body[index] != bodyIdx) {
        if (elements.size()) {
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx, dt);
          contacts.push_back(constraint);
          numConstraints++;
          elements.clear();
        }
        bodyIdx = particles->body[index];
      } 
      elements.push_back(index - bodies[particles->body[index]]->offset);
    }
  } 
  
  if (elements.size()) {
    constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
    StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx, dt);
    contacts.push_back(constraint);
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
  _BuildContacts(particles, bodies, contacts, dt);
}

void Collision::FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
  pxr::VtArray<Constraint*>& contacts, float dt)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles, dt);
  _BuildContacts(particles, bodies, contacts, dt);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId, float dt)
{
  const size_t offset = ((Body*)body + geomId * sizeof(Body))->offset;
  pxr::GfVec3f contact;
  _contacts.resize(n);
  for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
    _contacts[elemIdx].SetGeometryIndex(geomId);
    _StoreContactLocation(particles, elements[elemIdx], body, _contacts[elemIdx], dt);
  }
}

void Collision::SolveVelocities(Particles* particles, float dt)
{
  for (size_t elemIdx = 0; elemIdx < _contacts.size(); ++elemIdx) {
    _SolveVelocity(particles, _c2p[elemIdx], dt);
  }
}

//----------------------------------------------------------------------------------------
// Plane Collision
//----------------------------------------------------------------------------------------
PlaneCollision::PlaneCollision(const float restitution, const float friction,
  const pxr::GfVec3f& normal, const pxr::GfVec3f& position) 
  : Collision(restitution, friction)
  , _position(position)
  , _normal(normal)
{
}

void PlaneCollision::_FindContact(size_t index, Particles* particles, float dt)
{
  if (!Affects(index))return;
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * dt);
  float d = pxr::GfDot(_normal, predicted - _position) - particles->radius[index];
  SetHit(index, d < 0.f);
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int elem, const Body* body, Contact& location, float dt)
{
  const pxr::GfVec3f predicted(particles->position[elem] + particles->velocity[elem] * dt);
  float d = pxr::GfDot(_normal, predicted - _position) - particles->radius[elem];
  const pxr::GfVec3f intersection = predicted + _normal * -d;
  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], d);
  location.SetCoordinates(coords);

  location.SetPosition(intersection);
  location.SetNormal(_normal);
  location.SetResponse(pxr::GfVec3f(0.f));
}

void PlaneCollision::_SolveVelocity(Particles* particles, size_t index, float dt)
{
  if(!CheckHit(index))return;
  return;
  // Tangential component of relative motion
  const pxr::GfVec3f tangent = 
    (particles->velocity[index] - (_normal * pxr::GfDot(particles->velocity[index], _normal))) * -1.f;

  const pxr::GfVec3f velocity = 
    _normal * _contacts[_p2c[index]].GetT() * _restitution/* + tangent * _friction*/;

  particles->velocity[index] += velocity;
  
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
  SetHit(index, local.GetLengthSq() < radius2);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int elem, const Body* body, Contact& location, float dt)
{
}


void SphereCollision::_SolveVelocity(Particles* particles, size_t index, float dt)
{
  /*
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) - particles->radius[index];

  // Tangential component of relative motion
  const pxr::GfVec3f tangent = 
    (particles->velocity[index] - (_normal * pxr::GfDot(particles->velocity[index], _normal))) * -1.f;

  if (d < 0.f) velocity += _normal * -d * _restitution + tangent * _friction * dt;
  */
}



JVR_NAMESPACE_CLOSE_SCOPE