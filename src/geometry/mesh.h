#ifndef JVR_GEOMETRY_MESH_H
#define JVR_GEOMETRY_MESH_H

#include <pxr/usd/usdGeom/mesh.h>


#include "../geometry/triangle.h"
#include "../geometry/deformable.h"
#include "../geometry/halfEdge.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh : public Deformable {
public:
  enum Flag {
    NEIGHBORS     = 1 << 0,
    HALFEDGES     = 1 << 1,
    TRIANGLEPAIRS = 1 << 2
  };
  Mesh(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Mesh(const pxr::UsdGeomMesh& usdMesh, const pxr::GfMatrix4d& world);
  virtual ~Mesh();

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

  pxr::VtArray<HalfEdge>& GetEdges();
  HalfEdgeGraph* GetEdgesGraph();
  size_t GetLongestEdgeInTriangle(const pxr::GfVec3i& vertices);

  const pxr::VtArray<pxr::VtArray<int>>& GetNeighbors();
  void ComputeNeighbors(size_t pointIdx, pxr::VtArray<int>& neighbors);
  
  const Triangle* GetTriangle(uint32_t index) const {return &_triangles[index];};
  pxr::VtArray<Triangle>& GetTriangles(){return _triangles;};
  pxr::VtArray<TrianglePair>& GetTrianglePairs();

  size_t GetNumTriangles()const {return _triangles.size();};
  size_t GetNumSamples()const {return _faceVertexIndices.size();};
  size_t GetNumFaces()const {return _faceVertexCounts.size();};
  size_t GetNumEdges()const { return _halfEdges.GetNumEdges(); };

  float GetAverageEdgeLength();

  size_t GetFaceNumVertices(uint32_t idx) const {return _faceVertexCounts[idx];};
  size_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);
  void GetCutVerticesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);
  void GetCutEdgesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);

  void ComputeHalfEdges();
  void ComputeNeighbors();
  void ComputeTrianglePairs();

  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();
  void Triangulate();
  void TriangulateFace(const HalfEdge* edge);

  void GetAllTrianglePairs(pxr::VtArray<TrianglePair>& pairs);

  void Prepare(bool connectivity=true);

  DirtyState Sync(pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, float time) override;


  // points (deformation)
  void SetPositions(const pxr::GfVec3f* positions, size_t n) override;
  void SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions) override;

  // topology
  void Set(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<int>& faceVertexCounts,
    const pxr::VtArray<int>& faceVertexIndices, 
    bool init=true
  );
  void SetTopology(
    const pxr::VtArray<int>& faceVertexCounts,
    const pxr::VtArray<int>& faceVertexIndices,
    bool init=true
  );
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge);
  bool CollapseEdge(HalfEdge* edge);
  bool RemovePoint(size_t index);

  // Flatten
  void DisconnectEdges(const pxr::VtArray<int>& edges);
  void Flatten(const pxr::VtArray<pxr::GfVec2d>& uvs, const pxr::TfToken& interpolation);

  void Inflate(uint32_t index, float value);
  bool ClosestIntersection(const pxr::GfVec3f& origin, 
    const pxr::GfVec3f& direction, Location& location, float maxDistance);

  // test (to be removed)
  void Random2DPattern(size_t numFaces);
  void PolygonSoup(size_t numPolygons, 
    const pxr::GfVec3f& minimum=pxr::GfVec3f(-1.f), 
    const pxr::GfVec3f& maximum=pxr::GfVec3f(1.f));
  void ColoredPolygonSoup(size_t numPolygons, 
    const pxr::GfVec3f& minimum = pxr::GfVec3f(-1.f),
    const pxr::GfVec3f& maximum = pxr::GfVec3f(1.f));
  void MaterializeSamples(const pxr::VtArray<pxr::GfVec3f>& points, float size=0.1f);
  void OpenVDBSphere(const float radius, 
    const pxr::GfVec3f& center=pxr::GfVec3f(0.f));
  void Randomize(float value);
  void TriangularGrid2D(float spacing, const pxr::GfMatrix4f& space=pxr::GfMatrix4f(1.f));
  void RegularGrid2D(float spacing, const pxr::GfMatrix4f& space=pxr::GfMatrix4f(1.f));
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
  int                                 _flags;
  // polygonal description
  pxr::VtArray<int>                   _faceVertexCounts;  
  pxr::VtArray<int>                   _faceVertexIndices;

  // shell data (vertices)
  pxr::VtArray< pxr::VtArray<int> >   _shells;

  // triangle data
  pxr::VtArray<Triangle>              _triangles;
  pxr::VtArray<TrianglePair>          _trianglePairs;

  // half-edge data
  HalfEdgeGraph                       _halfEdges;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
