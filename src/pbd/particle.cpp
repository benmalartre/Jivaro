#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

#include "../geometry/geometry.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"
#include "../pbd/particle.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

template<typename T> 
T* _ResizeArray(T* oldData, size_t oldSize, size_t newSize)
{
  T* newData = NULL;
  if(newSize > 0) {
    newData = new T[newSize];
    memcpy(newData, oldData, oldSize * sizeof(T));
    memset(newData + oldSize, 0.f, (newSize - oldSize) * sizeof(T));
  } 
  if(oldSize) delete [] oldData;
  return newData;
}

Particles::~Particles()
{
  _EnsureDataSize(0);
}

void Particles::_EnsureDataSize(size_t desired)
{
  size_t size = std::floor(num / BLOCK_SIZE) * BLOCK_SIZE;
  if(desired && size > desired)return;

  size = desired ? ((desired + BLOCK_SIZE) / BLOCK_SIZE) * BLOCK_SIZE : 0;
  state =     _ResizeArray<short>(state, num, size);
  body =      _ResizeArray<int>(body, num, size);
  mass =      _ResizeArray<float>(mass, num, size);
  invMass =   _ResizeArray<float>(invMass, num, size);
  radius =    _ResizeArray<float>(radius, num, size);
  counter =   _ResizeArray<pxr::GfVec2f>(counter, num, size);
  rest =      _ResizeArray<pxr::GfVec3f>(rest, num, size);
  input =     _ResizeArray<pxr::GfVec3f>(input, num, size);
  position =  _ResizeArray<pxr::GfVec3f>(position, num, size);
  predicted = _ResizeArray<pxr::GfVec3f>(predicted, num, size);
  velocity =  _ResizeArray<pxr::GfVec3f>(velocity, num, size);
  color =     _ResizeArray<pxr::GfVec3f>(color, num, size);
  rotation =  _ResizeArray<pxr::GfQuatf>(rotation, num, size);

}

void Particles::AddBody(Body* item, const pxr::GfMatrix4d& matrix)
{
  Geometry* geom = item->GetGeometry();
  if(geom->GetType() < Geometry::POINT) return;

  size_t base = num;
  size_t numPoints = geom->GetNumPoints();
  _EnsureDataSize(base + numPoints);
  size_t size = base + numPoints;
  size_t index = num > 0 ? body[num - 1] + 1 : 0;
  float m = item->GetMass();
  float w = m < 1e-6f ? 0.f : 1.f / m;

  const pxr::GfVec3f* points = ((Deformable*)geom)->GetPositionsCPtr();
  const pxr::GfRange3d range = geom->GetBoundingBox().GetRange();
  pxr::GfVec3f pos;
  size_t idx;
  for (size_t idx = base; idx < size; ++idx) {
    
    pos = matrix.Transform(points[idx - base]);
    float bY = float(idx) / float(size);
    mass[idx] = m;
    invMass[idx] = w;
    radius[idx] = item->GetRadius();
    rest[idx] = pos;
    input[idx] = pos;
    position[idx] = pos;
    predicted[idx] = pos;
    rotation[idx] = pxr::GfQuatf(1.f);
    velocity[idx] = item->GetVelocity();
    body[idx] = index;
    color[idx] = (pxr::GfVec3f(RANDOM_LO_HI(0.f, 0.2f)+0.6) + item->GetColor()) * 0.5f * bY;
    state[idx] = ACTIVE;
    counter[idx] = pxr::GfVec2f(0.f);
  }

  item->SetOffset(base);
  item->SetNumPoints(numPoints);
  num += numPoints;
}

void Particles::RemoveBody(Body* item) 
{
  const size_t base = item->GetOffset();
  const size_t shift = item->GetNumPoints();
  const size_t remaining = num - (base + shift);
  size_t lhi, rhi;

  for (size_t r = 0; r < remaining; ++r) {
    lhi = base + r;
    rhi = base + shift + r;
    mass[lhi]      = mass[rhi];
    invMass[lhi]   = invMass[rhi];
    radius[lhi]    = radius[rhi];
    rest[lhi]      = rest[rhi];
    input[lhi]     = input[rhi];
    position[lhi]  = position[rhi];
    predicted[lhi] = predicted[rhi];
    rotation[lhi]  = rotation[rhi];
    velocity[lhi]  = velocity[rhi];
    body[lhi]      = body[rhi] - 1;
    color[lhi]     = color[rhi];
    state[lhi]     = state[rhi];
    counter[lhi]   = counter[rhi];
  }

  size_t size = ((num - shift + BLOCK_SIZE) / BLOCK_SIZE) * BLOCK_SIZE;
  _EnsureDataSize(size);

  num -= shift;
}

void Particles::RemoveAllBodies()
{
  _EnsureDataSize(0);
  num = 0;
}

void Particles::SetAllState( short s)
{
  for(size_t p=0;p<num;++p) state[p] = s;
}

void Particles::SetBodyState(Body* item, short s)
{
  const size_t begin = item->GetOffset();
  const size_t end = begin +item->GetNumPoints();

  for (size_t r = begin; r < end; ++r) {
    state[r] = s;
  }
}

void Particles::ResetCounter(const std::vector<Constraint*>& constraints, size_t c)
{
  for (size_t p=0; p< num; ++p)counter[p][c] = 0.f;

  for (auto& constraint : constraints)
    for (auto& elem : constraint->GetElements())
      counter[elem][c]+=1.f;
}

void Body::UpdateParameters(pxr::UsdPrim& prim, float time)
{
  pxr::UsdPbdBodyAPI bodyApi(prim);
  bodyApi.GetSimulationEnabledAttr().Get(&_simulationEnabled, time);
  bodyApi.GetRadiusAttr().Get(&_radius, time);
  bodyApi.GetMassAttr().Get(&_mass, time);
  bodyApi.GetVelocityAttr().Get(&_velocity, time);
  bodyApi.GetDampAttr().Get(&_damp, time);
  bodyApi.GetSelfCollisionEnabledAttr().Get(&_selfCollisionEnabled, time);
  bodyApi.GetSelfCollisionRadiusAttr().Get(&_selfCollisionRadius, time);
  bodyApi.GetSelfCollisionFrictionAttr().Get(&_selfCollisionFriction, time);
  bodyApi.GetSelfCollisionRestitutionAttr().Get(&_selfCollisionRestitution, time);

  bool active;
  float stiffness, damp;
  
  for(auto& constraintsIt: _constraints) {
    if(prim.HasAPI<pxr::UsdPbdConstraintAPI>(constraintsIt.first)) {
      pxr::UsdPbdConstraintAPI api(prim, constraintsIt.first);

      api.GetStiffnessAttr().Get(&stiffness);
      api.GetDampAttr().Get(&damp);
      api.GetConstraintEnabledAttr().Get(&active);
      for(Constraint* constraint: constraintsIt.second->constraints) {
        constraint->SetStiffness(stiffness);
        constraint->SetDamp(damp);
        constraint->SetActive(active);
      }
    }
  }
}

void Body::UpdateParticles(Particles* particles)
{
  for(size_t p = _offset; p < _offset + _numPoints; ++p) {
    if(!_simulationEnabled)
      particles->state[p] = Particles::MUTE;
    else if(particles->state[p] != Particles::IDLE)
      particles->state[p] = Particles::ACTIVE;

    particles->radius[p] = _radius;
    particles->mass[p] = _mass;
    particles->invMass[p] = _mass > 1e-9 ? 1.f/_mass : 0.f;
  }
}

ConstraintsGroup* 
Body::AddConstraintsGroup(const pxr::TfToken& name, short type)
{
  if(_constraints.find(name) != _constraints.end())
    return _constraints[name];
  _constraints[name] = new ConstraintsGroup({_geometry->GetPrim(), name, type, {}}); 
  return _constraints[name];
}

size_t 
Body::GetNumConstraintsGroup()
{
  return _constraints.size();
};

 ConstraintsGroup* Body::GetConstraintsGroup(const pxr::TfToken& group)
{
  if(_constraints.find(group) != _constraints.end())return _constraints[group];
  return NULL;
};



JVR_NAMESPACE_CLOSE_SCOPE