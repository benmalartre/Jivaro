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
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;
class Mesh;
class Curve;
class Points;
struct Point;
struct Edge;
struct Triangle;

class OctreeIntersector : public Intersector {
  enum ElemType {
    POINT,
    EDGE,
    TRIANGLE,
    POLYGON
  };

protected:
  struct Element {
    Element(void* ptr) : _ptr(ptr) {};
    virtual bool Touch(Geometry* geometry, 
        const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) = 0;
    virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point , 
      Hit* hit, float maxDistance=-1.f) = 0;
    void* _ptr;
  };

  struct PointElement : public Element {
    PointElement(Point* point) : Element((void*)point) {};
    virtual bool Touch(Geometry* geometry,
      const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) override;
    virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& query , 
      Hit* hit, float maxDistance = -1.f);
  };

  struct EdgeElement : public Element {
    EdgeElement(Edge* edge) : Element((void*)edge) {};
    virtual bool Touch(Geometry* geometry,
      const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) override;
    virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& query ,
      Hit* hit, float maxDistance = -1.f);
  };

  struct TriangleElement : public Element {
    TriangleElement(Triangle* triangle) : Element((void*)triangle) {};
    virtual bool Touch(Geometry* geometry,
      const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) override;
    virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& query , 
      Hit* hit, float maxDistance = -1.f);
  };

public: 
  class Cell {

  public:
    Cell() : _depth(0), _min(-1, -1, -1), _max(1, 1, 1), _isLeaf(true)
    {
      for (int i = 0; i < 8; i++) _child[i] = NULL;
    };

    Cell(const pxr::GfVec3f& minp, const pxr::GfVec3f& maxp, int depth = 0) :
      _depth(depth), _isLeaf(true), _min(minp), _max(maxp)
    {
      for (int i = 0; i < 8; i++) _child[i] = NULL;
    };

    ~Cell();

    // depth in octree
    int GetDepth() const { return _depth; };

    // center
    pxr::GfVec3f GetCenter() { return (pxr::GfVec3f((_min + _max) * 0.5f)); };
    pxr::GfVec3f GetHalfSize() { return (pxr::GfVec3f((_max - _min) * 0.5f)); };

    // squared value
    inline float _Squared(float v) const { return v * v; };

    // distance
    inline float GetDistance1D(float p, float lower, float upper) const
    {
      if (p < lower)return lower - p;
      if (p > upper)return p - upper;
      return fmin(p - lower, upper - p);
    };
    float GetDistance(const pxr::GfVec3f& point) const;

    // intersect sphere
    bool IntersectSphere(const pxr::GfVec3f& center, const float radius) const;

    // leaf
    bool IsLeaf() const { return _isLeaf; };

    // get cell
    Cell* GetCell(int x) { return _child[x]; }
    const Cell* GetCell(int x) const { return _child[x]; }

    // triangles size
    int NumElements() const { return _elements.size(); };
    std::vector<Element*>& GetElements() { return _elements; };
    Element* GetElement(unsigned index) { return _elements[index]; };

    // insert a triangle
    void Insert(Element* e) { _elements.push_back(e); };

    // split into 8
    void Split(Geometry* geometry);

    // get bounding box
    void GetBoundingBox(Geometry* geometry, pxr::VtArray<int>& vertices);

    // get furthest corner
    void GetFurthestCorner(const pxr::GfVec3f& point, pxr::GfVec3f& corner);

    // build the tree
    void BuildTree(std::vector<Element>& elements, Geometry* geometry);
    void ClearTree();

  private:
    // depth in octree
    int _depth;

    // bounding box
    pxr::GfVec3f _min, _max;

    // leaf ?
    bool _isLeaf;

    // children
    Cell* _child[8];

    // elements
    std::vector<Element*> _elements;
  };

protected:
  void _GetClosestCell(const pxr::GfVec3f& point, Cell*& closestCell) const;
  void _RecurseGetClosestCell(const pxr::GfVec3f& point, const Cell* cell, 
    float& closestDistance, Cell*& closestCell) const;
  void _GetNearbyCells(const pxr::GfVec3f& point, const Cell* cell, 
    std::vector<Cell*>& cells, float closestDistance) const;
  void _RecurseGetNearbyCells(const Cell* cell, const pxr::GfVec3f& center, 
    const float radius, std::vector<Cell*>& cells) const;

public:
  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update(const std::vector<Geometry*>& geometries) override;
  virtual bool Raycast(const pxr::GfRay& ray, Hit* hit, 
    double maxDistance=-1, double* minDistance=NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Hit* hit, 
    double maxDistance=-1.f, double* minDistance=NULL) const override;

private:
  // static members
  static const int MAX_ELEMENTS_NUMBER;

  Cell                  _octree;
  std::vector<Element*> _elements;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_OCTREE_H 