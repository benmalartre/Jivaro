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
  uint64_t  minimum;
  uint64_t  maximum;
  size_t    data;

  bool operator <(const Morton& other) const {
    return minimum < other.maximum;
  }
  bool operator <=(const Morton& other) const {
    return minimum <= other.maximum;
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
uint32_t MortonFindSplit(Morton* mortoms, int first, int last);
uint64_t MortonConstraintPointInBox(uint64_t point, uint64_t bmin, uint64_t bmax);

bool MortonCheckBoxIntersects(uint64_t pmin, uint64_t pmax, uint64_t bmin, uint64_t bmax);
bool MortonCheckPointInside(uint64_t point, uint64_t bmin, uint64_t bmax);


uint64_t MortonNegate(uint64_t code)
{
  uint64_t xMask = 0x1249249249249249;
  uint64_t yMask = xMask << 1;
  uint64_t zMask = xMask << 2;

  //invert
  uint64_t d = ~code;
  uint64_t xSum = (d | ~xMask) + 1;
  uint64_t ySum = (d | ~yMask) + 1;
  uint64_t zSum = (d | ~zMask) + 1;

  return (xSum & xMask) | (ySum & yMask) | (zSum & zMask);
}

uint64_t MortonAdd(uint64_t lhs, uint64_t rhs)
{
  //invert sign bits
  uint64_t code1 = lhs ^ 0x7000000000000000;
  uint64_t code2 = rhs ^ 0x7000000000000000;

  uint64_t xMask = 0x1249249249249249;
  uint64_t yMask = xMask << 1;
  uint64_t zMask = xMask << 2;

  uint64_t xSum = (code1 | ~xMask) + (code2 & xMask);
  uint64_t ySum = (code1 | ~yMask) + (code2 & yMask);
  uint64_t zSum = (code1 | ~zMask) + (code2 & zMask);

  uint64_t result = (xSum & xMask) | (ySum & yMask) | (zSum & zMask);
  return result ^ 0x7000000000000000;
}

uint64_t MortonSubtract(uint64_t lhs, uint64_t rhs)
{
  return MortonAdd(lhs, MortonNegate(rhs));
}

uint64_t MortonShiftRight(uint64_t code, int shift)
{
  return (code & 0x0fffffffffffffff) >> (3 * shift) | 0x7000000000000000;
}

uint64_t MortonShiftLeft(uint64_t code, int shift)
{
  return (((code << (3 * shift)) & 0x0fffffffffffffff) | (code & 0x7000000000000000));
}
 
JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_MORTOM_H
