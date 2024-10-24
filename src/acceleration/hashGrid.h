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

// https://matthias-research.github.io/pages/tenMinutePhysics/11-hashing.pdf

class HashGrid
{
protected:
  typedef int64_t (HashGrid::*HashFunc)(const GfVec3i& intCoords) const;

  // hash from integer coordinates
  inline int64_t _HashCoordsMuller(const GfVec3i& intCoords) const {
    int64_t h = (intCoords[0] * 92837111) ^ (intCoords[1] * 689287499) ^ (intCoords[2] * 283923481);
    return std::abs(h) % _tableSize;
  };
  inline int64_t _HashCoordsPixar(const GfVec3i& intCoords) const {
    int64_t h = TfHash()(intCoords);
    return std::abs(h) % _tableSize;
  };

  // integer coordinates
  inline GfVec3i _IntCoords(const GfVec3f& coords) const{
    return GfVec3i(
      std::floorf(coords[0] * _scl),
      std::floorf(coords[1] * _scl),
      std::floorf(coords[2] * _scl)
    );
  };

public:
  enum HashMethod {
    MULLER,
    PIXAR
  };

  HashGrid(float spacing = 1.f, short hashMethod=MULLER) 
    : _tableSize(0), _spacing(spacing>1e-6f ? spacing : 1e-6f), _scl(1.f/_spacing), _hashMethod(hashMethod) {
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

  void Init(size_t n, const GfVec3f* positions, float radius);
  void Update(const GfVec3f* positions);

  size_t Closests(size_t index, const GfVec3f* positions,
    std::vector<int>& closests, float distance) const;
  size_t Closests(size_t index, const GfVec3f* positions, const GfVec3f* velocities, float ft,
    std::vector<int>& closests, float distance) const;

  void SetSpacing(float spacing) { _spacing = spacing > 1e-6f ? spacing : 1e-6f; _scl = 1.f/_spacing; };
  GfVec3f GetColor(const GfVec3f& point);

private:
  size_t                            _n;
  float                             _spacing;
  float                             _scl;
  size_t                            _tableSize;
  std::vector<int>                  _cellStart;
  std::vector<int>                  _cellEntries;
  short                             _hashMethod;
  HashFunc                          _HashCoords;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_HASHGRID_H
