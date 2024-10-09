#include <pxr/base/gf/vec2i.h>
#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/primSpec.h>
#include "../ui/utils.h"
#include "../ui/fonts.h"
#include "../ui/layerEditor.h"
#include "../app/view.h"
#include "../app/application.h"
#include "../app/commands.h"

JVR_NAMESPACE_OPEN_SCOPE

#define LayerHierarchyEditorSeed 3456823
#define IdOf ToImGuiID<3456823, size_t>

typedef enum CompositionArcListType { 
  ReferenceList, 
  PayloadList, 
  InheritList, 
  SpecializeList 
} CompositionArcListType;


ImGuiWindowFlags LayerEditorUI::_flags = 0
  | ImGuiWindowFlags_NoResize
  | ImGuiWindowFlags_NoTitleBar
  | ImGuiWindowFlags_NoMove;


LayerEditorUI::LayerEditorUI(View* parent)
  :BaseUI(parent, UIType::LAYEREDITOR)
{
}

LayerEditorUI::~LayerEditorUI()
{
}

static void 
_HandleDragAndDrop(const pxr::SdfPrimSpecHandle &primSpec, const Selection &selection) 
{
  static pxr::SdfPathVector payload;
  // Drag and drop
  ImGuiDragDropFlags srcFlags = 0;
  srcFlags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
  srcFlags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening
                                                            // foreign treenodes/tabs while dragging
  // src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
  if (ImGui::BeginDragDropSource(srcFlags)) {
    payload.clear();
    if (selection.IsSelected(primSpec.GetSpec())) {
      for (const auto &selectedPath : selection.GetSelectedPaths()) {
        payload.push_back(selectedPath);
      }
    } else {
      payload.push_back(primSpec->GetPath());
    }
    if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
      ImGui::Text("Moving %s", primSpec->GetPath().GetString().c_str());
    }
    ImGui::SetDragDropPayload("DND", &payload, sizeof(SdfPathVector), ImGuiCond_Once);
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    ImGuiDragDropFlags targetFlags = 0;
    // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a
    // target) to do something target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow
    // rectangle
    if (const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("DND", targetFlags)) {
      pxr::SdfPathVector source(*(pxr::SdfPathVector *)pl->Data);
      //ExecuteAfterDraw<PrimReparent>(primSpec->GetLayer(), source, primSpec->GetPath());
      std::cout << "reparent under " << primSpec->GetPath() << std::endl;
    }
    ImGui::EndDragDropTarget();
  }
}

static void 
_HandleDragAndDrop(pxr::SdfLayerHandle layer, const Selection &selection) 
{
  static pxr::SdfPathVector payload;
  // Drop on the layer
  if (ImGui::BeginDragDropTarget()) {
    ImGuiDragDropFlags targetFlags = 0;
    if (const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("DND", targetFlags)) {
      pxr::SdfPathVector source(*(pxr::SdfPathVector *)pl->Data);
      //ExecuteAfterDraw<PrimReparent>(layer, source, SdfPath::AbsoluteRootPath());
      std::cout << "reparent under " << pxr::SdfPath::AbsoluteRootPath() << std::endl;
    }
    ImGui::EndDragDropTarget();
  }
}

inline void 
  _AddTooltip(const char *text) {
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void 
_AddMiniToolbar(pxr::SdfLayerRefPtr layer, const pxr::SdfPrimSpecHandle &prim) 
{
  if (ImGui::Button(ICON_FA_PLUS)) {
      if (prim == pxr::SdfPrimSpecHandle()) {
        //ExecuteAfterDraw<PrimNew>(layer, FindNextAvailableTokenString(SdfPrimSpecDefaultName));
      } else {
        //ExecuteAfterDraw<PrimNew>(prim, FindNextAvailableTokenString(SdfPrimSpecDefaultName));
      }
  }
  _AddTooltip("New child prim");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_SQUARE_PLUS) && prim) {
    auto parent = prim->GetNameParent();
    if (parent) {
      //ExecuteAfterDraw<PrimNew>(parent, FindNextAvailableTokenString(prim->GetName()));
    } else {
      //ExecuteAfterDraw<PrimNew>(layer, FindNextAvailableTokenString(prim->GetName()));
    }
  }
  _AddTooltip("New sibbling prim");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_CLONE) && prim) {
    //ExecuteAfterDraw<PrimDuplicate>(prim, FindNextAvailableTokenString(prim->GetName()));
  }
  _AddTooltip("Duplicate");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_ARROW_UP) && prim) {
    //ExecuteAfterDraw<PrimReorder>(prim, true);
  }
  _AddTooltip("Move up");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_ARROW_DOWN) && prim) {
    //ExecuteAfterDraw<PrimReorder>(prim, false);
  }
  _AddTooltip("Move down");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_TRASH) && prim) {
    //ExecuteAfterDraw<PrimRemove>(prim);
  }
  _AddTooltip("Remove");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_COPY) && prim) {
    //ExecuteAfterDraw<PrimCopy>(prim);
  }
  _AddTooltip("Copy");
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_PASTE) && prim) {
    //ExecuteAfterDraw<PrimPaste>(prim);
  }
  _AddTooltip("Paste");
}


static void 
_AddBackgroundSelection(const pxr::SdfPrimSpecHandle &currentPrim, 
  const Selection &selection, bool selected) 
{
  ImVec4 colorSelected = selected ? ImVec4(0.5, 0.5, 0.5, 0.5) : ImVec4(0.75, 0.60, 0.33, 0.2);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, selected ? colorSelected : ImVec4(0.0, 0.0, 0.0, 0.0));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.0, 0.0, 0.0, 0.0));
  ImGui::PushStyleColor(ImGuiCol_Header, colorSelected);
  ImVec2 sizeArg(0.0, TABLE_ROW_DEFAULT_HEIGHT);
  const auto selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
  if (ImGui::Selectable("##backgroundSelectedPrim", selected, selectableFlags, sizeArg)) {
      if (currentPrim) {
        //ExecuteAfterDraw<EditorSetSelection>(currentPrim->GetLayer(), currentPrim->GetPath());
        //selectedPrim = currentPrim;
      }
  }
  ImGui::SetItemAllowOverlap();
  ImGui::SameLine();
  ImGui::PopStyleColor(3);
}

static void 
_AddLayerNavigation(pxr::SdfLayerRefPtr layer) {
  if (!layer)
    return;
  if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
    //ExecuteAfterDraw<EditorSetPreviousLayer>();
  }
  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_ARROW_RIGHT)) {
    //ExecuteAfterDraw<EditorSetNextLayer>();
  }
  ImGui::SameLine();
  {
    /*ScopedStyleColor layerIsDirtyColor(ImGuiCol_Text,
      layer->IsDirty() ? ImGui::GetColorU32(ImGuiCol_Text) : ImU32(ImColor{ ColorGreyish }));*/

    if (ImGui::Button(ICON_FA_RECYCLE)) {
      //ExecuteAfterDraw(&SdfLayer::Reload, layer, false);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILE)) {
      //ExecuteAfterDraw(&SdfLayer::Save, layer, false);
    }
  }
  ImGui::SameLine();
  if (!layer)
    return;

  {
    /*ScopedStyleColor textBackground(ImGuiCol_Header, ImU32(ImColor{ ColorPrimHasComposition }));*/
    ImGui::Selectable("##LayerNavigation");
    if (ImGui::BeginPopupContextItem()) {
      //DrawLayerActionPopupMenu(layer);
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("Layer: %s", layer->GetDisplayName().c_str());
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("%s", layer->GetRealPath().c_str());
      ImGui::EndTooltip();
    }
  }
}

static void 
_AddTopNodeLayerRow(const pxr::SdfLayerRefPtr &layer, const Selection &selection, float &selectedPosY) 
{
  ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
  int nodeId = 0;
  if (layer->GetRootPrims().empty()) {
    treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
  }
  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);
  auto rootPrim = layer->GetPrimAtPath(pxr::SdfPath::AbsoluteRootPath());
  _AddBackgroundSelection(rootPrim, selection, selection.IsSelected(rootPrim.GetSpec()));
  _HandleDragAndDrop(layer, selection);
  ImGui::SetItemAllowOverlap();
  std::string label = std::string(ICON_FA_FILE) + " " + layer->GetDisplayName();

  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);
  bool unfolded = ImGui::TreeNodeBehavior(IdOf(pxr::SdfPath::AbsoluteRootPath().GetHash()), treeNodeFlags, label.c_str());
  ImGui::PopStyleColor(2);
  
  if (!ImGui::IsItemToggledOpen() && ImGui::IsItemClicked()) {
    //ExecuteAfterDraw<EditorSetSelection>(layer, SdfPath::AbsoluteRootPath());;
  }

  if (ImGui::BeginPopupContextItem()) {
    _AddMiniToolbar(layer, pxr::SdfPrimSpec());
    ImGui::Separator();
    if (ImGui::MenuItem("Add sublayer")) {
      //DrawSublayerPathEditDialog(layer, "");
    }
    if (ImGui::MenuItem("Add root prim")) {
      ADD_COMMAND(CreatePrimCommand, Application::Get()->GetWorkStage()->GetRootLayer(), pxr::SdfPath("ZOB"), 0);
      //ExecuteAfterDraw<PrimNew>(layer, FindNextAvailableTokenString(SdfPrimSpecDefaultName));
    }
    const char *clipboard = ImGui::GetClipboardText();
    const bool clipboardEmpty = !clipboard || clipboard[0] == 0;
    if (!clipboardEmpty && ImGui::MenuItem("Paste path as Overs")) {
      //ExecuteAfterDraw<LayerCreateOversFromPath>(layer, std::string(ImGui::GetClipboardText()));
    }
    if (ImGui::MenuItem("Paste")) {
      //ExecuteAfterDraw<PrimPaste>(rootPrim);
    }
    ImGui::Separator();
    //DrawLayerActionPopupMenu(layer);

    ImGui::EndPopup();
  }
  if (unfolded) {
      ImGui::TreePop();
  }
  if (!layer->GetSubLayerPaths().empty()) {
    ImGui::TableSetColumnIndex(3);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 0.0));
    ImGui::PushItemWidth(-FLT_MIN); // removes the combo label.
    if (ImGui::BeginCombo("Sublayers", "Sublayers", ImGuiComboFlags_NoArrowButton)) {
      for (const auto &pathIt : layer->GetSubLayerPaths()) {
        const std::string &path = pathIt;
        if (ImGui::MenuItem(path.c_str())) {
          auto subLayer = SdfLayer::FindOrOpenRelativeToLayer(layer, path);
          if (subLayer) {
            //ExecuteAfterDraw<EditorFindOrOpenLayer>(subLayer->GetRealPath());
          }
        }
      }
      ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
  }

  if (selectedPosY != -1) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.7, 0.5, 0.7));
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 160);
    ImGui::SetCursorPosY(selectedPosY);
    _AddMiniToolbar(layer, layer->GetPrimAtPath(selection.GetAnchorPath()));
    ImGui::PopStyleColor();
  }
}

/// Traverse all the path of the layer and store them in a vector. Apply a filter to only traverse the path
/// that should be displayed, the ones inside the collapsed part of the tree view
void TraverseOpenedPaths(const pxr::SdfLayerRefPtr &layer, std::vector<pxr::SdfPath> &paths) {
  paths.clear();
  std::stack<pxr::SdfPath> st;
  st.push(pxr::SdfPath::AbsoluteRootPath());
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  ImGuiStorage *storage = window->DC.StateStorage;
  while (!st.empty()) {
    const pxr::SdfPath path = st.top();
    st.pop();
    const ImGuiID pathHash = IdOf(path.GetHash());
    const bool isOpen = storage->GetInt(pathHash, 0) != 0;
    if (isOpen) {
      if (layer->HasField(path, pxr::SdfChildrenKeys->PrimChildren)) {
        const std::vector<pxr::TfToken> &children =
          layer->GetFieldAs<std::vector<pxr::TfToken>>(path, pxr::SdfChildrenKeys->PrimChildren);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          st.push(path.AppendChild(*it));
        }
      }
      if (layer->HasField(path, pxr::SdfChildrenKeys->VariantSetChildren)) {
        const std::vector<pxr::TfToken> &variantSetchildren =
          layer->GetFieldAs<std::vector<pxr::TfToken>>(path, pxr::SdfChildrenKeys->VariantSetChildren);
        // Skip the variantSet paths and show only the variantSetChildren
        for (auto vSetIt = variantSetchildren.rbegin(); vSetIt != variantSetchildren.rend(); ++vSetIt) {
          auto variantSetPath = path.AppendVariantSelection(*vSetIt, "");
          if (layer->HasField(variantSetPath, pxr::SdfChildrenKeys->VariantChildren)) {
            const std::vector<pxr::TfToken> &variantChildren =
              layer->GetFieldAs<std::vector<pxr::TfToken>>(variantSetPath, pxr::SdfChildrenKeys->VariantChildren);
            const std::string &variantSet = variantSetPath.GetVariantSelection().first;
            for (auto vChildrenIt = variantChildren.rbegin(); vChildrenIt != variantChildren.rend(); ++vChildrenIt) {
              st.push(path.AppendVariantSelection(pxr::TfToken(variantSet), *vChildrenIt));
            }
          }
        }
      }
    }
    paths.push_back(path);
  }
}

void 
_AddTreeNodePopup(pxr::SdfPrimSpecHandle &primSpec)
{
  if (!primSpec)
    return;

  if (ImGui::MenuItem("Add child")) {
    std::cout << "add child.." << std::endl;
    //ExecuteAfterDraw<PrimNew>(primSpec, FindNextAvailablePrimName(SdfPrimSpecDefaultName));
  }
  auto parent = primSpec->GetNameParent();
  if (parent) {
    if (ImGui::MenuItem("Add sibling")) {
      std::cout << "add sibling..." << std::endl;
      //ExecuteAfterDraw<PrimNew>(parent, FindNextAvailablePrimName(primSpec->GetName()));
    }
  }
  if (ImGui::MenuItem("Duplicate")) {
    std::cout << "duplicate..." << std::endl;
    //ExecuteAfterDraw<PrimDuplicate>(primSpec, FindNextAvailablePrimName(primSpec->GetName()));
  }
  if (ImGui::MenuItem("Remove")) {
    std::cout << "remove..." << std::endl;
    //ExecuteAfterDraw<PrimRemove>(primSpec);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy")) {
    std::cout << "copy..." << std::endl;
    //ExecuteAfterDraw<PrimCopy>(primSpec);
  }
  if (ImGui::MenuItem("Paste")) {
    std::cout << "paste..." << std::endl;
    //ExecuteAfterDraw<PrimPaste>(primSpec);
  }
  ImGui::Separator();
  if (ImGui::BeginMenu("Create composition")) {
    std::cout << "create composition..." << std::endl;
    //DrawPrimCreateCompositionMenu(primSpec);
    ImGui::EndMenu();
  }

  ImGui::Separator();
  if (ImGui::MenuItem("Copy prim path")) {
    std::cout << "copy prim path..." << std::endl;
    ImGui::SetClipboardText(primSpec->GetPath().GetString().c_str());
  }

  /*
  NamePopupUI* popup = new NamePopupUI((int)200, (int)200, 200, 300);
  Application::Get()->SetPopup(popup);
  */
}

// Returns unfolded
static bool 
_AddTreeNodePrimName(const bool &primIsVariant, pxr::SdfPrimSpecHandle &primSpec, 
  const Selection &selection, bool hasChildren) 
{
  // Format text differently when the prim is a variant
  std::string primSpecName;
  if (primIsVariant) {
    auto variantSelection = primSpec->GetPath().GetVariantSelection();
    primSpecName = std::string("{") + variantSelection.first.c_str() + ":" + variantSelection.second.c_str() + "}";
  } else {
    primSpecName = primSpec->GetPath().GetName();
  }
  ImGui::PushStyleColor(ImGuiCol_Text, primIsVariant ? ImU32(ImColor::HSV(0.2 / 7.0f, 0.5f, 0.8f)) : ImGui::GetColorU32(ImGuiCol_Text));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);

  ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
  nodeFlags |= hasChildren && !primSpec->HasVariantSetNames() ? ImGuiTreeNodeFlags_Leaf
                                                              : ImGuiTreeNodeFlags_None; // ImGuiTreeNodeFlags_DefaultOpen;
  auto cursor = ImGui::GetCursorPos(); // Store position for the InputText to edit the prim name
  auto unfolded = ImGui::TreeNodeBehavior(IdOf(primSpec->GetPath().GetHash()), nodeFlags, primSpecName.c_str());

  // Edition of the prim name
  static pxr::SdfPrimSpecHandle editNamePrim;
  if (!ImGui::IsItemToggledOpen() && ImGui::IsItemClicked()) {
      //ExecuteAfterDraw<EditorSetSelection>(primSpec->GetLayer(), primSpec->GetPath());
      if (editNamePrim != pxr::SdfPrimSpecHandle() && editNamePrim != primSpec) {
          editNamePrim = pxr::SdfPrimSpecHandle();
      }
      if (!primIsVariant && ImGui::IsMouseDoubleClicked(0)) {
          editNamePrim = primSpec;
      }
  }
  if (primSpec == editNamePrim) {
      ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 1.0));
      ImGui::SetCursorPos(cursor);
      UIUtils::AddPrimSpecifier(primSpec); // Draw the prim name editor
      if (ImGui::IsItemDeactivatedAfterEdit() || !ImGui::IsItemFocused()) {
          editNamePrim = pxr::SdfPrimSpecHandle();
      }
      ImGui::PopStyleColor();
  }
  ImGui::PopStyleColor(3);

  return unfolded;
}

/// Draw a node in the primspec tree
static void 
_AddSdfPrimRow(const pxr::SdfLayerRefPtr &layer, const pxr::SdfPath &primPath, 
  const Selection &selection, int nodeId, float &selectedPosY) 
{
  pxr::SdfPrimSpecHandle primSpec = layer->GetPrimAtPath(primPath);
  std::cout << "add sdf prim row : " << primPath << std::endl;

  if (!primSpec)
    return;

  pxr::SdfPrimSpecHandle previousSelectedPrim;

  std::cout << "selection anchor path : " << selection.GetAnchorPath() << std::endl;
  
  auto selectedPrim = layer->GetPrimAtPath(selection.GetAnchorPath()); // TODO should we have a function for that ?
  std::cout << "selected prim : " << selectedPrim << std::endl;
  bool primIsVariant = primPath.IsPrimVariantSelectionPath();

  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);

  ImGui::PushID(nodeId);

  nodeId = 0; // reset the counter
  // Edit buttons
  if (selectedPrim == primSpec) {
    selectedPosY = ImGui::GetCursorPosY();
  }

  _AddBackgroundSelection(primSpec, selection, selectedPrim == primSpec);

  // Drag and drop on Selectable
  _HandleDragAndDrop(primSpec, selection);

  // Draw the tree column
  auto childrenNames = primSpec->GetNameChildren();

  ImGui::SameLine();
  TreeIndenter<LayerHierarchyEditorSeed, pxr::SdfPath> indenter(primPath);
  bool unfolded = _AddTreeNodePrimName(primIsVariant, primSpec, selection, childrenNames.empty());

  // Right click will open the quick edit popup menu
  if (ImGui::BeginPopupContextItem()) {
    _AddMiniToolbar(layer, primSpec);
    ImGui::Separator();
    _AddTreeNodePopup(primSpec);
    ImGui::EndPopup();
  }

  // We want transparent combos
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 0.0));

  // Draw the description column
  ImGui::TableSetColumnIndex(1);
  ImGui::PushItemWidth(-FLT_MIN); // removes the combo label. The col needs to have a fixed size
  UIUtils::AddPrimSpecifier(primSpec, ImGuiComboFlags_NoArrowButton);
  ImGui::PopItemWidth();
  ImGui::TableSetColumnIndex(2);
  ImGui::PushItemWidth(-FLT_MIN); // removes the combo label. The col needs to have a fixed size
  UIUtils::AddPrimType(primSpec, ImGuiComboFlags_NoArrowButton);
  ImGui::PopItemWidth();
  // End of transparent combos
  ImGui::PopStyleColor();

  // Draw composition summary
  ImGui::TableSetColumnIndex(3);
  AddPrimCompositionSummary(primSpec);
  ImGui::SetItemAllowOverlap();

  // Draw children
  if (unfolded) {
    ImGui::TreePop();
  }

  ImGui::PopID();
}

