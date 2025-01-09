#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stageCache.h>
#include "../ui/contentBrowser.h"
#include "../utils/strings.h"
#include "../app/view.h"
#include "../app/model.h"
#include "../app/commands.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ContentBrowserUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;

ContentBrowserUI::ContentBrowserUI(View* parent)
  : BaseUI(parent, UIType::CONTENTBROWSER)
{
}

ContentBrowserUI::~ContentBrowserUI()
{
}

static void 
DrawLayerTooltip(SdfLayerHandle layer) 
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

static bool 
PassOptionsFilter(SdfLayerHandle layer, 
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

static inline void DrawSaveButton(SdfLayerHandle layer) 
{
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TRANSPARENT_COLOR));
  if (ImGui::SmallButton(layer->IsDirty() ? "###Save" : "  ###Save")) {
    ADD_COMMAND(SaveLayerCommand, layer);
    std::cout << "SAVE LAYER : " << layer->GetUniqueIdentifier() << std::endl;
  }
  ImGui::PopStyleColor();
}

static inline void DrawSelectStageButton(SdfLayerHandle layer, 
  bool isStage, SdfLayerHandle* selectedStage) 
{
  if (isStage) {
    if (selectedStage && *selectedStage == layer) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 1.0, 1.0, 1.0));
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6, 0.6, 0.6, 1.0));
    }
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TRANSPARENT_COLOR));
  }
  
  if (ImGui::SmallButton(layer->IsDirty() ? "###Select" : "  ###Select")) {
    //ExecuteAfterDraw(&SdfLayer::Save, layer, true);
    std::cout << "Select Stage : " << layer->GetUniqueIdentifier() << std::endl;
  }
  ImGui::PopStyleColor();
  /*
  ScopedStyleColor style(ImGuiCol_Button, ImVec4(ColorTransparent), ImGuiCol_Text,
    isStage ? ((selectedStage && *selectedStage == layer) ? 
      ImVec4(1.0, 1.0, 1.0, 1.0) : ImVec4(0.6, 0.6, 0.6, 1.0)) : ImVec4(ColorTransparent));
  if (ImGui::SmallButton(ICON_FA_DESKTOP "###Stage")) {
    ExecuteAfterDraw<EditorSetCurrentStage>(layer);
  }
  */
}

static void DrawContentBrowserMenuBar(ContentBrowserOptions& options) {
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
  ImGui::PopStyleColor();
}

/// Draw a popup menu with the possible action on a layer
static void DrawLayerActionPopupMenu(SdfLayerHandle layer) {
  if (!layer)
    return;
  if (ImGui::MenuItem("Edit layer")) {
    //ExecuteAfterDraw<EditorSetCurrentLayer>(layer);
    std::cout << "SET CURRENT LAYER" << layer.GetUniqueIdentifier() << std::endl;
  }
  if (!layer->IsAnonymous() && ImGui::MenuItem("Reload")) {
    ADD_COMMAND(ReloadLayerCommand, layer);
    std::cout << "RELOAD LAYER" << layer->GetRealPath() << std::endl;
  }
  if (ImGui::MenuItem("Open as Stage")) {
    ADD_COMMAND(OpenSceneCommand, layer->GetRealPath());
  }
  if (layer->IsDirty() && !layer->IsAnonymous() && ImGui::MenuItem("Save layer")) {
    ADD_COMMAND(SaveLayerCommand, layer);
    std::cout << "SAVE LAYER" << layer->GetRealPath() << std::endl;
  }
  if (ImGui::MenuItem("Save layer as")) {
    //ExecuteAfterDraw<EditorSaveLayerAs>(layer);
    std::cout << "SAVE AS LAYER" << layer->GetRealPath() << std::endl;
  }

  ImGui::Separator();

  // Not sure how safe this is with the undo/redo
  if (layer->IsMuted() && ImGui::MenuItem("Unmute")) {
    //ExecuteAfterDraw<LayerUnmute>(layer);
    std::cout << "UNMUTE LAYER" << layer->GetRealPath() << std::endl;
  }
  if (!layer->IsMuted() && ImGui::MenuItem("Mute")) {
    //ExecuteAfterDraw<LayerMute>(layer);
    std::cout << "MUTE LAYER" << layer->GetRealPath() << std::endl;
  }

  ImGui::Separator();
  if (ImGui::MenuItem("Copy layer path")) {
    ImGui::SetClipboardText(layer->GetRealPath().c_str());
  }
}

static void DrawLayerSet(UsdStageCache& cache, SdfLayerHandleSet& layerSet, SdfLayerHandle* selectedLayer, 
  SdfLayerHandle* selectedStage, const ContentBrowserOptions& options, const ImVec2& listSize = ImVec2(0, -10)) 
{

  static TextFilter filter;
  filter.Draw();

  ImGui::PushItemWidth(-1);
  if (ImGui::BeginListBox("##DrawLayerSet", listSize)) {
    // Sort the layer set
    std::vector<SdfLayerHandle> sortedSet(layerSet.begin(), layerSet.end());
    std::sort(sortedSet.begin(), sortedSet.end(), [&](const auto& t1, const auto& t2) {
      return LayerNameFromOptions(t1, options) < LayerNameFromOptions(t2, options);
    });
    //
    for (const auto& layer : sortedSet) {
      if (!layer)
        continue;
      std::string layerName = LayerNameFromOptions(layer, options);
      const bool passSearchFilter = filter.PassFilter(layerName.c_str());
      if (passSearchFilter) {
        const bool isStage = cache.FindOneMatching(layer);
        if (!PassOptionsFilter(layer, options, isStage)) {
          continue;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
        ImGui::PushID(layer->GetUniqueIdentifier());
        DrawSelectStageButton(layer, isStage, selectedStage);
        ImGui::SameLine();
        DrawSaveButton(layer);
        ImGui::PopStyleVar();
        ImGui::SameLine();
        DrawLayerDescriptionRow(layer, isStage, layerName, selectedLayer, selectedStage);

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
  const GfVec2f min(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  static ContentBrowserOptions options;
  DrawContentBrowserMenuBar(options);

  // TODO: we might want to remove completely the editor here, just pass as selected layer and a selected stage
  
  SdfLayerHandle selectedLayer(_model->GetRootLayer());
  SdfLayerHandle selectedStage(_model->GetStage() 
    ? SdfLayerHandle(_model->GetRootLayer()) : SdfLayerHandle());
  auto layers = SdfLayer::GetLoadedLayers();
  //DrawLayerSet(app->GetStageCache(), layers, &selectedLayer, &selectedStage, options);
  
  
  ImGui::End();
  return true;
}

JVR_NAMESPACE_CLOSE_SCOPE