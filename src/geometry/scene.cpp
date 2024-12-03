
#include <pxr/base/vt/value.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdShade/material.h>


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
  for(auto& prim: _prims) 
    if(prim.second.geom)
      delete prim.second.geom;
}

void 
Scene::Init(const UsdStageRefPtr& stage)
{
  std::cout << "-----------------------------------------------------------------------------------" << std::endl;
  std::cout << "SCENE INIT CALLED" << std::endl;

  for (UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<UsdShadeMaterial>()) {
      std::cout << "FOUND MATERIAL : " << prim.GetPath() << std::endl;
    }

  std::cout << "SCENE INIT END" << std::endl;
}

void
Scene::Sync(const UsdStageRefPtr& stage, const UsdTimeCode& time)
{
  UsdTimeCode activeTime = UsdTimeCode(time);
  UsdGeomXformCache xformCache(activeTime);

  for(auto& itPrim: _prims) {
    UsdPrim prim = stage->GetPrimAtPath(itPrim.first);
    Geometry* geometry = itPrim.second.geom;
    if (!geometry->IsInput() || !prim.IsValid())
      continue;
    
    GfMatrix4d matrix(xformCache.GetLocalToWorldTransform(prim));
    geometry->Sync(matrix, time);
  }
}

Mesh* Scene::AddMesh(const SdfPath& path, const GfMatrix4d& xfo)
{
  _prims[path] = { new Mesh(xfo) };
  return (Mesh*)_prims[path].geom;
}

Voxels* Scene::AddVoxels(const SdfPath& path, Mesh* mesh, float radius)
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

Geometry* Scene::AddGeometry(const SdfPath& path, short type, const GfMatrix4d& xfo)
{
  const auto& primIt = _prims.find(path);
  if( primIt != _prims.end())
    return primIt->second.geom;

  if(type == Geometry::XFORM) {
    _prims[path] = {new Xform(xfo), HdChangeTracker::Clean};
    return _prims[path].geom;
  } else if (type == Geometry::PLANE) {
    _prims[path] = { new Plane(xfo), HdChangeTracker::Clean};
    return _prims[path].geom;
  } else if (type == Geometry::SPHERE) {
    _prims[path] = { new Sphere(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CUBE) {
    _prims[path] = { new Cube(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CONE) {
    _prims[path] = { new Cone(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CAPSULE) {
    _prims[path] = { new Capsule(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::POINT) {
    _prims[path] = { new Points(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::CURVE) {
    _prims[path] = { new Curve(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::MESH) {
    _prims[path] = { new Mesh(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else if (type == Geometry::INSTANCER) {
    _prims[path] = { new Instancer(xfo), HdChangeTracker::Clean };
    return _prims[path].geom;
  } else {
    return NULL;
  }
}

void Scene::AddGeometry(const SdfPath& path, Geometry* geom)
{
  _prims[path] = {geom, HdChangeTracker::Clean};
}


GfMatrix4d _GetParentXform(const UsdPrim& prim,
  const UsdTimeCode& time) 
{
  const UsdPrim& parent = prim.GetParent();
  if (!parent.IsValid())return GfMatrix4d(1.0);

  if (parent.IsA<UsdGeomXformable>()) {
    UsdGeomXformable xformable(parent);
    return xformable.ComputeLocalToWorldTransform(time);
  } else {
    return _GetParentXform(parent, time);
  }
}

void Scene::InjectGeometry(UsdStageRefPtr& stage, 
 const SdfPath& path, Geometry* geometry, const UsdTimeCode& time)
{
  UsdPrim prim = stage->GetPrimAtPath(path);
  if(!prim.IsValid()) {
    switch (geometry->GetType()) {
      case Geometry::XFORM:
        prim = UsdGeomXform::Define(stage, path).GetPrim();
        break;
      case Geometry::PLANE:
        prim = UsdGeomPlane::Define(stage, path).GetPrim();
        break;
      case Geometry::SPHERE:
        prim = UsdGeomSphere::Define(stage, path).GetPrim();
        break;
      case Geometry::CONE:
        prim = UsdGeomCone::Define(stage, path).GetPrim();
        break;
      case Geometry::CUBE:
        prim = UsdGeomCube::Define(stage, path).GetPrim();
        break;
      case Geometry::CAPSULE:
        prim = UsdGeomCapsule::Define(stage, path).GetPrim();
        break;
      case Geometry::POINT:
        prim = UsdGeomPoints::Define(stage, path).GetPrim();
        break;
      case Geometry::CURVE:
        prim = UsdGeomBasisCurves::Define(stage, path).GetPrim();
        break;
      case Geometry::MESH:
        prim = UsdGeomMesh::Define(stage, path).GetPrim();
        break;
      case Geometry::INSTANCER:
        prim = UsdGeomPointInstancer::Define(stage, path).GetPrim();
        break;
      case Geometry::VOXEL:
        prim = UsdGeomPoints::Define(stage, path).GetPrim();
        break;
      default:
        return;
    }
  }

  const GfMatrix4d& parent = _GetParentXform(prim, time);
  geometry->SetPrim(prim);
  geometry->Inject(parent, time);
  geometry->SetInputOutput();
}

void Scene::RemoveGeometry(const SdfPath& path)
{
  if(_prims.find(path) != _prims.end())
    _prims.erase(path);
}

void Scene::RemoveAllGeometries()
{
  _prims.clear();
}


Curve* Scene::AddCurve(const SdfPath & path, const GfMatrix4d & xfo)
{
  _prims[path] = { new Curve() };
  return (Curve*)_prims[path].geom;
}

Points* Scene::AddPoints(const SdfPath& path, const GfMatrix4d& xfo)
{
  _prims[path] = { new Points() };
  return (Points*)_prims[path].geom;
}

void Scene::Remove(const SdfPath & path)
{
  const auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* geometry = primIt->second.geom;
    _prims.erase(primIt);
    delete geometry;
  }
}

Geometry*
Scene::GetGeometry(size_t index)
{
  _PrimMap::iterator primIt = _prims.begin();
  std::advance(primIt, index);
  return primIt != _prims.end() ? primIt->second.geom : NULL;
}

Geometry*
Scene::GetGeometry(const SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return _prims[path].geom;
  }
  return NULL;
}

Scene::_Prim*
Scene::GetPrim(const SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return &_prims[path];
  }
  return NULL;
}

SdfPath
Scene::GetInstancerBinding(const SdfPath& path)
{
  return SdfPath();
}


void 
Scene::MarkPrimDirty(const SdfPath& path, HdDirtyBits bits)
{
  Scene::_Prim* prim = GetPrim(path);
  if(prim) prim->bits = bits; 
}

// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

HdMeshTopology
Scene::GetMeshTopology(SdfPath const& id)
{
  if(IsMesh(id)) {
    Mesh* mesh = (Mesh*)_prims[id].geom;
    return HdMeshTopology(
      UsdGeomTokens->catmullClark,
      UsdGeomTokens->rightHanded,
      mesh->GetFaceCounts(),
      mesh->GetFaceConnects());
  }
  return HdMeshTopology();
}

HdBasisCurvesTopology
Scene::GetBasisCurvesTopology(SdfPath const& id)
{  
  if (IsCurves(id)) {
    Curve* curve = (Curve*)_prims[id].geom;
    return HdBasisCurvesTopology(
      HdTokens->linear,
      TfToken(),
      HdTokens->nonperiodic,
      curve->GetCvCounts(), VtArray<int>());
  }
  return HdBasisCurvesTopology();
}

GfRange3d 
Scene::GetExtent(SdfPath const& id)
{

  if (_prims.find(id) != _prims.end()) {
    Geometry* geometry = _prims[id].geom;
    if(geometry->GetType() < Geometry::POINT) {
      // TODO implicit geometry handling
    } else {
     return geometry->GetBoundingBox().GetRange();
    }
  }
 
  
  return GfRange3d();
}


GfMatrix4d
Scene::GetTransform(SdfPath const & id)
{
  if(_prims.find(id) != _prims.end())return _prims[id].geom->GetMatrix(); 
  return GfMatrix4d(1.0);
}

bool 
Scene::IsMesh(const SdfPath& id)
{
  return _prims[id].geom->GetType() == Geometry::MESH;
}

bool
Scene::IsCurves(const SdfPath& id)
{
  return _prims[id].geom->GetType() == Geometry::CURVE;
}

bool
Scene::IsPoints(const SdfPath& id)
{
  return _prims[id].geom->GetType() == Geometry::POINT;
}

const SdfPath
Scene::GetPrimPath(Geometry* geom) const
{
  for (auto& prim : _prims) {
    if(prim.second.geom == geom)return prim.first;
  }
  return SdfPath();
}


TfToken
Scene::GetRenderTag(SdfPath const& id)
{
  if(_prims.find(id)!=_prims.end()) {
    return HdRenderTagTokens->geometry;
  }
  return HdRenderTagTokens->hidden;
}

VtValue 
Scene::Get(SdfPath const& id, TfToken const& key)
{
  const _Prim& prim = _prims[id];
  const short type = prim.geom->GetType();

  if(type == Geometry::INSTANCER) {
    Instancer* instancer = (Instancer*)prim.geom;
    if (key == HdInstancerTokens->instanceTranslations) {
      return VtValue(instancer->GetPositions());
    } else if (key == HdTokens->displayColor) {
      return VtValue(instancer->GetColors());
    } else if (key == HdInstancerTokens->instanceIndexBase) {
      return VtValue(instancer->GetProtoIndices());
    } else if (key == HdTokens->indices) {
      return instancer->HaveIndices() ? 
        VtValue(instancer->GetIndices()) :  VtValue();
    } else if (key == HdInstancerTokens->instanceScales) {
      return VtValue(instancer->GetScales());
    } else if (key == HdInstancerTokens->instanceRotations) {
      return VtValue(instancer->GetRotations());
    } else {
    }
  } else {
    if (key == HdTokens->points) {
      return VtValue(((Deformable*)prim.geom)->GetPositions());
    } else if (key == HdTokens->displayColor) {
      VtArray<GfVec3f>& colors =
        ((Points*)prim.geom)->GetColors();
      if(colors.size())return VtValue(colors);
    } else if (key == HdTokens->widths) {
      return VtValue(((Points*)prim.geom)->GetWidths());
    } else if (key == HdTokens->normals) {
      if(((Deformable*)prim.geom)->HaveNormals()) {
        return VtValue(((Points*)prim.geom)->GetNormals());
      }
    }
  }
  return VtValue();
}


// -----------------------------------------------------------------------//
/// \name Materials
// -----------------------------------------------------------------------//

SdfPath 
Scene::GetMaterialId(SdfPath const &rprimId)
{
  SdfPath materialId;
  TfMapLookup(_materialBindings, rprimId, &materialId);
  return materialId;
}

VtValue 
Scene::GetMaterialResource(SdfPath const &materialId)
{
  if (VtValue *material = TfMapLookupPtr(_materials, materialId))
    return *material;
  
  return VtValue();
}


JVR_NAMESPACE_CLOSE_SCOPE