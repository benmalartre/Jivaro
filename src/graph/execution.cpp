#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
//#include <pxr/usd/usdExec/execConnectableAPI.h>
//#include <pxr/usd/usdExec/execNode.h>
//#include <pxr/usd/usdExec/execGraph.h>
#include "../graph/graph.h"
#include "../graph/execution.h"
#include "../command/block.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE


ExecutionGraph* TestUsdExecAPI()
{
  /*
  UndoBlock editBlock;
  UsdStageRefPtr stage = Application::Get()->GetWorkStage();

  const SdfPath GRAPH_PATH("/graph");
  const TfToken GET("get");
  const TfToken MUL("multiply");
  const TfToken SET("set");

  UsdExecGraph graph = UsdExecGraph::Define(stage, GRAPH_PATH);

  UsdExecNode get = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(GET));
  UsdUINodeGraphNodeAPI api(get);
  api.CreatePosAttr().Set(GfVec2f(0, 0));
  api.CreateDisplayColorAttr().Set(GfVec3f(0.5));
  UsdExecInput inPrim = get.CreateInput(TfToken("Primitive"), SdfValueTypeNames->Asset);
  inPrim.SetConnectability(UsdExecTokens->interfaceOnly);
  UsdExecInput inAttr = get.CreateInput(TfToken("Attribute"), SdfValueTypeNames->Token);
  inAttr.SetConnectability(UsdExecTokens->interfaceOnly);
  UsdExecOutput inVal = get.CreateOutput(TfToken("Value"), SdfValueTypeNames->Vector3f);
  inVal.Set(VtValue(GfVec3f(0.f)));

  UsdExecNode mul = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(MUL));
  api = UsdUINodeGraphNodeAPI(mul);
  api.CreatePosAttr().Set(GfVec2f(120, 0));
  api.CreateDisplayColorAttr().Set(GfVec3f(0.5));
  UsdExecInput factor = mul.CreateInput(TfToken("Factor"), SdfValueTypeNames->Float);
  factor.Set(VtValue(1.f));
  UsdExecInput input = mul.CreateInput(TfToken("Input"), SdfValueTypeNames->Vector3f);
  UsdExecOutput output = mul.CreateOutput(TfToken("Output"), SdfValueTypeNames->Vector3f);

  UsdExecNode set = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(SET));
  api = UsdUINodeGraphNodeAPI(set);
  api.CreatePosAttr().Set(GfVec2f(240, 0));
  api.CreateDisplayColorAttr().Set(GfVec3f(0.5));
  UsdExecInput outPrim = set.CreateInput(TfToken("Primitive"), SdfValueTypeNames->Asset);
  outPrim.SetConnectability(UsdExecTokens->interfaceOnly);
  UsdExecInput outAttr = set.CreateInput(TfToken("Attribute"), SdfValueTypeNames->Token);
  outAttr.SetConnectability(UsdExecTokens->interfaceOnly);
  UsdExecInput outVal0 = set.CreateInput(TfToken("Value0"), SdfValueTypeNames->Vector3f);
  outVal0.Set(VtValue(GfVec3f(0.f)));
  UsdExecInput outVal1 = set.CreateInput(TfToken("Value1"), SdfValueTypeNames->Vector3f);
  outVal1.Set(VtValue(GfVec3f(0.f)));
  UsdExecInput outVal2 = set.CreateInput(TfToken("Value2"), SdfValueTypeNames->Vector3f);
  outVal2.Set(VtValue(GfVec3f(0.f)));


  input.ConnectToSource(inVal);
  outVal0.ConnectToSource(output);

  stage->SetDefaultPrim(graph.GetPrim());
  return new ExecutionGraph(graph.GetPrim());
  */
  return nullptr;
}


