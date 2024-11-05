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

JVR_NAMESPACE_OPEN_SCOPE


class View;
namespace UI 
{
  const GfVec2f BUTTON_LARGE_SIZE(64.f, 64.f);
  const GfVec2f BUTTON_NORMAL_SIZE(24.f, 24.f);
  const GfVec2f BUTTON_MINI_SIZE(16.f, 20.f);

  /// Height of a row in the property editor
  constexpr float TABLE_ROW_DEFAULT_HEIGHT = 22.f;

  enum ItemState {
    STATE_DEFAULT,
    STATE_SELECTED,
    STATE_DISABLED
  };

  std::string BrowseFile(int x, int y, const char* folder, const char* filters[],
    const int numFilters, const char* name, bool forWriting);

  inline std::string HiddenLabel(const char* label) {
    return std::string("##").append(label);}

  void HelpMarker(const char* desc);

  bool AddComboWidget(const char* label, const char** names, 
    const size_t count, int& last, size_t width=300);
  bool AddComboWidget(const char* label, const TfToken* tokens, 
    const size_t count, TfToken& last, size_t width=300);

  void AddAttributeDisplayName(const UsdAttribute& attribute);
  VtValue AddTokenWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode);
  VtValue AddColorWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode);
  VtValue AddAttributeWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode);

  template <typename VectorType, int DataType, int N>
  VtValue AddVectorWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode);

  template <typename MatrixType, int DataType, int Rows, int Cols>
  VtValue AddMatrixWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode);

  bool AddIconButton(const char* icon, short state, CALLBACK_FN func);
  bool AddIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  bool AddTransparentIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  bool AddCheckableIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);

  void AddPropertyMiniButton(const char* btnStr, int rowId, 
    const ImVec4& btnColor = ImVec4(0.0, 0.7, 0.0, 1.0));
  void AddPrimKind(const SdfPrimSpecHandle& primSpec);
  void AddPrimType(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  void AddPrimSpecifier(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  void AddPrimInstanceable(const SdfPrimSpecHandle& primSpec);
  void AddPrimHidden(const SdfPrimSpecHandle& primSpec);
  void AddPrimActive(const SdfPrimSpecHandle& primSpec);
  void AddPrimName(const SdfPrimSpecHandle& primSpec);

  template <typename MatrixType, int DataType, int Rows, int Cols>
  VtValue 
  AddMatrixWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode)
  {
    VtValue value;
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
      return VtValue(buffer);
    }
    return VtValue();
  }

  template <typename VectorType, int DataType, int N>
  VtValue 
  AddVectorWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode)
  {
    VectorType buffer;
    attribute.Get<VectorType>(&buffer, timeCode);
    constexpr const char* format = DataType == ImGuiDataType_S32 ? "%d" : DecimalPrecision;
    ImGui::InputScalarN(attribute.GetName().GetText(), DataType, buffer.data(), N,
      NULL, NULL, format, ImGuiInputTextFlags());

    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(VectorType(buffer));
    }
    return VtValue();
  }
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H