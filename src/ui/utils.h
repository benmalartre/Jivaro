#ifndef JVR_UI_UTILS_H
#define JVR_UI_UTILS_H

#include <pxr/usd/usd/attribute.h>
#include "../common.h"
#include "../utils/icons.h"
#include "../ui/style.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"


PXR_NAMESPACE_OPEN_SCOPE

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using
// a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
	
// callback prototype
typedef void(*IconPressedFunc)(...);

template<typename FuncT, typename ...ArgsT>
static void IconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::BeginGroup();
  ImGui::Image(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size)
  );
  ImGui::EndGroup();
}

template<typename FuncT, typename ...ArgsT>
bool AddIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    return true;
  }
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool AddTransparentIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    return true;
  }
  ImGui::PopStyleColor();
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool AddCheckableIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;
  if(state == ICON_SELECTED) {
    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_ACTIVE_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_ACTIVE_COLOR);
  }
  
  if (ImGui::ImageButton( 
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    if(state == ICON_SELECTED) ImGui::PopStyleColor(2);
    return true;
  }
  if(state == ICON_SELECTED) ImGui::PopStyleColor(2);
  
  return false;
}

template <typename MatrixType, int DataType, int Rows, int Cols>
pxr::VtValue AddMatrixWidget(const std::string &label, const VtValue &value) 
{
    MatrixType mat = value.Get<MatrixType>();
    bool valueChanged = false;
    ImGui::PushID(label.c_str());
    ImGui::InputScalarN("###row0", DataType, mat.data(), Cols, NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
    valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::InputScalarN("###row1", DataType, mat.data() + Cols, Cols, NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
    valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    if /* constexpr */ (Rows > 2) {
        ImGui::InputScalarN("###row2", DataType, mat.data() + 2 * Cols, Cols, NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
        valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
        if /*constexpr */ (Rows > 3) {
            ImGui::InputScalarN("###row3", DataType, mat.data() + 3 * Cols, Cols, NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
            valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
        }
    }
    ImGui::PopID();
    if (valueChanged) {
        return pxr::VtValue(MatrixType(mat));
    }
    return pxr::VtValue();
}

template <typename VectorType, int DataType, int N>
pxr::VtValue AddVectorWidget(const std::string& label, const VtValue& value) 
{
    VectorType buffer(value.Get<VectorType>());
    constexpr const char* format = DataType == ImGuiDataType_S32 ? "%d" : DecimalPrecision;
    ImGui::InputScalarN(label.c_str(), DataType, buffer.data(), N, NULL, NULL, format, ImGuiInputTextFlags());
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        return pxr::VtValue(VectorType(buffer));
    }
    return pxr::VtValue();
}

pxr::VtValue AddTokenWidget(const std::string &label, const TfToken &token, const VtValue &allowedTokens) {
    pxr::VtValue newToken;
    if (!allowedTokens.IsEmpty() && allowedTokens.IsHolding<pxr::VtArray<pxr::TfToken>>()) {
        pxr::VtArray<pxr::TfToken> tokensArray = allowedTokens.Get<pxr::VtArray<pxr::TfToken>>();
        if (ImGui::BeginCombo(label.c_str(), token.GetString().c_str())) {
            for (auto tk : tokensArray) {
                if (ImGui::Selectable(tk.GetString().c_str(), false)) {
                    newToken = tk;
                }
            }
            ImGui::EndCombo();
        }
    } else {
        std::string tokenAsString = token.GetString();
        ImGui::InputText(label.c_str(), &tokenAsString);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            newToken = TfToken(tokenAsString);
        }
    }
    return newToken;
}

pxr::VtValue AddTokenWidget(const std::string &label, const VtValue &value, const VtValue &allowedTokens) 
{
    if (value.IsHolding<pxr::TfToken>()) {
        pxr::TfToken token = value.Get<pxr::TfToken>();
        return AddTokenWidget(label, token, allowedTokens);
    } else {
        return AddAttributeWidget(label, value);
    }
}

/// Returns the new value if the value was edited, otherwise an empty VtValue
pxr::VtValue AddAttributeWidget(const std::string &label, const VtValue &value) 
{

    if (value.IsHolding<pxr::GfVec3f>()) {
        return AddVectorWidget<pxr::GfVec3f, ImGuiDataType_Float, 3>(label, value);
    } else if (value.IsHolding<pxr::GfVec2f>()) {
        return AddVectorWidget<pxr::GfVec2f, ImGuiDataType_Float, 2>(label, value);
    } else if (value.IsHolding<pxr::GfVec4f>()) {
        return AddVectorWidget<pxr::GfVec4f, ImGuiDataType_Float, 4>(label, value);
    } else if (value.IsHolding<pxr::GfVec3d>()) {
        return AddVectorWidget<pxr::GfVec3d, ImGuiDataType_Double, 3>(label, value);
    } else if (value.IsHolding<pxr::GfVec2d>()) {
        return AddVectorWidget<pxr::GfVec2d, ImGuiDataType_Double, 2>(label, value);
    } else if (value.IsHolding<pxr::GfVec4d>()) {
        return AddVectorWidget<pxr::GfVec4d, ImGuiDataType_Double, 4>(label, value);
    } else if (value.IsHolding<pxr::GfVec4i>()) {
        return AddVectorWidget<pxr::GfVec4i, ImGuiDataType_S32, 4>(label, value);
    } else if (value.IsHolding<pxr::GfVec3i>()) {
        return AddVectorWidget<pxr::GfVec3i, ImGuiDataType_S32, 3>(label, value);
    } else if (value.IsHolding<pxr::GfVec2i>()) {
        return AddVectorWidget<pxr::GfVec2i, ImGuiDataType_S32, 2>(label, value);
    } else if (value.IsHolding<bool>()) {
        bool isOn = value.Get<bool>();
        if (ImGui::Checkbox(label.c_str(), &isOn)) {
            return pxr::VtValue(isOn);
        }
    } else if (value.IsHolding<double>()) {
        double dblValue = value.Get<double>();
        ImGui::InputDouble(label.c_str(), &dblValue);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            return pxr::VtValue(dblValue);
        }
    } else if (value.IsHolding<float>()) {
        float fltValue = value.Get<float>();
        ImGui::InputFloat(label.c_str(), &fltValue);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            return pxr::VtValue(fltValue);
        }
    } else if (value.IsHolding<int>()) {
        int intValue = value.Get<int>();
        ImGui::InputInt(label.c_str(), &intValue);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            return pxr::VtValue(intValue);
        }
    } else if (value.IsHolding<TfToken>()) {
        pxr::TfToken token = value.Get<Tpxr::fToken>();
        return AddTokenWidget(label, token);
    } else if (value.IsHolding<std::string>()) {
        std::string stringValue = value.Get<std::string>();
        ImGui::InputText(label.c_str(), &stringValue);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            return pxr::VtValue(stringValue);
        }
    } else if (value.IsHolding<SdfAssetPath>()) {
        pxr::SdfAssetPath sdfAssetPath = value.Get<pxr::SdfAssetPath>();
        std::string assetPath = sdfAssetPath.GetAssetPath();
        ImGui::InputText(label.c_str(), &assetPath);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            return pxr::VtValue(assetPath);
        }
    } else if (value.IsHolding<pxr::GfMatrix4d>()) { // Matrices are in row order
        return AddMatrixWidget<pxr::GfMatrix4d, ImGuiDataType_Double, 4, 4>(label, value);
    } else if (value.IsHolding<pxr::GfMatrix4f>()) {
        return AddMatrixWidget<pxr::GfMatrix4f, ImGuiDataType_Float, 4, 4>(label, value);
    } else if (value.IsHolding<pxr::GfMatrix3d>()) {
        return AddMatrixWidget<pxr::GfMatrix3d, ImGuiDataType_Double, 3, 3>(label, value);
    } else if (value.IsHolding<pxr::GfMatrix3f>()) {
        return AddMatrixWidget<pxr::GfMatrix3f, ImGuiDataType_Float, 3, 3>(label, value);
    } else if (value.IsHolding<pxr::GfMatrix2d>()) {
        return AddMatrixWidget<pxr::GfMatrix2d, ImGuiDataType_Double, 2, 2>(label, value);
    } else if (value.IsHolding<pxr::GfMatrix2f>()) {
        return AddMatrixWidget<pxr::GfMatrix2f, ImGuiDataType_Float, 2, 2>(label, value);
    } // TODO: Array values should be handled outside DrawVtValue
    else if (value.IsArrayValued() && value.GetArraySize() == 1 && value.IsHolding<VtArray<float>>()) {
        pxr::VtArray<float> fltArray = value.Get<VtArray<float>>();
        float fltValue = fltArray[0];
        ImGui::InputFloat(label.c_str(), &fltValue);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            fltArray[0] = fltValue;
            return pxr::VtValue(fltArray);
        }
    } else if (value.IsArrayValued() && value.GetArraySize() > 5) {
        ImGui::Text("%s with %zu values", value.GetTypeName().c_str(), value.GetArraySize());
    } else {
        std::stringstream ss;
        ss << value;
        ImGui::Text("%s", ss.str().c_str());
    }
    return pxr::VtValue();
}

pxr::VtValue AddColorWidget(const std::string &label, const VtValue &value) 
{
    if (value.IsHolding<pxr::GfVec3f>()) {
        pxr::GfVec3f buffer(value.Get<pxr::GfVec3f>());
        if (ImGui::ColorEdit3(label.c_str(), buffer.data())) {
            return VtValue(GfVec3f(buffer));
        }
    } else if (value.IsArrayValued() && value.GetArraySize() == 1 && value.IsHolding<pxr::VtArray<pxr::GfVec3f>>()) {
        pxr::VtArray<pxr::GfVec3f> colorArray = value.Get<pxr::VtArray<pxr::GfVec3f>>();
        pxr::GfVec3f colorValue = colorArray[0];
        if (ImGui::ColorEdit3(label.c_str(), colorValue.data())) {
            colorArray[0] = colorValue;
            return VtValue(colorArray);
        }
    } else {
        return DrawVtValue(label, value);
    }
    return VtValue();
}


/*
template <typename PropertyT> 
static std::string 
GetDisplayName(const PropertyT &property) {
  return property.GetNamespace().GetString() + (property.GetNamespace() == TfToken() 
    ? std::string() 
    : std::string(":")) + property.GetBaseName().GetString();
}

inline void 
RightAlignNextItem(const char *str) {
  ImGui::SetCursorPosX(
    ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 
    ImGui::CalcTextSize(str).x - ImGui::GetScrollX() -
    2 * ImGui::GetStyle().ItemSpacing.x);
}

/// Specialization for DrawPropertyMiniButton, between UsdAttribute and UsdRelashionship
template <typename UsdPropertyT> 
const char*
SmallButtonLabel();
template <> const char *SmallButtonLabel<pxr::UsdAttribute>() { return "(a)"; };
template <> const char *SmallButtonLabel<pxr::UsdRelationship>() { return "(r)"; };

template <typename UsdPropertyT> 
void 
DrawMenuClearAuthoredValues(UsdPropertyT &property){};

template <> 
void 
DrawMenuClearAuthoredValues(pxr::UsdAttribute &attribute) {
    if (attribute.IsAuthored()) {
        if (ImGui::MenuItem(ICON_FA_EJECT " Clear")) {
          //ExecuteAfterDraw(&pxr::UsdAttribute::Clear, attribute);
        }
    }
}

template <typename UsdPropertyT> 
void 
DrawMenuRemoveProperty(UsdPropertyT &property){};

template <> 
void 
DrawMenuRemoveProperty(pxr::UsdAttribute &attribute) {
    if (ImGui::MenuItem(ICON_FA_TRASH " Remove property")) {
      //ExecuteAfterDraw(&pxr::UsdPrim::RemoveProperty, attribute.GetPrim(), attribute.GetName());
    }
}

template <typename UsdPropertyT> 
void 
DrawMenuSetKey(UsdPropertyT &property, pxr::UsdTimeCode currentTime){};

template <> 
void 
DrawMenuSetKey(pxr::UsdAttribute &attribute, pxr::UsdTimeCode currentTime) {
  if (attribute.GetVariability() == pxr::SdfVariabilityVarying && 
    attribute.HasValue() && 
    ImGui::MenuItem(ICON_FA_KEY " Set key")) {
      pxr::VtValue value;
      attribute.Get(&value, currentTime);
      //ExecuteAfterDraw<AttributeSet>(attribute, value, currentTime);
  }
}


static void 
DrawPropertyMiniButton(const char *btnStr, const ImVec4 &btnColor = ImVec4(MiniButtonUnauthoredColor)) {
  ImGui::PushStyleColor(ImGuiCol_Text, btnColor);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TransparentColor));
  ImGui::SmallButton(btnStr);
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

template <typename UsdPropertyT> 
void 
DrawMenuEditConnection(pxr::UsdPropertyT &property) {}

template <> 
void 
DrawMenuEditConnection(UsdAttribute &attribute) {
  if (ImGui::MenuItem(ICON_FA_LINK " Create connection")) {
    //DrawModalDialog<CreateConnectionDialog>(attribute);
  }
}

// TODO: relationship
template <typename UsdPropertyT> 
void 
DrawMenuCreateValue(UsdPropertyT &property){};

template <> 
void 
DrawMenuCreateValue(pxr::UsdAttribute &attribute) {
  if (!attribute.HasValue()) {
    if (ImGui::MenuItem(ICON_FA_DONATE " Create value")) {
      //ExecuteAfterDraw<AttributeCreateDefaultValue>(attribute);
    }
  }
}

// Property mini button, should work with UsdProperty, UsdAttribute and UsdRelationShip
template <typename UsdPropertyT>
void 
DrawPropertyMiniButton(pxr::UsdPropertyT &property, const pxr::UsdEditTarget &editTarget, 
  pxr::UsdTimeCode currentTime) {
  ImVec4 propertyColor = property.IsAuthoredAt(editTarget) 
    ? ImVec4(MiniButtonAuthoredColor) 
    : ImVec4(MiniButtonUnauthoredColor);
  DrawPropertyMiniButton(SmallButtonLabel<UsdPropertyT>(), propertyColor);
  if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
    DrawMenuSetKey(property, currentTime);
    DrawMenuCreateValue(property);
    DrawMenuClearAuthoredValues(property);
    DrawMenuRemoveProperty(property);
    DrawMenuEditConnection(property);
    if (ImGui::MenuItem(ICON_FA_COPY " Copy attribute path")) {
      ImGui::SetClipboardText(property.GetPath().GetString().c_str());
    }
    ImGui::EndPopup();
  }
}
*/

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H