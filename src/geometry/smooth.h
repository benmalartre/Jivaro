#ifndef JVR_GEOMETRY_SMOOTH_H
#define JVR_GEOMETRY_SMOOTH_H

#include <pxr/base/vt/array.h>
#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

template <typename T>
class Smooth
{
  public:
    Smooth(size_t numPoints, pxr::VtArray<float>& weights);
    ~Smooth();
    size_t GetNumPoints(){return _points.size();};
    void SetNeighbors(size_t pointIndex, size_t numNeighbors, int* neighbors);
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
  size_t                      _numPoints;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SUBDIV_H
