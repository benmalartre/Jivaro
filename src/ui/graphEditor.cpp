#include <iostream>
#include <string>
#include <pxr/pxr.h>
#include <pxr/base/tf/type.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/ndr/property.h>
#include <pxr/usd/ndr/node.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usdUI/backdrop.h>
#include <pxr/usd/usdExec/execConnectableAPI.h>
#include <pxr/usd/usdExec/execNode.h>
#include <pxr/usd/usdExec/execGraph.h>


#include "../utils/color.h"
#include "../utils/keys.h"
#include "../ui/graphEditor.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../command/block.h"
#include "../command/command.h"



JVR_NAMESPACE_OPEN_SCOPE

bool 
_ConnexionPossible(const pxr::SdfValueTypeName& lhs, const pxr::SdfValueTypeName& rhs)
{
  if (lhs.GetDimensions() == rhs.GetDimensions())return true;
  return false;
}

static
const pxr::TfToken& _ConvertExpendedStateToToken(short state)
{
  switch (state) {
  case GraphEditorUI::ExpendedState::COLLAPSED:
    return pxr::UsdUITokens->closed;
  case GraphEditorUI::ExpendedState::CONNECTED:
    return pxr::UsdUITokens->minimized;
  case GraphEditorUI::ExpendedState::EXPENDED:
    return pxr::UsdUITokens->open;
  default:
    return pxr::UsdUITokens->open;
  }
}

static
const short _ConvertExpendedStateToEnum(const pxr::TfToken& state)
{
  if (state == pxr::UsdUITokens->closed)
    return GraphEditorUI::ExpendedState::COLLAPSED;
  else if (state == pxr::UsdUITokens->minimized)
    return GraphEditorUI::ExpendedState::CONNECTED;
  else
    return GraphEditorUI::ExpendedState::EXPENDED;
}

bool
GraphEditorUI::_ConnexionPossible(const GraphEditorUI::Port* lhs, const GraphEditorUI::Port* rhs)
{
  const GraphEditorUI::Node* lhsNode = lhs->GetNode();
  const pxr::UsdPrim& lhsPrim = lhsNode->GetPrim();
  const GraphEditorUI::Node* rhsNode = rhs->GetNode();
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
  }
  else if (lhsPrim.IsA<pxr::UsdExecNode>()) {
    pxr::UsdExecNode lhsExec(lhsPrim);
    pxr::UsdExecOutput output = lhsExec.GetOutput(lhs->GetName());

    pxr::UsdExecNode rhsExec(rhsPrim);
    pxr::UsdExecInput input = rhsExec.GetInput(rhs->GetName());

    return pxr::UsdExecConnectableAPI::CanConnect(output, input);
  }

  return false;
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

int
_GetColorFromAttribute(const pxr::UsdAttribute& attr)
{
  /*
  enum ColorGraph {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFFFF6600,
  GRAPH_COLOR_INTEGER           = 0xFF336611,
  GRAPH_COLOR_ENUM              = 0xFF339911,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_VECTOR2           = 0xFFFFCC00,
  GRAPH_COLOR_VECTOR3           = 0xFFFFFF00,
  GRAPH_COLOR_VECTOR4           = 0xFFFFFF66,
  GRAPH_COLOR_COLOR             = 0xFFFF0000,
  GRAPH_COLOR_ROTATION          = 0xFFCCFFFF,
  GRAPH_COLOR_QUATERNION        = 0xFF66FFFF,
  GRAPH_COLOR_MATRIX3           = 0xFF00FFFF,
  GRAPH_COLOR_MATRIX4           = 0xFF33CCFF,
  GRAPH_COLOR_STRING            = 0xFFCC99FF,
  GRAPH_COLOR_SHAPE             = 0xFFFF3399,
  GRAPH_COLOR_TOPOLOGY          = 0xFFCCCCCC,
  GRAPH_COLOR_GEOMETRY          = 0xFFFF3366,
  GRAPH_COLOR_LOCATION          = 0xFF555577,
  GRAPH_COLOR_CONTOUR           = 0xFF000000
  */
  pxr::SdfValueTypeName vtn = attr.GetTypeName();
  if (vtn == pxr::SdfValueTypeNames->Bool || 
    vtn == pxr::SdfValueTypeNames->BoolArray) return GRAPH_COLOR_BOOL;
  else if (vtn == pxr::SdfValueTypeNames->Int ||
    vtn == pxr::SdfValueTypeNames->IntArray) return GRAPH_COLOR_INTEGER;
  else if (vtn == pxr::SdfValueTypeNames->UChar ) return GRAPH_COLOR_ENUM;
  else if (vtn == pxr::SdfValueTypeNames->Float ||
    vtn == pxr::SdfValueTypeNames->FloatArray ||
    vtn == pxr::SdfValueTypeNames->Double ||
    vtn == pxr::SdfValueTypeNames->FloatArray) return GRAPH_COLOR_FLOAT;
  else if (vtn == pxr::SdfValueTypeNames->Float2 ||
    vtn == pxr::SdfValueTypeNames->Float2Array) return GRAPH_COLOR_VECTOR2;
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
    vtn == pxr::SdfValueTypeNames->Normal3dArray) return GRAPH_COLOR_VECTOR3;
  else if (vtn == pxr::SdfValueTypeNames->Float4 ||
    vtn == pxr::SdfValueTypeNames->Float4Array) return GRAPH_COLOR_VECTOR4;
  else if (vtn == pxr::SdfValueTypeNames->Color4f ||
    vtn == pxr::SdfValueTypeNames->Color4fArray) return GRAPH_COLOR_COLOR;
  else if (vtn == pxr::SdfValueTypeNames->Asset ||
    vtn == pxr::SdfValueTypeNames->AssetArray) return GRAPH_COLOR_PRIM;
  else return GRAPH_COLOR_UNDEFINED;
}

static ImColor
_GetHoveredColor(int color)
{
  pxr::GfVec4f origin = UnpackColor4<pxr::GfVec4f>(color);
  return ImColor(
    (origin[0] + 1.f) * 0.5f,
    (origin[1] + 1.f) * 0.5f,
    (origin[2] + 1.f) * 0.5f,
    (origin[3] + 1.f) * 0.5f
  );
}

static pxr::UsdPrim TestUsdShadeAPI()
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
  pxr::UsdExecInput inPrim = get.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  inPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput inAttr = get.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  inAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecOutput inVal = get.CreateOutput(pxr::TfToken("Value"), pxr::SdfValueTypeNames->Vector3f);
  inVal.Set(pxr::VtValue(pxr::GfVec3f(0.f)));

  pxr::UsdExecNode mul = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(MUL));
  api = pxr::UsdUINodeGraphNodeAPI(mul);
  api.CreatePosAttr().Set(pxr::GfVec2f(120, 0));
  pxr::UsdExecInput factor = mul.CreateInput(pxr::TfToken("Factor"), pxr::SdfValueTypeNames->Float);
  factor.Set(pxr::VtValue(1.f));
  pxr::UsdExecInput input = mul.CreateInput(pxr::TfToken("Input"), pxr::SdfValueTypeNames->Vector3f);
  pxr::UsdExecOutput output = mul.CreateOutput(pxr::TfToken("Output"), pxr::SdfValueTypeNames->Vector3f);

  pxr::UsdExecNode set = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(SET));
  api = pxr::UsdUINodeGraphNodeAPI(set);
  api.CreatePosAttr().Set(pxr::GfVec2f(240, 0));
  pxr::UsdExecInput outPrim = set.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  outPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput outAttr = set.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  outAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput outVal0 = set.CreateInput(pxr::TfToken("Value0"), pxr::SdfValueTypeNames->Vector3f);
  outVal0.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal1 = set.CreateInput(pxr::TfToken("Value1"), pxr::SdfValueTypeNames->Vector3f);
  outVal1.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal2 = set.CreateInput(pxr::TfToken("Value2"), pxr::SdfValueTypeNames->Vector3f);
  outVal2.Set(pxr::VtValue(pxr::GfVec3f(0.f)));


  input.ConnectToSource(inVal);
  outVal0.ConnectToSource                                                                             (output);

  stage->SetDefaultPrim(graph.GetPrim());

  return graph.GetPrim();
}

