
#include "pxr/base/tf/regTest.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/error.h"
#include "pxr/base/tf/errorMark.h"

#include "pxr/base/arch/functionLite.h"
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/ndr/property.h>
#include <pxr/usd/ndr/node.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usdUI/backdrop.h>
#include <pxr/usd/usdExec/execConnectableAPI.h>
#include <pxr/usd/usdExec/execNode.h>
#include <pxr/usd/usdExec/execGraph.h>

#include "../graph/graph.h"
#include "../command/command.h"
#include "../app/application.h"


JVR_NAMESPACE_OPEN_SCOPE

// Node constructor
//------------------------------------------------------------------------------
Graph::Node::Node(pxr::UsdPrim& prim)
  : _prim(prim)
  , _dirty(DIRTY_POSITION|DIRTY_SIZE)
{
  if (_prim.IsValid())
  {
    _name = _prim.GetName();
  }
}

// Node destructor
//------------------------------------------------------------------------------
Graph::Node::~Node()
{

}

// Get node
//------------------------------------------------------------------------------
const Graph::Node*
Graph::GetNode(const pxr::UsdPrim& prim) const
{
  for (const Graph::Node* node : _nodes) {
    if(node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

Graph::Node* 
Graph::Graph::GetNode(const pxr::UsdPrim& prim)
{
  for (Graph::Node* node : _nodes) {
    if (node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

// Node add input
//------------------------------------------------------------------------------
void 
Graph::Node::AddInput(pxr::UsdAttribute& attribute, const pxr::TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
 
}

// Node add output
//------------------------------------------------------------------------------
void 
Graph::Node::AddOutput(pxr::UsdAttribute& attribute, const pxr::TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
}

// Node add io port
//------------------------------------------------------------------------------
void
Graph::Node::AddPort(pxr::UsdAttribute& attribute, const pxr::TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
}


// Node get port
//------------------------------------------------------------------------------
Graph::Port* 
Graph::Node::GetPort(const pxr::TfToken& name)
{
  for (auto& port : _ports) {
    if (port.GetName() == name) return &port;
  }
  return NULL;
}

// Node check authored values
//------------------------------------------------------------------------------
bool
Graph::Node::HasPort(const pxr::TfToken& name)
{
  for (auto& port : _ports) {
    if (port.GetName() == name) return true;
  }
  return false;
}


// Port
//------------------------------------------------------------------------------
Graph::Port::Port(Graph::Node* node, size_t flags, 
  const pxr::TfToken& label, pxr::UsdAttribute& attr)
  : _node(node), _flags(flags), _label(label), _attr(attr)
{
}

Graph::Port::Port(Graph::Node* node, size_t flags,
  const pxr::TfToken& label)
  : _node(node), _flags(flags), _label(label)
{
}

pxr::SdfPath
Graph::Port::GetPath()
{
  return _node->GetPrim().GetPath().AppendProperty(GetName());
}


bool 
Graph::Port::IsConnected(Graph* graph, Graph::Connexion* foundConnexion)
{
  for (auto& connexion : graph->GetConnexions()) {
    if (connexion->GetStart() == this || connexion->GetEnd() == this) {
      foundConnexion = connexion;
      return true;
    }
  }
  return false;
}

// Graph constructor
//------------------------------------------------------------------------------
Graph::Graph()
{
}

Graph::Graph(pxr::UsdPrim& prim)
  : _prim(prim)
{
}


// Graph destructor
//------------------------------------------------------------------------------
Graph::~Graph()
{
  Clear();
}


// Graph populate
//------------------------------------------------------------------------------
void Graph::Populate(pxr::UsdPrim& prim)
{
  Clear();
  _prim = prim;
  _DiscoverNodes();
  _DiscoverConnexions();
}

// Graph clear
//------------------------------------------------------------------------------
void Graph::Clear()
{
  for (auto& connexion : _connexions)delete connexion;
  for (auto& node : _nodes) delete node;
  _nodes.clear();
  _connexions.clear();
}

// Add node
//------------------------------------------------------------------------------
void
Graph::AddNode(Graph::Node* node)
{
  _nodes.push_back(node);
}

// Remove node
//------------------------------------------------------------------------------
void
Graph::RemoveNode(Graph::Node* node)
{

}

// Add connexion
//------------------------------------------------------------------------------
void
Graph::AddConnexion(Graph::Connexion* connexion)
{
  _connexions.push_back(connexion);
}

// Remove connexion
//------------------------------------------------------------------------------
void
Graph::RemoveConnexion(Graph::Connexion* connexion)
{

}

/*
void 
Graph::_DiscoverNodes(pxr::UsdPrim& prim)
{
  
  if (!prim.IsValid())return;
 
  for (pxr::UsdPrim child : prim.GetChildren()) {
    Graph::Node* node = new Graph::Node(child);
    //node->SetBackgroundColor(pxr::GfVec3f(0.5f, 0.65f, 0.55f));
    AddNode(node);
  }
 
}


void
Graph::_DiscoverConnexions(pxr::UsdPrim& prim)
{
  
  for (pxr::UsdPrim child : prim.GetChildren()) {
    if (child.IsA<pxr::UsdShadeShader>()) {
      pxr::UsdShadeShader shader(child);
      for (pxr::UsdShadeInput& input : shader.GetInputs()) {
        if (input.GetConnectability() != pxr::UsdShadeTokens->full) continue;
        pxr::UsdShadeInput::SourceInfoVector connexions = input.GetConnectedSources();
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
    } else if (child.IsA<pxr::UsdExecNode>()) {
      pxr::UsdExecNode node(child);
      for (pxr::UsdExecInput& input : node.GetInputs()) {
        if (input.GetConnectability() != pxr::UsdShadeTokens->full) continue;
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
  
}
       
*/

static bool 
_ConnexionPossible(const pxr::SdfValueTypeName& lhs, const pxr::SdfValueTypeName& rhs)
{
  if (lhs.GetDimensions() == rhs.GetDimensions())return true;
  return false;
}


bool
Graph::ConnexionPossible(const Graph::Port* lhs, const Graph::Port* rhs)
{
  const Graph::Node* lhsNode = lhs->GetNode();
  const pxr::UsdPrim& lhsPrim = lhsNode->GetPrim();
  const Graph::Node* rhsNode = rhs->GetNode();
  const pxr::UsdPrim& rhsPrim = rhsNode->GetPrim();

  if (lhsPrim.IsA<pxr::UsdShadeShader>()) {
    pxr::UsdShadeShader lhsShader(lhsPrim);
    pxr::UsdShadeOutput output = lhsShader.GetOutput(lhs->GetName());

    pxr::UsdShadeShader rhsShader(rhsPrim);
    pxr::UsdShadeInput input = rhsShader.GetInput(lhs->GetName());

    if (!output.IsDefined() || !input.IsDefined()) {
      return false;
    }
    return pxr::UsdShadeConnectableAPI::CanConnect(output, input);
  } else if (lhsPrim.IsA<pxr::UsdExecNode>()) {
    pxr::UsdExecNode lhsExec(lhsPrim);
    pxr::UsdExecOutput output = lhsExec.GetOutput(lhs->GetName());

    pxr::UsdExecNode rhsExec(rhsPrim);
    pxr::UsdExecInput input = rhsExec.GetInput(rhs->GetName());

    return pxr::UsdExecConnectableAPI::CanConnect(output, input);
  }

  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE