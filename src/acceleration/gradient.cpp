
#include "../acceleration/bvh.h"
#include "../acceleration/gradient.h"
#include "../geometry/smooth.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE

// https://github.com/IasonManolas/Geodesics_in_heat/blob/master/src/geodesicdistance.hpp



/*
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
*/
	



JVR_NAMESPACE_CLOSE_SCOPE