// Item base class
//------------------------------------------------------------------------------
GraphEditorUI::Item::Item()
  : _pos(0), _size(0), _color(0), _state(0)
{
}

GraphEditorUI::Item::Item(const pxr::GfVec2f& pos, const pxr::GfVec2f& size, int color)
  : _pos(pos), _size(size), _color(color), _state(0)
{
}

GraphEditorUI::Item::Item(int color)
  : _pos(0), _size(0), _color(color), _state(0)
{
}

void
GraphEditorUI::Item::SetState(size_t flag, bool value)
{
  if (value) _state |= flag;
  else _state &= ~flag;
}

bool 
GraphEditorUI::Item::GetState(size_t flag)
{
  return _state & flag;
}

void 
GraphEditorUI::Item::SetPosition(const pxr::GfVec2f& pos)
{
  _pos = pos;
}

void
GraphEditorUI::Item::SetSize(const pxr::GfVec2f& size)
{
  _size = size;
}

void 
GraphEditorUI::Item::SetColor(const pxr::GfVec3f& color)
{
  _color = ImColor(
    int(color[0] * 255),
    int(color[1] * 255),
    int(color[2] * 255),
    255);
}

void 
GraphEditorUI::Item::SetColor(int color)
{
  _color = color;
}

bool 
GraphEditorUI::Item::Contains(const pxr::GfVec2f& pos, const pxr::GfVec2f& extend)
{
  if (pos[0] >= _pos[0] - extend[0] &&
    pos[0] <= _pos[0] + _size[0] + extend[0] &&
    pos[1] >= _pos[1] - extend[1] &&
    pos[1] <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

bool 
GraphEditorUI::Item::Intersect(const pxr::GfVec2f& start, const pxr::GfVec2f& end)
{
  pxr::GfRange2f marqueBox(start, end);
  pxr::GfRange2f nodeBox(_pos, _pos + _size);
  if (!nodeBox.IsOutside(marqueBox)) return true;
  else return false;
}

// Port
//------------------------------------------------------------------------------
GraphEditorUI::Port::Port(GraphEditorUI::Node* node, GraphEditorUI::Port::Flag flags, 
  const TfToken& label, pxr::UsdAttribute& attr)
  : GraphEditorUI::Item(_GetColorFromAttribute(attr))
  , _attr(attr)
{
  _node = node;
  _label = label;
  _flags = flags;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
}

/*
GraphEditorUI::Port::Port(GraphEditorUI::Node* node, const pxr::UsdShadeInput& port)
  : GraphEditorUI::Item(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = true;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
  _attr = port.GetAttr();
}

GraphEditorUI::Port::Port(GraphEditorUI::Node* node, const pxr::UsdShadeOutput& port)
  : GraphEditorUI::Item(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = false;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
  _attr = port.GetAttr();
}
*/

bool 
GraphEditorUI::Port::IsConnected(GraphEditorUI* editor, GraphEditorUI::Connexion* foundConnexion)
{
  GraphEditorUI::Graph* graph = editor->GetGraph();
  for (auto& connexion : graph->GetConnexions()) {
    if (connexion->GetStart() == this || connexion->GetEnd() == this) {
      foundConnexion = connexion;
      return true;
    }
  }
  return false;
}

bool 
GraphEditorUI::Port::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  if (position[0] + NODE_PORT_RADIUS >= _pos[0] - extend[0] &&
    position[0] + NODE_PORT_RADIUS <= _pos[0] + _size[0] + extend[0] &&
    position[1] + NODE_PORT_RADIUS >= _pos[1] - extend[1] &&
    position[1] + NODE_PORT_RADIUS <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

pxr::SdfPath
GraphEditorUI::Port::GetPath()
{
  return _node->GetPrim().GetPath().AppendProperty(GetName());
}

void
GraphEditorUI::Port::Draw(GraphEditorUI* editor)
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  ImVec2 portTextOffset;
  const pxr::GfVec2f offset = editor->GetOffset();
  const float scale = editor->GetScale();
  const pxr::GfVec2f p = 
    editor->GridPositionToViewPosition(_node->GetPosition());
  
  static const ImVec2 inputPortOffset(
    NODE_PORT_RADIUS * scale, NODE_PORT_RADIUS * scale);
  static const ImVec2 outputPortOffset(
    - NODE_PORT_RADIUS * scale, NODE_PORT_RADIUS * scale);

  portTextOffset = ImGui::CalcTextSize(_label.GetText());
  portTextOffset.x = -NODE_PORT_RADIUS * 2.f * scale;
  portTextOffset.y -= NODE_PORT_VERTICAL_SPACING * 0.5f * scale;

  drawList->AddText(
    p + (_pos * scale) - portTextOffset,
    ImColor(0, 0, 0, 255),
    _label.GetText());

  if (_flags & GraphEditorUI::Port::INPUT) {
    drawList->AddCircleFilled(
      p + _pos * scale,
      GetState(ITEM_STATE_HOVERED) ? NODE_PORT_RADIUS * scale * 1.2f : NODE_PORT_RADIUS * scale,
      _color
    );
  }

  if (_flags & GraphEditorUI::Port::OUTPUT) {
    drawList->AddCircleFilled(
      p + (_pos + pxr::GfVec2f(_node->GetWidth(), 0.f)) * scale,
      GetState(ITEM_STATE_HOVERED) ? NODE_PORT_RADIUS * scale * 1.2f : NODE_PORT_RADIUS * scale,
      _color
    );
  }
}

// Connexion description
//------------------------------------------------------------------------------
GraphEditorUI::ConnexionData 
GraphEditorUI::Connexion::GetDescription()
{
  GraphEditorUI::ConnexionData datas;
  datas.p0 = _start->GetPosition() + pxr::GfVec2f(_start->GetNode()->GetWidth(), 0.f) + _start->GetNode()->GetPosition();
  datas.p3 = _end->GetPosition() + _end->GetNode()->GetPosition();

  const float length = (datas.p3 - datas.p0).GetLength();
  const pxr::GfVec2f offset(0.25f * length, 0.f);

  datas.p1 = datas.p0 + offset;
  datas.p2 = datas.p3 - offset;
  datas.numSegments =
    pxr::GfMax(static_cast<int>(length * NODE_CONNEXION_RESOLUTION), 1);
  return datas;
}

pxr::GfRange2f 
GraphEditorUI::Connexion::GetBoundingBox()
{
  pxr::GfVec2f minBox = _start->GetPosition() + _start->GetNode()->GetPosition();
  pxr::GfVec2f maxBox = _end->GetPosition() + _end->GetNode()->GetPosition();
  if (minBox[0] > maxBox[0])std::swap(minBox[0], maxBox[0]);
  if (minBox[1] > maxBox[1])std::swap(minBox[1], maxBox[1]);

  return pxr::GfRange2f(minBox, maxBox);
}

bool 
GraphEditorUI::Connexion::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  const GraphEditorUI::ConnexionData datas = GetDescription();
  pxr::GfVec2f closest = ImBezierCubicClosestPoint(
      datas.p0, datas.p1, datas.p2, datas.p3, position, 32);
  return (position - closest).GetLength() < extend[0];
}

bool 
GraphEditorUI::Connexion::Intersect(const pxr::GfVec2f& start,
  const pxr::GfVec2f& end)
{
  return false;
}

// Connexion draw
//------------------------------------------------------------------------------
void 
GraphEditorUI::Connexion::Draw(GraphEditorUI* graph)
{
  const GraphEditorUI::ConnexionData datas = GetDescription();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const float scale = graph->GetScale();
  if (GetState(ITEM_STATE_HOVERED)) {
    drawList->AddBezierCubic(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      _GetHoveredColor(_color),
      NODE_CONNEXION_THICKNESS * scale * 1.4,
      32);
  }

  else if(GetState(ITEM_STATE_SELECTED)) 
    drawList->AddBezierCubic(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      ImColor(255,255,255,255),
      NODE_CONNEXION_THICKNESS * graph->GetScale(),
      32);

  else
    drawList->AddBezierCubic(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      _color,
      NODE_CONNEXION_THICKNESS * graph->GetScale(),
      32);
}


// Node constructor
//------------------------------------------------------------------------------
GraphEditorUI::Node::Node(pxr::UsdPrim prim, bool write)
  : GraphEditorUI::Item()
  , _prim(prim)
  , _expended(COLLAPSED)
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
        pxr::TF_CODING_ERROR(
          "Invalid prim for applying UsdUINodeGepahNodeAPI : %s.", 
          prim.GetPath().GetText());
        return;
      }
    }
  }
  
  if (prim.IsA<pxr::UsdGeomMesh>()) {
    pxr::UsdGeomMesh mesh(prim);
    for (const auto& attrName : mesh.GetSchemaAttributeNames()) {
      pxr::UsdAttribute attr = mesh.GetPrim().GetAttribute(attrName);
      AddPort(attr, attrName);
    }
  }
  else if (prim.IsA<pxr::UsdShadeNodeGraph>()) {
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

  pxr::UsdUINodeGraphNodeAPI api(prim);
  api.GetPosAttr().Get(&_pos);
  pxr::TfToken expended;

  api.GetExpansionStateAttr().Get(&expended);
  _expended = _ConvertExpendedStateToEnum(expended);
}

