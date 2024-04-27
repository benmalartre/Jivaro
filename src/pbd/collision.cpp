#include <pxr/base/work/loops.h>

#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../pbd/utils.h"
#include "../pbd/tokens.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

const float Collision::TOLERANCE_MARGIN = 0.02f;

//
// Hits  (parallel First Pass)
//
size_t _NumHits(unsigned int num)
{
  static const int numToBits[16] = 
    { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

  if (0 == num)return numToBits[0];

  int nibble = num & 0xf;
  return numToBits[nibble] + _NumHits(num >> 4);
}

size_t Collision::GetNumHits()
{
  size_t numHits=0;
  for(size_t i=0; i< _hits.size(); ++i) {
    numHits += _NumHits(_hits[i]);
  }
  return numHits;
}

// 
// Contacts
//
void Collision::UpdateContacts(Particles* particles)
{
  size_t idx = 0;
  for (auto& contact : _contacts) {

    contact->Update(this, particles, _c2p[idx++]);
  }
}

void Collision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& constraints, float ft)
{
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft));
  _BuildContacts(particles, bodies, constraints, ft);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, float ft)
{
  for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
    Contact* contact = new Contact();
    _StoreContactLocation(particles, elements[elemIdx], contact, ft);
    _contacts.push_back(contact);
  }
}

void Collision::_ResetContacts(Particles* particles)
{
  const size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));
  _contacts.clear();
  _contacts.reserve(numParticles);
  _p2c.resize(numParticles, -1);
  _c2p.clear();
  _c2p.reserve(numParticles);

}

void Collision::_BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& constraints, float ft)
{
  CollisionConstraint* constraint = NULL;
  size_t numParticles = particles->GetNumParticles();
  size_t numBodies = bodies.size();

  pxr::VtArray<int> elements;
  int bodyIdx = -1;

  Mask::Iterator iterator(this, 0, numParticles);
  size_t particleToContactIdx = 0;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    if (CheckHit(index)) {
      _p2c[index] = particleToContactIdx++;
      _c2p.push_back(index);
      if (particles->GetBody(index) != bodyIdx || elements.size() >= Constraint::BlockSize) {
        if (elements.size()) {
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          
          StoreContactsLocation(particles, & elements[0], elements.size(), ft);
          constraints.push_back(constraint);
          elements.clear();
        }
        bodyIdx = particles->GetBody(index);
      } 
      elements.push_back(index - bodies[particles->GetBody(index)]->GetOffset());
    }
  } 
  
  if (elements.size()) {
    constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
    StoreContactsLocation(particles, & elements[0], elements.size(), ft);
    constraints.push_back(constraint);
  }

}

void Collision::_FindContacts(size_t begin, size_t end, Particles* particles, float ft)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    _FindContact(particles, index, ft);
  }
}


// 
// Init
//
void Collision::Init(size_t numParticles) 
{
}


// 
// Update
//
void Collision::Update(const pxr::UsdPrim& prim, double time){}

void Collision::_UpdateParameters( const pxr::UsdPrim& prim, double time)
{

  prim.GetAttribute(PBDTokens->restitution).Get(&_restitution, time);
  prim.GetAttribute(PBDTokens->friction).Get(&_friction, time);
}


// Velocity
pxr::GfVec3f Collision::GetVelocity(Particles* particles, size_t index)
{

  const pxr::GfVec3f torque = _collider->GetTorque();
  const pxr::GfVec3f tangent =
    (GetGradient(particles, index) ^ torque).GetNormalized();

  return _collider->GetVelocity() + tangent * torque.GetLength();
};

void Collision::SolveVelocities(size_t begin, size_t end, Particles* particles, float dt)
{
  for (size_t elemIdx = begin; elemIdx < end; ++elemIdx) {
    _SolveVelocity(particles, _c2p[elemIdx], dt);
  }
}

void Collision::_SolveVelocity(Particles* particles, size_t index, float dt)
{
  if(!CheckHit(index))return;    

  const  pxr::GfVec3f normal = GetContactNormal(index);

  // Relative normal and tangential velocities
  const pxr::GfVec3f velocity = 
   (particles->GetVelocity(index) - GetContactVelocity(index));
  const float vn = pxr::GfDot(velocity, normal);

  const pxr::GfVec3f vt = velocity - normal * vn;
  const float vtLen = vt.GetLength();

  // Friction
  if (vtLen > 0.000001) {
    const float friction = pxr::GfMin(dt * _friction, vtLen);
    particles->GetVelocity(index) -= vt.GetNormalized() * friction;
  }

  // Restitution
  const float threshold = 2.f * 9.81 * dt;
  const float e = pxr::GfAbs(vn) <= threshold ? 0.0 : _restitution;
  const float vnTilde = GetContactSpeed(index);
  const float restitution = -vn + pxr::GfMax(-e * vnTilde, 0.f);
  particles->GetVelocity(index) += normal * restitution;

}

pxr::GfVec3f Collision::GetContactPosition(size_t index) const 
{
  return _contacts[_p2c[index]]->GetCoordinates();
}

pxr::GfVec3f Collision::GetContactNormal(size_t index) const 
{
  return _contacts[_p2c[index]]->GetNormal();
}

pxr::GfVec3f Collision::GetContactVelocity(size_t index) const 
{
  return _contacts[_p2c[index]]->GetVelocity();
}

float Collision::GetContactSpeed(size_t index) const 
{
  return _contacts[_p2c[index]]->GetSpeed();
}

float Collision::GetContactDepth(size_t index) const
{
  return _contacts[_p2c[index]]->GetDepth();
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
  return pxr::GfDot(_normal, particles->GetPredicted(index) - _position) -
    particles->GetRadius(index);
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
  _position =  plane->GetOrigin();
  _normal = plane->GetNormal();
}


void PlaneCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);
  float d = pxr::GfDot(_normal, predicted - _position)  - (particles->GetRadius(index) + TOLERANCE_MARGIN);
  SetHit(index, d < 0.f);
  
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->GetPosition(index) + particles->GetVelocity(index) * ft;
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->GetRadius(index);
  const pxr::GfVec3f intersection = predicted + _normal * -d;

  contact->Init(this, particles, index, Location::AWAITING_INDEX);
  contact->SetCoordinates(intersection);
  contact->SetT(d);

  particles->GetColor(index) = pxr::GfVec3f((1.f - d), d, 0.5f);
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


void SphereCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  const float radius = _radius + particles->GetRadius(index) + TOLERANCE_MARGIN;
  const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);
  SetHit(index, (predicted - _center).GetLength() < radius);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);

  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if (nL > 0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1).GetNormalized();

  const float d = nL - (_radius - particles->GetRadius(index));

  const pxr::GfVec3f intersection = predicted  + normal * -d;

  contact->Init(this, particles, index, Location::AWAITING_INDEX);
  contact->SetCoordinates(intersection);
  contact->SetT(d);

  particles->GetColor(index) = pxr::GfVec3f(0.5f,1.f,0.7f);
}

float SphereCollision::GetValue(Particles* particles, size_t index)
{
  return (particles->GetPredicted(index) - _center).GetLength() -
    (_radius + particles->GetRadius(index));
}
  
pxr::GfVec3f SphereCollision::GetGradient(Particles* particles, size_t index)
{
  return (particles->GetPredicted(index) - _center).GetNormalized();
}




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


void MeshCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  pxr::GfRay ray(particles->GetVelocity(index), particles->GetPosition(index));
  Mesh* mesh = (Mesh*)_collider;
  Location hit;
  //mesh->Raycast(ray, &hit);
  //SetHit(index, false);
  //const float radius = particles->GetRadius(index);
  //const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);
  //SetHit(index, (predicted - _center).GetLength() < radius);
  SetHit(index, false);
}

void MeshCollision::_StoreContactLocation(Particles* particles, int index, Contact* location, float ft)
{
  /*
  const pxr::GfVec3f velocity = particles->GetVelocity(index) * ft;
  const pxr::GfVec3f predicted(particles->GetPosition(index) + velocity);
  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if(nL>0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(0.f,0.f,0.f);

  const pxr::GfVec3f intersection = _center + normal * (_radius + particles->GetRadius(index));

  const pxr::GfVec3f vPlane(0.f, 0.f, 0.f); // plane velocity
  const pxr::GfVec3f vRel = particles->GetVelocity(index) - vPlane;
  const float vn = pxr::GfDot(vRel, normal);

  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);
  */
}

float MeshCollision::GetValue(Particles* particles, size_t index)
{
  return 0.f;
  /*return (particles->GetPredicted(index) - _center).GetLength() -
    _radius - particles->GetRadius(index);*/
}
  
pxr::GfVec3f MeshCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);// return (particles->GetPredicted(index) - _center).GetNormalized();
}


//----------------------------------------------------------------------------------------
// Self Collision
//----------------------------------------------------------------------------------------
size_t SelfCollision::TYPE_ID = Collision::SELF;

SelfCollision::SelfCollision(Particles* particles, const pxr::SdfPath& path, 
  float restitution, float friction, float radius)
  : Collision(NULL, path, restitution, friction)
  , _radius(radius)
  , _particles(particles)
  , _grid(NULL)
{
  
  _UpdateAccelerationStructure();
}

SelfCollision::~SelfCollision()
{
  
}

void SelfCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateAccelerationStructure();
}

size_t SelfCollision::GetNumContacts(size_t index)
{
  return _counts[index];
}

 size_t SelfCollision::GetTotalNumContacts()
 {
    return _contacts.size();
 }

Contact* SelfCollision::GetContacts(size_t index)
{
  return _contacts[_offsets[index]];
}

// 
// Contacts
//
void SelfCollision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& constraints, float ft)
{
  _ResetContacts(particles);

  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&SelfCollision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft));
      
  _BuildContacts(particles, bodies, constraints, ft);
}


void SelfCollision::UpdateContacts(Particles* particles)
{
  /*
  size_t idx = 0;
  for (auto& contact : _contacts) {

    contact->Update(this, particles, _c2p[idx++]);
  }
  */
}

void SelfCollision::SolveVelocities(size_t begin, size_t end, Particles* particles, float dt)
{
  /*
  for (size_t elemIdx = begin; elemIdx < end; ++elemIdx) {
    _SolveVelocity(particles, _c2p[elemIdx], dt);
  }
  */
}

void SelfCollision::_ResetContacts(Particles* particles)
{
  size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));

  _contacts.clear();
  _contacts.reserve(numParticles*MAX_COLLIDE_PARTICLES);
  _p2c.resize(numParticles, -1);
  _c2p.clear();
  _c2p.reserve(numParticles*MAX_COLLIDE_PARTICLES);

  _datas.resize(numParticles);
  for (_Contacts& contacts : _datas)
    contacts.clear();

}

void SelfCollision::_FindContacts(size_t begin, size_t end, Particles* particles, float ft)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    _FindContact(particles, index, ft);
  }
}

void SelfCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);

  std::vector<int> closests;
  _grid.Closests(index, particles->GetPositionCPtr(), closests);

  bool intersect = false;
  for(int closest: closests) {
    if ((predicted - particles->GetPosition(closest)).GetLength() <
      (particles->GetRadius(index) + particles->GetRadius(closest) + Collision::TOLERANCE_MARGIN)) {
        Contact* contact = new Contact();
        _StoreContactLocation(particles, index, closest, contact, ft);
        _datas[index].push_back(contact);
        intersect = true;
    }
  }
  SetHit(index, false);
}


void SelfCollision::_StoreContactLocation(Particles* particles, int index, int other, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted(particles->GetPosition(index) + particles->GetVelocity(index) * ft);

  pxr::GfVec3f normal = predicted - particles->GetPosition(other);
  const float nL = normal.GetLength();
  if (nL > 0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1).GetNormalized();

  const float d = nL - (particles->GetRadius(other) - particles->GetRadius(index));

  const pxr::GfVec3f intersection = predicted  + normal * -d;

  contact->Init(this, particles, other, other);
  contact->SetCoordinates(intersection);
  contact->SetT(d);

  particles->GetColor(index) = pxr::GfVec3f(0.5f,1.f,0.7f);
}


void SelfCollision::_BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& constraints, float ft)
{

std::cout << "build contact start" << std::endl;
  CollisionConstraint* constraint = NULL;
  size_t numParticles = particles->GetNumParticles();

  std::cout << "compute num contacts" << std::endl;
  size_t numContacts = 0;
  for (SelfCollision::_Contacts& contacts : _datas)
    numContacts += contacts.size();
    std::cout << numContacts << std::endl;
  _contacts.clear();
  _contacts.reserve(numContacts);
  std::cout << "num contacts " << numContacts << std::endl;


  pxr::VtArray<int> elements;

  _offsets.clear();
  _counts.clear();

  _offsets.resize(numParticles, 0);
  _counts.resize(numParticles, 0);
 
  size_t contactsOffset = 0;

  Mask::Iterator iterator(this, 0, numParticles);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    _counts[index] = _datas[index].size();
    _offsets[index] = contactsOffset;
    if(_counts[index]) {
      for(Contact* contact: _datas[index])
        _contacts.push_back(contact);

      contactsOffset += _counts[index];
      elements.push_back(index);
    } 
    
    if ((elements.size() >= Constraint::BlockSize) || iterator.End()) {
      if (elements.size()) {
        constraint = new CollisionConstraint(particles, this, elements);
        constraints.push_back(constraint);
        elements.clear();
      } 
    }
  }

  std::cout << "build contact done" << std::endl;
}

void SelfCollision::_UpdateAccelerationStructure()
{  
  size_t numParticles = _particles->GetNumParticles();
  _grid.Init(numParticles,_particles->GetPositionCPtr() , 2.f * _radius);
} 


float SelfCollision::GetValue(Particles* particles, size_t index)
{

  return 0.f;
 
}
  
pxr::GfVec3f SelfCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);
}

// Velocity
pxr::GfVec3f SelfCollision::GetVelocity(Particles* particles, size_t index)
{
   return pxr::GfVec3f(0.f);
};


JVR_NAMESPACE_CLOSE_SCOPE