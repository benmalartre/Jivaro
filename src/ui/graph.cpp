#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usdUI/backdrop.h>

#include "../utils/color.h"
#include "../ui/ui.h"
#include "../ui/graph.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"



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
  else return GRAPH_COLOR_UNDEFINED;
}

static ImColor _GetHoveredColor(int color)
{
  pxr::GfVec4f origin = UnpackColor(color);
  return ImColor(
    (origin[0] + 1.f) * 0.5f,
    (origin[1] + 1.f) * 0.5f,
    (origin[2] + 1.f) * 0.5f,
    (origin[3] + 1.f) * 0.5f
  );
}

GraphUI::Item::Item()
  : _pos(0), _size(0), _color(0), _state(0)
{
}

GraphUI::Item::Item(const pxr::GfVec2f& pos, const pxr::GfVec2f& size, int color)
  : _pos(pos), _size(size), _color(color), _state(0)
{
}

GraphUI::Item::Item(int color)
  : _pos(0), _size(0), _color(color), _state(0)
{
}

void GraphUI::Item::SetState(size_t flag, bool value)
{
  if (value) _state |= flag;
  else _state &= ~flag;
}

bool GraphUI::Item::GetState(size_t flag)
{
  return _state & flag;
}

void GraphUI::Item::SetPosition(const pxr::GfVec2f& pos)
{
  _pos = pos;
}

void GraphUI::Item::SetSize(const pxr::GfVec2f& size)
{
  _size = size;
}

void GraphUI::Item::SetColor(const pxr::GfVec3f& color)
{
  _color = ImColor(
    int(color[0] * 255),
    int(color[1] * 255),
    int(color[2] * 255),
    255);
}

void GraphUI::Item::SetColor(int color)
{
  _color = color;
}

bool GraphUI::Item::Contains(const pxr::GfVec2f& pos, const pxr::GfVec2f& extend)
{
  if (pos[0] >= _pos[0] - extend[0] &&
    pos[0] <= _pos[0] + _size[0] + extend[0] &&
    pos[1] >= _pos[1] - extend[1] &&
    pos[1] <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

bool GraphUI::Item::Intersect(const pxr::GfVec2f& start, const pxr::GfVec2f& end)
{
  pxr::GfRange2f marqueBox(start, end);
  pxr::GfRange2f nodeBox(_pos, _pos + _size);
  if (!nodeBox.IsOutside(marqueBox)) return true;
  else return false;
}

GraphUI::Port::Port(GraphUI::Node* node, bool io, const std::string& label, pxr::UsdAttribute& attr)
  : GraphUI::Item(GetColorFromAttribute(attr))
  , _attr(attr)
{
  _node = node;
  _label = label;
  _io = io;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
}

/*
GraphUI::Port::Port(GraphUI::Node* node, const pxr::UsdShadeInput& port)
  : GraphUI::Item(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = true;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
  _attr = port.GetAttr();
}

GraphUI::Port::Port(GraphUI::Node* node, const pxr::UsdShadeOutput& port)
  : GraphUI::Item(GetColorFromAttribute(pxr::UsdAttribute(port)))
{
  _node = node;
  _label = port.GetBaseName().GetText();
  _io = false;
  _size = pxr::GfVec2f(2.f * NODE_PORT_RADIUS);
  _attr = port.GetAttr();
}
*/

bool GraphUI::Port::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  if (position[0] + NODE_PORT_RADIUS >= _pos[0] - extend[0] &&
    position[0] + NODE_PORT_RADIUS <= _pos[0] + _size[0] + extend[0] &&
    position[1] + NODE_PORT_RADIUS >= _pos[1] - extend[1] &&
    position[1] + NODE_PORT_RADIUS <= _pos[1] + _size[1] + extend[1])return true;
  return false;
}

void GraphUI::Port::Draw(GraphUI* editor)
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

  if (_io) {
    portTextOffset = ImGui::CalcTextSize(_label.c_str());
    portTextOffset.x = -NODE_PORT_RADIUS * 2.f * scale;
    portTextOffset.y -= NODE_PORT_VERTICAL_SPACING * 0.5f * scale;

    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale * 1.5f,
      GetState(ITEM_STATE_HOVERED) ? NODE_CONTOUR_SELECTED : NODE_CONTOUR_DEFAULT
    );

    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale,
      _color
    );
    drawList->AddText(
      p + (_pos * scale) - portTextOffset,
      ImColor(0, 0, 0, 255),
      _label.c_str());
  }
  else {
    portTextOffset = ImGui::CalcTextSize(_label.c_str());
    portTextOffset.x += NODE_PORT_RADIUS * 2.f * scale;
    portTextOffset.y -= NODE_PORT_VERTICAL_SPACING * 0.5f * scale;
    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale * 1.5f,
      GetState(ITEM_STATE_HOVERED) ? NODE_CONTOUR_SELECTED : NODE_CONTOUR_DEFAULT
    );

    drawList->AddCircleFilled(
      p + _pos * scale,
      NODE_PORT_RADIUS * scale,
      _color
    );
    drawList->AddText(
      p + (_pos * scale) - portTextOffset,
      ImColor(0, 0, 0, 255),
      _label.c_str());
  }
}

