#ifndef JVR_GEOMETRY_VOXELIZE_H
#define JVR_GEOMETRY_VOXELIZE_H

#include <bitset>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/vt/array.h>
#include "../common.h"
#include "../acceleration/bvh.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE

class Points;

class Voxels : public Points {
public:
  Voxels();
  void Init(Points* geometry, float radius);
  void Trace(short axis);
  void Proximity();
  void Build();

  size_t GetNumCells();
  pxr::GfVec3f GetCellPosition(size_t cellIdx);
  BVH* GetTree() { return &_bvh; };
  const BVH* GetTree() const { return &_bvh; };

  float GetRadius() { return _radius; };

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

private:
  void _TraceWork(const size_t begin, const size_t end, short axis);
  void _ProximityWork(size_t begin, size_t end);
  size_t _ComputeFlatIndex(size_t x, size_t y, size_t z, short axis);

  pxr::GfVec3i            _resolution;
  std::vector<uint8_t>    _data;
  Points*                 _geometry;
  BVH                     _bvh;
  float                   _radius;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_VOXELIZE_H