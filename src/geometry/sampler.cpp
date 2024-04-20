#include <map>
#include <unordered_map>
#include <cassert>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/range3f.h>
#include "../geometry/sampler.h"

JVR_NAMESPACE_OPEN_SCOPE

// Estimate the geodesic distance between two points using their position and normals.
// See c95-f95_199-a16-paperfinal-v5 "3.2 Sampling with Geodesic Distance".  This estimation
// assumes that we have a smooth gradient between the two points, since it doesn't have any
// information about complex surface changes between the points.
float _ApproximateGeodesicDistance(const pxr::GfVec3f& p1, const pxr::GfVec3f& p2, 
  const pxr::GfVec3f& n1, const pxr::GfVec3f& n2)
{
#if 0
    return (p1 - p2).GetLengthSq();
#else
    pxr::GfVec3f v = p2-p1;
    v.Normalize();

    float c1 = n1 * v;
    float c2 = n2 * v;
    float result = (p1 - p2).GetLengthSq();
    // Check for division by zero:
    if(fabs(c1 - c2) > 0.0001)
        result *= (asin(c1) - asin(c2)) / (c1 - c2);
    return result;
#endif
}

float _TriangleArea(const pxr::GfVec3f &a, const pxr::GfVec3f &b, const pxr::GfVec3f &c)
{
  float ab = (a - b).GetLength();
  float bc = (b - c).GetLength();
  float ca = (c - a).GetLength();
  float p = (ab + bc + ca) / 2;
  return pxr::GfSqrt(p * (p - ab) * (p - bc) * (p - ca));
}


