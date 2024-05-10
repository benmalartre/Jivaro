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
  : Element(Element::CONSTRAINT)
  , _elements(elems)
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
  const pxr::GfVec2f* counter = &particles->counter[0];
  for(const auto& elem: _elements) 
    particles->predicted[elem] += _correction[corrIdx++] / counter[elem][0];
}

size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
const char* StretchConstraint::TYPE_NAME = "stretch";
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
  const size_t offset = body->GetOffset();
  const pxr::GfMatrix4d& m = geometry->GetMatrix();
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] - offset]) - 
       m.Transform(positions[_elements[elemIdx * ELEM_SIZE] - offset])).GetLength();
  }
}

void StretchConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;

  const float alpha =  _compliance / (dt * dt);
  size_t a, b;
  float w0, w1, W, C, length, lagrange;
  pxr::GfVec3f gradient, correction, damp;

  const pxr::GfVec3f* predicted = &particles->predicted[0];
  const pxr::GfVec3f* velocity = &particles->velocity[0];
  const float* invMass = &particles->invMass[0];
  
  for(size_t elem = 0; elem  < numElements; ++elem) {
    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 1];

    w0 = invMass[a];
    w1 = invMass[b];

    W = w0 + w1;
    if(W < 1e-6f) continue;

    gradient = predicted[a] - predicted[b];
    length = gradient.GetLength();

    C = length - _rest[elem];

    damp = pxr::GfDot(velocity[a] * dt * dt,  gradient) * gradient * _damping;

    correction = -C / (length * length * W + alpha) * gradient - damp;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 1] -= w1 * correction;
    
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
  size_t offset = body->GetOffset();

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
      allElements.push_back(a + offset);
      allElements.push_back(b + offset);
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
        allElements.push_back(curveStartIdx + segmentIdx + offset);
        allElements.push_back(curveStartIdx + segmentIdx + 1 + offset);
      }
      curveStartIdx += numCVs;
    }
  }

  size_t numElements = allElements.size();
  std::cout << "num stretch spring : " << (numElements/2) << std::endl;
  size_t numBlocks = 0;
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize * StretchConstraint::ELEM_SIZE, numElements);
  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    constraints.push_back(new StretchConstraint(body, blockElements, stiffness, damping));
    first += Constraint::BlockSize * StretchConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * StretchConstraint::ELEM_SIZE, numElements);
    numBlocks++;
  }
  std::cout << "spit in " << numBlocks << " blocks of " << Constraint::BlockSize << std::endl;
}

size_t BendConstraint::TYPE_ID = Constraint::BEND;
const char* BendConstraint::TYPE_NAME = "bend";
size_t BendConstraint::ELEM_SIZE = 3;

BendConstraint::BendConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const pxr::GfMatrix4d& m = geometry->GetMatrix();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    /*pxr::GfVec3f center = (
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] - offset]),
        m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] - offset]),
          m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] - offset])) / 3.f;
    _rest[elemIdx] = (center - m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]])).GetLength();*/
    
    _rest[elemIdx] = (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] - offset]) - 
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] - offset])).GetLength();
      
  }
}

void BendConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;
  const float alpha = _compliance / (dt * dt);

  pxr::GfVec3f x0, x1, x2, center, h, correction, damp;
  float w0, w1, w2, W, hL, C;
  size_t a, b, c;

  const pxr::GfVec3f* predicted = &particles->predicted[0];
  const pxr::GfVec3f* velocity = &particles->velocity[0];
  const float* invMass = &particles->invMass[0];
  
  for(size_t elem = 0; elem  < numElements; ++elem) {
    /*
    // wip not working
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

    center = (x0 + x1 + x2) / 3.f;
    h = x1 - center;
    hL = h.GetLength();


    if(hL > 1e-6f)C = 1.f - (_rest[elem]/hL);
    else C = 0.f;

	  correction = -C / (W + alpha) * h;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 1] -= 2.f * w1 * correction;
    _correction[elem * ELEM_SIZE + 2] += w2 * correction;
    */

   // easy solution stretch constraint on base edge :
    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 2];

    w0 = invMass[a];
    w1 = invMass[b];

    W = w0 + w1;
    if(W < 1e-6f) continue;

    h = predicted[a] - predicted[b];
    hL = h.GetLength();

    C = hL - _rest[elem];

    damp = pxr::GfDot(velocity[a] * dt * dt,  h) * h * _damping;
    correction = -C / (hL * hL * W + alpha) * h -damp;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 2] -= w1 * correction;
    
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


bool _IsAdjacent(size_t index, const int* adjacents, size_t num)
{
  for(size_t i = 0; i < num; ++i)
    if(adjacents[i] == index)return true;
  return false;
}

int _FindBestVertex(const pxr::GfVec3f* positions, size_t p, size_t n, 
  const int* neighbors, size_t numNeighbors, const int* adjacents, size_t numAdjacents)
{
  int best = -1;
  float minCosine = FLT_MAX;
  for (size_t o = 0; o < numNeighbors; ++o) {
    if (neighbors[n] == neighbors[o])continue;
    if(_IsAdjacent(neighbors[o], adjacents, numAdjacents))continue;

    const pxr::GfVec3f e0 = (positions[neighbors[n]] - positions[p]);
    const pxr::GfVec3f e1 = (positions[neighbors[o]] - positions[p]);
    float cosine = pxr::GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
    if (cosine < minCosine) {
      minCosine = cosine;
      best = o;
    }
  }
  return best;
}

pxr::GfVec2i _FindBestBoundaryVertices(const pxr::GfVec3f* positions, size_t p, 
  const int* neighbors, int num, const bool* boundaries)
{
  pxr::GfVec2i result(-1,-1);
  int best = -1;
  float minCosine = FLT_MAX;
  for (size_t m = 0; m < num; ++m) {
    if(!boundaries[neighbors[m]])continue;
    for (size_t n = 0; n < num; ++n) {
      if (neighbors[m] == neighbors[n] || !boundaries[neighbors[n]])continue;

      const pxr::GfVec3f e0 = (positions[neighbors[m]] - positions[p]);
      const pxr::GfVec3f e1 = (positions[neighbors[n]] - positions[p]);
      float cosine = pxr::GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
      if (cosine < minCosine) {
        minCosine = cosine;
        result = m > n ? pxr::GfVec2i(n, m) : pxr::GfVec2i(m, n);
      }
    }
  }
  return result;
}

void CreateBendConstraints(Body* body, std::vector<Constraint*>& constraints,
  float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  size_t cnt = 0;

  if (geometry->GetType() == Geometry::MESH) {
    Mesh* mesh = (Mesh*)geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

    const size_t numPoints = mesh->GetNumPoints();
    const pxr::GfMatrix4d& m = mesh->GetMatrix();
    const HalfEdgeGraph* graph = mesh->GetEdgesGraph();
    const pxr::VtArray<bool>& boundaries = graph->GetBoundaries();

    for (size_t p = 0; p < numPoints; ++p) {

      size_t numAdjacents = mesh->GetNumAdjacents(p);
      const int* adjacents = mesh->GetAdjacents(p);

      size_t numNeighbors = mesh->GetNumNeighbors(p);
      const int* neighbors = mesh->GetNeighbors(p);

      if(boundaries[p]) {
        if(numNeighbors <= 2)continue;

        pxr::GfVec2i pair = _FindBestBoundaryVertices(positions, p, neighbors, numNeighbors, &boundaries[0]);
        if(pair[0] == -1 || pair[1] == -1)continue;

        allElements.push_back(neighbors[pair[0]] + offset);
        allElements.push_back(p + offset);
        allElements.push_back(neighbors[pair[1]] + offset);
        cnt++;

      } else {
        for (size_t n = 0; n < numNeighbors; ++n) {
          if(_IsAdjacent(neighbors[n], adjacents, numAdjacents))continue;
          int o = _FindBestVertex(positions, p, n, neighbors, numNeighbors, adjacents, numAdjacents);

          if (o > -1 && neighbors[o] > neighbors[n]) {
            allElements.push_back(neighbors[n] + offset);
            allElements.push_back(p + offset);
            allElements.push_back(neighbors[o] + offset);
            cnt++;
          }
        }
      }
    }

    std::cout << "num bend spring : " << cnt << std::endl;

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
  size_t numBlocks = 0;
  size_t first = 0;
  size_t last = pxr::GfMin(Constraint::BlockSize * BendConstraint::ELEM_SIZE, numElements);
  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    BendConstraint* bend = new BendConstraint(body, blockElements, stiffness, damping);
    constraints.push_back(bend);
    first += Constraint::BlockSize * BendConstraint::ELEM_SIZE;
    if(first >= numElements)break;
    last = pxr::GfMin(last + Constraint::BlockSize * BendConstraint::ELEM_SIZE, numElements);
    numBlocks++;
  }

  std::cout << "split in " << numBlocks << " blocks of " << Constraint::BlockSize << std::endl;
}

size_t DihedralConstraint::TYPE_ID = Constraint::DIHEDRAL;
const char* DihedralConstraint::TYPE_NAME = "dihedral";
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
const char* CollisionConstraint::TYPE_NAME = "collision";

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

  size_t index, other, elem, c;
  float minDistance, distance, w0, w1, w, d;
  pxr::GfVec3f gradient, correction, damp;

  for (elem = 0; elem < numElements; ++elem) {
    index = _elements[elem];

    for(c = 0; c < collision->GetNumContacts(index); ++c) {
      other = collision->GetContactComponent(index, c);
      minDistance = particles->radius[index] + particles->radius[other];

      gradient = particles->predicted[index] - particles->predicted[other];
      distance = gradient.GetLength() + pxr::GfMax(-collision->GetContactInitDepth(index, c) - selfVMax * dt, 0.f);
      w0 = particles->invMass[index];
      w1 = particles->invMass[other];
      w = w0 + w1;

      if(distance > minDistance || w < 1e-6)continue;

      d = (minDistance - distance) / distance;

      damp = pxr::GfDot(particles->velocity[index] * dt * dt,  gradient) * gradient * _damping;
      correction =  w0 / w *  gradient * d - damp;

      _correction[elem] += correction;

       pxr::GfVec3f relativeVelocity = (particles->predicted[index] + correction) - particles->position[index] - _collision->GetContactVelocity(index, c) *dt;
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