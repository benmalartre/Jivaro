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
  
  TrianglePairGraph(const VtArray<int>& faceCounts, 
    const VtArray<int>& faceIndices, const GfVec3f *positions);

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

  float _ComputeEdgeBoundingBoxArea(const GfVec3f *positions, size_t vertex0, size_t vertex1);
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
  VtArray<int>          offsets;
  VtArray<float>        values; 

  float Get(size_t index, size_t n) const {
    return values[offsets[index] + n];
  };
};


// face: area of each face
// vertex: mixed voronoi area of each vertex 
struct MeshAreas {
  VtArray<float>       face;
  VtArray<float>       vertex;

  GfVec4f              faceInfos;    // total, min, max, avg
  GfVec4f              vertexInfos;  // total, min, max, avg
};



class Mesh : public Deformable {
public:
  enum Flag {
    HALFEDGES         = 1 << 0,
    ADJACENTS         = 1 << 1,
    NEIGHBORS         = 1 << 2,
    TRIANGLEPAIRS     = 1 << 3
  };
  Mesh(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Mesh(const UsdGeomMesh& usdMesh, const GfMatrix4d& world, size_t connectivity=0);
  virtual ~Mesh();

  size_t GetFlags(){return _flags;};
  const VtArray<int>& GetFaceCounts() const { return _faceVertexCounts;};
  const VtArray<int>& GetFaceConnects() const { return _faceVertexIndices;};
  VtArray<int>& GetFaceCounts() { return _faceVertexCounts;};
  VtArray<int>& GetFaceConnects() { return _faceVertexIndices;};

  GfVec3f GetPosition(size_t idx) const;
  GfVec3f GetTrianglePosition(const Triangle* T) const;                        // triangle position
  GfVec3f GetTriangleVertexPosition(const Triangle* T, uint32_t index) const;  // vertex position
  GfVec3f GetTriangleNormal(const Triangle* T) const;                          // triangle normal
  GfVec3f GetTriangleVertexNormal(const Triangle* T, uint32_t index) const;    // vertex normal
  GfVec3f GetTriangleNormal(uint32_t triangleID) const;                        // triangle normal
  GfVec3f GetTriangleVelocity(uint32_t triangleId) const;                      // triangle velocity

  const HalfEdge* GetEdge(size_t index) const {return _halfEdges.GetEdge(index);};
  HalfEdge* GetEdge(size_t index){return _halfEdges.GetEdge(index);};
  VtArray<HalfEdge>& GetEdges(){return _halfEdges.GetEdges();};
  HalfEdgeGraph* GetEdgesGraph(){return &_halfEdges;};
  const VtArray<HalfEdge>& GetEdges()const{return _halfEdges.GetEdges();};
  const HalfEdgeGraph* GetEdgesGraph()const{return &_halfEdges;};

  size_t GetNumAdjacents(size_t index) const;
  size_t GetTotalNumAdjacents() const;
  const int* GetAdjacents(size_t index) const;
  int GetAdjacent(size_t index, size_t adjacent) const;
  int GetAdjacentIndex(size_t index, size_t adjacent) const;
  
  size_t GetNumNeighbors(size_t index) const;
  size_t GetTotalNumNeighbors() const;
  const int* GetNeighbors(size_t index) const;
  int GetNeighbor(size_t index, size_t neighbor) const;
  int GetNeighborIndex(size_t index, size_t neighbor) const;
  
  Triangle* GetTriangle(uint32_t index) {return &_triangles[index];};
  const Triangle* GetTriangle(uint32_t index) const {return &_triangles[index];};
  VtArray<Triangle>& GetTriangles(){return _triangles;};
  const VtArray<Triangle>& GetTriangles() const {return _triangles;};
  VtArray<TrianglePair>& GetTrianglePairs();

  size_t GetNumTriangles()const {return _triangles.size();};
  size_t GetNumSamples()const {return _faceVertexIndices.size();};
  size_t GetNumFaces()const {return _faceVertexCounts.size();};
  size_t GetNumEdges()const { return _halfEdges.GetNumEdges(); };

  size_t GetFaceNumVertices(uint32_t idx) const {return _faceVertexCounts[idx];};
  size_t GetFaceVertexIndex(uint32_t face, uint32_t vertex);
  void GetCutVerticesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts);
  void GetCutEdgesFromUVs(const VtArray<GfVec2d>& uvs, VtArray<int>* cuts);

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

  void GetAllTrianglePairs(VtArray<TrianglePair>& pairs, bool unique=false);

  void Init(size_t connectivity=0);

  // points (deformation)
  void SetPositions(const GfVec3f* positions, size_t n) override;
  void SetPositions(const VtArray<GfVec3f>& positions) override;

  // topology
  void Set(
    const VtArray<GfVec3f>& positions,
    const VtArray<int>& faceVertexCounts,
    const VtArray<int>& faceVertexIndices, 
    bool init=true,
    size_t connectivity=0
  );
  void SetTopology(
    const VtArray<int>& faceVertexCounts,
    const VtArray<int>& faceVertexIndices,
    bool init=true,
    size_t connectivity=0
  );
  bool FlipEdge(HalfEdge* edge);
  bool SplitEdge(HalfEdge* edge);
  bool CollapseEdge(HalfEdge* edge);
  bool RemovePoint(size_t index);

  // Flatten
  void DisconnectEdges(const VtArray<int>& edges);
  void Flatten(const VtArray<GfVec2d>& uvs, const TfToken& interpolation);

  void Inflate(uint32_t index, float value);
  bool ClosestIntersection(const GfVec3f& origin, 
    const GfVec3f& direction, Location& location, float maxDistance);

  // test (to be removed)
  void Random2DPattern(size_t numFaces);
  void PolygonSoup(size_t numPolygons, 
    const GfVec3f& minimum=GfVec3f(-1.f), 
    const GfVec3f& maximum=GfVec3f(1.f));
  void ColoredPolygonSoup(size_t numPolygons, 
    const GfVec3f& minimum = GfVec3f(-1.f),
    const GfVec3f& maximum = GfVec3f(1.f));
  void OpenVDBSphere(const float radius, 
    const GfVec3f& center=GfVec3f(0.f));
  void Randomize(float value);
  void Cube();
  void TriangularGrid2D(float spacing, const GfMatrix4f& space=GfMatrix4f(1.f));
  void RegularGrid2D(float spacing, const GfMatrix4f& space=GfMatrix4f(1.f));
  void RegularGrid2D(size_t subdivX, size_t subdivY, float sizeX, float sizeY, 
    const GfMatrix4f& space=GfMatrix4f(1.f));
  void VoronoiDiagram(const std::vector<GfVec3f>& points);


  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default()) override;

private:
  int                                 _flags;
  // polygonal description
  VtArray<int>                   _faceVertexCounts;  
  VtArray<int>                   _faceVertexIndices;

  // triangle data
  VtArray<Triangle>              _triangles;
  VtArray<TrianglePair>          _trianglePairs;

  // half-edge data
  HalfEdgeGraph                       _halfEdges;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif
