#ifndef JVR_ACCELERATION_BVH_H
#define JVR_ACCELERATION_BVH_H

#include <vector>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/component.h"
#include "../acceleration/mortom.h"
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

    bool IsLeaf() const;
    bool IsRoot() const;
    bool IsGeom() const;
    const Cell* GetRoot() const;
    const Cell* GetGeom() const;
    const BVH* GetIntersector() const;

    void SetData(void* data){_data = data;};
    void* GetData() const {return _data;};
    void SetLeft(Cell* cell) { _left = cell; };
    void SetRight(Cell* cell) { _right = cell; };
    void SetParent(Cell* cell) { _parent = cell; };
    void SetType(uint8_t type) { _type = type; };
    uint8_t GetType() { return _type; };
    Cell* GetLeft() { return _left; };
    Cell* GetRight() { return _right; };

    // debug
    void GetLeaves(std::vector<Cell*>& leaves);
    void GetCells(std::vector<Cell*>& cells);

    Geometry* GetGeometry();
    Mortom SortCellsByPair(std::vector<Mortom>& mortoms);
    

    void Init(Geometry* geometry);

    bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
      double maxDistance = FLT_MAX, double* minDistance = NULL) const;
    bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
      double maxDistance = FLT_MAX) const;

  protected:
    void _FinishSort(std::vector<Mortom>& cells);
    Cell* _RecurseSortCellsByPair(std::vector<Mortom>& mortoms, int first, int last);
    void _SortTrianglesByPair(std::vector<Mortom>& mortoms, Geometry* geometry);

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
    const pxr::GfVec3i p = WorldToMortom(*root, point);
    return Encode3D(p);
  }

  static pxr::GfVec3d ComputeCodeAsColor(BVH::Cell* root, const pxr::GfVec3d& point)
  {
    uint64_t mortom = ComputeCode(root, point);
    pxr::GfVec3i p = Decode3D(mortom);
    return pxr::GfVec3d(p[0] / (float)MORTOM_MAX_L, p[1] / (float)MORTOM_MAX_L, p[2] / (float)MORTOM_MAX_L);
  }

  Cell* GetRoot() { return &_root; };
  int GetGeometryIndex(Geometry* geom) const;

  
  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update(const std::vector<Geometry*>& geometries) override;
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance) const override;

private:
  Cell                        _root;
  std::vector<Geometry*>      _geometries;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
