#ifndef AMN_ACCELERATION_BVH_H
#define AMN_ACCELERATION_BVH_H

#include <limits>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/range3f.h>


class Geometry;

class BVH
{ 
  static const uint8_t NUM_PLANE_SET_NORMALS = 7; 
  static const pxr::GfVec3f _planeSetNormals[NUM_PLANE_SET_NORMALS]; 
  struct Extents 
  { 
    Extents() 
    { 
      for (uint8_t i = 0;  i < NUM_PLANE_SET_NORMALS; ++i) {
        d[i][0] = std::numeric_limits<float>::infinity();
        d[i][1] = -std::numeric_limits<float>::infinity(); 
      }
    } 
    void ExtendBy(const Extents& e) 
    { 
      for (uint8_t i = 0;  i < NUM_PLANE_SET_NORMALS; ++i) { 
        if (e.d[i][0] < d[i][0]) d[i][0] = e.d[i][0]; 
        if (e.d[i][1] > d[i][1]) d[i][1] = e.d[i][1]; 
      } 
    } 
    /* inline */ 
    pxr::GfVec3f Centroid() const 
    { 
      return pxr::GfVec3f( 
        d[0][0] + d[0][1] * 0.5, 
        d[1][0] + d[1][1] * 0.5, 
        d[2][0] + d[2][1] * 0.5); 
    } 
    bool Intersect(const float*, const float*, float&, float&, uint8_t&) const; 
    float d[NUM_PLANE_SET_NORMALS][2]; 
    const Geometry* geometry; 
  }; 
 
  struct Octree 
  { 
    Octree(const Extents& sceneExtents) 
    { 
      float xDiff = sceneExtents.d[0][1] - sceneExtents.d[0][0]; 
      float yDiff = sceneExtents.d[1][1] - sceneExtents.d[1][0]; 
      float zDiff = sceneExtents.d[2][1] - sceneExtents.d[2][0]; 
      float maxDiff = std::max(xDiff, std::max(yDiff, zDiff)); 
      pxr::GfVec3f minPlusMax( 
        sceneExtents.d[0][0] + sceneExtents.d[0][1], 
        sceneExtents.d[1][0] + sceneExtents.d[1][1], 
        sceneExtents.d[2][0] + sceneExtents.d[2][1]); 
      bbox[0] = (minPlusMax - maxDiff) * 0.5f; 
      bbox[1] = (minPlusMax + maxDiff) * 0.5f; 
      root = new OctreeNode; 
    } 

    ~Octree() { DeleteOctreeNode(root); } 

    void Insert(const Extents* extents) { Insert(root, extents, bbox, 0); } 
    void Build() { Build(root, bbox); }; 

    struct OctreeNode 
    { 
      OctreeNode* child[8] = { nullptr }; 
      std::vector<const Extents *> nodeExtentsList; // pointer to the objects extents 
      Extents nodeExtents; // extents of the octree node itself 
      bool isLeaf = true; 
    }; 

    struct QueueElement 
    { 
      const OctreeNode *node; // octree node held by this element in the queue 
      float t; // distance from the ray origin to the extents of the node 
      QueueElement(const OctreeNode *n, float tn) : node(n), t(tn) {} 
      // priority_queue behaves like a min-heap
      friend bool operator < (const QueueElement &a, const QueueElement &b) { 
        return a.t > b.t; 
      } 
    }; 

    OctreeNode* root = nullptr; // make unique son don't have to manage deallocation 
    pxr::GfRange3f bbox; 

  private: 

    void DeleteOctreeNode(OctreeNode*& node) 
    { 
      for (uint8_t i = 0; i < 8; i++) { 
        if (node->child[i] != nullptr) { 
          DeleteOctreeNode(node->child[i]); 
        } 
      } 
      delete node; 
    } 

