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

    HalfEdge* edge = it.Next();
    while (edge) {
      size_t a = edge->vertex;
      size_t b = edges[edge->next].vertex;
      _edges.push_back(a);
      _edges.push_back(b);
      _rest.push_back((positions[b] - positions[a]).GetLength());
      edge = it.Next();
    }
  }

  std::cout << "stretch constraint got " << (_edges.size() / 2) << " edges..." << std::endl;
  /*
  _stretchStiffness = stretchStiffness;
  _compressionStiffness = compressionStiffness;
  _p[0] = p1;
  _p[1] = p2;

  memset(&_c[0], 0.f, 2 * sizeof(pxr::GfVec3f));

  const pxr::GfVec3f* positions = &particles->position[0];
  _restLength = (positions[p2] - positions[p1]).GetLength();
  */
}

bool StretchConstraint::Solve(Particles* particles)
{ 

  const size_t numPoints = _body[0]->numPoints;
  memset(&_correction[0], 0.f, numPoints * sizeof(pxr::GfVec3f));

  const size_t numEdges = _edges.size() / 2;
  for (size_t edge = 0; edge < numEdges; ++edge) {
    const size_t p1 = _edges[edge * 2] + _body[0]->offset;
    const size_t p2 = _edges[edge * 2 + 1] + _body[0]->offset;
    const pxr::GfVec3f& x1 = particles->predicted[p1];
    const pxr::GfVec3f& x2 = particles->predicted[p2];

    const float im1 =
      pxr::GfIsClose(particles->mass[p1], 0, 0.0000001) ?
      0.f :
      1.f / particles->mass[p1];

    const float im2 =
      pxr::GfIsClose(particles->mass[p2], 0, 0.0000001) ?
      0.f :
      1.f / particles->mass[p2];

    float sum = im1 + im2;
    if (pxr::GfIsClose(sum, 0.f, 0.0000001f))
      return false;

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

    _correction[_edges[edge * 2]] = im1 * c;
    _correction[_edges[edge * 2 + 1]] = -im2 * c;
  }

  return true;
  
}

// this one has to happen serialy
void StretchConstraint::Apply(Particles* particles, const float dt)
{
  const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point] * dt;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE