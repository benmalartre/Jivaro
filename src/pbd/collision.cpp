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

const float Collision::TOLERANCE_MARGIN = 0.05f;

// 
// Contacts
//
void Collision::UpdateContacts(Particles* particles)
{
  size_t idx = 0;

  for (size_t c = 0; c < particles->GetNumParticles(); ++c)
    if(_contacts.IsUsed(c))
      _contacts.Get(c)->Update(this, particles, _c2p[idx++]);

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
    const size_t index = elements[elemIdx];
    _StoreContactLocation(particles, index, _contacts.Use(index), ft);
  }
}

void Collision::_ResetContacts(Particles* particles)
{
  const size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));

  _c2p.clear();
  _c2p.reserve(numParticles);

  _contacts.Resize(numParticles, 1);
  _contacts.ResetAllUsed();

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
      _c2p.push_back(index);
      if (particles->body[index] != bodyIdx || elements.size() >= Constraint::BlockSize) {
        if (elements.size()) {
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          
          StoreContactsLocation(particles, & elements[0], elements.size(), ft);
          constraints.push_back(constraint);
          elements.clear();
        }
        bodyIdx = particles->body[index];
      } 
      elements.push_back(index);
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


void Collision::SolveVelocities(Particles* particles, float dt)
{
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_SolveVelocities, this,
      std::placeholders::_1, std::placeholders::_2, particles, dt), 32);
}

void Collision::_SolveVelocities(size_t begin, size_t end, Particles* particles, float dt)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    if(!CheckHit(index))continue;

    /*
    const  pxr::GfVec3f normal = GetContactNormal(index);

    // Relative normal and tangential velocities
    const pxr::GfVec3f velocity = particles->velocity[index] - GetContactVelocity(index);
    const float vn = pxr::GfDot(velocity, normal);
    const float vnn = pxr::GfDot(pxr::GfVec3f(0.f, 1.f, 0.f), normal);

    const pxr::GfVec3f vt = velocity - normal * vn;
    const float vtLen = vt.GetLength();

    // Friction
    if (vn < 0.f) {
      particles->velocity[index] -= vt * vnn  * _friction;
    }

    // Restitution
    const float threshold = 2.f * 9.81 * dt;
    const float e = pxr::GfAbs(vn) <= threshold ? 0.0 : _restitution;
    const float vnTilde = GetContactSpeed(index);
    const float restitution = -vn + pxr::GfMax(-_restitution   * vnTilde, 0.f);
    particles->velocity[index] += normal * restitution;
    */

    particles->velocity[index] = pxr::GfVec3f(0.f);
    particles->position[index] = GetContactPosition(index);
    particles->predicted[index] = GetContactPosition(index);
    
  }
}

size_t Collision::GetContactHit(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->GetHit();
}

size_t Collision::GetContactComponent(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->GetComponentIndex();
}

pxr::GfVec3f Collision::GetContactPosition(size_t index, size_t c) const 
{
  return pxr::GfVec3f(_contacts.Get(index, c)->GetCoordinates());
}

pxr::GfVec3f Collision::GetContactNormal(size_t index,size_t c) const 
{
  return _contacts.Get(index, c)->GetNormal();
}

pxr::GfVec3f Collision::GetContactVelocity(size_t index, size_t c) const 
{
  return _contacts.Get(index, c)->GetVelocity();
}

float Collision::GetContactSpeed(size_t index, size_t c) const 
{
  return _contacts.Get(index, c)->GetSpeed();
}

float Collision::GetContactDepth(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->GetDepth();
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
  return pxr::GfDot(_normal, particles->predicted[index] - _position) -
    particles->radius[index];
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
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  float d = pxr::GfDot(_normal, predicted - _position)  - (particles->radius[index] + TOLERANCE_MARGIN);
  SetHit(index, d < 0.f);
  
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->position[index] + particles->velocity[index] * ft;
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->radius[index];
  const pxr::GfVec3f intersection = predicted + _normal * -d;

  contact->Init(this, particles, index);
  contact->SetCoordinates(intersection);
  contact->SetT(d);

  //particles->color[index] = pxr::GfVec3f(1.f, 1.f, 0.f);
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
  const float radius = _radius + particles->radius[index] + TOLERANCE_MARGIN;
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  SetHit(index, (predicted - _center).GetLength() < radius);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);

  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if (nL > 0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1).GetNormalized();

  const float d = nL - (_radius + particles->radius[index]);

  const pxr::GfVec3f intersection = predicted  + normal * -d;

  contact->Init(this, particles, index);
  contact->SetCoordinates(intersection);
  contact->SetT(d);

  //particles->color[index] = pxr::GfVec3f(0.5f,1.f,0.7f);
}

