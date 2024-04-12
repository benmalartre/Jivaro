#ifndef JVR_TEST_UTILS_H
#define JVR_TEST_UTILS_H

#include <pxr/base/gf/matrix4d.h> 
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/path.h>

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Solver;
class Mesh;
class Sphere;
class Plane;

Solver* _GenerateSolver(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Plane* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Mesh* _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float size, const pxr::GfMatrix4d& m);

Sphere* _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4d& m);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H