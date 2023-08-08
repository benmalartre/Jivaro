#ifndef JVR_UI_UTILS_H
#define JVR_UI_UTILS_H
#include <cassert>
#include <functional>

#include <pxr/usd/usd/attribute.h>
#include <pxr/base/gf/matrix2f.h>
#include <pxr/base/gf/matrix2d.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec2h.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3h.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/vec4h.h>

#include <pxr/usd/sdf/listEditorProxy.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/listOp.h>

#include "../common.h"
#include "../ui/style.h"
#include "../ui/fonts.h"
#include "../ui/utils.h"
#include "../utils/icons.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_stdlib.h"

PXR_NAMESPACE_OPEN_SCOPE
// Unfortunately Inherit and Specialize are just alias to SdfPath
// To reuse templated code we create new type SdfInherit and SdfSpecialize 
template <int InheritOrSpecialize> struct SdfInheritOrSpecialize : pxr::SdfPath {
  SdfInheritOrSpecialize() : pxr::SdfPath() {}
  SdfInheritOrSpecialize(const pxr::SdfPath &path) : pxr::SdfPath(path) {}
};

using SdfInherit = SdfInheritOrSpecialize<0>;
using SdfSpecialize = SdfInheritOrSpecialize<1>;
PXR_NAMESPACE_CLOSE_SCOPE

JVR_NAMESPACE_OPEN_SCOPE

const pxr::GfVec2f BUTTON_LARGE_SIZE(64.f, 64.f);
const pxr::GfVec2f BUTTON_NORMAL_SIZE(24.f, 24.f);
const pxr::GfVec2f BUTTON_MINI_SIZE(16.f, 20.f);

/// Height of a row in the property editor
constexpr float TABLE_ROW_DEFAULT_HEIGHT = 22.f;

class View;
class UIUtils {
public:

  static void HelpMarker(const char* desc);

