#ifndef JVR_GEOMETRY_SAMPLES_H
#define JVR_GEOMETRY_SAMPLES_H

// https://github.com/zewt/maya-implicit-skinning/blob/master/src/meshes/vcg_lib/utils_sampling.cpp

#include <iostream>
#include <pxr/pxr.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"
#include "../geometry/triangle.h"

JVR_NAMESPACE_OPEN_SCOPE

/// Sample points
struct Sample
{
  GfVec3i  elemIdx;
  int           cellIdx;
  GfVec3f  baryWeights;

  GfVec3f GetPosition(const GfVec3f* positions) const;
  GfVec3f GetNormal(const GfVec3f* normals) const;
  GfVec3f GetTangent(const GfVec3f* positions, const GfVec3f* normals) const;

};

#define DOT_EPSILON 33.f
struct Cell {
  std::vector<Sample> samples;
  int                 firstSampleIdx;
  int                 sampleCnt;
};

/// Sample triangle mesh
/*
void PoissonSampling(
  float radius, int nbSamples,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<int>& triangles,
  VtArray<Sample>& samples);*/

void StochasticSampling(
  int nbSamples,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<Triangle>& triangles,
  VtArray<Sample>& samples);

void PoissonSampling(
  float radius, int nbSamples,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<Triangle>& triangles,
  VtArray<Sample>& samples);

void GridSampling(
  float radius,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<Triangle>& triangles,
  VtArray<Sample>& samples);


JVR_NAMESPACE_CLOSE_SCOPE

#endif