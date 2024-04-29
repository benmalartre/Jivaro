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

Instancer* _GenerateInstancer(size_t numPoints, size_t numPrototypes)
{

  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(RANDOM_LO_HI(-5,5), pointIdx, RANDOM_LO_HI(-5,5));
    scales[pointIdx] = pxr::GfVec3f(RANDOM_0_1 + 0.5);
    protoIndices[pointIdx] = RANDOM_LO_HI(0, numPrototypes-1);
    indices[pointIdx] = pointIdx;
    colors[pointIdx] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
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

  pxr::VtArray<pxr::GfVec3f> points = instancer->GetPositions();
  //pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  //pxr::VtArray<pxr::GfQuath> rotations(numPoints);

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] += pxr::GfVec3f(RANDOM_LO_HI(-1,1), RANDOM_LO_HI(-1,1), RANDOM_LO_HI(-1,1));
    //scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    //rotations[pointIdx] = pxr::GfQuath::GetIdentity();
  }

  instancer->SetPositions(points);
}


void TestInstancer::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;


  // root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  const size_t numProtos = 8;

  // instancer
  _instancerId = rootId.AppendChild(pxr::TfToken("instancer"));
  _instancer = _GenerateInstancer(32, numProtos);

  // prototypes

  _protos.resize(numProtos);
  _protosId.resize(numProtos);
  for(size_t p  =0; p < numProtos; ++p) {
    _protosId[p] = rootId.AppendChild(pxr::TfToken("proto_"+std::to_string(p)));
    _protos[p] = new Mesh();
    _GenerateRandomTriangle(_protos[p]);
    _instancer->AddPrototype(_protosId[p]);
  }

  // ground
  _groundId = rootId.AppendChild(pxr::TfToken("ground"));
  _ground = new Mesh(pxr::GfMatrix4d().SetScale(pxr::GfVec3f(10.f)));
  _GenerateRandomGround(_ground);

  // inject in usd scene
  for(size_t p  =0; p < numProtos; ++p) {
    _scene.InjectGeometry(stage, _protosId[p], _protos[p], 1.f);
  }
  _scene.InjectGeometry(stage, _groundId, _ground, 1.f);
  _scene.InjectGeometry(stage, _instancerId, _instancer, 1.f);
  
  for(size_t i = 2; i < 101 ; i+= 10) {
    float time = i;
    _UpdateInstancer(_instancer, time);
    _scene.InjectGeometry(stage, _instancerId, _instancer, time);
  }
  

  //_instancer = (Instancer*)_scene.AddGeometry(_instancerId, Geometry::INSTANCER, pxr::GfMatrix4d());
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