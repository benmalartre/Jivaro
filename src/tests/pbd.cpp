#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/scene.h"

#include "../tests/utils.h"
#include "../tests/pbd.h"

JVR_NAMESPACE_OPEN_SCOPE


void _InitControls(pxr::UsdStageRefPtr& stage)
{
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  pxr::UsdPrim controlPrim = stage->DefinePrim(rootPrim.GetPath().AppendChild(pxr::TfToken("Controls")));
  controlPrim.CreateAttribute(pxr::TfToken("Density"), pxr::SdfValueTypeNames->Int).Set(10000);
  controlPrim.CreateAttribute(pxr::TfToken("Radius"), pxr::SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(pxr::TfToken("Length"), pxr::SdfValueTypeNames->Float).Set(4.f);
  controlPrim.CreateAttribute(pxr::TfToken("Scale"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Amplitude"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  controlPrim.CreateAttribute(pxr::TfToken("Frequency"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Width"), pxr::SdfValueTypeNames->Float).Set(0.1f);
}

void TestPBD::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _InitControls(stage);
  _solver = new Solver();
  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("Solver"));

  pxr::GfQuatf rotate(45.f*DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotate.Normalize();
  pxr::GfMatrix4f matrix = 
    pxr::GfMatrix4f(1.f).SetScale(pxr::GfVec3f(5.f));
  float size = .25f;

  _GenerateCollidePlane(stage, rootId);
  
  
  for(size_t x = 0; x < 6; ++x) {
    std::string name = "cloth" + std::to_string(x);
    pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken(name));
    _GenerateClothMesh(stage, clothPath, size,
      matrix * pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(x*6.f, 5.f, 0.f)));
    
  }

  std::vector<pxr::UsdGeomSphere> spheres;
  _Sources sources;
  float mass = 0.1f;
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene->AddMesh(prim.GetPath(), mesh);
      
      Body* body = _solver->AddBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass);
      //mass *= 2;
      _solver->AddConstraints(body);
      _bodyMap[prim.GetPath()] = body;
      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Points* points = new Points(usdPoints, xform);
      _scene->AddPoints(prim.GetPath(), points);

      _solver->AddBody((Geometry*)points, pxr::GfMatrix4f(xform), RANDOM_LO_HI(0.5f, 5.f));

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    }
  }
  _solver->AddForce(new GravitationalForce());
  _solver->WeightBoundaries();
  _solver->LockPoints();
  
  //_solver->AddForce(new DampingForce());
  
  pxr::GfVec3f pos;
  double radius;
  float restitution = 0.25;
  float friction = 0.5f;
  for (auto& sphere: spheres) {
    sphere.GetRadiusAttr().Get(&radius);
    pxr::GfMatrix4f m(sphere.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
    _solver->AddCollision(new SphereCollision(restitution, friction, m, (float)radius));
  } 

  _solver->AddCollision(new PlaneCollision(1.f, 1.f, 
    pxr::GfVec3f(0.f, 1.f, 0.f), pxr::GfVec3f(0.f, -0.1f, 0.f)));


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
          radii[elem + offsetIdx] = 0.2f;
          colors[elem + offsetIdx] = hitColor;
        }
      }
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Constraints")) {
      
    } else {
      pxr::UsdPrim usdPrim = stage->GetPrimAtPath(execPrim.first);
      if (usdPrim.IsValid() && usdPrim.IsA<pxr::UsdGeomMesh>()) {
        std::cout << "we found a mesh check for associated body" << std::endl;
        const auto& bodyIt = _bodyMap.find(usdPrim.GetPath());
        if (bodyIt != _bodyMap.end()) {
          Body* body = bodyIt->second;
          std::cout << "body found for " << bodyIt->first << ": " << body << std::endl;
          Mesh* mesh = (Mesh*)execPrim.second.geom;
          mesh->SetPositions(&_solver->GetParticles()->position[body->offset], mesh->GetNumPoints());
        } else {
          std::cout << "no body found for " << bodyIt->first << std::endl;
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

}

JVR_NAMESPACE_CLOSE_SCOPE