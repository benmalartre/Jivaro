
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../geometry/utils.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../geometry/sampler.h"
#include "../app/scene.h"
#include "../app/application.h"
#include "../app/commands.h"

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
  _prims[path] = { new Mesh() };
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
  auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* geometry = primIt->second.geom;
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
    return _prims[path].geom;
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
  pxr::VtVec3fArray points;
  if (_prims.find(id) != _prims.end()) {
    points = _prims[id].geom->GetPositions();
  }
 
  TF_FOR_ALL(it, points) {
    range.UnionWith(*it);
  }
  return range;
}


pxr::GfMatrix4d
Scene::GetTransform(pxr::SdfPath const & id)
{
    return pxr::GfMatrix4d(1);
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
  pxr::VtValue value;
  if (key == pxr::HdTokens->points) {
    // Each of the prim types hold onto their points
    return pxr::VtValue(_prims[id].geom->GetPositions());
  }
  else if (key == pxr::HdTokens->displayColor) {
    if (IsMesh(id)) {
      Mesh* mesh = (Mesh*)_prims[id].geom;
      pxr::VtArray<pxr::GfVec3f> colors(mesh->GetNumPoints());
      for (auto& color : colors)color = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
      return pxr::VtValue(colors);
    }
  } 
  else if (key == pxr::HdTokens->widths) {
    return pxr::VtValue(_prims[id].geom->GetRadius());
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

  if (interpolation == pxr::HdInterpolationVertex) {
    primvars.emplace_back(pxr::HdTokens->points, interpolation,
      pxr::HdPrimvarRoleTokens->point);

    primvars.emplace_back(pxr::HdTokens->displayColor, interpolation,
      pxr::HdPrimvarRoleTokens->color);
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
  pxr::UsdPrim control = stage->DefinePrim(rootPrim.GetPath().AppendChild(pxr::TfToken("Controls")));
  control.CreateAttribute(pxr::TfToken("Length"), pxr::SdfValueTypeNames->Float).Set(4.f);
  control.CreateAttribute(pxr::TfToken("Amplitude"), pxr::SdfValueTypeNames->Float).Set(1.f);
  control.CreateAttribute(pxr::TfToken("Frequency"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  size_t numStrands = 0;
  pxr::UsdPrimRange primRange = stage->TraverseAll();
  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken curveName(prim.GetName().GetString() + "RT");
      pxr::SdfPath curvePath(rootId.AppendChild(curveName));
      _sourcesMap[curvePath] = { prim.GetPath(), pxr::HdChangeTracker::Clean };
      Curve* curve = AddCurve(curvePath);
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
      numStrands += samples.size();

      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      curve->MaterializeSamples(samples, 4, &positions[0], &normals[0]);
      for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {
        curve->SetRadii(curveIdx, 1.f);
      }
      _samplesMap[curvePath] = samples;
    }
  }
}


void 
Scene::UpdateExec(double time)
{
  
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  pxr::UsdGeomXformCache xformCache(time);

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::UsdPrim controlPrim = rootPrim.GetChild(pxr::TfToken("Controls"));
  float length, amplitude, frequency;
  controlPrim.GetAttribute(pxr::TfToken("Length")).Get(&length);
  controlPrim.GetAttribute(pxr::TfToken("Amplitude")).Get(&amplitude);
  controlPrim.GetAttribute(pxr::TfToken("Frequency")).Get(&frequency);

  for (auto& execPrim : _prims) {
    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(_sourcesMap[execPrim.first].first);
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

    pxr::VtArray<pxr::GfVec3f>& points = execPrim.second.geom->GetPositions();
    const _Samples samples = _samplesMap[execPrim.first];
    for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
      const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
      const pxr::GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
      const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
      const pxr::GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
      points[sampleIdx * 4] = xform.Transform(position);
      points[sampleIdx * 4 + 1] = xform.Transform(position + normal * 0.33 * length + bitangent * pxr::GfCos(position[2] + time * frequency) * amplitude * 0.33);
      points[sampleIdx * 4 + 2] = xform.Transform(position + normal * 0.66 * length + bitangent * pxr::GfCos(position[2] + time * frequency) * amplitude * 0.66);
      points[sampleIdx * 4 + 3] = xform.Transform(position + normal * length + bitangent * pxr::GfCos(position[2] + time * frequency) * amplitude);
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