    void Insert(OctreeNode*& node, const Extents* extents, 
      const pxr::GfRange3f& bbox, uint32_t depth) 
    { 
      if (node->isLeaf) { 
        if (node->nodeExtentsList.size() == 0 || depth == 16) { 
          node->nodeExtentsList.push_back(extents); 
        } 
        else { 
          node->isLeaf = false; 
          // Re-insert extents held by this node
          while (node->nodeExtentsList.size()) { 
            Insert(node, node->nodeExtentsList.back(), bbox, depth); 
            node->nodeExtentsList.pop_back(); 
          } 
          // Insert new extent
          Insert(node, extents, bbox, depth); 
        } 
      } else { 
        // Need to compute in which child of the current node this extents 
        // should be inserted into
        pxr::GfVec3f extentsCentroid = extents->Centroid(); 
        pxr::GfVec3f nodeCentroid = (bbox.GetMin() + bbox.GetMax()) * 0.5f; 
        pxr::GfVec3f childBBoxMin, childBBoxMax;
        uint8_t childIndex = 0; 
        // x-axis
        if (extentsCentroid[0] > nodeCentroid[0]) { 
            childIndex = 4; 
            childBBoxMin[0] = nodeCentroid[0]; 
            childBBoxMax[0] = bbox.GetMax()[0]; 
        } 
        else { 
            childBBoxMin[0] = bbox.GetMin[0]; 
            childBBoxMax[0] = nodeCentroid[0]; 
        } 
        // y-axis
        if (extentsCentroid[1] > nodeCentroid[1]) { 
            childIndex += 2; 
            childBBoxMin[1] = nodeCentroid[1]; 
            childBBoxMax[1] = bbox.GetMax()[1]; 
        } 
        else { 
            childBBoxMin[1] = bbox.GetMin()[1]; 
            childBBoxMax[1] = nodeCentroid[1]; 
        } 
        // z-axis
        if (extentsCentroid[2] > nodeCentroid[2]) { 
            childIndex += 1; 
            childBBoxMin[2] = nodeCentroid[2]; 
            childBBoxMax[2] = bbox.GetMax()[2]; 
        } 
        else { 
            childBBoxMin[2] = bboxGetMin()[2]; 
            childBBoxMax[2] = nodeCentroid[2]; 
        } 

        // Create the child node if it doesn't exsit yet and then insert the extents in it
        if (node->child[childIndex] == nullptr) 
            node->child[childIndex] = new OctreeNode; 
        pxr::GfRange3f childBBox(childBBoxMin, childBBoxMax); 
        Insert(node->child[childIndex], extents, childBBox, depth + 1); 
      } 
    } 

    void Build(OctreeNode*& node, const pxr::GfRange3f& bbox) 
    { 
      if (node->isLeaf) { 
        for (const auto& e: node->nodeExtentsList) { 
          node->nodeExtents.ExtendBy(*e); 
        } 
      } else { 
        for (uint8_t i = 0; i < 8; ++i) { 
          if (node->child[i]) { 
            pxr::GfVec3f childBBoxMin, childBBoxMax; 
            pxr::GfVec3f centroid = bbox.Centroid(); 
            // x-axis
            childBBoxMin[0] = (i & 4) ? centroid[0] : bbox.GetMin()[0]; 
            childBBoxMax[0] = (i & 4) ? bbox.GetMax()[0] : centroid[0]; 
            // y-axis
            childBBoxMin[1] = (i & 2) ? centroid[1] : bbox.GetMin()[1]; 
            childBBoxMax[1] = (i & 2) ? bbox.GetMax()[1] : centroid[1]; 
            // z-axis
            childBBoxMin[2] = (i & 1) ? centroid[2] : bbox.GetMin()[2]; 
            childBBoxMax[2] = (i & 1) ? bbox.GetMax()[2] : centroid[2]; 

            // Inspect child
            pxr::GfRange3f childBBox(childBBoxMin, childBBoxMax);
            Build(node->child[i], childBBox); 

            // Expand extents with extents of child
            node->nodeExtents.ExtendBy(node->child[i]->nodeExtents); 
          } 
        } 
      } 
    } 
  }; 
 
  std::vector<Extents> _extentsList; 
  Octree* _octree = nullptr; 
public: 
  BVH(std::vector<std::unique_ptr<const Mesh>>& m); 
  bool Intersect(const Vec3f&, const Vec3f&, const uint32_t&, float&) const; 
  ~BVH() { delete _octree; } 
}; 

#endif