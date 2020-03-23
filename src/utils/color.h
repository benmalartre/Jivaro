#pragma once

#include "../default.h"

AMN_NAMESPACE_OPEN_SCOPE

// index to random color
static inline unsigned RandomColorByIndex(unsigned index)
{
  srand(index);
  return rand();
}

static inline int PackColor(float r, float g, float b, float a)
{
  return 0;
}

static inline pxr::GfVec4f UnpackColor(uint32_t packed)
{
  uint8_t alpha = (packed & 0xFF000000) >> 24;
  uint8_t blue = (packed & 0x00FF0000) >> 16;
  uint8_t green = (packed & 0x0000FF00) >> 8;
  uint8_t red = (packed & 0x000000FF);

  return pxr::GfVec4f(
    (float)red/255.f, 
    (float)green/255.f, 
    (float)blue/255.f, 
    (float)alpha/255.f
  );
}

// color for procedural GL textures
union GLColor 
{
  int32_t  packed;
  struct {
    char r;
    char g;
    char b;
    char a;
  } components;
};

// pack & unpack color
//------------------------------------------------------------------------------
static int PackColor(const pxr::GfVec3f& c)
{
  int code = 0;
  code |= (((int)255 * 255) & 255) << 24;
  code |= (((int)c[0] * 255) & 255) << 16;
  code |= (((int)c[1] * 255) & 255) << 8;
  code |= (((int)c[2] * 255) & 255);
  return code;
}

static float PackColorAsFloat(const pxr::GfVec3f& c)
{
  int code = PackColor(c);
  return *(float*)&code;
}

static pxr::GfVec3f UnpackColor(const int& code)
{
  int r = (code >> 16) & 255;
  int g = (code >> 8) & 255;
  int b = code & 255;
  return pxr::GfVec3f(
    (float)r/255.f,
    (float)g/255.f,
    (float)b/255.f
  );
}

static pxr::GfVec3f UnpackColorAsFloat(const float& code)
{
  int icode = *(int*)&code;
  return UnpackColor(icode);
}

AMN_NAMESPACE_CLOSE_SCOPE