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
  enum Type {
    ROOT,
    BRANCH,
    LEAF
  };

  enum Element {
    GEOMETRY,
    TRIPAIR,
    TRIANGLE,
    SEGMENT,
    POINT,
    INVALID
  };

  struct Data {
    Geometry* geometry;
    short     elemType;
  };

  // constructor
  BVH();
  BVH(BVH* parent);
  BVH(BVH* parent, Geometry* geometry);
  BVH(BVH* parent, BVH* lhs);
  BVH(BVH* parent, BVH* lhs, BVH* RHS);
  BVH(BVH* parent, TrianglePair* pair, const pxr::GfRange3d& range);
  BVH(BVH* parent, Triangle* tri, const pxr::GfRange3d& range);

  // destructor
  ~BVH() {
    if (_left)delete _left;
    if (_right)delete _right;
    if (_type == BVH::ROOT && _data)delete _data;
  };

  bool IsLeaf() const;
  bool IsRoot() const;
  void SetParent(BVH* cell) { _parent = cell; };
  BVH* GetParent() { return _parent; };
  const BVH* GetParent() const { return _parent; };
  BVH* GetRoot();
  const BVH* GetRoot() const;

  void SetLeft(BVH* cell) { _left = cell; };
  void SetRight(BVH* cell) { _right = cell; };
  BVH* GetLeft() { return _left; };
  BVH* GetRight() { return _right; };

  Geometry* GetGeometry();
  const Geometry* GetGeometry() const;
  short GetElementType();

  void Init(const std::vector<Geometry*>& geometries);
  void Init(Geometry* geometry, BVH* parent);
  void Update(const std::vector<Geometry*>& geometries);
  void Update(Geometry* geometry, BVH* parent);
  bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const;
  bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance = -1.f, double* minDistance=NULL) const;

  const pxr::GfRange3d& GetBoundingBox() const override;
  pxr::GfRange3d& GetBoundingBox() override;
  size_t GetNumCells();

  static void EchoNumHits();

private:
  void _SortCellsByPair(std::vector<BVH*>& cells, std::vector<BVH*>& results);
  void _SortTrianglesByPair(std::vector<BVH*>& leaves, Geometry* geometry);

  bool _RaycastGeometry(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const;
  bool _RaycastTrianglePair(const pxr::GfVec3f* points, const pxr::GfRay& ray, Hit* hit,
    double maxDistance, double* minDistance = NULL) const;

  bool _ClosestGeometry(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance, double* minDistance = NULL) const;
  bool _ClosestTrianglePair(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Hit* hit,
    double maxDistance, double* minDistance = NULL) const;

  BVH*      _parent;
  BVH*      _left;
  BVH*      _right;
  void*     _data;
  uint8_t   _type;

}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_BVH_H
