#ifndef JVR_ACCELERATION_BVH_H
#define JVR_ACCELERATION_BVH_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/triangle.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class BVH : public Intersector
{ 
public:
  enum ElementType {
    GEOMETRY,
    TRIPAIR,
    TRIANGLE,
    SEGMENT,
    POINT
  };

  class Cell : public pxr::GfRange3d
  {
  public:
    Cell(Cell* p=NULL) : _parent(p){};
    virtual ~Cell() {};
    virtual bool Raycast(const pxr::GfRay& r, Hit* h, 
      double maxDistance=-1.f, double *minDistance=NULL) const = 0;
    virtual bool Closest(const pxr::GfVec3f& point, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const = 0;
    virtual bool IsLeaf() const = 0;
    virtual bool IsRoot() const = 0;
    void SetParent(Cell* cell) { _parent = cell; };
    Cell* GetParent() { return _parent; };
    const Cell* GetParent() const { return _parent; };
    Cell* GetRoot();
    const Cell* GetRoot() const;
  private:
    Cell* _parent;
  };

  class Branch : public Cell
  {
  public:
    Branch(Cell* p);
    Branch(Cell* p, Cell* lhs);
    Branch(Cell* p, Cell* lhs, Cell* rhs);

    void SetLeft(Cell* cell) { _left = cell; };
    void SetRight(Cell* cell) { _right = cell; };
    Cell* GetLeft() { return _left; };
    Cell* GetRight() { return _right; };
    bool IsLeaf() const override { return false; };
    bool IsRoot() const override { return false; };
    bool Raycast(const pxr::GfRay& r, Hit* h,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    bool Closest(const pxr::GfVec3f& point, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Branch() override {
      if (_left) delete _left;
      if (_right) delete _right;
    };

  private:
    Cell* _left;
    Cell* _right;
  };

  class Root : public Branch
  {
  public:
    Root(Geometry* geometry, Cell* parent);
    bool IsLeaf() const override { return false; };
    bool IsRoot() const override { return true; };

    Geometry* GetGeometry() { return _geom; };
    const Geometry* GetGeometry() const { return _geom; };

  private:
    mutable Geometry* _geom;
  };

  class Leaf : public Cell
  {
  public:
    typedef bool (BVH::Leaf::*ImplRaycastFunc)(
      const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance) const;

    typedef bool (BVH::Leaf::* ImplClosestFunc)(
      const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
      double maxDistance, double* minDistance) const;

    Leaf(Cell* p) {};
    Leaf(Cell* p, Geometry* geometry);
    Leaf(Cell* p, TrianglePair* pair, const pxr::GfRange3d& bbox);
    Leaf(Cell* p, Triangle* triangle, const pxr::GfRange3d& bbox);

    bool IsLeaf() const override { return true; };
    bool IsRoot() const override { return false; };

    Geometry* GetGeometry();
    const Geometry* GetGeometry() const;

    void Set(void* data) { _data = data; };
    const void* Get() const { return _data; };
    bool Raycast(const pxr::GfRay& ray, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    bool Closest(const pxr::GfVec3f& point, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Leaf() override {};

    bool _RaycastGeometry(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;
    bool _RaycastTrianglePair(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;

    bool _ClosestGeometry(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;
    bool _ClosestTrianglePair(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;
    /* bool _ClosestTriangle(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;*/

  private:
    ImplRaycastFunc _raycastImpl;
    ImplClosestFunc _closestImpl;
    void*           _data;
  };

  // destructor
  ~BVH() {
    delete _root;
  };

  void Init(const std::vector<Geometry*>& geometries) override;
  void Init(Geometry* geometry, Cell* parent);
  void Update(const std::vector<Geometry*>& geometries) override;
  void Update(Geometry* geometry, Cell* parent);
  bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;

  const pxr::GfRange3d& GetBoundingBox() const override;
  pxr::GfRange3d& GetBoundingBox() override;
  size_t GetNumCells();

private:
  void _SortCellsByPair(std::vector<Cell*>& cells, std::vector<Cell*>& results);
  void _SortTrianglesByPair(std::vector<Cell*>& leaves, Geometry* geometry);

  Root*   _root;
  short   _elemType;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