SdfValueTypeName 
_GetRuntimeTypeName(SdfValueTypeName vtn)
{
  if (vtn == SdfValueTypeNames->Bool ||
    vtn == SdfValueTypeNames->BoolArray) return SdfValueTypeNames->BoolArray;
  else if (vtn == SdfValueTypeNames->Int ||
    vtn == SdfValueTypeNames->IntArray) return SdfValueTypeNames->IntArray;
  else if (vtn == SdfValueTypeNames->UChar) return SdfValueTypeNames->UCharArray;
  else if (vtn == SdfValueTypeNames->Float ||
    vtn == SdfValueTypeNames->FloatArray ||
    vtn == SdfValueTypeNames->Double ||
    vtn == SdfValueTypeNames->DoubleArray) return SdfValueTypeNames->FloatArray;
  else if (vtn == SdfValueTypeNames->Float2 ||
    vtn == SdfValueTypeNames->Float2Array) return SdfValueTypeNames->Float2Array;
  else if (vtn == SdfValueTypeNames->Float3 ||
    vtn == SdfValueTypeNames->Vector3f ||
    vtn == SdfValueTypeNames->Float3Array ||
    vtn == SdfValueTypeNames->Vector3fArray ||
    vtn == SdfValueTypeNames->Point3f ||
    vtn == SdfValueTypeNames->Point3fArray ||
    vtn == SdfValueTypeNames->Normal3f ||
    vtn == SdfValueTypeNames->Normal3fArray ||
    vtn == SdfValueTypeNames->Vector3d ||
    vtn == SdfValueTypeNames->Double3Array ||
    vtn == SdfValueTypeNames->Vector3dArray ||
    vtn == SdfValueTypeNames->Point3d ||
    vtn == SdfValueTypeNames->Point3dArray ||
    vtn == SdfValueTypeNames->Normal3d ||
    vtn == SdfValueTypeNames->Normal3dArray) return SdfValueTypeNames->Float3Array;
  else if (vtn == SdfValueTypeNames->Float4 ||
    vtn == SdfValueTypeNames->Float4Array) return SdfValueTypeNames->Float4Array;
  else if (vtn == SdfValueTypeNames->Color4f ||
    vtn == SdfValueTypeNames->Color4fArray) return SdfValueTypeNames->Color3fArray;
  else if (vtn == SdfValueTypeNames->Asset ||
    vtn == SdfValueTypeNames->AssetArray) return SdfValueTypeNames->AssetArray;
  else if (vtn == SdfValueTypeNames->Token ||
    vtn == SdfValueTypeNames->TokenArray) return SdfValueTypeNames->TokenArray;
  else
    return SdfValueTypeNames->Int;
}

// Graph constructor
//------------------------------------------------------------------------------
ExecutionGraph::ExecutionGraph(const UsdPrim& prim) 
  : Graph(prim)
{
  Populate(prim);
}

// Graph destructor
//------------------------------------------------------------------------------
ExecutionGraph::~ExecutionGraph()
{
}

ExecutionGraph::ExecutionNode::ExecutionNode(UsdPrim& prim)
  : Graph::Node(prim)
{
  _PopulatePorts();
}

void ExecutionGraph::ExecutionNode::_PopulatePorts()
{
  /*
  if (_prim.IsA<UsdExecGraph>()) {
    UsdExecGraph graph(_prim);
    for (const auto& input : graph.GetInputs()) {
      UsdAttribute attr = input.GetAttr();
      AddInput(attr, input.GetBaseName());
    }
    for (const auto& output : graph.GetOutputs()) {
      UsdAttribute attr = output.GetAttr();
      AddOutput(attr, output.GetBaseName());
    }
  }
  else if (_prim.IsA<UsdExecNode>()) {
    UsdExecNode node(_prim);
    for (const auto& input : node.GetInputs()) {
      UsdAttribute attr = input.GetAttr();
      AddInput(attr, input.GetBaseName());
    }
    for (const auto& output : node.GetOutputs()) {
      UsdAttribute attr = output.GetAttr();
      AddOutput(attr, output.GetBaseName());
    }
  }
  */
}

void
ExecutionGraph::_DiscoverNodes() 
{
  if (!_prim.IsValid())return;

  for (UsdPrim child : _prim.GetChildren()) {
    ExecutionGraph::ExecutionNode* node = new ExecutionGraph::ExecutionNode(child);
    AddNode(node);
  }
}

void
ExecutionGraph::_DiscoverConnexions()
{
  for (UsdPrim child : _prim.GetChildren()) {
    /*
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
    */
  }
}

JVR_NAMESPACE_CLOSE_SCOPE