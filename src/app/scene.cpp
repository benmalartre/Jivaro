
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
  _meshes[path] = Mesh();
  return &_meshes[path];
}

Voxels* Scene::AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius)
{
  _voxels[path] = Voxels();
  Voxels* voxels = &_voxels[path];
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
  _curves[path] = Curve();
  return &_curves[path];
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  HdRenderIndex& index = GetRenderIndex();
  index.InsertRprim(pxr::HdPrimTypeTokens->points, this, path);
    _points[path] = Points();

  return &_points[path];
}

void Scene::Remove(const pxr::SdfPath & path)
{
  auto& meshIt = _meshes.find(path);
  if (meshIt != _meshes.end()) {
    HdRenderIndex& index = GetRenderIndex();
    index.RemoveRprim(path);
    _meshes.erase(meshIt);
    return;
  }

  auto& curvesIt = _curves.find(path);
  if (curvesIt != _curves.end()) {
    HdRenderIndex& index = GetRenderIndex();
    index.RemoveRprim(path);
    _curves.erase(curvesIt);
    return;
  }

  auto& pointsIt = _points.find(path);
  if (pointsIt != _points.end()) {
    HdRenderIndex& index = GetRenderIndex();
    index.RemoveRprim(path);
    _points.erase(pointsIt);
    return;
  }

  auto& voxelsIt = _voxels.find(path);
  if (voxelsIt != _voxels.end()) {
    HdRenderIndex& index = GetRenderIndex();
    index.RemoveRprim(path);
    _voxels.erase(voxelsIt);
    return;
  }
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
  if (_meshes.find(path) != _meshes.end()) {
    return(Geometry*)& _meshes[path];
  } else if (_curves.find(path) != _curves.end()) {
    return(Geometry*)&_curves[path];
  }if (_points.find(path) != _points.end()) {
    return(Geometry*)&_points[path];
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
  if(_meshes.find(id) != _meshes.end()) {
    Mesh* mesh = &_meshes[id];
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
  if(_curves.find(id) != _curves.end()) {
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
  if (_meshes.find(id) != _meshes.end()) {
    points = _meshes[id].GetPositions();
  }
  else if (_curves.find(id) != _curves.end()) {
    points = _curves[id].GetPositions();
  }
  else if (_points.find(id) != _points.end()) {
    points = _points[id].GetPositions();
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
  return (_meshes.find(id) != _meshes.end());
}

bool
Scene::IsCurves(const pxr::SdfPath& id)
{
  return (_curves.find(id) != _curves.end());
}

bool
Scene::IsPoints(const pxr::SdfPath& id)
{
  return (_points.find(id) != _points.end());
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
  /*if (_hiddenRprims.find(id) != _hiddenRprims.end()) {
    return HdRenderTagTokens->hidden;
  }*/

  if (Mesh* mesh = TfMapLookupPtr(_meshes, id)) {
    return HdRenderTagTokens->geometry;
  }
  else if (_curves.count(id) > 0) {
    return HdRenderTagTokens->geometry;
  }
  else if (_points.count(id) > 0) {
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
    if (IsMesh(id)) {
      return VtValue(_meshes[id].GetPositions());
    }
    else if (IsCurves(id)) {
      return VtValue(_curves[id].GetPositions());
    }
    else if (IsPoints(id)) {
      return VtValue(_points[id].GetPositions());
    }
  }
  else if (key == pxr::HdTokens->displayColor) {
    if (IsMesh(id)) {
      Mesh* mesh = &_meshes[id];
      pxr::VtArray<pxr::GfVec3f> colors(mesh->GetNumPoints());
      for (auto& color : colors)color = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
      return VtValue(colors);
    }
  }
  /*
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
    if (IsMesh(id))return VtValue(_meshes[id].GetPositions());
    else if (IsCurves(id)) return VtValue(_curves[id].GetPositions());
    else if (IsPoints(id)) return VtValue(_points[id].GetPositions());
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