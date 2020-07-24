#include "graph.h"
#include "node.h"
#include "../graph/stage.h"
#include "../graph/node.h"
#include "../graph/input.h"
#include "../graph/output.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../ui/ui.h"
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/attribute.h>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
GraphUI::GraphUI(View* parent, const std::string& filename, bool docked)
  : BaseUI(parent, "Graph", docked)
  , _hoveredNode(NULL), _currentNode(NULL)
  , _hoveredPort(NULL), _currentPort(NULL), _hoveredConnexion(NULL)
  , _scale(1.f), _fontIndex(0), _fontScale(1.0), _offset(pxr::GfVec2f(0.f, 0.f))
  , _drag(false), _grab(false), _navigate(false), _connect(false)
{
  //_filename = filename;
  _id = 0;
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoNav
    | ImGuiWindowFlags_NoScrollWithMouse
    | ImGuiWindowFlags_NoScrollbar;

  //GraphTreeUI* tree = new GraphTreeUI();
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory();

  for (int i = 0; i < 12; ++i) {
    pxr::UsdPrim prim =
      stage->DefinePrim(pxr::SdfPath(pxr::TfToken("/node" + std::to_string(i))));
    NodeUI* node = new NodeUI(prim);
    node->SetPosition(pxr::GfVec2f(
      rand() / 255,
      rand() / 255));
    node->SetSize(pxr::GfVec2f(120, 32));

    node->AddInput("Input1", pxr::SdfValueTypeNames->Vector3f);
    node->AddInput("Input2", pxr::SdfValueTypeNames->Vector3f);
    node->AddOutput("Output", pxr::SdfValueTypeNames->Vector3f);
    
    _nodes.push_back(node);
  }
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

  DrawGrid();

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
    const pxr::GfVec2f startTangent(startPos[0] * 0.75f + endPos[0] * 0.25f, startPos[1]);
    const pxr::GfVec2f endTangent(startPos[0] * 0.25f + endPos[0] * 0.75f, endPos[1]);

    drawList->AddBezierCurve(
      startPos, 
      startTangent, 
      endTangent, 
      endPos, 
      _connector.color, 
      NODE_CONNEXION_THICKNESS * _scale);
  }
  else if (_grab) {
    const pxr::GfVec2f viewPos = GetPosition();
    drawList->AddRect(
      _grabData.start + viewPos,
      _grabData.end + viewPos,
      ImColor(255, 128, 0, 255),
      0.f, 0, 2);

    drawList->AddRectFilled(
      _grabData.start + viewPos,
      _grabData.end + viewPos,
      ImColor(255, 128, 0, 64),
      0.f, 0);
  }

  /*
  ImGui::SetCursorPos(ImVec2(64, 64));
  static bool value;
  ImGui::Checkbox("FUCK", &value);
  */
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
      _trees.push_back(new GraphTreeUI(usdStage));
    }
  }
  */
  
  
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
    NodeUI nodeUI(child);

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

  NodeUI* hovered = NULL;
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

void GraphUI::_GetPortUnderMouse(const pxr::GfVec2f& mousePos, NodeUI* node)
{
  const pxr::GfVec2f relativePosition =
    ViewPositionToGridPosition(mousePos) - node->GetPosition();

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
      if (mods & GLFW_MOD_ALT)_navigate = true;
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
          GetRelativeMousePosition(mousePos[0], mousePos[1], _grabData.start[0], _grabData.start[1]);
          _grabData.end = _grabData.start;
          _grab = true;
        }
      }
    }
    else if (action == GLFW_RELEASE) {
      _navigate = false;
      _drag = false;
      if (_connect) EndConnexion();
      else if (_grab)GrabSelect(mods);
      _grab = false;
    }
  }
  _parent->SetDirty();
}

void GraphUI::Keyboard(int key, int scancode, int action, int mods)
{
  std::cout << "GRAPH UI KEYBOARD!!! " << std::endl;
  std::cout << "KEY : " << key << (char)key << std::endl;
  if (key == GLFW_KEY_DELETE) {
    std::cout << "GRAPH UI : DELETE SELECTED NODES !!! " << std::endl;
  }
}

void GraphUI::MouseMove(int x, int y)
{
  if (_navigate) {
    _offset += pxr::GfVec2f(x - _lastX, y - _lastY) / _scale;
    _parent->SetDirty();
  }
  else if (_drag) {
    for (auto& node : _selected) {
      node->Move(pxr::GfVec2f(x - _lastX, y - _lastY) / _scale);
    }
    _parent->SetDirty();
  }
  else if (_grab) {
    GetRelativeMousePosition(x, y, _grabData.end[0], _grabData.end[1]);
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
  _connector.endPort = _hoveredPort;
}

void GraphUI::EndConnexion()
{
  if (_connector.startPort && _connector.endPort) {
    ConnexionUI* connexion = new ConnexionUI(_connector.startPort, 
      _connector.endPort, _connector.color);
    _connexions.push_back(connexion);
  }
  _connect = false;
}

void GraphUI::ClearSelection()
{
  if(_selected.size())
    for (auto& node : _selected)
      node->SetState(ITEM_STATE_SELECTED, false);
  _selected.clear();
}

void GraphUI::AddToSelection(NodeUI* node, bool bringToFront) 
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

void GraphUI::RemoveFromSelection(NodeUI* node)
{
  node->SetState(ITEM_STATE_SELECTED, false);
  _selected.erase(node);
}

void GraphUI::GrabSelect(int mod)
{
  pxr::GfVec2f start = ViewPositionToGridPosition(_grabData.start);
  pxr::GfVec2f end = ViewPositionToGridPosition(_grabData.end);

  if(start[0] > end[0])std::swap(start[0], end[0]);
  if(start[1] > end[1])std::swap(start[1], end[1]);

  ClearSelection();
  for (auto& node : _nodes) {
    if (node->Intersect(start, end)) AddToSelection(node, false);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE