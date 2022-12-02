#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdExec/execConnectableAPI.h>
#include <pxr/usd/usdExec/execNode.h>
#include <pxr/usd/usdExec/execGraph.h>
#include "../graph/graph.h"
#include "../graph/execution.h"
#include "../command/block.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

ExecutionGraph* TestUsdExecAPI()
{
  UndoBlock editBlock;
  pxr::UsdStageRefPtr stage = GetApplication()->GetWorkStage();

  const pxr::SdfPath GRAPH_PATH("/graph");
  const pxr::TfToken GET("get");
  const pxr::TfToken MUL("multiply");
  const pxr::TfToken SET("set");

  pxr::UsdExecGraph graph = pxr::UsdExecGraph::Define(stage, GRAPH_PATH);

  pxr::UsdExecNode get = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(GET));
  pxr::UsdUINodeGraphNodeAPI api(get);
  api.CreatePosAttr().Set(pxr::GfVec2f(0, 0));
  api.CreateDisplayColorAttr().Set(pxr::GfVec3f(0.5));
  pxr::UsdExecInput inPrim = get.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  inPrim.SetConnectability(UsdExecTokens->interfaceOnly);
  pxr::UsdExecInput inAttr = get.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  inAttr.SetConnectability(UsdExecTokens->interfaceOnly);
  pxr::UsdExecOutput inVal = get.CreateOutput(pxr::TfToken("Value"), pxr::SdfValueTypeNames->Vector3f);
  inVal.Set(pxr::VtValue(pxr::GfVec3f(0.f)));

  pxr::UsdExecNode mul = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(MUL));
  api = pxr::UsdUINodeGraphNodeAPI(mul);
  api.CreatePosAttr().Set(pxr::GfVec2f(120, 0));
  api.CreateDisplayColorAttr().Set(pxr::GfVec3f(0.5));
  pxr::UsdExecInput factor = mul.CreateInput(pxr::TfToken("Factor"), pxr::SdfValueTypeNames->Float);
  factor.Set(pxr::VtValue(1.f));
  pxr::UsdExecInput input = mul.CreateInput(pxr::TfToken("Input"), pxr::SdfValueTypeNames->Vector3f);
  pxr::UsdExecOutput output = mul.CreateOutput(pxr::TfToken("Output"), pxr::SdfValueTypeNames->Vector3f);

  pxr::UsdExecNode set = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(SET));
  api = pxr::UsdUINodeGraphNodeAPI(set);
  api.CreatePosAttr().Set(pxr::GfVec2f(240, 0));
  api.CreateDisplayColorAttr().Set(pxr::GfVec3f(0.5));
  pxr::UsdExecInput outPrim = set.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  outPrim.SetConnectability(UsdExecTokens->interfaceOnly);
  pxr::UsdExecInput outAttr = set.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  outAttr.SetConnectability(UsdExecTokens->interfaceOnly);
  pxr::UsdExecInput outVal0 = set.CreateInput(pxr::TfToken("Value0"), pxr::SdfValueTypeNames->Vector3f);
  outVal0.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal1 = set.CreateInput(pxr::TfToken("Value1"), pxr::SdfValueTypeNames->Vector3f);
  outVal1.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal2 = set.CreateInput(pxr::TfToken("Value2"), pxr::SdfValueTypeNames->Vector3f);
  outVal2.Set(pxr::VtValue(pxr::GfVec3f(0.f)));


  input.ConnectToSource(inVal);
  outVal0.ConnectToSource(output);

  stage->SetDefaultPrim(graph.GetPrim());
  return new ExecutionGraph(graph.GetPrim());
}

