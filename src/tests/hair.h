#ifndef JVR_TEST_HAIR_H
#define JVR_TEST_HAIR_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class TestHair : public Execution {
public:
  friend class Scene;
  TestHair() : Execution(){};
  void InitExec(pxr::UsdStageRefPtr& stage) override;
  void UpdateExec(pxr::UsdStageRefPtr& stage, float time) override;
  void TerminateExec(pxr::UsdStageRefPtr& stage) override;

protected:
  void _InitControls(pxr::UsdStageRefPtr& stage);
  void _QueryControls(pxr::UsdStageRefPtr& stage);
  pxr::HdDirtyBits _HairEmit(pxr::UsdStageRefPtr& stage, Curve* curve, 
    pxr::UsdGeomMesh& mesh, pxr::GfMatrix4d& xform, double time);


private:
  pxr::TfHashMap<pxr::SdfPath, _Sources, pxr::SdfPath::Hash> _sourcesMap;
  int _density;
  float _length, _amplitude, _frequency, _width, _scale, _radius;
  pxr::GfVec3f _color;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H