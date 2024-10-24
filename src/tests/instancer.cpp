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

void _CreateRandomTriangle(Mesh* proto)
{
  VtArray<GfVec3f> positions(3);
  for(size_t i = 0 ; i < 3; ++i) {
    positions[i][0] = RANDOM_LO_HI(-1.f, 1.f);
    positions[i][1] = RANDOM_LO_HI(-1.f, 1.f);
    positions[i][2] = RANDOM_LO_HI(-1.f, 1.f);
  }
  VtArray<int> faceCounts = { 3 };
  VtArray<int> faceIndices = { 0, 1, 2 };
  proto->Set(positions, faceCounts, faceIndices);
}

void _CreateRandomGround(Mesh* ground)
{
  ground->RegularGrid2D(0.005);
  VtArray<GfVec3f>& positions = ground->GetPositions();

  float octaves = 16;
  float persistence = 0.5f;
  float amplitude = 0.2f;

  for(size_t p = 0; p < positions.size(); ++p) {
    positions[p][1] += Perlin::Perlin3D(
      positions[p][0], 
      positions[p][1], 
      positions[p][2], 
      octaves, 
      persistence) * amplitude;
  }
}

Instancer* _CreateInstancer(size_t numPoints, size_t numPrototypes)
{

  VtArray<GfVec3f> points(numPoints);
  VtArray<GfVec3f> scales(numPoints);
  VtArray<int64_t> indices(numPoints);
  VtArray<int> protoIndices(numPoints);
  VtArray<GfQuath> rotations(numPoints);
  VtArray<GfVec3f> colors(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = GfVec3f(RANDOM_LO_HI(-5,5), pointIdx, RANDOM_LO_HI(-5,5));
    scales[pointIdx] = GfVec3f(RANDOM_0_1 + 0.5);
    protoIndices[pointIdx] = RANDOM_LO_HI(0, numPrototypes);
    indices[pointIdx] = pointIdx;
    colors[pointIdx] = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    rotations[pointIdx] = GfQuath::GetIdentity();
  }

  Instancer* instancer = new Instancer;
  instancer->Set(
    points, 
    &protoIndices,
    &indices,
    &scales,
    &rotations,
    &colors
  );

  return instancer;
}

void _UpdateInstancer(Instancer* instancer, float time)
{
  size_t numPoints = instancer->GetNumPoints();

  VtArray<GfVec3f> points = instancer->GetPositions();
  //VtArray<GfVec3f> scales(numPoints);
  //VtArray<GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] += GfVec3f(RANDOM_LO_HI(-1,1), RANDOM_LO_HI(-1,1), RANDOM_LO_HI(-1,1));
    //scales[pointIdx] = GfVec3f(cells[pointIdx]->GetSize());
    //rotations[pointIdx] = GfQuath::GetIdentity();
  }

  instancer->SetPositions(points);

}


void TestInstancer::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;


  // root prim
  UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const SdfPath  rootId = rootPrim.GetPath();
  const size_t numProtos = 8;

  // instancer
  _instancerId = rootId.AppendChild(TfToken("instancer"));
  _instancer = _CreateInstancer(32, numProtos);
  _instancer->SetInputOutput();
  _scene.AddGeometry(_instancerId, _instancer);

  _scene.MarkPrimDirty(_instancerId, HdChangeTracker::DirtyTransform |
                                   HdChangeTracker::DirtyPrimvar |
                                   HdChangeTracker::DirtyInstanceIndex);

  // prototypes

  _protos.resize(numProtos);
  _protosId.resize(numProtos);
  for(size_t p  =0; p < numProtos; ++p) {
    _protosId[p] = rootId.AppendChild(TfToken("proto_"+std::to_string(p)));
    _protos[p] = new Mesh();
    _protos[p]->SetInputOutput();
    _CreateRandomTriangle(_protos[p]);
    _scene.AddGeometry(_protosId[p], _protos[p]);
    _instancer->AddPrototype(_protosId[p]);
  }

  // ground
  _groundId = rootId.AppendChild(TfToken("ground"));
  _ground = new Mesh(GfMatrix4d().SetScale(GfVec3f(10.f)));
  _CreateRandomGround(_ground);

  // inject in usd scene
  for(size_t p  =0; p < numProtos; ++p) {
    _scene.InjectGeometry(stage, _protosId[p], _protos[p], 1.f);
  }
  _scene.InjectGeometry(stage, _groundId, _ground, 1.f);
  _scene.AddGeometry(_groundId, _ground);
  
  _scene.InjectGeometry(stage, _instancerId, _instancer, 1.f);
  _scene.MarkPrimDirty(_instancerId, HdChangeTracker::AllDirty);
  

}

void TestInstancer::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _UpdateInstancer(_instancer, time);
  //_scene.InjectGeometry(stage, _instancerId, _instancer, time);
  
}

void TestInstancer::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;


}

JVR_NAMESPACE_CLOSE_SCOPE