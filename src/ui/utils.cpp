#include <pxr/usd/sdf/primSpec.h>

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
    GetApplication()->AddDeferredCommand(
      std::bind(&Application::SetPopup, GetApplication(), popup)
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

// 2 local struct to differentiate Inherit and Specialize which have the same underlying type
struct Inherit {};
struct Specialize {};

static bool HasComposition(const pxr::SdfPrimSpecHandle &primSpec) 
{
  return primSpec->HasReferences() || primSpec->HasPayloads() || 
    primSpec->HasInheritPaths() || primSpec->HasSpecializes();
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

/// Remove SdfPath from their lists (Inherits and Specialize)
template <typename ListT> inline 
void RemovePathFromList(const SdfPrimSpecHandle &primSpec, const SdfPath &item);
template <> inline 
void RemovePathFromList<Inherit>(const SdfPrimSpecHandle &primSpec, const SdfPath &item) 
{
    primSpec->GetInheritPathList().RemoveItemEdits(item);
}
template <> inline 
void RemovePathFromList<Specialize>(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &item) 
{
    primSpec->GetSpecializesList().RemoveItemEdits(item);
}

///
template <typename PathOriginT> 
void AddSdfPathMenuItems(const pxr::SdfPrimSpecHandle &primSpec, const pxr::SdfPath &path) {
  if (ImGui::MenuItem("Remove")) {
    if (!primSpec)
      return;
    std::function<void()> removeAssetPath = [=]() { RemovePathFromList<PathOriginT>(primSpec, path); };
    //ExecuteAfterDraw<UsdFunctionCall>(primSpec->GetLayer(), removeAssetPath);
  }
  if (ImGui::MenuItem("Copy path")) {
    ImGui::SetClipboardText(path.GetString().c_str());
  }
}

/// Draw the menu items for AssetPaths (SdfReference and SdfPayload)
template <typename AssetPathT> 
void AddAssetPathMenuItems(const pxr::SdfPrimSpecHandle &primSpec, const AssetPathT &assetPath) {

  if (ImGui::MenuItem("Inspect")) {
    auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(assetPath.GetAssetPath());
    //ExecuteAfterDraw<EditorFindOrOpenLayer>(realPath);
  }
  if (ImGui::MenuItem("Open as Stage")) {
    auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(assetPath.GetAssetPath());
    //ExecuteAfterDraw<EditorOpenStage>(realPath);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Remove")) {
    // I am not 100% sure this is safe as we copy the primSpec instead of its location
    // The command UsdFunctionCall will store it between here and the actual call, so between this time it
    // might be invalidated by another process. Unlikely but possible
    // TODO: make a command that stores the location of the prim
    if (!primSpec)
        return;
    std::function<void()> removeAssetPath = [=]() { RemoveAssetPathFromList(primSpec, assetPath); };
    //ExecuteAfterDraw<UsdFunctionCall>(primSpec->GetLayer(), removeAssetPath);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy asset path")) {
    ImGui::SetClipboardText(assetPath.GetAssetPath().c_str());
  }
}


template <typename ListT>
static void AddSdfPathRow(const char *operationName, const pxr::SdfPath &path,
  pxr::SdfPrimSpecHandle &primSpec) {
  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);
  if (ImGui::SmallButton(ICON_FA_TRASH)) { // TODO: replace with the proper mini button menu
    if (!primSpec)
      return;
    // DUPLICATED CODE that should go away with the mini button menu
    std::function<void()> removePath = [=]() { RemovePathFromList<ListT>(primSpec, path); };
    //ExecuteAfterDraw<UsdFunctionCall>(primSpec->GetLayer(), removePath);
  }
  ImGui::TableSetColumnIndex(1);
  ImGui::Text("%s", operationName);
  ImGui::TableSetColumnIndex(2);
  ImGui::Text("%s", path.GetString().c_str());
  if (ImGui::BeginPopupContextItem(path.GetString().c_str())) {
    AddSdfPathMenuItems<ListT>(primSpec, path);
    ImGui::EndPopup();
  }
}

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

void AddPrimPayloads(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec->HasPayloads())
    return;
  if (ImGui::BeginTable("##DrawPrimPayloads", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    //TableSetupColumns("", "Payloads", "", "");
    ImGui::TableHeadersRow();
    IterateListEditorItems(primSpec->GetPayloadList(), AddAssetPathRow<SdfPayload>, primSpec);
    ImGui::EndTable();
  }
}

void AddPrimReferences(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec->HasReferences())
    return;

  if (ImGui::BeginTable("##DrawPrimReferences", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    //TableSetupColumns("", "References", "", "");
    ImGui::TableHeadersRow();
    IterateListEditorItems(primSpec->GetReferenceList(), AddAssetPathRow<SdfReference>, primSpec);
    ImGui::EndTable();
  }
}

void AddPrimInherits(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec->HasInheritPaths())
    return;
  if (ImGui::BeginTable("##AddPrimInherits", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    //TableSetupColumns("", "Inherit", "");
    ImGui::TableHeadersRow();
    IterateListEditorItems(primSpec->GetInheritPathList(), AddSdfPathRow<Inherit>, primSpec);
    ImGui::EndTable();
  }
}

void AddPrimSpecializes(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec->HasSpecializes())
    return;
  if (ImGui::BeginTable("##AddPrimSpecializes", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    //TableSetupColumns("", "Specialize", "");
    ImGui::TableHeadersRow();
    IterateListEditorItems(primSpec->GetSpecializesList(), AddSdfPathRow<Specialize>, primSpec);
    ImGui::EndTable();
  }
}

void AddPrimCompositions(const pxr::SdfPrimSpecHandle &primSpec) {
  if (!primSpec || !HasComposition(primSpec))
      return;
  if (ImGui::CollapsingHeader("Composition", ImGuiTreeNodeFlags_DefaultOpen)) {
    AddPrimReferences(primSpec);
    AddPrimPayloads(primSpec);
    AddPrimInherits(primSpec);
    AddPrimSpecializes(primSpec);
  }
}

template <typename PathListT>
inline void AddSdfPathSummary(std::string &&header, const char *operation, 
  const pxr::SdfPath &path, const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) 
{
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TRANSPARENT_COLOR));
  ImGui::PushID(menuItemId++);
  if (ImGui::Button(header.empty() ? "###emptyheader" : header.c_str())) {
    //ExecuteAfterDraw<EditorInspectLayerLocation>(primSpec->GetLayer(), path);
  }
  ImGui::SameLine();
  std::string summary = path.GetString(); // TODO: add target prim and offsets
  ImGui::Selectable(summary.c_str());
  if (ImGui::BeginPopupContextItem()) {
    AddSdfPathMenuItems<PathListT>(primSpec, path);
    ImGui::EndPopup();
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
}

template <typename AssetPathT>
inline void AddAssetPathSummary(std::string &&header, const char *operation, const AssetPathT &assetPath,
                                 const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TRANSPARENT_COLOR));
  ImGui::PushID(menuItemId++);
  if (ImGui::Button(header.empty() ? "###emptyheader" : header.c_str())) {
    auto realPath = primSpec->GetLayer()->ComputeAbsolutePath(assetPath.GetAssetPath());
    auto layerOrOpen = SdfLayer::FindOrOpen(realPath);
    //ExecuteAfterDraw<EditorInspectLayerLocation>(layerOrOpen, assetPath.GetPrimPath());
  }
  ImGui::PopID();
  ImGui::SameLine();
  std::string summary(operation);
  summary += " ";
  summary += assetPath.GetAssetPath().empty() ? "" : "@" + assetPath.GetAssetPath() + "@";
  summary += assetPath.GetPrimPath().GetString().empty() ? "" : "<" + assetPath.GetPrimPath().GetString() + ">";
  ImGui::PushID(menuItemId++);
  ImGui::Text("%s", summary.c_str());
  if (ImGui::BeginPopupContextItem("###AssetPathMenuItems")) {
    AddAssetPathMenuItems(primSpec, assetPath);
    ImGui::EndPopup();
  }
  ImGui::PopID();
  ImGui::PopStyleColor();
}

void AddReferenceSummary(const char *operation, const pxr::SdfReference &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) {
   AddAssetPathSummary(ICON_FA_LINK , operation, assetPath, primSpec, menuItemId);
}

void AddPayloadSummary(const char *operation, const pxr::SdfPayload &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) {
  AddAssetPathSummary(ICON_FA_WEIGHT_HANGING, operation, assetPath, primSpec, menuItemId);
}

void AddInheritsSummary(const char *operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) {
    AddSdfPathSummary<Inherit>(ICON_FA_LINK_SLASH, operation, path, primSpec, menuItemId);
}

void AddSpecializesSummary(const char *operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId) {
  AddSdfPathSummary<Specialize>(ICON_FA_RUPEE_SIGN, operation, path, primSpec, menuItemId);
}

void AddPrimCompositionSummary(const pxr::SdfPrimSpecHandle &primSpec) {
    if (!primSpec || !HasComposition(primSpec))
        return;
    int menuItemId = 0;
    IterateListEditorItems(primSpec->GetReferenceList(), AddReferenceSummary, primSpec, menuItemId);
    IterateListEditorItems(primSpec->GetPayloadList(), AddPayloadSummary, primSpec, menuItemId);
    IterateListEditorItems(primSpec->GetInheritPathList(), AddInheritsSummary, primSpec, menuItemId);
    IterateListEditorItems(primSpec->GetSpecializesList(), AddSpecializesSummary, primSpec, menuItemId);
}

JVR_NAMESPACE_CLOSE_SCOPE