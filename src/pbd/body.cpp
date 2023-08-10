#include "../pbd/body.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  void Body::SetGeometry(Geometry* geom, const pxr::GfMatrix4f& matrix)
  {
    _geom = geom;
    size_t size = geom->GetNumPoints();
    
    _position.resize(size);
    _predicted.resize(size);
    _velocity.resize(size);

    const pxr::VtArray<pxr::GfVec3f>& points = geom->GetPositions();
    for (size_t p = 0; p < size; ++p) {
      const pxr::GfVec3f pos = matrix.Transform(points[p]);
      _position[p] = pos;
      _predicted[p] = pos;
      _velocity[p] = pxr::GfVec3f(0.f);
    }
  }

  void Body::ConstrainPositions(float di)
  {
    for(auto& constraint: _constraints)
      constraint.ConstrainPositions(di);

    for(auto& staticConstraint: _staticConstraints)
        staticConstraint.ConstrainPositions(di);
  }

  void Body::ConstrainVelocities()
  {
    for(auto& constraint: _constraints)
      constraint.ConstrainVelocities();

    for(auto& staticConstraint: _staticConstraints)
      staticConstraint.ConstrainVelocities();
  }

}
JVR_NAMESPACE_CLOSE_SCOPE