#ifndef JVR_ACCELERATION_KDTREE_H
#define JVR_ACCELERATION_KDTREE_H

#include <vector>
#include <pxr/base/gf/vec3f.h>
#include "../acceleration/intersector.h"

// https://github.com/YevgeniyEngineer/KDTree/tree/main (maybe this is not working :D)

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
    explicit Cell(const IndexPoint *point = nullptr)
      : point(point), left(nullptr), right(nullptr)
    {
    }
    ~Cell()
    {
      left = nullptr;
      right = nullptr;
    }

    short             axis;
    const IndexPoint  *point;
    Cell              *left = nullptr;
    Cell              *right = nullptr;
  };


  using IndexDistance = std::pair<size_t, float>; // Index + Distance
  static const size_t MAX_NUM_POINTS = 32;

  KDTree &operator=(const KDTree &other) = delete;
  KDTree(const KDTree &other) = delete;
  KDTree &operator=(KDTree &&other) noexcept = default;
  KDTree(KDTree &&other) noexcept = default;

  KDTree() : _root(nullptr) {};
  ~KDTree() {};
  

  Cell* GetRoot() { return _root; };
  const Cell* GetRoot() const { return _root; };

  Cell* GetCell(size_t index) { return &_cells[index]; };
  const Cell* GetCell(size_t index) const { return &_cells[index]; };

  Cell* AddCell(const IndexPoint* point = nullptr);

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
    bool operator()(const IndexPoint& lhs, const IndexPoint& rhs) {
      return (lhs.position[d] < rhs.position[d]);
    }
    size_t d;
  };

  struct CompareDistances
  {
    inline bool operator()(const IndexDistance &d1, const IndexDistance &d2) const noexcept
    {
      return (d1.second > d2.second);
    }
  };

 
  size_t _GetIndex(const Cell* cell) const;

  Cell* _BuildTreeRecursively(const pxr::GfRange3d& range, 
    size_t depth, size_t begin, size_t end);

  void _RecurseClosest(const Cell *cell, const pxr::GfVec3f &point, 
    size_t index, double &minDistanceSq, Cell *&nearest) const;

  Cell*                           _root = nullptr;
  std::vector<IndexPoint>         _points;
  std::vector<Cell>               _cells;
  size_t                          _numComponents;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_KDTREE_H