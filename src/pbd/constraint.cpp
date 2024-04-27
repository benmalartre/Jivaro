#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

Constraint::Constraint(size_t elementSize, size_t offset, float stiffness, 
  float damping, const pxr::VtArray<int>& elems) 
  : _elements(elems)
  , _offset(offset)
  , _stiffness(stiffness)
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
  for(const auto& elem: _elements) {
    particles->GetPredicted(elem + _offset) += _correction[corrIdx++];
  }
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
size_t StretchConstraint::ELEM_SIZE = 2;

StretchConstraint::StretchConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, body->GetOffset(), stiffness, damping, elems)
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

    const size_t a = _elements[elem * ELEM_SIZE + 0] + _offset;
    const size_t b = _elements[elem * ELEM_SIZE + 1] + _offset;

    const pxr::GfVec3f& p0 = particles->GetPredicted(a);
    const pxr::GfVec3f& p1 = particles->GetPredicted(b);

    const float im0 = particles->GetInvMass(a);
    const float im1 = particles->GetInvMass(b);

    float K = im0 + im1;
    pxr::GfVec3f n = p0 - p1;
    const float d = n.GetLength();
    const float C = d - _rest[elem];

    if(d < 1e-6) continue;

    n /= d;
    float alpha = 0.0;
    if (!pxr::GfIsClose(_stiffness, 0.0, 1e-6f))
    {
      alpha = 1.f / (_stiffness * dt * dt);
      K += alpha;
    }

	  if (pxr::GfAbs(K) == 0.f) continue;

	  const pxr::GfVec3f correction = n * -(1.f / K) * C;

    _correction[elem * ELEM_SIZE + 0] += im0 * correction;
    _correction[elem * ELEM_SIZE + 1] -= im1 * correction;
  }
}


void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 0] + _offset));
    positions.push_back(
      particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 1] + _offset));
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
  : Constraint(ELEM_SIZE, body->GetOffset(), stiffness, damping, elems)
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


  const pxr::GfVec3f* x[4];
  float invMass[4];
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    const size_t a = _elements[elem * ELEM_SIZE + 0] + _offset;
    const size_t b = _elements[elem * ELEM_SIZE + 1] + _offset;
    const size_t c = _elements[elem * ELEM_SIZE + 2] + _offset;

    const pxr::GfVec3f center = 
      (particles->GetPredicted(a) + particles->GetPredicted(b) + particles->GetPredicted(c)) / 3.f;

    x[0] = &particles->GetPredicted(c);
    x[1] = &center;
    x[2] = &particles->GetPredicted(a);
    x[3] = &particles->GetPredicted(b);

    invMass[0] =  particles->GetInvMass(c);
    invMass[1] = (particles->GetInvMass(a) + particles->GetInvMass(b) + particles->GetInvMass(c)) / 3.f;
    invMass[2] =  particles->GetInvMass(a);
    invMass[3] =  particles->GetInvMass(b);

    float energy = 0.0;
    for (unsigned char k = 0; k < 4; k++)
      for (unsigned char j = 0; j < 4; j++)
        energy += pxr::GfDot(*x[k], *x[j]);
    energy *= 0.5;

    pxr::GfVec3f gradient[4];
    for (unsigned char k = 0; k < 4; k++)
      for (unsigned char j = 0; j < 4; j++)
        gradient[j] += *x[k];

    float sumNormGradient = 0.0;
    for (unsigned int j = 0; j < 4; j++)
    {
      if (invMass[j] != 0.0)
        sumNormGradient += invMass[j] * gradient[j].GetLengthSq();
    }
    
    float alpha = 0.0;
    if (pxr::GfAbs(_stiffness) > 1e-6)
    {
      alpha = 1.f / (_stiffness * dt * dt);
      sumNormGradient += alpha;
    }

    if (pxr::GfAbs(sumNormGradient) < 1e-6)continue;

    const float deltaLambda = -energy / sumNormGradient;

    _correction[elem * ELEM_SIZE + 0] += deltaLambda * invMass[2] * gradient[2];
    _correction[elem * ELEM_SIZE + 1] += deltaLambda * invMass[3] * gradient[3];
    _correction[elem * ELEM_SIZE + 2] += deltaLambda * invMass[0] * gradient[0];
    _correction[elem * ELEM_SIZE + 3] += deltaLambda * invMass[1] * gradient[1];
  }
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 0] + _offset));
    positions.push_back(
      particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 1] + _offset));
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
          allElements.push_back(best);
          allElements.push_back(p);
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
  : Constraint(ELEM_SIZE, body->GetOffset(), stiffness, damping, elems)
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
    const pxr::GfVec3f p0 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] + _offset]);
    const pxr::GfVec3f p1 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] + _offset]);
    const pxr::GfVec3f p2 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] + _offset]);
    const pxr::GfVec3f p3 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 3] + _offset]);

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

    const size_t a = _elements[elem * ELEM_SIZE + 0] + _offset;
    const size_t b = _elements[elem * ELEM_SIZE + 1] + _offset;
    const size_t c = _elements[elem * ELEM_SIZE + 2] + _offset;
    const size_t d = _elements[elem * ELEM_SIZE + 3] + _offset;

    const pxr::GfVec3f& p0 = particles->GetPredicted(a);
    const pxr::GfVec3f& p1 = particles->GetPredicted(b);
    const pxr::GfVec3f& p2 = particles->GetPredicted(c);
    const pxr::GfVec3f& p3 = particles->GetPredicted(d);

    const float invMass0 = particles->GetInvMass(a);
    const float invMass1 = particles->GetInvMass(b);
    const float invMass2 = particles->GetInvMass(c);
    const float invMass3 = particles->GetInvMass(d);

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
    positions.push_back(particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 2] + _offset));
    positions.push_back(particles->GetPosition(_elements[elemIdx * ELEM_SIZE + 3] + _offset));
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
size_t CollisionConstraint::ELEM_SIZE = 1;

CollisionConstraint::CollisionConstraint(Body* body, Collision* collision,
  const pxr::VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(ELEM_SIZE, 0, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::GEOM)
  , _Solve(&CollisionConstraint::_SolveGeom)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements);

}

CollisionConstraint::CollisionConstraint(Particles* particles, SelfCollision* collision, 
  const pxr::VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(ELEM_SIZE, 0, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::SELF)
  , _Solve(&CollisionConstraint::_SolveSelf)
{
 const size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements);
}


void CollisionConstraint::_SolveGeom(Particles* particles, float dt)
{
  _ResetCorrection();
  const size_t numElements = _elements.size() >> (ELEM_SIZE - 1);

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];
    const float im0 = particles->GetInvMass(index);
    const float d = _collision->GetContactDepth(index);
    if (d >= 0.f) continue;

    pxr::GfVec3f normal = _collision->GetContactNormal (index);

    const pxr::GfVec3f correction = normal * -d;

    _correction[elem * ELEM_SIZE + 0] += im0 * correction;
    
    // relative motion
    const pxr::GfVec3f relM = 
      (particles->GetVelocity(index) - _collision->GetVelocity(particles, index)) * dt;

    // tangential component of relative motion 
    pxr::GfVec3f relT = relM - normal * pxr::GfCross(relM, normal).GetLength();

    _correction[elem * ELEM_SIZE + 0] -= im0 * relT * _collision->GetFriction();
  }
}

void CollisionConstraint::_SolveSelf(Particles* particles, float dt)
{
  _ResetCorrection();

  SelfCollision* collision = (SelfCollision*)_collision;

  const size_t numElements = _elements.size() >> (ELEM_SIZE - 1);
  const pxr::GfVec3f* positions = particles->GetPositionCPtr();
  const float* radius = particles->GetRadiusCPtr();

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];
    const float im0 = particles->GetInvMass(index);

    size_t numContacts = collision->GetNumContacts(index);
    const Contact* contacts = collision->GetContacts(index);

    for(size_t contact = 0; contact < numContacts; ++contact) {

      const float d = contacts[contact].GetDepth();
      if (d >= 0.f) continue;

      const pxr::GfVec3f normal = contacts[contact].GetNormal();
      const pxr::GfVec3f correction = normal * -d;

      _correction[elem] += im0 * correction;
      /*
      // relative motion
      const pxr::GfVec3f relM = 
        (particles->GetVelocity(index) - contacts[contact].GetVelocity()) * dt;

      // tangential component of relative motion   
      pxr::GfVec3f relT = relM - normal * pxr::GfCross(relM, normal).GetLength();

      _correction[elem] -= im0 * relT * collision->GetFriction(); 
      */
    }

  }
}

void CollisionConstraint::Solve(Particles* particles, float dt)
{
  (this->*_Solve)(particles, dt);
}

void CollisionConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions, pxr::VtArray<float>& radius)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
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