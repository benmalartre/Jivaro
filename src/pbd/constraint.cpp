#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../pbd/constraint.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

size_t Constraint::GetNumParticles()
{
  return _body[0]->numPoints;
}

void Constraint::ResetCorrection()
{
  const size_t numPoints = _correction.size();
  memset(&_correction[0], 0.f, numPoints * sizeof(pxr::GfVec3f));
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;

StretchConstraint::StretchConstraint(Body* body, const float stretchStiffness, 
  const float compressionStiffness)
  : Constraint(body)
  , _stretch(stretchStiffness)
  , _compression(compressionStiffness) 
{
  _correction.resize(body->numPoints);

  if (body->geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    HalfEdgeGraph::ItUniqueEdge it(mesh->GetEdgesGraph());
    const auto& edges = mesh->GetEdges();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    HalfEdge* edge = it.Next();
    while (edge) {
      size_t a = edge->vertex;
      size_t b = edges[edge->next].vertex;
      _edges.push_back(pxr::GfVec2i(a, b));
      _rest.push_back((m.Transform(positions[b]) - m.Transform(positions[a])).GetLength());
      edge = it.Next();
    }
  }
}

StretchConstraint::StretchConstraint(Body* body, const pxr::VtArray<pxr::GfVec2i>& edges,
  const float stretchStiffness, const float compressionStiffness)
  : Constraint(body)
  , _stretch(stretchStiffness)
  , _compression(compressionStiffness) 
  , _edges(edges)
{
  _correction.resize(_edges.size() * 2);

  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();
  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();
  size_t numEdges = _edges.size();
  _rest.resize(numEdges);
  for(size_t edgeIdx = 0; edgeIdx < numEdges; ++edgeIdx) {
    const auto& edge = _edges[edgeIdx];
    _rest[edgeIdx] = 
      (m.Transform(positions[edge[1]]) - m.Transform(positions[edge[0]])).GetLength();
  }
}

bool StretchConstraint::Solve(Particles* particles)
{ 
  ResetCorrection();

  const size_t numEdges = _edges.size();
  for (size_t index = 0; index < numEdges; ++index) {
    const size_t i1 = _edges[index][0];
    const size_t i2 = _edges[index][1];
    const size_t p1 = i1 + _body[0]->offset;
    const size_t p2 = i2 + _body[0]->offset;
    const pxr::GfVec3f& x1 = particles->predicted[p1];
    const pxr::GfVec3f& x2 = particles->predicted[p2];

    const float im1 =
      pxr::GfIsClose(particles->mass[p1], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p1];

    const float im2 =
      pxr::GfIsClose(particles->mass[p2], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p2];

    float sum = im1 + im2;
    if (pxr::GfIsClose(sum, 0.f, 0.0000001f))
      continue;

    pxr::GfVec3f n = x2 - x1;
    const float d = n.GetLength();
    const float rest = _rest[index];
    if (pxr::GfIsClose(d, rest, 0.0000001f))
      continue;

    n.Normalize();

    pxr::GfVec3f c;
    if (d < rest)
      c = _compression * n * (d - rest) * sum;
    else
      c = _stretch * n * (d - rest) * sum;

    _correction[index * 2] += im1 * c;
    _correction[index * 2 + 1] += -im2 * c;
  }

  return true;
  
}

// this one has to happen serialy
void StretchConstraint::Apply(Particles* particles, const float di)
{
  //const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  size_t corrIdx = 0;
  for(const auto& edge: _edges) {
    particles->predicted[edge[0] + offset] += _correction[corrIdx++] * di;
    particles->predicted[edge[1] + offset] += _correction[corrIdx++] * di;
  }
  /*
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point] * di;
  }
  */
}

void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numEdges = _edges.size();
  results.resize(2 * numEdges);
  for (size_t edge = 0; edge < numEdges; ++edge) {
    results[edge * 2    ] = particles->position[_edges[edge][0] + _body[0]->offset];
    results[edge * 2 + 1] = particles->position[_edges[edge][1] + _body[0]->offset];
  }
}

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stretchStiffness, const float compressionStiffness)
{
  pxr::VtArray<pxr::GfVec2i> allEdges;

  if (body->geometry->GetType() == Geometry::MESH) {
    
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    HalfEdgeGraph::ItUniqueEdge it(mesh->GetEdgesGraph());
    const auto& edges = mesh->GetEdges();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    HalfEdge* edge = it.Next();
    while (edge) {
      size_t a = edge->vertex;
      size_t b = edges[edge->next].vertex;
      allEdges.push_back(pxr::GfVec2i(a, b));
      edge = it.Next();
    }
  }

  size_t numEdges = allEdges.size();
  std::cout << "num edges for stretch constraint : " << numEdges << std::endl;
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize, numEdges);
  while(true) {
    pxr::VtArray<pxr::GfVec2i> blockEdges(allEdges.begin()+first, allEdges.begin()+last);
    std::cout << "block edges size : " << blockEdges.size() << std::endl;
    StretchConstraint* stretch = new StretchConstraint(body, blockEdges, 
      stretchStiffness, compressionStiffness);
    constraints.push_back(stretch);
    first += Constraint::BlockSize;
    if(first >= numEdges)break;
    last = pxr::GfMin(last + Constraint::BlockSize, numEdges);
  }
}



size_t BendConstraint::TYPE_ID = Constraint::BEND;

BendConstraint::BendConstraint(Body* body, const float stiffness)
  : Constraint(body)
  , _stiffness(stiffness)
{
  _correction.resize(body->numPoints);

  if (body->geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    const size_t numPoints = mesh->GetNumPoints();
    const auto& neighbors = mesh->GetNeighbors();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    std::map<int, int> existing;
    for (size_t p = 0; p < numPoints; ++p) {
      for (const auto& n1 : neighbors[p]) {
        int best = -1;
        float minCosine = 0.f;
        for (const auto& n2 : neighbors[p]) {
          if (n1 == n2)continue;
          const pxr::GfVec3f e0 = positions[n1] - positions[p];
          const pxr::GfVec3f e1 = positions[n2] - positions[p];
          float cosine = pxr::GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
          if (cosine < minCosine) {
            minCosine = cosine;
            best = n2;
          }
        }
        if (best >= 0 && existing[best] != n1) {
          existing[n1] = best;
          _edges.push_back(pxr::GfVec3i(n1, best, p));
          pxr::GfVec3f center = (positions[n1] + positions[best] + positions[p]) / 3.f;
          _rest.push_back((positions[p] - center).GetLength());
        }
      }
    }
  }
}

BendConstraint::BendConstraint(Body* body, const pxr::VtArray<pxr::GfVec3i>& edges,
  const float stiffness)
  : Constraint(body)
  , _stiffness(stiffness)
  , _edges(edges)
{
  _correction.resize(body->numPoints);

  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();

  size_t numEdges = _edges.size();
  _rest.resize(numEdges);
  for(size_t edgeIdx = 0; edgeIdx < numEdges; ++edgeIdx) {
    const auto& edge = _edges[edgeIdx];
    pxr::GfVec3f center = (positions[edge[0]] + positions[edge[1]] + positions[edge[2]]) / 3.f;
    _rest[edgeIdx] = (positions[edge[2]] - center).GetLength();
  }
}

