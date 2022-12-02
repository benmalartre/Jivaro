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
class HierarchyNode : public Graph::Node {
  public:
    HierarchyNode(pxr::UsdPrim& prim);
    ~HierarchyNode() {};

    Graph::Port* GetParentPort() { return &_ports[0]; };
    Graph::Port* GetChildrenPort() { return &_ports[1]; };

  protected:
    void _PopulatePorts() override;
  };

public:
  HierarchyGraph(pxr::SdfLayerRefPtr &layer, pxr::UsdPrim& prim);
  ~HierarchyGraph()         override;

  virtual void Populate(pxr::UsdPrim& prim) override;

protected:
  virtual void _DiscoverNodes() override;
  virtual void _DiscoverConnexions() override;

  void _RecurseNodes(HierarchyNode* node);

private:
  pxr::SdfLayerRefPtr          _layer;
  pxr::UsdPrim                 _prim;

  float                        _currentX;
  float                        _currentY;

};
JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_GRAPH_HIERARCHY_H