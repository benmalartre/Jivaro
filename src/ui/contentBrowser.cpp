#include "../ui/contentBrowser.h"
#include "../utils/strings.h"
#include "../app/view.h"
#include "../app/application.h""


PXR_NAMESPACE_OPEN_SCOPE

ContentBrowserUI::ContentBrowserUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
{
}

ContentBrowserUI::~ContentBrowserUI()
{
}

void 
ContentBrowserUI::DrawLayerTooltip(pxr::SdfLayerHandle layer) 
{
  ImGui::SetTooltip("%s\n%s", layer->GetRealPath().c_str(), layer->GetIdentifier().c_str());
  auto assetInfo = layer->GetAssetInfo();
  if (!assetInfo.IsEmpty()) {
    if (assetInfo.CanCast<VtDictionary>()) {
      auto assetInfoDict = assetInfo.Get<VtDictionary>();
      TF_FOR_ALL(keyValue, assetInfoDict) {
        std::stringstream ss;
        ss << keyValue->second;
        ImGui::SetTooltip("%s %s", keyValue->first.c_str(), ss.str().c_str());
      }
    }
  }
}

bool 
ContentBrowserUI::PassOptionsFilter(SdfLayerHandle layer, 
   const ContentBrowserOptions& options, bool isStage)
{
  if (!options._filterAnonymous) {
    if (layer->IsAnonymous())
      return false;
  }
  if (!options._filterFiles) {
    if (!layer->IsAnonymous())
      return false;
  }
  if (!options._filterModified) {
    if (layer->IsDirty())
      return false;
  }
  if (!options._filterUnmodified) {
    if (!layer->IsDirty())
      return false;
  }
  if (!options._filterStage && isStage) {
    return false;
  }
  if (!options._filterLayer && !isStage) {
    return false;
  }
  return true;
}

inline void DrawSaveButton(SdfLayerHandle layer) 
{
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TRANSPARENT_COLOR));
  if (ImGui::SmallButton(layer->IsDirty() ? "###Save" : "  ###Save")) {
    //ExecuteAfterDraw(&SdfLayer::Save, layer, true);
    std::cout << "SAVE LAYER : " << layer->GetUniqueIdentifier() << std::endl;
  }
  ImGui::PopStyleColor();
}

static inline std::string LayerNameFromOptions(SdfLayerHandle layer, const ContentBrowserOptions& options) {
  if (options._showAssetName) {
    return layer->GetAssetName();
  }
  else if (options._showDisplayName) {
    return layer->GetDisplayName();
  }
  else if (options._showRealPath) {
    return layer->GetRealPath();
  }
  return layer->GetIdentifier();
}

static inline void DrawLayerDescriptionRow(SdfLayerHandle layer, bool isStage, std::string& layerName, bool selected,
  SdfLayerHandle* selectedLayer) {
  ImGui::PushStyleColor(ImGuiCol_Text, isStage ? ImVec4(1.0, 1.0, 1.0, 1.0) : ImVec4(0.6, 0.6, 0.6, 1.0));
  if (ImGui::Selectable(layerName.c_str(), selected)) {
    if (selectedLayer) {
      *selectedLayer = layer;
    }
  }
  if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
    if (!isStage) {
      //ExecuteAfterDraw<EditorOpenStage>(layer->GetRealPath());
      std::cout << "OPEN STAGE : " << layer->GetRealPath() << std::endl;
    }
    else {
      std::cout << "SET CURRENT STAGE..." << layer.GetUniqueIdentifier() << std::endl;
      //ExecuteAfterDraw<EditorSetCurrentStage>(layer);
    }
  }
}

template <typename SdfLayerSetT>
void DrawLayerSet(UsdStageCache& cache, SdfLayerSetT& layerSet, SdfLayerHandle* selectedLayer,
  const ContentBrowserOptions& options, const ImVec2& listSize = ImVec2(0, -10)) {

  // Sort the layer set
  std::vector<SdfLayerHandle> sortedSet(layerSet.begin(), layerSet.end());
  std::sort(sortedSet.begin(), sortedSet.end(), [&](const auto& t1, const auto& t2) {
    return LayerNameFromOptions(t1, options) < LayerNameFromOptions(t2, options);
  });
  static TextFilter filter;
  filter.Draw();
  ImGui::PushItemWidth(-1);
  if (ImGui::BeginListBox("##DrawLayerSet", listSize)) {
    for (const auto& layer : sortedSet) {
      if (!layer)
        continue;
      bool selected = selectedLayer && *selectedLayer == layer;
      std::string layerName = LayerNameFromOptions(layer, options);
      const bool passSearchFilter = filter.PassFilter(layerName.c_str());
      if (passSearchFilter) {
        const bool isStage = cache.FindOneMatching(layer);
        if (!PassOptionsFilter(layer, options, isStage)) {
          continue;
        }
        ImGui::PushID(layer->GetUniqueIdentifier());
        DrawSaveButton(layer);
        ImGui::SameLine();
        DrawLayerDescriptionRow(layer, isStage, layerName, selected, selectedLayer);
        if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 2) {
          DrawLayerTooltip(layer);
        }
        if (ImGui::BeginPopupContextItem()) {
          DrawLayerActionPopupMenu(layer);
          ImGui::EndPopup();
        }
        ImGui::PopID();
      }
    }
    ImGui::EndListBox();
  }
  ImGui::PopItemWidth();
}

bool ContentBrowserUI::Draw()
{
  bool opened;
  int flags =
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoMove;

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());

  static ContentBrowserOptions options;
  if (ImGui::BeginMenuBar()) {
    bool selected = true;
    if (ImGui::BeginMenu("Filter")) {
      ImGui::MenuItem("Anonymous", nullptr, &options._filterAnonymous, true);
      ImGui::MenuItem("File", nullptr, &options._filterFiles, true);
      ImGui::Separator();
      ImGui::MenuItem("Unmodified", nullptr, &options._filterUnmodified, true);
      ImGui::MenuItem("Modified", nullptr, &options._filterModified, true);
      ImGui::Separator();
      ImGui::MenuItem("Stage", nullptr, &options._filterStage, true);
      ImGui::MenuItem("Layer", nullptr, &options._filterLayer, true);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Show")) {
      if (ImGui::MenuItem("Asset name", nullptr, &options._showAssetName, true)) {
        options._showRealPath = options._showIdentifier = options._showDisplayName = false;
      }
      if (ImGui::MenuItem("Identifier", nullptr, &options._showIdentifier, true)) {
        options._showAssetName = options._showRealPath = options._showDisplayName = false;
      }
      if (ImGui::MenuItem("Display name", nullptr, &options._showDisplayName, true)) {
        options._showAssetName = options._showIdentifier = options._showRealPath = false;
      }
      if (ImGui::MenuItem("Real path", nullptr, &options._showRealPath, true)) {
        options._showAssetName = options._showIdentifier = options._showDisplayName = false;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // TODO: we might want to remove completely the editor here, just pass as selected layer and a selected stage
  SdfLayerHandle selected(GetApplication()->GetWorkStage()->GetRootLayer());
  auto layers = SdfLayer::GetLoadedLayers();
  //DrawLayerSet(GetApplication()->GetStageCache(), layers, &selected, options);
  /*
  if (selected != editor.GetCurrentLayer()) {
    editor.SetCurrentLayer(selected);
  }
  */
  ImGui::End();
  return true;
}

PXR_NAMESPACE_CLOSE_SCOPE