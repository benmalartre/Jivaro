#ifndef AMN_APPLICATION_INTERSECTOR_H
#define AMN_APPLICATION_INTERSECTOR_H
#pragma once

#include "../common.h"
#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

static pxr::GfPlane AMN_DEFAULT_PLANE(pxr::GfVec3d(0, 1, 0), pxr::GfVec3d(0));

// https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c
template<typename SCALAR>
int SolveQuadric(const SCALAR c[3], SCALAR s[2]);
template<typename SCALAR>
int SolveCubic(const SCALAR c[4], SCALAR s[3]);
template<typename SCALAR>
int SolveQuartic(const SCALAR c[5], SCALAR s[4]);


bool DiscIntersection(const pxr::GfRay& ray, const pxr::GfVec3d& n, 
  const pxr::GfVec3d& p, const double radius, double* distance);

bool RingIntersection(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance);

bool CylinderIntersection(const pxr::GfRay& localRay, const double radius, 
  const double height, double* distance);

//float TorusIntersection( const pxr::GfVec3f& ro, const pxr::GfVec3f& rd, 
//  float radius, float section);

bool TorusIntersection( const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance);

pxr::GfVec3d TorusNormal( const pxr::GfVec3d& pos, const double radius, const double section);

AMN_NAMESPACE_CLOSE_SCOPE

#endif //AMN_APPLICATION_INTERSECTOR_H