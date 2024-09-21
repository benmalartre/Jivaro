#ifndef JVR_GEOMETRY_MESH_H
#define JVR_GEOMETRY_MESH_H

#include <vector>
#include <unordered_map>
#include <pxr/usd/usdGeom/mesh.h>

#include "../geometry/component.h"
#include "../geometry/triangle.h"
#include "../geometry/deformable.h"
#include "../geometry/halfEdge.h"

JVR_NAMESPACE_OPEN_SCOPE

//https://www.highperformancegraphics.org/posters23/Fast_Triangle_Pairing_for_Ray_Tracing.pdf
//
class TrianglePairGraph {
public:
  using _Pairs = std::vector<std::pair<size_t, size_t>> ;
  using _Pair = _Pairs::value_type;
  
  TrianglePairGraph(const pxr::VtArray<int>& faceCounts, 
    const pxr::VtArray<int>& faceIndices, const pxr::GfVec3f *positions);

  _Pairs& GetPairs() { return _pairs; };

protected:
  struct _HalfEdge {
    float   area;
    size_t  vertex0;
    size_t  vertex1;
    size_t  triangle;

    bool operator<(const _HalfEdge& other) const {
      return 
        area > other.area || 
        (area == other.area && vertex0 > other.vertex0) || 
        (area == other.area && vertex0 ==other.vertex0 && vertex1 > other.vertex1);
    }

    bool operator==(const _HalfEdge& other) const {
      return vertex0 == other.vertex0 && vertex1 == other.vertex1;
    }
  };

  float _ComputeEdgeBoundingBoxArea(const pxr::GfVec3f *positions, size_t vertex0, size_t vertex1);
  bool _PairTriangles(size_t tri0, size_t tri11);
  void _SingleTriangle(size_t tri);

private:
  std::vector<_HalfEdge>  _halfEdges;
  std::vector<bool>       _paired;
  _Pairs                  _pairs;
};


// offsets = vertex cotangent weights offset in value array
// values = per adjacents vertex pair cotangent weight
struct MeshCotangentWeights {
  pxr::VtArray<int>          offsets;
  pxr::VtArray<float>        values; 

  float Get(size_t index, size_t n) {
    return values[offsets[index] + n];
  };
};


// face: area of each face
// vertex: mixed voronoi area of each vertex 
struct MeshAreas {
  pxr::VtArray<float>       face;
  pxr::VtArray<float>       vertex;

  pxr::GfVec4f              faceInfos;    // total, min, max, avg
  pxr::GfVec4f              vertexInfos;  // total, min, max, avg
};



class Mesh : public Deformable {
public:
  enum Flag {
    HALFEDGES         = 1 << 0,
    ADJACENTS         = 1 << 1,
    NEIGHBORS         = 1 << 2,
    TRIANGLEPAIRS     = 1 << 3
  };
  Mesh(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Mesh(const pxr::UsdGeomMesh& usdMesh, const pxr::GfMatrix4d& world, size_t connectivity=0);
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

  const HalfEdge* GetEdge(size_t index) const {return _halfEdges.GetEdge(index);};
  HalfEdge* GetEdge(size_t index){return _halfEdges.GetEdge(index);};
  pxr::VtArray<HalfEdge>& GetEdges(){return _halfEdges.GetEdges();};
  HalfEdgeGraph* GetEdgesGraph(){return &_halfEdges;};
  const pxr::VtArray<HalfEdge>& GetEdges()const{return _halfEdges.GetEdges();};
  const HalfEdgeGraph* GetEdgesGraph()const{return &_halfEdges;};

  size_t GetNumAdjacents(size_t index);
  size_t GetTotalNumAdjacents();
  const int* GetAdjacents(size_t index);
  int GetAdjacent(size_t index, size_t adjacent);
  int GetAdjacentIndex(size_t index, size_t adjacent);
  
  size_t GetNumNeighbors(size_t index);
  size_t GetTotalNumNeighbors();
  const int* GetNeighbors(size_t index);
  int GetNeighbor(size_t index, size_t neighbor);
  int GetNeighborIndex(size_t index, size_t neighbor);
  
  Triangle* GetTriangle(uint32_t index) {return &_triangles[index];};
  const Triangle* GetTriangle(uint32_t index) const {return &_triangles[index];};
  pxr::VtArray<Triangle>& GetTriangles(){return _triangles;};
  pxr::VtArray<TrianglePair>& GetTrianglePairs();

  size_t GetNumTriangles()const {return _triangles.size();};
  size_t GetNumSamples()const {return _faceVertexIndices.size();};
  size_t GetNumFaces()const {return _faceVertexCounts.size();};
  size_t GetNumEdges()const { return _halfEdges.GetNumEdges(); };

  size_t GetFaceNumVertices(uint32_t idx) const {return _faceVertexCounts[idx];};
  size_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);
  void GetCutVerticesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);
  void GetCutEdgesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts);

  void ComputeHalfEdges();
  void ComputeNeighbors();
  void ComputeAdjacents();
  void ComputeTrianglePairs();
  float ComputeAverageEdgeLength();

  void ComputeCotangentWeights(MeshCotangentWeights& results);
  void ComputeAreas(MeshAreas& results);

  float TriangleArea(uint32_t index);
  float AveragedTriangleArea();
  void Triangulate();
  void TriangulateFace(const HalfEdge* edge);

  void GetAllTrianglePairs(pxr::VtArray<TrianglePair>& pairs, bool unique=false);

  void Init(size_t connectivity=0);

  // points (deformation)
  void SetPositions(const pxr::GfVec3f* positions, size_t n) override;
  void SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions) override;

  // topology
  void Set(
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<int>& faceVertexCounts,
    const pxr::VtArray<int>& faceVertexIndices, 
    bool init=true,
    size_t connectivity=0
  );
  void SetTopology(
    const pxr::VtArray<int>& faceVertexCounts,
    const pxr::VtArray<int>& faceVertexIndices,
    bool init=true,
    size_t connectivity=0
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
  void OpenVDBSphere(const float radius, 
    const pxr::GfVec3f& center=pxr::GfVec3f(0.f));
  void Randomize(float value);
  void Cube();
  void TriangularGrid2D(float spacing, const pxr::GfMatrix4f& space=pxr::GfMatrix4f(1.f));
  void RegularGrid2D(float spacing, const pxr::GfMatrix4f& space=pxr::GfMatrix4f(1.f));
  void VoronoiDiagram(const std::vector<pxr::GfVec3f>& points);


  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;

private:
  int                                 _flags;
  // polygonal description
  pxr::VtArray<int>                   _faceVertexCounts;  
  pxr::VtArray<int>                   _faceVertexIndices;

  // triangle data
  pxr::VtArray<Triangle>              _triangles;
  pxr::VtArray<TrianglePair>          _trianglePairs;

  // half-edge data
  HalfEdgeGraph                       _halfEdges;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif
