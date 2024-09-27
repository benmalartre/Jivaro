#include <pxr/base/work/loops.h>

#include "../utils/color.h"
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
#include "../pbd/solver.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

const float Collision::TOLERANCE_MARGIN = .1f;
const size_t Collision::PACKET_SIZE = 64;


void Collision::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  const float r = 1.f;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_contacts.IsUsed(index)) {
      Contact* contact = _contacts.Get(index);
      positions.push_back(pxr::GfVec3f(contact->GetCoordinates()));
      colors.push_back(color);
      radius.push_back(r);
    }
}

// 
// Contacts
//
void Collision::UpdateContacts(Particles* particles)
{
  pxr::WorkParallelForN(
      particles->GetNumParticles(),
      std::bind(&Collision::_UpdateContacts, this,
        std::placeholders::_1, std::placeholders::_2, particles), PACKET_SIZE);
}

void Collision::_UpdateContacts(size_t begin, size_t end, Particles* particles)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_contacts.IsUsed(index))
      _contacts.Get(index)->Update(
        GetGradient(particles, index),
        GetVelocity(particles, index),
        GetValue(particles, index)
      );
}

void Collision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& constraints, float ft)
{
  Init(particles->GetNumParticles());
  _ResetContacts(particles);
  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft), PACKET_SIZE);
  _BuildContacts(particles, bodies, constraints, ft);
}

void Collision::StoreContactsLocation(Particles* particles, int* elements, size_t n, float ft, bool solveInitialPenetration)
{
  if(solveInitialPenetration)
    for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
      const size_t index = elements[elemIdx];
      _StoreContactLocation(particles, index, _contacts.Use(index), ft);
      _ResolveInitialPenetration(particles, index);
    }
  
  else
    for (size_t elemIdx = 0; elemIdx < n; ++elemIdx) {
      const size_t index = elements[elemIdx];
      _StoreContactLocation(particles, index, _contacts.Use(index), ft);
    }
}

void Collision::_ResolveInitialPenetration(Particles* particles, size_t index)
{
  const float d = GetContactInitDepth(index); 
  if(d > 0) return;
  const pxr::GfVec3f offset = GetContactNormal(index) * -d;

  particles->position[index] += offset;
  particles->predicted[index] += offset;
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
  size_t bodyIdx = INVALID_INDEX;

  Mask::Iterator iterator(this, 0, numParticles);
  size_t particleToContactIdx = 0;
  size_t numHits = 0;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    if (CheckHit(index)) {
      numHits++;
      _c2p.push_back(index);
      if (particles->body[index] != bodyIdx || elements.size() >= Constraint::BlockSize) {
        if (elements.size()) {
          constraint = new CollisionConstraint(bodies[bodyIdx], this, elements);
          StoreContactsLocation(particles, & elements[0], elements.size(), ft, true);
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
    StoreContactsLocation(particles, & elements[0], elements.size(), ft, true);
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

float Collision::GetContactDepth(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->GetDepth();
}

float Collision::GetContactInitDepth(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->GetInitDepth();
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
  float d = pxr::GfDot(predicted - _position, _normal)  - particles->radius[index];
  pxr::GfVec3f intersection = predicted + _normal * -d;

  contact->Init(_normal,GetVelocity(particles, index), d);
  contact->SetCoordinates(intersection);
  contact->SetDistance(d);

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
  float nL = normal.GetLength();
  if (nL > 0.0000001f)normal.Normalize();
  else normal = pxr::GfVec3f(0.f, 1.f, 0.f);

  float d = nL - (_radius + particles->radius[index]);

  pxr::GfVec3f intersection = predicted  + normal * -d;

  contact->Init(normal,GetVelocity(particles, index), d);
  contact->SetCoordinates(intersection);
  contact->SetDistance(d);
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
  _UpdateParameters(prim, time);
  _UpdateAccelerationStructure();
}

void MeshCollision::Init(size_t numParticles)
{
  _query.resize(numParticles, Location());
  _closest.resize(numParticles, ClosestPoint());
}

void MeshCollision::_CreateAccelerationStructure()
{
  uint64_t T = CurrentTime();
  _bvh = new BVH();
  _bvh->Init({_collider});
} 

void MeshCollision::_UpdateAccelerationStructure()
{
  _bvh->Update();
} 


void MeshCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  pxr::GfRay ray(particles->velocity[index], particles->position[index]);
  Mesh* mesh = (Mesh*)_collider;
  double maxDistance = particles->velocity[index].GetLength() * ft + particles->radius[index];
  double minDistance = DBL_MAX;
  _bvh->Closest(particles->position[index], &_closest[index], DBL_MAX);
    ;
  /*
    SetHit(index, true);
  else
    SetHit(index, false);
  */
 
  
  
  
  
  //mesh->Raycast(ray, &hit);
  //SetHit(index, false);
  //const float radius = particles->radius[index];
  //const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  //SetHit(index, (predicted - _center).GetLength() < radius);
  
}

void MeshCollision::_StoreContactLocation(Particles* particles, int index, Contact* location, float ft)
{
  const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);
  /*
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

float 
MeshCollision::GetValue(Particles* particles, size_t index)
{
  return 0.f;
  /*return (particles->predicted[index] - _center).GetLength() -
    _radius - particles->radius[index];*/
}
  
pxr::GfVec3f 
MeshCollision::GetGradient(Particles* particles, size_t index)
{
  return pxr::GfVec3f(0.f);// return (particles->predicted[index] - _center).GetNormalized();
}

void 
MeshCollision::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  const float r = .1f;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_closest[index].IsValid()) {
      positions.push_back(pxr::GfVec3f(_closest[index].GetPoint()));
      colors.push_back(color);
      radius.push_back(r);
    }
}


