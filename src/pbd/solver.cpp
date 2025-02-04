#include <iostream>

#include <pxr/base/work/loops.h>

#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>

#include "../utils/color.h"
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
#include "../pbd/constraint.h"
#include "../pbd/force.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../app/time.h"


JVR_NAMESPACE_OPEN_SCOPE

static const size_t NUM_TIMES = 6;
static const char* TIME_NAMES[NUM_TIMES] = {
  "find contacts",
  "integrate particles",
  "solve constraints",
  "update contacts",
  "solve contacts",
  "update particles"
};

Solver::Solver(Scene* scene, UsdPrim& prim)
  : Xform(UsdGeomXform(prim), GfMatrix4d(1.0))
  , _particles()
  , _scene(scene)
  , _selfCollisions(nullptr)
  , _subSteps(5)
  , _sleepThreshold(0.001f)
  , _paused(true)
  , _startTime(1.f)
  , _solverId(prim.GetPath())
  , _gravity(nullptr)
  , _damp(nullptr)
{
  _frameTime = 1.f / Time::Get()->GetFPS();
  _stepTime = _frameTime / static_cast<float>(_subSteps);

  _timer = new Timer();
  _timer->Init("xpbd solver", NUM_TIMES, &TIME_NAMES[0]);

  _pointsId = _solverId.AppendChild(TfToken("Particles"));
  _points = (Points*)_scene->AddGeometry(_pointsId, Geometry::POINT, GfMatrix4d(1.0));

  _curvesId = _solverId.AppendChild(TfToken("Constraints"));
  _curves = (Curve*)_scene->AddGeometry(_curvesId, Geometry::CURVE, GfMatrix4d(1.0));

  //UsdSolver
  UsdPbdSolver solver(prim);
  UsdAttribute gravityAttr = solver.GetGravityAttr();
   _gravity = new GravityForce(gravityAttr);
  AddElement(_gravity, NULL, _solverId.AppendProperty(gravityAttr.GetName()));

}

Solver::~Solver()
{
  for (auto& constraint : _constraints)delete constraint;
  for (auto& contact : _contacts)delete contact;
  for (auto& body : _bodies)delete body;
  for (auto& collision: _collisions)delete collision;
  for (auto& force : _forces)delete force;
  _scene->RemoveGeometry(_pointsId);
  _scene->RemoveGeometry(_curvesId);
  if(_selfCollisions)
    delete _selfCollisions;
  delete _curves;
  delete _points;
  delete _timer;
}

void Solver::AddElement(Element* element, Geometry* geom, const SdfPath& path)
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

SdfPath Solver::GetElementPath(Element* element)
{
  return _elements[element].first;
}

Geometry* Solver::GetElementGeometry(Element* element)
{
  return _elements[element].second;
}

Element* Solver::GetElement(const SdfPath& path)
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


Body* Solver::CreateBody(Geometry* geom, const GfMatrix4d& matrix, 
  float mass, float radius, float damping, bool attach)
{
  size_t base = _particles.GetNumParticles();
  GfVec3f wirecolor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  UsdPbdBodyAPI api(geom->GetPrim());

  api.GetMassAttr().Get(&mass, UsdTimeCode::Default());
  api.GetDampAttr().Get(&damping, UsdTimeCode::Default());
  Body* body = new Body(geom, base, geom->GetNumPoints(), wirecolor, mass, radius, damping);

  _particles.AddBody(body, matrix);
  if(attach)
    CreateConstraints(body, Constraint::ATTACH, 10000.f, 0.25f);

  //if(_showPoints)UpdatePointsDisplay();
  //else ClearPointsDisplay();

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

void Solver::SetBodyVelocity(Body* body, const GfVec3f& velocity)
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
}

