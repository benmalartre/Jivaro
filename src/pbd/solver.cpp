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
  "integrate particles",
  "solve constraints",
  "update contacts",
  "solve contacts",
  "update particles",
  "solve velocities"
};

Solver::Solver(Scene* scene, const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world)
  : Xform(xform, world)
  , _scene(scene)
  , _subSteps(5)
  , _sleepThreshold(0.001f)
  , _paused(true)
  , _startTime(1.f)
  , _solverId(xform.GetPrim().GetPath())
  , _gravity(nullptr)
  , _damp(nullptr)
{
  _frameTime = 1.f / Time::Get()->GetFPS();
  _stepTime = _frameTime / static_cast<float>(_subSteps);

  _timer = new Timer();
  _timer->Init("xpbd solver", NUM_TIMES, &TIME_NAMES[0]);

  _pointsId = _solverId.AppendChild(pxr::TfToken("Particles"));
  _points = (Points*)_scene->AddGeometry(_pointsId, Geometry::POINT, pxr::GfMatrix4d(1.0));

  pxr::UsdAttribute gravityAttr = xform.GetPrim().GetAttribute(PBDTokens->gravity);
   _gravity = new GravityForce(gravityAttr);
  AddElement(_gravity, NULL, _solverId.AppendChild(PBDTokens->gravity));

  pxr::UsdAttribute dampAttr = xform.GetPrim().GetAttribute(PBDTokens->damp);
  _damp = new DampForce(dampAttr);
  AddElement(_damp, NULL, _solverId.AppendChild(PBDTokens->damp));
}

