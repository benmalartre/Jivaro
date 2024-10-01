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

static const size_t NUM_TIMES = 6;
static const char* TIME_NAMES[NUM_TIMES] = {
  "find contacts",
  "integrate particles",
  "solve constraints",
  "update contacts",
  "solve contacts",
  "update particles"
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

  _curvesId = _solverId.AppendChild(pxr::TfToken("Constraints"));
  _curves = (Curve*)_scene->AddGeometry(_curvesId, Geometry::CURVE, pxr::GfMatrix4d(1.0));

  pxr::UsdAttribute gravityAttr = xform.GetPrim().GetAttribute(PBDTokens->gravity);
   _gravity = new GravityForce(gravityAttr);
  AddElement(_gravity, NULL, _solverId.AppendProperty(PBDTokens->gravity));

  pxr::UsdAttribute dampAttr = xform.GetPrim().GetAttribute(PBDTokens->damp);
  _damp = new DampForce(dampAttr);
  AddElement(_damp, NULL, _solverId.AppendProperty(PBDTokens->damp));
}

Solver::~Solver()
{
  for (auto& constraint : _constraints)delete constraint;
  for (auto& body : _bodies)delete body;
  for (auto& force : _forces)delete force;
  _scene->RemoveGeometry(_pointsId);
  delete _curves;
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
  //geom->CreateAttribute(geom->GetPrim(), PBDTokens->mass, pxr::SdfValueTypeNames->Float);
  geom->GetAttributeValue(PBDTokens->mass, pxr::UsdTimeCode::Default(), &mass);
  geom->GetAttributeValue(PBDTokens->damp, pxr::UsdTimeCode::Default(), &damping);
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
}

void Solver::CreateConstraints(Body* body, short type, float stiffness, float damping)
{
  Geometry* geom = body->GetGeometry();
  size_t start = _constraints.size();
  ConstraintsGroup* group = NULL;
  switch(type) {
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
        //TF_WARN("Dihedral constraints can only be applied on meshes !");
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

void Solver::LockPoints(Body* body, pxr::VtArray<int>& elements)
{
  const size_t offset = body->GetOffset();
  for(size_t i = 0; i < elements.size(); ++i) {
    _particles.mass[elements[i] + offset] = 0.f;
    _particles.invMass[elements[i] + offset] = 0.f;
  }
}

void Solver::UpdatePoints()
{
  size_t numParticles = _particles.GetNumParticles();
  
  pxr::VtArray<pxr::GfVec3f> positions(numParticles);
  memcpy(&positions[0], &_particles.position[0], numParticles * sizeof(pxr::GfVec3f));
  pxr::VtArray<pxr::GfVec3f> colors(numParticles);
  memcpy(&colors[0], &_particles.color[0], numParticles * sizeof(pxr::GfVec3f));
  pxr::VtArray<float> widths(numParticles);
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


    _scene->MarkPrimDirty(_pointsId, pxr::HdChangeTracker::AllDirty);
  }
}

void Solver::UpdateCurves()
{
  size_t numConstraints = _constraints.size();

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<float> widths;
  pxr::VtArray<pxr::GfVec3f> colors;
  pxr::VtArray<int> counts;

  for(size_t c = 0; c < numConstraints; ++c) {
    //if(_constraints[c]->GetTypeId() != Constraint::BEND) continue;
    _constraints[c]->GetPoints(&_particles, positions, widths, colors);

    for(size_t d = 0; d < _constraints[c]->GetNumElements(); ++d)
      counts.push_back(2);
  }

  _curves->SetTopology(positions, widths, counts);
  _curves->SetColors(colors);

  _scene->MarkPrimDirty(_curvesId, pxr::HdChangeTracker::AllDirty);
}

void Solver::WeightBoundaries(Body* body)
{
  size_t offset = body->GetOffset();
  size_t numPoints = body->GetNumPoints();

  if(body->GetGeometry()->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->GetGeometry();
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

void Solver::_PrepareContacts()
{
  _timer->Start(0);
  for (auto& contact : _contacts)
    delete contact;
  _contacts.clear();

  for (auto& collision : _collisions)
    collision->FindContacts(&_particles, _bodies, _contacts, _frameTime);

  _particles.ResetCounter(_contacts, 1);
  _timer->Stop();
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
  for (const Force* force : _forces)
    force->Apply(begin, end, &_particles, _stepTime);

  for (size_t index = begin; index < end; ++index) {
    if(_particles.state[index] == Particles::IDLE)
      if((predicted[index] - position[index]).GetLength() > _sleepThreshold )
        _particles.state[index] = Particles::ACTIVE;

    if(_particles.state[index] != Particles::ACTIVE)continue;

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
  short* state = &_particles.state[0];

  float invDt = 1.f / _stepTime, vL;
  const float vMax = 5.f;

  const double velDecay = std::exp(std::log(0.95f) * _stepTime);

  for(size_t index = begin; index < end; ++index) {
    if (state[index] != Particles::ACTIVE)continue;
    // update velocity
    velocity[index] = (predicted[index] - position[index]) * invDt;
    velocity[index] *= velDecay;
    vL = velocity[index].GetLength();
    if (vL < _sleepThreshold) {
      state[index] = Particles::IDLE;
      velocity[index] = pxr::GfVec3f(0.f);
    } else if(vL > vMax) {
      velocity[index] = velocity[index].GetNormalized() * vMax;
    }



    // update position
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
  UpdateParameters(stage, time);
  UpdateCollisions(stage, time);
 
  size_t numParticles = _particles.GetNumParticles();
  if (pxr::GfIsClose(time, _startTime, 0.001f)) {
    Reset();
  } else {
    Step();
  }

  UpdatePoints();
  UpdateCurves();
  UpdateGeometries();

}

void 
Solver::UpdateVelocities()
{

}

void Solver::Reset()
{

  // reset
  _particles.RemoveAllBodies();


  for (size_t b = 0; b < _bodies.size(); ++b) {
    const pxr::GfMatrix4d& matrix = _bodies[b]->GetGeometry()->GetMatrix();
    _particles.AddBody(_bodies[b], matrix);
  }

  _particles.ResetCounter(_constraints, 0);

  if(_bodies.size()) {
    WeightBoundaries(_bodies[0]);
    pxr::VtArray<int> locked({0});//,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21});
    LockPoints(_bodies[0], locked);
  }

  _particles.SetAllState(Particles::ACTIVE);
  UpdateCurves();
}

void Solver::Step()
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();

  size_t packetSize = numParticles / (numThreads > 1 ? numThreads - 1 : 1);

  _PrepareContacts();
  for(size_t si = 0; si < _subSteps; ++si) {
    
    _timer->Start(1);
    // integrate particles
    pxr::WorkParallelForN(
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
    pxr::WorkParallelForN(
      numParticles,
      std::bind(&Solver::_UpdateParticles, this,
        std::placeholders::_1, std::placeholders::_2), packetSize);
    _timer->Stop();

  }
  
  _timer->Update();
  _timer->Log();
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

  const auto* positions = &_particles.position[0];
  _ElementMap::iterator it = _elements.begin();
  size_t offset = 0;
  for (; it != _elements.end(); ++it)
  {
    if(it->first->GetType() == Element::BODY) {
      pxr::SdfPath id = it->second.first;
      Geometry* geometry = it->second.second;
      if(geometry->GetType() >= Geometry::POINT) {
        Deformable* deformable = (Deformable*)geometry;
        size_t numPoints = deformable->GetNumPoints();
        pxr::VtArray<pxr::GfVec3f> results(numPoints);
      
        for (size_t p = 0; p < numPoints; ++p) {
          results[p] = deformable->GetInverseMatrix().Transform(positions[offset + p]);
        }
        deformable->SetPositions(&results[0], numPoints);
        _scene->MarkPrimDirty(id, pxr::HdChangeTracker::AllDirty);

        offset += numPoints;
      }
    }
  }
}

void Solver::UpdateParameters(pxr::UsdStageRefPtr& stage, float time)
{
  pxr::UsdPrim prim = stage->GetPrimAtPath(_solverId);
  _frameTime = 1.f / static_cast<float>(Time::Get()->GetFPS());
  prim.GetAttribute(PBDTokens->substeps).Get(&_subSteps, time);
  _stepTime = _frameTime / static_cast<float>(_subSteps);
  prim.GetAttribute(PBDTokens->sleep).Get(&_sleepThreshold, time);

  if(_gravity)_gravity->Update(time);
  if (_damp)_damp->Update(time);
  for(auto& body: _bodies) {
    prim = body->GetGeometry()->GetPrim();
    body->UpdateParameters(prim, time);
  }
}



JVR_NAMESPACE_CLOSE_SCOPE