pxr::SdfValueTypeName 
_GetRuntimeTypeName(pxr::SdfValueTypeName vtn)
{
  if (vtn == pxr::SdfValueTypeNames->Bool ||
    vtn == pxr::SdfValueTypeNames->BoolArray) return pxr::SdfValueTypeNames->BoolArray;
  else if (vtn == pxr::SdfValueTypeNames->Int ||
    vtn == pxr::SdfValueTypeNames->IntArray) return pxr::SdfValueTypeNames->IntArray;
  else if (vtn == pxr::SdfValueTypeNames->UChar) return pxr::SdfValueTypeNames->UCharArray;
  else if (vtn == pxr::SdfValueTypeNames->Float ||
    vtn == pxr::SdfValueTypeNames->FloatArray ||
    vtn == pxr::SdfValueTypeNames->Double ||
    vtn == pxr::SdfValueTypeNames->DoubleArray) return pxr::SdfValueTypeNames->FloatArray;
  else if (vtn == pxr::SdfValueTypeNames->Float2 ||
    vtn == pxr::SdfValueTypeNames->Float2Array) return pxr::SdfValueTypeNames->Float2Array;
  else if (vtn == pxr::SdfValueTypeNames->Float3 ||
    vtn == pxr::SdfValueTypeNames->Vector3f ||
    vtn == pxr::SdfValueTypeNames->Float3Array ||
    vtn == pxr::SdfValueTypeNames->Vector3fArray ||
    vtn == pxr::SdfValueTypeNames->Point3f ||
    vtn == pxr::SdfValueTypeNames->Point3fArray ||
    vtn == pxr::SdfValueTypeNames->Normal3f ||
    vtn == pxr::SdfValueTypeNames->Normal3fArray ||
    vtn == pxr::SdfValueTypeNames->Vector3d ||
    vtn == pxr::SdfValueTypeNames->Double3Array ||
    vtn == pxr::SdfValueTypeNames->Vector3dArray ||
    vtn == pxr::SdfValueTypeNames->Point3d ||
    vtn == pxr::SdfValueTypeNames->Point3dArray ||
    vtn == pxr::SdfValueTypeNames->Normal3d ||
    vtn == pxr::SdfValueTypeNames->Normal3dArray) return pxr::SdfValueTypeNames->Float3Array;
  else if (vtn == pxr::SdfValueTypeNames->Float4 ||
    vtn == pxr::SdfValueTypeNames->Float4Array) return pxr::SdfValueTypeNames->Float4Array;
  else if (vtn == pxr::SdfValueTypeNames->Color4f ||
    vtn == pxr::SdfValueTypeNames->Color4fArray) return pxr::SdfValueTypeNames->Color3fArray;
  else if (vtn == pxr::SdfValueTypeNames->Asset ||
    vtn == pxr::SdfValueTypeNames->AssetArray) return pxr::SdfValueTypeNames->AssetArray;
  else if (vtn == pxr::SdfValueTypeNames->Token ||
    vtn == pxr::SdfValueTypeNames->TokenArray) return pxr::SdfValueTypeNames->TokenArray;
  else
    return pxr::SdfValueTypeNames->Int;
}

// Graph constructor
//------------------------------------------------------------------------------
ExecutionGraph::ExecutionGraph(pxr::UsdPrim& prim) 
  : Graph()
{
    Populate(prim);
}

// Graph destructor
//------------------------------------------------------------------------------
ExecutionGraph::~ExecutionGraph()
{
}

ExecutionGraph::ExecutionNode::ExecutionNode(pxr::UsdPrim& prim)
  : Graph::Node(prim)
{
  _PopulatePorts();
}

void ExecutionGraph::ExecutionNode::_PopulatePorts()
{
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
}

void
ExecutionGraph::_DiscoverNodes() 
{
  if (!_prim.IsValid())return;

  for (pxr::UsdPrim child : _prim.GetChildren()) {
    ExecutionGraph::ExecutionNode* node = new ExecutionGraph::ExecutionNode(child);
    AddNode(node);
  }
}

void
ExecutionGraph::_DiscoverConnexions()
{
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
}

JVR_NAMESPACE_CLOSE_SCOPE