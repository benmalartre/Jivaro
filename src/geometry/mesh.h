#ifndef AMN_GEOMETRY_MESH_H
#define AMN_GEOMETRY_MESH_H

#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "triangle.h"
#include "geometry.h"

AMN_NAMESPACE_OPEN_SCOPE

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
    VIRTUAL
  };

  uint32_t                index;     // half edge index
  uint32_t                vertex;    // vertex index
  struct HalfEdge*        twin;      // opposite half-edge
  struct HalfEdge*        next;      // next half-edge
  uint8_t                 latency;   // edge latency

  HalfEdge():vertex(0),twin(NULL),next(NULL),latency(REAL){};
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

class Mesh : public Geometry {
public:
  Mesh();
  Mesh(const Mesh* other, bool normalize = true);
  ~Mesh();

  const pxr::VtArray<int>& GetFaceCounts() const { return _faceCounts;};
  const pxr::VtArray<int>& GetFaceConnects() const { return _faceConnects;};
  pxr::VtArray<int>& GetFaceCounts() { return _faceCounts;};
  pxr::VtArray<int>& GetFaceConnects() { return _faceConnects;};

  pxr::GfVec3f GetPosition(size_t idx) const;
  pxr::GfVec3f GetPosition(const Triangle* T) const;                   // triangle position
  pxr::GfVec3f GetPosition(const Triangle* T, uint32_t index) const;   // vertex position
  pxr::GfVec3f GetNormal(const Triangle* T) const;                     // triangle normal
  pxr::GfVec3f GetNormal(const Triangle* T, uint32_t index) const;     // vertex normal
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
  uint32_t GetFaceNumVertices(uint32_t idx) const {return _faceCounts[idx];};
  uint32_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);

  void ComputeHalfEdges();
  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();

  void Init(
    const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<int>& counts, 
    const pxr::VtArray<int>& connects);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);

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
  uint32_t                            _numFaceVertices;

  // polygonal description
  pxr::VtArray<int>                   _faceCounts;  
  pxr::VtArray<int>                   _faceConnects;

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

  // half-edge data
  pxr::VtArray<HalfEdge>              _halfEdges;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
