#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/primSpec.h>
#include "../ui/utils.h"
#include "../ui/layerEditor.h"
#include "../app/view.h"
#include "../app/application.h"


JVR_NAMESPACE_OPEN_SCOPE

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
DrawBackgroundSelection(const pxr::SdfPrimSpecHandle& currentPrim, pxr::SdfPrimSpecHandle& selectedPrim) 
{
  const bool selected = currentPrim == selectedPrim;
  const auto selectedColor = ImGui::GetColorU32(ImGuiCol_Header);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, selected ? selectedColor : 0);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, selectedColor);
  ImVec2 sizeArg(0.0, 20);
  const auto selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
  if (ImGui::Selectable("##backgroundSelectedPrim", selected, selectableFlags, sizeArg)) {
    selectedPrim = currentPrim;
  }
  ImGui::SetItemAllowOverlap();
  ImGui::SameLine();
  ImGui::PopStyleColor(2);
}

void DrawTreeNodePopup(pxr::SdfPrimSpecHandle &primSpec) {
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
    //ExecuteAfterDraw<PrimRemove>(primSpec);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy")) {
    //ExecuteAfterDraw<PrimCopy>(primSpec);
  }
  if (ImGui::MenuItem("Paste")) {
    //ExecuteAfterDraw<PrimPaste>(primSpec);
  }
  ImGui::Separator();
  if (ImGui::BeginMenu("Create composition")) {
    //DrawPrimCreateCompositionMenu(primSpec);
    ImGui::EndMenu();
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy prim path")) {
    ImGui::SetClipboardText(primSpec->GetPath().GetString().c_str());
  }
}


// Returns unfolded
static bool 
DrawTreeNodePrimName(const bool& primIsVariant, pxr::SdfPrimSpecHandle& primSpec,
  pxr::SdfPrimSpecHandle& selectedPrim, bool hasChildren) 
{
  // Format text differently when the prim is a variant
  std::string primSpecName;
  if (primIsVariant) {
    auto variantSelection = primSpec->GetPath().GetVariantSelection();
    primSpecName = std::string("{") + variantSelection.first.c_str() + ":" + variantSelection.second.c_str() + "}";
  }
  else {
    primSpecName = primSpec->GetPath().GetName();
  }
  ImGui::PushStyleColor(ImGuiCol_Text,
    primIsVariant ? ImU32(ImColor::HSV(0.2 / 7.0f, 0.5f, 0.8f)) : ImGui::GetColorU32(ImGuiCol_Text));

  ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
  nodeFlags |= hasChildren && !primSpec->HasVariantSetNames() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
  auto cursor = ImGui::GetCursorPos(); // Store position for the InputText to edit the prim name
  auto unfolded = ImGui::TreeNodeEx(primSpecName.c_str(), nodeFlags);

  // Edition of the prim name
  static pxr::SdfPrimSpecHandle editNamePrim;
  if (!ImGui::IsItemToggledOpen() && ImGui::IsItemClicked()) {
    selectedPrim = primSpec;
    if (editNamePrim != pxr::SdfPrimSpecHandle() && editNamePrim != selectedPrim) {
      editNamePrim = pxr::SdfPrimSpecHandle();
    }
    if (!primIsVariant && ImGui::IsMouseDoubleClicked(0)) {
      editNamePrim = primSpec;
    }
  }
  if (primSpec == editNamePrim) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 1.0));
    ImGui::SetCursorPos(cursor);
    UIUtils::AddPrimName(primSpec); // Draw the prim name editor
    if (ImGui::IsItemDeactivatedAfterEdit() || !ImGui::IsItemFocused()) {
      editNamePrim = pxr::SdfPrimSpecHandle();
    }
    ImGui::PopStyleColor();
  }
  ImGui::PopStyleColor();
  return unfolded;
}

/// Draw a node in the primspec tree
static void 
DrawPrimSpecRow(pxr::SdfPrimSpecHandle primSpec, pxr::SdfPrimSpecHandle& selectedPrim, int nodeId, float& selectedPosY) 
{
  if (!primSpec)
    return;
  bool primIsVariant = primSpec->GetPath().IsPrimVariantSelectionPath();

  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);

  // ImGui::PushID(nodeId);
  ImGui::PushID(primSpec->GetPath().GetToken().Hash());
  nodeId = 0; // reset the counter
  // Edit buttons
  if (selectedPrim == primSpec) {
    selectedPosY = ImGui::GetCursorPosY();
  }
  DrawBackgroundSelection(primSpec, selectedPrim);

  // Drag and drop on Selectable
  //HandleDragAndDrop(primSpec);

  // Draw the tree column
  auto childrenNames = primSpec->GetNameChildren();

  ImGui::SameLine();
  bool unfolded = DrawTreeNodePrimName(primIsVariant, primSpec, selectedPrim, childrenNames.empty());
  // Right click will open the quick edit popup menu
  if (ImGui::BeginPopupContextItem()) {
    DrawTreeNodePopup(primSpec);
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
  //UIUtils::AddPrimCompositionSummary(primSpec);
  ImGui::SetItemAllowOverlap();

  // Draw children
  if (unfolded) {
    pxr::SdfVariantSetsProxy variantSetMap = primSpec->GetVariantSets();
    TF_FOR_ALL(varSetIt, variantSetMap) {
      const pxr::SdfVariantSetSpecHandle& varSetSpec = varSetIt->second;
      const pxr::SdfVariantSpecHandleVector& variants = varSetSpec->GetVariantList();
      TF_FOR_ALL(varIt, variants) {
        const pxr::SdfPrimSpecHandle& variantSpec = (*varIt)->GetPrimSpec();
        DrawPrimSpecRow(variantSpec, selectedPrim, nodeId++, selectedPosY);
      }
    }
    for (int i = 0; i < childrenNames.size(); ++i) {
      DrawPrimSpecRow(childrenNames[i], selectedPrim, nodeId++, selectedPosY);
    }
    ImGui::TreePop();
  }
  ImGui::PopID();
}

bool LayerEditorUI::Draw()
{
  SetInteracting(true);
  const ImGuiStyle& style = ImGui::GetStyle();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(_parent->GetMin()),
    ImVec2(_parent->GetSize()),
    ImColor(style.Colors[ImGuiCol_WindowBg])
  );

  _layer = GetApplication()->GetWorkStage()->GetRootLayer();
  if (_layer) {
    //DrawLayerNavigation(_layer);
    //DrawMiniToolbar(_layer, _prim);

    auto tableCursor = ImGui::GetCursorPosY();
    float selectedPosY = -1;

    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##DrawPrimSpecTree", 4, tableFlags)) {
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

      DrawBackgroundSelection(pxr::SdfPrimSpecHandle(), _prim);
      //HandleDragAndDrop(layer);
      ImGui::SetItemAllowOverlap();
      std::string label = _layer->GetDisplayName();
      bool unfolded = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags);

      if (!ImGui::IsItemToggledOpen() && ImGui::IsItemClicked()) {
        _prim = pxr::SdfPrimSpecHandle();
      }

      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Add root prim")) {
          GetApplication()->AddCommand(std::shared_ptr<CreatePrimCommand>(
            new CreatePrimCommand(GetApplication()->GetWorkspace()->GetWorkLayer(), "/root")));
        }
        ImGui::EndPopup();
      }
      if (unfolded) {
        for (const auto& child : _layer->GetRootPrims()) {
          DrawPrimSpecRow(child, _prim, nodeId++, selectedPosY);
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
    /*
    if (ImGui::IsItemHovered() && _prim) {
      AddShortcut<PrimRemove, ImGuiKey_Delete>(selectedPrim);
      AddShortcut<PrimCopy, ImGuiKey_LeftCtrl, ImGuiKey_C>(selectedPrim);
      AddShortcut<PrimPaste, ImGuiKey_LeftCtrl, ImGuiKey_V>(selectedPrim);
      AddShortcut<PrimDuplicate, ImGuiKey_LeftCtrl, ImGuiKey_D>(selectedPrim,
        FindNextAvailablePrimName(selectedPrim->GetName()));

    }*/
  }

  ImGui::End();
  return true;
};
  

JVR_NAMESPACE_CLOSE_SCOPE