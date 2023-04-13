// Mesh
//----------------------------------------------

#include <cmath>
#include <unordered_map>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/mesh.h>

#include "../geometry/mesh.h"
#include "../geometry/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

const HalfEdge* 
Mesh::GetLongestEdgeInTriangle(const HalfEdge* edge)
{
  return edge->GetLongestEdgeInTriangle(&_positions[0]);
}

Mesh::Mesh()
  : Geometry(Geometry::MESH)
  , _flags(0)
  , _halfEdges()
{
  _initialized = false;
}

Mesh::Mesh(const Mesh* other, bool normalize)
  : Geometry(other, Geometry::MESH, normalize)
  , _flags(0)
{
  _initialized = true;
  _normals = other->_normals;

  size_t numTriangles = other->GetNumTriangles();
  _triangles.resize(numTriangles);
  memcpy(
    &_triangles[0], 
    &other->_triangles[0], 
    numTriangles * sizeof(Triangle));
}

Mesh::Mesh(const pxr::UsdGeomMesh& mesh)
  : Geometry(Geometry::MESH)
  , _flags(0)
{
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  pointsAttr.Get(&_positions, pxr::UsdTimeCode::Default());
  faceVertexCountsAttr.Get(&_faceVertexCounts, pxr::UsdTimeCode::Default());
  faceVertexIndicesAttr.Get(&_faceVertexIndices, pxr::UsdTimeCode::Default());

  Init();
}

Mesh::~Mesh()
{
}

size_t Mesh::GetFaceVertexIndex(uint32_t face, uint32_t vertex)
{
  size_t accum = 0;
  for(size_t i=0; i < face; ++i)accum += _faceVertexCounts[i];
  return _faceVertexIndices[accum + vertex];
}

void Mesh::GetCutVerticesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts)
{
  size_t numVertices = _positions.size();

  pxr::VtArray<int> visited(numVertices);
  for (auto& v : visited) v = 0;
  
  pxr::VtArray<pxr::GfVec2d> uvPositions(numVertices);
  
  size_t uvIndex = 0;
  for (const int& faceVertexIndex : _faceVertexIndices) {
    pxr::GfVec2d uv = uvs[uvIndex++];
    switch (visited[faceVertexIndex]) {
      case 0:
        uvPositions[faceVertexIndex] = uv;
        visited[faceVertexIndex]++;
        break;
      case 1:
        if (uv != uvPositions[faceVertexIndex]) {
          cuts->push_back(faceVertexIndex);
          visited[faceVertexIndex]++;
        }
        break;
      default:
        continue;
    }
  }
  
  cuts->resize(2);
  (*cuts)[0] = 1;
  (*cuts)[1] = 3;
}

static bool _IsHalfEdgesUVSeparated(const HalfEdge* lhs, const HalfEdge* rhs)
{
  return false;
}


void Mesh::GetCutEdgesFromUVs(const pxr::VtArray<pxr::GfVec2d>& uvs, pxr::VtArray<int>* cuts)
{
  /*
  cuts->reserve(_uniqueEdges.size());
  for (const HalfEdge& halfEdge: _halfEdges) {
    if (halfEdge.latency == HalfEdge::REAL && halfEdge.twin) {
      const HalfEdge* next = &_halfEdges[halfEdge.next];
      const pxr::GfVec2d& start_lhs = uvs[halfEdge.index];
      const pxr::GfVec2d& end_lhs = uvs[next->index];
      const HalfEdge* twin = &_halfEdges[halfEdge.twin];
      const pxr::GfVec2d& start_rhs = uvs[twin->next];
      const pxr::GfVec2d& end_rhs = uvs[_halfEdges[twin->next].next];
      if (start_lhs != end_rhs) cuts->push_back(halfEdge.index);
      if (end_rhs != start_rhs) cuts->push_back(halfEdge.twin);
    }
  }
*/
}

pxr::GfVec3f 
Mesh::GetPosition(size_t idx) const
{
  return _positions[idx];
}

