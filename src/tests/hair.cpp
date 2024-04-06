#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/imaging/hd/changeTracker.h>

#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/scene.h"

#include "../tests/hair.h"

JVR_NAMESPACE_OPEN_SCOPE

int density;
float length, amplitude, frequency, width, scale, radius;

static void _InitControls(pxr::UsdStageRefPtr& stage)
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

  _InitControls(stage);
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

  _Sources sources;

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      Curve* curve = new Curve();
      _HairEmit(stage, curve, pxr::UsdGeomMesh(prim), pxr::GfMatrix4d(1.0), 1.f);
      
      _scene->AddCurve(prim.GetPath().AppendPath(pxr::SdfPath(pxr::TfToken("Hair"))), curve);
      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    }
  }
  
}


void TestHair::UpdateExec(pxr::UsdStageRefPtr& stage, double time, double startTime)
{
}

void TestHair::TerminateExec(pxr::UsdStageRefPtr& stage)
{
}

JVR_NAMESPACE_CLOSE_SCOPE