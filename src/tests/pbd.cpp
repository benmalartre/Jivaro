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


void TestPBD::_TraverseStageFindingElements(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if(prim.HasAPI<UsdPbdBodyAPI>()) {
      if (prim.IsA<UsdGeomMesh>()) {
        _clothes.push_back(new Mesh(UsdGeomMesh(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _clothesId.push_back(prim.GetPath());
      };
    } else if (prim.HasAPI<UsdPbdCollisionAPI>()) {
      if(prim.IsA<UsdGeomMesh>()) {
        _colliders.push_back(new Mesh(UsdGeomMesh(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<UsdGeomSphere>()) {
        _colliders.push_back(new Sphere(UsdGeomSphere(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<UsdGeomCube>()) {
        _colliders.push_back(new Cube(UsdGeomCube(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<UsdGeomCylinder>()) {
        _colliders.push_back(new Cylinder(UsdGeomCylinder(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      } else if (prim.IsA<UsdGeomCapsule>()) {
        _colliders.push_back(new Capsule(UsdGeomCapsule(prim), 
          xformCache.GetLocalToWorldTransform(prim)));
        _collidersId.push_back(prim.GetPath());
      }
      
    }
}

void TestPBD::_AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path)
{
  UsdPrim prim = stage->GetPrimAtPath(path);

  if(prim.IsValid()) {
    UsdGeomXformable xformable(prim);
    GfVec3f scale = GfVec3f(1.f, 1.f, 1.f);
    GfVec3d translate0 = GfVec3f(0.f, 0.f, 0.f);
    GfVec3d translate1 = GfVec3f(0.f, -10.f, 0.f);
    GfVec3d translate2 = GfVec3f(0.f, 10.f, 0.f);

    UsdGeomXformOp op = xformable.AddScaleOp();
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


void TestPBD::InitExec(UsdStageRefPtr& stage)
{

  std::cout << "Test PBD Init Execution" << std::endl;
  if (!stage) return;

  _scene.Init(stage);
  std::cout << " - initialized scene" << std::endl;

  // get root prim
  UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const SdfPath  rootId = rootPrim.GetPath();
  std::cout << " - found root prim" << rootId << std::endl;

  _TraverseStageFindingElements(stage);
  std::cout << " - traversed stage finding elements : " << _colliders.size() << std::endl;

  for(size_t c = 0; c < _clothes.size(); ++c) {
    _clothes[c]->SetInputOutput();
    _scene.AddGeometry(_clothesId[c], _clothes[c]);
  }
 
  
  // create solver with attributes
  _solverId = rootId.AppendChild(TfToken("Solver"));
  _solver = _CreateSolver(&_scene, stage, _solverId, 5);
  _scene.AddGeometry(_solverId, _solver);
  std::cout << " - created solver " << _solverId << std::endl;
  
  // create cloth meshes
  float size = .025f;


  for(size_t x = 0; x < 0; ++x) {
    std::string name = "Cloth_"+std::to_string(x);
    SdfPath clothPath = rootId.AppendChild(TfToken(name));
    const GfMatrix4d matrix = GfMatrix4d(1.f).SetScale(10.f) *
      GfMatrix4d(1.f).SetTranslate({ 0.f, 10.f + x, 0.f });
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
    std::cout << " - created cloth " << _clothesId[c] << std::endl;
    
  }
  
  float restitution, friction;

  bool createCollisions = true;
  if(createCollisions) {

    for (size_t c = 0; c < _collidersId.size(); ++c) {
      _scene.AddGeometry(_collidersId[c], _colliders[c]);
      _colliders[c]->SetInputOnly();
      Collision* collision = NULL;

      _colliders[c]->GetAttributeValue(UsdPbdTokens->pbdFriction, UsdTimeCode::Default(), &friction);
      _colliders[c]->GetAttributeValue(UsdPbdTokens->pbdRestitution, UsdTimeCode::Default(), &restitution);

      switch(_colliders[c]->GetType()) {
        case Geometry::CUBE:
          std::cout << " - created cube collision " << _clothesId[c] << std::endl;
          collision = new BoxCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::SPHERE:
          std::cout << " - created sphere collision " << _clothesId[c] << std::endl;
          collision = new SphereCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CYLINDER:
          std::cerr << "Collision shape CYLINDER not implemented" << std::endl;
          break;

        case Geometry::CAPSULE:
          std::cout << " - created capsule collision " << _clothesId[c] << std::endl;
          collision = new CapsuleCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CONE:
          std::cerr << "Collision shape CONE not implemented" << std::endl;
          break;

        case Geometry::MESH:
          std::cout << " - created mesh collision " << _clothesId[c] << std::endl;
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
    _groundId = rootId.AppendChild(TfToken("Ground"));
    _ground = _CreateCollidePlane(stage, _groundId);
    _ground->SetMatrix(
      GfMatrix4d().SetTranslate(GfVec3f(0.f, -0.5f, 0.f)));
    _scene.AddGeometry(_groundId, _ground);

    Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
    _solver->AddElement(collision, _ground, _groundId);
  }
  
  
  _solver->Reset();
  
  
  
}

void TestPBD::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);
}

void TestPBD::TerminateExec(UsdStageRefPtr& stage)
{
  std::cout << "TEST PBD TERMINATE EXEC" << std::endl;
  if (!stage) return;
  _scene.RemoveGeometry(_solverId);
  delete _solver;
  std::cout << "TEST PBD TERMINATED" << std::endl;
}

void TestPBD::PopulateSceneIndex(HdSceneIndexBase* index)
{
  std::cout << "TestPBD POPULATE SceneIndex Called!!!" << std::endl;
  //ExecSceneIndex* sceneIndex = (ExecSceneIndex*)index;
}

void TestPBD::RemoveFromSceneIndex(HdSceneIndexBase* index)
{
  std::cout << "TestPBD REMOVE SceneIndex Called!!!" << std::endl;
}

JVR_NAMESPACE_CLOSE_SCOPE