#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/curves.h>

#include "../ui/propertyEditor.h"
#include "../ui/utils.h"
#include "../command/manager.h"
#include "../geometry/utils.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/selection.h"
#include "../app/time.h"
#include "../app/commands.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PropertyEditorUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;

PropertyEditorUI::PropertyEditorUI(View* parent)
  : BaseUI(parent, UIType::PROPERTYEDITOR)
  , _focused(-1)
{
}

PropertyEditorUI::~PropertyEditorUI()
{
}

void 
PropertyEditorUI::SetPrim(const UsdPrim& prim)
{
  if(prim.IsValid())_prim = prim;
  _initialized = false;
}

void 
PropertyEditorUI::OnSelectionChangedNotice(const SelectionChangedNotice& n)
{
  Application* app = Application::Get();
  Selection* selection = app->GetModel()->GetSelection();
  if (selection->GetNumSelectedItems()) {
    UsdPrim prim = app->GetModel()->GetStage()->GetPrimAtPath(selection->GetSelectedPaths().back());
    SetPrim(prim);
  }
  else {
    SetPrim(UsdPrim());
  }
}

bool 
PropertyEditorUI::_DrawAssetInfo(const UsdPrim& prim) 
{
  VtDictionary assetInfo = prim.GetAssetInfo();
  if (assetInfo.empty())
    return false;

  if (ImGui::BeginTable("##DrawAssetInfo", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
    ImGui::TableSetupColumn("Asset info");
    ImGui::TableSetupColumn("");

    ImGui::TableHeadersRow();

    TF_FOR_ALL(keyValue, assetInfo) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%s", keyValue->first.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      UsdAttribute attribute = prim.GetAttribute(TfToken(keyValue->first));
      VtValue modified = UI::AddAttributeWidget(attribute, UsdTimeCode::Default());
      if (!modified.IsEmpty()) {
        UndoBlock editBlock;
        prim.SetAssetInfoByKey(TfToken(keyValue->first), modified);
      }
    }
    ImGui::EndTable();
  }
  return true;
}


void _XXX_CALLBACK__(int index)
{
  std::cout << "XXXXXXXXXX CALLBACK !!!" << index << std::endl;
}

// TODO Share the code,
static void DrawPropertyMiniButton(ImGuiID id=0)
{
  UI::AddIconButton(
    id, ICON_FA_GEAR, ICON_DEFAULT,
    std::bind(_XXX_CALLBACK__, id));
  ImGui::SameLine();
}

void
PropertyEditorUI::_DrawAttributeTypeInfo(const UsdAttribute& attribute) 
{
  SdfValueTypeName attributeTypeName = attribute.GetTypeName();
  TfToken attributeRoleName = attribute.GetRoleName();
  ImGui::Text("%s(%s)", attributeRoleName.GetString().c_str(), 
    attributeTypeName.GetAsToken().GetString().c_str());
}

bool 
PropertyEditorUI::_DrawXformsCommon(UsdTimeCode time)
{
  UsdGeomXformCommonAPI xformApi(_prim);

  if (xformApi) {
    ManipTargetDescList targets(1);
    ManipTargetDesc& target = targets[0];
    target.path = _prim.GetPath();
    _GetManipTargetXformVectors(xformApi, target.previous, time);
    _GetManipTargetXformVectors(xformApi, target.current, time);

    Window* window = _parent->GetWindow();
    
    if (ImGui::BeginTable("##DrawXformsCommon", 3, 
      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      //ImGui::PushFont(window->GetMediumFont(1));
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
      ImGui::TableSetupColumn("Transform");
      ImGui::TableSetupColumn("Value");
      //ImGui::PopFont();

      ImGui::TableHeadersRow();

      //ImGui::PushFont(window->GetRegularFont(1));

      // Translate
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton(1);

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("translation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputScalarN("Translation", ImGuiDataType_Double, 
        target.current.translation.data(), 3, NULL, NULL, DecimalPrecision);
      
      if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Tab))) {
        ADD_COMMAND(TranslateCommand, Application::Get()->GetModel()->GetStage(), targets, time);
      }

      // Rotation
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton(2);

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("rotation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Rotation", target.current.rotation.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Tab))) {
        ADD_COMMAND(RotateCommand, Application::Get()->GetModel()->GetStage(), targets, time);
      }
      // Scale
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton(3);

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("scale");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Scale", target.current.scale.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Tab))) {
        ADD_COMMAND(ScaleCommand, Application::Get()->GetModel()->GetStage(), targets, time);
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton(4);

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("pivot");

      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat3("Pivot", target.current.pivot.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Tab))) {
        ADD_COMMAND(PivotCommand, Application::Get()->GetModel()->GetStage(), targets, time);
      }

      // TODO rotation order
      ImGui::EndTable();
      //ImGui::PopFont();
    }
    
    return true;
  }
  return false;
}


VtValue 
PropertyEditorUI::_DrawAttributeValue(const UsdAttribute& attribute, 
  const UsdTimeCode& timeCode) 
{
  VtValue value;
  attribute.Get(&value, timeCode);
  if(value.IsHolding<TfToken>()) {
    return UI::AddTokenWidget(attribute, timeCode);
  } else if (attribute.GetRoleName() == TfToken("Color")) {
    return UI::AddColorWidget(attribute, timeCode);
  }
  return UI::AddAttributeWidget(attribute, timeCode);
}

