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
}

bool StretchConstraint::Solve(Particles* particles, const float di)
{ 
  const size_t numPoints = _body[0]->numPoints;
  memset(&_correction[0], 0.f, numPoints * sizeof(pxr::GfVec3f));

  const size_t numEdges = _edges.size() / 2;
  for (size_t edge = 0; edge < numEdges; ++edge) {
    const size_t i1 = _edges[edge * 2];
    const size_t i2 = _edges[edge * 2 + 1];
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

    _correction[i1] += im1 * c * di;
    _correction[i2] += -im2 * c * di;
  }

  return true;
  
}

// this one has to happen serialy
void StretchConstraint::Apply(Particles* particles)
{
  const size_t numPoints = _body[0]->numPoints;
  const size_t offset = _body[0]->offset;
  for (size_t point = 0; point < numPoints; ++point) {
    particles->predicted[point + offset] += _correction[point];
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
    const auto& triPairs = mesh->GetTrianglePairs();

    for (const auto& triPair : triPairs) {
      //triPair.left->vertices
    }
  }
}


/*
// Compute matrix Q for quadratic bending
const Vector3r* x[4] = { &p2, &p3, &p0, &p1 };

const Vector3r e0 = *x[1] - *x[0];
const Vector3r e1 = *x[2] - *x[0];
const Vector3r e2 = *x[3] - *x[0];
const Vector3r e3 = *x[2] - *x[1];
const Vector3r e4 = *x[3] - *x[1];

const Real c01 = MathFunctions::cotTheta(e0, e1);
const Real c02 = MathFunctions::cotTheta(e0, e2);
const Real c03 = MathFunctions::cotTheta(-e0, e3);
const Real c04 = MathFunctions::cotTheta(-e0, e4);

const Real A0 = static_cast<Real>(0.5) * (e0.cross(e1)).norm();
const Real A1 = static_cast<Real>(0.5) * (e0.cross(e2)).norm();

const Real coef = -3.f / (2.f * (A0 + A1));
const Real K[4] = { c03 + c04, c01 + c02, -c01 - c03, -c02 - c04 };
const Real K2[4] = { coef * K[0], coef * K[1], coef * K[2], coef * K[3] };

for (unsigned char j = 0; j < 4; j++)
{
  for (unsigned char k = 0; k < j; k++)
  {
    Q(j, k) = Q(k, j) = K[j] * K2[k];
  }
  Q(j, j) = K[j] * K2[j];
}

return true;
*/


JVR_NAMESPACE_CLOSE_SCOPE