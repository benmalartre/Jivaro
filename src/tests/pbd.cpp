#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/scene.h"
#include "../app/application.h"

#include "../tests/utils.h"
#include "../tests/pbd.h"

JVR_NAMESPACE_OPEN_SCOPE

void TestPBD::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath();
   pxr::GfQuatf rotate(0.f, 0.3827f, 0.9239f, 0.f);
  rotate.Normalize();

  // create collide ground
  _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, _groundId);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  _scene->AddGeometry(_groundId, _ground);
  
  // create solver with attributes
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(_scene, stage, _solverId);
  _scene->AddGeometry(_solverId, _solver);

  // create cloth meshes
  float size = .1f;
  rotate = pxr::GfQuatf(45.f * DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotate.Normalize();
  pxr::GfMatrix4d matrix =
    pxr::GfMatrix4d(1.0).SetScale(pxr::GfVec3f(5.f));
  
  for(size_t x = 0; x < 0; ++x) {
    
    std::string name = "cloth" + std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    _GenerateClothMesh(stage, clothPath, size,
      matrix * pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(x * 6.f, 5.f, 0.f)));
    
  }
  std::cout << "created cloth meshes" << std::endl;

  std::map<pxr::SdfPath, Sphere*> spheres;
  
  for (size_t x = 0; x < 1; ++x) {
    std::cout << "collide sphere" << std::endl;
    std::string name = "sphere_collide_" + std::to_string(x);
    pxr::SdfPath collidePath = rootId.AppendChild(pxr::TfToken(name));
    spheres[collidePath] =
      _GenerateCollideSphere(stage, collidePath, RANDOM_0_1 + 4.f, 
      pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(x * 6.f, 0.f, 0.f)));

    //sphere.GetRadiusAttr().Get(&radius);
    //pxr::GfMatrix4f m(sphere.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
  }
   
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene->AddMesh(prim.GetPath(), mesh);

      Body* body = _solver->CreateBody((Geometry*)mesh, pxr::GfMatrix4f(xform), 0.1f, 0.1f, 0.1f);
      _solver->CreateConstraints(body, Constraint::STRETCH, 10000.f, 0.2f);
      _solver->CreateConstraints(body, Constraint::BEND, 2000.f, 0.2f);
      _solver->AddElement(body, mesh, prim.GetPath());

      _bodyMap[prim.GetPath()] = body;
    }
  }
  Force* gravity = new GravitationalForce(pxr::GfVec3f(0.f, -9.8f, 0.f));
  _solver->AddForce(gravity);

  //_solver->AddElement(gravity, NULL, _groundId);

  _solver->WeightBoundaries();
  _solver->LockPoints();
  
  //_solver->AddForce(new DampingForce());
  
  pxr::GfVec3f pos;
  double radius;
  float restitution = 0.25;
  float friction = 0.5f;
  for (auto& sphere: spheres) {
    Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
    _solver->AddElement(collision, sphere.second, sphere.first);
  } 

  //Collision* collision = new PlaneCollision(_ground, _groundId, 1.f, 1.f);
  //_solver->AddElement(collision, _ground, _groundId);

  _scene->Update(stage, _solver->GetStartFrame());
  _solver->GetParticles()->SetAllState(Particles::ACTIVE);
  _solver->Update(stage, _solver->GetStartFrame());

}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene->Update(stage, time);
  _solver->Update(stage, time);

  
  pxr::UsdGeomXformCache xformCache(time);

  const size_t numParticles = _solver->GetNumParticles();
  const pxr::GfVec3f hitColor(1.f, 0.2f, 0.3f);
  Geometry* geom;
 
  for (auto& execPrim : _scene->GetPrims()) {

    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(execPrim.first);
    if (usdPrim.IsValid() && usdPrim.IsA<pxr::UsdGeomMesh>()) {
      const auto& bodyIt = _bodyMap.find(usdPrim.GetPath());
      if (bodyIt != _bodyMap.end()) {
        Body* body = bodyIt->second;
        Mesh* mesh = (Mesh*)execPrim.second.geom;
        mesh->SetPositions(&_solver->GetParticles()->_position[body->GetOffset()], mesh->GetNumPoints());
      } 

      execPrim.second.bits =
        pxr::HdChangeTracker::Clean |
        pxr::HdChangeTracker::DirtyPoints |
        pxr::HdChangeTracker::DirtyWidths |
        pxr::HdChangeTracker::DirtyPrimvar;
    }

    
  }
}

void TestPBD::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _scene->Remove(meshPath);
    }
  }
  delete _solver;
  delete _ground;

}

JVR_NAMESPACE_CLOSE_SCOPE