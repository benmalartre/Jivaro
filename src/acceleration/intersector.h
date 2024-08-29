#ifndef JVR_ACCELERATION_INTERSECTOR_H
#define JVR_ACCELERATION_INTERSECTOR_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/vt/array.h>
#include "../geometry/location.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Location;
class Intersector : public pxr::GfRange3d
{ 
public:
 static const int INVALID_GEOMETRY = INT_MAX;
  enum ElementType {
    POINT,
    EDGE,
    TRIANGLE,
    POLYGON,
    INVALID
  };

  struct _Geom {
    size_t      index;
    size_t      start;
    size_t      end;
    Geometry*   geom;
  };

public:
  virtual ~Intersector(){};

  size_t GetGeometryIndex(Geometry* geom) const;
  const Geometry* GetGeometry(size_t index) const {return _geoms[index].geom;};
  Geometry* GetGeometry(size_t index) {return _geoms[index].geom;};
  size_t GetNumGeometries() const {return _geoms.size();};

  //used for visually debug
  virtual void GetCells(pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<pxr::GfVec3f>& sizes, 
    pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf=true){};


  virtual void Init(const std::vector<Geometry*>& geometries) = 0;

  // overriden by derived classes
  virtual void Update() {};
  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance=DBL_MAX, double* minDistance=NULL) const = 0;
    
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance=DBL_MAX) const = 0;

protected:
  virtual void _Init(const std::vector<Geometry*>& geometries);
   std::vector<_Geom> _geoms;
}; 

JVR_NAMESPACE_CLOSE_SCOPE
#endif // JVR_ACCELERATION_INTERSECTOR_H