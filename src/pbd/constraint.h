#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H

#include <map>
#include <float.h>

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Constraint
{
public:
  static const int INVALID_INDEX = std::numeric_limits<int>::max();

  Constraint(const size_t numberOfParticles)
  {
    _particles.resize(numberOfParticles);
    _corrections.resize(numberOfParticles);
  }

  size_t GetNumParticles() const { return _particles.size(); };
  virtual ~Constraint() {};
  virtual size_t& GetTypeId() const = 0;

  virtual bool Init(Particles* particles) { return true; };
  virtual bool Update(Particles* particles) { return true; };
  virtual bool Solve(Particles* particles, const size_t iter) = 0;
  virtual void Apply(Particles* particles, const size_t index) = 0;

  pxr::VtArray<int>& GetParticles() {return _particles;};
  int* GetParticlesPtr() {return &_particles[0];};
  const int* GetParticlesCPtr() const {return &_particles[0];};

protected:
  inline size_t _GetParticleIndex(size_t index) {
    for (size_t p = 0; p < _particles.size(); ++p)
      if (_particles[p] == index) return p;
    return INVALID_INDEX;
  }
  enum ConstraintType {
    DISTANCE = 1,
    BEND
  };

  pxr::VtArray<int>          _particles;
  pxr::VtArray<pxr::GfVec3f> _corrections;
};

class DistanceConstraint : public Constraint
{
public:
  DistanceConstraint() : Constraint(2), _restLength(0.f), _stretchStiffness(1.f), _compressionStiffness(1.f) {}
  virtual size_t& GetTypeId() const { return TYPE_ID; }

  bool Init(Particles* particles, const size_t p1, const size_t p2, 
    const float stretchStiffnes, const float compressionStiffness);
  bool Solve(Particles* particles, const size_t iter);
  void Apply(Particles* particles, const size_t index) override;

protected:
  static size_t  TYPE_ID;
  float          _restLength;
  float          _stretchStiffness;
  float          _compressionStiffness;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
