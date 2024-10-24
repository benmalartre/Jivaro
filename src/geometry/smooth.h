#ifndef JVR_GEOMETRY_SMOOTH_H
#define JVR_GEOMETRY_SMOOTH_H

#include <functional>

#include <pxr/base/vt/array.h>
#include <pxr/base/work/loops.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
class Smooth
{
  public:
    Smooth(size_t numPoints, VtArray<float>& weights);
    ~Smooth();
    size_t GetNumPoints(){return _points.size();};
    void SetNeighbors(size_t pointIndex, size_t numNeighbors, const int* neighbors);
    void SetDatas(size_t pointIndex, const T& data);
    const T& GetDatas(size_t pointIndex);
    void Compute(size_t numIterations);

  struct _Point
  {
    T                data[2];
    std::vector<int> neighbors;
  };

private:
  void _ComputeRange(size_t startIndex, size_t endIndex, bool flip);
  void _ComputeOne(size_t pointIndex, bool flip);

  std::vector<_Point>         _points;
  std::vector<float>          _weights;
  bool                        _flip;

};

template <typename T>
Smooth<T>::~Smooth()
{
  _points.clear();
}

template <typename T>
Smooth<T>::Smooth(size_t numPoints, VtArray<float>& weights)
{

  _weights.resize(numPoints);
  if (weights.size() == numPoints) {
    memcpy((void*)&_weights[0], (void*)&weights[0], numPoints * sizeof(float));
  }
  else {
    for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx)
      _weights[pointIdx] = 1.f;
  }
  _points.resize(numPoints);
}

template <typename T>
void Smooth<T>::SetNeighbors(size_t pointIndex, size_t numNeighbors, const int* neighbors)
{
  _points[pointIndex].neighbors.resize(numNeighbors);
  for (size_t iii = 0; iii < numNeighbors; ++iii)
  {
    _points[pointIndex].neighbors[iii] = neighbors[iii];
  }
}

template <typename T>
void Smooth<T>::SetDatas(size_t pointIndex, const T& data)
{
  _points[pointIndex].data[0] = data;
  _points[pointIndex].data[1] = data;
}

template <typename T>
const T& Smooth<T>::GetDatas(size_t pointIndex)
{
  return _points[pointIndex].data[_flip];
}

template <typename T>
void Smooth<T>::_ComputeRange(size_t startIdx, size_t endIdx, bool flip)
{
  for (size_t pointIdx = startIdx; pointIdx < endIdx; ++pointIdx) {
    _ComputeOne(pointIdx, flip);
  }
}

template <typename T>
void Smooth<T>::_ComputeOne(size_t pointIndex, bool flip)
{
  const float weight = _weights[pointIndex];
  if (weight < 0.0000000001f)return;
  const Smooth::_Point& point = _points[pointIndex];
  size_t numNeighbors = point.neighbors.size();

  T average(0.f);
  size_t numActiveNeighbors = 0;
  for (size_t neighborIdx = 0; neighborIdx < numNeighbors; ++neighborIdx)
  {
    size_t neighborIndex = point.neighbors[neighborIdx];
    if (neighborIndex < _points.size()) {
      average += _points[neighborIndex].data[flip];
      numActiveNeighbors++;
    }
  }

  if (numActiveNeighbors) {
    average /= (float)numActiveNeighbors;
    _points[pointIndex].data[1 - flip] =
      (_points[pointIndex].data[flip]) * (1.f - weight) +
      (_points[pointIndex].data[flip] + average) * 0.5f * weight;
  }
}

template <typename T>
void Smooth<T>::Compute(size_t numIterations)
{
  _flip = false;
  for (size_t iterIdx = 0; iterIdx < numIterations; ++iterIdx) {
    WorkParallelForN(
      _points.size(), 
      std::bind(&Smooth<T>::_ComputeRange, this, std::placeholders::_1, std::placeholders::_2, _flip),
      32
    );
    _flip = 1 - _flip;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SMOOTH_H
