#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/transform.h>

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
  , _mode(OUTPUT)
  , _wirecolor(GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim()
{
  SetMatrix(GfMatrix4d(1.0));
}

Geometry::Geometry(int type, const GfMatrix4d& world)
  : _type(type)
  , _mode(OUTPUT)
  , _wirecolor(GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim()
{
  SetMatrix(world);
}

Geometry::Geometry(const UsdPrim& prim, const GfMatrix4d& world)
  : _type(INVALID)
  , _mode(INPUT)
  , _wirecolor(GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1))
  , _prim(prim)
{
  if(prim.IsA<UsdGeomXform>())_type = Geometry::XFORM;
  else if(prim.IsA<UsdGeomPlane>())_type = Geometry::PLANE;
  else if(prim.IsA<UsdGeomSphere>())_type = Geometry::SPHERE;
  else if(prim.IsA<UsdGeomCapsule>())_type = Geometry::CAPSULE;
  else if(prim.IsA<UsdGeomCone>())_type = Geometry::CONE;
  else if(prim.IsA<UsdGeomCylinder>())_type = Geometry::CYLINDER;
  else if(prim.IsA<UsdGeomCube>())_type = Geometry::CUBE;
  else if(prim.IsA<UsdGeomBasisCurves>())_type = Geometry::CURVE;
  else if(prim.IsA<UsdGeomMesh>())_type = Geometry::MESH;
  else if(prim.IsA<UsdGeomPoints>())_type = Geometry::POINT;
  else if(prim.IsA<UsdGeomPointInstancer>())_type = Geometry::INSTANCER;
  else _type = Geometry::INVALID;
  SetMatrix(world);
}


void 
Geometry::SetMatrix(const GfMatrix4d& matrix) 
{ 
  _prevMatrix = _matrix;
  _matrix = matrix; 
  _invMatrix = matrix.GetInverse();
  _ComputeVelocity();
};


void Geometry::_ComputeVelocity()
{
  const GfVec3f deltaP(_matrix.GetRow3(3) -_prevMatrix.GetRow3(3));
  _velocity = GfVec3f(deltaP / Geometry::FrameDuration);

  const GfQuatd rotation = _matrix.ExtractRotationQuat();
  const GfQuatd previous = _prevMatrix.ExtractRotationQuat();
  const GfQuatd deltaR = (previous * rotation.GetInverse());
  
  _torque = GfVec3f(deltaR.GetImaginary() * (deltaR.GetReal() / Geometry::FrameDuration));

}

const GfVec3d 
Geometry::GetTranslate()
{
  return _matrix.ExtractTranslation();
}

const GfVec3d
Geometry::GetScale()
{
  GfTransform transform(_matrix);
  return transform.GetScale();
}

const GfQuatd
Geometry::GetRotation()
{
  return _matrix.ExtractRotationQuat();
}

const GfVec3f Geometry::GetTorque() const
{
  return _torque;
}

const GfVec3f Geometry::GetVelocity() const
{
  return _velocity;
}

Geometry::DirtyState 
Geometry::Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  SetMatrix(matrix);
  return _Sync(matrix, time);
}

void Geometry::Inject(const GfMatrix4d& parent,
  const UsdTimeCode& time)
{
  UsdGeomXformable xformable(_prim);
  UsdGeomXformOp op = xformable.MakeMatrixXform();

  GfMatrix4d local = parent.GetInverse() * GetMatrix();
  op.Set(local, time);

  UsdGeomBoundable usdBoundable(_prim);
  VtArray<GfVec3f> bounds(2);
  bounds[0] = GfVec3f(_bbox.GetRange().GetMin());
  bounds[1] = GfVec3f(_bbox.GetRange().GetMax());
  usdBoundable.CreateExtentAttr().Set(bounds, time);

  _Inject(parent, time);
}

const GfBBox3d 
Geometry::GetBoundingBox(bool worldSpace) const 
{ 
  if(!worldSpace)return _bbox;

  GfBBox3d worldBBox( _bbox.ComputeAlignedRange());
  return worldBBox;
};


JVR_NAMESPACE_CLOSE_SCOPE