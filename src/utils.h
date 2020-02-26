#pragma once

#include "default.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
namespace AMN {

  #define RANDOM_0_1 ((float)rand() / (float)RAND_MAX)
  
  #define RANDOM_0_X(HI) ((float)rand() / (float) RAND_MAX * (HI))

  #define RANDOM_LO_HI(LO, HI) ((LO) + (float)rand() / \
    (float)(RAND_MAX / ((HI) - (LO))))

  #define MIN(a,b) (((a)<(b))?(a):(b))
  #define MAX(a,b) (((a)>(b))?(a):(b))
  #define CLAMP(a, min, max) ((a)<(min)?(min):(a)>(max)?(max):(a))
  #define RESCALE(value, inmin, inmax, outmin, outmax) \
    (((value) - (inmin))*((outmax)-(outmin))/((inmax)-(inmin))+(outmin))

  // x=target variable, y=mask
  #define BITMASK_SET(x,y) ((x) |= (y))
  #define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
  #define BITMASK_FLIP(x,y) ((x) ^= (y))
  #define BITMASK_CHECK(x,y) (((x) & (y)) == (y))

  // print vectors (debug)
  void PrintVector(const pxr::GfVec2i& v, const char* t);
  void PrintVector(const pxr::GfVec3f& v, const char* t);
  void PrintVector(const pxr::GfVec4f& v, const char* t);

  // index to random color
  static inline unsigned RandomColorByIndex(unsigned index)
  {
    srand(index);
    return rand();
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

} // namespace AMN
