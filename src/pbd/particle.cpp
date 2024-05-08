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
  float m = addBody->GetMass();
  float w = pxr::GfIsClose(m, 0.f, 0.000001f) ? 0.f : 1.f / m;
  mass.resize(size);
  invMass.resize(size);
  radius.resize(size);
  rest.resize(size);
  position.resize(size);
  predicted.resize(size);
  velocity.resize(size);
  body.resize(size);
  color.resize(size);
  state.resize(size);
  cnt.resize(size);

  const pxr::VtArray<pxr::GfVec3f>& points = ((Deformable*)geom)->GetPositions();
  pxr::GfVec3f pos;
  size_t idx;
  for (size_t idx = base; idx < base + numPoints; ++idx) {
    pos = matrix.Transform(points[idx - base]);
    mass[idx] = m;
    invMass[idx] = w;
    radius[idx] = addBody->GetRadius();
    rest[idx] = pos;
    position[idx] = pos;
    predicted[idx] = pos;
    velocity[idx] = addBody->GetVelocity();
    body[idx] = index;
    color[idx] = (pxr::GfVec3f(RANDOM_LO_HI(0.f, 0.2f)+0.6) + addBody->GetColor()) * 0.5f;
    state[idx] = ACTIVE;
    cnt[idx] = pxr::GfVec2f(0.f);
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
    position[lhi]  = position[rhi];
    predicted[lhi] = predicted[rhi];
    velocity[lhi]  = velocity[rhi];
    body[lhi]      = body[rhi] - 1;
    color[lhi]     = color[rhi];
    state[lhi]     = state[rhi];
    cnt[lhi]       = cnt[rhi];

  }

  size_t size = position.size() - shift;
  mass.resize(size);
  invMass.resize(size);
  radius.resize(size);
  rest.resize(size);
  position.resize(size);
  predicted.resize(size);
  velocity.resize(size);
  body.resize(size);
  color.resize(size);
  state.resize(size);
  cnt.resize(size);
}

void Particles::RemoveAllBodies()
{
  mass.clear();
  invMass.clear();
  radius.clear();
  rest.clear();
  position.clear();
  predicted.clear();
  velocity.clear();
  body.clear();
  color.clear();
  state.clear();
  cnt.clear();
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