// NodeUI destructor
//------------------------------------------------------------------------------
GraphEditorUI::Node::~Node()
{

}



void 
GraphEditorUI::Node::Init()
{
  /*
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.GetPosAttr();
  if (posAttr && posAttr.HasAuthoredValue())
  {
    posAttr.Get(&_pos);
  }

  pxr::UsdAttribute sizeAttr = api.GetSizeAttr();
  if (sizeAttr && sizeAttr.HasAuthoredValue())
  {
    sizeAttr.Get(&_size);
  }
  else
  {
    _size[0] = 128;
    _size[1] = 64;
  }

  _size[1] += _ports.size() * NODE_PORT_VERTICAL_SPACING;
  */

}

void 
GraphEditorUI::Node::AddInput(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  GraphEditorUI::Port port(this, 
    GraphEditorUI::Port::INPUT, name, attribute);
  _ports.push_back(port);
 
}

void 
GraphEditorUI::Node::AddOutput(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  GraphEditorUI::Port port(this, 
    GraphEditorUI::Port::OUTPUT, name, attribute);
  _ports.push_back(port);
}

void
GraphEditorUI::Node::AddPort(pxr::UsdAttribute& attribute, const pxr::TfToken& name)
{
  GraphEditorUI::Port port(this, 
    GraphEditorUI::Port::Flag(GraphEditorUI::Port::INPUT | GraphEditorUI::Port::OUTPUT),
    name, attribute);
  _ports.push_back(port);
}

GraphEditorUI::Port* 
GraphEditorUI::Node::GetPort(const pxr::TfToken& name)
{
  for (auto& port : _ports) {
    if (port.GetName() == name) return &port;
  }
  return NULL;
}

void 
GraphEditorUI::Node::SetPosition(const pxr::GfVec2f& pos)
{
  _pos = pos;
  /*
  UndoBlock editBlock;
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  api.CreatePosAttr().Set(pxr::VtValue(pos));
  */
}

void 
GraphEditorUI::Node::SetSize(const pxr::GfVec2f& size)
{
  _size = size;
  /*
  UndoBlock editBlock;
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  api.CreatePosAttr().Set(pxr::VtValue(size));
  */
}

void 
GraphEditorUI::Node::SetColor(const pxr::GfVec3f& color)
{
  _color = PackColor3<pxr::GfVec3f>(color);
  /*
  UndoBlock editBlock;
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  api.CreatePosAttr().Set(pxr::VtValue(color));
  */
}

void 
GraphEditorUI::Node::ComputeSize(GraphEditorUI* editor)
{
  if (_dirty & GraphEditorUI::Node::DIRTY_SIZE) {
    float width =
      ImGui::CalcTextSize(_name.GetText()).x + 2 * NODE_PORT_HORIZONTAL_SPACING +
      (NODE_EXPENDED_SIZE + 2 * NODE_HEADER_PADDING);
    float height = NODE_HEADER_HEIGHT + NODE_HEADER_PADDING;
    float inputWidth = 0, outputWidth = 0;
    GraphEditorUI::Connexion* connexion = NULL;
    for (auto& port : _ports) {
      float w = ImGui::CalcTextSize(port.GetName().GetText()).x +
        NODE_PORT_HORIZONTAL_SPACING;
      if (w > inputWidth)inputWidth = w;
      switch (_expended) {
      case COLLAPSED:
        break;
      case CONNECTED:
        if (port.IsConnected(editor, connexion)) {
          height += NODE_PORT_VERTICAL_SPACING;
        }
        break;
      case EXPENDED:
        height += NODE_PORT_VERTICAL_SPACING;
        break;
      }
    }

    float headerMid = NODE_HEADER_HEIGHT * 0.5;
    switch (_expended) {
    case COLLAPSED:
    {
      float currentY = headerMid;
      for (auto& port : _ports) {
        port.SetPosition(pxr::GfVec2f(0.f, currentY));
      }
      break;
    }
    case CONNECTED:
    {
      float currentY = NODE_HEADER_HEIGHT + NODE_HEADER_PADDING;
      for (auto& port : _ports) {
        if (port.IsConnected(editor, connexion)) {
          port.SetPosition(pxr::GfVec2f(0.f, currentY));
          currentY += NODE_PORT_VERTICAL_SPACING;
        }
        else {
          port.SetPosition(pxr::GfVec2f(0.f, headerMid));
        }
      }
      break;
    }
    case EXPENDED:
    {
      float currentY = NODE_HEADER_HEIGHT + NODE_HEADER_PADDING;
      for (auto& port : _ports) {
        port.SetPosition(pxr::GfVec2f(0.f, currentY));
        currentY += NODE_PORT_VERTICAL_SPACING;
      }
      break;
    }
    }

    SetSize(pxr::GfVec2f(width, height));
    _dirty = GraphEditorUI::Node::DIRTY_CLEAN;
  }
}

void
GraphEditorUI::Node::Update()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.GetPosAttr();
  pxr::GfVec2f pos;
  posAttr.Get(&pos);
  if (!pxr::GfIsClose(pos, _pos, 0.0000001)) {
    _dirty |= GraphEditorUI::Node::DIRTY_POSITION;
    _pos = pos;
  }

  pxr::UsdAttribute expendedAttr = api.GetExpansionStateAttr();
  pxr::TfToken expended;
  expendedAttr.Get(&expended);
  if (_ConvertExpendedStateToEnum(expended) != _expended) {
    _dirty |= GraphEditorUI::Node::DIRTY_SIZE;
    _expended = _ConvertExpendedStateToEnum(expended);
  }
}

bool 
GraphEditorUI::Node::IsVisible(GraphEditorUI* editor)
{
  return true;
}


// Node draw
//------------------------------------------------------------------------------
void 
GraphEditorUI::Node::Draw(GraphEditorUI* editor) 
{
  Window* window = editor->GetWindow();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  if (IsVisible(editor)) {
    ImGui::SetWindowFontScale(editor->GetFontScale());
    ImGui::PushFont(window->GetFont(editor->GetFontIndex()));

    const float scale = editor->GetScale();
    const pxr::GfVec2f offset = editor->GetOffset();
    const pxr::GfVec2f p = editor->GetPosition() + (GetPosition() + offset) * scale;
    const float x = p[0];
    const float y = p[1];
    const pxr::GfVec2f s = GetSize() * scale;
    const ImColor selectedColor = GetState(ITEM_STATE_SELECTED) ?
      ImColor(255, 255, 255, 255) :
      ImColor(0, 0, 0, 255);

    // body background
    drawList->AddRectFilled(
      ImVec2(x, y),
      ImVec2(x + s[0], y + s[1]),
      ImColor(_backgroundColor[0], _backgroundColor[1], _backgroundColor[2], 1.0),
      NODE_CORNER_ROUNDING * scale,
      ImDrawCornerFlags_All);
    
    // head background
    drawList->AddRectFilled(
      ImVec2(x, y),
      ImVec2(x + s[0], y + NODE_HEADER_HEIGHT * scale),
      ImColor(150, 160, 190, 255),
      NODE_CORNER_ROUNDING * scale,
      ImDrawCornerFlags_All);

    // border
    if (GetState(ITEM_STATE_SELECTED)) {
      drawList->AddRect(
        ImVec2(x, y),
        ImVec2(x + s[0], y + s[1]),
        selectedColor,
        NODE_CORNER_ROUNDING * scale,
        ImDrawCornerFlags_All,
        0.1 * scale);
    }

    drawList->AddText(p + pxr::GfVec2f(NODE_PORT_PADDING, 
      NODE_HEADER_PADDING) * scale, ImColor(0, 0, 0, 255), _name.GetText());
    ImGui::PopFont();

    // expended state
    const pxr::GfVec2f expendOffset((GetWidth() - (NODE_EXPENDED_SIZE + 2 * NODE_HEADER_PADDING)), NODE_HEADER_PADDING);
    const pxr::GfVec2f expendPos = p + expendOffset * scale;
    const pxr::GfVec2f expendSize(NODE_EXPENDED_SIZE * scale);
    const pxr::GfVec2f elementOffset(0.f, NODE_EXPENDED_SIZE * scale * 0.4);
    const pxr::GfVec2f elementSize(NODE_EXPENDED_SIZE * scale, NODE_EXPENDED_SIZE * scale * 0.3);
    const ImColor expendColor(0, 0, 0, 255);

    ImGui::SetCursorPos((GetPosition() + expendOffset + offset) * scale);

    static char expendedName[128];
    strcpy(expendedName, "##");
    strcat(expendedName, (const char*)this);

    if (ImGui::Selectable(&expendedName[0], true, ImGuiSelectableFlags_SelectOnClick, expendSize)) {
      short nextExpendedState = (_expended + 1) % 3;
      ADD_COMMAND(ExpendNodeCommand, 
        { GetPrim().GetPath() }, 
        _ConvertExpendedStateToToken(nextExpendedState));
      _expended = nextExpendedState;
    }

    drawList->AddRectFilled(
      expendPos + 2 * elementOffset,
      expendPos + 2 * elementOffset + elementSize,
      expendColor,
      0);

    if (_expended > COLLAPSED) {
      drawList->AddRectFilled(
        expendPos + elementOffset,
        expendPos + elementOffset + elementSize,
        expendColor,
        0);
    }

    if (_expended > CONNECTED) {
      drawList->AddRectFilled(
        expendPos,
        expendPos + elementSize,
        expendColor,
        0);
    }
    
    // ports
    switch (_expended) {
    case COLLAPSED:
      break;
    case CONNECTED:
      break;
    case EXPENDED:
      break;
    }

    GraphEditorUI::Connexion* connexion = NULL;
    if (_expended != COLLAPSED) {
      ImGui::PushFont(window->GetFont(editor->GetFontIndex()));
      int numPorts = _ports.size();
      for (int i = 0; i < numPorts; ++i) {
        if (_expended == EXPENDED) _ports[i].Draw(editor);
        else {
          if (_ports[i].IsConnected(editor, connexion)) _ports[i].Draw(editor);
        }
      }
     
      ImGui::PopFont();
    }
  }
} 

// Node constructor
//------------------------------------------------------------------------------
GraphEditorUI::Graph::Graph(pxr::UsdPrim& prim)
  : GraphEditorUI::Node(prim, false)
{
  if (prim.HasAPI<pxr::UsdUISceneGraphPrimAPI>()) {
    Populate(prim);
  } else {
    if (pxr::UsdUISceneGraphPrimAPI::CanApply(prim)) {
      pxr::UsdUISceneGraphPrimAPI::Apply(prim);
      Populate(prim);
    }
    else {
      pxr::TF_CODING_ERROR(
        "Invalid prim for applying UsdUINodeGraphNodeAPI : %s.", 
        prim.GetPath().GetText());
      return;
    }
  }
}

// Node destructor
//------------------------------------------------------------------------------
GraphEditorUI::Graph::~Graph()
{
  _nodes.clear();
}

void GraphEditorUI::Graph::Populate(pxr::UsdPrim& prim)
{
  _prim = prim;
  _nodes.clear();
  _connexions.clear();
  _DiscoverNodes(_prim);
  _DiscoverConnexions(_prim);
}

void GraphEditorUI::Graph::Clear()
{
  _prim = pxr::UsdPrim();
  _nodes.clear();
  _connexions.clear();
}

// Add node
//------------------------------------------------------------------------------
void 
GraphEditorUI::Graph::AddNode(GraphEditorUI::Node* node)
{
  _nodes.push_back(node);
}

// Remove node
//------------------------------------------------------------------------------
void 
GraphEditorUI::Graph::RemoveNode(GraphEditorUI::Node* node)
{
  
}

// Add connexion
//------------------------------------------------------------------------------
void
GraphEditorUI::Graph::AddConnexion(GraphEditorUI::Connexion* connexion)
{
  _connexions.push_back(connexion);
}

// Remove connexion
//------------------------------------------------------------------------------
void
GraphEditorUI::Graph::RemoveConnexion(GraphEditorUI::Connexion* connexion)
{

}


// Get node
//------------------------------------------------------------------------------
const GraphEditorUI::Node*
GraphEditorUI::Graph::GetNode(const pxr::UsdPrim& prim) const
{
  for (const GraphEditorUI::Node* node : _nodes) {
    if(node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

GraphEditorUI::Node* 
GraphEditorUI::Graph::GetNode(const pxr::UsdPrim& prim)
{
  for (GraphEditorUI::Node* node : _nodes) {
    if (node->GetPrim().GetPath() == prim.GetPath())return node;
  }
  return NULL;
}

void 
GraphEditorUI::Graph::_DiscoverNodes(pxr::UsdPrim& prim)
{
  for (pxr::UsdPrim child : prim.GetChildren()) {
    _RecurseNodes(child);
  }
}

void 
GraphEditorUI::Graph::_RecurseNodes(pxr::UsdPrim& prim)
{
  if (!prim.IsValid())return;
 
  if (!prim.IsPseudoRoot()) {
    GraphEditorUI::Node* defaultNode = new GraphEditorUI::Node(prim);
    defaultNode->SetBackgroundColor(pxr::GfVec3f(0.5f, 0.65f, 0.55f));
    AddNode(defaultNode);
  }

  for (pxr::UsdPrim child : prim.GetChildren()) {
    _RecurseNodes(child);
  }
}

void
GraphEditorUI::Graph::_DiscoverConnexions(pxr::UsdPrim& prim)
{
  if (prim.IsA<pxr::UsdShadeNodeGraph>()) {
    for (pxr::UsdPrim child : prim.GetChildren()) {
      _RecurseConnexions(child);
    }
  }
  else if (prim.IsA<pxr::UsdExecGraph>()) {
    for (pxr::UsdPrim child : prim.GetChildren()) {
      _RecurseConnexions(child);
    }
  }
}

void
GraphEditorUI::Graph::_RecurseConnexions(pxr::UsdPrim& prim)
{ 
  if (prim.IsA<pxr::UsdShadeShader>()) {
    pxr::UsdShadeShader shader(prim);
    for (pxr::UsdShadeInput& input : shader.GetInputs()) {
      if (input.GetConnectability() == pxr::UsdShadeTokens->full) {
        pxr::UsdShadeInput::SourceInfoVector connexions = input.GetConnectedSources();
        for (auto& connexion : connexions) {
          GraphEditorUI::Node* source = GetNode(connexion.source.GetPrim());
          GraphEditorUI::Node* destination = GetNode(prim);
          if (!source || !destination)continue;
          GraphEditorUI::Port* start = source->GetPort(connexion.sourceName);
          GraphEditorUI::Port* end = destination->GetPort(input.GetBaseName());

          if (start && end) {
            GraphEditorUI::Connexion* connexion = new Connexion(start, end, start->GetColor());
            _connexions.push_back(connexion);
          }
        }
      }
    }
  } else if (prim.IsA<pxr::UsdExecNode>()) {
    pxr::UsdExecNode node(prim);
    for (pxr::UsdExecInput& input : node.GetInputs()) {
      if (input.GetConnectability() == pxr::UsdShadeTokens->full) {
        pxr::UsdExecInput::SourceInfoVector connexions = input.GetConnectedSources();
        for (auto& connexion : connexions) {
          GraphEditorUI::Node* source = GetNode(connexion.source.GetPrim());
          GraphEditorUI::Node* destination = GetNode(prim);
          if (!source || !destination)continue;
          GraphEditorUI::Port* start = source->GetPort(connexion.sourceName);
          GraphEditorUI::Port* end = destination->GetPort(input.GetBaseName());

          if (start && end) {
            GraphEditorUI::Connexion* connexion = new Connexion(start, end, start->GetColor());
            _connexions.push_back(connexion);
          }
        }
      }
    }
  }
  for (pxr::UsdPrim child: prim.GetChildren()) {
    _RecurseConnexions(child);
  }
}

//==============================================================================
// GRAPH UI
//==============================================================================
// ImGui window flags
//------------------------------------------------------------------------------
ImGuiWindowFlags GraphEditorUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoScrollbar;

// constructor
//------------------------------------------------------------------------------
GraphEditorUI::GraphEditorUI(View* parent)
  : BaseUI(parent, UIType::GRAPHEDITOR)
  , _graph(NULL), _hoveredNode(NULL), _currentNode(NULL)
  , _hoveredPort(NULL), _currentPort(NULL), _hoveredConnexion(NULL)
  , _scale(1.f), _invScale(1.f), _fontIndex(0), _fontScale(1.0)
  , _offset(pxr::GfVec2f(0.f, 0.f)), _dragOffset(pxr::GfVec2f(0.f, 0.f))
  , _drag(false), _marque(false), _navigate(0), _connect(false)
{
  //_filename = filename;
  _id = 0;
  
  Init();
  /*
  _stage = pxr::UsdStage::CreateInMemory();
  pxr::SdfPath meshPath(pxr::TfToken("/mesh"));
  pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh::Define(_stage, meshPath);
  pxr::SdfPath deformedPath(pxr::TfToken("/deformed"));
  pxr::UsdGeomMesh deformed = pxr::UsdGeomMesh::Define(_stage, deformedPath);
  pxr::SdfPath push1Path(pxr::TfToken("/push1"));
  pxr::UsdPrim push1 = _stage->DefinePrim(push1Path);

  pxr::SdfPath graphPath(pxr::TfToken("/graph"));
  pxr::UsdPrim graphPrim = _stage->DefinePrim(graphPath);

  _graph = new Graph(graphPrim);


  Node* getterNode = new Node(mesh.GetPrim());
  getterNode->SetPosition(pxr::GfVec2f(0.f, 0.f));
  _graph->AddNode(getterNode);

  Node* setterNode = new Node(deformed.GetPrim(), true);
  setterNode->SetPosition(pxr::GfVec2f(600.f, 0.f));
  _graph->AddNode(setterNode);
  

  push1.CreateAttribute(TfToken("position"), pxr::SdfValueTypeNames->Float3Array, true);
  push1.CreateAttribute(TfToken("normal"), pxr::SdfValueTypeNames->Float3Array, true);
  push1.CreateAttribute(TfToken("result"), pxr::SdfValueTypeNames->Float3Array, true);
  Node* pushNode = new Node(push1.GetPrim());
  pushNode->SetPosition(pxr::GfVec2f(300.f, 0.f));
  pushNode->AddInput(TfToken("position"), pxr::SdfValueTypeNames->Float3Array);
  pushNode->AddInput(TfToken("normal"), pxr::SdfValueTypeNames->Float3Array);
  pushNode->AddOutput(TfToken("result"), pxr::SdfValueTypeNames->Float3Array);

  _graph->AddNode(pushNode);
  */
  
 // Read("C:/Users/graph/Documents/bmal/src/Amnesie/build/src/Release/graph/test.usda");

}

// destructor
//------------------------------------------------------------------------------
GraphEditorUI::~GraphEditorUI()
{
}


// callbacks
//------------------------------------------------------------------------------
static void
RefreshGraphCallback(GraphEditorUI* editor)
{
  Selection* selection = GetApplication()->GetSelection();

  if (selection->GetNumSelectedItems()) {
    Selection::Item& item = selection->GetItem(0);
    if (item.type == Selection::PRIM) {
      pxr::UsdStageRefPtr stage = GetApplication()->GetWorkStage();
      pxr::UsdPrim selected = stage->GetPrimAtPath(item.path);
      
      if (selected.IsValid() && (selected.IsA<pxr::UsdShadeNodeGraph>() || selected.IsA<UsdExecGraph>())) {
        editor->Populate(selected);
        return;
      }
    }
  }
  editor->Clear();
}

// populate
//------------------------------------------------------------------------------
bool
GraphEditorUI::Populate(pxr::UsdPrim& prim)
{
  if (_graph)delete _graph;
  _graph = new Graph(prim);
  return true;
}

// update
//------------------------------------------------------------------------------
void
GraphEditorUI::Update()
{
  for (auto& node : _graph->GetNodes()) {
    node->Update();
  }
}

void
GraphEditorUI::Clear()
{
  if (_graph) {
    _graph->Clear();
    delete _graph;
  }
  _graph = NULL;
}

// read
//------------------------------------------------------------------------------
bool 
GraphEditorUI::Read(const std::string& filename)
{
  return true;
}

// save
//------------------------------------------------------------------------------
bool 
GraphEditorUI::Write(const std::string& filename)
{
  return true;
}


// term
//------------------------------------------------------------------------------
void 
GraphEditorUI::Term()
{
}

// fonts
//------------------------------------------------------------------------------
void 
GraphEditorUI::UpdateFont()
{
   _fontIndex = size_t(pxr::GfSqrt(_scale));
   switch(_fontIndex) {
     case 0:
       _fontScale = _scale;
       break;
     case 1:
       _fontScale = _scale * 0.5f;
       break;
     default:
       _fontScale = RESCALE(_scale, _fontIndex, _fontIndex * _fontIndex, 0.5, 1.0);
       break;
   }
}

// draw grid
//------------------------------------------------------------------------------
void 
GraphEditorUI::DrawGrid()
{
  const float width = GetWidth();
  const float height = GetHeight();
  const float baseX = GetX();
  const float baseY = GetY();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  const pxr::GfVec2f max(min + size);

  ImGui::PushClipRect(
    min, 
    max, 
    true);

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    min,
    max,
    ImGuiCol_WindowBg);

  float step = 32 * _scale;
  int nX = width / step + 1;
  int nY = height / step + 1;
  pxr::GfVec2f so = _offset * _scale;
  for (int x = 0; x < nX; ++x)
    drawList->AddLine(
      ImVec2(baseX + x*step + so[0], baseY),
      ImVec2(baseX + x*step + so[0], baseY + height),
      ImGuiCol_PlotLines);

  for (int y = 0; y < nY; ++y)
    drawList->AddLine(
      ImVec2(baseX, baseY + y*step + so[1]), 
      ImVec2(baseX + width, baseY + y*step + so[1]), 
      ImGuiCol_PlotLines);
    
  ImGui::PopClipRect();
}

// draw
//------------------------------------------------------------------------------
bool 
GraphEditorUI::Draw()
{
  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);

  //DrawGrid();
  //ImGui::PushFont(GetWindow()->GetRegularFont(_fontIndex));
  ImGui::SetWindowFontScale(_fontScale);

  if (_graph) {
    ImGui::SetWindowFontScale(1.0);
    for (auto& node : _graph->GetNodes()) {
      node->ComputeSize(this);
    }
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    for (auto& node : _graph->GetNodes()) {
      node->Draw(this);
    }

    for (auto& connexion : _graph->GetConnexions()) {
      connexion->Draw(this);
    }

    if (_connect) {
      const GraphEditorUI::Node* startNode = _connector.startPort->GetNode();
      const pxr::GfVec2f viewPos = GetPosition();
      pxr::GfVec2f portPos = _connector.startPort->GetPosition();
      if (_connector.inputOrOutput) portPos += pxr::GfVec2f(startNode->GetWidth(), 0.f);
      const pxr::GfVec2f startPos = GridPositionToViewPosition(portPos + startNode->GetPosition());
      const pxr::GfVec2f endPos = ImGui::GetMousePos();
      const pxr::GfVec2f
        startTangent(startPos[0] * 0.75f + endPos[0] * 0.25f, startPos[1]);
      const pxr::GfVec2f
        endTangent(startPos[0] * 0.25f + endPos[0] * 0.75f, endPos[1]);

      drawList->AddBezierCurve(
        startPos,
        startTangent,
        endTangent,
        endPos,
        _connector.color,
        NODE_CONNEXION_THICKNESS * _scale);
    }
    else if (_marque) {
      const pxr::GfVec2f viewPos = GetPosition();
      drawList->AddRect(
        _marquee.start + viewPos,
        _marquee.end + viewPos,
        ImColor(255, 128, 0, 255),
        0.f, 0, 2);

      drawList->AddRectFilled(
        _marquee.start + viewPos,
        _marquee.end + viewPos,
        ImColor(255, 128, 0, 64),
        0.f, 0);
    }
  }

  ImGui::SetCursorPos(ImVec2(0, 0));

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
    0, ICON_FA_TRASH, ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)RefreshGraphCallback, this
    );
  ImGui::SameLine();

  const pxr::GfVec2f mousePos = ImGui::GetMousePos() - _parent->GetMin();
  if (mousePos[0] > 0 && mousePos[0] < 100 && mousePos[1] > 0 && mousePos[1] < 32) {
    _parent->SetFlag(View::DISCARDMOUSEBUTTON);
  }
  DiscardEventsIfMouseInsideBox(pxr::GfVec2f(0, 0), pxr::GfVec2f(100, 64));
  if (ImGui::Button("TEST")) {
    Selection* selection = GetApplication()->GetSelection();
    bool done = false;
    if (selection->GetNumSelectedItems()) {
      Selection::Item& item = selection->GetItem(0);
      if (item.type == Selection::PRIM) {
        pxr::UsdStageRefPtr stage = GetApplication()->GetWorkStage();
        pxr::UsdPrim selected = stage->GetPrimAtPath(item.path);
        if (selected.IsValid() && selected.IsA<pxr::UsdShadeNodeGraph>()) {
          _graph->Populate(selected);
          done = true;
        }
      }
    } if (!done) {
      //_stage->Export("C:/Users/graph/Documents/bmal/src/Amnesie/build/src/Release/graph/test.usda");
      pxr::UsdPrim graphPrim = TestUsdShadeAPI();
      Populate(graphPrim);
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("SAVE")) {
    const std::string identifier = "./usd/graph.usda";
    //pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory();
    //stage->GetRootLayer()->TransferContent(GetApplication()->GetStage()->GetRootLayer());
    std::cout << "ON SAVE DEFAULT PRIM : " << GetApplication()->GetDisplayStage()->GetDefaultPrim().GetPath() << std::endl;
    GetApplication()->GetDisplayStage()->Export(identifier);
  }
  ImGui::SameLine();

  if (ImGui::Button("LOAD")) {
    const std::string filename = "C:\\Users\\graph\\Documents\\bmal\\src\\Jivaro\\build\\src\\Release\\usd\\graph.usda";
    std::cout << filename << std::endl;
    ADD_COMMAND(OpenSceneCommand, filename);
  }
  ImGui::SameLine();

  //ImGui::PopFont();
  ImGui::End();

  return
    (_hoveredPort != NULL) ||
    (_hoveredConnexion != NULL) ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered();
};

