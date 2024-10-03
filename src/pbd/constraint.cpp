#include <pxr/base/work/loops.h>
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../pbd/tokens.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"


JVR_NAMESPACE_OPEN_SCOPE

Constraint::Constraint(size_t elementSize, float stiffness, 
  float damp, const pxr::VtArray<int>& elems) 
  : Element(Element::CONSTRAINT)
  , _elements(elems)
  , _compliance(stiffness > 0.f ? 1.f / stiffness : 0.f)
  , _damp(damp)
  , _color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1)
{
  const size_t numElements = elems.size() / elementSize;

  _correction.resize(numElements);
  _gradient.resize(numElements+1);
}

void Constraint::SetElements(const pxr::VtArray<int>& elems)
{
  const size_t numElements = _elements.size() / GetElementSize();
  _correction.resize(numElements);
  _gradient.resize(numElements+1);
}

void Constraint::SetStiffness(float stiffness)
{
  _compliance = stiffness > 0.f ? 1.f / stiffness : 0.f;
}

void Constraint::SetDamp(float damp)
{
  _damp = damp;
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

ConstraintsGroup* CreateConstraintsGroup(Body* body, const pxr::TfToken& name, short type, 
  const pxr::VtArray<int>& allElements, size_t elementSize, size_t blockSize, Geometry* target)
{

  ConstraintsGroup* group = body->AddConstraintsGroup(name, type);

  size_t numElements = allElements.size();
  size_t numBlocks = 0;
  size_t first = 0;
  size_t last = pxr::GfMin(blockSize * elementSize, numElements);

  float stiffness = 10000.f;
  float damping = 0.1;

  //Geometry* geometry = body->GetGeometry();
  //geometry->GetAttributeValue(pxr::TfToken("name"), pxr::UsdTimeCode::Default(), &stiffness);

  while(true) {
    pxr::VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    switch (type) {
      case Constraint::ATTACH:
        group->constraints.push_back(new AttachConstraint(body, blockElements, stiffness, damping));
        break;

      case Constraint::PIN:
        group->constraints.push_back(new PinConstraint(body, blockElements, target, stiffness, damping));
        break;

      case Constraint::STRETCH: 
        group->constraints.push_back(new StretchConstraint(body, blockElements, stiffness, damping));
        break;

      case Constraint::SHEAR: 
        group->constraints.push_back(new ShearConstraint(body, blockElements, stiffness, damping));
        break;

      case Constraint::BEND:
        group->constraints.push_back(new BendConstraint(body, blockElements, stiffness, damping));
        break;

      case Constraint::DIHEDRAL:
        group->constraints.push_back(new DihedralConstraint(body, blockElements, stiffness, damping));
        break;
    }
    
    first += blockSize * elementSize;
    if(first >= numElements)break;
    last = pxr::GfMin(last + blockSize * elementSize, numElements);
    numBlocks++;
  }
  std::cout << name << ": " << "created " << numBlocks << " blocks of " << blockSize << "(max)" <<std::endl;

  return group;
}

/*
float Constraint::_ComputeLagrangeMultiplier(Particles* particles, size_t elem)
{
  const size_t N = GetElementSize();
  float result = 0.f, m;
  for(size_t n = 0; n < N; ++n) {
    m = particles->mass[_elements[elem * N + n]];
    result += 
      _gradient[n][0] * m * _gradient[n][0] +
      _gradient[n][1] * m * _gradient[n][1] +
      _gradient[n][2] * m * _gradient[n][2];
  }
  return result;
}

void Constraint::_Solve(size_t N, const float C, const pxr::GfVec3f* gradients, const float dt)
{
  _ResetCorrection();

  const size_t N = GetElementSize();
  const float rN = 1.f / static_cast<float>(N);
  const size_t numElements = _elements.size() / N;
  const size_t offset = _body[0]->offset;
  size_t comp, part;

  // Calculate time-scaled compliance
  const float alpha = _compliance / (dt * dt);

  for(size_t elem = 0; elem  < numElements; ++elem) {
    const float C = _CalculateValue(particles, elem);

    // Calculate the derivative of the constraint function
    _CalculateGradient(particles, elem);

    // Skip if the gradient is sufficiently small
    constexpr float eps = 1e-6f;
    float gradientNormSq = 0.f;
    for(size_t n = 0; n < N; ++n)
      gradientNormSq += _gradient[n].GetLengthSq();
    
    if(gradientNormSq < eps)
      continue;

    // Calculate delta lagrange multiplier
    const float deltaLagrange =
      -C / (_ComputeLagrangeMultiplier(particles, elem) + alpha);

    for(size_t n = 0; n < N; ++n) {
      comp = elem * N + n;
      part = _elements[comp] + offset;
     _correction[comp] +=
       (_gradient[n] * particles->mass[part] * deltaLagrange) - 
       pxr::GfDot(particles->velocity[part] * dt * dt,  _gradient[N]) * _gradient[N] * _damping;
    }
  }
}
*/


//-----------------------------------------------------------------------------------------
//   ATTACH CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t AttachConstraint::TYPE_ID = Constraint::ATTACH;
size_t AttachConstraint::ELEM_SIZE = 1;

AttachConstraint::AttachConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
  , _body(body)
{
  Geometry* geometry = body->GetGeometry();
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const size_t offset = body->GetOffset();
  const pxr::GfMatrix4d& m = geometry->GetMatrix();
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
}

void AttachConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const float alpha =  _compliance / (dt * dt);

  const pxr::GfVec3f* predicted = &particles->predicted[0];
  const pxr::GfVec3f* velocity = &particles->velocity[0];

  const size_t offset = _body->GetOffset();
  Geometry* geometry = _body->GetGeometry();

  const pxr::GfMatrix4d& m = geometry->GetMatrix();
  const pxr::GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();

  pxr::GfVec3f gradient, damp;
  size_t index = 0;
  float length;
  
  for(auto& elem: _elements) {
    gradient = predicted[elem] - m.Transform(positions[elem - offset]);
    length = gradient.GetLength();

    damp = pxr::GfDot(velocity[elem],  gradient) * gradient * _damp * dt * dt;

    _correction[index++] -= length / (length * length + alpha) * gradient - damp;
  }
}

void AttachConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
}

//-----------------------------------------------------------------------------------------
//   PIN CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t PinConstraint::TYPE_ID = Constraint::PIN;
size_t PinConstraint::ELEM_SIZE = 1;

 PinConstraint::PinConstraint(Body* body, const pxr::VtArray<int>& elems, Geometry* target,
  float stiffness, float damping)
    : Constraint(ELEM_SIZE, stiffness, damping, elems)
    , _target(target)
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
}

void PinConstraint::Solve(Particles* particles, float dt)
{
}

void PinConstraint::GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors)
{
}

//-----------------------------------------------------------------------------------------
//   STRETCH CONSTRAINT
//-----------------------------------------------------------------------------------------
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

float StretchConstraint::_CalculateValue(Particles* particles, size_t elem)
{
  const pxr::GfVec3f& p0 = particles->predicted[_elements[elem * ELEM_SIZE + 0]];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[elem * ELEM_SIZE + 1]];
  return -((p1 - p0).GetLength() - _rest[elem]);
}

void StretchConstraint::_CalculateGradient(Particles* particles, size_t elem)
{
  const pxr::GfVec3f& p0 = particles->predicted[_elements[elem * ELEM_SIZE + 0]];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[elem * ELEM_SIZE + 1]];

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
  _gradient[2] = pxr::GfVec3f(0.f);
}

void StretchConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;

  const float alpha =  _compliance / (dt * dt);
  size_t a, b;
  float w0, w1, W, C, length;
  pxr::GfVec3f gradient, correction, damp0, damp1;

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
    if(length<1e-6f)continue;

    gradient /= length;

    C = length - _rest[elem];

    damp0 = pxr::GfDot(velocity[a],  gradient) * gradient * _damp * dt * dt;
    damp1 = pxr::GfDot(velocity[b],  gradient) * gradient * _damp * dt * dt;

    correction = -C / (length * length * W + alpha) * gradient;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction - damp0;
    _correction[elem * ELEM_SIZE + 1] -= w1 * correction - damp1;
    
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

static void _GetMeshStretchElements(Mesh* mesh, pxr::VtArray<int>& allElements, size_t offset)
{
  if(!(mesh->GetFlags() & Mesh::HALFEDGES))mesh->ComputeHalfEdges();  
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
}

static void _GetCurveStretchElements(Curve* curve, pxr::VtArray<int>& allElements, size_t offset)
{
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

ConstraintsGroup* CreateStretchConstraints(Body* body, float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if (geometry->GetType() == Geometry::MESH) 
    _GetMeshStretchElements((Mesh*)geometry, allElements, offset);
  else if (geometry->GetType() == Geometry::CURVE)
    _GetCurveStretchElements((Curve*)geometry, allElements, offset);
  
  if(allElements.size())
    return CreateConstraintsGroup(body, 
      PBDTokens->stretch, Constraint::STRETCH,
        allElements, StretchConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;

}


//-----------------------------------------------------------------------------------------
//   BEND CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t BendConstraint::TYPE_ID = Constraint::BEND;
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

float BendConstraint::_CalculateValue(Particles* particles, size_t index)
{
  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0]];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1]];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[index * ELEM_SIZE + 2]];

  const pxr::GfVec3f center = (p0 + p1 + p2) / 3.f;
  pxr::GfVec3f delta = p2 - center;

  const float length = delta.GetLength();
  if (pxr::GfIsClose(length, 0.f, 0.0000001f))return 1.f;

  return 1.f - (_rest[index] / length);
}

void BendConstraint::_CalculateGradient(Particles* particles, size_t index)
{
  const pxr::GfVec3f& p0 = particles->predicted[_elements[index * ELEM_SIZE + 0]];
  const pxr::GfVec3f& p1 = particles->predicted[_elements[index * ELEM_SIZE + 1]];
  const pxr::GfVec3f& p2 = particles->predicted[_elements[index * ELEM_SIZE + 2]];

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

    damp = pxr::GfDot(velocity[a],  h) * h * _damp * dt * dt;
    correction = -C / (hL * hL * W + alpha) * h - damp;

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

void _GetMeshBendElements(Mesh* mesh, pxr::VtArray<int>& allElements, size_t offset) 
{
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

  if(!(mesh->GetFlags() & Mesh::NEIGHBORS))mesh->ComputeNeighbors();  
  if(!(mesh->GetFlags() & Mesh::ADJACENTS))mesh->ComputeAdjacents();  

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
    } else {
      for (size_t n = 0; n < numNeighbors; ++n) {
        if(_IsAdjacent(neighbors[n], adjacents, numAdjacents))continue;
        int o = _FindBestVertex(positions, p, n, neighbors, numNeighbors, adjacents, numAdjacents);

        if (o > -1 && neighbors[o] > neighbors[n]) {
          allElements.push_back(neighbors[n] + offset);
          allElements.push_back(p + offset);
          allElements.push_back(neighbors[o] + offset);
        }
      }
    }
  }
}

ConstraintsGroup* CreateBendConstraints(Body* body, float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  size_t cnt = 0;

  if (geometry->GetType() == Geometry::MESH) {
    _GetMeshBendElements((Mesh*)geometry, allElements, offset);
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

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      PBDTokens->bend, Constraint::BEND,
        allElements, BendConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
}

//-----------------------------------------------------------------------------------------
//   SHEAR CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t ShearConstraint::TYPE_ID = Constraint::SHEAR;

ShearConstraint::ShearConstraint(Body* body, const pxr::VtArray<int>& elems,
  float stiffness, float damping)
  : StretchConstraint(body, elems, stiffness, damping)
{
}

static void _GetMeshShearElements(Mesh* mesh, pxr::VtArray<int>& allElements, size_t offset)
{
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();

  const size_t numFaces = mesh->GetNumFaces();
  const pxr::GfMatrix4d& m = mesh->GetMatrix();
  const pxr::VtArray<int>& counts = mesh->GetFaceCounts();
  const pxr::VtArray<int>& connects = mesh->GetFaceConnects();
  size_t numVertices;
  size_t shift = 0;
  for (size_t f = 0; f < numFaces; ++f) {
    numVertices = counts[f];
    for (size_t v = 0; v < numVertices; ++v) {
      size_t o = (v + 2) % numVertices;
      if (v > o)continue;
      else {
        allElements.push_back(offset + connects[shift + v]);
        allElements.push_back(offset + connects[shift + o]);
      }
    }
    shift += numVertices;
  }
}

static void _GetCurveShearElements(Curve* curve, pxr::VtArray<int>& allElements, size_t offset)
{
  size_t totalNumSegments = curve->GetTotalNumSegments();
  size_t curveStartIdx = 0;
  for (size_t curveIdx = 0; curveIdx < curve->GetNumCurves(); ++curveIdx) {
    size_t numCVs = curve->GetNumCVs(curveIdx);
    size_t numSegments = curve->GetNumSegments(curveIdx);
    for(size_t segmentIdx = 0; segmentIdx < numSegments - 1; ++numSegments) {
      allElements.push_back(curveStartIdx + segmentIdx);
      allElements.push_back(curveStartIdx + segmentIdx + 2);
    }
    curveStartIdx += numCVs;
  }
}

ConstraintsGroup* CreateShearConstraints(Body* body, float stiffness, float damping)
{
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if (geometry->GetType() == Geometry::MESH) 
    _GetMeshShearElements((Mesh*)geometry, allElements, offset);
  else if (geometry->GetType() == Geometry::CURVE) 
    _GetCurveShearElements((Curve*)geometry, allElements, offset);

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      PBDTokens->shear, Constraint::SHEAR,
        allElements, StretchConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
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
    const pxr::GfVec3f x0 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0]]);
    const pxr::GfVec3f x1 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]]);
    const pxr::GfVec3f x2 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2]]);
    const pxr::GfVec3f x3 = m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 3]]);

    pxr::GfVec3f n1 = pxr::GfCross(x1 - x0, x2 - x0).GetNormalized();
    pxr::GfVec3f n2 = pxr::GfCross(x1 - x0, x3 - x0).GetNormalized();

    float dot = pxr::GfClamp(pxr::GfDot(n1, n2), 0.f, 1.f);

    _rest[elemIdx] = std::acos(dot);
  }
}

float DihedralConstraint::_CalculateValue(Particles* particles, size_t index)
{
  const size_t a = _elements[index * ELEM_SIZE + 0];
  const size_t b = _elements[index * ELEM_SIZE + 1];
  const size_t c = _elements[index * ELEM_SIZE + 2];
  const size_t d = _elements[index * ELEM_SIZE + 3];

  const pxr::GfVec3f& p0 = particles->predicted[a];
  const pxr::GfVec3f& p1 = particles->predicted[b];
  const pxr::GfVec3f& p2 = particles->predicted[c];
  const pxr::GfVec3f& p3 = particles->predicted[d];

  const float& im0 = particles->mass[a];
  const float& im1 = particles->mass[b];
  const float& im2 = particles->mass[c];
  const float& im3 = particles->mass[d];

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
  const size_t a = _elements[elemIdx * ELEM_SIZE + 0];
  const size_t b = _elements[elemIdx * ELEM_SIZE + 1];
  const size_t c = _elements[elemIdx * ELEM_SIZE + 2];
  const size_t d = _elements[elemIdx * ELEM_SIZE + 3];

  const pxr::GfVec3f& p0 = particles->predicted[a];
  const pxr::GfVec3f& p1 = particles->predicted[b];
  const pxr::GfVec3f& p2 = particles->predicted[c];
  const pxr::GfVec3f& p3 = particles->predicted[d];

  const float& im0 = particles->mass[a];
  const float& im1 = particles->mass[b];
  const float& im2 = particles->mass[c];
  const float& im3 = particles->mass[d];

  pxr::GfVec3f e = p3 - p2;
  const float edgeLen = e.GetLength();
  if (edgeLen < 1e-9) {
    _gradient[0] = _gradient[1] = _gradient[3] = _gradient[4] = pxr::GfVec3f(0.f);
    return;
  }

  const float invEdgeLen = 1.f / edgeLen;

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


void DihedralConstraint::Solve(Particles* particles, float dt)
{
  _ResetCorrection();

  size_t numElements = _elements.size() / ELEM_SIZE;
  size_t a, b, c, d;
  pxr::GfVec3f x0, x1, x2, x3, n1, n2, q0, q1, q2, q3;
  float w0, w1, w2, w3, dot, angle, lambda;

  float alpha = _compliance / dt / dt;
  
  for(size_t elem = 0; elem  < numElements; ++elem) {

    a = _elements[elem * ELEM_SIZE + 0];
    b = _elements[elem * ELEM_SIZE + 1];
    c = _elements[elem * ELEM_SIZE + 2];
    d = _elements[elem * ELEM_SIZE + 3];

    x0 = particles->predicted[a];
    x1 = particles->predicted[b] - x0;
    x2 = particles->predicted[c] - x0;
    x3 = particles->predicted[d] - x0;

    w0 = particles->invMass[a];
    w1 = particles->invMass[b];
    w2 = particles->invMass[c];
    w3 = particles->invMass[d];

    n1 = pxr::GfCross(x1, x2);
    n2 = pxr::GfCross(x1, x3);

    n1.Normalize();
    n2.Normalize();
    dot = pxr::GfClamp(pxr::GfDot(n1, n2), 0.f, 1.f);

    angle = std::acos(dot);	
    if (angle < 1e-6 || std::isnan(dot)) continue;

    q2 =  (x1 ^ n2 + n1 ^ x1 * dot) / ((x1 ^ x2).GetLength() + 1e-6f);
    q3 =  (x1 ^ n1 + n2 ^ x1 * dot) / ((x1 ^ x3).GetLength() + 1e-6f);
    q1 = -(x2 ^ n2 + n1 ^ x2 * dot) / ((x1 ^ x2).GetLength() + 1e-6f) - 
                       (x3 ^ n1 + n2 ^ x3 * dot) / ((x1 ^ x3).GetLength() + 1e-6f);
    q0 = -q1 - q2 - q3;


    float denom = alpha + (w0 * pxr::GfDot(q0, q0) + w1 * pxr::GfDot(q1, q1) + w2 * pxr::GfDot(q2, q2) + w3 * pxr::GfDot(q3, q3));
	  if (denom < 1e-6f) continue;
		lambda = -pxr::GfSqrt(1.0f - dot * dot) * (angle - _rest[elem]) / denom;

    _correction[elem * ELEM_SIZE + 0] += w0 * lambda * q0;
    _correction[elem * ELEM_SIZE + 1] += w1 * lambda * q1;
    _correction[elem * ELEM_SIZE + 2] += w2 * lambda * q2;
    _correction[elem * ELEM_SIZE + 3] += w3 * lambda * q3;
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

ConstraintsGroup* CreateDihedralConstraints(Body* body, float stiffness, float damping)
{
  size_t offset = body->GetOffset();
  pxr::VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  if (geometry->GetType() == Geometry::MESH) {  
    Mesh* mesh = (Mesh*)geometry;
    const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
    pxr::VtArray<TrianglePair> triPairs;
    mesh->GetAllTrianglePairs(triPairs, false);
    for(const auto& triPair: triPairs) {
      pxr::GfVec4i vertices = triPair.GetVertices();
      for(size_t i = 0; i < 4; ++i)allElements.push_back(vertices[i] + offset);
    }
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      PBDTokens->dihedral, Constraint::DIHEDRAL,
        allElements, DihedralConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
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

float CollisionConstraint::_CalculateValue(Particles* particles, size_t elem)
{
  return _collision->GetValue(particles, _elements[elem]);
}

void CollisionConstraint::_CalculateGradient(Particles* particles, size_t elem)
{
  _gradient[0] = _collision->GetContactNormal(_elements[elem]);
}

// this one has to happen serialy
void CollisionConstraint::Apply(Particles* particles)
{
  size_t corrIdx = 0;
  const pxr::GfVec2f* counter = &particles->counter[0];
  for(const auto& elem: _elements)
    particles->predicted[elem] += _correction[corrIdx++] / counter[elem][1];
}

pxr::GfVec3f CollisionConstraint::_ComputeFriction(const pxr::GfVec3f& correction, 
  const pxr::GfVec3f& relativeVelocity)
{
  pxr::GfVec3f friction(0.f);
  float correctionLength = correction.GetLength();
  if (_collision->GetFriction() > 0 && correctionLength > 0.f)
  {
    pxr::GfVec3f correctionNorm = correction / correctionLength;

    pxr::GfVec3f tangentialVelocity = relativeVelocity - correctionNorm *
      pxr::GfDot(relativeVelocity, correctionNorm);
    float tangentialLength = tangentialVelocity.GetLength();
    float maxTangential = correctionLength * _collision->GetFriction();

    friction = -tangentialVelocity * pxr::GfMin(maxTangential / tangentialLength, 1.0f);
  }

  return friction + correction * _collision->GetRestitution();
}

static float vMax = 5.f;

void CollisionConstraint::_SolveGeom(Particles* particles, float dt)
{
  _ResetCorrection();
  const size_t numElements = _elements.size();
  pxr::GfVec3f damp;
  const float alpha =  _compliance / (dt * dt);
  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];

    const pxr::GfVec3f position = _collision->GetContactPosition(index);
    const pxr::GfVec3f normal = _collision->GetContactNormal(index);
    const pxr::GfVec3f velocity = _collision->GetContactVelocity(index);
    const float d = _collision->GetContactDepth(index) + 
      pxr::GfMax(_collision->GetContactInitDepth(index) - vMax * dt, 0.f);
    if(d > 0.f)continue;

    damp = pxr::GfDot((particles->velocity[index] - velocity),  normal) * normal * _damp * dt * dt;

    _correction[elem] = -d * normal - damp;

    /*pxr::GfVec3f relativeVelocity = ((particles->predicted[index] + _correction[elem]) - 
      particles->position[index]) - velocity * dt;*/
    pxr::GfVec3f relativeVelocity = (particles->velocity[index] - velocity) * dt;
    
    pxr::GfVec3f friction = _ComputeFriction(_correction[elem], relativeVelocity);
    
    _correction[elem] +=  friction;
  }
}

static float selfVMax = 5.f;

void CollisionConstraint::_SolveSelf(Particles* particles, float dt)
{
  _ResetCorrection();

  SelfCollision* collision = (SelfCollision*)_collision;

  const size_t numElements = _elements.size();
  const float alpha = .1f / (dt * dt);
  size_t index, other, elem, c;
  float minDistance, distance, w0, w1, w, d;
  pxr::GfVec3f normal, correction, accum, velocity, damp;

  for (elem = 0; elem < numElements; ++elem) {
    index = _elements[elem];

    accum = pxr::GfVec3f(0.f);
    velocity = pxr::GfVec3f(0.f);

    size_t numContactUsed = 0;
    for(c = 0; c < collision->GetNumContacts(index); ++c) {
      other = collision->GetContactComponent(index, c);

      normal = collision->GetContactNormal(index, c);
      d = collision->GetContactDepth(index, c) 
        + pxr::GfMax(collision->GetContactInitDepth(index, c) - selfVMax * dt, 0.f);

      if(d >= 0.f) continue;
      w0 = particles->invMass[index];
      w1 = particles->invMass[other];     
      w = w0 + w1;
      if(w < 1e-6) continue;
    
      damp = pxr::GfDot((particles->velocity[index] -  
        _collision->GetContactVelocity(index, c)),  normal) * normal * _damp * dt * dt; 
      correction =  w0 / w *  -d * normal - damp;
      //correction += _collision->GetFriction() * (particles->velocity[index] * dt * dt * -d);

      accum += correction;

      velocity += (particles->velocity[index] - _collision->GetContactVelocity(index, c));

      numContactUsed++;
    }

    if(numContactUsed) {
      float rN = 1.f / (float)numContactUsed;
      _correction[elem] = accum * rN;
            
		  pxr::GfVec3f friction = _ComputeFriction(_correction[elem], velocity * rN);
      _correction[elem]  += w0 / w * friction;
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