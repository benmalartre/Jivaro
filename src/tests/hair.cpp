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

int density;
float length, amplitude, frequency, width, scale, radius;

static void _InitControls(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  pxr::UsdPrim controlPrim = stage->DefinePrim(rootPrim.GetPath().AppendChild(pxr::TfToken("Controls")));
  controlPrim.CreateAttribute(pxr::TfToken("Density"), pxr::SdfValueTypeNames->Int).Set(10000);
  controlPrim.CreateAttribute(pxr::TfToken("Radius"), pxr::SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(pxr::TfToken("Length"), pxr::SdfValueTypeNames->Float).Set(4.f);
  controlPrim.CreateAttribute(pxr::TfToken("Scale"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Amplitude"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  controlPrim.CreateAttribute(pxr::TfToken("Frequency"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Width"), pxr::SdfValueTypeNames->Float).Set(0.1f);
}

static void _QueryControls(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::UsdPrim controlPrim = rootPrim.GetChild(pxr::TfToken("Controls"));

  controlPrim.GetAttribute(pxr::TfToken("Density")).Get(&density);
  controlPrim.GetAttribute(pxr::TfToken("Radius")).Get(&radius);
  controlPrim.GetAttribute(pxr::TfToken("Length")).Get(&length);
  controlPrim.GetAttribute(pxr::TfToken("Scale")).Get(&scale);
  controlPrim.GetAttribute(pxr::TfToken("Amplitude")).Get(&amplitude);
  controlPrim.GetAttribute(pxr::TfToken("Frequency")).Get(&frequency);
  controlPrim.GetAttribute(pxr::TfToken("Width")).Get(&width);
}

pxr::HdDirtyBits _HairEmit(pxr::UsdStageRefPtr& stage, Curve* curve, pxr::UsdGeomMesh& mesh, pxr::GfMatrix4d& xform, double time)
{
  uint64_t T = CurrentTime();
  pxr::HdDirtyBits bits = pxr::HdChangeTracker::Clean;
 
  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<pxr::GfVec3f> normals;
  pxr::VtArray<int> counts;
  pxr::VtArray<int> indices;
  pxr::VtArray<Triangle> triangles;
  pxr::VtArray<Sample> samples;

  mesh.GetPointsAttr().Get(&positions);
  mesh.GetFaceVertexCountsAttr().Get(&counts);
  mesh.GetFaceVertexIndicesAttr().Get(&indices);

  //uint64_t T1 = CurrentTime() - T;
  //T = CurrentTime();
  TriangulateMesh(counts, indices, triangles);
  //uint64_t T2 = CurrentTime() - T;
  //T = CurrentTime();
  ComputeVertexNormals(positions, counts, indices, triangles, normals);
  //uint64_t T3 = CurrentTime() - T;
  //T = CurrentTime();
  PoissonSampling(radius, density, positions, normals, triangles, samples);
  //uint64_t T4 = CurrentTime() - T;
  //T = CurrentTime();  

  size_t numCVs = 4 * samples.size();


  pxr::VtArray<pxr::GfVec3f> points(numCVs);
  pxr::VtArray<float> radii(numCVs);
  pxr::VtArray<int> cvCounts(samples.size());

  for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {

    const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
    const pxr::GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
    const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
    const pxr::GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
    const float tangentFactor = pxr::GfCos(position[2] * scale + time * frequency) * amplitude;


    points[sampleIdx * 4] = xform.Transform(position);
    points[sampleIdx * 4 + 1] = xform.Transform(position + normal * 0.33 * length + bitangent * tangentFactor * 0.2);
    points[sampleIdx * 4 + 2] = xform.Transform(position + normal * 0.66 * length + bitangent * tangentFactor * 0.6);
    points[sampleIdx * 4 + 3] = xform.Transform(position + normal * length + bitangent * tangentFactor);

    radii[sampleIdx * 4] = width * 1.f;
    radii[sampleIdx * 4 + 1] = width * 0.8f;
    radii[sampleIdx * 4 + 2] = width * 0.4f;
    radii[sampleIdx * 4 + 3] = width * 0.2f;

    cvCounts[sampleIdx] = 4;
  }
  //uint64_t T5 = CurrentTime() - T; 
  //T = CurrentTime();

  curve->SetTopology(points, radii, cvCounts);

  //uint64_t T6 = CurrentTime() - T;
  //T = CurrentTime();

  return pxr::HdChangeTracker::DirtyTopology;
}


void TestHair::InitExec(pxr::UsdStageRefPtr& stage)
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
      _HairEmit(stage, curve, pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim), 1.f);
      
      _Sources sources;
      pxr::SdfPath hairPath = prim.GetPath().AppendPath(pxr::SdfPath(pxr::TfToken("Hair")));
      _scene.AddCurve(hairPath, curve);
      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
      _sourcesMap[hairPath] = sources;
    }
  }
  
}

void TestHair::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _QueryControls(stage);

  for(auto& source: _sourcesMap) {
    const pxr::SdfPath& path = source.first;
    _Sources& sources = source.second;

    Scene::_Prim* prim = _scene.GetPrim(path);
    Curve* curve = (Curve*)prim->geom;

    double time = Application::Get()->GetTime().GetActiveTime();
    pxr::UsdGeomXformCache xformCache(time);
    pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

    pxr::UsdGeomMesh mesh(stage->GetPrimAtPath(sources[0].first));
    prim->bits = _HairEmit(stage, curve, mesh, xformCache.GetLocalToWorldTransform(mesh.GetPrim()), time);    
  }
}

void TestHair::TerminateExec(pxr::UsdStageRefPtr& stage)
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