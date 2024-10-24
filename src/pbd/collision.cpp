#include <usdPbd/collisionAPI.h>
#include <pxr/base/work/loops.h>

#include "../utils/color.h"
#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../pbd/utils.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/contact.h"
#include "../pbd/solver.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

const size_t Collision::PACKET_SIZE = 64;
const float Collision::TOLERANCE_MARGIN = 0.005f;


void Collision::Reset()
{
  _contacts.ResetAllUsed();
}

void Collision::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  const float r = 0.05f;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_contacts.IsUsed(index)) {
      Contact* contact = _contacts.Get(index);
      points.push_back(pxr::GfVec3f(contact->GetPoint()));
      colors.push_back(color);
      radius.push_back(r);
    }
}

void Collision::GetNormals(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors, pxr::VtArray<int>& counts)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  const float r = 0.05f;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_contacts.IsUsed(index)) {
      Contact* contact = _contacts.Get(index);
      if(contact) {
        const pxr::GfVec3f position = pxr::GfVec3f(contact->GetPoint());
        points.push_back(position);
        points.push_back(position + pxr::GfVec3f(contact->GetNormal()));
        colors.push_back(color);
        colors.push_back(color);
        radius.push_back(r);
        radius.push_back(r);
        counts.push_back(2);
      }
    }
}

void Collision::GetVelocities(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors, pxr::VtArray<int>& counts)
{
}

// 
// Contacts
//
void Collision::UpdateContacts(Particles* particles)
{
  pxr::WorkParallelForN(
      particles->GetNumParticles(),
      std::bind(&Collision::_UpdateContacts, this, particles, 
        std::placeholders::_1, std::placeholders::_2), PACKET_SIZE);
}

void Collision::_UpdateContacts(Particles* particles, size_t begin, size_t end)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_contacts.IsUsed(index)) {
      Contact* contact = _contacts.Get(index);
      contact->Update(
        GetGradient(particles, index),
        GetVelocity(particles, index),
        GetValue(particles, index)
      );
    }
}

void Collision::FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
  std::vector<Constraint*>& constraints, float ft)
{

  Init(particles->GetNumParticles());
  
  _ResetContacts(particles);
  if(!_enabled)return; 

  pxr::WorkParallelForN(particles->GetNumParticles(),
    std::bind(&Collision::_FindContacts, this, particles,
      std::placeholders::_1, std::placeholders::_2, ft), PACKET_SIZE);
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
  Body* currentBody = nullptr;

  Mask::Iterator iterator(this, 0, numParticles);
  size_t particleToContactIdx = 0;
  size_t numHits = 0;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    if (CheckHit(index)) {
      numHits++;
      _c2p.push_back(index);
      if (particles->body[index] != currentBody || elements.size() >= Constraint::BlockSize) {
        if (elements.size()) {
          constraint = new CollisionConstraint(currentBody, this, elements, _stiffness, _damp);
          StoreContactsLocation(particles, & elements[0], elements.size(), ft);
          constraints.push_back(constraint);
          elements.clear();
        }
        currentBody = particles->body[index];
      } 
      elements.push_back(index);
    }
  } 
  
  if (elements.size()) {
    constraint = new CollisionConstraint(currentBody, this, elements);
    StoreContactsLocation(particles, & elements[0], elements.size(), ft);
    constraints.push_back(constraint);
  }
}

void Collision::_FindContacts(Particles* particles, size_t begin, size_t end, float ft)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    _FindContact(particles, index, ft);
  }
}

void 
Collision::CreateContactConstraints(Particles* particles, const std::vector<Body*>& bodies,
    std::vector<Constraint*>& constraints)
{
  Constraint* constraint = nullptr;
  if(_contacts.GetTotalNumUsed()) {
    if(GetTypeId() == Collision::MESH) {
      const Mesh* mesh = (const Mesh*)_collider;
      const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
      const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();

      pxr::VtArray<int> elements;
      pxr::VtArray<Contact*> contacts;

      Mask::Iterator iterator(this, 0, particles->GetNumParticles());
      size_t index = iterator.Begin();
      for (; index != Mask::INVALID_INDEX; index = iterator.Next()) {
        if(!_contacts.GetNumUsed(index)) continue;
        
        Contact* contact = _contacts.Get(index);
        
        if(contact->IsTouching()) {
          //const pxr::GfVec3f velocity = mesh->GetTriangleVelocity(contact->GetComponentIndex());
          //particles->position[p] = desired;
          //particles->predicted[p] = desired;
          elements.push_back(index);
          contacts.push_back(contact);
          particles->color[index] = pxr::GfVec3f(1.f, 0.f, 0.f);
        }  
        else
          particles->color[index] = pxr::GfVec3f(0.5f, 0.5f, 0.5f);

        if ((elements.size() >= Constraint::BlockSize) || iterator.End()) {
          if (elements.size()) {
            constraint = new ContactConstraint(particles->body[index], elements, this, contacts, _stiffness, _damp);
            constraints.push_back(constraint);
            elements.clear();
            contacts.clear();
          } 
        }
      }
    }
    // TODO implement other collision types
  }


  /*
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
  */
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

void Collision::_UpdateParameters(const pxr::UsdPrim& prim, double time)
{
  pxr::UsdPbdCollisionAPI api(prim);
  api.GetCollisionEnabledAttr().Get(&_enabled, time);
  api.GetRestitutionAttr().Get(&_restitution, time);
  api.GetFrictionAttr().Get(&_friction, time);
  api.GetDampAttr().Get(&_damp, time);
  api.GetMarginAttr().Get(&_margin, time);
  api.GetMaxSeparationVelocityAttr().Get(&_maxSeparationVelocity, time);
  api.GetCollisionStiffnessAttr().Get(&_stiffness, time);
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
  return pxr::GfVec3f(_contacts.Get(index, c)->GetPoint());
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

void Collision::SetContactTouching(size_t index, bool touching, size_t c)
{
  _contacts.Get(index, c)->SetTouching(touching);
}

bool Collision::IsContactTouching(size_t index, size_t c) const
{
  return _contacts.Get(index, c)->IsTouching();
}

static void _InitializeContact(Particles* particles, int index, Contact* contact, 
  const pxr::GfVec3f& position, const pxr::GfVec3f &normal, const pxr::GfVec3f &velocity, const float d)
{

  const pxr::GfVec3f intersection = position + normal * d;

  contact->Init(normal, velocity, d);
  contact->SetPoint(intersection);
  contact->SetDistance(d);
  
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
    (particles->radius[index]);
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
  float d = pxr::GfDot(_normal, predicted - _position)  - (particles->radius[index] + _margin);
  SetHit(index, d < 0.f);
}

void PlaneCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->predicted[index];
  float d = pxr::GfDot(_normal, predicted - _position)  - particles->radius[index];
  
  _InitializeContact(particles, index, contact, predicted, _normal, GetVelocity(particles, index), d);

}

//----------------------------------------------------------------------------------------
// Box Collision
//----------------------------------------------------------------------------------------
size_t BoxCollision::TYPE_ID = Collision::BOX;

BoxCollision::BoxCollision(Geometry* collider, const pxr::SdfPath& path, 
  float restitution, float friction)
  : Collision(collider, path, restitution, friction)
{
  _UpdateSize();
}

void BoxCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateSize();
  _UpdateParameters(prim, time);
}

void BoxCollision::_UpdateSize()
{
  Cube* cube = (Cube*)_collider;
  _size = cube->GetSize();
} 


pxr::GfVec3f _PointOnBox(const pxr::GfVec3f& local, double size, const pxr::GfMatrix4d& m)
{
  const double halfSize = size * 0.5f;
  const double xDiff = pxr::GfAbs(local[0]) - halfSize;
  const double yDiff = pxr::GfAbs(local[1]) - halfSize;
  const double zDiff = pxr::GfAbs(local[2]) - halfSize;

  pxr::GfVec3f normal;
  double d;
  if(xDiff <= yDiff && xDiff <= zDiff) {
    d = xDiff;
    normal = local[0] > 0.f ? pxr::GfVec3f(-1.f, 0.f, 0.f) : pxr::GfVec3f(1.f, 0.f, 0.f);
  } else if(yDiff <= zDiff) {
    d = yDiff;
    normal = local[1] > 0.f ? pxr::GfVec3f(0.f, -1.f, 0.f) : pxr::GfVec3f(0.f, 1.f, 0.f);
  } else {
    d = zDiff;
    normal = local[2] > 0.f ? pxr::GfVec3f(0.f, 0.f, -1.f) : pxr::GfVec3f(0.f, 0.f, 1.f);
  }

  return m.Transform(normal * d);
}

void BoxCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  const pxr::GfVec3f velocity = particles->velocity[index] * ft;
  const pxr::GfVec3f predicted(particles->position[index] + velocity);
  Cube* cube = (Cube*) _collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;

  SetHit(index, cube->SignedDistance(predicted) - particles->radius[index] / scaleFactor < Collision::TOLERANCE_MARGIN);
}

void BoxCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->predicted[index];// + particles->velocity[index] * ft;
  Cube* cube = (Cube*) _collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;
  
  const pxr::GfVec3f local = _collider->GetInverseMatrix().Transform(predicted);
  
  const pxr::GfVec3f closest = _PointOnBox(local, _size, _collider->GetMatrix());

  pxr::GfVec3f normal = predicted - closest;
  const float d = cube->SignedDistance(predicted) - particles->radius[index] / scaleFactor;
  normal.Normalize();

  _InitializeContact(particles, index, contact, predicted, normal, GetVelocity(particles, index), d);
}

float BoxCollision::GetValue(Particles* particles, size_t index)
{
  Cube* cube = (Cube*) _collider;
  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;
  return cube->SignedDistance(particles->predicted[index]) - particles->radius[index] / scaleFactor;
}
  
pxr::GfVec3f BoxCollision::GetGradient(Particles* particles, size_t index)
{
  const pxr::GfVec3f local = _collider->GetInverseMatrix().Transform(particles->predicted[index]);

  const pxr::GfVec3f closest = _PointOnBox(local, _size, _collider->GetMatrix());

  return (particles->predicted[index] - closest).GetNormalized();
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
  const pxr::GfVec3f velocity = particles->velocity[index] * ft;
  pxr::GfVec3f predicted(particles->position[index] + velocity);
  predicted = _collider->GetInverseMatrix().Transform(predicted);
  SetHit(index, predicted.GetLength() - particles->radius[index] < _radius);
}

void SphereCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->predicted[index];// + particles->velocity[index] * ft;

  Sphere* sphere = (Sphere*)_collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;

  const float d = sphere->SignedDistance(particles->predicted[index]) - particles->radius[index] / scaleFactor;

  const pxr::GfVec3f normal = (predicted - _center).GetNormalized();

  _InitializeContact(particles, index, contact, predicted, normal, GetVelocity(particles, index), d);

}

float SphereCollision::GetValue(Particles* particles, size_t index)
{
  Sphere* sphere = (Sphere*)_collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;

  return sphere->SignedDistance(particles->predicted[index]) - particles->radius[index] / scaleFactor;
}
  
pxr::GfVec3f SphereCollision::GetGradient(Particles* particles, size_t index)
{
  return (particles->predicted[index] - _center).GetNormalized();
}


//----------------------------------------------------------------------------------------
// Capsule Collision
//----------------------------------------------------------------------------------------
size_t CapsuleCollision::TYPE_ID = Collision::CAPSULE;

CapsuleCollision::CapsuleCollision(Geometry* collider, const pxr::SdfPath& path, 
  float restitution, float friction)
  : Collision(collider, path, restitution, friction)
{
  Capsule* capsule = (Capsule*)collider;
  _UpdateRadiusAndHeight();
}

void CapsuleCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateRadiusAndHeight();
  _UpdateParameters(prim, time);
}

void CapsuleCollision::_UpdateRadiusAndHeight()
{
  Capsule* capsule = (Capsule*)_collider;
  _radius = capsule->GetRadius();
  _height = capsule->GetHeight();
} 


static pxr::GfVec3f _PointOnCapsuleSegment(const pxr::GfVec3f &p, 
  const pxr::TfToken &axis, double height)
{
  pxr::GfVec3f a, b;
  if(axis == pxr::UsdGeomTokens->x) {
    a = pxr::GfVec3f(-height*0.5f, 0.f, 0.f);
    b = pxr::GfVec3f(height*0.5f, 0.f, 0.f);
  } else if(axis == pxr::UsdGeomTokens->y) {
    a = pxr::GfVec3f(0.f, -height*0.5f, 0.f);
    b = pxr::GfVec3f(0.f, height*0.5f, 0.f);
  } else if(axis == pxr::UsdGeomTokens->z) {
    a = pxr::GfVec3f(0.f, 0.f, -height*0.5f);
    b = pxr::GfVec3f(0.f, 0.f, height*0.5f);
  }

  const pxr::GfVec3f pa(p - a), ba(b - a);
  return a + ba * pxr::GfClamp(pxr::GfDot(pa, ba) / pxr::GfDot(ba, ba), 0.f, 1.f );
}
 
void CapsuleCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  Capsule* capsule = (Capsule*)_collider;
  pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;
  const float d = capsule->SignedDistance(predicted) - (particles->radius[index] + _margin) / scaleFactor;

  SetHit(index, d < Collision::TOLERANCE_MARGIN);
}

void CapsuleCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  const pxr::GfVec3f predicted = particles->predicted[index];// + particles->velocity[index] * ft;

  Capsule* capsule = (Capsule*)_collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;

  pxr::GfVec3f normal = GetGradient(particles, index);
  float d = capsule->SignedDistance(predicted) - particles->radius[index] / scaleFactor;

  _InitializeContact(particles, index, contact, predicted, normal, GetVelocity(particles, index), d);
}

float CapsuleCollision::GetValue(Particles* particles, size_t index)
{
  Capsule* capsule = (Capsule*)_collider;

  const pxr::GfVec3d scale = _collider->GetScale();
  const float scaleFactor = (scale[0] + scale[1] + scale[2]) / 3.f + 1e-9;

  return capsule->SignedDistance(particles->predicted[index]) - particles->radius[index] / scaleFactor;
}
  
pxr::GfVec3f CapsuleCollision::GetGradient(Particles* particles, size_t index)
{
  Capsule* capsule = (Capsule*)_collider;
  const pxr::GfVec3f local = _collider->GetInverseMatrix().Transform(particles->predicted[index]);
  const pxr::GfVec3f closest = _PointOnCapsuleSegment(local, capsule->GetAxis(), capsule->GetHeight());

  const pxr::GfVec3f surface = closest + (local - closest).GetNormalized() * _radius;
  pxr::GfVec3f world = _collider->GetMatrix().Transform(surface);

  if((local - closest).GetLengthSq() < _radius * _radius)
    return (world - particles->predicted[index]).GetNormalized();
  else
    return (particles->predicted[index] - world).GetNormalized();
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

MeshCollision::~MeshCollision()
{
}

void MeshCollision::Update(const pxr::UsdPrim& prim, double time)
{
  _UpdateParameters(prim, time);
  _UpdateAccelerationStructure();
}

void MeshCollision::Init(size_t numParticles)
{
  _query.clear();
  _query.resize(numParticles, Location());
  _closest.clear();
  _closest.resize(numParticles, Location());
}

void MeshCollision::_CreateAccelerationStructure()
{
  _bvh.Init({_collider});
} 

void MeshCollision::_UpdateAccelerationStructure()
{
  _bvh.Update();
} 


void MeshCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  Mesh* mesh = (Mesh*)_collider;
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();

  const pxr::GfVec3f predicted = particles->predicted[index] + particles->velocity[index] * ft;
  const float maxDistance = particles->velocity[index].GetLength() * ft + particles->radius[index] + _margin;

  if(_bvh.Closest(predicted, &_closest[index], maxDistance * 4.f)) {

    const Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());

    const pxr::GfVec3f position = 
      _closest[index].ComputePosition(positions, &triangle->vertices[0], 3, &mesh->GetMatrix());

    const pxr::GfVec3f normal = 
      _closest[index].ComputeNormal(normals, &triangle->vertices[0], 3, &mesh->GetMatrix());

    const pxr::GfVec3f delta = predicted - position;
    SetHit(index, (delta.GetLength() < particles->radius[index]) ||
      (pxr::GfDot(delta.GetNormalized(), normal) < _margin));
  }
    
  else
    SetHit(index, false);
  
}

void MeshCollision::_StoreContactLocation(Particles* particles, int index, Contact* contact, float ft)
{
  contact->Set(_closest[index]);

  Mesh* mesh = (Mesh*)GetGeometry();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();

  const Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());

  const pxr::GfVec3f position = 
    _closest[index].ComputePosition(positions, &triangle->vertices[0], 3, &mesh->GetMatrix());

  const pxr::GfVec3f normal = 
    _closest[index].ComputeNormal(normals, &triangle->vertices[0], 3, &mesh->GetMatrix());

  const float d = pxr::GfDot(particles->predicted[index] - position, normal)  - particles->radius[index];
  contact->Init(normal, pxr::GfVec3f(0.f), d);

  _InitializeContact(particles, index, contact, particles->predicted[index], normal, GetVelocity(particles, index), d);

}

float 
MeshCollision::GetValue(Particles* particles, size_t index)
{
  Mesh* mesh = (Mesh*)GetGeometry();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();
  
  const Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());

  const pxr::GfVec3f position = 
    _closest[index].ComputePosition(positions, &triangle->vertices[0], 3, &mesh->GetMatrix());

  const pxr::GfVec3f normal = 
    _closest[index].ComputeNormal(normals, &triangle->vertices[0], 3, &mesh->GetMatrix());

  return pxr::GfDot(particles->predicted[index] - position, normal)  - particles->radius[index];

}
  
pxr::GfVec3f 
MeshCollision::GetGradient(Particles* particles, size_t index)
{
  if(!_closest[index].IsValid())return pxr::GfVec3f(0.f);
  Mesh* mesh = (Mesh*)GetGeometry();
  const pxr::GfVec3f* normals = mesh->GetNormalsCPtr();
  const Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());

  return _closest[index].ComputeNormal(normals, &triangle->vertices[0], 3, &mesh->GetMatrix());
}


pxr::GfVec3f 
MeshCollision::GetVelocity(Particles* particles, size_t index)
{
  if(!_closest[index].IsValid())return pxr::GfVec3f(0.f);
  Mesh* mesh = (Mesh*)GetGeometry();

  const pxr::GfVec3f torque = _collider->GetTorque();
  const pxr::GfVec3f tangent = (GetGradient(particles, index) ^ torque).GetNormalized();

  return _collider->GetVelocity() + tangent * torque.GetLength() +
    mesh->GetTriangleVelocity(_closest[index].GetComponentIndex());
}

void 
MeshCollision::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f color(1.f, 0.f, 0.f);
  const float r = 0.05f;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_closest[index].IsValid()) {
      points.push_back(pxr::GfVec3f(_closest[index].GetPoint()));
      colors.push_back(color);
      radius.push_back(r);
    }
}

void 
MeshCollision::GetNormals(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors, pxr::VtArray<int>& counts)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f red(1.f, 0.f, 0.f);
  const float r = 0.05f;
  Mesh* mesh = (Mesh*)_collider;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_closest[index].IsValid()) {
      Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());
      const pxr::GfVec3f position =
        _closest[index].ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &mesh->GetMatrix());
      const pxr::GfVec3f normal =
        _closest[index].ComputeNormal(mesh->GetNormalsCPtr(), &triangle->vertices[0], 3, &mesh->GetMatrix());

      points.push_back(position);
      points.push_back(position + normal);
      colors.push_back(red);
      colors.push_back(red);
      radius.push_back(r);
      radius.push_back(r);
      counts.push_back(2);
    }
}

void 
MeshCollision::GetVelocities(Particles* particles, pxr::VtArray<pxr::GfVec3f>& points,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors, pxr::VtArray<int>& counts)
{
  Mask::Iterator iterator(this, 0, particles->GetNumParticles());
  const pxr::GfVec3f green(0.f, 1.f, 0.f);
  const float r = 0.05f;

  bool particleVelocity = true;
  Mesh* mesh = (Mesh*)_collider;
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next())
    if(_closest[index].IsValid()) {
      if(particleVelocity) {

        points.push_back(particles->position[index]);
        points.push_back(particles->position[index] + particles->velocity[index] * Geometry::FrameDuration);
        colors.push_back(green);
        colors.push_back(green);
        radius.push_back(r);
        radius.push_back(r);
        counts.push_back(2);
      } else {
        Triangle* triangle = mesh->GetTriangle(_closest[index].GetComponentIndex());
        const pxr::GfVec3f velocity = mesh->GetMatrix().TransformDir(triangle->GetVelocity(mesh->GetPositionsCPtr(), mesh->GetPreviousCPtr()));
        const pxr::GfVec3f position =
          _closest[index].ComputePosition(mesh->GetPositionsCPtr(), &triangle->vertices[0], 3, &mesh->GetMatrix());

        points.push_back(position);
        points.push_back(position + velocity);
        colors.push_back(green);
        colors.push_back(green);
        radius.push_back(r);
        radius.push_back(r);
        counts.push_back(2);
      }
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
  double avgRadius = 0.0;
  size_t numParticles = _particles->GetNumParticles();

  for (size_t p = 0; p < numParticles; ++p) {
    Body* body = particles->body[p];
    avgRadius += particles->radius[p] * body->GetSelfCollisionRadius();
  }
  avgRadius /= static_cast<float>(numParticles);
  _grid.Init(numParticles, &_particles->predicted[0], avgRadius * 2.f);
}

SelfCollision::~SelfCollision()
{
  
}

void SelfCollision::_UpdateParameters(const pxr::UsdPrim& prim, double time)
{
  pxr::UsdPbdCollisionAPI api(prim);
  api.GetCollisionEnabledAttr().Get(&_enabled, time);
  api.GetRestitutionAttr().Get(&_restitution, time);
  api.GetFrictionAttr().Get(&_friction, time);
  api.GetDampAttr().Get(&_damp, time);
  api.GetMarginAttr().Get(&_margin, time);
  api.GetMaxSeparationVelocityAttr().Get(&_maxSeparationVelocity, time);
  api.GetCollisionStiffnessAttr().Get(&_stiffness);
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
    std::bind(&SelfCollision::_FindContacts, this, particles,
      std::placeholders::_1, std::placeholders::_2, ft), PACKET_SIZE);

  _BuildContacts(particles, bodies, constraints, ft);

}

void SelfCollision::UpdateContacts(Particles* particles)
{
  pxr::WorkParallelForN(
      particles->GetNumParticles(),
      std::bind(&SelfCollision::_UpdateContacts, this, particles, 
        std::placeholders::_1, std::placeholders::_2), PACKET_SIZE);
}

void SelfCollision::_UpdateContacts(Particles* particles, size_t begin, size_t end)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    //pxr::GfVec3f color = RandomColorByIndex(index);
    if (_contacts.IsUsed(index))
      for (size_t c = 0; c < _contacts.GetNumUsed(index); ++c) {
        Contact* contact = _contacts.Get(index, c);
        size_t other = contact->GetComponentIndex();

        contact->Update(
          GetGradient(particles, index, other), 
          GetVelocity(particles, index, other), 
          GetValue(particles, index, other)
         );
        /*
        if (index % 32 == 0) {
          particles->color[index] = color;
          particles->color[other] = color;
        }
        */
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

void SelfCollision::_FindContacts(Particles* particles, size_t begin, size_t end, float ft)
{
  Mask::Iterator iterator(this, begin, end);
  for (size_t index = iterator.Begin(); index != Mask::INVALID_INDEX; index = iterator.Next()) {
    _FindContact(particles, index, ft);
  }
}

void SelfCollision::_FindContact(Particles* particles, size_t index, float ft)
{
  std::vector<int> closests;

  Body* body = particles->body[index];
  if(!body->GetSelfCollisionEnabled())return;
  const float radiusMultiplier = body->GetSelfCollisionRadius();
  size_t numCollide = 0;
  _grid.Closests(index, &particles->predicted[0], /*&particles->velocity[0], ft,*/
    closests,  2.f * ( particles->radius[index] * radiusMultiplier + TOLERANCE_MARGIN));
  for(int closest: closests) {
    if(numCollide >= PARTICLE_MAX_CONTACTS)break;

    Body* other = particles->body[closest];
    if(other != body) continue;
    if(_AreConnected(index, closest))continue;

    pxr::GfVec3f ip(particles->position[index] + particles->velocity[index] * ft);
    pxr::GfVec3f cp(particles->position[closest] + particles->velocity[closest] * ft);
    
    if((ip - cp).GetLength() < ((particles->radius[index] + particles->radius[closest]) * radiusMultiplier)) {
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
  const pxr::GfVec3f ip(particles->predicted[index]);
  const pxr::GfVec3f op(particles->predicted[other]);
  pxr::GfVec3f normal = ip - op;
  float nL = normal.GetLength();

  Body* body = particles->body[index];
  
  const float radiusMultiplier = body->GetSelfCollisionRadius();
  float d = nL - (particles->radius[index] + particles->radius[other]) * radiusMultiplier;

  contact->Init(normal.GetNormalized(), GetVelocity(particles, index, other), d);
  contact->SetPoint(particles->predicted[index] + d * normal.GetNormalized());
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
  if(_particles->num)
    _grid.Update(&_particles->predicted[0]);
} 

float SelfCollision::GetValue(Particles* particles, size_t index, size_t other)
{
  Body* body = particles->body[index];
  const float radiusMultiplier = body->GetSelfCollisionRadius();

  return (particles->predicted[index] - particles->predicted[other]).GetLength() - 
    (particles->radius[index] + particles->radius[other]) * radiusMultiplier;
}
  
pxr::GfVec3f SelfCollision::GetGradient(Particles* particles, size_t index, size_t other)
{
  return (particles->predicted[index] - particles->predicted[other]).GetNormalized();
}

// Velocity
pxr::GfVec3f SelfCollision::GetVelocity(Particles* particles, size_t index, size_t other)
{
  return particles->velocity[other];
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
        if(!(mesh->GetFlags() & Mesh::NEIGHBORS))
          mesh->ComputeNeighbors();
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