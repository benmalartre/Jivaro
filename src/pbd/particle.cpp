#include "../geometry/geometry.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE


void Particles::AddBody(Body* body, const pxr::GfMatrix4f& matrix)
{
  Geometry* geom = body->geometry;
  size_t base = _position.size();
  size_t add = geom->GetNumPoints();
  size_t newSize = base + add;
  size_t index = _body.size() ? _body.back() + 1 : 0;
  _mass.resize(newSize);
  _radius.resize(newSize);
  _position.resize(newSize);
  _predicted.resize(newSize);
  _velocity.resize(newSize);
  _body.resize(newSize);

  const pxr::VtArray<pxr::GfVec3f>& points = geom->GetPositions();
  for (size_t p = 0; p < add; ++p) {
    const pxr::GfVec3f pos = matrix.Transform(points[p]);
    size_t idx = base + p;
    _mass[idx] = 1.f;
    _radius[idx] = 1.f;
    _position[idx] = pos;
    _predicted[idx] = pos;
    _velocity[idx] = pxr::GfVec3f(0.f);
    _body[idx] = index;
  }
}

void Particles::RemoveBody(Body* body) 
{
  const size_t base = body->offset;
  const size_t shift = body->numPoints;
  const size_t remaining = _position.size() - (base + shift);
  size_t lhi, rhi;

  for (size_t r = 0; r < remaining; ++r) {
    lhi = base + r;
    rhi = base + shift + r;
    _mass[lhi] = _mass[rhi];
    _radius[lhi] = _radius[rhi];
    _position[lhi] = _position[rhi];
    _predicted[lhi] = _predicted[rhi];
    _velocity[lhi] = _velocity[rhi];
    _body[lhi] = _body[rhi] - 1;
  }

  size_t newSize = _position.size() - shift;
  _mass.resize(newSize);
  _radius.resize(newSize);
  _position.resize(newSize);
  _predicted.resize(newSize);
  _velocity.resize(newSize);
  _body.resize(newSize);
  
}

JVR_NAMESPACE_CLOSE_SCOPE