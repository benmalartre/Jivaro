#ifndef AMN_OBJECTS_MESH_H
#define AMN_OBJECTS_MESH_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "triangle.h"

AMN_NAMESPACE_OPEN_SCOPE

struct PointOnMesh{
  uint32_t     triangleId;    // global triangle index
  pxr::GfVec3f baryCoords;    // barycentric coordinates
};

struct HalfEdge
{
  uint32_t                index;     // half edge index
  uint32_t                vertex;    // vertex index
  struct HalfEdge*        twin;      // opposite half-edge
  struct HalfEdge*        next;      // next half-edge
  bool                    latent;    // virtual edge

  HalfEdge():vertex(0),twin(NULL),next(NULL),latent(true){};
  inline size_t GetTriangleIndex() const {return index / 3;};
  void GetTriangleNormal(const pxr::GfVec3f* positions, 
    pxr::GfVec3f& normal) const;
  void GetVertexNormal(const pxr::GfVec3f* normals, pxr::GfVec3f& normal) const;
  bool GetFacing(const pxr::GfVec3f* positions, const pxr::GfVec3f& v) const;
  bool GetFacing(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  float GetDot(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  short GetFlags(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, 
    const pxr::GfVec3f& v, float creaseValue) const;
  float GetWeight(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
};

class Mesh {
public:
  Mesh();
  Mesh(const Mesh* other, bool normalize = true);
  ~Mesh();

  const pxr::GfVec3f* GetPositionsCPtr(){return &_position[0];};
  const pxr::GfVec3f* GetNormalsCPtr(){return &_normal[0];};

  pxr::GfVec3f GetPosition(const Triangle* T) const;                   // triangle position
  pxr::GfVec3f GetPosition(const Triangle* T, uint32_t index) const;   // vertex position
  pxr::GfVec3f GetNormal(const Triangle* T) const;                     // triangle normal
  pxr::GfVec3f GetNormal(const Triangle* T, uint32_t index) const;     // vertex normal
  pxr::GfVec3f GetTriangleNormal(uint32_t triangleID) const;           // triangle normal
  pxr::GfVec3f GetPosition(uint32_t index) const;                      // vertex position
  pxr::GfVec3f GetNormal(uint32_t index) const;                        // vertex normal
  
  pxr::GfVec3f GetPosition(const PointOnMesh& point) const ;
  pxr::GfVec3f GetNormal(const PointOnMesh& point) const;
  Triangle* GetTriangle(uint32_t index){return &_triangles[index];};
  pxr::VtArray<Triangle>& GetTriangles(){return _triangles;};

  uint32_t GetNumPoints()const {return _numPoints;};
  uint32_t GetNumTriangles()const {return _numTriangles;};
  uint32_t GetNumSamples()const {return _numSamples;};
  uint32_t GetNumFaces()const {return _numFaces;};

  void ComputeHalfEdges();
  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& counts, 
    const pxr::VtArray<int>& connects);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Inflate(uint32_t index, float value);
  void ComputeBoundingBox();
  pxr::GfBBox3d& GetBoundingBox();
  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, PointOnMesh& point, float maxDistance);
  bool IsInitialized(){return _initialized;};
  void SetInitialized(bool initialized){_initialized = initialized;};

private:
  // infos
  uint32_t                            _numTriangles;
  uint32_t                            _numPoints;
  uint32_t                            _numSamples;
  uint32_t                            _numFaces;

  // polygonal description
  pxr::VtArray<int>                   _faceCounts;  
  pxr::VtArray<int>                   _faceConnects;

  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _position;
  pxr::VtArray<pxr::GfVec3f>          _normal;
  pxr::VtArray<bool>                  _boundary;
  pxr::VtArray<int>                   _shell;
  pxr::VtArray< pxr::VtArray<int> >   _neighbors;

  // shell data (vertices)
  pxr::VtArray< pxr::VtArray<int> >   _shells;

  // triangle data
  pxr::VtArray<Triangle>              _triangles;

  // half-edge data
  pxr::VtArray<HalfEdge>              _halfEdges;

  // bounding box
  pxr::GfBBox3d                       _bbox;
  bool _initialized;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
