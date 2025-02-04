#include <pxr/base/work/loops.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/tf/stopwatch.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/points.h>
#include "../geometry/voxels.h"
#include "../geometry/location.h"
#include "../geometry/deformable.h"
#include "../geometry/intersection.h"
#include "../acceleration/bvh.h"

JVR_NAMESPACE_OPEN_SCOPE

// constructor
//--------------------------------------------------------------------------------
Voxels::Voxels()
  : Deformable(Geometry::VOXEL, GfMatrix4d(1.0))
  , _geometry(NULL)
  , _radius(1.f)
{
};

// compute num cells
//--------------------------------------------------------------------------------
size_t 
Voxels::GetNumCells()
{
  return 
    static_cast<size_t>(_resolution[0]) * 
    static_cast<size_t>(_resolution[1]) * 
    static_cast<size_t>(_resolution[2]);
}

// compute index in flat vector
//--------------------------------------------------------------------------------
size_t 
Voxels::_ComputeFlatIndex(size_t x, size_t y, size_t z, short axis)
{
  switch(axis) {
    case 0:
      return y * (size_t)_resolution[0] * (size_t)_resolution[1] + x * (size_t)_resolution[0] + z;
    case 1:
      return x * (size_t)_resolution[0] * (size_t)_resolution[1] + z * (size_t)_resolution[0] + y;
    case 2:
      return z * (size_t)_resolution[0] * (size_t)_resolution[1] + y * (size_t)_resolution[0] + x;
    default:
      return z * (size_t)_resolution[0] * (size_t)_resolution[1] + y * (size_t)_resolution[0] + x;
  }
}

// init voxel grid 
//--------------------------------------------------------------------------------
void Voxels::Init(Deformable* geometry, float radius)
{
  _radius = radius;
  _geometry = geometry;
  if (_radius < 0.0001) _radius = 0.0001;
  const GfVec3f size(geometry->GetBoundingBox(true).GetRange().GetSize());
  _resolution = GfVec3i(
    GfClamp(size[0] / _radius, 1, std::numeric_limits<int>::max()),
    GfClamp(size[1] / _radius, 1, std::numeric_limits<int>::max()),
    GfClamp(size[2] / _radius, 1, std::numeric_limits<int>::max())
  );
  size_t numVoxels = GetNumCells();
  _data.resize(numVoxels);
  memset(&_data[0], 0, numVoxels * sizeof(uint8_t));
  // build an aabb tree of the geometry
  _bvh.Init({ _geometry });
}

// trace voxel grid (axis direction)
//--------------------------------------------------------------------------------
void Voxels::_TraceWork(const size_t begin, const size_t end, short axis)
{
  GfBBox3d bbox = _geometry->GetBoundingBox(true);
  const GfRange3d range(bbox.GetRange());
  const GfVec3f size(range.GetSize());
  const GfVec3f minExtents(range.GetMin());

  // this is the bias we apply to step 'off' a triangle we hit, not very robust
  const float eps = 0.000001f * size[axis];
  uint32_t x = begin;

  for (uint32_t y = 0; y < _resolution[(axis + 2) % 3]; ++y)
  {
    bool inside = false;

    GfVec3f rayDir(0.f);
    rayDir[axis] = 1.f;

    GfVec3f rayStart = minExtents;
    rayStart[(axis + 1) % 3] += (x + 0.5f) * _radius;
    rayStart[(axis + 2) % 3] += (y + 0.5f) * _radius;

    while (true)
    {
      // calculate ray start
      Location hit;
      double minDistance = DBL_MAX;
      if (_bvh.Raycast(GfRay(rayStart, rayDir), &hit, DBL_MAX, &minDistance))
      {
        // calculate cell in which intersection occurred
        const float zpos = rayStart[axis] + hit.GetDistance() * rayDir[axis];
        const float zhit = (zpos - minExtents[axis]) / _radius;

        uint32_t z = uint32_t(floorf((rayStart[axis] - minExtents[axis]) / _radius + 0.5f));
        uint32_t zend = std::min(uint32_t(floorf(zhit + 0.5f)), uint32_t(_resolution[axis] - 1));

        if (inside)
        {
          // march along column setting bits 
          for (uint32_t k = z; k < zend; ++k)
            _data[_ComputeFlatIndex(x, y, k, axis)] += 1;
        }

        inside = !inside;

        rayStart += rayDir * (hit.GetDistance() + eps);

      }
      else {
        break;
      }
    }
  }
}

void Voxels::Trace(short axis)
{
  WorkParallelForN(
    _resolution[(axis + 1) % 3], std::bind(
      &Voxels::_TraceWork, 
      this, 
      std::placeholders::_1,     // begin
      std::placeholders::_2,     // end)
      axis
    ), 1); // grain size fixed to 1 eliminating one nested loop  
}

// closest point query voxel grid
//--------------------------------------------------------------------------------
void Voxels::_ProximityWork(size_t begin, size_t end)
{
  const GfVec3f* points = _geometry->GetPositionsCPtr();
  const float threshold = 0.5f * _radius;

  for (size_t cell = begin; cell < end; ++cell) {
    const GfVec3f point = GetCellPosition(cell);
    Location hit;
    if(_bvh.Closest(point, &hit, threshold))
      _data[cell] += 3;
  }
}

void Voxels::Proximity()
{
  WorkParallelForN(GetNumCells(), std::bind(
    &Voxels::_ProximityWork, 
    this,
    std::placeholders::_1,     // begin
    std::placeholders::_2      // end
  ), 32);
}

GfVec3f Voxels::GetCellPosition(size_t cellIdx)
{
  const GfRange3d range(_geometry->GetBoundingBox(true).GetRange());
  size_t x = cellIdx % _resolution[0];
  size_t y = (cellIdx / _resolution[0]) % _resolution[1];
  size_t z = cellIdx / (_resolution[0] * _resolution[1]);

  return GfVec3f(range.GetMin()) + GfVec3f((x + 0.5f) * _radius, (y + 0.5f) * _radius, (z + 0.5f) * _radius);
}

void Voxels::Build(float randomize)
{
  size_t numCells = GetNumCells();
  RemoveAllPoints();
  size_t numHits = 0;
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    if (_data[cellIdx] > 1) {
      const GfVec3f offset(
        RANDOM_0_1 * randomize * _radius,
        RANDOM_0_1 * randomize * _radius,
        RANDOM_0_1 * randomize * _radius);

      AddPoint(GetCellPosition(cellIdx) + offset, 0.1f);
      numHits++;
    }
  }
}

Geometry::DirtyState 
Voxels::_Sync(const GfMatrix4d& matrix, const UsdTimeCode& time)
{
  if(_prim.IsValid() && _prim.IsA<UsdGeomPoints>())
  {
    _previous = _positions;
    UsdGeomPoints usdPoints(_prim);
    const size_t nbPositions = _positions.size();
    usdPoints.GetPointsAttr().Get(&_positions, time);
  }
  return Geometry::DirtyState::DEFORM;
}

void 
Voxels::_Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time)
{
  if(_prim.IsA<UsdGeomPoints>()) {
    UsdGeomPoints usdPoints(_prim);
    usdPoints.CreatePointsAttr().Set(GetPositions(), time);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE
