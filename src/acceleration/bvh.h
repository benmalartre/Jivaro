#ifndef JVR_ACCELERATION_BVH_H
#define JVR_ACCELERATION_BVH_H

#include <vector>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/range3d.h>
#include "../geometry/intersection.h"
#include "../geometry/component.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class BVH;

struct Mortom {
  uint64_t code;
  BVH* cell;

  inline bool operator <(const Mortom& other) {
    return code < other.code;
  }
};

class BVH : public Intersector
{ 
public:
  enum Type {
    ROOT,
    BRANCH,
    LEAF
  };

  // constructor
  BVH(BVH* parent=NULL, BVH* lhs=NULL, BVH* rhs=NULL);
  BVH(BVH* parent, Geometry* geometry);
  BVH(BVH* parent, Component* component, const pxr::GfRange3d& range);

  // destructor
  ~BVH() {
    if (_left)delete _left;
    if (_right)delete _right;
  };

  bool IsLeaf() const;
  bool IsRoot() const;
  BVH* GetRoot();
  const BVH* GetRoot() const;

  void SetLeft(BVH* cell) { _left = cell; };
  void SetRight(BVH* cell) { _right = cell; };
  BVH* GetLeft() { return _left; };
  BVH* GetRight() { return _right; };

  // debug
  void GetLeaves(std::vector<BVH*>& leaves);
  void GetCells(std::vector<BVH*>& cells);

  Geometry* GetGeometry();
  Geometry* GetGeometry() const;

  // override base class
  void Init(Geometry* geometry);
  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update(const std::vector<Geometry*>& geometries) override;
  virtual bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.f) const override;

  size_t GetNumCells();
  uint64_t ComputeCode(const pxr::GfVec3d& point);
  pxr::GfVec3d ComputeCodeAsColor(const pxr::GfVec3d& point);
 
protected:
  BVH* _RecurseSortCellsByPair(std::vector<Mortom>& mortoms, int first, int last);
  Mortom _SortCellsByPair(std::vector<Mortom>& mortoms);
  void _SortTrianglesByPair(std::vector<Mortom>& mortoms, Geometry* geometry);

  bool _Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const;
  bool _Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance) const;

private:
  BVH*      _parent;
  BVH*      _left;
  BVH*      _right;
  void*     _data;
  uint8_t   _type;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