Solver::~Solver()
{
  for (auto& constraint : _constraints)delete constraint;
  for (auto& body : _bodies)delete body;
  for (auto& force : _force)delete force;
  _scene->RemoveGeometry(_pointsId);
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


Body* Solver::CreateBody(Geometry* geom, const pxr::GfMatrix4d& matrix, 
  float mass, float radius, float damping)
{
  size_t base = _particles.GetNumParticles();
  pxr::GfVec3f wirecolor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  Body* body = new Body(geom, base, geom->GetNumPoints(), wirecolor, mass, radius, damping);
  _particles.AddBody(body, matrix);

  UpdatePoints();
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

void Solver::SetBodyVelocity(Body* body, const pxr::GfVec3f& velocity)
{
  const size_t offset = body->GetOffset();
  const size_t num = body->GetNumPoints();
  body->SetVelocity(velocity);
  for(size_t index = 0; index < num; ++index) {
    _particles.velocity[index + offset] = velocity * _particles.invMass[index + offset];
  }
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

void Solver::AddConstraint(Constraint* constraint) 
{ 
  _constraints.push_back(constraint); 
  const pxr::VtArray<int>& elements = constraint->GetElements();
  for(const auto& elem: elements)_particles.m[elem]++;
};

void Solver::GetConstraintsByType(short type, std::vector<Constraint*>& results)
{
  for(auto& constraint: _constraints)
    if(constraint->GetTypeId() == type)
      results.push_back(constraint);
}

void Solver::LockPoints(Body* body, pxr::VtArray<int>& elements)
{
  for(size_t i = 0; i < elements.size(); ++i) {
    _particles.mass[elements[i] + body->GetOffset()] = 0.f;
    _particles.invMass[elements[i] + body->GetOffset()] = 0.f;
  }
}

void Solver::UpdatePoints()
{
  size_t numParticles = _particles.GetNumParticles();
  _points->SetPositions(&_particles.position[0], numParticles);
  _points->SetWidths(&_particles.radius[0], numParticles);
  _points->SetColors(&_particles.color[0], numParticles);

  _scene->MarkPrimDirty(_pointsId, pxr::HdChangeTracker::AllDirty);
}

void Solver::WeightBoundaries()
{
  for(const auto& _body: _bodies) {
    size_t offset = _body->GetOffset();
    size_t numPoints = _body->GetNumPoints();
  
    if(_body->GetGeometry()->GetType() == Geometry::MESH) {
      Mesh* mesh = (Mesh*)_body->GetGeometry();
      HalfEdgeGraph* graph = mesh->GetEdgesGraph();
      const pxr::VtArray<bool>& boundaries = graph->GetBoundaries();
      for(size_t p = 0; p < boundaries.size(); ++p){
        if(boundaries[p]) {
          size_t index = p + offset;
          
          if(_particles.mass[index] > 0.f) {
            _particles.mass[index] *= 1.2f;
            _particles.invMass[index] = 1.f / _particles.mass[index];
          }
        }
      }
    }
  } 
}

void Solver::_PrepareContacts()
{
  //_timer->Start();
  memset(&_particles.n[0], 0, _particles.GetNumParticles() * sizeof(int));

  for (auto& contact : _contacts)delete contact;
  _contacts.clear();

  for (auto& collision : _collisions)
    collision->FindContacts(&_particles, _bodies, _contacts, _frameTime);

  for (auto& contact : _contacts)
    for (auto& elem : contact->GetElements())
      _particles.n[elem]++;

  //_timer->Stop();
}
  
void Solver::_UpdateContacts()
{
  for (auto& collision : _collisions)
    collision->UpdateContacts(&_particles);
}

void Solver::_IntegrateParticles(size_t begin, size_t end)
{
  pxr::GfVec3f* velocity = &_particles.velocity[0];

  // compute predicted position
  pxr::GfVec3f* predicted = &_particles.predicted[0];
  pxr::GfVec3f* position = &_particles.position[0];

  // apply external forces
  for (const Force* force : _force)
    force->Apply(begin, end, &_particles, _stepTime);

  for (size_t index = begin; index < end; ++index) {
    //if(_particles.state[index] != Particles::ACTIVE)continue;

    position[index] = predicted[index];
    predicted[index] = position[index] + velocity[index] * _stepTime;
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end)
{
  const int* body = &_particles.body[0];
  const pxr::GfVec3f* predicted = &_particles.predicted[0];
  pxr::GfVec3f* position = &_particles.position[0];
  pxr::GfVec3f* velocity = &_particles.velocity[0];
  pxr::GfVec3f* previous = &_particles.previous[0];
  short* state = &_particles.state[0];

  float invDt = 1.f / _stepTime;

  for(size_t index = begin; index < end; ++index) {
    //if (state[index] != Particles::ACTIVE)continue;
    // update velocity
    velocity[index] = (predicted[index] - position[index]) * invDt;

    if (velocity[index].GetLength() < _sleepThreshold) {
      //state[index] = Particles::IDLE;
      velocity[index] = pxr::GfVec3f(0.f);
    }

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


void Solver::Update(pxr::UsdStageRefPtr& stage, float time)
{
  UpdateCollisions(stage, time);
  UpdateParameters(stage, time);
 
  size_t numParticles = _particles.GetNumParticles();
  if (pxr::GfIsClose(time, _startTime, 0.001f)) {
    Reset();

    pxr::VtArray<float> widths(numParticles);
    for(size_t r=0; r < numParticles; ++r)widths[r] = _particles.radius[r] * 2.f;
    
    _points->SetPositions(&_particles.position[0], numParticles);
    _points->SetWidths(&widths[0], numParticles);
    _points->SetColors(&_particles.color[0], numParticles);

    _scene->MarkPrimDirty(_pointsId, pxr::HdChangeTracker::AllDirty);
  } else {
    Step();

    _points->SetPositions(&_particles.position[0], numParticles);
    _points->SetColors(&_particles.color[0], numParticles);
    _scene->MarkPrimDirty(_pointsId, pxr::HdChangeTracker::DirtyPoints|pxr::HdChangeTracker::DirtyPrimvar);
  }

}

void Solver::Reset()
{
  // reset
  size_t offset = 0;
  _particles.RemoveAllBodies();

  for (size_t b = 0; b < _bodies.size(); ++b) {
    const pxr::GfMatrix4d& matrix = _bodies[b]->GetGeometry()->GetMatrix();
    _particles.AddBody(_bodies[b], matrix);
  }

  WeightBoundaries();
  //LockPoints();

  _particles.SetAllState(Particles::ACTIVE);
}



void Solver::Step()
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();

  _PrepareContacts();

  for(size_t si = 0; si < _subSteps; ++si) {
    
    //_timer->Start1();
    // integrate particles
    pxr::WorkParallelForN(
      numParticles,
      std::bind(&Solver::_IntegrateParticles, this,
        std::placeholders::_1, std::placeholders::_2));
    //_timer->Next();
    // solve and apply constraint
    _SolveConstraints(_constraints);

    //_timer->Next();
    //_timer->Next();
    _UpdateContacts();
    // solve and apply contacts
    _SolveConstraints(_contacts);
    //_timer->Next();
    // update particles
    pxr::WorkParallelForN(
      numParticles,
      std::bind(&Solver::_UpdateParticles, this,
        std::placeholders::_1, std::placeholders::_2));
    //_timer->Next();
    // solve velocities
    //_timer->Stop();

  }
  
  //_timer->Update();
  //_timer->Log();

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
  prim.GetAttribute(PBDTokens->substeps).Get(&_subSteps, time);
  _stepTime = _frameTime / static_cast<float>(_subSteps);
  prim.GetAttribute(PBDTokens->sleepThreshold).Get(&_sleepThreshold, time);

  if(_gravity)_gravity->Update(time);
  if (_damp)_damp->Update(time);
}


JVR_NAMESPACE_CLOSE_SCOPE