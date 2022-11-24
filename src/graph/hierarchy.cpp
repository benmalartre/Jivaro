#include <pxr/usd/sdf/types.h
#include "../graph/graph.h"


JVR_NAMESPACE_OPEN_SCOPE

HierarchyGraph::HierarchyGraph(pxr::SdfLayerRefPtr &layer)
  : _layer(layer)
{

}

JVR_NAMESPACE_CLOSE_SCOPE