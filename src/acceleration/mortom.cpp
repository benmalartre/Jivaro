
#include "../acceleration/mortom.h"


JVR_NAMESPACE_OPEN_SCOPE


pxr::GfVec3d MortomToWorld(const pxr::GfRange3d& range, const pxr::GfVec3i& p)
{
  const pxr::GfVec3d scale(range.GetSize() / (float)MORTOM_MAX_L);
  const pxr::GfVec3d min(range.GetMin());
  return pxr::GfVec3d(
    scale[0] * p[0] + min[0],
    scale[1] * p[1] + min[1],
    scale[2] * p[2] + min[2]
  );
}

pxr::GfVec3i WorldToMortom(const pxr::GfRange3d& range, const pxr::GfVec3d& p)
{
  const pxr::GfVec3d scale(range.GetSize() / (float)MORTOM_MAX_L);
  const pxr::GfVec3d min(range.GetMin());
  const pxr::GfVec3d invScale(1.0 / scale[0], 1.0 / scale[1], 1.0 / scale[2]);

  pxr::GfVec3i r({
    (int)(invScale[0] * (p[0] - min[0])),
    (int)(invScale[1] * (p[1] - min[1])),
    (int)(invScale[2] * (p[2] - min[2]))
    });
  ClampMortom(r);
  return r;
}

void ClampMortom(pxr::GfVec3i& p)
{
  for (size_t axis = 0; axis < 3; ++axis) {
    if (p[axis] < 0)p[axis] = 0;
    else if (p[axis] > MORTOM_MAX_L) p[axis] = MORTOM_MAX_L;
  }
}

// ENCODING
static uint64_t MORTON_ENCODE_MAGIC_BITS[6] = { 
  0x1fffff, 
  0x1f00000000ffff, 
  0x1f0000ff0000ff, 
  0x100f00f00f00f00f,
  0x10c30c30c30c30c3,
  0x1249249249249249 
};

static inline uint64_t _SplitBy3bits(const uint32_t a) {
  const uint64_t* masks = &MORTON_ENCODE_MAGIC_BITS[0];
  uint64_t x = ((uint64_t)a) & masks[0];
  x = (x | x << 32) & masks[1];
  x = (x | x << 16) & masks[2];
  x = (x | x << 8) & masks[3];
  x = (x | x << 4) & masks[4];
  x = (x | x << 2) & masks[5];
  return x;
}

uint32_t Encode2D(const pxr::GfVec2i& p)
{
  return 0;
}


uint64_t Encode3D(const pxr::GfVec3i& p)
{
  return _SplitBy3bits(p[0]) | (_SplitBy3bits(p[1]) << 1) | (_SplitBy3bits(p[2]) << 2);
}


// DECODING
static uint64_t MORTON_DECODE_MAGIC_BITS[6] = { 
  0x1fffff, 
  0x1f00000000ffff, 
  0x1f0000ff0000ff,
  0x100f00f00f00f00f,
  0x10c30c30c30c30c3, 
  0x1249249249249249 
};

static inline uint32_t _GetThirdBits(const uint64_t m) {
  uint64_t* masks = &MORTON_DECODE_MAGIC_BITS[0];
  uint64_t x = m & masks[5];
  x = (x ^ (x >> 2)) & masks[4];
  x = (x ^ (x >> 4)) & masks[3];
  x = (x ^ (x >> 8)) & masks[2];
  x = (x ^ (x >> 16)) & masks[1];
  x = (x ^ (x >> 32)) & masks[0];
  return x;
}


pxr::GfVec2i Decode2D(uint32_t code)
{
  return pxr::GfVec2i();
}

pxr::GfVec3i Decode3D(uint64_t code)
{
  uint32_t x = _GetThirdBits(code);
  uint32_t y = _GetThirdBits(code >> 1);
  uint32_t z = _GetThirdBits(code >> 2);
  return pxr::GfVec3i(x, y, z);
}

JVR_NAMESPACE_CLOSE_SCOPE