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
  class Cell : public pxr::GfRange3d
  {
  public:
    Cell() {};
    virtual ~Cell() {};
    virtual bool Raycast(const pxr::GfRay& r, Hit* h, 
      double maxDistance=-1.f, double *minDistance=NULL) const = 0;
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
    Leaf() {};
    Leaf(const TrianglePair& data);

    void Set(const TrianglePair& data) { _data = data; };
    const TrianglePair& Get() const { return _data; };
    bool Raycast(const pxr::GfRay& ray, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Leaf() override {};

  private:
    TrianglePair _data;
  };

  Branch* root;
  // destructor
  ~BVH() {
    delete root;
  };

  void Init(const std::vector<Geometry*>& geometries) override;
  void Update(const std::vector<Geometry*>& geometries) override;
  bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const override;

private:
  void _SortGeometriesByPair(std::vector<Cell*>& cells,
    std::vector<Cell*>& results);

  void _SortTrianglesByPair(std::vector<Cell*>& leaves,
    Geometry* geometry, std::vector<Triangle*>& triangles);
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
