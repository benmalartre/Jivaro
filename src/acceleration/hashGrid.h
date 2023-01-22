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

  void Init(const std::vector<Geometry*>& geometries) override;
  void Update(const std::vector<Geometry*>& geometries) override;
  bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const override {
    return false;
  }
  bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance) const override {
    return false;
  }
  size_t Closests(const pxr::GfVec3f& point, float maxDist, 
    std::vector<int>& closests);

  void SetSpacing(float spacing) { _spacing = spacing; };
  pxr::GfVec3f GetColor(const pxr::GfVec3f& point);

protected:
  // hash from integer coordinates
  inline int64_t _HashCoords(const pxr::GfVec3i& intCoords) {
    std::cout << "hash coords table size : " << _tableSize << std::endl;
    //return pxr::TfHash()(_IntCoords(intCoords)) % _tableSize;
    int64_t h = (intCoords[0] * 92837111) ^ (intCoords[1] * 689287499) ^ (intCoords[2] * 283923481);	// fantasy function
    return std::abs(h) % _tableSize;
  };

  // integer coordinates
  inline pxr::GfVec3i _IntCoords(const pxr::GfVec3f& coords) {
    std::cout << "int coords " << coords << std::endl;
    std::cout << "spacing " << _spacing << std::endl;
    std::cout << "axis x " << (coords[0] / _spacing) << std::endl;
    std::cout << "axis y " << (coords[1] / _spacing) << std::endl;
    std::cout << "axis z " << (coords[2] / _spacing) << std::endl;

    return pxr::GfVec3i(
      std::floorf(coords[0] / _spacing),
      std::floorf(coords[1] / _spacing),
      std::floorf(coords[2] / _spacing)
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
    return static_cast<int32_t>(key & 0xffffffff);
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
