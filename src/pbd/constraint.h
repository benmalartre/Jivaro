#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H



#include <map>
#include <float.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/geometry.h"


JVR_NAMESPACE_OPEN_SCOPE

namespace PBD {
  class Solver;
  class Constraint
  {
  public:
    Constraint(const unsigned int numberOfBodies)
    {
      _bodies.resize(numberOfBodies);
    }

    unsigned int GetNumBodies() const { return static_cast<unsigned int>(_bodies.size()); }
    virtual ~Constraint() {};
    virtual int& GetTypeId() const = 0;

    virtual bool Init(Solver* solver) { return true; };
    virtual bool Update(Solver* solver) { return true; };
    virtual bool Solve(Solver* solver, const unsigned int iter) { return true; };

    bool Contains(unsigned int body) {
      return std::find(_bodies.begin(), _bodies.end(), body) != _bodies.end();
    };

  protected:
    std::vector<unsigned int> _bodies;
  };

  class DistanceConstraint : public Constraint
  {
  public:
    DistanceConstraint() : Constraint(2), _restLength(0.f), _stiffness(1.f) {}
    virtual int& GetTypeId() const { return TYPE_ID; }

    virtual bool Init(Solver* solver, const unsigned int p1, const unsigned int p2, const float stiffness);
    virtual bool Solve(Solver* solver, const unsigned int iter);

  protected:
    static int  TYPE_ID;
    float       _restLength;
    float       _stiffness;
  };

  class RestoreConstraint : public Constraint
  {
  public:
    RestoreConstraint() : Constraint(2), _stiffness(1.f) {}
    virtual int& GetTypeId() const { return TYPE_ID; }

    virtual bool Init(Solver* solver, const unsigned int p1, const float stiffness);
    virtual bool Solve(Solver* solver, const unsigned int iter);

  protected:
    static int  TYPE_ID;
    float       _stiffness;
  };
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
