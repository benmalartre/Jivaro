
#ifndef JVR_GEOMETRY_PERLIN_H
#define JVR_GEOMETRY_PERLIN_H

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace Perlin {
  float Perlin1D(float x, int octaves, float persistence);
  float Perlin2D(float x, float y, int octaves, float persistence);
  float Perlin3D(float x, float y, float z, int octaves, float persistence);

  // periodic versions of the same function, inspired by the Renderman pnoise() functions
  float Perlin3DPeriodic(float x, float y, float z, int px, int py, int pz, int octaves, float persistence);
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_PERLIN_H