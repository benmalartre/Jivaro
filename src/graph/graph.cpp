
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
    _name = prim.GetName();

    if (prim.HasAPI<pxr::UsdUINodeGraphNodeAPI>()) {
      pxr::UsdUINodeGraphNodeAPI api(prim);
      pxr::TfTokenVector attributeNames = api.GetSchemaAttributeNames(true);
    } else {
      if (pxr::UsdUINodeGraphNodeAPI::CanApply(prim)) {
        pxr::UsdUINodeGraphNodeAPI::Apply(prim);
      }
      else {
        std::cout <<
          "Invalid prim for applying UsdUINodeGraphNodeAPI : %s." <<
          prim.GetPath().GetText() << std::endl;
        return;
      }
    }

    Update();

    /*
  
    if (prim.IsA<pxr::UsdShadeNodeGraph>()) {
      pxr::UsdShadeNodeGraph graph(prim);
      for (const auto& input : graph.GetInputs()) {
        pxr::UsdAttribute attr = input.GetAttr();
        AddInput(attr, input.GetBaseName());
      }
      for (const auto& output : graph.GetOutputs()) {
        pxr::UsdAttribute attr = output.GetAttr();
        AddOutput(attr, output.GetBaseName());
      }
    }
    else if (prim.IsA<pxr::UsdShadeShader>()) {
      pxr::UsdShadeShader shader(prim);
      for (const auto& input : shader.GetInputs()) {
        pxr::UsdAttribute attr = input.GetAttr();
        AddInput(attr, input.GetBaseName());
      }
      for (const auto& output : shader.GetOutputs()) {
        pxr::UsdAttribute attr = output.GetAttr();
        AddOutput(attr, output.GetBaseName());
      }
    }
    else if (prim.IsA<pxr::UsdExecGraph>()) {
      pxr::UsdExecGraph graph(prim);
      for (const auto& input : graph.GetInputs()) {
        pxr::UsdAttribute attr = input.GetAttr();
        AddInput(attr, input.GetBaseName());
      }
      for (const auto& output : graph.GetOutputs()) {
        pxr::UsdAttribute attr = output.GetAttr();
        AddOutput(attr, output.GetBaseName());
      }
    }
    else if (prim.IsA<pxr::UsdExecNode>()) {
      pxr::UsdExecNode node(prim);
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
Graph::Node::AddInput(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  Graph::Port port(this, 
    Graph::Port::INPUT, name, attribute);
  _ports.push_back(port);
 
}

// Node add output
//------------------------------------------------------------------------------
void 
Graph::Node::AddOutput(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  Graph::Port port(this, 
    Graph::Port::OUTPUT, name, attribute);
  _ports.push_back(port);
}

// Node add io port
//------------------------------------------------------------------------------
void
Graph::Node::AddPort(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  Graph::Port port(this, 
    Graph::Port::Flag(Graph::Port::INPUT | Graph::Port::OUTPUT),
    name, attribute);
  _ports.push_back(port);
}


// Node update
//------------------------------------------------------------------------------
void
Graph::Node::Update()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  std::cout << "update from usd " << _prim.GetPath() << std::endl;
  pxr::UsdAttribute posAttr = api.GetPosAttr();
  if (!posAttr.IsValid()) {
    posAttr = api.CreatePosAttr(pxr::VtValue(_pos));
  }
  posAttr.Get(&_pos);

  pxr::UsdAttribute sizeAttr = api.GetSizeAttr();
  if (!sizeAttr.IsValid()) {
    sizeAttr = api.CreateSizeAttr(pxr::VtValue(_size));
  }
  sizeAttr.Get(&_size);

  pxr::UsdAttribute expendedAttr = api.GetExpansionStateAttr();
  if (!expendedAttr.IsValid()) {
    expendedAttr = api.CreateExpansionStateAttr(pxr::VtValue(pxr::UsdUITokens->closed));
  }
  expendedAttr.Get(&_expended);

  pxr::UsdAttribute colorAttr = api.GetDisplayColorAttr();
  if (!colorAttr.IsValid()) {
    colorAttr = api.CreateDisplayColorAttr(pxr::VtValue(DEFAULT_NODE_COLOR));
  }
  colorAttr.Get(&_color);

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

void 
Graph::Node::SetPosition(const pxr::GfVec2f& pos) 
{ 
  _pos = pos; 
  _dirty |= DIRTY_POSITION; 
};
      
void 
Graph::Node::SetSize(const pxr::GfVec2f& size)
{ 
  _size = size; 
  _dirty |= DIRTY_SIZE; 
};

void 
Graph::Node::SetExpended(const pxr::TfToken& expended) 
{ 
  _expended = expended; 
  _dirty |= DIRTY_SIZE;
};

void
Graph::Node::SetColor(const pxr::GfVec3f& color)
{
  _color = color;
  _dirty |= DIRTY_COLOR;
};

// Port
//------------------------------------------------------------------------------
Graph::Port::Port(Graph::Node* node, Graph::Port::Flag flags, 
  const pxr::TfToken& label, pxr::UsdAttribute& attr)
  : _node(node), _flags(flags), _label(label), _attr(attr)
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


// Graph destructor
//------------------------------------------------------------------------------
Graph::~Graph()
{
  for(auto& connexion: _connexions)delete connexion;
  for(auto& node: _nodes)delete node;
  _connexions.clear();
  _nodes.clear();
}

// Graph populate
//------------------------------------------------------------------------------
void Graph::Populate(pxr::UsdPrim& prim)
{
  if (!prim.HasAPI<pxr::UsdUISceneGraphPrimAPI>()) {
    if (pxr::UsdUISceneGraphPrimAPI::CanApply(prim)) {
      pxr::UsdUISceneGraphPrimAPI::Apply(prim);
    } else {
      std::cout <<
        "Invalid prim for applying UsdUINodeGraphNodeAPI : " <<
        prim.GetPath().GetText() << "." << std::endl;
      return;
    }
  }
  _prim = prim;
  _nodes.clear();
  _connexions.clear();
  _DiscoverNodes();
  _DiscoverConnexions();
}

// Graph clear
//------------------------------------------------------------------------------
void Graph::Clear()
{
  _prim = pxr::UsdPrim();
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