#ifndef JVR_ACCELERATION_GRID_H
#define JVR_ACCELERATION_GRID_H

#include <float.h>
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"
#include "../geometry/geometry.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Component;
class Point;
class Edge;
class Triangle;
class Mesh;
class Curve;
class Points;
struct Triangle;

class Grid3D : public Intersector {
  enum ElemType {
    POINT,
    EDGE,
    TRIANGLE,
    NUM_ELEM_TYPES
  };

  typedef bool (Grid3D::*IntersectFunc)(
    Geometry* geometry, const pxr::GfRay &ray, Location* hit, 
    double maxDistance, double* minDistance);

public:

  typedef std::vector<Component*> _Components;

  struct Cell
  {
    Cell(uint32_t id):index(id){};

    void Insert(Point* point);
    void Insert(Edge* edge);
    void Insert(Triangle* triangle);

    bool Raycast(const Geometry* geometry, const pxr::GfRay& ray, 
      Location* hit, double maxDistance=-1, double* minDistance=NULL) const;
    
    // data
    _Components                           components;
    uint32_t                              index;

  };

  struct Node
  {

  };

  Grid3D():_cells(NULL),_numCells(0){};
  ~Grid3D()
  {
     DeleteCells();
  }
  pxr::GfVec3f GetCellPosition(uint32_t index);
  pxr::GfVec3f GetCellMin(uint32_t index);
  pxr::GfVec3f GetCellMax(uint32_t index);
  inline uint32_t NumCells(){return _numCells;};
  uint32_t* GetResolution(){return &_resolution[0];};
  inline uint32_t GetResolutionX(){return _resolution[0];};
  inline uint32_t GetResolutionY(){return _resolution[1];};
  inline uint32_t GetResolutionZ(){return _resolution[2];};
  // delete all cells
  void DeleteCells();

  // geometries
  void Init(const std::vector<Geometry*>& geometries) override;
  void Update() override;

  void InsertMesh(Mesh* mesh);
  void InsertCurve(Curve* curve);
  void InsertPoints(Points* points);

  // intersect a ray with the grid
  bool Raycast(const pxr::GfRay& ray, Location* hitPoint,
    double maxDistance=DBL_MAX, double* minDistance=NULL) const override;
  bool Closest(const pxr::GfVec3f& point, ClosestPoint* hit,
    double maxDistance=DBL_MAX) const override;

  Cell* GetCell(uint32_t index);
  Cell* GetCell(uint32_t x, uint32_t y, uint32_t z);
  Cell* GetCell(const pxr::GfVec3f& pos);
  
  pxr::GfVec3f GetCellDimension(){return _cellDimension;};
  void IndexToXYZ(const uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z);
  void XYZToIndex(const uint32_t x, const uint32_t y, const uint32_t z, 
    uint32_t& index);
  pxr::GfVec3f GetColorXYZ(const uint32_t index);

  void GetCells(pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<pxr::GfVec3f>& sizes, 
    pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf) override;

protected:


private:
  uint32_t                _resolution[3];
  pxr::GfVec3f            _cellDimension;
  // cells
  Cell**                  _cells;
  uint32_t                _numCells;

};

class MultiGrid3D : public Intersector
{
public:
  // geometries
  void Init(const std::vector<Geometry*>& geometries) override;
  void Update() override;

  // intersect a ray with the multi-grid
  bool Raycast(const pxr::GfRay& ray, Location* hitPoint,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, ClosestPoint* hit,
    double maxDistance = DBL_MAX) const override;

private:
  std::vector<Grid3D>  _grids;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif