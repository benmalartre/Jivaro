#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/tokens.h>
#include "../ui/style.h"
#include "../ui/explorer.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/window.h"
#include "../app/view.h"


PXR_NAMESPACE_OPEN_SCOPE

// static data
ImGuiWindowFlags ExplorerUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove;

ImGuiTreeNodeFlags ExplorerUI::_treeFlags =
  ImGuiTreeNodeFlags_OpenOnArrow |
  ImGuiTreeNodeFlags_OpenOnDoubleClick |
  ImGuiTreeNodeFlags_SpanAvailWidth;

// constructor
ExplorerUI::ExplorerUI(View* parent) 
  : BaseUI(parent, "Explorer")
  , _root(NULL)
  , _current(NULL)
  , _last(NULL)
{
  _parent->SetDirty();
}

// destructor
ExplorerUI::~ExplorerUI()
{
}

void 
ExplorerUI::OnSceneChangedNotice(const SceneChangedNotice& n)
{
  Update();
}

void
ExplorerUI::OnSelectionChangedNotice(const SelectionChangedNotice& n)
{
  Select();
}

void 
ExplorerUI::Init()
{
  Update();
  _parent->SetDirty();
  _initialized = true;
}


void 
ExplorerUI::MouseButton(int button, int action, int mods)
{
  Application* app = GetApplication();
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_RELEASE) {
      if (_current) {
        _current->_selected = 1 - _current->_selected;
        if (mods & GLFW_MOD_CONTROL) {
          app->ToggleSelection({ _current->_prim.GetPath() });
        } else {
          app->SetSelection({ _current->_prim.GetPath() });
        }
      }
    }
  }
}

void 
ExplorerUI::MouseMove(int x, int y)
{
}

void
ExplorerUI::Keyboard(int key, int scancode, int action, int mods)
{
}

void 
ExplorerUI::Update()
{
  if (GetApplication()->GetStage()) {
    RecurseStage();
  }
}

void
ExplorerUI::Select()
{
  Selection* selection = GetApplication()->GetSelection();
  const size_t selectionHash = selection->GetHash();

  if (_selectionHash != selectionHash) {
    if (GetApplication()->GetStage()) {
      for (auto& item : _map) {
        item.second->_selected = false;
      }
      for (auto& selected : selection->GetItems()) {
        if (_map.find(selected.path) != _map.end()) {
          _map[selected.path]->_selected = true;
        }
      }
    }
    _selectionHash = selectionHash;
  }
}

void 
ExplorerUI::DrawItemBackground(ImDrawList* drawList,
  const ExplorerItem* item, bool& flip)
{
  if (!item) return;
  ImVec2 pos = ImGui::GetCursorPos();
  const float width = (float)GetWidth();
  if (item->_selected) {
    drawList->AddRectFilled(
      { pos.x, pos.y },
      { pos.x + width, pos.y + EXPLORER_LINE_HEIGHT },
      ImColor(SELECTED_COLOR));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + EXPLORER_LINE_HEIGHT },
        ImColor(BACKGROUND_COLOR));
    else
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + EXPLORER_LINE_HEIGHT },
        ImColor(ALTERNATE_COLOR));
  }

  ImGui::SetCursorPos(ImVec2(pos.x, pos.y + EXPLORER_LINE_HEIGHT));
  if (item->_expanded) {
    for (const auto child : item->_items) {
      flip = !flip;
      DrawItemBackground(drawList, child, flip);
    }
  }
}

void 
ExplorerUI::DrawBackground(float localMouseX, float localMouseY)
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const auto& style = ImGui::GetStyle();

  float scrollOffsetH = ImGui::GetScrollX();
  float scrollOffsetV = ImGui::GetScrollY();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  const pxr::GfVec2f max(min + size);

  ImVec2 clipRectMin = min;
  ImVec2 clipRectMax = max;

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax.x += style.ScrollbarSize;
  }
 
  drawList->PushClipRect(clipRectMin, clipRectMax);
  
  bool flip = false;
  
  ImGui::SetCursorPos(
    ImVec2(
      clipRectMin.x,
      clipRectMin.y - scrollOffsetV + EXPLORER_LINE_HEIGHT));

  for (auto& item : _root->_items) {
    DrawItemBackground(drawList, item, flip);
    flip = !flip;
  }

  drawList->PopClipRect();
}

void 
ExplorerUI::_UpdateSelection(ExplorerItem* item, bool isLeaf)
{
  Application* app = GetApplication();
  if (isLeaf ? ImGui::IsItemClicked() : 
    ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    _last = _current;
    _current = item;
  }
}

void 
ExplorerUI::DrawItemType(ExplorerItem* item)
{
  ImGui::Text("%s", item->_prim.GetTypeName().GetText());

  _UpdateSelection(item, !item->_items.size());
  ImGui::NextColumn();
}

