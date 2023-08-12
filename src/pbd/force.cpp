#include <pxr/base/work/loops.h>

#include "../pbd/force.h"
#include "../pbd/body.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {

  void Force::RemoveBody(Solver* solver, size_t index)
  {
    if (!HasParticles()) return;
    Body* body = solver->GetBody(index);
    if (!body) return;

    size_t offset = body->offset;
    size_t remove = body->numPoints;

    size_t removeStart = Solver::INVALID_INDEX;
    size_t removeEnd = Solver::INVALID_INDEX;

    for (size_t p = 0; p < _particles.size(); ++p) {
      if (_particles[p] < offset) continue;
      if (_particles[p] < offset + remove) {
        if (removeStart == Solver::INVALID_INDEX) {
          removeStart = p;
          removeEnd = p;
        } else {
          removeEnd = p;
        }
      } else {
        _particles[p] -= remove;
      }

      if (removeStart != Solver::INVALID_INDEX) {
        _particles.erase(_particles.begin() + removeStart, _particles.begin() + removeEnd);
      }
    }
  }

  GravitationalForce::GravitationalForce() 
    : _gravity(0.f, -9.18f, 0.f)
  {
  }

  GravitationalForce::GravitationalForce(const pxr::GfVec3f& gravity) 
    : _gravity(gravity)
  {
  }

  void GravitationalForce::Apply(Solver* solver, const float dt)
  {
    pxr::GfVec3f* velocities = solver->GetVelocityPtr();
    if (HasParticles()) {
      for (const int& particle : _particles) {
        velocities[particle] += dt * _gravity;
      }
    } else {
      size_t numParticles = solver->GetNumParticles();
      for (size_t particle = 0; particle < numParticles; ++particle) {
        velocities[particle] += dt * _gravity;
      }
    }
  }
} // namespace PBD

JVR_NAMESPACE_CLOSE_SCOPE