bool LayerEditorUI::Draw()
{
  const ImGuiStyle& style = ImGui::GetStyle();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(_parent->GetMin()),
    ImVec2(_parent->GetSize()),
    ImColor(style.Colors[ImGuiCol_WindowBg])
  );

  _layer = Application::Get()->GetWorkStage()->GetRootLayer();
  const Selection* selection = Application::Get()->GetSelection();
  if (_layer) {
    _AddLayerNavigation(_layer);
    //DrawMiniToolbar(_layer, _prim);

    auto tableCursor = ImGui::GetCursorPosY();
    float selectedPosY = -1;

    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    
    /*
    if (ImGui::BeginTable("##DrawArrayEditor", 4, tableFlags)) {
      ImGui::TableSetupColumn("Hierarchy");
      ImGui::TableSetupColumn("Spec", ImGuiTableColumnFlags_WidthFixed, 40);
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100);
      ImGui::TableSetupColumn("Composition");
      ImGui::TableSetupScrollFreeze(4, 1);
      ImGui::TableHeadersRow();

      ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
      int nodeId = 0;
      if (_layer->GetRootPrims().empty()) {
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
      }
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      AddBackgroundSelection(pxr::SdfPrimSpecHandle(), _prim);
      //HandleDragAndDrop(layer);
      ImGui::SetItemAllowOverlap();
      std::string label = _layer->GetDisplayName();
      bool unfolded = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags);

      if (!ImGui::IsItemToggledOpen() && ImGui::IsItemClicked()) {
        _prim = pxr::SdfPrimSpecHandle();
      }

      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Add root prim")) {
          ADD_COMMAND(CreatePrimCommand,
            Application::Get()->GetCurrentLayer(), "/root");
        }
        ImGui::EndPopup();
      }
      if (unfolded) {
        for (const auto& child : _layer->GetRootPrims()) {
          _AddPrimSpecRow(child, _prim, nodeId++, selectedPosY);
        }
        ImGui::TreePop();
      }
      if (selectedPosY != -1) {
        //ScopedStyleColor highlightButton(ImGuiCol_Button, ImVec4(ColorButtonHighlight));
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 160);
        ImGui::SetCursorPosY(selectedPosY);
        //DrawMiniToolbar(_layer, _prim);
      }
      ImGui::EndTable();
    }
    
    if (ImGui::IsItemHovered() && _prim) {
      AddShortcut<PrimRemove, ImGuiKey_Delete>(selectedPrim);
      AddShortcut<PrimCopy, ImGuiKey_LeftCtrl, ImGuiKey_C>(selectedPrim);
      AddShortcut<PrimPaste, ImGuiKey_LeftCtrl, ImGuiKey_V>(selectedPrim);
      AddShortcut<PrimDuplicate, ImGuiKey_LeftCtrl, ImGuiKey_D>(selectedPrim,
        FindNextAvailablePrimName(selectedPrim->GetName()));

    }*/
    if (ImGui::BeginTable("##DrawArrayEditor", 4, _flags)) {
        ImGui::TableSetupScrollFreeze(4, 1);
        ImGui::TableSetupColumn("Prim hierarchy");
        ImGui::TableSetupColumn("Spec", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableSetupColumn("Composition");

        ImGui::TableHeadersRow();

        std::vector<pxr::SdfPath> paths;

        // Find all the opened paths
        paths.reserve(1024);
        TraverseOpenedPaths(_layer, paths);

        int nodeId = 0;
        float selectedPosY = -1;
        const size_t arraySize = paths.size();
        pxr::SdfPathVector pathPrefixes;
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(arraySize));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::PushID(row);
                const pxr::SdfPath &path = paths[row];
                if (path.IsAbsoluteRootPath()) {
                    _AddTopNodeLayerRow(_layer, *selection, selectedPosY);
                } else {
                    _AddSdfPrimRow(_layer, path, *selection, row, selectedPosY);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
  }
  ImGui::End();
  return true;
};
  

JVR_NAMESPACE_CLOSE_SCOPE