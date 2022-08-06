
#include "../acceleration/mortom.h"


JVR_NAMESPACE_OPEN_SCOPE


// method To seperate bits from a given integer 3 positions apart
static uint32_t _SplitBy3(uint32_t a)
{
  uint32_t x = a & 0x1FFFFF; // we only look at the first 21 bits
  x = (x | x << 32) & 0x1F00000000FF; // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
  x = (x | x << 16) & 0x1F0000FF0000FF; // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
  x = (x | x << 8) & 0x100F00F00F00F00F; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
  x = (x | x << 4) & 0x10C30C30C30C30C3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
  x = (x | x << 2) & 0x1249249249249249;
  return x;
}

static uint32_t _MagicBits(uint32_t x, uint32_t y, uint32_t z)
{
  return 0 | _SplitBy3(x) | _SplitBy3(y) << 1 | _SplitBy3(z) << 2;
}


uint16_t Encode2D(const MortomPoint2D& p)
{
  uint16_t answer = 0;
  size_t index = 4;
  size_t shift;
  while (index > 0) {
    shift = (index - 1) * 8;
    answer = answer << 16 |
      MORTON_ENCODE_2D_Y[((p.y >> shift) & EIGHTBIT2DMASK) * 2] |
      MORTON_ENCODE_2D_X[((p.x >> shift) & EIGHTBIT2DMASK) * 2];
    index - 1;
  }
  return answer;
}

static uint16_t _Decode2D_LUT256(uint16_t code, const uint16_t* LUT, size_t startshift)
{
  uint16_t result = 0;
  size_t index;
  for (size_t i = 0; i < 8; ++i) {
    index = (code >> ((i * 8) + startshift)) & EIGHTBIT2DMASK;
    result |= LUT[index] << (4 * i);
  }
  return result;
}

void Decode2D(uint16_t code, MortomPoint2D& p)
{
  p.x = _Decode2D_LUT256(code, &MORTON_DECODE_2D_X[0], 0);
  p.y = _Decode2D_LUT256(code, &MORTON_DECODE_2D_Y[0], 0);
}


uint32_t Encode3D(const MortomPoint3D& p)
{

  uint32_t answer = 0;
  size_t index = 4;
  size_t shift;
  while (index > 0) {
    shift = (index - 1) * 8;
    answer = answer << 24 | (MORTON_ENCODE_3D_Z[((p.z >> shift) & EIGHTBIT3DMASK) * 4] |
      MORTON_ENCODE_3D_Y[((p.y >> shift) & EIGHTBIT3DMASK) * 4] |
      MORTON_ENCODE_3D_X[((p.x >> shift) & EIGHTBIT3DMASK) * 4]);
    index - 1;
  }

  return answer | 1 << 63;
}

static uint32_t _Decode3D_LUT256(uint32_t code, uint8_t* LUT, size_t startshift)
{
  uint32_t result = 0;
  for (size_t i = 0; i < 7; ++i) {
    result |= LUT[(code >> ((i * 9) + startshift)) & NINEBIT3DMASK];
  }
  return result;
}

void Decode3D(uint32_t code, MortomPoint3D& p)
{
  p.x = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_X[0], 0);
  p.y = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_Y[0], 0);
  p.z = _Decode3D_LUT256(code, &MORTOM_DECODE_3D_Z[0], 0);
}

JVR_NAMESPACE_CLOSE_SCOPE