#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <string>
#include <limits>
#include <map>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/tf/callContext.h>
#include <pxr/base/tf/warning.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

#include "../common.h"
#include "../utils/timer.h"
#include "../geometry/implicit.h"
#include "../graph/graph.h"
#include "../pbd/element.h"
#include "../pbd/particle.h"


JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
class Constraint;
class Force;
class GravityForce;
class DampForce;
class Collision;
class Geometry;
class Points;
class Curve;
class Scene;
class Timer;

class Solver : public Xform {
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  typedef std::map<Element*, std::pair<SdfPath, Geometry*>> _ElementMap;

  explicit Solver(Scene* scene, UsdPrim& prim);
  ~Solver();
  
  void CreateConstraints(Body* body, short type, float stiffness=10000.f, float damping=0.1f);

  // attributes
  float GetSleepThreshold() { return _sleepThreshold; };
  void SetSleepThreshold(float threshold) { _sleepThreshold = threshold; };
  float GetStartTime() { return _startTime; };
  void SetStartTime(float startFrame) { _startTime = startFrame; };

  // system
  size_t GetNumParticles() { return _particles.GetNumParticles(); };
  size_t GetNumConstraints() { return _constraints.size(); };
  size_t GetNumForces() { return _forces.size(); };
  size_t GetNumCollisions() { return _collisions.size(); };

  // bodies
  std::vector<Body*> GetBodies(){return _bodies;};
  const std::vector<Body*> GetBodies() const {return _bodies;};
  Body* CreateBody(Geometry* geom, const GfMatrix4d& m, 
    float mass, float radius, float damping, bool attach);
  void AddBody(Body* body);
  void RemoveBody(Geometry* geom);
  Body* GetBody(size_t index);
  Body* GetBody(Geometry* geom);
  size_t GetBodyIndex(Geometry* geom);

  void SetBodyVelocity(Body* body, const GfVec3f& velocity);

  // forces
  void AddForce(Force* force) { _forces.push_back(force); };
  Force* GetForce(size_t idx) { return _forces[idx]; };

  // constraints
  void AddConstraint(Constraint* constraint);
  Constraint* GetConstraint(size_t idx) { return _constraints[idx]; };
  void GetConstraintsByType(short type, std::vector<Constraint*>& results);
  void UpdateConstraintsDisplay();
  void ClearConstraintsDisplay();

  // contacts
  void AddContact(Constraint* contact) { _contacts.push_back(contact); };
  Constraint* GetContact(size_t idx) { return _contacts[idx]; };
  void GetContactByType(short type, std::vector<Constraint*>& results);

  // collisions
  std::vector<Collision*> GetCollisions(){return _collisions;};
  void AddCollision(Collision* collision);
  Collision* GetCollision(size_t idx = 0) { return _collisions[idx]; };
  std::vector<Constraint*>& GetContacts() { return _contacts; };
  const std::vector<Constraint*>& GetContacts() const { return _contacts; };

  // particles
  Particles* GetParticles() { return &_particles; };
  void LockPoints(Body* body, VtArray<int>& elements);
  void AttachPoints(Body* body, VtArray<int>& elements);
  void PinPoints(Body* body, Geometry* target, VtArray<int>& elements);

  void UpdatePointsDisplay();
  void ClearPointsDisplay();
  void WeightBoundaries(Body* body);
  Points* GetPoints(){return _points;};
  SdfPath GetPointsId(){return _pointsId;};
  
  // solver 
  void Update(UsdStageRefPtr& stage, float time);
  void UpdateInputs(UsdStageRefPtr& stage, float time);
  void UpdateParameters(UsdStageRefPtr& stage, float time);
  void UpdateCollisions(UsdStageRefPtr& stage, float time);
  void UpdateGeometries();
  void Reset(UsdStageRefPtr& stage);
  void Step(UsdStageRefPtr& stage, float time);

  // elements
  void AddElement(Element* element, Geometry* geom, const SdfPath& path);
  void RemoveElement(Element* element);
  SdfPath GetElementPath(Element* element);
  Geometry* GetElementGeometry(Element* element);
  Element* GetElement(const SdfPath& path);
  const _ElementMap& GetElements(){return _elements;};

private:
  void _PrepareContacts();
  void _UpdateContacts();

  void _SolveConstraints(std::vector<Constraint*>& constraints);
  void _SolveVelocities(std::vector<Constraint*>& constraints);

  void _IntegrateParticles(size_t begin, size_t end);
  void _UpdateParticles(size_t begin, size_t end);

  int                                 _subSteps;
  float                               _sleepThreshold;
  float                               _startTime;
  float                               _frameTime;
  float                               _stepTime;

  bool                                _initialized;
  bool                                _paused;	

  // system
  Particles                           _particles;
  std::vector<Constraint*>            _constraints; // static
  std::vector<Constraint*>            _contacts;    // dynamic
  std::vector<Collision*>             _collisions;
  Collision*                          _selfCollisions;
  std::vector<Body*>                  _bodies;
  std::vector<Force*>                 _forces;
  GravityForce*                       _gravity;
  DampForce*                          _damp;

  // scene
  _ElementMap                         _elements;
  Scene*                              _scene;
  Points*                             _points;
  Curve*                              _curves;
  SdfPath                             _pointsId;
  SdfPath                             _curvesId;
  SdfPath                             _solverId;

  // display
  bool                                _showPoints;
  bool                                _showConstraints;
  size_t                              _showConstraintsMask;

  // timing
  Timer*                              _timer;

  friend struct Particles;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
