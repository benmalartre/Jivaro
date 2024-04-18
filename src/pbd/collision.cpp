#include <pxr/base/work/loops.h>

#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../acceleration/bvh.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

const float Collision::TOLERANCE_FACTOR = 1.2f;

void Collision::_ResetContacts(Particles* particles)
{
  const size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));
}

void Collision::_BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& contacts, float ft)
{
  CollisionConstraint* constraint = NULL;
  size_t numParticles = particles->_position.size();
  size_t numBodies = bodies.size();

  _p2c.resize(numParticles, -1);
  _c2p.clear();
  _c2p.reserve(numParticles);
  _c2pIdx = 0;
  pxr::VtArray<int> elements;
  _numContacts = 0;
  size_t numConstraints = 0;
  int bodyIdx = -1;

  for (size_t index = 0; index < numParticles; ++index) {
    if (CheckHit(index)) {
      _p2c[index] = _numContacts++;
      _c2p.push_back(index);
      if (particles->_body[index] != bodyIdx) {
        if (elements.size()) {
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          StoreContactsLocation(particles, & elements[0], elements.size(), bodies[0], bodyIdx, ft);
          contacts.push_back(constraint);
          numConstraints++;
          elements.clear();
        }
        bodyIdx = particles->_body[index];
      } 
      elements.push_back(index - bodies[particles->_body[index]]->GetOffset());
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

void Collision::_UpdateParameters( const pxr::UsdPrim& prim, double time)
{

  prim.GetAttribute(pxr::TfToken("Restitution")).Get(&_restitution, time);
  prim.GetAttribute(pxr::TfToken("Friction")).Get(&_friction, time);
}

void Collision::Init(size_t numParticles) 
{
  _p2c.resize(numParticles, -1);
  _c2p.resize(numParticles, -1);
  _c2pIdx = 0;
}

void Collision::Update(const pxr::UsdPrim& prim, double time){}

void Collision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& contacts, float ft)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft));
  _BuildContacts(particles, bodies, contacts, ft);
}

void Collision::FindContactsSerial(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& contacts, float ft)
{
  _ResetContacts(particles);
  _FindContacts(0, particles->GetNumParticles(), particles, ft);
  _BuildContacts(particles, bodies, contacts, ft);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, 
  const Body* body, size_t geomId, float ft)
{
  const size_t offset = ((Body*)body + geomId * sizeof(Body))->GetOffset();
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

void Collision::_SolveVelocity(Particles* particles, size_t index, float dt)
{
  if(!CheckHit(index))return;    

  pxr::GfVec3f normal = GetGradient(particles, index);

  // Relative normal and tangential velocities
  const pxr::GfVec3f v = particles->_velocity[index] - pxr::GfVec3f(0.f);
  const float vn = pxr::GfDot(v, normal);
  const pxr::GfVec3f vt = v - normal * vn;
  const float vtLen = vt.GetLength();

  // Friction
  if (vtLen > 1e-6 && _friction > 1e-6) {
    float lambdaN = -(1.f/_friction);
    const float Fn = -lambdaN / (dt * dt);
    const float friction = pxr::GfMin(dt * _friction * Fn, vtLen);
    particles->_velocity[index] -= vt.GetNormalized() * friction;
  }

  // Restitution
  const float threshold = 2.f * 9.81 * dt;
  const float e = pxr::GfAbs(vn) <= threshold ? 0.0 : _restitution;
  const float vnTilde = GetContactT(index);
  const float restitution = -vn + pxr::GfMax(-e * vnTilde, 0.f);
  particles->_velocity[index] += normal * restitution;
}

//----------------------------------------------------------------------------------------
// Plane Collision
//----------------------------------------------------------------------------------------
size_t PlaneCollision::TYPE_ID = Collision::PLANE;

PlaneCollision::PlaneCollision(Geometry* collider, const pxr::SdfPath& path, 
  float restitution, float friction) 
  : Collision(collider, path, restitution, friction)
{
  Plane* plane = (Plane*)collider;
  _UpdatePositionAndNormal();
}

float PlaneCollision::GetValue(Particles* particles, size_t index)
{
  return pxr::GfDot(_normal, particles->_predicted[index] - _position) -
    particles->_radius[index];
}

pxr::GfVec3f PlaneCollision::GetGradient(Particles* particles, size_t index)
{
  return _normal;
}

void PlaneCollision::Update(const pxr::UsdPrim& prim, double time) 
{
  _UpdatePositionAndNormal();
  _UpdateParameters(prim, time);
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

  const pxr::GfVec3f predicted(particles->_position[index] + particles->_velocity[index] * ft);
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->_radius[index] * TOLERANCE_FACTOR;
  SetHit(index, d < 0.f);
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int index, 
  const Body* body, Location & location, float ft)
{
  const pxr::GfVec3f velocity = particles->_velocity[index] * ft;
  const pxr::GfVec3f predicted(particles->_position[index] + velocity);
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->_radius[index];

  const pxr::GfVec3f intersection = predicted + _normal * -d;

  const pxr::GfVec3f relativeVelocity = particles->_velocity[index] - _collider->GetVelocity();
  const float vn = pxr::GfDot(relativeVelocity, _normal);
  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);

  particles->_color[index] = pxr::GfVec3f((1.f - d), d, 0.2f);
}

