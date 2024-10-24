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

  class Cell : public GfRange3d
  {
  public:
    enum Type {
      ROOT,
      BRANCH,
      LEAF
    };

    // constructor
    Cell();
    Cell(size_t lhs, BVH::Cell* left, size_t rhs, BVH::Cell* right);
    Cell(Component* component, const GfRange3d& range);

    bool IsLeaf() const { return _type == BVH::Cell::LEAF; };
    bool IsRoot() const { return _type == BVH::Cell::ROOT; };
    bool IsBranch() const { return _type == BVH::Cell::BRANCH; };

    void SetData(void* data) { _data = data; };
    void SetLeft(size_t cell) { _left = cell; };
    void SetRight(size_t cell) { _right = cell; };
    void SetType(uint8_t type) { _type = type; };

    void* GetData() const { return _data; };
    uint8_t GetType() const { return _type; };
    size_t GetLeft() const { return _left; };
    size_t GetRight() const { return _right; };
 
  private:
    size_t    _left;
    size_t    _right;
    void*     _data;
    uint8_t   _type;
  };

public:
  BVH() {};
  ~BVH() {};

  Cell* GetRoot() { return _root; };
  const Cell* GetRoot() const { return _root; };

  Cell* GetCell(size_t index) { return &_cells[index]; };
  const Cell* GetCell(size_t index) const { return &_cells[index]; };

  size_t AddCell(BVH::Cell* left, BVH::Cell* right);
  size_t AddCell(Component* component, const GfRange3d& range);

  const Geometry* GetGeometryFromCell(const BVH::Cell* cell) const;
  size_t GetGeometryIndexFromCell(const BVH::Cell* cell) const;

  Morton SortCells();
  GfRange3f UpdateCells();

  // infos
  size_t GetNumComponents(){return _numComponents;};
  size_t GetNumLeaves(){return _mortons.size();};
  size_t GetNumCells(){return _cells.size();};

   // visual debug
  void GetCells(VtArray<GfVec3f>& positions, VtArray<GfVec3f>& sizes, 
    VtArray<GfVec3f>& colors, bool branchOrLeaf) override;

  GfVec3f GetMortonColor(const GfVec3f &point);

  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;

  virtual bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  virtual bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance) const override;

  void GetLeaves(const BVH::Cell* cell, std::vector<const Cell*>& leaves) const;
  void GetBranches(const BVH::Cell* cell, std::vector<const Cell*>& branches) const;
  
protected:
  GfVec3f _ComputeHitPoint(Location* hit) const;
  uint64_t _ComputeCode(const GfVec3d& point) const;
  GfVec3d _ComputeCodeAsColor(const GfVec3d& point) const;
  const Morton& _CellToMorton(size_t  cellIdx) const;

  size_t _GetIndex(const BVH::Cell* cell) const;
  BVH::Cell* _GetCell(size_t index);
  const BVH::Cell* _GetCell(size_t index) const;
  size_t _RecurseSortCells(size_t first, size_t last);
  GfRange3f _RecurseUpdateCells(BVH::Cell* cell);

  bool _Raycast(const BVH::Cell* cell, const GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const;
  bool _Closest(const BVH::Cell* cell, const GfVec3f& point, Location* hit,
    double maxDistanceSq = DBL_MAX) const;

private:
  Cell*                           _root;
  std::vector<Cell>               _cells;
  std::vector<Morton>             _mortons;
  std::vector<int>                _cellToMorton;
  size_t                          _numComponents;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H