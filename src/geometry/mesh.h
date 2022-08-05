#ifndef JVR_GEOMETRY_MESH_H
#define JVR_GEOMETRY_MESH_H

#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <float.h>
#include "triangle.h"
#include "geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

struct Location {
  Geometry*             geometry;      // geometry ptr
  uint32_t              id;            // element index
  pxr::GfVec3f          baryCoords;    // barycentric coordinates
};

struct HalfEdge
{
  enum Latency {
    REAL,
    IMPLICIT,
    VIRTUAL,
    ANY
  };

  uint32_t                index;     // half edge index
  uint32_t                vertex;    // vertex index
  //uint32_t                face;      // face index
  struct HalfEdge*        twin;      // opposite half-edge
  struct HalfEdge*        next;      // next half-edge
  uint8_t                 latency;   // edge latency

  HalfEdge():vertex(0)/*,face(0),triangle(0)*/,twin(NULL),next(NULL),latency(REAL){};
  inline size_t GetTriangleIndex() const {return index / 3;};
  void GetTriangleNormal(const pxr::GfVec3f* positions, 
    pxr::GfVec3f& normal) const;
  void GetVertexNormal(const pxr::GfVec3f* normals, pxr::GfVec3f& normal) const;
  bool GetFacing(const pxr::GfVec3f* positions, const pxr::GfVec3f& v) const;
  bool GetFacing(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  float GetDot(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  
  /*
  short GetFlags(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals, 
    const pxr::GfVec3f& v, float creaseValue) const;
  float GetWeight(const pxr::GfVec3f* positions, const pxr::GfVec3f* normals,
    const pxr::GfVec3f& v) const;
  */
};

class Mesh : public Geometry {
public:
  Mesh();
  Mesh(const Mesh* other, bool normalize = true);
  Mesh(const pxr::UsdGeomMesh& usdMesh);
  ~Mesh();

  const pxr::VtArray<int>& GetFaceCounts() const { return _faceVertexCounts;};
  const pxr::VtArray<int>& GetFaceConnects() const { return _faceVertexIndices;};
  pxr::VtArray<int>& GetFaceCounts() { return _faceVertexCounts;};
  pxr::VtArray<int>& GetFaceConnects() { return _faceVertexIndices;};

  pxr::GfVec3f GetPosition(size_t idx) const;
  pxr::GfVec3f GetTrianglePosition(const Triangle* T) const;                        // triangle position
  pxr::GfVec3f GetTriangleVertexPosition(const Triangle* T, uint32_t index) const;  // vertex position
  pxr::GfVec3f GetTriangleNormal(const Triangle* T) const;                          // triangle normal
  pxr::GfVec3f GetTriangleVertexNormal(const Triangle* T, uint32_t index) const;    // vertex normal
  pxr::GfVec3f GetTriangleNormal(uint32_t triangleID) const;                        // triangle normal

  const std::vector<HalfEdge*> GetUniqueEdges();
  
  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };
  pxr::GfVec3f GetPosition(const Location& point) const ;
  pxr::GfVec3f GetNormal(const Location& point) const;
  Triangle* GetTriangle(uint32_t index){return &_triangles[index];};
  pxr::VtArray<Triangle>& GetTriangles(){return _triangles;};

  uint32_t GetNumTriangles()const {return _numTriangles;};
  uint32_t GetNumSamples()const {return _numSamples;};
  uint32_t GetNumFaces()const {return _numFaces;};
  uint32_t GetFaceNumVertices(uint32_t idx) const {return _faceVertexCounts[idx];};
  uint32_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);
  void GetCutVerticesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);
  void GetCutEdgesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);

  void ComputeHalfEdges();
  void ComputeNeighbors();
  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();

  void SetTopology(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& faceVertexCounts, 
    const pxr::VtArray<int>& faceVertexIndices);

  void Init();

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);

  // retopo
  void SetAllEdgesLatencyReal();
  void UpdateTopologyFromHalfEdges();
  void SplitEdge(size_t index);
  void CollapseEdge(size_t index);

  // Flatten
  void DisconnectEdges(const pxr::VtArray<int>& edges);
  void Flatten(const pxr::VtArray<pxr::GfVec2d>& uvs, const pxr::TfToken& interpolation);

  void Inflate(uint32_t index, float value);
  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, Location& point, float maxDistance);

  // test (to be removed)
  void PolygonSoup(size_t numPolygons, 
    const pxr::GfVec3f& minimum=pxr::GfVec3f(-1.f), 
    const pxr::GfVec3f& maximum=pxr::GfVec3f(1.f));
  void OpenVDBSphere(const float radius, 
    const pxr::GfVec3f& center=pxr::GfVec3f(0.f));
  void Randomize(float value);
  void TriangularGrid2D(float width, float height, 
    const pxr::GfMatrix4f& space, float size);
  void VoronoiDiagram(const std::vector<pxr::GfVec3f>& points);

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

private:
  // infos
  uint32_t                            _numTriangles;
  uint32_t                            _numSamples;
  uint32_t                            _numFaces;

  // polygonal description
  pxr::VtArray<int>                   _faceVertexCounts;  
  pxr::VtArray<int>                   _faceVertexIndices;

  // colors
  pxr::VtArray<pxr::GfVec3f>          _colors;
  GeomInterpolation                   _colorsInterpolation;

  // vertex data
  pxr::VtArray<bool>                  _boundary;
  pxr::VtArray<int>                   _shell;
  pxr::VtArray< pxr::VtArray<int> >   _neighbors;

  // shell data (vertices)
  pxr::VtArray< pxr::VtArray<int> >   _shells;

  // triangle data
  pxr::VtArray<Triangle>              _triangles;
  pxr::VtArray<TrianglePair>          _trianglePairs;

  // half-edge data
  pxr::VtArray<HalfEdge>              _halfEdges;
  pxr::VtArray<int>                   _uniqueEdges;
  pxr::VtArray<int>                   _vertexHalfEdge;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
