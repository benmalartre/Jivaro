
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../command/router.h"
#include "../command/block.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../app/scene.h"

JVR_NAMESPACE_OPEN_SCOPE

Scene::Scene(pxr::HdRenderIndex* parentIndex, pxr::SdfPath const& delegateID)
  : HdSceneDelegate(parentIndex, delegateID)
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
Scene::Save(const std::string& filename)
{

}

void 
Scene::Export(const std::string& filename)
{
  
}

void
Scene::Update(double time)
{
  /*
  for (auto& meshMapIt : _meshes) {
    pxr::UsdGeomMesh mesh(_stage->GetPrimAtPath(meshMapIt.first));
    mesh.GetPointsAttr().Set(meshMapIt.second.GetPositions(), time);
  }
  */
}

Mesh* Scene::AddMesh(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  HdRenderIndex& index = GetRenderIndex();
  index.InsertRprim(pxr::HdPrimTypeTokens->mesh, this, path);
  _prims[path] = new Mesh();
  return (Mesh*)_prims[path];
}

Voxels* Scene::AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius)
{
  _prims[path] = new Voxels();
  Voxels* voxels = (Voxels*)_prims[path];
  voxels->Init(mesh, radius);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build();
  return voxels;
}

Curve* Scene::AddCurve(const pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
{
  HdRenderIndex& index = GetRenderIndex();
  index.InsertRprim(pxr::HdPrimTypeTokens->basisCurves, this, path);
  _prims[path] = new Curve();
  return (Curve*)_prims[path];
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  HdRenderIndex& index = GetRenderIndex();
  index.InsertRprim(pxr::HdPrimTypeTokens->points, this, path);
    _prims[path] = new Points();

  return (Points*)_prims[path];
}

Geometry* Scene::Remove(const pxr::SdfPath & path)
{
  auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* result = primIt->second;
    HdRenderIndex& index = GetRenderIndex();
    index.RemoveRprim(path);
    _prims.erase(primIt);
    return result;
  }
  return NULL;
}


void Scene::TestVoronoi()
{
  pxr::SdfPath path("/Voronoi");
  Mesh mesh;
  std::vector<pxr::GfVec3f> points(1024);
  for (auto& point : points) {
    point[0] = (float)rand() / (float)RAND_MAX - 0.5f;
    point[1] = 0.f;
    point[2] = (float)rand() / (float)RAND_MAX - 0.5f;
  }
  mesh.VoronoiDiagram(points);
}

Geometry*
Scene::GetGeometry(const pxr::SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return _prims[path];
  }
  return NULL;
}


/*virtual*/
bool
Scene::IsEnabled(pxr::TfToken const& option) const
{
    if (option == pxr::HdOptionTokens->parallelRprimSync) {
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

pxr::HdMeshTopology
Scene::GetMeshTopology(pxr::SdfPath const& id)
{
  if(_prims.find(id) != _prims.end() && _prims[id]->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)_prims[id];
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
  if (_prims.find(id) != _prims.end() && _prims[id]->GetType() == Geometry::CURVE) {
    //return _curves.find(id).GetTopology();
  }
  return pxr::HdBasisCurvesTopology();
}

pxr::PxOsdSubdivTags
Scene::GetSubdivTags(pxr::SdfPath const& id)
{
    return pxr::PxOsdSubdivTags();
}

pxr::GfRange3d 
Scene::GetExtent(pxr::SdfPath const& id)
{
  GfRange3d range;
  VtVec3fArray points;
  if (_prims.find(id) != _prims.end()) {
    points = _prims[id]->GetPositions();
  }
 
  TF_FOR_ALL(it, points) {
    range.UnionWith(*it);
  }
  return range;
}


pxr::GfMatrix4d
Scene::GetTransform(SdfPath const & id)
{
    return GfMatrix4d(1);
}

size_t
Scene::SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  VtValue* sampleValues)
{
  return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
    nullptr);
}

/*virtual*/
size_t
Scene::SampleIndexedPrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues,
  pxr::VtIntArray* sampleIndices)
{
  return _SamplePrimvar(id, key, maxNumSamples, sampleTimes, sampleValues,
    sampleIndices);
}

size_t
Scene::_SamplePrimvar(pxr::SdfPath const& id,
  pxr::TfToken const& key,
  size_t maxNumSamples,
  float* sampleTimes,
  pxr::VtValue* sampleValues,
  pxr::VtIntArray* sampleIndices)
{
  /*
  //pxr::SdfPath cachePath = ConvertIndexPathToCachePath(id);
  _HdPrimInfo* primInfo = _GetHdPrimInfo(id);
  if (*TF_VERIFY(primInfo)) {
    if (sampleIndices) {
      sampleIndices[0] = VtIntArray(0);
    }
    // Retrieve the multi-sampled result.
    size_t nSamples = primInfo->adapter
      ->SamplePrimvar(primInfo->usdPrim, id, key,
        pxr::UsdTimeCode::Default(), maxNumSamples, sampleTimes, sampleValues,
        sampleIndices);
    return nSamples;
  }
  */
  return 0;

}

bool 
Scene::IsMesh(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id]->GetType() == Geometry::MESH);
}

bool
Scene::IsCurves(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id]->GetType() == Geometry::CURVE);
}

bool
Scene::IsPoints(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id]->GetType() == Geometry::POINT);
}

bool
Scene::GetVisible(pxr::SdfPath const & id)
{
    return true;
}

/*virtual*/
bool
Scene::GetDoubleSided(pxr::SdfPath const & id)
{
    return false;
}

/*virtual*/
pxr::HdCullStyle
Scene::GetCullStyle(pxr::SdfPath const &id)
{
  return pxr::HdCullStyleNothing;
}

/*virtual*/
pxr::VtValue
Scene::GetShadingStyle(pxr::SdfPath const &id)
{
    return VtValue();
}

/*virtual*/
pxr::HdDisplayStyle
Scene::GetDisplayStyle(pxr::SdfPath const& id)
{
    return pxr::HdDisplayStyle();
}

pxr::TfToken
Scene::GetRenderTag(pxr::SdfPath const& id)
{

  if(_prims.find(id)!=_prims.end()) {
    return HdRenderTagTokens->geometry;
  }
  return HdRenderTagTokens->hidden;
}


pxr::VtValue
Scene::Get(pxr::SdfPath const& id, TfToken const& key)
{
  VtValue value;
  if (key == pxr::HdTokens->points) {
    // Each of the prim types hold onto their points
    return VtValue(_prims[id]->GetPositions());
  }
  /*
  else if (key == pxr::HdTokens->displayColor) {
    if (IsMesh(id)) {
      Mesh* mesh = &_meshes[id];
      pxr::VtArray<pxr::GfVec3f> colors(mesh->GetNumPoints());
      for (auto& color : colors)color = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
      return VtValue(colors);
    }
  }
  
  else if (key == HdInstancerTokens->scale) {
    if (_instancers.find(id) != _instancers.end()) {
      return VtValue(_instancers[id].scale);
    }
  }
  else if (key == HdInstancerTokens->rotate) {
    if (_instancers.find(id) != _instancers.end()) {
      return VtValue(_instancers[id].rotate);
    }
  }
  else if (key == HdInstancerTokens->translate) {
    if (_instancers.find(id) != _instancers.end()) {
      return VtValue(_instancers[id].translate);
    }
  }
  
  else {
    // Check if key is a primvar
    _Primvars::iterator pvIt;
    if (_FindPrimvar(id, key, &pvIt)) {
      if (pvIt->indices.empty()) {
        value = pvIt->value;
      }
      else {
        // Flatten primvar
        value = pvIt->value;
        if (value.IsArrayValued()) {
          value = _ComputeFlattenedValue(value, pvIt->indices);
        }
      }

    }
  }*/
  return value;
}


/*virtual*/
pxr::VtValue
Scene::GetIndexedPrimvar(pxr::SdfPath const& id, pxr::TfToken const& key, 
                                        pxr::VtIntArray *outIndices) 
{
  if (key == HdTokens->points) {
    // Each of the prim types hold onto their points
    return VtValue(_prims[id]->GetPositions());
  }
  /*
  else {
    // Check if key is a primvar
    _Primvars::iterator pvIt;
    if (_FindPrimvar(id, key, &pvIt)) {
      value = pvIt->value;
      if (outIndices) {
        *outIndices = pvIt->indices;
      }
    }
  }
  */
  return pxr::VtValue();
}


pxr::VtArray<pxr::TfToken>
Scene::GetCategories(pxr::SdfPath const& id)
{
    return pxr::VtArray<pxr::TfToken>();
}

std::vector<pxr::VtArray<pxr::TfToken>>
Scene::GetInstanceCategories(pxr::SdfPath const &instancerId)
{
    return std::vector<pxr::VtArray<pxr::TfToken>>();
}

pxr::HdIdVectorSharedPtr
Scene::GetCoordSysBindings(pxr::SdfPath const& id)
{
    return nullptr;
}

// -------------------------------------------------------------------------- //
// Primvar Support Methods
// -------------------------------------------------------------------------- //

pxr::HdPrimvarDescriptorVector Scene::GetPrimvarDescriptors(pxr::SdfPath const& id,
  pxr::HdInterpolation interpolation)
{
  pxr::HdPrimvarDescriptorVector primvars;

  if (interpolation == HdInterpolationVertex) {
    primvars.emplace_back(HdTokens->points, interpolation,
      HdPrimvarRoleTokens->point);

    primvars.emplace_back(HdTokens->displayColor, interpolation,
      HdPrimvarRoleTokens->color);
  }
  
  /*
  if (interpolation == HdInterpolationInstance && _hasInstancePrimvars &&
    _instancers.find(id) != _instancers.end()) {
    primvars.emplace_back(HdInstancerTokens->scale, interpolation);
    primvars.emplace_back(HdInstancerTokens->rotate, interpolation);
    primvars.emplace_back(HdInstancerTokens->translate, interpolation);
  }
  

  auto const cit = _primvars.find(id);
  if (cit != _primvars.end()) {
    _Primvars const& pvs = cit->second;
    for (auto const& pv : pvs) {
      if (pv.interp == interpolation) {
        primvars.emplace_back(pv.name, pv.interp, pv.role,
          !pv.indices.empty());
      }
    }
  }
  */

  return primvars;
}
JVR_NAMESPACE_CLOSE_SCOPE