#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/instancer.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"

#include "../tests/utils.h"
#include "../tests/geodesic.h"

#include "../app/application.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

bool _ConstraintPointOnMesh(Mesh* mesh, const pxr::GfVec3f &point, Location* hit, pxr::GfVec3f* result=NULL)
{
  bool found(false);
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  pxr::VtArray<TrianglePair>& pairs = mesh->GetTrianglePairs();
  size_t numPairs = pairs.size();

  const pxr::GfMatrix4d &invMatrix = mesh->GetInverseMatrix();
  const pxr::GfMatrix4d &matrix = mesh->GetMatrix();

  Location localHit(*hit);

  pxr::GfVec3f localPoint(invMatrix.Transform(point));

  if(!localHit.GetComponentIndex() == Location::INVALID_INDEX)
    localHit.ConvertToLocal(invMatrix);

  for(size_t t = 0; t < numPairs; ++t) {
    TrianglePair* pair = &pairs[t];

    if (pair->Closest(positions, localPoint, &localHit)) 
      found = true;
    
  }
  if(found) {
    localHit.ConvertToWorld(matrix);
    if (result)
      *result = pxr::GfVec3f(localHit.GetPoint());
    hit->Set(localHit);
    return true;
  }
  return false;
}

void 
TestGeodesic::_BenchmarckClosestPoints()
{
  size_t N = 1000;
  pxr::VtArray<pxr::GfVec3f> points(N);
  for(auto& point: points)
    point = pxr::GfVec3f(RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10));

  pxr::VtArray<pxr::GfVec3f> result1(N);

  uint64_t startT1 = CurrentTime();
  
  for (size_t n = 0; n < N; ++n) {
    Location hit;
    pxr::GfVec3f result;
    for (size_t m = 0; m < _meshes.size(); ++m)
      if (_ConstraintPointOnMesh((Mesh*)_meshes[m], points[n], &hit, &result))
        result1[n] = pxr::GfVec3f(hit.GetPoint());
  }

  uint64_t elapsedT1 = CurrentTime() - startT1;
  
  pxr::VtArray<pxr::GfVec3f> result2(N);

  uint64_t startT2 = CurrentTime();
  for(size_t n = 0; n < N  ; ++n) {
    Location hit;
    if(_bvh.Closest(points[n], &hit, DBL_MAX)) {
      result2[n] = pxr::GfVec3f(hit.GetPoint());
    }
  }
  uint64_t elapsedT2 = CurrentTime() - startT2;

  std::cout << "================== benchmark closest points with " << N << " random points" << std::endl;
  std::cout << "brute force took : " << (elapsedT1 * 1e-6) << " seconds" << std::endl;
  std::cout << "with bvh took : " << (elapsedT2 * 1e-6) << " seconds" << std::endl;

}

// thread task
void TestGeodesic::_ClosestPointQuery(size_t begin, size_t end, const pxr::GfVec3f* positions, pxr::GfVec3f* results)
{
  for (size_t index = begin; index < end; ++index) {
    double minDistance = DBL_MAX;
    Location hit;
    if (_bvh.Closest(positions[index], &hit, DBL_MAX)) {
      const Geometry* collided = _bvh.GetGeometry(hit.GetGeometryIndex());
      const pxr::GfMatrix4d& matrix = collided->GetMatrix();
      switch (collided->GetType()) {
      case Geometry::MESH:
      {
        results[index] = pxr::GfVec3f(hit.GetPoint());
        break;
      }
      case Geometry::CURVE:
      {
        //Curve* curve = (Curve*)collided;
        //Edge* edge = curve->GetEdge(hit.GetElementIndex());
        //results[index] = hit.GetPosition(collided->GetPositionsCPtr(), &edge->vertices[0], 2, curve->GetMatrix());
        break;
      }
      default:
        continue;
      }
    }
  }
}

void 
TestGeodesic::_BenchmarckClosestPoints2()
{
  size_t N = 1000000;
  pxr::VtArray<pxr::GfVec3f> points(N);
  for(auto& point: points)
    point = pxr::GfVec3f(RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10));

  uint64_t startT = CurrentTime();
  pxr::VtArray<pxr::GfVec3f> results(N);

  pxr::WorkParallelForN(N,
    std::bind(&TestGeodesic::_ClosestPointQuery, this, std::placeholders::_1, 
      std::placeholders::_2, &points[0], &results[0]), N / 32);

  uint64_t elapsedT = CurrentTime() - startT;
  

  std::cout << "================== benchmark parallel closest points with " << N << " random points" << std::endl;
  std::cout << "with bvh took : " << (elapsedT * 1e-6) << " seconds" << std::endl;

}

void TestGeodesic::InitExec(pxr::UsdStageRefPtr& stage)
{
  std::cout << "test geodesic init scene " << std::endl;
  if (!stage) return;

  _GetRootPrim(stage);

  _TraverseStageFindingMeshes(stage);

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<float> widths;
  pxr::VtArray<pxr::GfVec3f> colors;

  std::cout << "found " << _meshes.size() << " meshes in teh stage" << std::endl;

  // create bvh
  if (_meshes.size()) {
    std::cout << "init kdtree" << std::endl;

    uint64_t startT = CurrentTime();
    _kdtree.Init(_meshes);
    uint64_t elapsedT = CurrentTime() - startT;

    std::cout << "================== kdtree initialize : "  << std::endl;
    std::cout << "- build took      : " << (elapsedT * 1e-6) << " seconds" << std::endl;
    std::cout << "- num components  : " << _kdtree.GetNumComponents() << std::endl;
    std::cout << "- num cells       : " << _kdtree.GetNumCells() << std::endl;
    

    startT = CurrentTime();
    _bvh.Init(_meshes);

    elapsedT = CurrentTime() - startT;

    std::cout << "================== bvh initialize : "  << std::endl;
    std::cout << "- build took      : " << (elapsedT * 1e-6) << " seconds" << std::endl;
    std::cout << "- num components  : " << _bvh.GetNumComponents() << std::endl;
    std::cout << "- num leaves      : " << _bvh.GetNumLeaves() << std::endl;
    std::cout << "- num cells       : " << _bvh.GetNumCells() << std::endl;

    for(size_t m = 0; m < _meshes.size(); ++m)
      _scene.AddGeometry(_meshesId[m], _meshes[m]);

    /*
    pxr::GfVec3f bmin(_bvh.GetMin());
    pxr::GfVec3f bmax(_bvh.GetMax());

    pxr::GfVec3f size(_bvh.GetSize());

    std::cout << "bvh size : " << size << std::endl;

    float radius = pxr::GfMin(size[0], size[1], size[2])*0.5f;
    std::cout << "bvh radius : " << radius << std::endl;

    pxr::GfVec3f dimensions(
      pxr::GfCeil(size[0] / radius),
      pxr::GfCeil(size[1] / radius),
      pxr::GfCeil(size[2] / radius)
    );

    size = dimensions * radius;

    pxr::GfVec3f step(
      size[0] / dimensions[0], 
      size[1] / dimensions[1], 
      size[2] / dimensions[2]);
    pxr::GfVec3f offset = bmin + pxr::GfVec3f(radius) * 0.5f;

    size_t total = dimensions[0] * dimensions[1] * dimensions[2];
    positions.resize(total);
    widths.resize(total);
    colors.resize(total);

    size_t index;
    for(size_t z = 0; z < dimensions[2]; ++z) 
      for(size_t y = 0; y < dimensions[1]; ++y)
        for(size_t x = 0; x < dimensions[0]; ++x) {
          index = z * dimensions[0] * dimensions[1] + y * dimensions[0] + x;

          positions[index] = pxr::GfVec3f(  
            x * step[0] + offset[0], 
            y * step[1] + offset[1],
            z * step[2] + offset[2]
          );
          widths[index] = radius;
          colors[index] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
        }
    */
  }

  
  _points= new Points();

  _points->SetPositions(positions);
  _points->SetWidths(widths);
  _points->SetColors(colors);
  //_points->SetInputOnly();

  _pointsId = _rootId.AppendChild(pxr::TfToken("points"));
  _scene.AddGeometry(_pointsId, _points);
  

  _bvhId = _rootId.AppendChild(pxr::TfToken("bvh"));
  _instancer = _SetupBVHInstancer(stage, _bvhId, &_bvh, true);
  _scene.AddGeometry(_bvhId, (Geometry*)_instancer );
  _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);

  _xformId = _rootId.AppendChild(pxr::TfToken("xform"));
  _xform = new Xform();

  _scene.AddGeometry(_xformId, (Geometry*) _xform);
  _scene.InjectGeometry(stage, _xformId, _xform, pxr::UsdTimeCode::Default());

  _BenchmarckClosestPoints();
  _BenchmarckClosestPoints2();
  
}


void TestGeodesic::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _bvh.Update();
  if (_meshes.size()) {
    size_t numPoints = _meshes[0]->GetNumPoints();
    const pxr::GfVec3f* positions = ((Deformable*)_meshes[0])->GetPositionsCPtr();
    const pxr::GfRange3f range(_meshes[0]->GetBoundingBox().GetRange());

    const pxr::GfMatrix4d& xform = _xform->GetMatrix();
    pxr::GfVec3f seed(xform[3][0], xform[3][1], xform[3][2]);

    pxr::GfMatrix4f matrix;
    pxr::GfVec3f position;
    Location hit1;
    pxr::VtArray<pxr::GfVec3f> points(4);
    pxr::VtArray<pxr::GfVec3f> colors(4);
    pxr::VtArray<float> widths(4, range.GetSize().GetLength() / 250.f);
    points[0] = seed;
    colors[2] = pxr::GfVec3f(1.f, 0.f, 0.f);
    colors[1] = pxr::GfVec3f(0.f, 1.f, 0.f);
    colors[0] = pxr::GfVec3f(0.f, 0.f, 1.f);
    colors[3] = pxr::GfVec3f(1.f, 1.f, 0.f);

    // bvh accelerated query
    if (_bvh.Closest(seed, &hit1, DBL_MAX)) {
      points[1] = pxr::GfVec3f(hit1.GetPoint());
    }

    // brute force reference
    Location hit2;
    pxr::GfVec3f result;
    for (size_t m = 0; m < _meshes.size(); ++m)
      if (_ConstraintPointOnMesh((Mesh*)_meshes[m], seed, &hit2, &result))
        points[2] = pxr::GfVec3f(hit2.GetPoint());

    // kdtree accelerated query (point only)
    Location hit3;
    if (_kdtree.Closest(seed, &hit3, DBL_MAX)) {
      points[3] = pxr::GfVec3f(hit3.GetPoint());
     
    }


    _points->SetPositions(points);
    _points->SetColors(colors);
    _points->SetWidths(widths);

    _scene.MarkPrimDirty(_pointsId, pxr::HdChangeTracker::DirtyPoints | pxr::HdChangeTracker::DirtyPrimvar);

    colors.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {

      colors[i][0] = RESCALE(positions[i][0], range.GetMin()[0], range.GetMax()[0], 0.f, 1.f);
      colors[i][1] = RESCALE(positions[i][1], range.GetMin()[1], range.GetMax()[1], 0.f, 1.f);
      colors[i][2] = RESCALE(positions[i][2], range.GetMin()[2], range.GetMax()[2], 0.f, 1.f);
    }
    ((Mesh*)_meshes[0])->SetColors(colors);

    for (size_t m = 0; m < _meshes.size(); ++m)
      _scene.MarkPrimDirty(_meshesId[m], pxr::HdChangeTracker::DirtyPrimvar);

    if (!pxr::GfIsClose(points[1], points[2], 0.0001f)) {
      std::cout << "divergence : " << (points[2] - points[1]) << std::endl;
      std::cout << "distances  : " << (points[1] - seed).GetLength() << " vs " << (points[2] - seed).GetLength() << std::endl;
    }
  }

}

void TestGeodesic::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _scene.Remove(_pointsId);
}

void TestGeodesic::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
    }
}



JVR_NAMESPACE_CLOSE_SCOPE