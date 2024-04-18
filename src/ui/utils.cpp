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

void
UIUtils::IconButton(const char* icon, short state, CALLBACK_FN func)
{
  ImGui::BeginGroup();
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))func();
  ImGui::EndGroup();
}

bool
UIUtils::AddIconButton(const char* icon, short state, CALLBACK_FN func)
{
  if (ImGui::Button(icon, BUTTON_NORMAL_SIZE))
  {
    func();
    return true;
  }
  return false;
}

bool
UIUtils::AddIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
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
UIUtils::AddTransparentIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
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
UIUtils::AddCheckableIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func)
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
    Application::Get()->AddDeferredCommand(
      std::bind(&Application::SetPopup, Application::Get(), popup)
    );
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
  ImGui::Text("%s", attribute.GetName().GetText());
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
/// Create a standard UI for entering a SdfPath.
/// This is used for inherit and specialize
struct CreateSdfPathModalDialog : public ModalDialog {

    CreateSdfPathModalDialog(const SdfPrimSpecHandle &primSpec) : _primSpec(primSpec){};
    ~CreateSdfPathModalDialog() override {}

    void Draw() override {
        if (!_primSpec) {
            CloseModal();
            return;
        }
        // TODO: We will probably want to browse in the scene hierarchy to select the path
        //   create a selection tree, one day
        ImGui::Text("%s", _primSpec->GetPath().GetString().c_str());
        if (ImGui::BeginCombo("Operation", GetListEditorOperationName(_operation))) {
            for (int n = 0; n < GetListEditorOperationSize(); n++) {
                if (ImGui::Selectable(GetListEditorOperationName(n))) {
                    _operation = n;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::InputText("Target prim path", &_primPath);
        DrawOkCancelModal([=]() { OnOkCallBack(); });
    }

    virtual void OnOkCallBack() = 0;

    const char *DialogId() const override { return "Sdf path"; }

    SdfPrimSpecHandle _primSpec;
    std::string _primPath;
    int _operation = 0;
};

/// UI used to create an AssetPath having a file path to a layer and a
/// target prim path.
/// This is used by References and Payloads which share the same interface
struct CreateAssetPathModalDialog : public ModalDialog {

    CreateAssetPathModalDialog(const SdfPrimSpecHandle &primSpec) : _primSpec(primSpec){};
    ~CreateAssetPathModalDialog() override {}

    void Draw() override {
        if (!_primSpec) {
            CloseModal();
            return;
        }
        ImGui::Text("%s", _primSpec->GetPath().GetString().c_str());

        if (ImGui::BeginCombo("Operation", GetListEditorOperationName(_operation))) {
            for (int n = 0; n < GetListEditorOperationSize(); n++) {
                if (ImGui::Selectable(GetListEditorOperationName(n))) {
                    _operation = n;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::InputText("File path", &_assetPath);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FILE)) {
            ImGui::OpenPopup("Asset path browser");
        }
        if (ImGui::BeginPopupModal("Asset path browser")) {
            DrawFileBrowser();
            ImGui::Checkbox("Use relative path", &_relative);
            ImGui::Checkbox("Unix compatible", &_unixify);
            if (ImGui::Button("Use selected file")) {
                if (_relative) {
                    _assetPath = GetFileBrowserFilePathRelativeTo(_primSpec->GetLayer()->GetRealPath(), _unixify);
                } else {
                    _assetPath = GetFileBrowserFilePath();
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::InputText("Target prim path", &_primPath);
        ImGui::InputDouble("Layer time offset", &_timeOffset);
        ImGui::InputDouble("Layer time scale", &_timeScale);
        DrawOkCancelModal([=]() { OnOkCallBack(); });
    }

    virtual void OnOkCallBack() = 0;

    const char *DialogId() const override { return "Asset path"; }

    SdfLayerOffset GetLayerOffset() const {
        return (_timeScale != 1.0 || _timeOffset != 0.0) ? SdfLayerOffset(_timeOffset, _timeScale) : SdfLayerOffset();
    }

    SdfPrimSpecHandle _primSpec;
    std::string _assetPath;
    std::string _primPath;
    int _operation = 0;

    bool _relative = false;
    bool _unixify = false;
    double _timeScale = 1.0;
    double _timeOffset = 0.0;
};
*/
/*
// Inheriting, but could also be done with templates, would the code be cleaner ?
struct CreateReferenceModalDialog : public CreateAssetPathModalDialog {
    CreateReferenceModalDialog(const SdfPrimSpecHandle &primSpec) : CreateAssetPathModalDialog(primSpec) {}
    const char *DialogId() const override { return "Create reference"; }
    void OnOkCallBack() override {
        SdfReference reference(_assetPath, SdfPath(_primPath), GetLayerOffset());
        ExecuteAfterDraw<PrimCreateReference>(_primSpec, _operation, reference);
    }
};

struct CreatePayloadModalDialog : public CreateAssetPathModalDialog {
    CreatePayloadModalDialog(const SdfPrimSpecHandle &primSpec) : CreateAssetPathModalDialog(primSpec) {}
    const char *DialogId() const override { return "Create payload"; }
    void OnOkCallBack() override {
        SdfPayload payload(_assetPath, SdfPath(_primPath), GetLayerOffset());
        ExecuteAfterDraw<PrimCreatePayload>(_primSpec, _operation, payload);
    }
};

struct CreateInheritModalDialog : public CreateSdfPathModalDialog {
    CreateInheritModalDialog(const SdfPrimSpecHandle &primSpec) : CreateSdfPathModalDialog(primSpec) {}
    const char *DialogId() const override { return "Create inherit"; }
    void OnOkCallBack() override { ExecuteAfterDraw<PrimCreateInherit>(_primSpec, _operation, SdfPath(_primPath)); }
};

struct CreateSpecializeModalDialog : public CreateSdfPathModalDialog {
    CreateSpecializeModalDialog(const SdfPrimSpecHandle &primSpec) : CreateSdfPathModalDialog(primSpec) {}
    const char *DialogId() const override { return "Create specialize"; }
    void OnOkCallBack() override { ExecuteAfterDraw<PrimCreateSpecialize>(_primSpec, _operation, SdfPath(_primPath)); }
};
*/
void AddPrimCreateReference(const pxr::SdfPrimSpecHandle &primSpec) 
{ 
  //DrawModalDialog<CreateReferenceModalDialog>(primSpec); 
}

void AddPrimCreatePayload(const pxr::SdfPrimSpecHandle &primSpec) 
{ 
  //DrawModalDialog<CreatePayloadModalDialog>(primSpec); 
}

void AddPrimCreateInherit(const pxr::SdfPrimSpecHandle &primSpec) 
{ 
  //DrawModalDialog<CreateInheritModalDialog>(primSpec); 
}
void AddPrimCreateSpecialize(const pxr::SdfPrimSpecHandle &primSpec) 
{ 
  //DrawModalDialog<CreateSpecializeModalDialog>(primSpec); 
  }

/// Remove a Asset Path from a primspec
inline void RemoveAssetPathFromList(pxr::SdfPrimSpecHandle primSpec, 
  const pxr::SdfReference &item) 
{
    primSpec->GetReferenceList().RemoveItemEdits(item);
}

inline void RemoveAssetPathFromList(pxr::SdfPrimSpecHandle primSpec, 
  const pxr::SdfPayload &item) 
  {
    primSpec->GetPayloadList().RemoveItemEdits(item);
}

/////////////// Summaries used in the layer scene editor
template <typename ArcT>
inline void AddSdfPathSummary(std::string &&header, pxr::SdfListOpType operation, const pxr::SdfPath &path,
                               const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  ImGui::PushStyleColor(ImGuiCol_Button, 0);
  ImGui::PushID(menuItemId++);
  if (ImGui::Button(ICON_FA_TRASH)) {
    RemoveArc(primSpec, ArcT(path));
  }
  ImGui::SameLine();
  std::string summary = path.GetString();
  if (ImGui::SmallButton(summary.c_str())) {
    SelectArcType(primSpec, path);
  }
  if (ImGui::BeginPopupContextItem()) {
    AddSdfPathMenuItems<ArcT>(primSpec, path);
    ImGui::EndPopup();
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
}

template <typename AssetPathT>
inline void AddAssetPathSummary(std::string &&header, SdfListOpType operation, const AssetPathT &assetPath,
                                 const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  ImGui::PushStyleColor(ImGuiCol_Button, 0);
  ImGui::PushID(menuItemId++);
  if (ImGui::Button(ICON_FA_TRASH)) {
    RemoveArc(primSpec, assetPath);
  }
  ImGui::PopID();
  ImGui::SameLine();
  std::string summary = GetListEditorOperationName(operation);
  summary += " ";
  summary += assetPath.GetAssetPath().empty() ? "" : "@" + assetPath.GetAssetPath() + "@";
  summary += assetPath.GetPrimPath().GetString().empty() ? "" : "<" + assetPath.GetPrimPath().GetString() + ">";
  ImGui::PushID(menuItemId++);
  if(ImGui::Button(summary.c_str())) {
    SelectArcType(primSpec, assetPath);
  }
  if (ImGui::BeginPopupContextItem("###AssetPathMenuItems")) {
    AddAssetPathMenuItems<AssetPathT>(primSpec, assetPath);
    ImGui::EndPopup();
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
}

void AddReferenceSummary(pxr::SdfListOpType operation, const pxr::SdfReference &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  AddAssetPathSummary("References", operation, assetPath, primSpec, menuItemId);
}

void AddPayloadSummary(pxr::SdfListOpType operation, const pxr::SdfPayload &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  AddAssetPathSummary("Payloads", operation, assetPath, primSpec, menuItemId);
}

void AddInheritPathSummary(pxr::SdfListOpType operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  AddSdfPathSummary<SdfInherit>("Inherits", operation, path, primSpec, menuItemId);
}

void AddSpecializesSummary(pxr::SdfListOpType operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  AddSdfPathSummary<pxr::SdfSpecialize>("Specializes", operation, path, primSpec, menuItemId);
}


void 
AddPrimCompositionSummary(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec || !HasComposition(primSpec))
    return;
  ImGui::PushStyleColor(ImGuiCol_Button, 0);
  
  // Buttons are too far appart
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, -FLT_MIN));
  int buttonId = 0;
  // First draw the Buttons, Ref, Pay etc
  constexpr ImGuiPopupFlags buttonFlags = ImGuiPopupFlags_MouseButtonLeft;
  
  CREATE_COMPOSITION_BUTTON(Reference, Ref, Reference)
  CREATE_COMPOSITION_BUTTON(Payload, Pay, Payload)
  CREATE_COMPOSITION_BUTTON(InheritPath, Inh, InheritPath)
  CREATE_COMPOSITION_BUTTON(Specialize, Inh, Specializes)

  // TODO: stretch each paths to match the cell size. Add ellipsis at the beginning if they are too short
  // The idea is to see the relevant informations, and be able to quicly click on them
  // - another thought ... replace the common prefix by an ellipsis ? (only for asset paths)
  int itemId = 0;

  IterateListEditorItems(primSpec->GetReferenceList(), AddPathInRow<pxr::SdfReference>, primSpec, &itemId);
  IterateListEditorItems(primSpec->GetPayloadList(), AddPathInRow<pxr::SdfPayload>, primSpec, &itemId);
  IterateListEditorItems(primSpec->GetInheritPathList(), AddPathInRow<pxr::SdfInherit>, primSpec, &itemId);
  IterateListEditorItems(primSpec->GetSpecializesList(), AddPathInRow<pxr::SdfSpecialize>, primSpec, &itemId);

  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
}

JVR_NAMESPACE_CLOSE_SCOPE