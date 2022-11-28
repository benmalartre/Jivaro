#ifndef JVR_GRAPH_HIERARCHY_H
#define JVR_GRAPH_HIERARCHY_H

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include "../common.h"
#include "../graph/graph.h"

JVR_NAMESPACE_OPEN_SCOPE

class HierarchyGraph : public Graph
{
public:
  HierarchyGraph(pxr::SdfLayerRefPtr &layer);
  ~HierarchyGraph()         override;

private:
    pxr::SdfLayerRefPtr&        _layer;
    pxr::SdfLayerRefPtr         _anonymous;
    pxr::UsdStageRefPtr         _stage;
};
JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GRAPH_HIERARCHY_H