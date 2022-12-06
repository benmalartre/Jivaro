#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/sampler.h"
#include "../acceleration/bvh.h"
#include "../app/application.h"
#include "../app/time.h"

#include "../utils/timer.h"

#include <iostream>

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/curves.h>



JVR_NAMESPACE_OPEN_SCOPE

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
    pxr::TfToken name(name + std::to_string(rayIndex));

    pxr::UsdGeomSphere origin = 
      pxr::UsdGeomSphere::Define(stage, rayGroup.GetPath().AppendChild(name));
    
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
_SetupResults(pxr::UsdStageRefPtr& stage, std::vector<pxr::GfVec3f>& points)
{

  pxr::UsdGeomXform pntGroup =
    pxr::UsdGeomXform::Define(stage, stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("intersection")));

  size_t rayIndex = 0;
  pxr::VtArray<pxr::GfVec3f> colors(1);


  std::string name = "ray_intersection_";
  for (auto& point : points) {
    colors[0] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    pxr::TfToken name(name + std::to_string(rayIndex));

    pxr::UsdGeomSphere origin =
      pxr::UsdGeomSphere::Define(stage, pntGroup.GetPath().AppendChild(name));

    origin.AddTranslateOp().Set(pxr::GfVec3d(point));
    origin.CreateRadiusAttr().Set(0.2);
    origin.CreateDisplayColorAttr().Set(colors);

    rayIndex++;

  }


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
_SetupBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh)
{
  std::vector<BVH*> cells;
  bvh->GetCells(cells);
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
      pxr::GfVec3f(bvh->ComputeCodeAsColor(
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


PBDSolver::PBDSolver() 
  : _gravity(0,-1,0)
  , _timeStep(0.05)
{
}


PBDSolver::~PBDSolver()
{

}

void PBDSolver::Reset()
{
  _system.Reset();
}

void PBDSolver::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  size_t offset = _system.AddGeometry(geom, m);
  AddConstraints(geom, offset);
}

void PBDSolver::RemoveGeometry(Geometry* geom)
{
  _system.RemoveGeometry(geom);
}

void PBDSolver::AddColliders(std::vector<Geometry*>& colliders)
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetWorkspace()->GetExecStage();
  size_t numRays = 256;
  std::vector<pxr::GfRay> rays(numRays);
  for (size_t r = 0; r < numRays; ++r) {
    rays[r] = pxr::GfRay(pxr::GfVec3f(0.f), 
      pxr::GfVec3f(RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1)).GetNormalized());
  }

  std::vector<pxr::GfVec3f> result;

  _SetupRays(stage, rays);

  _colliders = colliders;

  for (auto& collider : colliders) {
    _SetupHairs(stage, collider);
  }

  std::cout << "### build bvh (mortom) : " << std::endl;
  uint64_t T = CurrentTime();
  BVH bvh;
  bvh.Init(_colliders);
  std::cout << "   build boundary volume hierarchy : " << ((CurrentTime() - T) * 1e-9) << std::endl;

  T = CurrentTime();
  for (auto& ray : rays) {
    double minDistance;
    Hit hit;
    if (bvh.Raycast(ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      result.push_back(position);
    }
  }
  std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
  _SetupBVHInstancer(stage, &bvh);

  std::vector<BVH*> cells;
  bvh.GetCells(cells);
  std::cout << "   num cells : " << cells.size() << std::endl;


  _SetupResults(stage, result);
}

void PBDSolver::UpdateColliders()
{
  BVH bvh;
  bvh.Init(_colliders);
  /*
  {
    double minDistance;
    Hit hit;
    if (bvh.Closest(pxr::GfVec3f(0.f), &hit, -1, &minDistance)) {
      std::cout << "CLOSEST HIT :" << std::endl;
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      std::cout << "   pos : " << position << std::endl;
      std::cout << "   tri : " << hit.GetElementIndex() << std::endl;
    }
  }
  */
  {
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Hit hit;
    if (bvh.Raycast(ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(&position);
    }
  }
}

void PBDSolver::AddConstraints(Geometry* geom, size_t offset)
{
  if (geom->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geom;
    std::vector<HalfEdge*> edges = mesh->GetUniqueEdges();
    for (HalfEdge* edge : edges) {
      PBDDistanceConstraint* constraint = new PBDDistanceConstraint();
      constraint->Init(this, edge->vertex + offset, edge->next->vertex + offset, 0.5f);
      _constraints.push_back(constraint);
    }
  }
}

void PBDSolver::SatisfyConstraints()
{
  Time& time = GetApplication()->GetTime();
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < _constraints.size(); i++) {
      PBDConstraint* c = _constraints[i];
      c->Solve(this, 1);
    }
    // Constrain one particle of the cloth to origo
    _system.GetPosition(0) =
      _system.GetInitPositions()[0] + 
        pxr::GfVec3f(0, sin(time.GetActiveTime()) * 5.f + 1.f, 0.f);

    for (int i = 0; i < _system.GetNumParticles(); ++i) {
      pxr::GfVec3f& p = _system.GetPosition(i);


      //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
      p[1] = pxr::GfMax(p[1], 0.f);
      //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
    }

  }
}

void PBDSolver::Step()
{
  _system.AccumulateForces(_gravity);
  _system.Integrate(_timeStep);
  UpdateColliders();
  SatisfyConstraints();
  
  _system.UpdateGeometries();
}

JVR_NAMESPACE_CLOSE_SCOPE