#include <pxr/base/vt/value.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../utils/timer.h"
#include "../geometry/utils.h"
#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../geometry/sampler.h"
#include "../pbd/force.h"
#include "../pbd/solver.h"
#include "../pbd/collision.h"
#include "../pbd/constraint.h"
#include "../app/application.h"
#include "../app/commands.h"
#include "../app/scene.h"

JVR_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
}

Scene::~Scene()
{
}

void 
Scene::Init(const pxr::UsdStageRefPtr& stage)
{

}

void
Scene::Update(const pxr::UsdStageRefPtr& stage, double time)
{
  pxr::UsdTimeCode activeTime = pxr::UsdTimeCode(time);
  pxr::UsdGeomXformCache xformCache(activeTime);

  for(auto& itPrim: _prims) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(itPrim.first);
    Geometry* geometry = itPrim.second.geom;
    pxr::GfMatrix4d matrix(xformCache.GetLocalToWorldTransform(prim));
    geometry->Sync(prim, matrix, time);
  }
}

void
Scene::Save(const std::string& filename)
{

}

void 
Scene::Export(const std::string& filename)
{
  
}

Mesh* Scene::AddMesh(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  _prims[path] = { new Mesh(xfo) };
  return (Mesh*)_prims[path].geom;
}

void Scene::AddMesh(const pxr::SdfPath& path, Mesh* mesh)
{
  _prims[path] = {mesh};
}

Voxels* Scene::AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius)
{
  _prims[path] = { new Voxels() };
  Voxels* voxels = (Voxels*)_prims[path].geom;
  voxels->Init(mesh, radius);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build();
  return voxels;
}


Geometry* Scene::AddGeometry(const pxr::SdfPath& path, short type, const pxr::GfMatrix4d& xfo)
{
  const auto& primIt = _prims.find(path);
  if( primIt != _prims.end())
    return primIt->second.geom;

  if(type == Geometry::XFORM) {
    _prims[path] = {new Xform(xfo)};
    return _prims[path].geom;
  }
  else if (type == Geometry::PLANE) {
    _prims[path] = { new Plane(xfo) };
    return _prims[path].geom;
  } else if (type == Geometry::SPHERE) {
    _prims[path] = { new Sphere(xfo) };
    return _prims[path].geom;
  } else if (type == Geometry::MESH) {
    _prims[path] = { new Mesh(xfo) };
    return _prims[path].geom;
  } else if (type == Geometry::CURVE) {
    _prims[path] = { new Curve(xfo) };
    return _prims[path].geom;
  } else {
    return NULL;
  }
}

void Scene::AddGeometry(const pxr::SdfPath& path, Geometry* geom)
{
  _prims[path] = {geom};
}

Curve* Scene::AddCurve(const pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
{
  _prims[path] = { new Curve() };
  return (Curve*)_prims[path].geom;
}

void Scene::AddCurve(const pxr::SdfPath & path, Curve* curve)
{
  _prims[path] = { curve };
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  _prims[path] = { new Points() };
  return (Points*)_prims[path].geom;
}

void Scene::AddPoints(const pxr::SdfPath& path, Points* points)
{
 _prims[path] = {points};
}

void Scene::Remove(const pxr::SdfPath & path)
{
  const auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* geometry = primIt->second.geom;
    _prims.erase(primIt);
    delete geometry;
  }
}

Geometry*
Scene::GetGeometry(const pxr::SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return _prims[path].geom;
  }
  return NULL;
}

Scene::_Prim*
Scene::GetPrim(const pxr::SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return &_prims[path];
  }
  return NULL;
}

// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

pxr::HdMeshTopology
Scene::GetMeshTopology(pxr::SdfPath const& id)
{
  if(IsMesh(id)) {
    Mesh* mesh = (Mesh*)_prims[id].geom;
    return pxr::HdMeshTopology(
      pxr::UsdGeomTokens->catmullClark,
      pxr::UsdGeomTokens->rightHanded,
      mesh->GetFaceCounts(),
      mesh->GetFaceConnects());
  }
  return pxr::HdMeshTopology();
}

pxr::HdBasisCurvesTopology
Scene::GetBasisCurvesTopology(pxr::SdfPath const& id)
{  
  if (IsCurves(id)) {
    Curve* curve = (Curve*)_prims[id].geom;
    return pxr::HdBasisCurvesTopology(
      pxr::HdTokens->linear,
      pxr::TfToken(),
      pxr::HdTokens->nonperiodic,
      curve->GetCvCounts(), pxr::VtArray<int>());
  }
  return pxr::HdBasisCurvesTopology();
}

pxr::GfRange3d 
Scene::GetExtent(pxr::SdfPath const& id)
{
  pxr::GfRange3d range;
  if (_prims.find(id) != _prims.end()) {
    if(_prims[id].geom->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
    } else {
      const pxr::VtVec3fArray& points = 
        ((Points*)_prims[id].geom)->GetPositions();
      TF_FOR_ALL(it, points) {
        range.UnionWith(*it);
      }
    }
  }
 
  
  return range;
}


pxr::GfMatrix4d
Scene::GetTransform(pxr::SdfPath const & id)
{
  if(_prims.find(id) != _prims.end())return _prims[id].geom->GetMatrix(); 
}

bool 
Scene::IsMesh(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::MESH);
}

bool
Scene::IsCurves(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::CURVE);
}

bool
Scene::IsPoints(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::POINT);
}

const pxr::SdfPath
Scene::GetPrimPath(Geometry* geom) const
{
  for (auto& prim : _prims) {
    if(prim.second.geom == geom)return prim.first;
  }
  return pxr::SdfPath();
}


pxr::TfToken
Scene::GetRenderTag(pxr::SdfPath const& id)
{
  if(_prims.find(id)!=_prims.end()) {
    return pxr::HdRenderTagTokens->geometry;
  }
  return pxr::HdRenderTagTokens->hidden;
}

pxr::VtValue 
Scene::Get(pxr::SdfPath const& id, pxr::TfToken const& key)
{
  if (key == pxr::HdTokens->points) {
    if(_prims[id].geom->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
      return pxr::VtValue();
    } else {
      return pxr::VtValue(((Points*)_prims[id].geom)->GetPositions());
    }
  } else if (key == pxr::HdTokens->displayColor) {
    if(_prims[id].geom->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
      return pxr::VtValue();
    } else {
      pxr::VtArray<pxr::GfVec3f>& colors =
        ((Points*)_prims[id].geom)->GetColors();
      if(colors.size())return pxr::VtValue(colors);
    }
  } else if (key == pxr::HdTokens->widths) {
    if(_prims[id].geom->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
      return pxr::VtValue();
    } else {
      return pxr::VtValue(((Points*)_prims[id].geom)->GetRadius());
    }
    
  }
  return pxr::VtValue();
}




JVR_NAMESPACE_CLOSE_SCOPE