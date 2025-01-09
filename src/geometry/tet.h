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
  Tet(const UsdGeomMesh& usdMesh);
  ~Tet();

  const VtArray<int>& GetFaceCounts() const { return _faceVertexCounts;};
  const VtArray<int>& GetFaceConnects() const { return _faceVertexIndices;};
  VtArray<int>& GetFaceCounts() { return _faceVertexCounts;};
  VtArray<int>& GetFaceConnects() { return _faceVertexIndices;};

  GfVec3f GetPosition(size_t idx) const;
  GfVec3f GetTrianglePosition(const Triangle* T) const;                   // triangle position
  GfVec3f GetTriangleVertexPosition(const Triangle* T, uint32_t index) const;   // vertex position
  GfVec3f GetTriangleNormal(const Triangle* T) const;                     // triangle normal
  GfVec3f GetTriangleVertexNormal(const Triangle* T, uint32_t index) const;     // vertex normal
  GfVec3f GetTriangleNormal(uint32_t triangleID) const;           // triangle normal
  
  void SetDisplayColor(GeomInterpolation interp, 
    const VtArray<GfVec3f>& colors);
  const VtArray<GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };
  GfVec3f GetPosition(const Location& point) const ;
  GfVec3f GetNormal(const Location& point) const;
  Triangle* GetTriangle(uint32_t index){return &_triangles[index];};
  VtArray<Triangle>& GetTriangles(){return _triangles;};

  uint32_t GetNumTriangles()const {return _numTriangles;};
  uint32_t GetNumSamples()const {return _numSamples;};
  uint32_t GetNumFaces()const {return _numFaces;};
  uint32_t GetNumFaceVertices() const {return _numFaceVertices;};
  uint32_t GetFaceNumVertices(uint32_t idx) const {return _faceVertexCounts[idx];};
  uint32_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);
  void GetCutVerticesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts);
  void GetCutEdgesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts);

  void ComputeHalfEdges();
  void ComputeNeighbors();
  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();

  void SetTopology(
    const VtArray<GfVec3f>& positions, 
    const VtArray<int>& faceVertexCounts, 
    const VtArray<int>& faceVertexIndices);

  void Init();

  void Update(const VtArray<GfVec3f>& positions);

  // Flatten
  void DisconnectEdges(const VtArray<int>& edges);
  void Flatten(const VtArray<GfVec2d>& uvs, const TfToken& interpolation);

  void Inflate(uint32_t index, float value);
  bool ClosestIntersection(const GfVec3f& origin, 
    const GfVec3f& direction, Location& point, float maxDistance);

  // test (to be removed)
  void PolygonSoup(size_t numPolygons, 
    const GfVec3f& minimum=GfVec3f(-1.f), 
    const GfVec3f& maximum=GfVec3f(1.f));
  void OpenVDBSphere(const float radius, 
    const GfVec3f& center=GfVec3f(0.f));
  void Randomize(float value);
  void TriangularGrid2D(float width, float height, 
    const GfMatrix4f& space, float size);
  void VoronoiDiagram(const std::vector<GfVec3f>& points);

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const GfVec3f& point, Location* hit,
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
  VtArray<int>                   _faceVertexCounts;  
  VtArray<int>                   _faceVertexIndices;

  // colors
  VtArray<GfVec3f>          _colors;
  GeomInterpolation                   _colorsInterpolation;

  // vertex data
  VtArray<bool>                  _boundary;
  VtArray<int>                   _shell;
  VtArray< VtArray<int> >   _neighbors;

  // triangle data
  VtArray<Triangle>              _triangles;

  // half-edge data
  VtArray<HalfEdge>              _halfEdges;
  VtArray<int>                   _uniqueEdges;
  VtArray<int>                   _vertexHalfEdge;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
