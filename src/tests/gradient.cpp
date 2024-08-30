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
#include "../tests/gradient.h"

#include "../app/application.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

pxr::GfVec3f _ConstraintPointOnMesh(Mesh* mesh, const pxr::GfVec3f &point)
{
  float minDistance = FLT_MAX;
  size_t index = Component::INVALID_INDEX;
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  size_t numTriangles = mesh->GetNumTriangles();

  Location hit;
  for(size_t t = 0; t < numTriangles; ++t) {
    Triangle* triangle = mesh->GetTriangle(t);

    triangle->Closest(positions, point, &hit);
  }

  if(hit.GetComponentIndex() != Component::INVALID_INDEX) 
  {
    Triangle* triangle = mesh->GetTriangle(hit.GetComponentIndex());
    return hit.ComputePosition(positions, &triangle->vertices[0], 3);
  }
  return pxr::GfVec3f() ;
}

void TestGradient::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _GetRootPrim(stage);

  _TraverseStageFindingMesh(stage);

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<float> widths;
  pxr::VtArray<pxr::GfVec3f> colors;

  // create bvh
  if (_mesh && _mesh->GetPrim().IsValid()) {
    _bvh.Init({_mesh});

    _scene.AddGeometry(_meshId, _mesh);

    pxr::GfVec3f bmin(_bvh.GetMin());
    pxr::GfVec3f bmax(_bvh.GetMax());

    pxr::GfVec3f size(_bvh.GetSize());

    float radius = pxr::GfMin(size[0], size[1], size[2])*0.5f;

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

    _gradient.Init((Mesh*)_mesh);

    pxr::UsdPrim prim = _mesh->GetPrim();
    std::cout << prim.GetPath() << std::endl;
    

  }

  _points= new Points();

  _points->SetPositions(positions);
  _points->SetWidths(widths);
  _points->SetColors(colors);
  //_points->SetInputOnly();

  _pointsId = _rootId.AppendChild(pxr::TfToken("points"));
  _scene.AddGeometry(_pointsId, _points);


  if(true)
  {
    size_t N = 1000;
    pxr::VtArray<pxr::GfVec3f> points(N);
    for(auto& point: points)
      point = pxr::GfVec3f(RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10), RANDOM_LO_HI(-10,10));

    pxr::VtArray<pxr::GfVec3f> result1(N);

    uint64_t startT1 = CurrentTime();
    for(size_t n = 0; n < N; ++n) {
      result1[n] = _ConstraintPointOnMesh(_mesh, points[n]);
    }
    uint64_t elapsedT1 = CurrentTime() - startT1;

    std::cout << (elapsedT1 * 1e-9) << std::endl;
    
    pxr::VtArray<pxr::GfVec3f> result2(N);

    uint64_t startT2 = CurrentTime();
    for(size_t n = 0; n < N; ++n) {
      Location hit;
      if(_bvh.Closest(points[n], &hit, FLT_MAX)) {
        result2[n] = hit.ComputePosition(_mesh->GetPositionsCPtr(), 
          &_mesh->GetTriangle(hit.GetComponentIndex())->vertices[0], 3);
      }
    }
    uint64_t elapsedT2 = CurrentTime() - startT2;

    std::cout << "brute force took : " << (elapsedT1 * 1e-6) << " seconds" << std::endl;
    std::cout << "with bvh took : " << (elapsedT2 * 1e-6) << " seconds" << std::endl;

  }

  _bvhId = _rootId.AppendChild(pxr::TfToken("bvh"));
  _instancer = _SetupBVHInstancer(stage, _bvhId, &_bvh, false);
  _scene.AddGeometry(_bvhId, (Geometry*)_instancer );
  _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);
  
}


void TestGradient::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  if(_mesh) {
    size_t numPoints = _mesh->GetNumPoints();
    const pxr::GfVec3f* positions = _mesh->GetPositionsCPtr();
    const pxr::GfRange3f range(_mesh->GetBoundingBox().GetRange());

    pxr::GfVec3f seed(
      RANDOM_LO_HI(-10, 10), 
      RANDOM_LO_HI(-10, 10), 
      RANDOM_LO_HI(-10, 10)
    );
    
    pxr::GfMatrix4f matrix;
    pxr::GfVec3f position;
    Location hit;
    pxr::VtArray<pxr::GfVec3f> points(3);
    pxr::VtArray<pxr::GfVec3f> colors(3);
    pxr::VtArray<float> widths(3, 1.f);
    points[0] = seed; 
    colors[0] = pxr::GfVec3f(1.f,0.f,0.f);
    colors[1] = pxr::GfVec3f(0.f,1.f,0.f);
    colors[2] = pxr::GfVec3f(0.f,0.f,1.f);

    //pxr::GfSqrt(_bvh.GetDistanceSquared(seed)) + _bvh.GetSize().GetLength() * 0.1f;
 
    if(_bvh.Closest(seed, &hit, FLT_MAX)) {
      Triangle* triangle = _mesh->GetTriangle(hit.GetComponentIndex());
      pxr::GfVec3f closest = hit.ComputePosition(positions, &triangle->vertices[0], 3);
      points[1] = closest;
    }
    

    /* Brute force
    float minDistance = FLT_MAX;
    for(size_t t = 0; t < _mesh->GetNumTriangles(); ++t) {
      Triangle* triangle = _mesh->GetTriangle(t);
      triangle->Closest(positions, seed, &hit);
    }

    Triangle* triangle = _mesh->GetTriangle(hit.GetComponentIndex());
    pxr::GfVec3f closest = hit.ComputePosition(positions, &triangle->vertices[0], 3);
    points[1] = closest;
    */

    seed = _ConstraintPointOnMesh(_mesh, seed);
    points[2] = seed;

    _points->SetPositions(points);
    _points->SetColors(colors);
    _points->SetWidths(widths);
  
    _scene.MarkPrimDirty(_pointsId, pxr::HdChangeTracker::DirtyPoints|pxr::HdChangeTracker::DirtyPrimvar);

    colors.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {

      colors[i][0] = RESCALE(positions[i][0], range.GetMin()[0], range.GetMax()[0], 0.f, 1.f);
      colors[i][1] = RESCALE(positions[i][1], range.GetMin()[1], range.GetMax()[1], 0.f, 1.f);
      colors[i][2] = RESCALE(positions[i][2], range.GetMin()[2], range.GetMax()[2], 0.f, 1.f);
    }
    _mesh->SetColors(colors);

    _scene.MarkPrimDirty(_meshId, pxr::HdChangeTracker::DirtyPrimvar);
  }

}

void TestGradient::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _scene.Remove(_pointsId);
}

void TestGradient::_TraverseStageFindingMesh(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _mesh = new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim));
      _meshId = prim.GetPath();
      break;
    }
}



JVR_NAMESPACE_CLOSE_SCOPE