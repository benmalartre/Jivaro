#include "../pbd/solver.h"
#include "../pbd/constraint.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/voxels.h"
#include "../geometry/curve.h"
#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../app/application.h"
#include "../app/time.h"

#include "../utils/timer.h"

#include <iostream>

#include <pxr/base/work/loops.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/curves.h>




JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {

  static void 
  BenchmarkParallelEvaluation(Solver* solver)
  {
    std::cout << "benchmark parallel evaluation" << std::endl;
    for (size_t grain = 1; grain <= 2048; grain *=2) {
      uint64_t startT = CurrentTime();
      //solver->SetGrain(grain);
      solver->Reset();
      for (size_t t = 0; t < 250; ++t) {
        solver->Step();
      }
      std::cout << "[parallel] (grain: " << grain << ") took " << ((CurrentTime() - startT) * 1e-9) << " seconds" << std::endl;

    }
    /*
    uint64_t startT = CurrentTime();
    Body* body = solver->GetBody();
    size_t last = body->GetNumParticles();
    body->Reset(0, last);
    for (size_t t = 0; t < 250; ++t) {
      body->AccumulateForces(0, last, solver->GetGravity());
      body->Integrate(0, last, solver->GetTimeStep());

      for (size_t i = 0; i < last; ++i) {
        pxr::GfVec3f& p = body->GetPosition(i);

        //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
        p[1] = pxr::GfMax(p[1], 0.f);
        //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
      }
      
      size_t numConstraints = solver->GetNumConstraints();
      for (size_t i = 0; i < numConstraints; ++i) {
        Constraint* c = solver->GetConstraint(i);
        c->Solve(solver, 1);
      }
    }
    std::cout << "[serial] (1 thread) took " << ((CurrentTime() - startT) * 1e-9) << " seconds" << std::endl;
    */

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
    curve.CreateWidthsAttr().Set(widths);
    curve.SetWidthsInterpolation(pxr::UsdGeomTokens->vertex);
    curve.CreateCurveVertexCountsAttr().Set(curveVertexCount);

    pxr::UsdGeomPrimvar crvColorPrimvar = curve.CreateDisplayColorPrimvar();
    crvColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
    crvColorPrimvar.SetElementSize(1);
    crvColorPrimvar.Set(colors);
    
    
  }

  static void
  _SetupVoxels(pxr::UsdStageRefPtr& stage, Voxels* voxels, float radius)
  {
    const pxr::VtArray<pxr::GfVec3f>& positions = voxels->GetPositions();

    std::cout << "hash grid radius : " << (voxels->GetRadius() * 2.f) << std::endl;
    HashGrid grid(voxels->GetRadius() * 2.f);
    grid.Init({ (Geometry*)voxels });

    size_t numPoints = positions.size();
    pxr::UsdGeomPoints points =
      pxr::UsdGeomPoints::Define(
        stage,
        stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("Voxels")));

    points.CreatePointsAttr().Set(pxr::VtValue(positions));

    pxr::VtArray<float> widths(numPoints);
    pxr::VtArray<pxr::GfVec3f> colors(numPoints);
    for (size_t p = 0; p < numPoints; ++p) {
      widths[p] = radius;
      colors[p] = grid.GetColor(positions[p]);
    }
    points.CreateWidthsAttr().Set(pxr::VtValue(widths));
    points.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
    //points.CreateNormalsAttr().Set(pxr::VtValue({pxr::GfVec3f(0, 1, 0)}));
    points.CreateDisplayColorAttr().Set(pxr::VtValue(colors));
    points.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);

  }

  static void
  _TestHashGrid(Voxels* voxels, float radius)
  {
    size_t N = 1000000;
    float step = 10.f / float(N);
    std::vector<std::vector<int>> result1;
    std::vector<std::vector<int>> result2;
    const pxr::GfVec3f* points = voxels->GetPositionsCPtr();
    {
      std::cout << "HashGrid MULLER : " << radius << std::endl;
      HashGrid grid(radius * 2.f, HashGrid::MULLER);
      grid.Init({ (Geometry*)voxels });
      result1.resize(N);
      std::vector<int> closests(voxels->GetNumPoints());
      size_t falsePositive = 0;
      size_t numChecks = 0;
      uint64_t T = CurrentTime();
      for (size_t t = 0; t < N; ++t) {
        //std::cout << "search closests for query point " << pxr::GfVec3f(0.f, t * 0.1f, 0.f) << std::endl;
        const pxr::GfVec3f point(0.f, t * step, 0.f);
        size_t n = grid.Closests(point, radius, closests);
        if (!n) continue;
        numChecks += n;
        //std::cout << "found " << n << " closests points with search radius " << radius << std::endl;
        for (size_t c = 0; c < n; ++c) {
          if ((points[closests[c]] - point).GetLengthSq() < (radius * radius)) {
            result1[t].push_back(closests[c]);
          }
          else {
            //std::cout << closests[c] << " : " << points[closests[c]] << std::endl;
            falsePositive++;
          }
        }
      }
      std::cout << "  closests took " << ((CurrentTime() - T) * 1e-9) << " seconds for " << N << " points " << std::endl;
      std::cout << "  false positives : " << falsePositive << std::endl;
      std::cout << "  num checks : " << numChecks << std::endl;
    }

    {
      std::cout << "HashGrid PIXAR : " << radius << std::endl;
      HashGrid grid(radius * 2.f, HashGrid::PIXAR);
      grid.Init({ (Geometry*)voxels });
      result2.resize(N);
      std::vector<int> closests(voxels->GetNumPoints());
      size_t falsePositive = 0;
      size_t numChecks = 0;
      uint64_t T = CurrentTime();
      for (size_t t = 0; t < N; ++t) {
        //std::cout << "search closests for query point " << pxr::GfVec3f(0.f, t * 0.1f, 0.f) << std::endl;
        const pxr::GfVec3f point(0.f, t * step, 0.f);
        size_t n = grid.Closests(point, radius, closests);
        if (!n) continue;
        numChecks += n;
        //std::cout << "found " << n << " closests points with search radius " << radius << std::endl;
        for (size_t c = 0; c < n; ++c) {
          if ((points[closests[c]] - point).GetLengthSq() < (radius * radius)) {
            result2[t].push_back(closests[c]);
          }
          else {
            //std::cout << closests[c] << " : " << points[closests[c]] << std::endl;
            falsePositive++;
          }
        }
      }
      std::cout << "  closests took " << ((CurrentTime() - T) * 1e-9) << " seconds for " << N << " points " << std::endl;
      std::cout << "  false positives : " << falsePositive << std::endl;
      std::cout << "  num checks : " << numChecks << std::endl;
    }

    /*
    std::cout << "compare results : " << std::endl;
    for (size_t t = 0; t < 100; ++t) {
      size_t s1 = result1[t].size();
      size_t s2 = result2[t].size();
      std::cout << "frame " << t << ": " << s1 << " vs " << s2 << std::endl;
      if (s1 == s2) {
        std::cout << "content equal ? " << (result1[t] == result2[t]) << std::endl;
      }
      else {

        std::cout << "content can not match because different size" << std::endl;
        std::cout << "result 1 : ";
        for (auto& e1 : result1[t]) std::cout << e1 << ",";
        std::cout << std::endl;
        std::cout << "points 1 : ";
        for (auto& e1 : result1[t]) std::cout << points[e1] << ",";
        std::cout << std::endl;

        std::cout << "result 2 : ";
        for (auto& e2 : result2[t]) std::cout << e2 << ",";
        std::cout << std::endl;
        std::cout << "points 2 : ";
        for (auto& e2 : result2[t]) std::cout << points[e2] << ",";
        std::cout << std::endl;

        std::cout << "query : " << pxr::GfVec3f(0.f, t * 0.1f, 0.f) << std::endl;
        std::cout << "----------------------------------------------------------------------" << std::endl;
  
      }
    }
    */
  }
  

  Solver::Solver()
    : _gravity(0, -1, 0)
    , _timeStep(1.f / 60.f)
    , _substeps(15)
    , _paused(true)
    , _grain(32)
  {
  }

  Solver::~Solver()
  {

  }

  void Solver::Reset()
  {
    UpdateColliders();
    // reset
    for (size_t p = 0; p < GetNumParticles(); ++p) {

    }
  }

  Body* Solver::GetBody(size_t index)
  {
    if (index < _bodies.size())return _bodies[index];
    return nullptr;
  }

  Body* Solver::GetBody(Geometry* geom)
  {
    for (auto& body : _bodies) {
      if (body->geometry == geom)return body;
    }
    return nullptr;
  }

  size_t Solver::GetBodyIndex(Geometry* geom)
  {
    for (size_t index = 0; index < _bodies.size(); ++index) {
      if (_bodies[index]->geometry == geom)return index;
    }
    return Solver::INVALID_INDEX;
  }

  void Solver::AddBody(Geometry* geom, const pxr::GfMatrix4f& matrix)
  {
    std::cout << "[system] add geometry : " << geom << std::endl;
    size_t base = _position.size();
    size_t add = geom->GetNumPoints();

    size_t newSize = base + add;
    _position.resize(newSize);


    const pxr::VtArray<pxr::GfVec3f>& points = geom->GetPositions();
    for (size_t p = 0; p < add; ++p) {
      const pxr::GfVec3f pos = matrix.Transform(points[p]);
      size_t idx = base + p;
      _position[idx] = pos;
    }

    Body* body = new Body({ 1.f, 1.f, 1.f, base, points.size(), geom });
    _bodies.push_back(body);

      /*
      float          damping;
    float          radius;
    float          mass;

    size_t         offset;
    size_t         numPoints;
    
    Geometry*      geometry;
      */
  }

  void Solver::RemoveBody(Geometry* geom)
  {
    std::cout << "[system] remove geometry : " << geom << std::endl;
    size_t index = GetBodyIndex(geom);
    if (index == Solver::INVALID_INDEX) return;

    Body* body = GetBody(index);
    size_t base = body->offset;
    size_t shift = body->numPoints;
    size_t remaining = _position.size() - (base + shift);

    for (size_t r = 0; r < remaining; ++r) {
      _position[base + r] = _position[base + shift + r];
    }

    size_t newSize = _position.size() - shift;
    _position.resize(newSize);

    Body* body = _bodies[index];
    _bodies.erase(_bodies.begin() + index);
    delete body;
  }

  /*
  void Solver::AddCollision(Geometry* collider)
  {
    _colliders.push_back(collider);
    
    float radius = 0.2f;
    Voxels voxels;
    voxels.Init(collider, radius);
    voxels.Trace(0);
    voxels.Trace(1);
    voxels.Trace(2);
    voxels.Build();
    _SetupVoxels(stage, &voxels, radius);

    _TestHashGrid(&voxels, radius);
    
    BenchmarkParallelEvaluation(this);
    
    size_t numRays = 2048;
    std::vector<pxr::GfRay> rays(numRays);
    for (size_t r = 0; r < numRays; ++r) {
      rays[r] = pxr::GfRay(pxr::GfVec3f(0.f), 
        pxr::GfVec3f(RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1)).GetNormalized());
    }

    std::vector<pxr::GfVec3f> result;


    _colliders = colliders;

    std::cout << "### build bvh (morton) : " << std::endl;
    for(auto& collider: _colliders) {
      std::cout << " collider : " << collider << std::endl;
    }
    uint64_t T = CurrentTime();
    BVH bvh;
    bvh.Init(_colliders);
    std::cout << "   build boundary volume hierarchy : " << ((CurrentTime() - T) * 1e-9) << std::endl;

    T = CurrentTime();
    for (auto& ray : rays) {
      double minDistance = DBL_MAX;
      Hit hit;
      const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
      if (bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
        result.push_back(hit.GetPosition(colliders[hit.GetGeometryIndex()]));
      }
    }
    std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
    _SetupBVHInstancer(stage, &bvh);


    _SetupResults(stage, result);
   
  }

  void Solver::UpdateColliders()
  {

    BVH bvh;
    bvh.Init(_colliders);
    
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
    
    {
      pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
      double minDistance;
      Hit hit;
      const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
      if (bvh.Raycast(points, ray, &hit, -1, &minDistance)) {
        pxr::GfVec3f position;
        hit.GetPosition(_colliders[0]);
      }
    }
   
  }
  */

  void Solver::AddConstraints(Geometry* geom, size_t offset)
  {
    std::cout << "[solver] add constraints for type " << geom->GetType() << std::endl;
    if (geom->GetType() == Geometry::MESH) {
      std::cout << "[solver] add constraints for mesh : " << std::endl;
      Mesh* mesh = (Mesh*)geom;
      const pxr::VtArray<HalfEdge>& edges = mesh->GetEdges();
      std::cout << "[solver] num unique edges : " << edges.size() << std::endl;
      /*
      for (HalfEdge* edge : edges) {
        DistanceConstraint* constraint = new DistanceConstraint();
        constraint->Init(this, edge->vertex + offset, edge->next->vertex + offset, 0.5f);
        _constraints.push_back(constraint);
      }
      */
    } else if (geom->GetType() == Geometry::CURVE) {
      Curve* curve = (Curve*)geom;
      curve->GetTotalNumSegments();
      for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {

      }
    }
  }

  void Solver::SatisfyConstraints()
  {
    Time& time = GetApplication()->GetTime();
    for (int j = 0; j < 5; j++) {
      for (int i = 0; i < _constraints.size(); i++) {
        Constraint* c = _constraints[i];
        c->Solve(this, 1);
      }
      // Constrain one particle of the cloth to origo
      _body.GetPosition(0) =
        _body.GetRestPosition(0) + 
          pxr::GfVec3f(0, sin(time.GetActiveTime()) * 5.f + 1.f, 0.f);

      for (int i = 0; i < _body.GetNumParticles(); ++i) {
        pxr::GfVec3f& p = _body.GetPosition(i);


        //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
        p[1] = pxr::GfMax(p[1], 0.f);
        //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
      }

    }
  }

  void Solver::Step()
  {
    UpdateColliders();

   
    UpdateGeometries();
  }

  void Solver::UpdateGeometries()
  {
    /*
    std::map<Geometry*, _Geometry>::iterator it = _geometries.begin();
    for (; it != _geometries.end(); ++it)
    {
      Geometry* geom = it->first;
      size_t numPoints = geom->GetNumPoints();
      pxr::VtArray<pxr::GfVec3f> results(numPoints);
      const auto& positions = _body.GetPositions();
      for (size_t p = 0; p < numPoints; ++p) {
        results[p] = it->second.invMatrix.Transform(positions[it->second.offset + p]);
      }
      geom->SetPositions(&results[0], numPoints);
    }
  }
  */
}

JVR_NAMESPACE_CLOSE_SCOPE