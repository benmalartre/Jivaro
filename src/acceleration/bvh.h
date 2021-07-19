#ifndef AMN_ACCELERATION_BVH_H
#define AMN_ACCELERATION_BVH_H

#include <vector>
#include <limits>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../acceleration/intersector.h

AMN_NAMESPACE_OPEN_SCOPE

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
    Leaf(Geometry* geometry);

    void SetGeometry(Geometry* geometry) { _geom = geometry; };
    Geometry* GetGeometry() { return _geom; };
    bool Raycast(const pxr::GfRay& ray, Hit* hit,
      double maxDistance = -1.f, double* minDistance = NULL) const override;
    ~Leaf() override {};

  private:
    Geometry* _geom;
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
}; 

AMN_NAMESPACE_CLOSE_SCOPE

#endif