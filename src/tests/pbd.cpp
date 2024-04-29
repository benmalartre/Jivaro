#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/implicit.h"
#include "../geometry/utils.h"
#include "../geometry/scene.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

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
  _scene.AddGeometry(_groundId, _ground);
  
  // create solver with attributes
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId);
  _scene.AddGeometry(_solverId, _solver);

  // create cloth meshes
  float size = .1f;
  rotate = pxr::GfQuatf(45.f * DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotate.Normalize();
  pxr::GfMatrix4d matrix =
    pxr::GfMatrix4d().SetScale(pxr::GfVec3f(5.f));
  
  for(size_t x = 0; x < 0; ++x) {
    
    std::string name = "cloth" + std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    _GenerateClothMesh(stage, clothPath, size,
      matrix * pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(x * 6.f, 5.f, 0.f)));
    
  }
  std::cout << "created cloth meshes" << std::endl;

   // create collide spheres
  std::map<pxr::SdfPath, Sphere*> spheres;
  
  pxr::GfVec3f offset(10.f, 0.f, 0.f);
  pxr::GfVec3f axis(0.f,1.f,0.f);
  size_t n = 8;
  const double rStep = 360.0 / static_cast<double>(n);
/*
  for (size_t x = 0; x < n; ++x) {
    std::string name = "sphere_collide_" + std::to_string(x);
    pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
    pxr::GfRotation rotate(axis, x * rStep);
    spheres[collideId] =
      _GenerateCollideSphere(stage, collideId, RANDOM_0_1 + 2.f, pxr::GfMatrix4d().SetTranslate(rotate.TransformDir(offset)));

    _scene.AddGeometry(collideId, spheres[collideId]);
  }
  
  std::string name = "sphere_collide_ctr";
  pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
  spheres[collideId] =
    _GenerateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d());

    _AddAnimationSamples(stage, collideId);


  _scene.AddGeometry(collideId, spheres[collideId]);
  
*/
   
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      const pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene.AddGeometry(prim.GetPath(), mesh);

      Body* body = _solver->CreateBody((Geometry*)mesh, xform, 0.1f, 0.1f, 0.1f);
      _solver->CreateConstraints(body, Constraint::STRETCH, 1000.f, 0.f);
      //_solver->CreateConstraints(body, Constraint::BEND, 2000.f, 0.f);
      _solver->AddElement(body, mesh, prim.GetPath());

      _bodyMap[prim.GetPath()] = body;
    }
  }

  //_solver->AddElement(gravity, NULL, _groundId);

  _solver->WeightBoundaries();
  _solver->LockPoints();

  float restitution = 0.5f;
  float friction = 0.5f;
  for (auto& sphere : spheres) {
    Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
    _solver->AddElement(collision, sphere.second, sphere.first);
  }

  Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
  _solver->AddElement(collision, _ground, _groundId);

}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);

  for (auto& execPrim : _scene.GetPrims()) {

    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(execPrim.first);
    if (usdPrim.IsValid() && usdPrim.IsA<pxr::UsdGeomMesh>()) {
      const auto& bodyIt = _bodyMap.find(usdPrim.GetPath());
      if (bodyIt != _bodyMap.end()) {
        Body* body = bodyIt->second;
        Mesh* mesh = (Mesh*)execPrim.second.geom;
        mesh->SetPositions(&_solver->GetParticles()->position[body->GetOffset()], mesh->GetNumPoints());
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
      _scene.Remove(meshPath);
    }
  }
  delete _solver;
  delete _ground;

}

JVR_NAMESPACE_CLOSE_SCOPE