// Do a simple random sampling of the triangles.  Return the total surface area of the triangles.
float _CreateRawSamples(int numSamples,
                        const pxr::VtArray<pxr::GfVec3f>& points,
                        const pxr::VtArray<pxr::GfVec3f>& normals,
                        const pxr::VtArray<int>& triangles,
                        pxr::VtArray<Sample> &samples)
{
  // Calculate the area of each triangle.  We'll use this to randomly select triangles with probability proportional
  // to their area.
  std::vector<float> triArea;
  for(size_t triIdx = 0; triIdx < triangles.size() / 3; ++triIdx)
  {
    triArea.push_back(
      _TriangleArea(
        points[triangles[triIdx * 3 + 0]], 
        points[triangles[triIdx * 3 + 1]], 
        points[triangles[triIdx * 3 + 2]]
      ));
  }

  // Map the sums of the areas to a triangle index
  std::map<float, int> areaSumToIndex;
  float maxAreaSum = 0;
  for(int triIdx = 0; triIdx < triArea.size(); ++triIdx)
  {
      areaSumToIndex[maxAreaSum] = triIdx;
      maxAreaSum += triArea[triIdx];
  }

  for(int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
  {
    // Select a random triangle.
    float r = RANDOM_0_X(maxAreaSum);
    auto it = areaSumToIndex.upper_bound(r);
    assert(it != areaSumToIndex.begin());
    --it;
    size_t triIdx = it->second;

    // Select a random point on the triangle.
    float u = RANDOM_0_1;
    float v = RANDOM_0_1;

    samples.emplace_back();
    Sample& sample = samples.back();
    sample.elemIdx = pxr::GfVec3i(
      triangles[triIdx * 3 + 0],
      triangles[triIdx * 3 + 1], 
      triangles[triIdx * 3 + 2]);
    sample.baryWeights = 
      pxr::GfVec3f(1 - sqrt(u), sqrt(u) * (1 - v), v * sqrt(u));
  }

  return maxAreaSum;
}

static pxr::GfVec3i
_GetGridCoords(const pxr::GfRange3d& range, const pxr::GfVec3d& point, const pxr::GfVec3i dimensions)
{
  const pxr::GfVec3d& min = range.GetMin();
  const pxr::GfVec3d& size = range.GetSize();
  const pxr::GfVec3d scale = pxr::GfVec3d(
    size[0]/dimensions[0], 
    size[1]/dimensions[1], 
    size[2]/dimensions[2]
  );
  return pxr::GfVec3i(
    (int)((point[0] - min[0]) * 1.f / scale[0]),
    (int)((point[1] - min[1]) * 1.f / scale[1]),
    (int)((point[2] - min[2]) * 1.f / scale[2])
  );
}

static uint32_t
_GetGridIndex(const pxr::GfVec3i& coords, const pxr::GfVec3i& dimensions)
{
  return coords[2] * dimensions[0] * dimensions[1] + coords[1] * dimensions[0] + coords[0];
}

void _PoissonDiskFromSamples(const pxr::GfVec3f* positions,
                            const pxr::GfVec3f* normals,
                            float radius,
                            pxr::VtArray<Sample>& seeds,
                            pxr::VtArray<Sample>& samples)
{
  // Get the bounding box of the samples.
  pxr::GfRange3d bbox;
  for (auto& seed : seeds) {
    bbox.Union(seed.GetPosition(positions));
  }
  const pxr::GfVec3f bboxSize(bbox.GetSize());

  const float radiusSq = radius * radius;
  pxr::GfVec3f gridSize = bboxSize / radius;
  pxr::GfVec3i gridSizeI(pxr::GfFloor(gridSize[0]), pxr::GfFloor(gridSize[1]), pxr::GfFloor(gridSize[2]));
  gridSizeI[0] = pxr::GfMax(gridSizeI[0], 1);
  gridSizeI[1] = pxr::GfMax(gridSizeI[1], 1);
  gridSizeI[2] = pxr::GfMax(gridSizeI[2], 1);

  // Assign a cell ID to each seed.
  for(auto &seed: seeds)
  {
    pxr::GfVec3i coords = _GetGridCoords(bbox, seed.GetPosition(positions), gridSizeI);
    seed.cellIdx = _GetGridIndex(coords, gridSizeI);
  }

  // Sort seeds by cell ID.
  std::sort(seeds.begin(), seeds.end(), [](const Sample &lhs, const Sample&rhs) {
      return lhs.cellIdx < rhs.cellIdx;
  });

  // Map from cell IDs to Cell.
  std::unordered_map<int, Cell> cells;

  {
    int lastIdx = -1;
    std::unordered_map<int, Cell>::iterator lastIdIt;

    for(int seedIdx = 0; seedIdx < seeds.size(); ++seedIdx)
    {
      const auto &seed = seeds[seedIdx];
      if(seed.cellIdx == lastIdx)
      {
        ++lastIdIt->second.sampleCnt;
        continue;
      }

      // This is a new cell.
      Cell data;
      data.firstSampleIdx = seedIdx;
      data.sampleCnt = 1;

      auto result = cells.insert({seed.cellIdx, data});
      lastIdx = seed.cellIdx;
      lastIdIt = result.first;
    }
  }

  // Make a list of offsets to neighboring cell indexes, and to ourself (0).
  std::vector<int> neighborCellOffsets;
  {
    for(int x = -1; x <= +1; ++x)
    {
      for(int y = -1; y <= +1; ++y)
      {
        for(int z = -1; z <= +1; ++z)
        {
          neighborCellOffsets.push_back(_GetGridIndex(pxr::GfVec3i(x, y, z), gridSizeI));
        }
      }
    }
  }
  

  int maxTrials = 5;
  for(int trial = 0; trial < maxTrials; ++trial)
  {
    // Create sample points for each entry in cells.
    for(auto &it: cells)
    {
      int cellIdx = it.first;
      Cell &data = it.second;

      // This cell's  sample points start at firstSampleIdx.  On trial 0, try the first one.
      // On trial 1, try firstSampleIdx + 1.
      int nextSampleIdx = data.firstSampleIdx + trial;
      if(trial >= data.sampleCnt)
      {
        // There are no more points to try for this cell.
        continue;
      }
      const auto &candidate = seeds[nextSampleIdx];

      // See if this point conflicts with any other points in this cell, or with any points in
      // neighboring cells.  Note that it's possible to have more than one point in the same cell.
      bool conflict = false;
      for(int neighborCellOffset: neighborCellOffsets)
      {
        int neighborCellId = cellIdx + neighborCellOffset;
        const auto &it = cells.find(neighborCellId);
        if(it == cells.end())
            continue;

        const Cell &neighbor = it->second;
        for(const auto &sample: neighbor.samples)
        {
          float distance = _ApproximateGeodesicDistance(
            sample.GetPosition(positions), 
            candidate.GetPosition(positions),
            sample.GetNormal(normals), 
            candidate.GetNormal(normals)
          );
          if(distance < radiusSq)
          {
            // The candidate is too close to this existing sample.
            conflict = true;
            break;
          }
        }
        if(conflict)
          break;
      }

      if(conflict)
        continue;

      // Store the new sample.
      data.samples.push_back(candidate);
    }
  }

  // Copy the results to the output.
  for(const auto it: cells)
  {
    for(const auto &sample: it.second.samples)
    {
      samples.push_back(sample);
    }
  }
}

void
  PoissonSampling(float radius, int nbSamples,
    const pxr::VtArray<pxr::GfVec3f>& points,
    const pxr::VtArray<pxr::GfVec3f>& normals,
    const pxr::VtArray<Triangle>& triangles,
    pxr::VtArray<Sample>& samples)
{
  /*
  pxr::VtArray<Sample> seeds;
  pxr::VtArray<int> indices(triangles.size() * 3);
  for (size_t triIdx = 0; triIdx < triangles.size(); ++triIdx) {
    memcpy(&indices[triIdx * 3], &triangles[triIdx].vertices, sizeof(pxr::GfVec3i));
  }
  float surfaceArea = _CreateRawSamples(nbSamples, points, normals, indices, seeds);

  if (radius <= 0.f) {
    radius = std::sqrtf(surfaceArea / nbSamples) * 0.75f;
  }

  _PoissonDiskFromSamples(&points[0], &normals[0], radius, seeds, samples);
  */
  samples.resize(nbSamples);
  for (size_t sampleIdx = 0; sampleIdx < nbSamples; ++sampleIdx) {
    float u = RANDOM_0_1;
    float v = RANDOM_0_1;
    size_t triIndex = RANDOM_0_X(triangles.size() - 1);
    samples[sampleIdx] = { 
      triangles[triIndex].vertices, 
      0, 
      pxr::GfVec3f(u, v, 1 - (u+v))
    };
  }

} // namespace Sample

JVR_NAMESPACE_CLOSE_SCOPE
