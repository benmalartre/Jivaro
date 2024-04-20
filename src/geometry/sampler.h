#ifndef JVR_GEOMETRY_SAMPLES_H
#define JVR_GEOMETRY_SAMPLES_H

// https://github.com/zewt/maya-implicit-skinning/blob/master/src/meshes/vcg_lib/utils_sampling.cpp

#include <iostream>
#include <pxr/pxr.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"
#include "../geometry/location.h"
#include "../geometry/triangle.h"

JVR_NAMESPACE_OPEN_SCOPE


#define DOT_EPSILON 33.f
struct Cell {
  std::vector<Location> samples;
  int                 firstSampleIdx;
  int                 sampleCnt;
};

/// Sample triangle mesh
void PoissonSampling(
  float radius, int nbSamples,
  const pxr::VtArray<pxr::GfVec3f>& points,
  const pxr::VtArray<pxr::GfVec3f>& normals,
  const pxr::VtArray<int>& triangles,
  pxr::VtArray<Sample>& samples);

void PoissonSampling(
  float radius, int nbSamples,
  const pxr::VtArray<pxr::GfVec3f>& points,
  const pxr::VtArray<pxr::GfVec3f>& normals,
  const pxr::VtArray<Triangle>& triangles,
  pxr::VtArray<Sample>& samples);

JVR_NAMESPACE_CLOSE_SCOPE

#endif