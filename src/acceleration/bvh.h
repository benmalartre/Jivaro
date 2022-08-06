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
    COMPONENT
  };

  class Cell : public pxr::GfRange3d
  {
  public:
    Cell() {};
    virtual ~Cell() {};
    virtual bool Raycast(const pxr::GfRay& r, Hit* h, 
      double maxDistance=-1.f, double *minDistance=NULL) const = 0;
    virtual bool IsLeaf() const = 0;
  };

  class Branch : public Cell
  {
  public:
    Branch() {};
    Branch(Cell* s);
    Branch(Cell* lhs, Cell* rhs);

    void SetLeft(Cell* cell) { _left = cell; };
    void SetRight(Cell* cell) { _right = cell; };
    Cell* GetLeft() { return _left; };
    Cell* GetRight() { return _right; };
    bool IsLeaf() const override { return false; };
    bool Raycast(const pxr::GfRay& r, Hit* h,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Branch() override {
      if (_left) delete _left;
      if (_right) delete _right;
    };

  private:
    Cell* _left;
    Cell* _right;
  };

  class Leaf : public Cell
  {
  public:
    typedef bool (BVH::Leaf::*ImplRaycastFunc)(const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance) const;

    Leaf() {};
    Leaf(Geometry* geometry);
    Leaf(TrianglePair* pair);

    bool IsLeaf() const override { return true; };
    void Set(void* data) { _data = data; };
    const void* Get() const { return _data; };
    bool Raycast(const pxr::GfRay& ray, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Leaf() override {};

    bool _RaycastGeometry(const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;
    bool _RaycastTrianglePair(const pxr::GfRay& ray, Hit* hit,
      double maxDistance, double* minDistance = NULL) const;

  private:
    ImplRaycastFunc _raycastImpl;
    void*           _data;
  };

  // destructor
  ~BVH() {
    delete _root;
  };

  void Init(const std::vector<Geometry*>& geometries) override;
  void Init(Geometry* geometry);
  void Update(const std::vector<Geometry*>& geometries) override;
  void Update(Geometry* geometry);
  /*
  void Init(const Geometry* geometry);
  void Update(const Geometry* geometry);
  */
  bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;

  const pxr::GfBBox3d& GetBoundingBox() const override;
  size_t GetNumCells();

private:
  void _SortCellsByPair(std::vector<Cell*>& cells, std::vector<Cell*>& results);
  void _SortTrianglesByPair(std::vector<Cell*>& leaves, Geometry* geometry);

  Branch* _root;
  short   _elemType;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
