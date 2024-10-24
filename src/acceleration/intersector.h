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
class Intersector : public GfRange3d
{ 
public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
  static const size_t MINIMUM_COMPONENTS = 4;
  struct _Geom {
    Geometry*   geom;
    size_t      start;
    size_t      end;
  };

public:
  virtual ~Intersector(){};

  size_t GetGeometryIndex(Geometry* geom) const ;
  size_t GetGeometryCellsStartIndex(size_t index) const;
  size_t GetGeometryCellsEndIndex(size_t index) const;

  void SetGeometryCellsStartIndex(size_t index, size_t cell){_geoms[index].start = cell;};
  void SetGeometryCellsEndIndex(size_t index, size_t cell){_geoms[index].end = cell;};
  void SetGeometryCellIndices(size_t index, size_t start, size_t end);

  Geometry* GetGeometry(size_t index) { return _geoms[index].geom; };
  const Geometry* GetGeometry(size_t index) const {return _geoms[index].geom;};
  size_t GetNumGeometries() const {return _geoms.size();};

  //used for visually debug
  virtual void GetCells(VtArray<GfVec3f>& positions, VtArray<GfVec3f>& sizes, 
    VtArray<GfVec3f>& colors, bool branchOrLeaf=true){};


  virtual void Init(const std::vector<Geometry*>& geometries) = 0;

  // overriden by derived classes
  virtual void Update() {};
  virtual bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance=DBL_MAX, double* minDistance=NULL) const = 0;
    
  virtual bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance=DBL_MAX) const = 0;

protected:
  virtual void _Init(const std::vector<Geometry*>& geometries);
   bool _accelerated;

private:
   std::vector<_Geom> _geoms;
}; 

JVR_NAMESPACE_CLOSE_SCOPE
#endif // JVR_ACCELERATION_INTERSECTOR_H