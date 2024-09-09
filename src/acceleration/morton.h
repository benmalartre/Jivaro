#ifndef JVR_ACCELERATION_MORTON_H
#define JVR_ACCELERATION_MORTON_H

#include <vector>
#include "../common.h"
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3i.h>


JVR_NAMESPACE_OPEN_SCOPE

#ifdef WIN32
#include <intrin.h>
static uint32_t __inline __builtin_clz(uint32_t x) {
  unsigned long r = 0;
  _BitScanReverse(&r, x);
  return (31 - r);
}
#endif

#define MORTON_LONG_BITS  21
#define MORTON_MAX_L ((1 << (MORTON_LONG_BITS)) - 1)
#define MORTON_INVALID_INDEX std::numeric_limits<uint64_t>::max()

struct Morton {
  uint64_t  code;
  uint64_t  minimum;
  uint64_t  maximum;
  size_t    data;

  bool operator <(const Morton& other) const {
    return code < other.code;
  }
  bool operator <=(const Morton& other) const {
    return code <= other.code;
  }
};

// Conversion
pxr::GfVec3d MortonToWorld(const pxr::GfRange3d& range, const pxr::GfVec3i& p);
pxr::GfVec3i WorldToMorton(const pxr::GfRange3d& range, const pxr::GfVec3d& p);

// Encoding
uint32_t MortonEncode2D(const pxr::GfVec2i& p);
uint64_t MortonEncode3D(const pxr::GfVec3i& p);

// Decoding
pxr::GfVec2i MortonDecode2D(uint32_t code);
pxr::GfVec3i MortonDecode3D(uint64_t code);

// Utils
pxr::GfVec3i& MortonClamp(pxr::GfVec3i& p);
uint32_t MortonLeadingZeros(const uint64_t x);
uint32_t MortonFindSplit(const Morton* mortons, int first, int last);
uint32_t MortonLowerBound(const Morton* mortons, int first, int last, uint64_t code);
uint32_t MortonUpperBound(const Morton* mortons, int first, int last, uint64_t code);

pxr::GfVec3f MortonColor(const pxr::GfRange3d &range, const pxr::GfVec3d &p);
pxr::GfVec3f MortonColor(const Morton& morton);

uint64_t MortonBigMin( uint64_t zval, uint64_t minimum, uint64_t maximum);
uint64_t MortonLitMax( uint64_t zval, uint64_t minimum, uint64_t maximum);


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_MORTON_H
