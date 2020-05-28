#include "graph.h"
#include "node.h"
#include "../graph/stage.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/ui.h"
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/attribute.h>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
GraphUI::GraphUI(View* parent, const std::string& filename, bool docked)
  : BaseUI(parent, "Graph", docked)
  , _hoveredNode(NULL), _currentNode(NULL), _drawingNode(NULL), _zoom(1.f)
  , _fontIndex(0), _fontScale(1.0), _drag(false), _grab(false), _navigate(false)
{
  //_filename = filename;
  _id = 0;
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoNav
    | ImGuiWindowFlags_NoScrollWithMouse
    | ImGuiWindowFlags_NoScrollbar;

  //GraphTreeUI* tree = new GraphTreeUI();
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory();

  for (int i = 0; i < 1024; ++i) {
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
  if (_zoom < 1.0) {
    _fontIndex = 0;
    _fontScale = _zoom;
  }
  else if (_zoom < 2.0) {
    _fontIndex = 1;
    _fontScale = RESCALE(_zoom, 1.0, 2.0, 0.5, 1.0);
  }
  else {
    _fontIndex = 2;
    _fontScale = RESCALE(_zoom, 2.0, 4.0, 0.5, 1.0);
  }
}

// draw grid
//------------------------------------------------------------------------------
void GraphUI::DrawGrid()
{
  const float width = GetWidth();
  const float height = GetHeight();
  const float baseX = _parent->GetX();// +_offset[0];
  const float baseY = _parent->GetY();// +_offset[1];

  ImGui::PushClipRect(ImVec2(baseX, baseY), ImVec2(baseX + width, baseY + height), true);

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(baseX, baseY),
    ImVec2(baseX + width, baseY + height),
    ImColor(60, 60, 60, 255));

  float step = 32 * _zoom;
  int nX = width / step + 1;
  int nY = height / step + 1;
  pxr::GfVec2f so = _offset * _zoom;
  for (int x = 0; x < nX; ++x)
    drawList->AddLine(
      ImVec2(baseX + x*step + so[0], baseY),
      ImVec2(baseX + x*step + so[0], baseY + height),
      ImColor(255, 0, 0, 255));

  for (int y = 0; y < 32; ++y)
    drawList->AddLine(
      ImVec2(baseX, baseY + y*step + so[1]), 
      ImVec2(baseX + width, baseY + y*step + so[1]), 
      ImColor(0, 255, 0, 255));
    
  ImGui::PopClipRect();
}

void GraphUI::DrawTxt()
{
  ImGui::SetCursorPos(_offset * _zoom);
  ImGui::Text("ZOOM : %.3f", _zoom);
}


// draw
//------------------------------------------------------------------------------
bool GraphUI::Draw()
{
  std::cout << "DRAW GRAPH UI !!!" << std::endl;

  ImGui::SetNextWindowPos(_parent->GetMin());
  ImGui::SetNextWindowSize(_parent->GetSize());
  ImGui::Begin("Graph Editor", NULL, _flags);

  DrawGrid();

  ImGui::PushFont(GetWindow()->GetRegularFont(_fontIndex));
  ImGui::SetWindowFontScale(_fontScale);
  DrawTxt();

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  for (auto& node : _nodes) {
    _drawingNode = node;
    node->Draw(this);
  }

  if (_grab) {
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

  ImGui::PopFont();
  ImGui::End();
 
  return false;
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
  //_context = ImNodes::EditorContextCreate();
  
  
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
    pxr::GfVec2f pos = node->GetPos();
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

void GraphUI::MouseButton(int button, int action, int mods)
{
  const ImVec2& mousePos = ImGui::GetMousePos();
  _lastX = mousePos.x;
  _lastY = mousePos.y;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT)_navigate = true;
      else if (_hoveredNode) {
        if (_hoveredNode->GetState(ITEM_STATE_SELECTED))
          _drag = true;
        else {
          bool state = _hoveredNode->GetState(ITEM_STATE_SELECTED);
          _hoveredNode->SetState(ITEM_STATE_SELECTED, 1 - state);
          if (mods & GLFW_MOD_CONTROL) {
            if (1 - state) {
              _drag = true;
              AddToSelection(_hoveredNode, false);
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
              _drag = true;
              AddToSelection(_hoveredNode, true);
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
      if (_grab)GrabSelect(mods);
      _grab = false;
    }
  }
  _parent->SetDirty();
}

void GraphUI::MouseMove(int x, int y)
{
  if (_navigate) {
    _offset += pxr::GfVec2f(x - _lastX, y - _lastY) / _zoom;
    _parent->SetDirty();
  }
  else if (_drag) {
    for (auto& node : _selected) {
      node->Move(pxr::GfVec2f(x - _lastX, y - _lastY) / _zoom);
    }
    _parent->SetDirty();
  }
  else if (_grab) {
    GetRelativeMousePosition(x, y, _grabData.end[0], _grabData.end[1]);
    _parent->SetDirty();
  }
  else {
    const pxr::GfVec2f mousePos = ImGui::GetMousePos();
    pxr::GfVec2f viewPos;
    GetRelativeMousePosition(mousePos[0], mousePos[1], viewPos[0], viewPos[1]);
    NodeUI* hovered = NodeUnderMouse(viewPos);
    if (_hoveredNode && _hoveredNode != hovered)_hoveredNode->SetState(ITEM_STATE_HOVERED, false);
    if (hovered) {
      hovered->SetState(ITEM_STATE_HOVERED, true);
      _hoveredNode = hovered;
    }
    else _hoveredNode = NULL;
  }
  _lastX = x;
  _lastY = y;
  
}

void GraphUI::MouseWheel(int x, int y)
{
  pxr::GfVec2f mousePos = (ImGui::GetMousePos() - _parent->GetMin());
  pxr::GfVec2f originalPos = mousePos / _zoom ;

  _zoom += (x + y) * 0.1;
  _zoom = CLAMP(_zoom, 0.1, 4.0);
  pxr::GfVec2f scaledPos = mousePos / _zoom;
  _offset -= (originalPos - scaledPos);
  UpdateFont();
  _parent->SetDirty();
}

pxr::GfVec2f GraphUI::ViewPositionToGridPosition(const pxr::GfVec2f& mousePos)
{
  return mousePos / _zoom - _offset;
}

NodeUI* GraphUI::NodeUnderMouse(const pxr::GfVec2f& mousePos)
{
  pxr::GfVec2f gridPosition = ViewPositionToGridPosition(mousePos);

  for (auto node = _nodes.rbegin(); node != _nodes.rend(); ++node) {
    if ((*node)->Contains(gridPosition)) return *node;
  }
  return NULL;
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
  const pxr::GfVec2f start = ViewPositionToGridPosition(_grabData.start);
  const pxr::GfVec2f end = ViewPositionToGridPosition(_grabData.end);

  ClearSelection();
  for (auto& node : _nodes) {
    if (node->Intersect(start, end)) AddToSelection(node, false);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE