#ifndef JVR_PBD_ACCUM_H
#define JVR_PBD_ACCUM_H

#include <vector>

#include <pxr/base/vt/array.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Constraint;
class Force;

// per thread sparse data
// indices usage are encoded in _indices bits
// _values only store used indices data
template <typename T>
class Accum
{
public:
  Accum(size_t numPoints, const std::vector<Constraint*>& constraints, size_t start, size_t end);

  bool Use(size_t index);
  void Reset();
  T Get(size_t index) { return _values[index]; };

protected:
  pxr::VtArray<int> _indices;
  pxr::VtArray<T>   _values;
};

template <typename T>
using AccumVector = std::vector<Accum<T>>;

template <typename T>
void Accumulate(const AccumVector<T>& accums, pxr::VtArray<T>& result) {
  std::vector<int> currents(accums.size(), 0);

  for (size_t pointIndex = 0; pointIndex < result.size(); ++pointIndex) {
    for (size_t accumIndex = 0; accumIndex < currents.size(); ++accumIndex) {
      const Accum& accum = accums[accumIndex];
      if (accum.Use(pointIndex)) {
        result[pointIndex] += accum.Get(currents[accumIndex]);
        currents[accumIndex]++;
      }
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_ACCUM_H