void Solver::CreateConstraints(Body* body, short type, float stiffness, float damping)
{
  Geometry* geom = body->GetGeometry();
  size_t start = _constraints.size();
  ConstraintsGroup* group = NULL;
  switch(type) {
    case Constraint::ATTACH:
      group = CreateAttachConstraints(body, stiffness, damping);
      break;

    case Constraint::STRETCH:
      group = CreateStretchConstraints(body, stiffness, damping);
      break;

    case Constraint::SHEAR:
      group = CreateShearConstraints(body, stiffness, damping);
      break;

    case Constraint::BEND:
      group = CreateBendConstraints(body, stiffness, damping);
      break;

    case Constraint::DIHEDRAL:
      if(geom->GetType() == Geometry::MESH) {
        group = CreateDihedralConstraints(body, stiffness, damping);
      } else {
        TF_WARN("Dihedral constraints can only be applied on meshes !");
      }
      break;
  }
  if(group != NULL)
    for(size_t c = 0; c < group->constraints.size(); ++c)
      AddConstraint(group->constraints[c]);
}

void Solver::AddConstraint(Constraint* constraint) 
{ 
  _constraints.push_back(constraint); 
};

void Solver::GetConstraintsByType(short type, std::vector<Constraint*>& results)
{
  for(auto& constraint: _constraints)
    if(constraint->GetTypeId() == type)
      results.push_back(constraint);
}

void Solver::LockPoints(Body* body, VtArray<int>& elements)
{
  const size_t offset = body->GetOffset();
  for(size_t i = 0; i < elements.size(); ++i) {
    _particles.mass[elements[i] + offset] = 0.f;
    _particles.invMass[elements[i] + offset] = 0.f;
  }
}

void Solver::AttachPoints(Body* body, VtArray<int>& elements)
{
  Mesh* mesh = (Mesh*)body->GetGeometry();
  //CreateAttachConstraints()
}

void Solver::PinPoints(Body* body, Geometry* target, VtArray<int>& elements)
{
  Mesh* mesh = (Mesh*)body->GetGeometry();
  //CreatePinConstraints()
}

void Solver::UpdatePointsDisplay()
{
  size_t numParticles = _particles.GetNumParticles();
  
  VtArray<GfVec3f> positions(numParticles);
  memcpy(&positions[0], &_particles.position[0], numParticles * sizeof(GfVec3f));
  VtArray<GfVec3f> colors(numParticles);
  memcpy(&colors[0], &_particles.color[0], numParticles * sizeof(GfVec3f));
  VtArray<float> widths(numParticles);
  for(size_t p = 0; p<numParticles; ++p)
    widths[p] = 2.f * _particles.radius[p];

  size_t numCollisions = _collisions.size();
  for(size_t c = 0; c < numCollisions; ++c) {
    if(_collisions[c]->GetTypeId() == Collision::MESH) {
      _collisions[c]->GetPoints(&_particles, positions, widths, colors);
    }
  }

  size_t numPoints = positions.size();
  if(numPoints) {
    _points->SetPositions(&positions[0], numPoints);
    _points->SetWidths(&widths[0], numPoints);
    _points->SetColors(&colors[0], numPoints);


    _scene->MarkPrimDirty(_pointsId, HdChangeTracker::AllDirty);
  }
}

void Solver::ClearPointsDisplay()
{
  if(_points->GetNumPoints()) {
    _points->RemoveAllPoints();
    _scene->MarkPrimDirty(_pointsId, HdChangeTracker::AllDirty);
  }
}

void Solver::UpdateConstraintsDisplay()
{
  size_t numConstraints = _constraints.size();

  VtArray<GfVec3f> positions;
  VtArray<float> widths;
  VtArray<GfVec3f> colors;
  VtArray<int> counts;

  
  for(size_t c = 0; c < numConstraints; ++c) {
    if(_constraints[c]->GetTypeId() == Constraint::ATTACH) continue;
    _constraints[c]->GetPoints(&_particles, positions, widths, colors);

    for(size_t d = 0; d < _constraints[c]->GetNumElements(); ++d)
      counts.push_back(2);
  }
  /*
 size_t numCollisions = _collisions.size();
  for(size_t c = 0; c < numCollisions; ++c) {
    if(_collisions[c]->GetTypeId() != Collision::MESH) continue;
    _collisions[c]->GetNormals(&_particles, positions, widths, colors, counts);
    _c
    */
  _curves->SetTopology(positions, widths, counts);
  _curves->SetColors(colors);

  _scene->MarkPrimDirty(_curvesId, HdChangeTracker::AllDirty);
}

