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
  : HeadedUI(parent, "Explorer")
  , _counter(0)
{
  _parent->SetDirty();
}

// destructor
ExplorerUI::~ExplorerUI()
{
}

/*
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
*/

/*
void 
ExplorerUI::Init()
{
  //Update();
  _parent->SetDirty();
  _initialized = true;
}
*/

void 
ExplorerUI::MouseButton(int button, int action, int mods)
{
  Application* app = GetApplication();
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_RELEASE) {
      if (app->GetStage()->GetPrimAtPath(_current).IsValid()) {
        if (mods & GLFW_MOD_CONTROL) {
          app->ToggleSelection({ _current });
        } else {
          app->SetSelection({ _current });
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

/*
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
        BITMASK_CLEAR(item.second->flags, ExplorerUI::Item::SELECTED);
      }
      for (auto& selected : selection->GetItems()) {
        if (_map.find(selected.path) != _map.end()) {
          BITMASK_SET(_map[selected.path]->flags, ExplorerUI::Item::SELECTED);
        }
      }
    }
    _selectionHash = selectionHash;
  }
}

void 
ExplorerUI::DrawItemBackground(ImDrawList* drawList,
  ExplorerUI::Item* item, bool& flip)
{
  ImVec2 pos = ImGui::GetCursorPos();
  const float width = (float)GetWidth();
  if (BITMASK_CHECK(item->flags, ExplorerUI::Item::SELECTED)) {
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
  if (BITMASK_CHECK(item->flags, ExplorerUI::Item::EXPANDED)) {
    for (auto child : item->items) {
      flip = !flip;
      DrawItemBackground(drawList, &child, flip);
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

  for (auto& item : _root->items) {
    DrawItemBackground(drawList, &item, flip);
    flip = !flip;
  }

  drawList->PopClipRect();
}

void 
ExplorerUI::_UpdateSelection(ExplorerUI::Item* item, bool isLeaf)
{
  Application* app = GetApplication();
  if (isLeaf ? ImGui::IsItemClicked() : 
    ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    _last = _current;
    _current = item;
  }
}
*/
void 
ExplorerUI::DrawType(const pxr::UsdPrim& prim, bool selected)
{
  ImGui::Text("%s", prim.GetTypeName().GetText());
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    _current = prim.GetPath();
  }
  ImGui::NextColumn();
}

void
ExplorerUI::DrawVisibility(const pxr::UsdPrim& prim, bool visible, bool selected)
{
  const ImGuiStyle& style = ImGui::GetStyle();

  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, TRANSPARENT_COLOR);

  const Icon* visibleIcon = &ICONS[ICON_SIZE_SMALL][ICON_VISIBLE];
  const Icon* invisibleIcon = &ICONS[ICON_SIZE_SMALL][ICON_INVISIBLE];

  GLuint tex = visible ?
    visibleIcon->tex[selected] : invisibleIcon->tex[selected];

  ImGui::ImageButton(
    (void*)(size_t)tex, ImVec2(16, 16), ImVec2(0, 0), ImVec2(1, 1));

  if (ImGui::IsItemClicked()) {
    std::cout << "VISIBILITY CLICKED : " << visible << " : " << prim.GetPath() << std::endl;
    pxr::UsdGeomImageable imageable(prim);
    if (imageable) {
      if (visible)imageable.MakeInvisible();
      else imageable.MakeVisible();
      GetWindow()->ForceRedraw();
    }
  }

  ImGui::NextColumn();
  ImGui::PopStyleColor(3);
}

/*
void 
ExplorerUI::DrawItem(ExplorerUI::Item* current, bool heritedVisibility)
{
  ImGuiTreeNodeFlags itemFlags = _treeFlags;

  if (BITMASK_CHECK(current->flags, ExplorerUI::Item::SELECTED)) {
    itemFlags |= ImGuiTreeNodeFlags_Selected;
  }

  // parent
  if (current->items.size())
  {
    std::string key = "##" + current->path.GetString();
    bool currentOpen = ImGui::TreeNodeEx(key.c_str(), itemFlags);
    _UpdateSelection(current, false);

    if(BITMASK_CHECK(current->flags, ExplorerUI::Item::SELECTED)) {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_DEFAULT_COLOR);
    }
    ImGui::SameLine();
    ImGui::Text("%s", current->path.GetName().c_str());
    ImGui::NextColumn();

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
    ImGui::PopStyleColor();

    if (currentOpen)
    {
      BITMASK_SET(current->flags, ExplorerUI::Item::EXPANDED);
      for (auto& item : current->items)
        DrawItem(&item, BITMASK_CHECK(item.flags, ExplorerUI::Item::VISIBLE) && heritedVisibility);
      ImGui::TreePop();
    }
    else BITMASK_CLEAR(current->flags, ExplorerUI::Item::EXPANDED);
  }
  // leaf
  else
  {
    itemFlags |= ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::string key = "##" + current->path.GetString();
    ImGui::TreeNodeEx(key.c_str(), itemFlags);
       
    _UpdateSelection(current, true);

    if(BITMASK_CHECK(current->flags, ExplorerUI::Item::SELECTED)) {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_SELECTED_COLOR);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, TEXT_DEFAULT_COLOR);
    }

    ImGui::SameLine();
    ImGui::Text("%s", current->path.GetName().c_str());
    ImGui::NextColumn();

    BITMASK_CLEAR(current->flags, ExplorerUI::Item::EXPANDED);

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
    ImGui::PopStyleColor();
  }
}
*/

#define PrimDefaultColor {227.f/255.f, 227.f/255.f, 227.f/255.f, 1.0}
#define PrimInactiveColor {0.4, 0.4, 0.4, 1.0}
#define PrimInstanceColor {135.f/255.f, 206.f/255.f, 250.f/255.f, 1.0}
#define PrimPrototypeColor {118.f/255.f, 136.f/255.f, 217.f/255.f, 1.0}
#define PrimHasCompositionColor {222.f/255.f, 158.f/255.f, 46.f/255.f, 1.0}

static ImVec4 GetPrimColor(const UsdPrim& prim) {
  if (!prim.IsActive() || !prim.IsLoaded()) {
    return ImVec4(PrimInactiveColor);
  }
  if (prim.IsInstance()) {
    return ImVec4(PrimInstanceColor);
  }
  const auto hasCompositionArcs = prim.HasAuthoredReferences() || prim.HasAuthoredPayloads() || prim.HasAuthoredInherits() ||
    prim.HasAuthoredSpecializes() || prim.HasVariantSets();
  if (hasCompositionArcs) {
    return ImVec4(PrimHasCompositionColor);
  }
  if (prim.IsPrototype() || prim.IsInPrototype() || prim.IsInstanceProxy()) {
    return ImVec4(PrimPrototypeColor);
  }
  return ImVec4(PrimDefaultColor);
}

/// Recursive function to draw a prim and its descendants
void 
ExplorerUI::DrawPrim(const pxr::UsdPrim& prim, Selection* selection) 
{
  ImGuiTreeNodeFlags flags = _treeFlags;
  const auto& children = prim.GetFilteredChildren(
    pxr::UsdTraverseInstanceProxies(pxr::UsdPrimAllPrimsPredicate));
  if (children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }
  const bool selected = selection->IsSelected(prim);
  if (selected) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  ImGui::PushStyleColor(ImGuiCol_Text, GetPrimColor(prim));
  const bool unfolded = ImGui::TreeNodeEx(prim.GetName().GetText(), flags);
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    _current = prim.GetPath();
  }
  ImGui::PopStyleColor();
  ImGui::NextColumn();

  DrawType(prim, selected);
  pxr::UsdGeomImageable imageable(prim);
  if (imageable) {
    pxr::TfToken visibility;
    imageable.GetVisibilityAttr().Get<TfToken>(&visibility);
    const bool visible = (visibility != pxr::UsdGeomTokens->invisible);
    DrawVisibility(prim, visible, selected);
  }

  if (unfolded) {
    if (prim.IsActive()) {
      for (const auto& child : children) {
        DrawPrim(child, selection);
      }
    }
    ImGui::TreePop();
  }
}

bool 
ExplorerUI::Draw()
{
  std::cout << "Draw Explorer..." << std::endl;
  /// Draw the hierarchy of the stage
  Application* app = GetApplication();
  Selection* selection = app->GetSelection();
  pxr::UsdStageRefPtr stage = app->GetStage();
  if (!stage) return false;

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);

  const pxr::UsdPrim root = stage->GetPseudoRoot();
  pxr::SdfLayerHandle layer = stage->GetSessionLayer();

  // setup transparent background
  ImGui::PushStyleColor(ImGuiCol_Header, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, TRANSPARENT_COLOR);

  // setup columns
  ImGui::Columns(3);
  ImGui::SetColumnWidth(0, GetWidth() - 90);
  ImGui::SetColumnWidth(1, 60);
  ImGui::SetColumnWidth(2, 30);
  
  // draw title
  ImGui::PushFont(GetWindow()->GetMediumFont(2));
  ImGui::Text("Prim");
  ImGui::NextColumn();
  ImGui::Text("Type");
  ImGui::NextColumn();
  ImGui::Text("Vis");
  ImGui::NextColumn();
  ImGui::PopFont();

  ImGui::SetCursorPos(pxr::GfVec2f(0.f, EXPLORER_LINE_HEIGHT));
  /*
  bool unfolded = 
    ImGui::TreeNodeEx(stage->GetRootLayer()->GetDisplayName().c_str(), _treeFlags);*/

  /*
  // Unfold the selected paths.
  // TODO: This might be a behavior we don't want in some situations, so add a way to toggle it
  static SelectionHash lastSelectionHash = 0;
  if (UpdateSelectionHash(selectedPaths, lastSelectionHash)) { // We could use the imgui id as well instead of a static ??
    OpenSelectedPaths(selectedPaths);
    // TODO HighlightSelectedPaths();
  }
  */
  ImGui::PushFont(GetWindow()->GetRegularFont(1));
  const auto& children = root.GetFilteredChildren(
    pxr::UsdTraverseInstanceProxies(pxr::UsdPrimAllPrimsPredicate));
  for (const auto& child : children) {
    DrawPrim(child, selection);
  }
  ImGui::PopFont();
  ImGui::TreePop();
  ImGui::PopStyleColor(3);
  ImGui::End();

  return
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
}

PXR_NAMESPACE_CLOSE_SCOPE