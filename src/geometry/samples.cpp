#include <map>
#include <unordered_map>
#include "../geometry/samples.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

pxr::GfVec3f 
Sample::GetPosition(const pxr::GfVec3f* positions) const
{
  return 
    positions[elemIdx[0]] * baryWeights[0] +
    positions[elemIdx[1]] * baryWeights[1] +
    positions[elemIdx[2]] * baryWeights[2];
}

pxr::GfVec3f
Sample::GetNormal(const pxr::GfVec3f* normals) const
{
  return
    normals[elemIdx[0]] * baryWeights[0] +
    normals[elemIdx[1]] * baryWeights[1] +
    normals[elemIdx[2]] * baryWeights[2];
}

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
  for(int triIdx = 0; triIdx < triangles.size(); triIdx += 3)
  {
    int vertIdx0 = triangles[triIdx+0];
    int vertIdx1 = triangles[triIdx+1];
    int vertIdx2 = triangles[triIdx+2];
    const pxr::GfVec3f &v0 = points[vertIdx0];
    const pxr::GfVec3f &v1 = points[vertIdx1];
    const pxr::GfVec3f &v2 = points[vertIdx2];
    triArea.push_back(_TriangleArea(v0, v1, v2));
  }

  // Map the sums of the areas to a triangle index.  For example, if we have triangles with areas
  // 2, 7, 1 and 4, create:
  //
  // 0: 0
  // 2: 1
  // 9: 2
  // 10: 3
  //
  // A random number in 0...14 can then be mapped to an index.
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
    int triIdx = it->second;

    // Select a random point on the triangle.
    float u = RANDOM_0_1;
    float v = RANDOM_0_1;

    samples.emplace_back();
    Sample& sample = samples.back();
    sample.elemIdx = pxr::GfVec3i(
      triangles[triIdx * 3],
      triangles[triIdx * 3 + 1], 
      triangles[triIdx * 3 + 2]);
    sample.baryWeights = 
      pxr::GfVec3f(1 - sqrt(u), sqrt(u) * (1 - v), v * sqrt(u));
  }

  return maxAreaSum;
}

void _PoissonDiskFromSamples(Geometry* geometry,
                             float radius,
                             pxr::VtArray<Sample>& samples,
                             pxr::VtArray<Sample>& results)
{
  // Get the bounding box of the samples.
  pxr::GfRange3d bbox;
  const pxr::GfVec3f* positions = geometry->GetPositionsCPtr();
  const pxr::GfVec3f* normals = geometry->GetPositionsCPtr();
  for (auto& sample : samples) {
    bbox.Union(sample.GetPosition(positions));
  }
  const pxr::GfVec3f bboxSize(bbox.GetSize());

  const float radiusSq = radius*radius;
  pxr::GfVec3f gridSize = bboxSize / radius;
  pxr::GfVec3i gridSizeI(pxr::GfFloor(gridSize[0]), pxr::GfFloor(gridSize[1]), pxr::GfFloor(gridSize[2]));
  gridSizeI[0] = pxr::GfMax(gridSizeI[0], 1);
  gridSizeI[1] = pxr::GfMax(gridSizeI[1], 1);
  gridSizeI[2] = pxr::GfMax(gridSizeI[2], 1);

  // Assign a cell ID to each sample.
  for(auto &sample: samples)
  {
    /*
    pxr::GfVec3i idx = bbox.index_grid_cell(grid_size_int, p.pos);
    Idx3_cu offset(grid_size_int, idx);
    p.cell_id = offset.to_linear();
    */
    sample.cellIdx = 0;
  }

  // Sort samples by cell ID.
  std::sort(samples.begin(), samples.end(), [](const Sample &lhs, const Sample&rhs) {
      return lhs.cellIdx < rhs.cellIdx;
  });

  // Map from cell IDs to hash_data.  Each hash_data points to the range in raw_samples corresponding to that cell.
  // (We could just store the samples in hash_data.  This implementation is an artifact of the reference paper, which
  // is optimizing for GPU acceleration that we haven't implemented currently.)
  std::unordered_map<int, HashData> cells;

  {
    int lastIdx = -1;
    std::unordered_map<int, HashData>::iterator lastIdIt;

    for(int sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx)
    {
      const auto &sample = samples[sampleIdx];
      if(sample.cellIdx == lastIdx)
      {
        // This sample is in the same cell as the previous, so just increase the count.  Cells are
        // always contiguous, since we've sorted raw_samples by cell ID.
        ++lastIdIt->second.sampleCnt;
        continue;
      }

      // This is a new cell.
      HashData data;
      data.firstSampleIdx = sampleIdx;
      data.sampleCnt = 1;

      auto result = cells.insert({sample.cellIdx, data});
      lastIdx = sample.cellIdx;
      lastIdIt = result.first;
    }
  }

  // Make a list of offsets to neighboring cell indexes, and to ourself (0).
  std::vector<int> neighborCellOffsets;
  {
    /*
    pxr::GfVec3i offset(grid_size_int, 0);
    for(int x = -1; x <= +1; ++x)
    {
      for(int y = -1; y <= +1; ++y)
      {
        for(int z = -1; z <= +1; ++z)
        {
          offset.set_3d(x, y, z);
          neighborCellOffsets.push_back(offset.to_linear());
        }
      }
    }
    */
  }
 

  int maxTrials = 5;
  for(int trial = 0; trial < maxTrials; ++trial)
  {
    // Create sample points for each entry in cells.
    for(auto &it: cells)
    {
      int cellIdx = it.first;
      HashData &data = it.second;

      // This cell's raw sample points start at first_sample_idx.  On trial 0, try the first one.
      // On trial 1, try first_sample_idx + 1.
      int nextSampleIdx = data.firstSampleIdx + trial;
      if(trial >= data.sampleCnt)
      {
        // There are no more points to try for this cell.
        continue;
      }
      const auto &candidate = samples[nextSampleIdx];

      // See if this point conflicts with any other points in this cell, or with any points in
      // neighboring cells.  Note that it's possible to have more than one point in the same cell.
      bool conflict = false;
      for(int neighborCellOffset: neighborCellOffsets)
      {
        int neighborCellId = cellIdx + neighborCellOffset;
        const auto &it = cells.find(neighborCellId);
        if(it == cells.end())
            continue;

        const HashData &neighbor = it->second;
        for(const auto &sample: neighbor.samples)
        {
          float distance = _ApproximateGeodesicDistance(
            sample.GetPosition(positions), 
            candidate.GetPosition(positions),
            sample.GetNormal(normals), 
            candidate.GetNormal(normals));
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
      results.push_back(sample);
    }
  }
}


void
PoissonSampling(float radius, int nbSamples,
  const pxr::VtArray<pxr::GfVec3f>& points,
  const pxr::VtArray<pxr::GfVec3f>& normals,
  const pxr::VtArray<int>& triangles,
  pxr::VtArray<Sample>& samples)
{

}


JVR_NAMESPACE_CLOSE_SCOPE
