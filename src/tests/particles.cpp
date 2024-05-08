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
  voxels->Init(mesh, radius*2.5f);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build(0.05f);

  return voxels;
}

void TestParticles::_AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path)
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

void TestParticles::_TraverseStageFindingMeshes(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  for (pxr::UsdPrim prim : stage->TraverseAll())
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      _meshes.push_back(new Mesh(pxr::UsdGeomMesh(prim), xformCache.GetLocalToWorldTransform(prim)));
      _meshesId.push_back(prim.GetPath());
      _meshes.back()->SetInputOnly();
    } 
}


void TestParticles::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  float mass = 0.1f;
  float radius = 0.5f;
  float damping = 0.1f;
  float restitution = 0.05f;
  float friction = 0.9f;

  // get root prim
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  if(!rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    rootPrim = root.GetPrim();
    stage->SetDefaultPrim(rootPrim);
  }
  const pxr::SdfPath  rootId = rootPrim.GetPath();

  // find meshes for collision
  _TraverseStageFindingMeshes(stage);
  
    // create solver with particles
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId);
  _scene.AddGeometry(_solverId, _solver);


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
  */
  std::string name = "sphere_collide_ctr";
  pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
  spheres[collideId] =
    _GenerateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d(1.0), friction, restitution);

    //_AddAnimationSamples(stage, collideId);


  _scene.AddGeometry(collideId, spheres[collideId]);
  

  axis = pxr::GfVec3f(RANDOM_LO_HI(-1.f, 1.f), RANDOM_LO_HI(-1.f, 1.f), RANDOM_LO_HI(-1.f, 1.f));
  axis.Normalize();

  pxr::GfRotation rotation(axis, RANDOM_LO_HI(0.f, 360.f));

  pxr::GfMatrix4d scale = pxr::GfMatrix4d().SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d().SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, 25.f, 0.f));

  pxr::SdfPath emitterId;

  Application* app = Application::Get();
  Selection* selection = app->GetSelection();
  Mesh* emitter = NULL;

  if(selection->GetNumSelectedItems()) {
    for(size_t selected = 0; selected < selection->GetNumSelectedItems(); ++selected) {
      Selection::Item item = selection->GetItem(selected);
      if(item.type != Selection::PRIM) continue;
      pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
      if(prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
        emitterId = item.path;
        emitter = new Mesh(pxr::UsdGeomMesh(prim), 
          pxr::UsdGeomMesh(prim).ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
        emitter->SetInputOnly();
        break;
      }
    }
  }
  if(emitterId.IsEmpty()) {
    emitterId = _solverId.AppendChild(pxr::TfToken("emitter"));
    emitter = new Mesh(scale * rotate * translate);
    emitter->SetInputOnly();
    emitter->Cube();
    _scene.InjectGeometry(stage, emitterId, emitter, 1.f);
  }

  _voxels = _Voxelize(emitter, radius);
  delete emitter;

  std::cout << "voxels num cells " << _voxels->GetNumCells() << std::endl;
  std::cout << "voxels num points " << _voxels->GetNumPoints() << std::endl;

  
  //Points* points = new Points(pxr::UsdGeomPoints(voxels), xform);
  pxr::GfMatrix4d matrix(1.0);
  Body* body = _solver->CreateBody((Geometry*)_voxels, matrix, mass, radius, damping);
  _solver->AddElement(body, _voxels, emitterId);
  std::cout << "added particles" << std::endl;

/*
  for(size_t i = 0; i < _meshes.size(); ++i) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(_meshesId[i]);
    pxr::UsdGeomMesh usdMesh(prim);
    _scene.AddGeometry(prim.GetPath(), (Mesh*)_meshes[i]);
    Collision* collision = new MeshCollision(_meshes[i], prim.GetPath(), restitution, friction);
    _solver->AddElement(collision, _meshes[i], prim.GetPath());
    std::cout << "Add mesh collision " << prim.GetPath() << std::endl;
  }
*/

  

  bool createGroundCollision = true;
  if(createGroundCollision) {
    // create collide ground
    _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
    _ground = _GenerateCollidePlane(stage, _groundId, friction, restitution);
    _ground->SetMatrix(
      pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
    //_AddAnimationSamples(stage, _groundId);
    _scene.AddGeometry(_groundId, _ground);

    Collision* planeCollide = new PlaneCollision(_ground, _groundId, restitution, friction);
    _solver->AddElement(planeCollide, _ground, _groundId);

    std::cout << "added ground collision" << std::endl;
  }

  for (auto& sphere : spheres) {
    Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
    _solver->AddElement(collision, sphere.second, sphere.first);
  }
  std::cout<< "added sphere collision" <<std::endl;

  bool createSelfCollision = true;
  if (createSelfCollision) {
    pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));
    Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, restitution, friction);
    _solver->AddElement(selfCollide, NULL, selfCollideId);
    std::cout << "added self collision" << std::endl;
  }

  _lastTime = FLT_MAX;

  std::cout << "update first frame" << std::endl;
  Particles* particles = _solver->GetParticles();

  pxr::GfRange3f range;
  for (size_t p = 0; p < particles->GetNumParticles(); ++p) {
    range = range.UnionWith(particles->position[p]);
  }
  std::vector<int> used;
  for(size_t p = 0; p < _solver->GetNumParticles(); ++p)
    if (particles->position[p][1] < range.GetMidpoint()[1]) {
      used.push_back(p);
      particles->color[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }
  //collision->SetMask(_solver->GetNumParticles(), used);


  UpdateExec(stage, _solver->GetStartTime());

}


void TestParticles::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene.Sync(stage, time);

  if(time != _lastTime) {
    _solver->Update(stage, time);
    _lastTime = time;
  }

}

void TestParticles::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;
  _scene.RemoveGeometry(_solverId);
  delete _solver;
}

JVR_NAMESPACE_CLOSE_SCOPE