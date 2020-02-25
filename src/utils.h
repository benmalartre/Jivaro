#pragma once

#include "default.h"

namespace AMN {

  #define RANDOM_0_1 ((float)rand() / (float)RAND_MAX)
  
  #define RANDOM_0_X(HI) ((float)rand() / (float) RAND_MAX / (X))

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

} // namespace AMN
