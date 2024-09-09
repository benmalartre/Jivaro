
#include "../acceleration/bvh.h"
#include "../acceleration/gradient.h"
#include "../geometry/smooth.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE




void Gradient::Init(Mesh* mesh)
{
  size_t numPoints = mesh->GetNumPoints();
  _flags.resize(numPoints);
  _gradient.resize(numPoints);
  _value.resize(numPoints);
}
  
void Gradient::Compute(Mesh* mesh)
{

}

void Gradient::SetSeed(pxr::VtArray<int> &seed)
{
  for(size_t index = 0; index < seed.size(); ++index)
    BIT_SET(_flags[index], Gradient::SEED);
}

void Gradient::SetFixed(pxr::VtArray<int> &fixed)
{
  for(size_t index = 0; index < fixed.size(); ++index)
    BIT_SET(_flags[index], Gradient::FIXED);
}

void Gradient::_FindFeatures(Mesh* mesh)
{
  BVH bvh;
  bvh.Init({ mesh });


}

/**
 * 
 * t = h^2   
 * h is the mean spacing between adjacent nodes 
 * @return t 
 */
double
Gradient::_ComputeTime(Mesh* mesh) 
{
  double sum = 0.0;
  int n = 0;
  HalfEdgeGraph::ItUniqueEdge it(*mesh->GetEdgesGraph());
  HalfEdge* edge = it.Next();
  const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
  while(edge) {
    double d = (positions[edge->vertex] - positions[mesh->GetEdge(edge->next)->vertex]).GetLength();
    sum += d;
    n++;
   
  }
  return (sum /n ) * (sum /n);
}
	



JVR_NAMESPACE_CLOSE_SCOPE
