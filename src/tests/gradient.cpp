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

JVR_NAMESPACE_OPEN_SCOPE

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
  _points->SetInputOnly();

  _pointsId = _rootId.AppendChild(pxr::TfToken("points"));
  _scene.AddGeometry(_pointsId, _points);


  /*
  _bvhId = _rootId.AppendChild(pxr::TfToken("bvh"));
  _instancer = _SetupPointsInstancer(stage, _bvhId, _points);
  _scene.AddGeometry(_bvhId, (Geometry*)_instancer );
  _scene.MarkPrimDirty(_bvhId, pxr::HdChangeTracker::DirtyInstancer);
  */

}


void TestGradient::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  if(_mesh) {
    size_t numPoints = _mesh->GetNumPoints();
    /*
    pxr::UsdGeomMesh usdMesh(_mesh->GetPrim());
    pxr::UsdGeomPrimvar colorPrimVar = usdMesh.GetDisplayColorPrimvar();
    if(!colorPrimVar) 
      colorPrimVar = usdMesh.CreateDisplayColorPrimvar(
        pxr::UsdGeomTokens->vertex, numPoints);

    colorPrimVar.SetInterpolation(pxr::UsdGeomTokens->vertex);
    */
    pxr::VtArray<pxr::GfVec3f> colors(numPoints);
    for(auto& color: colors)color = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    _mesh->SetColors(colors);

    _scene.MarkPrimDirty(_meshId, pxr::HdChangeTracker::AllDirty);
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