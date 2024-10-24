#pragma once

#include "prim.h"
#include <pxr/usd/usdGeom/sphere.h>

JVR_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;

// To be able to have transform as in usd specs, implicit sphere are converted
// to triangles polygonal mesh with smooth per-vertex normals
// A sphere of resolution N will have N+1 longitudes, including extremity
// points, and N*2 latitudes
struct UsdEmbreeCube  : public UsdEmbreePrim {
  float                       _radius;
  int                         _resolution;
  VtArray<GfVec3f>  _vertices;
  VtArray<int>           _triangles;
  VtArray<int>           _samples;
  VtArray<GfVec3f>  _normals;
  VtArray<GfVec2f>  _uvs;
};

UsdEmbreeSphere* 
TranslateSphere(UsdEmbreeContext* ctxt,
                const UsdGeomSphere& usdSphere,
                const GfMatrix4d& worldMatrix,
                RTCScene scene);

void 
DeleteSphere(RTCScene scene, UsdEmbreeSphere* sphere);

void 
BuildPoints(int num_lats, 
            int num_longs, 
            VtArray<GfVec3f>& positions,
            VtArray<GfVec3f>& normals,
            VtArray<GfVec2f>& uvs, 
            float radius,
            float* worldMatrix);

void 
BuildTriangles(int num_lats,
              int num_longs,
              VtArray<int>& triangles);

JVR_NAMESPACE_CLOSE_SCOPE