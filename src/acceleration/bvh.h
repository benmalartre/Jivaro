#ifndef JVR_ACCELERATION_BVH_H
#define JVR_ACCELERATION_BVH_H

#include <vector>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/component.h"
#include "../acceleration/morton.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class BVH : public Intersector
{
public:
  static const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();


  class Cell : public pxr::GfRange3d
  {
  public:
    enum Type {
      ROOT,
      GEOM,
      BRANCH,
      LEAF
    };

    // constructor
    Cell();
    Cell(size_t parent, size_t lhs, const BVH::Cell* left, size_t rhs, const BVH::Cell* right);
    Cell(size_t parent, Geometry* geometry);
    Cell(size_t parent, Component* component, const pxr::GfRange3d& range);

    bool IsLeaf() const { return _type == BVH::Cell::LEAF; };
    bool IsRoot() const { return _type == BVH::Cell::ROOT; };
    bool IsBranch() const { return _type == BVH::Cell::BRANCH; };
    bool IsGeom() const { return _type == BVH::Cell::GEOM; };

    const Cell* GetRoot() const;
    const Cell* GetGeom() const;
    const BVH* GetIntersector() const;

    void SetData(void* data) { _data = data; };
    void SetLeft(size_t cell) { _left = cell; };
    void SetRight(size_t cell) { _right = cell; };
    void SetParent(size_t cell) { _parent = cell; };
    void SetType(uint8_t type) { _type = type; };

    void* GetData() const { return _data; };
    uint8_t GetType() const { return _type; };
    size_t GetLeft() const { return _left; };
    size_t GetRight() const { return _right; };
    size_t GetParent() const { return _parent; };
 
  private:
    BVH*      _bvh;
    size_t    _parent;
    size_t    _left;
    size_t    _right;
    void*     _data;
    uint8_t   _type;
  };

  struct _Geom {
    Geometry*   geom;
    size_t      index;
    size_t      start;
    size_t      end;
  };

public:
  BVH() {};
  ~BVH();

  static uint64_t ComputeCode(const BVH::Cell* root, const pxr::GfVec3d& point)
  {
    const pxr::GfVec3i p = WorldToMorton(*root, point);
    return MortonEncode3D(p);
  }

  static pxr::GfVec3d ComputeCodeAsColor(BVH::Cell* root, const pxr::GfVec3d& point)
  {
    uint64_t morton = ComputeCode(root, point);
    pxr::GfVec3i p = MortonDecode3D(morton);
    return pxr::GfVec3d(
      p[0] / (float)MORTOM_MAX_L, 
      p[1] / (float)MORTOM_MAX_L, 
      p[2] / (float)MORTOM_MAX_L
    );
  }

 
  Cell* GetRoot() { return &_root; };
  const Cell* GetRoot() const { return &_root; };

  Cell* GetCell(size_t index) { return &_cells[index]; };
  const Cell* GetCell(size_t index) const { return &_cells[index]; };

  BVH::Cell* AddCell(BVH::Cell* parent, BVH::Cell* left, BVH::Cell* right);
  BVH::Cell* AddCell(BVH::Cell* parent, Geometry* geometry);
  BVH::Cell* AddCell(BVH::Cell* parent, Component* component,
    const pxr::GfRange3d& range);

  void AddGeometry(BVH::Cell* cell, Geometry* geometry);
  const Geometry* GetGeometryFromCell(const BVH::Cell* cell) const;
  const Geometry* GetGeometry(size_t index) const;

  Morton SortCellsByPair(BVH::Cell* cell);
  pxr::GfRange3f UpdateCells();

   // visual debug
  void GetCells(pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<pxr::GfVec3f>& sizes, 
    pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf) override;

  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;

  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance) const override;

  void GetLeaves(const BVH::Cell* cell, std::vector<const Cell*>& leaves) const;
  void GetBranches(const BVH::Cell* cell, std::vector<const Cell*>& branches) const;
  
protected:
  int _FindSplit(int first, int last);
  size_t _GetIndex(const BVH::Cell* cell) const;
  BVH::Cell* _GetCell(size_t index);
  const BVH::Cell* _GetCell(size_t index) const;
  void _MortonSortTriangles(BVH::Cell* cell, Geometry* geometry);
  void _MortonSortTrianglePairs(BVH::Cell* cell, Geometry* geometry);
  Cell* _RecurseSortCellsByPair(BVH::Cell* cell, int first, int last);
  void _FinishSort(BVH::Cell* cell);

  pxr::GfRange3f _RecurseUpdateCells(BVH::Cell* cell, const Geometry* geometry);

  bool _Raycast(const BVH::Cell* cell, const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const;
  bool _Closest(const BVH::Cell* cell, const pxr::GfVec3f& point, Location* hit,
    double maxDistance = DBL_MAX) const;

private:
  Cell                            _root;
  std::vector<Cell>               _cells;
  std::vector<_Geom>              _geoms;
  std::vector<Morton>             _mortons;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H