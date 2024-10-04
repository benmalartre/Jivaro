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

void TestPBD::_AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);

  if(prim.IsValid()) {
    pxr::UsdGeomXformable xformable(prim);
    pxr::GfVec3d scale = pxr::GfVec3f(1.f, 1.f, 1.f);
    pxr::GfVec3d translate0 = pxr::GfVec3f(0.f, 0.f, 0.f);
    pxr::GfVec3d translate1 = pxr::GfVec3f(0.f, -10.f, 0.f);
    pxr::GfVec3d translate2 = pxr::GfVec3f(0.f, 10.f, 0.f);

    pxr::UsdGeomXformOp op = xformable.AddScaleOp();
    op.Set(scale);

    op = xformable.AddRotateYOp();
    op.Set( 0.f, 1);
    op.Set(-45.f, 26);
    op.Set( 45.f, 51);
    op.Set(-45.f, 76);
    op.Set( 720.f, 101);

    op = xformable.AddTranslateOp();
    op.Set(translate0, 101);
    op.Set(translate2, 126);
    op.Set(translate1, 151);
    op.Set(translate2, 176);
    op.Set(translate0, 201);

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
  
  // create solver with attributes
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _CreateSolver(&_scene, stage, _solverId, 5);
  _scene.AddGeometry(_solverId, _solver);

  // create cloth meshes
  float size = .016f;

  for(size_t x = 0; x < 5; ++x) {
    std::string name = "Cloth_"+std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    Mesh* clothMesh = _CreateClothMesh(stage, clothPath, size, 
      pxr::GfMatrix4d(1.f).SetScale(10.f) * pxr::GfMatrix4d(1.f).SetTranslate({0.f, 10.f+x, 0.f}));
    clothMesh->SetInputOutput();
    _clothMeshesId.push_back(clothPath);
    _clothMeshes.push_back(clothMesh);
    _scene.AddGeometry(clothPath, clothMesh);
  }

  for (size_t c = 0; c < _clothMeshesId.size(); ++c) {
    size_t offset = _solver->GetNumParticles();

    Body* body = _solver->CreateBody((Geometry*)_clothMeshes[c], 
      _clothMeshes[c]->GetMatrix(), 0.1f, size * 9.f, 0.1f);
    //_solver->CreateConstraints(body, Constraint::BEND, 20000.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::STRETCH, 60000.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::SHEAR, 60000.f, 0.1f);
    
    _solver->AddElement(body, _clothMeshes[c], _clothMeshesId[c]);
    
  }

  float restitution = 0.1f;
  float friction = 0.5f;

  bool createSphereCollision = true;
  if(createSphereCollision) {

   // create collide spheres
    std::map<pxr::SdfPath, Sphere*> spheres;
    
    pxr::GfVec3f offset(10.f, 0.f, 0.f);
    pxr::GfVec3f axis(0.f,1.f,0.f);
    size_t n = 8;
    const double rStep = 360.0 / static_cast<double>(n);


    std::string name = "sphere_collide_ctr";
    pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
    spheres[collideId] =
      _CreateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d(1.f));
    _AddAnimationSamples(stage, collideId);
    _scene.AddGeometry(collideId, spheres[collideId]);

    for (auto& sphere : spheres) {
      Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
      _solver->AddElement(collision, sphere.second, sphere.first);
     }
  }

  bool createGroundCollision = true;
  if(createGroundCollision) {
      // create collide ground
    _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
    _ground = _CreateCollidePlane(stage, _groundId);
    _ground->SetMatrix(
      pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
    _scene.AddGeometry(_groundId, _ground);

    Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
    _solver->AddElement(collision, _ground, _groundId);
  }

  bool createMeshCollision = true;
  if(createMeshCollision) {
    for (size_t c = 0; c < _collideMeshesId.size(); ++c) {
      _scene.AddGeometry(_collideMeshesId[c], _collideMeshes[c]);
      _collideMeshes[c]->SetInputOnly();
      Collision* meshCollide = new MeshCollision(_collideMeshes[c], _collideMeshesId[c], 1.f, 1.f);
      meshCollide->Init(_solver->GetNumParticles());
      _solver->AddElement(meshCollide, _collideMeshes[c], _collideMeshesId[c]);
      
    }
  }

  bool createSelfCollision = true;
  if (createSelfCollision) {
    pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, 0.f, 1.f);
    _solver->AddElement(selfCollide, NULL, selfCollideId);
  }

  _solver->Reset();
}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);
}

void TestPBD::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
  _scene.RemoveGeometry(_solverId);
  delete _solver;

}

JVR_NAMESPACE_CLOSE_SCOPE