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
#include "../pbd/element.h"
#include "../pbd/particle.h"


JVR_NAMESPACE_OPEN_SCOPE

struct Particles;
class Constraint;
class Force;
class Collision;
class Geometry;
class Points;
class Scene;
class Timer;
class Solver : public Xform {
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  typedef std::map<Element*, std::pair<pxr::SdfPath, Geometry*>> _ElementMap;

  explicit Solver(Scene* scene, const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world);
  ~Solver();
  
  void CreateConstraints(Body* body, short type, float stiffness=10000.f, float damping=0.1f);

  // attributes
  float GetSleepThreshold() { return _sleepThreshold; };
  void SetSleepThreshold(float threshold) { _sleepThreshold = threshold; };
  float GetStartFrame() { return _startFrame; };
  void SetStartFrame(float startFrame) { _startFrame = startFrame; };

  // system
  size_t GetNumParticles() { return _particles.GetNumParticles(); };
  size_t GetNumConstraints() { return _constraints.size(); };
  size_t GetNumForces() { return _force.size(); };
  size_t GetNumCollisions() { return _collisions.size(); };

  // bodies
  std::vector<Body*> GetBodies(){return _bodies;};
  const std::vector<Body*> GetBodies() const {return _bodies;};
  Body* CreateBody(Geometry* geom, const pxr::GfMatrix4f& m, float mass, float radius, float damping);
  void AddBody(Body* body);
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
  void GetConstraintsByType(short type, std::vector<Constraint*>& results);

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
  void LockPoints();
  void WeightBoundaries();
  Points* GetPoints(){return _points;};
  
  // solver 
  void Update(pxr::UsdStageRefPtr& stage, float time);
  void UpdateParameters(pxr::UsdStageRefPtr& stage, float time);
  void UpdateCollisions(pxr::UsdStageRefPtr& stage, float time);
  void UpdateGeometries();
  void Reset();
  void Step();

  // elements
  void AddElement(Element* element, Geometry* geom, const pxr::SdfPath& path);
  void RemoveElement(Element* element);
  pxr::SdfPath GetElementPath(Element* element);
  Geometry* GetElementGeometry(Element* element);
  Element* GetElement(const pxr::SdfPath& path);
  const _ElementMap& GetElements(){return _elements;};


private:
  void _ClearContacts();
  void _FindContacts();
  void _UpdateContacts();
  void _GetContactPositions(pxr::VtArray<pxr::GfVec3f>& positions, 
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors);
  void _SolveConstraints(std::vector<Constraint*>& constraints);
  void _SolveVelocities();

  void _IntegrateParticles(size_t begin, size_t end);
  void _UpdateParticles(size_t begin, size_t end);
  void _StepOne();

  int                                 _subSteps;
  float                               _sleepThreshold;
  float                               _frameTime;
  float                               _stepTime;
  float                               _t;
  float                               _startFrame;
  bool                                _paused;	

  // system
  Particles                           _particles;
  std::vector<Constraint*>            _constraints;
  std::vector<Constraint*>            _contacts;
  std::vector<Collision*>             _collisions;
  std::vector<Body*>                  _bodies;
  std::vector<Force*>                 _force;
  Force*                              _gravity;
  Force*                              _damp;

  // scene
  _ElementMap                         _elements;
  Scene*                              _scene;
  Points*                             _points;
  pxr::SdfPath                        _pointsId;
  pxr::SdfPath                        _solverId;

  // timing
  Timer*                              _timer;

  friend class Particles;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
