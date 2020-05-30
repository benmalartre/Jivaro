#include "node.h"
#include "graph.h"
#include "../app/window.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdGeom/xformable.h>

AMN_NAMESPACE_OPEN_SCOPE

int GetColorFromAttribute(const pxr::UsdAttribute& attr)
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
    vtn == pxr::SdfValueTypeNames->FloatArray) return GRAPH_COLOR_FLOAT;
  else if (vtn == pxr::SdfValueTypeNames->Float2 ||
    vtn == pxr::SdfValueTypeNames->Float2Array) return GRAPH_COLOR_VECTOR2;
  else if (vtn == pxr::SdfValueTypeNames->Float3 ||
    vtn == pxr::SdfValueTypeNames->Vector3f ||
    vtn == pxr::SdfValueTypeNames->Float3Array ||
    vtn == pxr::SdfValueTypeNames->Vector3fArray ) return GRAPH_COLOR_VECTOR3;
  else if (vtn == pxr::SdfValueTypeNames->Float4 ||
    vtn == pxr::SdfValueTypeNames->Float4Array) return GRAPH_COLOR_VECTOR4;
  else if (vtn == pxr::SdfValueTypeNames->Color4f ||
    vtn == pxr::SdfValueTypeNames->Color4fArray) return GRAPH_COLOR_COLOR;
  else if (vtn == pxr::SdfValueTypeNames->Asset ||
    vtn == pxr::SdfValueTypeNames->AssetArray) return GRAPH_COLOR_PRIM;
}

ItemUI::ItemUI()
  : _pos(0), _size(0), _color(0), _state(0)
{
}

ItemUI::ItemUI(const pxr::GfVec2f& pos, const pxr::GfVec2f& size, int color)
  : _pos(pos), _size(size), _color(color), _state(0)
{
}

ItemUI::ItemUI(int color)
  : _pos(0), _size(0), _color(color), _state(0)
{
}

void ItemUI::SetState(size_t flag, bool value)
{
  if (value) _state |= flag;
  else _state &= ~flag;
}

bool ItemUI::GetState(size_t flag)
{
  return _state & flag;
}

void ItemUI::SetPosition(const pxr::GfVec2f& pos)
{
  _pos = pos;
}

void ItemUI::SetSize(const pxr::GfVec2f& size)
{
  _size = size;
}

void ItemUI::SetColor(const pxr::GfVec3f& color)
{
  _color = ImColor(
    int(color[0] * 255),
    int(color[1] * 255),
    int(color[2] * 255),
    255);
}

void ItemUI::SetColor(int color)
{
  _color = color;
}

bool ItemUI::Contains(const pxr::GfVec2f& pos, const pxr::GfVec2f& extend)
{
  if (pos[0] >= _pos[0] - extend[0] &&
    pos[0] <= _pos[0] + _size[0] + extend[0] &&
    pos[1] >= _pos[1] - extend[1] &&
    pos[1] <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

bool ItemUI::Intersect(const pxr::GfVec2f& start, const pxr::GfVec2f& end)
{
  pxr::GfRange2f grabBox(start, end);
  pxr::GfRange2f nodeBox(_pos, _pos + _size);
  if (!nodeBox.IsOutside(grabBox)) return true;
  else return false;
}

PortUI::PortUI(NodeUI* node, bool io, const std::string& label, pxr::UsdAttribute& attr)
  :ItemUI(GetColorFromAttribute(attr))
{
  _node = node;
  _label = label;
  _io = io;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
}

PortUI::PortUI(NodeUI* node, const pxr::GraphInput& port)
  :ItemUI(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = true;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
}

PortUI::PortUI(NodeUI* node, const pxr::GraphOutput& port)
  :ItemUI(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = false;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
}

bool PortUI::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  if (position[0] + NODE_PORT_RADIUS >= _pos[0] - extend[0] &&
    position[0] + NODE_PORT_RADIUS <= _pos[0] + _size[0] + extend[0] &&
    position[1] + NODE_PORT_RADIUS >= _pos[1] - extend[1] &&
    position[1] + NODE_PORT_RADIUS <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

void PortUI::Draw(GraphUI* editor)
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const pxr::GfVec2f offset = editor->GetOffset();
  const float scale = editor->GetScale();
  const pxr::GfVec2f p = 
    editor->GridPositionToViewPosition(_node->GetPosition());
  
  if (_io) {
    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale * (GetState(ITEM_STATE_HOVERED) ? 1.25f : 1.f),
      _color
    );
    drawList->AddText(
      p + _pos * scale,
      ImColor(0, 0, 0, 255),
      _label.c_str());
  }
  else {
    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale * (GetState(ITEM_STATE_HOVERED) ? 1.25f : 1.f),
      _color
    );
    drawList->AddText(
      p + _pos * scale,
      ImColor(0, 0, 0, 255),
      _label.c_str());
  }
}

// ConnexionUI description
//------------------------------------------------------------------------------
ConnexionUIData ConnexionUI::GetDescription()
{
  ConnexionUIData datas;
  datas.p0 = _start->GetPosition() + _start->GetNode()->GetPosition();
  datas.p3 = _end->GetPosition() + _end->GetNode()->GetPosition();

  const float length = (datas.p3 - datas.p0).GetLength();
  const pxr::GfVec2f offset(0.25f * length, 0.f);

  datas.p1 = datas.p0 + offset;
  datas.p2 = datas.p3 - offset;
  datas.numSegments =
    pxr::GfMax(static_cast<int>(length * NODE_CONNEXION_RESOLUTION), 1);
  return datas;
}

pxr::GfRange2f ConnexionUI::GetBoundingBox()
{
  pxr::GfVec2f minBox = _start->GetPosition() + _start->GetNode()->GetPosition();
  pxr::GfVec2f maxBox = _end->GetPosition() + _end->GetNode()->GetPosition();
  if (minBox[0] > maxBox[0])std::swap(minBox[0], maxBox[0]);
  if (minBox[1] > maxBox[1])std::swap(minBox[1], maxBox[1]);

  return pxr::GfRange2f(minBox, maxBox);
}

bool ConnexionUI::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  return false;
}

bool ConnexionUI::Intersect(const pxr::GfVec2f& start,
  const pxr::GfVec2f& end)
{
  return false;
}

// ConnexionUI draw
//------------------------------------------------------------------------------
void ConnexionUI::Draw(GraphUI* graph)
{
  const ConnexionUIData datas = GetDescription();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const float scale = graph->GetScale();
  if (GetState(ITEM_STATE_HOVERED)) {
    drawList->AddBezierCurve(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      ImColor(255,255,255,255),
      NODE_CONNEXION_THICKNESS * scale * 1.4);
  }

  if(GetState(ITEM_STATE_SELECTED)) 
    drawList->AddBezierCurve(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      ImColor(255,0,0,255),
      NODE_CONNEXION_THICKNESS * graph->GetScale());

  else
    drawList->AddBezierCurve(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      _color,
      NODE_CONNEXION_THICKNESS * graph->GetScale());
}

// NodeUI constructor
//------------------------------------------------------------------------------
NodeUI::NodeUI(const pxr::UsdPrim& prim)
 : ItemUI(), _prim(prim)
{
  if(_prim.IsValid())
  {
    _name = prim.GetName();
    if(_prim.IsA<pxr::GraphNode>())
    {
      pxr::GraphNode node(_prim);
      _inputs.resize(node.NumInputs());
      for(int i=0;i<_inputs.size();++i)
      {
        _inputs[i] = PortUI(this, node.GetInput(i));
      }

      int offset = _inputs.size();
      _outputs.resize(node.NumOutputs());
      for(int i=0;i<_outputs.size();++i)
      {
        _outputs[i] = PortUI(this, node.GetOutput(i));
      }
    }
    else if(_prim.IsA<pxr::GraphGraph>())
    {
      //std::cout << "PRIM IS A GRAPH :D" << std::endl;
    }
  }
}

// NodeUI destructor
//------------------------------------------------------------------------------
NodeUI::~NodeUI()
{

}

void NodeUI::Init()
{
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

  _size[1] += _inputs.size() * NODE_PORT_SPACING;

}

void NodeUI::AddInput(const std::string& name, pxr::SdfValueTypeName type)
{
  pxr::UsdAttribute attr = _prim.CreateAttribute(pxr::TfToken(name), type);
  PortUI port(this, true, name, attr);
  //port.SetPosition(pxr::GfVec2f(0, _inputs.size() * NODE_PORT_SPACING));
  _inputs.push_back(port);

}

void NodeUI::AddOutput(const std::string& name, pxr::SdfValueTypeName type)
{
  pxr::UsdAttribute attr = _prim.CreateAttribute(pxr::TfToken(name), type);
  PortUI port(this, false, name, attr);
  //port.SetPosition(pxr::GfVec2f(0, _inputs.size() * NODE_PORT_SPACING));
  _outputs.push_back(port);
}

void NodeUI::ComputeSize()
{
  float width = ImGui::CalcTextSize(_name.c_str()).x + 32;
  
  float height = NODE_HEADER_HEIGHT;
  for (auto& input : _inputs) {
    input.SetPosition(pxr::GfVec2f(0.f, height));
    float w = ImGui::CalcTextSize(input.GetName().c_str()).x + 32;
    if (w > width)width = w;
    height += NODE_PORT_SPACING;
  }
  for (auto& output : _outputs) {
    float w = ImGui::CalcTextSize(output.GetName().c_str()).x + 32;
    if (w > width)width = w;
    output.SetPosition(pxr::GfVec2f(width, height));
    height += NODE_PORT_SPACING;
  }

  for (auto& output : _outputs) {
    output.SetPosition(pxr::GfVec2f(width, output.GetPosition()[1]));
  }

  _size = pxr::GfVec2f(width, height);
}

void NodeUI::Update()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.CreatePosAttr();
  posAttr.Set(_pos);

  pxr::UsdAttribute sizeAttr = api.CreateSizeAttr();
  sizeAttr.Set(_size);

}

// NodeUI draw
//------------------------------------------------------------------------------
void NodeUI::Draw(GraphUI* editor) 
{
  Window* window = editor->GetWindow();
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  ImGui::SetWindowFontScale(1.0);
  ImGui::PushFont(window->GetMediumFont(0));
  ComputeSize();


  ImGui::SetWindowFontScale(editor->GetFontScale());
  ImGui::PushFont(window->GetMediumFont(editor->GetFontIndex()));

  const float scale = editor->GetScale();
  const pxr::GfVec2f offset = editor->GetOffset();
  const ImVec2 p = editor->GetPosition() + (GetPosition() + offset) * scale;
  const float x = p.x;
  const float y = p.y;
  const ImVec2 s = GetSize() * scale;

  drawList->AddRectFilled(
    ImVec2(x, y),
    ImVec2(x + s.x, y + s.y),
    ImColor(60,60,60, 255),
    NODE_PORT_RADIUS * scale,
    ImDrawCornerFlags_All);

  drawList->AddRect(
    ImVec2(x, y),
    ImVec2(x + s.x, y + s.y),
    GetState(ITEM_STATE_SELECTED) ? ImColor(0, 255, 0, 255) : 
      GetState(ITEM_STATE_HOVERED) ? ImColor(255, 255, 0, 255) : ImColor(0,0,0,255),
    NODE_PORT_RADIUS * scale,
    ImDrawCornerFlags_All,
    2 * scale);

  drawList->AddText( p + pxr::GfVec2f(NODE_PORT_PADDING, 0), ImColor(0,0,0,255), _name.c_str() );

  ImGui::PopFont();
  
  ImGui::PushFont(window->GetRegularFont(editor->GetFontIndex()));

  int numInputs = _inputs.size();
  
  for(int i=0;i<numInputs;++i) _inputs[i].Draw(editor);

  int numOutputs = _outputs.size();
  ImGui::Indent(40);
  for(int i=0;i<numOutputs;++i) _outputs[i].Draw(editor);
  ImGui::PopFont();

} 


AMN_NAMESPACE_CLOSE_SCOPE