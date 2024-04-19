#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/application.h"
#include "../command/block.h"

#include "../tests/utils.h"
#include "../tests/particles.h"

JVR_NAMESPACE_OPEN_SCOPE

class TestParticles : public Execution {
public:
  friend class Scene;
  TestParticles() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;
private:
  Solver*                                                    _solver;
  Plane*                                                     _ground;
  Points*                                                    _points;
  pxr::SdfPath                                               _groundId;
  pxr::SdfPath                                               _solverId;
  pxr::SdfPath                                               _pointsId;

};

Execution* CreateTestParticles()
{
  return new TestParticles();
}


static Voxels* _Voxelize(pxr::UsdGeomMesh& usdMesh, pxr::SdfPath& path, float radius)
{
  Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
  Voxels *voxels = new Voxels();
  voxels->Init(&mesh, radius);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build();

  std::cout << "voxels num cells : " << voxels->GetNumCells() << std::endl;
  return voxels;
}


void TestParticles::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  
    // create solver with particles
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId);
  _scene.AddGeometry(_solverId, _solver);

  // create collide ground
  _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, _groundId);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  _scene.AddGeometry(_groundId, _ground);

  // create collide spheres
  std::map<pxr::SdfPath, Sphere*> spheres;
  /*
  pxr::GfVec3f offset(10.f, 0.f, 0.f);
  pxr::GfVec3f axis(0.f,1.f,0.f);
  size_t n = 8;
  const double rStep = 360.0 / static_cast<double>(n);

  for (size_t x = 0; x < n; ++x) {
    std::string name = "sphere_collide_" + std::to_string(x);
    pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
    pxr::GfRotation rotate(axis, x * rStep);
    spheres[collideId] =
      _GenerateCollideSphere(stage, collideId, RANDOM_0_1 + 2.f, pxr::GfMatrix4d(1.f).SetTranslate(rotate.TransformDir(offset)));

    _scene.AddGeometry(collideId, spheres[collideId]);
  }
  */
  std::string name = "sphere_collide_ctr";
  pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
  spheres[collideId] =
    _GenerateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d(1.f));

  _scene.AddGeometry(collideId, spheres[collideId]);
   

  float mass = 0.1f;
  float radius = 0.25f;
  float damping = 0.1f;

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      pxr::SdfPath voxelId = usdMesh.GetPath().AppendChild(pxr::TfToken("voxels"));

      Voxels* voxels = _Voxelize(usdMesh, voxelId, 0.25);
      pxr::SdfPath bvhId = _solverId.AppendChild(pxr::TfToken("bvh"));
      _SetupBVHInstancer(stage, bvhId,voxels->GetTree());
      
      //Points* points = new Points(pxr::UsdGeomPoints(voxels), xform);
      _scene.AddVoxels(voxelId, voxels);

      Body* body = _solver->CreateBody((Geometry*)voxels, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, (Geometry*)voxels, prim.GetPath());
      
      /*
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene.AddMesh(prim.GetPath(), mesh);
      Body* body = _solver->CreateBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, mesh, prim.GetPath());
      Scene::_Prim* sPrim = _scene.GetPrim(prim.GetPath());
      sPrim->bits = pxr::HdChangeTracker::AllDirty;
      */
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Points* points = new Points(usdPoints.GetPrim(), xform);
      _scene.AddGeometry(prim.GetPath(), points);

      Body* body = _solver->CreateBody((Geometry*)points, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, points, prim.GetPath());
    } else if(prim.IsA<pxr::UsdGeomBasisCurves>()) {

    }
  }
  Force* gravity = new GravitationalForce(pxr::GfVec3f(0.f, -9.81f, 0.f));
  _solver->AddForce(gravity);
  _solver->AddElement(gravity, NULL, _solverId.AppendChild(pxr::TfToken("Gravity")));


  float restitution = 0.5f;
  float friction = 0.5f;
  for (auto& sphere : spheres) {
    Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
    _solver->AddElement(collision, sphere.second, sphere.first);
  }

  Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
  _solver->AddElement(collision, _ground, _groundId);

  _solver->GetParticles()->SetAllState(Particles::ACTIVE);
  _solver->Update(stage, _solver->GetStartFrame());

}


void TestParticles::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);

}

void TestParticles::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _scene.Remove(meshPath);
    }
  }
  delete _solver;
  delete _ground;
}

JVR_NAMESPACE_CLOSE_SCOPE