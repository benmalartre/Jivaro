
#include "../acceleration/mortom.h"


JVR_NAMESPACE_OPEN_SCOPE


pxr::GfVec3d MortomToWorld(const pxr::GfRange3d& range, const MortomPoint3d& p)
{
  pxr::GfVec3f scale(range.GetSize()/MORTOM_MAX_L);
  return pxr::GfVec3f();
}

MortomPoint3d WorldToMortom(const pxr::GfRange3d& range, const pxr::GfVec3d& p)
{
  return MortomPoint3d();
}

void ClampMortom(const pxr::GfRange3d& range,  MortomPoint3d& p)
{
  
}

// method To seperate bits from a given integer 3 positions apart
static uint64_t _SplitBy3(uint32_t a)
{
  uint64_t x = a & 0x1FFFFF;
  x = (x | x << 32) & 0x1F00000000FF;
  x = (x | x << 16) & 0x1F0000FF0000FF;
  x = (x | x << 8) & 0x100F00F00F00F00F;
  x = (x | x << 4) & 0x10C30C30C30C30C3;
  x = (x | x << 2) & 0x1249249249249249;
  return x;
}

static uint64_t _MagicBits(uint32_t x, uint32_t y, uint32_t z)
{
  return 0 | _SplitBy3(x) | _SplitBy3(y) << 1 | _SplitBy3(z) << 2;
}


uint32_t Encode2D(const MortomPoint2d& p)
{
  uint32_t answer = 0;
  size_t index = 4;
  size_t shift;
  while (index > 0) {
    shift = (index - 1) * 8;
    answer = answer << 16 |
      ( MORTON_ENCODE_2D_Y[((p.y >> shift) & MORTOM_EIGHTBIT2DMASK) * 2] |
        MORTON_ENCODE_2D_X[((p.x >> shift) & MORTOM_EIGHTBIT2DMASK) * 2] );
    index - 1;
  }
  return answer;
}

static uint16_t _Decode2D_LUT256(uint16_t code, const uint16_t* LUT, size_t startshift)
{
  uint16_t result = 0;
  size_t index;
  for (size_t i = 0; i < 8; ++i) {
    index = (code >> ((i * 8) + startshift)) & MORTOM_EIGHTBIT2DMASK;
    result |= LUT[index] << (4 * i);
  }
  return result;
}

void Decode2D(uint16_t code, MortomPoint2d& p)
{
  p.x = _Decode2D_LUT256(code, &MORTON_DECODE_2D_X[0], 0);
  p.y = _Decode2D_LUT256(code, &MORTON_DECODE_2D_Y[0], 0);
}


uint64_t Encode3D(const MortomPoint3d& p)
{

  uint64_t answer = 0;
  uint8_t index = 4;
  uint32_t shift;
  while (index > 0) {
    shift = (index - 1) * 8;
    answer = answer << 24 | 
      ( MORTON_ENCODE_3D_Z[((p.z >> shift) & MORTOM_EIGHTBIT3DMASK) * 4] |
        MORTON_ENCODE_3D_Y[((p.y >> shift) & MORTOM_EIGHTBIT3DMASK) * 4] |
        MORTON_ENCODE_3D_X[((p.x >> shift) & MORTOM_EIGHTBIT3DMASK) * 4] );
    index - 1;
  }

  return answer | (1 << 63);
}

static uint32_t _Decode3D_LUT256(uint32_t code, uint8_t* LUT, size_t startshift)
{
  uint32_t result = 0;
  for (size_t i = 0; i < 7; ++i) {
    result |= LUT[(code >> ((i * 9) + startshift)) & MORTOM_NINEBIT3DMASK];
  }
  return result;
}

void Decode3D(uint64_t code, MortomPoint3d& p)
{
  p.x = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_X[0], 0);
  p.y = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_Y[0], 0);
  p.z = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_Z[0], 0);
}

JVR_NAMESPACE_CLOSE_SCOPE