void Solver::ClearConstraintsDisplay()
{
  if(_curves->GetNumCurves()) {
    _curves->RemoveAllCurves();
    _scene->MarkPrimDirty(_curvesId, HdChangeTracker::AllDirty);
  }
}

void Solver::WeightBoundaries(Body* body)
{
  return;
  size_t offset = body->GetOffset();
  size_t numPoints = body->GetNumPoints();

  if(body->GetGeometry()->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->GetGeometry();
    HalfEdgeGraph* graph = mesh->GetEdgesGraph();
    const VtArray<bool>& boundaries = graph->GetBoundaries();
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

void Solver::_PrepareContacts()
{
  _timer->Start(0);
  for (auto& contact: _contacts)
    delete contact;

  _contacts.clear();

  for (auto& collision : _collisions)
    collision->FindContacts(&_particles, _bodies, _contacts, _frameTime);

  if(_selfCollisions)
    _selfCollisions->FindContacts(&_particles, _bodies, _contacts, _frameTime);

  _particles.ResetCounter(_contacts, 1);
  _timer->Stop();
}



void Solver::_UpdateContacts()
{
  for (auto& collision : _collisions)
    collision->UpdateContacts(&_particles);

  if(_selfCollisions)
    _selfCollisions->UpdateContacts(&_particles);
}

void Solver::_IntegrateParticles(size_t begin, size_t end)
{
  GfVec3f* velocity = &_particles.velocity[0];

  // compute predicted position
  GfVec3f* previous = &_particles.previous[0];
  GfVec3f* predicted = &_particles.predicted[0];
  GfVec3f* position = &_particles.position[0];
  GfVec3f* colors = &_particles.color[0];

  // apply external forces
  for (const Force* force : _forces)
    force->Apply(begin, end, &_particles, _stepTime);

  for (size_t index = begin; index < end; ++index) {
    if(_particles.state[index] == Particles::IDLE)
      if((predicted[index] - position[index]).GetLength() > _sleepThreshold )
        _particles.state[index] = Particles::ACTIVE;

    if(_particles.state[index] != Particles::ACTIVE)continue;

    previous[index] = velocity[index];
    position[index] = predicted[index];
    predicted[index] = position[index] + velocity[index] * _stepTime;

    colors[index] = RandomColorByIndex(index);
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end)
{
  const GfVec3f* predicted = &_particles.predicted[0];
  const GfVec3f* input = &_particles.input[0];
  const float* mass = &_particles.mass[0];
  GfVec3f* position = &_particles.position[0];
  GfVec3f* previous = &_particles.previous[0];
  GfVec3f* velocity = &_particles.velocity[0];
  short* state = &_particles.state[0];

  float invDt = 1.f / _stepTime, vL;
  const float vMax = 25.f;

  const double velDecay = std::exp(std::log(0.95f) * _stepTime);

  for(size_t index = begin; index < end; ++index) {
    if (state[index] != Particles::ACTIVE)continue;
    
    // update velocity
    //previous[index] = velocity[index];
    velocity[index] = (predicted[index] - position[index]) * invDt; 
    velocity[index] *= velDecay;

    float damp = _particles.body[index]->GetDamp();
    velocity[index] -= velocity[index] * damp * _particles.invMass[index] * _stepTime;
    vL = velocity[index].GetLength();
    if (vL < _sleepThreshold) {
      velocity[index] = GfVec3f(0.f);
    } else if(vL > vMax) {
      velocity[index] = velocity[index].GetNormalized() * vMax;
    }
    
    // update position
    if (mass[index] == 0.f)
      position[index] = input[index];
    else
      position[index] = predicted[index];
  }
}

void 
Solver::_SolveConstraints(std::vector<Constraint*>& constraints)
{
  // solve constraints
  WorkParallelForEach(constraints.begin(), constraints.end(),
    [&](Constraint* constraint) {
      if(constraint->IsActive())
        constraint->SolvePosition(&_particles, _stepTime); 
    });
  
  // apply constraint serially
  for (auto& constraint : constraints)
    if(constraint->IsActive())
      constraint->ApplyPosition(&_particles);

}

void 
Solver::_SolveVelocities(std::vector<Constraint*>& constraints)
{
  // solve velocities
  WorkParallelForEach(constraints.begin(), constraints.end(),
    [&](Constraint* constraint) {
      if(constraint->IsActive())
        constraint->SolveVelocity(&_particles, _stepTime); 
    });

  // apply velocities serially
  for (auto& constraint : constraints)
    if(constraint->IsActive())
      constraint->ApplyVelocity(&_particles);

  // smooth velocities
  for(auto& body: _bodies)
    body->SmoothVelocities(&_particles, 1);
}


void Solver::Update(UsdStageRefPtr& stage, float time)
{
  size_t numParticles = _particles.GetNumParticles();
  if (!_initialized ||GfIsClose(time, _startTime, 0.001f)) {
    Reset(stage);
  } else {
    Step(stage, time);
  }

  if(_showPoints)UpdatePointsDisplay();
  else ClearPointsDisplay();
  if(_showConstraints)UpdateConstraintsDisplay();
  else ClearConstraintsDisplay();
  UpdateGeometries();

}


void Solver::Reset(UsdStageRefPtr& stage)
{
  UpdateInputs(stage, _startTime);
  UpdateParameters(stage, _startTime);
  UpdateCollisions(stage, _startTime);

  // reset
  _particles.RemoveAllBodies();

  for (size_t b = 0; b < _bodies.size(); ++b) {
    const GfMatrix4d& matrix = _bodies[b]->GetGeometry()->GetMatrix();
    _particles.AddBody(_bodies[b], matrix);
  }

  _particles.ResetCounter(_constraints, 0);
  
  size_t nL = 5;
  for (size_t b = 0; b < _bodies.size(); ++b) {
    WeightBoundaries(_bodies[b]);
    VtArray<int> locked(nL);
    Deformable* deformable = (Deformable*)_bodies[b]->GetGeometry();
    const GfVec3f* positions = deformable->GetPositionsCPtr();
    size_t numPoints = deformable->GetNumPoints();
    std::vector<std::pair<int, float>> pairs(numPoints);
    
    for(size_t i = 0; i < numPoints; ++i)
      pairs[i] = std::make_pair(i, positions[i][1]);

    std::sort(pairs.begin(), pairs.end(), [](auto& lhs, auto& rhs) {
      return lhs.second > rhs.second;
    });
    for(size_t i = 0; i < nL; ++i)
      locked[i]  = pairs[i].first;

    LockPoints(_bodies[b], locked);
  }

  _particles.SetAllState(Particles::ACTIVE);

  if(_selfCollisions)delete _selfCollisions;
  _selfCollisions = new SelfCollision(&_particles, 
    GetPrim().GetPath().AppendProperty(TfToken("selfCollide")), 0.5f, 0.5f);

  for(auto& constraint: _constraints)
    constraint->Reset(&_particles);
  
  if(_selfCollisions)
    _selfCollisions->Reset();

  for(auto& collision: _collisions)
    collision->Reset();

  UpdateConstraintsDisplay();
}

void Solver::Step(UsdStageRefPtr& stage, float time)
{
  UpdateInputs(stage, time);
  UpdateParameters(stage, time);
  UpdateCollisions(stage, time);

  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = WorkGetConcurrencyLimit();

  size_t packetSize = numParticles / (numThreads > 1 ? numThreads - 1 : 1);

  _PrepareContacts();
  for(size_t si = 0; si < _subSteps; ++si) {

    _SolveVelocities(_contacts);

    _timer->Start(1);
    // integrate particles
    WorkParallelForN(
      numParticles,
      std::bind(&Solver::_IntegrateParticles, this,
        std::placeholders::_1, std::placeholders::_2), packetSize);

    _timer->Next();
    // solve and apply constraint
    _SolveConstraints(_constraints);

    _timer->Next();
     _UpdateContacts();

    // solve and apply contacts
    _timer->Next();
    _SolveConstraints(_contacts);

    _timer->Next();
    // update particles
    WorkParallelForN(
      numParticles,
      std::bind(&Solver::_UpdateParticles, this,
        std::placeholders::_1, std::placeholders::_2), packetSize);
    _timer->Stop();

  }
  
  _timer->Update();
  _timer->Log();
}


void Solver::UpdateInputs(UsdStageRefPtr& stage, float time)
{
  for(size_t i = 0; i < _bodies.size(); ++i){
    UsdPrim prim = _bodies[i]->GetGeometry()->GetPrim();
    if(!prim.IsValid())continue;

    if (prim.IsA<UsdGeomMesh>()) {
      UsdGeomMesh mesh(prim);
      VtArray<GfVec3f> inputs;
      mesh.GetPointsAttr().Get(&inputs, time);

      const GfMatrix4d& matrix = _bodies[i]->GetGeometry()->GetMatrix();

      for (size_t p = 0; p < inputs.size(); ++p)
        _particles.input[_bodies[i]->GetOffset() + p] = GfVec3f(matrix.Transform(inputs[p]));

    }
  }
}

void Solver::UpdateCollisions(UsdStageRefPtr& stage, float time)
{
  for(size_t i = 0; i < _collisions.size(); ++i){
    SdfPath path = GetElementPath(_collisions[i]);
    UsdPrim prim = stage->GetPrimAtPath(path);
    _collisions[i]->Update(prim, time);
  }

  if(_selfCollisions)
    _selfCollisions->Update(GetPrim(), time);
}

void Solver::UpdateGeometries()
{
  const auto* positions = &_particles.position[0];
  _ElementMap::iterator it = _elements.begin();
  for (; it != _elements.end(); ++it)
  {
    if(it->first->GetType() == Element::BODY) {
      Body* body = (Body*)it->first;
      SdfPath id = it->second.first;
      Geometry* geometry = it->second.second;
      if(geometry->GetType() >= Geometry::POINT) {
        Deformable* deformable = (Deformable*)geometry;
        size_t numPoints = body->GetNumPoints();
        GfVec3f* output = deformable->GetPositionsPtr();
        size_t offset = body->GetOffset();
        GfRange3f range;
        for (size_t p = 0; p < numPoints; ++p) {
          const GfVec3f local(deformable->GetInverseMatrix().Transform(positions[offset + p]));
          range.UnionWith(local);
          output[p] = local;
        }
        deformable->SetBoundingBox(range);

        _scene->MarkPrimDirty(id, HdChangeTracker::DirtyPoints);
      }
    }
  }
  
}

void Solver::UpdateParameters(UsdStageRefPtr& stage, float time)
{
  UsdPrim prim = stage->GetPrimAtPath(_solverId);
  UsdPbdSolver solver(prim);

  _frameTime = 1.f / static_cast<float>(Time::Get()->GetFPS());
  solver.GetSubStepsAttr().Get(&_subSteps, time);
  _stepTime = _frameTime / static_cast<float>(_subSteps);
  solver.GetSleepThresholdAttr().Get(&_sleepThreshold, time);

  solver.GetShowPointsAttr().Get(&_showPoints, time);
  solver.GetShowConstraintsAttr().Get(&_showConstraints, time);

  if(_gravity)_gravity->Update(time);
  if (_damp)_damp->Update(time);

  for(auto& body: _bodies) {
    prim = body->GetGeometry()->GetPrim();
    body->UpdateParameters(prim, time);
    body->UpdateParticles(&_particles);
  }
}



JVR_NAMESPACE_CLOSE_SCOPE