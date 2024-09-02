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
  static const size_t INVALID_INDEX;
  static const double EPSILON;

  class Cell : public pxr::GfRange3d
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
    Cell(Component* component, const pxr::GfRange3d& range);

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
  size_t AddCell(Component* component,
    const pxr::GfRange3d& range);

  const Geometry* GetGeometryFromCell(const BVH::Cell* cell) const;
  size_t GetGeometryIndexFromCell(const BVH::Cell* cell) const;

  Morton SortCells();
  pxr::GfRange3f UpdateCells();


   // visual debug
  void GetCells(pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<pxr::GfVec3f>& sizes, 
    pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf) override;

  pxr::GfVec3f GetMortonColor(const pxr::GfVec3f &point);

  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;

  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance) const override;

  void GetLeaves(const BVH::Cell* cell, std::vector<const Cell*>& leaves) const;
  void GetBranches(const BVH::Cell* cell, std::vector<const Cell*>& branches) const;
  
protected:
  pxr::GfVec3f _ComputeHitPoint(Location* hit) const;
  uint64_t _ComputeCode(const pxr::GfVec3d& point) const;
  pxr::GfVec3d _ComputeCodeAsColor(const pxr::GfVec3d& point) const;
  size_t _FindSplit(size_t first, size_t last) const;
  size_t _FindClosestCell(uint64_t code) const;
  bool _RecurseClosestCell(const BVH::Cell* cell, const pxr::GfVec3f& point, 
    Location* hit, double maxDistance) const;

  size_t _GetIndex(const BVH::Cell* cell) const;
  BVH::Cell* _GetCell(size_t index);
  const BVH::Cell* _GetCell(size_t index) const;
  void _AddPoints(Geometry* geometry);
  void _AddTriangles(Geometry* geometry);
  void _AddTrianglePairs(Geometry* geometry);
  size_t _RecurseSortCells(size_t first, size_t last);
  pxr::GfRange3f _RecurseUpdateCells(BVH::Cell* cell);
  bool _ShouldCheckCell(const BVH::Cell* cell, const pxr::GfVec3f& point, 
    double radius) const;

  bool _Raycast(const BVH::Cell* cell, const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const;
  bool _Closest(const BVH::Cell* cell, const pxr::GfVec3f& point, Location* hit,
    double maxDistance = DBL_MAX) const;

private:
  Cell*                           _root;
  std::vector<Cell>               _cells;
  std::vector<Morton>             _mortons;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H