//----------------------------------------------------------------------------------------
// Sphere Collision
//----------------------------------------------------------------------------------------
size_t SphereCollision::TYPE_ID = Collision::SPHERE;
SphereCollision::SphereCollision(Geometry* collider, const pxr::SdfPath& path, 
  float restitution, float friction)
  : Collision(collider, path, restitution, friction)
{
  Sphere* sphere = (Sphere*)collider;
  _UpdateCenterAndRadius();
}

void SphereCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateCenterAndRadius();
  _UpdateParameters(prim, time);
}

void SphereCollision::_UpdateCenterAndRadius()
{
  Sphere* sphere = (Sphere*)_collider;
  _center = sphere->GetCenter();
  _radius = sphere->GetRadius();
} 


void SphereCollision::_FindContact(size_t index, Particles* particles, float ft)
{
  if (!Affects(index))return;
  const float radius = _radius + particles->_radius[index] * TOLERANCE_FACTOR;
  const pxr::GfVec3f predicted(particles->_position[index] + particles->_velocity[index] * ft);
  SetHit(index, (predicted - _center).GetLength() < radius);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int index, 
  const Body* body, Location& location, float ft)
{
  const pxr::GfVec3f velocity = particles->_velocity[index] * ft;
  const pxr::GfVec3f predicted(particles->_position[index] + velocity);
  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if(nL>0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(0.f,0.f,0.f);

  const pxr::GfVec3f intersection = _center + normal * (_radius + particles->_radius[index]);

  const pxr::GfVec3f relativeVelocity = particles->_velocity[index] - _collider->GetVelocity();
  const float vn = pxr::GfDot(relativeVelocity, normal);

  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);
}

float SphereCollision::GetValue(Particles* particles, size_t index)
{
  return (particles->_predicted[index] - _center).GetLength() -
    _radius - particles->_radius[index];
}
  
pxr::GfVec3f SphereCollision::GetGradient(Particles* particles, size_t index)
{
  return (particles->_predicted[index] - _center).GetNormalized();
}

const pxr::GfVec3f SphereCollision::GetContactNormal(size_t index) const 
{
  return (GetContactPosition(index) - _center).GetNormalized();
};

//----------------------------------------------------------------------------------------
// Mesh Collision
//----------------------------------------------------------------------------------------
size_t MeshCollision::TYPE_ID = Collision::MESH;
MeshCollision::MeshCollision(Geometry* collider, const pxr::SdfPath& path, 
  float restitution, float friction)
  : Collision(collider, path, restitution, friction)
{
  Mesh* mesh = (Mesh*)_collider;
  _CreateAccelerationStructure();
}

void MeshCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateAccelerationStructure();
  _UpdateParameters(prim, time);
}

void MeshCollision::_CreateAccelerationStructure()
{
  //_bvh = new BVH({_collider});
  //_bvh.Init({_collider});

  /*
  std::cout << "   build boundary volume hierarchy : " << ((CurrentTime() - T) * 1e-9) << std::endl;

  T = CurrentTime();
  for (auto& ray : rays) {
    double minDistance = DBL_MAX;
    Location hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
      result.push_back(hit.GetPosition(colliders[hit.GetGeometryIndex()]));
    }
  }
  std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
  _SetupBVHInstancer(stage, &bvh);
  */

} 

void MeshCollision::_UpdateAccelerationStructure()
{

} 


void MeshCollision::_FindContact(size_t index, Particles* particles, float ft)
{
  /*
  if (!Affects(index))return;
  const float radius = _radius + particles->_radius[index];
  const pxr::GfVec3f predicted(particles->_position[index] + particles->_velocity[index] * ft);
  SetHit(index, (predicted - _center).GetLength() < radius);
  */
}

void MeshCollision::_StoreContactLocation(Particles* particles, int index, 
  const Body* body, Location& location, float ft)
{
  /*
  const pxr::GfVec3f velocity = particles->_velocity[index] * ft;
  const pxr::GfVec3f predicted(particles->_position[index] + velocity);
  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if(nL>0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(0.f,0.f,0.f);

  const pxr::GfVec3f intersection = _center + normal * (_radius + particles->_radius[index]);

  const pxr::GfVec3f vPlane(0.f, 0.f, 0.f); // plane velocity
  const pxr::GfVec3f vRel = particles->_velocity[index] - vPlane;
  const float vn = pxr::GfDot(vRel, normal);

  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);
  */
}

float MeshCollision::GetValue(Particles* particles, size_t index)
{
  return 0.f;
  /*return (particles->_predicted[index] - _center).GetLength() -
    _radius - particles->_radius[index];*/
}
  
pxr::GfVec3f MeshCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);// return (particles->_predicted[index] - _center).GetNormalized();
}

const pxr::GfVec3f MeshCollision::GetContactNormal(size_t index) const 
{
  return (GetContactPosition(index)).GetNormalized();
};


JVR_NAMESPACE_CLOSE_SCOPE