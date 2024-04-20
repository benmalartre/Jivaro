#include "../geometry/instancer.h"
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
  proto->Set(positions, faceCounts, faceIndices);
}

void TestInstancer::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  _proto1Id = rootId.AppendChild(pxr::TfToken("proto1"));
  _proto2Id = rootId.AppendChild(pxr::TfToken("proto2"));
  _instancerId = root.AppendChild(pxr::TfToken("instancer"));

  _proto1 = new Mesh();
  _GenerateRandomTriangle(_proto1);
  _proto2 = new Mesh;
  _GenerateRandomTriangle(_proto1);

  //_proto1->InsertInStage(stage, _proto1Id);
  //_proto2->InsertInStage(stage, _proto2Id);

  _instancer = _scene.AddGeometry(_instancerId, Geometry::INSTANCER, pxr::GfMatrix4d(1.f));
}

void TestInstancer::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  
}

void TestInstancer::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;


}

JVR_NAMESPACE_CLOSE_SCOPE