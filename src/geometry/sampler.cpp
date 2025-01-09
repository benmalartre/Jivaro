#include <map>
#include <unordered_map>
#include <cassert>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/range3f.h>
#include "../geometry/sampler.h"

JVR_NAMESPACE_OPEN_SCOPE

GfVec3f 
Sample::GetPosition(const GfVec3f* positions) const
{
  return 
    positions[elemIdx[0]] * baryWeights[0] +
    positions[elemIdx[1]] * baryWeights[1] +
    positions[elemIdx[2]] * baryWeights[2];
}

GfVec3f
Sample::GetNormal(const GfVec3f* normals) const
{
  return(
    normals[elemIdx[0]] * baryWeights[0] +
    normals[elemIdx[1]] * baryWeights[1] +
    normals[elemIdx[2]] * baryWeights[2]
    );// .GetNormalized();
}

GfVec3f
Sample::GetTangent(const GfVec3f* positions, const GfVec3f* normals) const
{
  /*
  GfVec3f normal(normals[elemIdx[0]] * baryWeights[0] +
    normals[elemIdx[1]] * baryWeights[1] +
    normals[elemIdx[2]] * baryWeights[2]);
  normal.Normalize();

  GfVec3f side(0, 0, 1);
  if ((1.f - GfAbs(normal * side)) < 0.0)
  {
    side = GfVec3f(0, 1, 0);
    if ((1.f - GfAbs(normal * side)) < DOT_EPSILON)
    {
      side = GfVec3f(1, 0, 0);
    }
  }
  
  return (normal ^ side).GetNormalized();
  */
  return GetNormal(normals) ^ GfVec3f(0, 1, 0);


  /*
  const GfVec3f e0 = positions[elemIdx[1]] - positions[elemIdx[0]];
  const GfVec3f e1 = positions[elemIdx[2]] - positions[elemIdx[1]];
  const GfVec3f e2 = positions[elemIdx[0]] - positions[elemIdx[2]];

  return(
    (e1 ^ normals[elemIdx[0]]).GetNormalized() * baryWeights[0] +
    (e2 ^ normals[elemIdx[1]]).GetNormalized() * baryWeights[1] +
    (e0 ^ normals[elemIdx[2]]).GetNormalized() * baryWeights[2]
    ).GetNormalized();
*/
  /*// GET UP VECTOR
  MVector Curve::getUpV()
  {
      MVector tangent(m_samples[1][3][0] - m_samples[0][3][0],
                      m_samples[1][3][1] - m_samples[0][3][1],
                      m_samples[1][3][2] - m_samples[0][3][2]);
      tangent.normalize();
      MVector side(1,0,0);
      if((1.f - fabsf(tangent * side)) < DOT_EPSILON)
      {
          side = MVector(0,1,0);
          if((1.f - fabsf(tangent * side)) < DOT_EPSILON)
          {
              side = MVector(0,0,1);
          }
      }

      return (tangent ^ side).normal();
  }
  */
}

// Estimate the geodesic distance between two points using their position and normals.
// See c95-f95_199-a16-paperfinal-v5 "3.2 Sampling with Geodesic Distance".  This estimation
// assumes that we have a smooth gradient between the two points, since it doesn't have any
// information about complex surface changes between the points.
float _ApproximateGeodesicDistance(const GfVec3f& p1, const GfVec3f& p2, 
  const GfVec3f& n1, const GfVec3f& n2)
{
#if 0
    return (p1 - p2).GetLengthSq();
#else
    GfVec3f v = p2-p1;
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

float _TriangleArea(const GfVec3f &a, const GfVec3f &b, const GfVec3f &c)
{
  float ab = (a - b).GetLength();
  float bc = (b - c).GetLength();
  float ca = (c - a).GetLength();
  float p = (ab + bc + ca) / 2;
  return GfSqrt(p * (p - ab) * (p - bc) * (p - ca));
}


// Do a simple random sampling of the triangles.  Return the total surface area of the triangles.
float _CreateRawSamples(int numSamples,
                        const VtArray<GfVec3f>& points,
                        const VtArray<GfVec3f>& normals,
                        const VtArray<int>& triangles,
                        VtArray<Sample> &samples)
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
    sample.elemIdx = GfVec3i(
      triangles[triIdx * 3 + 0],
      triangles[triIdx * 3 + 1], 
      triangles[triIdx * 3 + 2]);
    sample.baryWeights = 
      GfVec3f(1 - sqrt(u), sqrt(u) * (1 - v), v * sqrt(u));
  }

  return maxAreaSum;
}

static GfVec3i
_GetGridCoords(const GfRange3d& range, const GfVec3d& point, const GfVec3i dimensions)
{
  const GfVec3d& min = range.GetMin();
  const GfVec3d& size = range.GetSize();
  const GfVec3d scale = GfVec3d(
    size[0]/dimensions[0], 
    size[1]/dimensions[1], 
    size[2]/dimensions[2]
  );
  return GfVec3i(
    (int)((point[0] - min[0]) * 1.f / scale[0]),
    (int)((point[1] - min[1]) * 1.f / scale[1]),
    (int)((point[2] - min[2]) * 1.f / scale[2])
  );
}

