#include <pxr/base/gf/ray.h>

#include "../geometry/geometry.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

Geometry::Geometry()
{
  _type = INVALID;
  _mode = INPUT|OUTPUT;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  SetMatrix(pxr::GfMatrix4d(1.0));
}

Geometry::Geometry(short type, const pxr::GfMatrix4d& world)
{
  _type = type;
  _mode = INPUT|OUTPUT;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  SetMatrix(world);
}

Geometry::Geometry(const Geometry* other, short type)
{
  _type = type;
  _mode = other->_mode;
  _wirecolor = other->_wirecolor;
  
  _bbox = other->_bbox;
  SetMatrix(other->GetMatrix());
}

void 
Geometry::SetMatrix(const pxr::GfMatrix4d& matrix) 
{ 
  _prevMatrix = _matrix;
  _matrix = matrix; 
  _invMatrix = matrix.GetInverse();
};

const pxr::GfVec3f Geometry::GetVelocity() const
{
  return pxr::GfVec3f(_matrix.GetRow3(3) -_prevMatrix.GetRow3(3));
}

Geometry::DirtyState 
Geometry::Sync(pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, float time)
{
  SetMatrix(matrix);
  return _Sync(prim, matrix, time);
}


const pxr::GfBBox3d& 
Geometry::GetBoundingBox(bool worldSpace) const 
{ 
  if(!worldSpace)return _bbox;
  pxr::GfBBox3d world(_bbox);
  world.Transform(_matrix);
  return world;
};

JVR_NAMESPACE_CLOSE_SCOPE