#include <algorithm>

#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

#include "../geometry/geometry.h"
#include "../geometry/points.h"
#include "../geometry/mesh.h"
#include "../geometry/smooth.h"

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

void Particles::_EnsureDataSize(size_t size)
{
  state =     _ResizeArray<short>(state, num, size);
  body =      _ResizeArray<Body*>(body, num, size);
  mass =      _ResizeArray<float>(mass, num, size);
  invMass =   _ResizeArray<float>(invMass, num, size);
  radius =    _ResizeArray<float>(radius, num, size);
  counter =   _ResizeArray<GfVec2f>(counter, num, size);
  rest =      _ResizeArray<GfVec3f>(rest, num, size);
  input =     _ResizeArray<GfVec3f>(input, num, size);
  previous =  _ResizeArray<GfVec3f>(previous, num, size);
  position =  _ResizeArray<GfVec3f>(position, num, size);
  predicted = _ResizeArray<GfVec3f>(predicted, num, size);
  velocity =  _ResizeArray<GfVec3f>(velocity, num, size);
  color =     _ResizeArray<GfVec3f>(color, num, size);
  rotation =  _ResizeArray<GfQuatf>(rotation, num, size);

}

void Particles::AddBody(Body* item, const GfMatrix4d& matrix)
{
  Geometry* geom = item->GetGeometry();
  if(geom->GetType() < Geometry::POINT) return;

  size_t base = num;
  size_t numPoints = geom->GetNumPoints();
  _EnsureDataSize(base + numPoints);
  size_t size = base + numPoints;
  float m = item->GetMass();
  float w = m < 1e-6f ? 0.f : 1.f / m;

  const GfVec3f* points = ((Deformable*)geom)->GetPositionsCPtr();
  const GfRange3d range = geom->GetBoundingBox().GetRange();
  GfVec3f pos;
  size_t idx;
  for (size_t idx = base; idx < size; ++idx) {
    
    pos = GfVec3f(matrix.Transform(points[idx - base]));
    float bY = float(idx) / float(size);
    state[idx] = ACTIVE;
    body[idx] = item;
    mass[idx] = m;
    invMass[idx] = w;
    radius[idx] = item->GetRadius();
    counter[idx] = GfVec2f(0.f);
    rest[idx] = pos;
    input[idx] = pos;
    previous[idx] = pos;
    position[idx] = pos;
    predicted[idx] = pos;
    velocity[idx] = item->GetVelocity();
    color[idx] = (GfVec3f(RANDOM_LO_HI(0.f, 0.2f)+0.6) + item->GetColor()) * 0.5f * bY;
    rotation[idx] = GfQuatf(1.f);
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
    previous[lhi]  = previous[rhi];
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

Body::~Body()
{
  for (auto& constraint : _constraints)
    delete constraint.second;
  _constraints.clear();
  delete _smoothKernel;
}

void 
Body::_InitSmoothKernel()
{
  VtFloatArray weights;
  _smoothKernel = new Smooth<GfVec3f>(_numPoints, weights);
  switch(_geometry->GetType()) {
    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)_geometry;
      if(!(mesh->GetFlags() & Mesh::ADJACENTS))mesh->ComputeAdjacents(); 

      for(size_t i = 0; i < _numPoints; ++i) {
        _smoothKernel->SetDatas(i, GfVec3f(0.f));
        _smoothKernel->SetNeighbors(i, mesh->GetNumAdjacents(i), mesh->GetAdjacents(i));
      }
      break;
    }
  }
}

void 
Body::SmoothVelocities(Particles* particles, size_t iterations)
{
  switch(_geometry->GetType()) {
    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)_geometry;
      if(!(mesh->GetFlags() & Mesh::ADJACENTS))mesh->ComputeAdjacents(); 
      const GfVec3f* velocities = &particles->velocity[0];

      for(size_t i = 0; i < _numPoints; ++i) {
        _smoothKernel->SetDatas(i, velocities[i + _offset]);
      }
      _smoothKernel->Compute(iterations);

      for(size_t i = 0; i < _numPoints; ++i) {
        particles->velocity[i + _offset] = _smoothKernel->GetDatas(i);
      }
      
    }
  }
}

void Body::UpdateParameters(UsdPrim& prim, float time)
{
  UsdPbdBodyAPI bodyApi(prim);
  bodyApi.GetSimulationEnabledAttr().Get(&_simulationEnabled, time);
  bodyApi.GetRadiusAttr().Get(&_radius, time);
  bodyApi.GetMassAttr().Get(&_mass, time);
  bodyApi.GetVelocityAttr().Get(&_velocity, time);
  bodyApi.GetDampAttr().Get(&_damp, time);
  bodyApi.GetSelfCollisionEnabledAttr().Get(&_selfCollisionEnabled, time);
  bodyApi.GetSelfCollisionRadiusAttr().Get(&_selfCollisionRadius, time);
  bodyApi.GetSelfCollisionFrictionAttr().Get(&_selfCollisionFriction, time);
  bodyApi.GetSelfCollisionRestitutionAttr().Get(&_selfCollisionRestitution, time);
  bodyApi.GetSelfCollisionDampAttr().Get(&_selfCollisionDamp, time);
  bodyApi.GetSelfCollisionMaxSeparationVelocityAttr().Get(&_selfCollisionMaxSeparationVelocity, time);

  bool active;
  float stiffness, damp;
  
  for(auto& constraintsIt: _constraints) {
    if(prim.HasAPI<UsdPbdConstraintAPI>(constraintsIt.first)) {
      UsdPbdConstraintAPI api(prim, constraintsIt.first);

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
Body::AddConstraintsGroup(const TfToken& name, short type)
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

 ConstraintsGroup* Body::GetConstraintsGroup(const TfToken& group)
{
  if(_constraints.find(group) != _constraints.end())return _constraints[group];
  return NULL;
};



JVR_NAMESPACE_CLOSE_SCOPE