  static void AddAttributeDisplayName(const pxr::UsdAttribute& attribute);
  static pxr::VtValue AddTokenWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddColorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddAttributeWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename VectorType, int DataType, int N>
  static pxr::VtValue AddVectorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename MatrixType, int DataType, int Rows, int Cols>
  static pxr::VtValue AddMatrixWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  static void IconButton(const char* icon, short state, CALLBACK_FN func);
  static bool AddIconButton(const char* icon, short state, CALLBACK_FN func);
  static bool AddIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  static bool AddTransparentIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  static bool AddCheckableIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);

  static void AddPropertyMiniButton(const char* btnStr, int rowId, 
    const ImVec4& btnColor = ImVec4(0.0, 0.7, 0.0, 1.0));
  static void AddPrimKind(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimType(const pxr::SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  static void AddPrimSpecifier(const pxr::SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  static void AddPrimInstanceable(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimHidden(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimActive(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimName(const pxr::SdfPrimSpecHandle& primSpec);

};

template <typename MatrixType, int DataType, int Rows, int Cols>
pxr::VtValue 
UIUtils::AddMatrixWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  MatrixType buffer = value.Get<MatrixType>();
  bool valueChanged = false;
    
  ImGui::PushID(attribute.GetName().GetText());
  ImGui::InputScalarN("###row0", DataType, buffer.data(), Cols, NULL, NULL, DecimalPrecision);
  valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
  ImGui::InputScalarN("###row1", DataType, buffer.data() + Cols, Cols, NULL, NULL, DecimalPrecision);
  valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
  if (Rows > 2) {
    ImGui::InputScalarN("###row2", DataType, buffer.data() + 2 * Cols, Cols, NULL, NULL, DecimalPrecision);
    valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    if (Rows > 3) {
      ImGui::InputScalarN("###row3", DataType, buffer.data() + 3 * Cols, Cols, NULL, NULL, DecimalPrecision);
      valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    }
  }
  ImGui::PopID();
    
  if (valueChanged) {
    return pxr::VtValue(buffer);
  }
  return pxr::VtValue();
}

template <typename VectorType, int DataType, int N>
pxr::VtValue 
UIUtils::AddVectorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  VectorType buffer;
  attribute.Get<VectorType>(&buffer, timeCode);
  constexpr const char* format = DataType == ImGuiDataType_S32 ? "%d" : DecimalPrecision;
  ImGui::InputScalarN(attribute.GetName().GetText(), DataType, buffer.data(), N,
    NULL, NULL, format, ImGuiInputTextFlags());
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    return pxr::VtValue(VectorType(buffer));
  }
  return pxr::VtValue();
}

// ExtraArgsT is used to pass additional arguments as the function passed as visitor
// might need more than the operation and the item
template <typename PolicyT, typename FuncT, typename... ExtraArgsT>
static void IterateListEditorItems(const pxr::SdfListEditorProxy<PolicyT> &listEditor, 
  const FuncT &call, ExtraArgsT... args) 
{
  // TODO: should we check if the list is already all explicit ??
  for (const typename PolicyT::value_type &item : listEditor.GetExplicitItems()) {
    call(pxr::SdfListOpTypeExplicit, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetOrderedItems()) {
    call(pxr::SdfListOpTypeOrdered, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetAddedItems()) {
    call(pxr::SdfListOpTypeAdded, item, args...); // return "add" as TfToken instead ?
  }
  for (const typename PolicyT::value_type &item : listEditor.GetPrependedItems()) {
    call(pxr::SdfListOpTypePrepended, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetAppendedItems()) {
    call(pxr::SdfListOpTypeAppended, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetDeletedItems()) {
    call(pxr::SdfListOpTypeDeleted, item, args...);
  }
}

/// The list available on a SdfListEditor
constexpr int GetListEditorOperationSize() { return 6; }

template <typename IntOrSdfListOpT> inline const char *GetListEditorOperationName(IntOrSdfListOpT index) {
    constexpr const char *names[GetListEditorOperationSize()] = 
      {"explicit", "add", "delete", "ordered", "prepend", "append"};
    return names[static_cast<int>(index)];
}

template <typename IntOrSdfListOpT> inline const char *GetListEditorOperationAbbreviation(IntOrSdfListOpT index) {
    constexpr const char *names[GetListEditorOperationSize()] =
      {"Ex", "Ad", "De", "Or", "Pr", "Ap"};
    return names[static_cast<int>(index)];
}

template <typename PolicyT>
inline void CreateListEditorOperation(pxr::SdfListEditorProxy<PolicyT> &&listEditor, pxr::SdfListOpType op,
                                      typename pxr::SdfListEditorProxy<PolicyT>::value_type item) 
{
  switch (op) {
  case pxr:: SdfListOpTypeAdded:
    listEditor.GetAddedItems().push_back(item);
    break;
  case pxr::SdfListOpTypePrepended:
    listEditor.GetPrependedItems().push_back(item);
    break;
  case pxr::SdfListOpTypeAppended:
      listEditor.GetAppendedItems().push_back(item);
      break;
  case pxr::SdfListOpTypeDeleted:
      listEditor.GetDeletedItems().push_back(item);
      break;
  case pxr::SdfListOpTypeExplicit:
      listEditor.GetExplicitItems().push_back(item);
      break;
  case pxr::SdfListOpTypeOrdered:
      listEditor.GetOrderedItems().push_back(item);
      break;
  default:
      assert(0);
  }
}

template <typename ListEditorT, typename OpOrIntT> 
inline auto GetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_)
 {
    const pxr::SdfListOpType op = static_cast<pxr::SdfListOpType>(op_);
    if (op == pxr::SdfListOpTypeOrdered) {
        return listEditor.GetOrderedItems();
    } else if (op == pxr::SdfListOpTypeAppended) {
        return listEditor.GetAppendedItems();
    } else if (op == pxr::SdfListOpTypeAdded) {
        return listEditor.GetAddedItems();
    } else if (op == pxr::SdfListOpTypePrepended) {
        return listEditor.GetPrependedItems();
    } else if (op == pxr::SdfListOpTypeDeleted) {
        return listEditor.GetDeletedItems();
    }
    return listEditor.GetExplicitItems();
};


template <typename ListEditorT, typename OpOrIntT, typename ItemsT> 
inline void SetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_, const ItemsT &items) 
{
    const pxr::SdfListOpType op = static_cast<pxr::SdfListOpType>(op_);
    if (op == pxr::SdfListOpTypeOrdered) {
        listEditor.SetOrderedItems(items);
    } else if (op == pxr::SdfListOpTypeAppended) {
        listEditor.SetAppendedItems(items);
    } else if (op == pxr::SdfListOpTypeAdded) {
        listEditor.SetAddedItems(items);
    } else if (op == pxr::SdfListOpTypePrepended) {
        listEditor.SetPrependedItems(items);
    } else if (op == pxr::SdfListOpTypeDeleted) {
        listEditor.SetDeletedItems(items);
    } else {
        listEditor.SetExplicitItems(items);
    }
};

template <typename Policy>
void CreateListEditorOperation(pxr::SdfListEditorProxy<Policy>&& listEditor, int operation,
  typename pxr::SdfListEditorProxy<Policy>::value_type item) {
  switch (operation) {
  case 0:
    listEditor.Add(item);
    break;
  case 1:
    listEditor.Prepend(item);
    break;
  case 2:
    listEditor.Append(item);
    break;
  case 3:
    listEditor.Remove(item);
    break;
  case 4:
    listEditor.GetExplicitItems().push_back(item);
    break;
  default:
    assert(0);
  }
}

/// Remove SdfPath from their lists (Inherits and Specialize)
template <typename ListT> inline 
void RemovePathFromList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &item);
template <> inline 
void RemovePathFromList<pxr::SdfInherit>(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &item) 
{
    primSpec->GetInheritPathList().RemoveItemEdits(item);
}
template <> inline 
void RemovePathFromList<pxr::SdfSpecialize>(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &item) 
{
    primSpec->GetSpecializesList().RemoveItemEdits(item);
}

// forward decl
template<typename ArcT>
inline void RemoveArc(const pxr::SdfPrimSpecHandle &primSpec, const ArcT &arc);


/// Draw the menu items for AssetPaths (SdfReference and SdfPayload)
template <typename AssetPathT> 
void AddAssetPathMenuItems(const pxr::SdfPrimSpecHandle &primSpec, const AssetPathT &assetPath) 
{
  if (ImGui::MenuItem("Select Arc")) {
    auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(assetPath.GetAssetPath());
    //ExecuteAfterDraw<EditorFindOrOpenLayer>(realPath);
  }
  if (ImGui::MenuItem("Open as Stage")) {
    auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(assetPath.GetAssetPath());
    //ExecuteAfterDraw<EditorOpenStage>(realPath);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Remove")) {
    if (!primSpec)
        return;
    RemoveArc(primSpec, assetPath);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy asset path")) {
    ImGui::SetClipboardText(assetPath.GetAssetPath().c_str());
  }
}

template <typename AssetPathT> 
AssetPathT AddSdfPathEditor(const pxr::SdfPrimSpecHandle &primSpec, 
  const AssetPathT& arc, ImVec2 outerSize) 
{
  AssetPathT updatedArc;
  std::string updatedPath = arc.GetString();
  constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths |
                                          ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable;
  if (ImGui::BeginTable("DrawSdfPathEditor", 1, tableFlags, outerSize)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##assetpath", &updatedPath);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      updatedArc = AssetPathT(updatedPath);
    }
    ImGui::EndTable();
  }
  return updatedArc;
}

template <typename ArcT> 
void AddSdfPathMenuItems(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &path) 
{
  if (ImGui::MenuItem("Remove")) {
    if (!primSpec)
        return;
    RemoveArc(primSpec, ArcT(path));
  }
  if (ImGui::MenuItem("Copy path")) {
    ImGui::SetClipboardText(path.GetString().c_str());
  }
}

/*
// Add a row in a table for SdfReference or SdfPayload
// It's templated to keep the formatting consistent between payloads and references
// and avoid to duplicate the code
template <typename AssetPathT>
static void AddAssetPathRow(const char *operationName, const AssetPathT &item,
                             const pxr::SdfPrimSpecHandle &primSpec) { // TODO:primSpec might not be useful here
  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);
  if (ImGui::SmallButton(ICON_FA_TRASH)) { // TODO: replace with the proper menu
    if (!primSpec)
      return;
    std::function<void()> removeAssetPath = [=]() { RemoveAssetPathFromList(primSpec, item); };
    //ExecuteAfterDraw<UsdFunctionCall>(primSpec->GetLayer(), removeAssetPath);
  }
  ImGui::TableSetColumnIndex(1);
  ImGui::Text("%s", operationName);
  ImGui::TableSetColumnIndex(2);
  std::string pathFormated;
  if (!item.GetAssetPath().empty()) {
    pathFormated += "@" + item.GetAssetPath() + "@";
  }
  if (!item.GetPrimPath().GetString().empty()) {
    pathFormated += "<" + item.GetPrimPath().GetString() + ">";
  }
  ImGui::Text("%s", pathFormated.c_str());
  if (ImGui::BeginPopupContextItem(item.GetPrimPath().GetString().c_str())) {
    AddAssetPathMenuItems(primSpec, item);
    ImGui::EndPopup();
  }
}
*/

/// Composition
// 2 local struct to differentiate Inherit and Specialize which have the same underlying type
struct Inherit {};
struct Specialize {};

static bool HasComposition(const pxr::SdfPrimSpecHandle &primSpec) 
{
  return
    primSpec->HasReferences() || 
    primSpec->HasPayloads() || 
    primSpec->HasInheritPaths() || 
    primSpec->HasSpecializes();
}

inline pxr::SdfReferencesProxy 
GetCompositionArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfReference &val) 
{
  return primSpec->GetReferenceList();
}

inline pxr::SdfPayloadsProxy 
GetCompositionArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPayload &val) 
{
  return primSpec->GetPayloadList();
}

inline pxr::SdfInheritsProxy 
GetCompositionArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfInherit &val) 
{
  return primSpec->GetInheritPathList();
}

inline pxr::SdfSpecializesProxy 
GetCompositionArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfSpecialize &val) 
{
  return primSpec->GetSpecializesList();
}

inline void 
ClearArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfReference &val) 
{
  return primSpec->ClearReferenceList();
}

inline void 
ClearArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPayload &val) 
{
  return primSpec->ClearPayloadList();
}

inline void 
ClearArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfInherit &val) 
{
  return primSpec->ClearInheritPathList();
}

inline void 
ClearArcList(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfSpecialize &val) 
{
  return primSpec->ClearSpecializesList();
}

template<typename ArcT>
inline void RemoveArc(const pxr::SdfPrimSpecHandle &primSpec, const ArcT &arc) {
  std::function<void()> removeItem = [=]() {
    GetCompositionArcList(primSpec, arc).RemoveItemEdits(arc);
    // Also clear the arc list if there are no more items
    if (GetCompositionArcList(primSpec, arc).HasKeys()) {
      ClearArcList(primSpec, arc);
    }
  };
  //ExecuteAfterDraw<UsdFunctionCall>(primSpec->GetLayer(), removeItem);
}

inline void 
SelectArcType(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfReference &ref) 
{
  auto realPath = ref.GetAssetPath().empty() ? primSpec->GetLayer()->GetRealPath()
                                              : primSpec->GetLayer()->ComputeAbsolutePath(ref.GetAssetPath());
  auto layerOrOpen = pxr::SdfLayer::FindOrOpen(realPath);
  //ExecuteAfterDraw<EditorSetSelection>(layerOrOpen, ref.GetPrimPath());
}

