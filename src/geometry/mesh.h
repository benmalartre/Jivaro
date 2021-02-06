#ifndef AMN_OBJECTS_MESH_H
#define AMN_OBJECTS_MESH_H


#include "../common.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include <vector>
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
    uint32_t     _triangleId;    // global triangle index
    pxr::GfVec3f _baryCoords;    // barycentric coordinates
};

class Mesh{
public:
    Mesh();
    Mesh(const Mesh* other, bool normalize = true);
    ~Mesh();

    const pxr::GfVec3f* GetPositionsCPtr(){return &_positions[0];};
    const pxr::GfVec3f* GetNormalsCPtr(){return &_normals[0];};

    pxr::GfVec3f GetPosition(const Triangle* T) const;                   // triangle position
    pxr::GfVec3f GetPosition(const Triangle* T, uint32_t index) const;   // vertex position
    pxr::GfVec3f GetNormal(const Triangle* T) const;                     // triangle normal
    pxr::GfVec3f GetNormal(const Triangle* T, uint32_t index) const;     // vertex normal
    pxr::GfVec3f GetTriangleNormal(uint32_t triangleID) const;           // triangle normal
    pxr::GfVec3f GetPosition(uint32_t index) const;                      // vertex position
    pxr::GfVec3f GetNormal(uint32_t index) const;                        // vertex normal
    
    pxr::GfVec3f GetPosition(const PointOnMesh& MP) const ;
    pxr::GfVec3f GetNormal(const PointOnMesh& MP) const;
    Triangle* GetTriangle(uint32_t index){return &_triangles[index];};
    std::vector<Triangle>& GetTriangles(){return _triangles;};

    
    uint32_t GetNumVertices()const {return _numVertices;};
    uint32_t GetNumTriangles()const {return _numTriangles;};
    uint32_t GetNumSamples()const {return _numSamples;};
    uint32_t GetNumFaces()const {return _numFaces;};

    float TriangleArea(uint32_t index);
    float AveragedTriangleArea();

    void Init(
      const std::vector<pxr::GfVec3f>& positions, 
      const std::vector<int>& counts, 
      const std::vector<int>& connects);

    void Update(const std::vector<pxr::GfVec3f>& positions);
    void Triangulate();
    void Inflate(uint32_t index, float value);
    void ComputeBoundingBox();
    pxr::GfBBox3d& GetBoundingBox();
    bool ClosestIntersection(const pxr::GfVec3f& P, const pxr::GfVec3f& N, 
      PointOnMesh& MP, float maxDistance);
    bool IsInitialized(){return _initialized;};
    void SetInitialized(bool initialized){_initialized = initialized;};

private:
    uint32_t _numTriangles;                  // total num triangles
    uint32_t _numVertices;                   // total num vertices
    uint32_t _numSamples;                    // total num samples
    uint32_t _numFaces;                      // total num faces

    std::vector<int>          _faceCount;    // num vertex per face
    std::vector<int>          _faceConnect;  // vertex indices per face

    std::vector<pxr::GfVec3f> _positions;    // vertices position
    std::vector<pxr::GfVec3f> _normals;      // vertices normal

    std::vector<Triangle>     _triangles;    // triangles array
    pxr::GfBBox3d             _bbox;         // axis aligned bounding box
    bool _initialized;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
