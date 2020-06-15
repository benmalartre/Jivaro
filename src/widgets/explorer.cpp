#pragma once

#include "explorer.h"
#include "../app/application.h"
#include "../app/window.h"
#include "../app/view.h"
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

  _parent->SetDirty();
  _visibleIcon = &AMN_ICONS["visible.png"];
  _invisibleIcon = &AMN_ICONS["invisible.png"];
}

// destructor
ExplorerUI::~ExplorerUI()
{
}

void ExplorerUI::Init()
{
  Update();
  _parent->SetDirty();
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
  if (item->_selected) {
    drawList->AddRectFilled(
      { 0, pos.y },
      { ImGui::GetWindowWidth(), pos.y + AMN_EXPLORER_LINE_HEIGHT },
      ImColor(_selectedColor));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { 0, pos.y },
        { ImGui::GetWindowWidth(), pos.y + AMN_EXPLORER_LINE_HEIGHT },
        ImColor(_backgroundColor));
    else
      drawList->AddRectFilled(
        { 0, pos.y },
        { ImGui::GetWindowWidth(), pos.y + AMN_EXPLORER_LINE_HEIGHT },
        ImColor(_alternateColor));
  }

  ImGui::SetCursorPos(ImVec2(0, pos.y + AMN_EXPLORER_LINE_HEIGHT));
  if (item->_expanded) {
    for (const auto child : item->_items) {
      flip = !flip;
      DrawItemBackground(drawList, child, flip);
    }
  }
}

void ExplorerUI::DrawBackground()
{
  
  auto* drawList = ImGui::GetBackgroundDrawList();
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
    { 0, -scrollOffsetV },
    { ImGui::GetWindowWidth(), -scrollOffsetV + AMN_EXPLORER_LINE_HEIGHT },
    ImColor(1, 0, 0, 1));

  ImGui::SetCursorPos(ImVec2(0, -scrollOffsetV + AMN_EXPLORER_LINE_HEIGHT));
  DrawItemBackground(drawList, _root, flip);
  ImGui::SetCursorPos(ImVec2(0, AMN_EXPLORER_LINE_HEIGHT));

  drawList->PopClipRect();
  
}

void ExplorerUI::DrawItemType(ExplorerItem* item)
{
  ImGui::Text(item->_prim.GetTypeName().GetText());
  ImGui::NextColumn();
}

void ExplorerUI::DrawItemVisibility(ExplorerItem* item, bool heritedVisibility)
{
  GLuint tex = item->_visible ? _visibleIcon->tex : _invisibleIcon->tex;
  ImVec4 col = heritedVisibility ?
    ImVec4(0, 0, 0, 1) : ImVec4(0.33, 0.33, 0.33, 1);

  ImGui::ImageButton(
    (void*)tex,
    ImVec2(16, 16),
    ImVec2(0, 0),
    ImVec2(1, 1),
    0,
    ImVec4(0, 0, 0, 0),
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
  //if (!_active)return false;
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  Application* app = GetApplication();
  
  if (app->GetStage())
  {
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
    for (int i = 0; i<numColorIDs; ++i)
      ImGui::PushStyleColor(
        colorIDs[i],
        ImVec4(0, 0, 0, 0));
    
    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, GetWidth() - 100);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 40);
    
    // draw title
    ImGui::PushFont(GetWindow()->GetBoldFont(0));
    ImGui::Text("Prim name");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopFont();
    
    DrawBackground();
    
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    DrawItem(_root, true);
    ImGui::PopFont();
    
    ImGui::PopStyleColor(numColorIDs);
  }
  
  ImGui::End();

  return
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
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