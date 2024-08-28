#ifndef JVR_ACCELERATION_MORTOM_H
#define JVR_ACCELERATION_MORTOM_H

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

#define MORTOM_LONG_BITS  21
#define MORTOM_MAX_L ((1 << (MORTOM_LONG_BITS)) - 1)

struct Morton {
  uint64_t  code;
  void*     data;

  bool operator <(const Morton& other) const {
    return code < other.code;
  }
  bool operator <=(const Morton& other) const {
    return code <= other.code;
  }
};

// CONVERSION
pxr::GfVec3d MortonToWorld(const pxr::GfRange3d& range, const pxr::GfVec3i& p);
pxr::GfVec3i WorldToMorton(const pxr::GfRange3d& range, const pxr::GfVec3d& p);
void ClampMorton(pxr::GfVec3i& p);

// ENCODING
uint32_t MortonEncode2D(const pxr::GfVec2i& p);
uint64_t MortonEncode3D(const pxr::GfVec3i& p);

// DECODING
pxr::GfVec2i MortonDecode2D(uint32_t code);
pxr::GfVec3i MortonDecode3D(uint64_t code);

uint32_t MortonLeadingZeros(const uint64_t x);
uint32_t MortonFindSplit(Morton* mortoms, int first, int last);
 
JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_MORTOM_H
