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
#include "../command/command.h"

PXR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PropertyUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;

PropertyUI::PropertyUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
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
    pxr::UsdPrim prim = app->GetStage()->GetPrimAtPath(selection->GetSelectedPrims().back());
    SetPrim(prim);
  }
  else {
    SetPrim(pxr::UsdPrim());
  }
}

bool PropertyUI::DrawAssetInfo(const pxr::UsdPrim& prim) 
{
  auto assetInfo = prim.GetAssetInfo();
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
      VtValue modified = AddAttributeWidget(attribute, pxr::UsdTimeCode::Default());
      if (!modified.IsEmpty()) {
        std::cout << "MODIFIED : " << modified << std::endl;
        //GetApplication()->AddUsdCommand()
        //ExecuteAfterDraw(&UsdPrim::SetAssetInfoByKey, prim, TfToken(keyValue->first), modified);
      }
    }
    ImGui::EndTable();
  }
  return true;
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
    
    if (ImGui::BeginTable("##DrawXformsCommon", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
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
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("translation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      //ImGui::InputFloat3("Translation", translationf.data(), DecimalPrecision);
      ImGui::InputScalarN("Translation", ImGuiDataType_Double, target.current.translation.data(), 3, NULL, NULL, DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<TranslateCommand>(
          new TranslateCommand(GetApplication()->GetStage(), targets, time)));
      }
      // Rotation
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("rotation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Rotation", target.current.rotation.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        GetApplication()->AddCommand(std::shared_ptr<RotateCommand>(
          new RotateCommand(GetApplication()->GetStage(), targets, time))
        );
      }
      // Scale
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("scale");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Scale", target.current.scale.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetScale, xformAPI, scale, currentTime);
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("pivot");

      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat3("Pivot", target.current.pivot.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetPivot, xformAPI, pivot, currentTime);
      }
      // TODO rotation order
      ImGui::EndTable();
      ImGui::PopFont();
    }
    
    return true;
  }
  return false;
}

/*
bool 
PropertyUI::_DrawVariantSetsCombos(pxr::UsdPrim &prim) {
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
  return true;
}
*/

bool 
PropertyUI::Draw()
{
  if (!_prim)return false;
  if (!_initialized)_initialized = true;

  if (_prim.IsA<pxr::UsdGeomXform>()) {
    pxr::UsdGeomXform xfo(_prim);
    pxr::TfTokenVector attrNames = xfo.GetSchemaAttributeNames();
  }

  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);
  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
    ImGuiCol_WindowBg
  );

  if (_DrawXformsCommon(pxr::UsdTimeCode::Default()))
    ImGui::Separator();

  if (ImGui::BeginTable("##DrawPropertyEditorTable", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
    ImGui::TableSetupColumn("Property name");
    ImGui::TableSetupColumn("Value");
    ImGui::TableHeadersRow();

    int miniButtonId = 0;

    // Draw attributes
    for (auto& attribute : _prim.GetAttributes()) {
      ImGui::TableNextRow(ImGuiTableRowFlags_None, 12);
      ImGui::TableSetColumnIndex(0);
      ImGui::PushID(miniButtonId++);
      //DrawPropertyMiniButton(attribute, editTarget, currentTime);
      ImGui::PopID();

      ImGui::TableSetColumnIndex(1);
      AddAttributeDisplayName(attribute);

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN); // Right align and get rid of widget label
      AddAttributeWidget(attribute, pxr::UsdTimeCode::Default());

      // TODO: in the hint ???
      // DrawAttributeTypeInfo(attribute);
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

  ImGui::End();
  return true;
};
  

PXR_NAMESPACE_CLOSE_SCOPE