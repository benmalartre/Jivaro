
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
#include "../geometry/sampler.h"
#include "../app/scene.h"
#include "../app/application.h"

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
}

Mesh* Scene::AddMesh(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  //HdRenderIndex& index = GetRenderIndex();
  //index.InsertRprim(pxr::HdPrimTypeTokens->mesh, this, path);
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
  //HdRenderIndex& index = GetRenderIndex();
  //index.InsertRprim(pxr::HdPrimTypeTokens->basisCurves, this, path);
  _prims[path] = new Curve();
  return (Curve*)_prims[path];
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  //HdRenderIndex& index = GetRenderIndex();
  //index.InsertRprim(pxr::HdPrimTypeTokens->points, this, path);
    _prims[path] = new Points();

  return (Points*)_prims[path];
}

void Scene::Remove(const pxr::SdfPath & path)
{
  auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* geometry = primIt->second;
    //HdRenderIndex& index = GetRenderIndex();
    //index.RemoveRprim(path);
    _prims.erase(primIt);
    delete geometry;
  }
}

Geometry*
Scene::GetGeometry(const pxr::SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return _prims[path];
  }
  return NULL;
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
  else if (key == pxr::HdTokens->displayColor) {
    if (IsMesh(id)) {
      Mesh* mesh = (Mesh*)_prims[id];
      pxr::VtArray<pxr::GfVec3f> colors(mesh->GetNumPoints());
      for (auto& color : colors)color = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
      return VtValue(colors);
    }
  }
  return value;
}


/*virtual*/
pxr::VtValue
Scene::GetIndexedPrimvar(pxr::SdfPath const& id, pxr::TfToken const& key, 
                                        pxr::VtIntArray *outIndices) 
{
  return pxr::VtValue();
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

void
Scene::InitExec()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));


  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _srcMap[meshPath] = prim.GetPath();
      Mesh* mesh = AddMesh(meshPath);
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::VtArray<pxr::GfVec3f> positions;
      usdMesh.GetPointsAttr().Get(&positions);

      pxr::VtArray<int> counts;
      pxr::VtArray<int> indices;
      usdMesh.GetFaceVertexCountsAttr().Get(&counts);
      usdMesh.GetFaceVertexIndicesAttr().Get(&indices);
      pxr::VtArray<Triangle> triangles;
      TriangulateMesh(counts, indices, triangles);
      pxr::VtArray<pxr::GfVec3f> normals;
      ComputeVertexNormals(positions, counts, indices, triangles, normals);
      pxr::VtArray<Sample> samples;
      PoissonSampling(0.1, 12000, positions, normals, triangles, samples);

      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);

      pxr::VtArray<pxr::GfVec3f> points(samples.size());
      for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
        points[sampleIdx] = xform.Transform(samples[sampleIdx].GetPosition(&positions[0]));
      }
      mesh->MaterializeSamples(points, 2.f);
      _samplesMap[meshPath] = samples;
    }
  }
}


void 
Scene::UpdateExec(double time)
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  pxr::UsdGeomXformCache xformCache(time);
  for (auto& execPrim : _prims) {
    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(_srcMap[execPrim.first]);
    pxr::UsdGeomMesh usdMesh(usdPrim);
    pxr::VtArray<pxr::GfVec3f> positions;
    usdMesh.GetPointsAttr().Get(&positions, pxr::UsdTimeCode(time));

    pxr::VtArray<int> counts;
    pxr::VtArray<int> indices;
    usdMesh.GetFaceVertexCountsAttr().Get(&counts);
    usdMesh.GetFaceVertexIndicesAttr().Get(&indices);
    pxr::VtArray<Triangle> triangles;
    TriangulateMesh(counts, indices, triangles);
    pxr::VtArray<pxr::GfVec3f> normals;
    ComputeVertexNormals(positions, counts, indices, triangles, normals);

    pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(usdPrim);

    pxr::VtArray<pxr::GfVec3f>& points = execPrim.second->GetPositions();
    const _Samples samples = _samplesMap[execPrim.first];
    for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
      const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
      const pxr::GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
      const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
      const pxr::GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
      points[sampleIdx * 3] = xform.Transform(position - tangent * 0.02f);
      points[sampleIdx * 3 + 1] = xform.Transform(position + bitangent * pxr::GfCos(position[1] + time * 0.5) + normal * 0.4f * (1.5f + pxr::GfSin(position[2] * 0.4 + time * 0.2) * 0.5f));
      points[sampleIdx * 3 + 2] = xform.Transform(position + tangent * 0.02f);
    }
  }
}

void 
Scene::TerminateExec()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      Remove(meshPath);
    }
  }
}
JVR_NAMESPACE_CLOSE_SCOPE