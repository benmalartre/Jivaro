#include "bvh.h"

const pxr::GfVec3f BVH::_planeSetNormals[BVH::NUM_PLANE_SET_NORMALS] = { 
  pxr::GfVec3f(1, 0, 0), 
  pxr::GfVec3f(0, 1, 0), 
  pxr::GfVec3f(0, 0, 1), 
  pxr::GfVec3f( sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f), 
  pxr::GfVec3f(-sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f), 
  pxr::GfVec3f(-sqrtf(3.f) / 3.f, -sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f), 
  pxr::GfVec3f( sqrtf(3.f) / 3.f, -sqrtf(3.f) / 3.f, sqrtf(3.f) / 3.f) 
}; 
 
BVH::BVH(std::vector<std::unique_ptr<const Mesh>>& m) 
{ 
  Extents sceneExtents;
  _extentsList.reserve(meshes.size()); 
  for (uint32_t i = 0; i < meshes.size(); ++i) { 
    for (uint8_t j = 0; j < NUM_PLANE_SET_NORMALS; ++j) { 
      for (const auto vtx : meshes[i]->vertexPool) { 
        float d = pxr::GfDot(planeSetNormals[j], vtx); 
        if (d < _extentsList[i].d[j][0]) _extentsList[i].d[j][0] = d; 
        if (d > _extentsList[i].d[j][1]) _extentsList[i].d[j][1] = d; 
      } 
    } 
    sceneExtents.ExtendBy(_extentsList[i]);  
    _extentsList[i].mesh = meshes[i].get();
  } 

  // Now that we have the extent of the scene we can start building our octree
  // Using C++ make_unique function here but you don't need to, just to learn something... 
  _octree = new Octree(sceneExtents); 

  for (uint32_t i = 0; i < meshes.size(); ++i) { 
    _octree->Insert(&_extentsList[i]); 
  } 

  // Build from bottom up
  _octree->Build(); 
} 
 
bool BVH::Extents::Intersect( 
  const float* precomputedNumerator, 
  const float* precomputedDenominator, 
  float& tNear,   // tn and tf in this method need to be contained 
  float& tFar,    // within the range [tNear:tFar] 
  uint8_t& planeIndex) const 
{ 
  numRayBoundingVolumeTests++; 
  for (uint8_t i = 0; i < NUM_PLANE_SET_NORMALS; ++i) { 
    float tNearExtents = (d[i][0] - precomputedNumerator[i]) / precomputedDenominator[i]; 
    float tFarExtents = (d[i][1] - precomputedNumerator[i]) / precomputedDenominator[i]; 
    if (precomputedDenominator[i] < 0) std::swap(tNearExtents, tFarExtents); 
    if (tNearExtents > tNear) tNear = tNearExtents, planeIndex = i; 
    if (tFarExtents < tFar) tFar = tFarExtents; 
    if (tNear > tFar) return false; 
  } 

  return true; 
} 
 
bool BVH::Intersect(const Vec3f& orig, const Vec3f& dir, 
  const uint32_t& rayId, float& tHit) const 
{ 
  tHit = std::numeric_limits<float>::infinity(); 
  const Mesh* intersectedMesh = nullptr; 
  float precomputedNumerator[BVH::NUM_PLANE_SET_NORMALS]; 
  float precomputedDenominator[BVH::NUM_PLANE_SET_NORMALS]; 
  for (uint8_t i = 0; i < NUM_PLANE_SET_NORMALS; ++i) { 
    precomputedNumerator[i] = pxr::GfDot(planeSetNormals[i], orig); 
    precomputedDenominator[i] = pxr::GfDot(planeSetNormals[i], dir); 
  } 

    /* 
    tNear = kInfinity; // set 
    for (uint32_t i = 0; i < meshes.size(); ++i) { 
        numRayVolumeTests++; 
        float tn = -kInfinity, tf = kInfinity; 
        uint8_t planeIndex; 
        if (extents[i].intersect(precomputedNumerator, precomputedDenominator, tn, tf, planeIndex)) { 
            if (tn < tNear) { 
                intersectedMesh = meshes[i].get(); 
                tNear = tn; 
                // normal = planeSetNormals[planeIndex];
            } 
        } 
    } 
    */ 
 
    uint8_t planeIndex; 
    // tNear, tFar for the intersected extents 
    float tNear = 0, tFar = std::numeric_limits<float>::infinity(); 
    if (!_octree->root->nodeExtents.Intersect(precomputedNumerator, 
      precomputedDenominator, tNear, tFar, planeIndex) || tFar < 0) 
        return false; 
    tHit = tFar; 
    std::priority_queue<BVH::Octree::QueueElement> queue; 
    queue.push(BVH::Octree::QueueElement(_octree->root, 0)); 
    while (!queue.empty() && queue.top().t < tHit) { 
      const Octree::OctreeNode *node = queue.top().node; 
      queue.pop(); 
      if (node->isLeaf) { 
        for (const auto& e: node->nodeExtentsList) { 
          float t = std::numeric_limits<float>::infinity(); 
          if (e->mesh->intersect(orig, dir, t) && t < tHit) { 
            tHit = t; 
            intersectedMesh = e->mesh; 
          } 
        } 
      } else { 
        for (uint8_t i = 0; i < 8; ++i) { 
          if (node->child[i] != nullptr) { 
            float tNearChild = 0, tFarChild = tFar; 
            if (node->child[i]->nodeExtents.intersect(precomputedNumerator, 
              precomputedDenominator, tNearChild, tFarChild, planeIndex)) { 
              float t = (tNearChild < 0 && tFarChild >= 0) ? 
                tFarChild : tNearChild; 
              queue.push(BVH::Octree::QueueElement(node->child[i], t)); 
            } 
          } 
        } 
      } 
    } 
 
    return (intersectedMesh != nullptr); 
} 