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


void TestPBD::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _collideMeshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
        xformCache.GetLocalToWorldTransform(prim)));
      _collideMeshesId.push_back(prim.GetPath());
    }
}


void TestPBD::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  // get root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  _TraverseStageFindingMeshes(stage);

  // create collide ground
  _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, _groundId);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  _scene.AddGeometry(_groundId, _ground);
  
  // create solver with attributes
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId, 5);
  _scene.AddGeometry(_solverId, _solver);

  // create cloth meshes
  float size = .01f;

  
  for(size_t x = 0; x < 5; ++x) {
    std::string name = "cloth_"+std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    _clothMeshesId.push_back(clothPath);
    _clothMeshes.push_back(_GenerateClothMesh(stage, clothPath, size, 
    pxr::GfMatrix4d(1.f).SetScale(10.f) * pxr::GfMatrix4d(1.f).SetTranslate({0.f, 10.f+x, 0.f})));

    _scene.AddGeometry(_clothMeshesId.back(), _clothMeshes.back());
    
  }
  std::cout << "created cloth meshes" << std::endl;

   // create collide spheres
  std::map<pxr::SdfPath, Sphere*> spheres;
  
  pxr::GfVec3f offset(10.f, 0.f, 0.f);
  pxr::GfVec3f axis(0.f,1.f,0.f);
  size_t n = 8;
  const double rStep = 360.0 / static_cast<double>(n);


  std::string name = "sphere_collide_ctr";
  pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
  spheres[collideId] =
    _GenerateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d(1.f));
  //_AddAnimationSamples(stage, collideId);
  _scene.AddGeometry(collideId, spheres[collideId]);


  
  for (size_t c = 0; c < _clothMeshesId.size(); ++c) {
    size_t offset = _solver->GetNumParticles();

    Body* body = _solver->CreateBody((Geometry*)_clothMeshes[c], 
      _clothMeshes[c]->GetMatrix(), 1.f, size * 9.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::BEND, 20000.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::STRETCH, 100000.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::SHEAR, 50000.f, 0.1f);
    
    _solver->AddElement(body, _clothMeshes[c], _clothMeshesId[c]);
    
  }

  //_solver->AddElement(gravity, NULL, _groundId);


  float restitution = 0.1f;
  float friction = 0.5f;
  bool createSphereCollision = true;
  if(createSphereCollision) {
    for (auto& sphere : spheres) {
      Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
      _solver->AddElement(collision, sphere.second, sphere.first);
     }
  }

  bool createGroundCollision = true;
  if(createGroundCollision) {
    Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
    _solver->AddElement(collision, _ground, _groundId);
  }

  bool createSelfCollision = true;
  if (createSelfCollision) {
    pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, 1.f, 1.f);
    _solver->AddElement(selfCollide, NULL, selfCollideId);
  }

  bool createMeshCollision = true;
  if(createMeshCollision) {
    for (size_t c = 0; c < _collideMeshesId.size(); ++c) {
      _scene.AddGeometry(_collideMeshesId[c], _collideMeshes[c]);
      _collideMeshes[c]->SetInputOnly();
      Collision* meshCollide = new MeshCollision(_collideMeshes[c], _collideMeshesId[c], 1.f, 1.f);
      _solver->AddElement(meshCollide, _collideMeshes[c], _collideMeshesId[c]);
      
    }
  }
}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  for(auto& collision: _solver->GetCollisions()){
    const Geometry* geometry = collision->GetGeometry();
    if(geometry)
      collision->Update(stage->GetPrimAtPath(geometry->GetPrim().GetPath()), time);
  }
    

  _solver->Update(stage, time);
}

void TestPBD::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
  _scene.RemoveGeometry(_solverId);
  delete _solver;

}

JVR_NAMESPACE_CLOSE_SCOPE