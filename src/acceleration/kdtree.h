#ifndef JVR_ACCELERATION_KDTREE_H
#define JVR_ACCELERATION_KDTREE_H

#include <vector>
#include <pxr/base/gf/vec3f.h>
#include "../acceleration/intersector.h"

// https://github.com/YevgeniyEngineer/KDTree/tree/main


JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class KDTree : public Intersector
{
  struct Cell;

public:
  using IndexDistance = std::pair<size_t, float>; // Index + Distance

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
  inline float _DistanceSq(const pxr::GfVec3f &p1, const pxr::GfVec3f &p2) const noexcept
  {
    return (p1 - p2).GetLengthSq();
  }

  struct CompareDistances
  {
    inline bool operator()(const IndexDistance &d1, const IndexDistance &d2) const noexcept
    {
      return (d1.second > d2.second);
    }
  };

 struct Cell final
  {
    explicit Cell(const pxr::GfVec3f &point, size_t index)
      : point(point), index(index), left(nullptr), right(nullptr)
    {
    }
    ~Cell()
    {
      left = nullptr;
      right = nullptr;
    }

    pxr::GfVec3f  point;
    size_t        index;
    Cell          *left = nullptr;
    Cell          *right = nullptr;
  };

  Cell* _BuildTreeRecursively(std::vector<Cell>::iterator begin,
    std::vector<Cell>::iterator end, std::size_t index) const;

  void _RecurseClosest(const Cell *cell, const pxr::GfVec3f &point, 
    size_t index, double &minDistanceSq, Cell *&nearest) const;

  Cell*                           _root = nullptr;
  std::vector<Cell>               _cells;
  size_t                          _numComponents;
}; 

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_KDTREE_H