#include <pxr/usd/sdf/payload.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/reference.h>

#include "../ui/utils.h"
#include "../ui/popup.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../command/block.h"


JVR_NAMESPACE_OPEN_SCOPE

namespace UI
{
bool
AddIconButton(const char* icon, short state, CALLBACK_FN func)
{
  ImGui::BeginGroup();
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func();
    ImGui::EndGroup();
    return true;
  }
  ImGui::EndGroup();
  return false;
}

bool
AddIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
{
  ImGui::PushID(id);
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE)) {
    func();
    ImGui::PopID();
    return true;
  }

  ImGui::PopID();
  return false;
}

bool
AddTransparentIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
{
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
  ImGui::PushID(id);
  bool clicked = false;
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func();
    clicked = true;
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
  return clicked;
}

bool
AddCheckableIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
{
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;
  const bool active = (state == ICON_SELECTED);
  if (active) {
    ImGui::PushStyleColor(ImGuiCol_Button, style->Colors[ImGuiCol_ButtonActive]);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style->Colors[ImGuiCol_ButtonActive]);
  }
  bool clicked = false;
  ImGui::PushID(id);
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func();
    clicked = true;
  }
  if (active) ImGui::PopStyleColor(2);
  ImGui::PopID();
  return clicked;
}

void 
HelpMarker(const char* desc)
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

