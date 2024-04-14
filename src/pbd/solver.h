#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <string>
#include <limits>
#include <map>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

#include "../common.h"
#include "../pbd/particle.h"
#include "../geometry/implicit.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
class Constraint;
class Force;
class Collision;
class Geometry;
class _Timer;
class Solver : public Xform {
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
  Solver(const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world);
  ~Solver();
  

  void AddConstraints(Body* body);

  // attributes
  float GetSleepThreshold() { return _sleepThreshold; };
  void SetSleepThreshold(float threshold) { _sleepThreshold = threshold; };

  // system
  size_t GetNumParticles() { return _particles.GetNumParticles(); };
  size_t GetNumConstraints() { return _constraints.size(); };
  size_t GetNumForces() { return _force.size(); };
  size_t GetNumCollisions() { return _collisions.size(); };

  // bodies
  pxr::VtArray<Body*> GetBodies(){return _bodies;};
  const pxr::VtArray<Body*> GetBodies() const {return _bodies;};
  Body* AddBody(Geometry* geom, const pxr::GfMatrix4f& m, float mass);
  void RemoveBody(Geometry* geom);
  Body* GetBody(size_t index);
  Body* GetBody(Geometry* geom);
  size_t GetBodyIndex(Geometry* geom);

  // forces
  void AddForce(Force* force) { _force.push_back(force); };
  Force* GetForce(size_t idx) { return _force[idx]; };

  // constraints
  void AddConstraint(Constraint* constraint) { _constraints.push_back(constraint); };
  Constraint* GetConstraint(size_t idx) { return _constraints[idx]; };
  void GetConstraintsByType(short type, pxr::VtArray<Constraint*>& results);

  // collisions
  void AddCollision(Collision* collision) { _collisions.push_back(collision); };
  Collision* GetCollision(size_t idx = 0) { return _collisions[idx]; };
  pxr::VtArray<Constraint*>& GetContacts() { return _contacts; };
  const pxr::VtArray<Constraint*>& GetContacts() const { return _contacts; };

  // particles
  Particles* GetParticles() { return &_particles; };
  void LockPoints();
  void WeightBoundaries();
  
  // solver 
  void Update(pxr::UsdStageRefPtr& stage, float time);
  void UpdateParameters(pxr::UsdStageRefPtr& stage, float time);
  void UpdateCollisions(pxr::UsdStageRefPtr& stage, float time);
  void UpdateGeometries();
  void Reset();
  void Step();

  // childrens
  void AddChild(Geometry* geom, const pxr::SdfPath& path);
  void RemoveChild(Geometry* geometry);
  pxr::SdfPath GetChild(Geometry* geom);

private:
  void _ClearContacts();
  void _FindContacts();
  void _SolveConstraints(pxr::VtArray<Constraint*>& constraints);
  void _SolveVelocities();

  void _IntegrateParticles(size_t begin, size_t end);
  void _UpdateParticles(size_t begin, size_t end);
  void _StepOneSerial();
  void _StepOne();

  int                                 _subSteps;
  float                               _sleepThreshold;
  float                               _frameTime;
  float                               _stepTime;
  float                               _startFrame;
  bool                                _paused;	
  bool                                _serial;	
  Geometry*                           _geom;

  // system
  Particles                           _particles;
  pxr::VtArray<Constraint*>           _constraints;
  pxr::VtArray<Constraint*>           _contacts;
  pxr::VtArray<Collision*>            _collisions;
  pxr::VtArray<Body*>                 _bodies;
  pxr::VtArray<Force*>                _force;
  std::map<void*, Geometry*>          _childrens;

  // timing
  _Timer*                             _timer;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
