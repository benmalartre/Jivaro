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

StretchConstraint::StretchConstraint(Body* body, const float stretchStiffness, const float compressionStiffness)
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
      _rest.push_back(m.Transform(positions[b] - positions[a]).GetLength());
      edge = it.Next();
    }
  }
}

bool StretchConstraint::Solve(Particles* particles)
{ 
  ResetCorrection();

  const size_t numEdges = _edges.size();
  for (size_t edge = 0; edge < numEdges; ++edge) {
    const size_t i1 = _edges[edge][0];
    const size_t i2 = _edges[edge][1];
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
    const float rest = _rest[edge];
    if (pxr::GfIsClose(d, rest, 0.0000001f))continue;
    n.Normalize();

    pxr::GfVec3f c;
    if (d < rest)
      c = _compression * n * (d - rest) * sum;
    else
      c = _stretch * n * (d - rest) * sum;

    _correction[i1] += im1 * c;
    _correction[i2] += -im2 * c;
  }

  return true;
  
}

// this one has to happen serialy
void StretchConstraint::Apply(Particles* particles, const float di)
{
  const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point] * di;
  }
}

void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numEdges = _edges.size();
  results.resize(2 * numEdges);
  for (size_t edge = 0; edge < numEdges; ++edge) {
    results[edge * 2    ] = particles->position[_edges[edge][0]];
    results[edge * 2 + 1] = particles->position[_edges[edge][1]];
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
          const pxr::GfVec3f e0 = m.Transform(positions[n1] - positions[p]);
          const pxr::GfVec3f e1 = m.Transform(positions[n2] - positions[p]);
          float cosine = pxr::GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
          if (cosine < minCosine) {
            minCosine = cosine;
            best = n2;
          }
        }
        if (best >= 0 && existing[best] != n1) {
          existing[n1] = best;
          _edges.push_back(pxr::GfVec3i(n1, best, p));
          pxr::GfVec3f center = m.Transform(positions[n1] + positions[best] + positions[p]) / 3.f;
          _rest.push_back((m.Transform(positions[p]) - center).GetLength());
        }
      }
    }
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
    if (pxr::GfIsClose(dist, 0.f, 0.00000000f))continue;

    dir *= (1.f - (_rest[edge] / dist));

    const float m1 = particles->mass[p1];
    const float m2 = particles->mass[p2];
    const float m3 = particles->mass[p3];

    const float w = m1 + m2 + 2.f * m3;
    if (pxr::GfIsClose(w, 0.f, 0.0000001f))continue;

    _correction[i1] += _stiffness * (2.f * m1 / w) * dir;
    _correction[i2] += _stiffness * (2.f * m2 / w) * dir;
    _correction[i3] += -_stiffness * (4.f * m3 / w) * dir;
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
    results[edge * 2] = particles->position[_edges[edge][0]];
    results[edge * 2 + 1] = particles->position[_edges[edge][1]];
  }
}


JVR_NAMESPACE_CLOSE_SCOPE