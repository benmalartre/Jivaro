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
    Cell(Cell* parent, Cell* lhs, Cell* rhs);
    Cell(Cell* parent, Geometry* geometry);
    Cell(Cell* parent, Component* component,
      const pxr::GfRange3d& range);

    // destructor
    ~Cell() {
      if (_left)delete _left;
      if (_right)delete _right;
    };

    bool IsLeaf() const { return _type == BVH::Cell::LEAF; };
    bool IsRoot() const { return _type == BVH::Cell::ROOT; };
    bool IsBranch() const { return _type == BVH::Cell::BRANCH; };
    bool IsGeom() const { return _type == BVH::Cell::GEOM; };

    const Cell* GetRoot() const;
    const Cell* GetGeom() const;
    const BVH* GetIntersector() const;

    void SetData(void* data){_data = data;};
    void* GetData() const {return _data;};
    void SetLeft(Cell* cell) { _left = cell; if (cell)cell->_parent = this; };
    void SetRight(Cell* cell) { _right = cell; if (cell)cell->_parent = this;};
    void SetParent(Cell* cell) { _parent = cell; };
    void SetType(uint8_t type) { _type = type; };
    uint8_t GetType() { return _type; };
    Cell* GetLeft() { return _left; };
    Cell* GetRight() { return _right; };
    Cell* GetParent() { return _parent; };

    void GetLeaves(std::vector<Cell*>& leaves) const;
    void GetBranches(std::vector<Cell*>& branches) const;

    Geometry* GetGeometry();
    const Geometry* GetGeometry() const;
    
    Morton SortCellsByPair(std::vector<Morton>& mortons);
    pxr::GfRange3f UpdateCells();

    void Init(Geometry* geometry);

    bool Raycast(const pxr::GfRay& ray, Location* hit,
      double maxDistance = DBL_MAX, double* minDistance = NULL) const;
    bool Closest(const pxr::GfVec3f& point, Location* hit, 
      double maxDistance = DBL_MAX) const;

  protected:
    bool _LeafRaycast(const pxr::GfRay& ray, Location* hit, 
      double maxDistance = DBL_MAX, double* minDistance = NULL) const;
    void _FinishSort(std::vector<Morton>& cells);
    Cell* _RecurseSortCellsByPair(std::vector<Morton>& mortons, int first, int last);
    void _MortonSortTriangles(std::vector<Morton>& mortons, Geometry* geometry);
    void _MortonSortTrianglePairs(std::vector<Morton>& mortons, Geometry* geometry);
    
  private:
    Cell*     _parent;
    Cell*     _left;
    Cell*     _right;
    void*     _data;
    uint8_t   _type;
  };

public:
  BVH() {};

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

   // visual debug
  void GetCells(pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<pxr::GfVec3f>& sizes, 
    pxr::VtArray<pxr::GfVec3f>& colors, bool branchOrLeaf) override;

  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;

  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance) const override;

private:
  Cell                        _root;
  std::vector<Cell*>          _geoms;
  std::vector<Cell*>          _branches;
  std::vector<Cell*>          _leaves;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H