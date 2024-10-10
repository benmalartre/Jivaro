#include <pxr/base/gf/ray.h>

#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/capsule.h>


#include "../geometry/geometry.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

float Geometry::FrameDuration = 1.f / 24.f;

Geometry::Geometry()
  : _type(INVALID)
  , _mode(INPUT|OUTPUT)
  , _wirecolor(pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim()
{
  SetMatrix(pxr::GfMatrix4d(1.0));
}

Geometry::Geometry(int type, const pxr::GfMatrix4d& world)
  : _type(type)
  , _mode(INPUT|OUTPUT)
  , _wirecolor(pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim()
{
  SetMatrix(world);
}

Geometry::Geometry(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world)
  : _type(INVALID)
  , _mode(INPUT)
  , _wirecolor(pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim(prim)
{
  if(prim.IsA<pxr::UsdGeomXform>())_type = Geometry::XFORM;
  else if(prim.IsA<pxr::UsdGeomPlane>())_type = Geometry::PLANE;
  else if(prim.IsA<pxr::UsdGeomSphere>())_type = Geometry::SPHERE;
  else if(prim.IsA<pxr::UsdGeomCapsule>())_type = Geometry::CAPSULE;
  else if(prim.IsA<pxr::UsdGeomCone>())_type = Geometry::CONE;
  else if(prim.IsA<pxr::UsdGeomCylinder>())_type = Geometry::CYLINDER;
  else if(prim.IsA<pxr::UsdGeomCube>())_type = Geometry::CUBE;
  else if(prim.IsA<pxr::UsdGeomBasisCurves>())_type = Geometry::CURVE;
  else if(prim.IsA<pxr::UsdGeomMesh>())_type = Geometry::MESH;
  else if(prim.IsA<pxr::UsdGeomPoints>())_type = Geometry::POINT;
  else if(prim.IsA<pxr::UsdGeomPointInstancer>())_type = Geometry::INSTANCER;
  else _type = Geometry::INVALID;
  SetMatrix(world);
}


void 
Geometry::SetMatrix(const pxr::GfMatrix4d& matrix) 
{ 
  _prevMatrix = _matrix;
  _matrix = matrix; 
  _invMatrix = matrix.GetInverse();
  _ComputeVelocity();
};


void Geometry::_ComputeVelocity()
{
  const pxr::GfVec3f deltaP(_matrix.GetRow3(3) -_prevMatrix.GetRow3(3));
  _velocity = pxr::GfVec3f(deltaP / Geometry::FrameDuration);

  const pxr::GfQuatd rotation = _matrix.ExtractRotationQuat();
  const pxr::GfQuatd previous = _prevMatrix.ExtractRotationQuat();
  const pxr::GfQuatd deltaR = (previous * rotation.GetInverse());
  
  _torque = pxr::GfVec3f(deltaR.GetImaginary() * (deltaR.GetReal() / Geometry::FrameDuration));

}

const pxr::GfVec3f Geometry::GetTorque() const
{
  return _torque;
}

const pxr::GfVec3f Geometry::GetVelocity() const
{
  return _velocity;
}

Geometry::DirtyState 
Geometry::Sync(const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  SetMatrix(matrix);
  return _Sync(matrix, time);
}

void Geometry::Inject(const pxr::GfMatrix4d& parent,
  const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomXformable xformable(_prim);
  pxr::UsdGeomXformOp op = xformable.MakeMatrixXform();

  pxr::GfMatrix4d local = parent.GetInverse() * GetMatrix();
  op.Set(local, time);

  pxr::UsdGeomBoundable usdBoundable(_prim);
  pxr::VtArray<pxr::GfVec3f> bounds(2);
  bounds[0] = pxr::GfVec3f(_bbox.GetRange().GetMin());
  bounds[1] = pxr::GfVec3f(_bbox.GetRange().GetMax());
  usdBoundable.CreateExtentAttr().Set(bounds, time);

  _Inject(parent, time);
}

const pxr::GfBBox3d 
Geometry::GetBoundingBox(bool worldSpace) const 
{ 
  if(!worldSpace)return _bbox;

  pxr::GfBBox3d worldBBox( _bbox.ComputeAlignedRange());
  return worldBBox;
};


JVR_NAMESPACE_CLOSE_SCOPE