pxr::GfVec3f 
Mesh::GetTrianglePosition(const Triangle* T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v=0;v<3;v++) center += _positions[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f 
Mesh::GetTriangleVertexPosition(const Triangle *T, uint32_t index) const
{
  return _positions[T->vertices[index]];
}

pxr::GfVec3f 
Mesh::GetPosition(const Location& point) const
{
  const Triangle* T = &_triangles[point.id];
  pxr::GfVec3f pos(0.f);
  for(uint32_t i = 0; i < 3; ++i) 
    pos += _positions[T->vertices[i]] * point.baryCoords[i];
  return pos;
}

pxr::GfVec3f 
Mesh::GetTriangleNormal(const Triangle *T) const
{
  pxr::GfVec3f center(0.f);
  for(uint32_t v = 0; v < 3; ++v)
      center += _normals[T->vertices[v]];
  center *= 1.f/3.f;
  return center;
}

pxr::GfVec3f 
Mesh::GetTriangleVertexNormal(const Triangle *T, uint32_t index) const
{
  return _normals[T->vertices[index]];
}

pxr::GfVec3f 
Mesh::GetNormal(const Location& point) const
{
  const Triangle* T = &_triangles[point.id];
  pxr::GfVec3f nrm(0.f);
  for(uint32_t i=0;i<3;i++) 
    nrm += _normals[T->vertices[i]] * point.baryCoords[i];
  return nrm;
}

pxr::GfVec3f 
Mesh::GetTriangleNormal(uint32_t triangleID) const
{
  const Triangle* T = &_triangles[triangleID];
  pxr::GfVec3f A = _positions[T->vertices[0]];
  pxr::GfVec3f B = _positions[T->vertices[1]];
  pxr::GfVec3f C = _positions[T->vertices[2]];

  B -= A;
  C -= A;

  return (B ^ C).GetNormalized();
}

const pxr::VtArray<HalfEdge*>&
Mesh::GetEdges()
{
  return _halfEdges.GetEdges();
}

bool Mesh::RemovePoint(size_t index)
{
  if (index >= _positions.size()) {
    std::cout << "error index out of range" << std::endl;
    return false;
  }
  _positions.erase(_positions.begin() + index);
  _normals.erase(_normals.begin() + index);

  return true;
}

void Mesh::ComputeHalfEdges()
{
  _halfEdges.ComputeGraph(this);
  _flags = Mesh::HALFEDGES;
}

HalfEdge* Mesh::GetLongestEdge()
{
  return _halfEdges.GetLongestEdge(&_positions[0]);
}

HalfEdge* Mesh::GetShortestEdge()
{
  return _halfEdges.GetShortestEdge(&_positions[0]);
}

HalfEdge* Mesh::GetRandomEdge()
{
  return _halfEdges.GetRandomEdge();
}


void Mesh::ComputeTrianglePairs()
{
  _trianglePairs.clear();
  _halfEdges.ComputeTrianglePairs(this);
  BITMASK_SET(_flags, Mesh::TRIANGLEPAIRS);
}

pxr::VtArray<TrianglePair>& 
Mesh::GetTrianglePairs()
{
  if (!(_flags & Mesh::TRIANGLEPAIRS)) {
    ComputeTrianglePairs();
  }
  return _trianglePairs;
}


void Mesh::ComputeNeighbors()
{
  _halfEdges.ComputeNeighbors(this);
  BITMASK_SET(_flags, Mesh::NEIGHBORS);
}

void Mesh::Set(
  const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& faceVertexCounts,
  const pxr::VtArray<int>& faceVertexIndices,
  bool init
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;
  _positions = positions;
  _normals = positions;
  if(init)Init();
}

void Mesh::SetTopology(
  const pxr::VtArray<int>& faceVertexCounts,
  const pxr::VtArray<int>& faceVertexIndices,
  bool init
)
{
  _faceVertexCounts = faceVertexCounts;
  _faceVertexIndices = faceVertexIndices;

  if(init)Init();
}

void Mesh::UpdateTopologyFromEdges()
{
  _halfEdges.UpdateTopologyFromEdges(this);
}

void Mesh::Init()
{
  size_t numPoints = _positions.size();
  // build triangles
  TriangulateMesh(_faceVertexCounts, _faceVertexIndices, _triangles);

  // compute normals
  ComputeVertexNormals(_positions, _faceVertexCounts, 
    _faceVertexIndices, _triangles, _normals);

  // compute half-edges
  ComputeHalfEdges();
  ComputeBoundingBox();
  // compute neighbors
  //ComputeNeighbors();
}

void 
Mesh::Update(const pxr::VtArray<pxr::GfVec3f>& positions)
{
  _positions = positions;
}

static int 
_CountPointCuts(const std::vector<bool>& doCutEdge, HalfEdge* start) {
  HalfEdge* current = start;

  int count = 0;
  /*
  bool search = true;
  while (search) {
    if (current->twin != HalfEdge::INVALID_INDEX) {
      if (doCutEdge[current->index] && doCutEdge[_halfEdges[current->twin]]) {
        count++;
      }
      current = current->twin->next;
    } else {
      search = false;
    }
    if (current == start)search = false;
  }
  current = start;
  
  bool reverseSearch = true;
  while (reverseSearch) {
    HalfEdge* previous = _GetPreviousEdge(current)->twin;
    if (doCutEdge[current->index] && doCutEdge[previous->twin->index]) {
      count++;
      current = previous;
    }
    else {
      reverseSearch = false;
    }
  }
  */
  return count;
}

bool Mesh::FlipEdge(HalfEdge* edge)
{
  return _halfEdges.FlipEdge(edge);
}

bool Mesh::SplitEdge(HalfEdge* edge)
{
  size_t numPoints = GetNumPoints();
  const pxr::GfVec3f p =
    (_positions[edge->vertex] + _positions[edge->next->vertex]) * 0.5f;

  AddPoint(p);
  return _halfEdges.SplitEdge(edge, numPoints);
}


bool Mesh::CollapseEdge(HalfEdge* edge)
{
  size_t p1 = edge->vertex;
  size_t p2 = edge->next->vertex;

  if (_halfEdges.CollapseEdge(edge)) {
    if (p1 > p2) {
      _positions[p2] = (_positions[p1] + _positions[p2]) * 0.5f;
      RemovePoint(p1);
    }
    else {
      _positions[p1] = (_positions[p1] + _positions[p2]) * 0.5f;
      RemovePoint(p2);
    }
  }
  return true;
}

void Mesh::DisconnectEdges(const pxr::VtArray<int>& cutEdges)
{

}

void Mesh::Flatten(const pxr::VtArray<pxr::GfVec2d>& uvs, const pxr::TfToken& interpolation)
{
  if (interpolation == pxr::UsdGeomTokens->vertex) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const pxr::GfVec2d& uv = uvs[i];
      _positions[i] = pxr::GfVec3f(uv[0], 0.f, uv[1]);
    }
  } else if (interpolation == pxr::UsdGeomTokens->faceVarying) {
    for (size_t i = 0; i < uvs.size(); ++i) {
      const pxr::GfVec2d& uv = uvs[i];
      _positions[_faceVertexIndices[i]] = pxr::GfVec3f(uv[0], 0.f, uv[1]);
    }
  }
}

