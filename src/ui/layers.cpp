#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <cctype>

#include <pxr/usd/usd/schemaRegistry.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerUtils.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/schema.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/camera.h>

#include "../ui/layers.h"
#include "../ui/utils.h"
#include "../app/view.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE
/*
#include "Editor.h"
#include "Commands.h"
#include "ModalDialogs.h"
#include "FileBrowser.h"
#include "LayerEditor.h"
#include "PrimSpecEditor.h"
#include "CompositionEditor.h"
#include "ImGuiHelpers.h"
#include "Constants.h"

struct AddVariantModalDialog : public ModalDialog {

    AddVariantModalDialog(SdfPrimSpecHandle &primSpec) : _primSpec(primSpec){};

    ~AddVariantModalDialog() override {}

    void Draw() override {
        if (!_primSpec) {
            CloseModal();
            return;
        }
        bool isVariant = _primSpec->GetPath().IsPrimVariantSelectionPath();
        ImGui::InputText("VariantSet name", &_variantSet);
        ImGui::InputText("Variant name", &_variant);
        if (isVariant) {
            ImGui::Checkbox("Add to variant edit list", &_addToEditList);
        }
        //
        if (ImGui::Button("Add")) {
            // TODO The call might not be safe as _primSpec is copied, so create an actual command instead
            std::function<void()> func = [=]() {
                SdfCreateVariantInLayer(_primSpec->GetLayer(), _primSpec->GetPath(), _variantSet, _variant);
                if (isVariant && _addToEditList) {
                    auto ownerPath = _primSpec->GetPath().StripAllVariantSelections();
                    // This won't work on doubly nested variants,
                    // when there is a Prim between the variants
                    auto ownerPrim = _primSpec->GetPrimAtPath(ownerPath);
                    if (ownerPrim) {
                        auto nameList = ownerPrim->GetVariantSetNameList();
                        if (!nameList.ContainsItemEdit(_variantSet)) {
                            ownerPrim->GetVariantSetNameList().Add(_variantSet);
                        }
                    }
                }
            };
            ExecuteAfterDraw<UsdFunctionCall>(_primSpec->GetLayer(), func);
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            CloseModal();
        }
    }

    const char *DialogId() const override { return "Add variant"; }
    SdfPrimSpecHandle _primSpec;
    std::string _variantSet;
    std::string _variant;
    bool _addToEditList = false;
};

// Look for a new name. If prefix ends with a number, it will increase its value until
// a valid name/token is found
static std::string FindNextAvailablePrimName(std::string prefix) {
    // Find number in the prefix
    size_t end = prefix.size() - 1;
    while (end > 0 && std::isdigit(prefix[end])) {
        end--;
    }
    size_t padding = prefix.size() - 1 - end;
    const std::string number = prefix.substr(end + 1, padding);
    auto value = number.size() ? std::stoi(number) : 0;
    std::ostringstream newName;
    padding = padding == 0 ? 4 : padding; // 4: default padding
    do {
        value += 1;
        newName.seekp(0, std::ios_base::beg); // rewind
        newName << prefix.substr(0, end + 1) << std::setfill('0') << std::setw(padding) << value;
        // Looking for existing token with the same name.
        // There might be a better solution here
    } while (TfToken::Find(newName.str()) != TfToken());
    return newName.str();
}

void DrawTreeNodePopup(SdfPrimSpecHandle &primSpec) {
    if (!primSpec)
        return;

    if (ImGui::MenuItem("Add child")) {
        ExecuteAfterDraw<PrimNew>(primSpec, FindNextAvailablePrimName(DefaultPrimSpecName));
    }
    auto parent = primSpec->GetNameParent();
    if (parent) {
        if (ImGui::MenuItem("Add sibling")) {
            ExecuteAfterDraw<PrimNew>(parent, FindNextAvailablePrimName(primSpec->GetName()));
        }
    }

    if (ImGui::MenuItem("Remove")) {
        ExecuteAfterDraw<PrimRemove>(primSpec);
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Create reference")) {
        DrawPrimCreateReference(primSpec);
    }
    if (ImGui::MenuItem("Create payload")) {
        DrawPrimCreatePayload(primSpec);
    }
    if (ImGui::MenuItem("Create inherit")) {
        DrawPrimCreateInherit(primSpec);
    }
    if (ImGui::MenuItem("Create specialize")) {
        DrawPrimCreateSpecialize(primSpec);
    }
    if (ImGui::MenuItem("Create variant")) {
        DrawModalDialog<AddVariantModalDialog>(primSpec);
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Copy prim path")) {
        ImGui::SetClipboardText(primSpec->GetPath().GetString().c_str());
    }
}

static void DrawBackgroundSelection(const SdfPrimSpecHandle &currentPrim, SdfPrimSpecHandle &selectedPrim) {
    const bool selected = currentPrim == selectedPrim;

    const auto selectedColor = ImGui::GetColorU32(ImGuiCol_Header);
    ScopedStyleColor scopedStyle(ImGuiCol_HeaderHovered, selected ? selectedColor : 0,
                                 ImGuiCol_HeaderActive, selectedColor);

    const auto selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
    if (ImGui::Selectable("##backgroundSelectedPrim", selected, selectableFlags)) {
        selectedPrim = currentPrim;
    }

    ImGui::SameLine();
}

static void HandleDragAndDrop(SdfPrimSpecHandle &primSpec) {
    static SdfPath payload;
    // Drag and drop
    ImGuiDragDropFlags srcFlags = 0;
    srcFlags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
    srcFlags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening
                                                             // foreign treenodes/tabs while dragging
    // src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
    if (ImGui::BeginDragDropSource(srcFlags)) {
        if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
            ImGui::Text("Moving %s", primSpec->GetPath().GetString().c_str());

        payload = SdfPath(primSpec->GetPath());
        ImGui::SetDragDropPayload("DND", &payload, sizeof(SdfPath), ImGuiCond_Once);
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags targetFlags = 0;
        // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a
        // target) to do something target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow
        // rectangle
        if (const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("DND", targetFlags)) {
            SdfPath source(*(SdfPath *)pl->Data);
            ExecuteAfterDraw<PrimReparent>(primSpec->GetLayer(), source, primSpec->GetPath());
        }
        ImGui::EndDragDropTarget();
    }
}

// Returns unfolded
static bool DrawTreeNodePrimName(const bool &primIsVariant, SdfPrimSpecHandle &primSpec, SdfPrimSpecHandle &selectedPrim,
                                 bool hasChildren) {
    // Format text differently when the prim is a variant
    std::string primSpecName;
    if (primIsVariant) {
        auto variantSelection = primSpec->GetPath().GetVariantSelection();
        primSpecName = std::string("{") + variantSelection.first.c_str() + ":" + variantSelection.second.c_str() + "}";
    } else {
        primSpecName = primSpec->GetPath().GetName();
    }
    ScopedStyleColor textColor(ImGuiCol_Text,
                               primIsVariant ? ImU32(ImColor::HSV(0.2 / 7.0f, 0.5f, 0.8f)) : ImGui::GetColorU32(ImGuiCol_Text));

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
     nodeFlags |= hasChildren && !primSpec->HasVariantSetNames() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
    auto cursor = ImGui::GetCursorPos(); // Store position for the InputText to edit the prim name
    auto unfolded = ImGui::TreeNodeEx(primSpecName.c_str(), nodeFlags);

    // Edition of the prim name
    static SdfPrimSpecHandle editNamePrim;
    if (ImGui::IsItemClicked()) {
        selectedPrim = primSpec;
        if (editNamePrim != SdfPrimSpecHandle() && editNamePrim != selectedPrim) {
            editNamePrim = SdfPrimSpecHandle();
        }
        if (!primIsVariant && ImGui::IsMouseDoubleClicked(0)) {
            editNamePrim = primSpec;
        }
    }
    if (primSpec == editNamePrim) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 1.0));
        ImGui::SetCursorPos(cursor);
        DrawPrimName(primSpec); // Draw the prim name editor
        if (ImGui::IsItemDeactivatedAfterEdit() || !ImGui::IsItemFocused()) {
            editNamePrim = SdfPrimSpecHandle();
        }
        ImGui::PopStyleColor();
    }
    return unfolded;
}

/// Draw a node in the primspec tree
static void DrawPrimSpecRow(SdfPrimSpecHandle primSpec, SdfPrimSpecHandle &selectedPrim, int nodeId) {

    if (!primSpec)
        return;
    bool primIsVariant = primSpec->GetPath().IsPrimVariantSelectionPath();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);

    ImGui::PushID(nodeId);
    nodeId = 0; // reset the counter

    DrawBackgroundSelection(primSpec, selectedPrim);

    // Drag and drop on Selectable
    HandleDragAndDrop(primSpec);

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
    DrawPrimSpecifierCombo(primSpec, ImGuiComboFlags_NoArrowButton);

    ImGui::TableSetColumnIndex(2);
    ImGui::PushItemWidth(-FLT_MIN); // removes the combo label. The col needs to have a fixed size
    DrawPrimType(primSpec, ImGuiComboFlags_NoArrowButton);

    // End of transparent combos
    ImGui::PopStyleColor();

    // Draw composition summary
    ImGui::TableSetColumnIndex(3);
    DrawPrimCompositionSummary(primSpec);

    // Draw children
    if (unfolded) {
        SdfVariantSetsProxy variantSetMap = primSpec->GetVariantSets();
        TF_FOR_ALL(varSetIt, variantSetMap) {
            const SdfVariantSetSpecHandle &varSetSpec = varSetIt->second;
            const SdfVariantSpecHandleVector &variants = varSetSpec->GetVariantList();
            TF_FOR_ALL(varIt, variants) {
                const SdfPrimSpecHandle &variantSpec = (*varIt)->GetPrimSpec();
                DrawPrimSpecRow(variantSpec, selectedPrim, nodeId++);
            }
        }

        for (int i = 0; i < childrenNames.size(); ++i) {
            DrawPrimSpecRow(childrenNames[i], selectedPrim, nodeId++);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void DrawLayerPrimHierarchy(SdfLayerRefPtr layer, SdfPrimSpecHandle &selectedPrim) {

    if (!layer)
        return;
    if (ImGui::Button("Add root prim")) {
        ExecuteAfterDraw<PrimNew>(layer, FindNextAvailablePrimName(DefaultPrimSpecName));
    }
    ImGui::SameLine();
    if (ImGui::Button("Move up")&&selectedPrim) {
        ExecuteAfterDraw<PrimReorder>(selectedPrim, true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Move down")&&selectedPrim) {
        ExecuteAfterDraw<PrimReorder>(selectedPrim, false);
    }
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
        if (layer->GetRootPrims().empty()) {
            treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        DrawBackgroundSelection(SdfPrimSpecHandle(), selectedPrim);

        std::string label = std::string(ICON_FA_FILE) + " " + layer->GetDisplayName();
        bool unfolded = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags );

        if (ImGui::IsItemClicked()) {
            selectedPrim = SdfPrimSpecHandle();
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Add root prim")) {
                ExecuteAfterDraw<PrimNew>(layer, FindNextAvailablePrimName(DefaultPrimSpecName));
            }
            ImGui::EndPopup();
        }
        if (unfolded) {
            for (const auto &child : layer->GetRootPrims()) {
                DrawPrimSpecRow(child, selectedPrim, nodeId++);
            }
            ImGui::TreePop();
        }
        ImGui::EndTable();
    }
}

*/
ImGuiWindowFlags LayersUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove;


