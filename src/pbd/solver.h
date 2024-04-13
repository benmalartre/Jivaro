#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <limits>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

// Helpers for benchmark time inside the solver
class _Timer{
  struct _Ts {
    void Start() { t = CurrentTime();}
    void End() {accum += CurrentTime() - t;num++;};
    void Reset() {accum = 0;num = 0;};
    double Average() {return num ? ((double)accum * 1e-9) / (double)num : 0;}
    double Elapsed() {return (double)accum * 1e-9;}

    uint64_t t;
    uint64_t accum;
    size_t   num;
  };

  void Init(size_t n, const char** names);
  void Start(size_t index=0);
  void Next();
  void Stop();
  void Update();
  void Log();

private:
  bool                      _rec;
  size_t                    _n;
  size_t                    _c

  std::vector<std::string>  _names;
  std::vector<Ts>           _timers;
  std::vector<double>       _accums;
  std::vector<double>       _avgs;
};


void _Timer::Init(size_t numTimes, const char** names)
{
  _n = numTimes;
  _c = 0;
  _timers.resize(_n, {0,0,0});
  _accums.resize(_n, 0.0);
  _avgs.resize(_n, 0.0);
  _names.resize(_n);
  for(size_t t=0; t< _n; ++t)
    _names[t] = names[t];
}

void _Timer::Start(size_t index) 
{ 
  _c = index;
  _timers[_c].Start(); 
  _rec = true;
}

void _Timer::Next() 
{ 
  if(_rec)_timers[_c++].End(); 
  if(_c >= _n)_c = 0;
  _timers[_c].Start();
}

void _Timer::Stop() 
{ 
  if(_rec=true) _timers[_c].End(); 
  _rec = false; 
}

void _Timer::Update() {
  for (size_t t = 0; t < _numTimes; ++t) {
    _accums[t] += _timers[t].Elapsed();
    _avgs[t] = (_avgs[t] + _timers[t].Average()) / 2.0;
    _timers[t].Reset();
  }
}

void _Timer::Log(const char** names){
  for (size_t t = 0; t < _numTimes; ++t) {
    std::cout << "## " << _names[t] << ": ";
    std::cout << "  - accum : " << _accums[t];
    std::cout << "  - avg : " << _avgs[t] << std::endl;
  }
}

struct Particles;
class Constraint;
class Force;
class Collision;
class Solver {
public:
  const static size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

  Solver(const pxr::UsdPrim& prim);
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
  void UpdateParameters(double time);
  void UpdateCollisions();
  void UpdateGeometries();
  void Reset();
  void Step(bool serial=false);

private:
  void _ClearContacts();
  void _FindContacts(bool serial = false);
  void _SolveConstraints(pxr::VtArray<Constraint*>& constraints, bool serial=false);
  void _SolveVelocities();

  void _IntegrateParticles(size_t begin, size_t end);
  void _UpdateParticles(size_t begin, size_t end);
  void _StepOneSerial();
  void _StepOne();

  size_t                              _subSteps;
  float                               _sleepThreshold;
  float                               _frameTime;
  float                               _stepTime;
  float                               _startFrame;
  bool                                _paused;		
  pxr::UsdPrim                        _prim;

  // system
  Particles                           _particles;
  pxr::VtArray<Constraint*>           _constraints;
  pxr::VtArray<Constraint*>           _contacts;
  pxr::VtArray<Collision*>            _collisions;
  pxr::VtArray<Body*>                 _bodies;
  pxr::VtArray<Force*>                _force;

  // timing
  _Timer                              _timer;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_SOLVER_H
