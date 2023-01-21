#ifndef JVR_ACCELERATION_HASHGRID_H
#define JVR_ACCELERATION_HASHGRID_H

#include <vector>
#include <pxr/base/tf/hash.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/component.h"
#include "../acceleration/mortom.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

#define HASH_GRID_MIN_SPACING 0.0000001f

class HashGrid : public Intersector
{
public:
  HashGrid(float spacing = 1.f) : _tableSize(0), _spacing(spacing) {};

  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update(const std::vector<Geometry*>& geometries) override;
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const override = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance) const override = 0;
  virtual size_t Closests(const pxr::GfVec3f& point, float maxDist, 
    std::vector<int>& closests);

  void SetSpacing(float spacing) { _spacing = spacing; };

protected:
  // hash from integer coordinates
  inline int64_t _HashCoords(const pxr::GfVec3i& intCoords) {
    return pxr::TfHash()(_IntCoords(intCoords)) % _tableSize;
  };

  // integer coordinates
  inline pxr::GfVec3i _IntCoords(const pxr::GfVec3f& coords) {
    return pxr::GfVec3i(
      pxr::GfFloor(coords[0] / _spacing),
      pxr::GfFloor(coords[1] / _spacing),
      pxr::GfFloor(coords[2] / _spacing)
    );
  };

  // hash from element position
  inline int64_t _HashPos(const pxr::GfVec3f& pos) {
    return _HashCoords(_IntCoords(pos));
  }

  // get element position
  const pxr::GfVec3f& _GetPoint(size_t elemIdx);

  // element mapping (geometry index, point index)
  inline int64_t _ComputeElementKey(int32_t meshIdx, int32_t pointIdx) {
    return (static_cast<int64_t>(meshIdx) << 32) | pointIdx;
  }

  inline int32_t _GetGeometryIndexFromElementKey(int64_t key) {
    return static_cast<int32_t>(key >> 32);
  }

  inline int32_t _GetPointIndexFromElementKey(int64_t key) {
    return key & 0xffffffff;
  }

private:
  std::vector<Geometry*>            _geometries;
  std::vector<const pxr::GfVec3f*>  _points;
  std::vector<uint64_t>             _mapping;
  float                             _spacing;
  size_t                            _tableSize;
  std::vector<int>                  _cellStart;
  std::vector<int>                  _cellEntries;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_HASHGRID_H
