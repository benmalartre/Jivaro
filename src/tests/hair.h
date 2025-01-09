#ifndef JVR_TEST_HAIR_H
#define JVR_TEST_HAIR_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class TestHair : public Execution {
public:
  friend class Scene;
  TestHair() : Execution(){};
  void InitExec(UsdStageRefPtr& stage) override;
  void UpdateExec(UsdStageRefPtr& stage, float time) override;
  void TerminateExec(UsdStageRefPtr& stage) override;

protected:
  void _InitControls(UsdStageRefPtr& stage);
  void _QueryControls(UsdStageRefPtr& stage);
  HdDirtyBits _HairEmit(UsdStageRefPtr& stage, Curve* curve, 
    UsdGeomMesh& mesh, GfMatrix4d& xform, double time);


private:
  TfHashMap<SdfPath, _Sources, SdfPath::Hash> _sourcesMap;
  int _density;
  float _length, _amplitude, _frequency, _width, _scale, _radius;
  GfVec3f _color;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H