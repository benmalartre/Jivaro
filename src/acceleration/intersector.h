#ifndef JVR_ACCELERATION_INTERSECTOR_H
#define JVR_ACCELERATION_INTERSECTOR_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Intersector : public pxr::GfRange3d
{ 
public:
  enum ElementType {
    POINT,
    EDGE,
    TRIANGLE,
    POLYGON,
    INVALID
  };

public:
  Intersector(){};
  virtual ~Intersector(){};

  int GetGeometryIndex(Geometry* geom) const;
  const std::vector<Geometry*>& GetGeometries() const {return _geometries;};
  const Geometry* GetGeometry(size_t index) const {return _geometries[index];};
  Geometry* GetGeometry(size_t index) {return _geometries[index];};

  virtual void Init(const std::vector<Geometry*>& geometries) = 0;

  // overriden by derived classes
  virtual void Update() {};
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit,
    double maxDistance=-1, double* minDistance=NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit,
    double maxDistance=-1.f) const = 0;

protected:
  virtual void _Init(const std::vector<Geometry*>& geometries){_geometries = geometries;};
   std::vector<Geometry*>      _geometries;
}; 

JVR_NAMESPACE_CLOSE_SCOPE
#endif // JVR_ACCELERATION_INTERSECTOR_H