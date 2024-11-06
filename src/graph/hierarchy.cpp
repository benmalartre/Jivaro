#include <pxr/usd/sdf/types.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include "../graph/hierarchy.h"
#include "../command/block.h"


JVR_NAMESPACE_OPEN_SCOPE

// Graph constructor
//------------------------------------------------------------------------------
HierarchyGraph::HierarchyGraph(SdfLayerRefPtr& layer) 
  : Graph(layer)
{
  Populate(_layer);
}

// Graph destructor
//------------------------------------------------------------------------------
HierarchyGraph::~HierarchyGraph()
{
}

// Graph populate
//------------------------------------------------------------------------------
void HierarchyGraph::Populate(SdfLayerRefPtr layer)
{
  _layer = layer;
  Clear();

  std::vector<SdfPath> paths;

  _layer->Traverse(SdfPath::AbsoluteRootPath(),
    [&paths, layer](SdfPath const& path) {
      if(path.IsPrimPath()) paths.push_back(path);
    });
  
  for(auto path: paths) {
    std::cout << path << std::endl;

    HierarchyGraph::HierarchyNode* node =
      new HierarchyGraph::HierarchyNode(path);

    AddNode(node);
    /*
    parent->AddChild(node);

    Graph::Connexion* connexion = 
      new Graph::Connexion(parent->GetChildrenPort(), node->GetParentPort());
    AddConnexion(connexion);
    _RecurseNodes(node);
    */
  
  }
  /*
  _DiscoverNodes();
  _DiscoverConnexions();
  */
}


HierarchyGraph::HierarchyNode::HierarchyNode(const SdfPath& path)
  : Graph::Node(path)
{
  _PopulatePorts();
}


void HierarchyGraph::HierarchyNode::_PopulatePorts()
{
  _ports.push_back(Graph::Port(this, Graph::Port::OUTPUT | Graph::Port::HIDDEN, ParentPortToken));
  _ports.push_back(Graph::Port(this, Graph::Port::INPUT | Graph::Port::HIDDEN, ChildrenPortToken));
}

/*
void
HierarchyGraph::_RecurseNodes(HierarchyGraph::HierarchyNode* parent)
{
  UsdPrim prim = _stage->GetPrimAtPath(parent->GetPrim().GetPath());
  for (auto& child : prim.GetChildren()) {
    HierarchyGraph::HierarchyNode* node =
      new HierarchyGraph::HierarchyNode(child, parent);

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
  
  std::cout << "execution discover connexions ..." << std::endl;
  for (UsdPrim child : _prim.GetChildren()) {
    if (child.IsA<UsdExecNode>()) {
      UsdExecNode node(child);
      for (UsdExecInput& input : node.GetInputs()) {
        if (input.GetConnectability() != UsdExecTokens->full) continue;
        UsdExecInput::SourceInfoVector connexions = input.GetConnectedSources();
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
  
}
*/
JVR_NAMESPACE_CLOSE_SCOPE