#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/pcp/layerStack.h>

#include "../ui/style.h"
#include "../ui/explorer.h"
#include "../app/application.h"
#include "../app/commands.h"
#include "../app/notice.h"
#include "../app/window.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

// static data
ImGuiWindowFlags ExplorerUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBackground;

ImGuiTreeNodeFlags ExplorerUI::_treeFlags =
  ImGuiTreeNodeFlags_OpenOnArrow |
  ImGuiTreeNodeFlags_OpenOnDoubleClick |
  ImGuiTreeNodeFlags_SpanAvailWidth;

static void ExploreLayerTree(pxr::SdfLayerTreeHandle tree, pxr::PcpNodeRef node) {
  if (!tree)
    return;
  auto obj = tree->GetLayer()->GetObjectAtPath(node.GetPath());
  if (obj) {
    std::string format;
    format += tree->GetLayer()->GetDisplayName();
    format += " ";
    format += obj->GetPath().GetString();
    if (ImGui::MenuItem(format.c_str())) {
      //ExecuteAfterDraw<EditorInspectLayerLocation>(tree->GetLayer(), obj->GetPath());
      std::cout << "INSPECT LAYER LOCATION : " << obj->GetPath() << std::endl;
    }
  }
  for (auto subTree : tree->GetChildTrees()) {
    ExploreLayerTree(subTree, node);
  }
}

static void ExploreComposition(pxr::PcpNodeRef root) {
  auto tree = root.GetLayerStack()->GetLayerTree();
  ExploreLayerTree(tree, root);
  TF_FOR_ALL(childNode, root.GetChildrenRange()) { ExploreComposition(*childNode); }
}

static void DrawUsdPrimEditMenuItems(const pxr::UsdPrim& prim) {
  if (ImGui::MenuItem("Toggle active")) {
    const bool active = !prim.IsActive();
    //ExecuteAfterDraw(&UsdPrim::SetActive, prim, active);
  }
  // TODO: Load and Unload are not in the undo redo :( ... make a command for them
  if (prim.HasAuthoredPayloads() && prim.IsLoaded() && ImGui::MenuItem("Unload")) {
    //ExecuteAfterDraw(&UsdPrim::Unload, prim);
  }
  if (prim.HasAuthoredPayloads() && !prim.IsLoaded() && ImGui::MenuItem("Load")) {
    //ExecuteAfterDraw(&UsdPrim::Load, prim, UsdLoadWithDescendants);
  }
  if (ImGui::MenuItem("Copy prim path")) {
    ImGui::SetClipboardText(prim.GetPath().GetString().c_str());
  }
  if (ImGui::BeginMenu("Inspect")) {
    ImGui::SetClipboardText(prim.GetPath().GetString().c_str());
    auto pcpIndex = prim.ComputeExpandedPrimIndex();
    if (pcpIndex.IsValid()) {
      auto rootNode = pcpIndex.GetRootNode();
      ExploreComposition(rootNode);
    }
    ImGui::EndMenu();
  }
}


// constructor
ExplorerUI::ExplorerUI(View* parent) 
  : BaseUI(parent, UIType::EXPLORER)
{
}

// destructor
ExplorerUI::~ExplorerUI()
{
}

void 
ExplorerUI::Init()
{
  //Update();
  _parent->SetDirty();
  _initialized = true;
}