void Mesh::Inflate(uint32_t index, float value)
{
  /*
  uint32_t offsetP = _submeshes[index]._offsetPositions;
  for(uint32_t i=0;i<_submeshes[index]._numPoints;i++)
  {
      _position[offsetP+i*3] += _normal[offsetP+i*3]*value;
      _position[offsetP+i*3+1] += _normal[offsetP+i*3+1]*value;
      _position[offsetP+i*3+2] += _normal[offsetP+i*3+2]*value;
  }
  */
}

float Mesh::TriangleArea(uint32_t i)
{
  Triangle* T = &_triangles[i];
  pxr::GfVec3f A = GetTriangleVertexPosition(T, 0);
  pxr::GfVec3f B = GetTriangleVertexPosition(T, 1);
  pxr::GfVec3f C = GetTriangleVertexPosition(T, 2);
  pxr::GfVec3f AB = B - A;
  pxr::GfVec3f AC = C - A;
  pxr::GfVec3f BC = C - B;

  float a = AB.GetLength();
  float b = AC.GetLength();
  float c = BC.GetLength();
  float s = (a + b + c) / 2;
  return sqrt(s * (s - a) * (s - b) * (s - c));
}

float Mesh::AveragedTriangleArea()
{
  if(_triangles.size())
  {
    float averaged = 0;
    for(uint32_t t=0;t<_triangles.size();t++)
        averaged += TriangleArea(t);
    averaged /= (float)_triangles.size();
    return averaged;
  }
  else return 0;
}

void Mesh::Triangulate()
{
  /*
  _halfEdges.SetAllEdgesLatencyReal();
  UpdateTopologyFromEdges();
  */
}

bool Mesh::ClosestIntersection(const pxr::GfVec3f& origin, 
  const pxr::GfVec3f& direction, Location& result, float maxDistance)
{
  pxr::GfRay ray(origin, direction);

  pxr::GfVec3d p0, p1, p2;
  pxr::GfVec3d baryCoords;
  double distance;
  double minDistance = DBL_MAX;
  bool frontFacing;
  bool hit = false;
  Triangle* tri;
  for(uint32_t t = 0;t<_triangles.size();t++)
  {
    tri = &_triangles[t];
    p0 = GetTriangleVertexPosition(tri, 0);
    p1 = GetTriangleVertexPosition(tri, 1);
    p2 = GetTriangleVertexPosition(tri, 2);

    if(ray.Intersect(p0, p1, p2, &distance, &baryCoords, &frontFacing, maxDistance))
    {
      if(distance < minDistance)
      {
        minDistance = distance;
        result.baryCoords = pxr::GfVec3f(baryCoords);
        result.id = tri->id;
        hit = true;
      }
    }
  }
  return hit;
}

void Mesh::PolygonSoup(size_t numPolygons, const pxr::GfVec3f& minimum, 
  const pxr::GfVec3f& maximum)
{
  size_t numTriangles = numPolygons;
  size_t numPoints = numPolygons * 3;
  pxr::VtArray<pxr::GfVec3f> position(numPoints);

  float radius = (maximum - minimum).GetLength() / 10.f;
  for(size_t i=0; i < numPolygons; ++i) {
    float bx = RANDOM_0_1;
    float by = RANDOM_0_1;
    float bz = RANDOM_0_1;
    pxr::GfVec3f center(
      minimum[0] * (1.f - bx) + maximum[0] * bx,
      minimum[1] * (1.f - by) + maximum[1] * by,
      minimum[2] * (1.f - bz) + maximum[2] * bz
    );

    pxr::GfVec3f normal(
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f),
      RANDOM_LO_HI(-1.f, 1.f)
    );

    normal.Normalize();

    position[i * 3 + 0] = center + pxr::GfVec3f(1.f, 0.f, 0.f);
    position[i * 3 + 1] = center + pxr::GfVec3f(0.f, 1.f, 0.f);
    position[i * 3 + 2] = center + pxr::GfVec3f(0.f, 0.f, 1.f);
  }

  pxr::VtArray<int> faceVertexCount(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  pxr::VtArray<int> faceVertexConnect(numPoints);
  for(size_t i=0; i < numPoints; ++i) {
    faceVertexConnect[i] = i;
  }
  Set(position, faceVertexCount, faceVertexConnect);
}

