#ifndef JVR_ACCELERATION_GRADIENT_H
#define JVR_ACCELERATION_GRADIENT_H

#include <vector>
#include <pxr/base/tf/hash.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/component.h"
#include "../acceleration/morton.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;

class Gradient
{
public:
  enum Flags {
    SEED,
    FIXED,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
  };
  
  void Init(Mesh* mesh);
  void Compute(Mesh* mesh);
  void SetSeed(pxr::VtArray<int> &seed);
  void SetFixed(pxr::VtArray<int> &seed);

protected:
  void _FindFeatures(Mesh* mesh);
  void _UpdateKroneckerDelta(Mesh* mesh);
  void _SolveCotanLaplace();
  double _ComputeTime(Mesh* mesh);


private:
  pxr::VtArray<int>           _flags;
  pxr::VtArray<pxr::GfVec3f>  _gradient;
  pxr::VtArray<pxr::GfVec3f>  _value;
  pxr::VtArray<pxr::GfVec3f>  _divergence;
  bool                        _flip;

  double                      _avgLength;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_GRADIENT_H
