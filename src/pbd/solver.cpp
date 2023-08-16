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
  , _solverIterations(4)
  , _collisionIterations(2)
  , _paused(true)
{
  _timeStep = 1.f / GetApplication()->GetTime().GetFPS();
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

void Solver::AddBody(Geometry* geom, const pxr::GfMatrix4f& matrix)
{
  std::cout << "[system] add geometry : " << geom << std::endl;
  size_t base = _particles.GetNumParticles();
  size_t add = geom->GetNumPoints();

  std::cout << "num particles before add : " << base << std::endl;

  Body* body = new Body({ 1.f, 1.f, 1.f, base, geom->GetNumPoints(), geom });
  _bodies.push_back(body);
  _particles.AddBody(body, matrix);

  std::cout << "num particles after add : " << _particles.GetNumParticles() << std::endl;



  /*
  float          damping;
float          radius;
float          mass;

size_t         offset;
size_t         numPoints;

Geometry*      geometry;
  */
}

void Solver::RemoveBody(Geometry* geom)
{
  std::cout << "[system] remove geometry : " << geom << std::endl;
  size_t index = GetBodyIndex(geom);
  if (index == Solver::INVALID_INDEX) return;

  Body* body = GetBody(index);
  _particles.RemoveBody(body);
  _particles = _particles;

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

void Solver::AddConstraints(Geometry* geom, size_t offset)
{
  std::cout << "[solver] add constraints for type " << geom->GetType() << std::endl;
  if (geom->GetType() == Geometry::MESH) {
    std::cout << "[solver] add constraints for mesh : " << std::endl;
    Mesh* mesh = (Mesh*)geom;
    const pxr::VtArray<HalfEdge>& edges = mesh->GetEdges();
    std::cout << "[solver] num unique edges : " << edges.size() << std::endl;
   
    for (const HalfEdge& edge : edges) {
      DistanceConstraint* constraint = new DistanceConstraint();
      constraint->Init(&_particles, edge.vertex + offset, edges[edge.next].vertex + offset, 0.5f, 0.5f);
      _constraints.push_back(constraint);
    }
    std::cout << "num constraints : " << _constraints.size() << std::endl;
  } else if (geom->GetType() == Geometry::CURVE) {
    Curve* curve = (Curve*)geom;
    curve->GetTotalNumSegments();
    for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {

    }
  }
}

void Solver::_IntegrateParticles(size_t begin, size_t end, const float dt)
{
  pxr::GfVec3f* velocity = &_particles.velocity[0];

  // apply external forces
  for (const Force* force : _force) {
    force->Apply(begin, end, velocity, dt);
  }

  // compute predicted position
  pxr::GfVec3f* predicted = &_particles.predicted[0];
  const pxr::GfVec3f* position = &_particles.position[0];
  for (size_t index = begin; index < end; ++index) {
    predicted[index] = position[index] + dt * velocity[index];
  }
}

void Solver::_UpdateParticles(size_t begin, size_t end, const float dt)
{
  const pxr::GfVec3f* predicted = &_particles.predicted[0];
  pxr::GfVec3f* position = &_particles.position[0];
  pxr::GfVec3f* velocity = &_particles.velocity[0];

  float invDt = 1.f / dt;
  float threshold2 = _sleepThreshold * dt;
  threshold2 *= threshold2;

  for(size_t index = begin; index < end; ++index) {
    // update velocity
    pxr::GfVec3f d = predicted[index] - position[index];
    velocity[index] = d * invDt;
    const float m = velocity[index].GetLengthSq();
    if (m < threshold2)
      velocity[index] = pxr::GfVec3f(0.f);

    // update position
    position[index] = predicted[index];
    if (position[index][1] < -5.f)position[index][1] = -5.f;
  }
}

void Solver::Step(float dt, bool serial)
{
  const size_t numParticles = _particles.GetNumParticles();
  if (!numParticles)return;

  size_t numThreads = pxr::WorkGetConcurrencyLimit();
  size_t numConstraints = _constraints.size();

  //std::cout << "num available threads : " << numThreads << std::endl;
  //std::cout << "num forces : " << _force.size() << std::endl;
  
 
  if (!serial && numParticles >= 2 * numThreads) {
    //std::cout << "parallel step " << std::endl;
    const size_t grain = numParticles / numThreads;
    //std::cout << "grain size : " << grain << std::endl;

    // integrate particles
    pxr::WorkParallelForN(
      numParticles,
      std::bind(&Solver::_IntegrateParticles, this,
        std::placeholders::_1, std::placeholders::_2, dt), grain);

    // TODO solve constraints and collisions
    // solve constraints
    pxr::WorkParallelForEach(_constraints.begin(), _constraints.end(),
      [&](Constraint* constraint) {constraint->Solve(&_particles, 1); });

    // apply constraint serially
    for (auto& constraint : _constraints)constraint->Apply(&_particles, dt);

    // update particles
    pxr::WorkParallelForN(
      numParticles,
      std::bind(&Solver::_UpdateParticles, this,
        std::placeholders::_1, std::placeholders::_2, dt), grain);

  }
  else {
    //std::cout << "serial step " << std::endl;

    // integrate particles
    _IntegrateParticles(0, numParticles, dt);

    // TODO solve constraints and collisions
    for (auto& constraint : _constraints) constraint->Solve(&_particles, 1);
    // apply constraint serially
    for (auto& constraint : _constraints)constraint->Apply(&_particles, dt);

    // update particles
    _UpdateParticles(0, numParticles, dt);
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