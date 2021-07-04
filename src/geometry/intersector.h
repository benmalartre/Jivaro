#ifndef AMN_APPLICATION_INTERSECTOR_H
#define AMN_APPLICATION_INTERSECTOR_H
#pragma once

#include "../common.h"
#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

static pxr::GfPlane AMN_DEFAULT_PLANE(pxr::GfVec3d(0, 1, 0), pxr::GfVec3d(0));

bool IntersectDisc(const pxr::GfRay& localRay, const double radius,
  double* distance);

bool IntersectRing(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance);

bool IntersectCylinder(const pxr::GfRay& localRay, const double radius, 
  const double height, double* distance);

bool IntersectTube(const pxr::GfRay& localRay, const double innerRadius,
  const double outerRadius, const double height, double* distance);

bool IntersectTorus( const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance);

bool IntersectTorusApprox(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance);


AMN_NAMESPACE_CLOSE_SCOPE

#endif //AMN_APPLICATION_INTERSECTOR_H