inline void 
SelectArcType(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPayload &pay) 
{
  auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(pay.GetAssetPath());
  auto layerOrOpen = pxr::SdfLayer::FindOrOpen(realPath);
  //ExecuteAfterDraw<EditorSetSelection>(layerOrOpen, pay.GetPrimPath());
}

inline void 
SelectArcType(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &path) {
  //ExecuteAfterDraw<EditorSetSelection>(primSpec->GetLayer(), path);
}

inline void AddArcTypeMenuItems(const pxr::SdfPrimSpecHandle &primSpec, 
  const pxr::SdfReference &ref) 
{
  AddAssetPathMenuItems(primSpec, ref);
}

inline void AddArcTypeMenuItems(const pxr::SdfPrimSpecHandle &primSpec, 
  const pxr::SdfPayload &pay) 
{
  AddAssetPathMenuItems(primSpec, pay);
}

inline void AddArcTypeMenuItems(const pxr::SdfPrimSpecHandle &primSpec,
  const pxr::SdfInherit &inh) 
{
  AddSdfPathMenuItems<pxr::SdfInherit>(primSpec, inh);
}
inline void AddArcTypeMenuItems(const pxr::SdfPrimSpecHandle &primSpec, 
  const pxr::SdfSpecialize &spe) 
{
  AddSdfPathMenuItems<pxr::SdfSpecialize>(primSpec, spe);
}

