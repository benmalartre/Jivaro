#ifndef JVR_TEST_UTILS_H
#define JVR_TEST_UTILS_H

#include <pxr/base/gf/matrix4f.h> 
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/path.h>

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

pxr::UsdPrim _GenerateSolver(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

Geometry* _GenerateCollidePlane(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path);

pxr::UsdPrim _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  float size, const pxr::GfMatrix4f& m);

pxr::UsdPrim _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4f& m);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_PBD_H