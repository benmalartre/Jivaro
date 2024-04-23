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



static Voxels* _Voxelize(Mesh* mesh, float radius)
{
  Voxels *voxels = new Voxels();
  voxels->Init(mesh, radius);
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


void TestParticles::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  float mass = 2.f;
  float radius = 0.25f;
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
  
    // create solver with particles
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(&_scene, stage, _solverId);
  _scene.AddGeometry(_solverId, _solver);

  // create collide ground
  _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, _groundId, friction, restitution);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  //_AddAnimationSamples(stage, _groundId);
  _scene.AddGeometry(_groundId, _ground);

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
      _GenerateCollideSphere(stage, collideId, RANDOM_0_1 + 2.f, pxr::GfMatrix4d(1.f).SetTranslate(rotate.TransformDir(offset)));

    _scene.AddGeometry(collideId, spheres[collideId]);
  }
  */
  std::string name = "sphere_collide_ctr";
  pxr::SdfPath collideId = rootId.AppendChild(pxr::TfToken(name));
  spheres[collideId] =
    _GenerateCollideSphere(stage, collideId, 4.f, pxr::GfMatrix4d(1.f), friction, restitution);

    _AddAnimationSamples(stage, collideId);


  _scene.AddGeometry(collideId, spheres[collideId]);
  

  axis = pxr::GfVec3f(RANDOM_LO_HI(-1.f, 1.f), RANDOM_LO_HI(-1.f, 1.f), RANDOM_LO_HI(-1.f, 1.f));
  axis.Normalize();

  pxr::GfRotation rotation(axis, RANDOM_LO_HI(0.f, 360.f));

  pxr::GfMatrix4d scale = pxr::GfMatrix4d().SetScale(pxr::GfVec3f(10.f, 10.f, 10.f));
  pxr::GfMatrix4d rotate = pxr::GfMatrix4d().SetRotate(rotation);
  pxr::GfMatrix4d translate = pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, 25.f, 0.f));


  pxr::SdfPath emitterId = _solverId.AppendChild(pxr::TfToken("emitter"));
  Mesh* emitter = new Mesh(scale * rotate * translate);
  emitter->Cube();
  emitter->SetInputOnly();

  _scene.MarkPrimDirty(emitterId, pxr::HdChangeTracker::AllDirty);
  
  _scene.InjectGeometry(stage, emitterId, emitter, 1.f);


  Voxels* voxels = _Voxelize(emitter, 0.5f);

  std::cout << "voxels num cells " << voxels->GetNumCells() << std::endl;
  std::cout << "voxels num points " << voxels->GetNumPoints() << std::endl;

  
  //Points* points = new Points(pxr::UsdGeomPoints(voxels), xform);

  Body* body = _solver->CreateBody((Geometry*)voxels, pxr::GfMatrix4f(), mass, radius, damping);
  _solver->AddElement(body, voxels, emitterId);
  
  /*

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {

      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      pxr::SdfPath voxelId = usdMesh.GetPath().AppendChild(pxr::TfToken("voxels"));

      
      
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene.AddMesh(prim.GetPath(), mesh);
      Body* body = _solver->CreateBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, mesh, prim.GetPath());
      Scene::_Prim* sPrim = _scene.GetPrim(prim.GetPath());
      sPrim->bits = pxr::HdChangeTracker::AllDirty;

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

  */
  Force* gravity = new GravitationalForce(pxr::GfVec3f(0.f, -9.81f, 0.f));
  _solver->AddElement(gravity, NULL, _solverId.AppendChild(pxr::TfToken("Gravity")));

  



  for (auto& sphere : spheres) {
    Collision* collision = new SphereCollision(sphere.second, sphere.first, restitution, friction);
    _solver->AddElement(collision, sphere.second, sphere.first);
  }

  Collision* collision = new PlaneCollision(_ground, _groundId, restitution, friction);
  _solver->AddElement(collision, _ground, _groundId);

  _lastTime = _solver->GetStartFrame();
  _solver->GetParticles()->SetAllState(Particles::ACTIVE);
  _solver->Update(stage, _lastTime);

  Particles* particles = _solver->GetParticles();

  pxr::GfRange3f range;
  for(auto& pos: particles->_position) {
    range = range.UnionWith(pos);
  }
  std::vector<int> used;
  for(size_t p = 0; p < _solver->GetNumParticles(); ++p)
    if (particles->_position[p][1] < range.GetMidpoint()[1]) {
      used.push_back(p);
      particles->_color[p] = pxr::GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    }
  //collision->SetMask(_solver->GetNumParticles(), used);
  

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