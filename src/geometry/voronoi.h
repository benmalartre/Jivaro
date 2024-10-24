#ifndef JVR_GEOMETRY_VORONOI_H
#define JVR_GEOMETRY_VORONOI_H

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/stage.h>
#include <vector>
#include <set>
#include "../common.h"
#include "mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

class Voronoi2D {
  struct Site_t {
    GfVec3f seed;
    int id;
    std::set<Site_t*> neighbors;
  };

  void Build(UsdStageRefPtr& stage, const VtArray<GfVec3f>& seeds);
  int GetNumSurroundingSites(size_t x, size_t y);
private:
  VtArray<Site_t>          _sites;
  VtArray<GfVec3f>    _points;
  VtArray<int>             _faceVertexCounts;
  VtArray<int>             _faceVertexIndices;

  std::vector<int>              _cells;
  size_t                        _resolutionX;
  size_t                        _resolutionY;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_VDB_H