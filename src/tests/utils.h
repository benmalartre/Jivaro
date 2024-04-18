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
class Points;
class Scene;
class BVH;


void _AddMainDemoLight();
Solver* _GenerateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Points* _GeneratePoints(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Mesh* _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float size, const pxr::GfMatrix4d& m);

Plane* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Sphere* _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4d& m);

void _SetupBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H