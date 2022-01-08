#ifndef JVR_COMMON_H
#define JVR_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <bitset>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <iostream>
#include <pxr/pxr.h>
#include <pxr/base/arch/defines.h>

#define JVR_EXPORT extern "C" 

#define DEGREES_TO_RADIANS 0.0174532925f
#define RADIANS_TO_DEGREES 57.2957795f

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
#define BITMASK_CHECK(x,y) ((x) & (y))

constexpr const char * DecimalPrecision = "%.5f";

#endif // JVR_COMMON_H

