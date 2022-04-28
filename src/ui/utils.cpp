#include <pxr/usd/sdf/primSpec.h>

#include "../ui/utils.h"
#include "../ui/popup.h"
#include "../app/view.h"
#include "../app/application.h"
#include "../command/block.h"


PXR_NAMESPACE_OPEN_SCOPE

void 
UIUtils::HelpMarker(const char* desc)
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

pxr::VtValue 
UIUtils::AddTokenWidget(const UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  VtValue allowedTokens;
  attribute.GetMetadata(TfToken("allowedTokens"), &allowedTokens);
  pxr::TfToken token;
  attribute.Get<pxr::TfToken>(&token, timeCode);
  pxr::VtValue newToken;
  if (!allowedTokens.IsEmpty() && allowedTokens.IsHolding<pxr::VtArray<pxr::TfToken>>()) {
    pxr::VtArray<pxr::TfToken> tokensArray = allowedTokens.Get<pxr::VtArray<pxr::TfToken>>();
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

pxr::VtValue 
UIUtils::AddAttributeWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode) 
{
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<pxr::GfVec3f>()) {
    return AddVectorWidget<pxr::GfVec3f, ImGuiDataType_Float, 3>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec2f>()) {
    return AddVectorWidget<pxr::GfVec2f, ImGuiDataType_Float, 2>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec4f>()) {
    return AddVectorWidget<pxr::GfVec4f, ImGuiDataType_Float, 4>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec3d>()) {
    return AddVectorWidget<pxr::GfVec3d, ImGuiDataType_Double, 3>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec2d>()) {
    return AddVectorWidget<pxr::GfVec2d, ImGuiDataType_Double, 2>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec4d>()) {
    return AddVectorWidget<pxr::GfVec4d, ImGuiDataType_Double, 4>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec4i>()) {
    return AddVectorWidget<pxr::GfVec4i, ImGuiDataType_S32, 4>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec3i>()) {
    return AddVectorWidget<pxr::GfVec3i, ImGuiDataType_S32, 3>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfVec2i>()) {
    return AddVectorWidget<pxr::GfVec2i, ImGuiDataType_S32, 2>(attribute, timeCode);
  } else if (value.IsHolding<bool>()) {
    bool isOn = value.Get<bool>();
    if (ImGui::Checkbox(attribute.GetName().GetText(), &isOn)) {
      return pxr::VtValue(isOn);
    }
  } else if (value.IsHolding<double>()) {
    double dblValue = value.Get<double>();
    ImGui::InputDouble(attribute.GetName().GetText(), &dblValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return pxr::VtValue(dblValue);
    }
  } else if (value.IsHolding<float>()) {
    float fltValue = value.Get<float>();
    ImGui::InputFloat(attribute.GetName().GetText(), &fltValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return pxr::VtValue(fltValue);
    }
  } else if (value.IsHolding<int>()) {
    int intValue = value.Get<int>();
    ImGui::InputInt(attribute.GetName().GetText(), &intValue);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return pxr::VtValue(intValue);
    }
  } else if (value.IsHolding<TfToken>()) {
    return AddTokenWidget(attribute, timeCode);
  } else if (value.IsHolding<std::string>()) {
    std::string stringValue = value.Get<std::string>();
    static char buf[255];
    strcpy(&buf[0], stringValue.c_str());
    ImGui::InputText(attribute.GetName().GetText(), &buf[0], 255);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return pxr::VtValue(std::string(buf));
    }
  } else if (value.IsHolding<SdfAssetPath>()) {
    pxr::SdfAssetPath sdfAssetPath = value.Get<pxr::SdfAssetPath>();
    std::string assetPath = sdfAssetPath.GetAssetPath();
    static char buf[255];
    strcpy(&buf[0], assetPath.c_str());
    ImGui::InputText(attribute.GetName().GetText(), &buf[0], 255);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      return pxr::VtValue(std::string(buf));
    }
  } else if (value.IsHolding<pxr::GfMatrix4d>()) {
    return AddMatrixWidget<pxr::GfMatrix4d, ImGuiDataType_Double, 4, 4>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfMatrix4f>()) {
    return AddMatrixWidget<pxr::GfMatrix4f, ImGuiDataType_Float, 4, 4>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfMatrix3d>()) {
    return AddMatrixWidget<pxr::GfMatrix3d, ImGuiDataType_Double, 3, 3>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfMatrix3f>()) {
    return AddMatrixWidget<pxr::GfMatrix3f, ImGuiDataType_Float, 3, 3>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfMatrix2d>()) {
    return AddMatrixWidget<pxr::GfMatrix2d, ImGuiDataType_Double, 2, 2>(attribute, timeCode);
  } else if (value.IsHolding<pxr::GfMatrix2f>()) {
    return AddMatrixWidget<pxr::GfMatrix2f, ImGuiDataType_Float, 2, 2>(attribute, timeCode);
  } else if (value.IsArrayValued() && value.GetArraySize() == 1 && value.IsHolding<VtArray<float>>()) {
    pxr::VtArray<float> fltArray = value.Get<VtArray<float>>();
    float fltValue = fltArray[0];
    ImGui::InputFloat(attribute.GetName().GetText(), &fltValue);
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

pxr::VtValue 
UIUtils::AddColorWidget(const UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  pxr::GfVec3f buffer;
  bool isArrayValued = false;
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<pxr::GfVec3f>()) {
    buffer = pxr::GfVec3f(value.Get<pxr::GfVec3f>());
  } else if (value.IsArrayValued() && 
    value.GetArraySize() == 1 && 
    value.IsHolding<pxr::VtArray<pxr::GfVec3f>>()) {
    pxr::VtArray<pxr::GfVec3f> array = value.Get<pxr::VtArray<pxr::GfVec3f>>();
    buffer = array[0];
    isArrayValued = true;
  } else {
    return pxr::VtValue();
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
    GetApplication()->GetActiveWindow()->SetPopup(popup);
    ImGui::PopStyleColor(3);
    return pxr::VtValue();
  }
  ImGui::PopStyleColor(3);
  ImGui::SameLine();
  ImGui::InputScalarN(attribute.GetName().GetText(), ImGuiDataType_Float, buffer.data(), 3,
    NULL, NULL, DecimalPrecision, ImGuiInputTextFlags());
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    if (isArrayValued) {
      return pxr::VtValue(pxr::VtArray<pxr::GfVec3f>({ buffer }));
    } else {
      return pxr::VtValue(buffer);
    }
  }
  return pxr::VtValue();
}

// TODO Share code as we want to share the style of the button, but not necessarily the behaviour
// DrawMiniButton ?? in a specific file ? OptionButton ??? OptionMenuButton ??
void 
UIUtils::AddPropertyMiniButton(const char* btnStr, int rowId, const ImVec4& btnColor) {
  ImGui::PushID(rowId);
  ImGui::PushStyleColor(ImGuiCol_Text, btnColor);
  ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
  ImGui::SmallButton(btnStr);
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopID();
}

void 
UIUtils::AddAttributeDisplayName(const UsdAttribute& attribute) {
  ImGui::Text(attribute.GetName().GetText());
}

void 
UIUtils::AddPrimKind(const SdfPrimSpecHandle& primSpec)
{

}

void 
UIUtils::AddPrimType(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags)
{

}

void
UIUtils::AddPrimSpecifier(const SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags)
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
UIUtils::AddPrimInstanceable(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isInstanceable = primSpec->GetInstanceable();
  if (ImGui::Checkbox("Instanceable", &isInstanceable)) {
    UndoBlock editBlock;
    primSpec->SetInstanceable(isInstanceable);
  }
}

void UIUtils::AddPrimHidden(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isHidden = primSpec->GetHidden();
  if (ImGui::Checkbox("Hidden", &isHidden)) {
    UndoBlock editBlock;
    primSpec->SetHidden(isHidden);
  }
}

void UIUtils::AddPrimActive(const SdfPrimSpecHandle& primSpec)
{
  if (!primSpec)return;
  bool isActive = primSpec->GetActive();
  if (ImGui::Checkbox("Active", &isActive)) {
    UndoBlock editBlock;
    primSpec->SetActive(isActive);
  }
}

void UIUtils::AddPrimName(const SdfPrimSpecHandle& primSpec)
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


/*
void UIUtils::AddPrimCompositionSummary(const SdfPrimSpecHandle& primSpec) {
  if (!primSpec || !HasComposition(primSpec))
    return;
  int menuItemId = 0;
  IterateListEditorItems(primSpec->GetReferenceList(), AddReferenceSummary, primSpec, menuItemId);
  IterateListEditorItems(primSpec->GetPayloadList(), AddPayloadSummary, primSpec, menuItemId);
  IterateListEditorItems(primSpec->GetInheritPathList(), AddInheritsSummary, primSpec, menuItemId);
  IterateListEditorItems(primSpec->GetSpecializesList(), AddSpecializesSummary, primSpec, menuItemId);
}
*/
PXR_NAMESPACE_CLOSE_SCOPE