//----------------------------------------------------------------------------------------
// Self Collision
//----------------------------------------------------------------------------------------
size_t SelfCollision::TYPE_ID = Collision::SELF;

SelfCollision::SelfCollision(Particles* particles, const pxr::SdfPath& path, 
  float restitution, float friction)
  : Collision(NULL, path, restitution, friction)
  , _particles(particles)
  , _grid(NULL)
  , _neighborsInitialized(false)
{
  _grid.Init(_particles->GetNumParticles(), &_particles->predicted[0], _particles->radius[0] * 2.f);
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

void SelfCollision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& constraints, float ft)
{
  if(!_neighborsInitialized)_ComputeNeighbors(bodies);

  _ResetContacts(particles);

  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&SelfCollision::_FindContacts, this,
      std::placeholders::_1, std::placeholders::_2, particles, ft), PACKET_SIZE);

  _BuildContacts(particles, bodies, constraints, ft);

}

void SelfCollision::UpdateContacts(Particles* particles)
{
  pxr::WorkParallelForN(
      particles->GetNumParticles(),
      std::bind(&SelfCollision::_UpdateContacts, this,
        std::placeholders::_1, std::placeholders::_2, particles), PACKET_SIZE);
}

void SelfCollision::_UpdateContacts(size_t begin, size_t end, Particles* particles)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    pxr::GfVec3f color = RandomColorByIndex(index);
    if (_contacts.IsUsed(index))
      for (size_t c = 0; c < _contacts.GetNumUsed(index); ++c) {
        size_t other = _contacts.Get(index, c)->GetComponentIndex();

        _contacts.Get(index, c)->Update(
          GetGradient(particles, index, other), 
          GetVelocity(particles, index, other), 
          GetValue(particles, index, other)
         );
        
        if(index % 32 == 0) {
          particles->color[index] = color;
          particles->color[other] = color;
        }
        //else particles->color[index] = pxr::GfVec3f(0.5f+RANDOM_LO_HI(-0.05f, 0.05f));
        
      }
  } 
}

