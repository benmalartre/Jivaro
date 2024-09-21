#include "../acceleration/distance.h"

JVR_NAMESPACE_OPEN_SCOPE

// Distance base class
//
DistanceMeasure::DistanceMeasure(const std::vector<double>* weights) 
{
  if (weights)
    _weights = new std::vector<double>(*weights);
  else
    _weights = (std::vector<double>*)NULL;
}

DistanceMeasure::~DistanceMeasure() {
  if (_weights) delete _weights;
}

// Chebyshew distance
//
double 
DistanceChebyshev::Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) 
{
  size_t i;
  double dist, test;
  if (_weights) {
    dist = (*_weights)[0] * pxr::GfAbs(lhs[0] - rhs[0]);
    for (i = 1; i < 3; i++) {
      test = (*_weights)[i] * pxr::GfAbs(lhs[i] - rhs[i]);
      if (test > dist) dist = test;
    }
  } else {
    dist = pxr::GfAbs(lhs[0] - rhs[0]);
    for (i = 1; i < 3; i++) {
      test = pxr::GfAbs(lhs[i] - rhs[i]);
      if (test > dist) dist = test;
    }
  }
  return dist;
}

double 
DistanceChebyshev::Compute1D(double x, double y, size_t dim) 
{
  if (_weights)
    return (*_weights)[dim] * pxr::GfAbs(x - y);
  else
    return pxr::GfAbs(x - y);
}

// Manhattan distance
//
double 
DistanceManhattan::Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) 
{
  size_t i;
  double dist = 0.0;
  if (_weights) {
    for (i = 0; i < 3; i++) dist += (*_weights)[i] * pxr::GfAbs(lhs[i] - rhs[i]);
  } else {
    for (i = 0; i < 3; i++) dist += pxr::GfAbs(lhs[i] - rhs[i]);
  }
  return dist;
}

double 
DistanceManhattan::Compute1D(double x, double y, size_t dim) 
{
  if (_weights)
    return (*_weights)[dim] * pxr::GfAbs(x - y);
  else
    return pxr::GfAbs(x - y);
}

// Euclidean distance (squared)
//
double 
DistanceEuclidean::Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) 
{
  size_t i;
  double dist = 0.0;
  if (_weights) {
    for (i = 0; i < 3; i++)
      dist += (*_weights)[i] * (lhs[i] - rhs[i]) * (lhs[i] - rhs[i]);
  } else {
    for (i = 0; i < 3; i++) dist += (lhs[i] - rhs[i]) * (lhs[i] - rhs[i]);
  }
  return dist;
}

double 
DistanceEuclidean::Compute1D(double x, double y, size_t dim) 
{
  if (_weights)
    return (*_weights)[dim] * (x - y) * (x - y);
  else
    return (x - y) * (x - y);
}

JVR_NAMESPACE_CLOSE_SCOPE