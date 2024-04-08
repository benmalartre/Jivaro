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
  CollisionConstraint* constraint = NULL;
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
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx);
          //constraint->StoreContactsLocation(particles, 1.f, particles->body[index]);
          contacts.push_back(constraint);
          _numContacts += elements.size();
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
    //constraint->StoreContactsLocation(particles, 1.f, bodyIdx);
    StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx);
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
  _BuildContacts(particles, bodies, contacts);
}

void Collision::FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
  pxr::VtArray<Constraint*>& contacts, float dt)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles, dt);
  _BuildContacts(particles, bodies, contacts);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId)
{
  const size_t offset = ((Body*)body + geomId * sizeof(Body))->offset;
  pxr::GfVec3f contact;
  _contacts.resize(n);
  _time.resize(n);
  for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
    _contacts[elemIdx].SetGeometryIndex(geomId);
    _StoreContactLocation(particles, elements[elemIdx], body, _contacts[elemIdx]);

  }
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

void PlaneCollision::_StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location)
{

}


void PlaneCollision::ResolveContact(Particles* particles, size_t index, float dt, Location& location)
{
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) - particles->radius[index];
  /*
  if (d < 0.f) return _position + _normal * -d;
  else return particles->position[index];
  */
}

void PlaneCollision::ResolveVelocity(Particles* particles, size_t index, float dt, pxr::GfVec3f& velocity)
{
  /*
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) - particles->radius[index];

  // Tangential component of relative motion
  const pxr::GfVec3f tangent = 
    (particles->velocity[index] - (_normal * pxr::GfDot(particles->velocity[index], _normal))) * -1.f;

  if (d < 0.f) return _normal * -d * _restitution + tangent * _friction * dt;
  else return pxr::GfVec3f(0.f);
  */

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

void SphereCollision::_StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location)
{

}

void SphereCollision::ResolveContact(Particles* particles, size_t index, float dt, Location& location)
{
  
}

void SphereCollision::ResolveVelocity(Particles* particles, size_t index, float dt, pxr::GfVec3f& velocity)
{
}



JVR_NAMESPACE_CLOSE_SCOPE