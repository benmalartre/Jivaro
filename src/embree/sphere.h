#pragma once

#include "prim.h"
#include "utils.h"
#include <pxr/usd/usdGeom/sphere.h>

AMN_NAMESPACE_OPEN_SCOPE

class AmnUsdEmbreeContext;

// To be able to have transform as in usd specs, implicit sphere are converted
// to triangles polygonal mesh with smooth per-vertex normals
// A sphere of resolution N will have N+1 longitudes, including extremity
// points, and N*2 latitudes
struct AmnUsdEmbreeSphere  : public AmnUsdEmbreePrim {
  float                       _radius;
  int                         _resolution;
  pxr::VtArray<pxr::GfVec3f>  _positions;
  pxr::VtArray<int>           _triangles;
  pxr::VtArray<int>           _samples;
  pxr::VtArray<pxr::GfVec3f>  _normals;
  pxr::VtArray<pxr::GfVec2f>  _uvs;
};

AmnUsdEmbreeSphere* TranslateSphere( AmnUsdEmbreeContext* ctxt,
                              const pxr::UsdGeomSphere& usdSphere);

void BuildPoints(int num_lats, 
                 int num_longs, 
                 pxr::VtArray<pxr::GfVec3f>& positions,
                 pxr::VtArray<pxr::GfVec3f>& normals,
                 pxr::VtArray<pxr::GfVec2f>& uvs, 
                 float radius,
                 float* worldMatrix);

void BuildTriangles(int num_lats,
                    int num_longs,
                    pxr::VtArray<int>& triangles);

AMN_NAMESPACE_CLOSE_SCOPE