void
ExplorerUI::DrawItemVisibility(ExplorerItem* item, bool heritedVisibility)
{
  const ImGuiStyle& style = ImGui::GetStyle();
  short state =  item->_selected ? ICON_SELECTED : ICON_DEFAULT;

  const Icon* visibleIcon = &ICONS[ICON_SIZE_SMALL][ICON_VISIBLE];
  const Icon* invisibleIcon = &ICONS[ICON_SIZE_SMALL][ICON_INVISIBLE];
  GLuint tex = 
    item->_visible ? visibleIcon->tex[state] : invisibleIcon->tex[state];
  const ImVec4& sel_col = 
    item->_selected ? TEXT_SELECTED_COLOR : TEXT_DEFAULT_COLOR;
  const ImVec4& col = 
    heritedVisibility ? sel_col : style.Colors[ImGuiCol_TextDisabled];
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, TRANSPARENT_COLOR);
  ImGui::ImageButton(
    (void*)(size_t)tex, ImVec2(16, 16), ImVec2(0, 0), ImVec2(1, 1));

  if (ImGui::IsItemClicked()) {
    item->_visible = !item->_visible;
    pxr::UsdGeomImageable imageable(item->_prim);
    if (imageable) {
      if (item->_visible)imageable.MakeVisible();
      else imageable.MakeInvisible();
      GetWindow()->ForceRedraw();
    }
  }

  ImGui::NextColumn();
  ImGui::PopStyleColor(3);
}

void 
ExplorerUI::DrawItem(ExplorerItem* current, bool heritedVisibility)
{
  if (!current) return;
  ImGuiTreeNodeFlags itemFlags = _treeFlags;

  if (current->_selected) {
    itemFlags |= ImGuiTreeNodeFlags_Selected;
  }

  // parent
  if (current->_items.size())
  {
    std::string key = "##" + current->_prim.GetPath().GetString();
    bool currentOpen = ImGui::TreeNodeEx(key.c_str(), itemFlags);
    _UpdateSelection(current, false);

    if(current->_selected) {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_DEFAULT_COLOR);
    }
    ImGui::SameLine();
    ImGui::Text("%s", current->_prim.GetName().GetText());
    ImGui::NextColumn();

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
    ImGui::PopStyleColor();

    if (currentOpen)
    {
      current->_expanded = true;
      for (const auto item : current->_items)
        DrawItem(item, current->_visible && heritedVisibility);
      ImGui::TreePop();
    }
    else current->_expanded = false;
  }
  // leaf
  else
  {
    itemFlags |= ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::string key = "##" + current->_prim.GetPath().GetString();
    ImGui::TreeNodeEx(key.c_str(), itemFlags);
       
    _UpdateSelection(current, true);

    if(current->_selected) {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_DEFAULT_COLOR);
    }

    ImGui::SameLine();
    ImGui::Text("%s", current->_prim.GetName().GetText());
    ImGui::NextColumn();

    current->_expanded = false;

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
    ImGui::PopStyleColor();
  }
}

bool 
ExplorerUI::Draw()
{
  if (!_initialized)Init();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);

  const ImVec2 localMousePos = ImGui::GetMousePos() - min;

  Application* app = GetApplication();
  
  if (app->GetStage())
  {
    // setup transparent background
    ImGui::PushStyleColor(ImGuiCol_Header, TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, TRANSPARENT_COLOR);

    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, GetWidth() - 100);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 40);
    
    // draw title
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    ImGui::Text("Prim");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopFont();
    
    DrawBackground(localMousePos.x, localMousePos.y);
    
    ImGui::PushFont(GetWindow()->GetRegularFont(0));
    ImGui::SetCursorPos(pxr::GfVec2f(0.f, EXPLORER_LINE_HEIGHT));
    for (auto& item : _root->_items) {
      DrawItem(item, true);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    
  }
  
  ImGui::End();

  

  return 
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
}


void 
ExplorerUI::RecurseStage()
{
  if (_root)delete _root;
  _root = new ExplorerItem();
  _root->_expanded = true;
  Application* app = GetApplication();
  if (app->GetStage()) {
    _root->_prim = app->GetStage()->GetPseudoRoot();
    _root->_visible = true;
    _map[_root->_prim.GetPath()] = _root;
    RecursePrim(_root);
  }
}

void
ExplorerUI::RecursePrim(ExplorerItem* currentItem)
{
  Selection* selection = GetApplication()->GetSelection();
  for (auto& childPrim : currentItem->_prim.GetChildren())
  {
    pxr::UsdAttribute visibilityAttr =
      pxr::UsdGeomImageable(childPrim).GetVisibilityAttr();
    bool visible = true;
    if (visibilityAttr.IsValid())
    {
      pxr::TfToken visibility;
      visibilityAttr.Get(&visibility);
      if (visibility == pxr::UsdGeomTokens->invisible)visible = false;
      else visible = true;
    }
    ExplorerItem* childItem = currentItem->AddItem(childPrim, visible, false, false);
    _map[childPrim.GetPath()] = childItem;
    RecursePrim(childItem);
  }
}

PXR_NAMESPACE_CLOSE_SCOPE