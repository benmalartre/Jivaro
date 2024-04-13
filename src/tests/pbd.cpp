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
  const pxr::SdfPath groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, groundId);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  _scene->AddGeometry(groundId, _ground);
  
  // create solver with attributes
  const pxr::SdfPath solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(stage, solverId);
  _scene->AddGeometry(solverId, _solver);

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

  std::vector<Sphere*> spheres;
  
  for (size_t x = 0; x < 3; ++x) {
    std::cout << "collide sphere" << std::endl;
    std::string name = "sphere_collide_" + std::to_string(x);
    pxr::SdfPath collidePath = rootId.AppendChild(pxr::TfToken(name));
    spheres.push_back(
      _GenerateCollideSphere(stage, collidePath, RANDOM_0_1 + 1.f, 
      pxr::GfMatrix4d(1.f).SetTranslate(pxr::GfVec3f(x * 6.f, 0.f, 0.f))));

    //sphere.GetRadiusAttr().Get(&radius);
    //pxr::GfMatrix4f m(sphere.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
  }
  _Sources sources;
  float mass = 0.1f;
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene->AddMesh(prim.GetPath(), mesh);
      std::cout << "add mesh to solver" << std::endl;
      Body* body = _solver->AddBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass);
      std::cout << "mesh added" << std::endl;
      //mass *= 2;
      std::cout << "add constraints to solver" << std::endl;
      _solver->AddConstraints(body);
      std::cout << "constyraint added" << std::endl;
      _bodyMap[prim.GetPath()] = body;
      
      //sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Points* points = new Points(usdPoints, xform);
      _scene->AddPoints(prim.GetPath(), points);
      std::cout << "add points to solver" << std::endl;
      _solver->AddBody((Geometry*)points, pxr::GfMatrix4f(xform), RANDOM_LO_HI(0.5f, 5.f));

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
      std::cout << "points added to solver" << std::endl;
    }
  }
  _solver->AddForce(new GravitationalForce(pxr::GfVec3f(0.f, -9.8f,0.f)));
  _solver->WeightBoundaries();
  _solver->LockPoints();
  
  //_solver->AddForce(new DampingForce());
  
  pxr::GfVec3f pos;
  double radius;
  float restitution = 0.25;
  float friction = 0.5f;
  for (auto& sphere: spheres) {
    _solver->AddCollision(new SphereCollision(sphere, restitution, friction));
  } 


  _solver->AddCollision(new PlaneCollision(_ground, 1.f, 1.f));


  pxr::SdfPath pointsPath(rootId.AppendChild(pxr::TfToken("Particles")));
  _sourcesMap[pointsPath] = sources;
  Points* points = _scene->AddPoints(pointsPath);
  Particles* particles = _solver->GetParticles();
  const size_t numParticles = _solver->GetNumParticles();
  points->SetPositions(&particles->position[0], numParticles);
  points->SetRadii(&particles->radius[0], numParticles);
  points->SetColors(&particles->color[0], numParticles);

  pxr::SdfPath collisionsPath(rootId.AppendChild(pxr::TfToken("Collisions")));
  _sourcesMap[collisionsPath] = sources;
  Points* collisions = _scene->AddPoints(collisionsPath);
  collisions->SetPositions(&particles->position[0], numParticles);
  collisions->SetRadii(&particles->radius[0], numParticles);
  collisions->SetColors(&particles->color[0], numParticles);

}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, double time, double startTime)
{
  _scene->Update(stage, time);
  _solver->UpdateParameters(stage->GetPrimAtPath(_solverId), time);

  if (pxr::GfIsClose(time, startTime, 0.01))
    _solver->Reset();
  else
    _solver->Step(false);
  
  pxr::UsdGeomXformCache xformCache(time);

  const size_t numParticles = _solver->GetNumParticles();
  const pxr::GfVec3f hitColor(1.f, 0.2f, 0.3f);
  Geometry* geom;
 
  for (auto& execPrim : _scene->GetPrims()) {

    if (execPrim.first.GetNameToken() == pxr::TfToken("Particles")) {
      Points* points = (Points*)_scene->GetGeometry(execPrim.first);

      points->SetPositions(&_solver->GetParticles()->position[0], numParticles);
      points->SetColors(&_solver->GetParticles()->color[0], numParticles);
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Collisions")) {
      /*
      const pxr::VtArray<Constraint*>& contacts = _solver->GetContacts();
      if (!contacts.size())continue;

      Points* points = (Points*)_scene->GetGeometry(execPrim.first);

      pxr::VtArray<pxr::GfVec3f>& positions = points->GetPositions();
      pxr::VtArray<pxr::GfVec3f>& colors = points->GetColors();
      pxr::VtArray<float>& radii = points->GetRadius();

      memset(&radii[0], 0.f, numParticles * sizeof(float));

      for (auto& contact : contacts) {
        const size_t offsetIdx = contact->GetBody(0)->offset;
        
        const pxr::VtArray<int>& elements = contact->GetElements();
        for(int elem: elements) {
          positions[elem + offsetIdx] = _solver->GetParticles()->position[elem + offsetIdx];
          radii[elem + offsetIdx] = 0.02f;
          colors[elem + offsetIdx] = hitColor;
        }
      }
      */
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Constraints")) {
      
    } else {
      pxr::UsdPrim usdPrim = stage->GetPrimAtPath(execPrim.first);
      if (usdPrim.IsValid() && usdPrim.IsA<pxr::UsdGeomMesh>()) {
        const auto& bodyIt = _bodyMap.find(usdPrim.GetPath());
        if (bodyIt != _bodyMap.end()) {
          Body* body = bodyIt->second;
          Mesh* mesh = (Mesh*)execPrim.second.geom;
          mesh->SetPositions(&_solver->GetParticles()->position[body->offset], mesh->GetNumPoints());
        } else {
        }
      }
    }

    execPrim.second.bits =
      pxr::HdChangeTracker::Clean |
      pxr::HdChangeTracker::DirtyPoints |
      pxr::HdChangeTracker::DirtyWidths |
      pxr::HdChangeTracker::DirtyPrimvar;
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