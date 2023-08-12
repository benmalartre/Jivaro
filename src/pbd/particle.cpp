#include "../pbd/particle.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD 
{
  void Particles::AddBody(Body* body, const pxr::GfMatrix4f& matrix)
  {
    Geometry* geom = body->geometry;
    size_t base = _position.size();
    size_t add = geom->GetNumPoints();
    size_t newSize = base + add;
    _position.resize(newSize);


    const pxr::VtArray<pxr::GfVec3f>& points = geom->GetPositions();
    for (size_t p = 0; p < add; ++p) {
      const pxr::GfVec3f pos = matrix.Transform(points[p]);
      size_t idx = base + p;
      _position[idx] = pos;
    }
  }

  void Particles::RemoveBody(Body* body) 
  {
    size_t base = body->offset;
    size_t shift = body->numPoints;
    size_t remaining = _position.size() - (base + shift);

    for (size_t r = 0; r < remaining; ++r) {
      const size_t lhi = base + r;
      const size_t rhi = base + shift + r;
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
} //namespace PBD

JVR_NAMESPACE_CLOSE_SCOPE