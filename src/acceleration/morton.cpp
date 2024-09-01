
#include "../acceleration/morton.h"


JVR_NAMESPACE_OPEN_SCOPE


pxr::GfVec3d MortonToWorld(const pxr::GfRange3d& range, const pxr::GfVec3i& p)
{
  const pxr::GfVec3d scale(range.GetSize() / (float)MORTOM_MAX_L);
  const pxr::GfVec3d min(range.GetMin());
  return pxr::GfVec3d(
    scale[0] * p[0] + min[0],
    scale[1] * p[1] + min[1],
    scale[2] * p[2] + min[2]
  );
}

pxr::GfVec3i WorldToMorton(const pxr::GfRange3d& range, const pxr::GfVec3d& p)
{
  const pxr::GfVec3d scale(range.GetSize() / (float)MORTOM_MAX_L);
  const pxr::GfVec3d min(range.GetMin());
  const pxr::GfVec3d invScale(1.0 / scale[0], 1.0 / scale[1], 1.0 / scale[2]);

  pxr::GfVec3i r({
    (int)(invScale[0] * (p[0] - min[0])),
    (int)(invScale[1] * (p[1] - min[1])),
    (int)(invScale[2] * (p[2] - min[2]))
    });
  return MortonClamp(r);
}

pxr::GfVec3i& MortonClamp(pxr::GfVec3i& p)
{
  for (size_t axis = 0; axis < 3; ++axis) {
    if (p[axis] < 0)p[axis] = 0;
    else if (p[axis] > MORTOM_MAX_L) p[axis] = MORTOM_MAX_L;
  }
  return p;
}

// ENCODING
static uint64_t MORTOM_ENCODE_2D_MASK[6] = { 
  0x00000000ffffffff, 
  0x0000ffff0000ffff, 
  0x00ff00ff00ff00ff,
  0x0f0f0f0f0f0f0f0f, 
  0x3333333333333333, 
  0x5555555555555555 
};

static inline uint64_t _SplitBy2bits(const uint32_t a) {
  uint64_t x = a;
  x = (x | (uint64_t)x << 32) & MORTOM_ENCODE_2D_MASK[0];
  x = (x | x << 16) & MORTOM_ENCODE_2D_MASK[1];
  x = (x | x << 8) & MORTOM_ENCODE_2D_MASK[2];
  x = (x | x << 4) & MORTOM_ENCODE_2D_MASK[3];
  x = (x | x << 2) & MORTOM_ENCODE_2D_MASK[4];
  x = (x | x << 1) & MORTOM_ENCODE_2D_MASK[5];
  return x;
}

uint32_t MortonEncode2D(const pxr::GfVec2i& p)
{
  return _SplitBy2bits(p[0]) | (_SplitBy2bits(p[1]) << 1);
}

static uint64_t MORTOM_ENCODE_3D_MASK[6] = { 
  0x00000000001fffff, 
  0x001f00000000ffff, 
  0x001f0000ff0000ff, 
  0x100f00f00f00f00f, 
  0x10c30c30c30c30c3, 
  0x1249249249249249 
};

static inline uint64_t _SplitBy3bits(const uint32_t a) {
  uint64_t x = ((uint64_t)a) & MORTOM_ENCODE_3D_MASK[0];
  x = (x | (uint64_t)x << 32) & MORTOM_ENCODE_3D_MASK[1];
  x = (x | x << 16) & MORTOM_ENCODE_3D_MASK[2];
  x = (x | x << 8) & MORTOM_ENCODE_3D_MASK[3];
  x = (x | x << 4) & MORTOM_ENCODE_3D_MASK[4];
  x = (x | x << 2) & MORTOM_ENCODE_3D_MASK[5];
  return x;
}

uint64_t MortonEncode3D(const pxr::GfVec3i& p)
{
  return _SplitBy3bits(p[0]) | (_SplitBy3bits(p[1]) << 1) | (_SplitBy3bits(p[2]) << 2);
}

// DECODING
static uint64_t MORTOM_DECODE_2D_MASK[6] = { 
  0x00000000ffffffff, 
  0x0000ffff0000ffff, 
  0x00ff00ff00ff00ff, 
  0x0f0f0f0f0f0f0f0f, 
  0x3333333333333333, 
  0x5555555555555555 
};

static inline uint32_t _GetSecondBits(const uint64_t m) {
  uint64_t x = m & MORTOM_DECODE_2D_MASK[5];
  x = (x ^ (x >> 1)) & MORTOM_DECODE_2D_MASK[4];
  x = (x ^ (x >> 2)) & MORTOM_DECODE_2D_MASK[3];
  x = (x ^ (x >> 4)) & MORTOM_DECODE_2D_MASK[2];
  x = (x ^ (x >> 8)) & MORTOM_DECODE_2D_MASK[1];
  x = (x ^ (x >> 16)) & MORTOM_DECODE_2D_MASK[0];
  return x;
}

pxr::GfVec2i MortonDecode2D(uint32_t code)
{
  uint32_t x = _GetSecondBits(code);
  uint32_t y = _GetSecondBits(code >> 1);
  return pxr::GfVec2i(x, y);
}

static uint64_t MORTOM_DECODE_3D_MASK[6] = { 
  0x00000000001fffff, 
  0x001f00000000ffff, 
  0x001f0000ff0000ff, 
  0x100f00f00f00f00f, 
  0x10c30c30c30c30c3, 
  0x1249249249249249 
};


static inline uint32_t _GetThirdBits(const uint64_t m) {
  uint64_t x = m & MORTOM_DECODE_3D_MASK[5];
  x = (x ^ (x >> 2)) & MORTOM_DECODE_3D_MASK[4];
  x = (x ^ (x >> 4)) & MORTOM_DECODE_3D_MASK[3];
  x = (x ^ (x >> 8)) & MORTOM_DECODE_3D_MASK[2];
  x = (x ^ (x >> 16)) & MORTOM_DECODE_3D_MASK[1];
  x = (x ^ (x >> 32)) & MORTOM_DECODE_3D_MASK[0];
  return x;
}

pxr::GfVec3i MortonDecode3D(uint64_t code)
{
  uint32_t x = _GetThirdBits(code);
  uint32_t y = _GetThirdBits(code >> 1);
  uint32_t z = _GetThirdBits(code >> 2);
  return pxr::GfVec3i(x, y, z);
}

uint32_t MortonLeadingZeros(const uint64_t x)
{
  uint32_t u32 = (x >> 32);
  uint32_t result = u32 ? __builtin_clz(u32) : 32;
  if (result == 32) {
    u32 = x & 0xFFFFFFFFUL;
    result += (u32 ? __builtin_clz(u32) : 32);
  }
  return result;
}

uint32_t MortonFindSplit(Morton* mortons, int first, int last)
{
  uint64_t firstCode = mortons[first].code;
  uint64_t lastCode = mortons[last].code;

  if (firstCode == lastCode)
    return (first + last) >> 1;

  uint32_t commonPrefix = MortonLeadingZeros(firstCode ^ lastCode);
  uint32_t split = first;
  uint32_t step = last - first;

  do
  {
    step = (step + 1) >> 1;
    uint32_t newSplit = split + step;

    if (newSplit < last)
    {
      uint64_t splitCode = mortons[newSplit].code;
      uint32_t splitPrefix = MortonLeadingZeros(firstCode ^ splitCode);
      if (splitPrefix > commonPrefix) {
        split = newSplit;
      }
    }
  } while (step > 1);

  return split;
}

uint64_t MortonConstraintPointInBox(uint64_t point, uint64_t bmin, uint64_t bmax)
{
  uint32_t x = _GetThirdBits(point);
  uint32_t y = _GetThirdBits(point >> 1);
  uint32_t z = _GetThirdBits(point >> 2);

  uint32_t minx = _GetThirdBits(bmin);
  uint32_t miny = _GetThirdBits(bmin >> 1);
  uint32_t minz = _GetThirdBits(bmin >> 2);

  uint32_t maxx = _GetThirdBits(bmax);
  uint32_t maxy = _GetThirdBits(bmax >> 1);
  uint32_t maxz = _GetThirdBits(bmax >> 2);

  return MortonEncode3D(pxr::GfVec3i(
    CLAMP(x, minx, maxx),
    CLAMP(y, miny, maxy),
    CLAMP(z, minz, maxz)
  ));
}

bool MortonCheckBoxIntersects(uint64_t pmin, uint64_t pmax, uint64_t bmin, uint64_t bmax)
{
  pxr::GfVec3f p0, p1, b0, b1;

  p0 = MortonDecode3D(pmin);
  p1 = MortonDecode3D(pmax);

  b0 = MortonDecode3D(bmin);
  b1 = MortonDecode3D(bmax);

  return !(pxr::GfRange3f(p0, p1).IsOutside(pxr::GfRange3f(b0, b1)));
}

bool MortonCheckPointInside(uint64_t point, uint64_t bmin, uint64_t bmax)
{
  pxr::GfVec3f p(MortonDecode3D(point));


  pxr::GfVec3f b0(MortonDecode3D(bmin));
  pxr::GfVec3f b1(MortonDecode3D(bmax));

  return pxr::GfRange3f(b0, b1).IsInside(p);
}

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