static uint32_t
_GetGridIndex(const GfVec3i& coords, const GfVec3i& dimensions)
{
  return coords[2] * dimensions[0] * dimensions[1] + coords[1] * dimensions[0] + coords[0];
}

void _PoissonDiskFromSamples(const GfVec3f* positions,
                            const GfVec3f* normals,
                            float radius,
                            VtArray<Sample>& seeds,
                            VtArray<Sample>& samples)
{
  // Get the bounding box of the samples.
  GfRange3d bbox;
  for (auto& seed : seeds) {
    bbox.Union(seed.GetPosition(positions));
  }
  const GfVec3f bboxSize(bbox.GetSize());

  const float radiusSq = radius * radius;
  GfVec3f gridSize = bboxSize / radius;
  GfVec3i gridSizeI(GfFloor(gridSize[0]), GfFloor(gridSize[1]), GfFloor(gridSize[2]));
  gridSizeI[0] = GfMax(gridSizeI[0], 1);
  gridSizeI[1] = GfMax(gridSizeI[1], 1);
  gridSizeI[2] = GfMax(gridSizeI[2], 1);

  // Assign a cell ID to each seed.
  for(auto &seed: seeds)
  {
    GfVec3i coords = _GetGridCoords(bbox, seed.GetPosition(positions), gridSizeI);
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
          neighborCellOffsets.push_back(_GetGridIndex(GfVec3i(x, y, z), gridSizeI));
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
StochasticSampling(int nbSamples,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<Triangle>& triangles,
  VtArray<Sample>& samples)
{
  size_t nbTriangles = triangles.size();
  std::vector<float>triangleAreas(nbTriangles);
  float totalArea = 0.f;

  for(size_t t = 0; t < nbTriangles; ++t) {
    triangleAreas[t] = triangles[t].GetArea(&points[0]);
    totalArea += triangleAreas[t];
  }

  std::vector<size_t>triangleSamples(nbTriangles, 0);
  size_t totalSamples = 0;
  for(size_t t = 0; t < nbTriangles; ++t) {
    triangleSamples[t] = nbSamples;//(size_t)(nbSamples * triangleAreas[t] / totalArea);
    totalSamples += triangleSamples[t];
  }

  samples.resize(totalSamples);
  size_t sampleIdx = 0;
  for (size_t t = 0; t < nbTriangles; ++t) {
    for(size_t i = 0; i < triangleSamples[t]; ++i) {
      float u = RANDOM_0_1;
      float v = RANDOM_0_1;
      samples[sampleIdx++] = { 
        triangles[t].vertices, 
        0, 
        GfVec3f(u, v, 1.f - (u+v))
      };
    }
  }
}


void
GridSampling(int nbSamples,
  const VtArray<GfVec3f>& points,
  const VtArray<GfVec3f>& normals,
  const VtArray<Triangle>& triangles,
  VtArray<Sample>& samples)
{
  size_t nbTriangles = triangles.size();
  std::vector<float>triangleAreas(nbTriangles);
  float totalArea = 0.f;

  for(size_t t = 0; t < nbTriangles; ++t) {
    triangleAreas[t] = triangles[t].GetArea(&points[0]);
    totalArea += triangleAreas[t];
  }

  std::vector<size_t>triangleSamples(nbTriangles, 0);
  size_t totalSamples = 0;
  for(size_t t = 0; t < nbTriangles; ++t) {
    triangleSamples[t] = (size_t)(nbSamples * triangleAreas[t] / totalArea);
    totalSamples += triangleSamples[t];
  }

  samples.resize(totalSamples);
  size_t sampleIdx = 0;
  for (size_t t = 0; t < nbTriangles; ++t) {
    for(size_t i = 0; i < triangleSamples[t]; ++i) {
      float u = RANDOM_0_1;
      float v = RANDOM_0_1;
      samples[sampleIdx++] = { 
        triangles[t].vertices, 
        0, 
        GfVec3f(u, v, 1.f - (u+v))
      };
    }
  }
}

void
  PoissonSampling(float radius, int nbSamples,
    const VtArray<GfVec3f>& points,
    const VtArray<GfVec3f>& normals,
    const VtArray<Triangle>& triangles,
    VtArray<Sample>& samples)
{
  
  VtArray<Sample> seeds;
  VtArray<int> indices(triangles.size() * 3);
  for (size_t triIdx = 0; triIdx < triangles.size(); ++triIdx) {
    memcpy(&indices[triIdx * 3], &triangles[triIdx].vertices, sizeof(GfVec3i));
  }
  float surfaceArea = _CreateRawSamples(nbSamples, points, normals, indices, seeds);

  if (radius <= 0.f) {
    radius = std::sqrtf(surfaceArea / nbSamples) * 0.75f;
  }

  _PoissonDiskFromSamples(&points[0], &normals[0], radius, seeds, samples);
}

JVR_NAMESPACE_CLOSE_SCOPE