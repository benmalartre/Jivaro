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
TestHair::_InitControls(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  pxr::UsdPrim controlPrim = stage->DefinePrim(rootPrim.GetPath().AppendChild(pxr::TfToken("Controls")));
  controlPrim.CreateAttribute(pxr::TfToken("Density"), pxr::SdfValueTypeNames->Int).Set(100);
  controlPrim.CreateAttribute(pxr::TfToken("Radius"), pxr::SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(pxr::TfToken("Length"), pxr::SdfValueTypeNames->Float).Set(4.f);
  controlPrim.CreateAttribute(pxr::TfToken("Scale"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Amplitude"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  controlPrim.CreateAttribute(pxr::TfToken("Frequency"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Width"), pxr::SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(pxr::TfToken("Color"), pxr::SdfValueTypeNames->Float3).Set(
    pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1));
}

void 
TestHair::_QueryControls(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::UsdPrim controlPrim = rootPrim.GetChild(pxr::TfToken("Controls"));

  controlPrim.GetAttribute(pxr::TfToken("Density")).Get(&_density);
  controlPrim.GetAttribute(pxr::TfToken("Radius")).Get(&_radius);
  controlPrim.GetAttribute(pxr::TfToken("Length")).Get(&_length);
  controlPrim.GetAttribute(pxr::TfToken("Scale")).Get(&_scale);
  controlPrim.GetAttribute(pxr::TfToken("Amplitude")).Get(&_amplitude);
  controlPrim.GetAttribute(pxr::TfToken("Frequency")).Get(&_frequency);
  controlPrim.GetAttribute(pxr::TfToken("Width")).Get(&_width);
  controlPrim.GetAttribute(pxr::TfToken("Color")).Get(&_color);
}

pxr::HdDirtyBits 
TestHair::_HairEmit(pxr::UsdStageRefPtr& stage, Curve* curve, pxr::UsdGeomMesh& mesh, 
  pxr::GfMatrix4d& xform, double time)
{
  uint64_t T = CurrentTime();
  pxr::HdDirtyBits bits = pxr::HdChangeTracker::Clean;
 
  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<pxr::GfVec3f> normals;
  pxr::VtArray<int> counts;
  pxr::VtArray<int> indices;
  pxr::VtArray<Triangle> triangles;
  pxr::VtArray<Sample> samples;

  mesh.GetPointsAttr().Get(&positions, time);
  mesh.GetFaceVertexCountsAttr().Get(&counts, time);
  mesh.GetFaceVertexIndicesAttr().Get(&indices, time);

  //uint64_t T1 = CurrentTime() - T;
  //T = CurrentTime();
  TriangulateMesh(counts, indices, triangles);
  //uint64_t T2 = CurrentTime() - T;
  //T = CurrentTime();
  ComputeVertexNormals(positions, counts, indices, triangles, normals);
  //uint64_t T3 = CurrentTime() - T;
  //T = CurrentTime();
  //PoissonSampling(_radius, _density, positions, normals, triangles, samples);
  StochasticSampling(_density, positions, normals, triangles, samples);
  //uint64_t T4 = CurrentTime() - T;
  //T = CurrentTime();  

  size_t numCVs = 4 * samples.size();


  pxr::VtArray<pxr::GfVec3f> points(numCVs);
  pxr::VtArray<pxr::GfVec3f> colors(numCVs);
  pxr::VtArray<float> radii(numCVs);
  pxr::VtArray<int> cvCounts(samples.size());


  for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {

    const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
    const pxr::GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
    const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
    const pxr::GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
    const float tangentFactor = pxr::GfCos(position[2] * _scale + time * _frequency) * _amplitude;

    colors[sampleIdx * 4] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 1] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 2] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    colors[sampleIdx * 4 + 3] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);


    points[sampleIdx * 4] = xform.Transform(position);
    points[sampleIdx * 4 + 1] = xform.Transform(position + normal * 0.33 * _length + bitangent * tangentFactor * 0.2);
    points[sampleIdx * 4 + 2] = xform.Transform(position + normal * 0.66 * _length + bitangent * tangentFactor * 0.6);
    points[sampleIdx * 4 + 3] = xform.Transform(position + normal * _length + bitangent * tangentFactor);

    radii[sampleIdx * 4] = _width * 1.f;
    radii[sampleIdx * 4 + 1] = _width * 0.8f;
    radii[sampleIdx * 4 + 2] = _width * 0.4f;
    radii[sampleIdx * 4 + 3] = _width * 0.2f;

    cvCounts[sampleIdx] = 4;
  }
  //uint64_t T5 = CurrentTime() - T; 
  //T = CurrentTime();

  curve->SetTopology(points, radii, cvCounts);
  curve->SetColors(colors);

  //uint64_t T6 = CurrentTime() - T;
  //T = CurrentTime();

  return pxr::HdChangeTracker::DirtyTopology;
}


void 
TestHair::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  _InitControls(stage);
  _QueryControls(stage);


  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      Curve* curve = new Curve();
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d matrix = xformCache.GetLocalToWorldTransform(prim);
      _HairEmit(stage, curve, usdMesh, matrix, 1.f);
      
      _Sources sources;
      pxr::SdfPath hairPath = prim.GetPath().AppendPath(pxr::SdfPath(pxr::TfToken("Hair")));
      _scene.AddGeometry(hairPath, curve);
      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
      _sourcesMap[hairPath] = sources;
    }
  }
  
}

void 
TestHair::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _QueryControls(stage);

  _scene.Sync(stage, time);
  for(auto& source: _sourcesMap) {
    const pxr::SdfPath& path = source.first;
    _Sources& sources = source.second;

    Scene::_Prim* prim = _scene.GetPrim(path);
    Curve* curve = (Curve*)prim->geom;

    double time = Time::Get()->GetActiveTime();
    pxr::UsdGeomXformCache xformCache(time);
    pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

    pxr::UsdGeomMesh mesh(stage->GetPrimAtPath(sources[0].first));
    pxr::GfMatrix4d matrix = xformCache.GetLocalToWorldTransform(mesh.GetPrim());
    prim->bits = _HairEmit(stage, curve, mesh, matrix, time);    
  }
}

void 
TestHair::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath();

  pxr::UsdPrimRange primRange = stage->TraverseAll();

   for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::SdfPath hairPath = prim.GetPath().AppendPath(pxr::SdfPath(pxr::TfToken("Hair")));
      _scene.Remove(hairPath);
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE