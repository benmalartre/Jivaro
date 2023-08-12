#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H

#include <map>
#include <float.h>

#include <pxr/base/vt/array.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  class Solver;
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
    virtual int& GetTypeId() const = 0;

    virtual bool Init(Solver* solver) { return true; };
    virtual bool Update(Solver* solver) { return true; };
    virtual bool Solve(Solver* solver, const size_t iter) { return true; };

    virtual void ConstrainPositions(float di) {};
    virtual void ConstrainVelocities() {};

    bool Contains(size_t body) {
      return std::find(_bodies.begin(), _bodies.end(), body) != _bodies.end();
    };

  protected:
    pxr::VtArray<int> _bodies;
    pxr::VtArray<int> _particles;
  };

  class DistanceConstraint : public Constraint
  {
  public:
    DistanceConstraint() : Constraint(2), _restLength(0.f), _stretchStiffness(1.f), _compressionStiffness(1.f) {}
    virtual int& GetTypeId() const { return TYPE_ID; }

    virtual bool Init(Solver* solver, const size_t p1, const size_t p2, const float stiffness);
    virtual bool Solve(Solver* solver, const size_t iter);

  protected:
    static int  TYPE_ID;
    float       _restLength;
    float       _stretchStiffness;
    float       _compressionStiffness;
  };
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