#define CREATE_COMPOSITION_BUTTON(NAME_, ABBR_, LIST_)                                                                        \
if (primSpec->Has##NAME_##s()) {                                                                                             \
  if (buttonId > 0)                                                                                                        \
    ImGui::SameLine();                                                                                                   \
  ImGui::PushID(buttonId++);                                                                                               \
  ImGui::SmallButton(#ABBR_);                                                                                              \
  if (ImGui::BeginPopupContextItem(nullptr, buttonFlags)) {                                                                \
    IterateListEditorItems(primSpec->Get##LIST_##List(), Add##LIST_##Summary, primSpec, buttonId);                \
    ImGui::EndPopup();                                                                                                   \
  }                                                                                                                        \
  ImGui::PopID();                                                                                                          \
}

inline std::string GetArcSummary(const pxr::SdfReference &arc) 
{ 
  return arc.IsInternal() ? arc.GetPrimPath().GetString() : arc.GetAssetPath(); 
}

inline std::string GetArcSummary(const pxr::SdfPayload &arc) 
{ 
  return arc.GetAssetPath(); 
}

inline std::string GetArcSummary(const pxr::SdfPath &arc) 
{ 
  return arc.GetString(); 
}

void AddReferenceSummary(pxr::SdfListOpType operation, const pxr::SdfReference &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddPayloadSummary(pxr::SdfListOpType operation, const pxr::SdfPayload &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddInheritPathSummary(pxr::SdfListOpType operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddSpecializesSummary(pxr::SdfListOpType operation, const pxr::SdfPath &path,
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

template <typename ArcT>
inline
void AddPathInRow(pxr::SdfListOpType operation, const ArcT &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int *itemId) {
    std::string path;
    path += GetArcSummary(assetPath);
    ImGui::PushID((*itemId)++);
    ImGui::SameLine();
    if(ImGui::Button(path.c_str())) {
      SelectArcType(primSpec, assetPath);
    }
    if (ImGui::BeginPopupContextItem("###AssetPathMenuItems")) {
      AddArcTypeMenuItems(primSpec, assetPath);
      ImGui::EndPopup();
    }
    ImGui::PopID();
}

void 
AddPrimCompositionSummary(const pxr::SdfPrimSpecHandle &primSpec);

// Works with SdfReference and SdfPayload
template <typename ReferenceOrPayloadT>
ReferenceOrPayloadT DrawReferenceOrPayloadEditor(const pxr::SdfPrimSpecHandle &primSpec, 
  const ReferenceOrPayloadT &ref, ImVec2 outerSize) 
{
  ReferenceOrPayloadT ret;
  std::string updatedPath = ref.GetAssetPath();
  std::string targetPath = ref.GetPrimPath().GetString();
  pxr::SdfLayerOffset layerOffset = ref.GetLayerOffset();
  float offset = layerOffset.GetOffset();
  float scale = layerOffset.GetScale();
  ImGui::PushID("DrawAssetPathArcEditor");
  constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths |
                                          ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable;
  if (ImGui::BeginTable("DrawAssetPathArcEditorTable", 4, tableFlags, outerSize)) {
    const float stretchLayer = (layerOffset == pxr::SdfLayerOffset()) ? 0.01 : 0.1;
    const float stretchTarget = (targetPath.empty()) ? 0.01 : 0.3 * (1 - 2 * stretchLayer);
    const float stretchPath = 1 - 2 * stretchLayer - stretchTarget;
    ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch, stretchPath);
    ImGui::TableSetupColumn("target", ImGuiTableColumnFlags_WidthStretch, stretchTarget);
    ImGui::TableSetupColumn("offset", ImGuiTableColumnFlags_WidthStretch, stretchLayer);
    ImGui::TableSetupColumn("scale", ImGuiTableColumnFlags_WidthStretch, stretchLayer);
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##assetpath", &updatedPath);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      ret = ref;
      ret.SetAssetPath(updatedPath);
    }
    // TODO more operation on the path: unixify, make relative, etc
    if (ImGui::BeginPopupContextItem("sublayer")) {
      ImGui::Text("%s", updatedPath.c_str());
      AddAssetPathMenuItems(primSpec, ref);
      ImGui::EndPopup();
    }
    ImGui::SameLine();

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##targetpath", &targetPath);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      ret = ref;
      ret.SetPrimPath(pxr::SdfPath(targetPath));
    }

    ImGui::TableSetColumnIndex(2);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputFloat("##offset", &offset);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      pxr::SdfLayerOffset updatedLayerOffset(layerOffset);
      updatedLayerOffset.SetOffset(offset);
      ret = ref;
      ret.SetLayerOffset(updatedLayerOffset);
    }

    ImGui::TableSetColumnIndex(3);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputFloat("##scale", &scale);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      pxr::SdfLayerOffset updatedLayerOffset(layerOffset);
      updatedLayerOffset.SetScale(scale);
      ret = ref;
      ret.SetLayerOffset(updatedLayerOffset);
    }
    ImGui::EndTable();
  }
  ImGui::PopID();
  return ret;
}


/// Add modal dialogs to add composition on primspec (reference, payload, inherit, specialize)
void AddPrimCreateReference(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreatePayload(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreateInherit(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreateSpecialize(const pxr::SdfPrimSpecHandle &primSpec);

/// Add multiple tables with the compositions (Reference, Payload, Inherit, Specialize)
void AddPrimCompositions(const pxr::SdfPrimSpecHandle &primSpec);

// Add a text summary of the composition
void AddPrimCompositionSummary(const pxr::SdfPrimSpecHandle &primSpec);

/// Function to convert a hash from usd to ImGuiID with a seed, to avoid collision with path coming from layer and stages.
template <ImU32 seed, typename T> inline ImGuiID ToImGuiID(const T &val) {
    return ImHashData(static_cast<const void *>(&val), sizeof(T), seed);
}

//// Correctly indent the tree nodes using a path. This is used when we are iterating in a list of paths as opposed to a tree.
//// It allocates a vector which might not be optimal, but it should be used only on visible items, that should mitigate the
//// allocation cost
template <ImU32 seed, typename PathT> struct TreeIndenter {
    TreeIndenter(const PathT &path) {
        path.GetPrefixes(&prefixes);
        for (int i = 0; i < prefixes.size(); ++i) {
            ImGui::TreePushOverrideID(ToImGuiID<seed>(prefixes[i].GetHash()));
        }
    }
    ~TreeIndenter() {
        for (int i = 0; i < prefixes.size(); ++i) {
            ImGui::TreePop();
        }
    }
    std::vector<PathT> prefixes;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H