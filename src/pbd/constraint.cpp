#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../pbd/constraint.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

void Constraint::ResetCorrection()
{
  const size_t numCorrections = _correction.size();
  memset(&_correction[0], 0.f, numCorrections * sizeof(pxr::GfVec3f));
}

// this one has to happen serialy
void Constraint::Apply(Particles* particles, const float di)
{
  const size_t offset = _body[0]->offset;
  size_t corrIdx = 0;
  for(const auto& elem: _elements) {
    particles->predicted[elem + offset] += _correction[corrIdx++] * di;
  }
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
size_t StretchConstraint::ELEM_SIZE = 2;

StretchConstraint::StretchConstraint(Body* body, const pxr::VtArray<int>& elems,
  const float stretchStiffness, const float compressionStiffness)
  : Constraint(body, elems)
  , _stretch(stretchStiffness)
  , _compression(compressionStiffness) 
{
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();
  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements * ELEM_SIZE);
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]) - 
       m.Transform(positions[_elements[elemIdx * ELEM_SIZE]])).GetLength();
  }
}

bool StretchConstraint::Solve(Particles* particles)
{ 
  ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    const size_t i0 = _elements[elemIdx * ELEM_SIZE + 0];
    const size_t i1 = _elements[elemIdx * ELEM_SIZE + 1];
    const size_t p0 = i0 + _body[0]->offset;
    const size_t p1 = i1 + _body[0]->offset;
    const pxr::GfVec3f& x0 = particles->predicted[p0];
    const pxr::GfVec3f& x1 = particles->predicted[p1];

    const float im0 =
      pxr::GfIsClose(particles->mass[p0], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p0];

    const float im1 =
      pxr::GfIsClose(particles->mass[p1], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p1];

    float sum = im0 + im1;
    if (pxr::GfIsClose(sum, 0.f, 0.0000001f))
      continue;

    pxr::GfVec3f n = x1 - x0;
    const float d = n.GetLength();
    const float rest = _rest[elemIdx];
    if (pxr::GfIsClose(d, rest, 0.0000001f))
      continue;

    n.Normalize();

    pxr::GfVec3f c;
    if (d < rest)
      c = _compression * n * (d - rest) * sum;
    else
      c = _stretch * n * (d - rest) * sum;

    _correction[elemIdx * ELEM_SIZE + 0] += im0 * c;
    _correction[elemIdx * ELEM_SIZE + 1] += -im1 * c;
  }

  return true;
  
}

void StretchConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  results.resize(numElements * 2);
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    results[elemIdx * ELEM_SIZE    ] = 
      particles->position[_elements[elemIdx * ELEM_SIZE + 0] + _body[0]->offset];
    results[elemIdx * ELEM_SIZE + 1] = 
      particles->position[_elements[elemIdx * ELEM_SIZE + 1] + _body[0]->offset];
  }
}

void CreateStretchConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stretchStiffness, const float compressionStiffness)
{
  pxr::VtArray<int> allElements;

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
    StretchConstraint* stretch = new StretchConstraint(body, blockElements, 
      stretchStiffness, compressionStiffness);
    constraints.push_back(stretch);
    first += Constraint::BlockSize * StretchConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * StretchConstraint::ELEM_SIZE, numElements);
  }
}

size_t BendConstraint::TYPE_ID = Constraint::BEND;
size_t BendConstraint::ELEM_SIZE = 3;

BendConstraint::BendConstraint(Body* body, const pxr::VtArray<int>& elems,
  const float stiffness)
  : Constraint(body, elems)
  , _stiffness(stiffness)
{
  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = body->geometry->GetMatrix();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements * ELEM_SIZE);
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    pxr::GfVec3f center = (
      positions[_elements[elemIdx * ELEM_SIZE + 0]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 1]] + 
      positions[_elements[elemIdx * ELEM_SIZE + 2]]) / 3.f;
    _rest[elemIdx] = (positions[_elements[elemIdx * ELEM_SIZE + 2]] - center).GetLength();
  }
}

bool BendConstraint::Solve(Particles* particles)
{
  ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    const size_t i0 = _elements[elemIdx * ELEM_SIZE + 0];
    const size_t i1 = _elements[elemIdx * ELEM_SIZE + 1];
    const size_t i2 = _elements[elemIdx * ELEM_SIZE + 2];

    const size_t p0 = i0 + _body[0]->offset;
    const size_t p1 = i1 + _body[0]->offset;
    const size_t p2 = i2 + _body[0]->offset;

    const pxr::GfVec3f& x0 = particles->predicted[p0];
    const pxr::GfVec3f& x1 = particles->predicted[p1];
    const pxr::GfVec3f& x2 = particles->predicted[p2];

    const pxr::GfVec3f center = (x0 + x1 + x2) / 3.f;
    pxr::GfVec3f dir = x2 - center;

    const float dist = dir.GetLength();
    if (pxr::GfIsClose(dist, 0.f, 0.0000001f))continue;

    dir *= (1.f - (_rest[elemIdx] / dist));

    const float im0 =
      pxr::GfIsClose(particles->mass[p0], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p0];

    const float im1 =
      pxr::GfIsClose(particles->mass[p1], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p1];

    const float im2 =
      pxr::GfIsClose(particles->mass[p2], 0, 0.0000001f) ?
      0.f :
      1.f / particles->mass[p2];


    float w = im0 + 2.f * im1 + im2;
    if (pxr::GfIsClose(w, 0.f, 0.0000001f))
      continue;

    _correction[elemIdx * ELEM_SIZE + 0] += _stiffness * (2.f * im0 / w) * dir;
    _correction[elemIdx * ELEM_SIZE + 1] += _stiffness * (2.f * im1 / w) * dir;
    _correction[elemIdx * ELEM_SIZE + 2] += -_stiffness * (4.f * im2 / w) * dir;
  }

  return true;
}

void BendConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  results.resize(numElements * 2);
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    results[elemIdx * ELEM_SIZE + 0] = 
      particles->position[_elements[elemIdx * ELEM_SIZE + 0] + _body[0]->offset];
    results[elemIdx * ELEM_SIZE + 1] = 
      particles->position[_elements[elemIdx * ELEM_SIZE + 1] + _body[0]->offset];
  }
}

void CreateBendConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness)
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
    BendConstraint* bend = new BendConstraint(body, blockElements, stiffness);
    constraints.push_back(bend);
    first += Constraint::BlockSize * BendConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * BendConstraint::ELEM_SIZE, numElements);
  }
}

size_t DihedralConstraint::TYPE_ID = Constraint::DIHEDRAL;
size_t DihedralConstraint::ELEM_SIZE = 4;

DihedralConstraint::DihedralConstraint(Body* body, const pxr::VtArray<int>& elems,
  const float stiffness)
  : Constraint(body, elems)
  , _stiffness(stiffness)
{
  const pxr::GfVec3f* positions = body->geometry->GetPositionsCPtr();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _correction.resize(numElements * ELEM_SIZE);
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {

    const pxr::GfVec3f& p0 = positions[_elements[elemIdx * 4    ]];
    const pxr::GfVec3f& p1 = positions[_elements[elemIdx * 4 + 1]];
    const pxr::GfVec3f& p2 = positions[_elements[elemIdx * 4 + 2]];
    const pxr::GfVec3f& p3 = positions[_elements[elemIdx * 4 + 3]];

    pxr::GfVec3f n1 = pxr::GfCross(p2 - p0, p3 - p0);
    n1 *= 1.f / n1.GetLengthSq();
    pxr::GfVec3f n2 = pxr::GfCross(p3 - p1, p2 - p1);
    n2 *= 1.f / n2.GetLengthSq();

    n1.Normalize();
    n2.Normalize();
    float dot = pxr::GfDot(n1, n2);

    if (dot < -1.f) dot = -1.f;
    if (dot > 1.f) dot = 1.f;

    _rest[elemIdx] = acos(dot);
  }
}

bool DihedralConstraint::Solve(Particles* particles)
{
  ResetCorrection();

  size_t numElements = _elements.size() / ELEM_SIZE;
  // derivatives from Bridson, Simulation of Clothing with Folds and Wrinkles
  // his modes correspond to the derivatives of the bending angle arccos(n1 dot n2) with correct scaling
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    
    const size_t i0 = _elements[elemIdx * ELEM_SIZE + 0];
    const size_t i1 = _elements[elemIdx * ELEM_SIZE + 1];
    const size_t i2 = _elements[elemIdx * ELEM_SIZE + 2];
    const size_t i3 = _elements[elemIdx * ELEM_SIZE + 3];

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

    const float rest = _rest[elemIdx];

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

    _correction[elemIdx * ELEM_SIZE    ] += -im0 * lambda * d0;
    _correction[elemIdx * ELEM_SIZE + 1] += -im1 * lambda * d1;
    _correction[elemIdx * ELEM_SIZE + 2] += -im2 * lambda * d2;
    _correction[elemIdx * ELEM_SIZE + 3] += -im3 * lambda * d3;
  }

  return true;
}

void DihedralConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results)
{
  const size_t numElements = _elements.size() / ELEM_SIZE;
  results.resize( numElements * 2);
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    results[elemIdx * 2] = particles->position[_elements[elemIdx * ELEM_SIZE + 2] + _body[0]->offset];
    results[elemIdx * 2 + 1] = particles->position[_elements[elemIdx * ELEM_SIZE + 3] + _body[0]->offset];
  }
}

void CreateDihedralConstraints(Body* body, pxr::VtArray<Constraint*>& constraints,
  const float stiffness)
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
    DihedralConstraint* dihedral = new DihedralConstraint(body, blockElements, stiffness);
    constraints.push_back(dihedral);
    first += Constraint::BlockSize * DihedralConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * DihedralConstraint::ELEM_SIZE, numElements);
  }
}



JVR_NAMESPACE_CLOSE_SCOPE