LayersUI::LayersUI(View* parent) 
  : BaseUI(parent, UIType::LAYEREDITOR)
{

}

LayersUI::~LayersUI()
{
}

static void
OnSetPreviousLayer()
{
  std::cout << "PREVIOUS LAYER CALLED..." << std::endl;
}

static void
OnSetNextLayer()
{
  std::cout << "NEXT LAYER CALLED..." << std::endl;
}

static void
OnSaveLayer()
{
  std::cout << "PREVIOUS LAYER CALLED..." << std::endl;
}

static void
OnReloadLayer()
{
  std::cout << "NEXT LAYER CALLED..." << std::endl;
}

void 
LayersUI::DrawNavigation(pxr::SdfLayerRefPtr layer) 
{
  if (!layer) return;
  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, LayersUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnSetPreviousLayer, this);
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, LayersUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnSetNextLayer, this);
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, LayersUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnReloadLayer, this);
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, LayersUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnSaveLayer, this);
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, LayersUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnSaveLayer, this);
  ImGui::SameLine();
  /*
  if (ImGui::Button("Move up")) {
    ExecuteAfterDraw<LayerMoveSubLayer>(selectedParent, selectedLayerPath, true);
  }
  ImGui::SameLine();
  if (ImGui::Button("Move down")) {
    ExecuteAfterDraw<LayerMoveSubLayer>(selectedParent, selectedLayerPath, false);
  }
  ImGui::Separator();
  constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
  if (ImGui::BeginTable("##DrawLayerSublayers", 1, tableFlags)) {
    ImGui::TableSetupColumn("Layers");
    ImGui::TableHeadersRow();
    DrawLayerSublayerTree(layer, SdfLayerRefPtr(), std::string(), selectedParent, selectedLayerPath);
    ImGui::EndTable();
  }
  */

  if (!layer)
    return;

  {
    //ScopedStyleColor textBackground(ImGuiCol_Header, ImU32(ImColor{ ColorPrimHasComposition }));
    ImGui::Selectable("##LayerNavigation");
    ImGui::SameLine();
    ImGui::Text("Layer: %s", layer->GetRealPath().c_str());
  }
}

bool LayersUI::Draw()
{
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  Application* app = GetApplication();

  if (app->GetWorkStage())
  DrawNavigation(app->GetWorkStage()->GetRootLayer());
    /*
    const ImVec2 localMousePos = ImGui::GetMousePos() - _parent->GetMin();

    Application* app = GetApplication();

    if (app->GetStage())
    {
        // setup transparent background
        ImGui::PushStyleColor(ImGuiCol_Header, TRANSPARENT_COLOR);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, TRANSPARENT_COLOR);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, TRANSPARENT_COLOR);

        // setup columns
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, _parent->GetWidth() - 100);
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
        for (auto& item : _root->_items) {
            DrawItem(item, true);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    }
    */
    ImGui::End();

    return false;
}

JVR_NAMESPACE_CLOSE_SCOPE
