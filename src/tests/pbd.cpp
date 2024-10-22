#include <pxr/usd/usd/prim.h>
#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

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


void TestPBD::_TraverseStageFindingElements(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if(prim.HasAPI<pxr::UsdPbdBodyAPI>()) {
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        _clothes.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _clothesId.push_back(prim.GetPath());
      };
    } else if (prim.HasAPI<pxr::UsdPbdCollisionAPI>()) {
      if(prim.IsA<pxr::UsdGeomMesh>()) {
        _colliders.push_back(new Mesh(pxr::UsdGeomMesh(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<pxr::UsdGeomSphere>()) {
        _colliders.push_back(new Sphere(pxr::UsdGeomSphere(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<pxr::UsdGeomCube>()) {
        _colliders.push_back(new Cube(pxr::UsdGeomCube(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<pxr::UsdGeomCylinder>()) {
        _colliders.push_back(new Cylinder(pxr::UsdGeomCylinder(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<pxr::UsdGeomCapsule>()) {
        _colliders.push_back(new Capsule(pxr::UsdGeomCapsule(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      }
      
    }
}

void TestPBD::_AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(path);

  if(prim.IsValid()) {
    pxr::UsdGeomXformable xformable(prim);
    pxr::GfVec3f scale = pxr::GfVec3f(1.f, 1.f, 1.f);
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

  _scene.Init(stage);
  
  // get root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  _TraverseStageFindingElements(stage);

  for(size_t c = 0; c < _clothes.size(); ++c) {
    _clothes[c]->SetInputOutput();
    _scene.AddGeometry(_clothesId[c], _clothes[c]);
  }
  
  // create solver with attributes
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _CreateSolver(&_scene, stage, _solverId, 5);
  _scene.AddGeometry(_solverId, _solver);
  
  // create cloth meshes
  float size = .025f;


  for(size_t x = 0; x < 0; ++x) {
    std::string name = "Cloth_"+std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    const pxr::GfMatrix4d matrix = pxr::GfMatrix4d(1.f).SetScale(10.f) *
      pxr::GfMatrix4d(1.f).SetTranslate({ 0.f, 10.f + x, 0.f });
    Mesh* clothMesh = _CreateClothMesh(stage, clothPath, size, matrix);
    
    _clothesId.push_back(clothPath);
    _clothes.push_back(clothMesh);
    _scene.AddGeometry(clothPath, clothMesh);
  }
  
  
  for (size_t c = 0; c < _clothesId.size(); ++c) {
    size_t offset = _solver->GetNumParticles();

    Body* body = _solver->CreateBody((Geometry*)_clothes[c], 
      _clothes[c]->GetMatrix(), 1.f, size * 9.f, 0.1f, true);
    
    _solver->CreateConstraints(body, Constraint::BEND, 2000.f, 0.1f);
    _solver->CreateConstraints(body, Constraint::STRETCH, 10000.f, 0.5f);
    _solver->CreateConstraints(body, Constraint::SHEAR, 6000.f, 0.1f);
    
    _solver->AddElement(body, _clothes[c], _clothesId[c]);
    
  }
  
  float restitution, friction;

  bool createCollisions = true;
  if(createCollisions) {

    for (size_t c = 0; c < _collidersId.size(); ++c) {
      _scene.AddGeometry(_collidersId[c], _colliders[c]);
      _colliders[c]->SetInputOnly();
      Collision* collision = NULL;

      _colliders[c]->GetAttributeValue(pxr::UsdPbdTokens->pbdFriction, pxr::UsdTimeCode::Default(), &friction);
      _colliders[c]->GetAttributeValue(pxr::UsdPbdTokens->pbdRestitution, pxr::UsdTimeCode::Default(), &restitution);

      switch(_colliders[c]->GetType()) {
        case Geometry::CUBE:
          collision = new BoxCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::SPHERE:
          collision = new SphereCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CYLINDER:
          std::cerr << "Collision shape CYLINDER not implemented" << std::endl;
          break;

        case Geometry::CAPSULE:
          collision = new CapsuleCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CONE:
          std::cerr << "Collision shape CONE not implemented" << std::endl;
          break;

        case Geometry::MESH:
          collision = new MeshCollision(_colliders[c], _collidersId[c], restitution, friction);
          ((MeshCollision*)collision)->Init(_solver->GetNumParticles());
          break;
      }
      
      if(collision)
        _solver->AddElement(collision, _colliders[c], _collidersId[c]);
      
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