// Connexion description
//------------------------------------------------------------------------------
GraphUI::ConnexionData GraphUI::Connexion::GetDescription()
{
  GraphUI::ConnexionData datas;
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

pxr::GfRange2f GraphUI::Connexion::GetBoundingBox()
{
  pxr::GfVec2f minBox = _start->GetPosition() + _start->GetNode()->GetPosition();
  pxr::GfVec2f maxBox = _end->GetPosition() + _end->GetNode()->GetPosition();
  if (minBox[0] > maxBox[0])std::swap(minBox[0], maxBox[0]);
  if (minBox[1] > maxBox[1])std::swap(minBox[1], maxBox[1]);

  return pxr::GfRange2f(minBox, maxBox);
}

bool GraphUI::Connexion::Contains(const pxr::GfVec2f& position,
  const pxr::GfVec2f& extend)
{
  return false;
}

bool GraphUI::Connexion::Intersect(const pxr::GfVec2f& start,
  const pxr::GfVec2f& end)
{
  return false;
}

// Connexion draw
//------------------------------------------------------------------------------
void GraphUI::Connexion::Draw(GraphUI* graph)
{
  const GraphUI::ConnexionData datas = GetDescription();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const float scale = graph->GetScale();
  if (GetState(ITEM_STATE_HOVERED)) {
    drawList->AddBezierCurve(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      _GetHoveredColor(_color),
      NODE_CONNEXION_THICKNESS * scale * 1.4);
  }

  else if(GetState(ITEM_STATE_SELECTED)) 
    drawList->AddBezierCurve(
      graph->GridPositionToViewPosition(datas.p0),
      graph->GridPositionToViewPosition(datas.p1),
      graph->GridPositionToViewPosition(datas.p2),
      graph->GridPositionToViewPosition(datas.p3),
      ImColor(255,255,255,255),
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

// Node constructor
//------------------------------------------------------------------------------
GraphUI::Node::Node(pxr::UsdPrim& prim)
 : GraphUI::Item(), _prim(prim)
{
  if(_prim.IsValid())
  {
    _name = prim.GetName();

    if(_prim.HasAPI<pxr::UsdUISceneGraphPrimAPI>())
    {

    }
    
    if (_prim.HasAPI<pxr::UsdUINodeGraphNodeAPI>())
    {
      /*
      pxr::UsdShadeShader node(_prim);
      _inputs.resize(node.GetInputs().size());
      for(int i=0;i<_inputs.size();++i)
      {
        _inputs[i] =
          PortUI(this, node.GetInput(pxr::TfToken(_inputs[i].GetName())));
      }

      int offset = _inputs.size();
      _outputs.resize(node.GetOutputs().size());
      for(int i=0;i<_outputs.size();++i)
      {
        _outputs[i] =
          PortUI(this, node.GetOutput(pxr::TfToken(_outputs[i].GetName())));
      }
    }
    else if(_prim.IsA<pxr::UsdShadeNodeGraph>())
    {
      //std::cout << "PRIM IS A GRAPH :D" << std::endl;
    }
    */
    }
  }
}

// NodeUI destructor
//------------------------------------------------------------------------------
GraphUI::Node::~Node()
{

}

void GraphUI::Node::Init()
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

  _size[1] += _inputs.size() * NODE_PORT_VERTICAL_SPACING;

}

void GraphUI::Node::AddInput(const std::string& name, pxr::SdfValueTypeName type)
{
  pxr::UsdAttribute attr = _prim.CreateAttribute(pxr::TfToken(name), type);
  GraphUI::Port port(this, true, name, attr);
  //port.SetPosition(pxr::GfVec2f(0, _inputs.size() * NODE_PORT_VERTICAL_SPACING));
  _inputs.push_back(port);

}

void GraphUI::Node::AddOutput(const std::string& name, pxr::SdfValueTypeName type)
{
  pxr::UsdAttribute attr = _prim.CreateAttribute(pxr::TfToken(name), type);
  GraphUI::Port port(this, false, name, attr);
  //port.SetPosition(pxr::GfVec2f(0, _inputs.size() * NODE_PORT_VERTICAL_SPACING));
  _outputs.push_back(port);
}

void GraphUI::Node::ComputeSize()
{
  float nameWidth = 
    ImGui::CalcTextSize(_name.GetText()).x + 2 * NODE_PORT_HORIZONTAL_SPACING;
  float inputWidth= 0, outputWidth = 0;
  float height = NODE_HEADER_HEIGHT + NODE_HEADER_PADDING;
  float width = 0.f;
  for (auto& input : _inputs) {
    input.SetPosition(pxr::GfVec2f(0.f, height + NODE_PORT_RADIUS));
    float w = ImGui::CalcTextSize(input.GetName().c_str()).x + 
      NODE_PORT_HORIZONTAL_SPACING;
    if (w > inputWidth)inputWidth = w;
    height += NODE_PORT_VERTICAL_SPACING;
  }
  for (auto& output : _outputs) {
    output.SetPosition(pxr::GfVec2f(width, height + NODE_PORT_RADIUS));
    float w = ImGui::CalcTextSize(output.GetName().c_str()).x + 
      NODE_PORT_HORIZONTAL_SPACING;
    if (w > outputWidth)outputWidth = w;
    height += NODE_PORT_VERTICAL_SPACING;
  }

  if( nameWidth > (inputWidth + outputWidth)) {
    width = nameWidth;
  } else {
    width = (inputWidth + outputWidth);
  }

  for (auto& output : _outputs) {
    output.SetPosition(pxr::GfVec2f(width, output.GetPosition()[1]));
  }

  _size = pxr::GfVec2f(width, height);
}

void GraphUI::Node::Update()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.CreatePosAttr();
  posAttr.Set(_pos);

  pxr::UsdAttribute sizeAttr = api.CreateSizeAttr();
  sizeAttr.Set(_size);

}

bool GraphUI::Node::IsVisible(GraphUI* editor)
{
  return true;
}

// Node draw
//------------------------------------------------------------------------------
void GraphUI::Node::Draw(GraphUI* editor) 
{
  Window* window = editor->GetWindow();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  if (IsVisible(editor)) {
    ImGui::SetWindowFontScale(1.0);
    ImGui::PushFont(window->GetMediumFont(0));
    ComputeSize();
    ImGui::PopFont();
    ImGui::SetWindowFontScale(editor->GetFontScale());
    ImGui::PushFont(window->GetMediumFont(editor->GetFontIndex()));

    const float scale = editor->GetScale();
    const pxr::GfVec2f offset = editor->GetOffset();
    const ImVec2 p = editor->GetPosition() + (GetPosition() + offset) * scale;
    const float x = p.x;
    const float y = p.y;
    const ImVec2 s = GetSize() * scale;

    // body background
    drawList->AddRectFilled(
      ImVec2(x, y),
      ImVec2(x + s.x, y + s.y),
      ImColor(90, 120, 180, 255),
      NODE_PORT_RADIUS * scale,
      ImDrawCornerFlags_All);

    // head background
    drawList->AddRectFilled(
      ImVec2(x, y),
      ImVec2(x + s.x, y + NODE_HEADER_HEIGHT * scale),
      ImColor(150, 160, 190, 255),
      NODE_PORT_RADIUS * scale,
      ImDrawCornerFlags_All);

    // border
    drawList->AddRect(
      ImVec2(x, y),
      ImVec2(x + s.x, y + s.y),
      GetState(ITEM_STATE_SELECTED) ? ImColor(255, 255, 255, 255) :
        GetState(ITEM_STATE_HOVERED) ? ImColor(60, 60, 60, 100) : 
          ImColor(0, 0, 0, 100),
      NODE_PORT_RADIUS * scale,
      ImDrawCornerFlags_All,
      2 * scale);

    drawList->AddText(p + pxr::GfVec2f(NODE_PORT_PADDING, 
      NODE_HEADER_PADDING), ImColor(0, 0, 0, 255), _name.GetText());

    ImGui::PopFont();

    ImGui::PushFont(window->GetRegularFont(editor->GetFontIndex()));

    int numInputs = _inputs.size();

    for (int i = 0; i<numInputs; ++i) _inputs[i].Draw(editor);

    int numOutputs = _outputs.size();
    ImGui::Indent(40);
    for (int i = 0; i<numOutputs; ++i) _outputs[i].Draw(editor);
    ImGui::PopFont();
  }
} 

ImGuiWindowFlags GraphUI::_flags = 
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
GraphUI::GraphUI(View* parent, const std::string& filename)
  : BaseUI(parent, "Graph")
  , _hoveredNode(NULL), _currentNode(NULL)
  , _hoveredPort(NULL), _currentPort(NULL), _hoveredConnexion(NULL)
  , _scale(1.f), _fontIndex(0), _fontScale(1.0), _offset(pxr::GfVec2f(0.f, 0.f))
  , _drag(false), _marque(false), _navigate(0), _connect(false)
{
  /*
  //_filename = filename;
  _id = 0;
  
  //GraphTreeUI* tree = new GraphTreeUI();
  _stage = pxr::UsdStage::CreateInMemory();

  for (int i = 0; i < 12; ++i) {
    pxr::UsdPrim prim =
      _stage->DefinePrim(pxr::SdfPath(pxr::TfToken("/node" + std::to_string(i))));
    NodeUI* node = new NodeUI(prim);
    node->SetPosition(pxr::GfVec2f(
      ((float)rand() / (float)RAND_MAX) * 255,
      ((float)rand() / (float)RAND_MAX) * 255));
    node->SetSize(pxr::GfVec2f(120, 32));

    node->AddInput("Input1", pxr::SdfValueTypeNames->Vector3f);
    node->AddInput("Input2", pxr::SdfValueTypeNames->Float);
    node->AddInput("Input3", pxr::SdfValueTypeNames->Color4f);
    node->AddInput("Input4", pxr::SdfValueTypeNames->FloatArray);
    node->AddOutput("Output1", pxr::SdfValueTypeNames->Vector3f);
    node->AddOutput("Output2", pxr::SdfValueTypeNames->Float);
    node->AddOutput("Output3", pxr::SdfValueTypeNames->Color4f);
    node->AddOutput("Output4", pxr::SdfValueTypeNames->FloatArray);
    
    _nodes.push_back(node);
  }
  */
}

// destructor
//------------------------------------------------------------------------------
GraphUI::~GraphUI()
{
  for (auto& node : _nodes)delete node;
}

// term
//------------------------------------------------------------------------------
void GraphUI::Term()
{
    //ImNodes::EditorContextFree(_context);
}

// fonts
//------------------------------------------------------------------------------
void GraphUI::UpdateFont()
{
  if (_scale < 1.0) {
    _fontIndex = 0;
    _fontScale = _scale;
  }
  else if (_scale < 2.0) {
    _fontIndex = 1;
    _fontScale = RESCALE(_scale, 1.0, 2.0, 0.5, 1.0);
  }
  else {
    _fontIndex = 2;
    _fontScale = RESCALE(_scale, 2.0, 4.0, 0.5, 1.0);
  }
}

// draw grid
//------------------------------------------------------------------------------
void GraphUI::DrawGrid()
{
  const float width = GetWidth();
  const float height = GetHeight();
  const float baseX = _parent->GetX();
  const float baseY = _parent->GetY();

  ImGui::PushClipRect(
    _parent->GetMin(), 
    _parent->GetMax(), 
    true);

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
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
bool GraphUI::Draw()
{
  ImGui::SetNextWindowPos(_parent->GetMin());
  ImGui::SetNextWindowSize(_parent->GetSize());
  
  ImGui::Begin("Graph Editor", NULL, _flags);

  //DrawGrid();

  ImGui::PushFont(GetWindow()->GetRegularFont(_fontIndex));
  ImGui::SetWindowFontScale(_fontScale);

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  for (auto& connexion : _connexions) {
    connexion->Draw(this);
  }

  for (auto& node : _nodes) {
    node->Draw(this);
  }

  if (_connect) {
    const pxr::GfVec2f viewPos = GetPosition();
    const pxr::GfVec2f startPos = GridPositionToViewPosition(
      _connector.startPort->GetPosition() +
      _connector.startPort->GetNode()->GetPosition());
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

  
  ImGui::SetCursorPos(ImVec2(64, 64));
  static bool value;
  ImGui::Checkbox("FUCK", &value);
  
  ImGui::PopFont();
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
void GraphUI::Init(const std::string& filename)
{
  _parent->SetDirty();
  
  /*
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  pxr::UsdPrimRange primRange = stage->Traverse();
  for(auto prim: primRange)
  {
    if(prim.GetTypeName() == pxr::TfToken("Stage"))
    {
      pxr::GraphStage stageNode(prim);
      std::string fileName;
      pxr::UsdAttribute fileNameAttr = stageNode.CreateFileNameAttr();
      fileNameAttr.Get(&fileName);

      pxr::UsdStageRefPtr usdStage = pxr::UsdStage::CreateInMemory();
      //_trees.push_back(new GraphTreeUI(usdStage));
    }
  }*/ 

}

// init
//------------------------------------------------------------------------------
void GraphUI::Init(const std::vector<pxr::UsdStageRefPtr>& stages)
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

void GraphUI::BuildGrid() 
{
  for (auto& node : _nodes) {
    pxr::GfVec2f pos = node->GetPosition();
  }
}


// build graph
//------------------------------------------------------------------------------ 
void GraphUI::_RecurseStagePrim(const pxr::UsdPrim& prim)
{
  for(auto child : prim.GetChildren())
  {
    Node node(child);

    _RecurseStagePrim(child);
  }
}

void GraphUI::BuildGraph()
{
   _RecurseStagePrim(_stage->GetPseudoRoot());
}

void GraphUI::_GetNodeUnderMouse(const pxr::GfVec2f& mousePos, bool useExtend)
{
  pxr::GfVec2f viewPos;
  GetRelativeMousePosition(mousePos[0], mousePos[1], viewPos[0], viewPos[1]);

  Node* hovered = NULL;
  for (auto node = _nodes.rbegin(); node != _nodes.rend(); ++node) {
    if ((*node)->Contains(ViewPositionToGridPosition(viewPos),
      useExtend ? pxr::GfVec2f(NODE_PORT_RADIUS * _scale) : pxr::GfVec2f(0.f))) {
      hovered = *node;
      break;
    }
  }
  if (_hoveredNode && _hoveredNode != hovered)_hoveredNode->SetState(ITEM_STATE_HOVERED, false);
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

void GraphUI::_GetPortUnderMouse(const pxr::GfVec2f& mousePos, Node* node)
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

  size_t numInputs = node->GetNumInputs();
  size_t numOutputs = node->GetNumOutputs();
  if (portIndex >= numInputs + numOutputs) return;

  Port* port = NULL;
  if (portIndex >= numInputs)
    port = &(node->GetOutputs()[portIndex - numInputs]);
  else
    port = &(node->GetInputs()[portIndex]);

  if (port->Contains(relativePosition)) {
    port->SetState(ITEM_STATE_HOVERED, true);
    _hoveredPort = port;
  }
  else _hoveredPort = NULL;
}

void GraphUI::_GetConnexionUnderMouse(const pxr::GfVec2f& mousePos)
{
  if (_hoveredConnexion) {
    _hoveredConnexion->SetState(ITEM_STATE_HOVERED, false);
    _hoveredConnexion = NULL;
  }
  const pxr::GfVec2f gridPosition = ViewPositionToGridPosition(mousePos);
  for (auto& connexion : _connexions) {
    pxr::GfRange2f range = connexion->GetBoundingBox();
    if (range.Contains(gridPosition)) {
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

void GraphUI::MouseButton(int button, int action, int mods)
{
  const pxr::GfVec2f& mousePos = ImGui::GetMousePos();
  _lastX = mousePos[0];
  _lastY = mousePos[1];

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT) _navigate = NavigateMode::PAN;
      else if (_hoveredPort) {
        _connect = true;
        StartConnexion();
      }
      else if (_hoveredNode) {
        if (_hoveredNode->GetState(ITEM_STATE_SELECTED))
          _drag = true;
        else {
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
          }
          else {
            ClearSelection();
            if (1 - state) {
              AddToSelection(_hoveredNode, true);
              _drag = true;
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
      _drag = false;
      if (_connect) EndConnexion();
      else if (_marque)MarqueeSelect(mods);
      _marque = false;
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

void GraphUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_DELETE) {
    std::cout << "GRAPH UI : DELETE SELECTED NODES !!! " << std::endl;
  }
}

void GraphUI::MouseMove(int x, int y)
{
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
    for (auto& node : _selected) {
      node->Move(pxr::GfVec2f(x - _lastX, y - _lastY) / _scale);
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

void GraphUI::MouseWheel(int x, int y)
{
  pxr::GfVec2f mousePos = (ImGui::GetMousePos() - _parent->GetMin());
  pxr::GfVec2f originalPos = mousePos / _scale ;

  _scale += (x + y) * 0.1;
  _scale = CLAMP(_scale, 0.1, 4.0);
  _invScale = 1.f / _scale;
  pxr::GfVec2f scaledPos = mousePos / _scale;
  _offset -= (originalPos - scaledPos);
  UpdateFont();
  _parent->SetDirty();
}

pxr::GfVec2f GraphUI::ViewPositionToGridPosition(const pxr::GfVec2f& mousePos)
{
  return mousePos / _scale - _offset;
}

pxr::GfVec2f GraphUI::GridPositionToViewPosition(const pxr::GfVec2f& gridPos)
{
  return (gridPos + _offset) * _scale + _parent->GetMin();
}

void GraphUI::StartConnexion()
{
  _connector.startPort = _hoveredPort;
  _connector.color = _connector.startPort->GetColor();
  _connector.endPort = NULL;
}

void GraphUI::UpdateConnexion()
{
  if(_hoveredPort) {
    if(_connector.startPort) {
      if(_hoveredPort == _connector.startPort)return;
      if (_hoveredPort->IsOutput())return;
      if(_hoveredPort->GetAttr().GetTypeName() == 
        _connector.startPort->GetAttr().GetTypeName())
        _connector.endPort = _hoveredPort;
    } else if(_connector.endPort) {
      if(_hoveredPort == _connector.endPort)return;
      if (_hoveredPort->IsInput())return;
      if(_hoveredPort->GetAttr().GetTypeName() == 
        _connector.endPort->GetAttr().GetTypeName())
      _connector.endPort = _hoveredPort;
    }
  }
}

void GraphUI::EndConnexion()
{
  if (_connector.startPort && _connector.endPort) {
    Connexion* connexion = new Connexion(_connector.startPort, 
      _connector.endPort, _connector.color);
    _connexions.push_back(connexion);
  }
  _connect = false;
  _connector.startPort = _connector.endPort = NULL;
}

void GraphUI::ClearSelection()
{
  if(_selected.size())
    for (auto& node : _selected)
      node->SetState(ITEM_STATE_SELECTED, false);
  _selected.clear();
}

void GraphUI::AddToSelection(Node* node, bool bringToFront) 
{
  node->SetState(ITEM_STATE_SELECTED, true);
  _selected.insert(node);
  
  if (bringToFront) {
    for (size_t i = 0; i < _nodes.size(); ++i) {
      if (_nodes[i] == node && _nodes.back() != node) {
        std::swap(_nodes[i], _nodes.back());
      }
    }
  }
}

void GraphUI::RemoveFromSelection(Node* node)
{
  node->SetState(ITEM_STATE_SELECTED, false);
  _selected.erase(node);
}

void GraphUI::MarqueeSelect(int mod)
{
  pxr::GfVec2f start = ViewPositionToGridPosition(_marquee.start);
  pxr::GfVec2f end = ViewPositionToGridPosition(_marquee.end);

  if(start[0] > end[0])std::swap(start[0], end[0]);
  if(start[1] > end[1])std::swap(start[1], end[1]);

  ClearSelection();
  for (auto& node : _nodes) {
    if (node->Intersect(start, end)) AddToSelection(node, false);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE