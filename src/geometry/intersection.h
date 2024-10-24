#ifndef JVR_GEOMETRY_INTERSECTION_H
#define JVR_GEOMETRY_INTERSECTION_H

#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE


static GfPlane DEFAULT_PLANE(GfVec3d(0, 1, 0), GfVec3d(0));

bool IntersectDisc(const GfRay& localRay, const double radius,
  double* distance);

bool IntersectRing(const GfRay& localRay, const double radius,
  const double section, double* distance);

bool IntersectCylinder(const GfRay& localRay, const double radius, 
  const double height, double* distance);

bool IntersectTube(const GfRay& localRay, const double innerRadius,
  const double outerRadius, const double height, double* distance);

bool IntersectTorus( const GfRay& localRay, const double radius, 
  const double section, double* distance);

bool IntersectTorusApprox(const GfRay& localRay, const double radius,
  const double section, double* distance);

bool IntersectTriangle(const GfRay& ray, const GfVec3f& a,
  const GfVec3f& b, const GfVec3f& c, double* distance, GfVec3f* uvw);

PXR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GEOMETRY_INTERSECTION_H