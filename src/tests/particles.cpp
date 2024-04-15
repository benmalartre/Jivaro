#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/implicit.h"
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
#include "../tests/particles.h"

JVR_NAMESPACE_OPEN_SCOPE


void TestParticles::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  const pxr::SdfPath  rootId = rootPrim.GetPath();
  
    // create solver with particles
  _solverId = rootId.AppendChild(pxr::TfToken("Solver"));
  _solver = _GenerateSolver(_scene, stage, _solverId);
  _scene->AddGeometry(_solverId, _solver);

  // create collide ground
  _groundId = rootId.AppendChild(pxr::TfToken("Ground"));
  _ground = _GenerateCollidePlane(stage, _groundId);
  _ground->SetMatrix(
    pxr::GfMatrix4d().SetTranslate(pxr::GfVec3f(0.f, -0.5f, 0.f)));
  _scene->AddGeometry(_groundId, _ground);
  Collision* collision = new PlaneCollision(_ground, 1.f, 0.f);
  _solver->AddCollision(collision);
  _solver->AddElement(collision, _ground, _groundId);

  float mass = 1.f;
  float radius = 0.3f;
  float damping = 0.1f;

  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene->AddMesh(prim.GetPath(), mesh);
      Body* body = _solver->AddBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, mesh, prim.GetPath());
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Points* points = new Points(usdPoints, xform);
      _scene->AddPoints(prim.GetPath(), points);

      Body* body = _solver->AddBody((Geometry*)points, pxr::GfMatrix4f(xform), mass, radius, damping);
      _solver->AddElement(body, points, prim.GetPath());
    } else if(prim.IsA<pxr::UsdGeomBasisCurves>()) {

    }
  }
  Force* gravity = new GravitationalForce(pxr::GfVec3f(0.f, -9.8f, 0.f));
  _solver->AddForce(gravity);
  _solver->AddElement(gravity, NULL, _solverId.AppendChild(pxr::TfToken("Gravity")));
  //_solver->AddForce(new DampingForce());
  
  pxr::GfVec3f pos;
  double r;
  float restitution = 0.25;
  float friction = 0.5f;
  /*
  for (auto& sphere: spheres) {
    sphere.GetRadiusAttr().Get(&r);
    pxr::GfMatrix4f m(sphere.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
    _solver->AddCollision(new SphereCollision(restitution, friction, m, (float)r));
  } 
  */

  _solver->GetParticles()->SetAllState(Particles::ACTIVE);

}


void TestParticles::UpdateExec(pxr::UsdStageRefPtr& stage, float time)
{
  _scene->Update(stage, time);
  _solver->Update(stage, time);
  
  pxr::UsdGeomXformCache xformCache(time);

  const size_t numParticles = _solver->GetNumParticles();

  Geometry* geom;

  const Solver::_ElementMap& elements = _solver->GetElements();


  for (auto& elemIt = elements.begin(); elemIt != elements.end(); ++elemIt) {
    Element* element = elemIt->first;
    pxr::SdfPath path = elemIt->second.first;
    Geometry* geometry = elemIt->second.second;

    if (path.GetNameToken() == pxr::TfToken("Particles")) {

      Points* points = (Points*)geometry;
      if (!points)continue;
      points->SetPositions(&_solver->GetParticles()->_position[0], numParticles);
      points->SetRadii(&_solver->GetParticles()->_radius[0], numParticles);
      std::cout << "particle 0 radius : " << _solver->GetParticles()->_radius[0] << std::endl;
      points->SetColors(&_solver->GetParticles()->_color[0], numParticles);

      Scene::_Prim* prim = _scene->GetPrim(path);
      prim->bits =
        pxr::HdChangeTracker::Clean |
        pxr::HdChangeTracker::DirtyPoints |
        pxr::HdChangeTracker::DirtyWidths |
        pxr::HdChangeTracker::DirtyPrimvar;
    }


    /*
    if (elemIt.second.first.GetNameToken() == pxr::TfToken("Particles")) {
      Particles* particles = (Particles*)elemIt.first;

      Points* points = (Points*)elemIt.first;

      points->SetPositions(&_solver->GetParticles()->_position[0], numParticles);
      points->SetColors(&_solver->GetParticles()->_color[0], numParticles);
    } else if (elemIt.second.first.GetNameToken() == pxr::TfToken("Collisions")) {
      
      const pxr::VtArray<Constraint*>& contacts = _solver->GetContacts();
      
      if (!contacts.size())continue;

      Points* points = (Points*)elemIt.first;

      pxr::VtArray<pxr::GfVec3f>& positions = points->GetPositions();
      pxr::VtArray<pxr::GfVec3f>& colors = points->GetColors();
      pxr::VtArray<float>& radii = points->GetRadius();

      memset(&radii[0], 0.f, numParticles * sizeof(float));

      for (auto& contact : contacts) {
        CollisionConstraint* constraint = (CollisionConstraint*)contact;
        Collision* collision = constraint->GetCollision();
        size_t offsetIdx = constraint->GetBody(0)->GetOffset();
        
        const pxr::VtArray<int>& elements = contact->GetElements();
        for(int elem: elements) {
          positions[elem + offsetIdx] = collision->GetContactPosition(elem);
          radii[elem + offsetIdx] = 0.2f;
          colors[elem + offsetIdx] = hitColor;
        }
      }
      
    } else if (elemIt.second.first.GetNameToken() == pxr::TfToken("Constraints")) {
      
    } else {
      
      pxr::UsdPrim usdPrim = stage->GetPrimAtPath(elemIt.second.first);
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
    */
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
      _scene->Remove(meshPath);
    }
  }
  delete _solver;
  delete _ground;
}

JVR_NAMESPACE_CLOSE_SCOPE