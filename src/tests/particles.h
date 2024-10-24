#ifndef JVR_TEST_PARTICLES_H
#define JVR_TEST_PARTICLES_H

#include "../acceleration/bvh.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Plane;
class Points;
class Sphere;
class Particles;

class TestParticles : public Execution {
public:
  friend class Scene;
  TestParticles() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;

protected:
  void _AddAnimationSamples(UsdStageRefPtr& stage, SdfPath& path);
  void _TraverseStageFindingElements(UsdStageRefPtr& stage);

private:

  Solver*                   _solver;
  Plane*                    _ground;
  Voxels*                   _voxels;

  SdfPath              _groundId;
  SdfPath              _solverId;
  SdfPath              _voxelsId;

  BVH                       _bvh;

  Mesh*                     _emitter;
  SdfPath              _emitterId;

  std::vector<Geometry*>    _colliders;
  std::vector<SdfPath> _collidersId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H