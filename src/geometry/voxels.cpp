#include <pxr/base/gf/vec3f.h>
#include "../geometry/voxels.h"
#include "../geometry/geometry.h"
#include "../geometry/intersection.h"
#include "../acceleration/bvh.h"

JVR_NAMESPACE_OPEN_SCOPE

// constructor
//--------------------------------------------------------------------------------
Voxels::Voxels()
  : _geometry(NULL)
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
void Voxels::Init(Geometry* geometry, float radius)
{
  _radius = radius;
  _geometry = geometry;
  if (_radius < 0.0001) _radius = 0.0001;
  const pxr::GfVec3f size(geometry->GetBoundingBox().GetRange().GetSize());
  _resolution = pxr::GfVec3i(
    pxr::GfClamp(size[0] / _radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[1] / _radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[2] / _radius, 1, std::numeric_limits<int>::max())
  );
  size_t numVoxels = GetNumCells();
  _data.resize(numVoxels);
  memset(&_data[0], 0, numVoxels * sizeof(uint8_t));

  // build an aabb tree of the geometry
  _bvh.Init({ _geometry });
}

// trace voxel grid (Z direction)
//--------------------------------------------------------------------------------
void Voxels::Trace(short axis)
{
  const pxr::GfRange3d& range(_geometry->GetBoundingBox().GetRange());
  const pxr::GfVec3f size(range.GetSize());
  const pxr::GfVec3f minExtents(range.GetMin());

  const pxr::GfVec3f* points = _geometry->GetPositionsCPtr();

  // this is the bias we apply to step 'off' a triangle we hit, not very robust
  const float eps = 0.00001f * size[axis];

  for (uint32_t x = 0; x < _resolution[(axis + 1) % 3]; ++x)
  {
    for (uint32_t y = 0; y < _resolution[(axis + 2) % 3]; ++y)
    {
      bool inside = false;

      pxr::GfVec3f rayDir(0.f);
      rayDir[axis] = 1.f;

      pxr::GfVec3f rayStart = minExtents;
      rayStart[(axis + 1) % 3] += (x + 0.5f) * _radius;
      rayStart[(axis + 2) % 3] += (y +0.5f) * _radius;

      while (true)
      {
        // calculate ray start
        Hit hit;
        double minDistance = DBL_MAX;
        if (_bvh.Raycast(points, pxr::GfRay(rayStart, rayDir), &hit, DBL_MAX, &minDistance))
        {
          // calculate cell in which intersection occurred
          const float zpos = rayStart[axis] + hit.GetT() * rayDir[axis];
          const float zhit = (zpos - minExtents[axis]) / _radius;

          uint32_t z = uint32_t(floorf((rayStart[axis] - minExtents[axis]) / _radius + 0.5f));
          uint32_t zend = std::min(uint32_t(floorf(zhit + 0.5f)), uint32_t(_resolution[axis] - 1));

          if (inside)
          {
            // march along column setting bits 
            for (uint32_t k = z; k < zend; ++k)
              _data[_ComputeFlatIndex(x, y, k, axis)] = uint32_t(-1);
          }

          inside = !inside;

          rayStart += rayDir * (hit.GetT() + eps);

        }
        else {
          if (inside) {
            // march along column setting bits 
            uint32_t z = uint32_t(floorf((rayStart[2] - minExtents[2]) / _radius + 0.5f));
            for (uint32_t k = z; k < _resolution[2]; ++k)
              _data[_ComputeFlatIndex(x, y, k, axis)] = uint32_t(-1);
          }
          break;
        }
          
      }
    }
  }
}

pxr::GfVec3f Voxels::GetCellPosition(size_t cellIdx)
{
  const pxr::GfRange3d& range = _geometry->GetBoundingBox().GetRange();
  size_t x = cellIdx % _resolution[0];
  size_t y = (cellIdx / _resolution[0]) % _resolution[1];
  size_t z = cellIdx / (_resolution[0] * _resolution[1]);

  return pxr::GfVec3f(range.GetMin()) + pxr::GfVec3f((x + 0.5f) * _radius, (y + 0.5f) * _radius, (z + 0.5f) * _radius);
}

void Voxels::Build()
{
  size_t numCells = GetNumCells();
  _positions.reserve(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    if (_data[cellIdx]) {
      _positions.push_back(GetCellPosition(cellIdx));
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE
