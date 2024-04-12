#include <pxr/base/work/loops.h>

#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
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
  pxr::VtArray<Constraint*>& contacts, float ft)
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
          StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx, ft);
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
    StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx, ft);
    contacts.push_back(constraint);
    _numContacts += elements.size();
    numConstraints++;
  }
}

void Collision::_FindContacts(size_t begin, size_t end, Particles* particles, float ft)
{
  for (size_t index = begin; index < end; ++index) {
    _FindContact(index, particles, ft);
  }
}

void Collision::FindContacts(Particles* particles, const pxr::VtArray<Body*>& bodies, 
  pxr::VtArray<Constraint*>& contacts, float ft)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft));
  _BuildContacts(particles, bodies, contacts, ft);
}

void Collision::FindContactsSerial(Particles* particles, const pxr::VtArray<Body*>& bodies,
  pxr::VtArray<Constraint*>& contacts, float ft)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles, ft);
  _BuildContacts(particles, bodies, contacts, ft);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, const Body* body, size_t geomId, float ft)
{
  const size_t offset = ((Body*)body + geomId * sizeof(Body))->offset;
  pxr::GfVec3f contact;
  _contacts.resize(n);
  for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
    _contacts[elemIdx].SetGeometryIndex(geomId);
    _StoreContactLocation(particles, elements[elemIdx], body, _contacts[elemIdx], ft);
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
PlaneCollision::PlaneCollision(Geometry* collider,  float restitution, float friction) 
  : Collision(collider, restitution, friction)
{
  Plane* plane = (Plane*)collider;
  _UpdatePositionAndNormal();
}

void PlaneCollision::_UpdatePositionAndNormal()
{
  Plane* plane = (Plane*)_collider;
  _position =  plane->GetOrigin(true);
  _normal = plane->GetNormal(true);
}


void PlaneCollision::_FindContact(size_t index, Particles* particles, float ft)
{
  if (!Affects(index))return;
  _UpdatePositionAndNormal();

  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->radius[index];
  SetHit(index, d < 0.f);
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int index, const Body* body, Location & location, float ft)
{
  const pxr::GfVec3f velocity = particles->velocity[index] * ft;
  const float vl = velocity.GetLength();
  const pxr::GfVec3f predicted(particles->position[index] + velocity);
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->radius[index];

  const pxr::GfVec3f intersection = predicted + _normal * -d;

  const pxr::GfVec3f vRel = particles->velocity[index] - /*collider velocity*/ pxr::GfVec3f(0.f);
  const float vn = pxr::GfDot(vRel, _normal);
  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);
}

void PlaneCollision::_SolveVelocity(Particles* particles, size_t index, float dt)
{

  if(!CheckHit(index))return;
    // need to rehabilit contact and save computation in there
    
  // Relative normal and tangential velocities
  // subtract collider velocity from point velocity
  const pxr::GfVec3f v = particles->velocity[index] - pxr::GfVec3f(0.f);
  const float vn = pxr::GfDot(v, _normal);
  const pxr::GfVec3f vt = v - _normal * vn;
  const float vt_len = vt.GetLength();

  /*
  // (30) Friction
  float lambda_n = -0.001f;
  if (vt_len > 0.000001) {
    const float Fn = -lambda_n / (dt * dt);
    const float friction = pxr::GfMin(dt * _friction * Fn, vt_len);
    particles->velocity[index] -= vt.GetNormalized() * friction;
  }
  */
/* (34) Restitution
  *
  * To avoid jittering we set e = 0 if vn is small (`threshold`).
  * 
  * Note: min() was replaced with max() due to the flipped sign convention.
  *
  * Note: `vn_tilde` is calculated in ContactSet before the position solve (Eq. 29)
  */


  const float threshold = 2.f * 5.f * dt;
  const float e = pxr::GfAbs(vn) <= threshold ? 0.0 : _restitution;
  const float vn_tilde = GetContactT(index);
  const float restitution = -vn + pxr::GfMin(-e * vn_tilde, 0.f);
  particles->velocity[index] += _normal * restitution;
}

float PlaneCollision::GetValue(Particles* particles, size_t index)
{
  const float d = 
    pxr::GfDot(_normal, particles->position[index] - _position)  - 
    particles->radius[index];
  return d < 0.f ? d : 0.f;
}
  
pxr::GfVec3f PlaneCollision::GetGradient(Particles* particles, size_t index)
{
  return _normal;
}


//----------------------------------------------------------------------------------------
// Sphere Collision
//----------------------------------------------------------------------------------------
SphereCollision::SphereCollision(Geometry* collider,   float restitution, float friction)
  : Collision(collider, restitution, friction)
{
  Sphere* sphere = (Sphere*)collider;
  /*
  _radius = sphere->GetRadius();
  */
}

void SphereCollision::_FindContact(size_t index, Particles* particles, float dt)
{
  if (!Affects(index))return;
  const float radius2 = _radius * _radius;
  const pxr::GfVec3f local = _invXform.Transform(particles->predicted[index] + particles->velocity[index] * dt * 2.f);
  SetHit(index, local.GetLengthSq() < radius2);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int elem, const Body* body, Location& location, float dt)
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


float SphereCollision::GetValue(Particles* particles, size_t index)
{
  return 0.f;
}
  
pxr::GfVec3f SphereCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);
}


JVR_NAMESPACE_CLOSE_SCOPE