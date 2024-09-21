#ifndef JVR_ACCELERATION_DISTANCE_H
#define JVR_ACCELERATION_DISTANCE_H

#include <vector>
#include <pxr/base/gf/vec3f.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE


// different distance metrics
//
enum DistanceType {
  CHEBYSHEV,
  MANHATTAN,
  EUCLIDEAN
};

// Distance measure base class
//
class DistanceMeasure {
public:
  DistanceMeasure(const std::vector<double>* weights = NULL);
  virtual ~DistanceMeasure();
  virtual double Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) = 0;
  virtual double Compute1D(double x, double y, size_t dim) = 0;

protected:
  std::vector<double>* _weights;
};

// Chebyshev distance (greatest difference along each dimension)
//
class DistanceChebyshev : virtual public DistanceMeasure {
public:
  double Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) override;
  double Compute1D(double x, double y, size_t dim) override;
    
};

// Manhattan distance (sum of absolute difference along each dimension)
//
class DistanceManhattan : virtual public DistanceMeasure {
public:
  double Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) override;
  double Compute1D(double x, double y, size_t dim) override;
};

// Euclidean distance (length of the line segment between the two points) (squared)
//
class DistanceEuclidean : virtual public DistanceMeasure {
public:
  double Compute(const pxr::GfVec3f& lhs, const pxr::GfVec3f& rhs) override;
  double Compute1D(double x, double y, size_t dim) override;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_DISTANCE_H