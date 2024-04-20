#include <pxr/usd/usdGeom/xform.h>

#include "../geometry/instancer.h"
#include "../geometry/mesh.h"
#include "../geometry/perlin.h"
#include "../tests/instancer.h"
#include "../tests/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

class Curve;
class Mesh;
class Points;
class BVH;

void _GenerateRandomTriangle(Mesh* proto)
{
  pxr::VtArray<pxr::GfVec3f> positions(3);
  for(size_t i = 0 ; i < 3; ++i) {
    positions[i][0] = RANDOM_LO_HI(-1.f, 1.f);
    positions[i][1] = RANDOM_LO_HI(-1.f, 1.f);
    positions[i][2] = RANDOM_LO_HI(-1.f, 1.f);
  }
  pxr::VtArray<int> faceCounts = { 3 };
  pxr::VtArray<int> faceIndices = { 0, 1, 2 };
  proto->Set(positions, faceCounts, faceIndices);
}

void _GenerateRandomGround(Mesh* ground)
{
  ground->RegularGrid2D(0.005);
  pxr::VtArray<pxr::GfVec3f>& positions = ground->GetPositions();

  float octaves = 3;
  float persistence = 0.5f;

  for(size_t p = 0; p < positions.size(); ++p) {
    positions[p][1] += Perlin::Perlin3D(
      positions[p][0], 
      positions[p][1], 
      positions[p][2], 
      octaves, 
      persistence);
  }
}

void TestInstancer::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;


  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  _proto1Id = rootId.AppendChild(pxr::TfToken("proto1"));
  _proto2Id = rootId.AppendChild(pxr::TfToken("proto2"));
  _groundId = rootId.AppendChild(pxr::TfToken("ground"));
  _instancerId = rootId.AppendChild(pxr::TfToken("instancer"));

  _proto1 = new Mesh();
  _GenerateRandomTriangle(_proto1);
  _proto2 = new Mesh;
  _GenerateRandomTriangle(_proto2);

  _ground = new Mesh();
  _GenerateRandomGround(_ground);



  _scene.InjectGeometry(stage, _proto1Id, _proto1, 1.f);
  _scene.InjectGeometry(stage, _proto2Id, _proto2, 1.f);
  _scene.InjectGeometry(stage, _groundId, _ground, 1.f);

  //_instancer = (Instancer*)_scene.AddGeometry(_instancerId, Geometry::INSTANCER, pxr::GfMatrix4d(1.f));
}

void TestInstancer::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  //_scene.Sync(stage, time);
  
}

void TestInstancer::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;


}

JVR_NAMESPACE_CLOSE_SCOPE