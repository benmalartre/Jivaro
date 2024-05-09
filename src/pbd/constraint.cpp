#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

Constraint::Constraint(size_t elementSize, float stiffness, 
  float damping, const pxr::VtArray<int>& elems) 
  : _elements(elems)
  , _compliance(stiffness > 0.f ? 1.f / stiffness : 0.f)
  , _damping(damping)
  , _color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1)
{
  const size_t numElements = elems.size() / elementSize;

  _correction.resize(_elements.size());
}

void Constraint::SetElements(const pxr::VtArray<int>& elems)
{
  const size_t numElements = _elements.size() / GetElementSize();
  _correction.resize(numElements);
}

void Constraint::_ResetCorrection()
{
  memset(&_correction[0], 0.f, _correction.size() * sizeof(pxr::GfVec3f));
}

// this one has to happen serialy
void Constraint::Apply(Particles* particles)
{
  size_t corrIdx = 0;
  for(const auto& elem: _elements) 
    particles->predicted[elem] += _correction[corrIdx++] / particles->counter[elem][0];
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
size_t StretchConstraint::ELEM_SIZE = 2;

StretchConstraint::StretchConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfMatrix4d& m = geometry->GetMatrix();
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]) - 
       m.Transform(positions[_elements[elemIdx * ELEM_SIZE]])).GetLength();
  }
}

void StretchConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;

  const float alpha =  _compliance / (dt * dt);
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    const size_t a = _elements[elem * ELEM_SIZE + 0];
    const size_t b = _elements[elem * ELEM_SIZE + 1];

    const float w0 = particles->invMass[a];
    const float w1 = particles->invMass[b];

    float w = w0 + w1;
    if(w < 1e-6f) continue;

    pxr::GfVec3f gradient = particles->predicted[a] - particles->predicted[b];
    const float length = gradient.GetLength();

    if(length < 1e-6f) continue;

    const float C = length - _rest[elem];

	  const pxr::GfVec3f correction = gradient.GetNormalized() / (w + alpha) * C;

    _correction[elem * ELEM_SIZE + 0] -= w0 * correction;
    _correction[elem * ELEM_SIZE + 1] += w1 * correction;
  }
}

void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->predicted[_elements[elemIdx * ELEM_SIZE + 0]]);
    positions.push_back(
      particles->predicted[_elements[elemIdx * ELEM_SIZE + 1]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
    colors.push_back(_color);
    colors.push_back(_color);
  }
}

void CreateStretchConstraints(Body* body, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  if (geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geometry;
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
  } else if (geometry->GetType() == Geometry::CURVE) {
    Curve* curve = (Curve*)geometry;
    size_t totalNumSegments = curve->GetTotalNumSegments();
    size_t curveStartIdx = 0;
    for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {
      size_t numCVs = curve->GetNumCVs(curveIdx);
      size_t numSegments = curve->GetNumSegments(curveIdx);
      for (size_t segmentIdx = 0; segmentIdx < numSegments; ++numSegments) {
        allElements.push_back(curveStartIdx + segmentIdx);
        allElements.push_back(curveStartIdx + segmentIdx + 1);
      }
      curveStartIdx += numCVs;
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
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();

  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = geometry->GetMatrix();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    /*
    pxr::GfVec3f center = (
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0]]),
        m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]),
          m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]])) / 3.f;
    

    _rest[elemIdx] = (center - m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]])).GetLength();*/
    _rest[elemIdx] = (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]]) - 
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0]])).GetLength();
  }
}

void BendConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;
  const float alpha = _compliance / (dt * dt);

  pxr::GfVec3f x0, x1, x2, center, h;
  float w0, w1, w2, W, hL, C;
  size_t a, b, c;

  float k = 1.01;
  
  for(size_t elem = 0; elem  < numElements; ++elem) {
    /*
    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 1];
    c = _elements[elem * ELEM_SIZE + 2];
   
    x0 = particles->predicted[a];
    x1 = particles->predicted[b];
    x2 = particles->predicted[c];

    w0 =  particles->invMass[a];
    w1 =  particles->invMass[b];
    w2 =  particles->invMass[c];

    W = w0 + 2.f * w1 + w2;
    if (W < 1e-6f) continue;

    center = (x0 + x1 + x2) / 3.f;
    h = x1 - center;
    hL = h.GetLength();
    if(hL<1e-6) continue;

    //h *= 1.f - (_rest[elem]/hL);
    pxr::GfVec3f d0 = -h;
    pxr::GfVec3f d1 = 2.f*h;
    pxr::GfVec3f d2 = -h;

    float lambda = (hL - _rest[elem]) / ( d0.GetLengthSq() * w0 + d1.GetLengthSq() * w1 + d2.GetLengthSq() * w2) + alpha;
    //h.Normalize();
    _correction[elem * ELEM_SIZE + 0] += lambda * d0;
    _correction[elem * ELEM_SIZE + 1] -= lambda * d1;
    _correction[elem * ELEM_SIZE + 2] += lambda * d2;
    */

    const size_t a = _elements[elem * ELEM_SIZE + 0];
    const size_t b = _elements[elem * ELEM_SIZE + 2];

    const float w0 = particles->invMass[a];
    const float w1 = particles->invMass[b];

    float w = w0 + w1;
    if(w < 1e-6f) continue;

    pxr::GfVec3f gradient = particles->predicted[a] - particles->predicted[b];
    const float length = gradient.GetLength();

    if(length < 1e-6f) continue;

    const float C = length - _rest[elem];

	  const pxr::GfVec3f correction = gradient.GetNormalized() / (w + alpha) * C;

    _correction[elem * ELEM_SIZE + 0] -= w0 * correction;
    _correction[elem * ELEM_SIZE + 2] += w1 * correction;
  }
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->predicted[_elements[elemIdx * ELEM_SIZE + 0]]);
    positions.push_back(
      particles->predicted[_elements[elemIdx * ELEM_SIZE + 2]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
    colors.push_back(_color);
    colors.push_back(_color);
  }
}

void CreateBendConstraints(Body* body, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if (geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

    const size_t numPoints = mesh->GetNumPoints();
    const auto& neighbors = mesh->GetNeighbors();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    const HalfEdgeGraph* graph = mesh->GetEdgesGraph();
    const pxr::VtArray<bool>& boundaries = graph->GetBoundaries();
    
    for (size_t p = 0; p < numPoints; ++p) {
      std::map<int, int> existing;
      bool isBoundary = boundaries[p];
      if(isBoundary && neighbors[p].size() <= 3) continue;
      for (const auto& n1 : neighbors[p]) {
        int best = -1;
        float minCosine = FLT_MAX;
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
        if (best > n1 && existing.find(best) == existing.end()) {
          existing[n1] = best;
          allElements.push_back(n1 + offset);
          allElements.push_back(p + offset);
          allElements.push_back(best + offset);
        }
      }
    }
  } else if (geometry->GetType() == Geometry::CURVE) {
    Curve* curve = (Curve*)geometry;
    size_t totalNumSegments = curve->GetTotalNumSegments();
    size_t curveStartIdx = 0;
    for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {
      size_t numCVs = curve->GetNumCVs(curveIdx);
      size_t numSegments = curve->GetNumSegments(curveIdx);
      for(size_t segmentIdx = 0; segmentIdx < numSegments - 1; ++numSegments) {
        allElements.push_back(curveStartIdx + segmentIdx);
        allElements.push_back(curveStartIdx + segmentIdx + 2);
        allElements.push_back(curveStartIdx + segmentIdx + 1);
      }
      curveStartIdx += numCVs;
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
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();

  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = geometry->GetMatrix();
  const size_t numElements = _elements.size() / ELEM_SIZE;
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

void DihedralConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  size_t numElements = _elements.size() / ELEM_SIZE;
  size_t a, b, c, d;
  pxr::GfVec3f x0, x1, x2, x3, e, n1, n2, d0, d1, d2, d3;
  float w0, w1, w2, w3, eL, invL, dot, phi, lambda;
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 1];
    c = _elements[elem * ELEM_SIZE + 2];
    d = _elements[elem * ELEM_SIZE + 3];

    x0 = particles->predicted[a];
    x1 = particles->predicted[b];
    x2 = particles->predicted[c];
    x3 = particles->predicted[d];

    w0 = particles->invMass[a];
    w1 = particles->invMass[b];
    w2 = particles->invMass[c];
    w3 = particles->invMass[d];

    if (w0 == 0.f && w1 == 0.f)continue;

    e = x3 - x2;
    eL = e.GetLength();
    if (eL < 1e-6) continue;

    invL = 1.f / eL;

    n1 = pxr::GfCross(x2 - x0, x3 - x0);
    n1 /= n1.GetLengthSq();

    n2 = pxr::GfCross(x3 - x1, x2 - x1);
    n2 /= n2.GetLengthSq();

    d0 = eL * n1;
    d1 = eL * n2;
    d2 = pxr::GfDot(x0 - x3, e) * invL * n1 + pxr::GfDot(x1 - x3, e) * invL * n2;
    d3 = pxr::GfDot(x2 - x0, e) * invL * n1 + pxr::GfDot(x2 - x1, e) * invL * n2;

    n1.Normalize();
    n2.Normalize();
    dot = pxr::GfDot(n1, n2);

    if (dot < -1.0) dot = -1.0;
    if (dot >  1.0) dot =  1.0;
    phi = acos(dot);	

    // Real phi = (-0.6981317 * dot * dot - 0.8726646) * dot + 1.570796;	// fast approximation

    lambda = w0 * d0.GetLengthSq() + w1 * d1.GetLengthSq() + w2 * d2.GetLengthSq() + w3 * d3.GetLengthSq();

    if (lambda == 0.0) continue;

    // stability
    // 1.5 is the largest magic number I found to be stable in all cases :-)
    //if (stiffness > 0.5 && fabs(phi - b.restAngle) > 1.5)		
    //	stiffness = 0.5;

    lambda = (phi - _rest[elem]) / lambda / _compliance;
    if (pxr::GfDot(n1 ^ n2, e) > 0.0)
      lambda = -lambda;

    _correction[elem * ELEM_SIZE + 0] += lambda * d0;
    _correction[elem * ELEM_SIZE + 1] += lambda * d1;
    _correction[elem * ELEM_SIZE + 2] += lambda * d2;
    _correction[elem * ELEM_SIZE + 3] += lambda * d3;
  }
}

void DihedralConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(particles->predicted[_elements[elemIdx * ELEM_SIZE + 2]]);
    positions.push_back(particles->predicted[_elements[elemIdx * ELEM_SIZE + 3]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
    colors.push_back(_color);
    colors.push_back(_color);
  }
}

void CreateDihedralConstraints(Body* body, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  if (geometry->GetType() == Geometry::MESH) {  
    Mesh* mesh = (Mesh*)geometry;
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

// Collision Constraint
//--------------------------------------------------------------------------------------------------
size_t CollisionConstraint::TYPE_ID = Constraint::COLLISION;

CollisionConstraint::CollisionConstraint(Body* body, Collision* collision,
  const pxr::VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(1, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::GEOM)
  , _Solve(&CollisionConstraint::_SolveGeom)
{
}

CollisionConstraint::CollisionConstraint(Particles* particles, SelfCollision* collision, 
  const pxr::VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(1, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::SELF)
  , _Solve(&CollisionConstraint::_SolveSelf)
{
}

// this one has to happen serialy
void CollisionConstraint::Apply(Particles* particles)
{
  size_t corrIdx = 0;
  for(const auto& elem: _elements)
    particles->predicted[elem] += _correction[corrIdx++] / particles->counter[elem][1] ;

}

pxr::GfVec3f CollisionConstraint::_ComputeFriction(const pxr::GfVec3f& correction, const pxr::GfVec3f& relativeVelocity)
{
  pxr::GfVec3f friction(0.f);
  float correctionLength = correction.GetLength();
  if (_collision->GetFriction() > 0 && correctionLength > 0.f)
  {
    pxr::GfVec3f correctionNorm = correction / correctionLength;

    pxr::GfVec3f tangentialVelocity = relativeVelocity - correctionNorm * pxr::GfDot(relativeVelocity, correctionNorm);
    float tangentialLength = tangentialVelocity.GetLength();
    float maxTangential = correctionLength * _collision->GetFriction();

    friction = -tangentialVelocity * pxr::GfMin(maxTangential / tangentialLength, 1.0f);
  }
  return friction;
}

static float vMax = 25.f;

void CollisionConstraint::_SolveGeom(Particles* particles, float dt)
{
  _ResetCorrection();
  const size_t numElements = _elements.size();

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];

    const pxr::GfVec3f normal = _collision->GetContactNormal(index);
    const float d = _collision->GetContactDepth(index) + pxr::GfMax(-_collision->GetContactInitDepth(index) - vMax * dt, 0.f) ;
    if(d > 0.f)continue;

    _correction[elem] += normal * -d;

    pxr::GfVec3f relativeVelocity = (particles->predicted[index] + _correction[elem]) - particles->position[index] - _collision->GetContactVelocity(index);
		pxr::GfVec3f friction = _ComputeFriction(_correction[elem], relativeVelocity);
    _correction[elem] +=  friction;
  
  }
}

static float selfVMax = 1.f;

void CollisionConstraint::_SolveSelf(Particles* particles, float dt)
{
  _ResetCorrection();

  SelfCollision* collision = (SelfCollision*)_collision;

  const size_t numElements = _elements.size();

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];

    for(size_t c = 0; c < collision->GetNumContacts(index); ++c) {
      const size_t other = collision->GetContactComponent(index, c);
      const float minDistance = particles->radius[index] + particles->radius[other];

      const pxr::GfVec3f gradient = particles->predicted[index] - particles->predicted[other];
      const float distance = gradient.GetLength() + pxr::GfMax(-collision->GetContactInitDepth(index, c) - selfVMax * dt, 0.f);
      const float w0 = particles->invMass[index];
      const float w1 = particles->invMass[other];
      float w = w0 + w1;

      if(distance > minDistance || w < 1e-6)continue;

      const float d = (minDistance - distance) / distance;

      const pxr::GfVec3f correction =  w0 / w *  gradient * d;

      _correction[elem] += correction;

       pxr::GfVec3f relativeVelocity = ((particles->predicted[index] + _correction[elem]) - particles->position[index]) - 
       ((particles->predicted[other] + (w1 / w * gradient * -d)) - particles->position[other]);
		  pxr::GfVec3f friction = _ComputeFriction(correction, relativeVelocity);
      _correction[elem] += w0 / w * friction;
    }
  }
}

void CollisionConstraint::Solve(Particles* particles, float dt)
{
  (this->*_Solve)(particles, dt);
}

void CollisionConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, 
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
  const size_t numElements = _elements.size() / GetElementSize();
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(_collision->GetContactPosition(_elements[elemIdx]));
    radius.push_back(0.05f);
    colors.push_back(_color);
  }
}

void CreateCollisionConstraint(Body* body, Collision* collision, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
}

JVR_NAMESPACE_CLOSE_SCOPE