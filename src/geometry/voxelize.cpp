#include <pxr/base/gf/vec3f.h>
#include "../geometry/voxelize.h"
#include "../geometry/geometry.h"
#include "../geometry/intersection.h"
#include "../acceleration/bvh.h"

JVR_NAMESPACE_OPEN_SCOPE

// constructor
//--------------------------------------------------------------------------------
Voxels::Voxels(Geometry* geometry, float radius)
  : _geometry(geometry)
  , _radius(radius)
{
  if (_radius < 0.0001) _radius = 0.0001;
  const pxr::GfVec3f size(geometry->GetBoundingBox().GetRange().GetSize());
  _resolution = pxr::GfVec3i(
    pxr::GfClamp(size[0] / radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[1] / radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[2] / radius, 1, std::numeric_limits<int>::max())
  );
  size_t numVoxels = GetNumCells();
  _data.resize(numVoxels);
  memset(&_data[0], 0, numVoxels * sizeof(uint8_t));
};

// compute num cells
//--------------------------------------------------------------------------------
size_t Voxels::GetNumCells()
{
  return 
    static_cast<size_t>(_resolution[0]) * 
    static_cast<size_t>(_resolution[1]) * 
    static_cast<size_t>(_resolution[2]);
}

size_t Voxels::_ComputeFlatIndex(size_t x, size_t y, size_t z)
{
  return z * (size_t)_resolution[0] * (size_t)_resolution[1] + y * (size_t)_resolution[0] + x;
}

// trace voxel grid (Z direction)
//--------------------------------------------------------------------------------
void Voxels::Trace()
{
  const pxr::GfRange3d& range(_geometry->GetBoundingBox().GetRange());
  const pxr::GfVec3f size(range.GetSize());
  const pxr::GfVec3f minExtents(range.GetMin());
  const pxr::GfVec3i resolution(
    pxr::GfClamp(size[0] / _radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[1] / _radius, 1, std::numeric_limits<int>::max()),
    pxr::GfClamp(size[2] / _radius, 1, std::numeric_limits<int>::max())
  );
  const pxr::GfVec3f delta(_radius);
  const pxr::GfVec3f offset(_radius * 0.5f);

  // build an aabb tree of the mesh
  BVH tree;
  tree.Init({ _geometry });
  const pxr::GfVec3f* points = _geometry->GetPositionsCPtr();

  // this is the bias we apply to step 'off' a triangle we hit, not very robust
  const float eps = 0.00001f * size[2];

  for (uint32_t x = 0; x < resolution[0]; ++x)
  {
    for (uint32_t y = 0; y < resolution[1]; ++y)
    {
      bool inside = false;

      pxr::GfVec3f rayDir(0.0f, 0.0f, 1.0f);
      pxr::GfVec3f rayStart = minExtents + pxr::GfVec3f(x * delta[0] + offset[0], y * delta[1] + offset[1], 0.0f);

      while (true)
      {
        // calculate ray start
        Hit hit;
        double minDistance = DBL_MAX;
        if (tree.Raycast(points, pxr::GfRay(rayStart, rayDir), &hit, DBL_MAX, &minDistance))
        {
          // calculate cell in which intersection occurred
          const float zpos = rayStart[2] + hit.GetT() * rayDir[2];
          const float zhit = (zpos - minExtents[2]) / delta[2];

          uint32_t z = uint32_t(floorf((rayStart[2] - minExtents[2]) / delta[2] + 0.5f));
          uint32_t zend = std::min(uint32_t(floorf(zhit + 0.5f)), uint32_t(resolution[2] - 1));

          if (inside)
          {
            // march along column setting bits 
            for (uint32_t k = z; k < zend; ++k)
              _data[_ComputeFlatIndex(x, y, k)] =
              uint32_t(-1);
          }

          inside = !inside;

          rayStart += rayDir * (hit.GetT() + eps);

        }
        else
          break;
      }
    }
  }
}

pxr::GfVec3f Voxels::GetCellPosition(size_t cellIdx)
{
  const pxr::GfRange3d& range = _geometry->GetBoundingBox().GetRange();
  size_t x = cellIdx % _resolution[0];
  size_t y = (cellIdx / _resolution[0]) % _resolution[1];
  size_t z = cellIdx / (_resolution[1] * _resolution[2]);

  std::cout << "flat index : " << cellIdx << std::endl;
  std::cout << "   ----> " << x << "," << y << "," << z << std::endl;

  return pxr::GfVec3f(range.GetMin()) + pxr::GfVec3f(x * _radius, y * _radius, z * _radius);
}

void Voxels::Build(pxr::VtArray<pxr::GfVec3f>& points)
{
  size_t numCells = GetNumCells();
  points.reserve(numCells);
  for (size_t cellIdx = 0; cellIdx < numCells; ++cellIdx) {
    if (_data[cellIdx]) {
      points.push_back(GetCellPosition(cellIdx));
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE
