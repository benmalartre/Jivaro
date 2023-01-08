#ifndef JVR_GEOMETRY_VOXELIZE_H
#define JVR_GEOMETRY_VOXELIZE_H

#include <bitset>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/vt/array.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Voxels {
public:
  Voxels(Geometry* geometry, float radius);
  void Trace();
  void Build(pxr::VtArray<pxr::GfVec3f>& points);

  size_t GetNumCells();
  pxr::GfVec3f GetCellPosition(size_t cellIdx);

private:
  size_t _ComputeFlatIndex(size_t x, size_t y, size_t z);

  pxr::GfVec3i            _resolution;
  std::vector<uint32_t>   _data;
  Geometry*               _geometry;
  float                   _radius;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_VOXELIZE_H