bool 
AddComboWidget(const char* label, const char** names, const size_t count, int &last, size_t width)
{
  bool changed(false);
  size_t lw = width / 3;
  ImGui::SetNextItemWidth(lw);
  ImGui::Text("%s", label);
  ImGui::SameLine();
  ImGui::SetNextItemWidth( width - (lw + 10));
  if (ImGui::BeginCombo(HiddenLabel(label).c_str(), names[last], ImGuiComboFlags_PopupAlignLeft))
  {
    for (int n = 0; n < count; ++n)
    {
      const bool isSelected = (last == n);
      if (ImGui::Selectable(names[n], isSelected)) {
        last = n;
        changed = true;
      }

      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

bool 
AddComboWidget(const char* label, const TfToken* tokens, const size_t count, TfToken &token, size_t width)
{
  bool changed(false);
  size_t lw = width / 3;
  ImGui::SetNextItemWidth(lw);
  ImGui::Text("%s", label);
  ImGui::SameLine();

  ImGui::SetNextItemWidth( width - (lw + 10));
  if (ImGui::BeginCombo(HiddenLabel(label).c_str(), token.GetString().c_str(), ImGuiComboFlags_PopupAlignLeft))
  {
    for (int n = 0; n < count; ++n)
    {
      const bool isSelected = (tokens[n] == token);
      if (ImGui::Selectable(tokens[n].GetString().c_str(), isSelected)){
        token = tokens[n]; 
        changed = true;
      }

      if (isSelected) 
        ImGui::SetItemDefaultFocus();
    }

    ImGui::EndCombo();
  }
  return changed;
}

VtValue 
AddTokenWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode)
{
  VtValue allowedTokens;
  attribute.GetMetadata(TfToken("allowedTokens"), &allowedTokens);
  TfToken token;
  attribute.Get<TfToken>(&token, timeCode);
  VtValue newToken;
  if (!allowedTokens.IsEmpty() && allowedTokens.IsHolding<VtArray<TfToken>>()) {
    VtArray<TfToken> tokensArray = allowedTokens.Get<VtArray<TfToken>>();
    if (ImGui::BeginCombo(attribute.GetName().GetText(), token.GetText())) {
      for (auto token : tokensArray) {
        if (ImGui::Selectable(token.GetString().c_str(), false)) {
          newToken = token;
        }
      }
      ImGui::EndCombo();
    }
  } else {
    std::string tokenAsString = token.GetString();
    static char buffer[255];
    strcpy(&buffer[0], tokenAsString.c_str());
    ImGui::InputText(attribute.GetName().GetText(), &buffer[0], 255);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      newToken = TfToken(buffer);
    }
  }
  return newToken;
}

VtValue 
AddAttributeWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode) 
{
  VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<GfVec3f>()) {
    return AddVectorWidget<GfVec3f, ImGuiDataType_Float, 3>(attribute, timeCode);
  } else if (value.IsHolding<GfVec2f>()) {
    return AddVectorWidget<GfVec2f, ImGuiDataType_Float, 2>(attribute, timeCode);
  } else if (value.IsHolding<GfVec4f>()) {
    return AddVectorWidget<GfVec4f, ImGuiDataType_Float, 4>(attribute, timeCode);
  } else if (value.IsHolding<GfVec3d>()) {
    return AddVectorWidget<GfVec3d, ImGuiDataType_Double, 3>(attribute, timeCode);
  } else if (value.IsHolding<GfVec2d>()) {
    return AddVectorWidget<GfVec2d, ImGuiDataType_Double, 2>(attribute, timeCode);
  } else if (value.IsHolding<GfVec4d>()) {
    return AddVectorWidget<GfVec4d, ImGuiDataType_Double, 4>(attribute, timeCode);
  } else if (value.IsHolding<GfVec4i>()) {
    return AddVectorWidget<GfVec4i, ImGuiDataType_S32, 4>(attribute, timeCode);
  } else if (value.IsHolding<GfVec3i>()) {
    return AddVectorWidget<GfVec3i, ImGuiDataType_S32, 3>(attribute, timeCode);
  } else if (value.IsHolding<GfVec2i>()) {
    return AddVectorWidget<GfVec2i, ImGuiDataType_S32, 2>(attribute, timeCode);
  } else if (value.IsHolding<bool>()) {
    bool isOn = value.Get<bool>();
    if (ImGui::Checkbox(attribute.GetName().GetText(), &isOn)) {
      return VtValue(isOn);
    }
  } else if (value.IsHolding<double>()) {
    double dblValue = value.Get<double>();
    ImGui::InputDouble(attribute.GetName().GetText(), &dblValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(dblValue);
    }
  } else if (value.IsHolding<float>()) {
    float fltValue = value.Get<float>();
    ImGui::InputFloat(attribute.GetName().GetText(), &fltValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(fltValue);
    }
  } else if (value.IsHolding<int>()) {
    int intValue = value.Get<int>();
    ImGui::InputInt(attribute.GetName().GetText(), &intValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(intValue);
    }
  } else if (value.IsHolding<TfToken>()) {
    return AddTokenWidget(attribute, timeCode);
  } else if (value.IsHolding<std::string>()) {
    std::string stringValue = value.Get<std::string>();
    static char buf[255];
    strcpy(&buf[0], stringValue.c_str());
    ImGui::InputText(attribute.GetName().GetText(), &buf[0], 255);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(std::string(buf));
    }
  } else if (value.IsHolding<SdfAssetPath>()) {
    SdfAssetPath sdfAssetPath = value.Get<SdfAssetPath>();
    std::string assetPath = sdfAssetPath.GetAssetPath();
    static char buf[255];
    strcpy(&buf[0], assetPath.c_str());
    ImGui::InputText(attribute.GetName().GetText(), &buf[0], 255);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return VtValue(std::string(buf));
    }
  } else if (value.IsHolding<GfMatrix4d>()) {
    return AddMatrixWidget<GfMatrix4d, ImGuiDataType_Double, 4, 4>(attribute, timeCode);
  } else if (value.IsHolding<GfMatrix4f>()) {
    return AddMatrixWidget<GfMatrix4f, ImGuiDataType_Float, 4, 4>(attribute, timeCode);
  } else if (value.IsHolding<GfMatrix3d>()) {
    return AddMatrixWidget<GfMatrix3d, ImGuiDataType_Double, 3, 3>(attribute, timeCode);
  } else if (value.IsHolding<GfMatrix3f>()) {
    return AddMatrixWidget<GfMatrix3f, ImGuiDataType_Float, 3, 3>(attribute, timeCode);
  } else if (value.IsHolding<GfMatrix2d>()) {
    return AddMatrixWidget<GfMatrix2d, ImGuiDataType_Double, 2, 2>(attribute, timeCode);
  } else if (value.IsHolding<GfMatrix2f>()) {
    return AddMatrixWidget<GfMatrix2f, ImGuiDataType_Float, 2, 2>(attribute, timeCode);
  } else if (value.IsArrayValued() && value.GetArraySize() == 1 && value.IsHolding<VtArray<float>>()) {
    VtArray<float> fltArray = value.Get<VtArray<float>>();
    float fltValue = fltArray[0];
    ImGui::InputFloat(attribute.GetName().GetText(), &fltValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      fltArray[0] = fltValue;
      return VtValue(fltArray);
    }
  } else if (value.IsArrayValued() && value.GetArraySize() > 5) {
    ImGui::Text("%s with %zu values", value.GetTypeName().c_str(), value.GetArraySize());
  } else {
    std::stringstream ss;
    ss << value;
    ImGui::Text("%s", ss.str().c_str());
  }
  return VtValue();
}