// init
//------------------------------------------------------------------------------
void 
GraphEditorUI::Init()
{
  _selectedNodes.clear();
  _selectedConnexions.clear();
  _currentNode = _hoveredNode = NULL;
  _currentPort = _hoveredPort = NULL;
  _graph = NULL;
  _drag = _marque = false;
  _parent->SetDirty();
  _currentX = _currentY = 0.f;
}

// init
//------------------------------------------------------------------------------
void 
GraphEditorUI::Init(const std::vector<pxr::UsdStageRefPtr>& stages)
{
  _parent->SetDirty();
  /*
  if(_trees.size())
    for(auto tree: _trees) delete tree;

  _trees.resize(stages.size());
  for(int i=0;i<stages.size();++i)
  {
    _trees[i] = new GraphTreeUI(stages[i]);
  }
  */
  //_context = ImNodes::EditorContextCreate();
  
};

void
GraphEditorUI::OnNewSceneNotice(const NewSceneNotice& n)
{
  Clear();
  _parent->SetDirty();
}

void
GraphEditorUI::OnSceneChangedNotice(const SceneChangedNotice& n)
{
  if (!_graph)return;
  if (!_graph->GetPrim().IsValid()) {
    Clear();
  } else {
    Populate(_graph->GetPrim());
  }
  _parent->SetDirty();
}

void
GraphEditorUI::OnAttributeChangedNotice(const AttributeChangedNotice& n)
{
  if (!_graph)return;
  Update();
}


void 
GraphEditorUI::BuildGrid() 
{

}


// build graph
//------------------------------------------------------------------------------ 
void 
GraphEditorUI::_RecurseStagePrim(const pxr::UsdPrim& prim, const pxr::SdfPath& skipPath)
{
  for(auto child : prim.GetChildren())
  {
    if (child.GetPath() == skipPath) continue;
    Node node(child);
    _RecurseStagePrim(child, skipPath);
  }
}

void 
GraphEditorUI::BuildGraph()
{
   _RecurseStagePrim(_stage->GetPseudoRoot(), pxr::SdfPath());
}

void 
GraphEditorUI::_GetNodeUnderMouse(const pxr::GfVec2f& mousePos, bool useExtend)
{
  pxr::GfVec2f viewPos;
  GetRelativeMousePosition(mousePos[0], mousePos[1], viewPos[0], viewPos[1]);

  Node* hovered = NULL;
  std::vector<GraphEditorUI::Node*>& nodes = _graph->GetNodes();
 
  for (auto node = nodes.rbegin(); node != nodes.rend(); ++node) {
    if ((*node)->Contains(ViewPositionToGridPosition(viewPos),
      useExtend ? pxr::GfVec2f(NODE_PORT_RADIUS * _scale) : pxr::GfVec2f(0.f))) {
      hovered = *node;
      break;
    }
  }
  if (_hoveredNode && _hoveredNode != hovered) {
    _hoveredNode->SetState(ITEM_STATE_HOVERED, false);
  }
  if (hovered) {
    hovered->SetState(ITEM_STATE_HOVERED, true);
    _hoveredNode = hovered;
  }
  else _hoveredNode = NULL;

  if (_hoveredPort) {
    _hoveredPort->SetState(ITEM_STATE_HOVERED, false);
    _hoveredPort = NULL;
  };

  if (_hoveredNode) {
    _GetPortUnderMouse(viewPos, _hoveredNode);
  }

  _GetConnexionUnderMouse(viewPos);
  if (_hoveredConnexion)_parent->SetDirty();
}

void 
GraphEditorUI::_GetPortUnderMouse(const pxr::GfVec2f& mousePos, Node* node)
{
  const pxr::GfVec2f relativePosition =
    ViewPositionToGridPosition(mousePos) - node->GetPosition();

  if (relativePosition[1] < NODE_HEADER_HEIGHT - NODE_PORT_RADIUS ||
    (relativePosition[0] > NODE_PORT_RADIUS &&
      relativePosition[0] < node->GetWidth() - NODE_PORT_RADIUS)) return;

  size_t portIndex =
    int((relativePosition[1] - (
      (NODE_HEADER_HEIGHT + NODE_HEADER_PADDING) - NODE_PORT_RADIUS)) / 
        (float)NODE_PORT_VERTICAL_SPACING);

  size_t numPorts = node->GetNumPorts();

  if (portIndex >= numPorts) return;

  Port* port = NULL;
  port = &(node->GetPorts()[portIndex]);

  if (port->Contains(relativePosition) && 
    port->GetFlags() & GraphEditorUI::Port::INPUT) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
    _inputOrOutput = 0;
  } else if (port->Contains(relativePosition - pxr::GfVec2f(node->GetWidth(), 0.f)) && 
    port->GetFlags() & GraphEditorUI::Port::OUTPUT) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
    _inputOrOutput = 1;
  } else {
    _hoveredPort = NULL;
    _inputOrOutput = -1;
  }
}

void 
GraphEditorUI::_GetConnexionUnderMouse(const pxr::GfVec2f& mousePos)
{
  if (_hoveredConnexion) {
    _hoveredConnexion->SetState(ITEM_STATE_HOVERED, false);
    _hoveredConnexion = NULL;
  }
  const pxr::GfVec2f gridPosition = ViewPositionToGridPosition(mousePos);
  for (auto& connexion : _graph->GetConnexions()) {
    const pxr::GfRange2f range = connexion->GetBoundingBox();
    const pxr::GfVec2f extend(2, 2);
    if (range.Contains(gridPosition) && connexion->Contains(mousePos, extend)) {
      _hoveredConnexion = connexion;
      _hoveredConnexion->SetState(ITEM_STATE_HOVERED, true);
      return;
    }
  }
  /*
  if (relativePosition[1] < NODE_HEADER_HEIGHT - NODE_PORT_RADIUS ||
    (relativePosition[0] > NODE_PORT_RADIUS &&
      relativePosition[0] < node->GetWidth() - NODE_PORT_RADIUS)) return;

  size_t portIndex =
    int((relativePosition[1] - (NODE_HEADER_HEIGHT - NODE_PORT_RADIUS)) / (float)NODE_PORT_SPACING);

  size_t numInputs = node->GetNumInputs();
  size_t numOutputs = node->GetNumOutputs();
  if (portIndex >= numInputs + numOutputs) return;

  PortUI* port = NULL;
  if (portIndex >= numInputs)
    port = &(node->GetOutputs()[portIndex - numInputs]);
  else
    port = &(node->GetInputs()[portIndex]);

  if (port->Contains(relativePosition)) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
  }
  else _hoveredPort = NULL;
  */
}

void 
GraphEditorUI::MouseButton(int button, int action, int mods)
{
  if (!_graph)return;
  const pxr::GfVec2f& mousePos = ImGui::GetMousePos();
  _lastX = mousePos[0];
  _lastY = mousePos[1];

  _GetNodeUnderMouse(mousePos, false);

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::PAN;
      else if (_hoveredPort) {
        _connect = true;
        StartConnexion();
      }
      else if (_hoveredNode) {
        if (_hoveredNode->GetState(ITEM_STATE_SELECTED)) {
          _drag = true;
          _dragOffset = pxr::GfVec2f(0.f, 0.f);
        } else {
          bool state = _hoveredNode->GetState(ITEM_STATE_SELECTED);
          _hoveredNode->SetState(ITEM_STATE_SELECTED, 1 - state);
          if (mods & GLFW_MOD_CONTROL) {
            if (1 - state) {
              AddToSelection(_hoveredNode, false);
              _drag = true;
            }
            else RemoveFromSelection(_hoveredNode);
          }
          else if (mods & GLFW_MOD_SHIFT) {
            AddToSelection(_hoveredNode, false);
            _drag = true;
            _dragOffset = pxr::GfVec2f(0.f, 0.f);
          }
          else {
            ClearSelection();
            if (1 - state) {
              AddToSelection(_hoveredNode, true);
              _drag = true;
              _dragOffset = pxr::GfVec2f(0.f, 0.f);
            }
          }
        }
      }
      else {
        if (_parent->GetFlag(View::ACTIVE)) {
          pxr::GfVec2f viewPos;
          GetRelativeMousePosition(mousePos[0], mousePos[1], _marquee.start[0], _marquee.start[1]);
          _marquee.end = _marquee.start;
          _marque = true;
        }
      }
    }
    
    else if (action == GLFW_RELEASE) {
      _navigate = NavigateMode::IDLE;
 
       if(_drag == true && _dragOffset.GetLength() > 0.000001f) {
        ADD_COMMAND(MoveNodeCommand, GetSelectedNodesPath(), _dragOffset);
      }
      _drag = false;
      if (_connect)EndConnexion();
      else if (_marque)MarqueeSelect(mods);
    }
  } 

  else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::PAN;
    } else if (action == GLFW_RELEASE) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::IDLE;
    }
  }

  else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::ZOOM;
    }
    else if (action == GLFW_RELEASE) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::IDLE;
    }
  }

  _parent->SetDirty();
}

void 
GraphEditorUI::Keyboard(int key, int scancode, int action, int mods)
{
  int mappedKey = GetMappedKey(key);

  if (action == GLFW_PRESS) {
    if (mappedKey == GLFW_KEY_DELETE) {
      std::cout << "GRAPH UI : DELETE SELECTED NODES !!! " << std::endl;
      for (auto& node : _selectedNodes) {
        _graph->RemoveNode(node);
      }
    }
    else if (mappedKey == GLFW_KEY_R) {
      std::cout << "GRAPH UI : RESET SCALE OFFSET !!! " << std::endl;
      ResetScaleOffset();
    }
    else if (mappedKey == GLFW_KEY_F) {
      std::cout << "GRAPH UI : FRAME SELECTED NODES !!! " << std::endl;
      FrameSelection();
    }
    else if (mappedKey == GLFW_KEY_A) {
      std::cout << "GRAPH UI : FRAME ALL NODES !!! " << std::endl;
      FrameAll();
    }
    else if (mappedKey == GLFW_KEY_TAB) {
      //NodePopupUI* popup = new NodePopupUI((int)_lastX, (int)_lastY, 200, 300);
      NamePopupUI* popup = new NamePopupUI((int)GetX() + GetWidth() * 0.5f - 100, (int)GetY() + GetHeight() * 0.5 - 50, 200, 100);
      _parent->GetWindow()->SetPopup(popup);
    }
  }
}

void 
GraphEditorUI::Input(int key)
{
  std::cout << "GRAPH EDITOR INPUT : " << key << std::endl;
}

