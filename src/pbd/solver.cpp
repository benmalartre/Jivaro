#include <iostream>

#include <pxr/base/work/loops.h>

#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../geometry/curve.h"
#include "../geometry/implicit.h"
#include "../app/application.h"
#include "../pbd/constraint.h"
#include "../pbd/force.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"
#include "../app/time.h"
#include "../app/scene.h"

JVR_NAMESPACE_OPEN_SCOPE

// Helpers for benchmark time inside the solver
class _Timer {
public:
  void Init(size_t n, const char** names);
  void Start(size_t index = 0);
  void Next();
  void Stop();
  void Update();
  void Log();

protected:
  struct _Ts {
    void Start() { t = CurrentTime(); }
    void End() { accum += CurrentTime() - t; num++; };
    void Reset() { accum = 0; num = 0; };
    double Average() { return num ? ((double)accum * 1e-9) / (double)num : 0; }
    double Elapsed() { return (double)accum * 1e-9; }

    uint64_t t;
    uint64_t accum;
    size_t   num;
  };

private:
  bool                      _rec;
  size_t                    _n;
  size_t                    _c;

  std::vector<std::string>  _names;
  std::vector<_Ts>           _timers;
  std::vector<double>       _accums;
  std::vector<double>       _avgs;
};


void _Timer::Init(size_t n, const char** names)
{
  _n = n;
  _c = 0;
  _timers.resize(_n, { 0,0,0 });
  _accums.resize(_n, 0.0);
  _avgs.resize(_n, 0.0);
  _names.resize(_n);
  for (size_t t = 0; t < _n; ++t)
    _names[t] = names[t];
}

void _Timer::Start(size_t index)
{
  _c = index;
  _timers[_c].Start();
  _rec = true;
}

void _Timer::Next()
{
  if (_rec)_timers[_c++].End();
  if (_c >= _n)_c = 0;
  _timers[_c].Start();
}

void _Timer::Stop()
{
  if (_rec = true) _timers[_c].End();
  _rec = false;
}

void _Timer::Update() {
  for (size_t t = 0; t < _n; ++t) {
    _accums[t] += _timers[t].Elapsed();
    _avgs[t] = (_avgs[t] + _timers[t].Average()) / 2.0;
    _timers[t].Reset();
  }
}

void _Timer::Log() {
  return;
  for (size_t t = 0; t < _n; ++t) {
    std::cout << "## " << _names[t] << ": ";
    std::cout << "  - accum : " << _accums[t];
    std::cout << "  - avg : " << _avgs[t] << std::endl;
  }
}

static const size_t NUM_TIMES = 5;
static const char* TIME_NAMES[NUM_TIMES] = {
  "find contacts",
  "integrate particles",
  "solve constraints",
  "update particles",
  "solve velocities"
};

Solver::Solver(Scene* scene, const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world)
  : Xform(xform, world)
  , _scene(scene)
  , _subSteps(5)
  , _sleepThreshold(0.1f)
  , _paused(true)
  , _serial(false)
  , _startFrame(1.f)
  , _solverId(xform.GetPrim().GetPath())
{
  _frameTime = 1.f / GetApplication()->GetTime().GetFPS();
  _stepTime = _frameTime / static_cast<float>(_subSteps);

  //for (size_t i = 0; i < NUM_TIMES; ++i) T_timers[i].Reset();
  _timer = new _Timer();
  _timer->Init(NUM_TIMES, &TIME_NAMES[0]);
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
}

void Solver::Reset()
{
  // reset
  for (size_t p = 0; p < GetNumParticles(); ++p) {
    _particles._position[p] = _particles._rest[p];
    _particles._predicted[p] = _particles._rest[p];
    _particles._velocity[p] = pxr::GfVec3f(0.f);
    if(_particles._state[p] != Particles::MUTE)
      _particles._state[p] = Particles::ACTIVE;
  }
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
  const pxr::GfVec3f* positions = &_particles._position[0];
  for (auto& body : _bodies) {
    size_t numPoints = body->GetNumPoints();
    for(size_t point = 0; point < 10; ++point) {
      _particles._mass[point + body->GetOffset()] = 0.f;
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
          _particles._mass[p + offset] *= 0.5f;
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
  if (_serial) {
    for (auto& collision : _collisions) {
      collision->FindContactsSerial(&_particles, _bodies, _contacts, _frameTime);
    }
  } else {
    size_t previous = 0;
    for (auto& collision : _collisions) {
      collision->FindContacts(&_particles, _bodies, _contacts, _frameTime);
      previous = _contacts.size();
    }
  }
}

void Solver::_IntegrateParticles(size_t begin, size_t end)
{

  pxr::GfVec3f* velocity = &_particles._velocity[0];
  int* body = &_particles._body[0];

  // apply external forces
  for (const Force* force : _force) {
    force->Apply(begin, end, &_particles, _frameTime);
  }

  // compute predicted position
  pxr::GfVec3f* predicted = &_particles._predicted[0];
  pxr::GfVec3f* position = &_particles._position[0];

  for (size_t index = begin; index < end; ++index) {

    position[index] = predicted[index];
    predicted[index] = position[index] + velocity[index] * _stepTime;
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end)
{
  const int* body = &_particles._body[0];
  const pxr::GfVec3f* predicted = &_particles._predicted[0];
  pxr::GfVec3f* position = &_particles._position[0];
  pxr::GfVec3f* velocity = &_particles._velocity[0];
  pxr::GfVec3f* previous = &_particles._previous[0];
  short* state = &_particles._state[0];

  float invDt = 1.f / _stepTime;

  for(size_t index = begin; index < end; ++index) {
    if (_particles._state[index] != Particles::ACTIVE)continue;
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
  if (_serial) {
    // solve constraints
    for (auto& constraint : constraints)constraint->Solve(&_particles, _stepTime);
    // apply result
    for (auto& constraint : constraints)constraint->Apply(&_particles);

  } else {
    // solve constraints
    pxr::WorkParallelForEach(constraints.begin(), constraints.end(),
      [&](Constraint* constraint) {constraint->Solve(&_particles, _stepTime); });
    
    // apply constraint serially
    for (auto& constraint : constraints)constraint->Apply(&_particles);
  }

}


void Solver::_SolveVelocities()
{
  for (auto& collision : _collisions) {
    size_t numContacts = collision->GetNumContacts();
    if (!numContacts) continue;
    collision->SolveVelocities(&_particles, _stepTime);
  }
}


void Solver::_StepOneSerial()
{
  const size_t numParticles = _particles.GetNumParticles();
  const size_t numContacts = _contacts.size();

  // integrate particles
  _IntegrateParticles(0, numParticles);

  // solve and apply constraint
  _SolveConstraints(_constraints);
  _SolveConstraints(_contacts);

  // update particles
  _UpdateParticles(0, numParticles);

  // solve velocities
  _SolveVelocities();
}

void Solver::_StepOne()
{
  const size_t numParticles = _particles.GetNumParticles();
  const size_t numContacts = _contacts.size();

  _timer->Start(1); 
  // integrate particles
  pxr::WorkParallelForN(
    numParticles,
    std::bind(&Solver::_IntegrateParticles, this,
      std::placeholders::_1, std::placeholders::_2));
  
  _timer->Next();
  // solve and apply constraint
  _SolveConstraints(_constraints);
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

void Solver::_GetContactPositions(pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  for (size_t i = 0; i < _collisions.size(); ++i) {
    std::vector<Location>& contacts = _collisions[i]->GetContacts();
    const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    float r = 0.2f;
    for (auto& contact : contacts) {
      positions.push_back(pxr::GfVec3f(contact.GetPointCoordinates()));
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
    _points->SetPositions(&_particles._position[0], numParticles);
    _points->SetRadii(&_particles._radius[0], numParticles);
    _points->SetColors(&_particles._color[0], numParticles);

    Scene::_Prim* prim = _scene->GetPrim(_pointsId);
    prim->bits = pxr::HdChangeTracker::AllDirty;
  } else {
    Step();

    _points->SetPositions(&_particles._position[0], numParticles);
    
    Scene::_Prim* prim = _scene->GetPrim(_pointsId);
    prim->bits = 
      pxr::HdChangeTracker::Clean | pxr::HdChangeTracker::DirtyPoints |
      pxr::HdChangeTracker::DirtyWidths | pxr::HdChangeTracker::DirtyPrimvar;
  }

  
}

void Solver::Step()
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();

  _timer->Start();
  _FindContacts();
  _timer->Stop();

  if (!_serial && numParticles >= 2 * numThreads) {
    //const size_t grain = numParticles / numThreads;
    for(size_t si = 0; si < _subSteps; ++si)
      _StepOne();
  }
  else {
    //std::cout << "serial step " << std::endl;
    for (size_t si = 0; si < _subSteps; ++si)
      _StepOneSerial();
  }

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
  prim.GetAttribute(pxr::TfToken("SubSteps")).Get(&_subSteps, time);
  _stepTime = _frameTime / static_cast<float>(_subSteps);
  prim.GetAttribute(pxr::TfToken("SleepThreshold")).Get(&_sleepThreshold, time);
}





JVR_NAMESPACE_CLOSE_SCOPE