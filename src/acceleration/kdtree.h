#ifndef JVR_ACCELERATION_KDTREE_H
#define JVR_ACCELERATION_KDTREE_H

#include <vector>
#include <queue>
#include <pxr/base/gf/vec3f.h>
#include "../acceleration/intersector.h"
#include "../acceleration/distance.h"


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class KDTree : public Intersector
{
public:

  struct IndexPoint {
    size_t        index;
    pxr::GfVec3f  position;

    explicit IndexPoint(size_t index, const pxr::GfVec3f& position)
      : index(index), position(position) {};
  };
  
  struct Cell : public pxr::GfRange3d
  {
    explicit Cell(const IndexPoint &point=IndexPoint(INVALID_INDEX, pxr::GfVec3f(0.f)))
      : point(point), left(INVALID_INDEX), right(INVALID_INDEX)
    {
    }
    ~Cell()
    {
    }

    short             axis;
    IndexPoint        point;
    size_t            left = INVALID_INDEX;
    size_t            right = INVALID_INDEX;
  };

  using IndexDistance = std::pair<size_t, float>; // Index + Distance


  // helper class for priority queue in k nearest neighbor search
  struct NN4Heap {
    size_t index;       // index of actual cell in _cells
    double distance;    // distance of this neighbor from query point

    (NN4Heap)(size_t i, double d) 
      : index(i), distance(d) {};

    inline bool operator<(const NN4Heap& other) {
      return distance < other.distance;
    };
  };

  typedef std::priority_queue<NN4Heap, std::vector<NN4Heap>> KNNSearchQueue;

  KDTree(DistanceType distanceType=DistanceType::EUCLIDEAN) 
    : _root(nullptr), _distanceType(distanceType), _distance(nullptr) {};
  ~KDTree() {delete _distance;};
  
  Cell* GetRoot() { return _root; };
  const Cell* GetRoot() const { return _root; };

  Cell* GetCell(size_t index) { return &_cells[index]; };
  const Cell* GetCell(size_t index) const { return &_cells[index]; };

  size_t AddCell(const IndexPoint& point=IndexPoint(INVALID_INDEX, pxr::GfVec3f(0.f)));

  const Geometry* GetGeometryFromCell(const Cell* cell) const;
  size_t GetGeometryIndexFromCell(const Cell* cell) const;

  // infos
  size_t GetNumComponents(){return _numComponents;};
  size_t GetNumCells(){return _cells.size();};

  // overrides Intersector
  virtual void Init(const std::vector<Geometry*>& geometries) override;
  virtual void Update() override;

  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = DBL_MAX, double* minDistance = NULL) const override;
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance) const override;

private:
  struct _CompareDimension {
    _CompareDimension(size_t dim) { d = dim; }
    inline bool operator()(const IndexPoint& lhs, const IndexPoint& rhs) const noexcept {
      return (lhs.position[d] < rhs.position[d]);
    }
    size_t d;
  };

  struct _CompareDistances
  {
    inline bool operator()(const IndexDistance &d1, const IndexDistance &d2) const noexcept {
      return (d1.second > d2.second);
    }
  };

  bool _ContainsSphere(const pxr::GfVec3f& center, double radius, KDTree::Cell *cell) ;
  bool _IntersectSphere(const pxr::GfVec3f& center, double radius, KDTree::Cell *cell);

  size_t _GetIndex(const Cell* cell) const;

  size_t _BuildTreeRecursively(const pxr::GfRange3d& range, 
    size_t depth, size_t begin, size_t end);

  void _RecurseClosest(const Cell *cell, const pxr::GfVec3f &point, 
    size_t index, double &minDistanceSq, Cell *&nearest) const;

  Cell*                           _root = nullptr;
  std::vector<IndexPoint>         _points;
  std::vector<Cell>               _cells;
  size_t                          _numComponents;
  DistanceType                    _distanceType;
  DistanceMeasure*                _distance;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_KDTREE_H