void 
ExplorerUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(_parent->GetWindow()->GetGlfwWindow(), &x, &y);
  if (x > GetX() + (GetWidth() - 50)) return;

  Application* app = Application::Get();
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_RELEASE) {
      if (app->GetWorkStage()->GetPrimAtPath(_current).IsValid()) {
        if (mods & GLFW_MOD_CONTROL) {
          app->ToggleSelection({ _current });
        }
        else {
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


void
ExplorerUI::DrawItemBackground(ImDrawList* drawList,
  bool selected, bool& flip)
{
  const ImGuiStyle& style = ImGui::GetStyle();
  ImVec2 pos = ImGui::GetCursorPos();
  const float width = (float)GetWidth();
  const float height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2 + style.ItemInnerSpacing.y;
  if (selected) {
    drawList->AddRectFilled(
      { pos.x, pos.y },
      { pos.x + width, pos.y + height },
      ImColor(style.Colors[ImGuiCol_ButtonActive]));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + height },
        ImColor(style.Colors[ImGuiCol_WindowBg]));
    else
      drawList->AddRectFilled(
        { pos.x, pos.y },
        { pos.x + width, pos.y + height },
        ImColor(style.Colors[ImGuiCol_ChildBg]));
  }
  ImGui::SetCursorPos(ImVec2(pos.x, pos.y + height));
}

void
ExplorerUI::DrawBackground()
{
  ImDrawList* drawList = ImGui::GetBackgroundDrawList();
  const auto& style = ImGui::GetStyle();
  const float scrollOffsetH = ImGui::GetScrollX();
  const float scrollOffsetV = ImGui::GetScrollY();
  pxr::GfVec2f clipRectMin(GetX(), GetY());
  pxr::GfVec2f clipRectMax(GetX() + GetWidth(), GetY() + GetHeight());

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax[0] += style.ScrollbarSize;
  }

  drawList->PushClipRect(clipRectMin, clipRectMax);

  bool flip = false;

  //ImGui::PushFont(GetWindow()->GetMediumFont(2));
  ImGui::SetCursorPos(
    ImVec2(
      clipRectMin[0],
      clipRectMin[1] - scrollOffsetV + ImGui::GetTextLineHeight() + style.FramePadding[1] * 2));
  //ImGui::PopFont();
  //ImGui::PushFont(GetWindow()->GetRegularFont(1));
  for (auto& item : _items) {
    DrawItemBackground(drawList, item.selected, flip);
    flip = !flip;
  }
  //ImGui::PopFont();
  drawList->PopClipRect();
}

void 
ExplorerUI::DrawType(const pxr::UsdPrim& prim, bool selected)
{
  ImGui::Text("%s", prim.GetTypeName().GetText());
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    _current = prim.GetPath();
  }
  ImGui::NextColumn();
}

static void _PushCurrentPath(const pxr::SdfPath& path, pxr::SdfPathVector& paths)
{
  if (path.IsEmpty())return;
  for (auto& _path : paths) {
    if (_path == path)return;
  }
  paths.push_back(path);
}

void
ExplorerUI::DrawVisibility(const pxr::UsdPrim& prim, bool visible, bool selected)
{
  const ImGuiStyle& style = ImGui::GetStyle();
  
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, TRANSPARENT_COLOR);
  
  const char* visibleIcon = ICON_FA_EYE;
  const char* invisibleIcon = ICON_FA_EYE_SLASH;

  Application* app = Application::Get();
  if (ImGui::Button(visible ? visibleIcon : invisibleIcon)) {
    _current = prim.GetPath();
    pxr::SdfPathVector paths = app->GetSelection()->GetSelectedPaths();
    _PushCurrentPath(_current, paths);
    ADD_COMMAND(ShowHideCommand, paths, ShowHideCommand::TOGGLE);
  }

  ImGui::NextColumn();
  ImGui::PopStyleColor(3);
}

void
ExplorerUI::DrawActive(const pxr::UsdPrim& prim, bool selected)
{
  const ImGuiStyle& style = ImGui::GetStyle();

  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, TRANSPARENT_COLOR);

  const char* activeIcon = ICON_FA_RIGHT_TO_BRACKET;
  const char* inactiveIcon = ICON_FA_PAUSE;

  Application* app = Application::Get();
  Selection* selection = app->GetSelection();
  
 
  if (ImGui::Button(selected ? activeIcon : inactiveIcon)) {
    _current = prim.GetPath();
    pxr::SdfPathVector paths = selection->GetSelectedPaths();
    _PushCurrentPath(_current, paths);
    ADD_COMMAND(ActivateCommand, paths, ActivateCommand::TOGGLE);
  }

  ImGui::NextColumn();
  ImGui::PopStyleColor(3);
}

static ImVec4 PrimDefaultColor(227.f/255.f, 227.f/255.f, 227.f/255.f, 1.0);
static ImVec4 PrimInactiveColor(0.4, 0.4, 0.4, 1.0);
static ImVec4 PrimInstanceColor(135.f/255.f, 206.f/255.f, 250.f/255.f, 1.0);
static ImVec4 PrimPrototypeColor(118.f/255.f, 136.f/255.f, 217.f/255.f, 1.0);
static ImVec4 PrimHasCompositionColor(222.f/255.f, 158.f/255.f, 46.f/255.f, 1.0);

static ImVec4 GetPrimColor(const pxr::UsdPrim& prim) {
  if (!prim.IsActive() || !prim.IsLoaded()) {
    return PrimInactiveColor;
  }
  if (prim.IsInstance()) {
    return PrimInstanceColor;
  }
  const auto hasCompositionArcs = 
    prim.HasAuthoredReferences() || prim.HasAuthoredPayloads() || 
    prim.HasAuthoredInherits() || prim.HasAuthoredSpecializes() || prim.HasVariantSets();
  if (hasCompositionArcs) {
    return PrimHasCompositionColor;
  }
  if (prim.IsPrototype() || prim.IsInPrototype() || prim.IsInstanceProxy()) {
    return PrimPrototypeColor;
  }
  return PrimDefaultColor;
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
  if (ImGui::IsItemClicked()) {
    _current = prim.GetPath();
  }

  if (ImGui::IsItemToggledOpen())_parent->SetFlag(View::DISCARDMOUSEBUTTON);
  ImGui::PopStyleColor();
  ImGui::NextColumn();

  DrawType(prim, selected);
  
  pxr::UsdGeomImageable imageable(prim);
  if (imageable) {
    pxr::TfToken visibility;
    imageable.GetVisibilityAttr().Get<pxr::TfToken>(&visibility);
    const bool visible = (visibility != pxr::UsdGeomTokens->invisible);
    DrawVisibility(prim, visible, selected);
  } else {
    ImGui::NextColumn();
  }

  DrawActive(prim, selected);

  _items.push_back({ prim.GetPath(), _mapping++, selected });

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
  /// Draw the hierarchy of the stage
  Application* app = Application::Get();
  Selection* selection = app->GetSelection();
  pxr::UsdStageRefPtr stage = app->GetWorkStage();
  const ImGuiStyle& style = ImGui::GetStyle();
  if (!stage) return false;

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  _items.clear();
  _mapping = 0;

  ImDrawList* backgroundList = ImGui::GetBackgroundDrawList();
  backgroundList->AddRectFilled(min, min + size, ImColor(style.Colors[ImGuiCol_WindowBg]));

  const pxr::UsdPrim root = stage->GetPseudoRoot();
  pxr::SdfLayerHandle layer = stage->GetSessionLayer();

  // setup transparent background
  ImGui::PushStyleColor(ImGuiCol_Header, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, TRANSPARENT_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, TRANSPARENT_COLOR);

  // setup columns
  ImGui::Columns(4);
  ImGui::SetColumnWidth(0, GetWidth() - 110);
  ImGui::SetColumnWidth(1, 60);
  ImGui::SetColumnWidth(2, 25);
  ImGui::SetColumnWidth(3, 25);

  // draw title
  //ImGui::PushFont(GetWindow()->GetMediumFont(2));
  ImGui::Text("Prim");
  ImGui::NextColumn();
  ImGui::Text("Type");
  ImGui::NextColumn();
  ImGui::Text(" V ");
  ImGui::NextColumn();
  ImGui::Text(" A ");
  ImGui::NextColumn();
  //ImGui::PopFont();

  //ImGui::SetCursorPos(pxr::GfVec2f(0.f, EXPLORER_LINE_HEIGHT));
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
  //ImGui::PushFont(GetWindow()->GetRegularFont(1));
  const auto& children = root.GetFilteredChildren(
    pxr::UsdTraverseInstanceProxies(pxr::UsdPrimAllPrimsPredicate));
  for (const auto& child : children) {
    DrawPrim(child, selection);
  }
  //ImGui::PopFont();
  DrawBackground();
  //ImGui::TreePop();
  ImGui::PopStyleColor(3);
  ImGui::End();

  return
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
}

JVR_NAMESPACE_CLOSE_SCOPE