#ifndef JVR_GEOMETRY_SKELETON_H
#define JVR_GEOMETRY_SKELETON_H

#include "../common.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Skeleton : public Geometry {
  struct Joint {
    pxr::GfVec3f origin;
    pxr::GfQuaternionf rotation;
    float length;
  };
  using Chain = std::vector<Joint>;
  
public:
  Skeleton();

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

private:
  std::vector<Chain>   _chains;
  std::vector<Joint*>  _roots;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SKELETON_H