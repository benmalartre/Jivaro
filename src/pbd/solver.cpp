#include "../acceleration/bvh.h"
#include "../acceleration/hashGrid.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/voxels.h"
#include "../geometry/curve.h"
#include "../app/application.h"
#include "../pbd/constraint.h"
#include "../pbd/force.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"
#include "../app/time.h"


#include <iostream>

#include <pxr/base/work/loops.h>


JVR_NAMESPACE_OPEN_SCOPE

Solver::Solver()
  : _gravity(0, -9.18, 0)
  , _subSteps(8)
  , _solverIterations(1)
  , _sleepThreshold(0.1f)
  , _paused(true)
{
  _frameTime = 1.f / 60.f;//GetApplication()->GetTime().GetFPS();
  _stepTime = _frameTime / static_cast<float>(_subSteps);
}

Solver::~Solver()
{
  for (auto& body : _bodies)delete body;
  for (auto& force : _force)delete force;
  for (auto& constraint : _constraints)delete constraint;
}

void Solver::Reset()
{
  //UpdateColliders();
  // reset
  for (size_t p = 0; p < GetNumParticles(); ++p) {
    _particles.position[p] = _particles.rest[p];
    _particles.predicted[p] = _particles.rest[p];
    _particles.velocity[p] = pxr::GfVec3f(0.f);
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
    if (body->geometry == geom)return body;
  }
  return nullptr;
}

size_t Solver::GetBodyIndex(Geometry* geom)
{
  for (size_t index = 0; index < _bodies.size(); ++index) {
    if (_bodies[index]->geometry == geom)return index;
  }
  return Solver::INVALID_INDEX;
}

Body* Solver::AddBody(Geometry* geom, const pxr::GfMatrix4f& matrix, float mass)
{
  std::cout << "[system] add geometry : " << geom << std::endl;
  size_t base = _particles.GetNumParticles();
  size_t add = geom->GetNumPoints();

  std::cout << "num particles before add : " << base << std::endl;

  pxr::GfVec3f wirecolor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  Body* body = new Body({ 1.f, 0.1f, mass, base, geom->GetNumPoints(), wirecolor, geom });
  _bodies.push_back(body);
  _particles.AddBody(body, matrix);

  std::cout << "num particles after add : " << _particles.GetNumParticles() << std::endl;

  return _bodies.back();
}

void Solver::RemoveBody(Geometry* geom)
{
  std::cout << "[system] remove geometry : " << geom << std::endl;
  size_t index = GetBodyIndex(geom);
  if (index == Solver::INVALID_INDEX) return;

  Body* body = GetBody(index);
  _particles.RemoveBody(body);

  _bodies.erase(_bodies.begin() + index);
  delete body;
}

/*
void Solver::AddCollision(Geometry* collider)
{
  _colliders.push_back(collider);

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
    Hit hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, DBL_MAX, &minDistance)) {
      result.push_back(hit.GetPosition(colliders[hit.GetGeometryIndex()]));
    }
  }
  std::cout << "   hit time (" << numRays << " rays) : " << ((CurrentTime() - T) * 1e-9) << std::endl;
  _SetupBVHInstancer(stage, &bvh);


  _SetupResults(stage, result);

}

void Solver::UpdateColliders()
{

  BVH bvh;
  bvh.Init(_colliders);

  {
    double minDistance;
    Hit hit;
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
    Hit hit;
    const pxr::GfVec3f* points = _colliders[0]->GetPositionsCPtr();
    if (bvh.Raycast(points, ray, &hit, -1, &minDistance)) {
      pxr::GfVec3f position;
      hit.GetPosition(_colliders[0]);
    }
  }

}
*/

void Solver::AddConstraints(Body* body)
{
  // 0.1, 0.01, 0.001, 0.0001, 0.00001
  static float __stretchStiffness = 2000.f;
  static float __bendStiffness = 0.0001f;

  static int __bodyIdx = 0;
  Geometry* geom = body->geometry;
  if (geom->GetType() == Geometry::MESH) {
    
    CreateStretchConstraints(body, _constraints, __stretchStiffness);

    std::cout << "body " << (__bodyIdx) <<  " stretch stiffness : " <<  __stretchStiffness <<
      "(compliance="<< (1.f/__stretchStiffness) << ")" <<std::endl;
    //__stretchStiffness *= 2.f;

    //CreateBendConstraints(body, _constraints, __bendStiffness);
    CreateDihedralConstraints(body, _constraints, __bendStiffness);
    std::cout << "body " << (__bodyIdx) <<  " bend stiffness : " <<  __bendStiffness <<
      "(compliance="<< (1.f/__bendStiffness) << ")" <<std::endl;
    __bendStiffness *= 10.f;

    __bodyIdx++;

   //


  } else if (geom->GetType() == Geometry::CURVE) {
    Curve* curve = (Curve*)geom;
    curve->GetTotalNumSegments();
    for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {

    }
  }
}

void Solver::GetConstraintsByType(short type, pxr::VtArray<Constraint*>& results)
{
  for(auto& constraint: _constraints)
    if(constraint->GetTypeId() == type)
      results.push_back(constraint);
}

void Solver::LockPoints()
{
  size_t particleIdx = 0;
  const pxr::GfVec3f* positions = &_particles.position[0];
  for (auto& body : _bodies) {
    size_t numPoints = body->numPoints;
    for(size_t point = 0; point < 10; ++point) {
      _particles.mass[point + body->offset] = 0.f;
    }
  }
}

void Solver::WeightBoundaries()
{
  for(const auto& body: _bodies) {
    size_t offset = body->offset;
    size_t numPoints = body->numPoints;
  
    if(body->geometry->GetType() == Geometry::MESH) {
      Mesh* mesh = (Mesh*)body->geometry;
      HalfEdgeGraph* graph = mesh->GetEdgesGraph();
      const pxr::VtArray<bool>& boundaries = graph->GetBoundaries();
      for(size_t p = 0; p < boundaries.size(); ++p){
        if(boundaries[p]) {
          _particles.mass[p + offset] *= 0.5f;
        }
      }
    }
  } 
}

void Solver::_IntegrateParticles(size_t begin, size_t end)
{
  pxr::GfVec3f* velocity = &_particles.velocity[0];

  // apply external forces
  for (const Force* force : _force) {
    force->Apply(begin, end, &_particles, _frameTime);
  }

  // compute predicted position
  pxr::GfVec3f* predicted = &_particles.predicted[0];
  const pxr::GfVec3f* position = &_particles.position[0];
  for (size_t index = begin; index < end; ++index) {
    predicted[index] = position[index] + _stepTime * velocity[index];
  }
}

void Solver::_ResolveCollisions(bool serial)
{
  if (serial) {
    for (auto& collision : _collisions) {
      collision->FindContactsSerial(&_particles);
      collision->ResolveContactsSerial(&_particles, 1.f);
    }
  } else {
    for (auto& collision : _collisions) {
      collision->FindContacts(&_particles);
      collision->ResolveContacts(&_particles, 1.f);
    }
  }
}

void Solver::_UpdateCollisions(bool serial)
{
  const float invDt = 1.f / _stepTime;
  if (serial) {
    for (auto& collision : _collisions) {
      collision->UpdateVelocitiesSerial(&_particles, invDt);
    }
  } else {
    for (auto& collision : _collisions) {
      collision->UpdateVelocities(&_particles, invDt);
    }
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end)
{
  const pxr::GfVec3f* predicted = &_particles.predicted[0];
  pxr::GfVec3f* position = &_particles.position[0];
  pxr::GfVec3f* velocity = &_particles.velocity[0];

  float invDt = 1.f / _stepTime;
  float threshold2 = _sleepThreshold * _stepTime;
  threshold2 *= threshold2;

  for(size_t index = begin; index < end; ++index) {
    // update velocity
    velocity[index] = (predicted[index] - position[index]) * invDt;
    const float m = velocity[index].GetLengthSq();
    if (m < threshold2)
      velocity[index] = pxr::GfVec3f(0.f);

    // update position
    position[index] = predicted[index];
  }
}

struct _T {
  uint64_t t;
  uint64_t accum;
  size_t   num;
  void Start() {
    t = CurrentTime();
  }
  void End() {
    accum += CurrentTime() - t;
    num++;
  };
  void Reset() {
    accum = 0;
    num = 0;
  };
  double Average() {
    if (num) {
      return ((double)accum * 1e-9) / (double)num;
    }
    return 0;
  }
  double Elapsed() {
    return (double)accum * 1e-9;
  }
};
static _T REMOVE_EDGE_AVG_T = { 0,0 };
static _T REMOVE_POINT_AVG_T = { 0,0 };


void Solver::_StepOneSerial()
{
  const size_t numParticles = _particles.GetNumParticles();

  // integrate particles
  _IntegrateParticles(0, numParticles);

  // solve collisions
  //_ResolveCollisions(false);

  // solve constraints
  for(auto& constraint: _constraints) {
    constraint->ResetLagrangeMultiplier();
  }

  for (size_t ci = 0; ci < _solverIterations; ++ci) {
    /*
    for (auto& constraint : _constraints) constraint->Solve(&_particles, ds);
    // apply constraint serially
    for (auto& constraint : _constraints)constraint->Apply(&_particles);
    */
    for (auto& constraint : _constraints)constraint->Solve(&_particles, _stepTime);
    for (auto& constraint : _constraints)constraint->Apply(&_particles);
  }

  // update particles
  _UpdateParticles(0, numParticles);
  // solve collisions
  //_UpdateCollisions(true);
}

void Solver::_StepOne()
{
  const size_t numParticles = _particles.GetNumParticles();

  // integrate particles
  pxr::WorkParallelForN(
    numParticles,
    std::bind(&Solver::_IntegrateParticles, this,
      std::placeholders::_1, std::placeholders::_2));

  // solve collisions
  //_ResolveCollisions(false);

  // solve constraints
  for(auto& constraint: _constraints) {
    constraint->ResetLagrangeMultiplier();
  }

  for (size_t ci = 0; ci < _solverIterations; ++ci) {
    pxr::WorkParallelForEach(_constraints.begin(), _constraints.end(),
      [&](Constraint* constraint) {constraint->Solve(&_particles, _stepTime); });

    // apply constraint serially
    for (auto& constraint : _constraints)constraint->Apply(&_particles);
  }

  // update particles
  pxr::WorkParallelForN(
    numParticles,
    std::bind(&Solver::_UpdateParticles, this,
      std::placeholders::_1, std::placeholders::_2));

  // update velocities
  //_UpdateCollisions(false);

}

void Solver::Step(bool serial)
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();
  size_t numConstraints = _constraints.size();


  //std::cout << "num available threads : " << numThreads << std::endl;
  //std::cout << "num forces : " << _force.size() << std::endl;

  if (!serial && numParticles >= 2 * numThreads) {
    //std::cout << "parallel step " << std::endl;
    //const size_t grain = numParticles / numThreads;
    for(size_t si = 0; si < _subSteps; ++si)
      _StepOne();
  }
  else {
    //std::cout << "serial step " << std::endl;
    for (size_t si = 0; si < _subSteps; ++si)
      _StepOneSerial();
  }

  //std::cout << _particles.GetPredicted() << std::endl;

  //UpdateGeometries();
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

JVR_NAMESPACE_CLOSE_SCOPE