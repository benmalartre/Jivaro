#ifndef AMN_COMMON_H
#define AMN_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <bitset>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <iostream>
#define AMN_EXPORT extern "C" 

#define DEGREES_TO_RADIANS 0.0174532925f
#define RADIANS_TO_DEGREES 57.2957795f

#define AMN_MAJOR_VERSION 0
#define AMN_MINOR_VERSION 0
#define AMN_PATCH_VERSION 0

#define AMN_VERSION 000

#define AMN_USE_NAMESPACES 1

#if AMN_USE_NAMESPACES

  #define AMN_NS AMN
  #define AMN_INTERNAL_NS amnInternal_v0_00__reserved__
  #define AMN_NS_GLOBAL ::AMN_NS

  namespace AMN_INTERNAL_NS { }

  // The root level namespace for all source in the Amnesia distribution.
  namespace AMN_NS {
      using namespace AMN_INTERNAL_NS;
  }

  #define AMN_NAMESPACE_OPEN_SCOPE   namespace AMN_INTERNAL_NS {
  #define AMN_NAMESPACE_CLOSE_SCOPE  }  
  #define AMN_NAMESPACE_USING_DIRECTIVE using namespace AMN_NS;

#else

  #define AMN_NS 
  #define AMN_NS_GLOBAL 
  #define AMN_NAMESPACE_OPEN_SCOPE   
  #define AMN_NAMESPACE_CLOSE_SCOPE 
  #define AMN_NAMESPACE_USING_DIRECTIVE

#endif // AMN_USE_NAMESPACES

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

#endif // AMN_COMMON_H

