#include "../geometry/geometry.h"
#include "../geometry/points.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

void Particles::AddBody(Body* addBody, const pxr::GfMatrix4d& matrix)
{
  Geometry* geom = addBody->GetGeometry();
  if(geom->GetType() < Geometry::POINT) return;

  size_t base = position.size();
  size_t numPoints = geom->GetNumPoints();
  size_t size = base + numPoints;
  size_t index = body.size() ? body.back() + 1 : 0;
  float w = pxr::GfIsClose(addBody->GetMass(), 0.f, 0.000001f) ? 0.f : 1.f / addBody->GetMass();
  mass.resize(size);
  invMass.resize(size);
  radius.resize(size);
  rest.resize(size);
  previous.resize(size);
  position.resize(size);
  predicted.resize(size);
  velocity.resize(size);
  body.resize(size);
  color.resize(size);
  state.resize(size);


  const pxr::VtArray<pxr::GfVec3f>& points = ((Deformable*)geom)->GetPositions();
  pxr::GfVec3f pos;
  size_t idx;
  for (size_t p = 0; p < numPoints; ++p) {
    pos = matrix.Transform(points[p]);
    idx = base + p;
    mass[idx] = addBody->GetMass();
    invMass[idx] = w;
    radius[idx] = addBody->GetRadius();
    rest[idx] = pos;
    previous[idx] = pos;
    position[idx] = pos;
    predicted[idx] = pos;
    velocity[idx] = pxr::GfVec3f(0.f);
    body[idx] = index;
    color[idx] = addBody->GetColor();
    state[idx] = ACTIVE;
  }

  addBody->SetOffset(base);
  addBody->SetNumPoints(numPoints);
}

void Particles::RemoveBody(Body* removeBody) 
{
  const size_t base = removeBody->GetOffset();
  const size_t shift = removeBody->GetNumPoints();
  const size_t remaining = GetNumParticles() - (base + shift);
  size_t lhi, rhi;

  for (size_t r = 0; r < remaining; ++r) {
    lhi = base + r;
    rhi = base + shift + r;
    mass[lhi]      = mass[rhi];
    invMass[lhi]   = invMass[rhi];
    radius[lhi]    = radius[rhi];
    rest[lhi]      = rest[rhi];
    previous[lhi]  = previous[rhi];
    position[lhi]  = position[rhi];
    predicted[lhi] = predicted[rhi];
    velocity[lhi]  = velocity[rhi];
    body[lhi]      = body[rhi] - 1;
    color[lhi]     = color[rhi];
    state[lhi]     = state[rhi];
  }

  size_t size = position.size() - shift;
  mass.resize(size);
  invMass.resize(size);
  radius.resize(size);
  rest.resize(size);
  previous.resize(size);
  position.resize(size);
  predicted.resize(size);
  velocity.resize(size);
  body.resize(size);
  color.resize(size);
  state.resize(size);
}

void Particles::RemoveAllBodies()
{
  mass.clear();
  invMass.clear();
  radius.clear();
  rest.clear();
  previous.clear();
  position.clear();
  predicted.clear();
  velocity.clear();
  body.clear();
  color.clear();
  state.clear();
}

void Particles::SetAllState( short s)
{
  for(auto& _s: state) _s = s;
}

void Particles::SetBodyState(Body* b, short s)
{
  const size_t begin = b->GetOffset();
  const size_t end = begin +b->GetNumPoints();

  for (size_t r = begin; r < end; ++r) {
    state[r] = s;
  }
}


JVR_NAMESPACE_CLOSE_SCOPE