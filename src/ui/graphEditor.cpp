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
#include <pxr/usd/usdUI/tokens.h>

#include "../common.h"
#include "../utils/timer.h"
#include "../utils/color.h"
#include "../utils/keys.h"
#include "../utils/usd.h"
#include "../command/manager.h"
#include "../ui/utils.h"
#include "../ui/popup.h"
#include "../ui/graphEditor.h"
#include "../app/commands.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/registry.h"
#include "../app/commands.h"
#include "../app/model.h"
#include "../app/commands.h"
#include "../graph/execution.h"
#include "../graph/hierarchy.h"


JVR_NAMESPACE_OPEN_SCOPE


static
const TfToken& _ConvertExpendedStateToToken(short state)
{
  switch (state) {
  case GraphEditorUI::ExpendedState::COLLAPSED:
    return UsdUITokens->closed;
  case GraphEditorUI::ExpendedState::CONNECTED:
    return UsdUITokens->minimized;
  case GraphEditorUI::ExpendedState::EXPENDED:
    return UsdUITokens->open;
  default:
    return UsdUITokens->open;
  }
}

static
const short _ConvertExpendedStateToEnum(const TfToken& state)
{
  if (state == UsdUITokens->closed)
    return GraphEditorUI::ExpendedState::COLLAPSED;
  else if (state == UsdUITokens->minimized)
    return GraphEditorUI::ExpendedState::CONNECTED;
  else
    return GraphEditorUI::ExpendedState::EXPENDED;
}

int
_GetColorFromAttribute(const UsdAttribute& attr)
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
  if (!attr.IsValid())return GRAPH_COLOR_UNDEFINED;
  SdfValueTypeName vtn = attr.GetTypeName();
  if (vtn == SdfValueTypeNames->Bool || 
    vtn == SdfValueTypeNames->BoolArray) return GRAPH_COLOR_BOOL;
  else if (vtn == SdfValueTypeNames->Int ||
    vtn == SdfValueTypeNames->IntArray) return GRAPH_COLOR_INTEGER;
  else if (vtn == SdfValueTypeNames->UChar ) return GRAPH_COLOR_ENUM;
  else if (vtn == SdfValueTypeNames->Float ||
    vtn == SdfValueTypeNames->FloatArray ||
    vtn == SdfValueTypeNames->Double ||
    vtn == SdfValueTypeNames->FloatArray) return GRAPH_COLOR_FLOAT;
  else if (vtn == SdfValueTypeNames->Float2 ||
    vtn == SdfValueTypeNames->Float2Array) return GRAPH_COLOR_VECTOR2;
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
    vtn == SdfValueTypeNames->Normal3dArray) return GRAPH_COLOR_VECTOR3;
  else if (vtn == SdfValueTypeNames->Float4 ||
    vtn == SdfValueTypeNames->Float4Array) return GRAPH_COLOR_VECTOR4;
  else if (vtn == SdfValueTypeNames->Color4f ||
    vtn == SdfValueTypeNames->Color4fArray) return GRAPH_COLOR_COLOR;
  else if (vtn == SdfValueTypeNames->Asset ||
    vtn == SdfValueTypeNames->AssetArray) return GRAPH_COLOR_PRIM;
  else return GRAPH_COLOR_UNDEFINED;
}

static ImColor
_GetHoveredColor(int color)
{
  GfVec4f origin = UnpackColor4<GfVec4f>(color);
  return ImColor(
    (origin[0] + 1.f) * 0.5f,
    (origin[1] + 1.f) * 0.5f,
    (origin[2] + 1.f) * 0.5f,
    (origin[3] + 1.f) * 0.5f
  );
}

static TfToken
_GetNextExpendedStateToken(const TfToken& token)
{
  if (token ==  UsdUITokens->closed)
    return UsdUITokens->minimized;
  else if (token == UsdUITokens->minimized)
    return UsdUITokens->open;
  else if (token == UsdUITokens->open)
    return UsdUITokens->closed;
}


// Item base class
//------------------------------------------------------------------------------
GraphEditorUI::Item::Item()
  : _pos(0), _size(0), _color(0), _state(0)
{
}

GraphEditorUI::Item::Item(const GfVec2f& pos, 
  const GfVec2f& size, int color)
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
GraphEditorUI::Item::SetPosition(const GfVec2f& pos)
{
  _pos = pos;
}

void
GraphEditorUI::Item::SetSize(const GfVec2f& size)
{
  _size = size;
}

void 
GraphEditorUI::Item::SetColor(const GfVec3f& color)
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
GraphEditorUI::Item::Contains(const GfVec2f& pos, 
  const GfVec2f& extend)
{
  if (pos[0] >= _pos[0] - extend[0] &&
    pos[0] <= _pos[0] + _size[0] + extend[0] &&
    pos[1] >= _pos[1] - extend[1] &&
    pos[1] <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

bool 
GraphEditorUI::Item::Intersect(const GfVec2f& start, 
  const GfVec2f& end)
{
  GfRange2f marqueBox(start, end);
  GfRange2f nodeBox(_pos, _pos + _size);
  if (!nodeBox.IsOutside(marqueBox)) return true;
  else return false;
}

// Port
//------------------------------------------------------------------------------
GraphEditorUI::Port::Port(GraphEditorUI::Node* node, 
  Graph::Port* port)
  : GraphEditorUI::Item(_GetColorFromAttribute(port->GetAttr()))
  , _node(node)
  , _port(port)
{
  _size = GfVec2f(2.f * NODE_PORT_RADIUS);
}

bool 
GraphEditorUI::Port::IsConnected(GraphEditorUI* editor, Graph::Connexion* foundConnexion)
{
  Graph* graph = editor->GetGraph();
  for (auto& connexion : graph->GetConnexions()) {
    if (connexion->GetStart() == Get() || connexion->GetEnd() == Get()) {
      foundConnexion = connexion;
      return true;
    }
  }
  return false;
}

bool 
GraphEditorUI::Port::Contains(const GfVec2f& position,
  const GfVec2f& extend)
{
  if (position[0] + NODE_PORT_RADIUS >= _pos[0] - extend[0] &&
    position[0] + NODE_PORT_RADIUS <= _pos[0] + _size[0] + extend[0] &&
    position[1] + NODE_PORT_RADIUS >= _pos[1] - extend[1] &&
    position[1] + NODE_PORT_RADIUS <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

void
GraphEditorUI::Port::Draw(GraphEditorUI* editor)
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  ImVec2 portTextOffset;
  const GfVec2f offset = editor->GetOffset();
  const float scale = editor->GetScale();
  const GfVec2f p = 
    editor->GridPositionToViewPosition(_node->GetPosition());
  
  static const ImVec2 inputPortOffset(
    NODE_PORT_RADIUS * scale, NODE_PORT_RADIUS * scale);
  static const ImVec2 outputPortOffset(
    - NODE_PORT_RADIUS * scale, NODE_PORT_RADIUS * scale);

  portTextOffset = ImGui::CalcTextSize(_port->GetLabel().GetText());
  portTextOffset.x = -NODE_PORT_RADIUS * 2.f * scale;
  portTextOffset.y -= NODE_PORT_VERTICAL_SPACING * 0.5f * scale;

  drawList->AddText(
    p + (_pos * scale) - portTextOffset,
    ImColor(0, 0, 0, 255),
    _port->GetLabel().GetText());

  if(Get()->GetFlags() & Graph::Port::OUTPUT) {
    drawList->AddCircleFilled(
      p + _pos * scale,
      GetState(ITEM_STATE_HOVERED) ? NODE_PORT_RADIUS * scale * 1.2f : NODE_PORT_RADIUS * scale,
      _color
    );
  }

  if (Get()->GetFlags() & Graph::Port::INPUT) {
    drawList->AddCircleFilled(
      p + (_pos + GfVec2f(_node->GetWidth(), 0.f)) * scale,
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
  GraphEditorUI::Port* start = GetStart();
  GraphEditorUI::Port* end = GetEnd();
  
  GraphEditorUI::Node* startNode = start->GetNode();
  GraphEditorUI::Node* endNode = end->GetNode();

  datas.p0 = start->GetPosition() + GfVec2f(startNode->GetWidth(), 0.f) + startNode->GetPosition();
  datas.p3 = end->GetPosition() + endNode->GetPosition();

  const float length = (datas.p3 - datas.p0).GetLength();
  const GfVec2f offset(0.25f * length, 0.f);

  datas.p1 = datas.p0 + offset;
  datas.p2 = datas.p3 - offset;
  datas.numSegments =
    GfMax(static_cast<int>(length * NODE_CONNEXION_RESOLUTION), 1);
  return datas;
}

GfRange2f 
GraphEditorUI::Connexion::GetBoundingBox()
{
  GfVec2f minBox = _start->GetPosition() + _start->GetNode()->GetPosition();
  GfVec2f maxBox = _end->GetPosition() + _end->GetNode()->GetPosition();
  if (minBox[0] > maxBox[0])std::swap(minBox[0], maxBox[0]);
  if (minBox[1] > maxBox[1])std::swap(minBox[1], maxBox[1]);

  return GfRange2f(minBox, maxBox);
}

bool 
GraphEditorUI::Connexion::Contains(const GfVec2f& position, 
  const GfVec2f& extend)
{
  const GraphEditorUI::ConnexionData datas = GetDescription();
  GfVec2f closest = ImBezierCubicClosestPoint(
      datas.p0, datas.p1, datas.p2, datas.p3, position, 32);
  return (position - closest).GetLength() < extend[0];
}

bool 
GraphEditorUI::Connexion::Intersect(const GfVec2f& start,
  const GfVec2f& end)
{
  return false;
}

// Connexion draw
//------------------------------------------------------------------------------
void 
GraphEditorUI::Connexion::Draw(GraphEditorUI* editor)
{
  const GraphEditorUI::ConnexionData datas = GetDescription();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const float scale = editor->GetScale();
  if (GetState(ITEM_STATE_HOVERED)) {
    drawList->AddBezierCubic(
      editor->GridPositionToViewPosition(datas.p0),
      editor->GridPositionToViewPosition(datas.p1),
      editor->GridPositionToViewPosition(datas.p2),
      editor->GridPositionToViewPosition(datas.p3),
      _GetHoveredColor(_color),
      NODE_CONNEXION_THICKNESS * scale * 1.4,
      32);
  }

  else if(GetState(ITEM_STATE_SELECTED)) 
    drawList->AddBezierCubic(
      editor->GridPositionToViewPosition(datas.p0),
      editor->GridPositionToViewPosition(datas.p1),
      editor->GridPositionToViewPosition(datas.p2),
      editor->GridPositionToViewPosition(datas.p3),
      ImColor(255,255,255,255),
      NODE_CONNEXION_THICKNESS * editor->GetScale(),
      32);

  else
    drawList->AddBezierCubic(
      editor->GridPositionToViewPosition(datas.p0),
      editor->GridPositionToViewPosition(datas.p1),
      editor->GridPositionToViewPosition(datas.p2),
      editor->GridPositionToViewPosition(datas.p3),
      _color,
      NODE_CONNEXION_THICKNESS * editor->GetScale(),
      32);
}


// Node constructor
//------------------------------------------------------------------------------
GraphEditorUI::Node::Node(Graph::Node* node)
  : GraphEditorUI::Item()
  , _node(node)
  , _dirty(DIRTY_POSITION|DIRTY_SIZE)
{
  Read();
  for(auto& port: node->GetPorts()) {
    _ports.push_back(GraphEditorUI::Port(this, &port));
  }
}


// NodeUI destructor
//------------------------------------------------------------------------------
GraphEditorUI::Node::~Node()
{
  Write();
}


int 
GraphEditorUI::Node::GetColor() const
{
  UsdUINodeGraphNodeAPI api(_node->GetPrim());
  GfVec3f color;
  api.GetDisplayColorAttr().Get(&color);
  return PackColor3<GfVec3f>(color);
   
}

GraphEditorUI::Port* 
GraphEditorUI::Node::GetPort(const TfToken& name)
{
  for (auto& port : _ports) {
    if (port.Get()->GetName() == name) return &port;
  }
  return NULL;
}


void 
GraphEditorUI::Node::SetColor(const GfVec3f& color)
{
  _color = PackColor3<GfVec3f>(color);
}


void 
GraphEditorUI::Node::ComputeSize(GraphEditorUI* editor)
{
  if (GetDirty() & GraphEditorUI::Node::DIRTY_SIZE) {

    float width =
      ImGui::CalcTextSize(_node->GetName().GetText()).x + 2 * NODE_PORT_HORIZONTAL_SPACING +
      (NODE_EXPENDED_SIZE + 2 * NODE_HEADER_PADDING);
    float height = NODE_HEADER_HEIGHT + NODE_HEADER_PADDING + RANDOM_0_X(10);
    float mid = NODE_HEADER_HEIGHT * 0.5;
    float inputWidth = 0, outputWidth = 0;
    Graph::Connexion* connexion = NULL;
    
    for (auto& port : _ports) {
      float w = ImGui::CalcTextSize(port.Get()->GetName().GetText()).x +
        NODE_PORT_HORIZONTAL_SPACING;
      if (w > inputWidth)inputWidth = w;
      if(_expended == UsdUITokens->closed) {
        port.SetPosition(GfVec2f(0.f, mid));
      } else if(_expended == UsdUITokens->minimized) {
        if (port.IsConnected(editor, connexion)) {
          port.SetPosition(GfVec2f(0.f, height));
          height += NODE_PORT_VERTICAL_SPACING;
        }
        else 
          port.SetPosition(GfVec2f(0.f, mid));
      } else if(_expended == UsdUITokens->open) {
        port.SetPosition(GfVec2f(0.f, height));
        height += NODE_PORT_VERTICAL_SPACING;
      }
    }

    SetSize(GfVec2f(width, height));
    SetDirty(GraphEditorUI::Node::DIRTY_CLEAN);
  }
}

void
GraphEditorUI::Node::Write()
{
  if(!_node->GetPrim().IsValid())return;

  if(!_node->GetPrim().HasAPI<UsdUINodeGraphNodeAPI>()) {
    UsdUINodeGraphNodeAPI api = 
      UsdUINodeGraphNodeAPI::Apply(_node->GetPrim());
    api.CreatePosAttr().Set(_pos);
    api.CreateSizeAttr().Set(_size);
    api.CreateExpansionStateAttr().Set(_expended);
    api.CreateDisplayColorAttr().Set(UnpackColor3<GfVec3f>(_color));
  } else {
    UsdUINodeGraphNodeAPI api(_node->GetPrim());
    api.GetPosAttr().Set(_pos);
    api.GetSizeAttr().Set(_size);
    api.GetExpansionStateAttr().Set(_expended);
    api.GetDisplayColorAttr().Set(UnpackColor3<GfVec3f>(_color));
  }
}

void
GraphEditorUI::Node::Read()
{
  if (_node->GetPrim().HasAPI<UsdUINodeGraphNodeAPI>()) {
    UsdUINodeGraphNodeAPI api(_node->GetPrim());
    api.GetPosAttr().Get(&_pos);
    api.GetSizeAttr().Get(&_size);
    api.GetExpansionStateAttr().Get(&_expended);
    GfVec3f color;
    api.GetDisplayColorAttr().Get(&color);
    _color = PackColor3<GfVec3f>(color);
  } else {
    UsdUINodeGraphNodeAPI api = 
      UsdUINodeGraphNodeAPI::Apply(_node->GetPrim());
    _pos = GfVec2f(0.f);
    api.CreatePosAttr().Set(_pos);
    _size = GfVec2f(100,32);
    api.CreateSizeAttr().Set(_size);
    _expended = UsdUITokens->closed;
    api.CreateExpansionStateAttr().Set(_expended);
    //api.CreateDisplayColorAttr().Set(UnpackColor3<GfVec3f>(RANDOM_LO_HI(0,65565)));
    api.CreateDisplayColorAttr().Set(GfVec3f(0.5f,0.5f,0.5f));
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
  const GfVec3f nodeColor = UnpackColor3<GfVec3f>(GetColor());
  if (IsVisible(editor)) {
    const float scale = editor->GetScale();
    const GfVec2f offset = editor->GetOffset();
    const GfVec2f p = editor->GetPosition() + (GetPosition() + offset) * scale;
    const float x = p[0];
    const float y = p[1];
    const GfVec2f s = GetSize() * scale;
    const ImColor selectedColor = GetState(ITEM_STATE_SELECTED) ?
      ImColor(255, 255, 255, 255) :
      ImColor(0, 0, 0, 255);

    // body background
    drawList->AddRectFilled(
      ImVec2(x, y),
      ImVec2(x + s[0], y + s[1]),
      ImColor(nodeColor[0], nodeColor[1], nodeColor[2], 1.f),
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
        1.f * scale);
    }

    drawList->AddText(p + GfVec2f(NODE_PORT_PADDING, 
      NODE_HEADER_PADDING) * scale, ImColor(0, 0, 0, 255), Get()->GetName().GetText());
    
    // expended state
    const GfVec2f expendOffset((GetWidth() - (NODE_EXPENDED_SIZE + 2 * NODE_HEADER_PADDING)), NODE_HEADER_PADDING);
    const GfVec2f expendPos = p + expendOffset * scale;
    const GfVec2f expendSize(NODE_EXPENDED_SIZE * scale);
    const GfVec2f elementOffset(0.f, NODE_EXPENDED_SIZE * scale * 0.4);
    const GfVec2f elementSize(NODE_EXPENDED_SIZE * scale, NODE_EXPENDED_SIZE * scale * 0.2);
    const ImColor expendColor(0, 0, 0, 255);

    ImGui::SetCursorPos((GetPosition() + expendOffset + offset) * scale);

    if (ImGui::Selectable(UI::HiddenLabel(_expended.GetString().c_str()).c_str(), 
      true, ImGuiSelectableFlags_SelectOnClick, expendSize)) {
      _expended = _GetNextExpendedStateToken(_expended);
      ADD_COMMAND(ExpendNodeCommand, {_node->GetPrim().GetPath()}, _expended);
      SetDirty(GraphEditorUI::Node::DIRTY_SIZE);
    }

    drawList->AddRectFilled(
      expendPos + 2 * elementOffset, 
      expendPos + 2 * elementOffset + elementSize, 
      expendColor, 0);

    if (_expended == UsdUITokens->minimized || _expended == UsdUITokens->open)
      drawList->AddRectFilled(
        expendPos + elementOffset, 
        expendPos + elementOffset + elementSize, 
        expendColor, 0);
  
    if (_expended == UsdUITokens->open)
      drawList->AddRectFilled(
        expendPos, 
        expendPos + elementSize, 
        expendColor, 0);
    
    // ports
    Graph::Connexion* connexion = NULL;
    if (_expended != UsdUITokens->closed) {
      ImGui::PushFont(window->GetFont(FONT_MEDIUM , editor->GetFontIndex()));
      int numPorts = _ports.size();
      for (int i = 0; i < numPorts; ++i) {
        if (_expended == UsdUITokens->open) _ports[i].Draw(editor);
        else {
          if (_ports[i].IsConnected(editor, connexion)) _ports[i].Draw(editor);
        }
      }
      ImGui::PopFont();
    }
  
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
  , _offset(GfVec2f(0.f, 0.f)), _dragOffset(GfVec2f(0.f, 0.f))
  , _drag(false), _marque(false), _navigate(0), _connect(false)
{
  //_filename = filename;
  _id = 0;
  
  Init();
  /*
  _stage = UsdStage::CreateInMemory();
  SdfPath meshPath(TfToken("/mesh"));
  UsdGeomMesh mesh = UsdGeomMesh::Define(_stage, meshPath);
  SdfPath deformedPath(TfToken("/deformed"));
  UsdGeomMesh deformed = UsdGeomMesh::Define(_stage, deformedPath);
  SdfPath push1Path(TfToken("/push1"));
  UsdPrim push1 = _stage->DefinePrim(push1Path);

  SdfPath graphPath(TfToken("/graph"));
  UsdPrim graphPrim = _stage->DefinePrim(graphPath);

  _graph = new Graph(graphPrim);


  Node* getterNode = new Node(mesh.GetPrim());
  getterNode->SetPosition(GfVec2f(0.f, 0.f));
  _graph->AddNode(getterNode);

  Node* setterNode = new Node(deformed.GetPrim(), true);
  setterNode->SetPosition(GfVec2f(600.f, 0.f));
  _graph->AddNode(setterNode);
  

  push1.CreateAttribute(TfToken("position"), SdfValueTypeNames->Float3Array, true);
  push1.CreateAttribute(TfToken("normal"), SdfValueTypeNames->Float3Array, true);
  push1.CreateAttribute(TfToken("result"), SdfValueTypeNames->Float3Array, true);
  Node* pushNode = new Node(push1.GetPrim());
  pushNode->SetPosition(GfVec2f(300.f, 0.f));
  pushNode->AddInput(TfToken("position"), SdfValueTypeNames->Float3Array);
  pushNode->AddInput(TfToken("normal"), SdfValueTypeNames->Float3Array);
  pushNode->AddOutput(TfToken("result"), SdfValueTypeNames->Float3Array);

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
  Selection* selection = editor->GetModel()->GetSelection();

  if (selection->GetNumSelectedItems()) {
    Selection::Item& item = selection->GetItem(0);
    if (item.type == Selection::PRIM) {
      UsdStageRefPtr stage = editor->GetModel()->GetStage();
      UsdPrim selected = stage->GetPrimAtPath(item.path);
      
      if (selected.IsValid()) {
        editor->Populate(new HierarchyGraph(stage, selected));
        return;
      }
    }
  }
  editor->Clear();
}

GraphEditorUI::Port*
GraphEditorUI::GetPort(Graph::Port* port)
{
  for (auto& node : _nodes) {
    if (node->Get() != port->GetNode()) continue;
    return node->GetPort(port->GetName());
  }
  return NULL;
}

// populate
//------------------------------------------------------------------------------
bool
GraphEditorUI::Populate(Graph* graph)
{
  Clear();
  _graph = graph;

  for (auto& node : _graph->GetNodes()) {
    _nodes.push_back(new GraphEditorUI::Node(node));
  }
  for (auto& connexion : _graph->GetConnexions()) {
    GraphEditorUI::Port* start = GetPort(connexion->GetStart());
    GraphEditorUI::Port* end = GetPort(connexion->GetEnd());
    if (start && end) {
      _connexions.push_back(new GraphEditorUI::Connexion(start, end, connexion, GRAPH_COLOR_FLOAT));
    }
  }
  UpdateFont();
  return true;
}

// rearrange
//------------------------------------------------------------------------------
/*
void
GraphEditorUI::Rearrange(Graph* graph, Node* node)
{
  
}
*/

// update
//------------------------------------------------------------------------------
void
GraphEditorUI::Write()
{
  for (auto& node : _nodes) {
    node->Write();
  }
}

void
GraphEditorUI::Read()
{
  for (auto& node : _nodes) {
    node->Read();
  }
}


void
GraphEditorUI::Clear()
{
  for (auto& connexion : _connexions)delete connexion;
  for (auto& node : _nodes)delete node;
  _connexions.clear();
  _nodes.clear();

  _selectedNodes.clear();
  _selectedConnexions.clear();
  _hoveredNode = nullptr;
  _currentNode = nullptr;
  _hoveredPort = nullptr;
  _currentPort = nullptr;
  _hoveredConnexion = nullptr;
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
  if(_scale <= 0.5f)
    _fontIndex = 0;
  else if(_scale <= 1.f)
    _fontIndex = 1;
  else
    _fontIndex = 2;
  
  _fontScale = _scale / FONT_SIZE_FACTOR[_fontIndex];
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

  const GfVec2f min(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());
  const GfVec2f max(min + size);

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
  GfVec2f so = _offset * _scale;
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
  const GfVec2f min(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  //DrawGrid();
  //ImGui::PushFont(GetWindow()->GetRegularFont(_fontIndex));
  if (_graph) {
    ImGui::SetWindowFontScale(1.0);
    for (auto& node : _nodes) {
      node->ComputeSize(this);
    }
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGui::SetWindowFontScale(GetFontScale());
    for (auto& connexion : _connexions) {
      connexion->Draw(this);
    }
    ImGui::PushFont(GetWindow()->GetFont(FONT_LARGE, GetFontIndex()));
    for (auto& node : _nodes) {
      node->Draw(this);
    }
    ImGui::PopFont();

    if (_connect) {
      const GraphEditorUI::Node* startNode = _connector.startPort->GetNode();
      const GfVec2f viewPos = GetPosition();
      GfVec2f portPos = _connector.startPort->GetPosition();
      if (_connector.inputOrOutput) portPos += GfVec2f(startNode->GetWidth(), 0.f);
      const GfVec2f startPos = GridPositionToViewPosition(portPos + startNode->GetPosition());
      const GfVec2f endPos = ImGui::GetMousePos();
      const GfVec2f
        startTangent(startPos[0] * 0.75f + endPos[0] * 0.25f, startPos[1]);
      const GfVec2f
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
      const GfVec2f viewPos = GetPosition();
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

  ImGui::SetWindowFontScale(1.0);
  ImGui::SetCursorPos(ImVec2(0, 0));

  UI::AddIconButton(0, ICON_FA_RECYCLE, UI::STATE_DEFAULT,
    std::bind(RefreshGraphCallback, this));
  ImGui::SameLine();

  const GfVec2f mousePos = ImGui::GetMousePos() - _parent->GetMin();
  if (mousePos[0] > 0 && mousePos[0] < 100 && mousePos[1] > 0 && mousePos[1] < 32) {
    _parent->SetFlag(View::DISCARDMOUSEBUTTON);
  }
  DiscardEventsIfMouseInsideBox(GfVec2f(0, 0), GfVec2f(100, 64));
  if (ImGui::Button("TEST")) {
    Selection* selection = _model->GetSelection();
    bool done = false;
    if (selection->GetNumSelectedItems()) {
      Selection::Item& item = selection->GetItem(0);
      if (item.type == Selection::PRIM) {
        UsdStageRefPtr stage = _model->GetStage();
        UsdPrim selected = stage->GetPrimAtPath(item.path);
        if (selected.IsValid() && selected.IsA<UsdShadeNodeGraph>()) {
          std::cout << "test button!" << std::endl;
          if (_graph)delete _graph;
          _graph = new ExecutionGraph(selected);
          done = true;
        }
      }
    } if (!done) {
      //_stage->Export("C:/Users/graph/Documents/bmal/src/Amnesie/build/src/Release/graph/test.usda");
      ExecutionGraph* graph = TestUsdExecAPI();
      std::cout << "not done!" << std::endl;
      Populate(graph);
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("SAVE")) {
    const std::string identifier = "./usd/graph.usda";
    //UsdStageRefPtr stage = UsdStage::CreateInMemory();
    std::cout << "ON SAVE DEFAULT PRIM : " << _model->GetStage()->GetDefaultPrim().GetPath() << std::endl;
    _model->GetDisplayStage()->Export(identifier);
  }
  ImGui::SameLine();

  if (ImGui::Button("LOAD")) {
    const std::string filename = "./usd/graph.usda";
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
}

// init
//------------------------------------------------------------------------------
void 
GraphEditorUI::Init(const std::vector<UsdStageRefPtr>& stages)
{
  _parent->SetDirty();
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
  if (!_graph) return;
  if (!_graph->GetPrim().IsValid()) {
    Clear();
  } else {
    _graph->Clear();
    _graph->Populate(_graph->GetPrim());
    Populate(_graph);
  }
  _parent->SetDirty();
}

void
GraphEditorUI::OnAttributeChangedNotice(const AttributeChangedNotice& n)
{
  if (!_graph)return;
  Read();
}


void 
GraphEditorUI::_GetNodeUnderMouse(const GfVec2f& mousePos, bool useExtend)
{
  GfVec2f viewPos;
  GetRelativeMousePosition(mousePos[0], mousePos[1], viewPos[0], viewPos[1]);

  Node* hovered = NULL;
 
  size_t numNodes = _nodes.size();
  if(!numNodes) return;
  for (int nodeIdx = numNodes - 1; nodeIdx >= 0; --nodeIdx) {
    if (_nodes[nodeIdx]->Contains(ViewPositionToGridPosition(viewPos),
      useExtend ? GfVec2f(NODE_PORT_RADIUS * _scale) : GfVec2f(0.f))) {
      hovered = _nodes[nodeIdx];
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
GraphEditorUI::_GetPortUnderMouse(const GfVec2f& mousePos, Node* node)
{
  const GfVec2f relativePosition =
    ViewPositionToGridPosition(mousePos) - node->GetPosition();

  if (relativePosition[1] < NODE_HEADER_HEIGHT - NODE_PORT_RADIUS ||
    (relativePosition[0] > NODE_PORT_RADIUS &&
      relativePosition[0] < node->GetWidth() - NODE_PORT_RADIUS)) return;

  size_t portIndex =
    int((relativePosition[1] - (
      (NODE_HEADER_HEIGHT + NODE_HEADER_PADDING) - NODE_PORT_RADIUS)) / 
        (float)NODE_PORT_VERTICAL_SPACING);

  size_t numPorts = node->Get()->GetNumPorts();

  if (portIndex >= numPorts) return;

  Port* port = NULL;
  port = &(node->GetPorts()[portIndex]);

  if (port->Contains(relativePosition) && 
    port->Get()->GetFlags() & Graph::Port::INPUT) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
    _inputOrOutput = 0;
  } else if (port->Contains(relativePosition - GfVec2f(node->GetWidth(), 0.f)) && 
    port->Get()->GetFlags() & Graph::Port::OUTPUT) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
    _inputOrOutput = 1;
  } else {
    _hoveredPort = NULL;
    _inputOrOutput = -1;
  }
}

void 
GraphEditorUI::_GetConnexionUnderMouse(const GfVec2f& mousePos)
{
  if (_hoveredConnexion) {
    _hoveredConnexion->SetState(ITEM_STATE_HOVERED, false);
    _hoveredConnexion = NULL;
  }
  const GfVec2f gridPosition = ViewPositionToGridPosition(mousePos);
  for (auto& connexion : _connexions) {
    const GfRange2f range = connexion->GetBoundingBox();
    const GfVec2f extend(2, 2);
    if (range.Contains(gridPosition) && connexion->Contains(mousePos, extend)) {
      _hoveredConnexion = connexion;
      _hoveredConnexion->SetState(ITEM_STATE_HOVERED, true);
      return;
    }
  }
}

void 
GraphEditorUI::MouseButton(int button, int action, int mods)
{
  pxr::GfVec2f mousePos = _parent->GetWindow()->GetMousePosition();
  _lastX = mousePos[0];
  _lastY = mousePos[1];

  if (!_graph)return;

  _GetNodeUnderMouse(mousePos, false);

  uint64_t now = CurrentTime();
  double diffMs = (now - _lastClick) * 1e-6;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if((now - _lastClick)<50) {
        std::cout << "Graph Editor Double click" << std::endl;
      }
      _lastClick = now;
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::PAN;
      else if (_hoveredPort) {
        _connect = true;
        StartConnexion();
      }
      else if (_hoveredNode) {
        if (_hoveredNode->GetState(ITEM_STATE_SELECTED)) {
          _drag = true;
          _dragOffset = GfVec2f(0.f, 0.f);
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
            _dragOffset = GfVec2f(0.f, 0.f);
          }
          else {
            ClearSelection();
            if (1 - state) {
              AddToSelection(_hoveredNode, true);
              _drag = true;
              _dragOffset = GfVec2f(0.f, 0.f);
            }
          }
        }
      }
      else {
        if (_parent->GetFlag(View::ACTIVE)) {
          GfVec2f viewPos;
          GetRelativeMousePosition(mousePos[0], mousePos[1], _marquee.start[0], _marquee.start[1]);
          _marquee.end = _marquee.start;
          _marque = true;
        }
      }
    }
    
    else if (action == GLFW_RELEASE) {
      _navigate = NavigateMode::IDLE;

      if (_drag == true && _dragOffset.GetLength() > 0.000001f) {
        ADD_COMMAND(MoveNodeCommand, GetSelectedNodesPath(), _dragOffset);
      } else if(diffMs > 10 && diffMs < 250){
        switch(mods) {
        case GLFW_MOD_SHIFT :
          _model->AddToSelection(GetSelectedNodesPath());
          break;
         
        default :
          _model->SetSelection(GetSelectedNodesPath());
          break;
        }
      }
      _drag = false;
      if (_connect)EndConnexion();
      else if (_marque)MarqueeSelect(mods);
    }
  } 

  else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (action == GLFW_PRESS) {
      _lastClick = now;
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::PAN;
    } else if (action == GLFW_RELEASE) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::IDLE;
    }
  }

  else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS) {
      _lastClick = now;
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::ZOOM;
    }
    else if (action == GLFW_RELEASE) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::IDLE;
    }
  }

  _parent->SetDirty();
}

static void _CreatePrimCallback(Model* model, const TfToken& token)
{
  Selection* selection = model->GetSelection();
  TfToken name(token.GetString() + "_" + RandomString(6));

  if (selection->GetNumSelectedItems()) {
    ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), selection->GetItem(0).path.AppendChild(name), token);
  }
  else {
    ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), SdfPath("/" + name.GetString()), token);
  }
}

void 
GraphEditorUI::Keyboard(int key, int scancode, int action, int mods)
{
  int mappedKey = GetMappedKey(key);

  if (action == GLFW_PRESS) {
    if (mappedKey == GLFW_KEY_DELETE) {
      std::cout << "GRAPH UI : DELETE SELECTED NODES !!! " << std::endl;
      for (auto& node : _selectedNodes) {
        _graph->RemoveNode(node->Get());
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
      GfVec2f mousePos = _parent->GetWindow()->GetMousePosition();
      ListPopupUI* popup = new ListPopupUI("Create Prim", mousePos[0], mousePos[1], 200, 100,
        std::bind(&_CreatePrimCallback, _model, std::placeholders::_1));

      ADD_DEFERRED_COMMAND(UIGenericCommand, std::bind(&WindowRegistry::SetPopup, popup));
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
      _offset += GfVec2f(x - _lastX, y - _lastY) / _scale;
      _parent->SetDirty();
      break;
    case NavigateMode::ZOOM:
      MouseWheel((x - _lastX) * _invScale, (y - _lastY) * _invScale);
      break;
    default:
      break;
    }
  }
  else if (_drag) {
    GfVec2f offset((x - _lastX) / _scale, (y - _lastY) / _scale);
    _dragOffset += offset;
    for (auto& node : _selectedNodes) {
      const GfVec2f pos(node->GetPosition() + offset);
      node->SetPosition(pos);
    }
    _parent->SetDirty();
  }
  else if (_marque) {
    GetRelativeMousePosition(x, y, _marquee.end[0], _marquee.end[1]);
    _parent->SetDirty();
  }
  else {
    _GetNodeUnderMouse(GfVec2f(x,y), true);
    if (_connect && _hoveredPort)UpdateConnexion();
    if (_hoveredPort)_parent->SetDirty();
  }
  _lastX = x;
  _lastY = y;
}

void 
GraphEditorUI::MouseWheel(int x, int y)
{
  GfVec2f mousePos = (GfVec2f(_lastX, _lastY) - _parent->GetMin());
  GfVec2f originalPos = mousePos / _scale ;

  _scale += y * 0.1;
  _scale = CLAMP(_scale, 0.01, 8.0);
  _invScale = 1.f / _scale;
  GfVec2f scaledPos = mousePos / _scale;
  _offset -= (originalPos - scaledPos);
  UpdateFont();
  _parent->SetDirty();
}

GfVec2f 
GraphEditorUI::ViewPositionToGridPosition(const GfVec2f& mousePos)
{
  return mousePos / _scale - _offset;
}

GfVec2f
GraphEditorUI::GridPositionToViewPosition(const GfVec2f& gridPos)
{
  return (gridPos + _offset) * _scale + GfVec2f(GetX(), GetY());
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
      if (_hoveredPort->Get()->IsOutput())return;
      if(_graph->ConnexionPossible(_connector.startPort->Get(), _hoveredPort->Get()))
        _connector.endPort = _hoveredPort;
    } else if(_connector.endPort) {
      if(_hoveredPort == _connector.endPort)return;
      if (_hoveredPort->Get()->IsInput())return;
      if(_graph->ConnexionPossible(_hoveredPort->Get(), _connector.endPort->Get()))
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
        _connector.endPort->Get()->GetPath(), _connector.startPort->Get()->GetPath());
    }
    else {
      ADD_COMMAND(ConnectNodeCommand,
        _connector.startPort->Get()->GetPath(), _connector.endPort->Get()->GetPath());
    }
  }
  _connect = false;
  _connector.startPort = _connector.endPort = NULL;
}

SdfPathVector
GraphEditorUI::GetSelectedNodesPath()
{
  SdfPathVector paths;
  for (auto& node: _selectedNodes) {
    paths.push_back(node->Get()->GetPrim().GetPath());
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
  
  if (bringToFront) {
    for (size_t i = 0; i < _nodes.size(); ++i) {
      if (_nodes[i] == node && _nodes.back() != node) {
        std::swap(_nodes[i], _nodes.back());
      }
    }
  }
  if(_graph->GetType() == Graph::Type::HIERARCHY) {
    //_model->SetSelection()
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
  GfVec2f start = ViewPositionToGridPosition(_marquee.start);
  GfVec2f end = ViewPositionToGridPosition(_marquee.end);

  if(start[0] > end[0])std::swap(start[0], end[0]);
  if(start[1] > end[1])std::swap(start[1], end[1]);

  ClearSelection();
  for (auto& node : _nodes) {
    if (node->Intersect(start, end)) AddToSelection(node, false);
  }
  _marque = false;
}

void 
GraphEditorUI::ResetScaleOffset() {
  _scale = 1.f;
  _invScale = 1.f;
  _offset = GfVec2f(0.f);
  UpdateFont();
  _parent->SetDirty();
}

void 
GraphEditorUI::FrameSelection()
{
  if (!_graph)return;
  if (!_selectedNodes.size())return;

  GfRange2f selectedRange;
  for (const auto& node : _selectedNodes) {
    selectedRange.ExtendBy(
      GfRange2f(
        node->GetPosition(),
        node->GetPosition() + node->GetSize()
      )
    );
  }
  GfVec2f size = selectedRange.GetSize();
  GfVec2f(GfMax(size[0], size[1]));
  
  if (GetWidth() > GetHeight()) {
    _scale = GetHeight() / selectedRange.GetSize()[1];
    _scale = CLAMP(_scale, 0.1, 4.0);
  }
  else {
    _offset = -selectedRange.GetCorner(0);
    _scale = GetWidth() / selectedRange.GetSize()[0];
  }
  _invScale = 1.f / _scale;
  _offset = -selectedRange.GetCorner(0) + GfVec2f(_invScale * GetWidth() * 0.25, 0.f);
  UpdateFont();
  _parent->SetDirty();
}

void 
GraphEditorUI::FrameAll()
{
  if (!_graph) return;
  GfRange2f allRange;
  for (const auto& node : _nodes) {
    allRange.ExtendBy(
      GfRange2f(
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