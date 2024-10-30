#include <pxr/usd/usd/prim.h>
#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/voxels.h"
#include "../geometry/points.h"
#include "../geometry/instancer.h"
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



static Voxels* _Voxelize(Mesh* mesh, float radius)
{
  Voxels *voxels = new Voxels();
  voxels->Init(mesh, radius*2.f);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build(0.0f);

  return voxels;
}


void TestParticles::_AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path)
{
  UsdPrim prim = stage->GetPrimAtPath(path);

  if(prim.IsValid()) {
    UsdGeomXformable xformable(prim);
    GfVec3d scale = GfVec3f(1.f, 1.f, 1.f);
    GfVec3d translate0 = GfVec3f(0.f, 0.f, 0.f);
    GfVec3d translate1 = GfVec3f(0.f, -10.f, 0.f);
    GfVec3d translate2 = GfVec3f(0.f, 10.f, 0.f);

    UsdGeomXformOp op = xformable.AddScaleOp();
    op.Set(scale);

    op = xformable.AddRotateYOp();
    op.Set( 0.f, 1);
    //op.Set(-45.f, 26);
    //op.Set( 45.f, 51);
    //op.Set(-45.f, 76);
    op.Set( 720.f, 101);

    op = xformable.AddTranslateOp();
    op.Set(translate0, 101);
    op.Set(translate2, 126);
    op.Set(translate1, 151);
    op.Set(translate2, 176);
    op.Set(translate0, 201);

  }
}

void TestParticles::_TraverseStageFindingElements(UsdStageRefPtr& stage)
{
  UsdGeomXformCache xformCache(UsdTimeCode::Default());
  for (UsdPrim prim : stage->TraverseAll())
    if (prim.HasAPI<UsdPbdCollisionAPI>()) {
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

void TestParticles::InitExec(UsdStageRefPtr& stage)
{
  if (!stage) return;

  float mass = 1.f;
  float radius = 0.25f;
  float damping = 0.1f;
  float restitution = 0.05f;
  float friction = 0.9f;

  // get root prim
  UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const SdfPath  rootId = rootPrim.GetPath();

  // find objects for collision
  _TraverseStageFindingElements(stage);
  
    // create solver with particles
  _solverId = rootId.AppendChild(TfToken("Solver"));
  _solver = _CreateSolver(&_scene, stage, _solverId);
  _scene.AddGeometry(_solverId, _solver);

  GfRotation rotation(GfVec3f(0.f,1.f,0.f), RANDOM_LO_HI(0.f, 360.f));

  GfMatrix4d scale = GfMatrix4d().SetScale(GfVec3f(10.f, 10.f, 10.f));
  GfMatrix4d rotate = GfMatrix4d().SetRotate(rotation);
  GfMatrix4d translate = GfMatrix4d().SetTranslate(GfVec3f(0.f, 25.f, 0.f));


  Application* app = Application::Get();
  Selection* selection = app->GetModel()->GetSelection();
  Mesh* emitter = NULL;

  if(selection->GetNumSelectedItems()) {
    for(size_t selected = 0; selected < selection->GetNumSelectedItems(); ++selected) {
      Selection::Item item = selection->GetItem(selected);
      if(item.type != Selection::PRIM) continue;
      UsdPrim prim = stage->GetPrimAtPath(item.path);
      if(prim.IsValid() && prim.IsA<UsdGeomMesh>()) {
        _emitterId = item.path;
        _emitter = new Mesh(UsdGeomMesh(prim), 
          UsdGeomMesh(prim).ComputeLocalToWorldTransform(UsdTimeCode::Default()));
        _emitter->SetInputOnly();
        break;
      }
    }
  }
  if(_emitterId.IsEmpty()) {
    std::cout << " nothing selected create cube emitter" << std::endl;
    _emitterId = _solverId.AppendChild(TfToken("emitter"));
    _emitter = new Mesh(scale * rotate * translate);
    _emitter->SetInputOnly();
    _emitter->Cube();
    _scene.InjectGeometry(stage, _emitterId, _emitter, 1.f);
  }

  _voxelsId = _solverId.AppendChild(TfToken("voxels"));
  _voxels = _Voxelize(_emitter, radius);
  _scene.InjectGeometry(stage, _voxelsId, _voxels);
  _scene.AddGeometry(_voxelsId, _voxels);

  UsdPbdBodyAPI::Apply(_voxels->GetPrim());

  std::cout << "voxels num cells " << _voxels->GetNumCells() << std::endl;
  std::cout << "voxels num points " << _voxels->GetNumPoints() << std::endl;

  
  //Points* points = new Points(UsdGeomPoints(voxels), xform);
  GfMatrix4d matrix(1.0);
  std::cout << "add particles " << std::endl;
  std::cout << _voxels << std::endl;

  Body* body = _solver->CreateBody((Geometry*)_voxels, matrix, mass, radius, damping, false);
  _solver->AddElement(body, _voxels, _emitterId);
  std::cout << "added particles" << _solver->GetNumParticles() << std::endl;


  bool createCollisions = true;
  if(createCollisions) {

    for (size_t c = 0; c < _collidersId.size(); ++c) {
      _scene.AddGeometry(_collidersId[c], _colliders[c]);
      _colliders[c]->SetInputOnly();
      Collision* collision = NULL;

      _colliders[c]->GetAttributeValue(UsdPbdTokens->pbdFriction, UsdTimeCode::Default(), &friction);
      _colliders[c]->GetAttributeValue(UsdPbdTokens->pbdRestitution, UsdTimeCode::Default(), &restitution);

      switch(_colliders[c]->GetType()) {
        case Geometry::PLANE:
          std::cerr << "Create collision shape PLANE for " << _collidersId[c] << std::endl;
          collision = new PlaneCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CUBE:
          std::cerr << "Create collision shape CUBE for " << _collidersId[c] << std::endl;
          collision = new BoxCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::SPHERE:
          std::cerr << "Create collision shape SPHERE for " << _collidersId[c] << std::endl;
          collision = new SphereCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CYLINDER:
          std::cerr << "Collision shape CYLINDER not implemented" << std::endl;
          break;

        case Geometry::CAPSULE:
          std::cerr << "Create collision shape CAPSULE for " << _collidersId[c] << std::endl;
          collision = new CapsuleCollision(_colliders[c], _collidersId[c], restitution, friction);
          break;

        case Geometry::CONE:
          std::cerr << "Collision shape CONE not implemented" << std::endl;
          break;

        case Geometry::MESH:
          std::cerr << "Create collision shape MESH for " << _collidersId[c] << std::endl;
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

  Particles* particles = _solver->GetParticles();

  _solver->Reset(stage);

}


void TestParticles::UpdateExec(UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);
  _solver->Update(stage, time);
}

void TestParticles::TerminateExec(UsdStageRefPtr& stage)
{
  if (!stage) return;
  _scene.RemoveGeometry(_solverId);
  //_scene.RemoveGeometry(_emitterId);
  delete _solver;
  delete _emitter;
}

JVR_NAMESPACE_CLOSE_SCOPE