#ifndef AMN_OBJECTS_MESH_H
#define AMN_OBJECTS_MESH_H


#include "../common.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <float.h>
#include <vector>
#include "aabb.h"
#include "triangle.h"

AMN_NAMESPACE_OPEN_SCOPE

/*
class Mesh {
private:
  pxr::GfMatrix4d           _xform;
  std::vector<pxr::GfVec3f> _points;
  std::vector<int>          _counts;
  std::vector<int>          _indices;
  std::vector<int>          _triangles;

};
*/

struct PointOnMesh{
    uint32_t _triangleID;        // global triangle index
    pxr::GfVec3f _baryCoords;    // barycentric coordinates
};

class Mesh{
public:
    uint32_t GetVertexIndex(uint32_t meshIndex, uint32_t index);
    const float* GetPositions(){return _positions;};
    const float* GetNormals(){return _normals;};
    const int* GetIndices(){return _indices;};
    const int* GetSamples(){return _samples;};
    float* GetSubMeshPositions(uint32_t index);
    float* GetSubMeshNormals(uint32_t index);
    int* GetSubMeshIndices(uint32_t index);
    int* GetSubMeshSamples(uint32_t index);
    pxr::GfVec3f GetPosition(const Triangle* T) const;                        // triangle position
    pxr::GfVec3f GetPosition(const Triangle* T, uint32_t index) const;        // vertex position
    pxr::GfVec3f GetNormal(const Triangle* T) const;                          // triangle normal
    pxr::GfVec3f GetNormal(const Triangle* T, uint32_t index) const;          // vertex normal
    pxr::GfVec3f GetTriangleNormal(uint32_t triangleID) const;                // triangle normal
    pxr::GfVec3f GetPosition(uint32_t meshIndex, uint32_t index) const;       // vertex position
    pxr::GfVec3f GetNormal(uint32_t meshIndex, uint32_t index) const;         // vertex normal
    
    pxr::GfVec3f GetPosition(const PointOnMesh& MP) const ;
    pxr::GfVec3f GetNormal(const PointOnMesh& MP) const;
    Triangle* GetTriangle(uint32_t index){return &_triangles[index];};
    std::vector<Triangle>& GetTriangles(){return _triangles;};

    Mesh();
    Mesh(uint32_t numSubMeshes, uint32_t totalNumTriangles, uint32_t totalNumVertices);
    Mesh(const Mesh* other, bool normalize = true);
    ~Mesh();
    void SetNumMeshes(uint32_t num);
    uint32_t GetNumMeshes()const {return _numMeshes;};
    uint32_t GetNumVertices()const {return _numVertices;};
    uint32_t GetNumTriangles()const {return _numTriangles;};
    uint32_t GetNumSamples()const {return _numSamples;};
    SubMesh* GetSubMesh(int index);
    const SubMesh* getSubMesh(int index) const;

    float TriangleArea(uint32_t index);
    float AveragedTriangleArea();

    void init(
      const std::vector<pxr::GfVec3f>& positions, 
      const std::vector<int>& counts, 
      const std::vector<int>& connects);

    void init(const float* positions, uint32_t numVertices, const int* indices, uint32_t numTriangles, uint32_t indexSubMesh);
    void update(const float* positions, uint32_t numVertices);
    void allocate();
    void triangulate();
    void inflate(uint32_t index, float value);
    void computeBoundingBox();
    AABB& getBoundingBox();
    bool closestIntersection(const MPoint& P, const MVector& N, PointOnMesh& MP, float maxDistance);
    bool isInitialized(){return _initialized;};
    void setInitialized(bool initialized){_initialized = initialized;};

private:
    std::vector<pxr::GfVec3f> _positions;    // vertices position
    std::vector<int>          _faceCounts;   // num vertex per face
    std::vector<int>          _faceConnect;  // vertex indices per face
    std::vector<pxr::GfVec3f> _normals;      // vertices normal
    std::vector<int>          _indices;      // triangle vertices indices
    std::vector<int>          _samples;      // normal vertices indices
    uint32_t _numTriangles;                  // total num triangles
    uint32_t _numVertices;                   // total num vertices
    uint32_t _numSamples;                    // total num samples
    std::vector<Triangle>     _triangles;    // triangles array
    AABB _bbox;                              // mesh axis aligned bounding box
    bool _initialized;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
