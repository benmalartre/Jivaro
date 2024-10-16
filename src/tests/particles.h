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
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

protected:
  void _AddAnimationSamples(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path);
  void _TraverseStageFindingElements(pxr::UsdStageRefPtr& stage);

private:

  Solver*                   _solver;
  Plane*                    _ground;
  Voxels*                   _voxels;

  pxr::SdfPath              _groundId;
  pxr::SdfPath              _solverId;
  pxr::SdfPath              _voxelsId;

  BVH                       _bvh;
  float                     _lastTime;

  Mesh*                     _emitter;
  pxr::SdfPath              _emitterId;

  std::vector<Geometry*>    _colliders;
  std::vector<pxr::SdfPath> _collidersId;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H