
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/curves.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/sdf/fileFormat.h>


#include "../utils/strings.h"
#include "../utils/files.h"
#include "../geometry/sampler.h"
#include "../app/workspace.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../command/router.h"
#include "../command/block.h"
#include "../acceleration/bvh.h"

JVR_NAMESPACE_OPEN_SCOPE

Workspace::Workspace() 
  : _execInitialized(false)
  , _execScene(NULL)
  , _execStage(NULL)
{
  _workStage = pxr::UsdStage::CreateInMemory("work");
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
}

Workspace::~Workspace()
{
  if(_execScene)delete _execScene;
  ClearStageCache();
  _workStage = nullptr;
}

bool 
Workspace::HasUnsavedWork()
{
  for (const auto& layer : SdfLayer::GetLoadedLayers()) {
    if (layer && layer->IsDirty() && !layer->IsAnonymous()) {
      return true;
    }
  }
  return false;
}

void 
Workspace::SetWorkStage(pxr::UsdStageCache::Id current)
{
  SetWorkStage(_stageCache.Find(current));
}

void 
Workspace::SetWorkStage(pxr::UsdStageRefPtr stage)
{
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  // NOTE: We set the default layer to the current stage root
  // this might have side effects
  if (!GetWorkLayer() && _workStage) {
    SetWorkLayer(_workStage->GetRootLayer());
  }
}

void 
Workspace::SetWorkLayer(SdfLayerRefPtr layer) 
{
  if (!layer)
    return;
  if (!_layerHistory.empty()) {
    if (GetWorkLayer() != layer) {
      if (_layerHistoryPointer < _layerHistory.size() - 1) {
        _layerHistory.resize(_layerHistoryPointer + 1);
      }
      _layerHistory.push_back(layer);
      _layerHistoryPointer = _layerHistory.size() - 1;
    }
  }
  else {
    _layerHistory.push_back(layer);
    _layerHistoryPointer = _layerHistory.size() - 1;
  }
}

void 
Workspace::SetWorkEditTarget(SdfLayerHandle layer) 
{
  if (GetWorkStage()) {
    GetWorkStage()->SetEditTarget(UsdEditTarget(layer));
  }
}

SdfLayerRefPtr 
Workspace::GetWorkLayer() 
{
  return _layerHistory.empty() ? SdfLayerRefPtr() : _layerHistory[_layerHistoryPointer];
}

void 
Workspace::SetPreviousLayer() 
{
  if (_layerHistoryPointer > 0) {
    _layerHistoryPointer--;
  }
}


void 
Workspace::SetNextLayer() 
{
  if (_layerHistoryPointer < _layerHistory.size() - 1) {
    _layerHistoryPointer++;
  }
}


void 
Workspace::UseLayer(SdfLayerRefPtr layer)
{
  if (layer) {
    SetWorkLayer(layer);
    //_settings._showContentBrowser = true;
  }
}


void 
Workspace::CreateLayer(const std::string& path)
{
  auto newLayer = SdfLayer::CreateNew(path);
  UseLayer(newLayer);
}

void 
Workspace::ImportLayer(const std::string& path) 
{
  auto newLayer = SdfLayer::FindOrOpen(path);
  UseLayer(newLayer);
}

//
void 
Workspace::ImportStage(const std::string& path, bool openLoaded) 
{
  auto newStage = UsdStage::Open(path, openLoaded ? UsdStage::LoadAll : UsdStage::LoadNone); // TODO: as an option
  if (newStage) {
    _stageCache.Insert(newStage);
    SetWorkStage(newStage);
    //_settings._showContentBrowser = true;
    //_settings._showViewport = true;
    //UpdateRecentFiles(_settings._recentFiles, path);
  }
}

void 
Workspace::SaveCurrentLayerAs(const std::string& path)
{
  auto newLayer = SdfLayer::CreateNew(path);
  if (newLayer && GetWorkLayer()) {
    newLayer->TransferContent(GetWorkLayer());
    newLayer->Save();
    UseLayer(newLayer);
  }
}