bool BendConstraint::Solve(Particles* particles)
{
  ResetCorrection();

  const size_t numEdges = _edges.size();
  for (size_t edge = 0; edge < numEdges; ++edge) {
    const size_t i1 = _edges[edge][0];
    const size_t i2 = _edges[edge][1];
    const size_t i3 = _edges[edge][2];

    const size_t p1 = i1 + _body[0]->offset;
    const size_t p2 = i2 + _body[0]->offset;
    const size_t p3 = i3 + _body[0]->offset;

    const pxr::GfVec3f& x1 = particles->predicted[p1];
    const pxr::GfVec3f& x2 = particles->predicted[p2];
    const pxr::GfVec3f& x3 = particles->predicted[p3];

    const pxr::GfVec3f center = (x1 + x2 + x3) / 3.f;
    pxr::GfVec3f dir = x3 - center;

    const float dist = dir.GetLength();
    if (pxr::GfIsClose(dist, 0.f, 0.0000001f))continue;

    dir *= (1.f - (_rest[edge] / dist));

    const float im1 =
      pxr::GfIsClose(particles->mass[p1], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p1];

    const float im2 =
      pxr::GfIsClose(particles->mass[p2], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p2];

    const float im3 =
      pxr::GfIsClose(particles->mass[p3], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p3];


    float w = im1 + 2.f * im2 + im3;
    if (pxr::GfIsClose(w, 0.f, 0.0000001f))
      continue;

    _correction[i1] += _stiffness * (2.f * im1 / w) * dir;
    _correction[i2] += _stiffness * (2.f * im2 / w) * dir;
    _correction[i3] += -_stiffness * (4.f * im3 / w) * dir;
  }

  return true;
}

// this one has to happen serialy
void BendConstraint::Apply(Particles* particles, const float di)
{
  const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point] * di;
  }
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numEdges = _edges.size();
  results.resize(2 * numEdges);
  for (size_t edge = 0; edge < numEdges; ++edge) {
    results[edge * 2] = particles->position[_edges[edge][0] + _body[0]->offset];
    results[edge * 2 + 1] = particles->position[_edges[edge][1] + _body[0]->offset];
  }
}

size_t DihedralConstraint::TYPE_ID = Constraint::DIHEDRAL;

DihedralConstraint::DihedralConstraint(Body* body, const float stiffness)
  : Constraint(body)
  , _stiffness(stiffness)
{
  _correction.resize(body->numPoints);

  if (body->geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    pxr::VtArray<TrianglePair> triPairs;
    mesh->GetAllTrianglePairs(triPairs);
    for(const auto& triPair: triPairs) {
      pxr::GfVec4i vertices = triPair.GetVertices();
      _vertices.push_back(vertices);

      const pxr::GfVec3f& p0 = mesh->GetPosition(vertices[0]);
      const pxr::GfVec3f& p1 = mesh->GetPosition(vertices[1]);
      const pxr::GfVec3f& p2 = mesh->GetPosition(vertices[2]);
      const pxr::GfVec3f& p3 = mesh->GetPosition(vertices[3]);

      pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0);
      n1 *= 1.f / n1.GetLengthSq();
      pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
      n2 *= 1.f / n2.GetLengthSq();

      n1.Normalize();
      n2.Normalize();
      float dot = pxr::GfDot(n1, n2);

      if (dot < -1.f) dot = -1.f;
      if (dot > 1.f) dot = 1.f;

      _rest.push_back(acos(dot));
    }
  }
}

DihedralConstraint::DihedralConstraint(Body* body, const pxr::VtArray<pxr::GfVec4i>& vertices,
  const float stiffness)
  : Constraint(body)
  , _stiffness(stiffness)
  , _vertices(vertices)
{
  _correction.resize(body->numPoints);

  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();

  size_t numVertices = _vertices.size();
  _rest.resize(numVertices);
  size_t index = 0;
  for(const auto& vertices: _vertices) {
    const pxr::GfVec3f& p0 = positions[vertices[0]];
    const pxr::GfVec3f& p1 = positions[vertices[1]];
    const pxr::GfVec3f& p2 = positions[vertices[2]];
    const pxr::GfVec3f& p3 = positions[vertices[3]];

    pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0);
    n1 *= 1.f / n1.GetLengthSq();
    pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
    n2 *= 1.f / n2.GetLengthSq();

    std::cout << "n1 : " << n1 << std::endl;
    std::cout << "n2 : " << n2 << std::endl;

    n1.Normalize();
    n2.Normalize();
    float dot = pxr::GfDot(n1, n2);

    if (dot < -1.f) dot = -1.f;
    if (dot > 1.f) dot = 1.f;

    _rest[index++] = acos(dot);
    std::cout << "rest angle : " << _rest.back() << std::endl;;
  }
}

bool DihedralConstraint::Solve(Particles* particles)
{
  ResetCorrection();

  // derivatives from Bridson, Simulation of Clothing with Folds and Wrinkles
  // his modes correspond to the derivatives of the bending angle arccos(n1 dot n2) with correct scaling
  size_t verticesIdx = 0;

  for(const auto& vertices: _vertices) {
    const size_t i0 = vertices[0];
    const size_t i1 = vertices[1];
    const size_t i2 = vertices[2];
    const size_t i3 = vertices[3];

    const pxr::GfVec3f& p0 = particles->predicted[i0 + _body[0]->offset];
    const pxr::GfVec3f& p1 = particles->predicted[i1 + _body[0]->offset];
    const pxr::GfVec3f& p2 = particles->predicted[i2 + _body[0]->offset];
    const pxr::GfVec3f& p3 = particles->predicted[i3 + _body[0]->offset];

    const float im0 =
      pxr::GfIsClose(particles->mass[i0 + _body[0]->offset], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[i0 + _body[0]->offset];

    const float im1 =
      pxr::GfIsClose(particles->mass[i1 + _body[0]->offset], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[i1 + _body[0]->offset];

    const float im2 =
      pxr::GfIsClose(particles->mass[i2 + _body[0]->offset], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[i2 + _body[0]->offset];

    const float im3 =
      pxr::GfIsClose(particles->mass[i3 + _body[0]->offset], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[i3 + _body[0]->offset];

    const float rest = _rest[verticesIdx++];

    pxr::GfVec3f e = p3 - p2;
    float edgeLen = e.GetLength();
    if (edgeLen < 1e-9) continue;

    float invEdgeLen = 1.f / edgeLen;

    pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0); 
    n1 /= n1.GetLengthSq();

    pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
    n2 /= n2.GetLengthSq();

    pxr::GfVec3f d0 = edgeLen * n1;
    pxr::GfVec3f d1 = edgeLen * n2;
    pxr::GfVec3f d2 = 
      pxr::GfDot(p0 - p3, e) * invEdgeLen * n1 + 
      pxr::GfDot(p1 - p3, e) * invEdgeLen * n2;
    pxr::GfVec3f d3 = 
      pxr::GfDot(p2 - p0, e) * invEdgeLen * n1 + 
      pxr::GfDot(p2 - p1, e) * invEdgeLen * n2;

    n1.Normalize();
    n2.Normalize();
    float dot = pxr::GfDot(n1, n2);

    if (dot < -1.0) dot = -1.0;
    if (dot > 1.0) dot = 1.0;
		float phi = acos(dot);

    // fast approximation
    //double phi = (-0.6981317 * dot * dot - 0.8726646) * dot + 1.570796;	

    float lambda = (
      d0.GetLengthSq() * im0 +
      d1.GetLengthSq() * im1 + 
      d2.GetLengthSq() * im2 + 
      d3.GetLengthSq() * im3);

    if (lambda == 0.0) continue;

    // stability
    // 1.5 is the largest magic number I found to be stable in all cases :-)
    //if (stiffness > 0.5 && Math.Abs(phi - RestAngle) > 1.5)		
    //	stiffness = 0.5;

    lambda = (phi - rest) / lambda * _stiffness;

    if (pxr::GfDot(pxr::GfCross(n1, n2), e) > 0.0)
      lambda = -lambda;

    _correction[i0] += -im0 * lambda * d0;
    _correction[i1] += -im1 * lambda * d1;
    _correction[i2] += -im2 * lambda * d2;
    _correction[i3] += -im3 * lambda * d3;
  }

  return true;
}

// this one has to happen serialy
void DihedralConstraint::Apply(Particles* particles, const float di)
{
  const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point] * di;
  }
}

void DihedralConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numElements = _vertices.size();
  results.resize(2 * numElements);
  for (size_t elem = 0; elem < numElements; ++elem) {
    results[elem * 2] = particles->position[_vertices[elem][2] + _body[0]->offset];
    results[elem * 2 + 1] = particles->position[_vertices[elem][3] + _body[0]->offset];
  }
}


JVR_NAMESPACE_CLOSE_SCOPE