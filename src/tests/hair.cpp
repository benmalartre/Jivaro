#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"


#include "../app/application.h"

#include "../tests/utils.h"
#include "../tests/hair.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
TestHair::_InitControls(UsdStageRefPtr& stage)
{
  UsdPrim rootPrim = stage->GetDefaultPrim();

  UsdPrim controlPrim = stage->DefinePrim(rootPrim.GetPath().AppendChild(TfToken("Controls")));
  controlPrim.CreateAttribute(TfToken("Density"), SdfValueTypeNames->Int).Set(100);
  controlPrim.CreateAttribute(TfToken("Radius"), SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(TfToken("Length"), SdfValueTypeNames->Float).Set(4.f);
  controlPrim.CreateAttribute(TfToken("Scale"), SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(TfToken("Amplitude"), SdfValueTypeNames->Float).Set(0.5f);
  controlPrim.CreateAttribute(TfToken("Frequency"), SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(TfToken("Width"), SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(TfToken("Color"), SdfValueTypeNames->Float3).Set(
    GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1));
}

void 
TestHair::_QueryControls(UsdStageRefPtr& stage)
{
  UsdPrim rootPrim = stage->GetDefaultPrim();
  UsdPrim controlPrim = rootPrim.GetChild(TfToken("Controls"));

  controlPrim.GetAttribute(TfToken("Density")).Get(&_density);
  controlPrim.GetAttribute(TfToken("Radius")).Get(&_radius);
  controlPrim.GetAttribute(TfToken("Length")).Get(&_length);
  controlPrim.GetAttribute(TfToken("Scale")).Get(&_scale);
  controlPrim.GetAttribute(TfToken("Amplitude")).Get(&_amplitude);
  controlPrim.GetAttribute(TfToken("Frequency")).Get(&_frequency);
  controlPrim.GetAttribute(TfToken("Width")).Get(&_width);
  controlPrim.GetAttribute(TfToken("Color")).Get(&_color);
}

HdDirtyBits 
TestHair::_HairEmit(UsdStageRefPtr& stage, Curve* curve, UsdGeomMesh& mesh, 
  GfMatrix4d& xform, double time)
{
  uint64_t T = ArchGetTickTime();
  HdDirtyBits bits = HdChangeTracker::Clean;
 
  VtArray<GfVec3f> positions;
  VtArray<GfVec3f> normals;
  VtArray<int> counts;
  VtArray<int> indices;
  VtArray<Triangle> triangles;
  VtArray<Sample> samples;

  mesh.GetPointsAttr().Get(&positions, time);
  mesh.GetFaceVertexCountsAttr().Get(&counts, time);
  mesh.GetFaceVertexIndicesAttr().Get(&indices, time);

  //uint64_t T1 = ArchGetTickTime() - T;
  //T = ArchGetTickTime();
  TriangulateMesh(counts, indices, triangles);
  //uint64_t T2 = ArchGetTickTime() - T;
  //T = ArchGetTickTime();
  ComputeVertexNormals(positions, counts, indices, triangles, normals);
  //uint64_t T3 = ArchGetTickTime() - T;
  //T = ArchGetTickTime();
  //PoissonSampling(_radius, _density, positions, normals, triangles, samples);
  StochasticSampling(_density, positions, normals, triangles, samples);
  //uint64_t T4 = ArchGetTickTime() - T;
  //T = ArchGetTickTime();  

  size_t numCVs = 4 * samples.size();


  VtArray<GfVec3f> points(numCVs);
  VtArray<GfVec3f> colors(numCVs);
  VtArray<float> radii(numCVs);
  VtArray<int> cvCounts(samples.size());


  for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {

    const GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
    const GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
    const GfVec3f bitangent = (normal ^ tangent).GetNormalized();
    const GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
    const float tangentFactor = GfCos(position[2] * _scale + time * _frequency) * _amplitude;

    colors[sampleIdx * 4] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 1] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 2] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 3] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);


    points[sampleIdx * 4] = GfVec3f(xform.Transform(position));
    points[sampleIdx * 4 + 1] = GfVec3f(xform.Transform(position + normal * 0.33 * _length + bitangent * tangentFactor * 0.2));
    points[sampleIdx * 4 + 2] = GfVec3f(xform.Transform(position + normal * 0.66 * _length + bitangent * tangentFactor * 0.6));
    points[sampleIdx * 4 + 3] = GfVec3f(xform.Transform(position + normal * _length + bitangent * tangentFactor));

    radii[sampleIdx * 4] = _width * 1.f;
    radii[sampleIdx * 4 + 1] = _width * 0.8f;
    radii[sampleIdx * 4 + 2] = _width * 0.4f;
    radii[sampleIdx * 4 + 3] = _width * 0.2f;

    cvCounts[sampleIdx] = 4;
  }
  //uint64_t T5 = ArchGetTickTime() - T; 
  //T = ArchGetTickTime();

  curve->SetTopology(points, radii, cvCounts);
  curve->SetColors(colors);

  //uint64_t T6 = ArchGetTickTime() - T;
  //T = ArchGetTickTime();

  return HdChangeTracker::DirtyTopology;
}


void 
TestHair::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  UsdPrimRange primRange = stage->TraverseAll();
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  UsdPrim rootPrim = stage->GetDefaultPrim();

  _InitControls(stage);
  _QueryControls(stage);


  for (UsdPrim prim : primRange) {
    if (prim.IsA<UsdGeomMesh>()) {
      Curve* curve = new Curve();
      UsdGeomMesh usdMesh(prim);
      GfMatrix4d matrix = xformCache.GetLocalToWorldTransform(prim);
      _HairEmit(stage, curve, usdMesh, matrix, 1.f);
      
      _Sources sources;
      SdfPath hairPath = prim.GetPath().AppendPath(SdfPath(TfToken("Hair")));
      _scene.AddGeometry(hairPath, curve);
      sources.push_back({ prim.GetPath(), HdChangeTracker::Clean });
      _sourcesMap[hairPath] = sources;
    }
  }
  
}

void 
TestHair::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _QueryControls(stage);

  _scene.Sync(stage, time);
  for(auto& source: _sourcesMap) {
    const SdfPath& path = source.first;
    _Sources& sources = source.second;

    Scene::_Prim* prim = _scene.GetPrim(path);
    Curve* curve = (Curve*)prim->geom;

    double time = Time::Get()->GetActiveTime();
    UsdGeomXformCache xformCache(time);
    UsdPrim rootPrim = stage->GetDefaultPrim();

    UsdGeomMesh mesh(stage->GetPrimAtPath(sources[0].first));
    GfMatrix4d matrix = xformCache.GetLocalToWorldTransform(mesh.GetPrim());
    prim->bits = _HairEmit(stage, curve, mesh, matrix, time);    
  }
}

void 
TestHair::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  UsdPrim rootPrim = stage->GetDefaultPrim();
  SdfPath rootId = rootPrim.GetPath();

  UsdPrimRange primRange = stage->TraverseAll();

   for (UsdPrim prim : primRange) {
    if (prim.IsA<UsdGeomMesh>()) {
      SdfPath hairPath = prim.GetPath().AppendPath(SdfPath(TfToken("Hair")));
      _scene.Remove(hairPath);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE