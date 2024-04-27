#include <iostream>

#include <pxr/base/work/loops.h>

#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../geometry/location.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../geometry/curve.h"
#include "../geometry/implicit.h"
#include "../geometry/scene.h"
#include "../pbd/tokens.h"
#include "../pbd/constraint.h"
#include "../pbd/force.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../app/time.h"


JVR_NAMESPACE_OPEN_SCOPE

static const size_t NUM_TIMES = 7;
static const char* TIME_NAMES[NUM_TIMES] = {
  "find contacts",
  "update contacts",
  "integrate particles",
  "solve constraints",
  "solve contacts",
  "update particles",
  "solve velocities"
};

Solver::Solver(Scene* scene, const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world)
  : Xform(xform, world)
  , _scene(scene)
  , _subSteps(5)
  , _sleepThreshold(0.1f)
  , _paused(true)
  , _startFrame(1.f)
  , _solverId(xform.GetPrim().GetPath())
  , _gravity(nullptr)
  , _damp(nullptr)
{
  _frameTime = 1.f / Time::Get()->GetFPS();
  _stepTime = _frameTime / static_cast<float>(_subSteps);

  _timer = new Timer();
  _timer->Init("xpbd solver", NUM_TIMES, &TIME_NAMES[0]);
  _points = new Points();


  _pointsId = _solverId.AppendChild(pxr::TfToken("Particles"));
  AddElement(&_particles, _points, _pointsId);
  _scene->AddGeometry(_pointsId, _points);
}

Solver::~Solver()
{
  for (auto& body : _bodies)delete body;
  for (auto& force : _force)delete force;
  for (auto& constraint : _constraints)delete constraint;
  delete _points;
  delete _timer;
}

void Solver::AddElement(Element* element, Geometry* geom, const pxr::SdfPath& path)
{
  _elements[element] = std::make_pair(path, geom);
  switch(element->GetType()) {
    case Element::COLLISION:
      AddCollision((Collision*)element);
      break;

    case Element::CONTACT:
      AddContact((Constraint*)element);
      break;

    case Element::CONSTRAINT:
      AddConstraint((Constraint*)element);
      break;

    case Element::FORCE:
      if(path.GetNameToken() == PBDTokens->gravity)_gravity = (Force*)element;
      else if(path.GetNameToken() == PBDTokens->damp)_damp = (Force*)element;
      AddForce((Force*)element);
      break;

    case Element::BODY:
      AddBody((Body*)element);
      break;
  }

  //else TF_WARN("There is already an element named %s", path.GetText());
}

void Solver::RemoveElement(Element* element)
{
  _elements.erase(element);
}

pxr::SdfPath Solver::GetElementPath(Element* element)
{
  return _elements[element].first;
}

Geometry* Solver::GetElementGeometry(Element* element)
{
  return _elements[element].second;
}

Element* Solver::GetElement(const pxr::SdfPath& path)
{
  for(auto& elem: _elements) 
    if(elem.second.first == path)return elem.first;
  return NULL;
}

Body* Solver::GetBody(size_t index)
{
  if (index < _bodies.size())return _bodies[index];
  return nullptr;
}

Body* Solver::GetBody(Geometry* geom)
{
  for (auto& body : _bodies) {
    if (body->GetGeometry() == geom)return body;
  }
  return nullptr;
}

size_t Solver::GetBodyIndex(Geometry* geom)
{
  for (size_t index = 0; index < _bodies.size(); ++index) {
    if (_bodies[index]->GetGeometry() == geom)return index;
  }
  return Solver::INVALID_INDEX;
}


Body* Solver::CreateBody(Geometry* geom, const pxr::GfMatrix4f& matrix, float mass, float radius, float damping)
{
  size_t base = _particles.GetNumParticles();
  pxr::GfVec3f wirecolor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  Body* body = new Body(geom, base, geom->GetNumPoints(), wirecolor, mass, radius, damping);
  _particles.AddBody(body, matrix);
  return body;
}

void Solver::AddBody(Body* body)
{
  _bodies.push_back(body);
}

void Solver::RemoveBody(Geometry* geom)
{
  size_t index = GetBodyIndex(geom);
  if (index == Solver::INVALID_INDEX) return;

  Body* body = GetBody(index);
  _particles.RemoveBody(body);

  _bodies.erase(_bodies.begin() + index);
  delete body;
}


void Solver::AddCollision(Collision* collision)
{
  _collisions.push_back(collision);
/*
  float radius = 0.2f;
  Voxels voxels;
  voxels.Init(collider, radius);
  voxels.Trace(0);
  voxels.Trace(1);
  voxels.Trace(2);
  voxels.Build();
  _SetupVoxels(stage, &voxels, radius);

  _TestHashGrid(&voxels, radius);

  BenchmarkParallelEvaluation(this);

  size_t numRays = 2048;
  std::vector<pxr::GfRay> rays(numRays);
  for (size_t r = 0; r < numRays; ++r) {
    rays[r] = pxr::GfRay(pxr::GfVec3f(0.f),
      pxr::GfVec3f(RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1), RANDOM_LO_HI(-1, 1)).GetNormalized());
  }

  std::vector<pxr::GfVec3f> result;


  _colliders = colliders;

  std::cout << "### build bvh (morton) : " << std::endl;
  for(auto& collider: _colliders) {
    std::cout << " collider : " << collider << std::endl;
  }
  uint64_t T = CurrentTime();
  BVH bvh;
  bvh.Init(_colliders);
  std::cout << "   build boundary volume hierarchy : " << ((CurrentTime() - T) * 1e-9) << std::endl;

  T = CurrentTime();
  for (auto& ray : rays) {
    double minDistance = DBL_MAX;
    Location hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
      result.push_back(hit.GetPosition(colliders[hit.GetGeometryIndex()]));
    }
  }
  std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
  _SetupBVHInstancer(stage, &bvh);


  _SetupResults(stage, result);
*/
}



void Solver::CreateConstraints(Body* body, short type, float stiffness, float damping)
{
  Geometry* geom = body->GetGeometry();

  switch(type) {
    case Constraint::STRETCH:
      CreateStretchConstraints(body, _constraints, stiffness, damping);
      break;
    case Constraint::BEND:
      CreateBendConstraints(body, _constraints, stiffness, damping);
      break;
    case Constraint::DIHEDRAL:
      if(geom->GetType() == Geometry::MESH) {
        CreateDihedralConstraints(body, _constraints, stiffness, damping);
      } else {
        //TF_WARN("Dihedral constraints can only be applied on meshes !");
      }
      break;
  }

}

void Solver::GetConstraintsByType(short type, std::vector<Constraint*>& results)
{
  for(auto& constraint: _constraints)
    if(constraint->GetTypeId() == type)
      results.push_back(constraint);
}

void Solver::LockPoints()
{
  size_t particleIdx = 0;
  const pxr::GfVec3f* positions = _particles.GetPositionCPtr();
  for (auto& body : _bodies) {
    size_t numPoints = body->GetNumPoints();
    for(size_t point = 0; point < 10; ++point) {
      _particles.SetMass(point + body->GetOffset(), 0.f);
      _particles.SetInvMass(point + body->GetOffset(), 0.f);
    }
  }
}

void Solver::WeightBoundaries()
{
  for(const auto& body: _bodies) {
    size_t offset = body->GetOffset();
    size_t numPoints = body->GetNumPoints();
  
    if(body->GetGeometry()->GetType() == Geometry::MESH) {
      Mesh* mesh = (Mesh*)body->GetGeometry();
      HalfEdgeGraph* graph = mesh->GetEdgesGraph();
      const pxr::VtArray<bool>& boundaries = graph->GetBoundaries();
      for(size_t p = 0; p < boundaries.size(); ++p){
        if(boundaries[p]) {
          size_t index = p + offset;
          _particles.SetMass(index, _particles.GetMass(index) * 0.5f);
          _particles.SetInvMass(index,1.f / _particles.GetMass(index));
        }
      }
    }
  } 
}

void Solver::_ClearContacts()
{
  for (auto& contact : _contacts)delete contact;
  _contacts.clear();
}

void Solver::_FindContacts()
{
  _ClearContacts();

  for (auto& collision : _collisions) {
    collision->UpdateContacts(&_particles);
    collision->FindContacts(&_particles, _bodies, _contacts, _frameTime);
  }
}

void Solver::_UpdateContacts()
{
  for (auto& collision : _collisions) {
    collision->UpdateContacts(&_particles);
  }
}

void Solver::_IntegrateParticles(size_t begin, size_t end)
{
  const pxr::GfVec3f* velocity = _particles.GetVelocityCPtr();

  // compute predicted position
  pxr::GfVec3f* predicted = _particles.GetPredictedPtr();
  pxr::GfVec3f* position = _particles.GetPositionPtr();

  // apply external forces
  for (const Force* force : _force)
    force->Apply(begin, end, &_particles, _stepTime);

  for (size_t index = begin; index < end; ++index) {
    position[index] = predicted[index];
    predicted[index] = position[index] + velocity[index] * _stepTime;
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end)
{
  const int* body = _particles.GetBodyCPtr();
  const pxr::GfVec3f* predicted = _particles.GetPredictedCPtr();
  pxr::GfVec3f* position = _particles.GetPositionPtr();
  pxr::GfVec3f* velocity = _particles.GetVelocityPtr();
  pxr::GfVec3f* previous = _particles.GetPreviousPtr()  ;
  short* state = _particles.GetStatePtr();

  float invDt = 1.f / _stepTime;

  for(size_t index = begin; index < end; ++index) {
    if (state[index] != Particles::ACTIVE)continue;
    // update velocity
    velocity[index] = (predicted[index] - position[index]) * invDt;
    /*
    if (velocity[index].GetLength() < 0.0000001f) {
      state[index] = Particles::IDLE;
      velocity[index] = pxr::GfVec3f(0.f);
    }
    */

    // update position
    previous[index] = position[index];
    position[index] = predicted[index];
  }
}

void Solver::_SolveConstraints(std::vector<Constraint*>& constraints)
{

  // solve constraints
  pxr::WorkParallelForEach(constraints.begin(), constraints.end(),
    [&](Constraint* constraint) {constraint->Solve(&_particles, _stepTime); });
  
  // apply constraint serially
  for (auto& constraint : constraints)constraint->Apply(&_particles);

}

// this can not work as multiple thread can writ eto same velocities
void Solver::_SolveVelocities()
{
  for (auto& collision : _collisions) {
    if (!collision->GetNumContacts()) continue;
    pxr::WorkParallelForN(
      collision->GetNumContacts(),
      std::bind(&Collision::SolveVelocities, collision,
        std::placeholders::_1, std::placeholders::_2, &_particles, _stepTime));
  }
}

void Solver::_StepOne()
{
  const size_t numParticles = _particles.GetNumParticles();
  const size_t numContacts = _contacts.size();

  _timer->Start(1);
  _UpdateContacts();

  _timer->Next();
  // integrate particles
  pxr::WorkParallelForN(
    numParticles,
    std::bind(&Solver::_IntegrateParticles, this,
      std::placeholders::_1, std::placeholders::_2));
  
  _timer->Next();
  // solve and apply constraint
  _SolveConstraints(_constraints);

  _timer->Next();
  // solve and apply contacts
  _SolveConstraints(_contacts);

  _timer->Next();
  // update particles
  pxr::WorkParallelForN(
    numParticles,
    std::bind(&Solver::_UpdateParticles, this,
      std::placeholders::_1, std::placeholders::_2));

  _timer->Next();
  // solve velocities
  _SolveVelocities();
  
  _timer->Stop();

  _t += _stepTime;

}

void Solver::_GetContactPositions(pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  for (size_t i = 0; i < _collisions.size(); ++i) {
    std::vector<Contact>& contacts = _collisions[i]->GetContacts();
    const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    float r = 0.2f;
    for (auto& contact : contacts) {
      positions.push_back(contact.GetCoordinates());
      radius.push_back(r);
      colors.push_back(color);
    }
  }
}

void Solver::Update(pxr::UsdStageRefPtr& stage, float time)
{
  UpdateCollisions(stage, time);
  UpdateParameters(stage, time);
 
  size_t numParticles = _particles.GetNumParticles();
  if (pxr::GfIsClose(time, _startFrame, 0.001f)) {
    Reset();
    _points->SetPositions(_particles.GetPositionCPtr(), numParticles);
    _points->SetRadii(_particles.GetRadiusCPtr(), numParticles);
    _points->SetColors(_particles.GetColorCPtr(), numParticles);

    Scene::_Prim* prim = _scene->GetPrim(_pointsId);
    prim->bits = pxr::HdChangeTracker::AllDirty;
  } else {
    Step();

    _points->SetPositions(_particles.GetPositionCPtr(), numParticles);
    _points->SetColors(_particles.GetColorCPtr(), numParticles);
    
    Scene::_Prim* prim = _scene->GetPrim(_pointsId);
    prim->bits = 
      pxr::HdChangeTracker::Clean | pxr::HdChangeTracker::DirtyPoints |
      pxr::HdChangeTracker::DirtyWidths | pxr::HdChangeTracker::DirtyPrimvar;
  }
}

void Solver::Reset()
{
  // reset
  size_t offset = 0;
  _particles.RemoveAllBodies();

  for (size_t b = 0; b < _bodies.size(); ++b)
    _particles.AddBody(_bodies[b], pxr::GfMatrix4f(_bodies[b]->GetGeometry()->GetMatrix()));


}

void Solver::Step()
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();

  _t = 0.f;

  _timer->Start();
  _FindContacts();
  _timer->Stop();


  for(size_t si = 0; si < _subSteps; ++si)
    _StepOne();
  
  _timer->Update();
  _timer->Log();

  //std::cout << _particles.GetPredicted() << std::endl;

  //UpdateGeometries();
}

void Solver::UpdateCollisions(pxr::UsdStageRefPtr& stage, float time)
{
  for(size_t i = 0; i < _collisions.size(); ++i){
    pxr::SdfPath path = GetElementPath(_collisions[i]);
    pxr::UsdPrim prim = stage->GetPrimAtPath(path);
    _collisions[i]->Update(prim, time);

  }
  /*
  BVH bvh;
  bvh.Init(_colliders);

  {
    double minDistance;
    Location hit;
    if (bvh.Closest(pxr::GfVec3f(0.f), &hit, -1, &minDistance)) {
      std::cout << "CLOSEST HIT :" << std::endl;
      pxr::GfVec3f position;
      hit.GetPosition(&position);
      std::cout << "   pos : " << position << std::endl;
      std::cout << "   tri : " << hit.GetElementIndex() << std::endl;
    }
  }

  {
    pxr::GfRay ray(pxr::GfVec3f(0.f, 5.f, 0.f), pxr::GfVec3f(0.f, -1.f, 0.f));
    double minDistance;
    Location hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(_colliders[0]);
    }
  }
  */
}


void Solver::UpdateGeometries()
{
  /*
  std::map<Geometry*, _Geometry>::iterator it = _geometries.begin();
  for (; it != _geometries.end(); ++it)
  {
    Geometry* geom = it->first;
    size_t numPoints = geom->GetNumPoints();
    pxr::VtArray<pxr::GfVec3f> results(numPoints);
    const auto& positions = _body.GetPositions();
    for (size_t p = 0; p < numPoints; ++p) {
      results[p] = it->second.invMatrix.Transform(positions[it->second.offset + p]);
    }
    geom->SetPositions(&results[0], numPoints);
  }
}
*/
}

void Solver::UpdateParameters(pxr::UsdStageRefPtr& stage, float time)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(_solverId);
  _frameTime = 1.f / static_cast<float>(Time::Get()->GetFPS());
  prim.GetAttribute(pxr::TfToken("SubSteps")).Get(&_subSteps, time);
  _stepTime = _frameTime / static_cast<float>(_subSteps);
  prim.GetAttribute(pxr::TfToken("SleepThreshold")).Get(&_sleepThreshold, time);

  if(_gravity)_gravity->Update(time);
  if (_damp)_damp->Update(time);
}


JVR_NAMESPACE_CLOSE_SCOPE