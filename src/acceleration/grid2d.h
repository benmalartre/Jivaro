#ifndef JVR_ACCELERATION_GRID_H
#define JVR_ACCELERATION_GRID_H

#include <float.h>
#include <vector>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2d.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"
#include "../geometry/geometry.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class Curve;
class Points;

class Grid2DIntersector : public Intersector {
  enum ElemType {
    POINT,
    EDGE,
    TRIANGLE,
    POLYGON
  };

  typedef bool (Grid2DIntersector::*IntersectFunc)(
    Geometry* geometry, const GfRay &ray, Location* hit, 
    double maxDistance, double* minDistance);

  static uint32_t SLICE_INDICES[27*3];
public:
  struct Element {
    uint16_t    geometry;
    void*       ptr;
  };

  struct Cell
  {
#ifdef _WIN32
    Cell(uint32_t index):_index(index) { 
      _color = GfVec3f(
        (float)rand() / RAND_MAX , 
        (float)rand() / RAND_MAX, 
        (float)rand() / RAND_MAX);
  }
#else
    Cell(uint32_t index):_index(index) { 
      _color = GfVec3f(
        (float)drand48(), 
        (float)drand48(), 
        (float)drand48()); 
    }
#endif

    bool Contains(const GfVec2f& pos);
    //void insert(Vertex* V){_points.push_back(V);};
    //void Insert(Point* point);
    //void Insert(Edge* edge);
    void Insert(Triangle* triangle) { 
      _elements.push_back({0, (void*)triangle}); 
    };
    bool Raycast(Geometry* geom, const GfRay& ray, Location* hit, 
      double maxDistance=-1, double* minDistance=NULL) const;
    

    // neighboring bits
    static void InitNeighborBits(uint32_t& neighborBits);
    static void ClearNeighborBitsSlice(uint32_t& neighborBits, short axis, 
      short slice);
    static bool CheckNeighborBit(const uint32_t neighborBits, uint32_t index);

    // data
    std::vector<Geometry*>  _geometries;
    std::vector<Element>    _elements;
    GfVec3f            _color;
    uint32_t                _index;
    bool                    _hit;
    uint32_t                _neighborBits;

  };

  Grid2DIntersector():_cells(NULL),_numCells(0){};
  ~Grid2DIntersector()
  {
      DeleteCells();
  }
  GfRange3f* GetRange(){return &_range;};
  GfVec3f GetCellPosition(uint32_t index);
  GfVec3f GetCellMin(uint32_t index);
  GfVec3f GetCellMax(uint32_t index);
  Geometry* GetGeometry(size_t index){return _geometries[index];};
  //void SetGeometry(Geometry* geom, size_t index){_mesh = mesh;};
  inline uint32_t NumCells(){return _numCells;};
  uint32_t* GetResolution(){return &_resolution[0];};
  inline uint32_t GetResolutionX(){return _resolution[0];};
  inline uint32_t GetResolutionY(){return _resolution[1];};
  void ResetCells();
  // delete all cells
  void DeleteCells();
  // clear all rays
  void ClearRays() { _rays.clear(); };

  // geometries
  void Init(const std::vector<Geometry*>& geometries) override;
  void Update() override;
  void InsertMesh(Mesh* mesh);
  void InsertCurve(Curve* curve);
  void InsertPoints(Points* points);

  // intersect a ray with the mesh
  bool Raycast(const GfRay& ray, Location* hitPoint, 
    double maxDistance=-1, double* minDistance=NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1, double* minDistance = NULL) const override;

  Cell* GetCell(uint32_t index);
  Cell* GetCell(uint32_t x, uint32_t y);
  Cell* GetCell(const GfVec3f& pos);
  void GetNeighbors(uint32_t index, std::vector<Cell*>& neighbors);
  void GetNeighbors(uint32_t index, std::vector<uint32_t>& neighbors);
  void GetIndices(uint32_t index, uint32_t& X, uint32_t& Y);
  void GetCellIndexAndWeights(const GfVec3f& pos, uint32_t& index, 
    GfVec3f& weights);
  GfVec3f GetCellDimension(){return _cellDimension;};
  void IndexToXY(const uint32_t index, uint32_t& x, uint32_t& y);
  void XYToIndex(const uint32_t x, const uint32_t y, uint32_t& index);

private:
  ElemType                _elementType;
  // bounding box of the mesh
  uint32_t                _resolution[2];
  GfVec3f            _cellDimension;
  GfRange3f          _range;
  // cells
  Cell**                  _cells;
  uint32_t                _numCells;

  // rays tested for intersection
  std::vector<GfRay> _rays;
  uint32_t                _numRays;


  // place an element into intersected cells
  //void PlacePoint(void);
  //void PlaceEdge(HalfEdge* he);
  //void PlaceTriangle(Triangle* t);
  //void PlacePoint(Vertex* V);
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif