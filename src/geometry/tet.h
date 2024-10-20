#ifndef JVR_GEOMETRY_TET_H
#define JVR_GEOMETRY_TET_H

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

PXR_NAMESPACE_OPEN_SCOPE


class Tet : public Geometry {
public:
  Tet();
  Tet(const Tet* other, bool normalize = true);
  Tet(const pxr::UsdGeomMesh& usdMesh);
  ~Tet();

  const pxr::VtArray<int>& GetFaceCounts() const { return _faceVertexCounts;};
  const pxr::VtArray<int>& GetFaceConnects() const { return _faceVertexIndices;};
  pxr::VtArray<int>& GetFaceCounts() { return _faceVertexCounts;};
  pxr::VtArray<int>& GetFaceConnects() { return _faceVertexIndices;};

  pxr::GfVec3f GetPosition(size_t idx) const;
  pxr::GfVec3f GetTrianglePosition(const Triangle* T) const;                   // triangle position
  pxr::GfVec3f GetTriangleVertexPosition(const Triangle* T, uint32_t index) const;   // vertex position
  pxr::GfVec3f GetTriangleNormal(const Triangle* T) const;                     // triangle normal
  pxr::GfVec3f GetTriangleVertexNormal(const Triangle* T, uint32_t index) const;     // vertex normal
  pxr::GfVec3f GetTriangleNormal(uint32_t triangleID) const;           // triangle normal
  
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
  uint32_t GetNumFaceVertices() const {return _numFaceVertices;};
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
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

private:
  // infos
  uint32_t                            _numTets;
  uint32_t                            _numSamples;
  uint32_t                            _numFaces;
  uint32_t                            _numFaceVertices;

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

  // triangle data
  pxr::VtArray<Triangle>              _triangles;

  // half-edge data
  pxr::VtArray<HalfEdge>              _halfEdges;
  pxr::VtArray<int>                   _uniqueEdges;
  pxr::VtArray<int>                   _vertexHalfEdge;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
