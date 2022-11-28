#include <pxr/usd/sdf/types.h
#include "../graph/graph.h"


JVR_NAMESPACE_OPEN_SCOPE

// Graph constructor
//------------------------------------------------------------------------------
HierarchyGraph::HierarchyGraph(pxr::SdfLayerRefPtr& layer) 
  : Graph()
{
  Populate(prim);
}

// Graph destructor
//------------------------------------------------------------------------------
HierarchyGraph::~HierarchyGraph()
{
}

HierarchyGraph::HierarchyNode::HierarchyNode(pxr::UsdPrim& prim)
  : Graph::Node(prim)
{
  _PopulatePorts();
}

void HierarchyGraph::HierarchyNode::_PopulatePorts()
{
    /*
  if (_prim.IsA<pxr::UsdExecGraph>()) {
    pxr::UsdExecGraph graph(_prim);
    for (const auto& input : graph.GetInputs()) {
      pxr::UsdAttribute attr = input.GetAttr();
      AddInput(attr, input.GetBaseName());
    }
    for (const auto& output : graph.GetOutputs()) {
      pxr::UsdAttribute attr = output.GetAttr();
      AddOutput(attr, output.GetBaseName());
    }
  }
  else if (_prim.IsA<pxr::UsdExecNode>()) {
    pxr::UsdExecNode node(_prim);
    for (const auto& input : node.GetInputs()) {
      pxr::UsdAttribute attr = input.GetAttr();
      AddInput(attr, input.GetBaseName());
    }
    for (const auto& output : node.GetOutputs()) {
      pxr::UsdAttribute attr = output.GetAttr();
      AddOutput(attr, output.GetBaseName());
    }
  }
  */
}

void
ExecutionGraph::_DiscoverNodes() 
{
  
  if (!_layer->GetRootPrims().empty())return;

  for (pxr::UsdPrim child : _prim.GetChildren()) {
    HierarchyGraph::HierarchyNode* node = new HierarchyGraph::HierarchyNode(child);
    AddNode(node);
  }
}

void
ExecutionGraph::_DiscoverConnexions()
{
    /*
  std::cout << "execution discover connexions ..." << std::endl;
  for (pxr::UsdPrim child : _prim.GetChildren()) {
    if (child.IsA<pxr::UsdExecNode>()) {
      pxr::UsdExecNode node(child);
      for (pxr::UsdExecInput& input : node.GetInputs()) {
        if (input.GetConnectability() != pxr::UsdExecTokens->full) continue;
        pxr::UsdExecInput::SourceInfoVector connexions = input.GetConnectedSources();
        for (auto& connexion : connexions) {
          Graph::Node* source = GetNode(connexion.source.GetPrim());
          Graph::Node* destination = GetNode(child);
          if (!source || !destination)continue;
          Graph::Port* start = source->GetPort(connexion.sourceName);
          Graph::Port* end = destination->GetPort(input.GetBaseName());

          if (start && end) {
            Graph::Connexion* connexion = new Connexion(start, end);
            _connexions.push_back(connexion);
          }
        }
      }
    }
  }
  */
}

JVR_NAMESPACE_CLOSE_SCOPE