void 
Workspace::CreateStage(const std::string& path)
{
  auto usdaFormat = pxr::SdfFileFormat::FindByExtension("usda");
  auto layer = SdfLayer::New(usdaFormat, path);
  if (layer) {
    auto newStage = UsdStage::Open(layer);
    if (newStage) {
      _stageCache.Insert(newStage);
      SetWorkStage(newStage);
      //_settings._showContentBrowser = true;
      //_settings._showViewport = true;
    }
  }
}

void Workspace::ClearStageCache() 
{
  _stageCache.Clear();
}


void
Workspace::OpenStage(const std::string& filename)
{
  _workStage = pxr::UsdStage::Open(filename);
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  _stageCache.Insert(_workStage);
}

void
Workspace::OpenStage(const pxr::UsdStageRefPtr& stage)
{
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromMemory(const std::string& name)
{
  /*
  _stageCache.Find()
  _alStages[path] = pxr::UsdStage::CreateInMemory(name);
  return  _allStages[path];
  */
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory(name);
  UndoRouter::Get().TrackLayer(stage->GetRootLayer());
  _stageCache.Insert(stage);
  return stage;
}

pxr::UsdStageRefPtr&
Workspace::AddStageFromDisk(const std::string& filename)
{
  
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  UndoBlock editBlock;
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  _stageCache.Insert(stage);
  pxr::SdfLayerHandle layer = stage->GetRootLayer();
  _workStage = stage;
  UndoRouter::Get().TrackLayer(_workStage->GetRootLayer());
  //_workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  //_workStage->SetDefaultPrim(stage->GetDefaultPrim());
  return stage;
  /*
  std::vector<std::string> tokens = SplitString(GetFileName(filename), ".");
  std::string name = tokens.front();
  pxr::SdfPath path("/" + name);
  pxr::SdfLayerRefPtr layer = pxr::SdfLayer::FindOrOpen(filename);
  pxr::SdfLayerRefPtr sessionLayer = pxr::SdfLayer::CreateAnonymous();
  _allStages[path] = pxr::UsdStage::Open(layer, sessionLayer);
  _workStage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
  return _allStages[path];
 */
}

pxr::SdfLayerHandle 
Workspace::AddLayerFromDisk(const std::string& filename)
{
  pxr::SdfLayerHandle handle = pxr::SdfLayer::FindOrOpen(filename);
  return handle;
}

pxr::UsdStageRefPtr&
Workspace::AddExecStage()
{
  _execStage = NULL;
  _execInitialized = false;

  InitExec();
  return _execStage;
}

void
Workspace::RemoveExecStage()
{
  _execStage = NULL;
  if (_execScene) {
    delete _execScene;
    _execScene = NULL;
  }
  _execInitialized = false;
}

void 
Workspace::InitExec()
{

  if (!_execInitialized) {
    _execStage = UsdStage::CreateInMemory("exec");
    _execScene = new Scene(_execStage);
    _solver = new PBDSolver();
   
    Time& time = GetApplication()->GetTime();
    _startFrame = time.GetStartTime();
    _lastFrame = time.GetActiveTime();
    
    _execStage->GetRootLayer()->TransferContent(_workStage->GetRootLayer());

    pxr::UsdGeomXformCache xformCache(_startFrame);

    pxr::UsdPrimRange primRange = _execStage->TraverseAll();
    for (pxr::UsdPrim prim : primRange) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        _execScene->AddMesh(prim.GetPath());
        _solver->AddGeometry(&_execScene->GetMeshes()[prim.GetPath()], 
          pxr::GfMatrix4f(xformCache.GetLocalToWorldTransform(prim)));
      }
    }

    std::vector<Geometry*> colliders;
    for (auto& mesh : _execScene->GetMeshes()) {
      colliders.push_back(&mesh.second);
    }
    _solver->AddColliders(colliders);
    _execStage->SetDefaultPrim(_execStage->GetPrimAtPath(
      _workStage->GetDefaultPrim().GetPath()));

    pxr::UsdGeomPoints points =
      pxr::UsdGeomPoints::Define(
        _execStage, 
        _execStage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("System")));

    PBDParticle* system = _solver->GetSystem();
    points.CreatePointsAttr().Set(pxr::VtValue(system->GetPositions()));

    size_t numParticles = system->GetNumParticles();
    
    pxr::VtArray<float> widths(numParticles);
    pxr::VtArray<pxr::GfVec3f> colors(numParticles);
    for(size_t p = 0; p < numParticles; ++p) {
      widths[p] = 0.02;
      colors[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }
    points.CreateWidthsAttr().Set(pxr::VtValue(widths));
    points.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
    points.CreateDisplayColorAttr().Set(pxr::VtValue(colors));
    points.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);
    
    _execInitialized = true;
    std::cout << "exec initialized..." << std::endl;
  }
}

void 
Workspace::UpdateExec(double time)
{
  /*
  for (auto& meshMapIt : _execScene->GetMeshes()) {
    pxr::SdfPath path = meshMapIt.first;
    Mesh* mesh = &meshMapIt.second;
    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::UsdGeomMesh input(_workStage->GetPrimAtPath(path));

    double t = pxr::GfSin(GetApplication()->GetTime().GetActiveTime() * 100);

    input.GetPointsAttr().Get(&positions, time);
    for (auto& position : positions) {
      position += pxr::GfVec3f(
        pxr::GfSin(position[0] + t),
        pxr::GfCos(position[0] + t) * 5.0,
        RANDOM_0_1 * 0.05
      );
    }
    mesh->Update(positions);
  }
  */
  if (time <= _startFrame) {
    _solver->Reset();
  }
  if(time > _lastFrame) {
    _solver->Step();
    _execScene->Update(time);
  }
  _lastFrame = (float)time;
}

void 
Workspace::TerminateExec()
{
  std::cout << "TERMINATE EXEC " << std::endl;
  delete _solver;
}

struct DebugRay {
  pxr::UsdGeomBasisCurves curves;
  pxr::UsdGeomPoints intersections;

  std::vector<pxr::GfRay> rays;
};

void InitializeDebugRay(pxr::UsdStageRefPtr& stage, DebugRay& data)
{
  pxr::UsdGeomXform rayGroup =
    pxr::UsdGeomXform::Define(stage, stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("rays")));

  data.intersections  =
    pxr::UsdGeomPoints::Define(stage, rayGroup.GetPath().AppendChild(pxr::TfToken("intersections")));

  data.curves =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("ray_curve")));

  data.curves.CreatePointsAttr();
  data.curves.CreateCurveVertexCountsAttr();

  pxr::UsdGeomPrimvar curvesColor = data.curves.CreateDisplayColorPrimvar();
  curvesColor.SetInterpolation(pxr::UsdGeomTokens->constant);
  curvesColor.SetElementSize(1);
}

void UpdateDebugRay(pxr::UsdStageRefPtr& stage, DebugRay& data)
{

}

static void
_SetupRays(pxr::UsdStageRefPtr& stage, std::vector<pxr::GfRay>& rays)
{

  pxr::UsdGeomXform rayGroup =
    pxr::UsdGeomXform::Define(stage, stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("rays")));

  size_t rayIndex = 0;
  pxr::VtArray<pxr::GfVec3f> points(rays.size() * 2);
  pxr::VtArray<int> curveVertexCount(rays.size());
  pxr::VtArray<pxr::GfVec3f> colors(1);
  
  std::string name = "ray_origin_";
  for (auto& ray : rays) {
    colors[0] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

    pxr::UsdGeomSphere origin = 
      pxr::UsdGeomSphere::Define(stage, rayGroup.GetPath().AppendChild(
        pxr::TfToken(name + std::to_string(rayIndex))
      ));
    
    origin.AddTranslateOp().Set(ray.GetPoint(0));
    origin.CreateRadiusAttr().Set(0.05);
    origin.CreateDisplayColorAttr().Set(colors);
    points[rayIndex * 2] = pxr::GfVec3f(ray.GetPoint(0));
    points[rayIndex * 2 + 1] = pxr::GfVec3f(ray.GetPoint(10));
    curveVertexCount[rayIndex] = 2;

    rayIndex++;

  }
  
  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("ray_curve")));

  curve.CreatePointsAttr().Set(points);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  pxr::UsdGeomPrimvar colorPrimvar = curve.CreateDisplayColorPrimvar();
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->constant);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);
  
}