VtValue 
AddColorWidget(const UsdAttribute& attribute, const UsdTimeCode& timeCode)
{
  GfVec3f buffer;
  bool isArrayValued = false;
  VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<GfVec3f>()) {
    buffer = GfVec3f(value.Get<GfVec3f>());
  } else if (value.IsArrayValued() && 
    value.GetArraySize() == 1 && 
    value.IsHolding<VtArray<GfVec3f>>()) {
    VtArray<GfVec3f> array = value.Get<VtArray<GfVec3f>>();
    buffer = array[0];
    isArrayValued = true;
  } else {
    return VtValue();
  }
  ImVec4 color(buffer[0], buffer[1], buffer[2], 1.0);
  ImGui::PushStyleColor(ImGuiCol_Button, color);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
  if (ImGui::Button("##pickColor", ImVec2(24.f, 24.f))) {
    const float scrollOffsetH = ImGui::GetScrollX();
    const float scrollOffsetV = ImGui::GetScrollY();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 position(
      windowPos.x + ImGui::GetCursorPosX() - scrollOffsetH,
      windowPos.y + ImGui::GetCursorPosY() - scrollOffsetV
    );

    ColorPopupUI* popup = new ColorPopupUI((int)position[0], (int)position[1], 
      200, 300, attribute, timeCode);
    Application::Get()->AddDeferredCommand(
      std::bind(&Application::SetPopup, Application::Get(), popup)
    );
    ImGui::PopStyleColor(3);
    return VtValue();
  }
  ImGui::PopStyleColor(3);
  ImGui::SameLine();
  ImGui::InputScalarN(attribute.GetName().GetText(), ImGuiDataType_Float, buffer.data(), 3,
    NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    if (isArrayValued) {
      return VtValue(VtArray<GfVec3f>({ buffer }));
    } else {
      return VtValue(buffer);
    }
  }
  return VtValue();
}

// TODO Share code as we want to share the style of the button, but not necessarily the behaviour
// DrawMiniButton ?? in a specific file ? OptionButton ??? OptionMenuButton ??
void 
AddPropertyMiniButton(const char* btnStr, int rowId, const ImVec4& btnColor) {
  ImGui::PushID(rowId);
  ImGui::PushStyleColor(ImGuiCol_Text, btnColor);
  ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
  ImGui::SmallButton(btnStr);
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopID();
}

void 
AddAttributeDisplayName(const UsdAttribute& attribute) {
  ImGui::Text("%s", attribute.GetName().GetText());
}

void 
AddPrimKind(const SdfPrimSpecHandle& primSpec)
{

}

void 
AddPrimType(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags)
{

}

void
AddPrimSpecifier(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags)
{
  const SdfSpecifier current = primSpec->GetSpecifier();
  SdfSpecifier selected = current;
  const std::string specifierName = TfEnum::GetDisplayName(current);
  if (ImGui::BeginCombo("Specifier", specifierName.c_str(), comboFlags)) {
    for (int n = SdfSpecifierDef; n < SdfNumSpecifiers; n++) {
      const SdfSpecifier displayed = static_cast<SdfSpecifier>(n);
      const bool isSelected = (current == displayed);
      if (ImGui::Selectable(TfEnum::GetDisplayName(displayed).c_str(), isSelected)) {
        selected = displayed;
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
    }

    if (selected != current) {
      UndoBlock editBlock;
      primSpec->SetSpecifier(selected);
    }

    ImGui::EndCombo();
  }
}

void 
AddPrimInstanceable(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isInstanceable = primSpec->GetInstanceable();
  if (ImGui::Checkbox("Instanceable", &isInstanceable)) {
    UndoBlock editBlock;
    primSpec->SetInstanceable(isInstanceable);
  }
}

void AddPrimHidden(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isHidden = primSpec->GetHidden();
  if (ImGui::Checkbox("Hidden", &isHidden)) {
    UndoBlock editBlock;
    primSpec->SetHidden(isHidden);
  }
}

void AddPrimActive(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isActive = primSpec->GetActive();
  if (ImGui::Checkbox("Active", &isActive)) {
    UndoBlock editBlock;
    primSpec->SetActive(isActive);
  }
}

void AddPrimName(const SdfPrimSpecHandle& primSpec)
{
  std::string nameBuffer = primSpec->GetName();
  ImGui::InputText("Prim Name", &nameBuffer);
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    auto primName = std::string(const_cast<char*>(nameBuffer.data()));
    if (primSpec->CanSetName(primName, nullptr)) {
      UndoBlock editBlock;
      primSpec->SetName(primName, true);
    }
  }
}
}



JVR_NAMESPACE_CLOSE_SCOPE