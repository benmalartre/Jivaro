#include <pxr/base/work/loops.h>

#include "../pbd/accum.h"
#include "../pbd/constraint.h"

JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
Accum<T>::Accum(size_t numPoints, const std::vector<Constraint*>& constraints, size_t start, size_t end)
{
  size_t numBitPoints = 
    numPoints > sizeof(int) ? numPoints / sizeof(int) + 1 : 1;
  _indices.resize(numBitPoints, 0);

  for(size_t index = start; index < end; ++index) {
    const Constraint* constraint = constraints[index];
    const size_t numParticles = constraint->GetNumParticles();
    const int* particles = constraint->GetParticlesCPtr();
    for(size_t p = 0; p < numParticles; ++p) {
      BITMASK_SET(
        _indices[particles[p] / sizeof(int)], 
        particles[p] % sizeof(int));
    }
  }
}

template <typename T>
bool Accum<T>::Use(size_t index)
{
  const size_t bitsIdx = index / sizeof(int);
  if (bitsIdx >= _indices.size())return false;
  return BIT_CHECK(_indices[bitsIdx], index % sizeof(int));
}

template <typename T>
void Accum<T>::Reset()
{
  for(auto& value: _values)
    value = T(0);
}

JVR_NAMESPACE_CLOSE_SCOPE