static void
_SetupFlip(pxr::UsdStageRefPtr& stage)
{
  std::cout << "SETUP FLIP !!!" << std::endl;
  pxr::UsdGeomMesh mesh =
    pxr::UsdGeomMesh::Define(stage, stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("mesh")));

  pxr::VtArray<pxr::GfVec3f> points = {
    {-1, 0, -1}, { 1, 0, -1}, {-1, 0,  1}, { 1, 0,  1}
  };
  mesh.CreatePointsAttr().Set(points);
  pxr::VtArray<int> indices = {
    0, 1, 3, 3, 2, 0
  };
  mesh.CreateFaceVertexIndicesAttr().Set(indices);
  pxr::VtArray<int> counts = {
    3, 3
  };
  mesh.CreateFaceVertexCountsAttr().Set(counts);

  Mesh* __mesh = new Mesh(mesh);
  __mesh->FlipEdge(0);
  __mesh->FlipEdge(0);
  __mesh->FlipEdge(0);
  __mesh->UpdateTopologyFromHalfEdges();

  mesh.GetFaceVertexCountsAttr().Set(__mesh->GetFaceCounts());
  mesh.GetFaceVertexIndicesAttr().Set(__mesh->GetFaceConnects());
}

static void 
_SetupSampler(pxr::UsdStageRefPtr& stage, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;

  pxr::VtArray<Sampler::Sample> samples;
  pxr::VtArray<int> triangles(mesh->GetNumTriangles() * 3);
  for (auto& triangle : mesh->GetTriangles()) {
    triangles[triangle.id * 3 + 0] = triangle.vertices[0];
    triangles[triangle.id * 3 + 1] = triangle.vertices[1];
    triangles[triangle.id * 3 + 2] = triangle.vertices[2];
  }
  Sampler::PoissonSampling(0.2f, 1000000, mesh->GetPositions(), mesh->GetNormals(), triangles, samples);

  size_t numSamples = samples.size();
  pxr::VtArray<pxr::GfVec3f> points(numSamples);
  pxr::VtArray<pxr::GfVec3f> scales(numSamples);
  pxr::VtArray<int64_t> indices(numSamples);
  pxr::VtArray<int> protoIndices(numSamples);
  pxr::VtArray<pxr::GfQuath> rotations(numSamples);
  pxr::VtArray<pxr::GfVec3f> colors(numSamples);

  for (size_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
    points[sampleIdx] = samples[sampleIdx].GetPosition(mesh->GetPositionsCPtr());
    scales[sampleIdx] = pxr::GfVec3f(0.2);
    protoIndices[sampleIdx] = 0;
    indices[sampleIdx] = sampleIdx;
    rotations[sampleIdx] = pxr::GfQuath::GetIdentity();
    colors[sampleIdx] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  }

  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("sampler_instancer")));

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(pxr::TfToken("proto_instance")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  pxr::UsdGeomPrimvarsAPI primvarsApi(instancer);
  pxr::UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(pxr::UsdGeomTokens->primvarsDisplayColor, pxr::SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);


}

static void
_SetupHairs(pxr::UsdStageRefPtr& stage, Geometry* geometry)
{
  if (geometry->GetType() != Geometry::MESH)return;
  Mesh* mesh = (Mesh*)geometry;

  pxr::VtArray<Sampler::Sample> samples;
  pxr::VtArray<int> triangles(mesh->GetNumTriangles() * 3);
  for (auto& triangle : mesh->GetTriangles()) {
    triangles[triangle.id * 3 + 0] = triangle.vertices[0];
    triangles[triangle.id * 3 + 1] = triangle.vertices[1];
    triangles[triangle.id * 3 + 2] = triangle.vertices[2];
  }
  Sampler::PoissonSampling(0.2f, 1000000, mesh->GetPositions(), mesh->GetNormals(), triangles, samples);

  size_t N = 32; // num cvs per curve
  size_t numSamples = samples.size();
  pxr::VtArray<pxr::GfVec3f> points(numSamples * N);
  pxr::VtArray<pxr::GfVec3f> scales(numSamples * N);
  pxr::VtArray<float> widths(numSamples * N);
  pxr::VtArray<int> curveVertexCount(numSamples);

  for (size_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
    const pxr::GfVec3f& origin = samples[sampleIdx].GetPosition(mesh->GetPositionsCPtr());
    const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(mesh->GetNormalsCPtr());
    curveVertexCount[sampleIdx] = N;
    for (int n = 0; n < N; ++n) {
      const float t = (float)n / (float)N;
      points[sampleIdx * N + n] = origin + normal * 0.1f * n + pxr::GfVec3f(RANDOM_LO_HI(-0.5,0.5)) * t;
      scales[sampleIdx * N + n] = pxr::GfVec3f(RANDOM_LO_HI(0.1,0.2));
      widths[sampleIdx * N + n] = RANDOM_LO_HI(0.1, 0.2);
    }
  }

  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("hair_curve")));

  curve.CreatePointsAttr().Set(points);
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->constant);
  curve.CreateWidthsAttr().Set(widths);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  pxr::UsdGeomPrimvar crvColorPrimvar = curve.CreateDisplayColorPrimvar();
  crvColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
  crvColorPrimvar.SetElementSize(1);
  //crvColorPrimvar.Set(colors);

  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);


}

static void
_SetupGroom(pxr::UsdStageRefPtr& stage)
{

  size_t N = 32; // num cvs per curve
  size_t numCurves = 16;
  pxr::VtArray<pxr::GfVec3f> points(numCurves * N);
  pxr::VtArray<float> widths(numCurves * N);
  pxr::VtArray<int> curveVertexCount(numCurves);

  for (size_t curveIdx = 0; curveIdx < numCurves; ++curveIdx) {
    pxr::GfVec3f origin(curveIdx, 0, 0);
    pxr::GfVec3f normal(0, 1, 0);
    curveVertexCount[curveIdx] = N;
    for (int n = 0; n < N; ++n) {
      const float t = (float)n / (float)N;
      points[curveIdx * N + n] = origin + normal * 0.1f * n + pxr::GfVec3f(RANDOM_LO_HI(-0.5,0.5)) * t;
      widths[curveIdx * N + n] = RANDOM_LO_HI(0.1, 0.2);
    }
  }

  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("hair_curve")));

  curve.CreatePointsAttr().Set(points);
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->constant);
  curve.CreateWidthsAttr().Set(widths);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  pxr::UsdGeomPrimvar crvColorPrimvar = curve.CreateDisplayColorPrimvar();
  crvColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
  crvColorPrimvar.SetElementSize(1);
  //crvColorPrimvar.Set(colors);

  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);


}

static void
_SetupBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh)
{
  std::vector<BVH::Cell*> cells;
  std::cout << "setup instancer " << bvh->GetRoot() << std::endl;
  bvh->GetRoot()->GetCells(cells);
  size_t numPoints = cells.size();
  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  pxr::VtArray<int> curveVertexCount(1);
  curveVertexCount[0] = numPoints;
  pxr::VtArray<float> widths(numPoints);
  widths[0] = 1.f;

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    colors[pointIdx] =
      pxr::GfVec3f(bvh->ComputeCodeAsColor(bvh->GetRoot(),
        pxr::GfVec3f(cells[pointIdx]->GetMidpoint())));
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
    widths[pointIdx] = RANDOM_0_1;
  }

  /*
  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_instancer")));

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(pxr::TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  pxr::UsdGeomPrimvarsAPI primvarsApi(instancer);
  pxr::UsdGeomPrimvar colorPrimvar = 
    primvarsApi.CreatePrimvar(pxr::UsdGeomTokens->primvarsDisplayColor, pxr::SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);
  */

  pxr::UsdGeomBasisCurves curve =
    pxr::UsdGeomBasisCurves::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_curve")));

  curve.CreatePointsAttr().Set(points);
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->constant);
  curve.CreateWidthsAttr().Set(widths);
  curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

  pxr::UsdGeomPrimvar crvColorPrimvar = curve.CreateDisplayColorPrimvar();
  crvColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
  crvColorPrimvar.SetElementSize(1);
  crvColorPrimvar.Set(colors);
  
  curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);
}


JVR_NAMESPACE_CLOSE_SCOPE