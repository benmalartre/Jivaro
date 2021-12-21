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
#define JVR_EXPORT extern "C" 

#define DEGREES_TO_RADIANS 0.0174532925f
#define RADIANS_TO_DEGREES 57.2957795f

#define JVR_MAJOR_VERSION 0
#define JVR_MINOR_VERSION 0
#define JVR_PATCH_VERSION 0

#define JVR_VERSION 000

#define JVR_USE_NAMESPACES 1

#if JVR_USE_NAMESPACES

  #define JVR_NS JVR
  #define JVR_INTERNAL_NS jvrInternal_v0_00__reserved__
  #define JVR_NS_GLOBAL ::JVR_NS

  namespace JVR_INTERNAL_NS { }

  // The root level namespace for all source in the Jivaro distribution.
  namespace JVR_NS {
      using namespace JVR_INTERNAL_NS;
  }

  #define JVR_NAMESPACE_OPEN_SCOPE   namespace JVR_INTERNAL_NS {
  #define JVR_NAMESPACE_CLOSE_SCOPE  }  
  #define JVR_NAMESPACE_USING_DIRECTIVE using namespace JVR_NS;

#else

  #define JVR_NS 
  #define JVR_NS_GLOBAL 
  #define JVR_NAMESPACE_OPEN_SCOPE   
  #define JVR_NAMESPACE_CLOSE_SCOPE 
  #define JVR_NAMESPACE_USING_DIRECTIVE

#endif // JVR_USE_NAMESPACES

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

