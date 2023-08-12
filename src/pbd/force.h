#ifndef JVR_PBD_FORCE_H
#define JVR_PBD_FORCE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

namespace PBD {
  class Solver;
  class Force
  {
  public:
    bool HasParticles() { return _particles.size() > 0; };
    void SetParticles(const pxr::VtArray<int>& particles) { _particles = particles; };

    void RemoveBody(Solver* solver, size_t index);

    virtual void Apply(Solver* solver, const float dt) = 0;

  protected:
    pxr::VtArray<int> _particles;
  };

  class GravitationalForce : public Force
  {
  public:
    GravitationalForce();
    GravitationalForce(const pxr::GfVec3f& gravity);

    void Apply(Solver* solver, const float dt);

  private:
    pxr::GfVec3f _gravity;
  };
} // namespace PBD

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_FORCE_H
