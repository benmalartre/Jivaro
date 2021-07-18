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


AMN_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ExplorerUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove;

ImGuiTreeNodeFlags ExplorerUI::_treeFlags =
  ImGuiTreeNodeFlags_OpenOnArrow |
  ImGuiTreeNodeFlags_OpenOnDoubleClick;

ImGuiSelectableFlags ExplorerUI::_itemFlags = 
  ImGuiSelectableFlags_SpanAllColumns |
  ImGuiSelectableFlags_SpanAvailWidth |
  ImGuiSelectableFlags_SelectOnClick;

// constructor
ExplorerUI::ExplorerUI(View* parent) 
  : BaseUI(parent, "Explorer")
  , _root(NULL)
  , _visibleIcon(NULL)
  , _invisibleIcon(NULL)
{
  _parent->SetDirty();
  _visibleIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_VISIBLE];
  _invisibleIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_INVISIBLE];

  pxr::TfWeakPtr<ExplorerUI> me(this);
  //pxr::TfNotice::Register(me, &BaseUI::ProcessNewScene);
}

// destructor
ExplorerUI::~ExplorerUI()
{
}

void ExplorerUI::Init()
{
  Update();
  _parent->SetDirty();
  _initialized = true;
}


void ExplorerUI::MouseButton(int action, int button, int mods)
{
}

void ExplorerUI::MouseMove(int x, int y)
{
}

void ExplorerUI::Update()
{
  if (GetApplication()->GetStage()) {
    RecurseStage();
  }
}

void ExplorerUI::DrawItemBackground(ImDrawList* drawList,
  const ExplorerItem* item, bool& flip)
{
  if (!item) return;
  ImVec2 pos = ImGui::GetCursorPos();
  const float width = (float)GetWidth();
  if (item->_selected) {
    drawList->AddRectFilled(
      { pos.x, pos.y },
      { pos.x + width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
      ImColor(AMN_SELECTED_COLOR));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
        ImColor(AMN_BACKGROUND_COLOR));
    else
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
        ImColor(AMN_ALTERNATE_COLOR));
  }

  ImGui::SetCursorPos(ImVec2(pos.x, pos.y + AMN_EXPLORER_LINE_HEIGHT));
  if (item->_expanded) {
    for (const auto child : item->_items) {
      flip = !flip;
      DrawItemBackground(drawList, child, flip);
    }
  }
}

void ExplorerUI::DrawBackground(float localMouseX, float localMouseY)
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const auto& style = ImGui::GetStyle();

  float scrollOffsetH = ImGui::GetScrollX();
  float scrollOffsetV = ImGui::GetScrollY();

  ImVec2 clipRectMin = _parent->GetMin();
  ImVec2 clipRectMax = _parent->GetMax();

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax.x += style.ScrollbarSize;
  }
 
  drawList->PushClipRect(clipRectMin, clipRectMax);
  
  bool flip = false;
  
  ImGui::SetCursorPos(
    ImVec2(
      clipRectMin.x,
      clipRectMin.y - scrollOffsetV + AMN_EXPLORER_LINE_HEIGHT));

  for (auto& item : _root->_items) {
    DrawItemBackground(drawList, item, flip);
    flip = !flip;
  }
  
  ImGui::SetCursorPos(ImVec2(0, AMN_EXPLORER_LINE_HEIGHT));

  drawList->AddCircleFilled(ImVec2(localMouseX, localMouseY) + _parent->GetMin(), 12.f, ImColor(0.f, 1.f, 0.f, 0.5f), 32);

  drawList->PopClipRect();
}

void ExplorerUI::DrawItemType(ExplorerItem* item)
{
  ImGui::Text("%s", item->_prim.GetTypeName().GetText());
  ImGui::NextColumn();
}

void ExplorerUI::DrawItemVisibility(ExplorerItem* item, bool heritedVisibility)
{
  const ImGuiStyle& style = ImGui::GetStyle();
  short state = 
    item->_selected ? AMN_ICON_SELECTED : AMN_ICON_DEFAULT;
  GLuint tex = 
    item->_visible ? _visibleIcon->tex[state] : _invisibleIcon->tex[state];
  const ImVec4& sel_col = 
    item->_selected ? AMN_TEXT_SELECTED_COLOR : AMN_TEXT_DEFAULT_COLOR;
  const ImVec4& col = 
    heritedVisibility ? sel_col : style.Colors[ImGuiCol_TextDisabled];
  ImGui::PushStyleColor(ImGuiCol_Button, AMN_TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, AMN_TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, AMN_TRANSPARENT_COLOR);
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

void ExplorerUI::DrawItem(ExplorerItem* current, bool heritedVisibility)
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
    key += "Selectable";
    if (ImGui::Selectable(key.c_str(), current->_selected, _itemFlags,
      ImVec2(GetWidth(), AMN_EXPLORER_LINE_HEIGHT))) {
      if (!current->_selected) {
        AMN_APPLICATION->AddToSelection(current->_prim.GetPath());
      }
      else {
        AMN_APPLICATION->RemoveFromSelection(current->_prim.GetPath());
      }
      current->_selected = !current->_selected;
    }

    if(current->_selected) {
      ImGui::PushStyleColor(ImGuiCol_Text, AMN_TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, AMN_TEXT_DEFAULT_COLOR);
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
       
    if (ImGui::IsItemClicked()) {
      if(!current->_selected) {
        AMN_APPLICATION->AddToSelection(current->_prim.GetPath());
      } else {
        AMN_APPLICATION->RemoveFromSelection(current->_prim.GetPath());
      }
      current->_selected = !current->_selected;
      //Notice::SelectionChanged().Send();
    }

    if(current->_selected) {
      ImGui::PushStyleColor(ImGuiCol_Text, AMN_TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, AMN_TEXT_DEFAULT_COLOR);
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

bool ExplorerUI::Draw()
{
  if (!_initialized)Init();

  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  const ImVec2 localMousePos = ImGui::GetMousePos() - _parent->GetMin();

  Application* app = GetApplication();
  
  if (app->GetStage())
  {
    // setup transparent background
    ImGui::PushStyleColor(ImGuiCol_Header, AMN_TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, AMN_TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, AMN_TRANSPARENT_COLOR);

    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, _parent->GetWidth() - 100);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 40);
    
    // draw title
    ImGui::PushFont(GetWindow()->GetBoldFont(0));
    ImGui::Text("Prim");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopFont();
    
    DrawBackground(localMousePos.x, localMousePos.y);
    
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    for (auto& item : _root->_items) {
      DrawItem(item, true);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    
  }
  
  ImGui::End();

  

  return true;/*
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();*/
}


void ExplorerUI::RecurseStage()
{
  if (_root)delete _root;
  _root = new ExplorerItem();
  _root->_expanded = true;
  Application* app = GetApplication();
  if (app->GetStage()) {
    _root->_prim = app->GetStage()->GetPseudoRoot();
    _root->_visible = true;
    RecursePrim(_root);
  }
}

void ExplorerUI::RecursePrim(ExplorerItem* currentItem)
{
  for (const auto& childPrim : currentItem->_prim.GetChildren())
  {
    ExplorerItem* childItem = currentItem->AddItem();
    childItem->_expanded = true;
    childItem->_prim = childPrim;
    pxr::UsdAttribute visibilityAttr =
      pxr::UsdGeomImageable(childPrim).GetVisibilityAttr();
    if (visibilityAttr.IsValid())
    {
      pxr::TfToken visible;
      visibilityAttr.Get(&visible);
      if (visible == pxr::UsdGeomTokens->invisible)currentItem->_visible = false;
      else childItem->_visible = true;
    }
    RecursePrim(childItem);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE