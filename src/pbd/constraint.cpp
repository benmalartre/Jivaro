#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

Constraint::Constraint(size_t elementSize, Body* body, float stiffness, 
  float damping, const pxr::VtArray<int>& elems) 
  : _elements(elems)
  , _stiffness(stiffness)
  , _compliance(stiffness > 0.f ? 1.f / stiffness : 0.f)
  , _damping(damping)
{
  const size_t numElements = elems.size() / elementSize;
  _body.resize(1);
  _body[0] = body;
  _gradient.resize(elementSize + 1);
  _correction.resize(_elements.size());
}

Constraint::Constraint(Body* body1, Body* body2, float stiffness, float damping)
  : _stiffness(stiffness)
  , _damping(damping)
{
  _body.resize(2);
  _body[0] = body1;
  _body[1] = body2;
}

float Constraint::_ComputeLagrangeMultiplier(Particles* particles, size_t elemIdx)
{
  const size_t N = GetElementSize();
  float result = 0.f, m;
  for(size_t n = 0; n < N; ++n) {
    m = particles->mass[_elements[elemIdx * N + n]];
    result += 
      _gradient[n][0] * m * _gradient[n][0] +
      _gradient[n][1] * m * _gradient[n][1] +
      _gradient[n][2] * m * _gradient[n][2];
  }
  return result;
}

void Constraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t N = GetElementSize();
  const float rN = 1.f / static_cast<float>(N);
  const size_t numElements = _elements.size() / N;
  const size_t offset = _body[0]->offset;
  size_t compIdx, partIdx;
  for(size_t elemIdx = 0; elemIdx  < numElements; ++elemIdx) {
    const float C = _CalculateValue(particles, elemIdx);

    // Calculate the derivative of the constraint function
    _CalculateGradient(particles, elemIdx);

    // Skip if the gradient is sufficiently small
    constexpr float very_small_value = 1e-12;
    float gradientNorm = 0.f;
    for(size_t n = 0; n < N; ++n)
      gradientNorm += _gradient[n].GetLength();
    
    if(gradientNorm < very_small_value)
      continue;

    // Calculate time-scaled compliance
    const float alpha = _compliance / (dt * dt);

    // Calculate delta lagrange multiplier
    const float deltaLagrange =
      -C / (_ComputeLagrangeMultiplier(particles, elemIdx) + alpha);

    for(size_t n = 0; n < N; ++n) {
      compIdx = elemIdx * N + n;
      partIdx = _elements[compIdx] + offset;
     _correction[compIdx] +=
       (_gradient[n] * particles->mass[partIdx] * deltaLagrange) - 
       pxr::GfDot(particles->velocity[partIdx] * dt * dt,  _gradient[N]) * _gradient[N] * _damping;
    }
  }
}

void Constraint::_ResetCorrection()
{
  memset(&_correction[0], 0.f, _correction.size() * sizeof(pxr::GfVec3f));
}

// this one has to happen serialy
void Constraint::Apply(Particles* particles)
{
  const size_t offset = _body[0]->offset;
  size_t corrIdx = 0;
  for(const auto& elem: _elements) {
    particles->predicted[elem + offset] += _correction[corrIdx++];
  }
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
size_t StretchConstraint::ELEM_SIZE = 2;

StretchConstraint::StretchConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, body, stiffness, damping, elems)
{
  if(body->geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();
  const pxr::GfVec3f* positions = ((Deformable*)body->geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]) - 
       m.Transform(positions[_elements[elemIdx * ELEM_SIZE]])).GetLength();
  }
}


float StretchConstraint::_CalculateValue(Particles* particles, size_t index)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1] + offset];
  return -((p1 - p0).GetLength() - _rest[index]);
}

void StretchConstraint::_CalculateGradient(Particles* particles, size_t index)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1] + offset];

  const pxr::GfVec3f delta = p1 - p0;
  const float length = delta.GetLength();

  constexpr float epsilon = 1e-24;
  const pxr::GfVec3f direction = 
    (length < epsilon) ? 
    pxr::GfVec3f(
      RANDOM_LO_HI(-1.f, 1.f), 
      RANDOM_LO_HI(-1.f, 1.f), 
      RANDOM_LO_HI(-1.f, 1.f)
    ).GetNormalized() : delta / length;

  _gradient[0] = direction;
  _gradient[1] = -direction;
  _gradient[2] = direction;
}

void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 0] + _body[0]->offset]);
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 1] + _body[0]->offset]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
  }
}

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;

  if (body->geometry->GetType() == Geometry::MESH) {
    
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    HalfEdgeGraph::ItUniqueEdge it(*mesh->GetEdgesGraph());
    const auto& edges = mesh->GetEdges();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    HalfEdge* edge = it.Next();
    size_t a, b;
    while (edge) {
      a = edge->vertex;
      b = edges[edge->next].vertex;
      allElements.push_back(a);
      allElements.push_back(b);
      edge = it.Next();
    }
  }

  size_t numElements = allElements.size();
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize * StretchConstraint::ELEM_SIZE, numElements);
  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    StretchConstraint* stretch = new StretchConstraint(body, blockElements, stiffness, damping);
    constraints.push_back(stretch);
    first += Constraint::BlockSize * StretchConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * StretchConstraint::ELEM_SIZE, numElements);
  }
}

size_t BendConstraint::TYPE_ID = Constraint::BEND;
size_t BendConstraint::ELEM_SIZE = 3;

BendConstraint::BendConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, body, stiffness, damping, elems)
{
  if(body->geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfVec3f* positions = ((Deformable*)body->geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    pxr::GfVec3f center = (
      positions[_elements[elemIdx * ELEM_SIZE + 0]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 1]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 2]]) / 3.f;
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]]) - 
      m.Transform(center)).GetLength();
  }
}

float BendConstraint::_CalculateValue(Particles* particles, size_t index)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1] + offset];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[index * ELEM_SIZE + 2] + offset];

  const pxr::GfVec3f center = (p0 + p1 + p2) / 3.f;
  pxr::GfVec3f delta = p2 - center;

  const float length = delta.GetLength();
  if (pxr::GfIsClose(length, 0.f, 0.0000001f))return 1.f;

  return 1.f - (_rest[index] / length);
}

void BendConstraint::_CalculateGradient(Particles* particles, size_t index)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1] + offset];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[index * ELEM_SIZE + 2] + offset];

  const pxr::GfVec3f center = (p0 + p1 + p2) / 3.f;
  pxr::GfVec3f delta = p2 - center;
  const float length = delta.GetLength();

  constexpr float epsilon = 1e-24;
  const pxr::GfVec3f direction = 
    (length < epsilon) ? 
    pxr::GfVec3f(
      RANDOM_LO_HI(-1.f, 1.f), 
      RANDOM_LO_HI(-1.f, 1.f), 
      RANDOM_LO_HI(-1.f, 1.f)
    ).GetNormalized() : delta / length;

  _gradient[0] = -direction;
  _gradient[1] = -direction;
  _gradient[2] = 2 * direction;
  _gradient[3] = (p1 - p0).GetNormalized();
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 0] + _body[0]->offset]);
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 1] + _body[0]->offset]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
  }
}

void CreateBendConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;

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
          allElements.push_back(n1);
          allElements.push_back(best);
          allElements.push_back(p);
        }
      }
    }
  }

  size_t numElements = allElements.size();
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize * BendConstraint::ELEM_SIZE, numElements);
  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    BendConstraint* bend = new BendConstraint(body, blockElements, stiffness, damping);
    constraints.push_back(bend);
    first += Constraint::BlockSize * BendConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * BendConstraint::ELEM_SIZE, numElements);
  }
}

size_t DihedralConstraint::TYPE_ID = Constraint::DIHEDRAL;
size_t DihedralConstraint::ELEM_SIZE = 4;

static float _GetCotangentTheta(const pxr::GfVec3f& a, const pxr::GfVec3f& b)
{
  const float cosTheta = pxr::GfDot(a, b);
  const float sinTheta = pxr::GfCross(a, b).GetLength();
  return cosTheta / sinTheta;
};

DihedralConstraint::DihedralConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, body, stiffness, damping, elems)
{
  if(body->geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfVec3f* positions = ((Deformable*)body->geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();
  const size_t numElements = _elements.size() / ELEM_SIZE;
  const size_t offset = body->offset;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    const pxr::GfVec3f p0 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0]]);
    const pxr::GfVec3f p1 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]);
    const pxr::GfVec3f p2 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]]);
    const pxr::GfVec3f p3 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 3]]);

    pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0).GetNormalized();
    pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1).GetNormalized();

    float dot = pxr::GfDot(n1, n2);

    if (dot < -1.f) dot = -1.f;
    if (dot > 1.f) dot = 1.f;

    _rest[elemIdx] = std::acos(dot);
  }
}

float DihedralConstraint::_CalculateValue(Particles* particles, size_t index)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1] + offset];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[index * ELEM_SIZE + 2] + offset];
  const pxr::GfVec3f& p3 = particles->predicted[_elements[index * ELEM_SIZE + 3] + offset];

  const float& im0 = particles->mass[_elements[index * ELEM_SIZE + 0] + offset];
  const float& im1 = particles->mass[_elements[index * ELEM_SIZE + 1] + offset];
  const float& im2 = particles->mass[_elements[index * ELEM_SIZE + 2] + offset];
  const float& im3 = particles->mass[_elements[index * ELEM_SIZE + 3] + offset];

  pxr::GfVec3f e = p3 - p2;
  float edgeLen = e.GetLength();
  if (edgeLen < 1e-9) return 0.f;

  pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0).GetNormalized(); 
  pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1).GetNormalized();

  float dot = pxr::GfDot(n1, n2);

  if (dot < -1.0) dot = -1.0;
  if (dot > 1.0) dot = 1.0;

  if (pxr::GfDot(pxr::GfCross(n1, n2), e) > 0.0)
    return -std::acos(dot) - _rest[index];
  else
    return std::acos(dot) - _rest[index];
}

void DihedralConstraint::_CalculateGradient(Particles* particles, size_t elemIdx)
{
  const size_t offset = _body[0]->offset;

  const pxr::GfVec3f& p0 = particles->predicted[_elements[elemIdx * ELEM_SIZE + 0] + offset];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[elemIdx * ELEM_SIZE + 1] + offset];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[elemIdx * ELEM_SIZE + 2] + offset];
  const pxr::GfVec3f& p3 = particles->predicted[_elements[elemIdx * ELEM_SIZE + 3] + offset];

  const float& im0 = particles->mass[_elements[elemIdx * ELEM_SIZE + 0] + offset];
  const float& im1 = particles->mass[_elements[elemIdx * ELEM_SIZE + 1] + offset];
  const float& im2 = particles->mass[_elements[elemIdx * ELEM_SIZE + 2] + offset];
  const float& im3 = particles->mass[_elements[elemIdx * ELEM_SIZE + 3] + offset];

  pxr::GfVec3f e = p3 - p2;
  const double edgeLen = e.GetLength();
  if (edgeLen < 1e-9) {
    _gradient[0] = _gradient[1] = _gradient[3] = _gradient[4] = pxr::GfVec3f(0.f);
    return;
  }

  const double invEdgeLen = 1.f / edgeLen;

  pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0);
  n1 /= n1.GetLengthSq();

  pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
  n2 /= n2.GetLengthSq();

  pxr::GfVec3f d0 = edgeLen * n1;
  pxr::GfVec3f d1 = edgeLen * n2;
  pxr::GfVec3f d2 =
    invEdgeLen * pxr::GfDot(p0 - p3, e) * n1 +
    invEdgeLen * pxr::GfDot(p1 - p3, e) * n2;
  pxr::GfVec3f d3 =
    invEdgeLen * pxr::GfDot(p2 - p0, e) * n1 +
    invEdgeLen * pxr::GfDot(p2 - p1, e) * n2;

  _gradient[0] = im0 * d0;
  _gradient[1] = im1 * d1;
  _gradient[2] = im2 * d2;
  _gradient[3] = im3 * d3;
  _gradient[4] = e.GetNormalized();
}

void DihedralConstraint::GetPoints(Particles* particles,
  pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(particles->position[_elements[elemIdx * ELEM_SIZE + 2] + _body[0]->offset]);
    positions.push_back(particles->position[_elements[elemIdx * ELEM_SIZE + 3] + _body[0]->offset]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
  }
}

void CreateDihedralConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  if (body->geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)body->geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    pxr::VtArray<TrianglePair> triPairs;
    mesh->GetAllTrianglePairs(triPairs);
    for(const auto& triPair: triPairs) {
      pxr::GfVec4i vertices = triPair.GetVertices();
      for(size_t i = 0; i < 4; ++i)allElements.push_back(vertices[i]);
    }
  }

  size_t numElements = allElements.size();
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize * DihedralConstraint::ELEM_SIZE, numElements);
  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    DihedralConstraint* dihedral = new DihedralConstraint(body, blockElements, stiffness, damping);
    constraints.push_back(dihedral);
    first += Constraint::BlockSize * DihedralConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * DihedralConstraint::ELEM_SIZE, numElements);
  }
}

size_t CollisionConstraint::TYPE_ID = Constraint::COLLISION;
size_t CollisionConstraint::ELEM_SIZE = 1;

CollisionConstraint::CollisionConstraint(Body* body, Collision* collision,
  const pxr::VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(ELEM_SIZE, body, stiffness, damping, elems)
  , _collision(collision)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements);
  _gradient.resize(ELEM_SIZE + 1);
}

/*
pxr::GfVec3f PlaneCollision::ResolveContact(Particles* particles, size_t index)
{
  float d = pxr::GfDot(_normal, particles->predicted[index] - _position) - particles->radius[index];
  if (d < 0.f) return _normal * -d;
  else return pxr::GfVec3f(0.f);
}

pxr::GfVec3f PlaneCollision::ResolveVelocity(Particles* particles, float depth, size_t index)
{
  pxr::GfVec3f tangent(-particles->velocity[index][0], 0.f, -particles->velocity[index][2]);

  return _normal * _restitution * depth + tangent * _friction;
}
*/

float CollisionConstraint::_CalculateValue(Particles* particles, size_t elem)
{
  return (pxr::GfDot(particles->predicted[_elements[elem]] - _collision->GetContactPosition(_elements[elem]),
    _collision->GetContactNormal(elem)) + particles->radius[_elements[elem]]);
}

void CollisionConstraint::_CalculateGradient(Particles* particles, size_t elem)
{
  _gradient[0] = _collision->GetContactNormal(_elements[elem]);
}


void CollisionConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(_collision->GetContactPosition(_elements[elemIdx]));
    radius.push_back(0.05f);
  }
}

void CreateCollisionConstraint(Body* body, Collision* collision, pxr::VtArray<Constraint*>& constraints,
  float stiffness, float damping)
{
}



JVR_NAMESPACE_CLOSE_SCOPE