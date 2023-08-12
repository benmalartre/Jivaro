#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H

#include <map>
#include <float.h>

#include <pxr/base/vt/array.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {

  class Particles;
  class Constraint
  {
  public:
    Constraint(const size_t numberOfBodies)
    {
      _bodies.resize(numberOfBodies);
    }

    size_t GetNumBodies() const { return _bodies.size(); }
    size_t GetNumParticles() const { return _particles.size(); };
    virtual ~Constraint() {};
    virtual size_t& GetTypeId() const = 0;

    virtual bool Init(Particles* particles) { return true; };
    virtual bool Update(Particles* particles) { return true; };
    virtual bool Solve(Particles* particles, const size_t iter) { return true; };

    virtual void ConstrainPositions(float di) {};
    virtual void ConstrainVelocities() {};

    bool Contains(size_t body) {
      return std::find(_bodies.begin(), _bodies.end(), body) != _bodies.end();
    };

  protected:
    enum ConstraintType {
      DISTANCE = 1,
      BEND
    };

    pxr::VtArray<int> _bodies;
    pxr::VtArray<int> _particles;
  };

  class DistanceConstraint : public Constraint
  {
  public:
    DistanceConstraint() : Constraint(2), _restLength(0.f), _stretchStiffness(1.f), _compressionStiffness(1.f) {}
    virtual size_t& GetTypeId() const { return TYPE_ID; }

    virtual bool Init(Particles* particles, const size_t p1, const size_t p2, 
      const float stretchStiffnes, const float compressionStiffness);
    virtual bool Solve(Particles* particles, const size_t iter);

  protected:
    static size_t  TYPE_ID;
    float          _restLength;
    float          _stretchStiffness;
    float          _compressionStiffness;
  };
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
