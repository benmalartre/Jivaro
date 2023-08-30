#include "../geometry/geometry.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

void Particles::AddBody(Body* b, const pxr::GfMatrix4f& m)
{
  Geometry* geom = b->geometry;
  size_t base = position.size();
  size_t add = geom->GetNumPoints();
  size_t newSize = base + add;
  size_t index = body.size() ? body.back() + 1 : 0;
  float w = pxr::GfIsClose(b->mass, 0.f, 0.0000001f) ? 0.f : 1.f / b->mass;
  mass.resize(newSize);
  radius.resize(newSize);
  rest.resize(newSize);
  position.resize(newSize);
  predicted.resize(newSize);
  velocity.resize(newSize);
  body.resize(newSize);

  const pxr::VtArray<pxr::GfVec3f>& points = geom->GetPositions();
  for (size_t p = 0; p < add; ++p) {
    const pxr::GfVec3f pos = m.Transform(points[p]);
    size_t idx = base + p;
    mass[idx] = w;
    radius[idx] = b->radius;
    rest[idx] = pos;
    position[idx] = pos;
    predicted[idx] = pos;
    velocity[idx] = pxr::GfVec3f(0.f);
    body[idx] = index;
  }
}

void Particles::RemoveBody(Body* b) 
{
  const size_t base = b->offset;
  const size_t shift = b->numPoints;
  const size_t remaining = position.size() - (base + shift);
  size_t lhi, rhi;

  for (size_t r = 0; r < remaining; ++r) {
    lhi = base + r;
    rhi = base + shift + r;
    mass[lhi] = mass[rhi];
    radius[lhi] = radius[rhi];
    rest[lhi] = rest[rhi];
    position[lhi] = position[rhi];
    predicted[lhi] = predicted[rhi];
    velocity[lhi] = velocity[rhi];
    body[lhi] = body[rhi] - 1;
  }

  size_t newSize = position.size() - shift;
  mass.resize(newSize);
  radius.resize(newSize);
  rest.resize(newSize);
  position.resize(newSize);
  predicted.resize(newSize);
  velocity.resize(newSize);
  body.resize(newSize);
  
}

JVR_NAMESPACE_CLOSE_SCOPE