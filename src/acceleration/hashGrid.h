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
#include "../acceleration/morton.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

#define HASH_GRID_MIN_SPACING 0.0000001f

class HashGrid
{
protected:
  typedef int64_t (HashGrid::*HashFunc)(const pxr::GfVec3i& intCoords);

  // hash from integer coordinates
  inline int64_t _HashCoordsMuller(const pxr::GfVec3i& intCoords) {
    int64_t h = (intCoords[0] * 92837111) ^ (intCoords[1] * 689287499) ^ (intCoords[2] * 283923481);
    return std::abs(h) % _tableSize;
  };
  inline int64_t _HashCoordsPixar(const pxr::GfVec3i& intCoords) {
    int64_t h = pxr::TfHash()(intCoords);
    return std::abs(h) % _tableSize;
  };

  // integer coordinates
  inline pxr::GfVec3i _IntCoords(const pxr::GfVec3f& coords) {
    return pxr::GfVec3i(
      std::floorf(coords[0] / _spacing),
      std::floorf(coords[1] / _spacing),
      std::floorf(coords[2] / _spacing)
    );
  };

public:
  enum HashMethod {
    MULLER,
    PIXAR
  };

  HashGrid(float spacing = 1.f, short hashMethod=PIXAR) 
    : _tableSize(0), _spacing(spacing), _hashMethod(hashMethod) {
    switch (_hashMethod) {
    case MULLER:
      _HashCoords = &HashGrid::_HashCoordsMuller;
      break;
    case PIXAR:
    default:
      _HashCoords = &HashGrid::_HashCoordsPixar;
      break;
    }
  };

   void Init(size_t n, const pxr::GfVec3f* positions, float radius);

  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance) const  {    return false;}
  size_t Closests(size_t index, const pxr::GfVec3f* positions,
    std::vector<int>& closests);

  void SetSpacing(float spacing) { _spacing = spacing; };
  pxr::GfVec3f GetColor(const pxr::GfVec3f& point);

protected:
  void _ClosestsFromHash(size_t index, const pxr::GfVec3f* positions, 
    int64_t hash, std::vector<int>& neighbors);

private:
size_t                              _n;
  float                             _spacing;
  size_t                            _tableSize;
  std::vector<int>                  _cellStart;
  std::vector<int>                  _cellEntries;
  short                             _hashMethod;
  HashFunc                          _HashCoords;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_HASHGRID_H
