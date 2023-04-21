
#include "../geometry/smooth.h"


JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
Smooth<T>::~Smooth<T>()
{
    _points.clear();
}

template <typename T>
Smooth<T>::Smooth<T>(size_t numPoints, pxr::VtArray<float>& weights)
{
  _numPoints = numPoints;
  _weights.resize(_numPoints);
  if(weights.size() == _numPoints) {
    memcpy((void*)&_weights[0], (void*)&weights[0], _numPoints * sizeof(float));
  } else {
    for(size_t pointIdx = 0; pointIdx < _numPoints; ++pointIdx)
      _weights[pointIdx] = 1.f;
  }
  _points.resize(numPoints);

  for(size_t p = 0; p < _numPoints; ++p)
  {
    _points[p]._data[0].resize(_numSamples);
    _points[p]._data[1].resize(_numSamples);
  }
}

template <typename T>
void Smooth<T>::SetNeighbors(size_t pointIndex, size_t numNeighbors, int* neighbors)
{
  _points[pointIndex]._neighbors.resize(numNeighbors);
  for(size_t iii = 0; iii < numNeighbors; ++iii)
  {
    _points[pointIndex]._neighbors[iii] = neighbors[iii];
  }
}

template <typename T>
void Smooth<T>::SetDatas(size_t pointIndex, const T& data)
{
  _points[pointIndex]._data[0] = data;
  _points[pointIndex]._data[1] = data;
}

template <typename T>
const T& Smooth<T>::GetDatas(size_t pointIndex)
{
  return _points[pointIndex]._data[_flip];
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
  const float weight = _weights[pointIndex] ;
  if(weight < 0.0000000001f)return;
  const SmoothPoint& point = _points[pointIndex];
  size_t numNeighbors = point._neighbors.size();

  T average(0.f);
  size_t numActiveNeighbors = 0;
  for(size_t neighborIdx = 0; neighborIdx < numNeighbors; ++neighborIdx)
  {
    size_t neighborIndex = _points[pointIndex]._neighbors[neighborIdx];
    if(neighborIndex < _points.size()) {
      average += _points[neighborIndex]._data[_flip];
      numActiveNeighbors++;
    }
  }

  if(numActiveNeighbors) {
    average /= (float)numActiveNeighbors; 
    _points[pointIndex]._data[1-flip] = 
      (_points[pointIndex]._data[flip]) * (1.f - weight) +
      (_points[pointIndex]._data[flip] + average) * 0.5f * weight;
  }
}

template <typename T>
void Smooth<T>::Compute(size_t numIterations)
{
  std::cout << "SMOTTH COMPUTE : " << numIterations << std::endl;
  bool flip = false;
  for (size_t iterIdx = 0; iterIdx < numIterations; ++iterIdx) {
    WorkParallelForN(_points.size(), std::bind(&Smooth<T>::_ComputeRange, this, _1, _2, flip));
    std::cout << "iter index " << iterIdx << std::endl;
  }
  
}

JVR_NAMESPACE_CLOSE_SCOPE