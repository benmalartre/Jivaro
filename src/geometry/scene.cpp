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
#include "../geometry/instancer.h"
#include "../geometry/sampler.h"
#include "../geometry/scene.h"

JVR_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
}

Scene::~Scene()
{
  for(auto& prim: _prims) {
    if(prim.second.geom)delete prim.second.geom;
  }
}

void 
Scene::Init(const pxr::UsdStageRefPtr& stage)
{

}

void
Scene::Sync(const pxr::UsdStageRefPtr& stage, const pxr::UsdTimeCode& time)
{
  pxr::UsdTimeCode activeTime = pxr::UsdTimeCode(time);
  pxr::UsdGeomXformCache xformCache(activeTime);

  for(auto& itPrim: _prims) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(itPrim.first);
    Geometry* geometry = itPrim.second.geom;
    if (!geometry->IsInput() || !prim.IsValid()) {
      continue;
    }
    pxr::GfMatrix4d matrix(xformCache.GetLocalToWorldTransform(prim));
    geometry->Sync(matrix, time);
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
    _prims[path] = {new Xform(xfo), pxr::HdChangeTracker::Clean};
    return _prims[path].geom;
  } else if (type == Geometry::PLANE) {
    _prims[path] = { new Plane(xfo), pxr::HdChangeTracker::Clean};
    return _prims[path].geom;
  } else if (type == Geometry::SPHERE) {
    _prims[path] = { new Sphere(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CUBE) {
    _prims[path] = { new Cube(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CONE) {
    _prims[path] = { new Cone(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CAPSULE) {
    _prims[path] = { new Capsule(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::POINT) {
    _prims[path] = { new Points(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CURVE) {
    _prims[path] = { new Curve(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::MESH) {
    _prims[path] = { new Mesh(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::INSTANCER) {
    _prims[path] = { new Instancer(xfo), pxr::HdChangeTracker::Clean };
    return _prims[path].geom;
  } else {
    return NULL;
  }
}

void Scene::AddGeometry(const pxr::SdfPath& path, Geometry* geom)
{
  _prims[path] = {geom};
}


pxr::GfMatrix4d _GetParentXform(const pxr::UsdPrim& prim,
  const pxr::UsdTimeCode& time) 
{
  const pxr::UsdPrim& parent = prim.GetParent();
  if (!parent.IsValid())return pxr::GfMatrix4d(1.0);

  if (parent.IsA<pxr::UsdGeomXformable>()) {
    pxr::UsdGeomXformable xformable(parent);
    return xformable.ComputeLocalToWorldTransform(time);
  } else {
    return _GetParentXform(parent, time);
  }
}

void Scene::InjectGeometry(pxr::UsdStageRefPtr& stage, 
 const pxr::SdfPath& path, Geometry* geometry, const pxr::UsdTimeCode& time)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);
  if(!prim.IsValid()) {
    switch (geometry->GetType()) {
      case Geometry::XFORM:
        prim = pxr::UsdGeomXform::Define(stage, path).GetPrim();
        break;
      case Geometry::PLANE:
        prim = pxr::UsdGeomPlane::Define(stage, path).GetPrim();
        break;
      case Geometry::SPHERE:
        prim = pxr::UsdGeomSphere::Define(stage, path).GetPrim();
        break;
      case Geometry::CONE:
        prim = pxr::UsdGeomCone::Define(stage, path).GetPrim();
        break;
      case Geometry::CUBE:
        prim = pxr::UsdGeomCube::Define(stage, path).GetPrim();
        break;
      case Geometry::CAPSULE:
        prim = pxr::UsdGeomCapsule::Define(stage, path).GetPrim();
        break;
      case Geometry::POINT:
        prim = pxr::UsdGeomPoints::Define(stage, path).GetPrim();
        break;
      case Geometry::CURVE:
        prim = pxr::UsdGeomBasisCurves::Define(stage, path).GetPrim();
        break;
      case Geometry::MESH:
        prim = pxr::UsdGeomMesh::Define(stage, path).GetPrim();
        break;
      case Geometry::INSTANCER:
        prim = pxr::UsdGeomPointInstancer::Define(stage, path).GetPrim();
        break;
      default:
        return;
    }
  }

  const pxr::GfMatrix4d& parent = _GetParentXform(prim, time);
  geometry->SetPrim(prim);
  geometry->Inject(parent, time);
}

void Scene::RemoveGeometry(const pxr::SdfPath& path)
{
  if(_prims.find(path) != _prims.end())_prims.erase(path);
}


Curve* Scene::AddCurve(const pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
{
  _prims[path] = { new Curve() };
  return (Curve*)_prims[path].geom;
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  _prims[path] = { new Points() };
  return (Points*)_prims[path].geom;
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

pxr::SdfPath
Scene::GetInstancerBinding(const pxr::SdfPath& path)
{
  return pxr::SdfPath();
}


void 
Scene::MarkPrimDirty(const pxr::SdfPath& path, pxr::HdDirtyBits bits)
{
  Scene::_Prim* prim = GetPrim(path);
  if(prim)prim->bits = bits; 
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
  if (_prims.find(id) != _prims.end()) {
    Geometry* geometry = _prims[id].geom;
    if(geometry->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
    } else {
     return geometry->GetBoundingBox().GetRange();
    }
  }
 
  
  return pxr::GfRange3d();
}


pxr::GfMatrix4d
Scene::GetTransform(pxr::SdfPath const & id)
{
  if(_prims.find(id) != _prims.end())return _prims[id].geom->GetMatrix(); 
  return pxr::GfMatrix4d(1.0);
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
  const _Prim& prim = _prims[id];
  const short type = prim.geom->GetType();

  if(type == Geometry::INSTANCER) {
    Instancer* instancer = (Instancer*)prim.geom;
    if (key == pxr::HdInstancerTokens->instanceTranslations) {
      return pxr::VtValue(instancer->GetPositions());
    } else if (key == pxr::HdTokens->displayColor) {
      return pxr::VtValue(instancer->GetColors());
    } else if (key == pxr::HdInstancerTokens->instanceIndexBase) {
      return pxr::VtValue(instancer->GetProtoIndices());
    } else if (key == pxr::HdTokens->indices) {
      return instancer->HaveIndices() ? 
        pxr::VtValue(instancer->GetIndices()) :  pxr::VtValue();
    } else if (key == pxr::HdInstancerTokens->instanceScales) {
      return pxr::VtValue(instancer->GetScales());
    } else if (key == pxr::HdInstancerTokens->instanceRotations) {
      return pxr::VtValue(instancer->GetRotations());
    }
    else {

    }
  } else {
    if (key == pxr::HdTokens->points) {
      std::cout << "get positions for " << id << std::endl;
      return pxr::VtValue(((Deformable*)prim.geom)->GetPositions());
    } else if (key == pxr::HdTokens->displayColor) {
      pxr::VtArray<pxr::GfVec3f>& colors =
        ((Points*)prim.geom)->GetColors();
      if(colors.size())return pxr::VtValue(colors);
    } else if (key == pxr::HdTokens->widths) {
      return pxr::VtValue(((Points*)prim.geom)->GetWidths());
    } else if (key == pxr::HdTokens->normals) {
      if(((Deformable*)prim.geom)->HaveNormals()) {
        return pxr::VtValue(((Points*)prim.geom)->GetNormals());
      }
    }
  }
  return pxr::VtValue();
}


JVR_NAMESPACE_CLOSE_SCOPE