void 
GraphEditorUI::MouseMove(int x, int y)
{
  if (!_graph)return;
  if (_navigate ) {
    switch (_navigate) {
    case NavigateMode::PAN:
      _offset += pxr::GfVec2f(x - _lastX, y - _lastY) / _scale;
      _parent->SetDirty();
      break;
    case NavigateMode::ZOOM:
      MouseWheel(x - _lastX, y - _lastY);
      break;
    default:
      break;
    }
  }
  else if (_drag) {
    for (auto& node : _selectedNodes) {
      _dragOffset += pxr::GfVec2f(x - _lastX, y - _lastY) / _scale;
      const pxr::GfVec2f pos(node->GetPosition() + pxr::GfVec2f(x - _lastX, y - _lastY) / _scale);
      node->SetPosition(pos);
    }
    _parent->SetDirty();
  }
  else if (_marque) {
    GetRelativeMousePosition(x, y, _marquee.end[0], _marquee.end[1]);
    _parent->SetDirty();
  }
  else {
    _GetNodeUnderMouse(pxr::GfVec2f(x,y), true);
    if (_connect && _hoveredPort)UpdateConnexion();
    if (_hoveredPort)_parent->SetDirty();
  }
  _lastX = x;
  _lastY = y;
}

void 
GraphEditorUI::MouseWheel(int x, int y)
{
  pxr::GfVec2f mousePos = (ImGui::GetMousePos() - _parent->GetMin());
  pxr::GfVec2f originalPos = mousePos / _scale ;

  _scale += (x + y) * 0.1;
  _scale = CLAMP(_scale, 0.1, 8.0);
  _invScale = 1.f / _scale;
  pxr::GfVec2f scaledPos = mousePos / _scale;
  _offset -= (originalPos - scaledPos);
  UpdateFont();
  _parent->SetDirty();
}

pxr::GfVec2f 
GraphEditorUI::ViewPositionToGridPosition(const pxr::GfVec2f& mousePos)
{
  return mousePos / _scale - _offset;
}

pxr::GfVec2f
GraphEditorUI::GridPositionToViewPosition(const pxr::GfVec2f& gridPos)
{
  return (gridPos + _offset) * _scale + pxr::GfVec2f(GetX(), GetY());
}

void 
GraphEditorUI::StartConnexion()
{
  _connector.inputOrOutput = _inputOrOutput;
  _connector.startPort = _hoveredPort;
  _connector.color = _connector.startPort->GetColor();
  _connector.endPort = NULL;
}

void 
GraphEditorUI::UpdateConnexion()
{
  if(_hoveredPort) {
    if(_connector.startPort) {
      if(_hoveredPort == _connector.startPort)return;
      if (_hoveredPort->IsOutput())return;
      if(_ConnexionPossible(_connector.startPort, _hoveredPort))
        _connector.endPort = _hoveredPort;
    } else if(_connector.endPort) {
      if(_hoveredPort == _connector.endPort)return;
      if (_hoveredPort->IsInput())return;
      if(_ConnexionPossible(_hoveredPort, _connector.endPort))
      _connector.endPort = _hoveredPort;
    }
  }
}

void 
GraphEditorUI::EndConnexion()
{
  if (_connector.startPort && _connector.endPort) {
    if (!_connector.inputOrOutput) {
      ADD_COMMAND(ConnectNodeCommand, 
        _connector.endPort->GetPath(), _connector.startPort->GetPath());
    }
    else {
      ADD_COMMAND(ConnectNodeCommand,
        _connector.startPort->GetPath(), _connector.endPort->GetPath());
    }
  }
  _connect = false;
  _connector.startPort = _connector.endPort = NULL;
}

pxr::SdfPathVector
GraphEditorUI::GetSelectedNodesPath()
{
  pxr::SdfPathVector paths;
  for (auto& node: _selectedNodes) {
    paths.push_back(node->GetPrim().GetPath());
  }
  return paths;
}

void 
GraphEditorUI::ClearSelection()
{
  if(_selectedNodes.size())
    for (auto& node : _selectedNodes)
      node->SetState(ITEM_STATE_SELECTED, false);
  _selectedNodes.clear();
  if (_selectedConnexions.size())
    for (auto& cnx : _selectedConnexions)
      cnx->SetState(ITEM_STATE_SELECTED, false);
  _selectedConnexions.clear();
}

void 
GraphEditorUI::AddToSelection(Node* node, bool bringToFront) 
{
  node->SetState(ITEM_STATE_SELECTED, true);
  _selectedNodes.insert(node);
  
  std::vector<GraphEditorUI::Node*>& nodes = _graph->GetNodes();
  if (bringToFront) {
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i] == node && nodes.back() != node) {
        std::swap(nodes[i], nodes.back());
      }
    }
  }
}

void 
GraphEditorUI::RemoveFromSelection(Node* node)
{
  node->SetState(ITEM_STATE_SELECTED, false);
  _selectedNodes.erase(node);
}

void
GraphEditorUI::AddToSelection(Connexion* connexion)
{
  connexion->SetState(ITEM_STATE_SELECTED, true);
  _selectedConnexions.insert(connexion);
}

void
GraphEditorUI::RemoveFromSelection(Connexion* connexion)
{
  connexion->SetState(ITEM_STATE_SELECTED, false);
  _selectedConnexions.erase(connexion);
}


void 
GraphEditorUI::MarqueeSelect(int mod)
{
  pxr::GfVec2f start = ViewPositionToGridPosition(_marquee.start);
  pxr::GfVec2f end = ViewPositionToGridPosition(_marquee.end);

  if(start[0] > end[0])std::swap(start[0], end[0]);
  if(start[1] > end[1])std::swap(start[1], end[1]);

  ClearSelection();
  for (auto& node : _graph->GetNodes()) {
    if (node->Intersect(start, end)) AddToSelection(node, false);
  }
  _marque = false;
}

void 
GraphEditorUI::ResetScaleOffset() {
  _scale = 1.f;
  _invScale = 1.f;
  _offset = pxr::GfVec2f(0.f);
  UpdateFont();
  _parent->SetDirty();
}

void 
GraphEditorUI::FrameSelection()
{
  if (!_graph)return;
  if (!_selectedNodes.size())return;

  pxr::GfRange2f selectedRange;
  for (const auto& node : _selectedNodes) {
    selectedRange.ExtendBy(
      pxr::GfRange2f(
        node->GetPosition(),
        node->GetPosition() + node->GetSize()
      )
    );
  }
  pxr::GfVec2f size = selectedRange.GetSize();
  pxr::GfVec2f(pxr::GfMax(size[0], size[1]));
  
  if (GetWidth() > GetHeight()) {
    _scale = GetHeight() / selectedRange.GetSize()[1];
    _scale = CLAMP(_scale, 0.1, 4.0);
  }
  else {
    _offset = -selectedRange.GetCorner(0);
    _scale = GetWidth() / selectedRange.GetSize()[0];
  }
  _invScale = 1.f / _scale;
  _offset = -selectedRange.GetCorner(0) + pxr::GfVec2f(_invScale * GetWidth() * 0.25, 0.f);
  UpdateFont();
  _parent->SetDirty();
}

void 
GraphEditorUI::FrameAll()
{
  if (!_graph) return;
  pxr::GfRange2f allRange;
  for (const auto& node : _graph->GetNodes()) {
    allRange.ExtendBy(
      pxr::GfRange2f(
        node->GetPosition(),
        node->GetPosition() + node->GetSize()
      )
    );
  }
  _offset = -allRange.GetCorner(0);
  if (GetHeight() > GetWidth()) {
    _scale = GetWidth() / allRange.GetSize()[0];
  } else {
    _scale = GetHeight() / allRange.GetSize()[1];
  }
  _scale = CLAMP(_scale, 0.1, 4.0);
  _invScale = 1.f / _scale;

  UpdateFont();
  _parent->SetDirty();

}

JVR_NAMESPACE_CLOSE_SCOPE