void 
PropertyEditorUI::_DrawAttributeValueAtTime(const UsdAttribute& attribute, 
  const UsdTimeCode& currentTime) 
{
  const ImGuiStyle& style = ImGui::GetStyle();
  VtValue value;
  const bool hasValue = attribute.Get(&value, currentTime);
  const bool hasConnections = attribute.HasAuthoredConnections();
  
  if (hasConnections) {
    SdfPathVector sources;
    attribute.GetConnections(&sources);
    for (SdfPath& connection : sources) {
      ImGui::PushID(connection.GetString().c_str());
      if (ImGui::Button("DELETE")) {
        UndoBlock editBlock;
        attribute.RemoveConnection(connection);
      }
      ImGui::SameLine();
      ImGui::TextColored(style.Colors[ImGuiCol_ButtonActive], " %s", 
        connection.GetString().c_str());
      ImGui::PopID();
    }
  }

  else if (hasValue) {
    VtValue modified = _DrawAttributeValue(attribute, currentTime);
    if (!modified.IsEmpty()) {
      UsdAttributeVector attributes = { attribute };
      ADD_COMMAND(SetAttributeCommand, attributes, modified, value, 
        attribute.GetNumTimeSamples() ? currentTime : UsdTimeCode::Default());
      _parent->SetInteracting(true);
    }
  }

  if (!hasValue && !hasConnections) {
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 0.5), "no value");
  }
}

bool 
PropertyEditorUI::_DrawVariantSetsCombos(UsdPrim &prim) 
{
  /*
  int buttonID = 0;
  if (!prim.HasVariantSets())
    return false;
  auto variantSets = prim.GetVariantSets();

  if (ImGui::BeginTable("##DrawVariantSetsCombos", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
    ImGui::TableSetupColumn("VariantSet");
    ImGui::TableSetupColumn("");

    ImGui::TableHeadersRow();

    for (auto variantSetName : variantSets.GetNames()) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      // Variant set mini button --- TODO move code from this function
      auto variantSet = variantSets.GetVariantSet(variantSetName);
      // TODO: how do we figure out if the variant set has been edited in this edit target ?
      // Otherwise after a "Clear variant selection" the button remains green and it visually looks like it did nothing
      ImVec4 variantColor =
        variantSet.HasAuthoredVariantSelection() 
        ? ImVec4(MiniButtonAuthoredColor) 
        : ImVec4(MiniButtonUnauthoredColor);
      ImGui::PushID(buttonID++);
      DrawPropertyMiniButton("(v)", variantColor);
      ImGui::PopID();
      if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::MenuItem("Clear variant selection")) {
          //ExecuteAfterDraw(&UsdVariantSet::ClearVariantSelection, variantSet);
        }
        ImGui::EndPopup();
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%s", variantSetName.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      if (ImGui::BeginCombo(variantSetName.c_str(), variantSet.GetVariantSelection().c_str())) {
        for (auto variant : variantSet.GetVariantNames()) {
          if (ImGui::Selectable(variant.c_str(), false)) {
            //ExecuteAfterDraw(&UsdVariantSet::SetVariantSelection, variantSet, variant);
          }
        }
        ImGui::EndCombo();
      }
    }
    ImGui::EndTable();
  }
  */
  return true;
}

bool 
PropertyEditorUI::Draw()
{
  
  if (!_initialized)_initialized = true;

  Time& time = *Time::Get();
  
  bool opened;
  const GfVec2f pos(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);
  
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    pos,
    pos + size,
    ImGuiCol_WindowBg
  );

  if (_prim.IsValid()) {
    if (_prim.IsA<UsdGeomXform>()) {
      UsdGeomXform xfo(_prim);
      TfTokenVector attrNames = xfo.GetSchemaAttributeNames();
    }

    if (_DrawAssetInfo(_prim))
      ImGui::Separator();

    if (_DrawXformsCommon(UsdTimeCode::Default()))
      ImGui::Separator();

    if (ImGui::BeginTable("##DrawPropertyEditorTable", 3,
      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24);
      ImGui::TableSetupColumn("Property name");
      ImGui::TableSetupColumn("Value");
      ImGui::TableHeadersRow();

      int miniButtonId = 0;

      // Draw attributes
      for (UsdAttribute& attribute : _prim.GetAttributes()) {
        ImGui::TableNextRow(ImGuiTableRowFlags_None, 12);
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(miniButtonId++);
        //DrawPropertyMiniButton(attribute, editTarget, currentTime);
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        UI::AddAttributeDisplayName(attribute);

        ImGui::TableSetColumnIndex(2);
        ImGui::PushItemWidth(-FLT_MIN);
        _DrawAttributeValueAtTime(attribute, time.GetActiveTime());
      }

      /*
      // Draw relations
      for (auto& relationship : prim.GetRelationships()) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(miniButtonId++);
        DrawPropertyMiniButton(relationship, editTarget, currentTime);
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        DrawUsdRelationshipDisplayName(relationship);

        ImGui::TableSetColumnIndex(2);
        DrawUsdRelationshipList(relationship);
      }
      */

      ImGui::EndTable();
    }
  }

  ImGui::End();
  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemFocused();
};
  

JVR_NAMESPACE_CLOSE_SCOPE