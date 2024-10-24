#ifndef JVR_GRAPH_EXECUTION_H
#define JVR_GRAPH_EXECUTION_H

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include "../common.h"
#include "../graph/graph.h"

JVR_NAMESPACE_OPEN_SCOPE

class ExecutionGraph : public Graph
{
  class ExecutionNode : public Graph::Node {
  public:
    ExecutionNode(UsdPrim& prim);
    ~ExecutionNode() {};
  protected:
    void _PopulatePorts() override;
  };

public:
  explicit ExecutionGraph(const UsdPrim& prim);
  ~ExecutionGraph();

protected:
  virtual void _DiscoverNodes() override;
  virtual void _DiscoverConnexions() override;

};

ExecutionGraph* TestUsdExecAPI();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GRAPH_EXECUTION_H