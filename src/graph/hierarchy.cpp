#include <pxr/usd/sdf/types.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include "../graph/hierarchy.h"
#include "../command/block.h"


JVR_NAMESPACE_OPEN_SCOPE

// Graph constructor
//------------------------------------------------------------------------------
HierarchyGraph::HierarchyGraph(const pxr::SdfLayerRefPtr& layer, const pxr::UsdPrim& prim) 
  : Graph(prim)
  , _layer(layer)
{
  Populate(prim);
}

// Graph destructor
//------------------------------------------------------------------------------
HierarchyGraph::~HierarchyGraph()
{
}

// Graph populate
//------------------------------------------------------------------------------
void HierarchyGraph::Populate(const pxr::UsdPrim& prim)
{
  _prim = prim;
  Clear();
  _DiscoverNodes();
  _DiscoverConnexions();
}


HierarchyGraph::HierarchyNode::HierarchyNode(pxr::UsdPrim& prim, 
  HierarchyGraph::HierarchyNode* parent)
  : Graph::Node(prim)
  , _parent(parent)
{
  _PopulatePorts();
}


void HierarchyGraph::HierarchyNode::_PopulatePorts()
{
  _ports.push_back(Graph::Port(this, Graph::Port::OUTPUT | Graph::Port::HIDDEN, ParentPortToken));
  _ports.push_back(Graph::Port(this, Graph::Port::INPUT | Graph::Port::HIDDEN, ChildrenPortToken));
}

void
HierarchyGraph::_RecurseNodes(HierarchyGraph::HierarchyNode* parent)
{
  pxr::SdfPrimSpecHandle primSpec = _layer->GetPrimAtPath(parent->GetPrim().GetPath());
  for (const auto& child : primSpec.GetSpec().GetNameChildren()) {
    pxr::UsdPrim childPrim = 
      parent->GetPrim().GetChild(pxr::TfToken(child->GetName()));
    HierarchyGraph::HierarchyNode* node =
      new HierarchyGraph::HierarchyNode(childPrim, parent);
    AddNode(node);
    parent->AddChild(node);

    Graph::Connexion* connexion = 
      new Graph::Connexion(parent->GetChildrenPort(), node->GetParentPort());
    AddConnexion(connexion);
    _RecurseNodes(node);
  }
}

void
HierarchyGraph::_DiscoverNodes() 
{
  HierarchyGraph::HierarchyNode* node = 
    new HierarchyGraph::HierarchyNode(_prim);
  AddNode(node);
  _RecurseNodes(node);

}

void
HierarchyGraph::_DiscoverConnexions()
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