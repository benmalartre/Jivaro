#ifndef JVR_UI_UTILS_H
#define JVR_UI_UTILS_H
#include <cassert>

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

#include "../common.h"
#include "../ui/style.h"
#include "../ui/fonts.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_stdlib.h"


JVR_NAMESPACE_OPEN_SCOPE

const pxr::GfVec2f BUTTON_LARGE_SIZE(64.f, 64.f);
const pxr::GfVec2f BUTTON_NORMAL_SIZE(24.f, 24.f);
const pxr::GfVec2f BUTTON_MINI_SIZE(14.f, 14.f);

class View;
class UIUtils {
public:
  // callback prototype
  typedef void(*CALLBACK_FN)(...);

  static void HelpMarker(const char* desc);

  static void AddAttributeDisplayName(const pxr::UsdAttribute& attribute);
  static pxr::VtValue AddTokenWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddColorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddAttributeWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename VectorType, int DataType, int N>
  static pxr::VtValue AddVectorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename MatrixType, int DataType, int Rows, int Cols>
  static pxr::VtValue AddMatrixWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template<typename FuncT, typename ...ArgsT>
  static void IconButton(const char* icon, short state, FuncT func, ArgsT... args);

  template<typename FuncT, typename ...ArgsT>
  static bool AddIconButton(const char* icon, short state, FuncT func, ArgsT... args);

  template<typename FuncT, typename ...ArgsT>
  static bool AddIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args);

  template<typename FuncT, typename ...ArgsT>
  static bool AddTransparentIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args);

  template<typename FuncT, typename ...ArgsT>
  static bool AddCheckableIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args);

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


template<typename FuncT, typename ...ArgsT>
void 
UIUtils::IconButton(const char* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::BeginGroup();
  ImGui::Button(icon, BUTTON_NORMAL_SIZE);
  ImGui::EndGroup();
}

template<typename FuncT, typename ...ArgsT>
bool 
UIUtils::AddIconButton(const char* icon, short state, FuncT func, ArgsT... args)
{
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func(args...);
    return true;
  }
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool 
UIUtils::AddIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::PushID(id);
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE)) {
    func(args...);
    ImGui::PopID();
    return true;
  }

  ImGui::PopID();
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool 
UIUtils::AddTransparentIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushID(id);
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func(args...);
    ImGui::PopID();
    return true;
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool 
UIUtils::AddCheckableIconButton(ImGuiID id, const char* icon, short state, FuncT func, ArgsT... args)
{
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;
  const bool active = (state == ICON_SELECTED);
  if(active) {
    ImGui::PushStyleColor(ImGuiCol_Button, style->Colors[ImGuiCol_ButtonActive]);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style->Colors[ImGuiCol_ButtonActive]);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
  }
  
  ImGui::PushID(id);
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func(args...);
    if(active) ImGui::PopStyleColor(3);
    ImGui::PopID();
    return true;
  }
  if (active) ImGui::PopStyleColor(3);
  ImGui::PopID();
  return false;
}

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
template <typename PolicyT, typename FuncT, typename ...ExtraArgsT>
static void 
IterateListEditorItems(const pxr::SdfListEditorProxy<PolicyT>& listEditor, const FuncT& call, ExtraArgsT... args) {
  // TODO: should we check if the list is already all explicit ??
  for (const typename PolicyT::value_type& item : listEditor.GetExplicitItems()) {
    call("explicit", item, args...);
  }
  for (const typename PolicyT::value_type& item : listEditor.GetOrderedItems()) {
    call("ordered", item, args...);
  }
  for (const typename PolicyT::value_type& item : listEditor.GetAddedItems()) {
    call("add", item, args...); // return "add" as TfToken instead ?
  }
  for (const typename PolicyT::value_type& item : listEditor.GetPrependedItems()) {
    call("prepend", item, args...);
  }
  for (const typename PolicyT::value_type& item : listEditor.GetAppendedItems()) {
    call("append", item, args...);
  }
  for (const typename PolicyT::value_type& item : listEditor.GetDeletedItems()) {
    call("delete", item, args...);
  }
}

/// The operations available on a SdfListEditor
constexpr int GetListEditorOperationSize() { return 5; }
inline const char* GetListEditorOperationName(int index) {
  constexpr const char* names[GetListEditorOperationSize()] = { "Add", "Prepend", "Append", "Remove", "Explicit" };
  return names[index];
}

template <typename PolicyT>
void CreateListEditorOperation(pxr::SdfListEditorProxy<PolicyT>&& listEditor, int operation,
  typename pxr::SdfListEditorProxy<PolicyT>::value_type item) {
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


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H