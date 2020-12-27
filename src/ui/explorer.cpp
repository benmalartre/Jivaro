#include "explorer.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../ui/style.h"
#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/tokens.h>

AMN_NAMESPACE_OPEN_SCOPE

// constructor
ExplorerUI::ExplorerUI(View* parent) 
  : BaseUI(parent, "Explorer")
  , _root(NULL)
  , _visibleIcon(NULL)
  , _invisibleIcon(NULL)
{
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoMove;

  _selectBaseFlags =
    ImGuiTreeNodeFlags_OpenOnArrow |
    ImGuiTreeNodeFlags_OpenOnDoubleClick;

  _parent->SetDirty();
  _visibleIcon = &AMN_ICONS[AMN_ICON_SMALL]["visible.png"];
  _invisibleIcon = &AMN_ICONS[AMN_ICON_SMALL]["invisible.png"];

  pxr::TfWeakPtr<ExplorerUI> me(this);
  pxr::TfNotice::Register(me, &BaseUI::ProcessNewScene);
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
      { 0, pos.y },
      { width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
      ImColor(AMN_SELECTED_COLOR));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
        ImColor(AMN_BACKGROUND_COLOR));
    else
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + AMN_EXPLORER_LINE_HEIGHT },
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

void ExplorerUI::DrawBackground()
{
  auto* drawList = ImGui::GetWindowDrawList();
  const auto& style = ImGui::GetStyle();

  float scrollOffsetH = ImGui::GetScrollX();
  float scrollOffsetV = ImGui::GetScrollY();

  ImVec2 clipRectMin(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
  ImVec2 clipRectMax(clipRectMin.x + ImGui::GetWindowWidth(),
    clipRectMin.y + ImGui::GetWindowHeight());

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax.y -= style.ScrollbarSize;
  }
 
  drawList->PushClipRect(clipRectMin, clipRectMax);
  bool flip = false;

  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
    ImColor(AMN_BACKGROUND_COLOR)
  );

  ImGui::SetCursorPos(
    ImVec2(
      _parent->GetX(), 
      _parent->GetY() - scrollOffsetV + AMN_EXPLORER_LINE_HEIGHT));

  for (auto& item : _root->_items) {
    DrawItemBackground(drawList, item, flip);
    flip = !flip;
  }
  ImGui::SetCursorPos(ImVec2(0, AMN_EXPLORER_LINE_HEIGHT));

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
  GLuint tex = item->_visible ? _visibleIcon->tex : _invisibleIcon->tex;
  ImVec4 col = heritedVisibility ?
    style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];

  ImGui::ImageButton(
    (void*)(size_t)tex,
    ImVec2(16, 16),
    ImVec2(0, 0),
    ImVec2(1, 1),
    0,
    AMN_TRANSPARENT_COLOR,
    col);

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
}

void ExplorerUI::DrawItem(ExplorerItem* current, bool heritedVisibility)
{
  if (!current) return;
  ImGuiTreeNodeFlags itemFlags = _selectBaseFlags;

  if (current->_selected) {
    itemFlags |= ImGuiTreeNodeFlags_Selected;
  }

  // parent
  if (current->_items.size())
  {
    bool currentOpen =
      ImGui::TreeNodeEx(
        current->_prim.GetPath().GetText(),
        itemFlags,
        current->_prim.GetName().GetText());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      current->_selected = !current->_selected;

    ImGui::NextColumn();

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);

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

    ImGui::TreeNodeEx(
      current->_prim.GetPath().GetText(),
      itemFlags,
      current->_prim.GetName().GetText());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      current->_selected = true;
    current->_expanded = false;

    ImGui::NextColumn();
    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
  }
}

bool ExplorerUI::Draw()
{
  if (!_initialized)Init();

  /*
  // setup colors
  const size_t numColorIDs = 7;
  int colorIDs[numColorIDs] = {
    ImGuiCol_WindowBg,
    ImGuiCol_Header,
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonActive,
    ImGuiCol_ButtonHovered
  };
  
  for (size_t ic = 0; ic<numColorIDs; ++ic)
    ImGui::PushStyleColor(
      colorIDs[ic],
      ImVec4(0, 0, 0, 0));
      */
  //if (!_active)return false;
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  Application* app = GetApplication();
  
  if (app->GetStage())
  {
   
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
    
    DrawBackground();
    
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    for (auto& item : _root->_items) {
      DrawItem(item, true);
    }
    ImGui::PopFont();
  }
  
  ImGui::End();

  //ImGui::PopStyleColor(numColorIDs);

  return true;
  /*
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