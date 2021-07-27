#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/ray.h>
#include "../acceleration/bvh.h"
#include "../geometry/geometry.h"

AMN_NAMESPACE_OPEN_SCOPE

// distance
static double _GetDistance1D(double value, double lower, double upper)
{
  if (value < lower)return lower - value;
  if (value > upper)return value - upper;
  return pxr::GfMin(value - lower, upper - value);
};

// get distance
static double _GetDistance(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  pxr::GfVec3d half = lhs->GetSize() * 0.5;
  pxr::GfVec3d center = lhs->GetMidpoint();
  const pxr::GfVec3d& minimum = rhs->GetMin();
  const pxr::GfVec3d& maximum = rhs->GetMax();
  float dx = _GetDistance1D(center[0], minimum[0] + half[0], maximum[0] - half[0]);
  float dy = _GetDistance1D(center[1], minimum[1] + half[1], maximum[1] - half[1]);
  float dz = _GetDistance1D(center[2], minimum[2] + half[2], maximum[2] - half[2]);
  return pxr::GfSqrt(dx * dx + dy * dy + dz * dz);
}

static pxr::GfVec3d _GetBoundingMinimum(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs).GetMin();
}

static pxr::GfVec3d _GetBoundingMaximum(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs).GetMax();
}

static pxr::GfRange3d _GetBoundingBox(const pxr::GfRange3d* lhs, const pxr::GfRange3d* rhs)
{
  return pxr::GfRange3d::GetUnion(*lhs, *rhs);
}

BVH::Leaf::Leaf(Geometry* geom)
{
  const pxr::GfBBox3d& bbox = _geom->GetBoundingBox();
  SetMin(pxr::GfVec3d(bbox.GetRange().GetMin()));
  SetMax(pxr::GfVec3d(bbox.GetRange().GetMax()));
  _geom = geom;
}

BVH::Branch::Branch(BVH::Cell* cell)
{
  SetMin(cell->GetMin());
  SetMax(cell->GetMax());
  SetLeft(cell);
  SetRight(NULL);
}

BVH::Branch::Branch(BVH::Cell* lhs, BVH::Cell* rhs)
{
  SetMin(_GetBoundingMinimum(lhs, rhs));
  SetMax(_GetBoundingMaximum(lhs, rhs));
  SetLeft(lhs);
  SetRight(rhs);
}

bool BVH::Branch::Raycast(const pxr::GfRay& ray, Hit* hit, double maxDistance, double* minDistance) const
{
  double enterDistance, exitDistance;
  if (ray.Intersect(*this, &enterDistance, &exitDistance)) {
    Hit leftHit(*hit), rightHit(*hit);
    if (_left)_left->Raycast(ray, &leftHit);
    if (_right)_right->Raycast(ray, &rightHit);
    if (leftHit.GetGeometry() && rightHit.GetGeometry()) {
      if (leftHit.GetT() < rightHit.GetT()) {
        hit->Set(leftHit); return true;
      } else {
        hit->Set(rightHit);; return true;
      }
    } else if (leftHit.GetGeometry()) {
      hit->Set(leftHit); return true;
    } else if (rightHit.GetGeometry()) {
      hit->Set(rightHit); return true;
    } else {
      return true;
    }
  }
  return false;
}

bool BVH::Leaf::Raycast(const pxr::GfRay& ray, Hit* hit,
  double maxDistance, double* minDistance) const
{
  double distance;
  if (ray.Intersect(*this, &distance)) {
    if (_geom->Raycast(ray, hit, maxDistance, minDistance)) {
      return true;
    }
  }
  return false;
}

static void _SortGeometriesByPair(std::vector<BVH::Cell*>& cells,
  std::vector<BVH::Cell*>& results)
{
  size_t numCells = cells.size();
  std::vector<short> used;
  used.resize(numCells);
  memset(&used[0], 0x0, numCells * sizeof(bool));

  for (size_t i = 0; i < numCells; ++i) {
    if (used[i]) continue;
    BVH::Cell* cell = cells[i];
    float min_distance = std::numeric_limits<float>::max();
    int closest_idx = -1;
    for (size_t j = 0; j < numCells; ++j) {
      BVH::Cell* other = cells[j];
      if (i == j)continue;
      float distance = _GetDistance(cell, other);
      if (distance < min_distance && !used[j]) {
        min_distance = distance;
        closest_idx = j;
      }
    }

    if (closest_idx >= 0) {
      used[closest_idx] = true;
      used[i] = true;
      results.push_back(new BVH::Branch(cell, cells[closest_idx]));
    }
    else {
      used[i] = true;
      results.push_back(new BVH::Branch(cell));
    }
  }
}

void BVH::Init(const std::vector<Geometry*>& geometries)
{
  std::vector<Cell*> leaves;
  leaves.reserve(geometries.size());
  for (Geometry* geom : geometries) {
    leaves.push_back(new BVH::Leaf(geom));
  }
  std::vector<BVH::Cell*> cells = leaves;
  std::vector<BVH::Cell*> results;
  _SortGeometriesByPair(leaves, results);
  while (results.size() > 2) {
    cells = results;
    results.clear();
    _SortGeometriesByPair(cells, results);
  }
  root = new BVH::Branch(results[0], results[1]);
  leaves.clear();
}

bool BVH::Raycast(const pxr::GfRay& ray, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  return root->Raycast(ray, hit, maxDistance, minDistance);
}

bool BVH::Closest(const pxr::GfVec3f& point, Hit* hit, 
  double maxDistance, double* minDistance) const
{
  return false;
}

void BVH::Update(const std::vector<Geometry*>& geometries)
{

}
AMN_NAMESPACE_CLOSE_SCOPE