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
class Xform;
class Mesh;
class Sphere;
class Plane;
class Points;
class Scene;
class BVH;
class Instancer;

void _AddMainDemoLight();
Solver* _GenerateSolver(Scene* scene, pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Points* _GeneratePoints(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Mesh* _GenerateMeshGrid(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  size_t subd=8, const pxr::GfMatrix4d& m=pxr::GfMatrix4d(1.0));

Mesh* _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float spacing=0.025f, const pxr::GfMatrix4d& m=pxr::GfMatrix4d(1.0));

Plane* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Sphere* _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4d& m);

Instancer* _SetupBVHInstancer(pxr::UsdStageRefPtr& stage, pxr::SdfPath& path, BVH* bvh);
void _UpdateBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh, Instancer* instancer);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H