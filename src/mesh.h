#pragma once

#include <embree3/rtcore.h>

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <math/vec3.h>

namespace embree {

  struct Vertex2{float x, y, z, r;};
  struct Triangle2{int v0,v1,v2;};

  struct UsdEmbreeMesh {
    unsigned                    _geomId;
    RTCGeometry                 _mesh;
    pxr::VtArray<pxr::GfVec3f>  _positions;
    pxr::VtArray<int>           _triangles;
  };

  UsdEmbreeMesh* TranslateMesh( RTCDevice& device, 
                                RTCScene& scene, 
                                const pxr::UsdGeomMesh& usdMesh, 
                                double time);

  int TriangulateMesh(const pxr::VtArray<int>& counts, 
                      const pxr::VtArray<int>& indices, 
                      pxr::VtArray<int>& triangles);

} // namespace embree
