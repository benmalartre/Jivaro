#ifndef JVR_GEOMETRY_INTERSECTION_H
#define JVR_GEOMETRY_INTERSECTION_H

#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE


static pxr::GfPlane DEFAULT_PLANE(pxr::GfVec3d(0, 1, 0), pxr::GfVec3d(0));

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

bool IntersectTriangle(const pxr::GfRay& ray, const pxr::GfVec3f& a,
  const pxr::GfVec3f& b, const pxr::GfVec3f& c, double* distance, pxr::GfVec3f* uvw);

PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_INTERSECTION_H