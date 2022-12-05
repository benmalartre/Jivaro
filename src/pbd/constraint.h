#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H



#include <map>
#include <float.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../geometry/geometry.h"


JVR_NAMESPACE_OPEN_SCOPE

class PBDSolver;
class PBDConstraint
{
public:
  PBDConstraint(const unsigned int numberOfBodies)
  {
    _bodies.resize(numberOfBodies);
  }

  unsigned int GetNumBodies() const { return static_cast<unsigned int>(_bodies.size()); }
  virtual ~PBDConstraint() {};
  virtual int& GetTypeId() const = 0;

  virtual bool Init(PBDSolver* solver) { return true; };
  virtual bool Update(PBDSolver* solver) { return true; };
  virtual bool Solve(PBDSolver* solver, const unsigned int iter) { return true; };

protected:
  std::vector<unsigned int> _bodies;
};

class PBDDistanceConstraint : public PBDConstraint
{
public:
  PBDDistanceConstraint() : PBDConstraint(2) {}
  virtual int& GetTypeId() const { return TYPE_ID; }

  virtual bool Init(PBDSolver* solver, const unsigned int p1, const unsigned int p2, const float stiffness);
  virtual bool Solve(PBDSolver* solver, const unsigned int iter);

protected:
  static int  TYPE_ID;
  float       _restLength;
  float       _stiffness;
};

class PBDRestoreConstraint : public PBDConstraint
{
public:
  PBDRestoreConstraint() : PBDConstraint(2) {}
  virtual int& GetTypeId() const { return TYPE_ID; }

  virtual bool Init(PBDSolver* solver, const unsigned int p1, const float stiffness);
  virtual bool Solve(PBDSolver* solver, const unsigned int iter);

protected:
  static int  TYPE_ID;
  float       _stiffness;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
