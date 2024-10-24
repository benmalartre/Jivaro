
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
//#include <pxr/usd/usdExec/execConnectableAPI.h>
//#include <pxr/usd/usdExec/execNode.h>
//#include <pxr/usd/usdExec/execGraph.h>

#include "../graph/graph.h"
#include "../command/command.h"
#include "../app/application.h"


JVR_NAMESPACE_OPEN_SCOPE

// Node constructor
//------------------------------------------------------------------------------
Graph::Node::Node(UsdPrim& prim)
  : _prim(prim)
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
Graph::GetNode(const UsdPrim& prim) const
{
  for (const Graph::Node* node : _nodes) {
    if(node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

Graph::Node* 
Graph::Graph::GetNode(const UsdPrim& prim)
{
  for (Graph::Node* node : _nodes) {
    if (node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

// Node add input
//------------------------------------------------------------------------------
void 
Graph::Node::AddInput(UsdAttribute& attribute, const TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
 
}

// Node add output
//------------------------------------------------------------------------------
void 
Graph::Node::AddOutput(UsdAttribute& attribute, const TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
}

// Node add io port
//------------------------------------------------------------------------------
void
Graph::Node::AddPort(UsdAttribute& attribute, const TfToken& name, size_t flags)
{
  Graph::Port port(this, flags, name, attribute);
  _ports.push_back(port);
}


// Node get port
//------------------------------------------------------------------------------
Graph::Port* 
Graph::Node::GetPort(const TfToken& name)
{
  for (auto& port : _ports) {
    if (port.GetName() == name) return &port;
  }
  return NULL;
}

// Node check authored values
//------------------------------------------------------------------------------
bool
Graph::Node::HasPort(const TfToken& name)
{
  for (auto& port : _ports) {
    if (port.GetName() == name) return true;
  }
  return false;
}


// Port
//------------------------------------------------------------------------------
Graph::Port::Port(Graph::Node* node, size_t flags, 
  const TfToken& label, UsdAttribute& attr)
  : _node(node), _flags(flags), _label(label), _attr(attr)
{
}

Graph::Port::Port(Graph::Node* node, size_t flags,
  const TfToken& label)
  : _node(node), _flags(flags), _label(label)
{
}

SdfPath
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
Graph::Graph(const UsdPrim& prim)
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
void Graph::Populate(const UsdPrim& prim)
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
Graph::_DiscoverNodes(UsdPrim& prim)
{
  
  if (!prim.IsValid())return;
 
  for (UsdPrim child : prim.GetChildren()) {
    Graph::Node* node = new Graph::Node(child);
    //node->SetBackgroundColor(GfVec3f(0.5f, 0.65f, 0.55f));
    AddNode(node);
  }
 
}


void
Graph::_DiscoverConnexions(UsdPrim& prim)
{
  
  for (UsdPrim child : prim.GetChildren()) {
    if (child.IsA<UsdShadeShader>()) {
      UsdShadeShader shader(child);
      for (UsdShadeInput& input : shader.GetInputs()) {
        if (input.GetConnectability() != UsdShadeTokens->full) continue;
        UsdShadeInput::SourceInfoVector connexions = input.GetConnectedSources();
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
    } else if (child.IsA<UsdExecNode>()) {
      UsdExecNode node(child);
      for (UsdExecInput& input : node.GetInputs()) {
        if (input.GetConnectability() != UsdShadeTokens->full) continue;
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

static bool 
_ConnexionPossible(const SdfValueTypeName& lhs, const SdfValueTypeName& rhs)
{
  if (lhs.GetDimensions() == rhs.GetDimensions())return true;
  return false;
}


bool
Graph::ConnexionPossible(const Graph::Port* lhs, const Graph::Port* rhs)
{
  const Graph::Node* lhsNode = lhs->GetNode();
  const UsdPrim& lhsPrim = lhsNode->GetPrim();
  const Graph::Node* rhsNode = rhs->GetNode();
  const UsdPrim& rhsPrim = rhsNode->GetPrim();

  if (lhsPrim.IsA<UsdShadeShader>()) {
    UsdShadeShader lhsShader(lhsPrim);
    UsdShadeOutput output = lhsShader.GetOutput(lhs->GetName());

    UsdShadeShader rhsShader(rhsPrim);
    UsdShadeInput input = rhsShader.GetInput(lhs->GetName());

    if (!output.IsDefined() || !input.IsDefined()) {
      return false;
    }
    return UsdShadeConnectableAPI::CanConnect(output, input);
  } /*else if (lhsPrim.IsA<UsdExecNode>()) {
    UsdExecNode lhsExec(lhsPrim);
    UsdExecOutput output = lhsExec.GetOutput(lhs->GetName());

    UsdExecNode rhsExec(rhsPrim);
    UsdExecInput input = rhsExec.GetInput(rhs->GetName());

    return UsdExecConnectableAPI::CanConnect(output, input);
  }*/

  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE