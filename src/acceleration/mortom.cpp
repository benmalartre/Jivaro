
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

uint32_t Encode2D(const pxr::GfVec2i& p)
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

uint64_t Encode3D(const pxr::GfVec3i& p)
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

pxr::GfVec2i Decode2D(uint32_t code)
{
  return pxr::GfVec2i();
}

static uint64_t MORTOM_DECODE_3D_MASH[6] = { 
  0x00000000001fffff, 
  0x001f00000000ffff, 
  0x001f0000ff0000ff, 
  0x100f00f00f00f00f, 
  0x10c30c30c30c30c3, 
  0x1249249249249249 
};


static inline uint32_t _GetThirdBits(const uint64_t m) {
  uint64_t x = m & MORTOM_DECODE_3D_MASH[5];
  x = (x ^ (x >> 2)) & MORTOM_DECODE_3D_MASH[4];
  x = (x ^ (x >> 4)) & MORTOM_DECODE_3D_MASH[3];
  x = (x ^ (x >> 8)) & MORTOM_DECODE_3D_MASH[2];
  x = (x ^ (x >> 16)) & MORTOM_DECODE_3D_MASH[1];
  x = (x ^ (x >> 32)) & MORTOM_DECODE_3D_MASH[0];
  return x;
}

pxr::GfVec3i Decode3D(uint64_t code)
{
  uint32_t x = _GetThirdBits(code);
  uint32_t y = _GetThirdBits(code >> 1);
  uint32_t z = _GetThirdBits(code >> 2);
  return pxr::GfVec3i(x, y, z);
}

JVR_NAMESPACE_CLOSE_SCOPE