#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/curves.h>

#include "../ui/property.h"
#include "../ui/utils.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/selection.h"
#include "../app/time.h"
#include "../command/command.h"
#include "../command/block.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PropertyUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;

PropertyUI::PropertyUI(View* parent, const std::string& name)
  : HeadedUI(parent, name)
  , _focused(-1)
{
}

PropertyUI::~PropertyUI()
{
}

void 
PropertyUI::SetPrim(const pxr::UsdPrim& prim)
{
  if(prim.IsValid())_prim = prim;
  _initialized = false;
}

void 
PropertyUI::OnSelectionChangedNotice(const SelectionChangedNotice& n)
{
  Application* app = GetApplication();
  Selection* selection = app->GetSelection();
  if (selection->GetNumSelectedItems()) {
    pxr::UsdPrim prim = app->GetWorkStage()->GetPrimAtPath(selection->GetSelectedPrims().back());
    SetPrim(prim);
  }
  else {
    SetPrim(pxr::UsdPrim());
  }
}

bool 
PropertyUI::_DrawAssetInfo(const pxr::UsdPrim& prim) 
{
  pxr::VtDictionary assetInfo = prim.GetAssetInfo();
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
      pxr::UsdAttribute attribute = prim.GetAttribute(pxr::TfToken(keyValue->first));
      VtValue modified = UIUtils::AddAttributeWidget(attribute, pxr::UsdTimeCode::Default());
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
static void DrawPropertyMiniButton(ImGuiID id=0, const ImVec4& color = ImVec4(BUTTON_ACTIVE_COLOR))
{
  Icon* icon = NULL;
  icon = &ICONS[ICON_SIZE_SMALL][ICON_OP];
  UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
    id, icon, ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)_XXX_CALLBACK__, id);
  ImGui::SameLine();
}

void
PropertyUI::_DrawAttributeTypeInfo(const UsdAttribute& attribute) 
{
  pxr::SdfValueTypeName attributeTypeName = attribute.GetTypeName();
  pxr::TfToken attributeRoleName = attribute.GetRoleName();
  ImGui::Text("%s(%s)", attributeRoleName.GetString().c_str(), 
    attributeTypeName.GetAsToken().GetString().c_str());
}

bool 
PropertyUI::_DrawXformsCommon(pxr::UsdTimeCode time)
{
  pxr::UsdGeomXformCommonAPI xformApi(_prim);

  if (xformApi) {
    HandleTargetDescList targets(1);
    HandleTargetDesc& target = targets[0];
    target.path = _prim.GetPath();
    _GetHandleTargetXformVectors(xformApi, target.previous, time);
    _GetHandleTargetXformVectors(xformApi, target.current, time);

    Window* window = _parent->GetWindow();
    
    if (ImGui::BeginTable("##DrawXformsCommon", 3, 
      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      ImGui::PushFont(window->GetMediumFont(1));
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
      ImGui::TableSetupColumn("Transform");
      ImGui::TableSetupColumn("Value");
      ImGui::PopFont();

      ImGui::TableHeadersRow();

      ImGui::PushFont(window->GetRegularFont(1));

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
      
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<TranslateCommand>(
          new TranslateCommand(GetApplication()->GetWorkStage(), targets, time)));
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
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<RotateCommand>(
          new RotateCommand(GetApplication()->GetWorkStage(), targets, time))
        );
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
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<ScaleCommand>(
          new ScaleCommand(GetApplication()->GetWorkStage(), targets, time))
        );
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton(4);

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("pivot");

      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat3("Pivot", target.current.pivot.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<PivotCommand>(
          new PivotCommand(GetApplication()->GetWorkStage(), targets, time))
        );
      }

      // TODO rotation order
      ImGui::EndTable();
      ImGui::PopFont();
    }
    
    return true;
  }
  return false;
}


VtValue 
PropertyUI::_DrawAttributeValue(const UsdAttribute& attribute, 
  const pxr::UsdTimeCode& timeCode) 
{
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  if(value.IsHolding<pxr::TfToken>()) {
    return UIUtils::AddTokenWidget(attribute, timeCode);
  } else if (attribute.GetRoleName() == TfToken("Color")) {
    return UIUtils::AddColorWidget(attribute, timeCode);
  }
  return UIUtils::AddAttributeWidget(attribute, timeCode);
}

void 
PropertyUI::_DrawAttributeValueAtTime(const pxr::UsdAttribute& attribute, 
  const pxr::UsdTimeCode& currentTime) 
{
  VtValue value;
  const bool hasValue = attribute.Get(&value, currentTime);

  if (hasValue) {
    VtValue modified = _DrawAttributeValue(attribute, currentTime);
    if (!modified.IsEmpty()) {
      UndoBlock editBlock;
      attribute.Set(modified, attribute.GetNumTimeSamples() ? 
        currentTime : UsdTimeCode::Default());
      _parent->SetInteracting(true);
    }
  }

  const bool hasConnections = attribute.HasAuthoredConnections();
  if (hasConnections) {
    pxr::SdfPathVector sources;
    attribute.GetConnections(&sources);
    for (pxr::SdfPath& connection : sources) {
      ImGui::PushID(connection.GetString().c_str());
      if (ImGui::Button("DELETE")) {
        UndoBlock editBlock;
        attribute.RemoveConnection(connection);
      }
      ImGui::SameLine();
      ImGui::TextColored(TEXT_SELECTED_COLOR, " %s", 
        connection.GetString().c_str());
      ImGui::PopID();
    }
  }

  if (!hasValue && !hasConnections) {
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 0.5), "no value");
  }
}

bool 
PropertyUI::_DrawVariantSetsCombos(pxr::UsdPrim &prim) 
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
PropertyUI::Draw()
{
  
  if (!_initialized)_initialized = true;

  Time& time = GetApplication()->GetTime();
  
  bool opened;
  const pxr::GfVec2f pos(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  ImGui::Begin(_name.c_str(), &opened, _flags);
  ImGui::SetWindowSize(size);
  ImGui::SetWindowPos(pos);
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    pos,
    pos + size,
    ImGuiCol_WindowBg
  );

  if (_prim.IsValid()) {
    if (_prim.IsA<pxr::UsdGeomXform>()) {
      pxr::UsdGeomXform xfo(_prim);
      pxr::TfTokenVector attrNames = xfo.GetSchemaAttributeNames();
    }

    if (_DrawAssetInfo(_prim))
      ImGui::Separator();

    if (_DrawXformsCommon(pxr::UsdTimeCode::Default()))
      ImGui::Separator();

    if (ImGui::BeginTable("##DrawPropertyEditorTable", 3,
      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24);
      ImGui::TableSetupColumn("Property name");
      ImGui::TableSetupColumn("Value");
      ImGui::TableHeadersRow();

      int miniButtonId = 0;

      // Draw attributes
      for (pxr::UsdAttribute& attribute : _prim.GetAttributes()) {
        ImGui::TableNextRow(ImGuiTableRowFlags_None, 12);
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(miniButtonId++);
        //DrawPropertyMiniButton(attribute, editTarget, currentTime);
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        UIUtils::AddAttributeDisplayName(attribute);

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
  return true;
};
  

JVR_NAMESPACE_CLOSE_SCOPE