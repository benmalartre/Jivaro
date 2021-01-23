#ifndef AMN_APPLICATION_INTERSECTOR_H
#define AMN_APPLICATION_INTERSECTOR_H
#pragma once

#include "../common.h"
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/ray.h>

AMN_NAMESPACE_OPEN_SCOPE

// https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c
template<typename T>
int SolveQuadric(const T c[3], T s[2]);
template<typename T>
int SolveCubic(const T c[4], T s[3]);
template<typename T>
int SolveQuartic(const T c[5], T s[4]);


float DiscIntersection(const pxr::GfRay& ray, const pxr::GfVec3d& n, 
  const pxr::GfVec3d& p, float radius);

float CylinderIntersection(const pxr::GfRay& localRay, float radius, 
  float height);

//float TorusIntersection( const pxr::GfVec3f& ro, const pxr::GfVec3f& rd, 
//  float radius, float section);

float TorusIntersection( const pxr::GfRay& localRay, float radius, 
  float section);

pxr::GfVec3d TorusNormal( const pxr::GfVec3d& pos, float radius, float section);

AMN_NAMESPACE_CLOSE_SCOPE

#endif //AMN_APPLICATION_INTERSECTOR_H