void SelfCollision::_ResetContacts(Particles* particles)
{
  size_t numParticles = particles->GetNumParticles();
  _hits.resize(numParticles / sizeof(int) + 1);
  memset(&_hits[0], 0, _hits.size() * sizeof(int));

  _c2p.clear();
  _c2p.reserve(numParticles * PARTICLE_MAX_CONTACTS);

  _contacts.Resize(numParticles, PARTICLE_MAX_CONTACTS);
  _contacts.ResetAllUsed();

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
  std::vector<int> closests;


  size_t numCollide = 0;
  _grid.Closests(index, &particles->predicted[0], closests, 4.f * particles->radius[index] + Collision::TOLERANCE_MARGIN);

  for(int closest: closests) {
    if(_AreConnected(index, closest))continue;
    if(numCollide >= PARTICLE_MAX_CONTACTS)break;

    pxr::GfVec3f ip(particles->position[index] + particles->velocity[index] * ft);
    pxr::GfVec3f cp(particles->position[closest] + particles->velocity[closest] * ft);
    if((ip - cp).GetLength() < 2.f * (particles->radius[index] + particles->radius[closest] + Collision::TOLERANCE_MARGIN)) {
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
  const pxr::GfVec3f ip(particles->position[index] + particles->velocity[index] * ft);
  const pxr::GfVec3f op(particles->position[other] + particles->velocity[other] * ft);
  pxr::GfVec3f normal = ip - op;
  float nL = normal.GetLength();
  
  float d = nL - (particles->radius[index] + particles->radius[other]);

  contact->Init(normal.GetNormalized(), GetVelocity(particles, index, other), d);
  contact->SetDistance(d);

}

void SelfCollision::_BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
  std::vector<Constraint*>& constraints, float ft)
{

  CollisionConstraint* constraint = NULL;
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
}

void SelfCollision::_UpdateAccelerationStructure()
{  
  _grid.Update(&_particles->predicted[0]);
} 

float SelfCollision::GetValue(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[index] - particles->predicted[other]).GetLength() - 
    (particles->radius[index] + particles->radius[other]);
}
  
pxr::GfVec3f SelfCollision::GetGradient(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[index] - particles->predicted[other]).GetNormalized();
}

// Velocity
pxr::GfVec3f SelfCollision::GetVelocity(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[index] - particles->position[index]) +
    (particles->predicted[other] - particles->position[other]) * .5f;
}


void 
SelfCollision::_ComputeNeighbors(const std::vector<Body*> &bodies)
{
  size_t numParticles = _particles->GetNumParticles();
  _neighborsCounts.resize(numParticles, 0);
  _neighborsOffsets.resize(numParticles, 0);
  _neighbors.clear();

  size_t neighborsOffset = 0;
  for(size_t b = 0; b < bodies.size(); ++b) {
    const Geometry* geometry = bodies[b]->GetGeometry();
    size_t offset = bodies[b]->GetOffset();
    switch (geometry->GetType()) {
      case Geometry::MESH:
      {
        Mesh* mesh = (Mesh*)geometry;
        size_t numPoints = mesh->GetNumPoints();
        for (size_t p = 0; p < numPoints; ++p) {
          size_t numNeighbors = mesh->GetNumNeighbors(p);
          for (size_t n = 0; n < numNeighbors; ++n)
            _neighbors.push_back(offset + mesh->GetNeighbor(p, n));
       
          _neighborsCounts[offset + p] = numNeighbors;
          _neighborsOffsets[offset + p] = neighborsOffset;
          neighborsOffset += numNeighbors;
        }
        break;
      }
    }
  }

  _neighborsInitialized = true;
}


bool 
SelfCollision::_AreConnected(size_t index, size_t other)
{
  int* neighbors = &_neighbors[_neighborsOffsets[index]];
  for(size_t n = 0; n < _neighborsCounts[index]; ++n) {
    if(neighbors[n] == other)return true;
  }
  return false;
}


JVR_NAMESPACE_CLOSE_SCOPE