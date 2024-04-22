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
#include <pxr/usd/usdGeom/capsule.h>


#include "../geometry/geometry.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../app/time.h"


JVR_NAMESPACE_OPEN_SCOPE

Geometry::Geometry()
{
  _type = INVALID;
  _mode = INPUT|OUTPUT;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  SetMatrix(pxr::GfMatrix4d(1.0));
}

Geometry::Geometry(int type, const pxr::GfMatrix4d& world)
{
  _type = type;
  _mode = INPUT|OUTPUT;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  SetMatrix(world);
}

Geometry::Geometry(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& world)
{
  if(prim.IsA<pxr::UsdGeomXform>())_type = Geometry::XFORM;
  else if(prim.IsA<pxr::UsdGeomPlane>())_type = Geometry::PLANE;
  else if(prim.IsA<pxr::UsdGeomSphere>())_type = Geometry::SPHERE;
  else if(prim.IsA<pxr::UsdGeomCapsule>())_type = Geometry::CAPSULE;
  else if(prim.IsA<pxr::UsdGeomCone>())_type = Geometry::CONE;
  else if(prim.IsA<pxr::UsdGeomCube>())_type = Geometry::CUBE;
  else if(prim.IsA<pxr::UsdGeomBasisCurves>())_type = Geometry::CURVE;
  else if(prim.IsA<pxr::UsdGeomMesh>())_type = Geometry::MESH;
  else if(prim.IsA<pxr::UsdGeomPoints>())_type = Geometry::POINT;
  else if(prim.IsA<pxr::UsdGeomPointInstancer>())_type = Geometry::INSTANCER;
  else _type = Geometry::INVALID;
  _mode = INPUT|OUTPUT;
  _wirecolor = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
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
  const pxr::GfVec3f delta(_matrix.GetRow3(3) -_prevMatrix.GetRow3(3));
  _velocity = pxr::GfVec3f(delta / (1.f / Time::Get()->GetFrameDuration()));

}

const pxr::GfVec3f Geometry::GetVelocity() const
{
  return _velocity;
}

Geometry::DirtyState 
Geometry::Sync(pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, const pxr::UsdTimeCode& time)
{
  SetMatrix(matrix);
  return _Sync(prim, matrix, time);
}

void Geometry::Inject(pxr::UsdPrim& prim, const pxr::GfMatrix4d& parent,
  const pxr::UsdTimeCode& time)
{
  pxr::UsdGeomXformable xformable(prim);
  pxr::UsdGeomXformOp op = xformable.MakeMatrixXform();


  pxr::GfMatrix4d local = parent.GetInverse() * GetMatrix();
  op.Set(local, time);

  pxr::UsdGeomBoundable usdBoundable(prim);
  pxr::VtArray<pxr::GfVec3f> bounds(2);
  bounds[0] = pxr::GfVec3f(_bbox.GetRange().GetMin());
  bounds[1] = pxr::GfVec3f(_bbox.GetRange().GetMax());
  usdBoundable.CreateExtentAttr().Set(bounds, time);

  _Inject(prim, parent, time);
}

const pxr::GfBBox3d 
Geometry::GetBoundingBox(bool worldSpace) const 
{ 
  if(!worldSpace)return _bbox;

  pxr::GfBBox3d worldBBox( _bbox.ComputeAlignedRange());
  return worldBBox;
};


JVR_NAMESPACE_CLOSE_SCOPE