void 
Mesh::MaterializeSamples(const pxr::VtArray<pxr::GfVec3f>& points, float size)
{
  size_t numTriangles = points.size();
  size_t numPoints = numTriangles * 3;
  pxr::VtArray<int> faceVertexCount(numTriangles);
  for (size_t i = 0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  pxr::VtArray<int> faceVertexConnect(numPoints);
  for (size_t i = 0; i < numPoints; ++i) {
    faceVertexConnect[i] = i;
  }

  pxr::VtArray<pxr::GfVec3f> positions(numPoints);
  for (size_t p = 0; p < numTriangles; ++p) {
    positions[p * 3] = points[p] - pxr::GfVec3f(size, 0, 0);
    positions[p * 3 + 1] = points[p] + pxr::GfVec3f(size, 0, 0);
    positions[p * 3 + 2] = points[p] + pxr::GfVec3f(0, size, 0);
  }
  Set(positions, faceVertexCount, faceVertexConnect);
}

void Mesh::ColoredPolygonSoup(size_t numPolygons, 
  const pxr::GfVec3f& minimum, const pxr::GfVec3f& maximum)
{
  //mesh->PolygonSoup(65535);
  pxr::GfMatrix4f space(1.f);
  TriangularGrid2D(10.f, 6.f, space, 0.2f);
  Randomize(0.05f);
}

Mesh* MakeOpenVDBSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{
  Mesh* mesh = new Mesh();
  
  mesh->OpenVDBSphere(6.66, pxr::GfVec3f(3.f, 7.f, 4.f));

  pxr::UsdGeomMesh vdbSphere = pxr::UsdGeomMesh::Define(stage, path);
  vdbSphere.CreatePointsAttr(pxr::VtValue(mesh->GetPositions()));
  vdbSphere.CreateNormalsAttr(pxr::VtValue(mesh->GetNormals()));
  vdbSphere.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh->GetFaceConnects()));
  vdbSphere.CreateFaceVertexCountsAttr(pxr::VtValue(mesh->GetFaceCounts()));

  vdbSphere.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));

  std::cout << "CREATED OPENVDB SPHERE !!!" << std::endl;

  return mesh;
}


void Mesh::Random2DPattern()
{
  size_t numFaces = RANDOM_0_X(5) + 3;
  pxr::VtArray<pxr::GfVec3f> points(numFaces * 2 + 2);
  pxr::VtArray<int> faceCounts(numFaces * 2);
  pxr::VtArray<int> faceConnects(numFaces * 6);

  for (size_t face = 0; face < numFaces; ++face) {
    faceCounts[face * 2    ] = 3;
    faceCounts[face * 2 + 1] = 3;
    faceConnects[face * 6    ] = face * 2;
    faceConnects[face * 6 + 1] = face * 2 + 2;
    faceConnects[face * 6 + 2] = face * 2 + 1;
    faceConnects[face * 6 + 3] = face * 2 + 2;
    faceConnects[face * 6 + 4] = face * 2 + 3;
    faceConnects[face * 6 + 5] = face * 2 + 1;

    points[face * 2] = pxr::GfVec3f(face, 0, RANDOM_0_1);
    points[face * 2 + 1] = pxr::GfVec3f(face, 0, -RANDOM_0_1);
  }

  points[numFaces * 2] = pxr::GfVec3f(numFaces, 0, RANDOM_0_1);
  points[numFaces * 2 + 1] = pxr::GfVec3f(numFaces, 0, -RANDOM_0_1);

  Set(points, faceCounts, faceConnects);
}


void Mesh::TriangularGrid2D(float width, float height, const pxr::GfMatrix4f& space, float size)
{
  size_t numX = (width / size ) * 0.5 + 1;
  size_t numY = (height / size) + 1;
  size_t numPoints = numX * numY;
  size_t numTriangles = (numX - 1) * 2 * (numY - 1);
  size_t numSamples = numTriangles * 3;
  pxr::VtArray<pxr::GfVec3f> position(numPoints);

  float spaceX = size * 2.0 / width;
  float spaceY = size / height;

  for(size_t y = 0; y < numY; ++y) {
    for(size_t x = 0; x < numX; ++x) {
      size_t vertexId = y * numX + x;
      if(y %2 == 0)position[vertexId][0] = x * spaceX + spaceX * 0.5f;
      else position[vertexId][0] = x * spaceX;
      position[vertexId][1] = 0.f;
      position[vertexId][2] = y * spaceY;
      position[vertexId] = space.Transform(position[vertexId]);
    }
  }
  
  pxr::VtArray<int> faceVertexCount(numTriangles);
  for(size_t i=0; i < numTriangles; ++i) {
    faceVertexCount[i] = 3;
  }

  size_t numRows = numY - 1;
  size_t numTrianglesPerRow = (numX - 1) ;
  pxr::VtArray<int> faceVertexConnect(numSamples);
  
  size_t k = 0;
  for(size_t i=0; i < numRows; ++i) {
    for (size_t j = 0; j < numTrianglesPerRow; ++j) {
      if (i % 2 == 0) {
        faceVertexConnect[k++] = (i + 1) * numX + j + 1; 
        faceVertexConnect[k++] = i * numX + j + 1;
        faceVertexConnect[k++] = i * numX + j;

        faceVertexConnect[k++] = i * numX + j; 
        faceVertexConnect[k++] = (i + 1) * numX + j;
        faceVertexConnect[k++] = (i + 1) * numX + j + 1;
      }
      else {
        faceVertexConnect[k++] = (i + 1) * numX + j; 
        faceVertexConnect[k++] = i * numX + j + 1;
        faceVertexConnect[k++] = i * numX + j;

        faceVertexConnect[k++] = i * numX + j + 1; 
        faceVertexConnect[k++] = (i + 1) * numX + j;
        faceVertexConnect[k++] = (i + 1) * numX + j + 1;
      }
    }
  }
  
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  for(size_t i=0; i < numPoints / 3; ++i) {
    pxr::GfVec3f color(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
    colors[i * 3] = color;
    colors[i * 3 + 1] = color;
    colors[i * 3 + 2] = color;
  }
 

  Set(position, faceVertexCount, faceVertexConnect);
}

void Mesh::OpenVDBSphere(float radius, const pxr::GfVec3f& center)
{

}

void Mesh::VoronoiDiagram(const std::vector<pxr::GfVec3f>& points)
{
  /*
  size_t numPoints = points.size();
  std::vector<mygal::Vector2<double>> _positions(numPoints);
  for (size_t p=0; p < numPoints; ++p) {
    _positions[p].x = points[p][0];
    _positions[p].y = points[p][2];
  }
  mygal::FortuneAlgorithm<double> algorithm(_positions);
  algorithm.construct();

  algorithm.bound(mygal::Box<double>{-0.55, -0.55, 0.55, 0.55});
  mygal::Diagram<double> diagram = algorithm.getDiagram();
  diagram.intersect(mygal::Box<double>{-0.5, -0.5, 0.5, 0.5});

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<int> faceVertexCounts;
  pxr::VtArray<int> faceVertexConnects;
  pxr::VtArray<pxr::GfVec3f> colors;

  for (const auto& site : diagram.getSites())
  {
    auto center = site.point;
    auto face = site.face;
    auto halfEdge = face->outerComponent;
    if (halfEdge == nullptr)
      continue;
    while (halfEdge->prev != nullptr)
    {
      halfEdge = halfEdge->prev;
      if (halfEdge == face->outerComponent)
        break;
    }
    auto start = halfEdge;
    const pxr::GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    size_t numFaceVertices = 0;
    if (halfEdge->origin != nullptr) {
      auto origin = (halfEdge->origin->point - center) + center;
      faceVertexConnects.push_back(positions.size());
      positions.push_back(pxr::GfVec3f(origin.x, 0.f, origin.y));
      colors.push_back(color);
      numFaceVertices++;
    }

    while (halfEdge != nullptr)
    {
      if (halfEdge->origin != nullptr && halfEdge->destination != nullptr)
      {
        auto destination = (halfEdge->destination->point - center) + center;
        faceVertexConnects.push_back(positions.size());
        positions.push_back(pxr::GfVec3f(destination.x, 0.f, destination.y));
        colors.push_back(color);
        numFaceVertices++;
      }
      halfEdge = halfEdge->next;
      if (halfEdge == start) {
        faceVertexCounts.push_back(numFaceVertices);
        break;
      }
    }
  }

  SetTopology(positions, faceVertexCounts, faceVertexConnects);
  */
}

void Mesh::Randomize(float value)
{
  for(auto& pos: _positions) {
    pos += pxr::GfVec3f(
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value),
      RANDOM_LO_HI(-value, value));
  }
}

JVR_NAMESPACE_CLOSE_SCOPE