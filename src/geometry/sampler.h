#ifndef JVR_GEOMETRY_SAMPLES_H
#define JVR_GEOMETRY_SAMPLES_H

// https://github.com/zewt/maya-implicit-skinning/blob/master/src/meshes/vcg_lib/utils_sampling.cpp

#include "../common.h"
#include "triangle.h"
#include <iostream>
#include "pxr/pxr.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/gf/vec3i.h"
#include "pxr/base/gf/vec3f.h"

JVR_NAMESPACE_OPEN_SCOPE

/// Sample points
namespace Sampler
{
  #define DOT_EPSILON 33.f
  struct Sample
  {
    pxr::GfVec3i  elemIdx;
    int           cellIdx;
    pxr::GfVec3f  baryWeights;

    pxr::GfVec3f GetPosition(const pxr::GfVec3f* positions) const;
    pxr::GfVec3f GetNormal(const pxr::GfVec3f* normals) const;
    pxr::GfVec3f GetTangent(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals) const;

  };

  struct Cell {
    std::vector<Sample> samples;
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
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif