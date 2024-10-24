//======================================================
// OCTREE DECLARATION
//======================================================
#ifndef JVR_ACCELERATION_OCTREE_H
#define JVR_ACCELERATION_OCTREE_H

#include <float.h>
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/ray.h>
#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/component.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class Curve;
class Points;
struct Point;
struct Edge;
struct Triangle;

class Octree : public Intersector {
public: 
  class Cell {

  public:
    Cell() : _depth(0), _min(-1, -1, -1), _max(1, 1, 1), _isLeaf(true)
    {
      for (int i = 0; i < 8; i++) _child[i] = NULL;
    };

    Cell(const GfVec3f& minp, const GfVec3f& maxp, int depth = 0) :
      _depth(depth), _isLeaf(true), _min(minp), _max(maxp)
    {
      for (int i = 0; i < 8; i++) _child[i] = NULL;
    };

    ~Cell();

    // depth in octree
    int GetDepth() const { return _depth; };

    // center
    GfVec3f GetCenter() { return (GfVec3f((_min + _max) * 0.5f)); };
    GfVec3f GetHalfSize() { return (GfVec3f((_max - _min) * 0.5f)); };

    // distance
    inline float _Squared(float v) const { return v * v; };
    inline float _GetDistance1D(float p, float lower, float upper) const
    {
      if (p < lower)return lower - p;
      if (p > upper)return p - upper;
      return fmin(p - lower, upper - p);
    };
    float GetDistance(const GfVec3f& point) const;

    // intersect sphere
    bool IntersectSphere(const GfVec3f& center, const float radius) const;

    // leaf
    bool IsLeaf() const { return _isLeaf; };

    // get cell
    Cell* GetCell(int x) { return _child[x]; }
    const Cell* GetCell(int x) const { return _child[x]; }

    // triangles size
    int NumElements() const { return _elements.size(); };
    std::vector<Component*>& GetElements() { return _elements; };
    Component* GetElement(unsigned index) { return _elements[index]; };

    // insert a triangle
    void Insert(Component* e) { _elements.push_back(e); };

    // split into 8
    void Split(const GfVec3f* points);

    // get bounding box
    void GetBoundingBox(const GfVec3f* points, VtArray<int>& vertices);

    // get furthest corner
    void GetFurthestCorner(const GfVec3f& point, GfVec3f& corner);

    // build the tree
    void BuildTree(Component* comonents, size_t num, Geometry* geometry);
    void ClearTree();

  private:
    // depth in octree
    int _depth;

    // bounding box
    GfVec3f _min, _max;

    // leaf ?
    bool _isLeaf;

    // children
    Cell* _child[8];

    // elements
    std::vector<Component*> _elements;
  };

protected:
  void _GetClosestCell(const GfVec3f& point, Cell*& closestCell) const;
  void _RecurseGetClosestCell(const GfVec3f& point, const Cell* cell, 
    float& closestDistance, Cell*& closestCell) const;
  void _GetNearbyCells(const GfVec3f& point, const Cell* cell, 
    std::vector<Cell*>& cells, float closestDistance) const;
  void _RecurseGetNearbyCells(const Cell* cell, const GfVec3f& center, 
    const float radius, std::vector<Cell*>& cells) const;

public:
  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;
  virtual bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance=DBL_MAX, double* minDistance=NULL) const override;
  virtual bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance=DBL_MAX) const override;

private:
  // static members
  static const int MAX_ELEMENTS_NUMBER;

  Cell                      _octree;
  std::vector<Component*>   _elements;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_OCTREE_H 