float SphereCollision::GetValue(Particles* particles, size_t index)
{
  return (particles->predicted[index] - _center).GetLength() -
    (_radius + particles->radius[index]);
}
  
pxr::GfVec3f SphereCollision::GetGradient(Particles* particles, size_t index)
{
  return (particles->predicted[index] - _center).GetNormalized();
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
  pxr::GfRay ray(particles->velocity[index], particles->position[index]);
  Mesh* mesh = (Mesh*)_collider;
  Location hit;
  //mesh->Raycast(ray, &hit);
  //SetHit(index, false);
  //const float radius = particles->radius[index];
  //const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  //SetHit(index, (predicted - _center).GetLength() < radius);
  SetHit(index, false);
}

void MeshCollision::_StoreContactLocation(Particles* particles, int index, Contact* location, float ft)
{
  /*
  const pxr::GfVec3f velocity = particles->velocity[index] * ft;
  const pxr::GfVec3f predicted(particles->position[index] + velocity);
  pxr::GfVec3f normal = predicted - _center;
  const float nL = normal.GetLength();
  if(nL>0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(0.f,0.f,0.f);

  const pxr::GfVec3f intersection = _center + normal * (_radius + particles->radius[index]);

  const pxr::GfVec3f vPlane(0.f, 0.f, 0.f); // plane velocity
  const pxr::GfVec3f vRel = particles->velocity[index] - vPlane;
  const float vn = pxr::GfDot(vRel, normal);

  const pxr::GfVec4f coords(intersection[0], intersection[1], intersection[2], vn);
  location.SetCoordinates(coords);
  */
}

float MeshCollision::GetValue(Particles* particles, size_t index)
{
  return 0.f;
  /*return (particles->predicted[index] - _center).GetLength() -
    _radius - particles->radius[index];*/
}
  
pxr::GfVec3f MeshCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);// return (particles->predicted[index] - _center).GetNormalized();
}


//----------------------------------------------------------------------------------------
// Self Collision
//----------------------------------------------------------------------------------------
size_t SelfCollision::TYPE_ID = Collision::SELF;

SelfCollision::SelfCollision(Particles* particles, const pxr::SdfPath& path, 
  float restitution, float friction, float thickness)
  : Collision(NULL, path, restitution, friction)
  , _particles(particles)
  , _grid(NULL)
  , _thickness(thickness)
{
  _grid.Init(_particles->GetNumParticles(), &_particles->position[0], _particles->radius[0]*0.5);
}

SelfCollision::~SelfCollision()
{
  
}

void SelfCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateAccelerationStructure();
}

// 
// Contacts
//
size_t SelfCollision::GetNumContacts(size_t index) const
{
  return _contacts.GetNumUsed(index);
}

 size_t SelfCollision::GetTotalNumContacts() const
 {
    return _contacts.GetTotalNumUsed();
 }

const Contact* SelfCollision::GetContacts(size_t index) const
{
  return _contacts.Get(index);
}

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
  size_t idx = 0;

  for (size_t c = 0; c < particles->GetNumParticles(); ++c)
    if (_contacts.IsUsed(c))
      for (size_t d = 0; d < _contacts.GetNumUsed(c); ++d) 
        _contacts.Get(c, d)->Update(this, particles, _c2p[idx++], _contacts.Get(c, d)->GetComponentIndex());

}


void SelfCollision::_ResetContacts(Particles* particles)
{
  size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));

  _c2p.clear();
  _c2p.reserve(numParticles * PARTICLE_MAX_CONTACTS);

  _contacts.Resize(numParticles, PARTICLE_MAX_CONTACTS);


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
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  std::vector<int> closests;

  size_t numCollide = 0;
  _grid.Closests(index, &particles->position[0], closests);

  for(int closest: closests) {
    if(numCollide >= PARTICLE_MAX_CONTACTS)break;

    const pxr::GfVec3f otherPredicted(particles->position[closest] + 
      particles->velocity[closest] * ft);

    if ((predicted - otherPredicted).GetLength() < 
      (particles->radius[index] + particles->radius[closest] + _thickness)) {
      Contact* contact = _contacts.Use(index);
      _StoreContactLocation(particles, index, closest, contact, ft);
      contact->SetComponentIndex(closest);
      numCollide++;
    }
  }
  SetHit(index, (numCollide > 0));
}


void SelfCollision::_StoreContactLocation(Particles* particles, int index, int other, 
  Contact* contact, float ft)
{
  const pxr::GfVec3f predicted(particles->position[index] + 
    particles->velocity[index] * ft);
  const pxr::GfVec3f otherPredicted(particles->position[other] + 
    particles->velocity[other] * ft);

  pxr::GfVec3f normal = predicted - otherPredicted;
  float nL = normal.GetLength();
  

  const float d = nL - (particles->radius[index] + particles->radius[other] + _thickness);

  const pxr::GfVec3f intersection = predicted  + normal * -d * 0.5;

  contact->Init(this, particles, index, other);
  contact->SetCoordinates(intersection);
  contact->SetT(-d);

  //particles->color[index] = _grid.GetColor(particles->position[index]);
  //particles->color[other] = _grid.GetColor(particles->position[index]);
}


void SelfCollision::_BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& constraints, float ft)
{
  SelfCollisionConstraint* constraint = NULL;
  size_t numParticles = particles->GetNumParticles();

  size_t numContacts = _contacts.GetTotalNumUsed();

  pxr::VtArray<int> elements;
 
  size_t contactsOffset = 0;
  size_t contactIdx = 0;
  Mask::Iterator iterator(this, 0, numParticles);
  size_t index = iterator.Begin();
  for (; index != Mask::INVALID_INDEX; index = iterator.Next()) {
    size_t numUsed = _contacts.GetNumUsed(index);
    if(numUsed) {
      for(size_t c = 0; c < numUsed; ++c) {
        _c2p.push_back(index); 

        size_t other = GetContactComponent(index, c);
        elements.push_back(index);
        elements.push_back(other);
      };
    } 
    
    if ((elements.size() >= Constraint::BlockSize) || iterator.End()) {
      if (elements.size()) {
        constraint = new SelfCollisionConstraint(particles, this, elements);
        constraints.push_back(constraint);
        elements.clear();
      } 
    }
  }
}

void SelfCollision::_UpdateAccelerationStructure()
{  
  size_t numParticles = _particles->GetNumParticles();
  _grid.Update(&_particles->position[0]);
} 

float SelfCollision::GetValue(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[other] - particles->predicted[index]).GetLength() - 
    (particles->radius[index] + particles->radius[other] + _thickness);
}
  
pxr::GfVec3f SelfCollision::GetGradient(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[other] - particles->predicted[index]).GetNormalized();
}

// Velocity
pxr::GfVec3f SelfCollision::GetVelocity(Particles* particles, size_t index, size_t other)
{
  return particles->velocity[other];
};

void SelfCollision::SolveVelocities(Particles* particles, float dt)
{
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&SelfCollision::_SolveVelocities, this,
      std::placeholders::_1, std::placeholders::_2, particles, dt), 32);
}


void SelfCollision::_SolveVelocities(size_t begin, size_t end, Particles* particles, float dt)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {

    if(!CheckHit(index))continue;

    pxr::GfVec3f correction;
    float numHits = 0;
    for(size_t c = 0; c < GetNumContacts(index); ++c) {
      if(!GetContactHit(index, c))continue;
      numHits+=1.f;
      size_t other = GetContactComponent(index, c);
      const pxr::GfVec3f normal = GetContactNormal(index, c);
      const pxr::GfVec3f velocity = 
        (particles->velocity[index] + particles->velocity[other]) * 0.5f;

      const pxr::GfVec3f correction0 = velocity - particles->velocity[index];
      const pxr::GfVec3f correction1 = velocity - particles->velocity[other];

      particles->velocity[index] += correction0;
      particles->velocity[other] += correction1;

    }


  }
  
}



JVR_NAMESPACE_CLOSE_SCOPE