#ifndef AMN_ACCELERATION_GRID_H
#define AMN_ACCELERATION_GRID_H

#include <float.h>
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"
#include "../geometry/triangle.h"
#include "../geometry/mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

class Grid3D {
 
  static uint32_t SLICE_INDICES[27*3];
public:
  struct _Triangle {
    Mesh*       mesh;
    Triangle*   tri;
  }
  struct Cell
  {
#ifdef _WIN32
    Cell(uint32_t index):_index(index) { 
      _color = pxr::GfVec3f(
        (float)rand() / RAND_MAX , 
        (float)rand() / RAND_MAX, 
        (float)rand() / RAND_MAX);
  }
#else
    Cell(uint32_t index):_index(index) { 
      _color = pxr::GfVec3f(
        (float)drand48(), 
        (float)drand48(), 
        (float)drand48()); 
    }
#endif

    bool Contains(const pxr::GfVec3f& pos);
    //void insert(Vertex* V){_points.push_back(V);};
    void Insert(Triangle* triangle){ _triangles.push_back(triangle); };
    const bool Intersect(Mesh* mesh, const pxr::GfRay &ray, 
      PointOnMesh* htPoint, double maxDistance, double* minDistance) const;

    // neighboring bits
    static void InitNeighborBits(uint32_t& neighborBits);
    static void ClearNeighborBitsSlice(uint32_t& neighborBits, short axis, 
      short slice);
    static bool CheckNeighborBit(const uint32_t neighborBits, uint32_t index);

    // data
    std::vector<_Triangle>  _triangles;
    pxr::GfVec3f            _color;
    uint32_t                _index;
    bool                    _hit;
    //Grid3D*                 _grid;
    uint32_t                _neighborBits;

  };

  Grid3D():_cells(NULL),_mesh(NULL),_numCells(0){};
  ~Grid3D()
  {
      DeleteCells();
  }
  pxr::GfRange3f* GetRange(){return &_range;};
  pxr::GfVec3f GetCellPosition(uint32_t index);
  pxr::GfVec3f GetCellMin(uint32_t index);
  pxr::GfVec3f GetCellMax(uint32_t index);
  Mesh* GetMesh(){return _mesh;};
  void SetMesh(Mesh* mesh){_mesh = mesh;};
  inline uint32_t NumCells(){return _numCells;};
  uint32_t* GetResolution(){return &_resolution[0];};
  inline uint32_t GetResolutionX(){return _resolution[0];};
  inline uint32_t GetResolutionY(){return _resolution[1];};
  inline uint32_t GetResolutionZ(){return _resolution[2];};
  void ResetCells();
  // delete all cells
  void DeleteCells();
  // clear all rays
  void ClearRays() { _rays.clear(); };

  // place a triangle mesh in the grid
  void PlaceIntoGrid(Mesh* mesh);
  //void PlaceIntoGrid(Mesh* mesh, std::vector<Vertex*>& points, const MMatrix& M, float cellSize);

  // intersect a ray with the mesh
  bool Intersect(const pxr::GfRay& ray, double maxDistance, 
    PointOnMesh* hitPoint) const;

  Cell* GetCell(uint32_t index);
  Cell* GetCell(uint32_t x, uint32_t y, uint32_t z);
  Cell* GetCell(const pxr::GfVec3f& pos);
  void GetNeighbors(uint32_t index, std::vector<Grid3D::Cell*>& neighbors);
  void GetNeighbors(uint32_t index, std::vector<uint32_t>& neighbors);
  void GetIndices(uint32_t index, uint32_t& X, uint32_t& Y, uint32_t& Z);
  void GetCellIndexAndWeights(const pxr::GfVec3f& pos, uint32_t& index, 
    pxr::GfVec3f& weights);
  pxr::GfVec3f GetCellDimension(){return _cellDimension;};
  void IndexToXYZ(const uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z);
  void XYZToIndex(const uint32_t x, const uint32_t y, const uint32_t z, 
    uint32_t& index);

private:
    // bounding box of the mesh
    uint32_t                _resolution[3];
    pxr::GfVec3f            _cellDimension;
    pxr::GfRange3f          _range;
    // cells
    Cell**                  _cells;
    uint32_t                _numCells;

    // rays tested for intersection
    std::vector<pxr::GfRay> _rays;
    uint32_t                _numRays;
    // mesh
    Mesh*                   _mesh;

    // place a triangle into intersected cells
    void PlaceTriangle(Triangle* t);
    //void PlacePoint(Vertex* V);
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif