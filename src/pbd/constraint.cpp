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

  const size_t numElements = _elements.size() >> (ELEM_SIZE - 1);
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    const size_t a = _elements[elem * ELEM_SIZE + 0];
    const size_t b = _elements[elem * ELEM_SIZE + 1];

    const pxr::GfVec3f p0 = particles->predicted[a];
    const pxr::GfVec3f p1 = particles->predicted[b];

    const float w0 = particles->invMass[a];
    const float w1 = particles->invMass[b];

    float w = w0 + w1;
    pxr::GfVec3f normal = p0 - p1;
    const float distance = normal.GetLength();
    const float C = distance - _rest[elem];

    if(distance < 1e-6f || w < 1e-6f) continue;

    w += _compliance / (dt * dt);

	  const pxr::GfVec3f correction = normal.GetNormalized() * -(1.f / w) * C;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 1] -= w1 * correction;
  }
}
void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 0]]);
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 1]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
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
    pxr::GfVec3f center = (
      positions[_elements[elemIdx * ELEM_SIZE + 0]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 1]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 2]]) / 3.f;
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]]) - 
      m.Transform(center)).GetLength();
  }
}

void BendConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() >> (ELEM_SIZE - 1);

  pxr::GfVec3f x[4],  gradient[4], center;
  float im[4], energy, accum, alpha, lambda;
  size_t a, b, c;
  unsigned char j, k;
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 1];
    c = _elements[elem * ELEM_SIZE + 2];

    center = (particles->predicted[a] + particles->predicted[b] + particles->predicted[c]) / 3.f;
    x[0] = particles->predicted[a];
    x[1] = particles->predicted[b];
    x[2] = particles->predicted[c];
    x[3] = center;

    im[0] =  particles->invMass[a];
    im[1] =  particles->invMass[b];
    im[2] =  particles->invMass[c];
    im[3] = (particles->invMass[a] + particles->invMass[b] + particles->invMass[c]) / 3.f;

    energy = 0.0;
    for (k = 0; k < 4; k++)
      for (j = 0; j < 4; j++)
        energy += pxr::GfDot(x[k], x[j]);
    energy *= 0.5;

    for (k = 0; k < 4; k++)
      for (j = 0; j < 4; j++)
        gradient[j] += x[k];

    alpha = 0.0;
    for (j = 0; j < 4; j++)
    {
      alpha += im[j] * gradient[j].GetLengthSq();
    }
    
    alpha += _compliance * dt * dt;
    if (pxr::GfAbs(alpha) < 1e-6)continue;

    lambda = -energy / alpha;

    _correction[elem * ELEM_SIZE + 0] += lambda * im[0] * gradient[0];
    _correction[elem * ELEM_SIZE + 1] += lambda * im[1] * gradient[1];
    _correction[elem * ELEM_SIZE + 2] += lambda * im[2] * gradient[2];
  }
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 0]]);
    positions.push_back(
      particles->position[_elements[elemIdx * ELEM_SIZE + 1]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
  }
}

void CreateBendConstraints(Body* body, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();

  if (geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geometry;
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
          allElements.push_back(p);
          allElements.push_back(best);
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
        allElements.push_back(curveStartIdx + segmentIdx + 1);
        allElements.push_back(curveStartIdx + segmentIdx + 2);
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

  const size_t numElements = _elements.size() >> (ELEM_SIZE - 1);
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    const size_t a = _elements[elem * ELEM_SIZE + 0];
    const size_t b = _elements[elem * ELEM_SIZE + 1];
    const size_t c = _elements[elem * ELEM_SIZE + 2];
    const size_t d = _elements[elem * ELEM_SIZE + 3];

    const pxr::GfVec3f& p0 = particles->predicted[a];
    const pxr::GfVec3f& p1 = particles->predicted[b];
    const pxr::GfVec3f& p2 = particles->predicted[c];
    const pxr::GfVec3f& p3 = particles->predicted[d];

    const float invMass0 = particles->invMass[a];
    const float invMass1 = particles->invMass[b];
    const float invMass2 = particles->invMass[c];
    const float invMass3 = particles->invMass[d];

    if (invMass0 == 0.0 && invMass1 == 0.0)continue;

    pxr::GfVec3f e = p3 - p2;
    float elen = e.GetLength();
    if (elen < 1e-6) continue;

    float invElen = 1.f / elen;

    pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0);
    n1 /= n1.GetLengthSq();

    pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
    n2 /= n2.GetLengthSq();

    pxr::GfVec3f d0 = elen * n1 * invMass0;
    pxr::GfVec3f d1 = elen * n2 * invMass1;
    pxr::GfVec3f d2 =
      (pxr::GfDot(p0 - p3, e) * invElen * n1 + 
        pxr::GfDot(p1 - p3, e) * invElen * n2) * invMass2;
    pxr::GfVec3f d3 = 
      (pxr::GfDot(p2 - p0, e) * invElen * n1 + 
        pxr::GfDot(p2 - p1, e) * invElen * n2) * invMass3;

    n1.Normalize();
    n2.Normalize();
    float dot = pxr::GfDot(n1, n2);

    if (dot < -1.0) dot = -1.0;
    if (dot >  1.0) dot =  1.0;
    float phi = acos(dot);	

    // Real phi = (-0.6981317 * dot * dot - 0.8726646) * dot + 1.570796;	// fast approximation

    float lambda = d0.GetLengthSq() + d1.GetLengthSq() + d2.GetLengthSq() + d3.GetLengthSq();

    if (lambda == 0.0) continue;

    // stability
    // 1.5 is the largest magic number I found to be stable in all cases :-)
    //if (stiffness > 0.5 && fabs(phi - b.restAngle) > 1.5)		
    //	stiffness = 0.5;

    lambda = (phi - _rest[elem]) / lambda * _compliance;
    if (pxr::GfDot(n1 ^ n2, e) > 0.0)
      lambda = -lambda;

	  const pxr::GfVec3f correction(0.f);

    _correction[elem * ELEM_SIZE + 0] += lambda * d0;
    _correction[elem * ELEM_SIZE + 1] += lambda * d1;
    _correction[elem * ELEM_SIZE + 2] += lambda * d2;
    _correction[elem * ELEM_SIZE + 3] += lambda * d3;
  }
}
void DihedralConstraint::GetPoints(Particles* particles,
  pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(particles->position[_elements[elemIdx * ELEM_SIZE + 2]]);
    positions.push_back(particles->position[_elements[elemIdx * ELEM_SIZE + 3]]);
    radius.push_back(0.02f);
    radius.push_back(0.02f);
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

void CollisionConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / GetElementSize();
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(_collision->GetContactPosition(_elements[elemIdx]));
    radius.push_back(0.05f);
  }
}

void CreateCollisionConstraint(Body* body, Collision* collision, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
}

JVR_NAMESPACE_CLOSE_SCOPE