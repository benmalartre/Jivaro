#include "../pbd/contact.h"
#include "../pbd/particles.h"

JVR_NAMESPACE_OPEN_SCOPE

size_t _GetElementDimensionFromGeometry(Geometry* geometry)
{
  size_t n = 1;
  switch(geometry->GetType()) {
    case Geometry::POINT:
      n = 1; break;

    case Geometry::EDGE:
      n = 2; break;

    case Geometry::MESH:
      n = 3; break;
  }
  return n;
}

void Contact::Init(Geometry* geometry, Particles* particles, size_t index)
{
  size_t n = _GetElementDimensionFromGeometry(geometry);

  _normal = GetNormal(geometry->GetPositionsCPtr(), 
    &_elemId[0], n, geometry->GetMatrix());

  _relV = particles->_velocity[index] - geometry->GetVelocity(1.f);
  _nrmV = pxr::GfDot(_relV, _normal);
}

void Contact::Update(Geometry* geometry, Particles* particles, size_t index)
{
  size_t n = _GetElementDimensionFromGeometry(geometry);

  const pxr::GfVec3f position = GetPosition(geometry->GetPositionsCPtr(), 
    &_elemId[0], n, geometry->GetMatrix());

  _normal = GetNormal(geometry->GetPositionsCPtr(), &_elemId[0], n, geometry->GetMatrix());

  const pxr::GfVec3f delta = position - particles->_position[index];

  _d = -pxr::GfDot(delta, _normal);

  }
  //GetPosition(const pxr::GfVec3f* positions, int* elements, size_t sz, const pxr::GfMatrix4d&)
  
  _p1 = particles->_position[index];
  _p2 = GetPosition()
}

JVR_NAMESPACE_CLOSE_SCOPE