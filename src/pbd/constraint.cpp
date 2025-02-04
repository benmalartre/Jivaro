#include <pxr/base/work/loops.h>


#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

#include <usdPbd/constraintAPI.h>


JVR_NAMESPACE_OPEN_SCOPE

Constraint::Constraint(size_t elementSize, float stiffness, 
  float damp, const VtArray<int>& elems) 
  : Element(Element::CONSTRAINT)
  , _elements(elems)
  , _compliance(stiffness > 0.f ? 1.f / stiffness : 0.f)
  , _damp(damp)
  , _color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1)
{
  const size_t numElements = elems.size() / elementSize;

  _correction.resize(elems.size());
  _gradient.resize(numElements+1);
}

void Constraint::SetElements(const VtArray<int>& elems)
{
  const size_t numElements = _elements.size() / GetElementSize();
  _correction.resize(elems.size());
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
  memset(&_correction[0], 0.f, _correction.size() * sizeof(GfVec3f));
}

// this one has to happen serialy
void Constraint::ApplyPosition(Particles* particles)
{
  size_t corrIdx = 0;
  const GfVec2f* counter = &particles->counter[0];
  
  for(const auto& elem: _elements)
    particles->predicted[elem] += _correction[corrIdx++] / counter[elem][0];
}

// this one has to happen serialy
void Constraint::ApplyVelocity(Particles* particles)
{
  size_t corrIdx = 0;
  const GfVec2f* counter = &particles->counter[0];
  for(const auto& elem: _elements)
    particles->velocity[elem] += _correction[corrIdx++] / counter[elem][0];
}

void Constraint::Reset(Particles* particles)
{
  _ResetCorrection();
}


ConstraintsGroup* CreateConstraintsGroup(Body* body, const TfToken& name, short type, 
  const VtArray<int>& allElements, size_t elementSize, size_t blockSize, void* data)
{

  ConstraintsGroup* group = body->AddConstraintsGroup(name, type);

  size_t numElements = allElements.size();
  size_t numBlocks = 0;
  size_t first = 0;
  size_t last = GfMin(blockSize * elementSize, numElements);

  float stiffness = 100000.f;
  float damping = 0.1;

  Geometry* geometry = body->GetGeometry();

  while(true) {
    VtArray<int> blockElements(allElements.begin()+first, allElements.begin()+last);
    switch (type) {
      case Constraint::ATTACH:
        group->constraints.push_back(new AttachConstraint(body, blockElements, stiffness, damping));
        break;

      case Constraint::PIN:
        group->constraints.push_back(new PinConstraint(body, blockElements, (Geometry*)data, stiffness, damping));
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
    last = GfMin(last + blockSize * elementSize, numElements);
    numBlocks++;
  }

  return group;
}



//-----------------------------------------------------------------------------------------
//   ATTACH CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t AttachConstraint::TYPE_ID = Constraint::ATTACH;
size_t AttachConstraint::ELEM_SIZE = 1;

AttachConstraint::AttachConstraint(Body* body, const VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
  , _body(body)
{
  Geometry* geometry = body->GetGeometry();
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const GfMatrix4d& m = geometry->GetMatrix();
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
}

void AttachConstraint::SolvePosition(Particles* particles, float dt)
{
  _ResetCorrection();

  const float alpha =  _compliance / (dt * dt);

  const GfVec3f* predicted = &particles->predicted[0];
  const GfVec3f* velocity = &particles->velocity[0];

  Geometry* geometry = _body->GetGeometry();

  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();

  size_t index = 0;

  for(auto& elem: _elements) {
    const GfVec3f gradient = predicted[elem] - particles->input[elem];
    const float length = gradient.GetLength();
    if(length < 1e-9)continue;

    const GfVec3f normal = gradient.GetNormalized();
    const GfVec3f correction = -length / (particles->invMass[index] + alpha) * gradient;
    const GfVec3f damp = GfDot(velocity[index] * dt * dt,  normal) * normal * _damp;

    _correction[index++] += particles->invMass[index] * correction - damp;
  }
}

void AttachConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions,
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
{
}

ConstraintsGroup* CreateAttachConstraints(Body* body, float stiffness, float damping, const VtArray<int>* elements)
{
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();
  VtArray<int> allElements;

  if(elements) {
    allElements.resize(elements->size());  
    for(size_t i = 0; i < elements->size(); ++i)
      allElements[i] = (*elements)[i] + offset;
  } else {
    allElements.resize(body->GetNumPoints());  
    for(size_t i = 0; i < body->GetNumPoints(); ++i)
      allElements[i] = i + offset;
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("attach"), Constraint::ATTACH,
        allElements, AttachConstraint::ELEM_SIZE, Constraint::BlockSize);
  
  return NULL;

}

//-----------------------------------------------------------------------------------------
//   PIN CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t PinConstraint::TYPE_ID = Constraint::PIN;
size_t PinConstraint::ELEM_SIZE = 1;

 PinConstraint::PinConstraint(Body* body, const VtArray<int>& elems, Geometry* target, 
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
  const GfMatrix4d& m = geometry->GetMatrix();
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;


  for(const auto& elem: elems) {

  }
}

void PinConstraint::SolvePosition(Particles* particles, float dt)
{
}

void PinConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions,
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
{
}

ConstraintsGroup* CreatePinConstraints(Body* body, Geometry* target, float stiffness, float damping,
  const VtArray<int> *elements)
{

  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();
  VtArray<int> allElements;
  if(elements) {
    allElements.resize(elements->size());  
    for(size_t i = 0; i < elements->size(); ++i)
      allElements[i] = (*elements)[i] + offset;
  } else {
    allElements.resize(body->GetNumPoints());  
    for(size_t i = 0; i < body->GetNumPoints(); ++i)
      allElements[i] = i + offset;
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("pin"), Constraint::PIN,
        allElements, AttachConstraint::ELEM_SIZE, Constraint::BlockSize, target);
 
  return NULL;

}

//-----------------------------------------------------------------------------------------
//   STRETCH CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t StretchConstraint::TYPE_ID = Constraint::STRETCH;
size_t StretchConstraint::ELEM_SIZE = 2;

StretchConstraint::StretchConstraint(Body* body, const VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const size_t offset = body->GetOffset();
  const GfMatrix4d& m = geometry->GetMatrix();
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = 
      (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] - offset]) - 
       m.Transform(positions[_elements[elemIdx * ELEM_SIZE] - offset])).GetLength();
  }
}

void StretchConstraint::Reset(Particles* particles)
{
  Constraint::Reset(particles);
  size_t numElements = _elements.size() / ELEM_SIZE;
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    _rest[elemIdx] = (particles->position[_elements[elemIdx * ELEM_SIZE + 1]] -
      particles->position[_elements[elemIdx * ELEM_SIZE]]).GetLength();
  }
}

void StretchConstraint::SolvePosition(Particles* particles, float dt)
{

  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;

  const float alpha =  _compliance / (dt * dt);
  size_t a, b;
  float w0, w1, W, C, length;
  GfVec3f gradient, normal, correction, damp;

  const GfVec3f* predicted = &particles->predicted[0];
  const GfVec3f* velocity = &particles->velocity[0];
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

    normal = gradient.GetNormalized();

    C = length - _rest[elem];

    damp = GfDot((velocity[a] + velocity[b]) * 0.5f * dt * dt,  normal) * normal * _damp;
    correction = -C / (W * length * length + alpha) * gradient - damp;
    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 1] -= w1 * correction;
  }
}

void StretchConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions, 
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
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

static void _GetMeshStretchElements(Mesh* mesh, VtArray<int>& allElements, size_t offset)
{
  if(!(mesh->GetFlags() & Mesh::HALFEDGES))mesh->ComputeHalfEdges();  
  const GfVec3f* positions = mesh->GetPositionsCPtr();
  HalfEdgeGraph::ItUniqueEdge it(*mesh->GetEdgesGraph());
  const auto& edges = mesh->GetEdges();
  const GfMatrix4d& m = mesh->GetMatrix();
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

static void _GetCurveStretchElements(Curve* curve, VtArray<int>& allElements, size_t offset)
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
  VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();
  
  if (geometry->GetType() == Geometry::MESH) 
    _GetMeshStretchElements((Mesh*)geometry, allElements, offset);
  else if (geometry->GetType() == Geometry::CURVE)
    _GetCurveStretchElements((Curve*)geometry, allElements, offset);
  
  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("stretch"), Constraint::STRETCH,
        allElements, StretchConstraint::ELEM_SIZE, Constraint::BlockSize);
  
  return NULL;

}


//-----------------------------------------------------------------------------------------
//   BEND CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t BendConstraint::TYPE_ID = Constraint::BEND;
size_t BendConstraint::ELEM_SIZE = 3;

BendConstraint::BendConstraint(Body* body, const VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const GfMatrix4d& m = geometry->GetMatrix();

  size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    /*
    GfVec3f center = (
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] - offset]),
        m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] - offset]),
          m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] - offset])) / 3.f;
    _rest[elemIdx] = (center - m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1]])).GetLength();
    */
    _rest[elemIdx] = (m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] - offset]) - 
      m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] - offset])).GetLength();
      
  }
}

void BendConstraint::SolvePosition(Particles* particles, float dt)
{
  _ResetCorrection();

  const size_t numElements = _elements.size() / ELEM_SIZE;
  const float alpha = _compliance / (dt * dt);

  GfVec3f x0, x1, x2, center, h, correction, damp;
  float w0, w1, w2, W, hL, C;
  size_t a, b, c;

  const GfVec3f* predicted = &particles->predicted[0];
  const GfVec3f* velocity = &particles->velocity[0];
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

    damp = GfDot(velocity[a] * dt * dt,  h) * h * _damp;
    correction = -C / (hL * hL * W + alpha) * h - damp;

    _correction[elem * ELEM_SIZE + 0] += w0 * correction;
    _correction[elem * ELEM_SIZE + 2] -= w1 * correction;

  }
}

void BendConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions, 
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
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

int _FindBestVertex(const GfVec3f* positions, size_t p, size_t n, 
  const int* neighbors, size_t numNeighbors, const int* adjacents, size_t numAdjacents)
{
  int best = -1;
  float minCosine = FLT_MAX;
  for (size_t o = 0; o < numNeighbors; ++o) {
    if (neighbors[n] == neighbors[o])continue;
    if(_IsAdjacent(neighbors[o], adjacents, numAdjacents))continue;

    const GfVec3f e0 = (positions[neighbors[n]] - positions[p]);
    const GfVec3f e1 = (positions[neighbors[o]] - positions[p]);
    float cosine = GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
    if (cosine < minCosine) {
      minCosine = cosine;
      best = o;
    }
  }
  return best;
}

GfVec2i _FindBestBoundaryVertices(const GfVec3f* positions, size_t p, 
  const int* neighbors, int num, const bool* boundaries)
{
  GfVec2i result(-1,-1);
  int best = -1;
  float minCosine = FLT_MAX;
  for (size_t m = 0; m < num; ++m) {
    if(!boundaries[neighbors[m]])continue;
    for (size_t n = 0; n < num; ++n) {
      if (neighbors[m] == neighbors[n] || !boundaries[neighbors[n]])continue;

      const GfVec3f e0 = (positions[neighbors[m]] - positions[p]);
      const GfVec3f e1 = (positions[neighbors[n]] - positions[p]);
      float cosine = GfDot(e0, e1) / (e0.GetLength() * e1.GetLength());
      if (cosine < minCosine) {
        minCosine = cosine;
        result = m > n ? GfVec2i(n, m) : GfVec2i(m, n);
      }
    }
  }
  return result;
}

void _GetMeshBendElements(Mesh* mesh, VtArray<int>& allElements, size_t offset) 
{
  const GfVec3f* positions = mesh->GetPositionsCPtr();

  if(!(mesh->GetFlags() & Mesh::NEIGHBORS))mesh->ComputeNeighbors();  
  if(!(mesh->GetFlags() & Mesh::ADJACENTS))mesh->ComputeAdjacents();  

  const size_t numPoints = mesh->GetNumPoints();
  const GfMatrix4d& m = mesh->GetMatrix();
  const HalfEdgeGraph* graph = mesh->GetEdgesGraph();
  const VtArray<bool>& boundaries = graph->GetBoundaries();

  for (size_t p = 0; p < numPoints; ++p) {

    size_t numAdjacents = mesh->GetNumAdjacents(p);
    const int* adjacents = mesh->GetAdjacents(p);

    size_t numNeighbors = mesh->GetNumNeighbors(p);
    const int* neighbors = mesh->GetNeighbors(p);

    if(boundaries[p]) {
      if(numNeighbors <= 2)continue;

      GfVec2i pair = _FindBestBoundaryVertices(positions, p, neighbors, numNeighbors, &boundaries[0]);
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
  VtArray<int> allElements;
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
        allElements.push_back(curveStartIdx + segmentIdx + offset);
        allElements.push_back(curveStartIdx + segmentIdx + 2 + offset);
        allElements.push_back(curveStartIdx + segmentIdx + 1 + offset);
      }
      curveStartIdx += numCVs;
    }
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("bend"), Constraint::BEND,
        allElements, BendConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
}

//-----------------------------------------------------------------------------------------
//   SHEAR CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t ShearConstraint::TYPE_ID = Constraint::SHEAR;

ShearConstraint::ShearConstraint(Body* body, const VtArray<int>& elems,
  float stiffness, float damping)
  : StretchConstraint(body, elems, stiffness, damping)
{
}

static void _GetMeshShearElements(Mesh* mesh, VtArray<int>& allElements, size_t offset)
{
  const GfVec3f* positions = mesh->GetPositionsCPtr();

  const size_t numFaces = mesh->GetNumFaces();
  const GfMatrix4d& m = mesh->GetMatrix();
  const VtArray<int>& counts = mesh->GetFaceCounts();
  const VtArray<int>& connects = mesh->GetFaceConnects();
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

static void _GetCurveShearElements(Curve* curve, VtArray<int>& allElements, size_t offset)
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
  VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if (geometry->GetType() == Geometry::MESH) 
    _GetMeshShearElements((Mesh*)geometry, allElements, offset);
  else if (geometry->GetType() == Geometry::CURVE) 
    _GetCurveShearElements((Curve*)geometry, allElements, offset);

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("shear"), Constraint::SHEAR,
        allElements, StretchConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
}

size_t DihedralConstraint::TYPE_ID = Constraint::DIHEDRAL;
size_t DihedralConstraint::ELEM_SIZE = 4;

static float _GetCotangentTheta(const GfVec3f& a, const GfVec3f& b)
{
  const float cosTheta = GfDot(a, b);
  const float sinTheta = GfCross(a, b).GetLength();
  return cosTheta / sinTheta;
};

DihedralConstraint::DihedralConstraint(Body* body, const VtArray<int>& elems,
  float stiffness, float damping)
  : Constraint(ELEM_SIZE, stiffness, damping, elems)
{
  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();

  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const GfMatrix4d& m = geometry->GetMatrix();
  const size_t numElements = _elements.size() / ELEM_SIZE;
  _rest.resize(numElements);
  for(size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    const GfVec3f x0(m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 0] - offset]));
    const GfVec3f x1(m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 1] - offset]));
    const GfVec3f x2(m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 2] - offset]));
    const GfVec3f x3(m.Transform(positions[_elements[elemIdx * ELEM_SIZE + 3] - offset]));

    GfVec3f n1 = GfCross(x1 - x0, x2 - x0).GetNormalized();
    GfVec3f n2 = GfCross(x1 - x0, x3 - x0).GetNormalized();

    float dot = GfClamp(GfDot(n1, n2), -1.f, 1.f);

    _rest[elemIdx] = std::acos(dot);
  }
}

void DihedralConstraint::SolvePosition(Particles* particles, float dt)
{
  _ResetCorrection();

  size_t numElements = _elements.size() / ELEM_SIZE;
  size_t a, b, c, d;
  GfVec3f x0, x1, x2, x3, n1, n2, q0, q1, q2, q3;
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

    n1 = GfCross(x1, x2);
    n2 = GfCross(x1, x3);

    n1.Normalize();
    n2.Normalize();
    dot = GfClamp(GfDot(n1, n2), 0.f, 1.f);

    angle = std::acos(dot);	
    if (angle < 1e-6 || std::isnan(dot)) continue;

    q2 =  (x1 ^ n2 + n1 ^ x1 * dot) / ((x1 ^ x2).GetLength() + 1e-6f);
    q3 =  (x1 ^ n1 + n2 ^ x1 * dot) / ((x1 ^ x3).GetLength() + 1e-6f);
    q1 = -(x2 ^ n2 + n1 ^ x2 * dot) / ((x1 ^ x2).GetLength() + 1e-6f) - 
                       (x3 ^ n1 + n2 ^ x3 * dot) / ((x1 ^ x3).GetLength() + 1e-6f);
    q0 = -q1 - q2 - q3;


    float denom = alpha + (w0 * GfDot(q0, q0) + w1 * GfDot(q1, q1) + w2 * GfDot(q2, q2) + w3 * GfDot(q3, q3));
	  if (denom < 1e-6f) continue;
		lambda = -GfSqrt(1.0f - dot * dot) * (angle - _rest[elem]) / denom;

    _correction[elem * ELEM_SIZE + 0] += w0 * lambda * q0;
    _correction[elem * ELEM_SIZE + 1] += w1 * lambda * q1;
    _correction[elem * ELEM_SIZE + 2] += w2 * lambda * q2;
    _correction[elem * ELEM_SIZE + 3] += w3 * lambda * q3;
  }
}

void DihedralConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions, 
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
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
  VtArray<int> allElements;
  Geometry* geometry = body->GetGeometry();
  if (geometry->GetType() == Geometry::MESH) {  
    Mesh* mesh = (Mesh*)geometry;
    const GfVec3f* positions = mesh->GetPositionsCPtr();
    VtArray<TrianglePair> triPairs;
    mesh->GetAllTrianglePairs(triPairs, false);
    for(const auto& triPair: triPairs) {
      GfVec4i vertices = triPair.GetVertices();
      for(size_t i = 0; i < 4; ++i)allElements.push_back(vertices[i] + offset);
    }
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("dihedral"), Constraint::DIHEDRAL,
        allElements, DihedralConstraint::ELEM_SIZE, Constraint::BlockSize);

  return NULL;
}

//-----------------------------------------------------------------------------------------
//   COLLISION / SELF-COLLISION CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t CollisionConstraint::TYPE_ID = Constraint::COLLISION;

CollisionConstraint::CollisionConstraint(Body* body, Collision* collision,
  const VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(1, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::GEOM)
  , _SolvePosition(&CollisionConstraint::_SolvePositionGeom)
  , _SolveVelocity(&CollisionConstraint::_SolveVelocityGeom)
{
}

CollisionConstraint::CollisionConstraint(Particles* particles, SelfCollision* collision, 
  const VtArray<int>& elems, float stiffness, float damping, float restitution, float friction)
  : Constraint(1, stiffness, damping, elems)
  , _collision(collision)
  , _mode(CollisionConstraint::SELF)
  , _SolvePosition(&CollisionConstraint::_SolvePositionSelf)
  , _SolveVelocity(&CollisionConstraint::_SolveVelocitySelf)
{
}

// this one has to happen serialy
void CollisionConstraint::ApplyPosition(Particles* particles)
{
  size_t corrIdx = 0;
  const GfVec2f* counter = &particles->counter[0];
  for(const auto& elem: _elements)
    particles->predicted[elem] += _correction[corrIdx++] / counter[elem][1];
}

// this one has to happen serialy
void CollisionConstraint::ApplyVelocity(Particles* particles)
{
  size_t corrIdx = 0;
  const GfVec2f* counter = &particles->counter[0];
  for (const auto& elem : _elements)
    particles->velocity[elem] += _correction[corrIdx++] / counter[elem][1];
}

GfVec3f CollisionConstraint::_ComputeFriction(const float friction, const GfVec3f& correction, 
  const GfVec3f& relativeVelocity)
{
  float correctionLength = correction.GetLength();
  if (friction > 0 && correctionLength > 0.f)
  {
    GfVec3f correctionNorm = correction / correctionLength;

    GfVec3f tangentialVelocity = relativeVelocity - correctionNorm *
      GfDot(relativeVelocity, correctionNorm);
    float tangentialLength = tangentialVelocity.GetLength();
    float maxTangential = correctionLength * friction;

    return -tangentialVelocity * GfMin(maxTangential / tangentialLength, 1.0f);
  }

  return GfVec3f(0.f);
}

void CollisionConstraint::_SolvePositionGeom(Particles* particles, float dt)
{
  _ResetCorrection();
  const size_t numElements = _elements.size();
  GfVec3f damp, position, gradient, normal, correction;

  const float stiffness = _collision->GetStiffness();
  _compliance = stiffness < 1e-9 ? 0.f : 1.f / stiffness;
  const float alpha = _compliance / (dt * dt);

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];

    const GfVec3f position = _collision->GetContactPosition(index);
    const GfVec3f normal = _collision->GetContactNormal(index);
    const GfVec3f velocity = _collision->GetContactVelocity(index);

    float d = _collision->GetContactDepth(index);

    if(particles->mass[index] < 1e-9 || d > 0.f) continue;

    particles->color[index] = GfVec3f(0.75, 0.75, 0.5);
    
    if(_collision->GetMaxSeparationVelocity() > 0.f)
      d += GfMax(-_collision->GetContactInitDepth(index) - 
      _collision->GetMaxSeparationVelocity() * dt, 0.f);

    const float lagrange = -d / (particles->invMass[index] + alpha);
    const GfVec3f correction = lagrange * particles->invMass[index] * normal;

    //const GfVec3f correction = -d * normal;
    damp = GfDot(correction, normal) * normal * _collision->GetDamp();
    _correction[elem] = correction - damp;

    GfVec3f friction = _ComputeFriction(_collision->GetFriction(), 
      _correction[elem], particles->velocity[index] - velocity);
    _correction[elem] +=  friction;
/*
    particles->color[index] = GfVec3f(
      RESCALE(_collision->GetContactInitDepth(index), 0, particles->radius[index], 0, 1),
      RESCALE(_collision->GetContactInitDepth(index), 0, particles->radius[index], 1, 0),
      0.f
    );
*/    
  }
}

void CollisionConstraint::_SolveVelocityGeom(Particles* particles, float dt)
{

  _ResetCorrection(); 
  const size_t numElements = _elements.size();

  for (size_t elem = 0; elem < numElements; ++elem) {
    const size_t index = _elements[elem];
    _collision->SetContactTouching(index, false); 
    if(_collision->GetContactDepth(index) > _collision->GetMargin()) continue;

    const GfVec3f previous = particles->previous[index];
    const GfVec3f normal = _collision->GetContactNormal(index);
    const GfVec3f normalVelocity = GfDot(previous, normal) * normal;
    const GfVec3f tangentVelocity = previous - normalVelocity;

    _correction[elem] += (-particles->velocity[index] +
       (-_collision->GetRestitution() * normalVelocity + (1.f - _collision->GetFriction()) * tangentVelocity) + 
       _collision->GetContactVelocity(index)) * 0.5f;

    _collision->SetContactTouching(index, true);
  }
}

void CollisionConstraint::_SolvePositionSelf(Particles* particles, float dt)
{

  _ResetCorrection();

  SelfCollision* collision = (SelfCollision*)_collision;

  _compliance = 0.0001f;
  const float alpha = _compliance / (dt * dt);

  const size_t numElements = _elements.size();
  size_t index, other, elem, c;
  float minDistance, distance, w0, w1, w, d;
  GfVec3f normal, correction, accum, velocity, damp;

  for (elem = 0; elem < numElements; ++elem) {
    index = _elements[elem];
    Body* body = particles->body[index];
    if(!body->GetSelfCollisionEnabled())continue;
    
    w0 = particles->invMass[index];
    if(w0 < 1e-6)continue;

    accum = GfVec3f(0.f);
    velocity = GfVec3f(0.f);

    const float selfMaxV = body->GetSelfCollisionMaxSeparationVelocity();

    size_t numContactUsed = 0;
    for(c = 0; c < collision->GetNumContacts(index); ++c) {
      other = collision->GetContactComponent(index, c);

      normal = collision->GetContactNormal(index, c);
      d = collision->GetContactDepth(index, c) + 
        GfMax(collision->GetContactInitDepth(index, c) - _collision->GetMaxSeparationVelocity() * dt, 0.f);
        
      w1 = particles->invMass[other];     
      w = w0 + w1;
      if(w < 1e-6) continue;

      damp = GfDot((particles->velocity[index] -  
        _collision->GetContactVelocity(index, c)) * dt * dt,  normal) * normal * _damp; 
      correction =  w0 / w *  -d * normal - damp;

      accum += correction;

      velocity = _collision->GetContactVelocity(index, c) * dt;

      numContactUsed++;
    }

    if(numContactUsed) {
      float rN = 1.f / (float)numContactUsed;
      _correction[elem] = accum * rN;
            
		  GfVec3f friction = _ComputeFriction(body->GetSelfCollisionFriction(), _correction[elem], velocity * rN);
      _correction[elem]  += w0 / w * friction;
    }
  }
}

void CollisionConstraint::_SolveVelocitySelf(Particles* particles, float dt)
{
  return;
  _ResetCorrection();
  const size_t numElements = _elements.size();
  GfVec3f velocity, normal;
  float d, w0, w1, w;
  for (size_t elem = 0; elem < numElements; ++elem) {
    size_t index = _elements[elem];

    velocity = GfVec3f(0.f);
    w0 = particles->invMass[index];
    if(w0 < 1e-6)continue;

    size_t numContactUsed = 0;
    for (size_t c = 0; c < _collision->GetNumContacts(index); ++c) {
      size_t other = _collision->GetContactComponent(index, c);

      w1 = particles->invMass[other];     
      w = w0 + w1;
      if(w < 1e-6) continue;

      velocity += _collision->GetContactVelocity(index, c);
      numContactUsed++;
    }
    
    if (numContactUsed) {
      float rN = 1.f / (float)numContactUsed;
      _correction[elem] = w0 / w * (-particles->velocity[index] + velocity * rN) * 0.5f;
    }
  }
}

void CollisionConstraint::SolvePosition(Particles* particles, float dt)
{
  (this->*_SolvePosition)(particles, dt);
}

void CollisionConstraint::SolveVelocity(Particles* particles, float dt)
{
  (this->*_SolveVelocity)(particles, dt);
}

void CollisionConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions, 
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
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


//-----------------------------------------------------------------------------------------
//   CONTACT CONSTRAINT
//-----------------------------------------------------------------------------------------
size_t ContactConstraint::TYPE_ID = Constraint::CONTACT;
size_t ContactConstraint::ELEM_SIZE = 1;

ContactConstraint::ContactConstraint(Body* body, const VtArray<int>& elems, Collision* collision,
  const VtArray<Contact*> &contacts, float stiffness, float damping)
    : Constraint(ELEM_SIZE, stiffness, damping, elems)
    , _collision(collision)
{
  Geometry* geometry = body->GetGeometry();
  if(geometry->GetType() < Geometry::POINT) {
    // TODO add warning message here
    return;
  }
  const size_t offset = body->GetOffset();
  const GfMatrix4d& m = geometry->GetMatrix();
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  size_t numElements = _elements.size() / ELEM_SIZE;

  for(auto& contact: contacts)
    _contacts.push_back(*contact);

}

void ContactConstraint::SolvePosition(Particles* particles, float dt)
{
  _ResetCorrection();

  const float alpha =  _compliance / (dt * dt);

  const GfVec3f* predicted = &particles->predicted[0];
  const GfVec3f* velocity = &particles->velocity[0];
  const float* radius = &particles->radius[0];

  const Geometry* geometry = _collision->GetGeometry();
  const GfVec3f* positions = ((Deformable*)geometry)->GetPositionsCPtr();
  const GfVec3f* normals = ((Deformable*)geometry)->GetNormalsCPtr();

  if(geometry->GetType() == Geometry::MESH) {
    
    Mesh* mesh = (Mesh*)geometry;
    for(size_t elem = 0; elem < _elements.size();++elem) {
      size_t index = _elements[elem];
      const Triangle* triangle = mesh->GetTriangle(_contacts[elem].GetComponentIndex());

      const GfVec3f position = 
        _contacts[elem].ComputePosition(positions, &triangle->vertices[0], 3, &mesh->GetMatrix());

      const GfVec3f normal = 
        _contacts[elem].ComputeNormal(normals, &triangle->vertices[0], 3, &mesh->GetMatrix());

      const GfVec3f desired = position + normal * radius[index];

      const GfVec3f gradient = predicted[index] - desired;
      const float length = gradient.GetLength();
      if(length < 1e-9)continue;

      const GfVec3f gradN = gradient.GetNormalized();
      const GfVec3f correction = -length / (particles->invMass[index] + alpha) * gradient;
      const GfVec3f damp = GfDot(velocity[index] * dt * dt, gradN) * gradN * _damp;

      _correction[elem] += particles->invMass[index] * correction - damp;
    }
  }

  
}

void ContactConstraint::GetPoints(Particles* particles, VtArray<GfVec3f>& positions,
  VtArray<float>& radius, VtArray<GfVec3f>& colors)
{
  const size_t numElements = _elements.size();
  for (size_t elemIdx = 0; elemIdx < numElements; ++elemIdx) {
    positions.push_back(
      particles->predicted[_elements[elemIdx]]);
    positions.push_back(GfVec3f(_contacts[elemIdx].GetPoint()));
    radius.push_back(0.02f);
    radius.push_back(0.02f);
    colors.push_back(_color);
    colors.push_back(_color);
  }
}

ConstraintsGroup* CreateContactConstraints(Body* body, Collision* collision, float stiffness, float damping,
  const VtArray<int> *elements)
{

  Geometry* geometry = body->GetGeometry();
  size_t offset = body->GetOffset();
  VtArray<int> allElements;
  if(elements) {
    if(!elements->size())return NULL;
    allElements.resize(elements->size());  
    for(size_t i = 0; i < elements->size(); ++i)
      allElements[i] = (*elements)[i] + offset;
  } else {
    allElements.resize(body->GetNumPoints());  
    for(size_t i = 0; i < body->GetNumPoints(); ++i)
      allElements[i] = i + offset;
  }

  if(allElements.size())
    return CreateConstraintsGroup(body, 
      TfToken("contact"), Constraint::CONTACT,
        allElements, ContactConstraint::ELEM_SIZE, Constraint::BlockSize, collision);
 
  return NULL;

}

JVR_NAMESPACE_CLOSE_SCOPE